# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Original-only adapter analysis and C calling-convention diagnostics.

Stops BEFORE the rasterizer. Never records reconstruction validation results.
"""
import json
import random
import shutil
import struct
import subprocess

import pefile
from unicorn import UC_HOOK_CODE, UC_HOOK_MEM_WRITE
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX,
    UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_ESP, UC_X86_REG_EFLAGS)
from verify_sprite_backend import (ROOT, BUILD, DATA, STACK, STOP, SAVED,
    cpu_image, inspect_original, verify_target)


def adapter_checks(pe):
    expected_relocs = [(0x57374, 0x50ebfc), (0x5737d, 0x4ba77c),
        (0x57386, 0x4ba784), (0x573a2, 0x4ba758), (0x573b6, 0x4ba75c),
        (0x573be, 0x4ba760), (0x573ca, 0x4ba764), (0x573dd, 0x4ba760),
        (0x573e3, 0x4ba770), (0x573e8, 0x4ba768), (0x573f5, 0x4ba764),
        (0x573fa, 0x4ba76c), (0x573ff, 0x63f2d8), (0x57404, 0x4ba774),
        (0x57409, 0x4ba778)]
    assert [(e.rva, struct.unpack('<I', pe.get_data(e.rva, 4))[0])
        for block in pe.DIRECTORY_ENTRY_BASERELOC for e in block.entries
        if e.type == 3 and 0x57370 <= e.rva < 0x57417] == expected_relocs
    assert pe.get_data(0x57417, 9) == b'\xcc' * 9
    rng = random.Random(0x57370)
    cases = 0
    for count in (-2147483648, -1, 0, 1, 2, 3):
        for image_id in (-2147483648, -1, 0, 1, 2, 3, 2147483647):
            for empty in (False, True):
                for mutation in (False, True):
                    cpu, base, size = cpu_image(pe)
                    descriptor, fallback = rng.randbytes(64), rng.randbytes(64)
                    hx, hy, render_word = (rng.getrandbits(32) for _ in range(3))
                    coefficient = rng.choice((0, DATA+256))
                    cpu.mem_write(DATA, struct.pack('<i2I', image_id, hx, hy))
                    cpu.mem_write(DATA+32, rng.randbytes(8))
                    cpu.mem_write(DATA+64, descriptor)
                    cpu.mem_write(DATA+128, struct.pack('<3I', 0,
                        0 if empty else DATA+64, DATA+64))
                    cpu.mem_write(0x51fb88, fallback)
                    cpu.mem_write(0x520388, struct.pack('<iI', count, DATA+128))
                    cpu.mem_write(0x4ba708, struct.pack('<3I', DATA+32, DATA, coefficient))
                    cpu.mem_write(0x50ebfc, struct.pack('<I', 0x4ba708))
                    cpu.mem_write(0x63f2d8, struct.pack('<I', render_word))
                    sp = STACK+0x8000
                    cpu.mem_write(sp, struct.pack('<I', STOP))
                    cpu.reg_write(UC_X86_REG_ESP, sp)
                    cpu.reg_write(UC_X86_REG_EFLAGS, 2)
                    saved = [rng.getrandbits(32) for _ in SAVED]
                    for reg, value in zip(SAVED, saved): cpu.reg_write(reg, value)
                    chosen = descriptor if 0 < image_id < count and not (image_id == 1 and empty) else fallback
                    local = bytearray(chosen)
                    if chosen is descriptor: local[24:32] = bytes(8)
                    d = struct.unpack('<16I', local)
                    new_x, new_y = (hx ^ 0xffffffff, hy ^ 0x80000000) if mutation else (hx, hy)
                    expected = [(0x4ba77c, DATA+32), (0x4ba784, coefficient),
                        (0x4ba758, new_x), (0x4ba75c, new_y),
                        (0x4ba760, (d[4] & 255) << 8), (0x4ba764, d[4] & 0xff00),
                        (0x4ba770, d[4] & 0xffff0000),
                        (0x4ba768, ((d[1] << 8) + ((d[4] & 255) << 8)) & 0xffffffff),
                        (0x4ba76c, ((d[2] << 8) + (d[4] & 0xff00)) & 0xffffffff),
                        (0x4ba774, render_word)]
                    writes, calls = [], []
                    before = bytes(cpu.mem_read(base, size))
                    def code(uc, address, length, unused):
                        if address == 0x4612e0:
                            lookup_sp = uc.reg_read(UC_X86_REG_ESP)
                            assert lookup_sp == sp-80
                            assert struct.unpack('<3I', uc.mem_read(lookup_sp, 12)) == (
                                0x45739a, sp-64, image_id & 0xffffffff)
                            assert writes == expected[:2]
                            calls.append('lookup')
                        elif address == 0x45739a:
                            assert bytes(uc.mem_read(sp-64, 64)) == local
                            if mutation:
                                # Instrumented perturbation, not an original lookup effect.
                                uc.mem_write(DATA+4, struct.pack('<2I', new_x, new_y))
                                uc.mem_write(0x4ba708, struct.pack('<3I', 0, DATA+512, 0))
                                uc.mem_write(0x50ebfc, struct.pack('<I', DATA+512))
                        elif address == 0x465bb5:
                            assert uc.reg_read(UC_X86_REG_ESI) == 0x4ba778
                            assert uc.reg_read(UC_X86_REG_ESP) == sp-72
                            assert struct.unpack('<I', uc.mem_read(sp-72, 4))[0] == 0x457412
                            assert struct.unpack('<I', uc.mem_read(sp-68, 4))[0] == saved[1]
                            assert uc.reg_read(UC_X86_REG_EAX) == render_word
                            assert uc.reg_read(UC_X86_REG_EDX) == new_y
                            assert bytes(uc.mem_read(sp-64, 64)) == local
                            calls.append('rasterizer_boundary')
                            uc.emu_stop()
                        else:
                            assert 0x457370 <= address < 0x457417 or 0x4612e0 <= address < 0x461352
                    def write(uc, access, address, length, value, unused):
                        if STACK <= address < STACK+0x10000: return
                        assert length == 4
                        writes.append((address, value & 0xffffffff))
                    cpu.hook_add(UC_HOOK_CODE, code)
                    cpu.hook_add(UC_HOOK_MEM_WRITE, write)
                    cpu.emu_start(0x457370, STOP, count=10000)
                    assert calls == ['lookup', 'rasterizer_boundary'] and writes == expected
                    assert [cpu.reg_read(r) for r in (SAVED[0], SAVED[2], SAVED[3])] == [saved[0], saved[2], saved[3]]
                    assert bytes(cpu.mem_read(DATA+64, 64)) == descriptor
                    after = bytearray(cpu.mem_read(base, size))
                    for address, _ in expected: after[address-base:address-base+4] = before[address-base:address-base+4]
                    if mutation:
                        for address, n in ((0x4ba708, 12), (0x50ebfc, 4)):
                            after[address-base:address-base+n] = before[address-base:address-base+n]
                    assert after == before
                    cases += 1
    return cases


def convention_checks():
    """Compile pure C probes; observe conventional argument locations, not fidelity."""
    source = BUILD/'sprite_adapter_abi_probe.c'
    obj, dll = source.with_suffix('.obj'), source.with_suffix('.dll')
    source.write_text('''extern void cdecl_sink(void *);
extern void __attribute__((fastcall)) fastcall_sink(void *);
extern void __attribute__((regparm(1))) regparm_sink(void *);
void cdecl_probe(void *p) { cdecl_sink(p); }
void fastcall_probe(void *p) { fastcall_sink(p); }
void regparm_probe(void *p) { regparm_sink(p); }
void cdecl_sink(void *p) { (void)p; for (;;) {} }
void __attribute__((fastcall)) fastcall_sink(void *p) { (void)p; for (;;) {} }
void __attribute__((regparm(1))) regparm_sink(void *p) { (void)p; for (;;) {} }
''', encoding='utf-8')
    cc, ld = shutil.which('gcc'), shutil.which('lld-link')
    if not cc or not ld: raise RuntimeError('GCC and LLD required for ABI diagnostics')
    commands = [[cc, '-m32', '-std=c89', '-pedantic-errors', '-O0',
        '-fno-optimize-sibling-calls', '-c', str(source), '-o', str(obj)],
        [ld, '/dll', '/noentry', '/nodefaultlib', '/safeseh:no', '/machine:x86',
        '/out:'+str(dll), str(obj), '/export:cdecl_probe', '/export:fastcall_probe',
        '/export:regparm_probe', '/export:cdecl_sink', '/export:fastcall_sink=@fastcall_sink@4',
        '/export:regparm_sink']]
    for command in commands: subprocess.run(command, cwd=ROOT, check=True)
    pe = pefile.PE(str(dll))
    symbols = {e.name.decode(): pe.OPTIONAL_HEADER.ImageBase+e.address
        for e in pe.DIRECTORY_ENTRY_EXPORT.symbols if e.name}
    observations = {}
    for kind in ('cdecl', 'fastcall', 'regparm'):
        cpu, _, _ = cpu_image(pe)
        sp = STACK+0x8000
        cpu.mem_write(sp, struct.pack('<2I', STOP, DATA))
        cpu.reg_write(UC_X86_REG_ESP, sp)
        cpu.reg_write(UC_X86_REG_ESI, 0x13579bdf)
        seen = []
        def boundary(uc, address, length, unused):
            if address != symbols[kind+'_sink']: return
            assert uc.reg_read(UC_X86_REG_ESI) == 0x13579bdf
            argument = (struct.unpack('<I', uc.mem_read(uc.reg_read(UC_X86_REG_ESP)+4, 4))[0]
                if kind == 'cdecl' else uc.reg_read(UC_X86_REG_ECX if kind == 'fastcall' else UC_X86_REG_EAX))
            assert argument == DATA
            seen.append({'argument_location': {'cdecl':'ESP+4', 'fastcall':'ECX', 'regparm':'EAX'}[kind],
                'esi': '0x13579bdf', 'packet': hex(DATA)})
            uc.emu_stop()
        cpu.hook_add(UC_HOOK_CODE, boundary)
        cpu.emu_start(symbols[kind+'_probe'], STOP, count=1000)
        assert len(seen) == 1
        observations[kind] = seen[0]
    return {'commands': commands, 'compiler': subprocess.check_output([cc, '--version'], text=True).splitlines()[0],
        'observations': observations}


if __name__ == '__main__':
    if not __debug__: raise RuntimeError('Analysis requires assertions')
    BUILD.mkdir(parents=True, exist_ok=True)
    original = pefile.PE(data=verify_target())
    inspect_original(original)
    cases = adapter_checks(original)
    probes = convention_checks()
    report = {'original_only_cases': cases, 'abi_probes': probes,
        'scope': 'Real adapter and lookup instructions; stop before rasterizer; no original-versus-C adapter validation',
        'limitations': 'ESI native call needs a supported ABI bridge or independently recovered C rasterizer; no rasterizer effects or adapter return execution tested'}
    (BUILD/'sprite_adapter_analysis.json').write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
    print(f'PASS: {cases} original-only adapter/lookup cases; 3 compiled C ABI diagnostics (none passes packet in ESI).')
