# Windows game initialization and shutdown lifecycle routines

Authentic input is `Ignition/Ignition/IGN_WIN.EXE`, 915,968 bytes, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`,
PE32 i386, preferred base VA `0x00400000`. Original assets are untouched.

## Routine Inventory

| Symbol Name | RVA / VA | Size | ABI | SHA-256 |
| --- | --- | --- | --- | --- |
| `Game_InitGame` | `0x00017270` / `0x00417270` | 58 | `__cdecl (void) -> int32_t` | `ca67f09b7bb19c2a7e24331ea64f2e29e8d81019fc94ef8e71f3831d27a3348a` |
| `Game_ShutdownGame` | `0x00017E90` / `0x00417E90` | 8 | `__cdecl (void) -> int32_t` | `2620d36c50712bde0380c5fafff9f1a14cd0efed596e6a107823739cbb966aa2` |

## FPO Records

| Symbol Name | RVA | Size | Locals | Params | Prolog | Regs Saved | Flags |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `Game_InitGame` | `0x00017270` | 58 | 0 | 0 | 0 | 0 | `0x0000` (FPO frame) |
| `Game_ShutdownGame` | `0x00017E90` | 8 | 0 | 0 | 0 | 0 | `0x0000` (FPO frame) |

## State Ownership and Data Layout

These state variables belong to the direct `platform` module (`decomp/src/platform.c`):

| Variable | VA / RVA | Size | Type | Initial Value | Description |
| --- | --- | --- | --- | --- | --- |
| `g_gameMainState` | `0x00639394` / `0x239394` | 4 | `int32_t` | `0` | Intro sequence / game mode: `2` = publisher logos, `1` = init menus, `0` = running |
| `g_gameMenuState` | `0x00563C3C` / `0x163C3C` | 4 | `int32_t` | `0` | Interactive menu active flag |
| `g_gameMenuSelected` | `0x00563D9C` / `0x163D9C` | 4 | `int32_t` | `0` | Menu selection processed flag |
| `g_gameRaceLoadStage` | `0x00553290` / `0x153290` | 4 | `int32_t` | `0` | Track and race asset load stage (1, 2, 3, 0) |
| `g_gameInRace` | `0x00525E5C` / `0x125E5C` | 4 | `int32_t` | `0` | Active race simulation and driving loop flag |
| `g_gameFlag563BF4` | `0x00563BF4` / `0x163BF4` | 4 | `int32_t` | `0` | Race completion / return-to-menu transition flag |
| `g_gameGraphicsInitErrorFlag` | `0x0054F970` / `0x14F970` | 4 | `int32_t` | `0` | Graphics initialization error indicator |

All seven variables reside in the uninitialized (BSS) portion of `.data` and default to `0`.

## Downstream Routine Dependencies

| Target Function | Target VA / RVA | Target ABI | Description |
| --- | --- | --- | --- |
| `Game_InitResources` | `0x00417EA0` / `0x00017EA0` | `__cdecl (void) -> int32_t` | Core resource initialization routine |
| `Game_InitState` | `0x004184C0` / `0x000184C0` | `__cdecl (void) -> int32_t` | Core game state and vehicle setup routine |
| `Game_CleanupResources` | `0x00420B70` / `0x00020B70` | `__cdecl (void) -> int32_t` | Game shutdown cleanup routine |

## Detailed Behavioral Analysis

### 1. Game_InitGame (0x00417270)
Authentic game initialization called once by `Game_ProcessFrame` when `g_nativeStage == 0`:
```x86asm
0x00417270: mov dword ptr [0x00639394], 2      ; g_gameMainState = 2
0x0041727a: xor eax, eax                       ; eax = 0
0x0041727c: mov dword ptr [0x00563c3c], eax    ; g_gameMenuState = 0
0x00417281: mov dword ptr [0x00563d9c], eax    ; g_gameMenuSelected = 0
0x00417286: mov dword ptr [0x00553290], eax    ; g_gameRaceLoadStage = 0
0x0041728b: mov dword ptr [0x00525e5c], eax    ; g_gameInRace = 0
0x00417290: mov dword ptr [0x00563bf4], eax    ; g_gameFlag563BF4 = 0
0x00417295: mov dword ptr [0x0054f970], eax    ; g_gameGraphicsInitErrorFlag = 0
0x0041729a: call 0x00417ea0                    ; Game_InitResources()
0x0041729f: call 0x004184c0                    ; Game_InitState()
0x004172a4: mov eax, 1                         ; return 1
0x004172a9: ret
```

Execution steps:
1. Sets `g_gameMainState = 2` (initiates publisher intro logos sequence).
2. Clears 6 game state flags to 0 (`g_gameMenuState`, `g_gameMenuSelected`, `g_gameRaceLoadStage`, `g_gameInRace`, `g_gameFlag563BF4`, `g_gameGraphicsInitErrorFlag`).
3. Calls `Game_InitResources()` (`0x00417EA0`).
4. Calls `Game_InitState()` (`0x004184C0`).
5. Returns `1`.

### 2. Game_ShutdownGame (0x00417E90)
Authentic game shutdown routine called once by `Game_ProcessFrame` when `g_nativeStage == 2`:
```x86asm
0x00417e90: call 0x00420b70                    ; Game_CleanupResources()
0x00417e95: xor eax, eax                       ; eax = 0
0x00417e97: ret                                ; return 0
```

Execution steps:
1. Calls `Game_CleanupResources()` (`0x00420B70`).
2. Clears `eax` to 0.
3. Returns `0`.
