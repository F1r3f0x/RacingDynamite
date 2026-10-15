# Windows platform and lifecycle helpers

Authentic input is `Ignition/Ignition/IGN_WIN.EXE`, 915,968 bytes, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`,
PE32 i386, preferred base VA `0x00400000`. Original assets are untouched.

## Routine Inventory

| Symbol Name | RVA / VA | Size | ABI | SHA-256 |
| --- | --- | --- | --- | --- |
| `Game_WindowProc` | `0x000122D0` / `0x004122D0` | 400 | stdcall | `461e5b4afef4dd9642c7ce031556749db188c41ea06480eab871723b337452e0` |
| `Game_GetTime` | `0x00012460` / `0x00412460` | 152 | cdecl | `bd900f4be8ea23b0f2e49bcdb780f817e1896e172c75c66ea448f30985fa909b` |
| `Game_InitSubsystems` | `0x00012500` / `0x00412500` | 47 | cdecl | `e96334c54403bffb6aadf238c02d5b1078ec74d724cd419db84d743d09ef5218` |
| `Game_PostClose` | `0x00012530` / `0x00412530` | 21 | cdecl | `a4273cc806e5f6887cc388279cff7dd3c3d31bda16354480f1552d4b351d5939` |
| `Game_ShutdownSubsystems` | `0x00012550` / `0x00412550` | 10 | cdecl | `9d322da9798d57756f64a6ddfe80ceb7eeab3e0cf8d01337a3b6a3a9f1d4dfc2` |
| `Game_SetPalette` | `0x00012560` / `0x00412560` | 19 | cdecl | `20949484266a1eb89c69091e72f67c67ae9b6615b115a5e47463892fdc25dbf0` |

## State Ownership and Data Layout

All state variables belong to the direct `platform` module (`decomp/src/platform.c`):

| Variable | VA / RVA | Size | Type | Description |
| --- | --- | --- | --- | --- |
| `g_nativeWindowInstance` | `0x004C5398` / `0xC5398` | 4 | `HINSTANCE` | Application instance handle set by `WinMain` |
| `g_nativeWindow` | `0x004C539C` / `0xC539C` | 4 | `HWND` | Game window handle published by `Gfx_SurfaceOpenNative` |
| `g_nativeWindowClassName` | `0x00493740` / `0x93740` | 9 | `const char[]` | Window class name (`"Ignition\0"`) |
| `g_nativeWindowTitle` | `0x00493778` / `0x93778` | 9 | `const char[]` | Window title (`"Ignition\0"`) |
| `g_nativeCursor` | `0x004C5360` / `0xC5360` | 4 | `HCURSOR` | Standard arrow cursor handle from `LoadCursorA(NULL, IDC_ARROW)` |
| `g_nativeWindowRect` | `0x004C5350` / `0xC5350` | 16 | `RECT` | Active window/client bounding rectangle |
| `g_nativeIsAppActive` | `0x004C5384` / `0xC5384` | 4 | `int` | Application activation flag from `WM_ACTIVATEAPP` |
| `g_nativeInitialized` | `0x004C5390` / `0xC5390` | 4 | `int` | Initialization flag set by `Game_InitSubsystems` |
| `g_nativeIsChangingDisplayMode` | `0x0049372C` / `0x9372C` | 4 | `int` | Guard flag suppressing quit on destroy during Alt+Enter |
| `g_nativePerfFrequency` | `0x004C5348` / `0xC5348` | 8 | `LARGE_INTEGER` | Performance counter frequency from `QueryPerformanceFrequency` |
| `g_nativePerfFrequencyAvailable` | `0x00493720` / `0x93720` | 4 | `int` | Boolean: 1 if high-resolution counter available, 0 if using `timeGetTime` |
| `g_nativeStartCounter` | `0x004C5368` / `0xC5368` | 8 | `LARGE_INTEGER` | Initial counter value recorded at startup |
| `g_nativeAccumulatedActiveTime` | `0x004937AC` / `0x937AC` | 4 | `unsigned int` | Accumulated active running time in milliseconds |
| `g_nativeLastActiveTime` | `0x004937B8` / `0x937B8` | 4 | `unsigned int` | Last recorded sample timestamp |

## Detailed Behavioral Analysis

### 1. Game_WindowProc (0x004122D0)
Stdcall window procedure registered with the `"Ignition"` window class:
- **`WM_CREATE` (`1`)**: Returns 0 immediately.
- **`WM_DESTROY` (`2`)**: If `g_nativeIsChangingDisplayMode == 0`, calls `Game_ShutdownSubsystems()` (`0x00412550`) and `PostQuitMessage(0)`; returns 0.
- **`WM_MOVE` (`3`) and `WM_SIZE` (`5`)**:
  - Fullscreen (`g_nativeFullscreen != 0`): queries `GetSystemMetrics(SM_CXSCREEN)` and `GetSystemMetrics(SM_CYSCREEN)`, calls `SetRect(&g_nativeWindowRect, 0, 0, width, height)`.
  - Windowed (`g_nativeFullscreen == 0`): calls `GetClientRect(hWnd, &g_nativeWindowRect)`, followed by two `ClientToScreen` calls for top-left and bottom-right points.
  - Forwarded to `DefWindowProcA`.
- **`WM_ACTIVATEAPP` (`0x1C`)**: Stores `wParam` into `g_nativeIsAppActive` (`0x004C5384`), then forwards to `DefWindowProcA`.
- **`WM_SETCURSOR` (`0x20`)**: If fullscreen, calls `SetCursor(NULL)`; if windowed, calls `SetCursor(g_nativeCursor)`. Returns 1.
- **`WM_KEYDOWN` (`0x100`)**: Absorbed; returns 0 without calling `DefWindowProcA`.
- **`WM_SYSKEYUP` (`0x105`)**: Checks `wParam == VK_RETURN (13)` and `g_nativeInitialized == 1`. If matched, toggles fullscreen display:
  - Sets `g_nativeIsChangingDisplayMode = 1`.
  - Calls `Input_ShutdownNative()` (`0x00455E20`).
  - Calls `Gfx_Shutdown()` (`0x00456BF0`).
  - Calls `DestroyWindow(hWnd)` (`0x004C539C`).
  - Toggles `g_nativeFullscreen = g_nativeFullscreen ^ 1`.
  - Calls `Gfx_Open()` (`0x00456BC0`).
  - Calls `Input_InitNative()` (`0x00455AC0`).
  - Clears `g_nativeIsChangingDisplayMode = 0`.
  - Forwards to `DefWindowProcA`.
- **Default**: All other messages forward directly to `DefWindowProcA(hWnd, uMsg, wParam, lParam)`.

### 2. Game_GetTime (0x00412460)
High-resolution active game timer:
- If `g_nativePerfFrequencyAvailable == 1`:
  - Queries `QueryPerformanceCounter(&now)`.
  - Computes `(now - g_nativeStartCounter) * 1000 / g_nativePerfFrequency`.
- If `g_nativePerfFrequencyAvailable == 0`:
  - Calls `timeGetTime()`.
- Updates `g_nativeAccumulatedActiveTime`:
  - If `g_nativeLastActiveTime == 0`, initializes it to the current time.
  - If `g_nativeIsAppActive != 0`, accumulates elapsed time: `g_nativeAccumulatedActiveTime += (now - g_nativeLastActiveTime)`.
  - Updates `g_nativeLastActiveTime = now`.
- Returns `g_nativeAccumulatedActiveTime`.

### 3. Game_InitSubsystems (0x00412500)
- Calls `Gfx_SelectBackend(0)` (`0x00456AF0`).
- Calls `Gfx_Open()` (`0x00456BC0`); returns 0 on failure.
- Calls `Input_InitNative()` (`0x00455AC0`); returns 0 on failure.
- Sets `g_nativeInitialized = 1`.
- Returns 1 on success.

### 4. Game_PostClose (0x00412530)
- Calls `PostMessageA(g_nativeWindow, WM_CLOSE, 0, 0)`.

### 5. Game_ShutdownSubsystems (0x00412550)
- Calls `Gfx_Shutdown()` (`0x00456BF0`).
- Tail-calls `Input_ShutdownNative()` (`0x00455E20`).

### 6. Game_SetPalette (0x00412560)
- Takes `const void *palette_container`.
- Reads `const unsigned char *data = *(const unsigned char **)palette_container`.
- Passes `data + 0x10` to `Gfx_SetPaletteNative(data + 0x10)` (`0x00456C40`).
