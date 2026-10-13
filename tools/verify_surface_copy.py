# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Byte copy with actual access bodies, independent reads/writes and native SDK."""
import copy
import hashlib
import struct
import pefile
from unicorn import UC_MEM_WRITE
import verify_surface_access as access
from verify_surface_lifecycle import ARENA, SIZE, RECORDS, Fault
from verify_matching import STACK
from windows_target import ROOT, BUILD, verify_target, validate_platform_imports
from windows_tracking import record_run
from build_decomp import build
from verify_native_presentation import native_surface_host

if not __debug__:
    raise RuntimeError('Verification requires assertions')
SOURCE = ARENA + 0x3000
assert SOURCE + 1024 <= ARENA + SIZE
SLOT = 0x50eb80
BODY = 'Gfx_SurfaceCopyPixelsNative'
ENTRY = 'Gfx_SurfaceCopyPixelsDispatch'
DLL = BUILD / 'surface_copy_validation.dll'
ROUTINES = {
    BODY: (0x5c1d0, 497, None, 'ab0e1a366b4876c8807819b388b91435c556b4546a92764e400e2eee3ecd3c39'),
    ENTRY: (0x56b70, 55, (55, 0, 9, 0), '8bfc4e41d08ca864cab508cc3db8631f6f1dacef871033cbdfc54d9c23e8996a')}
INPUTS = access.INPUTS + ['tools/verify_surface_copy.py', 'tools/native_copy_probe.c',
                         'tools/verify_native_presentation.py']


def signed(value):
    value &= 0xffffffff
    return value - 0x100000000 if value & 0x80000000 else value


def divide(value, divisor):
    return -(abs(value) // divisor) if value < 0 else value // divisor


class State(access.State):
    def __init__(self, seed):
        super().__init__(seed)
        self.data[SLOT] = bytearray(struct.pack('<I', 0x45c1d0))
        self.branch = None
        self.copied = 0

    def snapshot(self):
        return super().snapshot() + (bytes(self.data[SLOT]),)

    def take(self, address, size):
        try:
            region, offset = self.region(address, size)
        except Fault:
            # Unicorn's x86 unaligned DWORD split faults at the first unmapped
            # page, not the start of the still-mapped portion. Both images were
            # observed independently with the same prefix and fault address.
            if ARENA <= address < ARENA + SIZE < address + size:
                self.fault = ('read', ARENA + SIZE, size)
            raise
        value = int.from_bytes(region[offset:offset + size], 'little')
        self.events.append(('read', address, size, value))
        return value

    def store(self, address, size, value):
        try:
            region, offset = self.region(address, size)
        except Fault:
            self.fault = ('write', address, size)
            raise
        region[offset:offset + size] = value.to_bytes(size, 'little')
        self.events.append(('write', address, size, value))
        self.copied += size

    def transfer(self, source, destination, size):
        self.store(destination, size, self.take(source, size))
        return (source + size) & 0xffffffff, (destination + size) & 0xffffffff

    def apply(self, name, args):
        self.events = []
        self.fault = None
        self.seen = {}
        self.branch = None
        self.copied = 0
        try:
            if name == ENTRY:
                assert self.read(SLOT) == 0x45c1d0
            pixels, stride, sx, sy, width, height, record, dx, dy = args
            if self.read(record) == 0:
                return 0
            mode = self.read(record + 12)
            if mode == 0:
                if not self.lock(record, 3):
                    return 0
            elif mode == 1:
                if not self.unlock(record) or not self.lock(record, 3):
                    return 0
            source = (pixels + stride * sy + sx) & 0xffffffff
            pitch = self.read(record + 24)
            destination = (self.read(record + 20) + pitch * dy + dx) & 0xffffffff
            fast = source % 4 == 0 and stride % 4 == 0 and signed(width) % 160 == 0
            self.branch = 'fast' if fast else 'generic'
            if fast:
                groups = divide(signed(width), 160) & 0xffffffff
                rows = height
                while True:
                    remaining = groups
                    while True:
                        for offset in (0, 32, 64, 96, 128):
                            self.take((source + offset) & 0xffffffff, 4)
                        for _ in range(40):
                            source, destination = self.transfer(source, destination, 4)
                        remaining = (remaining - 1) & 0xffffffff
                        if not remaining:
                            break
                    source = (source + stride - width) & 0xffffffff
                    destination = (destination + pitch - width) & 0xffffffff
                    rows = (rows - 1) & 0xffffffff
                    if not rows:
                        break
            else:
                leading = (-destination) & 3
                tail = (destination + width) & 3
                middle = divide(signed(width - leading - tail), 4) & 0xffffffff
                for _ in range(max(0, signed(height))):
                    source_cursor, destination_cursor = source, destination
                    for size, count in ((1, leading), (4, middle), (1, tail)):
                        for _ in range(count):
                            source_cursor, destination_cursor = self.transfer(source_cursor, destination_cursor, size)
                    source = (source + stride) & 0xffffffff
                    destination = (destination + pitch) & 0xffffffff
            if self.read(record + 12) != mode:
                if not self.unlock(record):
                    return 0
                if mode != 0 and not self.lock(record, mode):
                    return 0
            return 1
        except Fault:
            return None


class Session(access.Session):
    def __init__(self, pe, state, original):
        scratch = copy.copy(state)
        scratch.data = {p: b for p, b in state.data.items() if p != SLOT}
        super().__init__(pe, scratch, original)
        if original:
            self.symbols.update({name: 0x400000 + row[0] for name, row in ROUTINES.items()})
            self.symbols.update(g_surfaceCopyPixels=SLOT, Gfx_InstallSurfaceDispatch=0x45b690)
        self.fields.append((SLOT, self.symbols['g_surfaceCopyPixels'], 4))
        self.events = []
        sp = STACK + 0x6000
        self.uc.mem_write(sp, struct.pack('<I', access.STOP))
        self.uc.reg_write(access.UC_X86_REG_ESP, sp)
        self.uc.reg_write(access.UC_X86_REG_EFLAGS, 2)
        self.uc.emu_start(self.symbols['Gfx_InstallSurfaceDispatch'], access.STOP, count=1000)
        assert self.uc.reg_read(access.UC_X86_REG_EAX) == 1
        assert self.uc.reg_read(access.UC_X86_REG_ESP) == sp + 4
        assert struct.unpack('<I', self.uc.mem_read(self.physical(SLOT), 4))[0] == self.symbols[BODY]

    def snapshot(self):
        target = struct.unpack('<I', self.uc.mem_read(self.physical(SLOT), 4))[0]
        if target == self.symbols[BODY]:
            target = 0x45c1d0
        return super().snapshot() + (struct.pack('<I', target),)

    def memory(self, uc, kind, address, size, value, data):
        previous = len(self.events)
        super().memory(uc, kind, address, size, value, data)
        if len(self.events) > previous and self.normalize(address) == SLOT:
            event = self.events[-1]
            if event[3] == self.symbols[BODY]:
                self.events[-1] = event[:3] + (0x45c1d0,)
        # Include the negative-pitch window below PIXELS, excluding SDK objects.
        if len(self.events) == previous and ARENA + 0x2600 <= address and address + size <= ARENA + SIZE:
            if kind != UC_MEM_WRITE:
                value = int.from_bytes(uc.mem_read(address, size), 'little')
            self.events.append(('write' if kind == UC_MEM_WRITE else 'read', address, size, value))

    def run(self, name, args, state, result, seed, policy):
        relocated = args[:6] + (self.physical(args[6]),) + args[7:]
        super().run(name, relocated, state, result, seed, policy)


def verify_surface_copy(record=True):
    original = pefile.PE(data=verify_target())
    build(dll=DLL)
    compiled = pefile.PE(str(DLL))
    validate_platform_imports(compiled)
    debug = next(d.struct for d in original.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {r: (n, local, args, bits) for r, n, local, args, bits in struct.iter_unpack(
        '<IIIHH', original.__data__[debug.PointerToRawData:debug.PointerToRawData + debug.SizeOfData])}
    for rva, size, row, digest in ROUTINES.values():
        assert (fpo.get(rva) == row) and hashlib.sha256(original.get_data(rva, size)).hexdigest() == digest
    assert original.get_data(0x5c3c0, 16) == b'\xc3' + b'\xcc' * 15
    coverage = {name: {'comparisons': 0, 'followups': 0, 'faults': 0, 'fast': 0,
                       'generic': 0, 'normal_copies': 0, 'sdk_failures': 0,
                       'bounded_invalid': 0, 'aliases': 0, 'mutations': 0} for name in ROUTINES}

    def sequence(state, name, args, repeats=1, normal=False, invalid=False):
        sessions = [Session(pe, state, index == 0) for index, pe in enumerate((original, compiled))]
        for step in range(repeats):
            policy = copy.deepcopy(state)
            policy.seen = {}
            result = state.apply(name, args)
            if normal:
                assert state.fault is None and result == 1 and state.copied == args[4] * args[5]
            if invalid:
                assert state.fault is not None and state.copied > 0
            total = sum(row['comparisons'] for row in coverage.values())
            for session in sessions:
                session.run(name, args, state, result, total, copy.deepcopy(policy))
            row = coverage[name]
            row['comparisons'] += 1
            row['followups'] += int(step > 0)
            row['faults'] += int(state.fault is not None)
            if state.branch:
                row[state.branch] += 1
            row['normal_copies'] += int(normal)
            row['sdk_failures'] += int(result == 0 and bool(state.seen))
            row['bounded_invalid'] += int(invalid)
            row['aliases'] += int(ARENA + 0x2600 <= args[0] < SOURCE)
            row['mutations'] += int(any(changes and occurrence < state.seen.get(api, 0)
                for (api, occurrence), (_, _, _, changes) in state.script.items()))
            if state.fault:
                break

    destination = RECORDS[0]
    for name in ROUTINES:
        for mode in (0, 1, 2, 3):
            for width in (8, 17, 32, 160, 320):
                for offset in range(4):
                    for height in (1, 2):
                        state = State(width)
                        state.put(destination + 12, mode)
                        state.put(destination + 8, int(mode != 0))
                        state.put(destination + 24, 352)
                        state.script['Lock', 0] = (0, access.PIXELS, 352, [])
                        sequence(state, name, (SOURCE + offset, 352, 0, 1, width, height, destination, offset, 1), 2, normal=True)
        for source_offset in range(4):
            for target_offset in range(4):
                for pitch in (15, 17, 352, 0xfffffff0):
                    state = State(17)
                    state.put(destination + 12, 2)
                    state.put(destination + 8, 1)
                    state.put(destination + 24, pitch)
                    sequence(state, name, (SOURCE + source_offset, 352, 0, 1, 17, 2, destination, target_offset, 1), normal=True)
        for mode in (0, 1, 2, 3):
            for api in access.FUNCTIONS:
                for reply in (1, 0x80004005, access.LOST):
                    state = State(9)
                    state.put(destination + 12, mode)
                    state.put(destination + 8, int(mode != 0))
                    state.script[api, 0] = (reply, access.PIXELS, 16, [])
                    if api == 'Restore':
                        state.script['IsLost', 0] = (access.LOST, access.PIXELS, 16, [])
                    sequence(state, name, (SOURCE, 352, 0, 0, 32, 2, destination, 0, 0), 2)
        for width, height, dx in ((0, 1, 0), (160, 0, 0), (0, 0, 0), (0xffffff60, 1, 0), (1, 1, 2)):
            state = State(3)
            state.put(destination + 12, 2)
            sequence(state, name, (ARENA + 0x3ee0, 160, 0, 0, width, height, destination, dx, 0), invalid=True)
        for active in (0, 1, 2, 0xffffffff):
            state = State(4)
            state.put(destination, active)
            sequence(state, name, (SOURCE + 1, 352, 0, 0, 32, 0, destination, 0, 0))
        for invalid_record in (0, 0xdead0000):
            sequence(State(5), name, (SOURCE, 352, 0, 0, 32, 2, invalid_record, 0, 0))
        for source_pointer in (0, 0xdead0000):
            sequence(State(6), name, (source_pointer, 352, 0, 0, 32, 2, destination, 0, 0))
        for source_offset, destination_offset in ((0, 4), (4, 0), (0, 0), (1, 2)):
            state = State(12)
            state.put(destination + 12, 2)
            state.put(destination + 8, 1)
            sequence(state, name, (access.PIXELS + source_offset, 160, 0, 0,
                32, 1, destination, destination_offset, 0), 2, normal=True)
        for api in access.FUNCTIONS:
            state = State(14)
            state.put(destination + 12, 1 if api == 'Unlock' else 0)
            state.put(destination + 8, int(api == 'Unlock'))
            if api == 'Restore':
                state.script['IsLost', 0] = (access.LOST, access.PIXELS, 16, [])
            state.script[api, 0] = (0, access.PIXELS + 16, 17,
                [(destination + 4, 7), (destination + 12, 4), (destination + 24, 99)])
            sequence(state, name, (SOURCE + 1, 352, 0, 0, 32, 2, destination, 1, 1), 2)
        state = State(15)
        state.script['Lock', 0] = (0, access.PIXELS, 16, [(destination, 0)])
        sequence(state, name, (SOURCE, 352, 0, 0, 32, 2, destination, 0, 0), 2)
        for target in RECORDS:
            for mode in (0, 1, 2, 3, 4, 0xffffffff):
                state = State(target)
                state.put(target + 12, mode)
                state.put(target + 8, int(mode != 0))
                sequence(state, name, (SOURCE, 64, 0, 0, 32, 2, target, 0, 0), normal=True)
    if record:
        for name, (rva, *_) in ROUTINES.items():
            for kind in ('compilation', 'emulation'):
                record_run(rva, kind, 'pass', inputs=INPUTS, artifact=DLL.relative_to(ROOT).as_posix(),
                    cases=coverage[name]['comparisons'] if kind == 'emulation' else 0,
                    command='uv run tools/verify_surface_copy.py', details={'coverage': coverage[name],
                    'scope': 'Real C89 byte/word copy and actual Lock/Unlock dependencies; independent original raw-offset oracle, SDK responses modeled; prefetch/order, mode restoration, ABI and invalid-size fault prefixes',
                    'limitations': 'Arbitrary caller-stack/record-pointer aliases, DF-set non-ABI callers, unmasked native fault atomicity/concurrency/full game and original toolchain unverified'})
        native = native_surface_host(DLL, 'tools/native_copy_probe.c', 'copy_native',
            '            installed->lpVtbl->Release(installed);',
            '            probe_native_copy(image,compiled,side,*draws[side]);\n',
            ('PASS original CopyPixels', 'PASS rebuilt CopyPixels'))
        details = native | {'copies': 54, 'copied_bytes': 11328, 'readback_bytes': 884736,
            'inactive_guards': 12, 'empty_height_calls': 12,
            'scope': 'Actual paired original/C89 byte-copy body/installed entry; owned64x64 32bit SDK surfaces, fast/generic across all four modes with real Lock/Unlock and exact output/outside bytes; test-owned seeds/disposal',
            'limitations': 'Positive bounded windowed offscreen copies only; native invalid sizes/faults/lost surfaces/primary rendering/full game/original compiler unresolved'}
        for name, (rva, *_) in ROUTINES.items():
            record_run(rva, 'native', 'pass', inputs=INPUTS, artifact=DLL.relative_to(ROOT).as_posix(),
                cases=3, command='uv run tools/verify_surface_copy.py', details=details | {'routine': name})
        print('PASS native byte copy:', native['stdout'], flush=True)
    print('PASS surface byte copy:', coverage, flush=True)
    return coverage


if __name__ == '__main__':
    import sys
    verify_surface_copy(record='--no-record' not in sys.argv)
