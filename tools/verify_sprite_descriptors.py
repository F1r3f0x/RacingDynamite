# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Complete authentic sprite descriptor lifecycle versus production C89 geputget routines."""
import hashlib
import random
import struct
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import (
    UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX, UC_X86_REG_EDX,
    UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_ESP,
    UC_X86_REG_EIP, UC_X86_REG_EFLAGS
)
from windows_target import ROOT, BUILD, TARGET, verify_target, validation_symbols
from windows_tracking import record_run
from build_decomp import build

if not __debug__:
    raise RuntimeError('Verification requires assertions')

DLL = BUILD / 'sprite_descriptors_validation.dll'
STACK = 0x7000000
STOP  = 0x7100000
ARENA = 0x7200000
HEAP  = ARENA + 0x10000

PRESERVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)

ROUTINES = {
    'Gfx_SpriteCreateDescriptorNative': (0x5c920, 24, 'fd5cdd32124a607730e33cc3717641031757e7524d4bc8252ea6d7a0fe506b0e'),
    'Gfx_SpriteFreeDescriptorNative': (0x5c940, 19, '6fbd0f1c63dd28502523b19906d18ca89eee90c5df9045d4c2471ca7147d7795'),
    'Gfx_SpriteCopyDescriptorNative': (0x5c960, 24, '7f21b8803e5134922b22aefa7896ec5d43d3aa997dddb3ad20939b8639a7cfc4'),
    'Gfx_CreateSpriteDescriptor': (0x61210, 143, 'd1ca418b53f65abb20e0ec43adeb04905cbd9b247a7c5c188818a512b3620dc9'),
    'Gfx_FreeSpriteDescriptor': (0x614b0, 31, '4b278fe6701f014a83c9ddb65ba7d69c2ca3c12814bedc44e9766dcd98408493'),
    'Gfx_SpriteReservedNative': (0x61a80, 3, '4bc724f3b1d0caf4fe369c18cba3102e6c4ea057f63fe1587e3973134a7f755e'),
}

EXPECTED_FPO = {
    0x5c920: (24, 0, 2, 0),
    0x5c940: (19, 0, 1, 0),
    0x5c960: (24, 0, 2, 0),
    0x61210: (143, 1, 2, 5133),
    0x614b0: (31, 0, 1, 257),
    0x61a80: (3, 0, 1, 0),
}

INPUTS = [
    'decomp/src/geputget.c',
    'decomp/include/geputget.h',
    'decomp/modules.json',
    'tools/build_decomp.py',
    'tools/verify_matching.py',
    'tools/verify_sprite_descriptors.py',
    'tools/windows_target.py',
    'tools/windows_tracking.py'
]


def inspect_original(pe):
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {r: (n, l, a, b) for r, n, l, a, b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData + debug.SizeOfData])}
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    for name, (rva, size, digest) in ROUTINES.items():
        assert fpo[rva] == EXPECTED_FPO[rva], f"FPO mismatch for {name}: {fpo[rva]} vs {EXPECTED_FPO[rva]}"
        raw = pe.get_data(rva, size)
        assert hashlib.sha256(raw).hexdigest() == digest, f"Digest mismatch for {name}"
        ins = list(md.disasm(raw, 0x400000 + rva))
        assert sum(i.size for i in ins) == size, f"Size mismatch for {name}"
        assert ins[-1].mnemonic == 'ret', f"Expected ret for {name}"

    # Verify padding bytes
    assert pe.get_data(0x5c938, 8) == b'\xcc' * 8
    assert pe.get_data(0x5c953, 13) == b'\xcc' * 13
    assert pe.get_data(0x5c978, 8) == b'\xcc' * 8
    assert pe.get_data(0x6129f, 1) == b'\xcc' * 1
    assert pe.get_data(0x614cf, 1) == b'\xcc' * 1
    assert pe.get_data(0x61a83, 13) == b'\xcc' * 13

    # Relocations in the 6 routines
    relocs = [e.rva for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries
              if e.type == 3 and any(r <= e.rva < r + sz for r, sz, _ in ROUTINES.values())]
    assert relocs == [0x61221, 0x61228, 0x61235, 0x61249], f"Unexpected relocations: {relocs}"

    # Verify call destinations in original
    def get_calls(rva, size):
        raw = pe.get_data(rva, size)
        return [(i.address, i.op_str) for i in md.disasm(raw, 0x400000 + rva) if i.mnemonic == 'call']

    assert get_calls(0x5c920, 24) == [(0x45c92a, '0x461210')]
    assert get_calls(0x5c940, 19) == [(0x45c945, '0x4614b0')]
    assert get_calls(0x5c960, 24) == [(0x45c96a, '0x4612e0')]
    assert get_calls(0x61210, 143) == [
        (0x461261, '0x460480'),
        (0x461276, '0x45f4a0'),
        (0x461289, '0x4612a0'),
    ]
    assert get_calls(0x614b0, 31) == [
        (0x4614b9, '0x45f8b0'),
        (0x4614c5, '0x45f8b0'),
    ]
    assert get_calls(0x61a80, 3) == []


class DescriptorSession:
    def __init__(self, pe, original):
        self.original = original
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.size = (pe.OPTIONAL_HEADER.SizeOfImage + 0xFFF) & ~0xFFF
        self.uc = Uc(UC_ARCH_X86, UC_MODE_32)
        self.uc.mem_map(self.base, self.size)
        self.uc.mem_write(self.base, pe.get_memory_mapped_image())
        self.uc.mem_map(STACK, 0x10000)
        self.uc.mem_map(STOP, 0x1000)
        self.uc.mem_map(ARENA, 0x40000)

        syms = validation_symbols(pe) if not original else {}
        self.symbols = {
            'Gfx_SpriteCreateDescriptorNative': 0x45c920 if original else syms['Gfx_SpriteCreateDescriptorNative'],
            'Gfx_SpriteFreeDescriptorNative': 0x45c940 if original else syms['Gfx_SpriteFreeDescriptorNative'],
            'Gfx_SpriteCopyDescriptorNative': 0x45c960 if original else syms['Gfx_SpriteCopyDescriptorNative'],
            'Gfx_CreateSpriteDescriptor': 0x461210 if original else syms['Gfx_CreateSpriteDescriptor'],
            'Gfx_FreeSpriteDescriptor': 0x4614b0 if original else syms['Gfx_FreeSpriteDescriptor'],
            'Gfx_SpriteReservedNative': 0x461a80 if original else syms['Gfx_SpriteReservedNative'],
            'g_nativeImageControl': 0x520380 if original else syms['g_nativeImageControl'],
            'g_nativeSpriteDefault': 0x51fb88 if original else syms['g_nativeSpriteDefault'],
            'malloc': 0x469400 if original else syms['malloc'],
            'free': 0x4693b0 if original else syms['free'],
        }

        self.events = []
        self.heap_cursor = HEAP
        self.uc.hook_add(UC_HOOK_CODE, self._hook_code)

    def _hook_code(self, uc, address, size, data):
        if address == self.symbols['free']:
            sp = uc.reg_read(UC_X86_REG_ESP)
            ret_addr, ptr = struct.unpack('<II', uc.mem_read(sp, 8))
            self.events.append(('free', ptr))
            uc.reg_write(UC_X86_REG_ESP, sp + 4)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)
            uc.reg_write(UC_X86_REG_EAX, 0)
            uc.reg_write(UC_X86_REG_ECX, 0xbadc0ffe)
            uc.reg_write(UC_X86_REG_EDX, 0x87654321)
        elif address == self.symbols['malloc']:
            sp = uc.reg_read(UC_X86_REG_ESP)
            ret_addr, nbytes = struct.unpack('<II', uc.mem_read(sp, 8))
            ptr = self.heap_cursor
            self.heap_cursor = (ptr + nbytes + 15) & ~15
            self.events.append(('malloc', nbytes, ptr))
            uc.reg_write(UC_X86_REG_ESP, sp + 4)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)
            uc.reg_write(UC_X86_REG_EAX, ptr)
            uc.reg_write(UC_X86_REG_ECX, 0xbadc0ffe)
            uc.reg_write(UC_X86_REG_EDX, 0x87654321)

    def invoke(self, name, args):
        self.events = []
        self.heap_cursor = HEAP
        sp = STACK + 0x8000
        words = [STOP] + [a & 0xFFFFFFFF for a in args]
        self.uc.mem_write(sp, struct.pack('<' + 'I' * len(words), *words))
        caller = bytes(self.uc.mem_read(sp, 0x8000))
        saved = [0x12345678 + i for i in range(4)]
        for reg, val in zip(PRESERVED, saved):
            self.uc.reg_write(reg, val)
        self.uc.reg_write(UC_X86_REG_ESP, sp)
        self.uc.reg_write(UC_X86_REG_EFLAGS, 2)

        self.uc.emu_start(self.symbols[name], STOP, count=1000000)

        assert self.uc.reg_read(UC_X86_REG_EIP) == STOP
        assert self.uc.reg_read(UC_X86_REG_ESP) == sp + 4
        assert [self.uc.reg_read(r) for r in PRESERVED] == saved
        assert (self.uc.reg_read(UC_X86_REG_EFLAGS) & 0x400) == 0
        assert bytes(self.uc.mem_read(sp, 0x8000)) == caller

        ret_val = self.uc.reg_read(UC_X86_REG_EAX)
        return ret_val, list(self.events)


def verify_sprite_descriptors(record=False):
    original_pe = pefile.PE(data=verify_target())
    inspect_original(original_pe)

    build(dll=DLL)
    compiled_pe = pefile.PE(str(DLL))

    coverage = {name: {'comparisons': 0, 'cases': 0} for name in ROUTINES}

    # 1. Gfx_SpriteReservedNative
    for seed in range(32):
        s_orig = DescriptorSession(original_pe, True)
        s_comp = DescriptorSession(compiled_pe, False)
        rng = random.Random(0x61a80 + seed)
        args = [rng.getrandbits(32)]
        r_orig = s_orig.invoke('Gfx_SpriteReservedNative', args)
        r_comp = s_comp.invoke('Gfx_SpriteReservedNative', args)
        assert r_orig == r_comp == (0, []), f"Reserved mismatch: {r_orig} vs {r_comp}"
        coverage['Gfx_SpriteReservedNative']['comparisons'] += 1
        coverage['Gfx_SpriteReservedNative']['cases'] += 1

    # 2. Gfx_FreeSpriteDescriptor & Gfx_SpriteFreeDescriptorNative
    for seed in range(32):
        rng = random.Random(0x614b0 + seed)
        desc_addr = ARENA + 0x100
        p1 = 0 if (seed % 4 == 0) else (ARENA + 0x20000 + rng.randint(0, 0x1000))
        p2 = 0 if (seed % 4 == 1) else (ARENA + 0x30000 + rng.randint(0, 0x1000))
        words = [rng.getrandbits(32) for _ in range(16)]
        words[6] = p1
        words[7] = p2

        # Test Gfx_FreeSpriteDescriptor
        s_orig = DescriptorSession(original_pe, True)
        s_comp = DescriptorSession(compiled_pe, False)
        s_orig.uc.mem_write(desc_addr, struct.pack('<16I', *words))
        s_comp.uc.mem_write(desc_addr, struct.pack('<16I', *words))
        r_orig = s_orig.invoke('Gfx_FreeSpriteDescriptor', [desc_addr])
        r_comp = s_comp.invoke('Gfx_FreeSpriteDescriptor', [desc_addr])
        # Compare events (free calls)
        assert r_orig[1] == r_comp[1], f"Free events mismatch: {r_orig[1]} vs {r_comp[1]}"
        coverage['Gfx_FreeSpriteDescriptor']['comparisons'] += 1
        coverage['Gfx_FreeSpriteDescriptor']['cases'] += 1

        # Test Gfx_SpriteFreeDescriptorNative
        s_orig = DescriptorSession(original_pe, True)
        s_comp = DescriptorSession(compiled_pe, False)
        s_orig.uc.mem_write(desc_addr, struct.pack('<16I', *words))
        s_comp.uc.mem_write(desc_addr, struct.pack('<16I', *words))
        r_orig = s_orig.invoke('Gfx_SpriteFreeDescriptorNative', [desc_addr])
        r_comp = s_comp.invoke('Gfx_SpriteFreeDescriptorNative', [desc_addr])
        assert r_orig[0] == r_comp[0] == 1, f"Free return mismatch: {r_orig[0]} vs {r_comp[0]}"
        assert r_orig[1] == r_comp[1], f"Free native events mismatch: {r_orig[1]} vs {r_comp[1]}"
        coverage['Gfx_SpriteFreeDescriptorNative']['comparisons'] += 1
        coverage['Gfx_SpriteFreeDescriptorNative']['cases'] += 1

    # 3. Gfx_SpriteCopyDescriptorNative
    for seed in range(64):
        rng = random.Random(0x5c960 + seed)
        s_orig = DescriptorSession(original_pe, True)
        s_comp = DescriptorSession(compiled_pe, False)

        default_words = [rng.getrandbits(32) for _ in range(16)]
        s_orig.uc.mem_write(s_orig.symbols['g_nativeSpriteDefault'], struct.pack('<16I', *default_words))
        s_comp.uc.mem_write(s_comp.symbols['g_nativeSpriteDefault'], struct.pack('<16I', *default_words))

        table_addr = ARENA + 0x1000
        rec_words = [rng.getrandbits(32) for _ in range(16)]
        rec_addr = ARENA + 0x2000
        s_orig.uc.mem_write(rec_addr, struct.pack('<16I', *rec_words))
        s_comp.uc.mem_write(rec_addr, struct.pack('<16I', *rec_words))

        capacity = 8
        table_ptrs = [0] * capacity
        table_ptrs[1] = rec_addr
        table_ptrs[3] = rec_addr
        s_orig.uc.mem_write(table_addr, struct.pack(f'<{capacity}I', *table_ptrs))
        s_comp.uc.mem_write(table_addr, struct.pack(f'<{capacity}I', *table_ptrs))

        for s in (s_orig, s_comp):
            s.uc.mem_write(s.symbols['g_nativeImageControl'] + 8, struct.pack('<II', capacity, table_addr))

        test_ids = [0, 1, 2, 3, 7, 8, 20, -1, -5]
        image_id = test_ids[seed % len(test_ids)]

        dest_addr = ARENA + 0x100
        s_orig.uc.mem_write(dest_addr, b'\x55' * 64)
        s_comp.uc.mem_write(dest_addr, b'\x55' * 64)

        r_orig = s_orig.invoke('Gfx_SpriteCopyDescriptorNative', [dest_addr, image_id])
        r_comp = s_comp.invoke('Gfx_SpriteCopyDescriptorNative', [dest_addr, image_id])
        assert r_orig[0] == r_comp[0] == 1
        d_orig = s_orig.uc.mem_read(dest_addr, 64)
        d_comp = s_comp.uc.mem_read(dest_addr, 64)
        assert d_orig == d_comp, f"Copied descriptor mismatch for id={image_id}"
        coverage['Gfx_SpriteCopyDescriptorNative']['comparisons'] += 1
        coverage['Gfx_SpriteCopyDescriptorNative']['cases'] += 1

    # 4. Gfx_CreateSpriteDescriptor & Gfx_SpriteCreateDescriptorNative
    for seed in range(64):
        rng = random.Random(0x61210 + seed)
        s_orig = DescriptorSession(original_pe, True)
        s_comp = DescriptorSession(compiled_pe, False)

        def_name_addr = ARENA + 0x300
        def_name = f'def_name_{seed}\0'.encode('ascii')
        def_pixels_addr = ARENA + 0x400
        def_w, def_h = 3, 2
        def_pixels = bytes([rng.randint(0, 255) for _ in range(def_w * def_h)])

        def_words = [0] * 16
        def_words[0] = def_name_addr
        def_words[1] = def_w
        def_words[2] = def_h
        def_words[4] = def_pixels_addr
        def_words[5] = def_w

        rec1_name_addr = ARENA + 0x500
        rec1_name = f'custom_sprite_{seed}\0'.encode('ascii')
        rec1_pixels_addr = ARENA + 0x600
        rec1_w, rec1_h = rng.randint(1, 8), rng.randint(1, 8)
        rec1_pixels = bytes([rng.randint(0, 255) for _ in range(rec1_w * rec1_h)])

        rec1_words = [0] * 16
        rec1_words[0] = rec1_name_addr
        rec1_words[1] = rec1_w
        rec1_words[2] = rec1_h
        rec1_words[4] = rec1_pixels_addr
        rec1_words[5] = rec1_w
        rec1_addr = ARENA + 0x700

        capacity = 8
        table_addr = ARENA + 0x1000
        table_ptrs = [0] * capacity
        table_ptrs[1] = rec1_addr

        for s in (s_orig, s_comp):
            s.uc.mem_write(def_name_addr, def_name)
            s.uc.mem_write(def_pixels_addr, def_pixels)
            s.uc.mem_write(s.symbols['g_nativeSpriteDefault'], struct.pack('<16I', *def_words))
            s.uc.mem_write(rec1_name_addr, rec1_name)
            s.uc.mem_write(rec1_pixels_addr, rec1_pixels)
            s.uc.mem_write(rec1_addr, struct.pack('<16I', *rec1_words))
            s.uc.mem_write(table_addr, struct.pack(f'<{capacity}I', *table_ptrs))
            s.uc.mem_write(s.symbols['g_nativeImageControl'] + 8, struct.pack('<II', capacity, table_addr))

        test_ids = [0, 1, 2, 7, 8, -1]
        image_id = test_ids[seed % len(test_ids)]

        dest_addr = ARENA + 0x100
        s_orig.uc.mem_write(dest_addr, b'\x00' * 64)
        s_comp.uc.mem_write(dest_addr, b'\x00' * 64)

        # Test Gfx_CreateSpriteDescriptor
        r_orig = s_orig.invoke('Gfx_CreateSpriteDescriptor', [dest_addr, image_id])
        r_comp = s_comp.invoke('Gfx_CreateSpriteDescriptor', [dest_addr, image_id])
        assert r_orig[1] == r_comp[1], f"Allocations mismatch: {r_orig[1]} vs {r_comp[1]}"
        d_orig = s_orig.uc.mem_read(dest_addr, 64)
        d_comp = s_comp.uc.mem_read(dest_addr, 64)
        assert d_orig == d_comp, f"Created descriptor mismatch for id={image_id}"
        # Validate that allocated string and allocated pixels match exactly
        out_words = struct.unpack('<16I', d_orig)
        assert out_words[6] == out_words[0], "Expected words[6] == words[0]"
        assert out_words[7] == out_words[4], "Expected words[7] == words[4]"
        assert out_words[5] == out_words[1], "Expected stride == width"
        str_orig = s_orig.uc.mem_read(out_words[0], 32).split(b'\0')[0]
        str_comp = s_comp.uc.mem_read(out_words[0], 32).split(b'\0')[0]
        assert str_orig == str_comp
        pix_orig = s_orig.uc.mem_read(out_words[4], out_words[1] * out_words[2])
        pix_comp = s_comp.uc.mem_read(out_words[4], out_words[1] * out_words[2])
        assert pix_orig == pix_comp
        coverage['Gfx_CreateSpriteDescriptor']['comparisons'] += 1
        coverage['Gfx_CreateSpriteDescriptor']['cases'] += 1

        # Test Gfx_SpriteCreateDescriptorNative
        s_orig = DescriptorSession(original_pe, True)
        s_comp = DescriptorSession(compiled_pe, False)
        for s in (s_orig, s_comp):
            s.uc.mem_write(def_name_addr, def_name)
            s.uc.mem_write(def_pixels_addr, def_pixels)
            s.uc.mem_write(s.symbols['g_nativeSpriteDefault'], struct.pack('<16I', *def_words))
            s.uc.mem_write(rec1_name_addr, rec1_name)
            s.uc.mem_write(rec1_pixels_addr, rec1_pixels)
            s.uc.mem_write(rec1_addr, struct.pack('<16I', *rec1_words))
            s.uc.mem_write(table_addr, struct.pack(f'<{capacity}I', *table_ptrs))
            s.uc.mem_write(s.symbols['g_nativeImageControl'] + 8, struct.pack('<II', capacity, table_addr))
            s.uc.mem_write(dest_addr, b'\x00' * 64)

        r_orig = s_orig.invoke('Gfx_SpriteCreateDescriptorNative', [dest_addr, image_id])
        r_comp = s_comp.invoke('Gfx_SpriteCreateDescriptorNative', [dest_addr, image_id])
        assert r_orig[0] == r_comp[0] == 1, f"Create native ret mismatch: {r_orig[0]} vs {r_comp[0]}"
        assert r_orig[1] == r_comp[1], f"Create native alloc mismatch: {r_orig[1]} vs {r_comp[1]}"
        d_orig = s_orig.uc.mem_read(dest_addr, 64)
        d_comp = s_comp.uc.mem_read(dest_addr, 64)
        assert d_orig == d_comp, f"Create native descriptor mismatch for id={image_id}"
        coverage['Gfx_SpriteCreateDescriptorNative']['comparisons'] += 1
        coverage['Gfx_SpriteCreateDescriptorNative']['cases'] += 1

    if record:
        for name, (rva, *_) in ROUTINES.items():
            for kind in ('compilation', 'emulation'):
                record_run(
                    rva, kind, 'pass',
                    inputs=INPUTS,
                    artifact=DLL.relative_to(ROOT).as_posix(),
                    cases=coverage[name]['cases'] if kind == 'emulation' else 0,
                    command='uv run tools/verify_sprite_descriptors.py',
                    details={
                        'coverage': coverage[name],
                        'scope': 'Authentic sprite descriptor lifecycle routines compared differentially with exact allocations, frees, string copy, pixel copy, descriptors and ABI against IGN_WIN.EXE',
                        'limitations': 'Validation DLL focused harness; no full rasterizer or complete playable game parity'
                    }
                )

    print('PASS sprite descriptors verification:', coverage, flush=True)
    return coverage


if __name__ == '__main__':
    verify_sprite_descriptors(record=True)
