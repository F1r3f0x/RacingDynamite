# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Complete authentic Workspace A triangle parents versus direct C89 renderer with real dependencies."""
import hashlib
import itertools
import random
import struct
import json
import shutil
import subprocess
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import UcError, UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EFLAGS
from verify_texture_spans import Session, build, validate_renderer_calls, STACK, STOP, ARENA, PRESERVED, INPUTS as RENDERER_INPUTS
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Assertions required')

DLL = BUILD / 'workspace_a_triangles_validation.dll'
ROUTINES = {
    'Gfx_RasterizeWorkspaceATriangle': (0x5d8a0, 1805, '78d4b4b04194828bc6b6683a6890568822a230369f947edda82cb6222c4640c4'),
    'Gfx_RasterizeWorkspaceAClippedTriangle': (0x5dfb0, 1561, '739bf60339f6915a8460f79fdaeb474af0426aab014e2b386bd74c2e37ac433e')
}
GLOBALS = {
    'g_triangleSpanWorkspaceAByteStep': (0x4baefc, 4),
    'g_triangleSpanWorkspaceAPackedStep': (0x4baf00, 4),
    'g_triangleSpanWorkspaceATextureWord': (0x4baf04, 4),
    'g_triangleSpanWorkspaceALeftSample': (0x4baf0c, 4),
    'g_triangleVertices': (0x4bad74, 48),
    'g_triangleTextureGradients': (0x4bada4, 8),
    'g_triangleEdgeUCorrections': (0x4badb4, 32),
    'g_triangleEdgeVCorrections': (0x4badd4, 32),
    'g_triangleGradientScratch': (0x4badf4, 32),
    'g_triangleEdgeSteps': (0x4baea4, 32),
    'g_triangleEdgeSplit': (0x4baec8, 20),
    'g_triangleEdgeContinuation': (0x4baedc, 16),
    'g_triangleViewportState': (0x63f2b0, 36),
    'g_triangleViewportFramebuffer': (0x63f2d4, 4),
    'g_triangleWorkspaceAOrdinaryPacket': (0x51aa28, 32),
    'g_triangleWorkspaceAClippedPacket': (0x51aa80, 32),
    'g_triangleWorkspaceARasterTexture': (0x51aa48, 4),
    'g_triangleWorkspaceARasterCursor': (0x51aa54, 4),
    'g_triangleWorkspaceARasterMinimumX': (0x51aa58, 4),
    'g_triangleWorkspaceARasterMaximumX': (0x51aa78, 4),
    'g_spriteWorkspaceABuffer0': (0x51aa5c, 4),
    'g_spriteWorkspaceABuffer1': (0x51aa60, 4),
    'g_spriteWorkspaceABuffer2': (0x51aa64, 4)
}
INPUTS = RENDERER_INPUTS + ['tools/verify_workspace_a_triangles.py']
TEXTURE, FRAMEBUFFER = ARENA + 0x40000, ARENA + 0x50000


class WorkspaceATriangleSession(Session):
    def __init__(self, pe, original, seed):
        super().__init__(pe, original, seed)
        if original:
            self.symbols.update({n: 0x400000 + r[0] for n, r in ROUTINES.items()})
            self.symbols.update({n: a for n, (a, _) in GLOBALS.items()})
        self.entries = {name: 0 for name in ROUTINES}
        for name in ROUTINES:
            self.uc.hook_add(UC_HOOK_CODE, self.entry, user_data=name,
                             begin=self.symbols[name], end=self.symbols[name])
        rng = random.Random(seed)
        for n, (_, size) in GLOBALS.items():
            self.uc.mem_write(self.symbols[n], rng.randbytes(size))
        self.uc.mem_map(TEXTURE, 0x30000)
        self.uc.mem_write(TEXTURE, rng.randbytes(0x30000))
        viewport = [256, 30, 1, 30, 1, 29 * 256 + 255, 64, 256, 29 * 256 + 255]
        self.uc.mem_write(self.symbols['g_triangleViewportState'], struct.pack('<9I', *viewport))
        self.uc.mem_write(self.symbols['g_triangleViewportFramebuffer'], struct.pack('<I', FRAMEBUFFER))
        for i in range(3):
            self.uc.mem_write(self.symbols['g_spriteWorkspaceABuffer' + str(i)],
                              struct.pack('<I', ARENA + 0x3000 + i * 0x3000))

    def entry(self, uc, address, size, name):
        self.entries[name] += 1

    def canonical(self, address):
        for n, (va, size) in GLOBALS.items():
            offset = address - self.symbols[n]
            if 0 <= offset < size:
                return va + offset
        return super().canonical(address)

    def invoke(self, name, args):
        self.reads, self.writes, self.fault = [], [], None
        sp = STACK + 0x8000
        self.uc.mem_write(sp, struct.pack('<' + 'I' * (len(args) + 1), STOP, *[v & 0xffffffff for v in args]))
        caller = bytes(self.uc.mem_read(sp, 0x8000))
        before = bytes(self.uc.mem_read(self.base, self.size))
        saved = [0x12345678 + i for i in range(4)]
        for reg, value in zip(PRESERVED, saved):
            self.uc.reg_write(reg, value)
        self.uc.reg_write(UC_X86_REG_ESP, sp)
        self.uc.reg_write(UC_X86_REG_EFLAGS, 2)
        exception = None
        try:
            self.uc.emu_start(self.symbols[name], STOP, count=1000000)
        except UcError as error:
            if self.fault is not None:
                exception = self.fault
            else:
                from unicorn import UC_ERR_EXCEPTION
                assert error.errno == UC_ERR_EXCEPTION
                ip = self.uc.reg_read(UC_X86_REG_EIP)
                op = next(Cs(CS_ARCH_X86, CS_MODE_32).disasm(self.uc.mem_read(ip, 16), ip)).mnemonic
                assert op in ('div', 'idiv')
                exception = op + ' #DE'
        if exception is None:
            assert self.uc.reg_read(UC_X86_REG_EIP) == STOP and self.uc.reg_read(UC_X86_REG_ESP) == sp + 4
            assert [self.uc.reg_read(reg) for reg in PRESERVED] == saved
            assert self.uc.reg_read(UC_X86_REG_EFLAGS) & 0x400 == 0
        assert bytes(self.uc.mem_read(sp, 0x8000)) == caller
        after = bytearray(self.uc.mem_read(self.base, self.size))
        for n, (_, size) in GLOBALS.items():
            offset = self.symbols[n] - self.base
            after[offset:offset + size] = before[offset:offset + size]
        assert bytes(after) == before, 'Unrelated image changed'
        return (self.reads, self.writes, exception, None,
                bytes(self.uc.mem_read(ARENA, 0x30000)) + bytes(self.uc.mem_read(TEXTURE, 0x30000)),
                tuple(bytes(self.uc.mem_read(self.symbols[n], size)) for n, (_, size) in GLOBALS.items()))


def compare_results(original, compiled, label):
    orig_reads, orig_stores, orig_fault, orig_ret, orig_arena, orig_globals = original
    comp_reads, comp_stores, comp_fault, comp_ret, comp_arena, comp_globals = compiled

    non_fb = lambda s: [x for x in s if not (FRAMEBUFFER <= x[0] < FRAMEBUFFER + 0x20000)]
    non_tex = lambda r: [x for x in r if not (TEXTURE <= x[0] < TEXTURE + 0x30000)]

    orig_filtered = (non_tex(orig_reads), non_fb(orig_stores), orig_fault, orig_ret, orig_arena, orig_globals)
    comp_filtered = (non_tex(comp_reads), non_fb(comp_stores), comp_fault, comp_ret, comp_arena, comp_globals)

    if orig_filtered == comp_filtered:
        return
    for kind, a, b in zip(('reads', 'stores', 'fault', 'return', 'arena', 'globals'), orig_filtered, comp_filtered):
        if a != b:
            if kind in ('reads', 'stores'):
                i = next((i for i, (x, y) in enumerate(zip(a, b)) if x != y), min(len(a), len(b)))
                raise AssertionError((label, kind, i, a[max(0, i - 3):i + 3], b[max(0, i - 3):i + 3]))
            raise AssertionError((label, kind, 'mismatch'))


def verify_workspace_a_triangles(record=False):
    original = pefile.PE(data=verify_target())
    for rva, size, digest in ROUTINES.values():
        assert hashlib.sha256(original.get_data(rva, size)).hexdigest() == digest
    commands = build(dll=DLL, exports=[*ROUTINES, *GLOBALS])
    compiled = pefile.PE(str(DLL))
    assert not getattr(compiled, 'DIRECTORY_ENTRY_IMPORT', [])
    validate_renderer_calls(compiled)
    rng = random.Random(0x5d8a0)
    rows = []
    shapes = [
        [(4 * 256, 4 * 256), (20 * 256, 8 * 256), (8 * 256, 20 * 256)],
        [(3 * 256, 5 * 256), (9 * 256, 20 * 256), (24 * 256, 12 * 256)],
        [(4 * 256, 5 * 256), (20 * 256, 5 * 256), (8 * 256, 20 * 256)],
        [(-3 * 256, -2 * 256), (32 * 256, 8 * 256), (8 * 256, 35 * 256)],
        [(2 * 256, 2 * 256), (4 * 256, 2 * 256), (8 * 256, 2 * 256)]
    ]
    for shape in shapes:
        for order in itertools.permutations(range(3)):
            vertices = [shape[i] for i in order]
            uv = [(i * 300 - 200, i * 170 + 80) for i in order]
            rows.append((vertices, uv))
    for seed in range(64):
        rows.append(([(rng.randint(-1000, 9000), rng.randint(-1000, 9000)) for _ in range(3)],
                     [(rng.randint(-2000, 2000), rng.randint(-2000, 2000)) for _ in range(3)]))
    legacy_shapes = [
        (4, 4, 20, 4, 4, 20), (4, 4, 4, 20, 20, 20), (8, 3, 3, 14, 25, 24),
        (8, 3, 25, 14, 3, 24), (4, 4, 5, 4, 4, 12), (4, 4, 5, 12, 4, 20),
        (4, 4, 20, 4, 12, 4), (4, 4, 4, 4, 4, 4), (8, 3, 12, 13, 16, 23)
    ]
    uv0 = [(512, 768), (4608, 1024), (1024, 4864)]
    for shape in legacy_shapes:
        for order in itertools.permutations(range(3)):
            for fraction in (0, 1, 127, 255):
                rows.append(([(shape[2 * i] * 256 + fraction, shape[2 * i + 1] * 256 + fraction) for i in order],
                             [uv0[i] for i in order]))
    coverage = {'comparisons': 0, 'followups': 0, 'faults': 0, 'frame_stores': 0}
    visits = {name: 0 for name in ROUTINES}
    for seed, (vertices, uv) in enumerate(rows):
        sessions = [WorkspaceATriangleSession(p, i == 0, seed) for i, p in enumerate((original, compiled))]
        packet = [0, *sum((list(v) for v in vertices), []), *sum((list(v) for v in uv), []), TEXTURE, 0]
        for q in sessions:
            q.uc.mem_write(ARENA + 0x100, struct.pack('<15I', *[v & 0xffffffff for v in packet]))
        results = [q.invoke('Gfx_RasterizeWorkspaceATriangle', [ARENA + 0x100]) for q in sessions]
        compare_results(*results, ('outer', seed, vertices, uv))
        coverage['comparisons'] += 1
        coverage['faults'] += int(results[0][2] is not None)
        assert sessions[0].entries == sessions[1].entries
        for name in visits:
            visits[name] += sessions[0].entries[name]
        for q in sessions:
            q.entries = {name: 0 for name in ROUTINES}
        coverage['frame_stores'] += sum(FRAMEBUFFER <= a < FRAMEBUFFER + 0x10000 for a, size, value in results[0][1])
        if results[0][2] is None:
            results = [q.invoke('Gfx_RasterizeWorkspaceATriangle', [ARENA + 0x100]) for q in sessions]
            compare_results(*results, ('followup', seed))
            assert sessions[0].entries == sessions[1].entries
            for name in visits:
                visits[name] += sessions[0].entries[name]
            coverage['comparisons'] += 1
            coverage['followups'] += 1
            coverage['faults'] += int(results[0][2] is not None)

    # Direct clipped entry consumes real prepared/sorted scratch
    for seed, (vertices, uv) in enumerate(rows[:40]):
        ordered = [[*v, *t] for v, t in zip(vertices, uv)]
        if ordered[0][1] > ordered[2][1]:
            ordered[0], ordered[2] = ordered[2], ordered[0]
        if ordered[0][1] > ordered[1][1]:
            ordered[0], ordered[1] = ordered[1], ordered[0]
        elif ordered[2][1] < ordered[1][1]:
            ordered[1], ordered[2] = ordered[2], ordered[1]
        values = [*sum((v[:2] for v in ordered), []), *sum((v[2:] for v in ordered), [])]
        min_x = min(v[0] for v in vertices)
        max_x = max(v[0] for v in vertices)
        sessions = [WorkspaceATriangleSession(p, i == 0, seed) for i, p in enumerate((original, compiled))]
        for q in sessions:
            q.uc.mem_write(q.symbols['g_triangleVertices'], struct.pack('<12i', *values))
            q.uc.mem_write(q.symbols['g_triangleWorkspaceARasterTexture'], struct.pack('<I', TEXTURE))
            q.uc.mem_write(q.symbols['g_triangleWorkspaceARasterCursor'], struct.pack('<I', FRAMEBUFFER))
            q.uc.mem_write(q.symbols['g_triangleWorkspaceARasterMinimumX'], struct.pack('<i', min_x))
            q.uc.mem_write(q.symbols['g_triangleWorkspaceARasterMaximumX'], struct.pack('<i', max_x))
        packet = [0, *sum((list(v) for v in vertices), []), *sum((list(v) for v in uv), []), TEXTURE, 0]
        for q in sessions:
            q.uc.mem_write(ARENA + 0x100, struct.pack('<15I', *[v & 0xffffffff for v in packet]))
        results = [q.invoke('Gfx_RasterizeWorkspaceAClippedTriangle', [ARENA + 0x100]) for q in sessions]
        compare_results(*results, ('clipped', seed, vertices, uv))
        coverage['comparisons'] += 1
        coverage['faults'] += int(results[0][2] is not None)
        assert sessions[0].entries == sessions[1].entries
        for name in visits:
            visits[name] += sessions[0].entries[name]

    print(f"PASS Workspace A triangles: {coverage['comparisons']} comparisons, "
          f"{coverage['followups']} followups, {coverage['faults']} faults, "
          f"{coverage['frame_stores']} frame stores, visits {visits}")

    if record:
        for rva, symbol in [(0x5d8a0, 'Gfx_RasterizeWorkspaceATriangle'),
                            (0x5dfb0, 'Gfx_RasterizeWorkspaceAClippedTriangle')]:
            record_run(rva, 'compilation', 'pass', inputs=INPUTS,
                       artifact=DLL.relative_to(ROOT).as_posix(), cases=1,
                       command='uv run tools/verify_workspace_a_triangles.py',
                       details={'routine': symbol, 'scope': 'Workspace A triangle parent compilation'})
            record_run(rva, 'emulation', 'pass', inputs=INPUTS,
                       artifact=DLL.relative_to(ROOT).as_posix(), cases=coverage['comparisons'],
                       command='uv run tools/verify_workspace_a_triangles.py',
                       details={'routine': symbol, 'scope': 'Differential emulation against authentic IGN_WIN.EXE',
                                'coverage': coverage, 'visits': visits})


def main():
    verify_workspace_a_triangles(record=True)


if __name__ == '__main__':
    main()
