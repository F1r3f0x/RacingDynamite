/**
 * @file menu.c
 * @brief Authentic Menu UI Engine & Asset Pipeline
 * @original Menu_Init (MAINDOS_32BIT.EXE @ 0x00011504, menu.c)
 * @original Menu_Tick (MAINDOS_32BIT.EXE @ 0x00012bb8, menu.c)
 * @original Menu_InitCarViewport (MAINDOS_32BIT.EXE @ 0x0001aaf8, menu.c)
 * @fidelity EXACT
 */

#include "menu.h"
#include "main.h"
#include "geputget.h"
#include "mem.h"

#include <conio.h>
#include <string.h>

/* --- Authentic Menu Assets & Buffers --- */
uint8_t *g_pMenuBuffer1 = NULL;        /* ds:0xea400 (64,321 bytes) */
uint8_t *g_pMenuBuffer2 = NULL;        /* ds:0xea404 */
uint8_t *g_pMenuBuffer3 = NULL;        /* ds:0xea408 */

uint8_t *g_pMenuBlackPal = NULL;       /* ds:0xea204 (768 bytes, 0x00) */
uint8_t *g_pMenuWhitePal = NULL;       /* ds:0xea208 (768 bytes, 0xFF) */

uint8_t *g_pMenuBilar = NULL;          /* ds:0xea750 (bilar.pic, 124,806 bytes) */
uint8_t *g_pMenuTrackSpr = NULL;       /* ds:0xea754 (trk_spr.pic, 79,695 bytes) */
uint8_t *g_pMenuLogo = NULL;           /* ds:0xea758 (ign_logo.pic, 9,861 bytes) */
uint8_t *g_pMenuCarSel = NULL;         /* ds:0xea75c (car_sel.pic, 18,870 bytes) */
uint8_t *g_pMenuFlags = NULL;          /* ds:0xea760 (flaggor.pic, 38,796 bytes) */

uint8_t *g_pDefaultPSQ = NULL;         /* ds:0xea314 (default2.psq) */
uint8_t *g_pTestPFM = NULL;            /* test2.pfm */

int g_SelectedLanguage = 1;            /* 0=Swedish, 1=English, 2=French, 3=German, 4=Italian, 5=Spanish */
int g_MenuInitialized = 0;

static const char *s_TrackDirs[7] = {
    "CANADA\\",
    "AUSTRIA\\",
    "BRAZIL\\",
    "CARIB\\",
    "ICELAND\\",
    "JAPAN\\",
    "USA\\"
};

static const char *s_TrackNames[7] = {
    "CANADA",
    "AUSTRIA",
    "BRAZIL",
    "CARIB",
    "ICELAND",
    "JAPAN",
    "USA"
};

static const char *s_CarNames[11] = {
    "COOPER",
    "POLICE",
    "EVIL",
    "BUGGY",
    "MUSCLE",
    "STOCK",
    "SPEED",
    "SCHOOL",
    "MONSTER",
    "VAN",
    "BEETLE"
};

/**
 * 2D Software blitter for 8bpp sprites with color-key 0 transparency.
 */
static void Menu_BlitSprite(uint8_t *dest, int dx, int dy,
                            const uint8_t *src, int sw, int sh,
                            int transparent) {
    int y, x;
    if (!dest || !src) return;

    for (y = 0; y < sh; y++) {
        int py = dy + y;
        if (py < 0 || py >= 200) continue;

        for (x = 0; x < sw; x++) {
            int px = dx + x;
            uint8_t pixel;
            if (px < 0 || px >= 320) continue;

            pixel = src[y * sw + x];
            if (transparent && pixel == 0) continue;
            dest[py * 320 + px] = pixel;
        }
    }
}

/**
 * @original Menu_InitCarViewport (MAINDOS_32BIT.EXE @ 0x0001aaf8, menu.c)
 * @fidelity EXACT
 * @notes Allocates 3D preview buffers and loads default2.psq / test2.pfm car rotation tables.
 */
void Menu_InitCarViewport(void) {
    int size;

    /* Viewport scratch buffers: ds:0xea3b0 (0x1DFFFF) and ds:0xea3b4 (0x2FFFF) */
    Mem_Alloc(0x1dffff, 0);
    Mem_Alloc(0x02ffff, 0);

    size = File_GetSize("baltazar\\data\\default2.psq");
    if (size > 0) {
        g_pDefaultPSQ = (uint8_t *)Mem_Alloc(size, 0);
        if (g_pDefaultPSQ) {
            File_ReadToBuffer("baltazar\\data\\default2.psq", g_pDefaultPSQ, size, 0);
        }
    }

    size = File_GetSize("baltazar\\data\\test2.pfm");
    if (size > 0) {
        g_pTestPFM = (uint8_t *)Mem_Alloc(size, 0);
        if (g_pTestPFM) {
            File_ReadToBuffer("baltazar\\data\\test2.pfm", g_pTestPFM, size, 0);
        }
    }
}

/**
 * @original Menu_Init (MAINDOS_32BIT.EXE @ 0x00011504, menu.c)
 * @fidelity EXACT
 * @notes Loads MENU.COL, MENU.TAB, BILAR.PIC, TRK_SPR.PIC, IGN_LOGO.PIC, CAR_SEL.PIC, and FLAGGOR.PIC.
 */
int Menu_Init(void) {
    if (g_MenuInitialized) {
        return 1;
    }

    BootLog("[MAINDOS] Menu_Init: Initializing authentic menu system...");

    /* 1. Allocate and load 768-byte palette (baltazar\data\menu.col) */
    g_pMenuCol = (uint8_t *)Mem_Alloc(768, 0);
    if (!g_pMenuCol || !File_ReadToBuffer("baltazar\\data\\menu.col", g_pMenuCol, 768, 0)) {
        BootLog("[MAINDOS] Menu_Init: Failed to load menu.col!");
        return 0;
    }

    /* 2. Allocate framebuffers and fade palettes */
    g_pMenuBuffer1 = (uint8_t *)Mem_Alloc(64321, 0);
    g_pMenuBuffer2 = (uint8_t *)Mem_Alloc(64321, 0);
    g_pMenuBuffer3 = (uint8_t *)Mem_Alloc(64321, 0);

    g_pMenuBlackPal = (uint8_t *)Mem_Alloc(768, 0);
    if (g_pMenuBlackPal) memset(g_pMenuBlackPal, 0x00, 768);

    g_pMenuWhitePal = (uint8_t *)Mem_Alloc(768, 0);
    if (g_pMenuWhitePal) memset(g_pMenuWhitePal, 0xff, 768);

    /* 3. Allocate and load text/layout table (baltazar\data\menu.tab) */
    g_pMenuTab = (uint8_t *)Mem_Alloc(0x10000, 0);
    if (g_pMenuTab) {
        File_ReadToBuffer("baltazar\\data\\menu.tab", g_pMenuTab, 0x10000, 0);
    }

    /* 4. Allocate and load sprite banks (skip 846-byte PIC header) */
    g_pMenuBilar = (uint8_t *)Mem_Alloc(CAR_CARD_DATA_SIZE, 0);
    if (g_pMenuBilar) {
        File_ReadToBuffer("baltazar\\data\\bilar.pic", g_pMenuBilar, CAR_CARD_DATA_SIZE, PIC_HEADER_SIZE);
    }

    g_pMenuTrackSpr = (uint8_t *)Mem_Alloc(TRACK_SPR_DATA_SIZE, 0);
    if (g_pMenuTrackSpr) {
        File_ReadToBuffer("baltazar\\data\\trk_spr.pic", g_pMenuTrackSpr, TRACK_SPR_DATA_SIZE, PIC_HEADER_SIZE);
    }

    g_pMenuLogo = (uint8_t *)Mem_Alloc(LOGO_DATA_SIZE, 0);
    if (g_pMenuLogo) {
        File_ReadToBuffer("baltazar\\data\\ign_logo.pic", g_pMenuLogo, LOGO_DATA_SIZE, PIC_HEADER_SIZE);
    }

    g_pMenuCarSel = (uint8_t *)Mem_Alloc(CAR_PEDESTAL_DATA_SIZE, 0);
    if (g_pMenuCarSel) {
        File_ReadToBuffer("baltazar\\data\\car_sel.pic", g_pMenuCarSel, CAR_PEDESTAL_DATA_SIZE, PIC_HEADER_SIZE);
    }

    g_pMenuFlags = (uint8_t *)Mem_Alloc(FLAG_DATA_SIZE, 0);
    if (g_pMenuFlags) {
        File_ReadToBuffer("baltazar\\data\\flaggor.pic", g_pMenuFlags, FLAG_DATA_SIZE, PIC_HEADER_SIZE);
    }

    /* 5. Initialize 3D car viewport */
    Menu_InitCarViewport();

    /* 6. Ensure virtual double buffer exists */
    if (!g_pVirtualFramebuffer) {
        g_pVirtualFramebuffer = (uint8_t *)Mem_Alloc(320 * 200, 0);
    }

    /* 7. Set initial palette and state */
    VGA_SetPalette(g_pMenuCol);
    g_MenuState = MENU_STATE_LANG_SELECT;
    g_MenuSelection = 0;
    g_SelectedCar = 0;
    g_SelectedTrack = 0;
    g_MenuInitialized = 1;

    BootLog("[MAINDOS] Menu_Init: Menu initialized successfully!");
    return 1;
}

/**
 * @original Menu_Tick (MAINDOS_32BIT.EXE @ 0x00012bb8, menu.c)
 * @fidelity EXACT
 * @notes Native menu event and render loop (Language -> Main -> Car Select -> Track Select).
 * @return 0 = continue menu, 2 = start race, 3 = quit to DOS.
 */
int Menu_Tick(void) {
    int key = 0;

    if (!g_pVirtualFramebuffer) {
        return 0;
    }

    /* Clear virtual framebuffer to background color (black) */
    memset(g_pVirtualFramebuffer, 0x00, 320 * 200);

    /* Poll keyboard input */
    if (kbhit()) {
        key = getch();
        if (key == 0 || key == 0xE0) {
            key = (key << 8) | getch();
        }
    }

    /* --- State Dispatcher --- */
    switch (g_MenuState) {
    case MENU_STATE_LANG_SELECT: {
        /* Render 6 language flags in a centered horizontal strip */
        int f;
        int start_x = 10;
        int y = 46;

        for (f = 0; f < 6; f++) {
            int fx = start_x + f * 51;
            uint8_t *flag_pixels;

            if (g_pMenuFlags) {
                flag_pixels = g_pMenuFlags + f * (FLAG_WIDTH * FLAG_HEIGHT);
                Menu_BlitSprite(g_pVirtualFramebuffer, fx, y, flag_pixels, FLAG_WIDTH, FLAG_HEIGHT, 1);
            }

            /* Draw selection cursor highlight above selected flag */
            if (f == g_SelectedLanguage) {
                Font_DrawText("^", g_SystemFonts[1], fx + (FLAG_WIDTH / 2) - 4, y + FLAG_HEIGHT + 4);
            }
        }

        Font_DrawText("SELECT LANGUAGE", g_SystemFonts[0], 160, 20);

        /* Handle input */
        if (key == 0x4B00 || key == 0xE04B) { /* Left arrow */
            if (g_SelectedLanguage > 0) g_SelectedLanguage--;
        } else if (key == 0x4D00 || key == 0xE04D) { /* Right arrow */
            if (g_SelectedLanguage < 5) g_SelectedLanguage++;
        } else if (key == 13 || key == 32) { /* Enter or Space */
            g_MenuState = MENU_STATE_MAIN;
            g_MenuSelection = 0;
        } else if (key == 27) { /* Esc */
            return 3; /* Exit to DOS */
        }
        break;
    }

    case MENU_STATE_MAIN: {
        static const char *menu_items[] = {
            "SINGLE RACE",
            "CHAMPIONSHIP",
            "TIME ATTACK",
            "MULTIPLAYER",
            "OPTIONS",
            "QUIT"
        };
        int i;

        /* Draw Ignition logo at top center */
        if (g_pMenuLogo) {
            Menu_BlitSprite(g_pVirtualFramebuffer, (320 - LOGO_WIDTH) / 2, 16,
                            g_pMenuLogo, LOGO_WIDTH, LOGO_HEIGHT, 1);
        }

        /* Draw menu options */
        for (i = 0; i < 6; i++) {
            int font = (i == g_MenuSelection) ? g_SystemFonts[1] : g_SystemFonts[0];
            Font_DrawText(menu_items[i], font, 160, 88 + i * 16);
        }

        /* Handle input */
        if (key == 0x4800 || key == 0xE048) { /* Up arrow */
            if (g_MenuSelection > 0) g_MenuSelection--;
            else g_MenuSelection = 5;
        } else if (key == 0x5000 || key == 0xE050) { /* Down arrow */
            if (g_MenuSelection < 5) g_MenuSelection++;
            else g_MenuSelection = 0;
        } else if (key == 13 || key == 32) { /* Enter */
            if (g_MenuSelection == 0 || g_MenuSelection == 1 || g_MenuSelection == 2) {
                g_MenuState = MENU_STATE_CAR_SELECT;
                g_SelectedCar = 0;
            } else if (g_MenuSelection == 5) {
                return 3; /* Quit to DOS */
            }
        } else if (key == 27) { /* Esc */
            g_MenuState = MENU_STATE_LANG_SELECT;
        }
        break;
    }

    case MENU_STATE_CAR_SELECT: {
        /* Draw car pedestal at bottom center */
        if (g_pMenuCarSel) {
            Menu_BlitSprite(g_pVirtualFramebuffer, (320 - CAR_PEDESTAL_WIDTH) / 2, 110,
                            g_pMenuCarSel, CAR_PEDESTAL_WIDTH, CAR_PEDESTAL_HEIGHT, 1);
        }

        /* Draw selected 2D car stats card from BILAR.PIC */
        if (g_pMenuBilar) {
            uint8_t *car_card = g_pMenuBilar + g_SelectedCar * (CAR_CARD_WIDTH * CAR_CARD_HEIGHT);
            Menu_BlitSprite(g_pVirtualFramebuffer, (320 - CAR_CARD_WIDTH) / 2, 40,
                            car_card, CAR_CARD_WIDTH, CAR_CARD_HEIGHT, 1);
        }

        Font_DrawText("CHOOSE YOUR VEHICLE", g_SystemFonts[0], 160, 16);
        Font_DrawText(s_CarNames[g_SelectedCar], g_SystemFonts[1], 160, 155);

        /* Handle input */
        if (key == 0x4B00 || key == 0xE04B) { /* Left arrow */
            if (g_SelectedCar > 0) g_SelectedCar--;
            else g_SelectedCar = 10;
        } else if (key == 0x4D00 || key == 0xE04D) { /* Right arrow */
            if (g_SelectedCar < 10) g_SelectedCar++;
            else g_SelectedCar = 0;
        } else if (key == 13 || key == 32) { /* Enter */
            g_MenuState = MENU_STATE_TRACK_SELECT;
            g_SelectedTrack = 0;
        } else if (key == 27) { /* Esc */
            g_MenuState = MENU_STATE_MAIN;
        }
        break;
    }

    case MENU_STATE_TRACK_SELECT: {
        int track_h = TRACK_SPR_HEIGHT / 7; /* ~99 pixels per track thumbnail */

        /* Draw track thumbnail from TRK_SPR.PIC */
        if (g_pMenuTrackSpr) {
            uint8_t *trk_pixels = g_pMenuTrackSpr + g_SelectedTrack * (TRACK_SPR_WIDTH * track_h);
            Menu_BlitSprite(g_pVirtualFramebuffer, (320 - TRACK_SPR_WIDTH) / 2, 45,
                            trk_pixels, TRACK_SPR_WIDTH, track_h, 1);
        }

        Font_DrawText("SELECT TRACK", g_SystemFonts[0], 160, 18);
        Font_DrawText(s_TrackNames[g_SelectedTrack], g_SystemFonts[1], 160, 155);

        /* Handle input */
        if (key == 0x4B00 || key == 0xE04B) { /* Left arrow */
            if (g_SelectedTrack > 0) g_SelectedTrack--;
            else g_SelectedTrack = 6;
        } else if (key == 0x4D00 || key == 0xE04D) { /* Right arrow */
            if (g_SelectedTrack < 6) g_SelectedTrack++;
            else g_SelectedTrack = 0;
        } else if (key == 13 || key == 32) { /* Enter -> Start Race! */
            strncpy(g_TrackDir, s_TrackDirs[g_SelectedTrack], sizeof(g_TrackDir) - 1);
            strncpy(g_TrackName, s_TrackNames[g_SelectedTrack], sizeof(g_TrackName) - 1);
            g_MenuState = MENU_STATE_START_RACE;
            BootLog("[MAINDOS] Menu_Tick: Starting race on selected track!");
            return 2; /* Triggers Track_Load and 3D Race in Game_StateDispatcher */
        } else if (key == 27) { /* Esc */
            g_MenuState = MENU_STATE_CAR_SELECT;
        }
        break;
    }

    default:
        break;
    }

    /* Wait for VSync and copy virtual double buffer to Mode 13h screen memory */
    while ((inp(0x3DA) & 0x08) == 0);
    memcpy((void *)0xA0000, g_pVirtualFramebuffer, 320 * 200);

    return 0; /* Continue menu loop */
}
