# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Constant surface bodies and actual forwarding, with no invented status meaning."""
import hashlib
import itertools
import random
import struct
import pefile
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_EIP, UC_X86_REG_ESP, UC_X86_REG_EFLAGS
from verify_matching import STACK, STOP, PRESERVED
from windows_target import ROOT, BUILD, verify_target, validation_symbols, validate_platform_imports
from windows_tracking import record_run
from build_decomp import build
from verify_surface_access import INPUTS as ACCESS_INPUTS
from verify_native_presentation import native_surface_host

if not __debug__:
    raise RuntimeError('Verification requires assertions')
CONFIG = 'Gfx_SurfaceConfigureSurfaceNative'
RESERVED = 'Gfx_SurfaceReservedNative'
ENTRY = 'Gfx_SurfaceConfigureSurfaceDispatch'
HASH = '7140f35dee6220b79b12aecc27acf5105bf3b77d1588e89fce345de7c16c72b7'
ROUTINES = {
    CONFIG: (0x5c150, 6, (6, 0, 5, 0), HASH, 5),
    RESERVED: (0x5c520, 6, (6, 0, 0, 0), HASH, 0),
    ENTRY: (0x56c60, 35, (35, 0, 5, 0), 'a6101b85671afd849e7b08e410e5db71224d69dfdaddd303e64e7e1f572ea8e3', 5)}
DLL = BUILD / 'surface_constants_validation.dll'
INPUTS = ACCESS_INPUTS + ['tools/verify_surface_constants.py',
    'tools/native_surface_constants.c', 'tools/verify_native_presentation.py']


class Session:
    def __init__(self, pe, original):
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.size = (pe.OPTIONAL_HEADER.SizeOfImage + 4095) & ~4095
        self.uc = Uc(UC_ARCH_X86, UC_MODE_32)
        self.uc.mem_map(self.base, self.size)
        self.uc.mem_write(self.base, pe.get_memory_mapped_image())
        self.uc.mem_map(STACK, 0x10000)
        self.uc.mem_map(STOP, 0x1000)
        self.symbols = ({name: 0x400000 + row[0] for name, row in ROUTINES.items()} |
            {'Gfx_InstallSurfaceDispatch': 0x45b690, 'g_surfaceConfigureSurface': 0x50eb78,
             'g_surfaceReserved': 0x50eb8c}) if original else validation_symbols(pe)
        sp = STACK + 0x6000
        self.uc.mem_write(sp, struct.pack('<I', STOP))
        self.uc.reg_write(UC_X86_REG_ESP, sp)
        self.uc.reg_write(UC_X86_REG_EFLAGS, 2)
        self.uc.emu_start(self.symbols['Gfx_InstallSurfaceDispatch'], STOP, count=1000)
        assert self.uc.reg_read(UC_X86_REG_EAX) == 1
        assert self.uc.reg_read(UC_X86_REG_ESP) == sp + 4
        for slot, name in (('g_surfaceConfigureSurface', CONFIG), ('g_surfaceReserved', RESERVED)):
            assert struct.unpack('<I', self.uc.mem_read(self.symbols[slot], 4))[0] == self.symbols[name]
        self.uc.hook_add(UC_HOOK_CODE, self.code)

    def code(self, uc, address, size, data):
        if address == self.symbols[self.target]:
            sp = uc.reg_read(UC_X86_REG_ESP)
            self.calls.append(tuple(struct.unpack('<' + 'I' * self.argc,
                uc.mem_read(sp + 4, 4 * self.argc))) if self.argc else ())

    def run(self, name, args, seed):
        self.target = CONFIG if name == ENTRY else name
        self.argc = len(args)
        self.calls = []
        sp = STACK + 0x8000
        self.uc.mem_write(sp, struct.pack('<' + 'I' * (len(args) + 1), STOP, *args))
        caller = bytes(self.uc.mem_read(sp, 0x8000))
        before = bytes(self.uc.mem_read(self.base, self.size))
        saved = [random.Random(seed + index).getrandbits(32) for index in range(4)]
        for reg, value in zip(PRESERVED, saved):
            self.uc.reg_write(reg, value)
        self.uc.reg_write(UC_X86_REG_ESP, sp)
        self.uc.reg_write(UC_X86_REG_EFLAGS, 2)
        self.uc.emu_start(self.symbols[name], STOP, count=1000)
        assert self.calls == [args], (name, args, self.calls)
        assert self.uc.reg_read(UC_X86_REG_EAX) == 2
        assert self.uc.reg_read(UC_X86_REG_EIP) == STOP and self.uc.reg_read(UC_X86_REG_ESP) == sp + 4
        assert [self.uc.reg_read(reg) for reg in PRESERVED] == saved
        assert self.uc.reg_read(UC_X86_REG_EFLAGS) & 0x400 == 0
        assert bytes(self.uc.mem_read(sp, 0x8000)) == caller
        assert bytes(self.uc.mem_read(self.base, self.size)) == before


def verify_surface_constants(record=True):
    original = pefile.PE(data=verify_target())
    build(dll=DLL)
    compiled = pefile.PE(str(DLL))
    validate_platform_imports(compiled)
    debug = next(d.struct for d in original.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {r: (n, local, args, bits) for r, n, local, args, bits in struct.iter_unpack(
        '<IIIHH', original.__data__[debug.PointerToRawData:debug.PointerToRawData + debug.SizeOfData])}
    sessions = (Session(original, True), Session(compiled, False))
    counts = {}
    for name, (rva, size, row, digest, argc) in ROUTINES.items():
        assert fpo[rva] == row and hashlib.sha256(original.get_data(rva, size)).hexdigest() == digest
        count = 0
        for args in itertools.product((0, 1, 0x80000000, 0xffffffff), repeat=argc):
            for session in sessions:
                session.run(name, args, count)
            count += 1
        counts[name] = count
    if record:
        for name, (rva, *_) in ROUTINES.items():
            for kind in ('compilation', 'emulation'):
                record_run(rva, kind, 'pass', inputs=INPUTS, artifact=DLL.relative_to(ROOT).as_posix(),
                    cases=counts[name] if kind == 'emulation' else 0,
                    command='uv run tools/verify_surface_constants.py', details={'cases': counts[name],
                    'scope': 'Actual constant2 bodies and live installed forwarding entry; target entry hook observes arguments without replacing execution; no image changes, caller stack/return and preserved registers',
                    'limitations': 'Status meaning and original source/toolchain unknown; no native full game certification'})
        native = native_surface_host(DLL, 'tools/native_surface_constants.c', 'surface_constants_native',
            '            installed->lpVtbl->Release(installed);',
            '            probe_native_surface_constants(image,compiled,side);\n',
            ('PASS original constant2', 'PASS rebuilt constant2'))
        for name, (rva, *_) in ROUTINES.items():
            record_run(rva, 'native', 'pass', inputs=INPUTS, artifact=DLL.relative_to(ROOT).as_posix(),
                cases=3, command='uv run tools/verify_surface_constants.py', details=native |
                {'calls': 72, 'routine': name, 'scope': 'Actual paired body/installer/entry constant2 returns with varied argument bits in three real windowed lifecycle scenarios; original WndProc reference',
                 'limitations': 'Constant status meaning unknown; no rebuilt startup/menu/render/input/audio/races or original toolchain certification'})
        print('PASS native surface constants:', native['stdout'], flush=True)
    print('PASS surface constants:', counts, flush=True)
    return counts


if __name__ == '__main__':
    import sys
    verify_surface_constants(record='--no-record' not in sys.argv)
