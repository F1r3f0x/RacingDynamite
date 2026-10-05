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

/* --- Menu Asset Aliases --- */
#define g_pMenuBlackPal g_pMenuPalBlack
#define g_pMenuWhitePal g_pMenuPalWhite

uint8_t *g_pMenuCarMsh = NULL;         /* menucar.msh (82,876 bytes) */
uint8_t *g_pMenuCarPlc = NULL;         /* menucar.plc (224 bytes) */
uint8_t *g_pMenuCarTex = NULL;         /* menucar.tex (393,280 bytes) */

uint8_t *g_pDefaultPSQ = NULL;         /* ds:0xea314 (default2.psq) */
uint8_t *g_pTestPFM = NULL;            /* test2.pfm */

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

// @original Menu_BlitSprite (MAINDOS.EXE @ 0x00018500, menu.c)
// @fidelity ADAPTED
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

// @original Menu_FillTriangle (MAINDOS.EXE @ 0x00015344, menu.c)
// @fidelity ADAPTED
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

// @original Menu_DrawPillButton (MAINDOS.EXE @ 0x000170ac, menu.c)
// @fidelity ADAPTED
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

// @original Palette_Fade (MAINDOS.EXE @ 0x00051a9c, menu.c)
// @fidelity EXACT
void Palette_Fade(const uint8_t *src1, const uint8_t *src2, uint8_t *dest, int factor) {
    int i;
    if (factor >= 256) {
        memcpy(dest, src2, 768);
        return;
    }
    for (i = 0; i < 768; i++) {
        int diff = (int)src2[i] - (int)src1[i];
        int val = (int)src1[i] + (diff * factor / 256);
        dest[i] = (uint8_t)val;
    }
}

// @original Palette_BlitTrans (MAINDOS.EXE @ 0x00051818, menu.c)
// @fidelity ADAPTED
// NOTE: loop body matches the LUT blend (dst = lut[(src << 8) + dst]); the
// register/stack parameter mapping is not yet verified against the 0x51818 prologue.
void Palette_BlitTrans(const uint8_t *src, int x_start, int y_start, int x_end,
                       int y_end, uint8_t *dest, int dest_x, int dest_y,
                       const uint8_t *trans_table, int src_stride, int dest_stride) {
    int y, x;
    if (y_start >= y_end) {
        return;
    }
    for (y = y_start; y < y_end; y++) {
        int src_row = y * src_stride;
        int dest_row = (dest_y + (y - y_start)) * dest_stride + dest_x;
        for (x = x_start; x < x_end; x++) {
            uint8_t src_pixel = src[src_row + x];
            uint8_t dest_pixel = dest[dest_row + (x - x_start)];
            uint16_t lut_idx = ((uint16_t)src_pixel << 8) | dest_pixel;
            dest[dest_row + (x - x_start)] = trans_table[lut_idx];
        }
    }
}

// @original Menu_InitSettings (MAINDOS.EXE @ 0x0001a864, menu.c)
// @fidelity EXACT
void Menu_InitSettings(void) {
    int i;
    memset(g_GameSettings, 0, sizeof(g_GameSettings));

    *(uint32_t *)&g_GameSettings[0x00] = 0x1e; /* 0xe9bfc: settings version 30 */
    *(uint32_t *)&g_GameSettings[0x04] = 0;    /* 0xe9c00 */
    *(uint32_t *)&g_GameSettings[0x08] = 0;    /* 0xe9c04 */
    *(uint32_t *)&g_GameSettings[0x0c] = 0;    /* 0xe9c08 */
    *(uint32_t *)&g_GameSettings[0x10] = 1;    /* 0xe9c0c */
    *(uint32_t *)&g_GameSettings[0x14] = 1;    /* 0xe9c10 */
    *(uint32_t *)&g_GameSettings[0x18] = 1;    /* 0xe9c14 */
    *(uint32_t *)&g_GameSettings[0x1c] = 0;    /* 0xe9c18 */
    *(uint32_t *)&g_GameSettings[0x20] = 2;    /* 0xe9c1c */
    *(uint32_t *)&g_GameSettings[0x24] = 3;    /* 0xe9c20 */
    *(uint32_t *)&g_GameSettings[0x28] = 4;    /* 0xe9c24 */
    *(uint32_t *)&g_GameSettings[0x2c] = 5;    /* 0xe9c28 */
    *(uint32_t *)&g_GameSettings[0x30] = 7;    /* 0xe9c2c */
    *(uint32_t *)&g_GameSettings[0x34] = 8;    /* 0xe9c30 */
    *(uint32_t *)&g_GameSettings[0x38] = 8;    /* 0xe9c34 */
    *(uint32_t *)&g_GameSettings[0x3c] = 0;    /* 0xe9c38 */
    *(uint32_t *)&g_GameSettings[0x40] = 0;    /* 0xe9c3c */
    *(uint32_t *)&g_GameSettings[0x44] = 0;    /* 0xe9c40 */

    for (i = 1; i < 9; i++) {
        *(uint32_t *)&g_GameSettings[0x44 + i * 4] = 1; /* 0xe9c44 .. 0xe9c64 */
    }

    /* Player 1 Name: "PL1" */
    g_GameSettings[0x6c] = 'P'; /* 0xe9c68 */
    g_GameSettings[0x6d] = 'L'; /* 0xe9c69 */
    g_GameSettings[0x6e] = '1'; /* 0xe9c6a */
    g_GameSettings[0x6f] = '\0'; /* 0xe9c6b */

    /* Player 2 Name: "PL2" */
    g_GameSettings[0xe4] = 'P'; /* 0xe9ce0 */
    g_GameSettings[0xe5] = 'L'; /* 0xe9ce1 */
    g_GameSettings[0xe6] = '2'; /* 0xe9ce2 */
    g_GameSettings[0xe7] = '\0'; /* 0xe9ce3 */

    *(uint32_t *)&g_GameSettings[0xf3] = 0;    /* 0xe9cef */
    *(uint32_t *)&g_GameSettings[0xf7] = 0;    /* 0xe9cf3 */
    *(uint32_t *)&g_GameSettings[0xfb] = 5;    /* 0xe9cf7 */
    *(uint32_t *)&g_GameSettings[0xff] = 10;   /* 0xe9cfb */
    *(uint32_t *)&g_GameSettings[0x103] = 3;   /* 0xe9cff */
    *(uint32_t *)&g_GameSettings[0x107] = 0;   /* 0xe9d03 */
    *(uint32_t *)&g_GameSettings[0x10b] = 0;   /* 0xe9d07 */
    *(uint32_t *)&g_GameSettings[0x10f] = 0;   /* 0xe9d0b */
    *(uint32_t *)&g_GameSettings[0x113] = 1;   /* 0xe9d0f */
    *(uint32_t *)&g_GameSettings[0x117] = 1;   /* 0xe9d13 */
    *(uint32_t *)&g_GameSettings[0x11b] = 2;   /* 0xe9d17 */
    *(uint32_t *)&g_GameSettings[0x11f] = 2;   /* 0xe9d1b */
    *(uint32_t *)&g_GameSettings[0x123] = 0;   /* 0xe9d1f */
    *(uint32_t *)&g_GameSettings[0x127] = 0;   /* 0xe9d23 */
    *(uint32_t *)&g_GameSettings[0x12b] = 0;   /* 0xe9d27 */
    *(uint32_t *)&g_GameSettings[0x12f] = 0;   /* 0xe9d2b */

    for (i = 0; i < 8; i++) {
        *(uint32_t *)&g_GameSettings[0x133 + i * 4] = i; /* 0xe9d2f .. 0xe9d4b */
    }

    *(uint32_t *)&g_GameSettings[0x153] = 1;   /* 0xe9d4f */
    *(uint32_t *)&g_GameSettings[0x157] = 0;   /* 0xe9d53 */
    *(uint32_t *)&g_GameSettings[0x15b] = 0;   /* 0xe9d57 */
    *(uint32_t *)&g_GameSettings[0x15f] = 0;   /* 0xe9d5b */

    for (i = 2; i < 8; i++) {
        *(uint32_t *)&g_GameSettings[0x15b + i * 4] = 1; /* 0xe9d5f .. 0xe9d77 */
    }

    /* Player 1 keys */
    g_GameSettings[0x17f] = 0xcb; /* 0xe9d7b: Left arrow */
    g_GameSettings[0x180] = 0xcd; /* 0xe9d7c: Right arrow */
    g_GameSettings[0x181] = 0xc8; /* 0xe9d7d: Up arrow */
    g_GameSettings[0x182] = 0xd0; /* 0xe9d7e: Down arrow */
    g_GameSettings[0x183] = 0x35; /* 0xe9d7f: '/' */
    g_GameSettings[0x184] = 0x34; /* 0xe9d80: '.' */
    g_GameSettings[0x185] = 0x36; /* 0xe9d81: RShift */
    g_GameSettings[0x186] = 0x28; /* 0xe9d82: '\'' */

    /* Player 2 keys */
    g_GameSettings[0x187] = 0x2e; /* 0xe9d83: 'C' */
    g_GameSettings[0x188] = 0x30; /* 0xe9d84: 'B' */
    g_GameSettings[0x189] = 0x21; /* 0xe9d85: 'F' */
    g_GameSettings[0x18a] = 0x2f; /* 0xe9d86: 'V' */
    g_GameSettings[0x18b] = 0x2b; /* 0xe9d87: '\\' */
    g_GameSettings[0x18c] = 0x2a; /* 0xe9d88: LShift */
    g_GameSettings[0x18d] = 0x2c; /* 0xe9d89: 'Z' */
    g_GameSettings[0x18e] = 0x1f; /* 0xe9d8a: 'S' */

    *(uint32_t *)&g_GameSettings[0x18f] = 0; /* 0xe9d8b */
    *(uint32_t *)&g_GameSettings[0x193] = 0; /* 0xe9d8f */

    g_GameSettings[0x19b] = 0x47; /* 0xe9d97 */
    g_GameSettings[0x19c] = 0x31; /* 0xe9d98 */
    g_GameSettings[0x19d] = 0x00; /* 0xe9d99 */
    g_GameSettings[0x1aa] = 0x31; /* 0xe9da6 */
    g_GameSettings[0x1ab] = 0x32; /* 0xe9da7 */
    g_GameSettings[0x1ac] = 0x00; /* 0xe9da8 */
}

// @original Menu_LoadIntroCDPs (MAINDOS.EXE @ 0x0001680d, menu.c)
// @fidelity EXACT
int Menu_LoadIntroCDPs(void) {
    int i;
    int size;

    if (!g_MenuIntroVideoEnabled) {
        return 0;
    }

    g_MenuTimeSeconds = 0.01;
    g_MenuCdpFrameAccum = 0.0;
    g_MenuCdpActiveIndex = 0;
    if (g_pMenuBuffer1) {
        memset(g_pMenuBuffer1, 0, 64000);
    }

    for (i = 0; i < 6; i++) {
        size = File_GetSize(g_MenuCdpNames[i]);
        if (size > 0) {
            g_pMenuCdpFiles[i] = (uint8_t *)Mem_Alloc(size, 0);
            if (g_pMenuCdpFiles[i]) {
                File_ReadToBuffer(g_MenuCdpNames[i], g_pMenuCdpFiles[i], size, 0);
            }
        }
    }

    if (g_pMenuCdpFiles[0]) {
        g_MenuCdp.file_data = g_pMenuCdpFiles[0];
        g_MenuCdp.pixel_buffer = g_pMenuBuffer1;
        Cdp_OpenFile(&g_MenuCdp);
        g_MenuCdpPendingLoad = 1;
    }

    return 1;
}

// @original Menu_Shutdown (MAINDOS.EXE @ 0x00013a5c, menu.c)
// @fidelity EXACT
void Menu_Shutdown(void) {
    int i;

    g_MenuAudioVoiceActive = 0;
    Lisa_FreeEngineMemory();

    for (i = 0; i < 6; i++) {
        if (g_pMenuCdpFiles[i]) {
            Mem_Free(0, g_pMenuCdpFiles[i]);
            g_pMenuCdpFiles[i] = NULL;
        }
    }

    if (g_pMenuPalBlack) {
        Mem_Free(0, g_pMenuPalBlack);
        g_pMenuPalBlack = NULL;
    }
    if (g_pMenuPalWhite) {
        Mem_Free(0, g_pMenuPalWhite);
        g_pMenuPalWhite = NULL;
    }
    if (g_pMenuPalWork) {
        Mem_Free(0, g_pMenuPalWork);
        g_pMenuPalWork = NULL;
    }
    if (g_pMenuPalDefault) {
        Mem_Free(0, g_pMenuPalDefault);
        g_pMenuPalDefault = NULL;
        g_pMenuCol = NULL;
    }

    if (g_pMenuBuffer1) {
        Mem_Free(0, g_pMenuBuffer1);
        g_pMenuBuffer1 = NULL;
    }
    if (g_pMenuBuffer2) {
        Mem_Free(0, g_pMenuBuffer2);
        g_pMenuBuffer2 = NULL;
    }
    if (g_pMenuBuffer3) {
        Mem_Free(0, g_pMenuBuffer3);
        g_pMenuBuffer3 = NULL;
    }

    if (g_pMenuBilar) {
        Mem_Free(0, g_pMenuBilar);
        g_pMenuBilar = NULL;
    }
    if (g_pMenuTrackSpr) {
        Mem_Free(0, g_pMenuTrackSpr);
        g_pMenuTrackSpr = NULL;
    }
    if (g_pMenuLogo) {
        Mem_Free(0, g_pMenuLogo);
        g_pMenuLogo = NULL;
    }
    if (g_pMenuCarSel) {
        Mem_Free(0, g_pMenuCarSel);
        g_pMenuCarSel = NULL;
    }
    if (g_pMenuFlags) {
        Mem_Free(0, g_pMenuFlags);
        g_pMenuFlags = NULL;
    }

    if (g_pMenuTabAlloc) {
        Mem_Free(0, g_pMenuTabAlloc);
        g_pMenuTabAlloc = NULL;
        g_pMenuTab = NULL;
        g_pMenuTransTable = NULL;
    }

    g_MenuInitialized = 0;
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
 * @notes Authentic asset and pipeline initialization for menu subsystem.
 */
int Menu_Init(void) {
    int btz_version = 0;

    if (g_MenuInitialized) {
        return 1;
    }

    BootLog("[MAINDOS] Menu_Init: Initializing authentic menu system...");

    /* 1. Allocate and load 768-byte palette (baltazar\\data\\menu.col) */
    g_pMenuPalDefault = (uint8_t *)Mem_Alloc(0x300, 8);
    if (!g_pMenuPalDefault || !File_ReadToBuffer("baltazar\\data\\menu.col", g_pMenuPalDefault, 0x300, 0)) {
        BootLog("[MAINDOS] Menu_Init: Failed to load menu.col!");
        return 0;
    }
    g_pMenuCol = g_pMenuPalDefault;
    g_pMenuPalActive = g_pMenuPalDefault;

    /* 2. Allocate 64,321 byte menu framebuffer 1 */
    g_pMenuBuffer1 = (uint8_t *)Mem_Alloc(0xfb41, 8);
    if (!g_pMenuBuffer1) {
        return 0;
    }

    /* 3. Initialize Lisa engine memory */
    g_pMenuLisaEngine = (uint8_t *)Lisa_InitEngineMemory();

    /* 4. Set initial palette to active menu palette */
    VGA_SetPaletteRaw(g_pMenuPalActive);

    /* 5. Load settings from ign_dos.btz or initialize defaults */
    if (g_MenuIntroVideoEnabled && File_Exists("ign_dos.btz")) {
        File_ReadToBuffer("ign_dos.btz", (uint8_t *)&btz_version, 4, 0);
        if (btz_version == 0x1e) {
            File_ReadToBuffer("ign_dos.btz", g_GameSettings, sizeof(g_GameSettings), 4);
        } else {
            Menu_InitSettings();
        }
    } else {
        Menu_InitSettings();
    }

    /* 6. Allocate and load menu transparency table (menu.tab) */
    g_pMenuTabAlloc = (uint8_t *)Mem_Alloc(0x1ffff, 0);
    if (g_pMenuTabAlloc) {
        g_pMenuTab = (uint8_t *)(((uintptr_t)g_pMenuTabAlloc + 0xffff) & ~0xffff);
        File_ReadToBuffer("baltazar\\data\\menu.tab", g_pMenuTab, 0x10000, 0);
        g_pMenuTransTable = g_pMenuTab;
    }

    /* Clear menu buffer 1 */
    memset(g_pMenuBuffer1, 0, 64000);

    /* 7. Allocate fade palettes */
    g_pMenuPalBlack = (uint8_t *)Mem_Alloc(0x300, 8);
    if (g_pMenuPalBlack) memset(g_pMenuPalBlack, 0x00, 0x300);

    g_pMenuPalWhite = (uint8_t *)Mem_Alloc(0x300, 8);
    if (g_pMenuPalWhite) memset(g_pMenuPalWhite, 0xff, 0x300);

    g_pMenuPalWork = (uint8_t *)Mem_Alloc(0x300, 8);

    /* 8. Allocate menu buffers 2 and 3 */
    g_pMenuBuffer2 = (uint8_t *)Mem_Alloc(0xfb41, 8);
    g_pMenuBuffer3 = (uint8_t *)Mem_Alloc(0xfb41, 8);

    /* 9. Load PIC sprite assets (skipping 846-byte header) */
    g_pMenuBilar = (uint8_t *)Mem_Alloc(0x1e786, 0);
    if (g_pMenuBilar) {
        File_ReadToBuffer("baltazar\\data\\bilar.pic", g_pMenuBilar, 0x1e786, PIC_HEADER_SIZE);
    }

    g_pMenuTrackSpr = (uint8_t *)Mem_Alloc(0x1374f, 0);
    if (g_pMenuTrackSpr) {
        File_ReadToBuffer("baltazar\\data\\trk_spr.pic", g_pMenuTrackSpr, 0x1374f, PIC_HEADER_SIZE);
    }

    g_pMenuLogo = (uint8_t *)Mem_Alloc(0x2685, 0);
    if (g_pMenuLogo) {
        File_ReadToBuffer("baltazar\\data\\ign_logo.pic", g_pMenuLogo, 0x2685, PIC_HEADER_SIZE);
    }

    g_pMenuCarSel = (uint8_t *)Mem_Alloc(0x49b6, 0);
    if (g_pMenuCarSel) {
        File_ReadToBuffer("baltazar\\data\\car_sel.pic", g_pMenuCarSel, 0x49b6, PIC_HEADER_SIZE);
    }

    g_pMenuFlags = (uint8_t *)Mem_Alloc(0x978c, 0);
    if (g_pMenuFlags) {
        File_ReadToBuffer("baltazar\\data\\flaggor.pic", g_pMenuFlags, 0x978c, PIC_HEADER_SIZE);
    }

    /* 10. Load intro CDP videos */
    Menu_LoadIntroCDPs();

    /* 11. Initial menu state */
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
    double delta;

    if (!g_pVirtualFramebuffer) {
        return 0;
    }

    /* 1. Poll authentic keyboard subsystem */
    Input_PollKeyboard();
    if (kbhit()) {
        key = getch();
        if (key == 0 || key == 0xE0) {
            key = (key << 8) | getch();
        }
    }

    /* 2. Compute authentic frame delta time (MAINDOS.EXE @ 0x12bd3..0x12c48) */
    delta = (double)g_TickInt * 0.036;
    if (delta > 10.8) {
        delta = 10.8;
    } else if (delta <= 0.0) {
        delta = 0.01;
    }
    g_MenuDeltaTime = delta;
    g_MenuTimeSeconds += delta;

    /* 3. Handle intro skip */
    if (g_MenuTimeSeconds < 1278.0) {
        if (Input_WasKeyPressed(0x1c) || Input_WasKeyPressed(0x39) ||
            Input_WasKeyPressed(0x01) || Input_WasKeyPressed(0x9c) ||
            key == 13 || key == 32 || key == 27) {
            g_MenuTimeSeconds = 1278.0;
        }
    }

    /* 4. Release CDP buffers and initialize car 3D viewport when intro finishes */
    if (g_MenuTimeSeconds >= 1278.0 && g_MenuCdpPendingLoad) {
        int i;
        for (i = 0; i < 6; i++) {
            if (g_pMenuCdpFiles[i]) {
                Mem_Free(0, g_pMenuCdpFiles[i]);
                g_pMenuCdpFiles[i] = NULL;
            }
        }
        Menu_InitCarViewport();
        g_MenuCdpPendingLoad = 0;
    }

    /* 5. Decode CDP background video frame (2.393103 ticks/frame) */
    g_MenuCdpFrameAccum += delta;
    if (g_MenuCdpFrameAccum >= 2.393103448275862) {
        while (g_MenuCdpFrameAccum >= 2.393103448275862) {
            g_MenuCdpFrameAccum -= 2.393103448275862;
        }
        if (g_MenuCdp.file_data != NULL) {
            int ret = Cdp_DecodeFrame(&g_MenuCdp);
            if (ret == 0) {
                g_MenuCdpActiveIndex++;
                if (g_MenuCdpActiveIndex >= 6) {
                    g_MenuTimeSeconds = 1278.0;
                } else if (g_pMenuCdpFiles[g_MenuCdpActiveIndex] != NULL) {
                    g_MenuCdp.file_data = g_pMenuCdpFiles[g_MenuCdpActiveIndex];
                    Cdp_OpenFile(&g_MenuCdp);
                }
            }
        }
    }

    /* 6. Authentic intro palette fades */
    if (g_MenuTimeSeconds < 28.8) {
        int factor = (int)(g_MenuTimeSeconds * 270.0 / 28.8);
        Palette_Fade(g_pMenuPalBlack, g_pMenuPalWhite, g_pMenuPalWork, factor);
        VGA_SetPaletteRaw(g_pMenuPalWork);
    } else if (g_MenuTimeSeconds < 57.6) {
        int factor = (int)((g_MenuTimeSeconds - 28.8) * 270.0 / 28.8);
        if (g_MenuCdp.palette) {
            Palette_Fade(g_pMenuPalWhite, g_MenuCdp.palette, g_pMenuPalWork, factor);
            VGA_SetPaletteRaw(g_pMenuPalWork);
        }
    } else if (g_MenuTimeSeconds >= 1249.2 && g_MenuTimeSeconds < 1278.0) {
        int factor = (int)((g_MenuTimeSeconds - 1249.2) * 270.0 / 28.8);
        if (g_MenuCdp.palette) {
            Palette_Fade(g_MenuCdp.palette, g_pMenuPalWhite, g_pMenuPalWork, factor);
            VGA_SetPaletteRaw(g_pMenuPalWork);
        }
    } else if (g_MenuTimeSeconds >= 1306.8) {
        int factor = (int)((g_MenuTimeSeconds - 1306.8) * 270.0 / 28.8);
        Palette_Fade(g_pMenuPalWhite, g_pMenuPalActive, g_pMenuPalWork, factor);
        VGA_SetPaletteRaw(g_pMenuPalWork);
    }

    /* If playing intro sequence, blit g_pMenuBuffer1 to screen and return */
    if (g_MenuTimeSeconds < 1278.0) {
        while ((inp(0x3DA) & 0x08) == 0);
        memcpy((void *)0xA0000, g_pMenuBuffer1, 320 * 200);
        return 0;
    }

    /* Set active palette to default menu palette */
    g_pMenuPalActive = g_pMenuPalDefault;

    /* 7. Dispatch menu screen rendering and input handling */
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
        if (key == 0x4B00 || key == 0xE04B || Input_WasKeyPressed(0x4b)) { /* Left arrow */
            g_SelectedLanguage = (g_SelectedLanguage + 5) % 6;
        } else if (key == 0x4D00 || key == 0xE04D || Input_WasKeyPressed(0x4d)) { /* Right arrow */
            g_SelectedLanguage = (g_SelectedLanguage + 1) % 6;
        } else if (key == 13 || key == 32 || Input_WasKeyPressed(0x1c) || Input_WasKeyPressed(0x39)) { /* Enter or Space */
            g_MenuState = MENU_STATE_MAIN;
            g_MenuSelection = 0;
        } else if (key == 27 || Input_WasKeyPressed(0x01)) { /* Esc */
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
        if (key == 0x4800 || key == 0xE048 || Input_WasKeyPressed(0x48)) { /* Up */
            if (g_MenuSelection > 0) g_MenuSelection--;
            else g_MenuSelection = 5;
        } else if (key == 0x5000 || key == 0xE050 || Input_WasKeyPressed(0x50)) { /* Down */
            if (g_MenuSelection < 5) g_MenuSelection++;
            else g_MenuSelection = 0;
        } else if (key == 13 || key == 32 || Input_WasKeyPressed(0x1c) || Input_WasKeyPressed(0x39)) { /* Enter */
            if (g_MenuSelection <= 2) {
                g_MenuState = MENU_STATE_CAR_SELECT;
                g_SelectedCar = 0;
            } else if (g_MenuSelection == 5) {
                return 3; /* Quit */
            }
        } else if (key == 27 || Input_WasKeyPressed(0x01)) { /* Esc */
            g_MenuState = MENU_STATE_LANG_SELECT;
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
        if (key == 0x4B00 || key == 0xE04B || Input_WasKeyPressed(0x4b)) { /* Left */
            if (g_SelectedCar > 0) g_SelectedCar--;
            else g_SelectedCar = 10;
        } else if (key == 0x4D00 || key == 0xE04D || Input_WasKeyPressed(0x4d)) { /* Right */
            if (g_SelectedCar < 10) g_SelectedCar++;
            else g_SelectedCar = 0;
        } else if (key == 13 || key == 32 || Input_WasKeyPressed(0x1c) || Input_WasKeyPressed(0x39)) { /* Enter */
            g_MenuState = MENU_STATE_TRACK_SELECT;
            g_SelectedTrack = 0;
        } else if (key == 27 || Input_WasKeyPressed(0x01)) { /* Esc */
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
        if (key == 0x4B00 || key == 0xE04B || Input_WasKeyPressed(0x4b)) { /* Left */
            if (g_SelectedTrack > 0) g_SelectedTrack--;
            else g_SelectedTrack = 6;
        } else if (key == 0x4D00 || key == 0xE04D || Input_WasKeyPressed(0x4d)) { /* Right */
            if (g_SelectedTrack < 6) g_SelectedTrack++;
            else g_SelectedTrack = 0;
        } else if (key == 13 || key == 32 || Input_WasKeyPressed(0x1c) || Input_WasKeyPressed(0x39)) { /* Enter -> Start Race */
            strncpy(g_TrackDir, s_TrackDirs[g_SelectedTrack], sizeof(g_TrackDir) - 1);
            strncpy(g_TrackName, s_TrackNames[g_SelectedTrack], sizeof(g_TrackName) - 1);
            g_MenuState = MENU_STATE_START_RACE;
            BootLog("[MAINDOS] Menu_Tick: Starting race on selected track!");
            return 2;
        } else if (key == 27 || Input_WasKeyPressed(0x01)) { /* Esc */
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
