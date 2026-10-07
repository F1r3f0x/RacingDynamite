# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Font_Load with real Mem_Free; explicit file/CRT-free/sprite boundary fixtures."""
import hashlib
import random
import struct
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import UC_HOOK_MEM_READ
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX,
                              UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EFLAGS)
from verify_font_parse import (ParserCPU, build_parser, oracle, INPUTS as PARSER_INPUTS,
                               inspect_original as inspect_parser, BUFFER)
from verify_font_cleanup import LIFECYCLE_FIELDS, SAVED, STACK, STOP
from verify_mem_free import PoolFixture, ARENA, ARENA_SIZE
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')
RVA, SIZE = 0x56420, 71
DLL = BUILD / 'font_load_validation.dll'
INPUTS = PARSER_INPUTS + ['tools/verify_font_load.py', 'tools/verify_mem_free.py', 'tools/build_decomp.py']
CALLERS = [0x4181d5, 0x4181e9, 0x4181fd, 0x418211, 0x418225, 0x418239,
           0x41824d, 0x418268, 0x41acfb, 0x41ad42, 0x41ad89, 0x41add0,
           0x41ae17, 0x41ae5e, 0x41aea5, 0x41aeec, 0x41af33]


def record(kind, outcome, cases=0, **details):
    record_run(RVA, kind, outcome, inputs=INPUTS,
               artifact=DLL.relative_to(ROOT).as_posix() if DLL.exists() else None,
               cases=cases, command=('uv run python tools/verify_font_load.py'
               if __name__ == '__main__' else 'uv run tools/verify_matching.py'), details=details)


def inspect_original(pe):
    code = pe.get_data(RVA, SIZE)
    assert hashlib.sha256(code).hexdigest() == 'f4c269fd2b0d4fbc491b4f4211b5dd22622c8c20d3d7e3f220eaa94093887441'
    assert pe.get_data(RVA + SIZE, 9) == b'\xcc' * 9
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {a: (n, local, params, bits) for a, n, local, params, bits in
           struct.iter_unpack('<IIIHH', pe.__data__[debug.PointerToRawData:
                                                     debug.PointerToRawData + debug.SizeOfData])}
    assert fpo[RVA] == (SIZE, 0, 2, 0x206)
    instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(code, 0x456420))
    assert sum(i.size for i in instructions) == SIZE
    assert len(instructions) == 28
    assert [(i.address, i.op_str) for i in instructions if i.mnemonic == 'call'] == [
        (0x456427, '0x4574a0'), (0x45644d, '0x456270'), (0x45645a, '0x45b000')]
    relocs = [(e.rva, struct.unpack('<I', pe.get_data(e.rva, 4))[0])
              for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries
              if e.type == 3 and RVA <= e.rva < RVA + SIZE]
    assert relocs == [(0x5643d, 0x4bab34)]
    for address in CALLERS:
        raw = pe.get_data(address - 0x400000, 8)
        assert raw[0] == 0xe8 and address + 5 + struct.unpack_from('<i', raw, 1)[0] == 0x456420
        assert raw[5:] == b'\x83\xc4\x08'
    # Authenticate the native boundaries inspected in Ghidra; do not claim their execution.
    for rva, size, digest in [
        (0x574a0, 230, '2a23d52a4c2588198a7749db30e5de8c2f5ecd150a65f5940dbc2a1a5db87ec3'),
        (0x5b000, 127, '40c455ee2388ee3160f714b07885f4cc9f3fb5401ab86cd2bea5da8749acd471')]:
        assert fpo[rva][0] == size
        assert hashlib.sha256(pe.get_data(rva, size)).hexdigest() == digest
    assert pe.get_data(0xbab34, 4) == b'\0' * 4
    inspect_parser(pe)


def put(state, name, value):
    state[name] = struct.pack('<I', value)


class LoaderCPU(ParserCPU):
    def __init__(self, pe, symbols=None):
        super().__init__(pe, symbols)
        for name, address in [('Font_Load', 0x456420), ('File_LoadToMemory', 0x4574a0),
                              ('Mem_Free', 0x45b000)]:
            self.entries[name] = address if self.original else symbols[name]
        self.entries['CRT_free'] = 0x4693b0 if self.original else symbols['free']
        self.fields.append(('g_memPools', 0x63c6a0 if self.original else symbols['g_memPools'], 1024))
        self.cpu.mem_map(ARENA, ARENA_SIZE)
        self.freed = False
        self.cpu.hook_add(UC_HOOK_MEM_READ, self.check_read, begin=BUFFER, end=BUFFER+0xffff)

    def check_read(self, cpu, access, address, size, value, unused):
        assert not self.freed, 'Read from released buffer'

    def boundary_return(self, value):
        cpu = self.cpu
        sp = cpu.reg_read(UC_X86_REG_ESP)
        cpu.reg_write(UC_X86_REG_EAX, value)
        # ABI permits these registers/flags to be destroyed by each dependency.
        cpu.reg_write(UC_X86_REG_ECX, 0xc1c1c1c1)
        cpu.reg_write(UC_X86_REG_EDX, 0xd2d2d2d2)
        cpu.reg_write(UC_X86_REG_EFLAGS, 0x43)
        cpu.reg_write(UC_X86_REG_EIP, struct.unpack('<I', cpu.mem_read(sp, 4))[0])
        cpu.reg_write(UC_X86_REG_ESP, sp + 4)

    def hook(self, cpu, address, length, unused):
        sp = cpu.reg_read(UC_X86_REG_ESP)
        if address == self.entries['File_LoadToMemory']:
            args = struct.unpack('<I', cpu.mem_read(sp + 4, 4))
            assert args == (self.filename,)
            assert self.state() == self.initial_state
            assert not self.calls
            self.calls.append(('File_LoadToMemory', args))
            if self.load_error is not None:
                cpu.mem_write(self.addresses_error, struct.pack('<I', self.load_error))
            self.boundary_return(BUFFER if self.loaded else 0)
        elif address == self.entries['Font_Parse']:
            args = struct.unpack('<II', cpu.mem_read(sp + 4, 8))
            assert self.loaded and args == (BUFFER, self.forwarded)
            assert self.state() == self.parse_state
            self.calls.append(('Font_Parse', args))
            if self.parse_result is not None:
                if self.parse_error is not None:
                    cpu.mem_write(self.addresses_error, struct.pack('<I', self.parse_error))
                self.boundary_return(self.parse_result)
        elif address == self.entries['Mem_Free']:
            args = struct.unpack('<II', cpu.mem_read(sp + 4, 8))
            assert self.loaded and args == (0, BUFFER)
            assert self.state() == self.free_state
            assert bytes(cpu.mem_read(BUFFER, len(self.blob))) == self.blob
            self.calls.append(('Mem_Free', args))
            assert bytes(cpu.mem_read(ARENA, ARENA_SIZE)) == self.pool_before
            # Execute the complete native/reconstructed hierarchy traversal.
        elif address == self.entries['CRT_free']:
            args = struct.unpack('<I', cpu.mem_read(sp + 4, 4))
            assert args == (BUFFER,) and self.tracked
            assert self.state() == self.free_state
            assert bytes(cpu.mem_read(ARENA, ARENA_SIZE)) == self.pool_before
            self.calls.append(('CRT_free', args))
            if self.free_error is not None:
                cpu.mem_write(self.addresses_error, struct.pack('<I', self.free_error))
            # Poison released bytes: there must be no subsequent parser/read access.
            cpu.mem_write(BUFFER, b'\xa5' * len(self.blob))
            self.freed = True
            self.boundary_return(self.free_result)
        else:
            super().hook(cpu, address, length, unused)

    def execute(self, state, blob, unused, filename, loaded, load_error,
                free_result, free_error, parse_result, parse_error, seed, returns, tracked=True):
        state = dict(state)
        fixture = PoolFixture()
        self.record_offset = fixture.match(63, 63, 15, BUFFER, 0x1000) if tracked else None
        self.pool_before = bytes(fixture.data)
        self.cpu.mem_write(ARENA, self.pool_before)
        table = [ARENA + 0x2000]*256; table[0] = ARENA
        state['g_memPools'] = struct.pack('<256I', *table)
        self.tracked = tracked
        self.load(state)
        self.addresses_error = next(a for n, a, _ in self.fields if n == 'g_fileErrorLine')
        self.root_entry, self.calls, self.descriptors = 'Font_Load', [], []
        self.initial_state, self.blob = dict(state), blob
        self.filename, self.forwarded, self.loaded = filename, unused, loaded
        self.load_error, self.free_result, self.free_error = load_error, free_result, free_error
        self.parse_result, self.parse_error = parse_result, parse_error
        self.returns = returns
        self.parse_state = dict(state)
        if load_error is not None: put(self.parse_state, 'g_fileErrorLine', load_error)
        if not loaded:
            result, final, calls = 0xffffffff, dict(self.parse_state), [('File_LoadToMemory', (filename,))]
            put(final, 'g_fileErrorLine', 1000)
            self.expected_calls, self.expected_snapshots = [], []
        else:
            if parse_result is None:
                (result, final, parser_calls), desc, snapshots = oracle(self.parse_state, blob, returns)
                self.expected_calls, self.expected_snapshots = desc, snapshots
            else:
                result, final, parser_calls = parse_result, dict(self.parse_state), []
                if parse_error is not None: put(final, 'g_fileErrorLine', parse_error)
                self.expected_calls, self.expected_snapshots = [], []
            self.free_state = dict(final)
            calls = [('File_LoadToMemory', (filename,)), ('Font_Parse', (BUFFER, unused))]
            calls += parser_calls + [('Mem_Free', (0, BUFFER))]
            if tracked:
                calls.append(('CRT_free', (BUFFER,)))
                if free_error is not None: put(final, 'g_fileErrorLine', free_error)
        cpu = self.cpu
        cpu.mem_write(BUFFER, blob)
        sp = STACK + 0x8000
        cpu.mem_write(sp, struct.pack('<III', STOP, filename, unused))
        stack = bytes(cpu.mem_read(sp, 0x8000))
        data_before = bytes(cpu.mem_read(BUFFER, 0x10000))
        rng = random.Random(seed)
        saved = [rng.getrandbits(32) for _ in SAVED]
        for reg, value in zip(SAVED, saved): cpu.reg_write(reg, value)
        cpu.reg_write(UC_X86_REG_ESP, sp)
        cpu.reg_write(UC_X86_REG_EFLAGS, 2)
        before = bytes(cpu.mem_read(self.base, self.size))
        cpu.emu_start(self.entries['Font_Load'], STOP, count=1000000)
        assert cpu.reg_read(UC_X86_REG_EIP) == STOP
        assert cpu.reg_read(UC_X86_REG_ESP) == sp + 4
        assert [cpu.reg_read(r) for r in SAVED] == saved
        assert not cpu.reg_read(UC_X86_REG_EFLAGS) & 0x400
        assert bytes(cpu.mem_read(sp, 0x8000)) == stack
        data_after = bytes(cpu.mem_read(BUFFER, 0x10000))
        assert data_after == (b'\xa5' * len(blob) + data_before[len(blob):] if loaded and tracked else data_before)
        pool_after = bytearray(self.pool_before)
        if loaded and tracked: struct.pack_into('<I', pool_after, self.record_offset+4, 0)
        assert bytes(cpu.mem_read(ARENA, ARENA_SIZE)) == bytes(pool_after)
        assert len(self.descriptors) == len(self.expected_calls)
        after = bytearray(cpu.mem_read(self.base, self.size))
        for _, address, size in self.fields:
            after[address-self.base:address-self.base+size] = before[address-self.base:address-self.base+size]
        assert bytes(after) == before, 'Unrelated image mutation'
        actual = cpu.reg_read(UC_X86_REG_EAX), self.state(), self.calls
        assert actual == (result, final, calls), (seed, actual[0], result)
        return actual


def verify_font_load():
    verify_target()
    phase = 'compilation'
    try:
        original = pefile.PE(str(TARGET)); inspect_original(original)
        commands = build_parser(DLL, include_loader=True)
        record('compilation', 'pass', commands=commands,
               scope='Extracted production Font_Load/parser/lifecycle and complete production mem.c; separate focused DLL')
        rebuilt = pefile.PE(str(DLL))
        symbols = {s.name.decode(): rebuilt.OPTIONAL_HEADER.ImageBase+s.address
                   for s in rebuilt.DIRECTORY_ENTRY_EXPORT.symbols if s.name}
        prefix = rebuilt.get_data(symbols['Font_Load']-rebuilt.OPTIONAL_HEADER.ImageBase, SIZE)
        record('raw_bytes', 'pass' if prefix == original.get_data(RVA, SIZE) else 'different',
               scope='Raw prefix diagnostic; compiled extent and instruction equality unverified')
        phase = 'emulation'
        rng, cases, counts = random.Random(RVA), 0, {'null': 0, 'real_parser': 0, 'modeled_parser': 0}

        def compare(loaded=True, flag=1, slot=0, pattern='sparse', magic=b'LFT\0', version=100,
                    enabled=1, cursor=0, registration=0, load_error=None, free_error=None,
                    parse_result=None, parse_error=None, tracked=True):
            nonlocal cases
            state = {n: rng.randbytes(size) for n, _, size in LIFECYCLE_FIELDS}
            put(state, 'g_fileErrorLine', rng.getrandbits(32))
            put(state, 'g_fontSystemInitialized', flag)
            put(state, 'g_memHandlesInitialized', enabled)
            state['g_memHandleCursor'] = struct.pack('<h', cursor)
            state['g_memHandleIds'] = struct.pack('<200h', *[rng.choice([0, 1, -1, 32767, -32768]) for _ in range(200)])
            statuses = [1]*200
            if registration is not None: statuses[registration] = 0
            state['g_memHandleStatus'] = struct.pack('<200I', *statuses)
            fonts = bytearray(state['g_fonts'])
            for i in range(30): struct.pack_into('<I', fonts, i*1600, rng.choice([1, 2, 0x80000000, 0xffffffff]))
            if slot is not None: struct.pack_into('<I', fonts, slot*1600, 0)
            state['g_fonts'] = bytes(fonts)
            blob = bytearray(rng.randbytes(0x1000)); blob[:4] = magic
            words = [0, 0x7fff, 0x8000, 0xffff]
            struct.pack_into('<4H', blob, 4, version, *[rng.choice(words) for _ in range(3)])
            for i in range(224):
                present = pattern == 'full' or pattern == 'sparse' and i in (0, 113, 223) or pattern == 'random' and rng.randrange(2)
                struct.pack_into('<I', blob, 12+4*i, rng.choice([0, 1, 0xfffffffe, 0x80000000, 0x7fffffff]) if present else 0xffffffff)
                struct.pack_into('<H', blob, 0x68c+2*i, rng.choice(words))
            returns = [rng.choice([0, 1, 0xffffffff, 0x80000000, rng.getrandbits(32)]) for _ in range(224)]
            # Raw filename dwords test forwarding only; the modeled loader never reads them.
            filename, unused, free_result = rng.getrandbits(32), rng.getrandbits(32), rng.getrandbits(32)
            a, b = LoaderCPU(original), LoaderCPU(rebuilt, symbols)
            args = (state, bytes(blob), unused, filename, loaded, load_error,
                    free_result, free_error, parse_result, parse_error, cases, returns, tracked)
            assert a.execute(*args) == b.execute(*args)
            counts['null' if not loaded else 'real_parser' if parse_result is None else 'modeled_parser'] += 1
            cases += 1

        for flag in [0, 1, 2, 0x80000000, 0xffffffff]:
            for error in [None, 2000, 2010, 2020, 2030, 2040, 0xffffffff]:
                compare(loaded=False, flag=flag, load_error=error)
        for slot in range(30):
            for pattern in ['empty', 'sparse', 'full']: compare(slot=slot, pattern=pattern)
        for flag in [0, 1, 2, 0x80000000, 0xffffffff]:
            for byte in range(4):
                magic = bytearray(b'LFT\0'); magic[byte] ^= 0xff
                compare(flag=flag, magic=bytes(magic), version=0, slot=None)
            for version in [0, 99, 101, 0x8000, 0xffff]: compare(flag=flag, version=version, slot=None)
            compare(flag=flag, slot=None)
        for enabled in [0, 1, 2, 0xffffffff]:
            for cursor in [0, 198, 199, 200, 32767]:
                for registration in [0, 199, None]:
                    compare(flag=0, enabled=enabled, cursor=cursor, registration=registration)
        for _ in range(60):
            compare(flag=rng.choice([0, 1, 2, 0xffffffff]), slot=rng.randrange(30), pattern='random',
                    load_error=rng.getrandbits(32), free_error=rng.getrandbits(32))
        for result in [0, 1, 29, 30, 0x7fffffff, 0x80000000, 0xfffffffe, 0xffffffff]:
            for free_error in [None, 0, 1000, 0xffffffff]:
                compare(parse_result=result, parse_error=rng.getrandbits(32),
                        load_error=rng.getrandbits(32), free_error=free_error)
        for slot in [0, 15, 29]:
            for version in [100, 99]:
                compare(slot=slot, version=version, tracked=False, free_error=0xffffffff)
        record('emulation', 'pass', cases=cases, case_groups=counts,
               scope='Return, full font/handle/error state, ordered calls, descriptors, parser/free boundary snapshots and ABI; actual parser/lazy handles and Mem_Free',
               limitations='File_LoadToMemory, CRT free and sprite creation modeled; real Mem_Free executes; 32 isolated parser-return fixtures; no native I/O/freeing, reentry, aliased/invalid buffers, instruction equality or native parity')
        print(f'PASS: {cases} Font_Load original-vs-C cases; {counts}; state, ordered calls and ABI equality.')
    except Exception as exc:
        record(phase, 'fail', error=str(exc))
        raise


if __name__ == '__main__':
    verify_font_load()
