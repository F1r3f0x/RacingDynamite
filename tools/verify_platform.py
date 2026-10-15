# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Complete authentic Windows platform subsystem verification against direct C89."""
import hashlib
import random
import shutil
import struct
import subprocess
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import (
    UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX, UC_X86_REG_EDX,
    UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_ESP,
    UC_X86_REG_EIP, UC_X86_REG_EFLAGS
)
from windows_target import ROOT, BUILD, TARGET, verify_target, validation_symbols
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions; run without -O')

DLL = BUILD / 'platform_validation.dll'
STACK = 0x7000000
STOP = 0x7100000
HOOK_BASE = 0x8000000
PRESERVED = [UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP]

ROUTINES = {
    'WinMain': (0x120A0, 390, '3460e40b77123335ede23ae49d122053a852b6398f037253ce07ca5520b2a88c'),
    'Game_ProcessFrame': (0x12230, 154, '4c50ba4401d440b7c1caa09ce8d55047f8d9eb38b235b81ebe5edbfcfae8153f'),
    'Game_WindowProc': (0x122D0, 400, '461e5b4afef4dd9642c7ce031556749db188c41ea06480eab871723b337452e0'),
    'Game_GetTime': (0x12460, 152, 'bd900f4be8ea23b0f2e49bcdb780f817e1896e172c75c66ea448f30985fa909b'),
    'Game_InitSubsystems': (0x12500, 47, 'e96334c54403bffb6aadf238c02d5b1078ec74d724cd419db84d743d09ef5218'),
    'Game_PostClose': (0x12530, 21, 'a4273cc806e5f6887cc388279cff7dd3c3d31bda16354480f1552d4b351d5939'),
    'Game_ShutdownSubsystems': (0x12550, 10, '9d322da9798d57756f64a6ddfe80ceb7eeab3e0cf8d01337a3b6a3a9f1d4dfc2'),
    'Game_SetPalette': (0x12560, 19, '20949484266a1eb89c69091e72f67c67ae9b6615b115a5e47463892fdc25dbf0'),
    'Game_InitGame': (0x17270, 58, 'ca67f09b7bb19c2a7e24331ea64f2e29e8d81019fc94ef8e71f3831d27a3348a'),
    'Game_ShutdownGame': (0x17E90, 8, '2620d36c50712bde0380c5fafff9f1a14cd0efed596e6a107823739cbb966aa2'),
    'Game_InitState': (0x184C0, 60, 'e1dc62b410b9864791e22d06c66fcc1bfed21de5c207ef533f2ab615c09b517f')
}

EXPECTED_FPO = {
    0x120A0: (390, 17, 4, 5148),
    0x12230: (154, 0, 0, 0),
    0x122D0: (400, 0, 4, 775),
    0x12460: (152, 0, 0, 522),
    0x12500: (47, 0, 0, 0),
    0x12530: (21, 0, 0, 262),
    0x12550: (10, 0, 0, 0),
    0x12560: (19, 0, 1, 0),
    0x17270: (58, 0, 0, 0),
    0x17E90: (8, 0, 0, 0),
    0x184C0: (60, 0, 0, 0)
}

GLOBALS = {
    'g_nativeWindowInstance': (0x4C5398, 4),
    'g_nativeWindow': (0x4C539C, 4),
    'g_nativeWindowClassName': (0x493740, 9),
    'g_nativeWindowTitle': (0x493778, 9),
    'g_nativeCursor': (0x4C5360, 4),
    'g_nativeWindowRect': (0x4C5350, 16),
    'g_nativeIsAppActive': (0x4C5384, 4),
    'g_nativeInitialized': (0x4C5390, 4),
    'g_nativeIsChangingDisplayMode': (0x49372C, 4),
    'g_nativePerfFrequency': (0x4C5348, 8),
    'g_nativePerfFrequencyAvailable': (0x493720, 4),
    'g_nativeStartCounter': (0x4C5368, 8),
    'g_nativeCurrentCounter': (0x4C5378, 8),
    'g_nativeAccumulatedActiveTime': (0x4937AC, 4),
    'g_nativeLastActiveTime': (0x4937B8, 4),
    'g_nativeFullscreen': (0x493728, 4),
    'g_nativeUnknownPlatformPtr': (0x4C53A0, 4),
    'g_nativeStage': (0x493734, 4),
    'g_nativeInitGuard': (0x4937B0, 4),
    'g_nativeShutdownGuard': (0x4937B4, 4),
    'g_nativeLastFrameTime': (0x49373C, 4),
    'g_gameMainState': (0x639394, 4),
    'g_gameMenuState': (0x563C3C, 4),
    'g_gameMenuSelected': (0x563D9C, 4),
    'g_gameRaceLoadStage': (0x553290, 4),
    'g_gameInRace': (0x525E5C, 4),
    'g_gameFlag563BF4': (0x563BF4, 4),
    'g_gameGraphicsInitErrorFlag': (0x54F970, 4),
    'g_cdAudioMediaPresent': (0x50EF4C, 4),
    'g_gameConfigValues': (0x50F2CC, 16),
    'g_CameraShakeActive': (0x4949E0, 4),
    'g_gameStateFlag5530DC': (0x5530DC, 4),
}

TRACKING_INPUTS = [
    'decomp/src/platform.c',
    'decomp/include/platform.h',
    'decomp/include/surface.h',
    'decomp/include/geputget.h',
    'decomp/target.json',
    'tools/verify_platform.py',
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
        if name == 'Game_WindowProc':
            # Contains jump table at 0x41244C; instructions end at 0x412448 (ret 0x10)
            assert any(i.mnemonic == 'ret' for i in ins)
        elif name == 'Game_ShutdownSubsystems':
            assert ins[-1].mnemonic == 'jmp'
        else:
            assert ins[-1].mnemonic == 'ret'

def build_validation_dll():
    BUILD.mkdir(parents=True, exist_ok=True)
    dlltool = shutil.which('llvm-dlltool')
    gcc = shutil.which('gcc')
    ld = shutil.which('lld-link')
    if not gcc or not ld or not dlltool:
        raise RuntimeError('Provisional gcc, lld-link and llvm-dlltool required')

    platform_obj = BUILD / 'platform_validation.obj'
    subprocess.run([gcc, '-m32', '-std=c89', '-pedantic-errors', '-Wall', '-Wextra', '-Werror',
        '-O2', '-fno-inline', '-fno-optimize-sibling-calls',
        '-I', str(ROOT/'decomp/include'), '-c', str(ROOT/'decomp/src/platform.c'),
        '-o', str(platform_obj)], check=True)

    fixture_src = BUILD / 'platform_fixtures.c'
    fixture_src.write_text('''
#include <windows.h>
#include <stdint.h>

volatile int32_t g_nativeFullscreen = 1;

LONGLONG __divdi3(LONGLONG a, LONGLONG b) {
    unsigned __int64 ua, ub, rem = 0, quot = 0;
    int i, neg = 0;
    if (b == 0) return 0;
    if (a < 0) { ua = (unsigned __int64)(-a); neg = !neg; } else { ua = (unsigned __int64)a; }
    if (b < 0) { ub = (unsigned __int64)(-b); neg = !neg; } else { ub = (unsigned __int64)b; }
    for (i = 63; i >= 0; i--) {
        rem = (rem << 1) | ((ua >> i) & 1);
        if (rem >= ub) {
            rem -= ub;
            quot |= ((unsigned __int64)1 << i);
        }
    }
    return neg ? -(LONGLONG)quot : (LONGLONG)quot;
}

int Gfx_SelectBackend(int backend) { (void)backend; return 1; }
int Gfx_Open(void) { return 1; }
int Gfx_Shutdown(void) { return 1; }
int Gfx_SetPaletteNative(const unsigned char *rgb) { (void)rgb; return 1; }
int Input_InitNative(void) { return 1; }
int Input_ShutdownNative(void) { return 1; }
int Mem_InitSystem(void) { return 1; }
int Mem_ShutdownSystem(void) { return 1; }
int Game_UpdateFrame(void) { return 1; }
int Game_InitResources(void) { return 1; }
int Game_CleanupResources(void) { return 1; }
int Audio_CheckCDMediaPresent(void) { return 1; }
int Audio_InitCDTracks(void) { return 1; }
''', encoding='utf-8')

    fixture_obj = BUILD / 'platform_fixtures.obj'
    subprocess.run([gcc, '-m32', '-std=c89', '-pedantic-errors', '-Wall', '-Wextra', '-Werror',
        '-O2', '-I', str(ROOT/'decomp/include'), '-c', str(fixture_src),
        '-o', str(fixture_obj)], check=True)

    libs = []
    groups = {
        'USER32': ['PostQuitMessage@4', 'GetSystemMetrics@4', 'SetRect@20',
                   'GetClientRect@8', 'ClientToScreen@8', 'DefWindowProcA@16',
                   'SetCursor@4', 'DestroyWindow@4', 'PostMessageA@16',
                   'LoadCursorA@8', 'LoadIconA@8', 'RegisterClassA@4',
                   'PeekMessageA@20', 'GetMessageA@16', 'TranslateMessage@4',
                   'DispatchMessageA@4'],
        'KERNEL32': ['QueryPerformanceCounter@4', 'QueryPerformanceFrequency@4'],
        'GDI32': ['GetStockObject@4'],
        'WINMM': ['timeGetTime@0', 'timeBeginPeriod@4', 'timeEndPeriod@4']
    }
    for dll_name, exports in groups.items():
        def_file = BUILD / f'plat_test_{dll_name.lower()}.def'
        lib_file = BUILD / f'plat_test_{dll_name.lower()}.lib'
        def_file.write_text(f'LIBRARY {dll_name}.dll\nEXPORTS\n' + '\n'.join(exports) + '\n', encoding='utf-8')
        subprocess.run([dlltool, '-m', 'i386', '-d', str(def_file), '-l', str(lib_file), '--kill-at'], check=True)
        libs.append(str(lib_file))

    fixture_exports = [
        'Gfx_SelectBackend', 'Gfx_Open', 'Gfx_Shutdown',
        'Gfx_SetPaletteNative', 'Input_InitNative', 'Input_ShutdownNative',
        'Mem_InitSystem', 'Mem_ShutdownSystem',
        'Game_UpdateFrame', 'Game_InitResources',
        'Game_CleanupResources', 'Audio_CheckCDMediaPresent',
        'Audio_InitCDTracks'
    ]
    exports = list(ROUTINES.keys()) + list(GLOBALS.keys()) + fixture_exports
    link_cmd = [ld, '/dll', '/noentry', '/nodefaultlib', '/machine:x86', '/safeseh:no',
                '/base:0x10000000', f'/out:{DLL}', str(platform_obj), str(fixture_obj)] + [f'/export:{e}' for e in exports] + libs
    subprocess.run(link_cmd, check=True)
    return DLL

# Slot definitions for hook stubs
SLOT_POST_QUIT_MESSAGE = 0
SLOT_GET_SYSTEM_METRICS = 1
SLOT_SET_RECT = 2
SLOT_GET_CLIENT_RECT = 3
SLOT_CLIENT_TO_SCREEN = 4
SLOT_DEF_WINDOW_PROC = 5
SLOT_SET_CURSOR = 6
SLOT_DESTROY_WINDOW = 7
SLOT_POST_MESSAGE = 8
SLOT_QPC = 9
SLOT_TIME_GET_TIME = 10
SLOT_GFX_SHUTDOWN = 11
SLOT_GFX_OPEN = 12
SLOT_INPUT_INIT = 13
SLOT_INPUT_SHUTDOWN = 14
SLOT_GFX_SELECT_BACKEND = 15
SLOT_GFX_SET_PALETTE = 16
SLOT_LOAD_CURSOR = 17
SLOT_LOAD_ICON = 18
SLOT_GET_STOCK_OBJECT = 19
SLOT_REGISTER_CLASS = 20
SLOT_QPF = 21
SLOT_TIME_BEGIN_PERIOD = 22
SLOT_TIME_END_PERIOD = 23
SLOT_MEM_INIT_SYSTEM = 24
SLOT_MEM_SHUTDOWN_SYSTEM = 25
SLOT_GAME_INIT_GAME = 26
SLOT_GAME_UPDATE_FRAME = 27
SLOT_GAME_SHUTDOWN_GAME = 28
SLOT_PEEK_MESSAGE = 29
SLOT_GET_MESSAGE = 30
SLOT_TRANSLATE_MESSAGE = 31
SLOT_DISPATCH_MESSAGE = 32
SLOT_GAME_INIT_RESOURCES = 33
SLOT_GAME_INIT_STATE = 34
SLOT_GAME_CLEANUP_RESOURCES = 35
SLOT_AUDIO_CHECK_CD = 36
SLOT_AUDIO_INIT_CD = 37

class MockContext:
    def __init__(self):
        self.calls = []
        self.qpc_value = 1000000
        self.time_value = 50000
        self.open_result = 1
        self.input_init_result = 1
        self.screen_cx = 640
        self.screen_cy = 480
        self.client_rect = (0, 0, 640, 480)
        self.screen_offset = (10, 20)
        self.register_class_result = 0x1234
        self.qpf_result = 1
        self.qpf_value = 3579545
        self.messages = []
        self.post_close_quits = False

class PlatformSession:
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
        # Fill hook slots with NOPs and RET to avoid invalid read
        for slot in range(64):
            self.uc.mem_write(HOOK_BASE + slot * 16, b'\x90\x90\x90\x90\xc3')

        self.symbols = {}
        if is_original:
            self.symbols.update({n: 0x400000 + r[0] for n, r in ROUTINES.items()})
            self.symbols.update({n: a for n, (a, _) in GLOBALS.items()})
            self._patch_original_iat_and_calls()
        else:
            syms = validation_symbols(pe)
            self.symbols.update(syms)
            self._patch_dll_iat_and_calls(pe)

        self.ctx = MockContext()
        self.uc.hook_add(UC_HOOK_CODE, self._hook_code)

    def _hook_slot_addr(self, slot):
        return HOOK_BASE + slot * 16

    def _patch_original_iat_and_calls(self):
        # IAT slots
        self.uc.mem_write(0x64c3cc, struct.pack('<I', self._hook_slot_addr(SLOT_POST_QUIT_MESSAGE)))
        self.uc.mem_write(0x64c3e0, struct.pack('<I', self._hook_slot_addr(SLOT_GET_SYSTEM_METRICS)))
        self.uc.mem_write(0x64c3d0, struct.pack('<I', self._hook_slot_addr(SLOT_SET_RECT)))
        self.uc.mem_write(0x64c3d8, struct.pack('<I', self._hook_slot_addr(SLOT_GET_CLIENT_RECT)))
        self.uc.mem_write(0x64c3d4, struct.pack('<I', self._hook_slot_addr(SLOT_CLIENT_TO_SCREEN)))
        self.uc.mem_write(0x64c444, struct.pack('<I', self._hook_slot_addr(SLOT_SET_CURSOR)))
        self.uc.mem_write(0x64c434, struct.pack('<I', self._hook_slot_addr(SLOT_DESTROY_WINDOW)))
        self.uc.mem_write(0x64c3dc, struct.pack('<I', self._hook_slot_addr(SLOT_POST_MESSAGE)))
        self.uc.mem_write(0x64c448, struct.pack('<I', self._hook_slot_addr(SLOT_DEF_WINDOW_PROC)))
        self.uc.mem_write(0x64c38c, struct.pack('<I', self._hook_slot_addr(SLOT_QPC)))
        self.uc.mem_write(0x64c47c, struct.pack('<I', self._hook_slot_addr(SLOT_TIME_GET_TIME)))
        self.uc.mem_write(0x64c2cc, struct.pack('<I', self._hook_slot_addr(SLOT_QPF)))
        self.uc.mem_write(0x64c2c4, struct.pack('<I', self._hook_slot_addr(SLOT_GET_STOCK_OBJECT)))
        self.uc.mem_write(0x64c430, struct.pack('<I', self._hook_slot_addr(SLOT_LOAD_CURSOR)))
        self.uc.mem_write(0x64c454, struct.pack('<I', self._hook_slot_addr(SLOT_LOAD_ICON)))
        self.uc.mem_write(0x64c458, struct.pack('<I', self._hook_slot_addr(SLOT_REGISTER_CLASS)))
        self.uc.mem_write(0x64c44c, struct.pack('<I', self._hook_slot_addr(SLOT_PEEK_MESSAGE)))
        self.uc.mem_write(0x64c450, struct.pack('<I', self._hook_slot_addr(SLOT_GET_MESSAGE)))
        self.uc.mem_write(0x64c3ec, struct.pack('<I', self._hook_slot_addr(SLOT_TRANSLATE_MESSAGE)))
        self.uc.mem_write(0x64c3f0, struct.pack('<I', self._hook_slot_addr(SLOT_DISPATCH_MESSAGE)))
        self.uc.mem_write(0x64c470, struct.pack('<I', self._hook_slot_addr(SLOT_TIME_BEGIN_PERIOD)))
        self.uc.mem_write(0x64c478, struct.pack('<I', self._hook_slot_addr(SLOT_TIME_END_PERIOD)))

        # Subroutines in IGN_WIN.EXE: replace their entry with jump to hook
        def hook_sub(addr, slot):
            target = self._hook_slot_addr(slot)
            rel = (target - (addr + 5)) & 0xFFFFFFFF
            self.uc.mem_write(addr, b'\xe9' + struct.pack('<I', rel))

        hook_sub(0x456bf0, SLOT_GFX_SHUTDOWN)
        hook_sub(0x456bc0, SLOT_GFX_OPEN)
        hook_sub(0x455ac0, SLOT_INPUT_INIT)
        hook_sub(0x455e20, SLOT_INPUT_SHUTDOWN)
        hook_sub(0x456af0, SLOT_GFX_SELECT_BACKEND)
        hook_sub(0x456c40, SLOT_GFX_SET_PALETTE)
        hook_sub(0x45b170, SLOT_MEM_INIT_SYSTEM)
        hook_sub(0x45b1a0, SLOT_MEM_SHUTDOWN_SYSTEM)
        hook_sub(0x4172b0, SLOT_GAME_UPDATE_FRAME)
        hook_sub(0x417ea0, SLOT_GAME_INIT_RESOURCES)
        hook_sub(0x420b70, SLOT_GAME_CLEANUP_RESOURCES)
        hook_sub(0x457b50, SLOT_AUDIO_CHECK_CD)
        hook_sub(0x457da0, SLOT_AUDIO_INIT_CD)

    def _patch_dll_iat_and_calls(self, pe):
        iat_map = {
            'PostQuitMessage': SLOT_POST_QUIT_MESSAGE,
            'GetSystemMetrics': SLOT_GET_SYSTEM_METRICS,
            'SetRect': SLOT_SET_RECT,
            'GetClientRect': SLOT_GET_CLIENT_RECT,
            'ClientToScreen': SLOT_CLIENT_TO_SCREEN,
            'SetCursor': SLOT_SET_CURSOR,
            'DestroyWindow': SLOT_DESTROY_WINDOW,
            'PostMessageA': SLOT_POST_MESSAGE,
            'DefWindowProcA': SLOT_DEF_WINDOW_PROC,
            'QueryPerformanceCounter': SLOT_QPC,
            'timeGetTime': SLOT_TIME_GET_TIME,
            'LoadCursorA': SLOT_LOAD_CURSOR,
            'LoadIconA': SLOT_LOAD_ICON,
            'GetStockObject': SLOT_GET_STOCK_OBJECT,
            'RegisterClassA': SLOT_REGISTER_CLASS,
            'QueryPerformanceFrequency': SLOT_QPF,
            'timeBeginPeriod': SLOT_TIME_BEGIN_PERIOD,
            'timeEndPeriod': SLOT_TIME_END_PERIOD,
            'PeekMessageA': SLOT_PEEK_MESSAGE,
            'GetMessageA': SLOT_GET_MESSAGE,
            'TranslateMessage': SLOT_TRANSLATE_MESSAGE,
            'DispatchMessageA': SLOT_DISPATCH_MESSAGE,
        }
        # In our DLL, hook fixture functions
        def hook_sym(name, slot):
            if name in self.symbols:
                addr = self.symbols[name]
                target = self._hook_slot_addr(slot)
                rel = (target - (addr + 5)) & 0xFFFFFFFF
                self.uc.mem_write(addr, b'\xe9' + struct.pack('<I', rel))

        hook_sym('Gfx_Shutdown', SLOT_GFX_SHUTDOWN)
        hook_sym('Gfx_Open', SLOT_GFX_OPEN)
        hook_sym('Input_InitNative', SLOT_INPUT_INIT)
        hook_sym('Input_ShutdownNative', SLOT_INPUT_SHUTDOWN)
        hook_sym('Gfx_SelectBackend', SLOT_GFX_SELECT_BACKEND)
        hook_sym('Gfx_SetPaletteNative', SLOT_GFX_SET_PALETTE)
        hook_sym('Mem_InitSystem', SLOT_MEM_INIT_SYSTEM)
        hook_sym('Mem_ShutdownSystem', SLOT_MEM_SHUTDOWN_SYSTEM)
        hook_sym('Game_UpdateFrame', SLOT_GAME_UPDATE_FRAME)
        hook_sym('Game_InitResources', SLOT_GAME_INIT_RESOURCES)
        hook_sym('Game_CleanupResources', SLOT_GAME_CLEANUP_RESOURCES)
        hook_sym('Audio_CheckCDMediaPresent', SLOT_AUDIO_CHECK_CD)
        hook_sym('Audio_InitCDTracks', SLOT_AUDIO_INIT_CD)

        # Patch IAT entries from pe.DIRECTORY_ENTRY_IMPORT
        if hasattr(pe, 'DIRECTORY_ENTRY_IMPORT'):
            for imp in pe.DIRECTORY_ENTRY_IMPORT:
                for entry in imp.imports:
                    clean_name = entry.name.decode('ascii') if entry.name else ''
                    clean_name = clean_name.lstrip('_').split('@')[0]
                    if clean_name in iat_map:
                        slot = iat_map[clean_name]
                        self.uc.mem_write(entry.address, struct.pack('<I', self._hook_slot_addr(slot)))

    def canonical(self, address):
        for n, (va, size) in GLOBALS.items():
            if n in self.symbols:
                offset = address - self.symbols[n]
                if 0 <= offset < size:
                    return va + offset
        for n, (rva, _, _) in ROUTINES.items():
            if n in self.symbols and self.symbols[n] == address:
                return 0x400000 + rva
        if STACK <= address < STACK + 0x10000:
            return 0x7000000
        return address

    def _hook_code(self, uc, address, size, user_data):
        if not (HOOK_BASE <= address < HOOK_BASE + 0x10000):
            return
        slot = (address - HOOK_BASE) // 16
        sp = uc.reg_read(UC_X86_REG_ESP)
        ret_addr = struct.unpack('<I', uc.mem_read(sp, 4))[0]

        if slot == SLOT_POST_QUIT_MESSAGE:
            code = struct.unpack('<I', uc.mem_read(sp + 4, 4))[0]
            self.ctx.calls.append(('PostQuitMessage', code))
            uc.reg_write(UC_X86_REG_EAX, 0)
            uc.reg_write(UC_X86_REG_ESP, sp + 8)  # stdcall 1 arg
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_GET_SYSTEM_METRICS:
            index = struct.unpack('<I', uc.mem_read(sp + 4, 4))[0]
            self.ctx.calls.append(('GetSystemMetrics', index))
            val = self.ctx.screen_cx if index == 0 else self.ctx.screen_cy
            uc.reg_write(UC_X86_REG_EAX, val)
            uc.reg_write(UC_X86_REG_ESP, sp + 8)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_SET_RECT:
            ptr, x1, y1, x2, y2 = struct.unpack('<5I', uc.mem_read(sp + 4, 20))
            self.ctx.calls.append(('SetRect', self.canonical(ptr), x1, y1, x2, y2))
            uc.mem_write(ptr, struct.pack('<4i', x1, y1, x2, y2))
            uc.reg_write(UC_X86_REG_EAX, 1)
            uc.reg_write(UC_X86_REG_ESP, sp + 24)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_GET_CLIENT_RECT:
            hwnd, ptr = struct.unpack('<2I', uc.mem_read(sp + 4, 8))
            self.ctx.calls.append(('GetClientRect', hwnd, self.canonical(ptr)))
            x1, y1, x2, y2 = self.ctx.client_rect
            uc.mem_write(ptr, struct.pack('<4i', x1, y1, x2, y2))
            uc.reg_write(UC_X86_REG_EAX, 1)
            uc.reg_write(UC_X86_REG_ESP, sp + 12)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_CLIENT_TO_SCREEN:
            hwnd, ptr = struct.unpack('<2I', uc.mem_read(sp + 4, 8))
            px, py = struct.unpack('<2i', uc.mem_read(ptr, 8))
            dx, dy = self.ctx.screen_offset
            self.ctx.calls.append(('ClientToScreen', hwnd, self.canonical(ptr), px, py))
            uc.mem_write(ptr, struct.pack('<2i', px + dx, py + dy))
            uc.reg_write(UC_X86_REG_EAX, 1)
            uc.reg_write(UC_X86_REG_ESP, sp + 12)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_DEF_WINDOW_PROC:
            hwnd, msg, wp, lp = struct.unpack('<4I', uc.mem_read(sp + 4, 16))
            self.ctx.calls.append(('DefWindowProcA', hwnd, msg, wp, lp))
            uc.reg_write(UC_X86_REG_EAX, 0x42)
            uc.reg_write(UC_X86_REG_ESP, sp + 20)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_SET_CURSOR:
            hcur = struct.unpack('<I', uc.mem_read(sp + 4, 4))[0]
            self.ctx.calls.append(('SetCursor', hcur))
            uc.reg_write(UC_X86_REG_EAX, 0)
            uc.reg_write(UC_X86_REG_ESP, sp + 8)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_DESTROY_WINDOW:
            hwnd = struct.unpack('<I', uc.mem_read(sp + 4, 4))[0]
            self.ctx.calls.append(('DestroyWindow', hwnd))
            uc.reg_write(UC_X86_REG_EAX, 1)
            uc.reg_write(UC_X86_REG_ESP, sp + 8)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_POST_MESSAGE:
            hwnd, msg, wp, lp = struct.unpack('<4I', uc.mem_read(sp + 4, 16))
            self.ctx.calls.append(('PostMessageA', hwnd, msg, wp, lp))
            if self.ctx.post_close_quits and msg == 0x10:
                self.ctx.messages.append((0x12, 0, 0))
            uc.reg_write(UC_X86_REG_EAX, 1)
            uc.reg_write(UC_X86_REG_ESP, sp + 20)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_QPC:
            ptr = struct.unpack('<I', uc.mem_read(sp + 4, 4))[0]
            self.ctx.calls.append(('QueryPerformanceCounter', self.canonical(ptr)))
            uc.mem_write(ptr, struct.pack('<q', self.ctx.qpc_value))
            uc.reg_write(UC_X86_REG_EAX, 1)
            uc.reg_write(UC_X86_REG_ESP, sp + 8)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_TIME_GET_TIME:
            self.ctx.calls.append(('timeGetTime',))
            uc.reg_write(UC_X86_REG_EAX, self.ctx.time_value)
            uc.reg_write(UC_X86_REG_ESP, sp + 4)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_GFX_SHUTDOWN:
            self.ctx.calls.append(('Gfx_Shutdown',))
            uc.reg_write(UC_X86_REG_EAX, 1)
            uc.reg_write(UC_X86_REG_ESP, sp + 4)  # cdecl 0 args
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_GFX_OPEN:
            self.ctx.calls.append(('Gfx_Open',))
            uc.reg_write(UC_X86_REG_EAX, self.ctx.open_result)
            uc.reg_write(UC_X86_REG_ESP, sp + 4)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_INPUT_INIT:
            self.ctx.calls.append(('Input_InitNative',))
            uc.reg_write(UC_X86_REG_EAX, self.ctx.input_init_result)
            uc.reg_write(UC_X86_REG_ESP, sp + 4)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_INPUT_SHUTDOWN:
            self.ctx.calls.append(('Input_ShutdownNative',))
            uc.reg_write(UC_X86_REG_EAX, 1)
            uc.reg_write(UC_X86_REG_ESP, sp + 4)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_GFX_SELECT_BACKEND:
            val = struct.unpack('<I', uc.mem_read(sp + 4, 4))[0]
            self.ctx.calls.append(('Gfx_SelectBackend', val))
            uc.reg_write(UC_X86_REG_EAX, 1)
            uc.reg_write(UC_X86_REG_ESP, sp + 4)  # caller cleans up (cdecl)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_GFX_SET_PALETTE:
            ptr = struct.unpack('<I', uc.mem_read(sp + 4, 4))[0]
            self.ctx.calls.append(('Gfx_SetPaletteNative', ptr))
            uc.reg_write(UC_X86_REG_EAX, 1)
            uc.reg_write(UC_X86_REG_ESP, sp + 4)  # cdecl
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_LOAD_CURSOR:
            hinst, cname = struct.unpack('<2I', uc.mem_read(sp + 4, 8))
            self.ctx.calls.append(('LoadCursorA', hinst, cname))
            uc.reg_write(UC_X86_REG_EAX, 0x5001)
            uc.reg_write(UC_X86_REG_ESP, sp + 12)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_LOAD_ICON:
            hinst, iname = struct.unpack('<2I', uc.mem_read(sp + 4, 8))
            self.ctx.calls.append(('LoadIconA', hinst, iname))
            uc.reg_write(UC_X86_REG_EAX, 0x6001)
            uc.reg_write(UC_X86_REG_ESP, sp + 12)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_GET_STOCK_OBJECT:
            obj = struct.unpack('<I', uc.mem_read(sp + 4, 4))[0]
            self.ctx.calls.append(('GetStockObject', obj))
            uc.reg_write(UC_X86_REG_EAX, 0x7001)
            uc.reg_write(UC_X86_REG_ESP, sp + 8)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_REGISTER_CLASS:
            wndclass_ptr = struct.unpack('<I', uc.mem_read(sp + 4, 4))[0]
            style, wndproc, extra1, extra2, hinst, hicon, hcursor, hbr, menu, cls = struct.unpack(
                '<10I', uc.mem_read(wndclass_ptr, 40))
            self.ctx.calls.append(('RegisterClassA', style, self.canonical(wndproc), extra1, extra2,
                                   hinst, hicon, hcursor, hbr, menu, self.canonical(cls)))
            uc.reg_write(UC_X86_REG_EAX, self.ctx.register_class_result)
            uc.reg_write(UC_X86_REG_ESP, sp + 8)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_QPF:
            ptr = struct.unpack('<I', uc.mem_read(sp + 4, 4))[0]
            self.ctx.calls.append(('QueryPerformanceFrequency', self.canonical(ptr)))
            if self.ctx.qpf_result:
                uc.mem_write(ptr, struct.pack('<q', self.ctx.qpf_value))
            uc.reg_write(UC_X86_REG_EAX, self.ctx.qpf_result)
            uc.reg_write(UC_X86_REG_ESP, sp + 8)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_TIME_BEGIN_PERIOD:
            p = struct.unpack('<I', uc.mem_read(sp + 4, 4))[0]
            self.ctx.calls.append(('timeBeginPeriod', p))
            uc.reg_write(UC_X86_REG_EAX, 0)
            uc.reg_write(UC_X86_REG_ESP, sp + 8)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_TIME_END_PERIOD:
            p = struct.unpack('<I', uc.mem_read(sp + 4, 4))[0]
            self.ctx.calls.append(('timeEndPeriod', p))
            uc.reg_write(UC_X86_REG_EAX, 0)
            uc.reg_write(UC_X86_REG_ESP, sp + 8)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_MEM_INIT_SYSTEM:
            self.ctx.calls.append(('Mem_InitSystem',))
            uc.reg_write(UC_X86_REG_EAX, 1)
            uc.reg_write(UC_X86_REG_ESP, sp + 4)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_MEM_SHUTDOWN_SYSTEM:
            self.ctx.calls.append(('Mem_ShutdownSystem',))
            uc.reg_write(UC_X86_REG_EAX, 1)
            uc.reg_write(UC_X86_REG_ESP, sp + 4)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_GAME_UPDATE_FRAME:
            self.ctx.calls.append(('Game_UpdateFrame',))
            uc.reg_write(UC_X86_REG_EAX, 1)
            uc.reg_write(UC_X86_REG_ESP, sp + 4)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_GAME_INIT_RESOURCES:
            self.ctx.calls.append(('Game_InitResources',))
            uc.reg_write(UC_X86_REG_EAX, 1)
            uc.reg_write(UC_X86_REG_ESP, sp + 4)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_AUDIO_CHECK_CD:
            self.ctx.calls.append(('Audio_CheckCDMediaPresent',))
            uc.reg_write(UC_X86_REG_EAX, 1)
            uc.reg_write(UC_X86_REG_ESP, sp + 4)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_AUDIO_INIT_CD:
            self.ctx.calls.append(('Audio_InitCDTracks',))
            uc.reg_write(UC_X86_REG_EAX, 1)
            uc.reg_write(UC_X86_REG_ESP, sp + 4)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_GAME_CLEANUP_RESOURCES:
            self.ctx.calls.append(('Game_CleanupResources',))
            uc.reg_write(UC_X86_REG_EAX, 1)
            uc.reg_write(UC_X86_REG_ESP, sp + 4)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_PEEK_MESSAGE:
            ptr, hwnd, min_m, max_m, rm = struct.unpack('<5I', uc.mem_read(sp + 4, 20))
            self.ctx.calls.append(('PeekMessageA', self.canonical(ptr), hwnd, min_m, max_m, rm))
            if len(self.ctx.messages) > 0:
                msg, wp, lp = self.ctx.messages[0]
                uc.mem_write(ptr, struct.pack('<7I', 0, msg, wp, lp, 0, 0, 0))
                uc.reg_write(UC_X86_REG_EAX, 1)
            else:
                uc.reg_write(UC_X86_REG_EAX, 0)
            uc.reg_write(UC_X86_REG_ESP, sp + 24)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_GET_MESSAGE:
            ptr, hwnd, min_m, max_m = struct.unpack('<4I', uc.mem_read(sp + 4, 16))
            self.ctx.calls.append(('GetMessageA', self.canonical(ptr), hwnd, min_m, max_m))
            if len(self.ctx.messages) > 0:
                msg, wp, lp = self.ctx.messages.pop(0)
            else:
                msg, wp, lp = (0x12, 0, 0)
            uc.mem_write(ptr, struct.pack('<7I', 0, msg, wp, lp, 0, 0, 0))
            ret = 0 if msg == 0x12 else 1
            uc.reg_write(UC_X86_REG_EAX, ret)
            uc.reg_write(UC_X86_REG_ESP, sp + 20)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_TRANSLATE_MESSAGE:
            ptr = struct.unpack('<I', uc.mem_read(sp + 4, 4))[0]
            msg, wp, lp = struct.unpack('<3I', uc.mem_read(ptr + 4, 12))
            self.ctx.calls.append(('TranslateMessage', msg, wp, lp))
            uc.reg_write(UC_X86_REG_EAX, 1)
            uc.reg_write(UC_X86_REG_ESP, sp + 8)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

        elif slot == SLOT_DISPATCH_MESSAGE:
            ptr = struct.unpack('<I', uc.mem_read(sp + 4, 4))[0]
            msg, wp, lp = struct.unpack('<3I', uc.mem_read(ptr + 4, 12))
            self.ctx.calls.append(('DispatchMessageA', msg, wp, lp))
            uc.reg_write(UC_X86_REG_EAX, 0)
            uc.reg_write(UC_X86_REG_ESP, sp + 8)
            uc.reg_write(UC_X86_REG_EIP, ret_addr)

    def invoke(self, name, args=(), state_init=None, ctx_init=None):
        self.ctx.calls = []
        if ctx_init:
            for k, v in ctx_init.items():
                setattr(self.ctx, k, list(v) if isinstance(v, list) else v)
        if state_init:
            for k, val in state_init.items():
                addr = self.symbols[k]
                _, size = GLOBALS[k]
                if isinstance(val, int):
                    fmt = '<i' if size == 4 else '<q' if size == 8 else '<b'
                    self.uc.mem_write(addr, struct.pack(fmt, val))
                elif isinstance(val, (bytes, bytearray)):
                    self.uc.mem_write(addr, bytes(val)[:size])
                elif isinstance(val, tuple) and len(val) == 4:
                    self.uc.mem_write(addr, struct.pack('<4i', *val))

        sp = STACK + 0x8000
        words = [STOP, *args]
        self.uc.mem_write(sp, struct.pack('<' + 'I'*len(words), *words))

        saved = [0x2468ACE0 + i for i in range(4)]
        for r, v in zip(PRESERVED, saved):
            self.uc.reg_write(r, v)
        self.uc.reg_write(UC_X86_REG_ESP, sp)
        self.uc.reg_write(UC_X86_REG_EFLAGS, 2)

        entry = self.symbols[name]
        self.uc.emu_start(entry, STOP, count=100000)

        assert self.uc.reg_read(UC_X86_REG_EIP) == STOP
        assert [self.uc.reg_read(r) for r in PRESERVED] == saved
        ret_val = self.uc.reg_read(UC_X86_REG_EAX)

        # Read back all global state
        state_out = {}
        for k, (_, size) in GLOBALS.items():
            state_out[k] = bytes(self.uc.mem_read(self.symbols[k], size))

        return ret_val, list(self.ctx.calls), state_out

def run_tests():
    verify_target()
    pe_orig = pefile.PE(str(TARGET))
    inspect_original(pe_orig)

    dll_path = build_validation_dll()
    pe_dll = pefile.PE(str(dll_path))

    sess_orig = PlatformSession(pe_orig, is_original=True)
    sess_dll = PlatformSession(pe_dll, is_original=False)

    total_comparisons = 0

    # 1. Test Game_WindowProc
    # We test multiple messages: WM_CREATE, WM_DESTROY, WM_MOVE, WM_SIZE, WM_ACTIVATEAPP, WM_SETCURSOR, WM_KEYDOWN, WM_SYSKEYDOWN, unhandled
    messages = [
        (1, 0, 0),                       # WM_CREATE
        (2, 0, 0),                       # WM_DESTROY (mode 0)
        (2, 0, 0),                       # WM_DESTROY (mode 1)
        (3, 0, 0),                       # WM_MOVE (fullscreen)
        (3, 0, 0),                       # WM_MOVE (windowed)
        (5, 0, 0),                       # WM_SIZE (fullscreen)
        (5, 0, 0),                       # WM_SIZE (windowed)
        (0x1C, 1, 0),                    # WM_ACTIVATEAPP (active=1)
        (0x1C, 0, 0),                    # WM_ACTIVATEAPP (active=0)
        (0x20, 0, 0),                    # WM_SETCURSOR (fullscreen)
        (0x20, 0, 0),                    # WM_SETCURSOR (windowed)
        (0x100, 0x41, 0),                # WM_KEYDOWN
        (0x105, 13, 0),                  # WM_SYSKEYDOWN VK_RETURN (init=1, fs=1 -> 0)
        (0x105, 13, 0),                  # WM_SYSKEYDOWN VK_RETURN (init=1, fs=0 -> 1)
        (0x105, 13, 0),                  # WM_SYSKEYDOWN VK_RETURN (init=0)
        (0x105, 32, 0),                  # WM_SYSKEYDOWN VK_SPACE
        (0xF, 0, 0),                     # WM_PAINT
        (0x111, 101, 0),                 # WM_COMMAND
        (0x201, 1, 0x00640064),          # WM_LBUTTONDOWN
        (0x400, 0, 0),                   # WM_USER
    ]

    for seed in range(25):
        rng = random.Random(seed)
        for idx, (msg, wp, lp) in enumerate(messages):
            hwnd = 0x10000 + rng.randint(1, 9999)
            fs = 1 if ('fullscreen' in str((idx, msg)) or (idx % 2 == 0)) else 0
            if idx == 1: mode = 0
            elif idx == 2: mode = 1
            else: mode = 0
            init_flag = 0 if idx == 14 else 1

            state = {
                'g_nativeWindow': hwnd,
                'g_nativeFullscreen': fs,
                'g_nativeIsChangingDisplayMode': mode,
                'g_nativeInitialized': init_flag,
                'g_nativeCursor': 0x5000 + idx,
                'g_nativeWindowRect': (0, 0, 0, 0),
                'g_nativeIsAppActive': rng.randint(0, 1),
            }
            ctx = {
                'screen_cx': rng.choice([640, 800, 1024, 1920]),
                'screen_cy': rng.choice([480, 600, 768, 1080]),
                'client_rect': (0, 0, rng.choice([640, 800]), rng.choice([480, 600])),
                'screen_offset': (rng.randint(0, 100), rng.randint(0, 100)),
            }

            ret_orig, calls_orig, state_orig = sess_orig.invoke('Game_WindowProc', (hwnd, msg, wp, lp), state, ctx)
            ret_dll, calls_dll, state_dll = sess_dll.invoke('Game_WindowProc', (hwnd, msg, wp, lp), state, ctx)

            assert ret_orig == ret_dll, f"WindowProc ret mismatch on msg {hex(msg)}: {ret_orig} vs {ret_dll}"
            assert calls_orig == calls_dll, f"WindowProc calls mismatch on msg {hex(msg)}:\norig: {calls_orig}\ndll: {calls_dll}"
            for k in GLOBALS:
                assert state_orig[k] == state_dll[k], f"WindowProc state mismatch on {k} for msg {hex(msg)}"
            total_comparisons += 1

    # 2. Test Game_GetTime
    for seed in range(30):
        rng = random.Random(seed + 100)
        for freq_avail in [0, 1]:
            for app_active in [0, 1]:
                for last_zero in [True, False]:
                    start_cnt = rng.randint(100000, 2000000)
                    qpc_val = start_cnt + rng.randint(10000, 500000)
                    freq_val = rng.choice([1193182, 3579545, 10000000])
                    time_val = rng.randint(100000, 5000000)
                    last_val = 0 if last_zero else (time_val - rng.randint(10, 100) if not freq_avail else rng.randint(1000, 5000))
                    accum_val = rng.randint(0, 500000)

                    state = {
                        'g_nativePerfFrequencyAvailable': freq_avail,
                        'g_nativePerfFrequency': freq_val,
                        'g_nativeStartCounter': start_cnt,
                        'g_nativeIsAppActive': app_active,
                        'g_nativeLastActiveTime': last_val,
                        'g_nativeAccumulatedActiveTime': accum_val,
                    }
                    ctx = {
                        'qpc_value': qpc_val,
                        'time_value': time_val,
                    }

                    ret_orig, calls_orig, state_orig = sess_orig.invoke('Game_GetTime', (), state, ctx)
                    ret_dll, calls_dll, state_dll = sess_dll.invoke('Game_GetTime', (), state, ctx)

                    assert ret_orig == ret_dll, f"GetTime ret mismatch: {ret_orig} vs {ret_dll}"
                    assert calls_orig == calls_dll, f"GetTime calls mismatch:\norig: {calls_orig}\ndll: {calls_dll}"
                    for k in GLOBALS:
                        assert state_orig[k] == state_dll[k], f"GetTime state mismatch on {k}"
                    total_comparisons += 1

    # 3. Test Game_InitSubsystems
    for seed in range(20):
        rng = random.Random(seed + 200)
        for open_res in [0, 1]:
            for init_input_res in [0, 1]:
                state = {
                    'g_nativeInitialized': 0,
                }
                ctx = {
                    'open_result': open_res,
                    'input_init_result': init_input_res,
                }
                ret_orig, calls_orig, state_orig = sess_orig.invoke('Game_InitSubsystems', (), state, ctx)
                ret_dll, calls_dll, state_dll = sess_dll.invoke('Game_InitSubsystems', (), state, ctx)

                assert ret_orig == ret_dll, f"InitSubsystems ret mismatch: {ret_orig} vs {ret_dll}"
                assert calls_orig == calls_dll, f"InitSubsystems calls mismatch:\norig: {calls_orig}\ndll: {calls_dll}"
                for k in GLOBALS:
                    assert state_orig[k] == state_dll[k], f"InitSubsystems state mismatch on {k}"
                total_comparisons += 1

    # 4. Test Game_PostClose
    for seed in range(20):
        rng = random.Random(seed + 300)
        hwnd = 0x20000 + rng.randint(1, 9999)
        state = {'g_nativeWindow': hwnd}
        ret_orig, calls_orig, state_orig = sess_orig.invoke('Game_PostClose', (), state)
        ret_dll, calls_dll, state_dll = sess_dll.invoke('Game_PostClose', (), state)

        assert calls_orig == calls_dll, f"PostClose calls mismatch:\norig: {calls_orig}\ndll: {calls_dll}"
        for k in GLOBALS:
            assert state_orig[k] == state_dll[k], f"PostClose state mismatch on {k}"
        total_comparisons += 1

    # 5. Test Game_ShutdownSubsystems
    for _ in range(10):
        ret_orig, calls_orig, state_orig = sess_orig.invoke('Game_ShutdownSubsystems', ())
        ret_dll, calls_dll, state_dll = sess_dll.invoke('Game_ShutdownSubsystems', ())

        assert calls_orig == calls_dll, f"ShutdownSubsystems calls mismatch:\norig: {calls_orig}\ndll: {calls_dll}"
        for k in GLOBALS:
            assert state_orig[k] == state_dll[k], f"ShutdownSubsystems state mismatch on {k}"
        total_comparisons += 1

    # 6. Test Game_SetPalette
    # We allocate a container in mapped memory: ptr -> pal_ptr
    PALETTE_DATA_ADDR = 0x3500000
    PALETTE_CONTAINER_ADDR = 0x3600000
    sess_orig.uc.mem_map(PALETTE_DATA_ADDR, 0x1000)
    sess_orig.uc.mem_map(PALETTE_CONTAINER_ADDR, 0x1000)
    sess_dll.uc.mem_map(PALETTE_DATA_ADDR, 0x1000)
    sess_dll.uc.mem_map(PALETTE_CONTAINER_ADDR, 0x1000)

    for seed in range(25):
        rng = random.Random(seed + 400)
        pal_data_ptr = PALETTE_DATA_ADDR + rng.randint(0, 0x800)
        sess_orig.uc.mem_write(PALETTE_CONTAINER_ADDR, struct.pack('<I', pal_data_ptr))
        sess_dll.uc.mem_write(PALETTE_CONTAINER_ADDR, struct.pack('<I', pal_data_ptr))

        ret_orig, calls_orig, state_orig = sess_orig.invoke('Game_SetPalette', (PALETTE_CONTAINER_ADDR,))
        ret_dll, calls_dll, state_dll = sess_dll.invoke('Game_SetPalette', (PALETTE_CONTAINER_ADDR,))

        assert calls_orig == calls_dll, f"SetPalette calls mismatch:\norig: {calls_orig}\ndll: {calls_dll}"
        expected_arg = pal_data_ptr + 0x10
        assert calls_orig == [('Gfx_SetPaletteNative', expected_arg)]
        total_comparisons += 1

    # 7. Test Game_ProcessFrame
    for seed in range(25):
        rng = random.Random(seed + 500)
        for stage in [0, 1, 2, 3]:
            for init_guard in [0, 1]:
                for shutdown_guard in [0, 1]:
                    for freq_avail in [0, 1]:
                        time_val = rng.randint(1000, 500000)
                        state = {
                            'g_nativeStage': stage,
                            'g_nativeInitGuard': init_guard,
                            'g_nativeShutdownGuard': shutdown_guard,
                            'g_nativePerfFrequencyAvailable': freq_avail,
                            'g_nativeLastFrameTime': 0,
                        }
                        ctx = {'time_value': time_val}
                        ret_orig, calls_orig, state_orig = sess_orig.invoke('Game_ProcessFrame', (), state, ctx)
                        ret_dll, calls_dll, state_dll = sess_dll.invoke('Game_ProcessFrame', (), state, ctx)

                        assert ret_orig == ret_dll, f"ProcessFrame ret mismatch for stage={stage}, init={init_guard}, shut={shutdown_guard}: {ret_orig} vs {ret_dll}"
                        assert calls_orig == calls_dll, f"ProcessFrame calls mismatch for stage={stage}, init={init_guard}, shut={shutdown_guard}:\norig: {calls_orig}\ndll: {calls_dll}"
                        for k in GLOBALS:
                            assert state_orig[k] == state_dll[k], f"ProcessFrame state mismatch on {k}"
                        total_comparisons += 1

    # 8. Test WinMain
    for seed in range(15):
        rng = random.Random(seed + 600)
        hinst = 0x400000 + rng.randint(0x1000, 0x9000)
        hprev = 0
        cmdline = 0
        show_cmd = 1

        # Trial A: RegisterClass fails
        ctx = {'register_class_result': 0}
        ret_orig, calls_orig, state_orig = sess_orig.invoke('WinMain', (hinst, hprev, cmdline, show_cmd), {}, ctx)
        ret_dll, calls_dll, state_dll = sess_dll.invoke('WinMain', (hinst, hprev, cmdline, show_cmd), {}, ctx)
        assert ret_orig == ret_dll == 0
        assert calls_orig == calls_dll
        for k in GLOBALS:
            assert state_orig[k] == state_dll[k]
        total_comparisons += 1

        # Trial B: Subsystems init fails
        ctx = {'register_class_result': 1, 'open_result': 0, 'qpf_result': 1, 'messages': [(0x12, 0, 0)]}
        ret_orig, calls_orig, state_orig = sess_orig.invoke('WinMain', (hinst, hprev, cmdline, show_cmd), {}, ctx)
        ret_dll, calls_dll, state_dll = sess_dll.invoke('WinMain', (hinst, hprev, cmdline, show_cmd), {}, ctx)
        assert ret_orig == ret_dll == 0
        assert calls_orig == calls_dll
        for k in GLOBALS:
            assert state_orig[k] == state_dll[k]
        total_comparisons += 1

        # Trial C: Inactive app, direct messages then quit
        exit_code = rng.randint(1, 100)
        ctx = {
            'register_class_result': 1,
            'open_result': 1,
            'input_init_result': 1,
            'qpf_result': 1,
            'messages': [(0x100, 0x41, 0), (0x12, exit_code, 0)]
        }
        state = {'g_nativeIsAppActive': 0}
        ret_orig, calls_orig, state_orig = sess_orig.invoke('WinMain', (hinst, hprev, cmdline, show_cmd), state, ctx)
        ret_dll, calls_dll, state_dll = sess_dll.invoke('WinMain', (hinst, hprev, cmdline, show_cmd), state, ctx)
        assert ret_orig == ret_dll == exit_code
        assert calls_orig == calls_dll
        for k in GLOBALS:
            assert state_orig[k] == state_dll[k]
        total_comparisons += 1

        # Trial D: Active app, message available then quit (timer fallback)
        exit_code = rng.randint(1, 100)
        ctx = {
            'register_class_result': 1,
            'open_result': 1,
            'input_init_result': 1,
            'qpf_result': 0,
            'messages': [(0x100, 0x42, 0), (0x12, exit_code, 0)]
        }
        state = {'g_nativeIsAppActive': 1}
        ret_orig, calls_orig, state_orig = sess_orig.invoke('WinMain', (hinst, hprev, cmdline, show_cmd), state, ctx)
        ret_dll, calls_dll, state_dll = sess_dll.invoke('WinMain', (hinst, hprev, cmdline, show_cmd), state, ctx)
        assert ret_orig == ret_dll == exit_code
        assert calls_orig == calls_dll
        for k in GLOBALS:
            assert state_orig[k] == state_dll[k]
        total_comparisons += 1

        # Trial E: Active app, idle frame (stage 2 shutdown -> Game_PostClose -> quit)
        ctx = {
            'register_class_result': 1,
            'open_result': 1,
            'input_init_result': 1,
            'qpf_result': 1,
            'post_close_quits': True,
            'messages': []
        }
        state = {
            'g_nativeIsAppActive': 1,
            'g_nativeStage': 2,
            'g_nativeShutdownGuard': 0,
            'g_nativeWindow': 0x30000 + seed,
        }
        ret_orig, calls_orig, state_orig = sess_orig.invoke('WinMain', (hinst, hprev, cmdline, show_cmd), state, ctx)
        ret_dll, calls_dll, state_dll = sess_dll.invoke('WinMain', (hinst, hprev, cmdline, show_cmd), state, ctx)
        assert ret_orig == ret_dll == 0
        assert calls_orig == calls_dll
        for k in GLOBALS:
            assert state_orig[k] == state_dll[k]
        total_comparisons += 1

        # Trial F: Active app, idle frame stage 0 init, then WM_QUIT
        exit_code = rng.randint(1, 100)
        ctx = {
            'register_class_result': 1,
            'open_result': 1,
            'input_init_result': 1,
            'qpf_result': 1,
            'messages': [(0x12, exit_code, 0)]
        }
        state = {
            'g_nativeIsAppActive': 1,
            'g_nativeStage': 0,
            'g_nativeInitGuard': 0,
            'g_nativeWindow': 0x30000 + seed,
        }
        ret_orig, calls_orig, state_orig = sess_orig.invoke('WinMain', (hinst, hprev, cmdline, show_cmd), state, ctx)
        ret_dll, calls_dll, state_dll = sess_dll.invoke('WinMain', (hinst, hprev, cmdline, show_cmd), state, ctx)
        assert ret_orig == ret_dll == exit_code
        assert calls_orig == calls_dll
        for k in GLOBALS:
            assert state_orig[k] == state_dll[k]
        total_comparisons += 1

    # 9. Test Game_InitGame
    for seed in range(30):
        rng = random.Random(seed + 700)
        cd_present = rng.choice([0, 1])
        cfg = [rng.randint(1, 100) for _ in range(4)]
        state = {
            'g_gameMainState': rng.randint(-1000, 1000),
            'g_gameMenuState': rng.randint(-1000, 1000),
            'g_gameMenuSelected': rng.randint(-1000, 1000),
            'g_gameRaceLoadStage': rng.randint(-1000, 1000),
            'g_gameInRace': rng.randint(-1000, 1000),
            'g_gameFlag563BF4': rng.randint(-1000, 1000),
            'g_gameGraphicsInitErrorFlag': rng.randint(-1000, 1000),
            'g_cdAudioMediaPresent': cd_present,
            'g_CameraShakeActive': rng.randint(-1000, 1000),
            'g_gameStateFlag5530DC': rng.randint(-1000, 1000),
            'g_gameConfigValues': struct.pack('<4i', *cfg),
        }
        ret_orig, calls_orig, state_orig = sess_orig.invoke('Game_InitGame', (), state)
        ret_dll, calls_dll, state_dll = sess_dll.invoke('Game_InitGame', (), state)

        assert ret_orig == ret_dll == 1, f"InitGame ret mismatch: {ret_orig} vs {ret_dll}"
        expected_calls = [('Game_InitResources',), ('Audio_CheckCDMediaPresent',)]
        if cd_present == 1:
            expected_calls.append(('Audio_InitCDTracks',))
        assert calls_orig == calls_dll == expected_calls, f"InitGame calls mismatch:\norig: {calls_orig}\ndll: {calls_dll}"
        for k in GLOBALS:
            assert state_orig[k] == state_dll[k], f"InitGame state mismatch on {k}"
        assert struct.unpack('<i', state_orig['g_gameMainState'])[0] == 1
        for flag in ['g_gameMenuState', 'g_gameMenuSelected', 'g_gameRaceLoadStage', 'g_gameInRace', 'g_gameFlag563BF4', 'g_gameGraphicsInitErrorFlag']:
            assert struct.unpack('<i', state_orig[flag])[0] == 0
        total_comparisons += 1

    # 10. Test Game_ShutdownGame
    for seed in range(30):
        rng = random.Random(seed + 800)
        state = {
            'g_gameMainState': rng.randint(-1000, 1000),
            'g_gameMenuState': rng.randint(-1000, 1000),
            'g_gameMenuSelected': rng.randint(-1000, 1000),
            'g_gameRaceLoadStage': rng.randint(-1000, 1000),
            'g_gameInRace': rng.randint(-1000, 1000),
            'g_gameFlag563BF4': rng.randint(-1000, 1000),
            'g_gameGraphicsInitErrorFlag': rng.randint(-1000, 1000),
        }
        ret_orig, calls_orig, state_orig = sess_orig.invoke('Game_ShutdownGame', (), state)
        ret_dll, calls_dll, state_dll = sess_dll.invoke('Game_ShutdownGame', (), state)

        assert ret_orig == ret_dll == 0, f"ShutdownGame ret mismatch: {ret_orig} vs {ret_dll}"
        assert calls_orig == calls_dll == [('Game_CleanupResources',)], f"ShutdownGame calls mismatch:\norig: {calls_orig}\ndll: {calls_dll}"
        for k in GLOBALS:
            assert state_orig[k] == state_dll[k], f"ShutdownGame state mismatch on {k}"
        total_comparisons += 1

    # 11. Test Game_InitState
    for seed in range(40):
        rng = random.Random(seed + 900)
        cd_present = rng.choice([0, 1])
        cfg = [rng.randint(-50000, 50000) for _ in range(4)]
        state = {
            'g_cdAudioMediaPresent': cd_present,
            'g_gameConfigValues': struct.pack('<4i', *cfg),
            'g_CameraShakeActive': rng.randint(-1000, 1000),
            'g_gameStateFlag5530DC': rng.randint(-1000, 1000),
            'g_gameMainState': rng.randint(-1000, 1000),
        }
        ret_orig, calls_orig, state_orig = sess_orig.invoke('Game_InitState', (), state)
        ret_dll, calls_dll, state_dll = sess_dll.invoke('Game_InitState', (), state)

        assert ret_orig == ret_dll == 1, f"InitState ret mismatch: {ret_orig} vs {ret_dll}"
        expected_calls = [('Audio_CheckCDMediaPresent',)]
        if cd_present == 1:
            expected_calls.append(('Audio_InitCDTracks',))
        assert calls_orig == calls_dll == expected_calls, f"InitState calls mismatch:\norig: {calls_orig}\ndll: {calls_dll}"
        for k in GLOBALS:
            assert state_orig[k] == state_dll[k], f"InitState state mismatch on {k}"
        assert struct.unpack('<i', state_orig['g_CameraShakeActive'])[0] == cfg[3]
        assert struct.unpack('<i', state_orig['g_gameStateFlag5530DC'])[0] == 1
        assert struct.unpack('<i', state_orig['g_gameMainState'])[0] == 1
        total_comparisons += 1

    print(f"PASS: {total_comparisons} differential trials passed with byte-for-byte state and event parity.")

    # Record runs for all 11 RVAs
    for name, (rva, _, _) in ROUTINES.items():
        for kind in ('compilation', 'emulation'):
            record_run(
                rva=rva,
                kind=kind,
                outcome='pass',
                inputs=TRACKING_INPUTS,
                artifact=DLL.relative_to(ROOT).as_posix(),
                cases=total_comparisons if kind == 'emulation' else 0,
                command='uv run python tools/verify_platform.py',
                details={
                    'symbol': name,
                    'trials': total_comparisons if kind == 'emulation' else 0,
                    'differential_oracle': 'authentic IGN_WIN.EXE in Unicorn emulation' if kind == 'emulation' else None,
                    'scope': 'C89 platform window procedure, application entry, and lifecycle compilation' if kind == 'compilation' else 'differential emulation',
                }
            )
    print("Recorded verification runs into database.")

if __name__ == '__main__':
    run_tests()
