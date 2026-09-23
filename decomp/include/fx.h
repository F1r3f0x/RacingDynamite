/*
 * fx.h - Special Effects, Particle Systems, HUD & Dynamic Object Rendering
 * Target: MAINDOS.EXE / IGN_WIN.EXE (Watcom C/C++ 10.6, 32-bit flat protected mode)
 */

#ifndef FX_H
#define FX_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"

#pragma pack(push, 4)

/* Vehicle Physics & Gameplay State (0x484c = 18,508 bytes per vehicle) */
typedef struct VehicleState {
    uint32_t flags;            /* 0x0000 */
    uint32_t active;           /* 0x0004 */
    double pos_x;              /* 0x0008 */
    double pos_y;              /* 0x0010 */
    double pos_z;              /* 0x0018 */
    double vel_x;              /* 0x0020 */
    double vel_y;              /* 0x0028 */
    double vel_z;              /* 0x0030 */
    uint8_t gap_38[0x58 - 0x38];
    double angle_yaw;          /* 0x0058 */
    double angle_pitch;        /* 0x0060 */
    uint8_t gap_68[0x288 - 0x68];
    double target_rpm;         /* 0x0288 */
    double engine_rpm;         /* 0x0290 */
    uint8_t gap_298[0x354 - 0x298];
    int crash_flag1;           /* 0x0354 */
    int crash_flag2;           /* 0x0358 */
    int crash_flag3;           /* 0x035c */
    uint8_t gap_360[0x39c - 0x360];
    int car_model_id;          /* 0x039c */
    int race_rank;             /* 0x03a0 */
    double lap_time;           /* 0x03a4 */
    uint8_t gap_3ac[0x528 - 0x3ac];
    int turbo_active;          /* 0x0528 */
    uint8_t gap_52c[0x99c - 0x52c];
    int voice_engine;          /* 0x099c */
    int voice_skid;            /* 0x09a0 */
    uint8_t gap_9a4[0x484c - 0x9a4];
} VehicleState;

/* Vehicle Configuration / Metadata Entry (0xc8 = 200 bytes per vehicle) */
typedef struct VehicleConfig {
    uint8_t pad_00[0x38];
    int cam_param1;            /* 0x38 */
    int cam_param2;            /* 0x3c */
    uint8_t pad_40[0x58 - 0x40];
    int lookat_offset_x;       /* 0x58 */
    int lookat_offset_y;       /* 0x5c */
    uint8_t pad_60[0xc8 - 0x60];
} VehicleConfig;

/* HUD Floating Message Record (0x78 = 120 bytes) */
typedef struct HUDMessage {
    int active;                /* 0x00 */
    char text[100];            /* 0x04 */
    int x;                     /* 0x68 */
    int y;                     /* 0x6c */
    int timer;                 /* 0x70 */
    int duration;              /* 0x74 */
} HUDMessage;

/* Audio Playback Request Parameters (0x20 = 32 bytes) */
typedef struct AudioSoundParams {
    int sound_id;              /* 0x00 */
    int pan;                   /* 0x04 */
    int volume;                /* 0x08 */
    int pitch;                 /* 0x0c */
    int priority;              /* 0x10 */
    int sample_rate;           /* 0x14 */
    int loop;                  /* 0x18 */
    int flags;                 /* 0x1c */
} AudioSoundParams;

#pragma pack(pop)

/* --- Authentic Global Variables for FX Subsystem --- */
extern VehicleState g_Vehicles[];
extern VehicleConfig g_VehicleConfigs[];
extern int g_ActiveVehicleIndex;
extern int g_NumRacers;
extern int g_DifficultyLevel;
extern void *g_pSpeedoGaugeSprite;
extern void *g_pSpeedoNeedleSprite;
extern HUDMessage g_HudFloatingMessages[];
extern int g_FontId_Large;
extern int g_FontId_Medium;
extern int g_FontId_Small;
extern int g_FontId_Menu;
extern int g_ActiveFontColor;
extern int g_FontAlignMode;
extern int g_PlayerCarChoice;
extern int g_PlayerCarModel;
extern uint8_t *g_PlayerHUDState;
extern int g_ScreenHeightAlt;
extern int g_VirtualFramebuffer;
extern int g_RenderTargetSurface;
extern int g_MenuCursorPos;
extern int g_IsDemoMode;
extern int g_IsSplitScreen;
extern int g_MasterTickCount;
extern uint8_t *g_SpeedoConfig;
extern uint8_t *g_SpeedoPosition;
extern int g_WeatherType;
extern uint8_t *g_LisaDrawQueueHead;
extern uint8_t *g_LisaFramebufferPtr1;
extern uint8_t *g_LisaFramebufferPtr2;
extern uint8_t *g_LisaShadingTablePtr;
extern uint8_t *g_ActiveTrackPalette;
extern double g_pActiveCamera[];
extern double g_pActiveCamera_P2[];
extern SceneryParticle g_ActiveParticle;
extern SceneryParticle g_SceneryParticles[200];
extern int g_ParticlePriorityTable[];
extern int g_WeatherActive[2];
extern int *g_pRainMesh_P1;
extern int *g_pRainMesh_P2;
extern int g_WeatherViewportCount;
extern int g_LightningEnabled;
extern int g_WeatherDropOffsetX[2];
extern int g_WeatherDropOffsetY[2];
extern double g_SnowflakeAngle[2];
extern uint8_t g_WeatherGridObjects[];
extern int g_WeatherAudioVoices[2];
extern int g_WeatherAnimTick;
extern int g_GlobalFrameCount;
extern int g_RainTextureId;
extern int g_TrackPathIntervalTable[];
extern float g_TrackPathTimer;
extern int g_NumTrackDynamicObjects1;
extern int g_NumTrackDynamicObjects2;
extern int g_NumTrackDynamicObjects3;
extern int g_NumTrackDynamicObjects4;
extern int g_TurboMeterFill;
extern int g_HudAnimTimers[];
extern int g_IsGamePaused;
extern double g_LapRecordSeconds;
extern int g_TargetLapTimeCents;
extern double g_HudAnimSinPhase;
extern int g_ShowDebugCoords;
extern double g_CameraPosX;
extern double g_CameraPosY;
extern double g_CameraPosZ;
extern double g_CameraAngleX;
extern double g_CameraAngleY;
extern int g_ConfirmationPromptType;
extern int g_pHudDrawCommandBuffer;
extern int g_MultiplayerMode;
extern int g_TimeTrialActive;
extern int g_IsAttractDemoMode;
extern int g_ChampionshipCredits;
extern int g_AudioCdTrackMode;
extern int g_CurrentCdTrackNumber;
extern void *g_pCreditIconSprite;
extern int g_PlayAgainPromptActive;
extern void *g_pCarIconSprites[];
extern int g_ShowSecondPlayerFlag;
extern int g_FontId_Disabled;
extern char s_QUIT_00494d94[];
extern int g_GraphicsResolutionMode;
extern int g_VideoModeActive;
extern char g_ErrorMessageBuffer[];
extern int g_GraphicsInitErrorFlag;
extern int g_CameraViewportWidth;
extern int g_CameraViewportHeight;
extern int g_SplitScreenMode;
extern int g_ScreenSizeSetting;
extern double g_ViewportScreenScaleTable[][2];
extern void *g_pLisaCommandQueueMirror;
extern void *g_pLisaActiveTABMirror;
extern void *g_pLisaFramebufferMirror1;
extern void *g_pLisaFramebufferMirror2;
extern int g_CameraFovHalfX;
extern int g_CameraFovHalfY;

extern double g_Const_DegToRad;
extern double g_Const_0_05;
extern double g_Const_0_1;
extern double g_Const_TwoPi;
extern double g_Const_Pi;
extern double g_Const_NegPi;
extern double g_Const_0_5;
extern double g_Const_1_0;
extern double g_Const_0_1_B;
extern double g_Const_160_0;
extern double g_Const_100_0;
extern double g_Const_DegToRad_B;
extern double g_Const_Neg100_0;
extern double g_Const_200_0;
extern double g_Const_0_005;
extern double g_Const_Inv16384;
extern double g_Const_0_0;
extern double g_Const_0_0_B;

/* --- External Function Prototypes --- */
void Font_PrintDirect(int x, int y, const char *text, void *surface, int width, int height, int color);
int Gfx_SetRenderTarget(int type, void *surface, int width, int height, int pitch);
int Gfx_SetClipRect(int x1, int y1, int x2, int y2);
void Gfx_DrawSprite(void *sprite, void *coords, int flags);
void Gfx_FreeSurface(void *surface);
int Gfx_RestoreSurface(void);
void Gfx_BlitTransparentLUT(int src, int x, int y, int w, int h, int dst, int dx, int dy, void *lut, int spitch, int dpitch);
void *Palette_AdjustRGB(int p1, int p2, const void *palette);

int Audio_PlaySound(void *params);
void *Audio_GetVoice(int voice_handle);
int Audio_StopSound(int sound_id);

float Math_RandomFloat0To1(void);
double Math_WrapAngle(double angle, double max_angle);
int Log_DebugPrintf(const char *fmt, ...);
double Timer_GetDeltaTime(void);
double Math_AngleMod(int val);

void Menu_AddLayoutItem(int x, int y, int w, int h, int flags);
void Menu_LayoutItems(void *items, int screen_width);
void Menu_ClearLayout(void);

char *HUD_FormatLapTime(void);
void HUD_RenderSpeedometerGauge(int player_idx);
int HUD_AddFloatingMessage(const char *msg, int x, int y, int duration);
void HUD_UpdateFloatingMessages(void);
void HUD_RenderFloatingMessages(void);
void Lisa_ResetRasterizerContext(void);

void FX_SpawnAmbientTrackParticles(void);
void Track_UpdateMovingPathNodes(void);
void Ghost_SaveGhostData(void);
void *Track_FindSurfaceHeight(int x, int y, int z, int p4, int p5);
void Track_UpdateDynamicObjects_Type1(void);
void Track_UpdateDynamicObjects_Type2(void);
void Track_UpdateDynamicObjects_Type3(void);
void Track_UpdateDynamicObjects_Type4(void);

#endif /* FX_H */
