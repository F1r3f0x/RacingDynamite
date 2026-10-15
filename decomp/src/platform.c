#include "platform.h"
#include "surface.h"
#include "geputget.h"
#include "mem.h"

/* Input subsystem entry points called without arguments in IGN_WIN.EXE */
int Input_InitNative();
int Input_ShutdownNative(void);

HINSTANCE volatile g_nativeWindowInstance = 0; /* VA 0x004C5398 */
HWND volatile g_nativeWindow = 0; /* VA 0x004C539C */
const char g_nativeWindowClassName[] = "Ignition"; /* VA 0x00493740 */
const char g_nativeWindowTitle[] = "Ignition"; /* VA 0x00493778 */
HCURSOR volatile g_nativeCursor = 0; /* VA 0x004C5360 */
RECT volatile g_nativeWindowRect = {0, 0, 0, 0}; /* VA 0x004C5350 */
volatile int32_t g_nativeIsAppActive = 0; /* VA 0x004C5384 */
volatile int32_t g_nativeInitialized = 0; /* VA 0x004C5390 */
volatile int32_t g_nativeIsChangingDisplayMode = 0; /* VA 0x0049372C */
volatile LARGE_INTEGER g_nativePerfFrequency = {{0, 0}}; /* VA 0x004C5348 */
volatile int32_t g_nativePerfFrequencyAvailable = 0; /* VA 0x00493720 */
volatile LARGE_INTEGER g_nativeStartCounter = {{0, 0}}; /* VA 0x004C5368 */
volatile LARGE_INTEGER g_nativeCurrentCounter = {{0, 0}}; /* VA 0x004C5378 */
volatile uint32_t g_nativeAccumulatedActiveTime = 0; /* VA 0x004937AC */
volatile uint32_t g_nativeLastActiveTime = 0; /* VA 0x004937B8 */
const void * volatile g_nativeUnknownPlatformPtr = 0; /* VA 0x004C53A0 */
volatile int32_t g_nativeStage = 0; /* VA 0x00493734 */
volatile int32_t g_nativeInitGuard = 0; /* VA 0x004937B0 */
volatile int32_t g_nativeShutdownGuard = 0; /* VA 0x004937B4 */
volatile uint32_t g_nativeLastFrameTime = 0; /* VA 0x0049373C */

volatile int32_t g_gameMainState = 0; /* VA 0x00639394 */
volatile int32_t g_gameMenuState = 0; /* VA 0x00563C3C */
volatile int32_t g_gameMenuSelected = 0; /* VA 0x00563D9C */
volatile int32_t g_gameRaceLoadStage = 0; /* VA 0x00553290 */
volatile int32_t g_gameInRace = 0; /* VA 0x00525E5C */
volatile int32_t g_gameFlag563BF4 = 0; /* VA 0x00563BF4 */
volatile int32_t g_gameGraphicsInitErrorFlag = 0; /* VA 0x0054F970 */
volatile int32_t g_cdAudioMediaPresent = 0; /* VA 0x0050EF4C */
volatile int32_t g_gameConfigValues[4] = {0, 0, 0, 0}; /* VA 0x0050F2CC */
volatile int32_t g_CameraShakeActive = 0; /* VA 0x004949E0 */
volatile int32_t g_gameStateFlag5530DC = 0; /* VA 0x005530DC */

/*
 * @original WinMain (IGN_WIN.EXE @ 0x004120A0, platform.c)
 * @fidelity EXACT
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    MSG msg;
    WNDCLASSA wc;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nShowCmd;

    g_nativeUnknownPlatformPtr = (const void *)0x00479558;
    g_nativeWindowInstance = hInstance;
    g_nativeCursor = LoadCursorA(NULL, MAKEINTRESOURCEA(32512));

    wc.style = CS_DBLCLKS; /* 8 */
    wc.lpfnWndProc = Game_WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = g_nativeWindowInstance;
    wc.hIcon = LoadIconA(NULL, MAKEINTRESOURCEA(32512));
    wc.hCursor = g_nativeCursor;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); /* 4 */
    wc.lpszMenuName = NULL;
    wc.lpszClassName = g_nativeWindowClassName;

    if (!RegisterClassA(&wc)) {
        return 0;
    }

    if (QueryPerformanceFrequency((LARGE_INTEGER *)&g_nativePerfFrequency)) {
        g_nativePerfFrequencyAvailable = 1;
        QueryPerformanceCounter((LARGE_INTEGER *)&g_nativeStartCounter);
    } else {
        g_nativePerfFrequencyAvailable = 0;
        timeBeginPeriod(1);
    }

    Mem_InitSystem();
    if (!Game_InitSubsystems()) {
        return 0;
    }

    for (;;) {
        if (g_nativeIsAppActive != 0) {
            if (PeekMessageA(&msg, NULL, 0, 0, PM_NOREMOVE)) {
                if (!GetMessageA(&msg, NULL, 0, 0)) {
                    return (int)msg.wParam;
                }
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            } else {
                if (!Game_ProcessFrame()) {
                    Game_PostClose();
                }
            }
        } else {
            if (!GetMessageA(&msg, NULL, 0, 0)) {
                return (int)msg.wParam;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
}

/*
 * @original Game_ProcessFrame (IGN_WIN.EXE @ 0x00412230, platform.c)
 * @fidelity EXACT
 */
int32_t Game_ProcessFrame(void) {
    int32_t stage = g_nativeStage;
    if (stage == 0) {
        if (g_nativeInitGuard == 0) {
            g_nativeInitGuard = 1;
            g_nativeLastFrameTime = Game_GetTime();
            Game_InitGame();
            g_nativeStage = 1;
            return 1;
        }
    } else if (stage == 1) {
        if (g_nativeShutdownGuard == 0) {
            g_nativeLastFrameTime = Game_GetTime();
            Game_UpdateFrame();
            return 1;
        }
    } else if (stage == 2) {
        if (g_nativeShutdownGuard == 0) {
            g_nativeLastFrameTime = Game_GetTime();
            Game_ShutdownGame();
            Mem_ShutdownSystem();
            if (g_nativePerfFrequencyAvailable == 0) {
                timeEndPeriod(1);
            }
            g_nativeShutdownGuard = 1;
            return 0;
        }
    }
    return 1;
}

/*
 * @original Game_WindowProc (IGN_WIN.EXE @ 0x004122D0, platform.c)
 * @fidelity EXACT
 */
LRESULT CALLBACK Game_WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        return 0;

    case WM_DESTROY:
        if (g_nativeIsChangingDisplayMode == 0) {
            Game_ShutdownSubsystems();
            PostQuitMessage(0);
        }
        return 0;

    case WM_MOVE:
    case WM_SIZE:
        if (g_nativeFullscreen != 0) {
            int cy = GetSystemMetrics(SM_CYSCREEN);
            int cx = GetSystemMetrics(SM_CXSCREEN);
            SetRect((LPRECT)&g_nativeWindowRect, 0, 0, cx, cy);
        } else {
            GetClientRect(hWnd, (LPRECT)&g_nativeWindowRect);
            ClientToScreen(hWnd, (LPPOINT)&g_nativeWindowRect.left);
            ClientToScreen(hWnd, (LPPOINT)&g_nativeWindowRect.right);
        }
        return DefWindowProcA(hWnd, uMsg, wParam, lParam);

    case WM_ACTIVATEAPP:
        g_nativeIsAppActive = (int32_t)wParam;
        return DefWindowProcA(hWnd, uMsg, wParam, lParam);

    case WM_SETCURSOR:
        if (g_nativeFullscreen != 0) {
            SetCursor(NULL);
        } else {
            SetCursor(g_nativeCursor);
        }
        return 1;

    case WM_KEYDOWN:
        return 0;

    case WM_SYSKEYUP:
        if (wParam == VK_RETURN && g_nativeInitialized == 1) {
            g_nativeIsChangingDisplayMode = 1;
            Input_ShutdownNative();
            Gfx_Shutdown();
            DestroyWindow(g_nativeWindow);
            g_nativeFullscreen = (g_nativeFullscreen == 1) ? 0 : 1;
            Gfx_Open();
            Input_InitNative();
            g_nativeIsChangingDisplayMode = 0;
        }
        return DefWindowProcA(hWnd, uMsg, wParam, lParam);

    default:
        return DefWindowProcA(hWnd, uMsg, wParam, lParam);
    }
}

/*
 * @original Game_GetTime (IGN_WIN.EXE @ 0x00412460, platform.c)
 * @fidelity EXACT
 */
uint32_t Game_GetTime(void) {
    uint32_t now;
    if (g_nativePerfFrequencyAvailable == 1) {
        LONGLONG elapsed;
        QueryPerformanceCounter((LARGE_INTEGER *)&g_nativeCurrentCounter);
        elapsed = g_nativeCurrentCounter.QuadPart - g_nativeStartCounter.QuadPart;
        now = (uint32_t)((elapsed * 1000) / g_nativePerfFrequency.QuadPart);
    } else {
        now = timeGetTime();
    }
    if (g_nativeLastActiveTime == 0) {
        g_nativeLastActiveTime = now;
    }
    if (g_nativeIsAppActive != 0) {
        g_nativeAccumulatedActiveTime += (now - g_nativeLastActiveTime);
    }
    g_nativeLastActiveTime = now;
    return g_nativeAccumulatedActiveTime;
}

/*
 * @original Game_InitSubsystems (IGN_WIN.EXE @ 0x00412500, platform.c)
 * @fidelity EXACT
 */
int32_t Game_InitSubsystems(void) {
    Gfx_SelectBackend(0);
    if (!Gfx_Open()) {
        return 0;
    }
    if (!Input_InitNative()) {
        return 0;
    }
    g_nativeInitialized = 1;
    return 1;
}

/*
 * @original Game_PostClose (IGN_WIN.EXE @ 0x00412530, platform.c)
 * @fidelity EXACT
 */
void Game_PostClose(void) {
    PostMessageA(g_nativeWindow, WM_CLOSE, 0, 0);
}

/*
 * @original Game_ShutdownSubsystems (IGN_WIN.EXE @ 0x00412550, platform.c)
 * @fidelity EXACT
 */
void Game_ShutdownSubsystems(void) {
    Gfx_Shutdown();
    Input_ShutdownNative();
}

/*
 * @original Game_SetPalette (IGN_WIN.EXE @ 0x00412560, platform.c)
 * @fidelity EXACT
 */
void Game_SetPalette(const void *palette_container) {
    const unsigned char *data = *(const unsigned char * const *)palette_container;
    Gfx_SetPaletteNative(data + 0x10);
}

/*
 * @original Game_InitGame (IGN_WIN.EXE @ 0x00417270, platform.c)
 * @fidelity EXACT
 */
int32_t Game_InitGame(void) {
    g_gameMainState = 2;
    g_gameMenuState = 0;
    g_gameMenuSelected = 0;
    g_gameRaceLoadStage = 0;
    g_gameInRace = 0;
    g_gameFlag563BF4 = 0;
    g_gameGraphicsInitErrorFlag = 0;
    Game_InitResources();
    Game_InitState();
    return 1;
}

/*
 * @original Game_ShutdownGame (IGN_WIN.EXE @ 0x00417E90, platform.c)
 * @fidelity EXACT
 */
int32_t Game_ShutdownGame(void) {
    Game_CleanupResources();
    return 0;
}

/*
 * @original Game_InitState (IGN_WIN.EXE @ 0x004184C0, platform.c)
 * @fidelity EXACT
 */
int32_t Game_InitState(void) {
    int32_t ecx;
    Audio_CheckCDMediaPresent();
    if (g_cdAudioMediaPresent == 1) {
        Audio_InitCDTracks();
    }
    for (ecx = 8; ecx < 16; ecx += 4) {
        g_CameraShakeActive = *(volatile int32_t *)((char *)g_gameConfigValues + ecx);
    }
    g_gameStateFlag5530DC = 1;
    g_gameMainState = 1;
    return 1;
}
