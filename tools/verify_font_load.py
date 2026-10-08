# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Font_Load: real file/allocator/parser/free chain and isolated wrapper fixtures."""
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
from verify_mem_alloc import (oracle as allocation_oracle, fixture as allocation_fixture,
                              NEW_PAGE, NEW_BLOCK, inspect_original as inspect_alloc)
from verify_mem_free import oracle as release_oracle, inspect_original as inspect_free
from verify_file_load import inspect_original as inspect_file, inspect_rebuilt as inspect_file_link
from verify_file_helpers import inspect_original as inspect_helpers
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')
RVA, SIZE = 0x56420, 71
DLL = BUILD / 'font_load_validation.dll'
INPUTS = PARSER_INPUTS + ['tools/verify_font_load.py', 'tools/verify_mem_free.py',
    'tools/verify_mem_alloc.py', 'tools/verify_file_load.py', 'tools/verify_file_helpers.py',
    'decomp/src/file.c', 'decomp/include/file.h', 'tools/build_decomp.py']
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



class IntegratedLoaderCPU(LoaderCPU):
    """Executes all game bodies; only CRT I/O/heap and sprites are intercepted."""
    def __init__(self, pe, symbols=None):
        super().__init__(pe, symbols)
        self.crt = {name: (address if self.original else symbols[name]) for name, address in [
            ('fopen', 0x469390), ('fclose', 0x469100), ('ftell', 0x469de0),
            ('fseek', 0x469d40), ('fsetpos', 0x469d20), ('fread', 0x469170),
            ('malloc', 0x469400), ('free', 0x4693b0)]}
        self.game = {name: (address if self.original else symbols[name]) for name, address in [
            ('File_CheckReadable', 0x4576b0), ('File_GetSize', 0x457630),
            ('File_GetStreamSize', 0x4575f0), ('Mem_Alloc', 0x45ae10)]}

    def hook(self, cpu, address, length, unused):
        sp = cpu.reg_read(UC_X86_REG_ESP)
        if address == getattr(self, 'loader_return', None):
            assert cpu.reg_read(UC_X86_REG_EAX) == self.load_pointer
            assert self.state() == self.parse_state, 'Loader return state/error'
            assert bytes(cpu.mem_read(ARENA, ARENA_SIZE)) == self.pool_before
            self.loader_return = None
        if address == self.entries['File_LoadToMemory']:
            assert self.state() == self.initial_state
            assert not self.calls
            args = struct.unpack('<I', cpu.mem_read(sp + 4, 4))
            assert args == (self.filename,)
            self.calls.append(('File_LoadToMemory', args))
            self.loader_return = struct.unpack('<I', cpu.mem_read(sp, 4))[0]
            return  # Never synthesize the game loader's return.
        name = next((n for n, a in self.game.items() if a == address), None)
        if name:
            count = 2 if name == 'Mem_Alloc' else 1
            args = struct.unpack('<' + 'I'*count, cpu.mem_read(sp + 4, 4*count))
            self.calls.append((name, args))
            return  # Observe, then execute actual helper/allocator instructions.
        name = next((n for n, a in self.crt.items() if a == address), None)
        if name:
            assert self.crt_index < len(self.crt_plan), 'Unexpected CRT call: ' + name
            expected_name, expected_args, result, pool, error_before, error_after = self.crt_plan[self.crt_index]
            assert name == expected_name, (name, expected_name)
            count = 4 if name == 'fread' else 3 if name == 'fseek' else 2 if name in ('fopen', 'fsetpos') else 1
            args = struct.unpack('<' + 'I'*count, cpu.mem_read(sp + 4, 4*count))
            if name == 'fopen':
                assert bytes(cpu.mem_read(args[0], 15)) == b'font_fixture.lf'
                mode = bytes(cpu.mem_read(args[1], 3)).split(b'\0')[0] + b'\0'
                args = (args[0], mode)
            elif name == 'fsetpos':
                args = (args[0], struct.unpack('<II', cpu.mem_read(args[1], 8)))
            assert args == expected_args, (name, args, expected_args)
            assert bytes(cpu.mem_read(ARENA, ARENA_SIZE)) == pool, 'Pool state at ' + name
            expected_state = dict(self.free_state if name == 'free' else self.initial_state)
            put(expected_state, 'g_fileErrorLine', error_before)
            assert self.state() == expected_state, 'State at ' + name
            self.calls.append(('CRT_' + name, args))
            self.crt_index += 1
            if name == 'fread':
                # This fixture really transfers bytes; parser input is not preloaded.
                cpu.mem_write(BUFFER, self.blob[:result])
            if name == 'free':
                assert bytes(cpu.mem_read(BUFFER, len(self.blob))) == self.blob
                cpu.mem_write(BUFFER, b'\xa5' * len(self.blob))
                self.freed = True
            cpu.mem_write(self.addresses_error, struct.pack('<I', error_after))
            self.boundary_return(result)
            return
        super().hook(cpu, address, length, unused)

    def execute_chain(self, state, blob, forwarded, scenario, fixture, malloc_returns,
                      returns, seed, io_error=None, free_error=None):
        """Independent instruction-derived file sequence + allocator/parser/free oracles."""
        state = dict(state)
        table = [ARENA + 0x2000]*256; table[0] = ARENA
        state['g_memPools'] = struct.pack('<256I', *table)
        self.filename = ARENA + 0xf0000
        fixture.data[0xf0000:0xf0010] = b'font_fixture.lf\0'
        initial_pool = bytes(fixture.data)
        self.initial_state, self.blob = dict(state), blob
        self.forwarded, self.returns = forwarded, returns
        self.freed, self.loader_return = False, None
        self.parse_result, self.parse_error = None, None
        self.crt_plan, expected_calls = [], [('File_LoadToMemory', (self.filename,))]
        pool = initial_pool
        error = struct.unpack('<I', state['g_fileErrorLine'])[0]
        def game(name, args):
            expected_calls.append((name, args))
        def crt(name, args, result, snapshot=None, mutation=None):
            nonlocal error
            after = error if mutation is None else mutation
            self.crt_plan.append((name, args, result, pool if snapshot is None else snapshot, error, after))
            expected_calls.append(('CRT_' + name, args))
            error = after
        stream_r, stream_b = ARENA + 0xf0100, ARENA + 0xf0200
        game('File_CheckReadable', (self.filename,))
        crt('fopen', (self.filename, b'r\0'), 0 if scenario == 'check_fail' else stream_r)
        load_error, pointer = 2030, 0
        if scenario != 'check_fail':
            crt('fclose', (stream_r,), 0xffffffff)
            game('File_GetSize', (self.filename,))
            crt('fopen', (self.filename, b'r\0'), stream_r)
            game('File_GetStreamSize', (stream_r,))
            crt('ftell', (stream_r,), 0xffffffff)
            # End seek always uses zero; cached -1 is used only for restoration.
            crt('fseek', (stream_r, 0, 2), 0xffffffff)
            size = 0 if scenario == 'size_zero' else len(blob)
            crt('ftell', (stream_r,), size)
            crt('fseek', (stream_r, 0xffffffff, 0), 0xffffffff)
            crt('fclose', (stream_r,), 0xffffffff)
            load_error = 2040
            if size:
                game('Mem_Alloc', (0, size))
                pointer, allocated, _, snapshots, requests = allocation_oracle(pool, 0, size, malloc_returns, {})
                for request, value, snapshot in zip(requests, malloc_returns, snapshots):
                    crt('malloc', (request,), value, snapshot)
                pool = allocated
                load_error = 2050
                if pointer:
                    assert pointer == BUFFER
                    crt('fopen', (self.filename, b'rb\0'), 0 if scenario == 'open_bin_fail' else stream_b)
                    load_error = 2000
                    if scenario != 'open_bin_fail':
                        crt('fsetpos', (stream_b, (0, 0)), 0xffffffff)
                        count = len(blob)-1 if scenario == 'read_short' else len(blob)
                        crt('fread', (BUFFER, 1, len(blob), stream_b), count, mutation=io_error)
                        load_error = 2010
                        if scenario != 'read_short':
                            crt('fclose', (stream_b,), 0xffffffff)
                            load_error = None
        self.loaded = load_error is None
        self.load_pointer = BUFFER if self.loaded else 0
        self.pool_before = pool
        self.parse_state = dict(state)
        put(self.parse_state, 'g_fileErrorLine', error if self.loaded else load_error)
        self.expected_calls, self.expected_snapshots = [], []
        if self.loaded:
            (result, final, parser_calls), descriptors, snapshots = oracle(self.parse_state, blob, returns)
            self.expected_calls, self.expected_snapshots = descriptors, snapshots
            self.free_state = dict(final)
            expected_calls += [('Font_Parse', (BUFFER, forwarded))] + parser_calls + [('Mem_Free', (0, BUFFER))]
            freed_result, final_pool, _, _ = release_oracle(pool, 0, BUFFER, ())
            assert freed_result == 1, 'Allocated buffer must be reachable by Mem_Free'
            error = struct.unpack('<I', final['g_fileErrorLine'])[0]
            free_result = random.Random(seed).getrandbits(32)
            crt('free', (BUFFER,), free_result, mutation=free_error)
            put(final, 'g_fileErrorLine', error)
        else:
            result, final, final_pool = 0xffffffff, dict(state), pool
            put(final, 'g_fileErrorLine', 1000)
            self.free_state = None
        cpu = self.cpu
        self.load(state)
        self.addresses_error = next(a for n, a, _ in self.fields if n == 'g_fileErrorLine')
        self.root_entry, self.calls, self.descriptors, self.crt_index = 'Font_Load', [], [], 0
        cpu.mem_write(ARENA, initial_pool)
        cpu.mem_write(BUFFER, b'\xcc' * 0x10000)
        initial_buffer = bytes(cpu.mem_read(BUFFER, 0x10000))
        sp = STACK + 0x8000
        cpu.mem_write(sp, struct.pack('<III', STOP, self.filename, forwarded))
        stack = bytes(cpu.mem_read(sp, 0x8000))
        saved = [random.Random(seed+i).getrandbits(32) for i in range(len(SAVED))]
        for reg, value in zip(SAVED, saved): cpu.reg_write(reg, value)
        cpu.reg_write(UC_X86_REG_ESP, sp); cpu.reg_write(UC_X86_REG_EFLAGS, 2)
        before = bytes(cpu.mem_read(self.base, self.size))
        cpu.emu_start(self.entries['Font_Load'], STOP, count=3000000)
        assert cpu.reg_read(UC_X86_REG_EIP) == STOP and self.loader_return is None
        assert cpu.reg_read(UC_X86_REG_ESP) == sp + 4
        assert [cpu.reg_read(r) for r in SAVED] == saved
        assert not cpu.reg_read(UC_X86_REG_EFLAGS) & 0x400
        assert bytes(cpu.mem_read(sp, 0x8000)) == stack
        assert self.crt_index == len(self.crt_plan)
        assert len(self.descriptors) == len(self.expected_calls)
        assert bytes(cpu.mem_read(ARENA, ARENA_SIZE)) == final_pool
        transferred = len(blob)-1 if scenario == 'read_short' else len(blob) if self.loaded else 0
        prefix = b'\xa5'*len(blob) if self.loaded else blob[:transferred]
        assert bytes(cpu.mem_read(BUFFER, 0x10000)) == prefix + initial_buffer[len(prefix):]
        after = bytearray(cpu.mem_read(self.base, self.size))
        for _, address, size in self.fields:
            after[address-self.base:address-self.base+size] = before[address-self.base:address-self.base+size]
        assert bytes(after) == before, 'Unrelated image mutation'
        actual = cpu.reg_read(UC_X86_REG_EAX), self.state(), self.calls, final_pool
        assert actual == (result, final, expected_calls, final_pool), (seed, actual[0], result, self.calls, expected_calls)
        return actual


def verify_integrated_load(original, rebuilt, symbols):
    rng, cases, groups = random.Random(0x574a0), 0, {}
    def compare(scenario='success', branch='page', failure=None, flag=0, slot=0,
                magic=b'LFT\0', version=100, enabled=1, cursor=0, registration=0,
                pattern='sparse', io_error=None, free_error=None, repeat=False):
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
        sprite_returns = [rng.choice([0, 1, 0xffffffff, 0x80000000, rng.getrandbits(32)]) for _ in range(224)]
        fixture = allocation_fixture(b=1 if branch == 'block' else 0, r=15, branch=branch)
        malloc_returns = {'page': (NEW_PAGE, NEW_BLOCK, BUFFER), 'block': (NEW_BLOCK, BUFFER), 'record': (BUFFER,)}[branch]
        if failure is not None: malloc_returns = malloc_returns[:failure] + (0,)
        args = (state, bytes(blob), rng.getrandbits(32), scenario, fixture, malloc_returns,
                sprite_returns, cases, io_error, free_error)
        a, b = IntegratedLoaderCPU(original), IntegratedLoaderCPU(rebuilt, symbols)
        actual = a.execute_chain(*args)
        assert actual == b.execute_chain(*args)
        if repeat:
            fixture.data[:] = actual[3]
            repeated = (actual[1], bytes(blob), rng.getrandbits(32), 'success', fixture,
                        (BUFFER,), sprite_returns, cases+1, io_error, free_error)
            assert a.execute_chain(*repeated) == b.execute_chain(*repeated)
            groups['reuse'] = groups.get('reuse', 0) + 1
            cases += 1
        key = 'alloc_fail' if failure is not None else scenario
        groups[key] = groups.get(key, 0) + 1
        cases += 1
    for flag in [0, 1, 2, 0x80000000, 0xffffffff]:
        for scenario in ['check_fail', 'size_zero', 'open_bin_fail', 'read_short']:
            for branch in ['page', 'block', 'record']: compare(scenario=scenario, branch=branch, flag=flag)
        for branch, length in [('page', 3), ('block', 2), ('record', 1)]:
            for failure in range(length): compare(branch=branch, failure=failure, flag=flag)
    for branch in ['page', 'block', 'record']:
        for slot in range(30):
            for pattern in ['empty', 'sparse', 'full']: compare(branch=branch, flag=1, slot=slot, pattern=pattern)
    for flag in [0, 1, 2, 0x80000000, 0xffffffff]:
        for byte in range(4):
            magic = bytearray(b'LFT\0'); magic[byte] ^= 0xff
            compare(flag=flag, magic=bytes(magic), version=0, slot=None)
        for version in [0, 99, 101, 0x8000, 0xffff]: compare(flag=flag, version=version, slot=None)
        compare(flag=flag, slot=None)
    for enabled in [0, 1, 2, 0xffffffff]:
        for cursor in [0, 198, 199, 200, 32767]:
            for registration in [0, 199, None]: compare(enabled=enabled, cursor=cursor, registration=registration)
    for _ in range(60):
        compare(branch=rng.choice(['page', 'block', 'record']), flag=rng.choice([0, 1, 2, 0xffffffff]),
                slot=rng.randrange(30), pattern='random', io_error=rng.getrandbits(32), free_error=rng.getrandbits(32))
    for branch in ['page', 'block', 'record']:
        compare(branch=branch, repeat=True)
    print(f'PASS: {cases} real-chain Font_Load differential executions; {groups}.')
    return cases, groups


def verify_font_load():
    verify_target()
    phase = 'compilation'
    try:
        original = pefile.PE(str(TARGET)); inspect_original(original)
        inspect_file(original); inspect_helpers(original); inspect_alloc(original); inspect_free(original)
        commands = build_parser(DLL, include_loader=True, real_file=True)
        record('compilation', 'pass', commands=commands,
               scope='Extracted production Font_Load/parser/lifecycle, complete production file.c and mem.c; separate wrapper translation unit; strict C89 focused DLL')
        rebuilt = pefile.PE(str(DLL))
        symbols = {s.name.decode(): rebuilt.OPTIONAL_HEADER.ImageBase+s.address
                   for s in rebuilt.DIRECTORY_ENTRY_EXPORT.symbols if s.name}
        inspect_file_link(rebuilt, symbols)
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
        integrated_cases, integrated_groups = verify_integrated_load(original, rebuilt, symbols)
        record('emulation', 'pass', cases=cases+integrated_cases, case_groups=counts,
               integrated_case_groups=integrated_groups,
               scope='Real File_LoadToMemory/helpers/Mem_Alloc/parser/lazy handles/Mem_Free chain; full state and hierarchy at CRT boundaries/return, transferred payload, descriptors, ordered calls and ABI; retained isolated wrapper suite',
               limitations='CRT I/O/malloc/free and sprite creation modeled; prior 333 isolated wrapper cases retain modeled loader and 32 modeled parser returns; no native I/O/heap, reentry, aliased/invalid buffers, instruction equality or game parity')
        print(f'PASS: {cases} Font_Load original-vs-C cases; {counts}; state, ordered calls and ABI equality.')
    except Exception as exc:
        record(phase, 'fail', error=str(exc))
        raise


if __name__ == '__main__':
    verify_font_load()
