# Windows application entry and frame dispatcher

Authentic input is `Ignition/Ignition/IGN_WIN.EXE`, 915,968 bytes, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`,
PE32 i386, preferred base VA `0x00400000`. Original assets are untouched.

## Routine Inventory

| Symbol Name | RVA / VA | Size | ABI | SHA-256 |
| --- | --- | --- | --- | --- |
| `WinMain` | `0x000120A0` / `0x004120A0` | 390 | `__stdcall (HINSTANCE, HINSTANCE, LPSTR, int) -> int` | `3460e40b77123335ede23ae49d122053a852b6398f037253ce07ca5520b2a88c` |
| `Game_ProcessFrame` | `0x00012230` / `0x00412230` | 154 | `__cdecl (void) -> int32_t` | `4c50ba4401d440b7c1caa09ce8d55047f8d9eb38b235b81ebe5edbfcfae8153f` |

## State Ownership and Data Layout

These state variables belong to the direct `platform` module (`decomp/src/platform.c`):

| Variable | VA / RVA | Size | Type | Description |
| --- | --- | --- | --- | --- |
| `g_nativeUnknownPlatformPtr` | `0x004C53A0` / `0xC53A0` | 4 | `const void *` | Pointer set by `WinMain` to `.rdata` address `0x00479558` |
| `g_nativeStage` | `0x00493734` / `0x93734` | 4 | `int32_t` | Top-level lifecycle stage: 0 = uninitialized, 1 = running, 2 = requested shutdown |
| `g_nativeInitGuard` | `0x004937B0` / `0x937B0` | 4 | `int32_t` | One-shot guard for stage 0 initialization (`Game_InitGame`) |
| `g_nativeShutdownGuard` | `0x004937B4` / `0x937B4` | 4 | `int32_t` | One-shot guard for stage 2 shutdown (`Game_ShutdownGame`) |
| `g_nativeLastFrameTime` | `0x0049373C` / `0x9373C` | 4 | `uint32_t` | Timestamp updated each frame via `Game_GetTime()` |

## Detailed Behavioral Analysis

### 1. WinMain (0x004120A0)
Authentic standard Win32 application entry point called from CRT startup with 4 arguments on stack, returning with `RET 16`:
1. Stores `.rdata` pointer `0x00479558` into `g_nativeUnknownPlatformPtr` (`0x004C53A0`).
2. Stores `hInstance` into `g_nativeWindowInstance` (`0x004C5398`).
3. Loads arrow cursor via `LoadCursorA(NULL, MAKEINTRESOURCEA(32512))` into `g_nativeCursor` (`0x004C5360`).
4. Constructs `WNDCLASSA` on stack:
   - `style = CS_DBLCLKS` (8)
   - `lpfnWndProc = Game_WindowProc` (`0x004122D0`)
   - `cbClsExtra = 0`, `cbWndExtra = 0`
   - `hInstance = g_nativeWindowInstance`
   - `hIcon = LoadIconA(NULL, MAKEINTRESOURCEA(32512))`
   - `hCursor = g_nativeCursor`
   - `hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH)` (4)
   - `lpszMenuName = NULL`
   - `lpszClassName = g_nativeWindowClassName` (`"Ignition"`)
5. Calls `RegisterClassA(&wc)`. If registration fails (returns 0), exits returning 0.
6. Calls `QueryPerformanceFrequency(&g_nativePerfFrequency)`. If successful:
   - Sets `g_nativePerfFrequencyAvailable = 1`.
   - Calls `QueryPerformanceCounter(&g_nativeStartCounter)`.
   Otherwise:
   - Sets `g_nativePerfFrequencyAvailable = 0`.
   - Calls `timeBeginPeriod(1)`.
7. Calls `Mem_InitSystem()` (`0x0045B170`).
8. Calls `Game_InitSubsystems()` (`0x00412500`). If subsystems fail to initialize, exits returning 0.
9. Enters the standard game message pump:
   - If `g_nativeIsAppActive != 0`:
     - Checks `PeekMessageA(&msg, NULL, 0, 0, PM_NOREMOVE)`.
     - If a message exists: calls `GetMessageA(&msg, NULL, 0, 0)`. If `GetMessageA` returns 0 (WM_QUIT), exits returning `(int)msg.wParam`. Otherwise translates and dispatches.
     - If no message exists: calls `Game_ProcessFrame()`. If it returns 0, calls `Game_PostClose()`.
   - If `g_nativeIsAppActive == 0`:
     - Blocks in `GetMessageA(&msg, NULL, 0, 0)`. If `GetMessageA` returns 0 (WM_QUIT), exits returning `(int)msg.wParam`. Otherwise translates and dispatches.

### 2. Game_ProcessFrame (0x00412230)
Frame dispatcher called during active idle loops:
1. Reads `stage = g_nativeStage`:
   - If `stage == 0`:
     - Checks `g_nativeInitGuard == 0`. If true, sets guard to 1, calls `Game_GetTime()` to update `g_nativeLastFrameTime`, calls `Game_InitGame()` (`0x00417270`), transitions `g_nativeStage = 1`, and returns 1.
   - If `stage == 1`:
     - Checks `g_nativeShutdownGuard == 0`. If true, calls `Game_GetTime()` to update `g_nativeLastFrameTime`, calls `Game_UpdateFrame()` (`0x004172B0`), and returns 1.
   - If `stage == 2`:
     - Checks `g_nativeShutdownGuard == 0`. If true, calls `Game_GetTime()` to update `g_nativeLastFrameTime`, calls `Game_ShutdownGame()` (`0x00417E90`), calls `Mem_ShutdownSystem()` (`0x0045B1A0`), conditionally calls `timeEndPeriod(1)` if `g_nativePerfFrequencyAvailable == 0`, sets `g_nativeShutdownGuard = 1`, and returns 0 (signaling `WinMain` to post close).
2. For all other conditions (including guards already active or unknown stage), returns 1.
