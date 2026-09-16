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

/* Function prototypes */
int  Menu_Init(void);
int  App_FrameTick(void);
int  App_Init(void);
void App_Shutdown(void);
void AI_FollowTrackSplines(int car_idx);
int  Game_Init(void);
int Track_LoadAllAssets(void);
int App_SetVideoMode(void);
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
void Car_SpawnExplosionEffects(void);
void Car_HandleElimination(void);
void Car_ChangeMesh(int car_idx);
void Car_ApplyMeshDamage(int car_idx, int impact_severity);
void Car_UpdateEffects(void);

#endif /* MAIN_H */
