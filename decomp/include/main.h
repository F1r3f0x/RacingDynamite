#ifndef MAIN_H
#define MAIN_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"
#include "getsurf.h"
#include "lisa3d.h"
#include "geputget.h"

/* Global Game Engine State */
extern int g_GameStage;             /* 0=Init, 1=Run, 2=Shutdown (MAINDOS @ 0x000D7C58, IGN_WIN @ 0x00493734) */
extern int g_MenuState;             /* Active menu state (IGN_WIN @ 0x00563C3C) */
extern int g_MenuSelection;         /* Selected menu item (IGN_WIN @ 0x00563D9C) */
extern int g_SelectedCar;
extern int g_SelectedTrack;
extern int g_InRace;                /* 1 if in race (IGN_WIN @ 0x00525E5C) */
extern uint8_t *g_pMenuCol;
extern uint8_t *g_pMenuTab;
extern int g_CheckpointCount;
extern uint8_t *g_pCheckpoints;
extern char g_TrackDir[64];
extern char g_TrackName[64];
extern uint8_t *g_pTrackCOL;
extern uint8_t *g_pTrackPIC;
extern uint8_t *g_pTrackSHD;
extern uint8_t *g_pTrackTAB;
extern uint8_t *g_pTrackPOS;
extern uint8_t *g_pTrackTRI;
extern uint8_t *g_pAIControllers;
extern int g_TrackWaypointCount;
extern uint8_t *g_pTrackSHD_Copy;
extern int g_MemoryAllocated;

#pragma pack(push, 4)
typedef struct SceneryObstacle {
    int type;                /* 0x00 */
    int model_index;         /* 0x04 */
    int active_state;        /* 0x08: 0=idle/checkpoint, 1=dynamic/flying, -1=disabled */
    double pos_x;            /* 0x0C */
    double pos_y;            /* 0x14 */
    double pos_z;            /* 0x1C */
    double vel_x;            /* 0x24 */
    double vel_y;            /* 0x2C */
    double vel_z;            /* 0x34 */
    double rot_x;            /* 0x3C */
    double rot_y;            /* 0x44 */
    double rot_z;            /* 0x4C */
    double ang_vel_x;        /* 0x54 */
    double ang_vel_y;        /* 0x5C */
    double ang_vel_z;        /* 0x64 */
    double bbox_min_x;       /* 0x6C */
    double bbox_max_x;       /* 0x74 */
    double bbox_min_z;       /* 0x7C */
    double bbox_max_z;       /* 0x84 */
    float mass;              /* 0x8C */
    float anim_timer;        /* 0x90 */
    int anim_max_ticks;      /* 0x94 */
} SceneryObstacle;
#pragma pack(pop)

extern int g_SceneryObstacleCount;
extern SceneryObstacle *g_pSceneryObstacles;

extern int g_ScreenWidth;
extern int g_ScreenHeight;
extern int g_ScreenBPP;
extern int g_ScreenMode;

extern int g_TimerTickCount;
extern double g_LastFrameTime;
extern int g_FrameStep;
extern int g_AccumulatedFrames;
extern int g_TotalFrameCount;
extern int g_IsGamePaused;
extern int g_GameMode;
extern int g_PauseStartTime;
extern double g_RaceTimeSeconds;
extern uint8_t *g_pVehicleShadowTable;

extern double g_CollisionContactX;
extern double g_CollisionContactZ;
extern double g_CollisionNormalAngle;

#pragma pack(push, 4)
typedef struct SceneryParticle {
    int type;                /* 0x00 */
    int pos_x;               /* 0x04 (scaled by 1024) */
    int pos_y;               /* 0x08 (scaled by 1024) */
    int pos_z;               /* 0x0C (scaled by 1024) */
    int vel_x;               /* 0x10 */
    int vel_y;               /* 0x14 */
    int vel_z;               /* 0x18 */
    int drag;                /* 0x1C */
    int gravity;             /* 0x20 */
    int field_24;            /* 0x24 */
    int field_28;            /* 0x28 */
    int field_2c;            /* 0x2C */
    int rot_x;               /* 0x30 */
    int rot_y;               /* 0x34 */
    int rot_z;               /* 0x38 */
    int field_3c;            /* 0x3C */
    int field_40;            /* 0x40 */
    int field_44;            /* 0x44 */
    int field_48;            /* 0x48 */
    int field_4c;            /* 0x4C */
    int life;                /* 0x50 */
    int field_54;            /* 0x54 */
    int field_58;            /* 0x58 */
    int field_5c;            /* 0x5C */
    int field_60;            /* 0x60 */
} SceneryParticle;

typedef struct CameraEffect {
    int type;                /* 0x00 */
    int pos_x;               /* 0x04 */
    int pos_y;               /* 0x08 */
    int pos_z;               /* 0x0C */
    int target_car;          /* 0x10 */
    int duration;            /* 0x14 */
    int active;              /* 0x18 */
} CameraEffect;

typedef struct DynamicObject {
    int field_0;             /* 0x00 */
    int pos_x;               /* 0x04 */
    int pos_y;               /* 0x08 */
    int pos_z;               /* 0x0C */
    int rot_x;               /* 0x10 */
    int rot_y;               /* 0x14 */
    int rot_z;               /* 0x18 */
    int field_1c;            /* 0x1C */
} DynamicObject;

typedef struct SoundChannel {
    int id;                  /* 0x00 */
    int sample_idx;          /* 0x04 */
    int pitch;               /* 0x08 */
    int volume;              /* 0x0C */
    int active;              /* 0x10 */
} SoundChannel;

typedef struct AmbientSoundEmitter {
    int sound_id;            /* 0x00 */
    int channel_id;          /* 0x04 */
    int pos_x;               /* 0x08 */
    int pos_y;               /* 0x0C */
    int pos_z;               /* 0x10 */
} AmbientSoundEmitter;

typedef struct TrackParticleEmitter {
    int particle_type;       /* 0x00 */
    int pos_x;               /* 0x04 */
    int pos_y;               /* 0x08 */
    int pos_z;               /* 0x0C */
    int interval;            /* 0x10 */
    int timer;               /* 0x14 */
    int lifetime;            /* 0x18 */
} TrackParticleEmitter;

typedef struct WeatherEmitter {
    int type;                /* 0x00 */
    int spawn_y;             /* 0x04 */
    int timer;               /* 0x08 */
} WeatherEmitter;

typedef struct TrackObject {
    int field_0;             /* 0x00 */
    void *mesh_data;         /* 0x04 */
    int dummy8;              /* 0x08 */
    int pos_x;               /* 0x0C */
    int pos_y;               /* 0x10 */
    int pos_z;               /* 0x14 */
    short dummy18;           /* 0x18 */
    short dummy1a;           /* 0x1A */
    short dummy1c;           /* 0x1C */
    short field_1e;          /* 0x1E */
    short field_20;          /* 0x20 */
} TrackObject;
#pragma pack(pop)

/* Additional engine global variables */
extern uint8_t *g_pTrackCollisions;
extern uint8_t *g_CarConfigs;
extern uint8_t *g_CarMeshes;
extern uint8_t *g_DynamicMeshBuffer;
extern void *g_DynamicObjectPointers;
extern DynamicObject *g_pDynamicObjects;
extern int g_ActiveVehicleCount;
extern int g_EliminationFlag;
extern int g_EliminationTargetCar;
extern int g_LanguageId;
extern int g_SplitScreenMode;
extern int g_EliminatedCarCount;
extern int g_RemainingCarCount;
extern int *g_pObstacleTriggerTable;
extern int *g_pObstacleStateTable;
extern double g_CountdownTimer;
extern int g_CurrentTick;
extern int g_StartTick;
extern int g_RacePhase;
extern int g_TotalRaceFrames;
extern int g_DemoMode;
extern int g_SelectedCameraView;
extern int g_ParticleEffectsEnabled;
extern uint8_t *g_pAIThreatTable;
extern CameraEffect g_CameraEffect;
extern int g_AmbientEmitterCount;
extern AmbientSoundEmitter *g_pAmbientEmitters;
extern int g_SoundMuted;
extern int g_SoundStateTable[32];
extern int g_TrackEmitterCount;
extern TrackParticleEmitter *g_pTrackEmitters;
extern int g_WeatherParticleCount;
extern WeatherEmitter *g_pWeatherEmitters;
extern int g_TrackCandidateCount;
extern TrackObject **g_pTrackCandidates;
extern void *g_pSRF_RaycastTable;
extern uint8_t g_GhostDataBuffer[0x49d4c];
extern int g_GhostCarLoaded;
extern uint8_t g_TrackBinaryCache[0x597];

/* Engine helper declarations */
double Math_RandomFloat(void);
void   Audio_PlaySample(int ch, int sample, int pan, int unk, int volume, int freq, int loop);
SoundChannel *Audio_GetChannel(int ch);
int    Lisa_CreateDynamicObject(int id, int type, int *ptr, int *mesh, int unk1, short unk2, int unk3, int unk4, int unk5);
int    Lisa_MoveDynamicObject(void *obj);
int    Lisa_DeleteDynamicObject(void *obj);
void  *Surface_GetHeightAtPoint(int x, int y, int z);
void   Surface_FreeSRF(void);
void   FX_SpawnParticle(SceneryParticle *p);
void   FX_SpawnCameraParticle(CameraEffect *c);
void   FX_UpdateEngineSmoke(void);
void   FX_UpdateTurboFlames(void);
void   FX_UpdateSparks(void);
void   HUD_ShowAnnouncementBanner(const char *msg, int param2, int param3, int param4);
void   HUD_DrawPlayerSplitTimer(int param1, int *param2);
void   HUD_DrawDemoWatermark(int a, int b, int c, int d);
void   HUD_UpdateLapCounters(void);
int    Input_IsKeyPressed(uint8_t scancode);
void   Obstacle_ResetActions(void);
void   Obstacle_SetTriggerState(int *state, int val);
void   Sound_FreeSample(int idx);

/* Function prototypes */
int  Menu_Init(void);
int  App_FrameTick(void);
int  App_Init(void);
void App_Shutdown(void);
void AI_FollowTrackSplines(int car_idx);
int  Game_Init(void);
int  Track_LoadAllAssets(void);
int  App_SetVideoMode(void);
void Track_LoadPlacementsAndCars(void);
void Mesh_LoadTrackAndCars(void);
void Texture_LoadAllPages(void);
int  Track_LoadSplines(void);
void Race_InitSceneAndCars(void);
void Race_ResolveVehicleCollisions(void);
void Race_CheckCheckpointTriggers(void);
void Car_UpdateAxleSpeeds(int car_idx);
void Car_VerticalDynamics(int car_idx);
void Car_PhysicsTick(int car_idx);
int  Math_Signum(int val);
void Physics_ReflectVelocityOffNormal(double *pVec);
void Car_CheckLandingStatus(int car_idx);
void Car_UpdateShadowTracking(int car_idx);
void Car_UpdateBodyVelocity(int car_idx);
double Timer_GetDeltaTime(void);
int  Collision_TestLineIntersection(double *car1, double *car2);
int  Collision_TestPolygonOverlap(float *poly1_x, float *poly1, int count1, float *poly2, int count2);
int  Collision_FilterTrackClearance(int param_1, int param_2, int param_3, int param_4, int is_wall);
void *Collision_RaycastVehicleSphere(int x, int y, int z, int car_idx);
void *Collision_TestTrackTriangles(uint32_t param_1, uint32_t param_2, int param_3, int param_4, int param_5);
void Car_SpawnExplosionEffects(int car1_idx, int car2_idx);
void Car_HandleElimination(void);
void Car_ChangeMesh(int car_idx);
void Car_ApplyMeshDamage(int car_idx, int impact_severity);
void Car_UpdateEffects(void);
void Audio_UpdateDynamicDoppler(void);
void Track_SpawnEnvironmentalParticles(void);
void Track_SpawnWeatherParticles(void);
void FX_UpdateSkidMarks(void);
void FX_UpdateTransparentSpriteObject(void *particle, int instance_idx);
void FX_UpdateTransparentSpriteObject2(void *particle, int instance_idx);
void FX_UpdateHandlePlotObject(void *particle, int instance_idx);
void FX_UpdateSuperPlotObject(void *particle, int instance_idx);
void Obstacle_SimulateDynamics(void *obstacle);
void FX_UpdateFlyingParticles(void *particle, int instance_idx);
void Obstacle_TriggerAction(void);
void AI_InitSteeringConeLookup(void);
void Ghost_LoadCarAndPath(void);
void Track_LoadBinaryCache(void);
void Sound_FreeAllSounds(void);
void Game_Shutdown(void);
void Race_UpdateCountdownAndFinish(void);
void HUD_UpdateRaceTimes(void);
void Input_ProcessRaceHotkeys(void);
void Input_PollPlayerVehicleControls(void);
void Ghost_SaveCarAndPath(void);
void Track_SaveBinaryCache(void);

#endif /* MAIN_H */
