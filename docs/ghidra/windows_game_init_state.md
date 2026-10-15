# Windows game state initialization routine

Authentic input is `Ignition/Ignition/IGN_WIN.EXE`, 915,968 bytes, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`,
PE32 i386, preferred base VA `0x00400000`. Original assets are untouched.

## Routine Inventory

| Symbol Name | RVA / VA | Size | ABI | SHA-256 |
| --- | --- | --- | --- | --- |
| `Game_InitState` | `0x000184C0` / `0x004184C0` | 60 | `__cdecl (void) -> int32_t` | `e1dc62b410b9864791e22d06c66fcc1bfed21de5c207ef533f2ab615c09b517f` |

## FPO Record

| Symbol Name | RVA | Size | Locals | Params | Prolog | Regs Saved | Flags |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `Game_InitState` | `0x000184C0` | 60 | 0 | 0 | 0 | 0 | `0x0000` (FPO frame) |

## State Ownership and Data Layout

These state variables belong to the direct `platform` module (`decomp/src/platform.c`):

| Variable | VA / RVA | Size | Type | Initial Value | Description |
| --- | --- | --- | --- | --- | --- |
| `g_cdAudioMediaPresent` | `0x0050EF4C` / `0x10EF4C` | 4 | `int32_t` | `0` | CD-ROM media present flag (`1` = audio CD available) |
| `g_gameConfigValues` | `0x0050F2CC` / `0x10F2CC` | 16 | `int32_t[4]` | `{0, 0, 0, 0}` | Core configuration and display options table |
| `g_CameraShakeActive` | `0x004949E0` / `0x0949E0` | 4 | `int32_t` | `0` | Camera elevation clamp and shake toggle flag |
| `g_gameStateFlag5530DC` | `0x005530DC` / `0x1530DC` | 4 | `int32_t` | `0` | State initialization flag |
| `g_gameMainState` | `0x00639394` / `0x239394` | 4 | `int32_t` | `0` | Intro sequence / game mode: set to `1` (menus initialized) |

All five variables reside in the `.data` section and default to `0`.

## Downstream Routine Dependencies

| Target Function | Target VA / RVA | Target ABI | Description |
| --- | --- | --- | --- |
| `Audio_CheckCDMediaPresent` | `0x00457B50` / `0x00057B50` | `__cdecl (void) -> int32_t` | Queries MCI device for audio CD media presence |
| `Audio_InitCDTracks` | `0x00457DA0` / `0x00057DA0` | `__cdecl (void) -> int32_t` | Queries MCI device for CD track count |

## Detailed Behavioral Analysis

Authentic game state initialization called by `Game_InitGame`:
```x86asm
0x004184c0: call 0x457b50                   ; Audio_CheckCDMediaPresent()
0x004184c5: mov eax, dword ptr [0x50ef4c]   ; eax = g_cdAudioMediaPresent
0x004184ca: cmp eax, 1                      ; is CD media present?
0x004184cd: jne 0x4184d4
0x004184cf: call 0x457da0                   ; Audio_InitCDTracks()
0x004184d4: mov ecx, 8
0x004184d9: mov eax, dword ptr [ecx + 0x50f2cc] ; eax = g_gameConfigValues[ecx / 4]
0x004184df: add ecx, 4
0x004184e2: mov dword ptr [0x4949e0], eax   ; g_CameraShakeActive = eax
0x004184e7: cmp ecx, 0x10                   ; loop until ecx == 16 (indices 2 and 3)
0x004184ea: jl 0x4184d9
0x004184ec: mov eax, 1                      ; eax = 1
0x004184f1: mov dword ptr [0x5530dc], eax   ; g_gameStateFlag5530DC = 1
0x004184f6: mov dword ptr [0x639394], eax   ; g_gameMainState = 1
0x004184fb: ret                             ; return 1
```

Execution steps:
1. Calls `Audio_CheckCDMediaPresent()` to probe CD media status via MCI.
2. Checks `g_cdAudioMediaPresent`. If equal to 1, calls `Audio_InitCDTracks()` to discover available audio tracks.
3. Loops over configuration offsets 8 and 12 (`g_gameConfigValues[2]` and `g_gameConfigValues[3]`), storing into `g_CameraShakeActive`.
4. Sets `g_gameStateFlag5530DC = 1`.
5. Sets `g_gameMainState = 1` (advancing intro/menu state from publisher logos to menu initialization).
6. Returns `1`.
