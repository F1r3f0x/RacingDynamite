#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include <stddef.h>
#include "lisa3d.h"
#include "mem.h"

/* Authentic Menu PIC Sprite Dimensions (from MAINDOS @ 0x00011a3e..0x00011afb) */
#define PIC_HEADER_SIZE         846     /* 0x34E bytes */

#define FLAG_WIDTH              61
#define FLAG_HEIGHT             106
#define FLAG_COUNT              6
#define FLAG_DATA_SIZE          38796   /* 0x978C */

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

/* Function Prototypes */
int Menu_Init(void);
int Menu_Tick(void);
void Menu_InitCarViewport(void);

#endif /* MENU_H */
