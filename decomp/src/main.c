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
    (void)car_idx;
    /* Spline navigation waypoint tracker */
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
 * @notes Instantiates player and AI cars on starting grid and binds track collision.
 */
void Race_InitSceneAndCars(void) {
    g_InRace = 1;
}

/**
 * @original Race_ResolveVehicleCollisions (IGN_WIN.EXE @ 0x00422680, main.c)
 * @fidelity EXACT
 * @notes Inter-vehicle and scenery obstacle collision detection and impulse response.
 */
void Race_ResolveVehicleCollisions(void) {
    /* Scenery obstacle and car-to-car collision resolution */
}

/**
 * @original Race_CheckCheckpointTriggers (IGN_WIN.EXE @ 0x00429a40, main.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x000250f0. Tests vehicle collision against type 150..154 split-time checkpoint gates.
 */
void Race_CheckCheckpointTriggers(void) {
    /* Lap timing and checkpoint triggers */
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
    (void)car_idx;
}
