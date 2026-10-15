#!/usr/bin/env python3
"""Differential verification for authentic Windows CD Audio routines.
Tests byte-for-byte state and event parity against authentic IGN_WIN.EXE in Unicorn emulation.
"""
import hashlib
from pathlib import Path
import random
import shutil
import struct
import subprocess

from capstone import Cs, CS_ARCH_X86, CS_MODE_32
import pefile
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import (
    UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_EBX,
    UC_X86_REG_ESP, UC_X86_REG_EBP, UC_X86_REG_ESI, UC_X86_REG_EDI,
    UC_X86_REG_EIP
)

from windows_target import ROOT, BUILD
from windows_tracking import record_run

DLL = BUILD / 'audio_validation.dll'

STACK = 0x7000000
STOP = 0x7100000
HOOK_BASE = 0x7200000

ROUTINES = {
    'Audio_StopCD': (0x57FC0, 31, '46ebcedec71524bae04d1209584d1d5095159a057c8b44e3521809efe80d5bc2'),
    'Audio_OpenCD': (0x58040, 89, 'b827c0b4fbab3f4a78aaa85396e801e9500f94ba82313a6090a058627d6788a4'),
    'Audio_CloseCD': (0x580A0, 31, '526162ad95278c952596a9ca0b48853d19d392abe5058e4f8548877b048ee81a'),
    'Audio_EjectCD': (0x580C0, 31, '4b75cc2471d34550330b8b52aeab8e325185db3f0f50632fb66a110769e847de'),
    'Audio_ShutdownSound': (0x59D30, 3, '4bc724f3b1d0caf4fe369c18cba3102e6c4ea057f63fe1587e3973134a7f755e'),
}

EXPECTED_FPO = {
    0x57FC0: (31, 0, 0, 0),
    0x58040: (89, 0, 0, 257),
    0x580A0: (31, 0, 0, 0),
    0x580C0: (31, 0, 0, 0),
    0x59D30: (3, 0, 0, 0),
}

GLOBALS = {
    'g_cdAudioReturnString': (0x50EF70, 256),
}

TRACKING_INPUTS = [
    'decomp/src/audio.c',
    'decomp/include/audio.h',
    'decomp/target.json',
    'tools/verify_audio.py',
    'tools/windows_target.py',
    'tools/windows_tracking.py'
]

def inspect_original(pe):
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {r: (n, l, a, b) for r, n, l, a, b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData + debug.SizeOfData])}
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    for name, (rva, size, digest) in ROUTINES.items():
        assert fpo[rva] == EXPECTED_FPO[rva], f"FPO mismatch for {name}: {fpo[rva]} vs {EXPECTED_FPO[rva]}"
        raw = pe.get_data(rva, size)
        assert hashlib.sha256(raw).hexdigest() == digest, f"Digest mismatch for {name}"
        ins = list(md.disasm(raw, 0x400000 + rva))
        assert len(ins) > 0

def validation_symbols(pe):
    base = pe.OPTIONAL_HEADER.ImageBase
    symbols = {}
    if hasattr(pe, 'DIRECTORY_ENTRY_EXPORT'):
        for exp in pe.DIRECTORY_ENTRY_EXPORT.symbols:
            if exp.name:
                symbols[exp.name.decode('ascii')] = base + exp.address
    return symbols

def build_validation_dll():
    BUILD.mkdir(parents=True, exist_ok=True)
    dlltool = shutil.which('llvm-dlltool')
    gcc = shutil.which('gcc')
    ld = shutil.which('lld-link')
    if not gcc or not ld or not dlltool:
        raise RuntimeError('Provisional gcc, lld-link and llvm-dlltool required')

    audio_obj = BUILD / 'audio_validation.obj'
    subprocess.run([gcc, '-m32', '-std=c89', '-pedantic-errors', '-Wall', '-Wextra', '-Werror',
        '-O2', '-fno-inline', '-fno-optimize-sibling-calls',
        '-I', str(ROOT/'decomp/include'), '-c', str(ROOT/'decomp/src/audio.c'),
        '-o', str(audio_obj)], check=True)

    def_file = BUILD / 'audio_test_winmm.def'
    lib_file = BUILD / 'audio_test_winmm.lib'
    def_file.write_text('LIBRARY winmm.dll\nEXPORTS\nmciSendStringA@16\n', encoding='utf-8')
    subprocess.run([dlltool, '-m', 'i386', '-d', str(def_file), '-l', str(lib_file), '--kill-at'], check=True)

    exports = list(ROUTINES.keys()) + list(GLOBALS.keys())
    link_cmd = [ld, '/dll', '/noentry', '/nodefaultlib', '/machine:x86', '/safeseh:no',
                '/base:0x10000000', f'/out:{DLL}', str(audio_obj)] + [f'/export:{e}' for e in exports] + [str(lib_file)]
    subprocess.run(link_cmd, check=True)
    return DLL

SLOT_MCI_SEND_STRING = 0

class MockContext:
    def __init__(self):
        self.calls = []
        self.mci_returns = []
        self.default_mci_return = 0

class AudioSession:
    def __init__(self, pe, is_original):
        self.is_original = is_original
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.size = (pe.OPTIONAL_HEADER.SizeOfImage + 0xFFF) & ~0xFFF
        self.uc = Uc(UC_ARCH_X86, UC_MODE_32)
        self.uc.mem_map(self.base, self.size)
        self.uc.mem_write(self.base, pe.get_memory_mapped_image())

        self.uc.mem_map(STACK, 0x10000)
        self.uc.mem_map(STOP, 0x1000)
        self.uc.mem_write(STOP, b'\xcc')

        self.uc.mem_map(HOOK_BASE, 0x10000)
        for slot in range(16):
            self.uc.mem_write(HOOK_BASE + slot * 16, b'\x90\x90\x90\x90\xc3')

        self.symbols = {}
        if is_original:
            self.symbols.update({n: 0x400000 + r[0] for n, r in ROUTINES.items()})
            self.symbols.update({n: a for n, (a, _) in GLOBALS.items()})
            # Patch IAT slot for mciSendStringA in IGN_WIN.EXE
            self.uc.mem_write(0x64C468, struct.pack('<I', self._hook_slot_addr(SLOT_MCI_SEND_STRING)))
        else:
            syms = validation_symbols(pe)
            self.symbols.update(syms)
            # Patch IAT in DLL
            if hasattr(pe, 'DIRECTORY_ENTRY_IMPORT'):
                for imp in pe.DIRECTORY_ENTRY_IMPORT:
                    for entry in imp.imports:
                        clean_name = entry.name.decode('ascii') if entry.name else ''
                        clean_name = clean_name.lstrip('_').split('@')[0]
                        if clean_name == 'mciSendStringA':
                            self.uc.mem_write(entry.address, struct.pack('<I', self._hook_slot_addr(SLOT_MCI_SEND_STRING)))

        self.ctx = MockContext()
        self.uc.hook_add(UC_HOOK_CODE, self._hook_code)

    def _hook_slot_addr(self, slot):
        return HOOK_BASE + slot * 16

    def _hook_code(self, uc, address, size, user_data):
        if not (HOOK_BASE <= address < HOOK_BASE + 0x1000):
            return
        slot = (address - HOOK_BASE) // 16
        sp = uc.reg_read(UC_X86_REG_ESP)
        ret_addr = struct.unpack('<I', uc.mem_read(sp, 4))[0]

        if slot == SLOT_MCI_SEND_STRING:
            cmd_ptr, ret_ptr, ret_len, hwnd = struct.unpack('<4I', uc.mem_read(sp + 4, 16))
            cmd_bytes = bytearray()
            while True:
                b = uc.mem_read(cmd_ptr + len(cmd_bytes), 1)[0]
                if b == 0:
                    break
                cmd_bytes.append(b)
            cmd_str = cmd_bytes.decode('ascii', errors='replace')
            self.ctx.calls.append(('mciSendStringA', cmd_str, ret_len, hwnd))
            ret_code = self.ctx.mci_returns.pop(0) if self.ctx.mci_returns else self.ctx.default_mci_return
            uc.reg_write(UC_X86_REG_EAX, ret_code)
            uc.reg_write(UC_X86_REG_ESP, sp + 20)  # stdcall 4 args = 16 bytes + 4 bytes ret addr
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

    def invoke(self, name, args=(), initial_globals=None, mci_returns=None):
        self.ctx.calls = []
        self.ctx.mci_returns = list(mci_returns) if mci_returns is not None else []
        self.ctx.default_mci_return = 0

        sp = STACK + 0x8000
        for arg in reversed(args):
            sp -= 4
            self.uc.mem_write(sp, struct.pack('<I', arg))
        sp -= 4
        self.uc.mem_write(sp, struct.pack('<I', STOP))

        self.uc.reg_write(UC_X86_REG_ESP, sp)
        self.uc.reg_write(UC_X86_REG_EBP, sp)
        self.uc.reg_write(UC_X86_REG_EBX, 0x11111111)
        self.uc.reg_write(UC_X86_REG_ESI, 0x22222222)
        self.uc.reg_write(UC_X86_REG_EDI, 0x33333333)

        if initial_globals:
            for gname, val in initial_globals.items():
                addr = self.symbols[gname]
                if isinstance(val, int):
                    self.uc.mem_write(addr, struct.pack('<i', val))
                elif isinstance(val, bytes):
                    self.uc.mem_write(addr, val)

        target = self.symbols[name]
        self.uc.emu_start(target, STOP, timeout=5000000)

        eax = self.uc.reg_read(UC_X86_REG_EAX)
        eax = struct.unpack('<i', struct.pack('<I', eax))[0]

        final_globals = {}
        for gname, (_, size) in GLOBALS.items():
            final_globals[gname] = bytes(self.uc.mem_read(self.symbols[gname], size))

        return eax, list(self.ctx.calls), final_globals

def run_tests():
    pe_orig = pefile.PE(str(ROOT / 'Ignition/Ignition/IGN_WIN.EXE'))
    inspect_original(pe_orig)

    dll_path = build_validation_dll()
    pe_dll = pefile.PE(str(dll_path))

    sess_orig = AudioSession(pe_orig, is_original=True)
    sess_dll = AudioSession(pe_dll, is_original=False)

    total_comparisons = 0

    # 1. Test Audio_StopCD
    for seed in range(30):
        rng = random.Random(seed + 100)
        mci_ret = rng.choice([0, 1, 2, 257, 0x1000])
        ret_orig, calls_orig, state_orig = sess_orig.invoke('Audio_StopCD', (), mci_returns=[mci_ret])
        ret_dll, calls_dll, state_dll = sess_dll.invoke('Audio_StopCD', (), mci_returns=[mci_ret])

        expected_ret = 1 if mci_ret == 0 else 0
        assert ret_orig == ret_dll == expected_ret, f"Audio_StopCD ret mismatch: {ret_orig} vs {ret_dll}"
        assert calls_orig == calls_dll == [('mciSendStringA', 'stop cdaudio', 256, 0)], f"Audio_StopCD calls mismatch"
        assert state_orig == state_dll
        total_comparisons += 1

    # 2. Test Audio_OpenCD
    for seed in range(40):
        rng = random.Random(seed + 200)
        # Test all permutations of error on call 1, 2, 3 or all succeed
        mode = rng.choice(['all_ok', 'fail_1', 'fail_2', 'fail_3'])
        if mode == 'all_ok':
            mci_rets = [0, 0, 0]
            exp_ret = 1
            exp_calls = [
                ('mciSendStringA', 'open cdaudio', 256, 0),
                ('mciSendStringA', 'set cdaudio audio all on', 256, 0),
                ('mciSendStringA', 'set cdaudio time format msf', 256, 0)
            ]
        elif mode == 'fail_1':
            mci_rets = [rng.randint(1, 1000), 0, 0]
            exp_ret = 0
            exp_calls = [('mciSendStringA', 'open cdaudio', 256, 0)]
        elif mode == 'fail_2':
            mci_rets = [0, rng.randint(1, 1000), 0]
            exp_ret = 0
            exp_calls = [
                ('mciSendStringA', 'open cdaudio', 256, 0),
                ('mciSendStringA', 'set cdaudio audio all on', 256, 0)
            ]
        else:
            mci_rets = [0, 0, rng.randint(1, 1000)]
            exp_ret = 0
            exp_calls = [
                ('mciSendStringA', 'open cdaudio', 256, 0),
                ('mciSendStringA', 'set cdaudio audio all on', 256, 0),
                ('mciSendStringA', 'set cdaudio time format msf', 256, 0)
            ]

        ret_orig, calls_orig, state_orig = sess_orig.invoke('Audio_OpenCD', (), mci_returns=mci_rets)
        ret_dll, calls_dll, state_dll = sess_dll.invoke('Audio_OpenCD', (), mci_returns=mci_rets)

        assert ret_orig == ret_dll == exp_ret, f"Audio_OpenCD ret mismatch: {ret_orig} vs {ret_dll}"
        assert calls_orig == calls_dll == exp_calls, f"Audio_OpenCD calls mismatch"
        assert state_orig == state_dll
        total_comparisons += 1

    # 3. Test Audio_CloseCD
    for seed in range(30):
        rng = random.Random(seed + 300)
        mci_ret = rng.choice([0, 1, 2, 257, 0x1000])
        ret_orig, calls_orig, state_orig = sess_orig.invoke('Audio_CloseCD', (), mci_returns=[mci_ret])
        ret_dll, calls_dll, state_dll = sess_dll.invoke('Audio_CloseCD', (), mci_returns=[mci_ret])

        expected_ret = 1 if mci_ret == 0 else 0
        assert ret_orig == ret_dll == expected_ret, f"Audio_CloseCD ret mismatch: {ret_orig} vs {ret_dll}"
        assert calls_orig == calls_dll == [('mciSendStringA', 'close cdaudio', 256, 0)], f"Audio_CloseCD calls mismatch"
        assert state_orig == state_dll
        total_comparisons += 1

    # 4. Test Audio_EjectCD
    for seed in range(30):
        rng = random.Random(seed + 400)
        mci_ret = rng.choice([0, 1, 2, 257, 0x1000])
        ret_orig, calls_orig, state_orig = sess_orig.invoke('Audio_EjectCD', (), mci_returns=[mci_ret])
        ret_dll, calls_dll, state_dll = sess_dll.invoke('Audio_EjectCD', (), mci_returns=[mci_ret])

        expected_ret = 1 if mci_ret == 0 else 0
        assert ret_orig == ret_dll == expected_ret, f"Audio_EjectCD ret mismatch: {ret_orig} vs {ret_dll}"
        assert calls_orig == calls_dll == [('mciSendStringA', 'set cdaudio door open', 256, 0)], f"Audio_EjectCD calls mismatch"
        assert state_orig == state_dll
        total_comparisons += 1

    # 5. Test Audio_ShutdownSound
    for seed in range(20):
        ret_orig, calls_orig, state_orig = sess_orig.invoke('Audio_ShutdownSound', ())
        ret_dll, calls_dll, state_dll = sess_dll.invoke('Audio_ShutdownSound', ())

        assert ret_orig == ret_dll == 0, f"Audio_ShutdownSound ret mismatch: {ret_orig} vs {ret_dll}"
        assert calls_orig == calls_dll == []
        assert state_orig == state_dll
        total_comparisons += 1

    print(f"PASS: {total_comparisons} audio differential trials passed with byte-for-byte state and event parity.")

    # Record runs for all 5 RVAs
    for name, (rva, _, _) in ROUTINES.items():
        for kind in ('compilation', 'emulation'):
            record_run(
                rva=rva,
                kind=kind,
                outcome='pass',
                inputs=TRACKING_INPUTS,
                artifact=DLL.relative_to(ROOT).as_posix(),
                cases=total_comparisons if kind == 'emulation' else 0,
                command='uv run python tools/verify_audio.py',
                details={
                    'symbol': name,
                    'trials': total_comparisons if kind == 'emulation' else 0,
                    'differential_oracle': 'authentic IGN_WIN.EXE in Unicorn emulation' if kind == 'emulation' else None,
                    'scope': 'C89 CD audio and sound interface compilation' if kind == 'compilation' else 'differential emulation',
                }
            )
    print("Recorded audio verification runs into database.")

if __name__ == '__main__':
    run_tests()
