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
    double pos_x;                  /* 0x0000 */
    double pos_y;                  /* 0x0008 */
    double pos_z;                  /* 0x0010 */
    double vel_x;                  /* 0x0018 */
    double vel_y;                  /* 0x0020 */
    double vel_z;                  /* 0x0028 */
    double accel_x;                /* 0x0030 */
    double accel_y;                /* 0x0038 */
    double accel_z;                /* 0x0040 */
    double rear_axle_pos_x;        /* 0x0048 */
    double rear_axle_pos_z;        /* 0x0050 */
    double rear_axle_vel_x;        /* 0x0058 */
    double rear_axle_vel_z;        /* 0x0060 */
    double front_axle_pos_x;       /* 0x0068 */
    double front_axle_pos_z;       /* 0x0070 */
    double front_axle_vel_x;       /* 0x0078 */
    double front_axle_vel_z;       /* 0x0080 */
    double angular_drag_x;         /* 0x0088 */
    double angular_drag_y;         /* 0x0090 */
    double angular_drag_z;         /* 0x0098 */
    double wheel_angle_fl;         /* 0x00a0 */
    uint8_t gap_a8[0xb0 - 0xa8];   /* 0x00a8 */
    double wheel_angle_fr;         /* 0x00b0 */
    double wheel_rot_vel_fl;       /* 0x00b8 */
    double wheel_rot_vel_fr;       /* 0x00c0 */
    uint8_t gap_c8[0xf8 - 0xc8];   /* 0x00c8 */
    double angle_yaw;              /* 0x00f8 */
    double angle_pitch;            /* 0x0100 */
    double angular_vel_roll;       /* 0x0108 */
    double angle_roll;             /* 0x0110 */
    double steering_angle;         /* 0x0118 */
    double ground_y;               /* 0x0120 */
    double ground_y_rear;          /* 0x0128 */
    uint8_t gap_130[0x150 - 0x130];/* 0x0130 */
    int wheel_surface_id[4];       /* 0x0150 */
    uint8_t gap_160[0x170 - 0x160];/* 0x0160 */
    int wheel_surface_info[64];    /* 0x0170 (4 wheels x 16 ints) */
    int is_airborne;               /* 0x0270 */
    int landing_impact;            /* 0x0274 */
    int landing_flag;              /* 0x0278 */
    int current_gear;              /* 0x027c */
    double gear_display_val;       /* 0x0280 */
    double target_rpm;             /* 0x0288 */
    double engine_rpm;             /* 0x0290 */
    int throttle_input;            /* 0x0298 */
    int brake_input;               /* 0x029c */
    uint8_t gap_2a0[0x2f8 - 0x2a0];/* 0x02a0 */
    int handbrake;                 /* 0x02f8 */
    uint8_t gap_2fc[0x33c - 0x2fc];/* 0x02fc */
    int surface_type;              /* 0x033c */
    uint8_t gap_340[0x344 - 0x340];/* 0x0340 */
    int checkpoint_pass1;          /* 0x0344 */
    int checkpoint_pass2;          /* 0x0348 */
    int checkpoint_pass3;          /* 0x034c */
    int checkpoint_counter;        /* 0x0350 */
    int crash_flag1;               /* 0x0354 */
    int crash_flag2;               /* 0x0358 */
    int crash_flag3;               /* 0x035c */
    uint8_t gap_360[0x364 - 0x360];/* 0x0360 */
    int current_node_idx;          /* 0x0364 */
    int target_node_idx;           /* 0x0368 */
    int road_surface_node;         /* 0x036c */
    int field_370;                 /* 0x0370 */
    int lap_number;                /* 0x0374 */
    uint8_t gap_378[0x39c - 0x378];/* 0x0378 */
    int car_model_id;              /* 0x039c */
    int race_rank;                 /* 0x03a0 */
    double lap_time;               /* 0x03a4 */
    uint8_t gap_3ac[0x528 - 0x3ac];/* 0x03ac */
    int is_finished;               /* 0x0528 */
    uint8_t gap_52c[0x534 - 0x52c];/* 0x052c */
    double turbo_charge;           /* 0x0534 */
    int turbo_active;              /* 0x053c */
    int turbo_flash_timer;         /* 0x0540 */
    uint8_t gap_544[0x558 - 0x544];/* 0x0544 */
    int mesh_damage_flag;          /* 0x0558 */
    int detached_wheel_mask;       /* 0x055c */
    int wreck_debris_flag;         /* 0x0560 */
    uint8_t gap_564[0x580 - 0x564];/* 0x0564 */
    int wrong_way_timer;           /* 0x0580 */
    double chassis_roll_spring;    /* 0x0584 */
    double chassis_pitch_spring;   /* 0x058c */
    uint8_t gap_594[0x5ac - 0x594];/* 0x0594 */
    int front_axle_offset;         /* 0x05ac */
    int rear_axle_offset;          /* 0x05b0 */
    uint8_t gap_5b4[0x5d4 - 0x5b4];/* 0x05b4 */
    int finish_banner_timer;       /* 0x05d4 */
    uint8_t gap_5d8[0x5f8 - 0x5d8];/* 0x05d8 */
    int turn_warning_timer;        /* 0x05f8 */
    uint8_t gap_5fc[0x60c - 0x5fc];/* 0x05fc */
    double elimination_timer;      /* 0x060c */
    uint8_t gap_614[0x99c - 0x614];/* 0x0614 */
    int voice_engine;              /* 0x099c */
    int voice_skid;                /* 0x09a0 */
    uint8_t gap_9a4[0x484c - 0x9a4];/* 0x09a4 */
} VehicleState;

/* Player HUD & Viewport State Record (0x4c = 76 bytes) */
typedef struct PlayerHUDState {
    int racer_icon_idx;            /* 0x00 */
    int field_04;                  /* 0x04 */
    int field_08;                  /* 0x08 */
    int field_0c;                  /* 0x0c */
    int field_10;                  /* 0x10 */
    uint8_t gap_14[0x2c - 0x14];   /* 0x14 */
    int vp_x1;                     /* 0x2c */
    int vp_y1;                     /* 0x30 */
    int vp_x2;                     /* 0x34 */
    int vp_y2;                     /* 0x38 */
    uint8_t gap_3c[0x4c - 0x3c];   /* 0x3c */
} PlayerHUDState;

/* Track Segment Attribute Record (12 bytes) */
typedef struct TrackSegmentAttribute {
    int flags;                     /* 0x00 */
    int surface_type;              /* 0x04 */
    int curve_severity;            /* 0x08 */
} TrackSegmentAttribute;

/* Screen Configuration Record */
typedef struct ScreenConfig {
    uint8_t pad_00[0x228];         /* 0x000 */
    int arrow_sprite_width;        /* 0x228 */
} ScreenConfig;

/* Dynamic 3D Object Transform Record (0x20 = 32 bytes) */
typedef struct DynamicObjectTransform {
    int handle;                /* 0x00 */
    int pos_x;                 /* 0x04 */
    int pos_y;                 /* 0x08 */
    int pos_z;                 /* 0x0c */
    int rot_x;                 /* 0x10 */
    int rot_y;                 /* 0x14 */
    int rot_z;                 /* 0x18 */
    int is_placed;             /* 0x1c */
} DynamicObjectTransform;

/* Track Road Sequence Spline Node (0x18 = 24 bytes) */
typedef struct RoadSequenceNode {
    int road_index;            /* 0x00 */
    double pitch1;             /* 0x04 */
    int road_index_rev;        /* 0x0c */
    double pitch2;             /* 0x10 */
} RoadSequenceNode;

/* Vehicle Configuration / Metadata Entry (0xc8 = 200 bytes per vehicle) */
typedef struct VehicleConfig {
    double cam_height;             /* 0x00 */
    double camera_yaw;             /* 0x08 */
    uint8_t pad_10[0x28 - 0x10];   /* 0x10 */
    double camera_pitch;           /* 0x28 */
    double chase_cam_dist_factor;  /* 0x30 */
    int cam_param1;                /* 0x38 */
    int cam_param2;                /* 0x3c */
    uint8_t pad_40[0x48 - 0x40];   /* 0x40 */
    double pitch_offset;           /* 0x48 */
    int chase_target_height;       /* 0x50 */
    int chase_camera_distance;     /* 0x54 */
    int viewport_h;                /* 0x58 */
    int viewport_w;                /* 0x5c */
    int unused_60;                 /* 0x60 */
    float camera_fov_preset;       /* 0x64 */
    double yaw_lag_angle;          /* 0x68 */
    double pitch_lag_angle;        /* 0x70 */
    double target_pos_x;           /* 0x78 */
    double target_pos_y;           /* 0x80 */
    double target_pos_z;           /* 0x88 */
    double cam_pos_x;              /* 0x90 */
    double cam_pos_y;              /* 0x98 */
    double cam_pos_z;              /* 0xa0 */
    double vehicle_yaw;            /* 0xa8 */
    double vehicle_pitch;          /* 0xb0 */
    double camera_yaw_desired;     /* 0xb8 */
    double elevation_smooth_offset;/* 0xc0 */
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
int Lisa_UpdateObjectSpatialGrid(void *obj);
int Lisa_MoveDynamicObject(void *obj);
int Lisa_DeleteDynamicObject(void *obj);
void Lisa_CullObjectsOrthographic(void);
int Lisa_SetDynamicObjectMesh(int a, void *b, void *c, void *d, int e, short f, int g, int h, int i);

void FX_SpawnAmbientTrackParticles(void);
void Track_UpdateMovingPathNodes(void);
void Ghost_SaveGhostData(void);
void *Track_FindSurfaceHeight(int x, int y, int z, int p4, int p5);
void Track_UpdateDynamicObjects_Type1(void);
void Track_UpdateDynamicObjects_Type2(void);
void Track_UpdateDynamicObjects_Type3(void);
void Track_UpdateDynamicObjects_Type4(void);

#endif /* FX_H */
