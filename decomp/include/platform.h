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
extern const void * volatile g_nativeUnknownPlatformPtr; /* VA 0x004C53A0 */
extern volatile int32_t g_nativeStage; /* VA 0x00493734 */
extern volatile int32_t g_nativeInitGuard; /* VA 0x004937B0 */
extern volatile int32_t g_nativeShutdownGuard; /* VA 0x004937B4 */
extern volatile uint32_t g_nativeLastFrameTime; /* VA 0x0049373C */

extern volatile int32_t g_gameMainState; /* VA 0x00639394 */
extern volatile int32_t g_gameMenuState; /* VA 0x00563C3C */
extern volatile int32_t g_gameMenuSelected; /* VA 0x00563D9C */
extern volatile int32_t g_gameRaceLoadStage; /* VA 0x00553290 */
extern volatile int32_t g_gameInRace; /* VA 0x00525E5C */
extern volatile int32_t g_gameFlag563BF4; /* VA 0x00563BF4 */
extern volatile int32_t g_gameGraphicsInitErrorFlag; /* VA 0x0054F970 */
extern volatile int32_t g_cdAudioMediaPresent; /* VA 0x0050EF4C */
extern volatile int32_t g_gameConfigValues[4]; /* VA 0x0050F2CC */
extern volatile int32_t g_CameraShakeActive; /* VA 0x004949E0 */
extern volatile int32_t g_gameStateFlag5530DC; /* VA 0x005530DC */
extern volatile int32_t g_gameSysGfxHandles[4]; /* VA 0x0054F960 */
extern volatile int32_t g_gameMouseCursorHandle; /* VA 0x00552D84 */
extern volatile int32_t g_gameTotalSeconds; /* VA 0x0049498C */
extern volatile int32_t g_gameTotalFrames; /* VA 0x00494988 */
extern volatile int32_t g_gameMainPoolId; /* VA 0x005530E8 */

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);
int32_t Game_ProcessFrame(void);
LRESULT CALLBACK Game_WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
uint32_t Game_GetTime(void);
int32_t Game_InitSubsystems(void);
void Game_PostClose(void);
void Game_ShutdownSubsystems(void);
void Game_SetPalette(const void *palette_container);

/* Game lifecycle entry points and downstream dependencies */
int32_t Game_InitGame(void);
int32_t Game_UpdateFrame(void);
int32_t Game_ShutdownGame(void);
int32_t Game_InitResources(void);
int32_t Game_InitState(void);
int32_t Game_CleanupResources(void);

/* Audio subsystem entry points called by Game_InitState */
int32_t Audio_CheckCDMediaPresent(void);
int32_t Audio_InitCDTracks(void);

/* Subsystem cleanup routines called by Game_CleanupResources */
int32_t Audio_StopCD(void);
int32_t Audio_CloseCD(void);
int32_t Audio_ShutdownSound(void);
void Menu_FreeBackground(void);
int32_t Log_Printf(const char *format, ...);
int32_t Log_Shutdown(void);

#endif /* IGNITION_PLATFORM_H */
