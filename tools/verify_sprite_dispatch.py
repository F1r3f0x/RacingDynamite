# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Complete authentic sprite dispatch routines versus direct C89 geputget routines."""
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

DLL = BUILD / 'sprite_dispatch_validation.dll'
STACK = 0x7000000
STOP  = 0x7100000
ARENA = 0x7200000

PRESERVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)

ROUTINES = {
    'Gfx_CloseDispatch': (0x56ca0, 6, 0, 'f54576d17cf81a5b459c28177c5d638406200de58ced731d331e852d85c3b108'),
    'Gfx_OptionDispatch': (0x56cb0, 15, 1, 'd5f47caf1c6baad1796a505d1ba148815dc490a6e2214fc8a00f3cc3233a4c64'),
    'Gfx_ConfigureDispatch': (0x56cc0, 35, 5, '55920e775768dfba2721249cf60a49b924696a84bfe47a8ad80af5f2da278154'),
    'Gfx_SetClipDispatch': (0x56cf0, 30, 4, '9f7d76bab0660dc8db60d9fea1ab576779f890831a2109d76b88fd01dd461208'),
    'Gfx_DrawListDispatch': (0x56d10, 15, 1, '12a4a53eb2752ffee638c4a6ad965e4e19bf62d9f28382d673ed854136bc9e0b'),
    'Gfx_DrawDispatch': (0x56d20, 25, 3, 'e04def2288d23041976ef0a043cea2b296558f74cf82cce58476bf765e804623'),
    'Gfx_HandleOpDispatch': (0x56d40, 20, 2, 'f76b5ff746f37d91b851c1a25161196baa69ffe94b4bfe1250a607da04aa97bc'),
    'Gfx_UninstalledDispatch_56D60': (0x56d60, 20, 2, 'a4229314acee10caef57e12ca7628576757d95c5c13577d49df6b50ad6d007fe'),
    'Gfx_UninstalledDispatch_56D80': (0x56d80, 15, 1, '5c64c5dbea01345484c79b6525b9279d4743a41e19e10ad7caf3c327dac204d2'),
    'Gfx_UninstalledDispatch_56D90': (0x56d90, 20, 2, '47cac51c899a539e2fb9e21ea04f64c46ba0c964ca4d20b5f8f3101586364b1d'),
    'Gfx_UninstalledDispatch_56DB0': (0x56db0, 15, 1, '8712df09524247c3b22eef18213a2c7d3289e0929ac9f3c54c221ead055087da'),
    'Gfx_ImageOpDispatch': (0x56dc0, 20, 2, 'a36dcdae6600a974858cc66f8c213e5b0e94e5aaa755c35fd54d6ea49daf9772'),
    'Gfx_CreateDescriptorDispatch': (0x56de0, 20, 2, '4b892e967e307cfd490ce8c9e026a2c2f96f7ca77b1aceafccf2eb388847f4fc'),
    'Gfx_FreeDescriptorDispatch': (0x56e00, 15, 1, 'ac56b05bb0bb49106928975f6c55be891221b8a15887fc446520ac74fcf4b15c'),
    'Gfx_CopyDescriptorDispatch': (0x56e10, 20, 2, '4df461e4396b427c52bde31bb1c9b2c9173c009db8427aa2fd4cb41f0bdbc1ed'),
    'Gfx_ReservedDispatch': (0x56e30, 15, 1, 'ed08e50628bdf08f53ad96b88e5d9dab86c6b337186a18f8d8e6951da1221f22'),
    'Gfx_GetStateDispatch': (0x56e40, 15, 1, '9d4fe8577c937219b06bcef289111aecbca1ca42481219197dc5b00852d4b5e2'),
    'Gfx_SetStateDispatch': (0x56e50, 15, 1, 'd4a6aa806bb86c3e10bc4475cdc896c4ece70748edf944be89dccc226f876fda'),
}

DISPATCH_PTRS = {
    'Gfx_CloseDispatch': 'g_spriteClose',
    'Gfx_OptionDispatch': 'g_spriteOption',
    'Gfx_ConfigureDispatch': 'g_spriteConfigure',
    'Gfx_SetClipDispatch': 'g_spriteSetClip',
    'Gfx_DrawListDispatch': 'g_spriteDrawList',
    'Gfx_DrawDispatch': 'g_spriteDraw',
    'Gfx_HandleOpDispatch': 'g_spriteHandleOp',
    'Gfx_UninstalledDispatch_56D60': 'g_spriteUninstalled56D60',
    'Gfx_UninstalledDispatch_56D80': 'g_spriteUninstalled56D80',
    'Gfx_UninstalledDispatch_56D90': 'g_spriteUninstalled56D90',
    'Gfx_UninstalledDispatch_56DB0': 'g_spriteUninstalled56DB0',
    'Gfx_ImageOpDispatch': 'g_spriteImageOp',
    'Gfx_CreateDescriptorDispatch': 'g_spriteCreateDescriptor',
    'Gfx_FreeDescriptorDispatch': 'g_spriteFreeDescriptor',
    'Gfx_CopyDescriptorDispatch': 'g_spriteCopyDescriptor',
    'Gfx_ReservedDispatch': 'g_spriteReserved',
    'Gfx_GetStateDispatch': 'g_spriteGetState',
    'Gfx_SetStateDispatch': 'g_spriteSetState',
}

ORIGINAL_PTR_VAS = {
    'g_spriteClose': 0x50eba8,
    'g_spriteOption': 0x50ebac,
    'g_spriteConfigure': 0x50ebb0,
    'g_spriteSetClip': 0x50ebb4,
    'g_spriteDrawList': 0x50ebb8,
    'g_spriteDraw': 0x50ebbc,
    'g_spriteHandleOp': 0x50ebc0,
    'g_spriteUninstalled56D60': 0x50ebc4,
    'g_spriteUninstalled56D80': 0x50ebc8,
    'g_spriteUninstalled56D90': 0x50ebcc,
    'g_spriteUninstalled56DB0': 0x50ebd0,
    'g_spriteCreateDescriptor': 0x50ebd4,
    'g_spriteImageOp': 0x50ebd8,
    'g_spriteFreeDescriptor': 0x50ebdc,
    'g_spriteCopyDescriptor': 0x50ebe0,
    'g_spriteReserved': 0x50ebe4,
    'g_spriteSetState': 0x50ebe8,
    'g_spriteGetState': 0x50ebec,
}

EXPECTED_FPO = {
    rva: (sz, 0, argc, 0) for rva, sz, argc, _ in ROUTINES.values()
}

INPUTS = [
    'decomp/src/geputget.c',
    'decomp/include/geputget.h',
    'decomp/modules.json',
    'tools/build_decomp.py',
    'tools/verify_matching.py',
    'tools/verify_sprite_dispatch.py',
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
        (0x56ca6, 10), (0x56cbf, 1), (0x56ce3, 13), (0x56d0e, 2),
        (0x56d1f, 1), (0x56d39, 7), (0x56d54, 12), (0x56d74, 12),
        (0x56d8f, 1), (0x56da4, 12), (0x56dbf, 1), (0x56dd4, 12),
        (0x56df4, 12), (0x56e0f, 1), (0x56e24, 12), (0x56e3f, 1),
        (0x56e4f, 1), (0x56e5f, 1)
    ]
    for addr, pad_len in padding_checks:
        assert pe.get_data(addr, pad_len) == b'\xcc' * pad_len, f"Padding mismatch at 0x{addr:x}"

    # Verify relocations across the 18 routines (each has exactly 1 relocation for the pointer)
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


def verify_sprite_dispatch(record=False):
    original_pe = pefile.PE(data=verify_target())
    inspect_original(original_pe)

    build(dll=DLL)
    compiled_pe = pefile.PE(str(DLL))

    coverage = {name: {'comparisons': 0, 'cases': 0} for name in ROUTINES}

    for seed in range(32):
        rng = random.Random(0x56ca0 + seed)
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
                    command='uv run tools/verify_sprite_dispatch.py',
                    details={
                        'coverage': coverage[name],
                        'scope': 'Authentic sprite dispatch table entry wrappers verified differentially against IGN_WIN.EXE with live target hooks and cdecl argument passing',
                        'limitations': 'Validation DLL focused harness; no full rasterizer or complete playable game parity'
                    }
                )

    print('PASS sprite dispatch verification:', coverage, flush=True)
    return coverage


if __name__ == '__main__':
    verify_sprite_dispatch(record=True)
