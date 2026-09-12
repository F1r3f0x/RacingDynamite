/*
 * lisa3d.c - Lisa 2 3D Rasterizer, Panorama Sky Renderer & CDP Animation System
 * Original file: lisa3d.c
 * Target: MAINDOS.EXE / IGN_WIN.EXE (Watcom C/C++ 10.6, 32-bit flat protected mode)
 */

#include "lisa3d.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Global active panorama backdrop buffer (64KB, 256x256 8bpp) */
uint8_t *g_pActivePAN = NULL;
uint8_t *g_pLisaTransparencyLUT = NULL;
uint8_t *g_pVirtualFramebuffer = NULL;
int g_ScreenWidth = 640;
int g_ScreenHeight = 480;
float g_CameraYaw = 0.0f;
float g_CameraPitch = 0.0f;

/**
 * @original Lisa_PrintVersion (IGN_WIN.EXE @ 0x0045b4f0, lisa3d.c)
 * @fidelity EXACT
 * @notes Prints Lisa 2 Development System version and UDS copyright header.
 */
int Lisa_PrintVersion(void) {
    printf("Lisa 2 Development System: %s\n", "Compilation 0.91.0");
    printf("Copyright (c) UDS, 1995-1996\n");
    return 0;
}

/**
 * @original Cdp_OpenFile (IGN_WIN.EXE @ 0x00412580, lisa3d.c)
 * @fidelity EXACT
 * @notes Validates CDP header magic "CDP\0", version 100, parses dimensions,
 *        frame count, and binds 768-byte embedded palette and frame stream offsets.
 */
int Cdp_OpenFile(CdpFile *cdp) {
    char *hdr;

    if (cdp == NULL || cdp->file_data == NULL) {
        return 0;
    }

    hdr = (char *)cdp->file_data;
    cdp->is_open = 0;

    if (hdr[0] == 'C' && hdr[1] == 'D' && hdr[2] == 'P' && hdr[3] == '\0') {
        if (*(int16_t *)(hdr + 4) != 100) {
            return 0;
        }

        cdp->frame_count = *(int16_t *)(hdr + 6);
        cdp->loop_flag = *(int16_t *)(hdr + 8);
        cdp->width = *(int16_t *)(hdr + 10);
        cdp->height = *(int16_t *)(hdr + 12);
        cdp->palette = (uint8_t *)(hdr + 0x10);
        cdp->current_frame = 0;
        cdp->is_open = 1;
        cdp->frame_data_start = (uint8_t *)(hdr + 0x310);
        cdp->cur_frame_ptr = (uint8_t *)(hdr + 0x310);
        return 1;
    }

    return 0;
}

/**
 * @original Cdp_DecompressRLE (IGN_WIN.EXE @ 0x00499abc, lisa3d.c)
 * @fidelity EXACT
 * @notes Delta-skip RLE decompression decoding skip commands (0xF7-0xFC),
 *        repeat runs (0xFD, 0xFE), end of frame (0xFF), and raw pixel literals.
 */
int Cdp_DecompressRLE(CdpFile *cdp, unsigned int unused) {
    uint8_t *dst;
    uint8_t *src;
    uint16_t count16;
    uint8_t count8;
    uint8_t val;
    (void)unused;

    if (cdp == NULL || cdp->pixel_buffer == NULL || cdp->cur_frame_ptr == NULL) {
        return 0;
    }

    dst = cdp->pixel_buffer;
    src = cdp->cur_frame_ptr;

    while (1) {
        uint8_t op = *src++;
        switch (op) {
            case 0xF6:
                *dst++ = *src++;
                break;
            case 0xF7:
                dst += 2;
                break;
            case 0xF8:
                dst += 3;
                break;
            case 0xF9:
                dst += 4;
                break;
            case 0xFA:
                dst += 5;
                break;
            case 0xFB:
                dst += *src++;
                break;
            case 0xFC:
                count16 = *(uint16_t *)src;
                src += 2;
                dst += count16;
                break;
            case 0xFD:
                count8 = *src++;
                val = *src++;
                memset(dst, val, count8);
                dst += count8;
                break;
            case 0xFE:
                count16 = *(uint16_t *)src;
                src += 2;
                val = *src++;
                memset(dst, val, count16);
                dst += count16;
                break;
            case 0xFF:
                cdp->cur_frame_ptr = src;
                cdp->current_frame++;
                if (cdp->current_frame >= cdp->frame_count) {
                    cdp->cur_frame_ptr = cdp->frame_data_start;
                    cdp->current_frame = 0;
                    if (cdp->loop_flag != 1) {
                        return 0;
                    }
                }
                return 1;
            default:
                *dst++ = op;
                break;
        }
    }
}

/**
 * @original Cdp_DecodeFrame (IGN_WIN.EXE @ 0x00412610, lisa3d.c)
 * @fidelity EXACT
 * @notes Advances animation stream and triggers inter-frame delta decompression.
 */
int Cdp_DecodeFrame(CdpFile *cdp) {
    if (cdp == NULL || cdp->is_open == 0) {
        return -1;
    }
    return Cdp_DecompressRLE(cdp, 0);
}

/**
 * @original Lisa_RenderPanorama (IGN_WIN.EXE @ 0x00438210, lisa3d.c)
 * @fidelity ADAPTED
 * @notes Cylindrical horizon background blitter sampling 64KB (256x256) .PAN
 *        texture based on camera yaw and pitch angles.
 */
void Lisa_RenderPanorama(void) {
    int x, y;
    int horizon_y;
    int base_u;
    int half_h;

    if (g_pActivePAN == NULL || g_pVirtualFramebuffer == NULL) {
        return;
    }

    half_h = g_ScreenHeight / 2;

    /* Compute horizontal wrap offset from camera yaw (0..2pi -> 0..255) */
    base_u = (int)((g_CameraYaw / (2.0f * (float)M_PI)) * 256.0f) % 256;
    if (base_u < 0) {
        base_u += 256;
    }

    /* Compute vertical horizon screen line from camera pitch */
    horizon_y = half_h + (int)((g_CameraPitch / 90.0f) * 64.0f);
    if (horizon_y < 0) horizon_y = 0;
    if (horizon_y > g_ScreenHeight) horizon_y = g_ScreenHeight;

    /* Render top sky/horizon rows */
    for (y = 0; y < horizon_y; y++) {
        int v = 128 - (horizon_y - y);
        uint8_t *pan_row;
        uint8_t *fb_row;

        if (v < 0) v = 0;
        if (v > 255) v = 255;

        pan_row = &g_pActivePAN[v * 256];
        fb_row = &g_pVirtualFramebuffer[y * g_ScreenWidth];

        for (x = 0; x < g_ScreenWidth; x++) {
            int u = (base_u + (x * 256) / g_ScreenWidth) % 256;
            fb_row[x] = pan_row[u];
        }
    }
}
