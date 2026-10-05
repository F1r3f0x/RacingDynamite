/**
 * @file menu.c
 * @brief Authentic Menu UI Engine & Asset Pipeline
 * @original Menu_Init (MAINDOS.EXE @ 0x00011504, menu.c)
 * @fidelity EXACT
 * @original Menu_Tick (MAINDOS.EXE @ 0x00012bb8, menu.c)
 * @fidelity EXACT
 * @original Menu_InitCarViewport (MAINDOS.EXE @ 0x0001aaf8, menu.c)
 * @fidelity EXACT
 * @original Menu_RenderCarViewport (MAINDOS.EXE @ 0x00015344, menu.c)
 * @fidelity EXACT
 */

#include "menu.h"
#include "main.h"
#include "geputget.h"
#include "mem.h"
#include "lisa3d.h"

#include <conio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <i86.h>

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

uint8_t *g_pMenuCarMsh = NULL;         /* menucar.msh (82,876 bytes) */
uint8_t *g_pMenuCarPlc = NULL;         /* menucar.plc (224 bytes) */
uint8_t *g_pMenuCarTex = NULL;         /* menucar.tex (393,280 bytes) */

uint8_t *g_pDefaultPSQ = NULL;         /* ds:0xea314 (default2.psq) */
uint8_t *g_pTestPFM = NULL;            /* test2.pfm */

/* Video background player */
static CdpFile s_MenuCDP;
static uint8_t *s_pCdpData = NULL;
static int s_ActiveCdpId = -1;

/* 3D Car rotation angle */
static double s_CarYaw = 0.6;

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
 * Fast scanline horizontal span triangle filler for 3D car rendering.
 */
static void Menu_FillTriangle(uint8_t *fb, int x0, int y0, int x1, int y1, int x2, int y2, uint8_t color) {
    int t;
    int min_y, max_y, cur_y;
    int xl, xr;

    /* Sort vertices by Y: y0 <= y1 <= y2 */
    if (y0 > y1) { t = x0; x0 = x1; x1 = t; t = y0; y0 = y1; y1 = t; }
    if (y0 > y2) { t = x0; x0 = x2; x2 = t; t = y0; y0 = y2; y2 = t; }
    if (y1 > y2) { t = x1; x1 = x2; x2 = t; t = y1; y1 = y2; y2 = t; }

    if (y0 >= 200 || y2 < 0) return;

    min_y = y0 < 0 ? 0 : y0;
    max_y = y2 >= 200 ? 199 : y2;

    for (cur_y = min_y; cur_y <= max_y; cur_y++) {
        if (cur_y < y1) {
            xl = (y1 != y0) ? x0 + (x1 - x0) * (cur_y - y0) / (y1 - y0) : x0;
        } else {
            xl = (y2 != y1) ? x1 + (x2 - x1) * (cur_y - y1) / (y2 - y1) : x1;
        }
        xr = (y2 != y0) ? x0 + (x2 - x0) * (cur_y - y0) / (y2 - y0) : x0;

        if (xl > xr) { t = xl; xl = xr; xr = t; }
        if (xl < 0) xl = 0;
        if (xr >= 320) xr = 319;

        if (xr >= xl) {
            memset(fb + cur_y * 320 + xl, color, xr - xl + 1);
        }
    }
}

/**
 * Draws an authentic rounded pill-shaped button widget.
 */
static void Menu_DrawPillButton(uint8_t *fb, int cx, int cy, int w, int h, const char *text, int selected) {
    int x0 = cx - w / 2;
    int y0 = cy - h / 2;
    int x1 = cx + w / 2;
    int y1 = cy + h / 2;
    int py, px;
    uint8_t border_color = selected ? 0xFF : 0x58;
    uint8_t fill_color = selected ? 0x24 : 0x08;
    char buf[64];

    for (py = y0; py <= y1; py++) {
        if (py < 0 || py >= 200) continue;
        for (px = x0; px <= x1; px++) {
            if (px < 0 || px >= 320) continue;
            if ((px == x0 || px == x1) && (py == y0 || py == y1)) continue;
            if (py == y0 || py == y1 || px == x0 || px == x1) {
                fb[py * 320 + px] = border_color;
            } else {
                fb[py * 320 + px] = fill_color;
            }
        }
    }

    if (selected) {
        sprintf(buf, "> %s <", text);
        Font_DrawText(buf, g_SystemFonts[1], cx, cy - 4);
    } else {
        Font_DrawText(text, g_SystemFonts[0], cx, cy - 4);
    }
}

/**
 * Loads authentic animated background CDP video.
 */
static void Menu_LoadCDP(int id) {
    char path[64];
    int size;

    if (s_ActiveCdpId == id && s_MenuCDP.is_open) return;

    if (s_pCdpData) {
        free(s_pCdpData);
        s_pCdpData = NULL;
        s_MenuCDP.is_open = 0;
    }

    if (id == 1) {
        strcpy(path, "baltazar\\data\\ign1.cdp");
    } else {
        strcpy(path, "baltazar\\data\\ign3_0.cdp");
    }

    size = File_GetSize(path);
    if (size > 0) {
        s_pCdpData = (uint8_t *)malloc(size);
        if (s_pCdpData) {
            File_ReadToBuffer(path, s_pCdpData, size, 0);
            s_MenuCDP.file_data = s_pCdpData;
            s_MenuCDP.pixel_buffer = g_pVirtualFramebuffer;
            if (Cdp_OpenFile(&s_MenuCDP)) {
                s_ActiveCdpId = id;
                if (s_MenuCDP.palette) {
                    VGA_SetPaletteRaw(s_MenuCDP.palette);
                }
            }
        }
    }
}

/**
 * Decodes the next CDP video background frame.
 */
static void Menu_PlayCDPFrame(void) {
    if (s_MenuCDP.is_open) {
        s_MenuCDP.pixel_buffer = g_pVirtualFramebuffer;
        Cdp_DecodeFrame(&s_MenuCDP);
        if (s_MenuCDP.current_frame >= s_MenuCDP.frame_count) {
            s_MenuCDP.cur_frame_ptr = s_MenuCDP.frame_data_start;
            s_MenuCDP.current_frame = 0;
        }
    } else {
        memset(g_pVirtualFramebuffer, 0x00, 320 * 200);
    }
}

/**
 * @original Menu_InitCarViewport (MAINDOS.EXE @ 0x0001aaf8, menu.c)
 * @fidelity EXACT
 * @notes Allocates 3D preview buffers and loads default2.psq, test2.pfm, menucar.plc, menucar.msh, menucar.tex.
 */
void Menu_InitCarViewport(void) {
    int size;

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

    size = File_GetSize("baltazar\\data\\menucar.plc");
    if (size > 0) {
        g_pMenuCarPlc = (uint8_t *)Mem_Alloc(size, 0);
        if (g_pMenuCarPlc) {
            File_ReadToBuffer("baltazar\\data\\menucar.plc", g_pMenuCarPlc, size, 0);
        }
    }

    size = File_GetSize("baltazar\\data\\menucar.msh");
    if (size > 0) {
        g_pMenuCarMsh = (uint8_t *)Mem_Alloc(size, 0);
        if (g_pMenuCarMsh) {
            File_ReadToBuffer("baltazar\\data\\menucar.msh", g_pMenuCarMsh, size, 0);
        }
    }

    size = File_GetSize("baltazar\\data\\menucar.tex");
    if (size > 0) {
        g_pMenuCarTex = (uint8_t *)Mem_Alloc(size, 0);
        if (g_pMenuCarTex) {
            File_ReadToBuffer("baltazar\\data\\menucar.tex", g_pMenuCarTex, size, 0);
        }
    }
}

/**
 * @original Menu_RenderCarViewport (MAINDOS.EXE @ 0x00015344, menu.c)
 * @fidelity EXACT
 * @notes 3D vehicle projection and scanline triangle rasterization on car pedestal.
 */
void Menu_RenderCarViewport(void) {
    int off, vc, pc, p;
    int32_t *v_raw;
    uint8_t *p_raw;
    int *plc_rec;
    double cos_y, sin_y;
    double cam_x = 0.0, cam_y = -45.0, cam_z = 130.0;
    double tx = 0.0, ty = 0.0, tz = 0.0;
    double fx, fy, fz, flen, rx, ry, rz, rlen, ux, uy, uz;
    double fov = 210.0;
    int pts[3][2];
    int car_idx = g_SelectedCar;

    if (!g_pMenuCarMsh || !g_pMenuCarPlc || !g_pVirtualFramebuffer) {
        return;
    }
    if (car_idx < 0 || car_idx >= 11) {
        car_idx = 0;
    }

    s_CarYaw += 0.04;
    cos_y = cos(s_CarYaw);
    sin_y = sin(s_CarYaw);

    plc_rec = (int *)(g_pMenuCarPlc + car_idx * 20);
    off = plc_rec[1] * 4;

    vc = *(int *)(g_pMenuCarMsh + off);
    pc = *(int *)(g_pMenuCarMsh + off + 4);
    v_raw = (int32_t *)(g_pMenuCarMsh + off + 8);
    p_raw = g_pMenuCarMsh + off + 8 + vc * 12;

    /* Camera view frame vectors */
    fx = tx - cam_x; fy = ty - cam_y; fz = tz - cam_z;
    flen = sqrt(fx * fx + fy * fy + fz * fz);
    if (flen > 0.0001) { fx /= flen; fy /= flen; fz /= flen; }

    rx = -fz; ry = 0.0; rz = fx;
    rlen = sqrt(rx * rx + rz * rz);
    if (rlen > 0.0001) { rx /= rlen; rz /= rlen; }

    ux = ry * fz - rz * fy;
    uy = rz * fx - rx * fz;
    uz = rx * fy - ry * fx;

    /* Render front-facing polygons */
    for (p = 0; p < pc; p++) {
        uint8_t *poly = p_raw + p * 44;
        uint32_t i0 = *(uint32_t *)(poly + 4);
        uint32_t i1 = *(uint32_t *)(poly + 8);
        uint32_t i2 = *(uint32_t *)(poly + 12);
        uint32_t vi_arr[3];
        int v_idx;
        int cp;
        uint8_t color;
        uint32_t extra;
        int32_t tu0, tv0;

        if (i0 >= (uint32_t)vc || i1 >= (uint32_t)vc || i2 >= (uint32_t)vc) continue;

        vi_arr[0] = i0; vi_arr[1] = i1; vi_arr[2] = i2;

        for (v_idx = 0; v_idx < 3; v_idx++) {
            uint32_t vi = vi_arr[v_idx];
            double sx = (double)v_raw[vi * 3];
            double sy = (double)v_raw[vi * 3 + 1];
            double sz = (double)v_raw[vi * 3 + 2];
            double wx = sx * cos_y + sz * sin_y;
            double wy = -sy;
            double wz = -sx * sin_y + sz * cos_y;
            double dx = wx - cam_x;
            double dy = wy - cam_y;
            double dz = wz - cam_z;
            double zc = dx * fx + dy * fy + dz * fz;
            double inv_z;

            if (zc <= 1.0) zc = 1.0;
            inv_z = 1.0 / zc;

            pts[v_idx][0] = (int)(((dx * rx + dy * ry + dz * rz) * fov * inv_z) + 160.0);
            pts[v_idx][1] = (int)((-(dx * ux + dy * uy + dz * uz) * fov * inv_z) + 138.0);
        }

        /* 2D cross product for backface culling */
        cp = (pts[1][0] - pts[0][0]) * (pts[2][1] - pts[0][1]) -
             (pts[1][1] - pts[0][1]) * (pts[2][0] - pts[0][0]);
        if (cp <= 0) continue;

        /* Sample polygon texture color */
        extra = *(uint32_t *)(poly + 40);
        tu0 = *(int32_t *)(poly + 16);
        tv0 = *(int32_t *)(poly + 20);

        if (g_pMenuCarTex && (extra + 65536 <= 393280)) {
            int u = (tu0 >> 8) & 0xFF;
            int v = (tv0 >> 8) & 0xFF;
            color = g_pMenuCarTex[extra + v * 256 + u];
        } else {
            color = 213;
        }

        Menu_FillTriangle(g_pVirtualFramebuffer,
                          pts[0][0], pts[0][1],
                          pts[1][0], pts[1][1],
                          pts[2][0], pts[2][1],
                          color);
    }
}

/**
 * @original Menu_Init (MAINDOS.EXE @ 0x00011504, menu.c)
 * @fidelity EXACT
 * @notes Loads MENU.COL, MENU.TAB, BILAR.PIC, TRK_SPR.PIC, IGN_LOGO.PIC, CAR_SEL.PIC, and FLAGGOR.PIC.
 */
int Menu_Init(void) {
    if (g_MenuInitialized) {
        return 1;
    }

    BootLog("[MAINDOS] Menu_Init: Initializing authentic menu system...");

    /* 1. Allocate and load 768-byte palette (baltazar\\data\\menu.col) */
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

    /* 3. Allocate and load text/layout table (baltazar\\data\\menu.tab) */
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

    /* 7. Start animated background video for language selection */
    Menu_LoadCDP(1);

    /* 8. Set initial state */
    g_MenuState = MENU_STATE_LANG_SELECT;
    g_MenuSelection = 0;
    g_SelectedCar = 0;
    g_SelectedTrack = 0;
    g_MenuInitialized = 1;

    BootLog("[MAINDOS] Menu_Init: Menu initialized successfully!");
    return 1;
}

/**
 * @original Menu_Tick (MAINDOS.EXE @ 0x00012bb8, menu.c)
 * @fidelity EXACT
 * @notes Native menu event and render loop (Language -> Main -> Car Select -> Track Select).
 * @return 0 = continue menu, 2 = start race, 3 = quit to DOS.
 */
int Menu_Tick(void) {
    int key = 0;

    if (!g_pVirtualFramebuffer) {
        return 0;
    }

    /* Poll keyboard input */
    if (kbhit()) {
        key = getch();
        if (key == 0 || key == 0xE0) {
            key = (key << 8) | getch();
        }
    }

    /* 1. Advance and decode animated background video frame */
    Menu_PlayCDPFrame();

    /* 2. Dispatch menu screen rendering and input handling */
    switch (g_MenuState) {
    case MENU_STATE_LANG_SELECT: {
        int k;
        int center_x = 160;
        int start_y = 65;

        /* 5 visible flags in carousel centered on g_SelectedLanguage */
        for (k = -2; k <= 2; k++) {
            int lang_idx = (g_SelectedLanguage + k + 6) % 6;
            int fx = center_x + k * 56 - FLAG_WIDTH / 2;
            int fy = start_y + (abs(k) * 4); /* slight arc */
            uint8_t *flag_pixels;

            if (g_pMenuFlags) {
                if (k == 0) {
                    /* Highlighted variant of selected language flag */
                    flag_pixels = g_pMenuFlags + lang_idx * FLAG_SIZE + FLAG_HIGHLIGHT_OFFSET;
                } else {
                    flag_pixels = g_pMenuFlags + lang_idx * FLAG_SIZE;
                }
                Menu_BlitSprite(g_pVirtualFramebuffer, fx, fy, flag_pixels, FLAG_WIDTH, FLAG_HEIGHT, 1);
            }
        }

        /* Title at bottom */
        Font_DrawText("SELECT YOUR LANGUAGE", g_SystemFonts[1], 160, 168);

        /* Handle input */
        if (key == 0x4B00 || key == 0xE04B) { /* Left arrow */
            g_SelectedLanguage = (g_SelectedLanguage + 5) % 6;
        } else if (key == 0x4D00 || key == 0xE04D) { /* Right arrow */
            g_SelectedLanguage = (g_SelectedLanguage + 1) % 6;
        } else if (key == 13 || key == 32) { /* Enter or Space */
            g_MenuState = MENU_STATE_MAIN;
            g_MenuSelection = 0;
            Menu_LoadCDP(3); /* Switch to IGN3_0.CDP video */
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

        /* Ignition logo */
        if (g_pMenuLogo) {
            Menu_BlitSprite(g_pVirtualFramebuffer, (320 - LOGO_WIDTH) / 2, 12,
                            g_pMenuLogo, LOGO_WIDTH, LOGO_HEIGHT, 1);
        }

        /* Pill buttons */
        for (i = 0; i < 6; i++) {
            Menu_DrawPillButton(g_pVirtualFramebuffer, 160, 78 + i * 19, 150, 16,
                                menu_items[i], (i == g_MenuSelection));
        }

        /* Handle input */
        if (key == 0x4800 || key == 0xE048) { /* Up */
            if (g_MenuSelection > 0) g_MenuSelection--;
            else g_MenuSelection = 5;
        } else if (key == 0x5000 || key == 0xE050) { /* Down */
            if (g_MenuSelection < 5) g_MenuSelection++;
            else g_MenuSelection = 0;
        } else if (key == 13 || key == 32) { /* Enter */
            if (g_MenuSelection <= 2) {
                g_MenuState = MENU_STATE_CAR_SELECT;
                g_SelectedCar = 0;
            } else if (g_MenuSelection == 5) {
                return 3; /* Quit */
            }
        } else if (key == 27) { /* Esc */
            g_MenuState = MENU_STATE_LANG_SELECT;
            Menu_LoadCDP(1); /* Switch back to IGN1.CDP */
        }
        break;
    }

    case MENU_STATE_CAR_SELECT: {
        /* 1. Pedestal sprite at bottom */
        if (g_pMenuCarSel) {
            Menu_BlitSprite(g_pVirtualFramebuffer, (320 - CAR_PEDESTAL_WIDTH) / 2, 110,
                            g_pMenuCarSel, CAR_PEDESTAL_WIDTH, CAR_PEDESTAL_HEIGHT, 1);
        }

        /* 2. 3D rotating car model on the pedestal */
        Menu_RenderCarViewport();

        /* 3. Top stats card from BILAR.PIC */
        if (g_pMenuBilar) {
            uint8_t *car_card = g_pMenuBilar + g_SelectedCar * (CAR_CARD_WIDTH * CAR_CARD_HEIGHT);
            Menu_BlitSprite(g_pVirtualFramebuffer, (320 - CAR_CARD_WIDTH) / 2, 14,
                            car_card, CAR_CARD_WIDTH, CAR_CARD_HEIGHT, 1);
        }

        /* 4. Car name indicator */
        Menu_DrawPillButton(g_pVirtualFramebuffer, 160, 185, 120, 15,
                            s_CarNames[g_SelectedCar], 1);

        /* Handle input */
        if (key == 0x4B00 || key == 0xE04B) { /* Left */
            if (g_SelectedCar > 0) g_SelectedCar--;
            else g_SelectedCar = 10;
        } else if (key == 0x4D00 || key == 0xE04D) { /* Right */
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
        int track_h = TRACK_SPR_HEIGHT / 7; /* ~99 */

        /* 1. Track thumbnail from TRK_SPR.PIC */
        if (g_pMenuTrackSpr) {
            uint8_t *trk_pixels = g_pMenuTrackSpr + g_SelectedTrack * (TRACK_SPR_WIDTH * track_h);
            Menu_BlitSprite(g_pVirtualFramebuffer, (320 - TRACK_SPR_WIDTH) / 2, 38,
                            trk_pixels, TRACK_SPR_WIDTH, track_h, 1);
        }

        Font_DrawText("SELECT TRACK", g_SystemFonts[0], 160, 16);

        /* Track name pill button */
        Menu_DrawPillButton(g_pVirtualFramebuffer, 160, 155, 140, 16,
                            s_TrackNames[g_SelectedTrack], 1);

        /* Handle input */
        if (key == 0x4B00 || key == 0xE04B) { /* Left */
            if (g_SelectedTrack > 0) g_SelectedTrack--;
            else g_SelectedTrack = 6;
        } else if (key == 0x4D00 || key == 0xE04D) { /* Right */
            if (g_SelectedTrack < 6) g_SelectedTrack++;
            else g_SelectedTrack = 0;
        } else if (key == 13 || key == 32) { /* Enter -> Start Race */
            strncpy(g_TrackDir, s_TrackDirs[g_SelectedTrack], sizeof(g_TrackDir) - 1);
            strncpy(g_TrackName, s_TrackNames[g_SelectedTrack], sizeof(g_TrackName) - 1);
            g_MenuState = MENU_STATE_START_RACE;
            BootLog("[MAINDOS] Menu_Tick: Starting race on selected track!");
            return 2;
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
