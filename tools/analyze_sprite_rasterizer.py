# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Bounded original-only native rasterizer analysis; no C fidelity results."""
import hashlib
import json
import random
import struct

import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import UC_HOOK_CODE, UC_HOOK_MEM_WRITE
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX,
    UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP,
    UC_X86_REG_ESP, UC_X86_REG_EFLAGS)
from verify_sprite_backend import BUILD, DATA, STACK, STOP, cpu_image, verify_target

REGISTERS = (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX,
    UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)
TABLE = 0x7300000


def u32(value):
    return value & 0xffffffff


def s32(value):
    return (value & 0x7fffffff) - (value & 0x80000000)


def put(cpu, address, values):
    cpu.mem_write(address, struct.pack('<'+'I'*len(values), *(u32(v) for v in values)))


def inspect(pe):
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    results = []
    for rva, size, digest in (
            (0x65bb5, 474, '2cdb543878a2ce01c034f50d8f679a881f1491d088a64746b18ce6273d464fbb'),
            (0x68c70, 631, 'c239f5eb8a12e6b59eb775b316f45e4d8c37ad81f2c0b66910cda5033f8f96b2'),
            (0x5ca50, 1813, '4458bb7690590a3289e6ce7d99b3a80338382fe2afc3b67a25891b2af0e5f8bf')):
        raw = pe.get_data(rva, size)
        instructions = list(md.disasm(raw, 0x400000+rva))
        assert sum(i.size for i in instructions) == size
        if rva == 0x65bb5:
            assert instructions[-1].address + instructions[-1].size == 0x465d8f
            assert instructions[-1].mnemonic == 'jmp'
            assert instructions[0].mnemonic == 'pushal'
            assert [i.address for i in instructions if i.mnemonic == 'popal'] == [0x465be8, 0x465d68]
            starts = {i.address for i in instructions}
            assert all(int(i.op_str, 16) in starts for i in instructions if i.mnemonic.startswith('j'))
            assert pe.get_data(0x65d8f, 1) == b'\xcc'
            assert pe.get_data(0x65bb5, 437)[-2:] == b'\x61\xc3'
        else:
            assert instructions[-1].mnemonic == 'ret'
        if digest:
            assert hashlib.sha256(raw).hexdigest() == digest
        results.append({'rva': hex(rva), 'size': size,
            'sha256': hashlib.sha256(raw).hexdigest(),
            'calls': [(hex(i.address), i.op_str) for i in instructions if i.mnemonic == 'call']})
        (BUILD/f'rasterizer_{rva:x}.txt').write_text('\n'.join(
            f'{i.address:08x}: {i.mnemonic} {i.op_str}' for i in instructions)+'\n', encoding='utf-8')
    # Both non-FPO entries are independently bounded; CA50 has authentic FPO.
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {a: (n,l,p,b) for a,n,l,p,b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    assert 0x65bb5 not in fpo and 0x68c70 not in fpo
    assert fpo[0x5ca50][0] == 1813 and fpo[0x5ca50][2] == 1
    assert pe.get_data(0x68ee7, 1) == b'\xcc'
    assert results[0]['calls'] == [('0x465be3', '0x468c70')]
    assert results[1]['calls'] == [('0x468e6b', '0x45ca50'), ('0x468edf', '0x45ca50')]
    for va, size in ((0x4bad28, 12), (0x4bad34, 20), (0x4baf1c, 116)):
        assert pe.get_data(va-0x400000, size) == bytes(size)
    for va in (0x63f2b4, 0x63f2c8, 0x63f2d4):
        s = next(s for s in pe.sections if s.VirtualAddress <= va-0x400000 < s.VirtualAddress+s.Misc_VirtualSize)
        assert va-0x400000-s.VirtualAddress >= s.SizeOfRawData
    assert pe.get_data(0xba778, 16) == struct.pack('<4I', 4, 0, 0x4ba758, 0)
    assert pe.get_data(0x5740d, 5) == bytes.fromhex('e8a3e70000')
    relocations = {e.rva for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries if e.type == 3}
    assert 0xba780 in relocations
    for result in results:
        r, n = int(result['rva'], 16), result['size']
        result['relocations'] = [(hex(site), hex(struct.unpack('<I', pe.get_data(site, 4))[0]))
            for site in sorted(relocations) if r <= site < r+n]
    return results


def untransformed_checks(pe):
    rng = random.Random(0x65bb5)
    table = rng.randbytes(65536)
    source = rng.randbytes(4096)
    framebuffer = rng.randbytes(4096)
    cases, exits, spans = 0, set(), set()
    # Widths exercise all narrow paths and every four-byte loop remainder.
    for width in (*range(1, 21), 33, 63):
        for px, py in ((4,4), (-2,4), (4,-2), (24,4), (4,12),
                (-80,4), (80,4), (4,-80), (4,80), (-2,-2), (24,12),
                (2,1), (30,15), (4,-4), (4,15), (30,4)):
            for fraction in (0, 255):
                x, y = px, py
                cpu, base, size = cpu_image(pe)
                cpu.mem_map(TABLE, 65536)
                cpu.mem_write(TABLE, table)
                cpu.mem_write(DATA+0x1000, source)
                cpu.mem_write(DATA+0x5000, framebuffer)
                ox, oy, tx, ty, height = -513, 769, 3, 2, 5
                put(cpu, DATA, [rng.getrandbits(32), DATA+32, DATA+64, 0])
                put(cpu, DATA+32, [u32(x*256+ox+fraction), u32(y*256+oy+fraction)])
                put(cpu, DATA+64, [ox, oy, tx*256, ty*256,
                    (tx+width)*256, (ty+height)*256, DATA+0x1000, TABLE])
                # Ordering is bottom, top, right, left; exclusive maxima.
                put(cpu, 0x63f2b4, [15, 1, 30, 2])
                put(cpu, 0x63f2c8, [128])
                put(cpu, 0x63f2d4, [DATA+0x5000])
                sp = STACK+0x8000
                put(cpu, sp, [STOP])
                cpu.reg_write(UC_X86_REG_ESP, sp)
                cpu.reg_write(UC_X86_REG_EFLAGS, 2)
                saved = [rng.getrandbits(32) for _ in REGISTERS]
                saved[4] = DATA
                for reg, value in zip(REGISTERS, saved): cpu.reg_write(reg, value)
                expected_writes = []
                def scratch(address, value):
                    expected_writes.append((address, 4, u32(value)))
                src, w, h = DATA+0x1000+ty*256+tx, width, height
                scratch(0x4bad28, src)
                scratch(0x4bad2c, w)
                scratch(0x4bad30, h)
                rejected = False
                if y < 1:
                    delta = 1-y
                    h -= delta
                    scratch(0x4bad30, h)
                    if h <= 0: rejected = True
                    else:
                        src += delta*256
                        scratch(0x4bad28, src)
                        y = 1
                if not rejected and y+h > 15:
                    h -= y+h-15
                    scratch(0x4bad30, h)
                    if h <= 0: rejected = True
                if not rejected and x < 2:
                    delta = 2-x
                    w -= delta
                    scratch(0x4bad2c, w)
                    if w <= 0: rejected = True
                    else:
                        src += delta
                        scratch(0x4bad28, src)
                        x = 2
                if not rejected and x+w > 30:
                    w -= x+w-30
                    scratch(0x4bad2c, w)
                    if w <= 0: rejected = True
                expected = bytearray(framebuffer)
                if not rejected:
                    spans.add(w)
                    for row in range(h):
                        destination = (y+row)*128+x
                        source_index = src-(DATA+0x1000)+row*256
                        for column in range(w):
                            expected[destination+column] = table[source[source_index+column]*256+framebuffer[destination+column]]
                        remaining = w
                        while remaining >= 6:
                            remaining -= 4
                            value = int.from_bytes(expected[destination+remaining:destination+remaining+4], 'little')
                            expected_writes.append((DATA+0x5000+destination+remaining, 4, value))
                        for column in range(remaining-1, -1, -1):
                            expected_writes.append((DATA+0x5000+destination+column, 1, expected[destination+column]))
                before = bytes(cpu.mem_read(base, size))
                data_before = bytes(cpu.mem_read(DATA, 65536))
                writes, path = [], set()
                def code(uc, address, length, unused):
                    assert 0x465bb5 <= address < 0x465d8f
                    path.add(address)
                def write(uc, access, address, length, value, unused):
                    if STACK <= address < STACK+0x10000: return
                    writes.append((address, length, value & ((1 << (8*length))-1)))
                cpu.hook_add(UC_HOOK_CODE, code)
                cpu.hook_add(UC_HOOK_MEM_WRITE, write)
                cpu.emu_start(0x465bb5, STOP, count=100000)
                assert writes == expected_writes, (width, x, y, writes, expected_writes)
                assert bytes(cpu.mem_read(DATA+0x5000, 4096)) == expected
                assert bytes(cpu.mem_read(DATA+0x1000, 4096)) == source
                assert bytes(cpu.mem_read(TABLE, 65536)) == table
                assert [cpu.reg_read(r) for r in REGISTERS] == saved
                assert cpu.reg_read(UC_X86_REG_ESP) == sp+4
                data_after = bytearray(cpu.mem_read(DATA, 65536))
                data_after[0x5000:0x6000] = data_before[0x5000:0x6000]
                assert data_after == data_before
                after = bytearray(cpu.mem_read(base, size))
                after[0x4bad28-base:0x4bad34-base] = before[0x4bad28-base:0x4bad34-base]
                assert after == before
                exits.update(path & {0x465d6a, 0x465d84, 0x465cf2, 0x465d04})
                cases += 1
    assert exits == {0x465d6a, 0x465d84, 0x465cf2, 0x465d04}
    assert {1,2,3,4,5,6,7,8}.issubset(spans)
    return {'cases': cases, 'paths': [hex(a) for a in sorted(exits)], 'visible_widths': sorted(spans)}


def transformed_checks(pe):
    """Execute the real entry and helper; stop before CA50, never synthesize return."""
    rng = random.Random(0x68c70)
    cases = 0
    for coefficients in ((65536,0,0,65536), (0,65536,65536,0),
            (-65536,0,0,-65536), (0,0,0,0), (12345,67890,67890,12345),
            (0x80000000,0x7fffffff,0x7fffffff,0x80000000)):
        for _ in range(12):
            cpu, base, size = cpu_image(pe)
            position = [rng.getrandbits(32) for _ in range(2)]
            descriptor = [rng.getrandbits(32) for _ in range(8)]
            put(cpu, DATA, [4, DATA+32, DATA+64, DATA+128])
            put(cpu, DATA+32, position)
            put(cpu, DATA+64, descriptor)
            put(cpu, DATA+128, coefficients)
            sp = STACK+0x8000
            put(cpu, sp, [STOP])
            cpu.reg_write(UC_X86_REG_ESP, sp)
            saved = [rng.getrandbits(32) for _ in REGISTERS]
            saved[4] = DATA
            for reg, value in zip(REGISTERS, saved): cpu.reg_write(reg, value)
            writes, seen = [], []
            expected = []
            state = {}
            def store(address, value):
                state[address] = u32(value)
                expected.append((address, 4, u32(value)))
            for a,v in ((0x4bad40,position[0]), (0x4bad44,position[1]),
                    (0x4bad38,DATA+64), (0x4bad3c,DATA+128),
                    (0x4baf3c,descriptor[3]), (0x4baf40,descriptor[5]-16),
                    (0x4baf44,descriptor[2]), (0x4baf48,descriptor[4]-16),
                    (0x4baf4c,descriptor[7]), (0x4baf50,descriptor[6])): store(a,v)
            def product(a,b):
                return u32((s32(a)*s32(b)) >> 16)
            nx, ny = u32(-descriptor[0]), u32(-descriptor[1])
            c0,c1,c2,c3 = coefficients
            store(0x4baf1c, product(nx,c0))
            store(0x4baf1c, state[0x4baf1c]+product(ny,c2))
            store(0x4baf20, product(nx,c1))
            store(0x4baf20, state[0x4baf20]+product(ny,c3))
            dx,dy = u32(descriptor[4]-16-descriptor[2]), u32(descriptor[5]-16-descriptor[3])
            store(0x4baf28, product(dx,c1))
            store(0x4baf24, product(dx,c0))
            store(0x4baf30, product(dy,c3)+state[0x4baf20])
            store(0x4baf38, state[0x4baf30]+state[0x4baf28])
            store(0x4baf28, state[0x4baf28]+state[0x4baf20])
            store(0x4baf2c, product(dy,c2)+state[0x4baf1c])
            store(0x4baf34, state[0x4baf2c]+state[0x4baf24])
            store(0x4baf24, state[0x4baf24]+state[0x4baf1c])
            for address in range(0x4baf1c,0x4baf3c,4):
                store(address, state[address]+position[((address-0x4baf1c)//4)%2])
            for offset, address in ((4,0x4baf1c),(8,0x4baf20),(12,0x4baf24),
                    (16,0x4baf28),(20,0x4baf2c),(24,0x4baf30), (32,0x4baf3c),
                    (40,0x4baf3c),(48,0x4baf40),(28,0x4baf44),(44,0x4baf44),
                    (36,0x4baf48),(52,0x4baf50),(56,0x4baf4c)):
                store(0x4baf54+offset, state[address])
            before = bytes(cpu.mem_read(base,size))
            data_before = bytes(cpu.mem_read(DATA,65536))
            def code(uc, address, length, unused):
                if address == 0x468c70:
                    assert uc.reg_read(UC_X86_REG_ESI) == 0x4bad34
                    assert uc.reg_read(UC_X86_REG_ESP) == sp-36
                    assert struct.unpack('<I',uc.mem_read(sp-36,4))[0] == 0x465be8
                    seen.append('helper')
                elif address == 0x45ca50:
                    assert uc.reg_read(UC_X86_REG_ESI) == 0x4baf54
                    assert uc.reg_read(UC_X86_REG_ESP) == sp-48
                    assert struct.unpack('<2I',uc.mem_read(sp-48,8)) == (0x468e70,0x4baf54)
                    # Original PUSHAD layout, including pre-push ESP and original ESI.
                    assert struct.unpack('<8I',uc.mem_read(sp-32,32)) == (
                        saved[5],saved[4],saved[6],sp,saved[1],saved[3],saved[2],saved[0])
                    seen.append('triangle_boundary')
                    uc.emu_stop()
                else:
                    assert 0x465bb5 <= address < 0x465bea or 0x468c70 <= address < 0x468ee7
            def write(uc, access, address, length, value, unused):
                if STACK <= address < STACK+0x10000: return
                writes.append((address,length,u32(value)))
            cpu.hook_add(UC_HOOK_CODE,code)
            cpu.hook_add(UC_HOOK_MEM_WRITE,write)
            cpu.emu_start(0x465bb5,STOP,count=10000)
            assert seen == ['helper','triangle_boundary'] and writes == expected
            assert bytes(cpu.mem_read(DATA,65536)) == data_before
            after = bytearray(cpu.mem_read(base,size))
            for address in state: after[address-base:address-base+4] = before[address-base:address-base+4]
            assert after == before
            cases += 1
    return cases


if __name__ == '__main__':
    BUILD.mkdir(parents=True, exist_ok=True)
    pe = pefile.PE(data=verify_target())
    if not __debug__: raise RuntimeError('Analysis requires assertions')
    report = {'routines': inspect(pe), 'untransformed': untransformed_checks(pe),
        'transformed_first_triangle_boundary_cases': transformed_checks(pe),
        'scope': 'Original-only entry framebuffer execution and transformed first triangle boundary; no C comparison',
        'limitations': 'No CA50 execution, transformed framebuffer or transformed return validation; no native runtime or instruction equality'}
    (BUILD/'sprite_rasterizer_analysis.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"PASS: {report['untransformed']['cases']} original-only framebuffer cases; "
        f"{report['transformed_first_triangle_boundary_cases']} transformed first-call boundary cases. "
        'No production-C or transformed framebuffer validation.')
