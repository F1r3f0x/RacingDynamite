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

/* Function prototypes */
int  Menu_Init(void);
int  App_FrameTick(void);
int  App_Init(void);
void App_Shutdown(void);
void AI_FollowTrackSplines(int car_idx);
int  Game_Init(void);
int  Track_LoadAllAssets(const char *track_dir, const char *track_name);
void Race_InitSceneAndCars(void);
void Race_ResolveVehicleCollisions(void);
void Race_CheckCheckpointTriggers(void);

#endif /* MAIN_H */
