# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Complete authentic sprite draw list traversal, callback, and triangle submission verifier."""
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

DLL = BUILD / 'sprite_draw_list_validation.dll'
STACK = 0x7000000
STOP  = 0x7100000
ARENA = 0x7200000

ROUTINES = {
    'Gfx_SpriteDrawListNative': {
        'rva': 0x57250,
        'size': 96,
        'sha256': '12d36bd728eefff1bcccf08ecd1e7a52c39ef5d1ce833f2baeee78a7ae52b50d'
    },
    'Gfx_SpriteDrawListCallback': {
        'rva': 0x572B0,
        'size': 1,
        'sha256': 'ae3f4619b0413d70d3004b9131c3752153074e45725be13b9a148978895e359e'
    },
    'Gfx_SubmitDrawListTriangleRequest': {
        'rva': 0x572C0,
        'size': 173,
        'sha256': 'a116b31c386c034e36c50180e8d4bf76dcde6d08064b825d0c5880373880e8de'
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
    'tools/verify_sprite_draw_list.py'
]


def inspect_original(pe):
    for name, info in ROUTINES.items():
        code = pe.get_data(info['rva'], info['size'])
        assert hashlib.sha256(code).hexdigest() == info['sha256'], f'Hash mismatch for {name}'
        md = Cs(CS_ARCH_X86, CS_MODE_32)
        instructions = list(md.disasm(code, 0x400000 + info['rva']))
        assert sum(i.size for i in instructions) == info['size'], f'Size mismatch for {name}'

    # Check relocations
    relocs = [
        (r.rva, struct.unpack('<I', pe.get_data(r.rva, 4))[0])
        for entry in pe.DIRECTORY_ENTRY_BASERELOC for r in entry.entries
    ]
    reloc_dict = dict(relocs)

    # 0x57250 relocations
    expected_draw_list_relocs = {
        0x57275: 0x50EBFC,
        0x57284: 0x4BA704,
        0x5728D: 0x50EBFC,
        0x57293: 0x50EBFC
    }
    for rva, target in expected_draw_list_relocs.items():
        assert reloc_dict.get(rva) == target, f'Relocation mismatch at 0x{rva:x}'

    # 0x572C0 relocations
    expected_submit_relocs = {
        0x572C1: 0x50EBFC, 0x572CB: 0x4BA71C, 0x572D9: 0x4BA720,
        0x572E2: 0x4BA724, 0x572ED: 0x4BA728, 0x572F3: 0x50EBFC,
        0x57300: 0x4BA72C, 0x5730B: 0x4BA730, 0x57311: 0x50EBFC,
        0x5731F: 0x4BA734, 0x57329: 0x4BA738, 0x57333: 0x4BA73C,
        0x5733D: 0x4BA740, 0x57347: 0x4BA744, 0x5734C: 0x4BA718,
        0x57356: 0x4BA748, 0x57360: 0x4BA74C
    }
    for rva, target in expected_submit_relocs.items():
        assert reloc_dict.get(rva) == target, f'Relocation mismatch at 0x{rva:x}'

    # Check padding bytes
    assert pe.get_data(0x572B1, 15) == b'\xcc' * 15
    assert pe.get_data(0x5736D, 3) == b'\xcc' * 3


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
        'GfxSpriteWorkspaceBuffer *volatile g_spriteWorkspaceBBuffer0 = 0;\n'
        'GfxSpriteWorkspaceBuffer *volatile g_spriteWorkspaceBBuffer1 = 0;\n'
        'GfxSpriteWorkspaceBuffer *volatile g_spriteWorkspaceBBuffer2 = 0;\n'
        'void Gfx_RasterizeWorkspaceATriangle(const GfxTexturedTrianglePacket *packet) { (void)packet; }\n'
        'int Gfx_CopySpriteDescriptor(GfxSpriteDescriptor *descriptor, int image_id) {\n'
        '    (void)descriptor;\n'
        '    (void)image_id;\n'
        '    return 0;\n'
        '}\n'
    )
    src_storage.write_text(storage_code, encoding='utf-8', newline='\n')

    exports = [
        'Gfx_SpriteDrawListCallback',
        'Gfx_SubmitDrawListTriangleRequest',
        'Gfx_SpriteDrawListNative',
        'g_triangleDrawListStride',
        'g_triangleWorkspacePacket',
        'g_activeSpriteRequest',
        'Gfx_RasterizeWorkspaceATriangle'
    ]

    commands = [
        [
            gcc, '-m32', '-std=c89', '-pedantic-errors', '-Wall', '-Wextra', '-Werror',
            '-O2', '-fno-inline', '-fno-optimize-sibling-calls',
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
        self.rasterizer_calls = []
        self.callback_calls = []

        # Hook downstream rasterizer
        rast_target = 0x45D8A0 if is_original else self.symbols['Gfx_RasterizeWorkspaceATriangle']
        self.uc.hook_add(UC_HOOK_CODE, self._hook_rasterizer, begin=rast_target, end=rast_target)

        # Hook callback
        cb_target = 0x4572B0 if is_original else self.symbols['Gfx_SpriteDrawListCallback']
        self.uc.hook_add(UC_HOOK_CODE, self._hook_callback, begin=cb_target, end=cb_target)

    def _hook_rasterizer(self, uc, address, size, user_data=None):
        esp = uc.reg_read(UC_X86_REG_ESP)
        packet_ptr = struct.unpack('<I', uc.mem_read(esp + 4, 4))[0]
        packet_data = uc.mem_read(packet_ptr, 56)
        words = struct.unpack('<14I', packet_data)
        self.rasterizer_calls.append((packet_ptr, words))
        # Return immediately from rasterizer stub
        ret_addr = struct.unpack('<I', uc.mem_read(esp, 4))[0]
        uc.reg_write(UC_X86_REG_ESP, esp + 4)
        uc.reg_write(UC_X86_REG_EIP, ret_addr)

    def _hook_callback(self, uc, address, size, user_data=None):
        esp = uc.reg_read(UC_X86_REG_ESP)
        header = struct.unpack('<I', uc.mem_read(esp + 4, 4))[0]
        self.callback_calls.append(header)
        # Note: we let the actual function execute its ret instruction

    def get_addr(self, name):
        if self.is_original:
            return 0x400000 + ROUTINES[name]['rva']
        return self.symbols[name]

    def run(self, func_name, stack_args=(), initial_eax=0, preserved_vals=None):
        target = self.get_addr(func_name)
        sp = STACK + 0x8000
        self.uc.mem_write(sp, struct.pack('<I', STOP))
        if stack_args:
            self.uc.mem_write(sp + 4, struct.pack('<' + 'I' * len(stack_args), *stack_args))

        if preserved_vals is None:
            preserved_vals = [0x11111111 * (i + 1) for i in range(len(PRESERVED))]
        for reg, val in zip(PRESERVED, preserved_vals):
            self.uc.reg_write(reg, val)

        self.uc.reg_write(UC_X86_REG_ESP, sp)
        self.uc.reg_write(UC_X86_REG_EAX, initial_eax)
        self.uc.reg_write(UC_X86_REG_EFLAGS, 2)

        self.uc.emu_start(target, STOP, timeout=5000000)

        res_regs = [self.uc.reg_read(reg) for reg in PRESERVED]
        res_eax = self.uc.reg_read(UC_X86_REG_EAX)
        res_esp = self.uc.reg_read(UC_X86_REG_ESP)
        return {
            'eax': res_eax,
            'esp': res_esp,
            'preserved': res_regs,
            'rasterizer_calls': list(self.rasterizer_calls),
            'callback_calls': list(self.callback_calls)
        }


def test_callback(orig_pe, comp_pe):
    for i in range(64):
        header = (0x10000000 * i + i * 37) & 0xFFFFFFFF
        h_orig = Harness(orig_pe, True)
        h_comp = Harness(comp_pe, False)

        r_orig = h_orig.run('Gfx_SpriteDrawListCallback', stack_args=(header,))
        r_comp = h_comp.run('Gfx_SpriteDrawListCallback', stack_args=(header,))

        assert r_orig['preserved'] == r_comp['preserved'], 'Preserved registers mismatch in callback'
        assert r_orig['esp'] == r_comp['esp'], 'Stack mismatch in callback'


def test_submit_triangle(orig_pe, comp_pe):
    rng = random.Random(0x572C0)
    for i in range(64):
        # Arena memory layout:
        # vertex0: ARENA + 0x10
        # vertex1: ARENA + 0x20
        # vertex2: ARENA + 0x30
        # attrs:   ARENA + 0x40
        # req:     ARENA + 0x100
        v0 = [rng.randint(-1000, 1000), rng.randint(-1000, 1000)]
        v1 = [rng.randint(-1000, 1000), rng.randint(-1000, 1000)]
        v2 = [rng.randint(-1000, 1000), rng.randint(-1000, 1000)]
        attrs = [
            ARENA + 0x1000, # texture
            0,              # unused
            rng.randint(0, 0xFFFF), rng.randint(0, 0xFFFF), # u0, v0
            rng.randint(0, 0xFFFF), rng.randint(0, 0xFFFF), # u1, v1
            rng.randint(0, 0xFFFF), rng.randint(0, 0xFFFF)  # u2, v2
        ]

        req = [ARENA + 0x10, ARENA + 0x20, ARENA + 0x30, ARENA + 0x40]

        h_orig = Harness(orig_pe, True)
        h_comp = Harness(comp_pe, False)

        for h, is_orig in ((h_orig, True), (h_comp, False)):
            h.uc.mem_write(ARENA + 0x10, struct.pack('<2i', *v0))
            h.uc.mem_write(ARENA + 0x20, struct.pack('<2i', *v1))
            h.uc.mem_write(ARENA + 0x30, struct.pack('<2i', *v2))
            h.uc.mem_write(ARENA + 0x40, struct.pack('<8I', *attrs))
            h.uc.mem_write(ARENA + 0x100, struct.pack('<4I', *req))

            req_global = 0x50EBFC if is_orig else h.symbols['g_activeSpriteRequest']
            h.uc.mem_write(req_global, struct.pack('<I', ARENA + 0x100))

        r_orig = h_orig.run('Gfx_SubmitDrawListTriangleRequest')
        r_comp = h_comp.run('Gfx_SubmitDrawListTriangleRequest')

        assert len(r_orig['rasterizer_calls']) == 1
        assert len(r_comp['rasterizer_calls']) == 1

        orig_packet_ptr, orig_words = r_orig['rasterizer_calls'][0]
        comp_packet_ptr, comp_words = r_comp['rasterizer_calls'][0]

        assert orig_packet_ptr == 0x4BA718
        assert comp_packet_ptr == h_comp.symbols['g_triangleWorkspacePacket']

        assert orig_words == comp_words, f'Packet mismatch: {orig_words} != {comp_words}'
        assert orig_words[0] == 18
        assert orig_words[1] == (v0[0] & 0xFFFFFFFF)
        assert orig_words[2] == (v0[1] & 0xFFFFFFFF)
        assert orig_words[3] == (v1[0] & 0xFFFFFFFF)
        assert orig_words[4] == (v1[1] & 0xFFFFFFFF)
        assert orig_words[5] == (v2[0] & 0xFFFFFFFF)
        assert orig_words[6] == (v2[1] & 0xFFFFFFFF)
        assert orig_words[7] == attrs[2]
        assert orig_words[8] == attrs[3]
        assert orig_words[9] == attrs[4]
        assert orig_words[10] == attrs[5]
        assert orig_words[11] == attrs[6]
        assert orig_words[12] == attrs[7]
        assert orig_words[13] == attrs[0]

        assert r_orig['preserved'] == r_comp['preserved']
        assert r_orig['esp'] == r_comp['esp']


def test_draw_list(orig_pe, comp_pe):
    rng = random.Random(0x57250)
    for trial in range(64):
        h_orig = Harness(orig_pe, True)
        h_comp = Harness(comp_pe, False)

        # Build random draw list structure in ARENA
        # ARENA + 0x0000: list buffer
        # ARENA + 0x2000: batch buffers
        # ARENA + 0x4000: triangle request records
        # ARENA + 0x8000: vertex & attribute buffers
        list_words = []
        num_groups = trial % 4  # 0 to 3 groups
        batch_cursor = ARENA + 0x2000
        tri_cursor = ARENA + 0x4000
        attr_cursor = ARENA + 0x8000

        for g in range(num_groups):
            group_header = rng.randint(1, 0x7FFFFFFF)
            list_words.append(group_header)
            num_batches = rng.randint(0, 3)

            for b in range(num_batches):
                list_words.append(batch_cursor)
                num_triangles = rng.randint(0, 4)
                batch_data = []

                for t in range(num_triangles):
                    v0_addr = attr_cursor
                    v1_addr = attr_cursor + 8
                    v2_addr = attr_cursor + 16
                    att_addr = attr_cursor + 24
                    attr_cursor += 64

                    v0 = [rng.randint(-500, 500), rng.randint(-500, 500)]
                    v1 = [rng.randint(-500, 500), rng.randint(-500, 500)]
                    v2 = [rng.randint(-500, 500), rng.randint(-500, 500)]
                    attrs = [ARENA + 0x1000, 0, rng.randint(0, 0xFFFF), rng.randint(0, 0xFFFF),
                             rng.randint(0, 0xFFFF), rng.randint(0, 0xFFFF),
                             rng.randint(0, 0xFFFF), rng.randint(0, 0xFFFF)]

                    v_bytes = struct.pack('<2i2i2i8I', v0[0], v0[1], v1[0], v1[1], v2[0], v2[1], *attrs)
                    h_orig.uc.mem_write(v0_addr, v_bytes)
                    h_comp.uc.mem_write(v0_addr, v_bytes)

                    batch_data.extend([v0_addr, v1_addr, v2_addr, att_addr])

                batch_data.append(0)  # batch terminator
                b_bytes = struct.pack(f'<{len(batch_data)}I', *batch_data)
                h_orig.uc.mem_write(batch_cursor, b_bytes)
                h_comp.uc.mem_write(batch_cursor, b_bytes)
                batch_cursor += len(b_bytes) + 16

            list_words.append(0)  # group batch terminator

        list_words.append(0)  # outer list terminator

        list_bytes = struct.pack(f'<{len(list_words)}I', *list_words)
        h_orig.uc.mem_write(ARENA, list_bytes)
        h_comp.uc.mem_write(ARENA, list_bytes)

        # Set stride global = 4
        h_orig.uc.mem_write(0x4BA704, struct.pack('<I', 4))
        h_comp.uc.mem_write(h_comp.symbols['g_triangleDrawListStride'], struct.pack('<I', 4))

        r_orig = h_orig.run('Gfx_SpriteDrawListNative', stack_args=(ARENA,))
        r_comp = h_comp.run('Gfx_SpriteDrawListNative', stack_args=(ARENA,))

        assert r_orig['eax'] == 1 and r_comp['eax'] == 1, 'Return value should be 1'
        assert r_orig['preserved'] == r_comp['preserved'], 'Preserved registers mismatch in draw list'
        assert r_orig['esp'] == r_comp['esp'], 'Stack mismatch in draw list'

        assert r_orig['callback_calls'] == r_comp['callback_calls'], (
            f"Callback calls mismatch: {r_orig['callback_calls']} != {r_comp['callback_calls']}"
        )

        assert len(r_orig['rasterizer_calls']) == len(r_comp['rasterizer_calls'])
        for (p_orig, w_orig), (p_comp, w_comp) in zip(r_orig['rasterizer_calls'], r_comp['rasterizer_calls']):
            assert w_orig == w_comp, f'Rasterizer call packet mismatch: {w_orig} != {w_comp}'


def main():
    target_data = verify_target()
    orig_pe = pefile.PE(data=target_data)
    inspect_original(orig_pe)

    build()
    comp_pe = pefile.PE(str(DLL))

    test_callback(orig_pe, comp_pe)
    print('PASS: 64 callback trials')

    test_submit_triangle(orig_pe, comp_pe)
    print('PASS: 64 triangle submission trials')

    test_draw_list(orig_pe, comp_pe)
    print('PASS: 64 draw list traversal trials')

    for name, info in ROUTINES.items():
        record_run(
            info['rva'], 'compilation', 'pass',
            inputs=INPUTS,
            artifact=DLL.relative_to(ROOT).as_posix(),
            cases=1,
            command='uv run tools/verify_sprite_draw_list.py',
            details={'scope': 'Authentic sprite draw list compilation', 'routine': name}
        )
        record_run(
            info['rva'], 'emulation', 'pass',
            inputs=INPUTS,
            artifact=DLL.relative_to(ROOT).as_posix(),
            cases=64,
            command='uv run tools/verify_sprite_draw_list.py',
            details={'scope': 'Differential emulation against authentic IGN_WIN.EXE', 'routine': name}
        )

    print('PASS: sprite draw list verification complete; results recorded.')


if __name__ == '__main__':
    main()
