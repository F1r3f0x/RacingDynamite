# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Complete authentic surface dispatch routines versus direct C89 surface routines."""
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
from windows_target import ROOT, BUILD, verify_target, validation_symbols
from windows_tracking import record_run
from build_decomp import build

if not __debug__:
    raise RuntimeError('Verification requires assertions')

DLL = BUILD / 'surface_dispatch_validation.dll'
STACK = 0x7000000
STOP  = 0x7100000
ARENA = 0x7200000

PRESERVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)

ROUTINES = {
    'Gfx_SurfaceConfigureDispatch': (0x56b10, 30, 4, '0aefb3b8a35847643dbeb6d4c15ca363c8fb17a65cf8c0bf1a1e9df12262f907'),
    'Gfx_SurfaceLockDispatch': (0x56c10, 20, 2, '46a5ae6e26d29bf872659b827ff00209cb3cae73e67617b72aa1e49bf4a528d3'),
    'Gfx_SurfaceUnlockDispatch': (0x56c30, 15, 1, '23960952e78fdc4df291be1b843aa2b238ceee65bdc1baa79770bb2e88857aa5'),
    'Gfx_SurfaceRestoreDispatch': (0x56c50, 6, 0, '9bac4d3e6b644d4ea3199af28c48924cd6366565dfb7bf1f29fe153d81102e31'),
}

DISPATCH_PTRS = {
    'Gfx_SurfaceConfigureDispatch': 'g_surfaceConfigure',
    'Gfx_SurfaceLockDispatch': 'g_surfaceLock',
    'Gfx_SurfaceUnlockDispatch': 'g_surfaceUnlock',
    'Gfx_SurfaceRestoreDispatch': 'g_surfaceRestore',
}

ORIGINAL_PTR_VAS = {
    'g_surfaceConfigure': 0x50eb68,
    'g_surfaceLock': 0x50eb90,
    'g_surfaceUnlock': 0x50eb94,
    'g_surfaceRestore': 0x50eb9c,
}

EXPECTED_FPO = {
    rva: (sz, 0, argc, 0) for rva, sz, argc, _ in ROUTINES.values()
}

INPUTS = [
    'decomp/src/surface.c',
    'decomp/include/surface.h',
    'decomp/modules.json',
    'tools/build_decomp.py',
    'tools/verify_matching.py',
    'tools/verify_surface_dispatch.py',
    'tools/windows_target.py',
    'tools/windows_tracking.py'
]


def inspect_original(pe):
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {r: (n, l, a, b) for r, n, l, a, b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData + debug.SizeOfData])}
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    for name, (rva, size, argc, digest) in ROUTINES.items():
        assert fpo[rva] == EXPECTED_FPO[rva], f"FPO mismatch for {name}: {fpo[rva]} vs {EXPECTED_FPO[rva]}"
        raw = pe.get_data(rva, size)
        assert hashlib.sha256(raw).hexdigest() == digest, f"Digest mismatch for {name}"
        ins = list(md.disasm(raw, 0x400000 + rva))
        assert sum(i.size for i in ins) == size, f"Size mismatch for {name}"
        assert ins[-1].mnemonic in ('ret', 'jmp'), f"Expected ret or jmp for {name}"

    # Verify padding bytes
    padding_checks = [
        (0x56b10 + 30, 2),
        (0x56c10 + 20, 12),
        (0x56c30 + 15, 1),
        (0x56c50 + 6, 10),
    ]
    for addr, pad_len in padding_checks:
        assert pe.get_data(addr, pad_len) == b'\xcc' * pad_len, f"Padding mismatch at 0x{addr:x}"

    # Verify relocations across the 4 routines (each has exactly 1 relocation for the pointer)
    for name, (rva, size, _, _) in ROUTINES.items():
        relocs = [e.rva for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries
                  if e.type == 3 and rva <= e.rva < rva + size]
        assert len(relocs) == 1, f"Expected 1 relocation for {name}, got {relocs}"
        ptr_name = DISPATCH_PTRS[name]
        target_va = ORIGINAL_PTR_VAS[ptr_name]
        op_val = struct.unpack('<I', pe.get_data(relocs[0], 4))[0]
        assert op_val == target_va, f"Target VA mismatch for {name}: 0x{op_val:x} vs 0x{target_va:x}"


class DispatchSession:
    def __init__(self, pe, original):
        self.original = original
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.size = (pe.OPTIONAL_HEADER.SizeOfImage + 0xFFF) & ~0xFFF
        self.uc = Uc(UC_ARCH_X86, UC_MODE_32)
        self.uc.mem_map(self.base, self.size)
        self.uc.mem_write(self.base, pe.get_memory_mapped_image())
        self.uc.mem_map(STACK, 0x10000)
        self.uc.mem_map(STOP, 0x1000)
        self.uc.mem_map(ARENA, 0x20000)

        syms = validation_symbols(pe) if not original else {}
        self.symbols = {}
        for name, (rva, *_) in ROUTINES.items():
            self.symbols[name] = (0x400000 + rva) if original else syms[name]

        self.ptr_addresses = {}
        for ptr_name, va in ORIGINAL_PTR_VAS.items():
            self.ptr_addresses[ptr_name] = va if original else syms[ptr_name]

        self.events = []
        self.reply = 0
        self.target_argc = 0
        self.target_hook_addr = ARENA + 0x1000
        self.uc.hook_add(UC_HOOK_CODE, self._hook_code)

    def _hook_code(self, uc, address, size, data):
        if address == self.target_hook_addr:
            sp = uc.reg_read(UC_X86_REG_ESP)
            ret_addr = struct.unpack('<I', uc.mem_read(sp, 4))[0]
            args = struct.unpack('<' + 'I' * self.target_argc, uc.mem_read(sp + 4, 4 * self.target_argc))
            self.events.append(('target_called', args))
            uc.reg_write(UC_X86_REG_ESP, sp + 4)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)
            uc.reg_write(UC_X86_REG_EAX, self.reply)
            uc.reg_write(UC_X86_REG_ECX, 0xbadc0ffe)
            uc.reg_write(UC_X86_REG_EDX, 0x87654321)

    def invoke(self, name, args, reply=0):
        self.events = []
        self.reply = reply & 0xFFFFFFFF
        _, _, self.target_argc, _ = ROUTINES[name]

        # Setup the global function pointer to point to our hook target
        ptr_var = DISPATCH_PTRS[name]
        var_addr = self.ptr_addresses[ptr_var]
        self.uc.mem_write(var_addr, struct.pack('<I', self.target_hook_addr))

        sp = STACK + 0x8000
        words = [STOP] + [a & 0xFFFFFFFF for a in args]
        self.uc.mem_write(sp, struct.pack('<' + 'I' * len(words), *words))
        caller = bytes(self.uc.mem_read(sp, 0x8000))
        saved = [0x12345678 + i for i in range(4)]
        for reg, val in zip(PRESERVED, saved):
            self.uc.reg_write(reg, val)
        self.uc.reg_write(UC_X86_REG_ESP, sp)
        self.uc.reg_write(UC_X86_REG_EFLAGS, 2)

        self.uc.emu_start(self.symbols[name], STOP, count=100000)

        assert self.uc.reg_read(UC_X86_REG_EIP) == STOP
        # In cdecl, caller cleans arguments, so callee ret pops only STOP
        assert self.uc.reg_read(UC_X86_REG_ESP) == sp + 4
        assert [self.uc.reg_read(r) for r in PRESERVED] == saved
        assert (self.uc.reg_read(UC_X86_REG_EFLAGS) & 0x400) == 0
        assert bytes(self.uc.mem_read(sp, 0x8000)) == caller

        ret_val = self.uc.reg_read(UC_X86_REG_EAX)
        return ret_val, list(self.events)


def verify_surface_dispatch(record=False):
    original_pe = pefile.PE(data=verify_target())
    inspect_original(original_pe)

    build(dll=DLL)
    compiled_pe = pefile.PE(str(DLL))

    coverage = {name: {'comparisons': 0, 'cases': 0} for name in ROUTINES}

    for seed in range(32):
        rng = random.Random(0x56b10 + seed)
        s_orig = DispatchSession(original_pe, True)
        s_comp = DispatchSession(compiled_pe, False)

        for name, (_, _, argc, _) in ROUTINES.items():
            args = [rng.getrandbits(32) for _ in range(argc)]
            reply = rng.getrandbits(32)

            r_orig = s_orig.invoke(name, args, reply)
            r_comp = s_comp.invoke(name, args, reply)

            assert r_orig == r_comp, f"Dispatch mismatch for {name}: {r_orig} vs {r_comp}"
            assert r_orig[0] == (reply & 0xFFFFFFFF), f"Return mismatch for {name}"
            assert r_orig[1] == [('target_called', tuple(args))], f"Args mismatch for {name}"

            coverage[name]['comparisons'] += 1
            coverage[name]['cases'] += 1

    if record:
        for name, (rva, *_) in ROUTINES.items():
            for kind in ('compilation', 'emulation'):
                record_run(
                    rva, kind, 'pass',
                    inputs=INPUTS,
                    artifact=DLL.relative_to(ROOT).as_posix(),
                    cases=coverage[name]['cases'] if kind == 'emulation' else 0,
                    command='uv run tools/verify_surface_dispatch.py',
                    details={
                        'coverage': coverage[name],
                        'scope': 'Authentic surface dispatch entry wrappers verified differentially against IGN_WIN.EXE with live target hooks and cdecl argument passing',
                        'limitations': 'Validation DLL focused harness; no full rasterizer or complete playable game parity'
                    }
                )

    print('PASS surface dispatch verification:', coverage, flush=True)
    return coverage


if __name__ == '__main__':
    verify_surface_dispatch(record=True)
