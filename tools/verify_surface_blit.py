# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""BltFast body/forwarder: raw-offset oracle and paired actual SDK pixels."""
import copy
import hashlib
import struct
import pefile
import verify_surface_access as access
from verify_surface_lifecycle import ARENA, SIZE, RECORDS, VTABLE, Fault
from verify_matching import STACK
from windows_target import ROOT, BUILD, verify_target, validate_platform_imports
from build_decomp import build
from windows_tracking import record_run
from verify_native_presentation import native_surface_host

if not __debug__:
    raise RuntimeError('Verification requires assertions')

SLOT = 0x50eb7c
METHOD = ARENA + 0x2500
assert METHOD + 4 <= ARENA + SIZE
BODY = 'Gfx_SurfaceBlitNative'
ENTRY = 'Gfx_SurfaceBlitDispatch'
DLL = BUILD / 'surface_blit_validation.dll'
ROUTINES = {
    BODY: (0x5c160, 108, (108, 4, 8, 524), 'e716f58cbe637634909a5de39555b22a4f99b391b14673f17879ecf036ccdde2'),
    ENTRY: (0x56b30, 50, (50, 0, 8, 0), '299c846485b18251d0a57631f64aba15d902a277fc877bf5465eb08e5efa2689')}
INPUTS = access.INPUTS + ['tools/verify_surface_blit.py', 'tools/native_blit_probe.c',
                         'tools/verify_native_presentation.py']


class State(access.State):
    def __init__(self, seed):
        super().__init__(seed)
        self.put(VTABLE + 0x1c, METHOD)
        self.reply = 0
        self.changes = []
        self.data[SLOT] = bytearray(struct.pack('<I', 0x45c160))

    def snapshot(self):
        return super().snapshot() + (bytes(self.data[SLOT]),)

    def apply(self, name, args):
        self.events = []
        self.fault = None
        try:
            if name == ENTRY:
                assert self.read(SLOT) == 0x45c160
            source, x, y, width, height, destination, dx, dy = args
            if self.read(source) == 0 or self.read(destination) == 0:
                return 0
            target = self.read(destination + 44)
            input_surface = self.read(source + 44)
            table = self.read(target, False)
            assert self.read(table + 0x1c, False) == METHOD
            rectangle = (x, y, (x + width) & 0xffffffff, (y + height) & 0xffffffff)
            self.events.append(('api', 'BltFast',
                (target, dx, dy, input_surface, rectangle, 0x10), self.snapshot()))
            for address, value in self.changes:
                self.put(address, value)
            return int(self.reply == 0)
        except Fault:
            return None


class Session(access.Session):
    def __init__(self, pe, state, original):
        scratch = copy.copy(state)
        scratch.data = {p: b for p, b in state.data.items() if p != SLOT}
        super().__init__(pe, scratch, original)
        if original:
            self.symbols.update({name: 0x400000 + row[0] for name, row in ROUTINES.items()})
            self.symbols.update(g_surfaceBlit=SLOT, Gfx_InstallSurfaceDispatch=0x45b690)
        self.fields.append((SLOT, self.symbols['g_surfaceBlit'], 4))
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
            target = 0x45c160
        return super().snapshot() + (struct.pack('<I', target),)

    def memory(self, uc, kind, address, count, value, data):
        previous = len(self.events)
        super().memory(uc, kind, address, count, value, data)
        if len(self.events) > previous and self.normalize(address) == SLOT:
            event = self.events[-1]
            if event[3] == self.symbols[BODY]:
                self.events[-1] = event[:3] + (0x45c160,)

    def code(self, uc, address, count, data):
        if address != METHOD:
            return
        sp = uc.reg_read(access.UC_X86_REG_ESP)
        ret, *raw = struct.unpack('<7I', uc.mem_read(sp, 28))
        assert STACK <= raw[4] and raw[4] + 16 < STACK + 0x10000
        rectangle = struct.unpack('<4I', uc.mem_read(raw[4], 16))
        args = (raw[0], raw[1], raw[2], raw[3], rectangle, raw[5])
        self.events.append(('api', 'BltFast', args, self.snapshot()))
        for p, value in self.policy.changes:
            uc.mem_write(self.physical(p), struct.pack('<I', value))
        for reg, value in ((access.UC_X86_REG_EAX, self.policy.reply),
                (access.UC_X86_REG_ECX, 0xdeadbeef), (access.UC_X86_REG_EDX, 0x12345678),
                (access.UC_X86_REG_ESP, sp + 28), (access.UC_X86_REG_EIP, ret),
                (access.UC_X86_REG_EFLAGS, 0x8d7)):
            uc.reg_write(reg, value)

    def run(self, name, args, state, result, seed, policy):
        # Both record pointers relocate; scalar coordinates preserve their bits.
        relocated = args[:5] + (self.physical(args[5]),) + args[6:]
        super().run(name, relocated, state, result, seed, policy)


def verify_surface_blit(record=True, original_only=False):
    original = pefile.PE(data=verify_target())
    if not original_only:
        build(dll=DLL)
        compiled = pefile.PE(str(DLL))
        validate_platform_imports(compiled)
    debug = next(d.struct for d in original.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {r: (n, local, args, bits) for r, n, local, args, bits in struct.iter_unpack(
        '<IIIHH', original.__data__[debug.PointerToRawData:debug.PointerToRawData + debug.SizeOfData])}
    for rva, size, row, digest in ROUTINES.values():
        assert fpo[rva] == row and hashlib.sha256(original.get_data(rva, size)).hexdigest() == digest
    coverage = {name: {'comparisons': 0, 'followups': 0, 'faults': 0, 'api_calls': 0,
                       'mutations': 0, 'aliases': 0} for name in ROUTINES}

    def sequence(state, name, args, repeats=1):
        sessions = [Session(pe, state, index == 0) for index, pe in enumerate(
            (original,) if original_only else (original, compiled))]
        for step in range(repeats):
            policy = copy.deepcopy(state)
            result = state.apply(name, args)
            count = sum(row['comparisons'] for row in coverage.values())
            calls = sum(event[0] == 'api' for event in state.events)
            if calls:
                assert calls == 1 and state.fault is None and result == int(state.reply == 0)
            for session in sessions:
                session.run(name, args, state, result, count, copy.deepcopy(policy))
            row = coverage[name]
            row['comparisons'] += 1
            row['followups'] += int(step > 0)
            row['faults'] += int(state.fault is not None)
            row['api_calls'] += calls
            row['mutations'] += int(bool(calls and state.changes))
            row['aliases'] += int(args[0] == args[5])
            if state.fault:
                break

    for name in ROUTINES:
        for source in RECORDS:
            for destination in (RECORDS[0], RECORDS[1], source):
                for active in (0, 1, 2, 0xffffffff):
                    for target_active in (0, 1, 2, 0xffffffff):
                        for reply in (0, 1, 0x80004005, 0x8876021c):
                            state = State(source)
                            state.put(source, active)
                            state.put(destination, target_active)
                            state.reply = reply
                            sequence(state, name, (source, 0x7fffffff, 0xffffffff,
                                0x80000002, 17, destination, 13, 7), 2)
        source, destination = RECORDS[:2]
        for rectangle in ((3, 5, 6, 4), (0, 0, 0, 0), (0xffffffff, 0x80000000, 0xffffffff, 0x7fffffff)):
            for reply in (0, 1, 0x80004005):
                state = State(23)
                state.reply = reply
                sequence(state, name, (source,) + rectangle + (destination, 13, 7))
        for bad_source, bad_destination in ((0, destination), (0xdead0000, destination),
                                           (source, 0), (source, 0xdead0000)):
            sequence(State(7), name, (bad_source, 3, 5, 6, 4, bad_destination, 13, 7))
        for pointer in (0, 0xdead0000):
            state = State(9)
            state.put(destination + 44, pointer)
            sequence(state, name, (source, 3, 5, 6, 4, destination, 13, 7))
        state = State(8)
        state.changes = [(source, 0), (destination + 44, ARENA + 48)]
        sequence(state, name, (source, 3, 5, 6, 4, destination, 13, 7), 2)
    assert all(row['api_calls'] > 0 and row['mutations'] == 1 for row in coverage.values())
    if record:
        assert not original_only
        for name, (rva, *_) in ROUTINES.items():
            for kind in ('compilation', 'emulation'):
                record_run(rva, kind, 'pass', inputs=INPUTS,
                    artifact=DLL.relative_to(ROOT).as_posix(),
                    cases=coverage[name]['comparisons'] if kind == 'emulation' else 0,
                    command='uv run tools/verify_surface_blit.py', details={'coverage': coverage[name],
                    'scope': 'Actual C89 SDK BltFast body/real installed forwarding entry; independent raw-offset oracle, HRESULT and SDK mutations modeled; exact record read order, source RECT, flags and ABI',
                    'limitations': 'Arbitrary caller-stack aliases/dispatch replacement/reentry/native faults, production frame integration/game and original compiler unresolved'})
        native = native_surface_host(DLL, 'tools/native_blit_probe.c', 'blit_native',
            '            installed->lpVtbl->Release(installed);',
            '            probe_native_blit(image,compiled,side,*draws[side]);\n',
            ('PASS original SDK BltFast', 'PASS rebuilt SDK BltFast'))
        details = native | {'copies': 24, 'destination_pixels_checked': 98304, 'inactive_guards': 96,
            'scope': 'Actual paired original/C89 BltFast and real installer/forwarding entry: owned64x64 SDK surfaces, actual format/pitch/masks, source3/5/6/4 to destination13/7, all destination and outside bytes checked; test-owned seed/interface disposal',
            'limitations': 'Windowed offscreen success only; full production frame/primary clipping, lost surfaces/native faults/reentry, startup/menu/input/audio/races and original toolchain unverified'}
        for name, (rva, *_) in ROUTINES.items():
            record_run(rva, 'native', 'pass', inputs=INPUTS,
                artifact=DLL.relative_to(ROOT).as_posix(), cases=3,
                command='uv run tools/verify_surface_blit.py', details=details | {'routine': name})
        print('PASS native blit:', native['stdout'], flush=True)
    print('PASS surface blit:', coverage, flush=True)
    return coverage


if __name__ == '__main__':
    import sys
    verify_surface_blit(record='--no-record' not in sys.argv)
