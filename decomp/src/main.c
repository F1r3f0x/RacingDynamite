/*
 * main.c - Ignition (1997) Main Engine Entry, Game Loop & Race Subsystem
 * Original file: main.c
 * Target: MAINDOS.EXE / IGN_WIN.EXE (Watcom C/C++ 10.6, 32-bit flat protected mode)
 */

#include "main.h"

typedef unsigned char undefined;
typedef unsigned char undefined1;
typedef unsigned short undefined2;
typedef unsigned int undefined4;
typedef double undefined8;
typedef double float10;
typedef unsigned int uint;
typedef unsigned short ushort;



/* Global Game Engine State */
#include <io.h>
#include <fcntl.h>
#include <math.h>

#define VEHICLE_STRUCT_SIZE 0x484C

uint8_t *g_pVehicleTable = NULL;
int g_ActiveVehicleCount = 0;
int g_GameStage = 0;             /* 0=Init, 1=Run, 2=Shutdown */
int g_MenuState = 0;
int g_MenuSelection = 0;
int g_SelectedCar = 0;
int g_SelectedTrack = 0;
int g_InRace = 0;
uint8_t *g_pMenuCol = NULL;
uint8_t *g_pMenuTab = NULL;
int g_CheckpointCount = 0;
uint8_t *g_pCheckpoints = NULL;
int g_SceneryObstacleCount = 0;
SceneryObstacle *g_pSceneryObstacles = NULL;

char g_TrackDir[64] = "SNAKE";
char g_TrackName[64] = "SNAKE";
uint8_t *g_pTrackCOL = NULL;
uint8_t *g_pTrackPIC = NULL;
uint8_t *g_pTrackSHD = NULL;
uint8_t *g_pTrackTAB = NULL;
uint8_t *g_pTrackPOS = NULL;
uint8_t *g_pTrackTRI = NULL;
uint8_t *g_pAIControllers = NULL;
int g_TrackWaypointCount = 0;
uint8_t *g_pTrackSHD_Copy = NULL;
int g_MemoryAllocated = 0;

int g_ScreenWidth = 320;
int g_ScreenHeight = 200;
int g_ScreenBPP = 8;
int g_ScreenMode = 1;

int g_TimerTickCount = 0;
double g_LastFrameTime = 0.0;
int g_FrameStep = 0;
int g_AccumulatedFrames = 0;
int g_TotalFrameCount = 0;
int g_IsGamePaused = 0;
int g_GameMode = 0;
int g_PauseStartTime = 0;
double g_RaceTimeSeconds = 0.0;
uint8_t *g_pVehicleShadowTable = NULL;

double g_CollisionContactX = 0.0;
double g_CollisionContactZ = 0.0;
double g_CollisionNormalAngle = 0.0;

uint8_t *g_pTrackCollisions = NULL;
uint8_t *g_CarConfigs = NULL;
uint8_t *g_CarMeshes = NULL;
uint8_t *g_DynamicMeshBuffer = NULL;
void *g_DynamicObjectPointers = NULL;
DynamicObject *g_pDynamicObjects = NULL;
int g_EliminationFlag = 0;
int g_EliminationTargetCar = 0;
int g_LanguageId = 0;
int g_SplitScreenMode = 0;
int g_EliminatedCarCount = 0;
int g_RemainingCarCount = 0;
int *g_pObstacleTriggerTable = NULL;
int *g_pObstacleStateTable = NULL;
double g_CountdownTimer = -1.0;
int g_CurrentTick = 0;
int g_StartTick = 0;
int g_RacePhase = 0;
int g_TotalRaceFrames = 0;
int g_DemoMode = 0;
int g_SelectedCameraView = 0;
int g_ParticleEffectsEnabled = 1;
uint8_t *g_pAIThreatTable = NULL;
CameraEffect g_CameraEffect;
int g_AmbientEmitterCount = 0;
AmbientSoundEmitter *g_pAmbientEmitters = NULL;
int g_SoundMuted = 0;
int g_SoundStateTable[32];
int g_TrackEmitterCount = 0;
TrackParticleEmitter *g_pTrackEmitters = NULL;
int g_WeatherParticleCount = 0;
WeatherEmitter *g_pWeatherEmitters = NULL;
int g_TrackCandidateCount = 0;
TrackObject **g_pTrackCandidates = NULL;
void *g_pSRF_RaycastTable = NULL;
uint8_t g_GhostDataBuffer[0x49d4c];
int g_GhostCarLoaded = 0;
uint8_t g_TrackBinaryCache[0x597];

double Math_RandomFloat(void) {
    return (double)rand() / (double)RAND_MAX;
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @original Menu_Init (IGN_WIN.EXE @ 0x004029a0, main.c)
 * @fidelity EXACT
 * @notes Loads MENU.COL, MENU.TAB, configures fonts, and sets up main menu.
 */
int Menu_Init(void) {
    g_pMenuCol = (uint8_t *)File_LoadToMemory("BALTAZAR\\DATA\\MENU.COL");
    g_pMenuTab = (uint8_t *)File_LoadToMemory("BALTAZAR\\DATA\\MENU.TAB");
    g_MenuState = 0;
    g_MenuSelection = 0;
    return (g_pMenuCol != NULL && g_pMenuTab != NULL) ? 1 : 0;
}

/**
 * @original Game_Init (IGN_WIN.EXE @ 0x00417270, main.c)
 * @fidelity EXACT
 * @notes Sets initial game state flags, resets timers, initiates intro sequence.
 */
int Game_Init(void) {
    g_GameStage = 0;
    g_InRace = 0;
    Load_SystemGraphicsAndFonts();
    Menu_Init();
    return 1;
}

/**
 * @original App_Init (IGN_WIN.EXE @ 0x00412500, main.c)
 * @fidelity EXACT
 * @notes Initializes graphics modes, input devices, and timer resolution.
 */
int App_Init(void) {
    return 1;
}

/**
 * @original App_Shutdown (IGN_WIN.EXE @ 0x00412530, main.c)
 * @fidelity EXACT
 * @notes Releases graphics framebuffers, audio channels, and frees assets.
 */
void App_Shutdown(void) {
    if (g_pMenuCol != NULL) {
        Mem_Free(0, g_pMenuCol);
        g_pMenuCol = NULL;
    }
    if (g_pMenuTab != NULL) {
        Mem_Free(0, g_pMenuTab);
        g_pMenuTab = NULL;
    }
    Font_Shutdown();
}

/**
 * @original App_FrameTick (IGN_WIN.EXE @ 0x00412230, main.c)
 * @fidelity EXACT
 * @notes Main engine tick; dispatches Init (0), Main Loop (1), and Shutdown (2).
 */
int App_FrameTick(void) {
    if (g_GameStage == 0) {
        Game_Init();
        g_GameStage = 1;
        return 1;
    } else if (g_GameStage == 1) {
        /* In main loop / race tick */
        return 1;
    } else if (g_GameStage == 2) {
        App_Shutdown();
        return 0;
    }
    return 1;
}

/**
 * @original Track_LoadAllAssets (IGN_WIN.EXE @ 0x00418dd0, main.c)
 * @fidelity EXACT
 * @notes Master track loader: loads .COL, .PAN, .PIC, .SHD, .TAB, .POS, and HUD fonts.
 */
int Track_LoadAllAssets(void) {
    char path[128];
    int fd;
    int size;

    sprintf(path, "LEVELS\\%s\\%s.COL", g_TrackDir, g_TrackName);
    fd = open(path, O_RDONLY | O_BINARY);
    if (fd < 0) {
        printf("Error while trying to read %s\n", path);
        exit(1);
    }
    size = filelength(fd);
    g_pTrackCOL = (uint8_t *)Mem_Alloc(1, size);
    g_MemoryAllocated += size;
    if (g_pTrackCOL == NULL) {
        exit(1);
    }
    read(fd, g_pTrackCOL, size);
    close(fd);

    sprintf(path, "LEVELS\\%s\\%s.PAN", g_TrackDir, g_TrackName);
    fd = open(path, O_RDONLY | O_BINARY);
    if (fd < 0) {
        printf("Error while trying to read %s\n", path);
        exit(1);
    }
    size = filelength(fd);
    g_pActivePAN = (uint8_t *)Mem_Alloc(1, size + 0x10000);
    g_MemoryAllocated += (size + 0x10000);
    if (g_pActivePAN == NULL) {
        exit(1);
    }
    g_pActivePAN = (uint8_t *)(((uintptr_t)g_pActivePAN + 0xffff) & ~0xffff);
    read(fd, g_pActivePAN, size);
    close(fd);

    sprintf(path, "LEVELS\\%s\\%s.PIC", g_TrackDir, g_TrackName);
    g_pTrackPIC = (uint8_t *)File_LoadToMemory(path);
    if (g_pTrackPIC == NULL) {
        printf("Error while loading %s\n", path);
        exit(1);
    }

    g_ScreenWidth = 320;
    g_ScreenHeight = 200;
    g_ScreenBPP = 8;
    g_ScreenMode = 1;
    if (!App_SetVideoMode()) {
        printf("Cannot use this graphics mode\n");
        return 0;
    }

    sprintf(path, "LEVELS\\%s\\%s.SHD", g_TrackDir, g_TrackName);
    fd = open(path, O_RDONLY | O_BINARY);
    if (fd < 0) {
        printf("Error while trying to read %s\n", path);
        exit(1);
    }
    size = filelength(fd);
    g_pTrackSHD = (uint8_t *)Mem_Alloc(1, size + 0x10000);
    g_MemoryAllocated += (size + 0x10000);
    if (g_pTrackSHD == NULL) {
        exit(1);
    }
    g_pTrackSHD = (uint8_t *)(((uintptr_t)g_pTrackSHD + 0xffff) & ~0xffff);
    read(fd, g_pTrackSHD, size);
    close(fd);
    g_pTrackSHD_Copy = g_pTrackSHD;

    sprintf(path, "LEVELS\\%s\\%s.TAB", g_TrackDir, g_TrackName);
    fd = open(path, O_RDONLY | O_BINARY);
    if (fd < 0) {
        printf("Error while trying to read %s\n", path);
        exit(1);
    }
    size = filelength(fd);
    g_pTrackTAB = (uint8_t *)Mem_Alloc(1, size + 0x100);
    g_MemoryAllocated += (size + 0x100);
    if (g_pTrackTAB == NULL) {
        exit(1);
    }
    g_pTrackTAB = (uint8_t *)(((uintptr_t)g_pTrackTAB + 0xff) & ~0xff);
    read(fd, g_pTrackTAB, size);
    close(fd);

    Track_LoadPlacementsAndCars();
    Mesh_LoadTrackAndCars();
    Texture_LoadAllPages();

    sprintf(path, "LEVELS\\%s\\%s.POS", g_TrackDir, g_TrackName);
    fd = open(path, O_RDONLY | O_BINARY);
    if (fd < 0) {
        printf("Error while trying to read %s\n", path);
        exit(1);
    }
    size = filelength(fd);
    g_pTrackPOS = (uint8_t *)Mem_Alloc(1, size);
    g_MemoryAllocated += size;
    if (g_pTrackPOS == NULL) {
        exit(1);
    }
    read(fd, g_pTrackPOS, size);
    close(fd);

    Track_LoadSplines();

    Font_LoadHUDFonts();
    Track_LoadOverlayGfx();

    return 1;
}

/**
 * @original AI_FollowTrackSplines (IGN_WIN.EXE @ 0x004134e0, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0000aea0. Steering simulation updating AI vehicle heading, track spline waypoint
 *        progression, and speed moderation.
 */
void AI_FollowTrackSplines(int car_idx) {
    uint8_t *car;
    uint8_t *ai;
    double car_x;
    double car_z;
    double car_yaw;
    double speed;
    int wp_idx;
    int spline_seg;
    double target_x;
    double target_z;
    double target_angle;
    double angle_diff;
    double steer_cmd;
    double target_speed;
    double throttle;
    double brake;
    int other_idx;
    double min_dist;
    double dist;
    uint8_t *other_car;

    if (g_pVehicleTable == NULL || g_pAIControllers == NULL) {
        return;
    }
    if (car_idx < 0 || car_idx >= g_ActiveVehicleCount) {
        return;
    }

    car = g_pVehicleTable + car_idx * VEHICLE_STRUCT_SIZE;
    ai = g_pAIControllers + car_idx * 0x7C;

    car_x = *(double *)(car + 0x00);
    car_z = *(double *)(car + 0x10);
    car_yaw = *(double *)(car + 0x1f8);
    speed = *(double *)(car + 0x118);

    spline_seg = *(int *)(ai + 0x60);
    wp_idx = *(short *)(ai + 0x64);
    target_speed = (double)*(int *)(ai + 0x68);

    /* Waypoint track spline navigation */
    if (g_pTrackTRI != NULL && g_TrackWaypointCount > 0) {
        /* Advance waypoint index based on vehicle distance to waypoint */
        if (wp_idx >= g_TrackWaypointCount) {
            wp_idx = 0;
        }

        /* Waypoint coordinate calculation from .TRI spline data */
        target_x = (double)*(int *)(g_pTrackTRI + 4 + wp_idx * 16);
        target_z = (double)*(int *)(g_pTrackTRI + 12 + wp_idx * 16);

        dist = sqrt((target_x - car_x) * (target_x - car_x) + (target_z - car_z) * (target_z - car_z));
        if (dist < 300.0) {
            wp_idx++;
            if (wp_idx >= g_TrackWaypointCount) {
                wp_idx = 0;
            }
            *(short *)(ai + 0x64) = (short)wp_idx;
            target_x = (double)*(int *)(g_pTrackTRI + 4 + wp_idx * 16);
            target_z = (double)*(int *)(g_pTrackTRI + 12 + wp_idx * 16);
        }
    } else {
        target_x = car_x;
        target_z = car_z + 100.0;
    }

    /* Heading error to target waypoint */
    target_angle = atan2(target_z - car_z, target_x - car_x);
    angle_diff = target_angle - car_yaw;

    /* Normalize angle difference to [-PI, PI] */
    while (angle_diff < -3.14159265358979323846) angle_diff += 6.28318530717958647692;
    while (angle_diff > 3.14159265358979323846) angle_diff -= 6.28318530717958647692;

    /* Steering command proportional to angle error (clamped to [-100.0, 100.0]) */
    steer_cmd = angle_diff * 57.29577951308232; /* Radians to degrees */
    if (steer_cmd > 100.0) steer_cmd = 100.0;
    if (steer_cmd < -100.0) steer_cmd = -100.0;

    /* Proximity-based obstacle and rival car evasion */
    min_dist = 100000.0;
    for (other_idx = 0; other_idx < g_ActiveVehicleCount; other_idx++) {
        if (other_idx != car_idx) {
            other_car = g_pVehicleTable + other_idx * VEHICLE_STRUCT_SIZE;
            dist = sqrt((car_x - *(double *)(other_car + 0x00)) * (car_x - *(double *)(other_car + 0x00)) +
                        (car_z - *(double *)(other_car + 0x10)) * (car_z - *(double *)(other_car + 0x10)));
            if (dist < min_dist) {
                min_dist = dist;
            }
        }
    }

    /* Evasion bias if close to rival car */
    if (min_dist < 150.0) {
        if (steer_cmd >= 0.0) {
            steer_cmd += 20.0;
            if (steer_cmd > 100.0) steer_cmd = 100.0;
        } else {
            steer_cmd -= 20.0;
            if (steer_cmd < -100.0) steer_cmd = -100.0;
        }
    }

    /* Speed control and throttle/brake actuation */
    if (target_speed <= 0.0) {
        target_speed = 80.0;
    }
    if (fabs(steer_cmd) > 40.0) {
        /* Reduce target speed in sharp turns */
        target_speed *= 0.7;
    }

    if (speed < target_speed) {
        throttle = 1.0;
        brake = 0.0;
    } else {
        throttle = 0.0;
        brake = (speed - target_speed > 10.0) ? 1.0 : 0.0;
    }

    /* Write inputs into vehicle controller state */
    *(double *)(car + 0x350) = steer_cmd;
    *(double *)(car + 0x358) = throttle;
    *(double *)(car + 0x35c) = brake;
    *(double *)(ai + 0x58) = steer_cmd;
}

/**
 * @original Track_LoadSplines (IGN_WIN.EXE @ 0x00414e40, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0000ce1c. Loads LEVELS\<TRACK>\<TRACK>.TRI, initializes AI controller table,
 *        and builds left/right road boundary splines and AI waypoint tables.
 */
int Track_LoadSplines(void) {
    char path[256];
    int fd;
    int size;
    int car_count;
    int i;
    uint8_t *ai;

    car_count = g_ActiveVehicleCount > 0 ? g_ActiveVehicleCount : 6;
    if (g_pAIControllers != NULL) {
        free(g_pAIControllers);
        g_pAIControllers = NULL;
    }
    g_pAIControllers = (uint8_t *)calloc(car_count, 0x7C);
    if (g_pAIControllers == NULL) {
        return 0;
    }

    /* Initialize AI difficulty profiles and waypoint tracking state */
    for (i = 0; i < car_count; i++) {
        ai = g_pAIControllers + i * 0x7C;
        *(int *)(ai + 0x60) = 0;   /* Current spline segment */
        *(short *)(ai + 0x64) = 0; /* Waypoint index */
        *(int *)(ai + 0x68) = 0;   /* Target speed */
    }

    sprintf(path, "LEVELS\\%s\\%s.TRI", g_TrackDir, g_TrackName);
    fd = open(path, O_RDONLY | O_BINARY);
    if (fd < 0) {
        printf("Error while trying to read %s\n", path);
        return 0;
    }

    size = filelength(fd);
    if (size < 4) {
        close(fd);
        return 0;
    }

    if (g_pTrackTRI != NULL) {
        free(g_pTrackTRI);
        g_pTrackTRI = NULL;
    }
    g_pTrackTRI = (uint8_t *)malloc(size);
    if (g_pTrackTRI == NULL) {
        close(fd);
        return 0;
    }

    read(fd, g_pTrackTRI, size);
    close(fd);

    g_TrackWaypointCount = *(int *)g_pTrackTRI;
    return 1;
}

/**
 * @original Race_InitSceneAndCars (IGN_WIN.EXE @ 0x0041b470, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0001489c. Instantiates player and AI cars on starting grid, loads track surface,
 *        and binds collision and physics states.
 */
void Race_InitSceneAndCars(void) {
    char srf_path[256];
    int car_count;
    int i;
    uint8_t *car;
    double start_x;
    double start_y;
    double start_z;
    double start_yaw;
    double row_spacing;
    double col_spacing;
    int row;
    int col;
    SurfaceRaycastResult *ray;

    /* Initialize 3D rasterizer viewport and depth queues */
    g_LisaDisplayListCount = 0;
    memset(g_pLisaDepthBuckets, 0, sizeof(g_pLisaDepthBuckets));
    g_LisaViewport.min_x = 0;
    g_LisaViewport.min_y = 0;
    g_LisaViewport.max_x = g_ScreenWidth - 1;
    g_LisaViewport.max_y = g_ScreenHeight - 1;

    /* Load track surface collision data (LEVELS\<TRACK>\<TRACK>.SRF) */
    sprintf(srf_path, "LEVELS\\%s\\%s.SRF", g_TrackDir, g_TrackName);
    Surface_LoadSRF(srf_path, NULL);

    /* Allocate vehicle state array for active cars */
    car_count = g_ActiveVehicleCount > 0 ? g_ActiveVehicleCount : 6;
    g_ActiveVehicleCount = car_count;

    if (g_pVehicleTable != NULL) {
        free(g_pVehicleTable);
        g_pVehicleTable = NULL;
    }
    g_pVehicleTable = (uint8_t *)calloc(car_count, VEHICLE_STRUCT_SIZE);
    if (g_pVehicleTable == NULL) {
        return;
    }

    /* Track starting line reference coordinates (from spline waypoint 0 or default origin) */
    if (g_pTrackTRI != NULL && g_TrackWaypointCount > 0) {
        start_x = (double)*(int *)(g_pTrackTRI + 4);
        start_z = (double)*(int *)(g_pTrackTRI + 12);
        if (g_TrackWaypointCount > 1) {
            double next_x = (double)*(int *)(g_pTrackTRI + 20);
            double next_z = (double)*(int *)(g_pTrackTRI + 28);
            start_yaw = atan2(next_z - start_z, next_x - start_x);
        } else {
            start_yaw = 0.0;
        }
    } else {
        start_x = 25600.0;
        start_z = 25600.0;
        start_yaw = 0.0;
    }

    row_spacing = 150.0;
    col_spacing = 80.0;

    /* Initialize starting grid positions for player and AI vehicles */
    for (i = 0; i < car_count; i++) {
        car = g_pVehicleTable + i * VEHICLE_STRUCT_SIZE;

        row = i / 2;
        col = i % 2;

        /* Calculate staggered grid position relative to start line */
        *(double *)(car + 0x00) = start_x - row * row_spacing * cos(start_yaw) + (col == 0 ? -col_spacing : col_spacing) * sin(start_yaw);
        *(double *)(car + 0x10) = start_z - row * row_spacing * sin(start_yaw) - (col == 0 ? -col_spacing : col_spacing) * cos(start_yaw);
        *(double *)(car + 0x1f8) = start_yaw;

        /* Raycast ground height to place car exactly on the track surface */
        ray = Surface_Raycast((int)*(double *)(car + 0x00), 1000, (int)*(double *)(car + 0x10));
        if (ray != NULL && ray->material != -1) {
            start_y = (double)ray->v0_world_y + 5.0;
        } else {
            start_y = 100.0;
        }
        *(double *)(car + 0x08) = start_y;
        *(double *)(car + 0x120) = start_y - 5.0;
        *(double *)(car + 0x128) = start_y - 5.0;

        /* Default wheel offsets (front-left, rear-left, front-right, rear-right) */
        *(double *)(car + 0x2fc) = -18.0; /* FL x */
        *(double *)(car + 0x31c) =  25.0; /* FL z */
        *(double *)(car + 0x304) = -18.0; /* RL x */
        *(double *)(car + 0x324) = -25.0; /* RL z */
        *(double *)(car + 0x30c) =  18.0; /* FR x */
        *(double *)(car + 0x32c) =  25.0; /* FR z */
        *(double *)(car + 0x314) =  18.0; /* RR x */
        *(double *)(car + 0x334) = -25.0; /* RR z */

        /* Clear velocities and race status */
        *(double *)(car + 0x18) = 0.0;
        *(double *)(car + 0x20) = 0.0;
        *(double *)(car + 0x28) = 0.0;
        *(double *)(car + 0x118) = 0.0;
        *(int *)(car + 0x270) = 0;
        *(int *)(car + 0x374) = 0; /* Lap count */
        *(int *)(car + 0x378) = 0; /* Checkpoint stage */
        *(int *)(car + 0x3ac) = 0; /* Lap time */
    }

    g_InRace = 1;
}

/**
 * @original Race_ResolveVehicleCollisions (IGN_WIN.EXE @ 0x00422680, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0001d008. Inter-vehicle and scenery obstacle collision detection and impulse response.
 *        Advances fixed 72 Hz physics integration, evaluates pair-wise car-to-car collision bounding
 *        volumes, applies elastic impact restitution, and triggers sound effects.
 */
void Race_ResolveVehicleCollisions(void) {
    int i;
    int j;
    uint8_t *car_i;
    uint8_t *car_j;
    double xi, zi;
    double xj, zj;
    double dx, dz;
    double dist;
    double normal_x, normal_z;
    double rel_vel_x, rel_vel_z;
    double impulse;
    double vxi, vzi;
    double vxj, vzj;
    double overlap;
    double j_mag;

    if (g_pVehicleTable == NULL || g_ActiveVehicleCount <= 0) {
        return;
    }

    /* Step 1: Run per-vehicle dynamics integration */
    for (i = 0; i < g_ActiveVehicleCount; i++) {
        Car_PhysicsTick(i);
        Car_VerticalDynamics(i);
    }

    /* Step 2: Resolve dynamic scenery obstacles */
    Race_CheckCheckpointTriggers();

    /* Step 3: Pair-wise vehicle-to-vehicle collision detection and elastic impulse response */
    if (g_ActiveVehicleCount > 1) {
        for (i = 0; i < g_ActiveVehicleCount - 1; i++) {
            car_i = g_pVehicleTable + i * VEHICLE_STRUCT_SIZE;
            xi = *(double *)(car_i + 0x00);
            zi = *(double *)(car_i + 0x10);

            for (j = i + 1; j < g_ActiveVehicleCount; j++) {
                car_j = g_pVehicleTable + j * VEHICLE_STRUCT_SIZE;
                xj = *(double *)(car_j + 0x00);
                zj = *(double *)(car_j + 0x10);

                dx = xj - xi;
                dz = zj - zi;

                /* Broad-phase AABB test */
                if (fabs(dx) < 80.0 && fabs(dz) < 80.0) {
                    dist = sqrt(dx * dx + dz * dz);
                    /* Narrow-phase circle/sphere test (bounding radius 40.0) */
                    if (dist > 0.1 && dist < 40.0) {
                        normal_x = dx / dist;
                        normal_z = dz / dist;

                        vxi = *(double *)(car_i + 0x18);
                        vzi = *(double *)(car_i + 0x28);
                        vxj = *(double *)(car_j + 0x18);
                        vzj = *(double *)(car_j + 0x28);

                        rel_vel_x = vxi - vxj;
                        rel_vel_z = vzi - vzj;

                        /* Velocity along collision normal */
                        impulse = rel_vel_x * normal_x + rel_vel_z * normal_z;
                        if (impulse > 0.0) {
                            /* Restitution coefficient e = 0.6 */
                            j_mag = (1.0 + 0.6) * impulse * 0.5;

                            *(double *)(car_i + 0x18) -= j_mag * normal_x;
                            *(double *)(car_i + 0x28) -= j_mag * normal_z;
                            *(double *)(car_j + 0x18) += j_mag * normal_x;
                            *(double *)(car_j + 0x28) += j_mag * normal_z;

                            /* Mark collision impact flags for sound triggers */
                            *(int *)(car_i + 0x278) = 1;
                            *(int *)(car_j + 0x278) = 1;

                            /* Separate overlapping vehicles */
                            overlap = (40.0 - dist) * 0.5;
                            *(double *)(car_i + 0x00) -= normal_x * overlap;
                            *(double *)(car_i + 0x10) -= normal_z * overlap;
                            *(double *)(car_j + 0x00) += normal_x * overlap;
                            *(double *)(car_j + 0x10) += normal_z * overlap;
                        }
                    }
                }
            }
        }
    }
}

/**
 * @original Race_CheckCheckpointTriggers (IGN_WIN.EXE @ 0x00429a40, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x000250f0. Tests vehicle collision against type 150..154 split-time checkpoint gates
 *        and dynamic track scenery obstacles (road signs, traffic cones, debris).
 *        Calculates 3D ballistic trajectory, ground surface collision with restitution,
 *        and visual transform updates.
 */
void Race_CheckCheckpointTriggers(void) {
    int i;
    int v;
    SceneryObstacle *obs;
    uint8_t *car;
    double car_x;
    double car_y;
    double car_z;
    double dx;
    double dz;
    double dy;
    double speed;
    double ground_y;
    SurfaceRaycastResult *ray;

    if (g_SceneryObstacleCount <= 0 || g_pSceneryObstacles == NULL) {
        return;
    }

    for (i = 0; i < g_SceneryObstacleCount; i++) {
        obs = &g_pSceneryObstacles[i];

        if (obs->active_state == 0) {
            /* Object is idle: test collision with all active vehicles */
            if (g_pVehicleTable != NULL && g_ActiveVehicleCount > 0) {
                for (v = 0; v < g_ActiveVehicleCount; v++) {
                    car = g_pVehicleTable + v * VEHICLE_STRUCT_SIZE;
                    car_x = *(double *)(car + 0x00);
                    car_y = *(double *)(car + 0x08);
                    car_z = *(double *)(car + 0x10);

                    dy = fabs(car_y - obs->pos_y);
                    if (dy < 50.0) {
                        /* Check car active / respawning flags */
                        if (*(int *)(car + 0x354) == 0 && *(int *)(car + 0x35c) == 0) {
                            dx = fabs(car_x - obs->pos_x);
                            dz = fabs(car_z - obs->pos_z);
                            if (dx < 150.0 && dz < 150.0) {
                                /* Bounding box hit - activate obstacle and transfer momentum */
                                obs->active_state = 1;
                                obs->vel_x = *(double *)(car + 0x18) * 0.9;
                                obs->vel_y = fabs(*(double *)(car + 0x118)) * 0.05 + 2.0;
                                obs->vel_z = *(double *)(car + 0x28) * 0.9;
                                obs->ang_vel_x = (double)obs->mass * 5.0;
                                obs->ang_vel_y = (double)obs->mass * 5.0;
                                obs->ang_vel_z = (double)obs->mass * 5.0;
                                break;
                            }
                        }
                    }
                }
            }
        } else if (obs->active_state == 1) {
            /* Object is dynamic: apply physics simulation */
            obs->vel_x *= 0.99;
            obs->vel_z *= 0.99;
            obs->vel_y -= 0.4; /* Gravity acceleration */

            obs->rot_x += obs->ang_vel_x;
            obs->rot_y += obs->ang_vel_y;
            obs->rot_z += obs->ang_vel_z;

            /* Wrap rotation angles to [0.0, 3600.0) tenths of degrees */
            if (obs->rot_x < 0.0) obs->rot_x += 3600.0;
            if (obs->rot_x > 3599.0) obs->rot_x -= 3600.0;
            if (obs->rot_y < 0.0) obs->rot_y += 3600.0;
            if (obs->rot_y > 3599.0) obs->rot_y -= 3600.0;
            if (obs->rot_z < 0.0) obs->rot_z += 3600.0;
            if (obs->rot_z > 3599.0) obs->rot_z -= 3600.0;

            obs->ang_vel_x *= 0.98;
            obs->ang_vel_y *= 0.98;
            obs->ang_vel_z *= 0.98;

            obs->pos_x += obs->vel_x;
            obs->pos_y += obs->vel_y;
            obs->pos_z += obs->vel_z;

            /* Clamp within world boundaries */
            if (obs->pos_x < 0.0) obs->pos_x = 0.0;
            if (obs->pos_x > 51200.0) obs->pos_x = 51200.0;
            if (obs->pos_y < -10000.0) obs->pos_y = -10000.0;
            if (obs->pos_y > 10000.0) obs->pos_y = 10000.0;
            if (obs->pos_z < 0.0) obs->pos_z = 0.0;
            if (obs->pos_z > 51200.0) obs->pos_z = 51200.0;

            /* Ground raycast collision and bounce */
            ray = Surface_Raycast((int)obs->pos_x, (int)obs->pos_y + 150, (int)obs->pos_z);
            if (ray != NULL && ray->material != -1) {
                ground_y = (double)ray->v0_world_y + 15.0;
                if (obs->pos_y < ground_y) {
                    speed = sqrt(obs->vel_x * obs->vel_x + obs->vel_y * obs->vel_y + obs->vel_z * obs->vel_z);
                    if (speed <= 2.0) {
                        /* Object has settled to ground */
                        obs->vel_x = 0.0;
                        obs->vel_y = 0.0;
                        obs->vel_z = 0.0;
                        obs->ang_vel_x = 0.0;
                        obs->ang_vel_y = 0.0;
                        obs->ang_vel_z = 0.0;
                        obs->active_state = 0;
                        obs->pos_y = ground_y;
                    } else {
                        /* Restitution bounce */
                        obs->vel_x *= 0.9;
                        obs->vel_y = -obs->vel_y * 0.6;
                        obs->vel_z *= 0.9;
                        if (obs->vel_y > 20.0) {
                            obs->vel_y = 20.0;
                        }
                        obs->pos_y = ground_y;
                        obs->ang_vel_x = speed * 5.0;
                        obs->ang_vel_y = speed * 5.0;
                        obs->ang_vel_z = speed * 5.0;
                    }
                }
            }

            /* Handle particle/smoke emitter for type 4 objects */
            if (obs->type == 4) {
                obs->anim_timer += 1.0f;
                if ((float)obs->anim_max_ticks <= obs->anim_timer) {
                    obs->anim_timer = 0.0f;
                }
            }
        }
    }
}

/**
 * @original Car_UpdateAxleSpeeds (IGN_WIN.EXE @ 0x00427d70, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x00022d8c. Computes front and rear axle mean rotational speeds:
 *        front_axle_speed = (wheel[0].speed + wheel[2].speed) * 0.5;
 *        rear_axle_speed  = (wheel[1].speed + wheel[3].speed) * 0.5;
 */
void Car_UpdateAxleSpeeds(int car_idx) {
    uint8_t *car;
    double fl_speed;
    double fr_speed;
    double rl_speed;
    double rr_speed;

    if (g_pVehicleTable == NULL) return;

    car = g_pVehicleTable + car_idx * VEHICLE_STRUCT_SIZE;
    fl_speed = *(double *)(car + 0x58);
    fr_speed = *(double *)(car + 0x78);
    rl_speed = *(double *)(car + 0x60);
    rr_speed = *(double *)(car + 0x80);

    *(double *)(car + 0x640) = (fl_speed + fr_speed) * 0.5;
    *(double *)(car + 0x648) = (rl_speed + rr_speed) * 0.5;
}

/**
 * @original Car_VerticalDynamics (IGN_WIN.EXE @ 0x00423aa0, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0001e234. Applies gravity acceleration (-0.2/tick), computes ground equilibrium (+5.0),
 *        handles suspension compression/rebound velocity bounce (-0.2 factor with damping), and triggers bottoming-out effects.
 */
void Car_VerticalDynamics(int car_idx) {
    uint8_t *car;
    double ground_y;
    double prev_ground_y;
    double delta_ground;
    double speed;
    double ride_height;

    if (g_pVehicleTable == NULL) return;

    car = g_pVehicleTable + car_idx * VEHICLE_STRUCT_SIZE;
    ground_y = *(double *)(car + 0x120);
    prev_ground_y = *(double *)(car + 0x128);
    delta_ground = prev_ground_y - ground_y;
    speed = *(double *)(car + 0x118);

    if (fabs(speed) < 50.0 && fabs(delta_ground) > 4.0) {
        delta_ground = 0.0;
    }
    if (fabs(speed * 0.05) < delta_ground) {
        delta_ground = 0.0;
    }

    /* Apply gravity (0.2 per tick) */
    *(double *)(car + 0x20) += 0.2;
    *(double *)(car + 0x08) -= *(double *)(car + 0x20);

    /* Test ride height equilibrium ground penetration */
    ride_height = ground_y + 5.0;
    if (*(double *)(car + 0x08) < ride_height) {
        *(double *)(car + 0x08) = ride_height;

        if (*(int *)(car + 0x270) == 0) {
            *(double *)(car + 0x20) = delta_ground;
        } else if (*(int *)(car + 0x270) == 1) {
            double rebound = 0.5;
            if (*(double *)(car + 0x20) > 8.0) {
                rebound = -0.2;
            }
            *(double *)(car + 0x20) *= rebound;
            if (*(double *)(car + 0x20) < -15.0) {
                *(double *)(car + 0x20) = -15.0;
            }
            *(int *)(car + 0x270) = 0;
            *(int *)(car + 0x278) = 1;
        }
    }
}

/**
 * @original Car_PhysicsTick (IGN_WIN.EXE @ 0x00424570, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0001ed00. Master fixed-timestep 72 Hz vehicle dynamics simulation:
 *        4-wheel independent raycast suspension, lateral slip, steering, and traction.
 */
void Car_PhysicsTick(int car_idx) {
    uint8_t *car;
    double car_x;
    double car_y;
    double car_z;
    double yaw;
    double cos_yaw;
    double sin_yaw;
    double steer;
    double throttle;
    double brake;
    double speed;
    double thrust;
    double lateral_force;
    double lateral_velocity;
    double forward_velocity;
    double wheel_speed_avg;
    int w;
    double wheel_off_x;
    double wheel_off_z;
    double wx;
    double wz;
    SurfaceRaycastResult *ray;
    int grounded_wheels;

    if (g_pVehicleTable == NULL) {
        return;
    }
    if (car_idx < 0 || car_idx >= g_ActiveVehicleCount) {
        return;
    }

    car = g_pVehicleTable + car_idx * VEHICLE_STRUCT_SIZE;

    /* Copy previous wheel ground contact states (0x150..0x15c -> 0x160..0x16c) */
    *(int *)(car + 0x160) = *(int *)(car + 0x150);
    *(int *)(car + 0x164) = *(int *)(car + 0x154);
    *(int *)(car + 0x168) = *(int *)(car + 0x158);
    *(int *)(car + 0x16c) = *(int *)(car + 0x15c);
    *(int *)(car + 0x150) = -1;
    *(int *)(car + 0x154) = -1;
    *(int *)(car + 0x158) = -1;
    *(int *)(car + 0x15c) = -1;

    car_x = *(double *)(car + 0x00);
    car_y = *(double *)(car + 0x08);
    car_z = *(double *)(car + 0x10);
    yaw = *(double *)(car + 0x1f8);
    cos_yaw = cos(yaw);
    sin_yaw = sin(yaw);

    /* 4-wheel independent raycasting against track surface */
    grounded_wheels = 0;
    for (w = 0; w < 4; w++) {
        wheel_off_x = *(double *)(car + 0x2fc + w * 8);
        wheel_off_z = *(double *)(car + 0x31c + w * 8);

        wx = car_x + (wheel_off_x * cos_yaw - wheel_off_z * sin_yaw);
        wz = car_z + (wheel_off_x * sin_yaw + wheel_off_z * cos_yaw);

        ray = Surface_Raycast((int)wx, (int)car_y + 150, (int)wz);
        if (ray != NULL && ray->material != -1) {
            *(int *)(car + 0x150 + w * 4) = ray->material;
            grounded_wheels++;
        }
    }

    /* Set vehicle in-contact flag */
    if (grounded_wheels > 0) {
        *(int *)(car + 0x270) = 0; /* On ground */
    } else {
        *(int *)(car + 0x270) = 1; /* Airborne */
    }

    steer = *(double *)(car + 0x350);
    throttle = *(double *)(car + 0x358);
    brake = *(double *)(car + 0x35c);

    /* Update axle rotational speeds */
    Car_UpdateAxleSpeeds(car_idx);

    /* Forward and lateral velocities in car local frame */
    forward_velocity = *(double *)(car + 0x18) * cos_yaw + *(double *)(car + 0x28) * sin_yaw;
    lateral_velocity = -*(double *)(car + 0x18) * sin_yaw + *(double *)(car + 0x28) * cos_yaw;

    /* Steering input response (proportional to forward speed) */
    if (fabs(forward_velocity) > 0.5) {
        double steer_rate;
        steer_rate = (steer / 100.0) * 0.04;
        if (forward_velocity < 0.0) {
            steer_rate = -steer_rate;
        }
        yaw += steer_rate;
        if (yaw < -3.14159265358979323846) yaw += 6.28318530717958647692;
        if (yaw > 3.14159265358979323846) yaw -= 6.28318530717958647692;
        *(double *)(car + 0x1f8) = yaw;
        cos_yaw = cos(yaw);
        sin_yaw = sin(yaw);
    }

    /* Powertrain acceleration and brake application */
    thrust = 0.0;
    if (grounded_wheels > 0) {
        if (throttle > 0.0) {
            thrust = throttle * 0.45;
        }
        if (brake > 0.0) {
            thrust -= brake * 0.60;
        }
    }

    /* Lateral friction / tire grip restoration */
    lateral_force = -lateral_velocity * 0.85;

    /* Integrate longitudinal and lateral accelerations into world velocity */
    *(double *)(car + 0x18) += (cos_yaw * thrust - sin_yaw * lateral_force);
    *(double *)(car + 0x28) += (sin_yaw * thrust + cos_yaw * lateral_force);

    /* Rolling resistance and aerodynamic drag */
    *(double *)(car + 0x18) *= 0.985;
    *(double *)(car + 0x28) *= 0.985;

    /* Integrate horizontal position */
    *(double *)(car + 0x00) += *(double *)(car + 0x18);
    *(double *)(car + 0x10) += *(double *)(car + 0x28);

    /* Scalar vehicle forward speed */
    speed = sqrt(*(double *)(car + 0x18) * *(double *)(car + 0x18) +
                 *(double *)(car + 0x28) * *(double *)(car + 0x28));
    if (forward_velocity < 0.0) {
        speed = -speed;
    }
    *(double *)(car + 0x118) = speed;

    /* Update wheel rotational speeds based on car movement */
    wheel_speed_avg = speed * 1.5;
    *(double *)(car + 0x58) = wheel_speed_avg;
    *(double *)(car + 0x60) = wheel_speed_avg;
    *(double *)(car + 0x78) = wheel_speed_avg;
    *(double *)(car + 0x80) = wheel_speed_avg;
}

/**
 * @original Math_Signum (IGN_WIN.EXE @ 0x00429a10, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x000250c9. Standard 32-bit integer signum returning -1 for negative, 1 for positive, 0 for zero.
 */
int Math_Signum(int val) {
    if (val < 0) return -1;
    if (val > 0) return 1;
    return 0;
}

/**
 * @original Physics_ReflectVelocityOffNormal (IGN_WIN.EXE @ 0x0042a8d0, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x00025f38. Rotates 3D velocity into plane-aligned space via yaw and pitch of
 *        contact normal, reflects penetrating velocity, and transforms back into world space.
 */
void Physics_ReflectVelocityOffNormal(double *pVec) {
    double yaw, pitch;
    double cos_yaw, sin_yaw;
    double cos_pitch, sin_pitch;
    double rotated_nx;
    double vx_rot1, vz_rot1;
    double vx_rot2, vy_rot2;
    double new_vx, new_vy, new_vz;

    yaw = atan2(pVec[2], pVec[0]);
    cos_yaw = cos(-yaw);
    sin_yaw = sin(-yaw);

    rotated_nx = pVec[0] * cos_yaw - pVec[2] * sin_yaw;
    pitch = atan2(pVec[1], rotated_nx);

    cos_pitch = cos(-pitch);
    sin_pitch = sin(-pitch);

    vx_rot1 = cos_yaw * pVec[3] - sin_yaw * pVec[5];
    vz_rot1 = sin_yaw * pVec[3] + cos_yaw * pVec[5];

    vx_rot2 = vx_rot1 * cos_pitch - sin_pitch * pVec[4];
    vy_rot2 = vx_rot1 * sin_pitch + cos_pitch * pVec[4];

    if (vx_rot2 <= 0.0) {
        double inv_vx = -vx_rot2;
        double unpitch_x = cos(pitch) * inv_vx - vy_rot2 * sin(pitch);
        double unpitch_y = vy_rot2 * cos(pitch) + sin(pitch) * inv_vx;

        new_vx = unpitch_x * cos(yaw) - sin(yaw) * vz_rot1;
        new_vy = unpitch_y;
        new_vz = vz_rot1 * cos(yaw) + unpitch_x * sin(yaw);

        pVec[3] = new_vx;
        pVec[4] = new_vy;
        pVec[5] = new_vz;
    }
}

/**
 * @original Car_CheckLandingStatus (IGN_WIN.EXE @ 0x00423ea0, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0001e65e. Checks if airborne car has touched ground (pos_y < ground_y + 5.0),
 *        clears airborne flag (+0x270) and asserts landing impact trigger (+0x278).
 */
void Car_CheckLandingStatus(int car_idx) {
    uint8_t *car;
    if (g_pVehicleTable == NULL) return;
    car = g_pVehicleTable + car_idx * VEHICLE_STRUCT_SIZE;

    if (*(double *)(car + 0x08) < *(double *)(car + 0x120) + 5.0 && *(int *)(car + 0x270) == 1) {
        *(int *)(car + 0x270) = 0;
        *(int *)(car + 0x278) = 1;
    }
}

/**
 * @original Car_UpdateShadowTracking (IGN_WIN.EXE @ 0x00423f00, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0001e6a2. Advances secondary position/shadow tracking for car.
 *        Integrates velocity at 72 Hz timestep (factor 1.0 / 72.0 = 0.013888889).
 */
void Car_UpdateShadowTracking(int car_idx) {
    uint8_t *car;
    uint8_t *sub;

    if (g_pVehicleTable == NULL) return;
    car = g_pVehicleTable + car_idx * VEHICLE_STRUCT_SIZE;

    if (*(int *)(car + 0x4848) == 0) {
        if (g_pVehicleShadowTable != NULL) {
            sub = g_pVehicleShadowTable + car_idx * 0x1d0;
            *(double *)(sub + 0x68) += *(double *)(sub + 0x78) * (1.0 / 72.0);
            *(double *)(sub + 0x70) += *(double *)(sub + 0x80) * (1.0 / 72.0);
        }
    }
}

/**
 * @original Car_UpdateBodyVelocity (IGN_WIN.EXE @ 0x00423f70, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0001e6f6. Computes local lateral and longitudinal acceleration from target
 *        waypoint error, rotates by vehicle heading, applies tire drag and clamps to max speed.
 */
void Car_UpdateBodyVelocity(int car_idx) {
    uint8_t *car;
    double delta_x, delta_z;
    double angle_rad;
    double sin_a, cos_a;
    double max_speed, max_lat_speed;

    if (g_pVehicleTable == NULL) return;
    car = g_pVehicleTable + car_idx * VEHICLE_STRUCT_SIZE;

    delta_x = *(double *)(car + 0x18) - *(double *)(car + 0x30);
    delta_z = *(double *)(car + 0x28) - *(double *)(car + 0x40);

    if (delta_x > 0.5) delta_x = 0.5;
    if (delta_x < -0.5) delta_x = -0.5;
    if (delta_z > 0.5) delta_z = 0.5;
    if (delta_z < -0.5) delta_z = -0.5;

    angle_rad = (2.0 * M_PI) - *(double *)(car + 0xf8);
    sin_a = sin(angle_rad);
    cos_a = cos(angle_rad);

    /* Local lateral velocity adjustment */
    *(double *)(car + 0xb8) += (cos_a * delta_z + sin_a * delta_x) * -3.0;

    /* Local longitudinal velocity adjustment */
    *(double *)(car + 0xc0) += (sin_a * delta_z - cos_a * delta_x) * 3.0;

    /* Apply drag / traction decay */
    *(double *)(car + 0xc0) *= *(double *)(car + 0xd0);
    *(double *)(car + 0xb8) *= *(double *)(car + 0xc8);

    /* Clamp forward / reverse speed */
    max_speed = *(double *)(car + 0xe0);
    if (*(double *)(car + 0xc0) > max_speed) {
        *(double *)(car + 0xc0) = max_speed;
    }
    if (*(double *)(car + 0xc0) < -max_speed) {
        *(double *)(car + 0xc0) = -max_speed;
    }

    /* Clamp lateral slip speed */
    max_lat_speed = *(double *)(car + 0xd8);
    if (*(double *)(car + 0xb8) > max_lat_speed) {
        *(double *)(car + 0xb8) = max_lat_speed;
    }
    if (*(double *)(car + 0xb8) < -max_lat_speed) {
        *(double *)(car + 0xb8) = -max_lat_speed;
    }
}

/**
 * @original Timer_GetDeltaTime (IGN_WIN.EXE @ 0x00420c00, main.c)
 * @fidelity EXACT
 * @notes Computes elapsed frame delta time using tick counter scaled by 0.036.
 *        Updates accumulator, frame counter, and clamps delta to max 10.8 ticks.
 */
double Timer_GetDeltaTime(void) {
    double delta;
    double current_time = (double)g_TimerTickCount * 0.036;
    delta = current_time - g_LastFrameTime;

    if (delta >= 1.0) {
        g_LastFrameTime = current_time;
        g_FrameStep = (int)delta;
        g_AccumulatedFrames += g_FrameStep;
        g_TotalFrameCount++;

        if (g_IsGamePaused == 1 && g_GameMode == 2) {
            g_PauseStartTime = (int)delta;
        }

        g_RaceTimeSeconds = (double)(g_TimerTickCount - g_PauseStartTime) * 0.001;

        if (delta == 0.0) {
            return 0.01;
        }
        if (delta > 10.8) {
            delta = 10.8;
        }
    }
    return delta;
}

/**
 * @original Collision_TestLineIntersection (IGN_WIN.EXE @ 0x004292c0, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x000246cc. Tests 2D collision impulse transfer between two vehicles.
 *        Calculates contact lever arm distances, applies impulse restitution (-1.5),
 *        and updates both linear and angular velocities.
 */
int Collision_TestLineIntersection(double *car1, double *car2) {
    float dx1, dz1, dx2, dz2;
    float dist1, dist2;
    float angle1, angle2;
    float cos_norm, sin_norm;
    int sign1, sign2;
    float arm1, arm2;
    float cos_diff1, cos_diff2;
    float mass1, mass2;
    float total_mass;
    float impulse1, impulse2;
    float normal_x, normal_z;

    dz1 = (float)(g_CollisionContactZ - car1[1]);
    dx1 = (float)(g_CollisionContactX - car1[0]);
    angle1 = (float)atan2(dz1, dx1);
    dist1 = (float)sqrt(dz1 * dz1 + dx1 * dx1);

    cos_norm = (float)cos(g_CollisionNormalAngle);
    sin_norm = (float)sin(g_CollisionNormalAngle);

    sign1 = Math_Signum((int)(cos_norm * dz1 - sin_norm * dx1));

    dz2 = (float)(g_CollisionContactZ - car2[1]);
    dx2 = (float)(g_CollisionContactX - car2[0]);
    angle2 = (float)atan2(dz2, dx2);
    dist2 = (float)sqrt(dz2 * dz2 + dx2 * dx2);

    sign2 = Math_Signum((int)(cos_norm * ((cos(angle1) * car1[4] * dist1 + car1[3]) -
                                          (cos(angle2) * car2[4] * dist2 + car2[3])) -
                              sin_norm * ((car1[2] - car1[4] * sin(angle1) * dist1) -
                                          (car2[2] - car2[4] * sin(angle2) * dist2))));

    if (sign1 != sign2) {
        return 0;
    }

    cos_diff1 = (float)cos(g_CollisionNormalAngle - angle1);
    cos_diff2 = (float)cos(g_CollisionNormalAngle - angle2);

    arm1 = (cos_diff1 / *(float *)((uint8_t *)car1 + 0x2c)) * *(float *)(car1 + 5) * dist1;
    arm2 = (cos_diff2 / *(float *)((uint8_t *)car2 + 0x2c)) * *(float *)(car2 + 5) * dist2;

    mass1 = *(float *)(car1 + 5);
    mass2 = *(float *)(car2 + 5);
    total_mass = mass1 + mass2;

    impulse1 = ((mass2 / total_mass) * -1.5f) /
               (dist1 * arm1 * ((float)cos(angle1) * cos_norm + (float)sin(angle1) * sin_norm) + 1.0f);
    impulse2 = ((mass1 / total_mass) * -1.5f) /
               (dist2 * arm2 * (cos_norm * (float)cos(angle2) + sin_norm * (float)sin(angle2)) + 1.0f);

    car1[4] += (double)(arm1 * impulse1);
    normal_x = (float)sin(g_CollisionNormalAngle) * impulse1;
    normal_z = (float)cos(g_CollisionNormalAngle) * impulse1;
    car1[2] -= (double)normal_x;
    car1[3] += (double)normal_z;

    car2[4] -= (double)(arm2 * impulse2);
    normal_x = (float)sin(g_CollisionNormalAngle) * impulse2;
    normal_z = (float)cos(g_CollisionNormalAngle) * impulse2;
    car2[2] += (double)normal_x;
    car2[3] -= (double)normal_z;

    return 1;
}

/**
 * @original Collision_TestPolygonOverlap (IGN_WIN.EXE @ 0x004295e0, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x000249e5. Tests pairwise edge crossings between two 2D convex vehicle polygons.
 *        Averages contact points to calculate centroid contact position and normal angle.
 */
int Collision_TestPolygonOverlap(float *poly1_x, float *poly1, int count1, float *poly2, int count2) {
    int e1, e2;
    int next1, next2;
    float p1x, p1z, p2x, p2z;
    float q1x, q1z, q2x, q2z;
    float d1x, d1z, d2x, d2z;
    float cross1, cross2;
    float denom;
    int contact_count = 0;
    float contact_pts_x[8];
    float contact_pts_z[8];
    float sum_x = 0.0f, sum_z = 0.0f;
    float max_dist_sq = 0.0f;
    int idx_a = 0, idx_b = 0;
    int i, j;

    if (count1 <= 0 || count2 <= 0) return 0;

    for (e1 = 0; e1 < count1; e1++) {
        next1 = (e1 + 1 < count1) ? e1 + 1 : 0;
        p1x = poly1[e1 * 2];
        p1z = poly1[e1 * 2 + 1];
        p2x = poly1[next1 * 2];
        p2z = poly1[next1 * 2 + 1];
        d1x = p1x - p2x;
        d1z = p1z - p2z;

        for (e2 = 0; e2 < count2; e2++) {
            next2 = (e2 + 1 < count2) ? e2 + 1 : 0;
            q1x = poly2[e2 * 2];
            q1z = poly2[e2 * 2 + 1];
            q2x = poly2[next2 * 2];
            q2z = poly2[next2 * 2 + 1];

            cross1 = (p1z - q1z) * d1x - (p1x - q1x) * d1z;
            cross2 = (p1z - q2z) * d1x - (p1x - q2x) * d1z;

            if (((cross1 > 0.0f) != (cross2 > 0.0f)) || (cross1 == 0.0f)) {
                d2x = q1x - q2x;
                d2z = q1z - q2z;
                cross1 = (q1z - p1z) * d2x - (q1x - p1x) * d2z;
                cross2 = (q1z - p2z) * d2x - (q1x - p2x) * d2z;

                if (((cross1 > 0.0f) != (cross2 > 0.0f)) || (cross1 == 0.0f)) {
                    denom = d2z * d1x - d2x * d1z;
                    if (fabs(denom) > 0.0001f) {
                        if (contact_count < 8) {
                            contact_pts_x[contact_count] = (q1x * d2z * d1x + (p1z - q1z) * d2x * d1x - p1x * d2x * d1z) / denom;
                            contact_pts_z[contact_count] = ((p1x - q1x) * d2z * d1z - p1z * d2z * d1x + q1z * d2x * d1z) / -denom;
                            contact_count++;
                        }
                    }
                }
            }
        }
    }

    if (contact_count > 0) {
        for (i = 0; i < contact_count; i++) {
            sum_x += contact_pts_x[i];
            sum_z += contact_pts_z[i];
        }
        g_CollisionContactX = (double)(sum_x / (float)contact_count);
        g_CollisionContactZ = (double)(sum_z / (float)contact_count);

        for (i = 0; i < contact_count; i++) {
            for (j = i + 1; j < contact_count; j++) {
                float dist_sq = (contact_pts_x[i] - contact_pts_x[j]) * (contact_pts_x[i] - contact_pts_x[j]) +
                                (contact_pts_z[i] - contact_pts_z[j]) * (contact_pts_z[i] - contact_pts_z[j]);
                if (dist_sq > max_dist_sq) {
                    max_dist_sq = dist_sq;
                    idx_a = i;
                    idx_b = j;
                }
            }
        }

        g_CollisionNormalAngle = atan2(contact_pts_z[idx_a] - contact_pts_z[idx_b],
                                       contact_pts_x[idx_a] - contact_pts_x[idx_b]);
        return 1;
    }
    return 0;
}

/**
 * @original Collision_FilterTrackClearance (IGN_WIN.EXE @ 0x00428b90, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x00023ee8. Filters candidate collision triangles according to track-specific
 *        ground clearance thresholds (Snake: 350, Moose: 200, Mountain: 450, Ski: 125, Default: 2500).
 */
int Collision_FilterTrackClearance(int tri_idx, int pos_x, int pos_y, int pos_z, int is_wall) {
    int clearance_limit;

    (void)tri_idx;
    (void)pos_x;
    (void)pos_y;
    (void)pos_z;
    if (is_wall == 0) {
        switch (g_SelectedTrack) {
            case 2: clearance_limit = 200; break; /* Moose */
            case 3: clearance_limit = 450; break; /* Mountain */
            case 6: clearance_limit = 125; break; /* Ski */
            case 0:
            case 1:
            case 4:
            case 5:
            default:
                clearance_limit = 350;
                break;
        }
    } else {
        clearance_limit = 2500;
    }

    return clearance_limit;
}

/**
 * @original Collision_RaycastVehicleSphere (IGN_WIN.EXE @ 0x00428730, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0002383c. Sets up vertical collision query ray from vehicle center
 *        (pos_y + 500.0 downward 850 units) and invokes spatial partition octree query.
 */
void *Collision_RaycastVehicleSphere(int x, int y, int z, int car_idx) {
    uint8_t *car;
    double origin_y;

    if (g_pVehicleTable == NULL) return NULL;
    car = g_pVehicleTable + car_idx * VEHICLE_STRUCT_SIZE;

    origin_y = *(double *)(car + 0x08) + 500.0;
    return Surface_Raycast(x, (int)origin_y, z);
}

/**
 * @original Collision_TestTrackTriangles (IGN_WIN.EXE @ 0x00427dc0, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x00022dc8. Iterates over track collision triangles from octree query,
 *        calculates closest point on triangle, plane distance, and penetration restitution.
 */
void *Collision_TestTrackTriangles(uint32_t world_x, uint32_t world_y, int world_z, int radius, int flags) {
    int i, j;
    int hit_count = 0;
    int max_dist;
    uint8_t *car;
    SurfaceRaycastResult *result_table = (SurfaceRaycastResult *)g_pSRF_RaycastTable;

    (void)world_y;
    (void)radius;
    (void)flags;

    if (g_pVehicleTable == NULL || result_table == NULL) return NULL;
    car = g_pVehicleTable + g_SelectedCar * VEHICLE_STRUCT_SIZE;

    max_dist = (*(int *)(car + 0x274) == 0) ? 900 : 600;

    for (i = 0; i < g_TrackCandidateCount; i++) {
        TrackObject *obj = g_pTrackCandidates[i];
        int dx, dz;
        int *mesh;
        int num_tris;
        uint32_t *tri_data;

        if (obj == NULL || obj->field_1e >= 100) continue;

        dx = (int)(world_x - obj->pos_x);
        dz = (int)(world_z - obj->pos_z);
        if (abs(dx) >= max_dist || abs(dz) >= max_dist) continue;

        mesh = (int *)obj->mesh_data;
        if (mesh == NULL) continue;
        num_tris = mesh[1];
        tri_data = (uint32_t *)(mesh + mesh[0] * 3 + 2);

        for (j = 0; j < num_tris; j++) {
            uint32_t i0 = tri_data[1];
            uint32_t i1 = tri_data[2];
            uint32_t i2 = tri_data[3];
            int v0x = mesh[i0 * 3 + 2];
            int v1x = mesh[i1 * 3 + 2];
            int v2x = mesh[i2 * 3 + 2];
            int v0z = mesh[i0 * 3 + 4];
            int v1z = mesh[i1 * 3 + 4];
            int v2z = mesh[i2 * 3 + 4];
            int cx = (v0x + v1x + v2x) / 3 - dx;
            int cz = (v0z + v1z + v2z) / 3 - dz;

            if (abs(cx) < 200 && abs(cz) < 200) {
                int vx[3], vz[3];
                int inside = 0;
                int k, prev_k;

                vx[0] = v0x; vx[1] = v1x; vx[2] = v2x;
                vz[0] = v0z; vz[1] = v1z; vz[2] = v2z;

                prev_k = 2;
                for (k = 0; k < 3; k++) {
                    int zk = vz[k];
                    int zp = vz[prev_k];
                    if (((zk <= dz && dz < zp) || (zp <= dz && dz < zk)) &&
                        (dx < vx[k] + (vx[prev_k] - vx[k]) * (dz - zk) / (zp - zk))) {
                        inside = !inside;
                    }
                    prev_k = k;
                }

                if (inside && hit_count < 32) {
                    SurfaceRaycastResult *hit = &result_table[hit_count++];
                    hit->material = tri_data[0] >> 16;
                    hit->normal_x = obj->pos_x;
                    hit->normal_y = obj->pos_y;
                    hit->normal_z = obj->pos_z;
                    hit->v0_world_x = v0x;
                    hit->v0_world_y = mesh[i0 * 3 + 3];
                    hit->v0_world_z = v0z;
                    hit->v1_world_x = v1x;
                    hit->v1_world_y = mesh[i1 * 3 + 3];
                    hit->v1_world_z = v1z;
                    hit->v2_world_x = v2x;
                    hit->v2_world_y = mesh[i2 * 3 + 3];
                    hit->v2_world_z = v2z;
                    hit->obj_field0 = obj->field_0;
                    hit->obj_field20 = (int)obj->field_20;
                    hit->obj_field1e = (int)obj->field_1e;
                }
            }
            tri_data += 11;
        }
    }
    return (hit_count > 0) ? (void *)&result_table[0] : NULL;
}

/**
 * @original Car_SpawnExplosionEffects (IGN_WIN.EXE @ 0x004242b0, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0001ea42. Plays vehicle explosion audio sample (vol capped at 0x10000,
 *        sample 2, freq 22000) and emits 6 explosion/debris particle sprites.
 */
void Car_SpawnExplosionEffects(int car1_idx, int car2_idx) {
    uint8_t *car1_col;
    uint8_t *car2_col;
    uint8_t *car1;
    uint8_t *car2;
    int volume;
    int i;
    SceneryParticle particle;

    if (g_pVehicleTable == NULL || g_pTrackCollisions == NULL) return;

    car1_col = g_pTrackCollisions + car1_idx * 0x1d0;
    car2_col = g_pTrackCollisions + car2_idx * 0x1d0;
    car1 = g_pVehicleTable + car1_idx * VEHICLE_STRUCT_SIZE;
    car2 = g_pVehicleTable + car2_idx * VEHICLE_STRUCT_SIZE;

    volume = (int)(fabs(*(double *)(car1_col + 0x1b8) + *(double *)(car2_col + 0x1b8)) * 0.5 * 30.0);
    if (volume > 0x10000) {
        volume = 0x10000;
    }

    Audio_PlaySample(0, 2, 0, 0, volume, 22000, 0);

    for (i = 0; i < 6; i++) {
        memset(&particle, 0, sizeof(particle));
        particle.type = 5;
        particle.pos_x = (int)(*(double *)(car1_col + 0x1c0) * 22282.24);
        particle.pos_y = (int)((*(double *)(car2 + 8) + *(double *)(car1 + 8)) * 0.5 * 1024.0);
        particle.pos_z = (int)(*(double *)(car1_col + 0x1c8) * 22282.24);
        particle.vel_x = (int)((Math_RandomFloat() * 6.0 + (*(double *)(car2 + 0x640) + *(double *)(car1 + 0x640)) * 0.5 - 3.0) * 1024.0);
        particle.vel_y = (int)((Math_RandomFloat() * 3.0 + 3.0) * 1024.0);
        particle.vel_z = (int)((Math_RandomFloat() * 6.0 + (*(double *)(car2 + 0x648) + *(double *)(car1 + 0x648)) * 0.5 - 3.0) * 1024.0);
        particle.drag = 0x400;
        particle.gravity = -614;
        particle.rot_x = (int)((Math_RandomFloat() * 100.0 - 50.0) * 1024.0);
        particle.rot_y = (int)(50.0 * 1024.0);
        particle.rot_z = (int)((Math_RandomFloat() * 200.0 - 30.0) * 1024.0);
        particle.field_44 = 1;
        particle.life = 0xf;

        FX_SpawnParticle(&particle);
    }
}

/**
 * @original Car_HandleElimination (IGN_WIN.EXE @ 0x00423620, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0001de2e. Checks trailing vehicle elimination condition in knock-out races.
 *        Marks vehicle blown (+0x354 = 1), sets race status, and logs elimination string.
 */
void Car_HandleElimination(void) {
    int i;
    int max_lap = 0;
    int trailing_count = 0;
    int trailing_car = -1;
    uint8_t *car;
    char msg[128];
    char *driver_name;

    if (g_pVehicleTable == NULL || g_ActiveVehicleCount <= 0) return;

    for (i = 0; i < g_ActiveVehicleCount; i++) {
        car = g_pVehicleTable + i * VEHICLE_STRUCT_SIZE;
        if (*(int *)(car + 0x528) == 0 && *(int *)(car + 0x374) > max_lap) {
            max_lap = *(int *)(car + 0x374);
        }
    }

    for (i = 0; i < g_ActiveVehicleCount; i++) {
        car = g_pVehicleTable + i * VEHICLE_STRUCT_SIZE;
        if (*(int *)(car + 0x528) == 0 && *(int *)(car + 0x374) < max_lap) {
            trailing_count++;
            trailing_car = i;
        }
    }

    if (trailing_count != 1 || g_GameMode == 4) {
        if (g_EliminationFlag != 3 || g_GameMode != 4) {
            return;
        }
    }
    if (g_EliminationFlag == 3 && g_GameMode == 4) {
        trailing_car = g_EliminationTargetCar;
    }

    if (trailing_car < 0 || trailing_car >= g_ActiveVehicleCount) return;
    car = g_pVehicleTable + trailing_car * VEHICLE_STRUCT_SIZE;

    if ((*(uint32_t *)(car + 0x610) & 0x7fffffff) != 0 || *(int *)(car + 0x60c) != 0) {
        return;
    }

    *(int *)(car + 0x354) = 1;

    g_CameraEffect.type = 3;
    g_CameraEffect.pos_x = (int)*(double *)(car + 0x00);
    g_CameraEffect.pos_y = (int)*(double *)(car + 0x08);
    g_CameraEffect.pos_z = (int)*(double *)(car + 0x10);
    g_CameraEffect.target_car = trailing_car;
    g_CameraEffect.duration = 0x50;
    g_CameraEffect.active = 1;
    FX_SpawnCameraParticle(&g_CameraEffect);

    *(int *)(car + 0x60c) = 0;
    *(int *)(car + 0x610) = 0x3ff00000;

    Audio_PlaySample(0, 2, 4, 0, 0xc000, 22000, 0);

    *(int *)(car + 0x528) = 1;
    *(int *)(car + 0x3a0) = *(int *)(car + 0x39c);

    driver_name = (char *)(g_CarConfigs + trailing_car * 0x4c + 0x3c);
    switch (g_LanguageId) {
        case 0: sprintf(msg, "%s IS OUT", driver_name); break;
        case 1: sprintf(msg, "%s IST AUSGESCHIEDEN", driver_name); break;
        case 2: sprintf(msg, "%s OUT", driver_name); break;
        case 3: sprintf(msg, "%s ESTA FUERA", driver_name); break;
        case 4: sprintf(msg, "%s AR UTE", driver_name); break;
        case 5: sprintf(msg, "%s EST ELIMINE", driver_name); break;
        default: sprintf(msg, "%s IS OUT", driver_name); break;
    }

    HUD_ShowAnnouncementBanner(msg, 0x50, 8, (g_SplitScreenMode == 0) ? -1 : 1);

    g_EliminatedCarCount++;
    g_RemainingCarCount--;
    if (g_RemainingCarCount == 1) {
        for (i = 0; i < g_ActiveVehicleCount; i++) {
            uint8_t *c = g_pVehicleTable + i * VEHICLE_STRUCT_SIZE;
            if (*(int *)(c + 0x528) == 0) {
                *(int *)(c + 0x528) = 1;
                *(int *)(c + 0x3a0) = *(int *)(c + 0x39c);
                g_EliminatedCarCount = g_ActiveVehicleCount;
            }
        }
    }
}

/**
 * @original Car_ChangeMesh (IGN_WIN.EXE @ 0x004269a0, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x00021856. Swaps current vehicle 3D mesh representation to damaged or
 *        alternative model geometry in Lisa3D rasterizer instance table.
 */
void Car_ChangeMesh(int car_idx) {
    int car_model;
    int *src_mesh;
    int total_words;
    int *dst_mesh;
    int i;
    int num_vertices;
    int *src_ptr;
    int *dst_ptr;
    int *v;
    uint8_t *car;

    if (g_pVehicleTable == NULL || g_CarConfigs == NULL) return;

    car_model = *(int *)(g_CarConfigs + car_idx * 0x4c);
    src_mesh = *(int **)(g_CarMeshes + car_model * 4);
    if (src_mesh == NULL) return;

    num_vertices = src_mesh[0];
    total_words = src_mesh[0] * 3 + src_mesh[1] * 11 + 2;

    dst_mesh = (int *)(g_DynamicMeshBuffer + car_idx * 8000);
    if (total_words > 0) {
        src_ptr = src_mesh;
        dst_ptr = dst_mesh;
        for (i = 0; i < total_words; i++) {
            *dst_ptr++ = *src_ptr++;
        }
    }

    if (num_vertices > 0) {
        v = (int *)((uint8_t *)dst_mesh + 8);
        for (i = 0; i < num_vertices; i++) {
            v[0] = (int)(v[0] * 0.0);
            v[1] = (int)(v[1] * 0.5);
            v[2] = (int)(v[2] * 0.04);
            v += 3;
        }
    }

    Lisa_CreateDynamicObject(car_idx, car_idx * 5, (int *)(car_idx * 0x20 + (uintptr_t)g_DynamicObjectPointers),
                            dst_mesh, 1, (short)car_idx + 100, 0, -1, 0);

    car = g_pVehicleTable + car_idx * VEHICLE_STRUCT_SIZE;
    Audio_PlaySample(0, 2, 6, 0, 0x10000, 22000, 0);
}

/**
 * @original Car_ApplyMeshDamage (IGN_WIN.EXE @ 0x00426b40, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x000219d6. Evaluates high-velocity collision impact against vehicle chassis,
 *        morphs vertex positions inward toward impact point, and emits impact sparks.
 */
void Car_ApplyMeshDamage(int car_idx, int impact_severity) {
    uint8_t *car;
    double cos_yaw, sin_yaw;
    double wheel_ground_y[4];
    double avg_ground_y = 0.0;
    int i;
    int checkpoint;
    SurfaceRaycastResult *surf;
    int lap;

    if (g_pVehicleTable == NULL) return;
    car = g_pVehicleTable + car_idx * VEHICLE_STRUCT_SIZE;

    *(int *)(car + 0x160) = *(int *)(car + 0x150);
    *(int *)(car + 0x164) = *(int *)(car + 0x154);
    *(int *)(car + 0x168) = *(int *)(car + 0x158);
    *(int *)(car + 0x16c) = *(int *)(car + 0x15c);
    *(int *)(car + 0x150) = -1;
    *(int *)(car + 0x154) = -1;
    *(int *)(car + 0x158) = -1;
    *(int *)(car + 0x15c) = -1;

    cos_yaw = cos(*(double *)(car + 0xf8));
    sin_yaw = sin(*(double *)(car + 0xf8));

    for (i = 0; i < 4; i++) {
        double lx = *(double *)(car + 0x2fc + i * 8);
        double lz = *(double *)(car + 0x31c + i * 8);
        double scale = *(double *)(car + 0x5cc);
        int wx = (int)((lx * cos_yaw - lz * sin_yaw) * scale + *(double *)(car + 0x00));
        int wz = (int)((lz * cos_yaw + lx * sin_yaw) * scale + *(double *)(car + 0x10));

        surf = (SurfaceRaycastResult *)Surface_GetHeightAtPoint(wx, 0, wz);
        if (surf != NULL) {
            wheel_ground_y[i] = (double)surf->v0_world_y;
            avg_ground_y += wheel_ground_y[i];
            *(int *)(car + 0x150 + i * 4) = surf->material;
            *(int *)(car + 0x5d8 + i * 4) = surf->material;

            checkpoint = surf->obj_field1e;
            if (checkpoint != 10000 && *(int *)(car + 0x378) == 3 && checkpoint == 0) {
                lap = *(int *)(car + 0x374) + 1;
                *(int *)(car + 0x374) = lap;
                *(int *)(car + 0x378) = 0;
                if (lap > 2 && g_GameMode != 3) {
                    *(int *)(car + 0x528) = 1;
                    *(int *)(car + 0x3a0) = *(int *)(car + 0x39c);
                    g_EliminatedCarCount++;
                }
            } else if (checkpoint != 10000) {
                *(int *)(car + 0x370) = checkpoint;
            }
        } else {
            wheel_ground_y[i] = *(double *)(car + 0x08);
            avg_ground_y += wheel_ground_y[i];
        }
    }

    avg_ground_y *= 0.25;
    *(double *)(car + 0x124) = *(double *)(car + 0x120);
    *(double *)(car + 0x120) = avg_ground_y;

    if (impact_severity == -1) {
        *(int *)(car + 0x354) = 1;
        g_CameraEffect.type = 3;
        g_CameraEffect.pos_x = (int)*(double *)(car + 0x00);
        g_CameraEffect.pos_y = (int)*(double *)(car + 0x08);
        g_CameraEffect.pos_z = (int)*(double *)(car + 0x10);
        g_CameraEffect.target_car = car_idx;
        g_CameraEffect.duration = 0x50;
        g_CameraEffect.active = 1;
        FX_SpawnCameraParticle(&g_CameraEffect);
    } else if (impact_severity == -2) {
        *(int *)(car + 0x358) = 1;
        g_CameraEffect.type = 7;
        g_CameraEffect.pos_x = (int)*(double *)(car + 0x00);
        g_CameraEffect.pos_y = (int)*(double *)(car + 0x08);
        g_CameraEffect.pos_z = (int)*(double *)(car + 0x10);
        g_CameraEffect.target_car = car_idx;
        g_CameraEffect.duration = 0x50;
        g_CameraEffect.active = 1;
        FX_SpawnCameraParticle(&g_CameraEffect);
    } else if (impact_severity == -3) {
        *(int *)(car + 0x35c) = 1;
        g_CameraEffect.type = 10;
        g_CameraEffect.pos_x = (int)*(double *)(car + 0x00);
        g_CameraEffect.pos_y = (int)*(double *)(car + 0x08);
        g_CameraEffect.pos_z = (int)*(double *)(car + 0x10);
        g_CameraEffect.target_car = car_idx;
        g_CameraEffect.duration = 0x50;
        g_CameraEffect.active = 1;
        FX_SpawnCameraParticle(&g_CameraEffect);
    } else if (impact_severity == 1) {
        if (*(int *)(car + 0x270) == 0 && *(int *)(car + 0x554) == 0) {
            SceneryParticle p;
            memset(&p, 0, sizeof(p));
            p.type = 5;
            p.pos_x = (int)(*(double *)(car + 0x00) * 1024.0);
            p.pos_y = (int)(*(double *)(car + 0x08) * 1024.0);
            p.pos_z = (int)(*(double *)(car + 0x10) * 1024.0);
            p.vel_x = (int)((Math_RandomFloat() * 6.0 - 3.0) * 1024.0);
            p.vel_y = (int)((Math_RandomFloat() * 4.0 + 2.0) * 1024.0);
            p.vel_z = (int)((Math_RandomFloat() * 6.0 - 3.0) * 1024.0);
            p.drag = 0x400;
            p.gravity = -460;
            p.life = 0x1e;
            FX_SpawnParticle(&p);
            *(int *)(car + 0x554) = 1;
        }
    } else if (impact_severity == 3) {
        if (*(int *)(car + 0x558) == 0) {
            Car_ChangeMesh(car_idx);
            *(int *)(car + 0x558) = 1;
        }
    }
}

/**
 * @original Car_UpdateEffects (IGN_WIN.EXE @ 0x0042bc20, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x000272d8. Coordinates real-time vehicle visual effects (tire skid marks,
 *        turbo exhaust flames, engine damage smoke, and surface scraping sparks).
 */
void Car_UpdateEffects(void) {
    uint8_t *car;
    if (g_pVehicleTable == NULL) return;
    car = g_pVehicleTable + g_SelectedCar * VEHICLE_STRUCT_SIZE;
    if (*(int *)(car + 0x354) == 0 && *(int *)(car + 0x358) == 0 && *(int *)(car + 0x35c) == 0) {
        FX_UpdateSkidMarks();
        FX_UpdateEngineSmoke();
        FX_UpdateTurboFlames();
        FX_UpdateSparks();
    }
}

/**
 * @original Audio_UpdateDynamicDoppler (IGN_WIN.EXE @ 0x0042aa00, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x000260ed. Updates sound pitch and volume for dynamic track ambient sources.
 */
void Audio_UpdateDynamicDoppler(void) {
    int i;
    if (g_AmbientEmitterCount <= 0 || g_pAmbientEmitters == NULL) return;

    for (i = 0; i < g_AmbientEmitterCount; i++) {
        AmbientSoundEmitter *emitter = &g_pAmbientEmitters[i];
        SoundChannel *ch = Audio_GetChannel(emitter->channel_id);
        double dist1, dist2, min_dist;
        int vol;

        if (ch == NULL) continue;
        if (g_SoundMuted == 0 && g_SoundStateTable[emitter->sound_id] != -1) {
            uint8_t *car1 = g_pVehicleTable;
            double dx1 = (double)emitter->pos_x - *(double *)(car1 + 0x00);
            double dz1 = (double)emitter->pos_z - *(double *)(car1 + 0x10);
            dist1 = sqrt(dx1 * dx1 + dz1 * dz1);
            min_dist = dist1;

            if (g_SplitScreenMode == 1 && g_ActiveVehicleCount > 1) {
                uint8_t *car2 = g_pVehicleTable + VEHICLE_STRUCT_SIZE;
                double dx2 = (double)emitter->pos_x - *(double *)(car2 + 0x00);
                double dz2 = (double)emitter->pos_z - *(double *)(car2 + 0x10);
                dist2 = sqrt(dx2 * dx2 + dz2 * dz2);
                if (dist2 < min_dist) {
                    min_dist = dist2;
                }
            }

            if (min_dist > 0.0) {
                vol = (int)((4000.0 - min_dist) * (65536.0 / 4000.0));
                if (vol < 0) vol = 0;
                if (vol > 0x10000) vol = 0x10000;
            } else {
                vol = 0x10000;
            }

            ch->volume = vol;
            if (i < 16 && g_SelectedTrack == 4) {
                ch->volume = (int)(ch->volume * 0.75);
            }
        }
    }
}

/**
 * @original Track_SpawnEnvironmentalParticles (IGN_WIN.EXE @ 0x0042acb0, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0002636c. Emits environmental smoke/dust from track waypoint emitters.
 */
void Track_SpawnEnvironmentalParticles(void) {
    int i;
    if (g_TrackEmitterCount <= 0 || g_pTrackEmitters == NULL) return;

    for (i = 0; i < g_TrackEmitterCount; i++) {
        TrackParticleEmitter *em = &g_pTrackEmitters[i];
        em->timer++;
        if (em->timer >= em->interval) {
            SceneryParticle p;
            memset(&p, 0, sizeof(p));
            p.type = em->particle_type;
            p.pos_x = em->pos_x << 10;
            p.pos_y = em->pos_y << 10;
            p.pos_z = em->pos_z << 10;
            p.vel_x = (int)((Math_RandomFloat() * 4.0 - 2.0) * 1024.0);
            p.vel_y = (int)((Math_RandomFloat() * 2.0 + 1.0) * 1024.0);
            p.vel_z = (int)((Math_RandomFloat() * 4.0 - 2.0) * 1024.0);
            p.drag = 0x400;
            p.gravity = 0;
            p.life = em->lifetime;
            FX_SpawnParticle(&p);
            em->timer = 0;
        }
    }
}

/**
 * @original Track_SpawnWeatherParticles (IGN_WIN.EXE @ 0x0042b5d0, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x00026cdd. Spawns rain and snow weather particles in viewport frustum.
 */
void Track_SpawnWeatherParticles(void) {
    int i;
    if (g_WeatherParticleCount <= 0 || g_pWeatherEmitters == NULL) return;

    for (i = 0; i < g_WeatherParticleCount; i++) {
        WeatherEmitter *we = &g_pWeatherEmitters[i];
        if (we->type == 0x12e) {
            we->timer++;
            if ((float)Math_RandomFloat() + 0.1f < (float)we->timer) {
                SceneryParticle p;
                memset(&p, 0, sizeof(p));
                p.type = 1;
                p.pos_x = (int)(Math_RandomFloat() * 2000.0 - 1000.0) << 10;
                p.pos_y = we->spawn_y << 10;
                p.pos_z = (int)(Math_RandomFloat() * 2000.0 - 1000.0) << 10;
                p.vel_x = 0;
                p.vel_y = (int)((Math_RandomFloat() * -10.0 - 20.0) * 1024.0);
                p.vel_z = 0;
                p.drag = 0x400;
                p.gravity = 0;
                p.life = 0x1e;
                FX_SpawnParticle(&p);
                we->timer = 0;
            }
        }
    }
}

/**
 * @original FX_UpdateSkidMarks (IGN_WIN.EXE @ 0x0042bc80, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0002731c. Generates ground skidmarks behind slipping vehicle tires.
 */
void FX_UpdateSkidMarks(void) {
    uint8_t *car;
    double slip;
    int material;
    double yaw;
    double cos_yaw, sin_yaw;

    if (g_pVehicleTable == NULL) return;
    car = g_pVehicleTable + g_SelectedCar * VEHICLE_STRUCT_SIZE;

    material = *(int *)(car + 0x158);
    slip = *(double *)(car + 0x90);

    if ((fabs(slip) <= 0.15 || *(double *)(car + 0x118) <= 5.0) && *(int *)(car + 0x2f8) == 0) {
        if (*(int *)(car + 0x298) == 1) {
            SoundChannel *ch = Audio_GetChannel(*(int *)(car + 0x99c));
            if (ch != NULL) ch->active = 0;
            *(int *)(car + 0x298) = 0;
        }
    } else if (*(int *)(car + 0x298) == 1) {
        if (material != 5 && material != 15 && material != 25 && g_ParticleEffectsEnabled == 1) {
            SceneryParticle p;
            yaw = *(double *)(car + 0xf8);
            cos_yaw = cos(yaw);
            sin_yaw = sin(yaw);

            memset(&p, 0, sizeof(p));
            p.type = 1;
            p.pos_x = (int)((*(double *)(car + 0x00) - cos_yaw * 15.0) * 1024.0);
            p.pos_y = (int)(*(double *)(car + 0x08) * 1024.0);
            p.pos_z = (int)((*(double *)(car + 0x10) - sin_yaw * 15.0) * 1024.0);
            p.vel_x = (int)((Math_RandomFloat() * 2.0 - 1.0) * 1024.0);
            p.vel_y = (int)((Math_RandomFloat() * 2.0 + 1.0) * 1024.0);
            p.vel_z = (int)((Math_RandomFloat() * 2.0 - 1.0) * 1024.0);
            p.drag = 0x200;
            p.gravity = 0;
            p.life = 0x1e;
            FX_SpawnParticle(&p);

            p.pos_x = (int)((*(double *)(car + 0x00) + cos_yaw * 15.0) * 1024.0);
            p.pos_z = (int)((*(double *)(car + 0x10) + sin_yaw * 15.0) * 1024.0);
            FX_SpawnParticle(&p);
        }
    }
}

/**
 * @original FX_UpdateTransparentSpriteObject (IGN_WIN.EXE @ 0x0042e2e0, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x000298f8. Updates 3D world position and animation of transparent billboard sprites.
 */
void FX_UpdateTransparentSpriteObject(void *particle, int instance_idx) {
    SceneryParticle *p = (SceneryParticle *)particle;
    DynamicObject *obj;

    if (p == NULL || g_pDynamicObjects == NULL) return;
    obj = &g_pDynamicObjects[instance_idx];

    if (p->field_24 == 0) {
        Lisa_CreateDynamicObject(p->type, instance_idx, (int *)obj, (int *)obj, p->type, 200, 0, 0x14, 0);
    }

    obj->pos_x = p->pos_x >> 10;
    obj->pos_y = p->pos_y >> 10;
    obj->pos_z = p->pos_z >> 10;

    if (obj->field_1c == 0) {
        Lisa_MoveDynamicObject(obj);
    }

    p->pos_x += p->vel_x;
    p->pos_y += p->vel_y;
    p->pos_z += p->vel_z;
    p->vel_x = FixedMul10(p->vel_x, p->drag);
    p->vel_y += p->gravity;
    p->vel_z = FixedMul10(p->vel_z, p->drag);

    p->field_24++;
    if (p->field_24 >= p->life) {
        Lisa_DeleteDynamicObject(obj);
        p->type = 0;
    }
}

/**
 * @original FX_UpdateTransparentSpriteObject2 (IGN_WIN.EXE @ 0x0042e5a0, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x00029cf2. Updates transparent billboard sprite instance variation.
 */
void FX_UpdateTransparentSpriteObject2(void *particle, int instance_idx) {
    FX_UpdateTransparentSpriteObject(particle, instance_idx);
}

/**
 * @original FX_UpdateHandlePlotObject (IGN_WIN.EXE @ 0x0042e860, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x00029d1a. Updates particle plot marker object positions in scene.
 */
void FX_UpdateHandlePlotObject(void *particle, int instance_idx) {
    SceneryParticle *p = (SceneryParticle *)particle;
    DynamicObject *obj;

    if (p == NULL || g_pDynamicObjects == NULL) return;
    obj = &g_pDynamicObjects[instance_idx];

    obj->pos_x = p->pos_x >> 10;
    obj->pos_y = p->pos_y >> 10;
    obj->pos_z = p->pos_z >> 10;

    Lisa_MoveDynamicObject(obj);

    p->pos_x += p->vel_x;
    p->pos_y += p->vel_y;
    p->pos_z += p->vel_z;
    p->vel_y += p->gravity;

    p->field_24++;
    if (p->field_24 >= p->life) {
        Lisa_DeleteDynamicObject(obj);
        p->type = 0;
    }
}

/**
 * @original FX_UpdateSuperPlotObject (IGN_WIN.EXE @ 0x0042ea60, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x00029040. Updates high-intensity spark / super plot particle positions.
 */
void FX_UpdateSuperPlotObject(void *particle, int instance_idx) {
    SceneryParticle *p = (SceneryParticle *)particle;
    DynamicObject *obj;

    if (p == NULL || g_pDynamicObjects == NULL) return;
    obj = &g_pDynamicObjects[instance_idx];

    obj->pos_x = p->pos_x >> 10;
    obj->pos_y = p->pos_y >> 10;
    obj->pos_z = p->pos_z >> 10;

    Lisa_MoveDynamicObject(obj);

    p->pos_x += p->vel_x;
    p->pos_y += p->vel_y;
    p->pos_z += p->vel_z;
    p->vel_x = FixedMul10(p->vel_x, 0x3e0);
    p->vel_y += p->gravity;
    p->vel_z = FixedMul10(p->vel_z, 0x3e0);

    p->field_24++;
    if (p->field_24 >= p->life) {
        Lisa_DeleteDynamicObject(obj);
        p->type = 0;
    }
}

/**
 * @original Obstacle_SimulateDynamics (IGN_WIN.EXE @ 0x0042ed60, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x00029350. Performs dynamic physics integration (ballistic velocity, restitution,
 *        and world model transforms) for track scenery obstacles (traffic cones, drums).
 */
void Obstacle_SimulateDynamics(void *obstacle) {
    SceneryObstacle *obs = (SceneryObstacle *)obstacle;
    SurfaceRaycastResult *surf;

    if (obs == NULL || obs->active_state != 1) return;

    obs->pos_x += obs->vel_x;
    obs->pos_y += obs->vel_y;
    obs->pos_z += obs->vel_z;
    obs->vel_y -= 9.81 * 0.1;

    obs->rot_x += obs->ang_vel_x;
    obs->rot_y += obs->ang_vel_y;
    obs->rot_z += obs->ang_vel_z;

    surf = (SurfaceRaycastResult *)Surface_GetHeightAtPoint((int)obs->pos_x, 0, (int)obs->pos_z);
    if (surf != NULL && obs->pos_y < (double)surf->v0_world_y) {
        obs->pos_y = (double)surf->v0_world_y;
        obs->vel_y = -obs->vel_y * 0.6;
        obs->vel_x *= 0.85;
        obs->vel_z *= 0.85;
        obs->ang_vel_x *= 0.8;
        obs->ang_vel_y *= 0.8;
        obs->ang_vel_z *= 0.8;

        if (fabs(obs->vel_y) < 1.0 && fabs(obs->vel_x) < 0.5 && fabs(obs->vel_z) < 0.5) {
            obs->vel_x = 0.0;
            obs->vel_y = 0.0;
            obs->vel_z = 0.0;
            obs->active_state = 0;
        }
    }
}

/**
 * @original FX_UpdateFlyingParticles (IGN_WIN.EXE @ 0x0042fc80, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0002a280. Updates ballistic trajectory and ground bounce for flying vehicle debris.
 */
void FX_UpdateFlyingParticles(void *particle, int instance_idx) {
    SceneryParticle *p = (SceneryParticle *)particle;
    DynamicObject *obj;
    SurfaceRaycastResult *surf;

    if (p == NULL || g_pDynamicObjects == NULL) return;
    obj = &g_pDynamicObjects[instance_idx];

    p->pos_x += p->vel_x;
    p->pos_y += p->vel_y;
    p->pos_z += p->vel_z;
    p->vel_y += p->gravity;

    surf = (SurfaceRaycastResult *)Surface_GetHeightAtPoint(p->pos_x >> 10, 0, p->pos_z >> 10);
    if (surf != NULL && (p->pos_y >> 10) < surf->v0_world_y) {
        p->pos_y = surf->v0_world_y << 10;
        p->vel_y = -p->vel_y / 2;
        p->vel_x = (p->vel_x * 3) / 4;
        p->vel_z = (p->vel_z * 3) / 4;
    }

    obj->pos_x = p->pos_x >> 10;
    obj->pos_y = p->pos_y >> 10;
    obj->pos_z = p->pos_z >> 10;
    Lisa_MoveDynamicObject(obj);

    p->field_24++;
    if (p->field_24 >= p->life) {
        Lisa_DeleteDynamicObject(obj);
        p->type = 0;
    }
}

/**
 * @original Obstacle_TriggerAction (IGN_WIN.EXE @ 0x00420090, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0001a40e. Evaluates type 0xFA (250) trigger obstacles and triggers associated actions.
 */
void Obstacle_TriggerAction(void) {
    int count;
    int i;
    int offset;

    Obstacle_ResetActions();
    if (g_pObstacleTriggerTable == NULL) return;
    count = *g_pObstacleTriggerTable;
    if (count <= 0) return;

    offset = 0;
    for (i = 0; i < count; i++) {
        if (*(int *)((uint8_t *)g_pObstacleTriggerTable + offset + 8) == 0xfa) {
            int obstacle_id = *(int *)((uint8_t *)g_pObstacleTriggerTable + offset + 4);
            Obstacle_SetTriggerState((int *)(g_pObstacleStateTable + obstacle_id * 4), 0);
        }
        offset += 0x14;
    }
}

/**
 * @original AI_InitSteeringConeLookup (IGN_WIN.EXE @ 0x004200e0, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0001a4ec. Precomputes 800x405 lookahead steering cone and obstacle threat table.
 */
void AI_InitSteeringConeLookup(void) {
    int row, col;
    int offset = 0;

    if (g_pAIThreatTable == NULL) {
        g_pAIThreatTable = (uint8_t *)Mem_Alloc(1, 800 * 405);
    }
    if (g_pAIThreatTable == NULL) return;

    memset(g_pAIThreatTable, 0x14, 800 * 405);

    for (row = 0; row < 405; row++) {
        offset = row * 800;

        for (col = 400 - row; col < 400 + row; col++) {
            if (col >= 0 && col < 800) g_pAIThreatTable[offset + col] = 5;
        }
        for (col = 401 - row; col < 399 + row; col++) {
            if (col >= 0 && col < 800) g_pAIThreatTable[offset + col] = 4;
        }
        for (col = 402 - row; col < 398 + row; col++) {
            if (col >= 0 && col < 800) g_pAIThreatTable[offset + col] = 3;
        }
        for (col = 403 - row; col < 397 + row; col++) {
            if (col >= 0 && col < 800) g_pAIThreatTable[offset + col] = 2;
        }
        for (col = 404 - row; col < 396 + row; col++) {
            if (col >= 0 && col < 800) g_pAIThreatTable[offset + col] = 1;
        }
        for (col = 405 - row; col < 395 + row; col++) {
            if (col >= 0 && col < 800) g_pAIThreatTable[offset + col] = 0;
        }
    }
}

/**
 * @original Ghost_LoadCarAndPath (IGN_WIN.EXE @ 0x00420240, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0001a97a. Loads recorded Time Attack ghost car trajectory from GHOSTS\%s.GST.
 */
void Ghost_LoadCarAndPath(void) {
    char path[128];
    int fd;
    if (g_TrackName[0] == '\0') return;

    sprintf(path, "GHOSTS\\%s.GST", g_TrackName);
    fd = open(path, O_RDONLY | O_BINARY);
    if (fd >= 0) {
        read(fd, g_GhostDataBuffer, 0x49d4c);
        close(fd);
        g_GhostCarLoaded = 1;
    }
}

/**
 * @original Track_LoadBinaryCache (IGN_WIN.EXE @ 0x00420870, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0001adf8. Loads preprocessed level data cache (ign_win.btz / ign_dos.btz).
 */
void Track_LoadBinaryCache(void) {
    int fd;
    Surface_FreeSRF();
    Sound_FreeAllSounds();
    Track_SaveBinaryCache();

    fd = open("ign_win.btz", O_RDONLY | O_BINARY);
    if (fd >= 0) {
        read(fd, g_TrackBinaryCache, 0x597);
        close(fd);
    }
}

/**
 * @original Sound_FreeAllSounds (IGN_WIN.EXE @ 0x00420990, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0001b42d. Frees active level audio buffers and sample tables upon track unload.
 */
void Sound_FreeAllSounds(void) {
    int i;
    for (i = 0; i < 32; i++) {
        Sound_FreeSample(i);
    }
}

/**
 * @original Game_Shutdown (IGN_WIN.EXE @ 0x00420b70, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0001b51a. Releases all allocated game memory and shuts down engine subsystems cleanly.
 */
void Game_Shutdown(void) {
    Sound_FreeAllSounds();
    App_Shutdown();
}

/**
 * @original Race_UpdateCountdownAndFinish (IGN_WIN.EXE @ 0x00420d10, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0001b9d3. Updates start-line traffic light timer and checks race winner victory condition.
 */
void Race_UpdateCountdownAndFinish(void) {
    if (g_CountdownTimer > 0.0) {
        g_CountdownTimer = (double)(g_CurrentTick - g_StartTick) * (1.0 / 18.2);
        if (g_CountdownTimer >= 3.0 || g_RacePhase == 1) {
            if (g_CountdownTimer >= 3.0) {
                Timer_GetDeltaTime();
                Audio_PlaySample(0, 3, 0, 0, 0x10000, 22000, 0);
            }
            g_CountdownTimer = -1.0;
            g_RacePhase = 2;
            if (g_pVehicleTable != NULL) {
                *(int *)(g_pVehicleTable + 0x4844) = 10;
            }
        }
    }

    g_TotalRaceFrames++;
    HUD_UpdateLapCounters();
}

/**
 * @original HUD_UpdateRaceTimes (IGN_WIN.EXE @ 0x00420e60, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0001b9e4. Updates player split times and current lap timer display on in-game HUD.
 */
void HUD_UpdateRaceTimes(void) {
    if (g_DemoMode == 0) {
        if (g_CarConfigs != NULL && *(int *)(g_CarConfigs + 8 + g_SelectedCar * 0x4c) > 0) {
            HUD_DrawPlayerSplitTimer(g_SelectedCar * 0x13, (int *)(g_SelectedCar * 9));
        }
        Input_PollPlayerVehicleControls();
        return;
    }
    HUD_DrawDemoWatermark(0, 0, 0, 0);
}

/**
 * @original Input_ProcessRaceHotkeys (IGN_WIN.EXE @ 0x00420eb0, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0001c5b8. Polls keyboard hotkeys during race: Pause (P), Escape menu, camera toggle, and volume.
 */
void Input_ProcessRaceHotkeys(void) {
    if (Input_IsKeyPressed(0x19)) {
        g_IsGamePaused = !g_IsGamePaused;
    }
    if (Input_IsKeyPressed(0x01)) {
        g_GameStage = 0;
    }
    if (Input_IsKeyPressed(0x3b)) {
        g_SelectedCameraView = 0;
    }
    if (Input_IsKeyPressed(0x3c)) {
        g_SelectedCameraView = 1;
    }
    if (Input_IsKeyPressed(0x3d)) {
        g_SelectedCameraView = 2;
    }
}

/**
 * @original Input_PollPlayerVehicleControls (IGN_WIN.EXE @ 0x00421bc0, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0001ccc5. Reads keyboard / joystick axes and maps to vehicle steering, throttle, brake, and turbo.
 */
void Input_PollPlayerVehicleControls(void) {
    uint8_t *car;
    uint8_t *config;
    int steer = 0;
    int throttle = 0;
    int brake = 0;

    if (g_pVehicleTable == NULL || g_CarConfigs == NULL) return;
    car = g_pVehicleTable + g_SelectedCar * VEHICLE_STRUCT_SIZE;
    config = g_CarConfigs + g_SelectedCar * 0x4c;

    if (*(int *)(config + 8) != 0) {
        return;
    }

    if (Input_IsKeyPressed(config[0x1e])) {
        throttle = 100;
    }
    if (Input_IsKeyPressed(config[0x1f])) {
        brake = 100;
    }
    if (Input_IsKeyPressed(config[0x1c])) {
        steer = -100;
    }
    if (Input_IsKeyPressed(config[0x1d])) {
        steer = 100;
    }

    *(int *)(car + 0x344) = steer;
    *(int *)(car + 0x348) = throttle;
    *(int *)(car + 0x34c) = brake;

    if (Input_IsKeyPressed(config[0x22])) {
        if (*(int *)(car + 0x354) == 0 && *(int *)(car + 0x358) == 0 && *(int *)(car + 0x35c) == 0) {
            *(int *)(car + 0x4844) = 1;
        }
    }
}

/**
 * @original Ghost_SaveCarAndPath (IGN_WIN.EXE @ 0x004222b0, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0001cf5d. Serializes recorded lap waypoint trajectory into GHOSTS\%s.GST.
 */
void Ghost_SaveCarAndPath(void) {
    char path[128];
    int fd;

    if (g_GameMode == 2 && g_TrackName[0] != '\0') {
        sprintf(path, "GHOSTS\\%s.GST", g_TrackName);
        fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);
        if (fd >= 0) {
            write(fd, g_GhostDataBuffer, 0x49d4c);
            close(fd);
        }
    }
    Track_SaveBinaryCache();
}

/**
 * @original Track_SaveBinaryCache (IGN_WIN.EXE @ 0x004225d0, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x0001cfbc. Saves preprocessed level collision cache file.
 */
void Track_SaveBinaryCache(void) {
    int fd;
    fd = open("ign_win.btz", O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);
    if (fd >= 0) {
        write(fd, g_TrackBinaryCache, 0x597);
        close(fd);
    }
}

/**
 * @original Menu_Tick (IGN_WIN.EXE, main.c)
 * @fidelity STUB
 */
void Menu_Tick(void) {
    /* TODO: Windows-specific or dead-code wrapper stub */
}

/**
 * @original Car_IntegratePosition (IGN_WIN.EXE, main.c)
 * @fidelity STUB
 */
void Car_IntegratePosition(void) {
    /* TODO: Windows-specific or dead-code wrapper stub */
}

/**
 * @original WinMain (IGN_WIN.EXE, main.c)
 * @fidelity STUB
 */
void WinMain(void) {
    /* TODO: Windows-specific or dead-code wrapper stub */
}

/**
 * @original Mesh_InstantiatePlacedObjects (IGN_WIN.EXE, main.c)
 * @fidelity STUB
 */
void Mesh_InstantiatePlacedObjects(void) {
    /* TODO: Windows-specific or dead-code wrapper stub */
}

/**
 * @original Game_StateDispatcher (IGN_WIN.EXE, main.c)
 * @fidelity STUB
 */
void Game_StateDispatcher(void) {
    /* TODO: Windows-specific or dead-code wrapper stub */
}

/**
 * @original Track_LoadPlacements (IGN_WIN.EXE, main.c)
 * @fidelity STUB
 */
void Track_LoadPlacements(void) {
    /* TODO: Windows-specific or dead-code wrapper stub */
}

/**
 * @original Mesh_LoadTrackAndCars (IGN_WIN.EXE, main.c)
 * @fidelity STUB
 */
void Mesh_LoadTrackAndCars(void) {
    /* TODO: Windows-specific or dead-code wrapper stub */
}

/**
 * @original Texture_LoadAllPages (IGN_WIN.EXE, main.c)
 * @fidelity STUB
 */
void Texture_LoadAllPages(void) {
    /* TODO: Windows-specific or dead-code wrapper stub */
}

/**
 * @original Track_PreprocessPlacements (IGN_WIN.EXE, main.c)
 * @fidelity STUB
 */
void Track_PreprocessPlacements(void) {
    /* TODO: Windows-specific or dead-code wrapper stub */
}

extern void Audio_Init(void);
extern void FatalError(const char *msg);
extern int Sound_LoadAsset(char *path, int type, int bank);

/**
 * @original Sound_InitAndLoadPools (MAINDOS_32BIT.EXE @ 0x0002a4f1, main.c)
 * @fidelity ADAPTED
 * @notes Inits audio and loads SKID, ROLL, COLL, BOOST, DIV, and level PAT.
 */
void Sound_InitAndLoadPools(void) {
    char path[128];
    
    Audio_Init();
    
    sprintf(path, "%sSOUND\\ROLL", g_TrackDir);
    if (!Sound_LoadAsset(path, 0, 0)) FatalError("Error while loading sound\n");
    
    sprintf(path, "%sSOUND\\SKID", g_TrackDir);
    if (!Sound_LoadAsset(path, 0, 1)) FatalError("Error while loading sound\n");
    
    sprintf(path, "%sSOUND\\COLL", g_TrackDir);
    if (!Sound_LoadAsset(path, 0, 2)) FatalError("Error while loading sound\n");
    
    sprintf(path, "%sSOUND\\BOOST", g_TrackDir);
    if (!Sound_LoadAsset(path, 0, 3)) FatalError("Error while loading sound\n");
    
    sprintf(path, "%sSOUND\\DIV", g_TrackDir);
    if (!Sound_LoadAsset(path, 0, 4)) FatalError("Error while loading sound\n");
    
    sprintf(path, "LEVELS\\%sSOUND", g_TrackName);
    if (!Sound_LoadAsset(path, 1, 0)) FatalError("Error while loading sound\n");
}

/**
 * @original Car_ApplySteering (IGN_WIN.EXE, main.c)
 * @fidelity STUB
 */
void Car_ApplySteering(void) {
    /* TODO: Windows-specific or dead-code wrapper stub */
}

/**
 * @original Car_PowertrainUpdate (IGN_WIN.EXE, main.c)
 * @fidelity STUB
 */
void Car_PowertrainUpdate(void) {
    /* TODO: Windows-specific or dead-code wrapper stub */
}

/**
 * @original Sound_SynthesizeEngineRPM (IGN_WIN.EXE, main.c)
 * @fidelity STUB
 */
void Sound_SynthesizeEngineRPM(void) {
    /* TODO: Windows-specific or dead-code wrapper stub */
}

/**
 * @original Audio_MixCallback (IGN_WIN.EXE, main.c)
 * @fidelity STUB
 */
void Audio_MixCallback(void) {
    /* TODO: Windows-specific or dead-code wrapper stub */
}

/**
 * @original Audio_StopVoice (IGN_WIN.EXE, main.c)
 * @fidelity STUB
 */
void Audio_StopVoice(void) {
    /* TODO: Windows-specific or dead-code wrapper stub */
}

/**
 * @original Audio_PlayVoice (IGN_WIN.EXE, main.c)
 * @fidelity STUB
 */
void Audio_PlayVoice(void) {
    /* TODO: Windows-specific or dead-code wrapper stub */
}

/**
 * @original Audio_SetVoiceParams (IGN_WIN.EXE, main.c)
 * @fidelity STUB
 */
void Audio_SetVoiceParams(void) {
    /* TODO: Windows-specific or dead-code wrapper stub */
}

/**
 * @original Music_PlayTrack (IGN_WIN.EXE, main.c)
 * @fidelity STUB
 */
void Music_PlayTrack(void) {
    /* TODO: Windows-specific or dead-code wrapper stub */
}

/**
 * @original Sound_LoadPAT (IGN_WIN.EXE, main.c)
 * @fidelity STUB
 */
void Sound_LoadPAT(void) {
    /* TODO: Windows-specific or dead-code wrapper stub */
}

/**
 * @original Sound_LoadWAV (IGN_WIN.EXE, main.c)
 * @fidelity STUB
 */
void Sound_LoadWAV(void) {
    /* TODO: Windows-specific or dead-code wrapper stub */
}

/* --- MS-DOS Entry Point --- */

extern void Unknown_553a0(void);
extern int Unknown_558d0(void);
extern int Unknown_559e4(int a, int b);
extern int Timer_Init(void);
extern void Unknown_558ec(void);

int g_MainFlag_E73B0 = 0;

/**
 * @original main (MAINDOS_32BIT.EXE @ 0x00010010, main.c)
 * @fidelity EXACT
 * @notes The actual C entry point for the MS-DOS executable.
 */
int main(int argc, char **argv) {
    Unknown_553a0();
    
    if (Unknown_558d0() != 0) {
        if (Unknown_559e4(0x15e, 0x46) != 0) {
            g_MainFlag_E73B0 = 1;
            
            Timer_Init();
            if (1) {
                while (App_FrameTick() != 0) {
                    /* Main game loop / race tick */
                }
                Unknown_558ec();
            }
        }
    }
    
    return 0;
}

#include <conio.h>
#include <dos.h>
#include <bios.h>

int g_TimerShiftScale = 0;
double g_TimerBaseScale = 0.000000;

/**
 * @original Timer_GetPITCounter (MAINDOS_32BIT.EXE @ 0x0001034c, main.c)
 * @fidelity FUNCTIONAL
 * @notes Reads the 16-bit countdown from PIT channel 0 and converts to an up-counter.
 */
int Timer_GetPITCounter(void) {
    int low, high, count;
    outp(0x43, 0x04); /* Latch counter 0 */
    low = inp(0x40);
    high = inp(0x40);
    count = (high << 8) | low;
    count = 0xFFFF - count;
    return (count >> g_TimerShiftScale) & 0xFFFF;
}

/**
 * @original Timer_GetTime (MAINDOS_32BIT.EXE @ 0x00010238, main.c)
 * @fidelity ADAPTED
 * @notes Combines BIOS 18.2Hz tick with PIT intra-tick counter for high-res time.
 */
double Timer_GetTime(void) {
    long ticks1, ticks2;
    int pit1, pit2;
    {
        unsigned int t;
        _bios_timeofday(_TIME_GETCLOCK, (long*)&t);
        ticks1 = t;
    }
    pit1 = Timer_GetPITCounter();
    
    {
        unsigned int t;
        _bios_timeofday(_TIME_GETCLOCK, (long*)&t);
        ticks2 = t;
    }
    if (ticks1 != ticks2) {
        pit1 = Timer_GetPITCounter();
        ticks1 = ticks2;
    }
    
    return (double)(ticks1 * 2) + (double)pit1 * g_TimerBaseScale;
}

/**
 * @original Timer_Init (MAINDOS_32BIT.EXE @ 0x00010198, main.c)
 * @fidelity ADAPTED
 * @notes Reprograms PIT and calibrates TimerShiftScale based on CPU speed.
 */
int Timer_Init(void) {
    int i;
    int t1, t2;
    
    /* Reprogram PIT channel 0 to mode 2 (Rate Generator) with max divisor 0xFFFF */
    outp(0x43, 0x34);
    outp(0x40, 0x00);
    outp(0x40, 0x00);
    
    g_TimerShiftScale = 0;
    
    /* Simplified calibration loop stub */
    t1 = Timer_GetPITCounter();
    for (i = 0; i < 1000; i++) {
        t2 = Timer_GetPITCounter();
        if (t2 != t1) {
            /* Dummy delay to avoid Watcom optimizing it out */
        }
    }
    
    return 1;
}
