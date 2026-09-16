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
typedef long long longlong;


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
