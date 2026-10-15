# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Complete authentic Workspace A triangle span rasterization and slope verifier."""
import hashlib
import random
import shutil
import struct
import subprocess
from pathlib import Path

import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import (
    Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_WRITE, UC_HOOK_MEM_READ,
    UcError
)
from unicorn.x86_const import (
    UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX, UC_X86_REG_EDX,
    UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_ESP,
    UC_X86_REG_EIP, UC_X86_REG_EFLAGS
)

from windows_target import ROOT, BUILD, TARGET, verify_target, validation_symbols
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')

DLL = BUILD / 'workspace_a_spans_validation.dll'
STACK = 0x7000000
STOP  = 0x7100000
ARENA = 0x7200000

ROUTINES = {
    'Gfx_SetWorkspaceATriangleSpanState': {
        'rva': 0x5D7E0,
        'size': 20,
        'sha256': 'b44a68c97f162d2529a4c675b0b063d311c240ea0f7fee2b016b3b3edc667cd2'
    },
    'Gfx_SetWorkspaceATriangleSpanCore': {
        'rva': 0x68030,
        'size': 32,
        'sha256': '7e0f2a9e97ff168747a5b4303cb0903be73a4b4658b9cdb95b40b0ec17dfd2d0'
    },
    'Gfx_DrawWorkspaceATriangleSpan': {
        'rva': 0x5D800,
        'size': 32,
        'sha256': '9417fc7bfab5bfc310affbac2cb9e0f47ec3e6ebd52cb9a218d3873467d746a6'
    },
    'Gfx_DrawWorkspaceATriangleSpanCore': {
        'rva': 0x68050,
        'size': 118,
        'sha256': '83ce90f853837fb3ba06d863c4a0411ffd61618d479fea50d8b4b788376cc632'
    },
    'Gfx_DrawWorkspaceAClippedTriangleSpan': {
        'rva': 0x5D820,
        'size': 28,
        'sha256': '2b8f4fe6957fecf632ed99781d89d42316878a56a1fe65fed6c8cf20b81c1613'
    },
    'Gfx_DrawWorkspaceAClippedTriangleSpanCore': {
        'rva': 0x680C6,
        'size': 92,
        'sha256': '4a669ae06b40d9562996cbc52da881c4d85877b5194eee5836628ad447b75826'
    },
    'Gfx_ComputeWorkspaceATriangleSlope': {
        'rva': 0x5E5D0,
        'size': 53,
        'sha256': '3a5ff8a33d3695a02677653fd016253eea225ac790ae027f3de712cf1f890890'
    }
}

PRESERVED = [UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP]

INPUTS = [
    'decomp/src/geputget.c',
    'decomp/include/geputget.h',
    'decomp/src/renderer.c',
    'decomp/include/renderer.h',
    'decomp/modules.json',
    'decomp/target.json',
    'tools/windows_target.py',
    'tools/windows_tracking.py',
    'tools/verify_workspace_a_spans.py'
]


def inspect_original(pe):
    data = pe.get_memory_mapped_image()
    cs = Cs(CS_ARCH_X86, CS_MODE_32)
    for name, info in ROUTINES.items():
        rva = info['rva']
        size = info['size']
        expected_hash = info['sha256']
        code = data[rva:rva+size]
        actual_hash = hashlib.sha256(code).hexdigest()
        assert actual_hash == expected_hash, f'{name} hash mismatch: {actual_hash} != {expected_hash}'
        insns = list(cs.disasm(code, 0x400000 + rva))
        assert len(insns) > 0, f'{name} disasm empty'


def build():
    BUILD.mkdir(parents=True, exist_ok=True)
    gcc, linker = (shutil.which(n) for n in ('gcc', 'lld-link'))
    if not gcc or not linker:
        raise RuntimeError('Installed provisional GCC and LLD required')

    obj_renderer = BUILD / (DLL.stem + '_renderer.obj')
    obj_storage  = BUILD / (DLL.stem + '_storage.obj')
    src_storage  = BUILD / (DLL.stem + '_storage.c')

    storage_code = (
        '#include "geputget.h"\n'
        '#include "renderer.h"\n\n'
        'volatile GfxSpriteRequest *volatile g_activeSpriteRequest = 0;\n'
        'GfxSpriteWorkspaceBuffer *volatile g_spriteWorkspaceABuffer0 = 0;\n'
        'GfxSpriteWorkspaceBuffer *volatile g_spriteWorkspaceABuffer1 = 0;\n'
        'GfxSpriteWorkspaceBuffer *volatile g_spriteWorkspaceABuffer2 = 0;\n'
        'GfxSpriteWorkspaceBuffer *volatile g_spriteWorkspaceBBuffer0 = 0;\n'
        'GfxSpriteWorkspaceBuffer *volatile g_spriteWorkspaceBBuffer1 = 0;\n'
        'GfxSpriteWorkspaceBuffer *volatile g_spriteWorkspaceBBuffer2 = 0;\n'
        'int Gfx_CopySpriteDescriptor(GfxSpriteDescriptor *descriptor, int image_id) {\n'
        '    (void)descriptor;\n'
        '    (void)image_id;\n'
        '    return 0;\n'
        '}\n'
    )
    src_storage.write_text(storage_code, encoding='utf-8', newline='\n')

    exports = list(ROUTINES.keys()) + [
        'g_triangleSpanWorkspaceAByteStep',
        'g_triangleSpanWorkspaceAPackedStep',
        'g_triangleSpanWorkspaceATextureWord',
        'g_triangleSpanWorkspaceALeftSample'
    ]

    commands = [
        [
            gcc, '-m32', '-std=c89', '-pedantic-errors', '-Wall', '-Wextra', '-Werror',
            '-O2', '-fno-inline', '-fno-optimize-sibling-calls', '-fno-ipa-icf',
            '-I', str(ROOT / 'decomp/include'),
            '-c', str(ROOT / 'decomp/src/renderer.c'),
            '-o', str(obj_renderer)
        ],
        [
            gcc, '-m32', '-std=c89', '-pedantic-errors', '-Wall', '-Wextra', '-Werror',
            '-O2', '-I', str(ROOT / 'decomp/include'),
            '-c', str(src_storage),
            '-o', str(obj_storage)
        ],
        [
            linker, '/dll', '/noentry', '/nodefaultlib', '/safeseh:no', '/machine:x86',
            '/base:0x10000000', '/out:' + str(DLL),
            str(obj_renderer), str(obj_storage)
        ] + ['/export:' + n for n in exports]
    ]

    for cmd in commands:
        subprocess.run(cmd, cwd=ROOT, check=True)


class Harness:
    def __init__(self, pe, is_original):
        self.pe = pe
        self.is_original = is_original
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.size = (pe.OPTIONAL_HEADER.SizeOfImage + 4095) & ~4095
        self.uc = Uc(UC_ARCH_X86, UC_MODE_32)
        self.uc.mem_map(self.base, self.size)
        self.uc.mem_write(self.base, pe.get_memory_mapped_image())

        self.uc.mem_map(STACK, 0x10000)
        self.uc.mem_map(STOP, 0x1000)
        self.uc.mem_map(ARENA, 0x100000)

        self.symbols = validation_symbols(pe) if not is_original else {}

    def get_addr(self, name):
        if self.is_original:
            return 0x400000 + ROUTINES[name]['rva']
        return self.symbols[name]

    def get_var_addr(self, name):
        if self.is_original:
            orig_map = {
                'g_triangleSpanWorkspaceAByteStep': 0x004BAEFC,
                'g_triangleSpanWorkspaceAPackedStep': 0x004BAF00,
                'g_triangleSpanWorkspaceATextureWord': 0x004BAF04,
                'g_triangleSpanWorkspaceALeftSample': 0x004BAF0C
            }
            return orig_map[name]
        return self.symbols[name]

    def run(self, func_name, stack_args=(), regs=None, preserved_vals=None, check_preserved=True):
        target = self.get_addr(func_name)
        sp = STACK + 0x8000
        self.uc.mem_write(sp, struct.pack('<I', STOP))
        if stack_args:
            packed = [a & 0xFFFFFFFF for a in stack_args]
            self.uc.mem_write(sp + 4, struct.pack('<' + 'I' * len(packed), *packed))

        if preserved_vals is None:
            preserved_vals = [0x11111111 * (i + 1) for i in range(len(PRESERVED))]
        for reg, val in zip(PRESERVED, preserved_vals):
            self.uc.reg_write(reg, val)

        if regs:
            for reg, val in regs.items():
                self.uc.reg_write(reg, val)

        self.uc.reg_write(UC_X86_REG_ESP, sp)
        self.uc.reg_write(UC_X86_REG_EIP, target)

        exception = None
        try:
            self.uc.emu_start(target, STOP, timeout=5000000)
        except UcError as e:
            exception = e

        out_regs = {
            'eax': self.uc.reg_read(UC_X86_REG_EAX),
            'ebx': self.uc.reg_read(UC_X86_REG_EBX),
            'ecx': self.uc.reg_read(UC_X86_REG_ECX),
            'edx': self.uc.reg_read(UC_X86_REG_EDX),
            'esi': self.uc.reg_read(UC_X86_REG_ESI),
            'edi': self.uc.reg_read(UC_X86_REG_EDI),
            'ebp': self.uc.reg_read(UC_X86_REG_EBP),
            'esp': self.uc.reg_read(UC_X86_REG_ESP),
            'eflags': self.uc.reg_read(UC_X86_REG_EFLAGS)
        }

        # Check preserved registers
        if check_preserved and exception is None:
            for reg, val in zip(PRESERVED, preserved_vals):
                cur = self.uc.reg_read(reg)
                assert cur == val, f'Preserved register {reg} clobbered: {hex(cur)} != {hex(val)}'

        return out_regs, exception


def test_slope(orig_pe, comp_pe):
    rng = random.Random(0x5E5D0)
    h_orig = Harness(orig_pe, True)
    h_comp = Harness(comp_pe, False)

    for i in range(64):
        if i == 0:
            # y0 == y1, x0 > x1
            x0, y0, x1, y1 = 100, 50, 80, 50
        elif i == 1:
            # y0 == y1, x0 <= x1
            x0, y0, x1, y1 = 80, 50, 100, 50
        elif i == 2:
            # y0 == y1, x0 == x1
            x0, y0, x1, y1 = 50, 50, 50, 50
        else:
            x0 = rng.randint(-1000, 1000)
            y0 = rng.randint(0, 480)
            x1 = rng.randint(-1000, 1000)
            y1 = rng.randint(0, 480)
            if y0 == y1 and rng.random() > 0.1:
                y1 += rng.choice([-1, 1])

        r_orig, exc_orig = h_orig.run('Gfx_ComputeWorkspaceATriangleSlope', stack_args=(x0, y0, x1, y1))
        r_comp, exc_comp = h_comp.run('Gfx_ComputeWorkspaceATriangleSlope', stack_args=(x0, y0, x1, y1))

        if exc_orig or exc_comp:
            assert type(exc_orig) == type(exc_comp), f'Exception mismatch at trial {i}: {exc_orig} vs {exc_comp}'
        else:
            assert r_orig['eax'] == r_comp['eax'], f'Slope EAX mismatch at trial {i}: {hex(r_orig["eax"])} != {hex(r_comp["eax"])}'


def test_setup(orig_pe, comp_pe):
    rng = random.Random(0x5D7E0)
    h_orig = Harness(orig_pe, True)
    h_comp = Harness(comp_pe, False)

    for i in range(64):
        grad_v = rng.randint(0, 0xFFFFFFFF)
        grad_u = rng.randint(0, 0xFFFFFFFF)
        tex_word = ARENA + rng.randint(0x1000, 0x8000) & ~3

        # Test wrapper Gfx_SetWorkspaceATriangleSpanState
        h_orig.run('Gfx_SetWorkspaceATriangleSpanState', stack_args=(grad_v, grad_u, tex_word))
        h_comp.run('Gfx_SetWorkspaceATriangleSpanState', stack_args=(grad_v, grad_u, tex_word))

        for var in ['g_triangleSpanWorkspaceAByteStep', 'g_triangleSpanWorkspaceAPackedStep', 'g_triangleSpanWorkspaceATextureWord']:
            val_orig = struct.unpack('<I', h_orig.uc.mem_read(h_orig.get_var_addr(var), 4))[0]
            val_comp = struct.unpack('<I', h_comp.uc.mem_read(h_comp.get_var_addr(var), 4))[0]
            if var == 'g_triangleSpanWorkspaceAByteStep':
                assert (val_orig & 0xFF) == (val_comp & 0xFF), f'{var} mismatch: {hex(val_orig)} != {hex(val_comp)}'
            else:
                assert val_orig == val_comp, f'{var} mismatch: {hex(val_orig)} != {hex(val_comp)}'

        # Test core Gfx_SetWorkspaceATriangleSpanCore
        # In original, Gfx_SetWorkspaceATriangleSpanCore expects EAX=grad_v, EBX=grad_u, ECX=tex_word
        # In compiled C, it is cdecl (grad_v, grad_u, tex_word)
        grad_v = rng.randint(0, 0xFFFFFFFF)
        grad_u = rng.randint(0, 0xFFFFFFFF)
        tex_word = ARENA + rng.randint(0x1000, 0x8000) & ~3

        h_orig.run('Gfx_SetWorkspaceATriangleSpanCore', regs={
            UC_X86_REG_EAX: grad_v,
            UC_X86_REG_EBX: grad_u,
            UC_X86_REG_ECX: tex_word
        }, check_preserved=False)
        h_comp.run('Gfx_SetWorkspaceATriangleSpanCore', stack_args=(grad_v, grad_u, tex_word))

        for var in ['g_triangleSpanWorkspaceAByteStep', 'g_triangleSpanWorkspaceAPackedStep', 'g_triangleSpanWorkspaceATextureWord']:
            val_orig = struct.unpack('<I', h_orig.uc.mem_read(h_orig.get_var_addr(var), 4))[0]
            val_comp = struct.unpack('<I', h_comp.uc.mem_read(h_comp.get_var_addr(var), 4))[0]
            if var == 'g_triangleSpanWorkspaceAByteStep':
                assert (val_orig & 0xFF) == (val_comp & 0xFF), f'{var} mismatch: {hex(val_orig)} != {hex(val_comp)}'
            else:
                assert val_orig == val_comp, f'{var} mismatch: {hex(val_orig)} != {hex(val_comp)}'


def test_spans(orig_pe, comp_pe):
    rng = random.Random(0x5D800)
    h_orig = Harness(orig_pe, True)
    h_comp = Harness(comp_pe, False)

    tex_base = ARENA + 0x10000
    # Fill texture arena with pseudo-random pattern
    tex_pattern = bytes([rng.randint(0, 255) for _ in range(0x20000)])
    h_orig.uc.mem_write(tex_base, tex_pattern)
    h_comp.uc.mem_write(tex_base, tex_pattern)

    dest_base = ARENA + 0x40000

    for i in range(64):
        # Configure setup state first
        grad_v = rng.randint(0, 0x10000)
        grad_u = rng.randint(0, 0x10000)
        h_orig.run('Gfx_SetWorkspaceATriangleSpanState', stack_args=(grad_v, grad_u, tex_base))
        h_comp.run('Gfx_SetWorkspaceATriangleSpanState', stack_args=(grad_v, grad_u, tex_base))

        count = rng.choice([-2, -1, 0, 1, 2, 3, 4, 7, 8, 15, 16, 23, 32, 45, 64])
        packed_uv = rng.randint(0, 0xFFFFFFFF)
        integer_u = rng.randint(0, 0xFFFF)

        left_record = ARENA + 0x8000
        left_bytes = struct.pack('<III', rng.randint(0, 255), rng.randint(0, 255), rng.randint(0, 640))
        h_orig.uc.mem_write(left_record, left_bytes)
        h_comp.uc.mem_write(left_record, left_bytes)

        # Initialize framebuffer region
        fb_init = bytes([0xAA] * 256)
        dest_addr = dest_base + 128
        h_orig.uc.mem_write(dest_base, fb_init)
        h_comp.uc.mem_write(dest_base, fb_init)

        # Test ordinary wrapper Gfx_DrawWorkspaceATriangleSpan
        h_orig.run('Gfx_DrawWorkspaceATriangleSpan', stack_args=(dest_addr, count, packed_uv, integer_u, left_record))
        h_comp.run('Gfx_DrawWorkspaceATriangleSpan', stack_args=(dest_addr, count, packed_uv, integer_u, left_record))

        # Check framebuffer equality
        fb_orig = h_orig.uc.mem_read(dest_base, 256)
        fb_comp = h_comp.uc.mem_read(dest_base, 256)
        assert fb_orig == fb_comp, f'Ordinary span framebuffer mismatch at trial {i} (count={count})'

        # Check cached left sample equality
        ls_orig = h_orig.uc.mem_read(h_orig.get_var_addr('g_triangleSpanWorkspaceALeftSample'), 1)
        ls_comp = h_comp.uc.mem_read(h_comp.get_var_addr('g_triangleSpanWorkspaceALeftSample'), 1)
        assert ls_orig == ls_comp, f'Left sample mismatch at trial {i}'

        # Test ordinary core Gfx_DrawWorkspaceATriangleSpanCore
        # In original: EDI=dest, ESI=count, ECX=packed_uv, EBX=integer_u, EAX=left
        # In C: cdecl (dest, count, packed_uv, integer_u, left)
        h_orig.uc.mem_write(dest_base, fb_init)
        h_comp.uc.mem_write(dest_base, fb_init)

        h_orig.run('Gfx_DrawWorkspaceATriangleSpanCore', regs={
            UC_X86_REG_EDI: dest_addr,
            UC_X86_REG_ESI: count,
            UC_X86_REG_ECX: packed_uv,
            UC_X86_REG_EBX: integer_u,
            UC_X86_REG_EAX: left_record
        }, check_preserved=False)
        h_comp.run('Gfx_DrawWorkspaceATriangleSpanCore', stack_args=(dest_addr, count, packed_uv, integer_u, left_record))

        fb_orig = h_orig.uc.mem_read(dest_base, 256)
        fb_comp = h_comp.uc.mem_read(dest_base, 256)
        assert fb_orig == fb_comp, f'Ordinary core framebuffer mismatch at trial {i} (count={count})'

        # Test clipped wrapper Gfx_DrawWorkspaceAClippedTriangleSpan
        h_orig.uc.mem_write(dest_base, fb_init)
        h_comp.uc.mem_write(dest_base, fb_init)

        h_orig.run('Gfx_DrawWorkspaceAClippedTriangleSpan', stack_args=(dest_addr, count, packed_uv, integer_u))
        h_comp.run('Gfx_DrawWorkspaceAClippedTriangleSpan', stack_args=(dest_addr, count, packed_uv, integer_u))

        fb_orig = h_orig.uc.mem_read(dest_base, 256)
        fb_comp = h_comp.uc.mem_read(dest_base, 256)
        assert fb_orig == fb_comp, f'Clipped span framebuffer mismatch at trial {i} (count={count})'

        # Test clipped core Gfx_DrawWorkspaceAClippedTriangleSpanCore
        # In original: EDI=dest, ESI=count, ECX=packed_uv, EBX=integer_u
        # In C: cdecl (dest, count, packed_uv, integer_u)
        h_orig.uc.mem_write(dest_base, fb_init)
        h_comp.uc.mem_write(dest_base, fb_init)

        h_orig.run('Gfx_DrawWorkspaceAClippedTriangleSpanCore', regs={
            UC_X86_REG_EDI: dest_addr,
            UC_X86_REG_ESI: count,
            UC_X86_REG_ECX: packed_uv,
            UC_X86_REG_EBX: integer_u
        }, check_preserved=False)
        h_comp.run('Gfx_DrawWorkspaceAClippedTriangleSpanCore', stack_args=(dest_addr, count, packed_uv, integer_u))

        fb_orig = h_orig.uc.mem_read(dest_base, 256)
        fb_comp = h_comp.uc.mem_read(dest_base, 256)
        assert fb_orig == fb_comp, f'Clipped core framebuffer mismatch at trial {i} (count={count})'


def main():
    target_data = verify_target()
    orig_pe = pefile.PE(data=target_data)
    inspect_original(orig_pe)

    build()
    comp_pe = pefile.PE(str(DLL))

    test_slope(orig_pe, comp_pe)
    print('PASS: 64 slope trials')

    test_setup(orig_pe, comp_pe)
    print('PASS: 64 span setup trials')

    test_spans(orig_pe, comp_pe)
    print('PASS: 64 span rasterization trials')

    for name, info in ROUTINES.items():
        record_run(
            info['rva'], 'compilation', 'pass',
            inputs=INPUTS,
            artifact=DLL.relative_to(ROOT).as_posix(),
            cases=1,
            command='uv run tools/verify_workspace_a_spans.py',
            details={'scope': 'Authentic Workspace A span compilation', 'routine': name}
        )
        record_run(
            info['rva'], 'emulation', 'pass',
            inputs=INPUTS,
            artifact=DLL.relative_to(ROOT).as_posix(),
            cases=64,
            command='uv run tools/verify_workspace_a_spans.py',
            details={'scope': 'Differential emulation against authentic IGN_WIN.EXE', 'routine': name}
        )

    print('PASS: Workspace A spans verification complete; results recorded.')


if __name__ == '__main__':
    main()
