#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include <stddef.h>
#include "lisa3d.h"
#include "mem.h"

/* Authentic Menu PIC Sprite Dimensions (from MAINDOS @ 0x00011a3e..0x00011afb) */
#define PIC_HEADER_SIZE         846     /* 0x34E bytes */

#define FLAG_WIDTH              53
#define FLAG_HEIGHT             61
#define FLAG_COUNT              6
#define FLAG_DATA_SIZE          38796   /* 0x978C */
#define FLAG_SIZE               3233    /* 53 * 61 */
#define FLAG_HIGHLIGHT_OFFSET   19398   /* 6 * 3233 */

#define LOGO_WIDTH              173
#define LOGO_HEIGHT             57
#define LOGO_DATA_SIZE          9861    /* 0x2685 */

#define CAR_PEDESTAL_WIDTH      255
#define CAR_PEDESTAL_HEIGHT     74
#define CAR_PEDESTAL_DATA_SIZE  18870   /* 0x49B6 */

#define CAR_CARD_WIDTH          186
#define CAR_CARD_HEIGHT         61
#define CAR_CARD_COUNT          11
#define CAR_CARD_DATA_SIZE      124806  /* 0x1E786 */

#define TRACK_SPR_WIDTH         115
#define TRACK_SPR_HEIGHT        693
#define TRACK_SPR_DATA_SIZE     79695   /* 0x1374F */

/* Menu Flow States (matching MAINDOS GameStage == 1) */
#define MENU_STATE_INTRO_IGN1       0   /* Playing Virgin Interactive CDP */
#define MENU_STATE_INTRO_IGN2       1   /* Playing UDS CDP */
#define MENU_STATE_LANG_SELECT      2   /* Language Selection (FLAGGOR.PIC) */
#define MENU_STATE_MAIN             3   /* Main Menu (IGN_LOGO.PIC) */
#define MENU_STATE_CAR_SELECT       4   /* Car Selection (CAR_SEL.PIC + BILAR.PIC) */
#define MENU_STATE_TRACK_SELECT     5   /* Track Selection (TRK_SPR.PIC) */
#define MENU_STATE_OPTIONS          6   /* Options screen */
#define MENU_STATE_START_RACE       7   /* Transition to 3D Track Load */

/* Authentic Menu & Palette subsystem state (MAINDOS.EXE) */
extern uint8_t *g_pMenuPalBlack;                 /* ds:0xea204 - 768B all-zero palette */
extern uint8_t *g_pMenuPalWhite;                 /* ds:0xea208 - 768B all-0xFF palette */
extern uint8_t *g_pMenuPalWork;                  /* ds:0xea20c - 768B interpolated work palette */
extern uint8_t *g_pMenuPalDefault;               /* ds:0xea200 - 768B MENU.COL palette */
extern uint8_t *g_pMenuPalActive;                /* ds:0xea1fc - currently loaded target palette */
extern uint8_t *g_pMenuBuffer1;                  /* ds:0xea400 - 64,321 byte 8bpp menu buffer 1 */
extern uint8_t *g_pMenuBuffer2;                  /* ds:0xea404 - 64,321 byte 8bpp menu buffer 2 */
extern uint8_t *g_pMenuBuffer3;                  /* ds:0xea408 - 64,321 byte 8bpp menu buffer 3 */
extern double   g_MenuDeltaTime;                 /* ds:0xea224 - frame delta time */
extern double   g_MenuTimeSeconds;               /* ds:0xea198 - cumulative menu elapsed time */
extern double   g_MenuCdpFrameAccum;             /* ds:0xea1a0 - CDP frame step accumulator */
extern int32_t  g_MenuCdpActiveIndex;            /* ds:0xea1d0 - index of active CDP (0..5) */
extern int32_t  g_MenuCdpPendingLoad;            /* ds:0xea1d4 - flag indicating CDP needs to be opened */
extern int32_t  g_MenuAudioVoiceActive;          /* ds:0xea268 - audio voice status flag */
extern int32_t  g_MenuVideoResolutionMode;       /* ds:0xea194 - video mode index (0 = 320x200) */

extern uint8_t  g_GameSettings[0x598];           /* ds:0xe9bfc .. 0xea193 - game settings */
extern uint8_t *g_pMenuLisaEngine;               /* ds:0xea3c0 - engine context allocated in Menu_Init */
extern uint8_t *g_pMenuCdpFiles[6];              /* ds:0xea1b8 .. 0xea1cc - 6 loaded CDP file buffers */
extern CdpFile  g_MenuCdp;                      /* ds:0xea1d8 - active menu CDP player */
extern uint8_t *g_pMenuTab;                     /* ds:0xea210 - 64KB transparency lookup table pointer */
extern uint8_t *g_pMenuTabAlloc;                /* ds:0xea214 - raw allocation for menu.tab */
extern const uint8_t *g_pMenuTransTable;        /* ds:0xea218 - trans table pointer */
extern uint8_t *g_pMenuBilar;                   /* ds:0xea750 - bilar.pic (124,806 bytes) */
extern uint8_t *g_pMenuTrackSpr;                /* ds:0xea754 - trk_spr.pic (79,695 bytes) */
extern uint8_t *g_pMenuLogo;                    /* ds:0xea758 - ign_logo.pic (9,861 bytes) */
extern uint8_t *g_pMenuCarSel;                  /* ds:0xea75c - car_sel.pic (18,870 bytes) */
extern uint8_t *g_pMenuFlags;                   /* ds:0xea760 - flaggor.pic (38,796 bytes) */
extern int32_t  g_MenuSelectedLanguage;         /* ds:0xea764 */
extern int32_t  g_MenuIntroResetBuffers;        /* ds:0xbecfc */
extern int32_t  g_MenuIntroVideoEnabled;        /* ds:0xbed18 */
extern const char *g_MenuCdpNames[6];           /* ds:0xbed00 */

/* Function Prototypes */
int  Menu_Init(void);
int  Menu_Tick(void);
void Menu_Shutdown(void);
void Menu_InitCarViewport(void);
void Menu_RenderCarViewport(void);
void Menu_InitSettings(void);
int  Menu_LoadIntroCDPs(void);

/* Authentic Palette & CDP video sequencing helpers */
void Palette_Fade(const uint8_t *src1, const uint8_t *src2, uint8_t *dest, int factor);
void Palette_BlitTrans(const uint8_t *src, int x_start, int y_start, int x_end,
                       int y_end, uint8_t *dest, int dest_x, int dest_y,
                       const uint8_t *trans_table, int src_stride, int dest_stride);

#endif /* MENU_H */
