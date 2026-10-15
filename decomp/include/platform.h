#ifndef IGNITION_PLATFORM_H
#define IGNITION_PLATFORM_H

#include <windows.h>
#include <stdint.h>

extern HINSTANCE volatile g_nativeWindowInstance; /* VA 0x004C5398 */
extern HWND volatile g_nativeWindow; /* VA 0x004C539C */
extern const char g_nativeWindowClassName[]; /* VA 0x00493740 */
extern const char g_nativeWindowTitle[]; /* VA 0x00493778 */
extern HCURSOR volatile g_nativeCursor; /* VA 0x004C5360 */
extern RECT volatile g_nativeWindowRect; /* VA 0x004C5350 */
extern volatile int32_t g_nativeIsAppActive; /* VA 0x004C5384 */
extern volatile int32_t g_nativeInitialized; /* VA 0x004C5390 */
extern volatile int32_t g_nativeIsChangingDisplayMode; /* VA 0x0049372C */
extern volatile LARGE_INTEGER g_nativePerfFrequency; /* VA 0x004C5348 */
extern volatile int32_t g_nativePerfFrequencyAvailable; /* VA 0x00493720 */
extern volatile LARGE_INTEGER g_nativeStartCounter; /* VA 0x004C5368 */
extern volatile LARGE_INTEGER g_nativeCurrentCounter; /* VA 0x004C5378 */
extern volatile uint32_t g_nativeAccumulatedActiveTime; /* VA 0x004937AC */
extern volatile uint32_t g_nativeLastActiveTime; /* VA 0x004937B8 */

LRESULT CALLBACK Game_WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
uint32_t Game_GetTime(void);
int32_t Game_InitSubsystems(void);
void Game_PostClose(void);
void Game_ShutdownSubsystems(void);
void Game_SetPalette(const void *palette_container);

#endif /* IGNITION_PLATFORM_H */
