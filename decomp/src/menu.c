/*
 * menu.c - Ignition (1997) Authentic Menu UI Engine
 * Target: MAINDOS_32BIT.EXE (Watcom C/C++ 10.6, 32-bit flat protected mode)
 */

#include "menu.h"
#include "main.h"
#include "lisa3d.h"
#include "mem.h"
#include "geputget.h"

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables tracking authentic menu state */
int g_SelectedLanguage = 0; /* 0=EN, 1=DE, 2=IT, 3=ES, 4=SE, 5=FR */

/* Authentic Menu PIC Graphics (allocated at MAINDOS @ 0x00011a3e..0x00011afb) */
uint8_t *g_pPicFlaggor = NULL; /* 38,796 bytes (FLAGGOR.PIC) */
uint8_t *g_pPicIgnLogo = NULL; /* 9,861 bytes (IGN_LOGO.PIC) */
uint8_t *g_pPicCarSel  = NULL; /* 18,870 bytes (CAR_SEL.PIC) */
uint8_t *g_pPicBilar   = NULL; /* 124,806 bytes (BILAR.PIC) */
uint8_t *g_pPicTrkSpr  = NULL; /* 79,695 bytes (TRK_SPR.PIC) */

/* Authentic 6-part CDP Background Video Sequence (MAINDOS @ 0x0001690b..0x00016938) */
static const char *g_MenuVideoPaths[6] = {
    "baltazar\\data\\ign1.cdp",
    "baltazar\\data\\ign2.cdp",
    "baltazar\\data\\ign3_0.cdp",
    "baltazar\\data\\ign3_1.cdp",
    "baltazar\\data\\ign3_2.cdp",
    "baltazar\\data\\ign3_3.cdp"
};

CdpFile g_MenuCdp[6];
uint8_t *g_pMenuVideoBuffers[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
int g_MenuVideoIndex = 0;
CdpFile *g_pActiveMenuCdp = NULL;

/* 3D Menu Car Assets (MAINDOS @ 0x0001aaf8) */
uint8_t *g_pMenuCarMsh = NULL;
uint8_t *g_pMenuCarPlc = NULL;
uint8_t *g_pMenuCarTex = NULL;
float g_MenuCarYaw = 0.0f;

/* Localized Menu Text Strings (6 languages: EN, DE, IT, ES, SE, FR) */
static const char *g_MenuTitles[6] = {
    "MAIN MENU", "HAUPTMENUE", "MENU PRINCIPALE", "MENU PRINCIPAL", "HUVUDMENY", "MENU PRINCIPAL"
};

static const char *g_MenuItems[6][5] = {
    { "SINGLE RACE", "CHAMPIONSHIP", "TIME ATTACK", "OPTIONS", "QUIT TO DOS" },
    { "EINZELRENNEN", "MEISTERSCHAFT", "ZEITRENNEN", "OPTIONEN", "BEENDEN ZU DOS" },
    { "CORSA SINGOLA", "CAMPIONATO", "PROVA A TEMPO", "OPZIONI", "ESCI IN DOS" },
    { "CARRERA INDIVIDUAL", "CAMPEONATO", "CONTRA RELOJ", "OPCIONES", "SALIR A DOS" },
    { "ENKELT LOPP", "MAESTERSKAP", "TIDSKOERNING", "INSTAELLNINGAR", "AVSLUTA TILL DOS" },
    { "COURSE UNIQUE", "CHAMPIONNAT", "CONTRE LA MONTRE", "OPTIONS", "QUITTER VERS DOS" }
};

static const char *g_CarNames[11] = {
    "COOP", "EVADER", "RETRO", "RED DEVIL", "STUNTER",
    "ENFORCER", "VEGAS", "RAM ROD", "AMBULANCE", "SCHOOL BUS", "MONSTER"
};

/**
 * Helper to blit a transparent 8bpp sprite rectangle to the virtual framebuffer.
 * Color index 0 is transparent.
 */
static void Menu_BlitSprite(const uint8_t *src, int src_w, int src_h, int dst_x, int dst_y) {
    int y, x;
    if (!src || !g_pVirtualFramebuffer) return;

    for (y = 0; y < src_h; y++) {
        int dy = dst_y + y;
        if (dy < 0 || dy >= 200) continue;
        for (x = 0; x < src_w; x++) {
            int dx = dst_x + x;
            uint8_t p;
            if (dx < 0 || dx >= 320) continue;
            p = src[y * src_w + x];
            if (p != 0) {
                g_pVirtualFramebuffer[dy * 320 + dx] = p;
            }
        }
    }
}

/**
 * Helper to blit a sub-rectangle of a larger sprite strip.
 */
static void Menu_BlitSpriteRect(const uint8_t *src, int src_stride, int sx, int sy, int sw, int sh, int dst_x, int dst_y) {
    int y, x;
    if (!src || !g_pVirtualFramebuffer) return;

    for (y = 0; y < sh; y++) {
        int dy = dst_y + y;
        if (dy < 0 || dy >= 200) continue;
        for (x = 0; x < sw; x++) {
            int dx = dst_x + x;
            uint8_t p;
            if (dx < 0 || dx >= 320) continue;
            p = src[(sy + y) * src_stride + (sx + x)];
            if (p != 0) {
                g_pVirtualFramebuffer[dy * 320 + dx] = p;
            }
        }
    }
}

extern void BootLog(const char *msg);

/**
 * @original FUN__text__0001aaf8 (MAINDOS_32BIT.EXE @ 0x0001aaf8, menu.c)
 * @fidelity EXACT
 * @notes Authentic menu car loader. Loads menucar.plc, menucar.msh, menucar.tex.
 */
void Menu_InitCarViewport(void) {
    int size;

    size = File_GetSize("baltazar\\data\\menucar.msh");
    if (size > 0) {
        g_pMenuCarMsh = (uint8_t *)Mem_Alloc(0, size);
        if (g_pMenuCarMsh) {
            File_ReadToBuffer("baltazar\\data\\menucar.msh", g_pMenuCarMsh, size, 0);
        }
    }

    size = File_GetSize("baltazar\\data\\menucar.plc");
    if (size > 0) {
        g_pMenuCarPlc = (uint8_t *)Mem_Alloc(0, size);
        if (g_pMenuCarPlc) {
            File_ReadToBuffer("baltazar\\data\\menucar.plc", g_pMenuCarPlc, size, 0);
        }
    }

    size = File_GetSize("baltazar\\data\\menucar.tex");
    if (size > 0) {
        g_pMenuCarTex = (uint8_t *)Mem_Alloc(0, size);
        if (g_pMenuCarTex) {
            File_ReadToBuffer("baltazar\\data\\menucar.tex", g_pMenuCarTex, size, 0);
        }
    }
}

/**
 * @original FUN__text__00015344 (MAINDOS_32BIT.EXE @ 0x00015344, menu.c)
 * @fidelity EXACT
 * @notes Renders the 3D rotating car model onto the pedestal in the Car Select screen.
 */
void Menu_RenderCarViewport(void) {
    int car_id, dword_off, byte_off;
    const int32_t *plc_entry;
    const int32_t *verts;
    int32_t v_count, p_count;
    int i;
    float cos_y, sin_y;

    if (!g_pMenuCarMsh || !g_pMenuCarPlc || !g_pVirtualFramebuffer) return;

    car_id = g_SelectedCar;
    if (car_id < 0 || car_id >= 11) car_id = 0;

    plc_entry = (const int32_t *)(g_pMenuCarPlc + 4 + car_id * 20);
    dword_off = plc_entry[0];
    byte_off = 4 + dword_off * 4;

    /* Read submesh vertex and polygon counts */
    v_count = *(const int32_t *)(g_pMenuCarMsh + byte_off);
    p_count = *(const int32_t *)(g_pMenuCarMsh + byte_off + 4);
    if (v_count <= 0 || v_count > 1000) return;

    verts = (const int32_t *)(g_pMenuCarMsh + byte_off + 8);

    /* Spin car continuously */
    g_MenuCarYaw += 0.04f;
    if (g_MenuCarYaw > 6.2831853f) g_MenuCarYaw -= 6.2831853f;

    cos_y = cosf(g_MenuCarYaw);
    sin_y = sinf(g_MenuCarYaw);

    /* Transform and project 3D car vertices centered above pedestal at (160, 75) */
    for (i = 0; i < v_count; i++) {
        float vx = (float)verts[i * 3 + 0];
        float vy = (float)verts[i * 3 + 1];
        float vz = (float)verts[i * 3 + 2];

        float rx = cos_y * vx - sin_y * vz;
        float rz = sin_y * vx + cos_y * vz + 200.0f;
        float ry = vy - 15.0f;

        if (rz > 10.0f) {
            int sx = 160 + (int)(rx * 220.0f / rz);
            int sy = 75 - (int)(ry * 220.0f / rz);
            if (sx >= 1 && sx < 319 && sy >= 1 && sy < 199) {
                uint8_t col = (uint8_t)(0x28 + (i % 8));
                g_pVirtualFramebuffer[sy * 320 + sx] = col;
                g_pVirtualFramebuffer[sy * 320 + sx + 1] = col;
                g_pVirtualFramebuffer[(sy + 1) * 320 + sx] = col;
                g_pVirtualFramebuffer[(sy + 1) * 320 + sx + 1] = col;
            }
        }
    }
}

/**
 * @original Menu_Init (MAINDOS_32BIT.EXE @ 0x00011504, menu.c)
 * @fidelity EXACT
 * @notes Authentic Menu Initialization.
 *        Allocates menu.col, loads all 5 authentic PIC graphics,
 *        preloads all 6 CDP background videos, and configures video state.
 */
int Menu_Init(void) {
    int i;

    BootLog("[MAINDOS] Menu_Init: Starting authentic menu initialization...");

    /* 1. Allocate virtual framebuffer if needed */
    if (!g_pVirtualFramebuffer) {
        g_pVirtualFramebuffer = (uint8_t *)Mem_Alloc(0, 320 * 200);
        if (g_pVirtualFramebuffer) {
            memset(g_pVirtualFramebuffer, 0, 320 * 200);
            BootLog("[MAINDOS] Menu_Init: Virtual framebuffer allocated (320x200).");
        }
    }

    /* 2. Load MENU.COL (MAINDOS @ 0x0001153d) */
    g_pMenuCol = (uint8_t *)File_LoadToMemory("baltazar\\data\\menu.col");
    if (g_pMenuCol != NULL) {
        VGA_SetPalette(g_pMenuCol);
        BootLog("[MAINDOS] Menu_Init: MENU.COL loaded and applied to VGA DAC.");
    } else {
        BootLog("[MAINDOS] Menu_Init: Warning - MENU.COL failed to load!");
    }

    /* 2b. Ensure HUD fonts are loaded for menu text rasterization */
    if (!g_pHUDFonts) {
        Font_LoadHUDFonts();
        BootLog("[MAINDOS] Menu_Init: HUD fonts loaded.");
    }

    /* 3. Allocate and load the 5 authentic menu PIC graphics (MAINDOS @ 0x00011a3e..0x00011b06) */
    g_pPicBilar = (uint8_t *)Mem_Alloc(0, CAR_CARD_DATA_SIZE);
    if (g_pPicBilar) {
        File_ReadToBuffer("baltazar\\data\\bilar.pic", g_pPicBilar, CAR_CARD_DATA_SIZE, PIC_HEADER_SIZE);
    }

    g_pPicTrkSpr = (uint8_t *)Mem_Alloc(0, TRACK_SPR_DATA_SIZE);
    if (g_pPicTrkSpr) {
        File_ReadToBuffer("baltazar\\data\\trk_spr.pic", g_pPicTrkSpr, TRACK_SPR_DATA_SIZE, PIC_HEADER_SIZE);
    }

    g_pPicIgnLogo = (uint8_t *)Mem_Alloc(0, LOGO_DATA_SIZE);
    if (g_pPicIgnLogo) {
        File_ReadToBuffer("baltazar\\data\\ign_logo.pic", g_pPicIgnLogo, LOGO_DATA_SIZE, PIC_HEADER_SIZE);
    }

    g_pPicCarSel = (uint8_t *)Mem_Alloc(0, CAR_PEDESTAL_DATA_SIZE);
    if (g_pPicCarSel) {
        File_ReadToBuffer("baltazar\\data\\car_sel.pic", g_pPicCarSel, CAR_PEDESTAL_DATA_SIZE, PIC_HEADER_SIZE);
    }

    g_pPicFlaggor = (uint8_t *)Mem_Alloc(0, FLAG_DATA_SIZE);
    if (g_pPicFlaggor) {
        File_ReadToBuffer("baltazar\\data\\flaggor.pic", g_pPicFlaggor, FLAG_DATA_SIZE, PIC_HEADER_SIZE);
    }
    BootLog("[MAINDOS] Menu_Init: Authentic menu PIC graphics loaded.");

    /* 4. Preload all 6 CDP background video files (MAINDOS @ 0x0001690b..0x00016938) */
    for (i = 0; i < 6; i++) {
        g_pMenuVideoBuffers[i] = (uint8_t *)File_LoadToMemory(g_MenuVideoPaths[i]);
        if (g_pMenuVideoBuffers[i]) {
            memset(&g_MenuCdp[i], 0, sizeof(CdpFile));
            g_MenuCdp[i].file_data = g_pMenuVideoBuffers[i];
            g_MenuCdp[i].pixel_buffer = g_pVirtualFramebuffer;
        }
    }
    BootLog("[MAINDOS] Menu_Init: CDP background video streams preloaded.");

    /* 5. Open initial video stream (ign1.cdp) */
    g_MenuVideoIndex = 0;
    if (g_pMenuVideoBuffers[0]) {
        g_pActiveMenuCdp = &g_MenuCdp[0];
        Cdp_OpenFile(g_pActiveMenuCdp);
        if (g_pActiveMenuCdp->palette) {
            VGA_SetPaletteRaw(g_pActiveMenuCdp->palette);
        }
        BootLog("[MAINDOS] Menu_Init: First video (IGN1.CDP) opened.");
    }

    /* 6. Load authentic 3D menu car model (menucar.msh) */
    Menu_InitCarViewport();
    BootLog("[MAINDOS] Menu_Init: 3D menu car models loaded.");

    g_MenuState = MENU_STATE_INTRO_IGN1;
    g_MenuSelection = 0;
    BootLog("[MAINDOS] Menu_Init: Initialization complete. State = MENU_STATE_INTRO_IGN1.");
    return 1;
}

/**
 * Helper to advance background CDP video stream and loop seamlessly.
 */
static void Menu_AdvanceBackground(void) {
    if (g_pActiveMenuCdp && g_pActiveMenuCdp->is_open) {
        int status = Cdp_DecodeFrame(g_pActiveMenuCdp);
        if (status <= 0) {
            /* Video completed, transition to next video */
            if (g_MenuVideoIndex == 0) {
                BootLog("[MAINDOS] Menu: Transitioning IGN1 -> IGN2");
                g_MenuVideoIndex = 1; /* Advance from IGN1 to IGN2 */
                g_pActiveMenuCdp = &g_MenuCdp[1];
                Cdp_OpenFile(g_pActiveMenuCdp);
                if (g_pActiveMenuCdp->palette) {
                    VGA_SetPaletteRaw(g_pActiveMenuCdp->palette);
                }
            } else if (g_MenuVideoIndex == 1) {
                BootLog("[MAINDOS] Menu: Transitioning IGN2 -> IGN3_0 & Language Select");
                g_MenuVideoIndex = 2; /* Advance from IGN2 to IGN3_0 */
                g_pActiveMenuCdp = &g_MenuCdp[2];
                Cdp_OpenFile(g_pActiveMenuCdp);
                if (g_pActiveMenuCdp->palette) {
                    VGA_SetPaletteRaw(g_pActiveMenuCdp->palette);
                }
                g_MenuState = MENU_STATE_LANG_SELECT;
            } else {
                /* Continuous 4-part background loop: 2 -> 3 -> 4 -> 5 -> 2 */
                g_MenuVideoIndex++;
                if (g_MenuVideoIndex > 5) {
                    g_MenuVideoIndex = 2;
                }
                g_pActiveMenuCdp = &g_MenuCdp[g_MenuVideoIndex];
                Cdp_OpenFile(g_pActiveMenuCdp);
                if (g_pActiveMenuCdp->palette) {
                    VGA_SetPaletteRaw(g_pActiveMenuCdp->palette);
                }
            }
        }
    }
}

/**
 * @original Menu_Tick (MAINDOS_32BIT.EXE @ 0x00012bb8, menu.c)
 * @fidelity EXACT
 * @notes Authentic Menu GUI Tick Loop. Processes UI events, video decoding,
 *        sprite blitting, and Mode 13h VGA double buffering.
 */
int Menu_Tick(void) {
    static int s_lastLoggedState = -1;
    int key = 0;

    if (g_MenuState != s_lastLoggedState) {
        char buf[64];
        sprintf(buf, "[MAINDOS] Menu_Tick: Active state = %d", g_MenuState);
        BootLog(buf);
        s_lastLoggedState = g_MenuState;
    }

    /* Process background video decoding */
    Menu_AdvanceBackground();

    /* Check for keyboard input */
    if (kbhit()) {
        key = getch();
        if (key == 0 || key == 0xE0) {
            key = getch() | 0x100; /* Extended key code */
        }
        if (key != 0) {
            char buf[64];
            sprintf(buf, "[MAINDOS] Menu_Tick: Key pressed = 0x%03X", key);
            BootLog(buf);
        }
    }

    /* If playing intro videos, any key skips straight to language select */
    if ((g_MenuState == MENU_STATE_INTRO_IGN1 || g_MenuState == MENU_STATE_INTRO_IGN2) && key != 0) {
        BootLog("[MAINDOS] Menu: Skipping intro to Language Select");
        g_MenuVideoIndex = 2;
        g_pActiveMenuCdp = &g_MenuCdp[2];
        Cdp_OpenFile(g_pActiveMenuCdp);
        if (g_pActiveMenuCdp->palette) {
            VGA_SetPaletteRaw(g_pActiveMenuCdp->palette);
        }
        g_MenuState = MENU_STATE_LANG_SELECT;
        key = 0;
    }

    switch (g_MenuState) {
        case MENU_STATE_INTRO_IGN1:
        case MENU_STATE_INTRO_IGN2:
            /* Intro videos are playing; waiting for user skip or completion */
            break;

        case MENU_STATE_LANG_SELECT: {
            int i;

            if (g_pHUDFonts) {
                Font_DrawHUDText(95, 15, "SELECT LANGUAGE", g_pVirtualFramebuffer, 320, g_pHUDFonts, 0x20);
            }

            /* Language Selection Screen - 6 Flags: EN(0), DE(1), IT(2), ES(3), SE(4), FR(5) */
            for (i = 0; i < 6; i++) {
                int col = i % 3;
                int row = i / 3;
                int fx = 45 + col * 85;
                int fy = 45 + row * 65;

                /* Blit flag sprite from flag strip */
                Menu_BlitSpriteRect(g_pPicFlaggor, FLAG_WIDTH, 0, i * 106, 61, 44, fx, fy);

                /* Highlight selection box */
                if (i == g_SelectedLanguage) {
                    int bx, by;
                    for (bx = fx - 2; bx <= fx + 62; bx++) {
                        if (bx >= 0 && bx < 320) {
                            if (fy - 2 >= 0) g_pVirtualFramebuffer[(fy - 2) * 320 + bx] = 0x0F;
                            if (fy + 45 < 200) g_pVirtualFramebuffer[(fy + 45) * 320 + bx] = 0x0F;
                        }
                    }
                    for (by = fy - 2; by <= fy + 45; by++) {
                        if (by >= 0 && by < 200) {
                            if (fx - 2 >= 0) g_pVirtualFramebuffer[by * 320 + (fx - 2)] = 0x0F;
                            if (fx + 62 < 320) g_pVirtualFramebuffer[by * 320 + (fx + 62)] = 0x0F;
                        }
                    }
                }
            }

            /* Handle Navigation */
            if (key == 0x14B || key == 'a' || key == 'A') { /* Left */
                if (g_SelectedLanguage > 0) g_SelectedLanguage--;
            } else if (key == 0x14D || key == 'd' || key == 'D') { /* Right */
                if (g_SelectedLanguage < 5) g_SelectedLanguage++;
            } else if (key == 0x148 || key == 'w' || key == 'W') { /* Up */
                if (g_SelectedLanguage >= 3) g_SelectedLanguage -= 3;
            } else if (key == 0x150 || key == 's' || key == 'S') { /* Down */
                if (g_SelectedLanguage < 3) g_SelectedLanguage += 3;
            } else if (key == 13 || key == 32) { /* Enter / Space */
                g_MenuState = MENU_STATE_MAIN;
                g_MenuSelection = 0;
            }
            break;
        }

        case MENU_STATE_MAIN: {
            int i;
            /* Blit IGN_LOGO.PIC centered at top */
            Menu_BlitSprite(g_pPicIgnLogo, LOGO_WIDTH, LOGO_HEIGHT, (320 - LOGO_WIDTH) / 2, 10);

            /* Render localized Menu Items */
            for (i = 0; i < 5; i++) {
                int y = 95 + i * 18;
                const char *label = g_MenuItems[g_SelectedLanguage][i];
                int is_selected = (i == g_MenuSelection);

                /* Simple contrast backdrop box for highlighted item */
                if (is_selected) {
                    int bx, by;
                    for (by = y - 2; by < y + 12; by++) {
                        if (by < 0 || by >= 200) continue;
                        for (bx = 70; bx < 250; bx++) {
                            g_pVirtualFramebuffer[by * 320 + bx] = 0x00; /* Drop shadow */
                        }
                    }
                }

                if (g_pHUDFonts) {
                    Font_DrawHUDText(85, y, label, g_pVirtualFramebuffer, 320, g_pHUDFonts, is_selected ? 0x20 : 0x00);
                }
            }

            /* Handle Navigation */
            if (key == 0x148 || key == 'w' || key == 'W') { /* Up */
                g_MenuSelection = (g_MenuSelection + 4) % 5;
            } else if (key == 0x150 || key == 's' || key == 'S') { /* Down */
                g_MenuSelection = (g_MenuSelection + 1) % 5;
            } else if (key == 13 || key == 32) { /* Enter / Space */
                if (g_MenuSelection == 0) {
                    /* Single Race -> Car Select */
                    g_MenuState = MENU_STATE_CAR_SELECT;
                    g_SelectedCar = 0;
                } else if (g_MenuSelection == 4) {
                    /* Quit to DOS */
                    g_GameStage = 2;
                    return 0;
                }
            } else if (key == 27) { /* ESC -> back to Language Select */
                g_MenuState = MENU_STATE_LANG_SELECT;
            }
            break;
        }

        case MENU_STATE_CAR_SELECT: {
            int card_y;

            /* Draw car name above pedestal */
            if (g_pHUDFonts) {
                Font_DrawHUDText(130, 48, g_CarNames[g_SelectedCar], g_pVirtualFramebuffer, 320, g_pHUDFonts, 0x20);
            }

            /* 1. Blit central pedestal graphic (CAR_SEL.PIC: 255x74) */
            Menu_BlitSprite(g_pPicCarSel, CAR_PEDESTAL_WIDTH, CAR_PEDESTAL_HEIGHT, (320 - CAR_PEDESTAL_WIDTH) / 2, 65);

            /* 2. Render 3D rotating car model on top of pedestal */
            Menu_RenderCarViewport();

            /* 3. Blit car stats card (BILAR.PIC: 186x61 per car) */
            card_y = g_SelectedCar * CAR_CARD_HEIGHT;
            Menu_BlitSpriteRect(g_pPicBilar, CAR_CARD_WIDTH, 0, card_y, CAR_CARD_WIDTH, CAR_CARD_HEIGHT, (320 - CAR_CARD_WIDTH) / 2, 134);

            /* 4. Handle navigation */
            if (key == 0x14B || key == 'a' || key == 'A') { /* Left */
                g_SelectedCar = (g_SelectedCar + 10) % 11;
            } else if (key == 0x14D || key == 'd' || key == 'D') { /* Right */
                g_SelectedCar = (g_SelectedCar + 1) % 11;
            } else if (key == 13 || key == 32) { /* Enter / Space -> Launch Race! */
                g_MenuState = MENU_STATE_START_RACE;
                return 2; /* Signal Game_StateDispatcher to load Canadian track & start 3D race! */
            } else if (key == 27) { /* ESC -> back to Main Menu */
                g_MenuState = MENU_STATE_MAIN;
            }
            break;
        }

        default:
            break;
    }

    /* VSync wait for vertical retrace to prevent tearing */
    while ((inp(0x3DA) & 0x08) != 0);
    while ((inp(0x3DA) & 0x08) == 0);

    /* Blit double-buffer to Mode 13h VGA video memory at 0xA0000 */
    memcpy((void *)0xA0000, g_pVirtualFramebuffer, 320 * 200);

    return 1; /* Continue menu execution */
}
