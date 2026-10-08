# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""File_LoadToMemory: real file helpers and Mem_Alloc; modeled CRT I/O and malloc."""
import hashlib
import random
import shutil
import struct
import subprocess

import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX,
    UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP,
    UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EFLAGS)

from verify_font_cleanup import SAVED, STACK, STOP
from build_decomp import STARTUP_BOUNDARY_SOURCE
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions; disable -O/PYTHONOPTIMIZE')

RVA, SIZE = 0x574a0, 230
ROUTINE_SHA = '2a23d52a4c2588198a7749db30e5de8c2f5ecd150a65f5940dbc2a1a5db87ec3'
DLL = BUILD / 'file_load_validation.dll'
DATA, DATA_SIZE = 0x7400000, 0x10000
ARENA, ARENA_SIZE = 0x8000000, 0x200000
INPUTS = [
    'decomp/src/geputget.c', 'decomp/include/geputget.h',
    'tools/build_decomp.py',
    'decomp/src/file.c', 'decomp/include/file.h',
    'decomp/src/mem.c', 'decomp/include/mem.h',
    'decomp/target.json', 'tools/verify_file_load.py',
    'tools/verify_matching.py', 'tools/windows_target.py', 'tools/windows_tracking.py'
]

BOUNDARIES = {
    'fopen': (0x69390, 21, 2, 0, '5f531b4e8d09786e07982b951f0259e58816996f5cdc227fe6438bf553e2bb17'),
    'fclose': (0x69100, 112, 1, 0x202, '77ed7a98cac6e47994360f0f3625796c2cb18ce4adc98cea5d1cd30c973d6af2'),
    'ftell': (0x69de0, 431, 1, 0x140b, '226f6ea54e9cf412aff7099c437890a19ccc0698054ebf69cc7d3dd5990e21f4'),
    'fseek': (0x69d40, 153, 3, 0x307, 'ae8af7be1dcb0f972852f6e90cdab27def44f6825d8b60d1c0cd08854a5ca833'),
    'fsetpos': (0x69d20, 27, 2, 0, '3acb37e5ee5cfda2dff629e546014b91c8349504f4bb397c1130740042e6a1c5'),
    'fread': (0x69170, 328, 4, 0x141e, '98ea765f070d22d1676c6684d2133c465b2a1902fa10fcbc9897c1a5a2904eaf'),
    'malloc': (0x69400, 20, 1, 0, '7d8692fed5df9efac5f7c0937ec31305b20b064cc084472afa53c39c7333fcfc'),
    'free': (0x691e0, 19, 1, 0, '5e3e2c34cb36b7c53d5a57004fce5ad9b4d4554b73b5bcfaea94d0c5aebc6383'),
}


def record(kind, outcome, cases=0, **details):
    record_run(RVA, kind, outcome, inputs=INPUTS,
        artifact=DLL.relative_to(ROOT).as_posix() if DLL.exists() else None,
        cases=cases, command=('uv run python tools/verify_file_load.py' if __name__ == '__main__'
        else 'uv run tools/verify_matching.py'), details=details)


def inspect_original(pe):
    raw = pe.get_data(RVA, SIZE)
    assert hashlib.sha256(raw).hexdigest() == ROUTINE_SHA
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {a: (n, l, p, b) for a, n, l, p, b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData + debug.SizeOfData])}
    assert fpo[RVA] == (SIZE, 2, 1, 0x30a)
    ins = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(raw, 0x400000 + RVA))
    assert sum(i.size for i in ins) == SIZE
    expected_calls = [
        (0x4574ab, 0x4576b0),  # File_CheckReadable
        (0x4574cc, 0x457630),  # File_GetSize
        (0x4574f0, 0x45ae10),  # Mem_Alloc
        (0x457517, 0x469390),  # fopen
        (0x457548, 0x469d20),  # fsetpos
        (0x457555, 0x469170),  # fread
        (0x457575, 0x469100),  # fclose
    ]
    assert [(i.address, int(i.op_str, 16)) for i in ins if i.mnemonic == 'call'] == expected_calls
    relocs = [(e.rva, struct.unpack('<I', pe.get_data(e.rva, 4))[0])
        for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries
        if e.type == 3 and RVA <= e.rva < RVA + SIZE]
    assert relocs == [
        (0x574bd, 0x4bab34),
        (0x574df, 0x4bab34),
        (0x57503, 0x4bab34),
        (0x57512, 0x47c040),
        (0x5752a, 0x4bab34),
        (0x57566, 0x4bab34),
    ]
    # Check mode "rb\0" at 0x47c040
    assert pe.get_data(0x7c040, 3) == b'rb\0'


def build_loader():
    cc, ld = shutil.which('clang'), shutil.which('lld-link')
    if not cc or not ld:
        raise RuntimeError('Clang and LLD required; no fallback')
    BUILD.mkdir(parents=True, exist_ok=True)
    fobj = BUILD / 'file_for_load.obj'
    mobj = BUILD / 'mem_for_load.obj'
    stub = BUILD / 'file_load_boundary.c'
    sobj = BUILD / 'file_load_boundary.obj'
    stub.write_text(
        STARTUP_BOUNDARY_SOURCE +
        'void *fopen(const char *f, const char *m) { (void)f; (void)m; for (;;) {} }\n'
        'int fclose(void *s) { (void)s; for (;;) {} }\n'
        'long ftell(void *s) { (void)s; for (;;) {} }\n'
        'int fseek(void *s, long p, int o) { (void)s; (void)p; (void)o; for (;;) {} }\n'
        'int fsetpos(void *s, const void *p) { (void)s; (void)p; for (;;) {} }\n'
        'unsigned int fread(void *b, unsigned int s, unsigned int n, void *f) { (void)b; (void)s; (void)n; (void)f; for (;;) {} }\n'
        'void *malloc(unsigned int n) { (void)n; for (;;) {} }\n'
        'void free(void *p) { (void)p; for (;;) {} }\n'
        'int g_fileErrorLine = 0;\n',
        encoding='utf-8')
    for path in [fobj, mobj, sobj, DLL, DLL.with_suffix('.lib'), DLL.with_suffix('.exp')]:
        path.unlink(missing_ok=True)
    flags = [cc, '--target=i686-pc-windows-msvc', '-std=c89', '-pedantic-errors',
        '-Wall', '-Wextra', '-Werror', '-O2', '-ffreestanding', '-fno-builtin',
        '-fno-inline', '-fno-vectorize', '-fno-slp-vectorize', '-mno-sse', '-mno-sse2',
        '-I', str(ROOT / 'decomp/include')]
    exports = [
        'File_LoadToMemory', 'File_CheckReadable', 'File_GetSize', 'File_GetStreamSize',
        'Mem_InitHandles', 'Mem_NextHandleId', 'Mem_RegisterHandle', 'Mem_ShutdownHandles',
        'Mem_ReleaseHandleId', 'Mem_Free', 'Mem_Alloc', 'g_memPools', 'g_fileErrorLine',
        'fopen', 'fclose', 'ftell', 'fseek', 'fsetpos', 'fread', 'malloc', 'free'
    ]
    commands = [
        flags + ['-c', str(ROOT / 'decomp/src/file.c'), '-o', str(fobj)],
        flags + ['-c', str(ROOT / 'decomp/src/mem.c'), '-o', str(mobj)],
        flags + ['-c', str(stub), '-o', str(sobj)],
        [ld, '/dll', '/noentry', '/nodefaultlib', '/machine:x86', '/base:0x10000000',
         '/out:' + str(DLL), str(fobj), str(mobj), str(sobj)] + ['/export:' + s for s in exports]
    ]
    for cmd in commands:
        subprocess.run(cmd, cwd=ROOT, check=True)
    assert all(p.is_file() and p.stat().st_size for p in [fobj, mobj, sobj, DLL])


def inspect_rebuilt(pe, symbols):
    text = next(s for s in pe.sections if s.Name.startswith(b'.text'))
    ins = Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        pe.get_data(text.VirtualAddress, text.Misc_VirtualSize),
        pe.OPTIONAL_HEADER.ImageBase + text.VirtualAddress)
    expected_calls = [
        symbols['File_CheckReadable'],
        symbols['File_GetSize'],
        symbols['Mem_Alloc'],
        symbols['fopen'],
        symbols['fsetpos'],
        symbols['fread'],
        symbols['fclose']
    ]
    start = symbols['File_LoadToMemory']
    end = min(a for a in symbols.values() if a > start)
    actual_calls = []
    for i in ins:
        if i.mnemonic != 'call':
            continue
        if start <= i.address < end:
            assert i.op_str.startswith('0x'), 'Indirect call in File_LoadToMemory'
            actual_calls.append(int(i.op_str, 16))
    assert actual_calls == expected_calls, f'Calls mismatch: {actual_calls} != {expected_calls}'


class LoaderCPU:
    def __init__(self, pe, symbols=None):
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.size = (pe.OPTIONAL_HEADER.SizeOfImage + 4095) & ~4095
        self.cpu = Uc(UC_ARCH_X86, UC_MODE_32)
        self.cpu.mem_map(self.base, self.size)
        self.cpu.mem_write(self.base, pe.get_memory_mapped_image())
        self.cpu.mem_map(DATA, DATA_SIZE)
        self.cpu.mem_map(ARENA, ARENA_SIZE)
        self.cpu.mem_map(STACK, 0x10000)
        self.cpu.mem_map(STOP, 0x1000)

        self.entry = 0x4574a0 if symbols is None else symbols['File_LoadToMemory']
        self.error_addr = 0x4bab34 if symbols is None else symbols['g_fileErrorLine']
        self.pools_addr = 0x63c6a0 if symbols is None else symbols['g_memPools']

        if symbols is None:
            self.boundaries = {0x400000 + info[0]: name for name, info in BOUNDARIES.items()}
        else:
            self.boundaries = {symbols[name]: name for name in BOUNDARIES}

        self.cpu.hook_add(UC_HOOK_CODE, self.hook)

    def hook(self, cpu, address, n, unused):
        if address not in self.boundaries:
            return
        sp = cpu.reg_read(UC_X86_REG_ESP)
        name = self.boundaries[address]
        assert self.call_idx < len(self.expected_calls), f'Extra boundary call: {name}'
        exp_name, exp_args, ret_val = self.expected_calls[self.call_idx]
        assert name == exp_name, f'Call order mismatch: {name} != {exp_name}'

        # Extract arguments
        if name in ('fopen', 'fsetpos'):
            words = struct.unpack('<II', cpu.mem_read(sp + 4, 8))
            if name == 'fopen':
                mode_str = bytes(cpu.mem_read(words[1], 3)).split(b'\0')[0] + b'\0'
                actual_args = (words[0], mode_str)
            else:  # fsetpos: stream, &pos (pos is 2 dwords)
                pos = struct.unpack('<II', cpu.mem_read(words[1], 8))
                actual_args = (words[0], pos)
        elif name == 'fread':
            words = struct.unpack('<IIII', cpu.mem_read(sp + 4, 16))
            actual_args = words
        elif name == 'fseek':
            words = struct.unpack('<III', cpu.mem_read(sp + 4, 12))
            actual_args = words
        else:  # fclose, ftell, malloc, free
            actual_args = struct.unpack('<I', cpu.mem_read(sp + 4, 4))

        assert actual_args == exp_args, f'{name} args mismatch: {actual_args} != {exp_args}'
        self.call_idx += 1

        cpu.reg_write(UC_X86_REG_EAX, ret_val)
        cpu.reg_write(UC_X86_REG_ECX, 0xc1c1c1c1)
        cpu.reg_write(UC_X86_REG_EDX, 0xd2d2d2d2)
        cpu.reg_write(UC_X86_REG_EFLAGS, 0x43)
        cpu.reg_write(UC_X86_REG_EIP, struct.unpack('<I', cpu.mem_read(sp, 4))[0])
        cpu.reg_write(UC_X86_REG_ESP, sp + 4)

    def invoke(self, filename_ptr, expected_calls, initial_error=0, seed=0):
        self.call_idx = 0
        self.expected_calls = expected_calls
        cpu = self.cpu

        # Initialize arena and pools
        cpu.mem_write(ARENA, b'\0' * ARENA_SIZE)
        table = [ARENA + 0x2000] * 256
        table[0] = ARENA
        cpu.mem_write(self.pools_addr, struct.pack('<256I', *table))
        cpu.mem_write(self.error_addr, struct.pack('<I', initial_error))

        sp = STACK + 0x8000
        stack = bytes((i + seed) % 256 for i in range(64))
        cpu.mem_write(sp, struct.pack('<II', STOP, filename_ptr) + stack)

        rng = random.Random(seed)
        saved = [rng.getrandbits(32) for _ in SAVED]
        for reg, val in zip(SAVED, saved):
            cpu.reg_write(reg, val)
        cpu.reg_write(UC_X86_REG_ESP, sp)
        cpu.reg_write(UC_X86_REG_EFLAGS, 2)

        cpu.emu_start(self.entry, STOP, count=500000)

        assert cpu.reg_read(UC_X86_REG_EIP) == STOP
        ret_eax = cpu.reg_read(UC_X86_REG_EAX)
        assert cpu.reg_read(UC_X86_REG_ESP) == sp + 4
        assert [cpu.reg_read(r) for r in SAVED] == saved
        assert not cpu.reg_read(UC_X86_REG_EFLAGS) & 0x400
        assert self.call_idx == len(expected_calls), f'Not all calls made: {self.call_idx}/{len(expected_calls)}'

        final_error = struct.unpack('<I', cpu.mem_read(self.error_addr, 4))[0]
        return ret_eax, final_error


def build_scenario(scenario, filename_ptr, size, stream_read, stream_bin, read_bytes, payload_ptr):
    """Generates expected calls sequence for a scenario."""
    calls = []
    # 1. File_CheckReadable -> fopen(filename, "r")
    calls.append(('fopen', (filename_ptr, b'r\0'), stream_read if scenario != 'check_fail' else 0))
    if scenario == 'check_fail':
        return 0, 2030, calls
    calls.append(('fclose', (stream_read,), 0))

    # 2. File_GetSize -> fopen(filename, "r"), ftell, fseek, ftell, fseek, fclose
    calls.append(('fopen', (filename_ptr, b'r\0'), stream_read))
    calls.append(('ftell', (stream_read,), 0))
    calls.append(('fseek', (stream_read, 0, 2), 0))
    calls.append(('ftell', (stream_read,), size if scenario != 'size_zero' else 0))
    calls.append(('fseek', (stream_read, 0, 0), 0))
    calls.append(('fclose', (stream_read,), 0))
    if scenario == 'size_zero':
        return 0, 2040, calls

    # 3. Mem_Alloc(0, size)
    # Inside Mem_Alloc: pool table is empty, so it allocates pages (256), block (128), payload (size)
    if scenario == 'alloc_fail':
        calls.append(('malloc', (256,), 0))
        return 0, 2050, calls
    page_ptr = ARENA + 0x1000
    block_ptr = ARENA + 0x2000
    calls.append(('malloc', (256,), page_ptr))
    calls.append(('malloc', (128,), block_ptr))
    calls.append(('malloc', (size,), payload_ptr))

    # 4. fopen(filename, "rb")
    calls.append(('fopen', (filename_ptr, b'rb\0'), stream_bin if scenario != 'open_bin_fail' else 0))
    if scenario == 'open_bin_fail':
        return 0, 2000, calls

    # 5. fsetpos(stream, &pos) with pos=[0, 0]
    calls.append(('fsetpos', (stream_bin, (0, 0)), 0))

    # 6. fread(buffer, 1, size, stream)
    calls.append(('fread', (payload_ptr, 1, size, stream_bin), read_bytes))
    if scenario == 'read_short':
        return 0, 2010, calls

    # 7. fclose(stream)
    calls.append(('fclose', (stream_bin,), 0))
    return payload_ptr, None, calls


def verify_file_load():
    verify_target()
    phase = 'compilation'
    try:
        original = pefile.PE(str(TARGET))
        inspect_original(original)
        build_loader()
        rebuilt = pefile.PE(str(DLL))
        symbols = {s.name.decode(): rebuilt.OPTIONAL_HEADER.ImageBase + s.address
            for s in rebuilt.DIRECTORY_ENTRY_EXPORT.symbols if s.name}
        inspect_rebuilt(rebuilt, symbols)

        record('compilation', 'pass',
            scope='Complete production file.c and mem.c; strict C89; provisional Clang/LLD; real helpers and allocator with nonreturning CRT boundaries',
            compiler='Clang/LLD 19.1.1 provisional i686-pc-windows-msvc')

        raw_rebuilt = rebuilt.get_data(symbols['File_LoadToMemory'] - rebuilt.OPTIONAL_HEADER.ImageBase, SIZE)
        raw_orig = original.get_data(RVA, SIZE)
        record('raw_bytes', 'pass' if raw_rebuilt == raw_orig else 'different',
            scope='Prefix diagnostic only; instruction equality unverified')

        phase = 'emulation'
        a = LoaderCPU(original)
        b = LoaderCPU(rebuilt, symbols)

        fn_ptr = DATA + 0x100
        # Write filename into DATA
        for cpu in (a.cpu, b.cpu):
            cpu.mem_write(fn_ptr, b'test_track.pic\0')

        cases = 0
        stream_r = DATA + 0x200
        stream_b = DATA + 0x300
        payload_base = ARENA + 0x10000

        scenarios = ['check_fail', 'size_zero', 'alloc_fail', 'open_bin_fail', 'read_short', 'success']
        test_sizes = [1, 16, 64, 256, 1024, 65536, 0x100000]

        for sc in scenarios:
            for sz in test_sizes:
                if sc in ('check_fail', 'size_zero') and sz != 16:
                    continue
                for init_err in (0, 1000, 2030, 0x12345678):
                    read_b = sz if sc != 'read_short' else (sz - 1 if sz > 1 else 0)
                    exp_ret, exp_err, calls = build_scenario(sc, fn_ptr, sz, stream_r, stream_b, read_b, payload_base)
                    final_expected_err = exp_err if exp_err is not None else init_err

                    ret_a, err_a = a.invoke(fn_ptr, calls, initial_error=init_err, seed=cases)
                    ret_b, err_b = b.invoke(fn_ptr, calls, initial_error=init_err, seed=cases)

                    assert ret_a == ret_b == exp_ret, f'Return mismatch: orig={ret_a}, rebuilt={ret_b}, exp={exp_ret}'
                    assert err_a == err_b == final_expected_err, f'Error mismatch: orig={err_a}, rebuilt={err_b}, exp={final_expected_err}'
                    cases += 1

        # Additional random fuzz cases
        rng = random.Random(0x574a0)
        for i in range(120):
            sc = rng.choice(scenarios)
            sz = rng.randint(1, 0x80000)
            init_err = rng.getrandbits(32)
            read_b = sz if sc != 'read_short' else rng.randint(0, sz - 1)
            exp_ret, exp_err, calls = build_scenario(sc, fn_ptr, sz, stream_r, stream_b, read_b, payload_base)
            final_expected_err = exp_err if exp_err is not None else init_err

            ret_a, err_a = a.invoke(fn_ptr, calls, initial_error=init_err, seed=cases)
            ret_b, err_b = b.invoke(fn_ptr, calls, initial_error=init_err, seed=cases)

            assert ret_a == ret_b == exp_ret
            assert err_a == err_b == final_expected_err
            cases += 1

        record('emulation', 'pass', cases=cases,
            scope='Exact ordered dependency calls, real File_CheckReadable, File_GetSize, File_GetStreamSize and Mem_Alloc execution; error precedence 2030/2040/2050/2000/2010; position words; ABI and state parity',
            limitations='CRT I/O and malloc explicitly modeled; no native filesystem, concurrency/reentry, instruction equality or full game parity')
        print(f'PASS: {cases} File_LoadToMemory differential cases passed.')
        return cases
    except Exception as exc:
        record(phase, 'fail', error=str(exc))
        raise


if __name__ == '__main__':
    verify_file_load()
