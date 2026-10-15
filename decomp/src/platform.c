#include "platform.h"
#include "surface.h"
#include "geputget.h"

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
