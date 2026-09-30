typedef unsigned char byte;
/*
 * lisa3d.c - Lisa 2 3D Rasterizer, Panorama Sky Renderer & CDP Animation System
 * Original file: lisa3d.c
 * Target: MAINDOS_32BIT.EXE (Watcom C/C++ 10.6, 32-bit flat protected mode)
 */

#include "lisa3d.h"

#ifndef LISA3D_EXTRA_TYPES
#define LISA3D_EXTRA_TYPES
typedef unsigned char byte;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
#endif

#ifndef BYTE_DEFINED
#define BYTE_DEFINED
typedef unsigned char byte;
#endif
extern uint8_t *g_pLisaTextureSheets;
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define fsin sin
#define fcos cos
#define fpatan(y, x) atan2(y, x)
#define __ultoa ultoa

static int __ftol(void) {
    return 0;
}

/* Global active panorama backdrop buffer (64KB, 256x256 8bpp) */
uint8_t *g_pActivePAN = NULL;
uint8_t *g_pLisaTransparencyLUT = NULL;
uint8_t *g_pVirtualFramebuffer = NULL;
LisaCamera *g_LisaCamera = NULL;
extern int g_ScreenWidth;
extern int g_ScreenHeight;
float g_CameraYaw = 0.0f;
float g_CameraPitch = 0.0f;
const LisaVertex *g_pLisaVertices = NULL;
uint8_t *g_pLisaDisplayList = NULL;
int g_LisaDisplayListCount = 0;
uint32_t **g_pLisaDrawQueue = NULL;
uint32_t **g_pLisaDepthBuckets = NULL;
LisaViewportClip g_LisaViewport = {0, 0, 320, 200};
int32_t g_LisaCullFlag = 0;
int32_t g_LisaMipmapTable[16] = {0};

/**
 * @original Lisa_PrintVersion (MAINDOS_32BIT.EXE @ 0x0045b4f0, lisa3d.c)
 * @fidelity ADAPTED
 * @notes Prints Lisa 2 Development System version and UDS copyright header.
 */
int Lisa_PrintVersion(void) {
    printf("Lisa 2 Development System: %s\n", "Compilation 0.91.0");
    printf("Copyright (c) UDS, 1995-1996\n");
    return 0;
}

/**
 * @original Cdp_OpenFile (MAINDOS_32BIT.EXE @ 0x00412580, lisa3d.c)
 * @fidelity ADAPTED
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
 * @original Cdp_DecompressRLE (MAINDOS_32BIT.EXE @ 0x00499abc, lisa3d.c)
 * @fidelity ADAPTED
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
 * @original Cdp_DecodeFrame (MAINDOS_32BIT.EXE @ 0x00412610, lisa3d.c)
 * @fidelity ADAPTED
 * @notes Advances animation stream and triggers inter-frame delta decompression.
 */
int Cdp_DecodeFrame(CdpFile *cdp) {
    if (cdp == NULL || cdp->is_open == 0) {
        return -1;
    }
    return Cdp_DecompressRLE(cdp, 0);
}

/**
 * @original Lisa_RenderPanorama (MAINDOS_32BIT.EXE @ 0x00438210, lisa3d.c)
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
    if (horizon_y < 20) horizon_y = 20;
    if (horizon_y > g_ScreenHeight - 20) horizon_y = g_ScreenHeight - 20;

    /* Render sky gradient (rows 180..195 of PAN) */
    for (y = 0; y < horizon_y - 16; y++) {
        int sky_v = 180 + (y * 15) / (horizon_y > 16 ? (horizon_y - 16) : 1);
        uint8_t *fb_row = &g_pVirtualFramebuffer[y * g_ScreenWidth];
        memset(fb_row, g_pActivePAN[sky_v * 256], g_ScreenWidth);
    }

    /* Render mountains silhouette (rows 0..15 in PAN) */
    for (y = horizon_y - 16; y < horizon_y; y++) {
        int v = y - (horizon_y - 16);
        uint8_t *pan_row = &g_pActivePAN[v * 256];
        uint8_t *fb_row = &g_pVirtualFramebuffer[y * g_ScreenWidth];

        for (x = 0; x < g_ScreenWidth; x++) {
            int u = (base_u + (x * 256) / g_ScreenWidth) % 256;
            fb_row[x] = pan_row[u];
        }
    }

    /* Render ground below horizon: perspective track & terrain */
    for (y = horizon_y; y < g_ScreenHeight; y++) {
        int ground_v = 200 + ((y - horizon_y) * 20) / (g_ScreenHeight - horizon_y);
        uint8_t *fb_row = &g_pVirtualFramebuffer[y * g_ScreenWidth];
        uint8_t ground_color = g_pActivePAN[ground_v * 256];
        int road_center = g_ScreenWidth / 2;
        int road_width = ((y - horizon_y) * 180) / (g_ScreenHeight - horizon_y);

        for (x = 0; x < g_ScreenWidth; x++) {
            if (x >= road_center - road_width && x <= road_center + road_width) {
                int is_curb = (x < road_center - road_width + 5) || (x > road_center + road_width - 5);
                int is_centerline = (abs(x - road_center) < 2) && (((y + (int)(g_CameraYaw * 60.0f)) / 4) % 2 == 0);
                if (is_centerline) {
                    fb_row[x] = 250;
                } else if (is_curb) {
                    fb_row[x] = (((y + (int)(g_CameraYaw * 60.0f)) / 4) % 2 == 0) ? 160 : 250;
                } else {
                    fb_row[x] = 236;
                }
            } else {
                fb_row[x] = ground_color;
            }
        }
    }
}

/**
 * @original Lisa_DrawTexturedTriangle_Op11_Unshaded (MAINDOS_32BIT.EXE @ 0x0044dc60, lisa3d.c)
 * @fidelity ADAPTED
 * @notes MAINDOS @ 0x0004d718. Processes Opcode 0x11 (textured triangle, unshaded, 44-byte packet)
 *        from display list: screen viewport clipping, 2D backface culling, depth calculation,
 *        area-based mipmap selection, and linked-list insertion into 0..5999 depth buckets.
 */
void Lisa_DrawTexturedTriangle_Op11_Unshaded(void) {
    const LisaVertex *verts = g_pLisaVertices;
    uint8_t *prim = g_pLisaDisplayList;
    int count = g_LisaDisplayListCount;
    int v0_idx, v1_idx, v2_idx;
    const int32_t *v0, *v1, *v2;
    int32_t v0_x, v0_y, v0_z;
    int32_t v1_x, v1_y, v1_z;
    int32_t v2_x, v2_y, v2_z;
    int max_x, min_x, max_y, min_y;
    int32_t cross, sum_z, depth;
    int32_t a0, a1, a2, a3, a4, a5, screen_area;
    int32_t u0, v0_uv, u1, v1_uv, u2, v2_uv, uv_area;
    int mip;
    uint32_t *node;
    uint32_t **next_queue;
    uint32_t *head;
    int32_t tex_idx, shift_u, shift_v;

    while (count > 0 && *prim == 0x11) {
        v0_idx = *(const int32_t *)(prim + 0x04) * 3;
        v1_idx = *(const int32_t *)(prim + 0x08) * 3;
        v2_idx = *(const int32_t *)(prim + 0x0C) * 3;
        v0 = (const int32_t *)((const uint8_t *)verts + v0_idx * 4);
        v1 = (const int32_t *)((const uint8_t *)verts + v1_idx * 4);
        v2 = (const int32_t *)((const uint8_t *)verts + v2_idx * 4);
        v0_x = v0[0]; v0_y = v0[1]; v0_z = v0[2];
        v1_x = v1[0]; v1_y = v1[1]; v1_z = v1[2];
        v2_x = v2[0]; v2_y = v2[1]; v2_z = v2[2];

        /* Screen boundary clipping check */
        if ((v0_y > g_LisaViewport.max_y) || (v0_x > g_LisaViewport.max_x) ||
            (v0_y < g_LisaViewport.min_y) || (v0_x < g_LisaViewport.min_x)) {
            max_x = v0_x > v1_x ? v0_x : v1_x;
            if (v2_x > max_x) max_x = v2_x;
            min_x = v0_x < v1_x ? v0_x : v1_x;
            if (v2_x < min_x) min_x = v2_x;
            max_y = v0_y > v1_y ? v0_y : v1_y;
            if (v2_y > max_y) max_y = v2_y;
            min_y = v0_y < v1_y ? v0_y : v1_y;
            if (v2_y < min_y) min_y = v2_y;

            if (max_x < g_LisaViewport.min_x || min_x > g_LisaViewport.max_x ||
                max_y < g_LisaViewport.min_y || min_y > g_LisaViewport.max_y) {
                prim += 0x2C;
                count--;
                continue;
            }
        }

        /* Backface culling: 2D cross product */
        cross = ((v1_y >> 4) - (v0_y >> 4)) * ((v2_x >> 4) - (v1_x >> 4)) +
                ((v2_y >> 4) - (v1_y >> 4)) * ((v0_x >> 4) - (v1_x >> 4));
        cross ^= g_LisaCullFlag;
        sum_z = v0_z + v1_z + v2_z;
        if (sum_z > 600 && cross > 0) {
            depth = sum_z >> 4;

            /* Screen area calculation */
            a0 = (v0_y >> 8) + (v2_y >> 8);
            a1 = (v0_x >> 8) - (v2_x >> 8);
            a2 = (v1_y >> 8) + (v2_y >> 8);
            a3 = (v2_x >> 8) - (v1_x >> 8);
            a4 = (v1_y >> 8) + (v0_y >> 8);
            a5 = (v1_x >> 8) - (v0_x >> 8);
            screen_area = (a0 * a1 + a2 * a3 + a4 * a5) * 3;
            if (screen_area < 0) screen_area = -screen_area;
            screen_area >>= 1;

            /* Texture UV area calculation */
            u0 = *(const int32_t *)(prim + 0x10) >> 8;
            v0_uv = *(const int32_t *)(prim + 0x14) >> 8;
            u1 = *(const int32_t *)(prim + 0x18) >> 8;
            v1_uv = *(const int32_t *)(prim + 0x1C) >> 8;
            u2 = *(const int32_t *)(prim + 0x20) >> 8;
            v2_uv = *(const int32_t *)(prim + 0x24) >> 8;

            uv_area = (v0_uv + v2_uv) * (u0 - u2) +
                      (v1_uv + v2_uv) * (u2 - u1) +
                      (v1_uv + v0_uv) * (u1 - u0);
            if (uv_area < 0) uv_area = -uv_area;
            mip = -4;
            if (uv_area < screen_area) {
                mip = -5;
            } else if (screen_area * 4 <= uv_area) {
                mip = (screen_area * 16 <= uv_area) ? -2 : -3;
            }

            node = *g_pLisaDrawQueue;
            node[1] = (uint32_t)v0_x;
            node[2] = (uint32_t)v0_y;
            node[3] = (uint32_t)v1_x;
            node[4] = (uint32_t)v1_y;
            node[5] = (uint32_t)v2_x;
            node[6] = (uint32_t)v2_y;
            node[7] = (uint32_t)(uintptr_t)(prim + 0x10);

            if (mip == -5 && g_LisaMipmapTable[-5 + 5] != g_LisaMipmapTable[0]) {
                tex_idx = *(const int32_t *)(prim + 0x28);
                shift_u = (*(const int32_t *)(prim + 0x10)) >> 14;
                shift_v = (*(const int32_t *)(prim + 0x14)) >> 14;
                node[0] = 0x16; /* Mipmapped opcode */
                node[8] = (uint32_t)((tex_idx + shift_v * 0x4000) * 4 + g_LisaMipmapTable[shift_u * 4]);
            } else {
                node[0] = 0x11; /* Standard opcode */
                node[8] = (uint32_t)(g_LisaMipmapTable[mip + 5] + *(const int32_t *)(prim + 0x28));
            }

            /* Depth clamping & bucket insertion */
            if (depth > 5999) depth = 5999;
            if (depth < 0) depth = 0;
            next_queue = g_pLisaDrawQueue + 2;
            head = g_pLisaDepthBuckets[depth];
            *next_queue = node + 9;
            *(g_pLisaDrawQueue + 1) = head;
            g_pLisaDepthBuckets[depth] = (uint32_t *)g_pLisaDrawQueue;
            g_pLisaDrawQueue = next_queue;
        }

        prim += 0x2C;
        count--;
    }

    g_pLisaDisplayList = prim;
    g_LisaDisplayListCount = count;
}

/**
 * @original Lisa_DrawTexturedTriangle_Op11_Shaded (MAINDOS_32BIT.EXE @ 0x0044e1b0, lisa3d.c)
 * @fidelity ADAPTED
 * @notes Processes Opcode 0x11 (textured triangle with Gouraud shading, 56-byte packet)
 *        from display list: screen boundary clipping, 2D backface culling, depth calculation,
 *        area-based mipmap selection, and linked-list insertion into 0..5999 depth buckets with vertex colors.
 */
void Lisa_DrawTexturedTriangle_Op11_Shaded(void) {
    const LisaVertex *verts = g_pLisaVertices;
    uint8_t *prim = g_pLisaDisplayList;
    int count = g_LisaDisplayListCount;
    int v0_idx, v1_idx, v2_idx;
    const int32_t *v0, *v1, *v2;
    int32_t v0_x, v0_y, v0_z;
    int32_t v1_x, v1_y, v1_z;
    int32_t v2_x, v2_y, v2_z;
    int max_x, min_x, max_y, min_y;
    int32_t cross, sum_z, depth;
    int32_t a0, a1, a2, a3, a4, a5, screen_area;
    int32_t u0, v0_uv, u1, v1_uv, u2, v2_uv, uv_area;
    int mip;
    uint32_t *node;
    uint32_t **next_queue;
    uint32_t *head;
    int32_t tex_idx, shift_u, shift_v;

    while (count > 0 && *prim == 0x11) {
        v0_idx = *(const int32_t *)(prim + 0x04) * 3;
        v1_idx = *(const int32_t *)(prim + 0x08) * 3;
        v2_idx = *(const int32_t *)(prim + 0x0C) * 3;
        v0 = (const int32_t *)((const uint8_t *)verts + v0_idx * 4);
        v1 = (const int32_t *)((const uint8_t *)verts + v1_idx * 4);
        v2 = (const int32_t *)((const uint8_t *)verts + v2_idx * 4);
        v0_x = v0[0]; v0_y = v0[1]; v0_z = v0[2];
        v1_x = v1[0]; v1_y = v1[1]; v1_z = v1[2];
        v2_x = v2[0]; v2_y = v2[1]; v2_z = v2[2];

        /* Screen boundary clipping check */
        if ((v0_y > g_LisaViewport.max_y) || (v0_x > g_LisaViewport.max_x) ||
            (v0_y < g_LisaViewport.min_y) || (v0_x < g_LisaViewport.min_x)) {
            max_x = v0_x > v1_x ? v0_x : v1_x;
            if (v2_x > max_x) max_x = v2_x;
            min_x = v0_x < v1_x ? v0_x : v1_x;
            if (v2_x < min_x) min_x = v2_x;
            max_y = v0_y > v1_y ? v0_y : v1_y;
            if (v2_y > max_y) max_y = v2_y;
            min_y = v0_y < v1_y ? v0_y : v1_y;
            if (v2_y < min_y) min_y = v2_y;

            if (max_x < g_LisaViewport.min_x || min_x > g_LisaViewport.max_x ||
                max_y < g_LisaViewport.min_y || min_y > g_LisaViewport.max_y) {
                prim += 0x38;
                count--;
                continue;
            }
        }

        /* Backface culling: 2D cross product */
        cross = ((v1_y >> 4) - (v0_y >> 4)) * ((v2_x >> 4) - (v1_x >> 4)) +
                ((v2_y >> 4) - (v1_y >> 4)) * ((v0_x >> 4) - (v1_x >> 4));
        cross ^= g_LisaCullFlag;
        sum_z = v0_z + v1_z + v2_z;
        if (sum_z > 600 && cross > 0) {
            depth = sum_z >> 4;

            /* Screen area calculation */
            a0 = (v0_y >> 8) + (v2_y >> 8);
            a1 = (v0_x >> 8) - (v2_x >> 8);
            a2 = (v1_y >> 8) + (v2_y >> 8);
            a3 = (v2_x >> 8) - (v1_x >> 8);
            a4 = (v1_y >> 8) + (v0_y >> 8);
            a5 = (v1_x >> 8) - (v0_x >> 8);
            screen_area = (a0 * a1 + a2 * a3 + a4 * a5) * 3;
            if (screen_area < 0) screen_area = -screen_area;
            screen_area >>= 1;

            /* Texture UV area calculation */
            u0 = *(const int32_t *)(prim + 0x10) >> 8;
            v0_uv = *(const int32_t *)(prim + 0x14) >> 8;
            u1 = *(const int32_t *)(prim + 0x18) >> 8;
            v1_uv = *(const int32_t *)(prim + 0x1C) >> 8;
            u2 = *(const int32_t *)(prim + 0x20) >> 8;
            v2_uv = *(const int32_t *)(prim + 0x24) >> 8;

            uv_area = (v0_uv + v2_uv) * (u0 - u2) +
                      (v1_uv + v2_uv) * (u2 - u1) +
                      (v1_uv + v0_uv) * (u1 - u0);
            if (uv_area < 0) uv_area = -uv_area;
            mip = -4;
            if (uv_area < screen_area) {
                mip = -5;
            } else if (screen_area * 4 <= uv_area) {
                mip = (screen_area * 16 <= uv_area) ? -2 : -3;
            }

            node = *g_pLisaDrawQueue;
            node[1] = (uint32_t)v0_x;
            node[2] = (uint32_t)v0_y;
            node[3] = (uint32_t)v1_x;
            node[4] = (uint32_t)v1_y;
            node[5] = (uint32_t)v2_x;
            node[6] = (uint32_t)v2_y;
            node[7] = (uint32_t)(uintptr_t)(prim + 0x10);

            if (mip == -5 && g_LisaMipmapTable[-5 + 5] != g_LisaMipmapTable[0]) {
                tex_idx = *(const int32_t *)(prim + 0x28);
                shift_u = (*(const int32_t *)(prim + 0x10)) >> 14;
                shift_v = (*(const int32_t *)(prim + 0x14)) >> 14;
                node[0] = 0x17; /* Mipmapped shaded opcode */
                node[8] = (uint32_t)((tex_idx + shift_v * 0x4000) * 4 + g_LisaMipmapTable[shift_u * 4]);
            } else {
                node[0] = 0x12; /* Shaded opcode */
                node[8] = (uint32_t)(g_LisaMipmapTable[mip + 5] + *(const int32_t *)(prim + 0x28));
            }
            node[9] = (uint32_t)(uintptr_t)(prim + 0x2C); /* Gouraud colors c0, c1, c2 */

            /* Depth clamping & bucket insertion */
            if (depth > 5999) depth = 5999;
            if (depth < 0) depth = 0;
            next_queue = g_pLisaDrawQueue + 2;
            head = g_pLisaDepthBuckets[depth];
            *next_queue = node + 10;
            *(g_pLisaDrawQueue + 1) = head;
            g_pLisaDepthBuckets[depth] = (uint32_t *)g_pLisaDrawQueue;
            g_pLisaDrawQueue = next_queue;
        }

        prim += 0x38;
        count--;
    }

    g_pLisaDisplayList = prim;
    g_LisaDisplayListCount = count;
}

/* Forward prototypes for Lisa 3D rendering pipeline */
int Lisa_RenderScene(void);
int * Lisa_InitEngineMemory(void);
void Lisa_FreeEngineMemory(void);
int Lisa_InitSpatialGrid(int grid_w, int grid_h, size_t max_objects);
LisaReturn64 Lisa_CreateDynamicObject(int obj_type, int object_id, LisaEntityTransform *entity, MshSubmesh *mesh, int mesh_flag1, short mesh_flag2, short mesh_flag3, short base_elevation, short mesh_flag4);
int Lisa_MoveDynamicObject(LisaEntityTransform *entity);
int Lisa_UpdateObjectSpatialGrid(LisaEntityTransform *entity);
LisaReturn64 Lisa_SetDynamicObjectMesh(int obj_type, int render_flags, LisaEntityTransform *entity, MshSubmesh *mesh, int mesh_flag1, short mesh_flag2, short mesh_flag3, short base_elevation, short mesh_flag4);
int Lisa_DeleteDynamicObject(LisaEntityTransform *entity);
LisaReturn64 Lisa_SetCameraViewport(void);
int Lisa_GenerateMipmaps(unsigned int *texture_data, int width, int height, int page_w, int page_h, int mip_count, int flag1, int flag2, char *name);
void Lisa_GenerateTextureSpanTable(int src_w, int src_h, int dst_w, int dst_h, int *span_table);
void Lisa_DownsampleTextureMipmap(byte *src, byte *dst, int src_w, int src_h, int dst_w, int dst_h);
void Lisa_FilterTextureBlock(byte *src, int dst, int width, int height, int stride, int color_map, int flags, int radius);
void Lisa_LoadOrCreateShadingTable(int shade_level, int lighting_mode);
unsigned int Lisa_FindClosestPaletteColor(int *rgb, int palette_offset);
LisaReturn64 Lisa_RenderSkyBackdrop(void);
int Lisa_CullObjectsOrthographic(void);
int Lisa_FrustumCullObjects(void);
int Lisa_CullObjects(void);
void Lisa_TransformVertices(void);
int Lisa_TransformVerticesPanorama(void);
int Lisa_ComputeObjectMatrix(int pos_x, int pos_y, int pos_z, int rot_y, int *out_val_matrix);
int Lisa_TransformSubmeshVerticesPanorama(int pos_x, int pos_y, int pos_z, int rot_y, int *out_val_matrix);
void Lisa_ComputeCameraRotationMatrix(int *out_val_matrix);
void Lisa_InitOpcodeTable(void);
int Lisa_SortDepthBuckets(void);
void Lisa_DrawTriangle_Op0F(void);
void Lisa_DrawTriangle_Op10(void);
int Lisa_RenderSubmeshes(void);
void Lisa_DrawTriangle_OpcodeHelper(int shd_table, int depth_bias);
void Lisa_DrawTriangle_Op14(void);
void Lisa_DrawBillboard_Op07(void);
void Lisa_DrawBillboard_Op08(void);
void Lisa_DrawTexturedTriangle_Op15(void);
void Lisa_DrawTexturedTriangle_Op15_Sub(void);
LisaReturn64 Lisa_InitRasterizerTables(int mode, unsigned int flags);
LisaReturn64 Lisa_ExecuteRasterizerCommands(int mode, unsigned int flags);

/* --- Automatically extracted globals for Lisa 3D rendering pipeline --- */

extern LisaDynamicObject **g_ppLisaNextFreeObject;
extern int g_LisaTexture_Base;
extern int g_LisaGridCellsX;
extern LisaDynamicObject **g_pLisaFreeObjectsArray;
extern int *g_pLisaSubmeshPolygon;
extern int *g_pLisaDrawCommandWritePtr;
extern int *g_pLisaActiveMipTable;
extern int *g_LisaDrawCommandBuffer;
/* extern int *g_ppLisaNextFreeObject; */
extern void *g_LisaOpcodeTable[];
extern void *g_LisaRasterizerJmpTable[];
extern void *g_pLisaShutdownCallbacks[];
extern void *g_LisaRenderTexturedOp15_FuncPtr;
extern void *g_LisaRenderTexturedOp11_FuncPtr;
extern void *g_LisaRasterizerJmpTable[];
extern void *g_pLisaShutdownCallbacks[];
extern void *g_LisaOpcodeTable[];
extern void *g_LisaRenderTexturedOp11_FuncPtr;
extern void *g_LisaRenderTexturedOp15_FuncPtr;
extern int s_rb;
extern int g_ViewportMinX;
extern int g_ViewportMinY;
extern int g_ViewportMaxX;
extern int g_ViewportMaxY;
extern int g_LisaDefaultOffset_X;
extern int g_LisaDefaultOffset_Y;
extern int g_LisaDefaultOffset_Z;
extern int g_LisaDefaultScale_X;
extern int g_LisaDefaultScale_Y;
extern int g_LisaBackfaceSign;
extern int g_LisaShadingEnabled;
extern int g_LisaDisableFiltering;
extern int g_LisaEnableMipmaps;
extern int g_LisaMipmapQuality;
extern int s_tab_tab;
extern int s_pal_checksum_fmt;
extern int s_pal_chk_str1;
extern int s_pal_chk_str2;
extern int g_SubpixelMinX;
extern int g_SubpixelMinY;
extern int g_SubpixelMaxX;
extern int g_SubpixelMaxY;
extern int g_LisaScanlinePitch;
extern int g_LisaScreenPitch;
extern int g_LisaActiveTextureID;
extern int g_LisaCameraOffsetX;
extern int g_LisaCameraOffsetY;
extern int g_LisaCameraDistance;
extern int g_LisaCameraPitch;
extern int g_LisaCameraYaw;
extern int g_LisaCameraMatrix_X;
extern int g_LisaCameraRoll;
extern int g_LisaCameraMatrix_00;
extern int g_LisaCameraMatrix_01;
extern int g_LisaCameraMatrix_02;
extern int g_LisaCameraMatrix_10;
extern int g_LisaCameraMatrix_11;
extern int g_LisaCameraMatrix_12;
extern int g_LisaCameraMatrix_20;
extern int g_LisaCameraMatrix_21;
extern int g_LisaCameraMatrix_22;
extern int g_LisaCameraFocalScale;
extern int g_LisaFrustumPlaneLeft;
extern int g_LisaFrustumPlaneRight;
extern int g_LisaFrustumPlaneTop;
extern int g_LisaFrustumPlaneBottom;
extern int g_LisaFrustumNear;
extern int g_LisaObjectMatrix_22;
extern int g_LisaPerspectiveDepthTable;
extern int g_pLisaActiveSubmesh;
extern int g_LisaActiveSubmeshFlags;
extern int g_LisaCurrentVertexIndex;
extern int g_LisaSubmeshFlags;
extern int g_LisaSubmeshLodLevel;
extern int g_LisaSubmeshPolyCount;
extern int g_LisaActiveLightingMode;
extern int g_LisaActiveMaterial;
extern int g_LisaTransformedVertices;
extern int g_LisaSubmeshVertexStride;
extern int g_LisaSubmeshClipMask;
extern int g_LisaSubmeshBoundRadius;
extern int g_LisaSubmeshVertexCount;
extern int g_LisaSubmeshPolyStride;
extern int g_LisaSubmeshCenterWorldX;
extern int g_LisaSubmeshCenterWorldY;
extern int g_LisaSubmeshCenterWorldZ;
extern int g_LisaCameraFocalLength;
extern int g_LisaSubmeshDepthOffset;
extern LisaDynamicObject **g_pLisaGridCells;
extern int g_pActiveSHD;
extern int g_LisaActiveShdSize;
extern int g_LisaActiveTabSize;
extern int g_pLisaTexturePagePointers;
extern int g_LisaTexturePageSizes;
extern int g_pLisaTexturePageTable1;
extern int g_pLisaTexturePageTable2;
extern int g_LisaActivePageCount;
extern int g_LisaGridCellsX;
extern int g_LisaDrawCommands;
/* g_pLisaFreeObjectsArray declared earlier */
extern int g_LisaVisibleObjects;
extern void **g_pLisaAllocatedBuffers;
extern int g_LisaAllocatedBufferCount;
extern int g_pLisaAllocatedBuffersEnd;
extern void *g_LisaAllocatedBufferMax;
extern int g_LisaTransformedVertices;
extern int g_LisaVisibleSubmeshes;
extern int g_pLisaDrawCommandTail;
extern int g_LisaGridWorldWidth;
extern int g_LisaGridWorldHeight;
extern double g_Const_0_1;
extern double g_Const_DegToRad;
extern double g_Const_TenthDegToRad;
extern double g_Const_Neg256;
extern double g_Const_1000;
extern double g_Const_TenthDegToRadFloat;
extern double g_Const_NegTenthDegToRad;
extern double g_Const_1024;
extern double g_Const_262144;
extern double g_Const_256;
extern double g_Const_512;
extern int *g_pLisaScanlineBuffer;
extern int *g_pLisaSpanBuffer;
extern int *g_pLisaEdgeBuffer;
extern double g_LisaCameraZoom;
extern double g_pLisaActiveShading;
extern int g_LisaClipLeft;
extern int g_LisaClipRight;
extern int g_LisaClipTop;
extern int g_LisaClipBottom;
extern int g_LisaClipSubpixelLeft;
extern int g_LisaClipSubpixelRight;
extern int g_LisaClipSubpixelTop;
extern int g_LisaClipSubpixelBottom;
extern int g_LisaViewportCenterX;
extern int g_LisaViewportCenterY;
extern int g_LisaViewportWidth;
extern int g_LisaViewportHeight;
extern double g_LisaAspectScale;
extern double g_LisaObjMat_00;
extern double g_LisaObjMat_01;
extern int g_LisaObjMat_02;
extern double g_LisaObjMat_10;
extern double g_LisaObjMat_11;
extern double g_LisaCameraMatrix_Y;
extern double g_LisaObjMat_12;
extern double g_LisaObjMat_20;
extern double g_LisaCameraMatrix_Z;
extern double g_LisaObjMat_21;
extern double g_LisaObjMat_Scale;
extern double g_LisaObjMat_CosYaw;
extern double g_LisaObjMat_SinYaw;
extern double g_LisaObjMat_CosPitch;
extern double g_LisaObjMat_SinPitch;
extern double g_LisaObjMat_CosRoll;
extern double g_LisaObjMat_SinRoll;
extern double g_LisaObjMat_Tmp1;
extern double g_LisaObjMat_Tmp2;
extern double g_LisaObjMat_Tmp3;
extern double g_LisaObjMat_Tmp4;
extern double g_LisaObjMat_Tmp5;
extern double g_LisaObjMat_Tmp6;
extern double g_LisaSubmeshTmp1;
extern double g_LisaRasterizerAccumulator;
extern int g_LisaSubmeshTmp2;
extern double g_LisaSubmeshTmp3;
extern double g_LisaSubmeshTmp4;
extern int *g_LisaSubmeshTmp5;
extern int *g_LisaSubmeshTmp6;
extern double g_LisaViewportQuarter;
extern double g_LisaViewportRemaining;

/**
 * @original Lisa_RenderScene (MAINDOS_32BIT.EXE @ 0x004466d0, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_RenderScene(void) {
  int page_base_addr;
  int *cmd_out_ptr;
  int page_idx;
  int page_size;
  int rand_val;
  int bucket_idx;
  int loop_count;
  int *queue_node;
  int cmd_index;
  int page_limit;

  g_SubpixelMinX = g_ViewportMinX << 8;
  g_SubpixelMinY = g_ViewportMinY << 8;
  g_SubpixelMaxX = (g_ViewportMaxX + 1) * 0x100;
  g_SubpixelMaxY = (g_ViewportMaxY + 1) * 0x100;

  if (g_LisaCamera->enable_sky != 0) {
    Lisa_RenderSkyBackdrop();
  }

  if (g_LisaCamera->enable_frustum_cull != 0) {
    Lisa_FrustumCullObjects();
  }

  if (g_LisaCamera->enable_transform != 0) {
    Lisa_TransformVertices();
  }

  if (g_LisaCamera->enable_submeshes != 0) {
    Lisa_RenderSubmeshes();
  }

  cmd_index = 0;

  if (g_LisaCamera->enable_depth_sort != 0) {
    for (bucket_idx = 5999; bucket_idx >= 0; bucket_idx--) {
      queue_node = (int *)g_pLisaDepthBuckets[bucket_idx];

      if (queue_node != (int *)0x0) {
        ((int *)g_LisaDrawCommands)[cmd_index] = *queue_node;
        cmd_index++;

        if (queue_node[1] != 0) {
          cmd_out_ptr = ((int *)g_LisaDrawCommands) + cmd_index;

          do {
            queue_node = (int *)queue_node[1];
            cmd_index++;
            *cmd_out_ptr = *queue_node;
            cmd_out_ptr++;
          } while (queue_node[1] != 0);
        }
      }

      g_pLisaDepthBuckets[bucket_idx] = 0;
    }
    ((int *)g_LisaDrawCommands)[cmd_index] = 0;
  }

  if (g_LisaEnableMipmaps != 0) {
    if (g_LisaEnableMipmaps < 10) {
      page_limit = g_LisaActivePageCount / 2;

      if (0 < page_limit) {
        loop_count = 0x28;

        do {
          page_idx = rand() % page_limit;
          page_base_addr = *(int *)(&g_pLisaTexturePageTable1 + page_idx * 8);
          page_size = *(int *)(&g_pLisaTexturePageTable2 + page_idx * 8);
          rand_val = rand();
          g_LisaRasterizerAccumulator = g_LisaRasterizerAccumulator + *(char *)((rand_val << 10) % page_size + page_base_addr);
          rand_val = rand();
          g_LisaRasterizerAccumulator = g_LisaRasterizerAccumulator + *(char *)((rand_val << 10) % page_size + page_base_addr);
          rand_val = rand();
          g_LisaRasterizerAccumulator = g_LisaRasterizerAccumulator + *(char *)((rand_val << 10) % page_size + page_base_addr);
          loop_count--;
        } while (loop_count != 0);
      }
    }
    else {
      page_limit = g_LisaActivePageCount / 2;

      if (0 < page_limit) {
        loop_count = 2;

        do {
          page_idx = rand() % page_limit;
          page_base_addr = *(int *)(&g_pLisaTexturePageTable1 + page_idx * 8);
          page_size = *(int *)(&g_pLisaTexturePageTable2 + page_idx * 8);
          rand_val = rand();
          g_LisaRasterizerAccumulator = g_LisaRasterizerAccumulator + *(char *)((rand_val << 10) % page_size + page_base_addr);
          rand_val = rand();
          g_LisaRasterizerAccumulator = g_LisaRasterizerAccumulator + *(char *)((rand_val << 10) % page_size + page_base_addr);
          rand_val = rand();
          g_LisaRasterizerAccumulator = g_LisaRasterizerAccumulator + *(char *)((rand_val << 10) % page_size + page_base_addr);
          loop_count--;
        } while (loop_count != 0);
      }
    }

    g_LisaEnableMipmaps = g_LisaEnableMipmaps + 1;
  }

  if (g_LisaEnableMipmaps == 100) {
    g_LisaEnableMipmaps = 0;
  }

  return g_LisaDrawCommands;
}

/**
 * @original Lisa_InitEngineMemory (MAINDOS_32BIT.EXE @ 0x004468d0, lisa3d.c)
 * @fidelity ADAPTED
 */
int * Lisa_InitEngineMemory(void) {
  void *draw_cmd_buf;
  void *alloc_bufs;
  void *temp_ptr1;
  void *cmd_tail;
  int *pIntPtr;
  void *shd_table;
  int i;
  int next_i;
  double sin_val;

  

  g_pLisaAllocatedBuffers = calloc(5000,4);
  g_LisaAllocatedBufferCount = 0;
  g_LisaVisibleObjects = calloc(0x5dc,4);
  g_LisaVisibleSubmeshes = calloc(3000,4);
  pIntPtr = calloc(0x1838,4);
  i = g_LisaActivePageCount + 2;
  g_pLisaDepthBuckets = pIntPtr;
  g_LisaActivePageCount = i;
  *(int **)(&g_pLisaTexturePagePointers + i * 4) = pIntPtr;
  *(int *)(&g_LisaTexturePageSizes + i * 4) = 0x1a90;

  for (i = 6000; i != 0; i = i + -1) {
    *pIntPtr = 0;
    pIntPtr = pIntPtr + 1;
  }

  g_LisaCamera = (LisaCamera *)calloc(1, sizeof(LisaCamera));
  g_LisaTransformedVertices = calloc(20000,0xc);
  g_LisaDrawCommandBuffer = calloc(14000,8);
  g_pLisaTextureSheets = calloc(1000,4);
  *(void **)(&g_pLisaTexturePageTable1 + g_LisaActivePageCount * 4) = g_pLisaTextureSheets;
  *(int *)(&g_pLisaTexturePageTable1 + (g_LisaActivePageCount + 1) * 4) = 4000;
  g_LisaActivePageCount = g_LisaActivePageCount + 2;
  g_LisaDrawCommands = g_LisaTransformedVertices;
  g_pLisaDrawCommandTail = calloc(150000,4);
  shd_table = calloc(0x1fa4,4);
  temp_ptr1 = g_LisaVisibleSubmeshes;
  alloc_bufs = g_pLisaAllocatedBuffers;
  g_LisaActivePageCount = g_LisaActivePageCount + 2;
  g_LisaActiveShdSize = shd_table;
  *(void **)(&g_pLisaTexturePagePointers + g_LisaActivePageCount * 4) = shd_table;
  *(int *)(&g_LisaTexturePageSizes + g_LisaActivePageCount * 4) = 0x2a30;
  *(void **)((int)alloc_bufs + (g_LisaAllocatedBufferCount + 3) * 4 + -0xc) = g_LisaVisibleObjects;
  draw_cmd_buf = g_LisaDrawCommandBuffer;
  pIntPtr = g_pLisaDepthBuckets;
  *(void **)((int)alloc_bufs + (g_LisaAllocatedBufferCount + 5) * 4 + -0x10) = temp_ptr1;
  temp_ptr1 = g_LisaTransformedVertices;
  *(int **)((int)alloc_bufs + (g_LisaAllocatedBufferCount + 5) * 4 + -0xc) = pIntPtr;
  cmd_tail = g_pLisaDrawCommandTail;
  pIntPtr = (int *)g_LisaCamera;
  *(LisaCamera **)((int)alloc_bufs + (g_LisaAllocatedBufferCount + 7) * 4 + -0x10) = g_LisaCamera;
  i = g_LisaAllocatedBufferCount + 9;
  g_LisaAllocatedBufferCount = i;
  *(void **)((int)alloc_bufs + i * 4 + -0x14) = temp_ptr1;
  *(void **)((int)alloc_bufs + i * 4 + -0x10) = draw_cmd_buf;
  draw_cmd_buf = g_pLisaTextureSheets;
  *(void **)((int)alloc_bufs + i * 4 + -0xc) = g_pLisaTextureSheets;
  *(void **)((int)alloc_bufs + i * 4 + -8) = cmd_tail;
  *(void **)((int)alloc_bufs + i * 4 + -4) = shd_table;

  if (pIntPtr == (int *)0x0) {
    return (int *)0x0;
  }

  g_LisaActiveShdSize = shd_table;

  if (temp_ptr1 == (void *)0x0) {
    return (int *)0x0;
  }

  if (g_LisaDrawCommandBuffer == (void *)0x0) {
    return (int *)0x0;
  }

  if (g_LisaVisibleObjects == (void *)0x0) {
    return (int *)0x0;
  }

  if (g_pLisaDrawCommandTail == (void *)0x0) {
    return (int *)0x0;
  }

  if (g_pLisaDepthBuckets == (int *)0x0) {
    return (int *)0x0;
  }

  if (shd_table == (void *)0x0) {
    return (int *)0x0;
  }

  if (draw_cmd_buf == (void *)0x0) {
    return (int *)0x0;
  }

  pIntPtr[0xe] = 1;
  pIntPtr[0xf] = 1;
  pIntPtr[0x10] = 1;
  pIntPtr[0x11] = 1;
  pIntPtr[0x12] = 1;
  pIntPtr[0x13] = 1;
  pIntPtr[0x14] = 1;
  pIntPtr[0x15] = 1;
  pIntPtr[0x16] = 1;
  pIntPtr[0x23] = 1;
  pIntPtr[0x24] = 1;
  pIntPtr[0x25] = 1;
  pIntPtr[0x20] = 0xfd;
  pIntPtr[0x21] = 0xcf;
  pIntPtr[0x22] = 0x2c;
  pIntPtr[0x26] = 10;
  pIntPtr[6] = 0;
  pIntPtr[8] = 0;
  pIntPtr[7] = 0;
  pIntPtr[9] = 0;
  pIntPtr[10] = 0;
  *pIntPtr = 0;
  pIntPtr[0xb] = 0;
  pIntPtr[1] = 0;
  pIntPtr[2] = 0;
  pIntPtr[4] = 0;
  pIntPtr[3] = 0;
  pIntPtr[5] = 0;
  pIntPtr[0xc] = 0;
  pIntPtr[0x29] = 0;
  pIntPtr[0x27] = 0xa0;
  pIntPtr[0xd] = 0x3ff00000;
  pIntPtr[0x28] = 100;
  i = -0x708;

  while( 1 ) {
    next_i = i + 1;
    sin_val = (double)fsin((double)i * (double)g_Const_0_1 * (double)g_Const_DegToRad);
    if (0x189b < next_i) break;
    *(float *)((int)shd_table + next_i * 4 + 0x1c1c) = (float)sin_val;
    i = next_i;
  }

  *(float *)((int)shd_table + next_i * 4 + 0x1c1c) = (float)sin_val;
  g_LisaActiveTabSize = (int)shd_table + 0xe10;
  return pIntPtr;
}

/**
 * @original Lisa_FreeEngineMemory (MAINDOS_32BIT.EXE @ 0x00446c30, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_FreeEngineMemory(void) {
  void *_Memory;
  int allocated_count;
  int i;
  int offset;

  

  i = 0;

  if (g_pLisaAllocatedBuffers != (void *)0x0) {
    _Memory = g_pLisaAllocatedBuffers;

    if (0 < g_LisaAllocatedBufferCount) {
      offset = 0;
      allocated_count = g_LisaAllocatedBufferCount;

      do {
        if (*(void **)(offset + (int)_Memory) != (void *)0x0) {
          free(*(void **)(offset + (int)_Memory));
          _Memory = g_pLisaAllocatedBuffers;
          allocated_count = g_LisaAllocatedBufferCount;
        }

        offset = offset + 4;
        i = i + 1;
      } while (i < allocated_count);
    }

    free(_Memory);
  }

  g_pLisaAllocatedBuffers = (void *)0x0;
  g_LisaAllocatedBufferCount = 0;
  g_LisaActivePageCount = 0;
  g_LisaEnableMipmaps = 1;
  g_LisaShadingEnabled = 0;
  return;
}

/**
 * @original Lisa_InitSpatialGrid (MAINDOS_32BIT.EXE @ 0x00446ca0, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_InitSpatialGrid(int grid_w, int grid_h, size_t max_objects) {
  int *next_free;
  void *allocated_array;
  int i;
  int old_buffer_count;

  g_LisaGridWorldWidth = grid_w;
  g_LisaGridWorldHeight = grid_h;
  
  g_LisaGridCellsX = ((grid_w + 0xff) >> 8);

  g_pLisaGridCells = (LisaDynamicObject **)calloc(((grid_h + 0xff) >> 8) * g_LisaGridCellsX, 4);
  
  g_pLisaFreeObjectsArray = (LisaDynamicObject **)calloc(max_objects + 5, 4);
  allocated_array = calloc(max_objects, sizeof(LisaDynamicObject));
  
  old_buffer_count = g_LisaAllocatedBufferCount;
  
  g_ppLisaNextFreeObject = g_pLisaFreeObjectsArray;
  
  g_LisaAllocatedBufferCount = old_buffer_count + 3;
  g_LisaAllocatedBufferMax = allocated_array;
  
  /* Register allocations to be freed later */
  g_pLisaAllocatedBuffers[old_buffer_count] = g_pLisaGridCells;
  g_pLisaAllocatedBuffers[old_buffer_count + 1] = g_pLisaFreeObjectsArray;
  g_pLisaAllocatedBuffers[old_buffer_count + 2] = allocated_array;

  if (g_pLisaGridCells != NULL && allocated_array != NULL && g_pLisaFreeObjectsArray != NULL) {
    next_free = (int *)g_pLisaFreeObjectsArray;

    if ((int)max_objects > 0) {
      for (i = 0; i < (int)max_objects; i++) {
        *next_free = (int)allocated_array;
        allocated_array = (void *)((int)allocated_array + sizeof(LisaDynamicObject));
        next_free++;
      }
    }

    g_pLisaFreeObjectsArray[max_objects] = (LisaDynamicObject *)0xffffffff;
    return 0;
  }

  return -1;
}

/**
 * @original Lisa_CreateDynamicObject (MAINDOS_32BIT.EXE @ 0x00446d90, lisa3d.c)
 * @fidelity ADAPTED
 */
LisaReturn64 Lisa_CreateDynamicObject(int obj_type, int object_id, LisaEntityTransform *entity, MshSubmesh *mesh, int mesh_flag1, short mesh_flag2, short mesh_flag3, short base_elevation, short mesh_flag4) {
  LisaDynamicObject *obj;
  int max_dist;
  int i;
  int *vertices;
  float fx, fy, fz;
  float dist;

  obj = *g_ppLisaNextFreeObject;

  if (obj == (LisaDynamicObject *)0xffffffff) {
    LisaReturn64 _r; _r.edx = object_id; _r.eax = 0xffffffff; return _r;
  }

  g_ppLisaNextFreeObject++;
  
  obj->self_ptr = obj;
  obj->mesh_data = mesh;
  obj->unknown_08 = mesh_flag1;
  obj->pos_x = entity->pos_x;
  obj->pos_y = entity->pos_y;
  obj->pos_z = entity->pos_z;
  obj->rot_x = (short)entity->rot_x;
  obj->rot_y = (short)entity->rot_y;
  obj->rot_z = (short)entity->rot_z;
  obj->unknown_1e = mesh_flag2;
  obj->unknown_20 = mesh_flag3;
  obj->unknown_22 = base_elevation;
  obj->unknown_24 = mesh_flag4;
  obj->next_in_cell = NULL;

  entity->dyn_obj = obj;
  entity->in_grid = 0;

  if (base_elevation == -1) {
    max_dist = -1000;
    
    if (mesh->vertex_count > 0) {
      vertices = (int *)((char *)mesh + 8);

      for (i = 0; i < mesh->vertex_count; i++) {
        /* If manhattan distance > max_dist, we check actual euclidean distance */
        if (max_dist < abs(vertices[0]) + abs(vertices[1]) + abs(vertices[2])) {
          fx = (float)vertices[0];
          fy = (float)vertices[1];
          fz = (float)vertices[2];
          dist = (float)sqrt(fx*fx + fy*fy + fz*fz);

          if (max_dist < (int)dist) {
            max_dist = (int)dist;
          }
        }
        vertices += 3;
      }
    }

    obj->unknown_22 = (short)max_dist;
  }

  { LisaReturn64 _r; _r.edx = 0; _r.eax = 0; return _r; }
}

/**
 * @original Lisa_MoveDynamicObject (MAINDOS_32BIT.EXE @ 0x00446eb0, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_MoveDynamicObject(LisaEntityTransform *entity) {
  LisaDynamicObject *obj;
  int cell_index;

  if (entity->in_grid != 1) {
    obj = entity->dyn_obj;
    
    obj->pos_x = entity->pos_x;
    obj->pos_y = entity->pos_y;
    obj->pos_z = entity->pos_z;
    
    obj->rot_x = (short)entity->rot_x;
    obj->rot_y = (short)entity->rot_y;
    obj->rot_z = (short)entity->rot_z;

    cell_index = (obj->pos_z >> 8) * g_LisaGridCellsX + (obj->pos_x >> 8);
    
    entity->in_grid = 1;
    
    obj->next_in_cell = g_pLisaGridCells[cell_index];
    g_pLisaGridCells[cell_index] = obj;
  }

  return 0;
}

/**
 * @original Lisa_UpdateObjectSpatialGrid (MAINDOS_32BIT.EXE @ 0x00446f30, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_UpdateObjectSpatialGrid(LisaEntityTransform *entity) {
  LisaDynamicObject *obj;
  LisaDynamicObject *curr;
  int old_cell_x, old_cell_z;
  int new_cell_x, new_cell_z;
  int old_cell_index, new_cell_index;

  obj = entity->dyn_obj;
  old_cell_x = obj->pos_x;
  old_cell_z = obj->pos_z;

  if (entity->pos_x >= 0 && entity->pos_z >= 0 && 
      entity->pos_x <= g_LisaGridWorldWidth && 
      entity->pos_z <= g_LisaGridWorldHeight) {
    
    obj->pos_x = entity->pos_x;
    obj->pos_y = entity->pos_y;
    obj->pos_z = entity->pos_z;
    obj->rot_x = (short)entity->rot_x;
    obj->rot_y = (short)entity->rot_y;
    obj->rot_z = (short)entity->rot_z;

    if (entity->in_grid == 1) {
      old_cell_x = old_cell_x >> 8;
      old_cell_z = old_cell_z >> 8;
      new_cell_x = obj->pos_x >> 8;
      new_cell_z = obj->pos_z >> 8;

      if (old_cell_x != new_cell_x || old_cell_z != new_cell_z) {
        old_cell_index = old_cell_z * g_LisaGridCellsX + old_cell_x;
        curr = g_pLisaGridCells[old_cell_index];

        if (curr == obj) {
          g_pLisaGridCells[old_cell_index] = obj->next_in_cell;
        } else {
          while (curr != NULL && curr->next_in_cell != obj) {
            curr = curr->next_in_cell;
          }
          if (curr != NULL) {
            curr->next_in_cell = obj->next_in_cell;
          }
        }

        new_cell_index = new_cell_z * g_LisaGridCellsX + new_cell_x;
        obj->next_in_cell = g_pLisaGridCells[new_cell_index];
        g_pLisaGridCells[new_cell_index] = obj;
      }
    }
    return 0;
  }

  return -1;
}

/**
 * @original Lisa_SetDynamicObjectMesh (MAINDOS_32BIT.EXE @ 0x00447070, lisa3d.c)
 * @fidelity ADAPTED
 */
LisaReturn64 Lisa_SetDynamicObjectMesh(int obj_type, int render_flags, LisaEntityTransform *entity, MshSubmesh *mesh, int mesh_flag1, short mesh_flag2, short mesh_flag3, short base_elevation, short mesh_flag4) {
  LisaDynamicObject *obj;
  int max_dist;
  int i;
  int *vertices;
  float fx, fy, fz;
  float dist;
  unsigned int ret_val;

  obj = entity->dyn_obj;
  
  obj->unknown_1e = mesh_flag2;
  obj->unknown_22 = base_elevation;
  obj->mesh_data = mesh;
  obj->unknown_08 = mesh_flag1;
  obj->unknown_20 = mesh_flag3;
  obj->unknown_24 = mesh_flag4;
  
  ret_val = ((unsigned int)render_flags >> 16) | ((unsigned int)base_elevation << 16);

  if (base_elevation == -1) {
    max_dist = -1000;
    
    if (mesh->vertex_count > 0) {
      vertices = (int *)((char *)mesh + 8);

      for (i = 0; i < mesh->vertex_count; i++) {
        if (max_dist < abs(vertices[0]) + abs(vertices[1]) + abs(vertices[2])) {
          fx = (float)vertices[0];
          fy = (float)vertices[1];
          fz = (float)vertices[2];
          dist = (float)sqrt(fx*fx + fy*fy + fz*fz);

          if (max_dist < (int)dist) {
            max_dist = (int)dist;
          }
        }
        vertices += 3;
      }
    }

    obj->unknown_22 = (short)max_dist;
  }

  { LisaReturn64 _r; _r.edx = ret_val; _r.eax = 0; return _r; }
}

/**
 * @original Lisa_DeleteDynamicObject (MAINDOS_32BIT.EXE @ 0x00447150, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_DeleteDynamicObject(LisaEntityTransform *entity) {
  LisaDynamicObject *obj;
  LisaDynamicObject *curr;
  int cell_index;

  if (entity->in_grid == 1) {
    obj = entity->dyn_obj;

    cell_index = (obj->pos_z >> 8) * g_LisaGridCellsX + (obj->pos_x >> 8);
    curr = g_pLisaGridCells[cell_index];

    if (curr == obj) {
      g_pLisaGridCells[cell_index] = obj->next_in_cell;
    } else {
      while (curr != NULL && curr->next_in_cell != obj) {
        curr = curr->next_in_cell;
      }
      if (curr != NULL) {
        curr->next_in_cell = obj->next_in_cell;
      }
    }

    entity->in_grid = 0;
  }

  return 0;
}

/**
 * @original Lisa_SetCameraViewport (MAINDOS_32BIT.EXE @ 0x004471e0, lisa3d.c)
 * @fidelity ADAPTED
 */
LisaReturn64 Lisa_SetCameraViewport(void) {
  int unused_val;
  int cam_val;
  unsigned int cam_val_u;
  LisaCamera *camera;

  cam_val = __ftol();
  camera = g_LisaCamera;
  camera->viewport_x = (int)cam_val;
  cam_val = __ftol();
  camera->viewport_y = (int)cam_val;
  cam_val = __ftol();
  camera->viewport_width = (int)cam_val;
  cam_val = __ftol();
  camera->fov_x = (int)cam_val;
  cam_val_u = __ftol();
  camera->fov_y = (int)cam_val_u;
  { LisaReturn64 _r; _r.edx = cam_val_u; _r.eax = 0; return _r; }
}

/**
 * @original Lisa_GenerateMipmaps (MAINDOS_32BIT.EXE @ 0x00447280, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_GenerateMipmaps(unsigned int *texture_data_ptr,int orig_width,int orig_height,int mip_levels,int page_width,int page_height ,int tex_height,int tex_width,char *tex_name) {
  char name_char;
  char *name_dst_ptr;
  char *name_dst_end;
  unsigned int *mip_array;
  unsigned int *mip_span_table;
  void *alloc_ptr;
  byte *aligned_page_ptr;
  int active_page_idx;
  int loop_count;
  int next_mip_idx;
  int cur_mip_idx;
  byte *dst_block_ptr;
  byte *src_block_ptr;
  unsigned int *fill_mip_ptr;
  int filtering_enabled;
  int ftol_val;
  byte *cur_src_page;
  int bucket_idx;
  byte *dst_row_ptr;
  byte *src_row_ptr;
  int x_blocks;
  int y_blocks;
  char name_buffer [1024];

  

  name_dst_ptr = name_buffer;

  do {
    name_char = tex_name[1];
    *name_dst_ptr = *tex_name;
    name_dst_ptr[1] = name_char;
    name_dst_end = name_dst_ptr + 4;
    name_dst_ptr[2] = tex_name[2];
    name_dst_ptr = name_dst_end;
    tex_name = tex_name + 3;
  } while (name_dst_end < (unsigned int *)(name_buffer + sizeof(name_buffer)));
  mip_array = calloc(0x15,4);
  g_LisaAllocatedBufferCount = g_LisaAllocatedBufferCount + 1;
  *(unsigned int **)(g_pLisaAllocatedBuffers + -4 + g_LisaAllocatedBufferCount * 4) = mip_array;
  mip_span_table = mip_array + 5;
  ftol_val = __ftol();
  bucket_idx = 0;

  if (0 < (int)ftol_val) {
    do {
      if (5999 < bucket_idx) break;
      bucket_idx = bucket_idx + 1;
    } while (bucket_idx < (int)ftol_val);
  }

  ftol_val = __ftol();
  loop_count = (int)ftol_val;
  cur_src_page = (byte *)*texture_data_ptr;
  *mip_array = (unsigned int)cur_src_page;
  mip_array[1] = (unsigned int)cur_src_page;
  mip_array[2] = (unsigned int)cur_src_page;
  mip_array[3] = (unsigned int)cur_src_page;
  mip_array[4] = (unsigned int)cur_src_page;
  *mip_span_table = (unsigned int)cur_src_page;
  next_mip_idx = 0;

  if (0 < mip_levels + -1) {
    cur_mip_idx = next_mip_idx;

    do {
      g_LisaEnableMipmaps = 1;
      g_LisaShadingEnabled = 1;
      alloc_ptr = calloc(tex_height * 0x100 + 0xffff,1);
      g_LisaAllocatedBufferCount = g_LisaAllocatedBufferCount + 1;
      *(void **)(g_pLisaAllocatedBuffers + -4 + g_LisaAllocatedBufferCount * 4) = alloc_ptr;
      aligned_page_ptr = (byte *)((int)alloc_ptr + 0xffffU & 0xffff0000);
      active_page_idx = g_LisaActivePageCount + 2;
      g_LisaActivePageCount = active_page_idx;
      *(byte **)(&g_pLisaTexturePagePointers + active_page_idx * 4) = aligned_page_ptr;
      next_mip_idx = cur_mip_idx + 1;
      *(int *)(&g_LisaTexturePageSizes + active_page_idx * 4) = tex_height << 8;

      if (next_mip_idx < 5) {
        mip_array[cur_mip_idx + 1] = (unsigned int)aligned_page_ptr;
      }

      active_page_idx = 0;

      if (0 < loop_count) {
        do {
          if (5999 < bucket_idx) break;
          active_page_idx = active_page_idx + 1;
          bucket_idx = bucket_idx + 1;
        } while (active_page_idx < loop_count);
      }

      if (0 < tex_height / page_height) {
        src_row_ptr = cur_src_page;
        loop_count = (int)(0x100 / (int)page_width);
        dst_row_ptr = aligned_page_ptr;
        y_blocks = tex_height / page_height;

        do {
          dst_block_ptr = dst_row_ptr;
          src_block_ptr = src_row_ptr;
          x_blocks = loop_count;

          if (0 < loop_count) {
            do {
              if (cur_mip_idx == 0) {
                Lisa_DownsampleTextureMipmap(src_block_ptr,dst_block_ptr,page_width,page_height,0x100,(int)name_buffer);
              }

              else {
                Lisa_FilterTextureBlock(src_block_ptr,(int)dst_block_ptr,page_width,page_height,0x100,(int)name_buffer,tex_width,

                             cur_mip_idx);
              }

              x_blocks = x_blocks + -1;
              dst_block_ptr = dst_block_ptr + page_width;
              src_block_ptr = src_block_ptr + page_width;
            } while (x_blocks != 0);
          }

          dst_row_ptr = dst_row_ptr + page_height * 0x100;
          src_row_ptr = src_row_ptr + page_height * 0x100;
          y_blocks = y_blocks + -1;
        } while (y_blocks != 0);
      }

      ftol_val = __ftol();
      loop_count = (int)ftol_val;
      cur_mip_idx = next_mip_idx;
      cur_src_page = aligned_page_ptr;
    } while (next_mip_idx < mip_levels + -1);
  }

  if (next_mip_idx < 4) {
    fill_mip_ptr = mip_array + next_mip_idx + 1;

    for (loop_count = 4 - next_mip_idx; loop_count != 0; loop_count = loop_count + -1) {
      *fill_mip_ptr = (unsigned int)cur_src_page;
      fill_mip_ptr = fill_mip_ptr + 1;
    }

  }

  filtering_enabled = g_LisaDisableFiltering == 0;
  *texture_data_ptr = (unsigned int)mip_span_table;

  if ((filtering_enabled) && (1 < mip_levels)) {
    Lisa_GenerateTextureSpanTable(*mip_array,(int)name_buffer,tex_width,tex_height,mip_span_table);
  }

  return 0;
}

/**
 * @original Lisa_GenerateTextureSpanTable (MAINDOS_32BIT.EXE @ 0x004475c0, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_GenerateTextureSpanTable(int src_tex_ptr,int palette_ptr,int dst_w,int dst_h,int *span_table) {
  unsigned int pixel_val;
  int temp_err;
  byte texel_color;
  unsigned int texel_b;
  unsigned int g00_temp;
  int i_counter;
  void *allocated_mem;
  unsigned int *diag_texel;
  byte *temp_byte_ptr;
  unsigned int texel_r;
  byte row_texel;
  int *span_ptr;
  int err_idx;
  unsigned int texel_g;
  byte next_texel;
  unsigned int *table_entry;
  unsigned int rgb_error_accum [3];
  unsigned int color_accum;
  int x_idx;
  unsigned int *table_ptr;
  unsigned int *cur_page_ptr;
  int vec_a [3];
  int b11_scaled;
  int *out_val_span;
  int y_offset;
  int row_offset;
  int y_idx;
  int *alloc_table;
  int r_interp_1;
  int g_interp_1;
  int b_interp_1;
  int b10_scaled;
  int g11_scaled;
  int g10_scaled;
  int r11_scaled;
  int r10_scaled;
  int b01_scaled;
  int b00_scaled;
  int g01_scaled;
  int g00_scaled;
  int r01_scaled;
  int r00_scaled;
  unsigned int *page_start;
  int g_interp_0;
  int b_interp_0;
  unsigned int r10;
  unsigned int g10;
  unsigned int b10;
  unsigned int r00;
  unsigned int g00;
  unsigned int b00;
  unsigned int r11;
  unsigned int g11;
  unsigned int b11;
  unsigned int r01;
  unsigned int g01;
  unsigned int b01;
  size_t page_size;
  int page_limit;
  int h_limit;

  

  alloc_table = calloc(0x10000,0x14);
  *alloc_table = 1;
  i_counter = 0x10000;
  span_ptr = alloc_table;

  do {
    *span_ptr = 0;
    span_ptr = span_ptr + 5;
    i_counter = i_counter + -1;
  } while (i_counter != 0);
  out_val_span = span_table;
  page_size = dst_h * 0x400 + 0xffff;
  y_offset = 0;
  page_limit = dst_h << 10;

  do {
    allocated_mem = calloc(page_size,1);
    g_LisaAllocatedBufferCount = g_LisaAllocatedBufferCount + 1;
    *(void **)(g_pLisaAllocatedBuffers + -4 + g_LisaAllocatedBufferCount * 4) = allocated_mem;
    diag_texel = (unsigned int *)((int)allocated_mem + 0xffffU & 0xffff0000);
    i_counter = g_LisaActivePageCount + 2;
    span_ptr = out_val_span + 4;
    g_LisaActivePageCount = i_counter;
    *(unsigned int **)(&g_pLisaTexturePagePointers + i_counter * 4) = diag_texel;
    *(int *)(&g_LisaTexturePageSizes + i_counter * 4) = page_limit;
    *out_val_span = diag_texel;
    y_idx = 0;
    out_val_span[1] = diag_texel;
    out_val_span[2] = diag_texel;
    out_val_span[3] = diag_texel;

    if (0 < dst_h) {
      h_limit = dst_h + -1;
      row_offset = 0;
      page_start = diag_texel;

      do {
        vec_a[0] = 0;
        vec_a[1] = 0;
        vec_a[2] = 0;
        x_idx = 0;
        cur_page_ptr = page_start;

        do {
          texel_color = ((byte*)src_tex_ptr)[y_offset + x_idx + row_offset];
          color_accum = ((((unsigned int)((((unsigned int)(color_accum)) >> 8))) << 8) | ((unsigned char)(texel_color)));

          if (x_idx < 0x3f) {
            temp_byte_ptr = (byte *)(y_offset + x_idx + src_tex_ptr + row_offset);
            next_texel = temp_byte_ptr[1];
          }

          else {
            temp_byte_ptr = (byte *)(y_offset + x_idx + src_tex_ptr + row_offset);
            next_texel = *temp_byte_ptr;
          }

          b11_scaled = ((((unsigned int)((((unsigned int)(b11_scaled)) >> 8))) << 8) | ((unsigned char)(next_texel)));

          if (y_idx < h_limit) {
            row_texel = temp_byte_ptr[0x100];

            if (x_idx < 0x3f) {
              diag_texel = (unsigned int *)(unsigned int)temp_byte_ptr[0x101];
            }

            else {
LAB_00447785:

              diag_texel = (unsigned int *)(unsigned int)row_texel;
            }

          }

          else {
            row_texel = *temp_byte_ptr;
            if (0x3e < x_idx) goto LAB_00447785;
            diag_texel = (unsigned int *)(unsigned int)temp_byte_ptr[1];
          }

          texel_r = (unsigned int)row_texel;
          texel_g = (unsigned int)next_texel;
          texel_b = (unsigned int)texel_color;
          pixel_val = (((int)diag_texel * 0x100 + texel_r) * 0x100 + texel_g) * 0x100 + texel_b;
          table_entry = alloc_table + ((pixel_val >> 0x11) + texel_r + texel_g + pixel_val & 0xffff) * 5;
          table_ptr = diag_texel;

          if (*table_entry != pixel_val) {
            *table_entry = pixel_val;
            color_accum = (unsigned int)((byte*)palette_ptr)[texel_b * 4];
            pixel_val = color_accum;
            r00 = color_accum;
            color_accum = (unsigned int)((byte*)palette_ptr)[1 + texel_b * 4];
            g00_temp = color_accum;
            g00 = color_accum;
            color_accum = (unsigned int)((byte*)palette_ptr)[2 + texel_b * 4];
            texel_b = color_accum;
            b00 = color_accum;
            r10 = (unsigned int)*(byte *)(palette_ptr + texel_g * 4);
            g10 = (unsigned int)*(byte *)(palette_ptr + 1 + texel_g * 4);
            b10 = (unsigned int)*(byte *)(palette_ptr + 2 + texel_g * 4);
            r01 = (unsigned int)*(byte *)(palette_ptr + texel_r * 4);
            g01 = (unsigned int)*(byte *)(palette_ptr + 1 + texel_r * 4);
            b01 = (unsigned int)*(byte *)(palette_ptr + 2 + texel_r * 4);
            r11 = (unsigned int)*(byte *)(palette_ptr + (int)diag_texel * 4);
            g11 = (unsigned int)*(byte *)(palette_ptr + 1 + (int)diag_texel * 4);
            table_ptr = cur_page_ptr;
            b11_scaled = 0;
            g11_scaled = 0;
            b11 = (unsigned int)*(byte *)(palette_ptr + 2 + (int)diag_texel * 4);
            r11_scaled = 0;
            b10_scaled = b10 << 2;
            b01_scaled = 0;
            g10_scaled = g10 << 2;
            r10_scaled = r10 << 2;
            g01_scaled = 0;
            r01_scaled = 0;
            b00_scaled = color_accum << 2;
            color_accum = 4;
            g00_scaled = g00_temp * 4;
            r00_scaled = pixel_val << 2;
            diag_texel = table_entry;

            do {
              i_counter = (int)(r01_scaled + r00_scaled + (r01_scaled + r00_scaled >> 0x1f & 3U)) >> 2;
              g_interp_0 = (int)(g01_scaled + g00_scaled + (g01_scaled + g00_scaled >> 0x1f & 3U)) >> 2;
              b_interp_0 = (int)(b01_scaled + b00_scaled + (b01_scaled + b00_scaled >> 0x1f & 3U)) >> 2;
              r_interp_1 = (int)(r11_scaled + r10_scaled + (r11_scaled + r10_scaled >> 0x1f & 3U)) >> 2;
              g_interp_1 = (int)(g11_scaled + g10_scaled + (g11_scaled + g10_scaled >> 0x1f & 3U)) >> 2;
              b_interp_1 = (int)(b11_scaled + b10_scaled + (b11_scaled + b10_scaled >> 0x1f & 3U)) >> 2;
              rgb_error_accum[0] = i_counter * 4;
              err_idx = 0;
              rgb_error_accum[1] = g_interp_0 << 2;
              rgb_error_accum[2] = b_interp_0 << 2;

              do {
                temp_err = vec_a[err_idx / 4] + 8 + rgb_error_accum[err_idx / 4];
                rgb_error_accum[err_idx / 4] = temp_err;

                if (0x3ff < temp_err) {
                  rgb_error_accum[err_idx / 4] = 0x3ff;
                }

                if (rgb_error_accum[err_idx / 4] < 0) {
                  rgb_error_accum[err_idx / 4] = 0;
                }

                err_idx = err_idx + 4;
              } while (err_idx < 0xc);

              texel_color = (&g_LisaObjectMatrix_22)

                      [(rgb_error_accum[1] & 0x3f0) * 4 +

                       ((rgb_error_accum[2] & 0x3f0) >> 4) + (rgb_error_accum[0] & 0x3f0) * 0x100];
              texel_g = (unsigned int)texel_color;
              *(byte *)(diag_texel + 1) = texel_color;
              next_texel = *(byte *)(palette_ptr + texel_g * 4);
              *(byte *)table_ptr = texel_color;
              vec_a[0] = (int)(rgb_error_accum[0] + (unsigned int)next_texel * -4) / 2;
              vec_a[1] = (int)(rgb_error_accum[1] + (unsigned int)*(byte *)(palette_ptr + 1 + texel_g * 4) * -4) / 2;
              vec_a[2] = (int)(rgb_error_accum[2] + (unsigned int)*(byte *)(palette_ptr + 2 + texel_g * 4) * -4) / 2;
              rgb_error_accum[0] = i_counter * 3 + r_interp_1;
              rgb_error_accum[1] = g_interp_0 * 3 + g_interp_1;
              err_idx = 0;
              rgb_error_accum[2] = b_interp_0 * 3 + b_interp_1;

              do {
                temp_err = vec_a[err_idx / 4] + 8 + rgb_error_accum[err_idx / 4];
                rgb_error_accum[err_idx / 4] = temp_err;

                if (0x3ff < temp_err) {
                  rgb_error_accum[err_idx / 4] = 0x3ff;
                }

                if (rgb_error_accum[err_idx / 4] < 0) {
                  rgb_error_accum[err_idx / 4] = 0;
                }

                err_idx = err_idx + 4;
              } while (err_idx < 0xc);

              texel_g = (unsigned int)(byte)(&g_LisaObjectMatrix_22)

                                   [(rgb_error_accum[1] & 0x3f0) * 4 +

                                    ((rgb_error_accum[2] & 0x3f0) >> 4) + (rgb_error_accum[0] & 0x3f0) * 0x100];

              *(char *)((int)diag_texel + 5) =

                   (&g_LisaObjectMatrix_22)

                   [(rgb_error_accum[1] & 0x3f0) * 4 +

                    ((rgb_error_accum[2] & 0x3f0) >> 4) + (rgb_error_accum[0] & 0x3f0) * 0x100];
              vec_a[0] = (int)(rgb_error_accum[0] + (unsigned int)*(byte *)(palette_ptr + texel_g * 4) * -4) / 2;
              vec_a[1] = (int)(rgb_error_accum[1] + (unsigned int)*(byte *)(palette_ptr + 1 + texel_g * 4) * -4) / 2;
              vec_a[2] = (int)(rgb_error_accum[2] + (unsigned int)*(byte *)(palette_ptr + 2 + texel_g * 4) * -4) / 2;
              rgb_error_accum[0] = (r_interp_1 + i_counter) * 2;
              rgb_error_accum[1] = (g_interp_1 + g_interp_0) * 2;
              err_idx = 0;
              rgb_error_accum[2] = (b_interp_1 + b_interp_0) * 2;

              do {
                temp_err = vec_a[err_idx / 4] + 8 + rgb_error_accum[err_idx / 4];
                rgb_error_accum[err_idx / 4] = temp_err;

                if (0x3ff < temp_err) {
                  rgb_error_accum[err_idx / 4] = 0x3ff;
                }

                if (rgb_error_accum[err_idx / 4] < 0) {
                  rgb_error_accum[err_idx / 4] = 0;
                }

                err_idx = err_idx + 4;
              } while (err_idx < 0xc);

              texel_g = (unsigned int)(byte)(&g_LisaObjectMatrix_22)

                                   [(rgb_error_accum[1] & 0x3f0) * 4 +

                                    ((rgb_error_accum[2] & 0x3f0) >> 4) + (rgb_error_accum[0] & 0x3f0) * 0x100];

              *(char *)((int)diag_texel + 6) =

                   (&g_LisaObjectMatrix_22)

                   [(rgb_error_accum[1] & 0x3f0) * 4 +

                    ((rgb_error_accum[2] & 0x3f0) >> 4) + (rgb_error_accum[0] & 0x3f0) * 0x100];
              vec_a[0] = (int)(rgb_error_accum[0] + (unsigned int)*(byte *)(palette_ptr + texel_g * 4) * -4) / 2;
              vec_a[1] = (int)(rgb_error_accum[1] + (unsigned int)*(byte *)(palette_ptr + 1 + texel_g * 4) * -4) / 2;
              vec_a[2] = (int)(rgb_error_accum[2] + (unsigned int)*(byte *)(palette_ptr + 2 + texel_g * 4) * -4) / 2;
              rgb_error_accum[0] = r_interp_1 * 3 + i_counter;
              i_counter = 0;
              rgb_error_accum[1] = g_interp_1 * 3 + g_interp_0;
              rgb_error_accum[2] = b_interp_1 * 3 + b_interp_0;

              do {
                err_idx = *(int *)((int)vec_a + i_counter) + 8 + *(int *)((int)rgb_error_accum + i_counter);
                *(int *)((int)rgb_error_accum + i_counter) = err_idx;

                if (0x3ff < err_idx) {
                  *(int *)((int)rgb_error_accum + i_counter) = 0x3ff;
                }

                if (*(int *)((int)rgb_error_accum + i_counter) < 0) {
                  *(int *)((int)rgb_error_accum + i_counter) = 0;
                }

                i_counter = i_counter + 4;
              } while (i_counter < 0xc);

              texel_g = (unsigned int)(byte)(&g_LisaObjectMatrix_22)

                                   [(rgb_error_accum[1] & 0x3f0) * 4 +

                                    ((rgb_error_accum[2] & 0x3f0) >> 4) + (rgb_error_accum[0] & 0x3f0) * 0x100];

              *(char *)((int)diag_texel + 7) =

                   (&g_LisaObjectMatrix_22)

                   [(rgb_error_accum[1] & 0x3f0) * 4 +

                    ((rgb_error_accum[2] & 0x3f0) >> 4) + (rgb_error_accum[0] & 0x3f0) * 0x100];
              vec_a[0] = (int)(rgb_error_accum[0] + (unsigned int)*(byte *)(palette_ptr + texel_g * 4) * -4) / 2;
              vec_a[1] = (int)(rgb_error_accum[1] + (unsigned int)*(byte *)(palette_ptr + 1 + texel_g * 4) * -4) / 2;
              table_ptr = table_ptr + 0x40;
              vec_a[2] = (int)(rgb_error_accum[2] + (unsigned int)*(byte *)(palette_ptr + 2 + texel_g * 4) * -4) / 2;
              b11_scaled = b11_scaled + b11;
              b10_scaled = b10_scaled - b10;
              g11_scaled = g11_scaled + g11;
              g10_scaled = g10_scaled - g10;
              r11_scaled = r11_scaled + r11;
              r10_scaled = r10_scaled - r10;
              b01_scaled = b01_scaled + b01;
              b00_scaled = b00_scaled - texel_b;
              g01_scaled = g01_scaled + g01;
              g00_scaled = g00_scaled - g00_temp;
              r01_scaled = r01_scaled + r01;
              r00_scaled = r00_scaled - pixel_val;
              color_accum = color_accum + -1;
              diag_texel = diag_texel + 1;
            } while (color_accum != 0);
            vec_a[0] = 0;
            vec_a[1] = 0;
            vec_a[2] = 0;
            color_accum = 0;
          }

          vec_a[2] = 0;
          vec_a[1] = 0;
          vec_a[0] = 0;
          pixel_val = table_entry[2];
          diag_texel = cur_page_ptr + 1;
          x_idx = x_idx + 1;
          *cur_page_ptr = table_entry[1];
          texel_g = table_entry[3];
          cur_page_ptr[0x40] = pixel_val;
          pixel_val = table_entry[4];
          cur_page_ptr[0x80] = texel_g;
          cur_page_ptr[0xc0] = pixel_val;
          cur_page_ptr = diag_texel;
        } while (x_idx < 0x40);
        row_offset = row_offset + 0x100;
        page_start = page_start + 0x100;
        y_idx = y_idx + 1;
      } while (y_idx < dst_h);
    }

    y_offset = y_offset + 0x40;
    out_val_span = span_ptr;

    if (0xff < y_offset) {
      free(alloc_table);
      return;
    }

  } while( 1 );
}

/**
 * @original Lisa_DownsampleTextureMipmap (MAINDOS_32BIT.EXE @ 0x00447fb0, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_DownsampleTextureMipmap(byte *src_ptr,byte *dst_ptr,int width,int height,int stride,int palette) {
  int pal_offset;
  byte src_pixel;
  int width_minus_1;
  unsigned int *accum_ptr;
  unsigned int chan_sum;
  unsigned int chan_val;
  int chan_offset;
  int next_chan_offset;
  int temp_val;
  int x;
  int y;
  unsigned int color_accum [4];
  int error_acc [3];

  

  y = 0;

  if (0 < height) {
    do {
      x = 0;
      error_acc[0] = 0;
      error_acc[1] = 0;
      error_acc[2] = 0;

      if (0 < width) {
        width_minus_1 = width + -1;

        do {
          chan_offset = 0;
          accum_ptr = color_accum;
          src_pixel = *src_ptr;

          do {
            chan_val = (unsigned int)*(byte *)((unsigned int)src_pixel * 4 + chan_offset + palette);
            *accum_ptr = chan_val;

            if (x < width_minus_1) {
              chan_sum = *(byte *)(chan_offset + (unsigned int)src_ptr[1] * 4 + palette) + chan_val;
            }

            else {
              chan_sum = chan_val * 2;
            }

            *accum_ptr = chan_sum;

            if (y < height + -1) {
              chan_val = (unsigned int)*(byte *)(chan_offset + (unsigned int)src_ptr[stride] * 4 + palette);
              chan_sum = *accum_ptr + chan_val;
              *accum_ptr = chan_sum;

              if (x < width_minus_1) {
                *accum_ptr = *(byte *)(chan_offset + (unsigned int)src_ptr[stride + 1] * 4 + palette) + chan_sum;
              }

              else {
                *accum_ptr = chan_val + chan_sum;
              }

            }

            else {
              chan_sum = *accum_ptr + chan_val;
              *accum_ptr = chan_sum;

              if (x < width_minus_1) {
                *accum_ptr = *(byte *)(chan_offset + (unsigned int)src_ptr[1] * 4 + palette) + chan_sum;
              }

              else {
                *accum_ptr = chan_sum + chan_val;
              }

            }

            accum_ptr = accum_ptr + 1;
            chan_offset = chan_offset + 1;
          } while (accum_ptr < color_accum + 3);
          chan_offset = 0;

          do {
            temp_val = *(int *)((int)error_acc + chan_offset) + 8 + *(int *)((int)color_accum + chan_offset);
            *(int *)((int)color_accum + chan_offset) = temp_val;

            if (0x3ff < temp_val) {
              *(int *)((int)color_accum + chan_offset) = 0x3ff;
            }

            if (*(int *)((int)color_accum + chan_offset) < 0) {
              *(int *)((int)color_accum + chan_offset) = 0;
            }

            chan_offset = chan_offset + 4;
          } while (chan_offset < 0xc);
          temp_val = 0;

          src_pixel = (&g_LisaObjectMatrix_22)

                  [(color_accum[1] & 0x3f0) * 4 +

                   ((color_accum[2] & 0x3f0) >> 4) + (color_accum[0] & 0x3f0) * 0x100];
          *dst_ptr = src_pixel;
          chan_offset = 0;

          do {
            next_chan_offset = chan_offset + 4;
            pal_offset = temp_val + (unsigned int)src_pixel * 4;
            temp_val = temp_val + 1;

            *(int *)((int)error_acc + chan_offset) =

                 (int)(*(int *)((int)color_accum + chan_offset) + (unsigned int)*(byte *)(pal_offset + palette) * -4) / 2;
            chan_offset = next_chan_offset;
          } while (next_chan_offset < 0xc);
          dst_ptr = dst_ptr + 1;
          src_ptr = src_ptr + 1;
          x = x + 1;
        } while (x < width);
      }

      src_ptr = src_ptr + (stride - width);
      y = y + 1;
      dst_ptr = dst_ptr + (stride - width);
    } while (y < height);
  }

  return;
}

/**
 * @original Lisa_FilterTextureBlock (MAINDOS_32BIT.EXE @ 0x004481f0, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_FilterTextureBlock(byte *src,int dst,int width,int height,int stride,int color_map, int flags,int radius) {
  byte pixel1;
  unsigned int pixel2;
  int chan_offset;
  unsigned int *accum_ptr;
  int tmp_val;
  int x;
  int y;
  int chan_idx;
  byte *src_bot_right_edge;
  int y_offset;
  byte *src_bot_center;
  int bot_left_offset;
  byte *src_top_right_edge;
  int neg_y_offset;
  byte *src_top_center;
  int top_left_offset;
  unsigned int rgb_accum [6];
  int rgb_error [3];

  

  y = 0;

  if (0 < height) {
    y_offset = radius * stride;
    src_bot_center = src + y_offset;
    src_bot_right_edge = src + y_offset + width + -1;
    bot_left_offset = (stride + -1) * radius;
    src_top_right_edge = src + (width - y_offset) + -1;
    neg_y_offset = -y_offset;
    src_top_center = src + -y_offset;
    top_left_offset = (-1 - stride) * radius;

    do {
      x = 0;
      rgb_error[0] = 0;
      rgb_error[1] = 0;
      rgb_error[2] = 0;

      if (0 < width) {
        do {
          chan_offset = 0;
          accum_ptr = rgb_accum;

          do {
            if (y - radius < 1) {
              if (x - radius < 1) {
                pixel1 = *src;
              }

              else {
                pixel1 = src[x - radius];
              }

              *accum_ptr = (unsigned int)*(byte *)(chan_offset + (unsigned int)pixel1 * 4 + color_map);

              if (x + radius < width) {
                pixel2 = (unsigned int)src[x + radius];
              }

              else {
                pixel2 = (unsigned int)src[width + -1];
              }

            }

            else {
              if (x == radius || x - radius < 0) {
                *accum_ptr = (unsigned int)*(byte *)(chan_offset + (unsigned int)*src_top_center * 4 + color_map);
              }

              else {
                *accum_ptr = (unsigned int)*(byte *)(chan_offset + (unsigned int)src[top_left_offset + x] * 4 + color_map);
              }

              if (radius + x < width) {
                pixel2 = (unsigned int)src[neg_y_offset + radius + x];
              }

              else {
                pixel2 = (unsigned int)*src_top_right_edge;
              }

            }

            *accum_ptr = *accum_ptr + (unsigned int)*(byte *)(chan_offset + pixel2 * 4 + color_map);

            if (y + radius < height) {
              if (x == radius || x - radius < 0) {
                pixel1 = *src_bot_center;
              }

              else {
                pixel1 = src[bot_left_offset + x];
              }

              *accum_ptr = *accum_ptr + (unsigned int)*(byte *)(chan_offset + (unsigned int)pixel1 * 4 + color_map);

              if (radius + x < width) {
                pixel2 = (unsigned int)src[y_offset + radius + x];
                goto LAB_004484c0;
              }

              *accum_ptr = *accum_ptr + (unsigned int)*(byte *)(chan_offset + (unsigned int)*src_bot_right_edge * 4 + color_map);
            }

            else {
              if (x == radius || x - radius < 0) {
                tmp_val = (height + -1) * stride;
                pixel1 = *(byte *)(chan_offset + (unsigned int)src[tmp_val] * 4 + color_map);
              }

              else {
                tmp_val = (height + -1) * stride;

                pixel1 = *(byte *)(chan_offset + (unsigned int)src[tmp_val + (x - radius)] * 4 + color_map)

                ;
              }

              *accum_ptr = *accum_ptr + (unsigned int)pixel1;

              if (radius + x < width) {
                pixel2 = (unsigned int)src[radius + x];
              }

              else {
                pixel2 = (unsigned int)src[tmp_val + width + -1];
              }

LAB_004484c0:

              *accum_ptr = *accum_ptr + (unsigned int)*(byte *)(chan_offset + pixel2 * 4 + color_map);
            }

            accum_ptr = accum_ptr + 1;
            chan_offset = chan_offset + 1;
          } while (accum_ptr < rgb_accum + 3);
          chan_offset = 0;

          do {
            tmp_val = *(int *)((int)rgb_error + chan_offset) + 8 + *(int *)((int)rgb_accum + chan_offset);
            *(int *)((int)rgb_accum + chan_offset) = tmp_val;

            if (0x3ff < tmp_val) {
              *(int *)((int)rgb_accum + chan_offset) = 0x3ff;
            }

            if (*(int *)((int)rgb_accum + chan_offset) < 0) {
              *(int *)((int)rgb_accum + chan_offset) = 0;
            }

            chan_offset = chan_offset + 4;
          } while (chan_offset < 0xc);
          chan_idx = 0;

          pixel1 = (&g_LisaObjectMatrix_22)

                  [(rgb_accum[0] & 0x3f0) * 0x100 +

                   ((rgb_accum[2] & 0x3f0) >> 4) + (rgb_accum[1] & 0x3f0) * 4];
          *(byte *)(dst + x) = pixel1;
          chan_offset = 0;

          do {
            tmp_val = chan_offset + 4;

            *(int *)((int)rgb_error + chan_offset) =

                 (int)(*(int *)((int)rgb_accum + chan_offset) +

                      (unsigned int)*(byte *)(chan_idx + color_map + (unsigned int)pixel1 * 4) * -4) / 2;
            chan_idx = chan_idx + 1;
            chan_offset = tmp_val;
          } while (tmp_val < 0xc);
          x = x + 1;
        } while (x < width);
      }

      dst = dst + stride;
      src_bot_right_edge = src_bot_right_edge + stride;
      y_offset = y_offset + stride;
      src_bot_center = src_bot_center + stride;
      bot_left_offset = bot_left_offset + stride;
      src_top_right_edge = src_top_right_edge + stride;
      neg_y_offset = neg_y_offset + stride;
      src_top_center = src_top_center + stride;
      top_left_offset = top_left_offset + stride;
      y = y + 1;
    } while (y < height);
  }

  return;
}

/**
 * @original Lisa_LoadOrCreateShadingTable (MAINDOS_32BIT.EXE @ 0x00448620, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_LoadOrCreateShadingTable(int shade_level,int palette_ptr) {
  byte *pal_byte_ptr;
  char char_tmp;
  unsigned int carry_flag;
  int min_dist_or_len;
  FILE *table_file;
  unsigned int checksum_shifted;
  int pal_rgb_offset;
  unsigned int checksum_accum;
  int pal_color_idx;
  size_t bytes_read;
  char *str_src;
  int dist_sq;
  unsigned int *chk_ptr1;
  unsigned int *chk_ptr2;
  char *str_dst1;
  char *str_dst2;
  int color_diff;
  unsigned int rgb18_idx;
  int best_color;
  unsigned int checksum_str_buf [5];
  char filename [32];

  

  checksum_accum = 0;
  min_dist_or_len = 0;

  do {
    pal_byte_ptr = (byte *)(palette_ptr + min_dist_or_len);
    min_dist_or_len = min_dist_or_len + 1;
    pal_color_idx = checksum_accum + *pal_byte_ptr;
    checksum_shifted = pal_color_idx * 2;
    carry_flag = (unsigned int)(pal_color_idx < 0);
    checksum_accum = checksum_shifted | carry_flag;
  } while (min_dist_or_len < 0x300);
  bytes_read = 0;
  checksum_str_buf[0] = 0xffffffff;
  filename[0] = '\0';
  str_src = &s_pal_chk_str2;

  do {
    str_dst2 = str_src;
    if (checksum_str_buf[0] == 0) break;
    checksum_str_buf[0] = checksum_str_buf[0] - 1;
    str_dst2 = str_src + 1;
    char_tmp = *str_src;
    str_src = str_dst2;
  } while (char_tmp != '\0');
  checksum_str_buf[0] = ~checksum_str_buf[0];
  min_dist_or_len = -1;
  str_src = filename;

  do {
    str_dst1 = str_src;
    if (min_dist_or_len == 0) break;
    min_dist_or_len = min_dist_or_len + -1;
    str_dst1 = str_src + 1;
    char_tmp = *str_src;
    str_src = str_dst1;
  } while (char_tmp != '\0');
  str_src = str_dst2 + -checksum_str_buf[0];
  str_dst2 = str_dst1 + -1;

  for (checksum_accum = checksum_str_buf[0] >> 2; checksum_accum != 0; checksum_accum = checksum_accum - 1) {
    *(int *)str_dst2 = *(int *)str_src;
    str_src = str_src + 4;
    str_dst2 = str_dst2 + 4;
  }

  for (checksum_accum = checksum_str_buf[0] & 3; checksum_accum != 0; checksum_accum = checksum_accum - 1) {
    *str_dst2 = *str_src;
    str_src = str_src + 1;
    str_dst2 = str_dst2 + 1;
  }

  __ultoa(checksum_shifted & 0xffff | carry_flag,(char *)checksum_str_buf,0x10);
  checksum_accum = 0xffffffff;
  chk_ptr1 = checksum_str_buf;

  do {
    chk_ptr2 = chk_ptr1;
    if (checksum_accum == 0) break;
    checksum_accum = checksum_accum - 1;
    chk_ptr2 = (unsigned int *)((int)chk_ptr1 + 1);
    checksum_shifted = *chk_ptr1;
    chk_ptr1 = chk_ptr2;
  } while ((char)checksum_shifted != '\0');
  checksum_accum = ~checksum_accum;
  min_dist_or_len = -1;
  str_src = filename;

  do {
    str_dst2 = str_src;
    if (min_dist_or_len == 0) break;
    min_dist_or_len = min_dist_or_len + -1;
    str_dst2 = str_src + 1;
    char_tmp = *str_src;
    str_src = str_dst2;
  } while (char_tmp != '\0');
  str_src = (char *)((int)chk_ptr2 - checksum_accum);
  str_dst2 = str_dst2 + -1;

  for (checksum_shifted = checksum_accum >> 2; checksum_shifted != 0; checksum_shifted = checksum_shifted - 1) {
    *(int *)str_dst2 = *(int *)str_src;
    str_src = str_src + 4;
    str_dst2 = str_dst2 + 4;
  }

  for (checksum_accum = checksum_accum & 3; checksum_accum != 0; checksum_accum = checksum_accum - 1) {
    *str_dst2 = *str_src;
    str_src = str_src + 1;
    str_dst2 = str_dst2 + 1;
  }

  checksum_accum = 0xffffffff;
  str_src = (char *)&s_pal_chk_str1;

  do {
    str_dst2 = str_src;
    if (checksum_accum == 0) break;
    checksum_accum = checksum_accum - 1;
    str_dst2 = str_src + 1;
    char_tmp = *str_src;
    str_src = str_dst2;
  } while (char_tmp != '\0');
  checksum_accum = ~checksum_accum;
  min_dist_or_len = -1;
  str_src = filename;

  do {
    str_dst1 = str_src;
    if (min_dist_or_len == 0) break;
    min_dist_or_len = min_dist_or_len + -1;
    str_dst1 = str_src + 1;
    char_tmp = *str_src;
    str_src = str_dst1;
  } while (char_tmp != '\0');
  str_src = str_dst2 + -checksum_accum;
  str_dst2 = str_dst1 + -1;

  for (checksum_shifted = checksum_accum >> 2; checksum_shifted != 0; checksum_shifted = checksum_shifted - 1) {
    *(int *)str_dst2 = *(int *)str_src;
    str_src = str_src + 4;
    str_dst2 = str_dst2 + 4;
  }

  for (checksum_accum = checksum_accum & 3; checksum_accum != 0; checksum_accum = checksum_accum - 1) {
    *str_dst2 = *str_src;
    str_src = str_src + 1;
    str_dst2 = str_dst2 + 1;
  }

  checksum_accum = 0xffffffff;
  str_src = (char *)&s_pal_checksum_fmt;

  do {
    str_dst2 = str_src;
    if (checksum_accum == 0) break;
    checksum_accum = checksum_accum - 1;
    str_dst2 = str_src + 1;
    char_tmp = *str_src;
    str_src = str_dst2;
  } while (char_tmp != '\0');
  checksum_accum = ~checksum_accum;
  min_dist_or_len = -1;
  str_src = filename;

  do {
    str_dst1 = str_src;
    if (min_dist_or_len == 0) break;
    min_dist_or_len = min_dist_or_len + -1;
    str_dst1 = str_src + 1;
    char_tmp = *str_src;
    str_src = str_dst1;
  } while (char_tmp != '\0');
  str_src = str_dst2 + -checksum_accum;
  str_dst2 = str_dst1 + -1;

  for (checksum_shifted = checksum_accum >> 2; checksum_shifted != 0; checksum_shifted = checksum_shifted - 1) {
    *(int *)str_dst2 = *(int *)str_src;
    str_src = str_src + 4;
    str_dst2 = str_dst2 + 4;
  }

  for (checksum_accum = checksum_accum & 3; checksum_accum != 0; checksum_accum = checksum_accum - 1) {
    *str_dst2 = *str_src;
    str_src = str_src + 1;
    str_dst2 = str_dst2 + 1;
  }

  table_file = (FILE *)fopen(filename,(const char *)&s_rb);

  if (table_file != (FILE *)0x0) {
    bytes_read = fread(&g_LisaObjectMatrix_22,1,0x40000,table_file);
    fclose(table_file);
  }

  if (bytes_read != 0x40000) {
    rgb18_idx = 0;

    do {
      pal_rgb_offset = 0;
      min_dist_or_len = 0x7f000000;
      pal_color_idx = 0;

      do {
        dist_sq = ((rgb18_idx & 0x3f000) >> 10) - (unsigned int)*(byte *)(pal_rgb_offset + palette_ptr);
        dist_sq = dist_sq * dist_sq;

        if (((dist_sq < min_dist_or_len) &&

            (color_diff = ((rgb18_idx & 0xfc0) >> 4) - (unsigned int)*(byte *)(pal_rgb_offset + 1 + palette_ptr),

            dist_sq = dist_sq + color_diff * color_diff, dist_sq < min_dist_or_len)) &&

           (color_diff = (rgb18_idx & 0x3f) * 4 - (unsigned int)*(byte *)(pal_rgb_offset + 2 + palette_ptr),

           dist_sq = dist_sq + color_diff * color_diff, dist_sq < min_dist_or_len)) {
          min_dist_or_len = dist_sq;
          best_color = pal_color_idx;
        }

        pal_rgb_offset = pal_rgb_offset + 3;
        pal_color_idx = pal_color_idx + 1;
      } while (pal_color_idx < 0x100);
      checksum_accum = rgb18_idx + 1;
      ((int*)&(g_LisaObjectMatrix_22))[rgb18_idx] = (char)best_color;
      rgb18_idx = checksum_accum;
    } while ((int)checksum_accum < 0x40000);
    table_file = (FILE *)fopen(filename,(const char *)&s_tab_tab);

    if (table_file != (FILE *)0x0) {
      fwrite(&g_LisaObjectMatrix_22,1,0x40000,table_file);
      fclose(table_file);
    }

  }

  return;
}

/**
 * @original Lisa_FindClosestPaletteColor (MAINDOS_32BIT.EXE @ 0x00448860, lisa3d.c)
 * @fidelity ADAPTED
 */
unsigned int Lisa_FindClosestPaletteColor(int *target_rgb,int palette_ptr) {
  unsigned int best_index;
  int color_offset;
  int dist_sq;
  unsigned int pal_color;
  int min_dist_sq;
  unsigned int color_index;

  

  min_dist_sq = 0x7f000000;
  color_offset = 0;
  color_index = 0;
  best_index = pal_color;

  do {
    dist_sq = *target_rgb - (unsigned int)*(byte *)(palette_ptr + color_offset);
    dist_sq = dist_sq * dist_sq;

    if (dist_sq < min_dist_sq) {
      pal_color = (unsigned int)*(byte *)(palette_ptr + 1 + color_offset);
      dist_sq = dist_sq + (target_rgb[1] - pal_color) * (target_rgb[1] - pal_color);

      if (dist_sq < min_dist_sq) {
        pal_color = (unsigned int)*(byte *)(palette_ptr + 2 + color_offset);
        dist_sq = dist_sq + (target_rgb[2] - pal_color) * (target_rgb[2] - pal_color);

        if (dist_sq < min_dist_sq) {
          best_index = color_index;
          min_dist_sq = dist_sq;
        }

      }

    }

    color_offset = color_offset + 3;
    color_index = color_index + 1;
  } while ((int)color_index < 0x100);
  return best_index;
}

/**
 * @original Lisa_RenderSkyBackdrop (MAINDOS_32BIT.EXE @ 0x00448990, lisa3d.c)
 * @fidelity ADAPTED
 */
LisaReturn64 Lisa_RenderSkyBackdrop(void) {
  float horiz_dist;
  int camera_ptr;
  int dir_x;
  int dir_z;
  int dir_y;
  unsigned int pitch_val;
  double vec_x;
  double vec_y;
  int pitch_angle;
  unsigned int yaw_angle;

  

  camera_ptr = g_LisaCamera;
  pitch_angle = __ftol();
  dir_x = (int)pitch_angle;
  pitch_angle = __ftol();
  dir_z = (int)pitch_angle;
  pitch_angle = __ftol();
  pitch_val = (unsigned int)((unsigned int)pitch_angle >> 0);
  dir_y = (int)pitch_angle;
  horiz_dist = (float)SQRT(vec_y * vec_y + vec_x * vec_x);

  if ((dir_z < 1) || ((int)horiz_dist < 1)) {
    if ((dir_z < 0) && (0 < (int)horiz_dist)) {
      fpatan((double)horiz_dist / (double)-dir_z,(double)1);
      pitch_angle = __ftol();
      *(double *)(camera_ptr + 0x18) = (double)(int)pitch_angle;
    }

    else if (dir_z == 0) {
      *(int *)(camera_ptr + 0x18) = 0;
      *(int *)(camera_ptr + 0x1c) = 0;
      pitch_angle = (unsigned int)pitch_val << 0;
    }

    else if ((dir_z == 0) && (ABS(horiz_dist) == 0.0)) {
      *(int *)(camera_ptr + 0x18) = 0;
      *(int *)(camera_ptr + 0x1c) = 0;
      pitch_angle = (unsigned int)pitch_val << 0;
    }

    else if ((dir_z < 0) && (ABS(horiz_dist) == 0.0)) {
      *(int *)(camera_ptr + 0x18) = 0;
      *(int *)(camera_ptr + 0x1c) = 0x40a51800;
      pitch_angle = (((int)(pitch_val) << 0) | ((unsigned int)(horiz_dist)));
    }

    else {
      pitch_angle = (unsigned int)pitch_val << 0;

      if (0 < dir_z) {
        pitch_angle = (((int)(pitch_val) << 0) | ((unsigned int)(horiz_dist)));

        if (ABS(horiz_dist) == 0.0) {
          *(int *)(camera_ptr + 0x18) = 0;
          *(int *)(camera_ptr + 0x1c) = 0x408c2000;
          pitch_angle = (((int)(pitch_val) << 0) | ((unsigned int)(horiz_dist)));
        }

      }

    }

  }

  else {
    fpatan((double)dir_z / (double)horiz_dist,(double)1);
    pitch_angle = __ftol();
    *(double *)(camera_ptr + 0x18) = (double)(int)pitch_angle;
  }

  pitch_val = (unsigned int)((unsigned int)pitch_angle >> 0);

  if ((dir_x < 1) || (dir_y < 1)) {
    if ((dir_y < 0) && (0 < dir_x)) {
      fpatan((double)-dir_y / (double)dir_x,(double)1);
      yaw_angle = __ftol();
      *(double *)(camera_ptr + 0x20) = (double)(int)yaw_angle;
    }

    else if ((dir_x < 0) && (dir_y < 0)) {
      fpatan((double)dir_x / (double)dir_y,(double)1);
      yaw_angle = __ftol();
      *(double *)(camera_ptr + 0x20) = (double)(int)yaw_angle;
    }

    else if ((dir_y < 1) || (-1 < dir_x)) {
      if (dir_x == 0) {
        if (0 < dir_y) {
          *(int *)(camera_ptr + 0x20) = 0;
          *(int *)(camera_ptr + 0x24) = 0;
          yaw_angle = (unsigned int)pitch_val << 0;
          goto LAB_00448c11;
        }

        if (dir_y < 0) {
          *(int *)(camera_ptr + 0x20) = 0;
          *(int *)(camera_ptr + 0x24) = 0x409c2000;
          yaw_angle = (unsigned int)pitch_val << 0;
          goto LAB_00448c11;
        }

      }

      if ((dir_y == 0) && (0 < dir_x)) {
        *(int *)(camera_ptr + 0x20) = 0;
        *(int *)(camera_ptr + 0x24) = 0x408c2000;
        yaw_angle = (unsigned int)pitch_val << 0;
      }

      else {
        yaw_angle = (unsigned int)pitch_val << 0;

        if ((dir_y == 0) && (yaw_angle = (unsigned int)pitch_val << 0, dir_x < 0)) {
          *(int *)(camera_ptr + 0x20) = 0;
          *(int *)(camera_ptr + 0x24) = 0x40a51800;
          yaw_angle = (unsigned int)pitch_val << 0;
        }

      }

    }

    else {
      fpatan((double)dir_y / (double)-dir_x,(double)1);
      yaw_angle = __ftol();
      *(double *)(camera_ptr + 0x20) = (double)(int)yaw_angle;
    }

  }

  else {
    fpatan((double)dir_x / (double)dir_y,(double)1);
    yaw_angle = __ftol();
    *(double *)(camera_ptr + 0x20) = (double)(int)yaw_angle;
  }

LAB_00448c11:

  *(double *)(camera_ptr + 0x28) = (double)*(int *)(camera_ptr + 0x7c);
  { LisaReturn64 _r; _r.eax = yaw_angle; _r.edx = 0; return _r; }
}

/**
 * @original Lisa_CullObjectsOrthographic (MAINDOS_32BIT.EXE @ 0x00448c30, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_CullObjectsOrthographic(void) {
    LisaDynamicObject *obj_ptr;
    LisaDynamicObject **visible_array;
    int obj_count;
    int grid_offset_z;
    int loop_counter;
    int array_index;
    int cell_index;
    int grid_index;
    int cam_z_int;
    int cam_x_int;
    int cam_y_int;
    int cell_idx;
    int count;

    g_LisaCamera->visible_obj_count = 0;
    cam_z_int = __ftol();
    cam_x_int = __ftol();
    cam_y_int = __ftol();
    
    // Abstract grid calculation
    array_index = ((int)cam_z_int * 36 + (int)cam_x_int) * 8;
    
    // Base cell calculation using Y
    cell_index = (*(int *)((char *)&g_LisaDefaultScale_Y + array_index) +
             ((int)((int)cam_y_int + ((int)cam_y_int >> 31 & 0xffU)) >> 8) + g_LisaDefaultOffset_X) * g_LisaGridCellsX;
             
    cam_z_int = __ftol();
    visible_array = (LisaDynamicObject **)g_LisaVisibleObjects;
    cell_idx = 3;

    // Final grid pointer offset
    grid_index = (int)g_pLisaGridCells +
            (cell_index + ((int)((int)cam_z_int + ((int)cam_z_int >> 31 & 0xffU)) >> 8) +
             *(int *)((char *)&g_LisaDefaultScale_X + array_index) + g_LisaDefaultOffset_Y) * 4;
             
    grid_offset_z = g_LisaDefaultOffset_Z;

    if (g_pLisaAllocatedBuffersEnd == 0) {
        while (grid_offset_z != -5000) {
            grid_offset_z = *(int *)(cell_idx * 4 + 0x499fa0);

            if (0 < grid_offset_z) {
                do {
                    obj_ptr = *(LisaDynamicObject **)(grid_index + 4);
                    grid_index += 4;

                    if ((obj_ptr != NULL) && (obj_ptr->self_ptr == obj_ptr)) {
                        obj_count = g_LisaCamera->visible_obj_count + 1;
                        g_LisaCamera->visible_obj_count = obj_count;
                        visible_array[obj_count - 1] = obj_ptr;

                        if (obj_ptr->next_in_cell != NULL) {
                            LisaDynamicObject **out_val_ptr = &visible_array[obj_count];

                            do {
                                obj_ptr = obj_ptr->next_in_cell;

                                if (obj_ptr->self_ptr == obj_ptr) {
                                    *out_val_ptr = obj_ptr;
                                    out_val_ptr++;
                                    g_LisaCamera->visible_obj_count++;
                                }

                            } while (obj_ptr->next_in_cell != NULL);
                        }
                    }
                    grid_offset_z--;
                } while (grid_offset_z != 0);
            }

            grid_offset_z = cell_idx + 1;
            cell_idx += 2;
            grid_index += (*(int *)(grid_offset_z * 4 + 0x499fa0) + g_LisaGridCellsX) * 4;
            grid_offset_z = *(int *)(cell_idx * 4 + 0x499fa0);
        }
    } else {
        while (grid_offset_z != -5000) {
            count = *(int *)(cell_idx * 4 + 0x499fa0);

            if (0 < count) {
                do {
                    obj_ptr = *(LisaDynamicObject **)(grid_index + 4);
                    grid_index += 4;

                    if ((obj_ptr != NULL) && (obj_ptr->self_ptr == obj_ptr)) {
                        if ((obj_ptr->unknown_24 == 0) || (obj_ptr->unknown_24 == g_pLisaAllocatedBuffersEnd)) {
                            obj_count = g_LisaCamera->visible_obj_count + 1;
                            g_LisaCamera->visible_obj_count = obj_count;
                            visible_array[obj_count - 1] = obj_ptr;
                        }

                        if (obj_ptr->next_in_cell != NULL) {
                            LisaDynamicObject **out_val_ptr = &visible_array[g_LisaCamera->visible_obj_count];

                            do {
                                obj_ptr = obj_ptr->next_in_cell;

                                if ((obj_ptr->self_ptr == obj_ptr) &&
                                   ((obj_ptr->unknown_24 == 0 || obj_ptr->unknown_24 == g_pLisaAllocatedBuffersEnd))) {
                                    *out_val_ptr = obj_ptr;
                                    out_val_ptr++;
                                    g_LisaCamera->visible_obj_count++;
                                }

                            } while (obj_ptr->next_in_cell != NULL);
                        }
                    }
                    count--;
                } while (count != 0);
            }

            grid_offset_z = cell_idx + 1;
            cell_idx += 2;
            grid_index += (*(int *)(grid_offset_z * 4 + 0x499fa0) + g_LisaGridCellsX) * 4;
            grid_offset_z = *(int *)(cell_idx * 4 + 0x499fa0);
        }
    }

    g_LisaCamera->submesh_count = g_LisaCamera->visible_obj_count;
    return 0;
}

/**
 * @original Lisa_FrustumCullObjects (MAINDOS_32BIT.EXE @ 0x00448e70, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_FrustumCullObjects(void) {
    LisaCamera *camera;
    LisaDynamicObject *obj_ptr;
    LisaDynamicObject **visible_array;
    LisaDynamicObject **out_val_ptr;
    
    int obj_count;
    int next_obj_count;
    int grid_offset_z;
    int cell_index;
    int grid_index;
    int cell_idx;
    int count;
    int layer_id;
    int grid_stride;
    int submesh_count;
    
    int cam_val1, cam_val2, cam_val3;
    double out_val_ST1;

    // Viewport and rotation vars
    int vp_center_x, vp_center_y, fov_x, fov_y;
    int min_x, max_x, min_y, max_y;
    int obj_x, obj_y, obj_z, rad_x, rad_y;
    LisaDynamicObject **grid_cells;
    int rot_matrix[9]; // Assuming Lisa_ComputeCameraRotationMatrix takes int[9]
    int proj_x, proj_y, z_depth;
    int obj_rot, obj_radius;

    camera = g_LisaCamera;
    camera->visible_obj_count = 0;

    if (camera->projection_type == 0) {
        cam_val1 = __ftol();
        cam_val2 = __ftol();
        cam_val3 = __ftol();
        grid_stride = ((int)cam_val1 * 36 + (int)cam_val2) * 8;

        cell_index = (*(int *)((char *)&g_LisaDefaultScale_Y + grid_stride) +
                 ((int)((int)cam_val3 + ((int)cam_val3 >> 31 & 0xffU)) >> 8) + g_LisaDefaultOffset_X) *
                g_LisaGridCellsX;
                
        cam_val1 = __ftol();
        layer_id = (int)g_pLisaAllocatedBuffersEnd;
        visible_array = (LisaDynamicObject **)g_LisaVisibleObjects;
        cell_idx = 3;

        grid_index = (int)g_pLisaGridCells +
                (cell_index + ((int)((int)cam_val1 + ((int)cam_val1 >> 31 & 0xffU)) >> 8) +
                 *(int *)((char *)&g_LisaDefaultScale_X + grid_stride) + g_LisaDefaultOffset_Y) * 4;
                 
        grid_offset_z = g_LisaDefaultOffset_Z;

        if (g_pLisaAllocatedBuffersEnd == 0) {
            while (grid_offset_z != -5000) {
                grid_offset_z = *(int *)(cell_idx * 4 + 0x499fa0);

                if (0 < grid_offset_z) {
                    do {
                        obj_ptr = *(LisaDynamicObject **)(grid_index + 4);
                        grid_index += 4;

                        if ((obj_ptr != NULL) && (obj_ptr->self_ptr == obj_ptr)) {
                            obj_count = camera->visible_obj_count;
                            next_obj_count = obj_count + 1;
                            camera->visible_obj_count = next_obj_count;
                            visible_array[obj_count] = obj_ptr;

                            if (obj_ptr->next_in_cell != NULL) {
                                out_val_ptr = visible_array + next_obj_count;
                                do {
                                    obj_ptr = obj_ptr->next_in_cell;
                                    if (obj_ptr->self_ptr == obj_ptr) {
                                        *out_val_ptr = obj_ptr;
                                        out_val_ptr++;
                                        camera->visible_obj_count++;
                                    }
                                } while (obj_ptr->next_in_cell != NULL);
                            }
                        }
                        grid_offset_z--;
                    } while (grid_offset_z != 0);
                }

                grid_offset_z = cell_idx + 1;
                cell_idx += 2;
                grid_index += (*(int *)(grid_offset_z * 4 + 0x499fa0) + g_LisaGridCellsX) * 4;
                grid_offset_z = *(int *)(cell_idx * 4 + 0x499fa0);
            }
        } else {
            while (grid_offset_z != -5000) {
                count = *(int *)(cell_idx * 4 + 0x499fa0);

                if (0 < count) {
                    do {
                        obj_ptr = *(LisaDynamicObject **)(grid_index + 4);
                        grid_index += 4;

                        if ((obj_ptr != NULL) && (obj_ptr->self_ptr == obj_ptr)) {
                            if ((obj_ptr->unknown_24 == 0) || (obj_ptr->unknown_24 == layer_id)) {
                                obj_count = camera->visible_obj_count;
                                camera->visible_obj_count = obj_count + 1;
                                visible_array[obj_count] = obj_ptr;
                            }

                            if (obj_ptr->next_in_cell != NULL) {
                                out_val_ptr = visible_array + camera->visible_obj_count;
                                do {
                                    obj_ptr = obj_ptr->next_in_cell;
                                    if ((obj_ptr->self_ptr == obj_ptr) &&
                                       ((obj_ptr->unknown_24 == 0 || obj_ptr->unknown_24 == layer_id))) {
                                        *out_val_ptr = obj_ptr;
                                        out_val_ptr++;
                                        camera->visible_obj_count++;
                                    }
                                } while (obj_ptr->next_in_cell != NULL);
                            }
                        }
                        count--;
                    } while (count != 0);
                }

                grid_offset_z = cell_idx + 1;
                cell_idx += 2;
                grid_index += (*(int *)(grid_offset_z * 4 + 0x499fa0) + g_LisaGridCellsX) * 4;
                grid_offset_z = *(int *)(cell_idx * 4 + 0x499fa0);
            }
        }
    } else {
        // Projection calculation
        fcos(camera->rot_y * 0.017453292519943295); // g_Const_TenthDegToRad approx
        grid_stride = camera->projection_type / 2;
        cam_val1 = __ftol();
        cam_val2 = __ftol();
        fsin(out_val_ST1);

        layer_id = (((int)cam_val1 + ((int)((int)cam_val2 + ((int)cam_val2 >> 31 & 0xffU)) >> 8)) - grid_stride) * g_LisaGridCellsX;
        cam_val1 = __ftol();
        cam_val2 = __ftol();
        
        cell_index = (int)g_pLisaAllocatedBuffersEnd;
        visible_array = (LisaDynamicObject **)g_LisaVisibleObjects;

        grid_cells = (LisaDynamicObject **)((int)g_pLisaGridCells +
                         ((layer_id + (int)cam_val1 +
                          ((int)((int)cam_val2 + ((int)cam_val2 >> 31 & 0xffU)) >> 8)) - grid_stride) * 4);

        if (g_pLisaAllocatedBuffersEnd == 0) {
            if (0 < camera->projection_type) {
                cell_idx = camera->projection_type;
                cell_index = g_LisaGridCellsX - cell_idx;

                do {
                    grid_stride = camera->projection_type;
                    if (0 < grid_stride) {
                        do {
                            obj_ptr = *grid_cells;

                            if ((obj_ptr != NULL) && (obj_ptr->self_ptr == obj_ptr)) {
                                obj_count = camera->visible_obj_count;
                                next_obj_count = obj_count + 1;
                                camera->visible_obj_count = next_obj_count;
                                visible_array[obj_count] = obj_ptr;

                                if (obj_ptr->next_in_cell != NULL) {
                                    out_val_ptr = visible_array + next_obj_count;
                                    do {
                                        obj_ptr = obj_ptr->next_in_cell;
                                        if (obj_ptr->self_ptr == obj_ptr) {
                                            *out_val_ptr = obj_ptr;
                                            out_val_ptr++;
                                            camera->visible_obj_count++;
                                        }
                                    } while (obj_ptr->next_in_cell != NULL);
                                }
                            }
                            grid_cells++;
                            grid_stride--;
                        } while (grid_stride != 0);
                    }
                    grid_cells += cell_index;
                    cell_idx--;
                } while (cell_idx != 0);
            }
        } else if (0 < camera->projection_type) {
            count = camera->projection_type;
            grid_stride = g_LisaGridCellsX - count;

            do {
                cell_idx = camera->projection_type;
                if (0 < cell_idx) {
                    do {
                        obj_ptr = *grid_cells;
                        if ((obj_ptr != NULL) && (obj_ptr->self_ptr == obj_ptr)) {
                            if ((obj_ptr->unknown_24 == 0) || (obj_ptr->unknown_24 == cell_index)) {
                                obj_count = camera->visible_obj_count;
                                camera->visible_obj_count = obj_count + 1;
                                visible_array[obj_count] = obj_ptr;
                            }

                            if (obj_ptr->next_in_cell != NULL) {
                                out_val_ptr = visible_array + camera->visible_obj_count;
                                do {
                                    obj_ptr = obj_ptr->next_in_cell;
                                    if ((obj_ptr->self_ptr == obj_ptr) &&
                                       ((obj_ptr->unknown_24 == 0 || obj_ptr->unknown_24 == cell_index))) {
                                        *out_val_ptr = obj_ptr;
                                        out_val_ptr++;
                                        camera->visible_obj_count++;
                                    }
                                } while (obj_ptr->next_in_cell != NULL);
                            }
                        }
                        grid_cells++;
                        cell_idx--;
                    } while (cell_idx != 0);
                }
                grid_cells += grid_stride;
                count--;
            } while (count != 0);
        }
    }

    vp_center_x = camera->viewport_x;
    vp_center_y = camera->viewport_y;
    fov_x = camera->fov_x;
    fov_y = camera->fov_y;
    
    // Bounds based on projection and rotation matrix
    min_x = g_SubpixelMinX >> 8;
    max_x = g_SubpixelMaxX >> 8;
    min_y = g_SubpixelMinY >> 8;
    max_y = g_SubpixelMaxY >> 8;
    
    Lisa_ComputeCameraRotationMatrix(rot_matrix); // fills 9 ints

    submesh_count = camera->visible_obj_count;
    camera->submesh_count = submesh_count;
    camera->visible_obj_count = 0; // Reset for actual culling step

    if (0 < submesh_count) {
        LisaDynamicObject **src_array = (LisaDynamicObject **)g_LisaVisibleObjects;
        LisaDynamicObject **dst_array = (LisaDynamicObject **)g_LisaVisibleObjects;

        do {
            obj_ptr = *src_array;
            
            // Re-read ftol inputs, presumably xyz pos of object relative to camera
            cam_val1 = __ftol();
            obj_x = (int)cam_val1;
            cam_val1 = __ftol();
            obj_y = (int)cam_val1;
            cam_val1 = __ftol();
            obj_z = (int)cam_val1;
            
            obj_radius = obj_ptr->unknown_22; // Offset 0x22 (34) -> unknown_22
            
            // Matrix multiply: rot_matrix[6, 7, 8] are tmp_val2, tmp_val1, tmp_val3
            z_depth = ((obj_z * rot_matrix[6] + obj_y * rot_matrix[7] + obj_x * rot_matrix[8]) >> 15) + vp_center_y;

            if (-1 < z_depth + obj_radius) {
                if (z_depth < vp_center_y) {
                    z_depth = vp_center_y;
                }

                proj_x = fov_x + (((obj_z * rot_matrix[3] + obj_y * rot_matrix[4] + obj_x * rot_matrix[5]) >> 15) *
                                  -vp_center_x) / z_depth;
                rad_x = 2 - (-vp_center_x * obj_radius) / z_depth;

                if ((min_x <= rad_x + proj_x) && (proj_x - rad_x <= max_x)) {
                    proj_y = fov_y + (((obj_z * rot_matrix[0] + obj_y * rot_matrix[1] + obj_x * rot_matrix[2]) >> 15) *
                                     -vp_center_y) / z_depth;
                    rad_y = 2 - (-vp_center_y * obj_radius) / z_depth;

                    if ((min_y < proj_y + rad_y) && (proj_y - rad_y < max_y)) {
                        camera->visible_obj_count++;
                        *dst_array = obj_ptr;
                        dst_array++;
                    }
                }
            }
            src_array++;
            submesh_count--;
        } while (submesh_count != 0);
    }
    return 0;
}

/**
 * @original Lisa_CullObjects (MAINDOS_32BIT.EXE @ 0x00449470, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_CullObjects(void) {
  float temp_f1;
  float temp_f2;
  float temp_f3;
  float temp_f4;
  float temp_f5;
  float temp_f6;
  float temp_f7;
  float temp_f8;
  int grid_offset;
  int *sub_out_ptr;
  LisaDynamicObject **out_val_ptr;
  unsigned int plane_idx;
  unsigned int cull_flag;
  LisaDynamicObject *obj_ptr;
  int *grid_index;
  int *count_ptr;
  float *matrix_ptr_next;
  float *matrix_ptr;
  int layer_id;
  int proj_type;
  LisaDynamicObject **visible_array;
  int visible_idx;
  LisaCamera *camera;
  int *plane_ptr;
  double rot_x_rad;
  double cos_rot_x;
  double cos_rot_y;
  double rot_z_rad;
  double sin_rot_y;
  double cos_rot_z;
  double sin_rot_x;
  int cam_val1;
  int cam_val2;
  int cam_val3;
  int cell_idx;
  int *submesh_count_ptr;
  int cell_limit;
  int *dst_array;
  double *cam_rot_y_ptr;
  float frustum_matrix [19];
  float matrix_end;

  

  camera = g_LisaCamera;
  proj_type = g_LisaCamera->projection_type;

  if (proj_type == 0) {
    g_LisaCamera->visible_obj_count = 0;
    grid_index = &camera->visible_obj_count;
    cam_rot_y_ptr = &g_LisaCamera->rot_y;
    cam_val1 = __ftol();
    cam_val2 = __ftol();
    proj_type = ((int)cam_val1 * 0x24 + (int)cam_val2) * 8;
    cam_val1 = __ftol();

    grid_offset = (*(int *)(&g_LisaDefaultScale_Y + proj_type) +

             ((int)((int)cam_val1 + ((int)cam_val1 >> 0x1f & 0xffU)) >> 8) + g_LisaDefaultOffset_X) *

            g_LisaGridCellsX;
    cam_val1 = __ftol();
    layer_id = g_pLisaAllocatedBuffersEnd;
    visible_array = g_LisaVisibleObjects;
    cell_idx = 3;

    proj_type = (int)g_pLisaGridCells +

             (grid_offset + ((int)((int)cam_val1 + ((int)cam_val1 >> 0x1f & 0xffU)) >> 8) +

              *(int *)(&g_LisaDefaultScale_X + proj_type) + g_LisaDefaultOffset_Y) * 4;
    grid_offset = g_LisaDefaultOffset_Z;

    if (g_pLisaAllocatedBuffersEnd == 0) {
      while (grid_offset != -5000) {
        grid_offset = *(int *)(cell_idx * 4 + 0x499fa0);

        if (0 < grid_offset) {
          do {
            obj_ptr = *(LisaDynamicObject **)(proj_type + 4);
            proj_type = proj_type + 4;

            if ((obj_ptr != (int *)0x0) && (obj_ptr->self_ptr == obj_ptr)) {
              layer_id = *grid_index;
              visible_array[layer_id] = (int)obj_ptr;
              layer_id = layer_id + 1;
              *grid_index = layer_id;

              if ((int)obj_ptr->next_in_cell != 0) {
                out_val_ptr = visible_array + layer_id;

                do {
                  obj_ptr = obj_ptr->next_in_cell;

                  if (obj_ptr->self_ptr == obj_ptr) {
                    *out_val_ptr = (int)obj_ptr;
                    out_val_ptr = out_val_ptr + 1;
                    *grid_index = *grid_index + 1;
                  }

                } while ((int)obj_ptr->next_in_cell != 0);
              }

            }

            grid_offset = grid_offset + -1;
          } while (grid_offset != 0);
        }

        grid_offset = cell_idx + 1;
        cell_idx = cell_idx + 2;
        proj_type = proj_type + (*(int *)(grid_offset * 4 + 0x499fa0) + g_LisaGridCellsX) * 4;
        grid_offset = *(int *)(cell_idx * 4 + 0x499fa0);
      }

    }

    else {
      while (grid_offset != -5000) {
        frustum_matrix[0] = *(float *)(cell_idx * 4 + 0x499fa0);

        if (0 < (int)frustum_matrix[0]) {
          do {
            obj_ptr = *(LisaDynamicObject **)(proj_type + 4);
            proj_type = proj_type + 4;

            if ((obj_ptr != (int *)0x0) && (obj_ptr->self_ptr == obj_ptr)) {
              if ((obj_ptr->unknown_24 == 0) || (obj_ptr->unknown_24 == layer_id)) {
                grid_offset = *grid_index;
                visible_array[grid_offset] = (int)obj_ptr;
                *grid_index = grid_offset + 1;
              }

              if ((int)obj_ptr->next_in_cell != 0) {
                out_val_ptr = visible_array + *grid_index;

                do {
                  obj_ptr = obj_ptr->next_in_cell;

                  if ((obj_ptr->self_ptr == obj_ptr) &&

                     ((obj_ptr->unknown_24 == 0 || (obj_ptr->unknown_24 == layer_id)))) {
                    *out_val_ptr = (int)obj_ptr;
                    out_val_ptr = out_val_ptr + 1;
                    *grid_index = *grid_index + 1;
                  }

                } while ((int)obj_ptr->next_in_cell != 0);
              }

            }

            frustum_matrix[0] = (float)((int)frustum_matrix[0] + -1);
          } while (frustum_matrix[0] != 0.0);
        }

        grid_offset = cell_idx + 1;
        cell_idx = cell_idx + 2;
        proj_type = proj_type + (*(int *)(grid_offset * 4 + 0x499fa0) + g_LisaGridCellsX) * 4;
        grid_offset = *(int *)(cell_idx * 4 + 0x499fa0);
      }

    }

  }

  else {
    g_LisaCamera->visible_obj_count = 0;
    count_ptr = &camera->visible_obj_count;
    cam_rot_y_ptr = &g_LisaCamera->rot_y;
    frustum_matrix[0] = (float)(proj_type / 3);
    fcos((double)*cam_rot_y_ptr * (double)g_Const_TenthDegToRad);
    cam_val1 = __ftol();
    cam_val2 = __ftol();
    fsin(sin_rot_x);

    layer_id = (((int)cam_val1 + ((int)((int)cam_val2 + ((int)cam_val2 >> 0x1f & 0xffU)) >> 8)) - proj_type / 2

             ) * g_LisaGridCellsX;
    cam_val1 = __ftol();
    cam_val2 = __ftol();
    grid_offset = g_pLisaAllocatedBuffersEnd;
    visible_array = g_LisaVisibleObjects;

    grid_index = (int *)((int)g_pLisaGridCells +

                     ((layer_id + (int)cam_val1 +

                      ((int)((int)cam_val2 + ((int)cam_val2 >> 0x1f & 0xffU)) >> 8)) - proj_type / 2) * 4);
    cell_limit = proj_type;

    if (g_pLisaAllocatedBuffersEnd == 0) {
      if (0 < proj_type) {
        grid_offset = g_LisaGridCellsX - proj_type;

        do {
          layer_id = proj_type;

          if (0 < proj_type) {
            do {
              out_val_ptr = (int *)*grid_index;

              if ((out_val_ptr != (int *)0x0) && ((int *)*out_val_ptr == out_val_ptr)) {
                visible_idx = *count_ptr;
                visible_array[visible_idx] = (int)out_val_ptr;
                visible_idx = visible_idx + 1;
                *count_ptr = visible_idx;

                if (*(int *)((int)out_val_ptr + 0x26) != 0) {
                  sub_out_ptr = visible_array + visible_idx;

                  do {
                    out_val_ptr = *(int **)((int)out_val_ptr + 0x26);

                    if ((int *)*out_val_ptr == out_val_ptr) {
                      *sub_out_ptr = (int)out_val_ptr;
                      sub_out_ptr = sub_out_ptr + 1;
                      *count_ptr = *count_ptr + 1;
                    }

                  } while (*(int *)((int)out_val_ptr + 0x26) != 0);
                }

              }

              grid_index = grid_index + 1;
              layer_id = layer_id + -1;
            } while (layer_id != 0);
          }

          grid_index = grid_index + grid_offset;
          cell_limit = cell_limit + -1;
        } while (cell_limit != 0);
      }

    }

    else if (0 < proj_type) {
      layer_id = g_LisaGridCellsX - proj_type;

      do {
        dst_array = (int *)proj_type;

        if (0 < proj_type) {
          do {
            out_val_ptr = (int *)*grid_index;

            if ((out_val_ptr != (int *)0x0) && ((int *)*out_val_ptr == out_val_ptr)) {
              if (((short)out_val_ptr[9] == 0) || ((short)out_val_ptr[9] == grid_offset)) {
                visible_idx = *count_ptr;
                visible_array[visible_idx] = (int)out_val_ptr;
                *count_ptr = visible_idx + 1;
              }

              if (*(int *)((int)out_val_ptr + 0x26) != 0) {
                sub_out_ptr = visible_array + *count_ptr;

                do {
                  out_val_ptr = *(int **)((int)out_val_ptr + 0x26);

                  if (((int *)*out_val_ptr == out_val_ptr) &&

                     (((short)out_val_ptr[9] == 0 || ((short)out_val_ptr[9] == grid_offset)))) {
                    *sub_out_ptr = (int)out_val_ptr;
                    sub_out_ptr = sub_out_ptr + 1;
                    *count_ptr = *count_ptr + 1;
                  }

                } while (*(int *)((int)out_val_ptr + 0x26) != 0);
              }

            }

            grid_index = grid_index + 1;
            dst_array = (int *)((int)dst_array + -1);
          } while (dst_array != (int *)0x0);
        }

        grid_index = grid_index + layer_id;
        cell_limit = cell_limit + -1;
      } while (cell_limit != 0);
    }

  }

  submesh_count_ptr = &camera->visible_obj_count;
  proj_type = g_LisaCamera->fov_x;
  camera = g_SubpixelMinX + proj_type * -0x100;

  if ((((g_LisaActiveMaterial != camera) || (g_SubpixelMaxX + proj_type * -0x100 != g_LisaSubmeshLodLevel)) ||

      (g_SubpixelMinY + g_LisaCamera->fov_y * -0x100 != g_LisaActiveSubmeshFlags)) ||

     (((g_SubpixelMaxY + g_LisaCamera->fov_y * -0x100 != g_LisaCameraPitch ||

       (g_LisaCamera->viewport_x != g_LisaSubmeshClipMask)) ||

      ((g_LisaCamera->viewport_y != g_LisaCameraDistance || (g_LisaMipmapQuality == 1)))))) {
    cull_flag = 0;
    g_LisaMipmapQuality = 0;
    g_LisaSubmeshLodLevel = g_SubpixelMaxX + proj_type * -0x100;
    grid_offset = g_LisaCamera->fov_y;
    g_LisaActiveSubmeshFlags = g_SubpixelMinY + grid_offset * -0x100;
    g_LisaCameraPitch = g_SubpixelMaxY + grid_offset * -0x100;
    g_LisaSubmeshClipMask = g_LisaCamera->viewport_x;
    g_LisaCameraDistance = g_LisaCamera->viewport_y;

    frustum_matrix[0] = ((float)((-1 - proj_type) * 0x100 + g_SubpixelMinX) * (float)g_Const_1000) /

                  ((float)g_LisaSubmeshClipMask * (float)g_Const_Neg256);

    frustum_matrix[3] = ((float)((1 - proj_type) * 0x100 + g_SubpixelMaxX) * (float)g_Const_1000) /

                  ((float)g_LisaSubmeshClipMask * (float)g_Const_Neg256);

    frustum_matrix[1] = ((float)((-1 - grid_offset) * 0x100 + g_SubpixelMinY) * (float)g_Const_1000) /

                  ((float)g_LisaCameraDistance * (float)g_Const_Neg256);

    frustum_matrix[7] = ((float)((1 - grid_offset) * 0x100 + g_SubpixelMaxY) * (float)g_Const_1000) /

                  ((float)g_LisaCameraDistance * (float)g_Const_Neg256);
    frustum_matrix[2] = 1000.0;
    frustum_matrix[4] = frustum_matrix[1];
    frustum_matrix[5] = 1000.0;
    frustum_matrix[6] = frustum_matrix[3];
    frustum_matrix[8] = 1000.0;
    frustum_matrix[9] = frustum_matrix[0];
    frustum_matrix[10] = frustum_matrix[7];
    frustum_matrix[0xb] = 1000.0;
    proj_type = 0;
    g_LisaActiveMaterial = camera;

    while( 1 ) {
      cull_flag = cull_flag + 1;
      plane_idx = cull_flag & 3;
      temp_f1 = frustum_matrix[plane_idx * 3];
      temp_f2 = *(float *)((int)frustum_matrix + proj_type + 8);
      temp_f3 = frustum_matrix[plane_idx * 3 + 2];
      temp_f4 = *(float *)((int)frustum_matrix + proj_type);
      temp_f5 = frustum_matrix[plane_idx * 3 + 1];
      temp_f6 = *(float *)((int)frustum_matrix + proj_type);

      *(float *)((int)&g_LisaFrustumPlaneLeft + proj_type) =

           frustum_matrix[plane_idx * 3 + 2] * *(float *)((int)frustum_matrix + proj_type + 4) -

           frustum_matrix[plane_idx * 3 + 1] * *(float *)((int)frustum_matrix + proj_type + 8);
      temp_f7 = frustum_matrix[plane_idx * 3];
      temp_f8 = *(float *)((int)frustum_matrix + proj_type + 4);
      *(float *)((int)&g_LisaFrustumPlaneRight + proj_type) = temp_f1 * temp_f2 - temp_f3 * temp_f4;
      *(float *)((int)&g_LisaFrustumPlaneTop + proj_type) = temp_f5 * temp_f6 - temp_f7 * temp_f8;

      temp_f1 = SQRT(*(float *)((int)&g_LisaFrustumPlaneLeft + proj_type) * *(float *)((int)&g_LisaFrustumPlaneLeft + proj_type)

                   + *(float *)((int)&g_LisaFrustumPlaneRight + proj_type) *

                     *(float *)((int)&g_LisaFrustumPlaneRight + proj_type) +

                     *(float *)((int)&g_LisaFrustumPlaneTop + proj_type) *

                     *(float *)((int)&g_LisaFrustumPlaneTop + proj_type));

      *(float *)((int)&g_LisaFrustumPlaneLeft + proj_type) =

           (*(float *)((int)&g_LisaFrustumPlaneLeft + proj_type) / temp_f1) * g_Const_TenthDegToRadFloat;

      *(float *)((int)&g_LisaFrustumPlaneRight + proj_type) =

           (*(float *)((int)&g_LisaFrustumPlaneRight + proj_type) / temp_f1) * g_Const_TenthDegToRadFloat;
      temp_f1 = (*(float *)((int)&g_LisaFrustumPlaneTop + proj_type) / temp_f1) * g_Const_TenthDegToRadFloat;
      if (0x2f < proj_type + 0xc) break;
      *(float *)((int)&g_LisaFrustumPlaneTop + proj_type) = temp_f1;
      proj_type = proj_type + 0xc;
    }

    *(float *)((int)&g_LisaFrustumPlaneTop + proj_type) = temp_f1;
  }

  rot_x_rad = (double)g_LisaCamera->rot_x * (double)g_Const_TenthDegToRad;
  cos_rot_x = (double)fcos(rot_x_rad);
  cos_rot_y = (double)fcos((double)*cam_rot_y_ptr * (double)g_Const_TenthDegToRad);
  rot_x_rad = (double)fsin(rot_x_rad);
  rot_z_rad = (double)g_LisaCamera->rot_z * (double)g_Const_TenthDegToRad;
  sin_rot_y = (double)fsin((double)*cam_rot_y_ptr * (double)g_Const_TenthDegToRad);
  cos_rot_z = (double)fcos(rot_z_rad);
  frustum_matrix[0] = (float)cos_rot_z;
  rot_z_rad = (double)fsin(rot_z_rad);
  g_LisaObjMat_CosRoll = (float)(rot_x_rad * sin_rot_y);
  g_LisaObjMat_CosPitch = (float)((double)g_LisaObjMat_CosRoll * rot_z_rad + (double)frustum_matrix[0] * cos_rot_y);
  g_LisaObjMat_SinPitch = (float)((double)frustum_matrix[0] * (double)g_LisaObjMat_CosRoll - cos_rot_y * rot_z_rad);
  g_LisaObjMat_SinRoll = (float)(cos_rot_x * rot_z_rad);
  g_LisaObjMat_Tmp1 = (float)((double)frustum_matrix[0] * cos_rot_x);
  g_LisaObjMat_Tmp2 = (float)-rot_x_rad;
  g_LisaObjMat_Tmp3 = (float)(rot_z_rad * rot_x_rad * cos_rot_y - (double)frustum_matrix[0] * sin_rot_y);
  g_LisaObjMat_21 = 0;
  g_LisaObjMat_Scale = 0;
  plane_ptr = &g_LisaFrustumPlaneTop;
  g_LisaObjMat_Tmp4 = (float)((double)frustum_matrix[0] * rot_x_rad * cos_rot_y + sin_rot_y * rot_z_rad);
  g_LisaObjMat_CosYaw = 0;
  g_LisaObjMat_Tmp5 = (float)(cos_rot_x * cos_rot_y);
  matrix_ptr = frustum_matrix;

  do {
    plane_ptr = plane_ptr + 3;
    cam_val1 = __ftol();
    *matrix_ptr = (float)cam_val1;
    cam_val1 = __ftol();
    matrix_ptr[1] = (float)cam_val1;
    cam_val1 = __ftol();
    matrix_ptr[2] = (float)cam_val1;
    matrix_ptr = matrix_ptr + 4;
  } while (plane_ptr < &g_LisaFrustumNear);
  cam_val1 = __ftol();
  cam_val2 = __ftol();
  cam_val3 = __ftol();
  cell_idx = *submesh_count_ptr;
  g_LisaCamera->submesh_count = cell_idx;
  *submesh_count_ptr = 0;
  matrix_ptr = frustum_matrix + 3;

  do {
    matrix_ptr_next = matrix_ptr + 4;

    *matrix_ptr = (float)-((int)matrix_ptr[-1] * (int)cam_val3 + (int)matrix_ptr[-2] * (int)cam_val2 +

                       (int)matrix_ptr[-3] * (int)cam_val1);
    matrix_ptr = matrix_ptr_next;
  } while (matrix_ptr_next < &matrix_end);
  dst_array = visible_array;

  if (0 < cell_idx) {
    do {
      proj_type = *visible_array;
      cull_flag = 0;
      matrix_ptr = frustum_matrix + 2;

      do {
        cull_flag = cull_flag | (int)matrix_ptr[-1] * obj_ptr->pos_y +

                          (int)matrix_ptr[-2] * obj_ptr->pos_x +

                          (int)*matrix_ptr * obj_ptr->pos_z + (int)matrix_ptr[1] +

                          obj_ptr->unknown_22 * 0x40000;
        if ((int)cull_flag < 0) break;
        matrix_ptr = matrix_ptr + 4;
      } while (matrix_ptr < frustum_matrix + 0x12);

      if (0 < (int)cull_flag) {
        *dst_array = proj_type;
        *submesh_count_ptr = *submesh_count_ptr + 1;
        dst_array = dst_array + 1;
      }

      visible_array = visible_array + 1;
      cell_idx = cell_idx + -1;
    } while (cell_idx != 0);
  }

  return 0;
}

/**
 * @original Lisa_TransformVertices (MAINDOS_32BIT.EXE @ 0x00449e70, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_TransformVertices(void) {
  /* Refactored and semantically cleaned */
  float roll_cos_f;
  float scale_x;
  float scale_y;
  int temp_x;
  int camera_int;
  int temp_z;
  int obj_y;
  int vert_z;
  int *dst_vert_ptr;
  int vert_x;
  int trans_y;
  int vert_y;
  int trans_z;
  double pitch_sin;
  double pitch_cos;
  double yaw_sin;
  double yaw_cos;
  double roll_sin;
  double roll_cos;
  int cam_pos_x;
  int cam_pos_y;
  int cam_pos_z_1;
  int cam_pos_z_2;
  int vert_idx;
  int i;
  int *verts;
  LisaDynamicObject *obj;
  MshSubmesh *mesh;
  LisaCamera *camera = g_LisaCamera;
  void **visible_submeshes = (void **)g_LisaVisibleSubmeshes;
  LisaDynamicObject **visible_objs = (LisaDynamicObject **)g_LisaVisibleObjects;

  g_LisaBackfaceSign = 0;

  if (camera->zoom <= 0.0) {
    g_LisaBackfaceSign = 0xffffffff;
  }

  if (300 < camera->viewport_x) {
    Lisa_TransformVerticesPanorama();
    return;
  }

  camera->vertex_counter = 0;
  g_LisaCameraFocalLength = camera->viewport_width << 2;
  pitch_sin = (double)camera->rot_x * (double)g_Const_NegTenthDegToRad;
  g_LisaCameraOffsetX = camera->fov_x << 8;
  pitch_cos = (double)fcos(pitch_sin);
  g_LisaCameraOffsetY = camera->fov_y << 8;
  yaw_sin = (double)camera->rot_y * (double)g_Const_NegTenthDegToRad;
  yaw_cos = (double)fcos(yaw_sin);
  pitch_sin = (double)fsin(pitch_sin);
  roll_sin = (double)camera->rot_z * (double)g_Const_NegTenthDegToRad;
  yaw_sin = (double)fsin(yaw_sin);
  roll_cos = (double)fcos(roll_sin);
  roll_sin = (double)fsin(roll_sin);
  roll_cos_f = (float)roll_cos;
  scale_x = (float)-camera->viewport_x * (float)camera->zoom * (float)g_Const_1024;
  scale_y = (float)-camera->viewport_y * (float)g_Const_1024;

  g_LisaObjMat_CosPitch =
       (float)(((double)roll_cos_f * yaw_cos - (double)(float)(pitch_sin * yaw_sin) * roll_sin) *
              (double)scale_x);
  g_LisaObjMat_SinPitch = (float)-((double)scale_x * pitch_cos * roll_sin);
  g_LisaObjMat_CosRoll = (float)((pitch_sin * yaw_cos * roll_sin + (double)roll_cos_f * yaw_sin) * (double)scale_x);

  g_LisaObjMat_SinRoll =
       (float)(((double)(float)(pitch_sin * yaw_sin) * (double)roll_cos_f + yaw_cos * roll_sin) *
              (double)scale_y);
  g_LisaObjMat_Tmp1 = (float)((double)scale_y * pitch_cos * (double)roll_cos_f);
  g_LisaObjMat_Tmp3 = (float)(-(yaw_sin * pitch_cos) * (double)g_Const_262144);
  g_LisaObjMat_Tmp4 = (float)(pitch_sin * (double)g_Const_262144);

  g_LisaObjMat_Tmp2 =
       (float)((yaw_sin * roll_sin - (double)(float)(pitch_sin * yaw_cos) * (double)roll_cos_f) *
              (double)scale_y);
  g_LisaObjMat_Tmp5 = (float)(pitch_cos * yaw_cos * (double)g_Const_262144);
  cam_pos_x = __ftol();
  g_LisaCameraMatrix_00 = (int)cam_pos_x;
  cam_pos_x = __ftol();
  g_LisaCameraMatrix_01 = (int)cam_pos_x;
  cam_pos_x = __ftol();
  g_LisaCameraMatrix_02 = (int)cam_pos_x;
  cam_pos_x = __ftol();
  g_LisaCameraMatrix_10 = (int)cam_pos_x;
  cam_pos_x = __ftol();
  g_LisaCameraMatrix_11 = (int)cam_pos_x;
  cam_pos_x = __ftol();
  g_LisaCameraMatrix_12 = (int)cam_pos_x;
  cam_pos_x = __ftol();
  g_LisaCameraMatrix_20 = (int)cam_pos_x;
  cam_pos_x = __ftol();
  g_LisaCameraMatrix_21 = (int)cam_pos_x;
  cam_pos_x = __ftol();
  g_LisaCameraMatrix_22 = (int)cam_pos_x;
  cam_pos_x = __ftol();
  cam_pos_y = __ftol();
  camera_int = cam_pos_y;
  cam_pos_y = __ftol();
  cam_pos_z_1 = __ftol();
  trans_z = (int)cam_pos_z_1;
  cam_pos_z_1 = __ftol();
  cam_pos_z_2 = __ftol();
  temp_z = (int)cam_pos_z_2;
  temp_x = g_LisaCameraMatrix_01 * trans_z + camera_int * g_LisaCameraMatrix_00 + g_LisaCameraMatrix_02 * temp_z;
  g_LisaSubmeshCenterWorldX = (int)(temp_x + (temp_x >> 0x1f & 0xfffU)) >> 0xc;
  temp_x = g_LisaCameraMatrix_11 * trans_z + camera_int * g_LisaCameraMatrix_10 + g_LisaCameraMatrix_12 * temp_z;
  g_LisaSubmeshCenterWorldZ = (int)(temp_x + (temp_x >> 0x1f & 0xfffU)) >> 0xc;
  camera_int = g_LisaCameraMatrix_21 * trans_z + camera_int * g_LisaCameraMatrix_20 + g_LisaCameraMatrix_22 * temp_z;
  i = 0;
  g_LisaSubmeshDepthOffset = (int)(camera_int + (camera_int >> 0x1f & 0xfffU)) >> 0xc;

  if (0 < camera->visible_obj_count) {
    do {
      temp_z = g_LisaTransformedVertices;
      obj = visible_objs[i];
      mesh = obj->mesh_data;
      trans_z = camera->vertex_counter;
      visible_submeshes[i * 2] = mesh;
      dst_vert_ptr = (int *)(temp_z + trans_z * 0xc);
      temp_z = obj->pos_x - (int)cam_pos_x;
      visible_submeshes[i * 2 + 1] = dst_vert_ptr;
      temp_x = obj->pos_y - (int)cam_pos_y;
      obj_y = obj->pos_z - (int)cam_pos_z_1;

      if ((obj->rot_x == 0 && obj->rot_y == 0) &&
          obj->rot_z == 0) {
        int vert_stride = 0;
        vert_idx = mesh->vertex_count;

        if (0 < vert_idx) {
          camera->vertex_counter = trans_z + vert_idx;
          verts = (int *)((char *)mesh + 8);

          do {
            vert_x = verts[vert_stride] + temp_z;
            vert_y = temp_x - verts[vert_stride + 1];
            trans_z = vert_stride + 2;
            vert_stride = vert_stride + 3;
            vert_z = obj_y + verts[trans_z];

            trans_y = (vert_x * g_LisaCameraMatrix_10 + g_LisaCameraMatrix_12 * vert_z + g_LisaCameraMatrix_11 * vert_y) -
                     g_LisaSubmeshCenterWorldZ;

            trans_z = g_LisaCameraFocalLength +
                     ((vert_x * g_LisaCameraMatrix_20 + g_LisaCameraMatrix_22 * vert_z + g_LisaCameraMatrix_21 * vert_y) -
                      g_LisaSubmeshDepthOffset >> 0x10);

            if (trans_z < g_LisaCameraFocalLength) {
              trans_z = g_LisaCameraFocalLength;
            }

            *dst_vert_ptr = g_LisaCameraOffsetX +
                       ((vert_x * g_LisaCameraMatrix_00 + g_LisaCameraMatrix_02 * vert_z + g_LisaCameraMatrix_01 * vert_y) -
                       g_LisaSubmeshCenterWorldX) / trans_z;
            vert_z = g_LisaCameraOffsetY;
            dst_vert_ptr[2] = trans_z;
            vert_idx = vert_idx + -1;
            dst_vert_ptr[1] = vert_z + trans_y / trans_z;
            dst_vert_ptr = dst_vert_ptr + 3;
          } while (vert_idx != 0);
        }

      }
      else {
        Lisa_ComputeObjectMatrix(temp_z,temp_x,obj_y,(int)obj,(int *)mesh);
      }

      i = i + 1;
    } while (i < camera->visible_obj_count);
  }
  return;
}

/**
 * @original Lisa_TransformVerticesPanorama (MAINDOS_32BIT.EXE @ 0x0044a3d0, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_TransformVerticesPanorama(void) {
  float f_cos_roll;
  int *p_submesh_verts;
  float f_scale_x;
  float f_scale_y;
  int tmp_world_y;
  int tmp_cam_ptr;
  int obj_pos_x;
  int obj_pos_z;
  int vert_pos_x;
  int vert_pos_z;
  int *p_out_verts;
  int vert_pos_y;
  int vert_view_y;
  int tmp_view_z;
  double d_pitch_sin;
  double d_pitch_cos;
  double d_yaw_sin;
  double d_yaw_cos;
  double d_roll_sin;
  double d_roll_cos;
  int l_cam_pos_x;
  int l_cam_pos_y;
  int l_cam_pos_z;
  int l_cam_pos_w;
  int num_vertices;
  int submesh_list_idx;
  int vis_obj_idx;
  int obj_iter;

  

  tmp_cam_ptr = g_LisaCamera;
  
  g_LisaCamera->vertex_counter = 0;
  d_pitch_sin = (double)g_LisaCamera->rot_x * (double)g_Const_NegTenthDegToRad;
  g_LisaCameraOffsetX = g_LisaCamera->fov_x << 8;
  g_LisaCameraOffsetY = g_LisaCamera->fov_y << 8;
  d_yaw_sin = (double)g_LisaCamera->rot_y * (double)g_Const_NegTenthDegToRad;
  d_pitch_sin = (double)fsin(d_pitch_sin);
  d_roll_sin = (double)g_LisaCamera->rot_z * (double)g_Const_NegTenthDegToRad;
  d_roll_cos = (double)fcos(d_roll_sin);
  d_roll_sin = (double)fsin(d_roll_sin);
  f_cos_roll = (float)d_roll_cos;
  f_scale_x = (float)-g_LisaCamera->fov_y * (float)g_LisaCamera->zoom * (float)g_Const_256;
  f_scale_y = (float)-g_LisaCamera->viewport_y * (float)g_Const_256;

  f_scale_x = (float)-g_LisaCamera->viewport_x * (float)g_LisaCamera->zoom * (float)g_Const_256;
  f_scale_y = (float)-g_LisaCamera->viewport_y * (float)g_Const_256;
       (float)(((double)f_cos_roll * d_yaw_cos - (double)(float)(d_pitch_sin * d_yaw_sin) * d_roll_sin) *

              (double)f_scale_x);
  g_LisaObjMat_SinPitch = (float)-((double)f_scale_x * d_pitch_cos * d_roll_sin);

  g_LisaObjMat_CosRoll =

       (float)(((double)(float)(d_pitch_sin * d_yaw_cos) * d_roll_sin + (double)f_cos_roll * d_yaw_sin) *

              (double)f_scale_x);

  g_LisaObjMat_SinRoll =

       (float)(((double)f_cos_roll * (double)(float)(d_pitch_sin * d_yaw_sin) + d_yaw_cos * d_roll_sin) *

              (double)f_scale_y);
  g_LisaObjMat_Tmp1 = (float)((double)f_scale_y * d_pitch_cos * (double)f_cos_roll);
  g_LisaObjMat_Tmp4 = (float)(d_pitch_sin * (double)g_Const_262144);
  g_LisaObjMat_Tmp3 = (float)(-(d_pitch_cos * d_yaw_sin) * (double)g_Const_262144);

  g_LisaObjMat_Tmp2 =

       (float)((d_yaw_sin * d_roll_sin - (double)f_cos_roll * (double)(float)(d_pitch_sin * d_yaw_cos)) *

              (double)f_scale_y);
  g_LisaObjMat_Tmp5 = (float)(d_yaw_cos * d_pitch_cos * (double)g_Const_262144);
  l_cam_pos_x = __ftol();
  g_LisaCameraMatrix_00 = (int)l_cam_pos_x;
  l_cam_pos_x = __ftol();
  g_LisaCameraMatrix_01 = (int)l_cam_pos_x;
  l_cam_pos_x = __ftol();
  g_LisaCameraMatrix_02 = (int)l_cam_pos_x;
  l_cam_pos_x = __ftol();
  g_LisaCameraMatrix_10 = (int)l_cam_pos_x;
  l_cam_pos_x = __ftol();
  g_LisaCameraMatrix_11 = (int)l_cam_pos_x;
  l_cam_pos_x = __ftol();
  g_LisaCameraMatrix_12 = (int)l_cam_pos_x;
  l_cam_pos_x = __ftol();
  g_LisaCameraMatrix_20 = (int)l_cam_pos_x;
  l_cam_pos_x = __ftol();
  g_LisaCameraMatrix_21 = (int)l_cam_pos_x;
  l_cam_pos_x = __ftol();
  g_LisaCameraMatrix_22 = (int)l_cam_pos_x;
  l_cam_pos_x = __ftol();
  l_cam_pos_y = __ftol();
  tmp_cam_ptr = (int)l_cam_pos_y;
  l_cam_pos_y = __ftol();
  l_cam_pos_z = __ftol();
  tmp_view_z = (int)l_cam_pos_z;
  l_cam_pos_z = __ftol();
  l_cam_pos_w = __ftol();
  obj_pos_x = (int)l_cam_pos_w;
  tmp_world_y = g_LisaCameraMatrix_00 * tmp_cam_ptr + g_LisaCameraMatrix_02 * obj_pos_x + g_LisaCameraMatrix_01 * tmp_view_z;
  g_LisaSubmeshCenterWorldX = (int)(tmp_world_y + (tmp_world_y >> 0x1f & 0xfffU)) >> 0xc;
  tmp_world_y = g_LisaCameraMatrix_10 * tmp_cam_ptr + g_LisaCameraMatrix_12 * obj_pos_x + g_LisaCameraMatrix_11 * tmp_view_z;
  g_LisaSubmeshCenterWorldZ = (int)(tmp_world_y + (tmp_world_y >> 0x1f & 0xfffU)) >> 0xc;
  tmp_cam_ptr = g_LisaCameraMatrix_20 * tmp_cam_ptr + g_LisaCameraMatrix_22 * obj_pos_x + g_LisaCameraMatrix_21 * tmp_view_z;
  obj_iter = 0;
  g_LisaSubmeshDepthOffset = (int)(tmp_cam_ptr + (tmp_cam_ptr >> 0x1f & 0xfffU)) >> 0xc;

  if (0 < g_LisaCamera->visible_obj_count) {
    submesh_list_idx = 0;
    vis_obj_idx = 0;

    do {
      tmp_view_z = g_LisaCamera;
      tmp_cam_ptr = *(int *)(g_LisaVisibleObjects + vis_obj_idx);
      p_submesh_verts = *(int **)(tmp_cam_ptr + 4);
      *(int **)(g_LisaVisibleSubmeshes + submesh_list_idx) = p_submesh_verts;
      tmp_view_z = *(int *)(tmp_view_z + 0x5c);
      p_out_verts = (int *)(g_LisaTransformedVertices + tmp_view_z * 0xc);
      *(int **)(g_LisaVisibleSubmeshes + 4 + submesh_list_idx) = p_out_verts;
      obj_pos_x = *(int *)(tmp_cam_ptr + 0xc) - (int)l_cam_pos_x;
      tmp_world_y = *(int *)(tmp_cam_ptr + 0x10) - (int)l_cam_pos_y;
      obj_pos_z = *(int *)(tmp_cam_ptr + 0x14) - (int)l_cam_pos_z;

      if ((*(short *)(tmp_cam_ptr + 0x18) == 0 && *(short *)(tmp_cam_ptr + 0x1a) == 0) &&

          *(short *)(tmp_cam_ptr + 0x1c) == 0) {
        num_vertices = *p_submesh_verts;

        if (0 < num_vertices) {
          g_LisaCamera->vertex_counter = tmp_view_z + num_vertices;
          tmp_cam_ptr = 2;

          do {
            vert_pos_x = obj_pos_x + p_submesh_verts[tmp_cam_ptr];
            vert_pos_y = tmp_world_y - p_submesh_verts[tmp_cam_ptr + 1];
            vert_pos_z = obj_pos_z + p_submesh_verts[tmp_cam_ptr + 2];

            vert_view_y = (g_LisaCameraMatrix_12 * vert_pos_z + g_LisaCameraMatrix_11 * vert_pos_y + g_LisaCameraMatrix_10 * vert_pos_x) -

                     g_LisaSubmeshCenterWorldZ;

            tmp_view_z = ((g_LisaCameraMatrix_22 * vert_pos_z + g_LisaCameraMatrix_21 * vert_pos_y + g_LisaCameraMatrix_20 * vert_pos_x) -

                      g_LisaSubmeshDepthOffset >> 0x10) + g_LisaCameraFocalLength;

            if (tmp_view_z < g_LisaCameraFocalLength) {
              tmp_view_z = g_LisaCameraFocalLength;
            }

            *p_out_verts = g_LisaCameraOffsetX +

                       (((g_LisaCameraMatrix_02 * vert_pos_z + g_LisaCameraMatrix_01 * vert_pos_y + g_LisaCameraMatrix_00 * vert_pos_x) -

                        g_LisaSubmeshCenterWorldX) / tmp_view_z) * 4;
            num_vertices = num_vertices + -1;
            p_out_verts[2] = tmp_view_z;
            p_out_verts[1] = g_LisaCameraOffsetY + (vert_view_y / tmp_view_z) * 4;
            p_out_verts = p_out_verts + 3;
            tmp_cam_ptr = tmp_cam_ptr + 3;
          } while (num_vertices != 0);
        }

      }

      else {
        Lisa_TransformSubmeshVerticesPanorama(obj_pos_x,tmp_world_y,obj_pos_z,tmp_cam_ptr,p_submesh_verts);
      }

      submesh_list_idx = submesh_list_idx + 8;
      vis_obj_idx = vis_obj_idx + 4;
      obj_iter = obj_iter + 1;
    } while (obj_iter < g_LisaCamera->visible_obj_count);
  }

  return 0;
}

/**
 * @original Lisa_ComputeObjectMatrix (MAINDOS_32BIT.EXE @ 0x0044a900, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_ComputeObjectMatrix(int pos_x,int pos_y,int pos_z,int obj_ptr,int *vertex_array) {
  ushort rot_y;
  ushort rot_x;
  ushort rot_z;
  short temp_rot;
  int local_y;
  int local_x;
  int local_z;
  int cam_offset_y;
  int vertex_idx;
  int *out_val_vertex;
  int cam_space_x;
  int cam_space_y;
  int cam_space_z;
  int proj_z;
  int mat_00;
  int mat_01;
  int mat_02;
  int mat_10;
  int mat_11;
  int mat_12;
  int mat_20;
  int mat_21;
  int mat_22;
  int num_vertices;

  

  cam_space_x = (g_LisaCameraMatrix_02 * pos_z + g_LisaCameraMatrix_01 * pos_y + g_LisaCameraMatrix_00 * pos_x) - g_LisaSubmeshCenterWorldX

  ;

  cam_space_y = (g_LisaCameraMatrix_12 * pos_z + g_LisaCameraMatrix_11 * pos_y + g_LisaCameraMatrix_10 * pos_x) - g_LisaSubmeshCenterWorldZ

  ;

  cam_space_z = (g_LisaCameraMatrix_22 * pos_z + g_LisaCameraMatrix_21 * pos_y + g_LisaCameraMatrix_20 * pos_x) - g_LisaSubmeshDepthOffset

  ;
  rot_y = *(ushort *)(obj_ptr + 0x1a);
  rot_x = *(ushort *)(obj_ptr + 0x18);
  rot_z = *(ushort *)(obj_ptr + 0x1c);

  if (0xe10 < (ushort)(rot_x | rot_y | rot_z)) {
    if ((short)rot_x < 0) {
      *(ushort *)(obj_ptr + 0x18) = ((ushort)(0xe0f - rot_x) / 0xe10) * 0xe10 + rot_x;
    }

    if ((short)rot_y < 0) {
      *(ushort *)(obj_ptr + 0x1a) = ((ushort)(0xe0f - rot_y) / 0xe10) * 0xe10 + rot_y;
    }

    if ((short)rot_z < 0) {
      *(ushort *)(obj_ptr + 0x1c) = ((ushort)(0xe0f - rot_z) / 0xe10) * 0xe10 + rot_z;
    }

    temp_rot = *(short *)(obj_ptr + 0x18);

    if (0xe10 < temp_rot) {
      *(ushort *)(obj_ptr + 0x18) = ((ushort)(temp_rot - 1U) / 0xe10) * -0xe10 + temp_rot;
    }

    temp_rot = *(short *)(obj_ptr + 0x1a);

    if (0xe10 < temp_rot) {
      *(ushort *)(obj_ptr + 0x1a) = ((ushort)(temp_rot - 1U) / 0xe10) * -0xe10 + temp_rot;
    }

    temp_rot = *(short *)(obj_ptr + 0x1c);

    if (0xe10 < temp_rot) {
      *(ushort *)(obj_ptr + 0x1c) = ((ushort)(temp_rot - 1U) / 0xe10) * -0xe10 + temp_rot;
    }

  }

  mat_00 = __ftol();
  mat_01 = __ftol();
  mat_02 = __ftol();
  mat_10 = __ftol();
  mat_11 = __ftol();
  mat_12 = __ftol();
  mat_20 = __ftol();
  mat_21 = __ftol();
  mat_22 = __ftol();
  local_x = g_LisaTransformedVertices;
  vertex_idx = 2;
  num_vertices = *vertex_array;

  if (0 < num_vertices) {
    local_y = g_LisaCamera->vertex_counter;
    g_LisaCamera->vertex_counter = local_y + num_vertices;
    out_val_vertex = (int *)(local_x + local_y * 0xc);

    do {
      local_x = vertex_array[vertex_idx];
      local_y = vertex_array[vertex_idx + 1];
      local_z = vertex_array[vertex_idx + 2];
      vertex_idx = vertex_idx + 3;

      proj_z = g_LisaCameraFocalLength +

               (local_z * (int)mat_22 + local_y * (int)mat_21 + local_x * (int)mat_20 + cam_space_z >> 0x10);

      if (proj_z < g_LisaCameraFocalLength) {
        proj_z = g_LisaCameraFocalLength;
      }

      *out_val_vertex = g_LisaCameraOffsetX +

                 (cam_space_x + local_z * (int)mat_02 + local_y * (int)mat_01 + local_x * (int)mat_00) / proj_z

      ;
      cam_offset_y = g_LisaCameraOffsetY;
      out_val_vertex[2] = proj_z;
      num_vertices = num_vertices + -1;

      out_val_vertex[1] = cam_offset_y + (cam_space_y + local_z * (int)mat_12 + local_y * (int)mat_11 + local_x * (int)mat_10

                           ) / proj_z;
      out_val_vertex = out_val_vertex + 3;
    } while (num_vertices != 0);
  }

  return 0;
}

/**
 * @original Lisa_TransformSubmeshVerticesPanorama (MAINDOS_32BIT.EXE @ 0x0044ae20, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_TransformSubmeshVerticesPanorama(int pos_x,int pos_y,int pos_z,int submesh_ptr,int *vertex_array) {
  ushort rot_y;
  ushort rot_x;
  ushort rot_z;
  short temp_rot;
  int local_y;
  int local_x;
  int local_z;
  int vertex_idx;
  int proj_z;
  int *out_val_vertex;
  int cam_space_x;
  int cam_space_y;
  int cam_space_z;
  int mat_00;
  int mat_01;
  int mat_02;
  int mat_10;
  int mat_11;
  int mat_12;
  int mat_20;
  int mat_21;
  int mat_22;
  int num_vertices;

  

  cam_space_x = (g_LisaCameraMatrix_02 * pos_z + g_LisaCameraMatrix_01 * pos_y + g_LisaCameraMatrix_00 * pos_x) - g_LisaSubmeshCenterWorldX

  ;

  cam_space_y = (g_LisaCameraMatrix_12 * pos_z + g_LisaCameraMatrix_11 * pos_y + g_LisaCameraMatrix_10 * pos_x) - g_LisaSubmeshCenterWorldZ

  ;

  cam_space_z = (g_LisaCameraMatrix_22 * pos_z + g_LisaCameraMatrix_21 * pos_y + g_LisaCameraMatrix_20 * pos_x) - g_LisaSubmeshDepthOffset

  ;
  rot_y = *(ushort *)(submesh_ptr + 0x1a);
  rot_x = *(ushort *)(submesh_ptr + 0x18);
  rot_z = *(ushort *)(submesh_ptr + 0x1c);

  if (0xe10 < (ushort)(rot_x | rot_y | rot_z)) {
    if ((short)rot_x < 0) {
      *(ushort *)(submesh_ptr + 0x18) = ((ushort)(0xe0f - rot_x) / 0xe10) * 0xe10 + rot_x;
    }

    if ((short)rot_y < 0) {
      *(ushort *)(submesh_ptr + 0x1a) = ((ushort)(0xe0f - rot_y) / 0xe10) * 0xe10 + rot_y;
    }

    if ((short)rot_z < 0) {
      *(ushort *)(submesh_ptr + 0x1c) = ((ushort)(0xe0f - rot_z) / 0xe10) * 0xe10 + rot_z;
    }

    temp_rot = *(short *)(submesh_ptr + 0x18);

    if (0xe10 < temp_rot) {
      *(ushort *)(submesh_ptr + 0x18) = ((ushort)(temp_rot - 1U) / 0xe10) * -0xe10 + temp_rot;
    }

    temp_rot = *(short *)(submesh_ptr + 0x1a);

    if (0xe10 < temp_rot) {
      *(ushort *)(submesh_ptr + 0x1a) = ((ushort)(temp_rot - 1U) / 0xe10) * -0xe10 + temp_rot;
    }

    temp_rot = *(short *)(submesh_ptr + 0x1c);

    if (0xe10 < temp_rot) {
      *(ushort *)(submesh_ptr + 0x1c) = ((ushort)(temp_rot - 1U) / 0xe10) * -0xe10 + temp_rot;
    }
  }

  mat_00 = __ftol();
  mat_01 = __ftol();
  mat_02 = __ftol();
  mat_10 = __ftol();
  mat_11 = __ftol();
  mat_12 = __ftol();
  mat_20 = __ftol();
  mat_21 = __ftol();
  mat_22 = __ftol();
  local_x = g_LisaTransformedVertices;
  num_vertices = *vertex_array;

  if (0 < num_vertices) {
    local_y = g_LisaCamera->vertex_counter;
    g_LisaCamera->vertex_counter = local_y + num_vertices;
    vertex_idx = 2;
    out_val_vertex = (int *)(local_x + local_y * 0xc);

    do {
      local_x = vertex_array[vertex_idx];
      local_y = vertex_array[vertex_idx + 1];
      local_z = vertex_array[vertex_idx + 2];

      proj_z = (local_y * (int)mat_21 + local_z * (int)mat_22 + local_x * (int)mat_20 + cam_space_z >> 0x10) +

              g_LisaCameraFocalLength;

      if (proj_z < g_LisaCameraFocalLength) {
        proj_z = g_LisaCameraFocalLength;
      }

      *out_val_vertex = g_LisaCameraOffsetX +

                 ((cam_space_x + local_y * (int)mat_01 + local_z * (int)mat_02 + local_x * (int)mat_00) / proj_z

                 ) * 4;
      num_vertices = num_vertices + -1;
      out_val_vertex[2] = proj_z;

      out_val_vertex[1] = g_LisaCameraOffsetY +

                   ((cam_space_y + local_z * (int)mat_12 + local_y * (int)mat_11 + local_x * (int)mat_10) /

                   proj_z) * 4;
      vertex_idx = vertex_idx + 3;
      out_val_vertex = out_val_vertex + 3;
    } while (num_vertices != 0);
  }

  return 0;
}

/**
 * @original Lisa_ComputeCameraRotationMatrix (MAINDOS_32BIT.EXE @ 0x0044b340, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_ComputeCameraRotationMatrix(int *out_val_matrix) {
  double rot_val1;
  double rot_val2;
  int matrix_val;

  

  rot_val1 = (double)g_LisaCamera->rot_x * (double)g_Const_NegTenthDegToRad;
  fcos(rot_val1);
  rot_val2 = (double)g_LisaCamera->rot_y * (double)g_Const_NegTenthDegToRad;
  fcos(rot_val2);
  fsin(rot_val1);
  rot_val1 = (double)g_LisaCamera->rot_z * (double)g_Const_NegTenthDegToRad;
  fsin(rot_val2);
  fcos(rot_val1);
  fsin(rot_val1);
  matrix_val = __ftol();
  *out_val_matrix = (int)matrix_val;
  matrix_val = __ftol();
  out_val_matrix[1] = (int)matrix_val;
  matrix_val = __ftol();
  out_val_matrix[2] = (int)matrix_val;
  matrix_val = __ftol();
  out_val_matrix[3] = (int)matrix_val;
  matrix_val = __ftol();
  out_val_matrix[4] = (int)matrix_val;
  matrix_val = __ftol();
  out_val_matrix[5] = (int)matrix_val;
  matrix_val = __ftol();
  out_val_matrix[6] = (int)matrix_val;
  matrix_val = __ftol();
  out_val_matrix[7] = (int)matrix_val;
  matrix_val = __ftol();
  out_val_matrix[8] = (int)matrix_val;
  return;
}

/**
 * @original Lisa_InitOpcodeTable (MAINDOS_32BIT.EXE @ 0x0044b480, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_InitOpcodeTable(void) {
  int idx;
  LisaCamera *camera;

  camera = g_LisaCamera;
  g_LisaViewportQuarter = (g_LisaCamera->viewport_width >> 2) + 1;
  g_LisaViewportRemaining = g_LisaCamera->viewport_width - g_LisaViewportQuarter;

  if (g_LisaDisableFiltering != 0) {
    if (g_LisaCamera->shading_mode == 0) {
      if (g_LisaShadingEnabled == 0) {
        g_LisaRenderTexturedOp11_FuncPtr = ((void *)0x0044c2e0);
      }

      else {
        g_LisaRenderTexturedOp11_FuncPtr = ((void *)0x0044c610);
      }

    }

    else {
      g_LisaRenderTexturedOp11_FuncPtr = ((void *)0x0044bb90);
    }

    g_LisaRenderTexturedOp15_FuncPtr = Lisa_DrawTexturedTriangle_Op15;
    g_LisaCamera->active_draw_cmd = 0;
    *(int *)(idx + 0x6c) = 0;
    return;
  }

  if (g_LisaCamera->shading_mode == 0) {
    if (g_LisaShadingEnabled == 0) {
      g_LisaRenderTexturedOp11_FuncPtr = ((void *)0x0044c2e0);
    }

    else {
      g_LisaRenderTexturedOp11_FuncPtr = Lisa_DrawTexturedTriangle_Op11_Unshaded;
    }

  }

  else {
    g_LisaRenderTexturedOp11_FuncPtr = ((void *)0x0044bb90);

    if (g_LisaShadingEnabled != 0) {
      g_LisaRenderTexturedOp11_FuncPtr = Lisa_DrawTexturedTriangle_Op11_Shaded;
    }

  }

  if (g_LisaShadingEnabled == 0) {
    g_LisaRenderTexturedOp15_FuncPtr = Lisa_DrawTexturedTriangle_Op15;
    g_LisaCamera->active_draw_cmd = 0;
    *(int *)(idx + 0x6c) = 0;
    return;
  }

  g_LisaRenderTexturedOp15_FuncPtr = Lisa_DrawTexturedTriangle_Op15_Sub;
  g_LisaCamera->active_draw_cmd = 0;
  *(int *)(idx + 0x6c) = 0;
  return;
}

/**
 * @original Lisa_SortDepthBuckets (MAINDOS_32BIT.EXE @ 0x0044b570, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_SortDepthBuckets(void) {
    int *draw_cmds = (int *)g_LisaDrawCommands;
    int **bucket_ptr = (int **)((char *)g_pLisaDepthBuckets + 0x5dbc);
    int bucket_idx = 5999;
    int cmd_count = 0;

    do {
        int *node = *bucket_ptr;

        if (node != NULL) {
            draw_cmds[cmd_count++] = *node;
            while (node[1] != 0) {
                node = (int *)node[1];
                draw_cmds[cmd_count++] = *node;
            }
        }

        *bucket_ptr = NULL;
        bucket_ptr--;
        bucket_idx--;
    } while (bucket_idx >= 0);

    draw_cmds[cmd_count] = 0;
    return 0;
}

/**
 * @original Lisa_DrawTriangle_Op0F (MAINDOS_32BIT.EXE @ 0x0044b770, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_DrawTriangle_Op0F(void) {
    uint32_t v0 = g_pLisaSubmeshPolygon[1];
    uint32_t v1 = g_pLisaSubmeshPolygon[2];
    uint32_t v2 = g_pLisaSubmeshPolygon[3];
    int *verts = (int *)g_LisaTransformedVertices;
    int x0 = verts[v0 * 3 + 0];
    int y0 = verts[v0 * 3 + 1];
    int x1 = verts[v1 * 3 + 0];
    int y1 = verts[v1 * 3 + 1];
    int x2 = verts[v2 * 3 + 0];
    int y2 = verts[v2 * 3 + 1];
    int max_x, min_x, max_y, min_y;
    int depth;
    int cross;

    max_x = (x1 > x0) ? x1 : x0;
    if (x2 > max_x) max_x = x2;
    if (max_x < g_SubpixelMinX) {
        g_pLisaSubmeshPolygon += 5;
        return;
    }

    min_x = (x0 < x1) ? x0 : x1;
    if (x2 < min_x) min_x = x2;
    if (min_x > g_SubpixelMaxX) {
        g_pLisaSubmeshPolygon += 5;
        return;
    }

    max_y = (y1 > y0) ? y1 : y0;
    if (y2 > max_y) max_y = y2;
    if (max_y < g_SubpixelMinY) {
        g_pLisaSubmeshPolygon += 5;
        return;
    }

    min_y = (y0 < y1) ? y0 : y1;
    if (y2 < min_y) min_y = y2;
    if (min_y > g_SubpixelMaxY) {
        g_pLisaSubmeshPolygon += 5;
        return;
    }

    depth = verts[v0 * 3 + 2] + verts[v1 * 3 + 2] + verts[v2 * 3 + 2];
    if (depth > 600) {
        cross = (y2 - y1) * (x0 - x1) + (y1 - y0) * (x2 - x1);
        if ((cross ^ g_LisaBackfaceSign) > 0) {
            uint32_t *cmd = (uint32_t *)*g_pLisaDrawCommandWritePtr;
            int **pwrite;
            int **buckets;

            cmd[0] = g_pLisaSubmeshPolygon[0] & 0xffff;
            cmd[1] = (uint32_t)x0;
            cmd[2] = (uint32_t)y0;
            cmd[4] = (uint32_t)x1;
            cmd[5] = (uint32_t)y1;
            cmd[7] = (uint32_t)x2;
            cmd[8] = (uint32_t)y2;
            cmd[10] = g_pLisaSubmeshPolygon[4];
            cmd[11] = 0;

            depth = depth >> 4;
            if (*(short *)(g_pLisaActiveSubmesh + 0x1e) > 99) {
                depth -= 0x5c;
                if (depth < 0) depth = 0;
            }
            if (depth > 5999) depth = 5999;

            g_pLisaDrawCommandWritePtr[2] = (int)(cmd + 12);
            pwrite = (int **)g_pLisaDrawCommandWritePtr;
            buckets = (int **)g_pLisaDepthBuckets;

            g_pLisaDrawCommandWritePtr = (int *)(pwrite + 2);
            pwrite[1] = buckets[depth];
            buckets[depth] = (int *)pwrite;
        }
    }

    g_pLisaSubmeshPolygon += 5;
}

/**
 * @original Lisa_DrawTriangle_Op10 (MAINDOS_32BIT.EXE @ 0x0044b980, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_DrawTriangle_Op10(void) {
    uint32_t v0 = g_pLisaSubmeshPolygon[1];
    uint32_t v1 = g_pLisaSubmeshPolygon[2];
    uint32_t v2 = g_pLisaSubmeshPolygon[3];
    int *verts = (int *)g_LisaTransformedVertices;
    int x0 = verts[v0 * 3 + 0];
    int y0 = verts[v0 * 3 + 1];
    int x1 = verts[v1 * 3 + 0];
    int y1 = verts[v1 * 3 + 1];
    int x2 = verts[v2 * 3 + 0];
    int y2 = verts[v2 * 3 + 1];
    int max_x, min_x, max_y, min_y;
    int depth;
    int cross;

    max_x = (x1 > x0) ? x1 : x0;
    if (x2 > max_x) max_x = x2;
    if (max_x < g_SubpixelMinX) {
        g_pLisaSubmeshPolygon += 5;
        return;
    }

    min_x = (x0 < x1) ? x0 : x1;
    if (x2 < min_x) min_x = x2;
    if (min_x > g_SubpixelMaxX) {
        g_pLisaSubmeshPolygon += 5;
        return;
    }

    max_y = (y1 > y0) ? y1 : y0;
    if (y2 > max_y) max_y = y2;
    if (max_y < g_SubpixelMinY) {
        g_pLisaSubmeshPolygon += 5;
        return;
    }

    min_y = (y0 < y1) ? y0 : y1;
    if (y2 < min_y) min_y = y2;
    if (min_y > g_SubpixelMaxY) {
        g_pLisaSubmeshPolygon += 5;
        return;
    }

    depth = verts[v0 * 3 + 2] + verts[v1 * 3 + 2] + verts[v2 * 3 + 2];
    if (depth > 600) {
        cross = (y2 - y1) * (x0 - x1) + (y1 - y0) * (x2 - x1);
        if ((cross ^ g_LisaBackfaceSign) > 0) {
            uint32_t *cmd = (uint32_t *)*g_pLisaDrawCommandWritePtr;
            int **pwrite;
            int **buckets;

            cmd[0] = g_pLisaSubmeshPolygon[0] & 0xffff;
            cmd[1] = (uint32_t)x0;
            cmd[2] = (uint32_t)y0;
            cmd[4] = 0;
            cmd[5] = (uint32_t)x1;
            cmd[6] = (uint32_t)y1;
            cmd[8] = 0;
            cmd[9] = (uint32_t)x2;
            cmd[10] = (uint32_t)y2;
            cmd[12] = 0;
            cmd[13] = g_pLisaSubmeshPolygon[4];

            depth = depth >> 4;
            if (*(short *)(g_pLisaActiveSubmesh + 0x1e) > 99) {
                depth -= 0x5c;
                if (depth < 0) depth = 0;
            }
            if (depth > 5999) depth = 5999;

            g_pLisaDrawCommandWritePtr[2] = (int)(cmd + 14);
            pwrite = (int **)g_pLisaDrawCommandWritePtr;
            buckets = (int **)g_pLisaDepthBuckets;

            g_pLisaDrawCommandWritePtr = (int *)(pwrite + 2);
            pwrite[1] = buckets[depth];
            buckets[depth] = (int *)pwrite;
        }
    }

    g_pLisaSubmeshPolygon += 5;
}

/**
 * @original Lisa_RenderSubmeshes (MAINDOS_32BIT.EXE @ 0x0044c1f0, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_RenderSubmeshes(void) {
    int *submesh_data;
    int obj_idx;
    int obj_offset;
    int submesh_offset;

    obj_idx = 0;
    Lisa_InitOpcodeTable();
    g_pLisaDrawCommandWritePtr = g_LisaDrawCommandBuffer;
    *g_LisaDrawCommandBuffer = g_pLisaDrawCommandTail;

    if (g_LisaCamera->visible_obj_count > 0) {
        submesh_offset = 0;
        obj_offset = 0;

        do {
            g_pLisaActiveSubmesh = *(int *)(g_LisaVisibleObjects + obj_offset);
            submesh_data = *(int **)(g_LisaVisibleSubmeshes + submesh_offset);
            g_pLisaActiveMipTable = *(int *)(g_pLisaTextureSheets + *(int *)(g_pLisaActiveSubmesh + 8) * 4);
            g_pLisaSubmeshPolygon = submesh_data + *submesh_data * 3 + 2;
            g_LisaTransformedVertices = ((int *)(g_LisaVisibleSubmeshes + submesh_offset))[1];

            for (g_LisaSubmeshPolyCount = submesh_data[1]; g_LisaSubmeshPolyCount > 0; g_LisaSubmeshPolyCount--) {
                ((void (*)(void)) g_LisaOpcodeTable[(char)*g_pLisaSubmeshPolygon])();
            }

            obj_idx++;
            submesh_offset += 8;
            obj_offset += 4;
        } while (obj_idx < g_LisaCamera->visible_obj_count);
    }

    g_LisaCamera->active_draw_cmd =
        (int)(((int)g_pLisaDrawCommandWritePtr - (int)g_LisaDrawCommandBuffer) +
              ((int)g_pLisaDrawCommandWritePtr - (int)g_LisaDrawCommandBuffer >> 0x1f & 7U)) >> 3;
    *g_pLisaDrawCommandWritePtr = 0;
    return 0;
}

/**
 * @original Lisa_DrawTriangle_OpcodeHelper (MAINDOS_32BIT.EXE @ 0x0044cb20, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_DrawTriangle_OpcodeHelper(int shd_table, int depth_bias) {
    uint32_t *poly = (uint32_t *)g_pLisaSubmeshPolygon;
    int *verts = (int *)g_LisaTransformedVertices;
    uint32_t v0 = poly[1];
    uint32_t v1 = poly[2];
    uint32_t v2 = poly[3];
    int x0 = verts[v0 * 3 + 0];
    int y0 = verts[v0 * 3 + 1];
    int z0 = verts[v0 * 3 + 2];
    int x1 = verts[v1 * 3 + 0];
    int y1 = verts[v1 * 3 + 1];
    int z1 = verts[v1 * 3 + 2];
    int x2 = verts[v2 * 3 + 0];
    int y2 = verts[v2 * 3 + 1];
    int z2 = verts[v2 * 3 + 2];
    int max_x, min_x, max_y, min_y;
    int depth, total_z;
    int cross;
    short submesh_type;

    max_x = (x1 > x0) ? x1 : x0;
    if (x2 > max_x) max_x = x2;
    if (max_x < g_SubpixelMinX) {
        g_pLisaSubmeshPolygon += 11;
        return;
    }

    min_x = (x0 < x1) ? x0 : x1;
    if (x2 < min_x) min_x = x2;
    if (min_x > g_SubpixelMaxX) {
        g_pLisaSubmeshPolygon += 11;
        return;
    }

    max_y = (y1 > y0) ? y1 : y0;
    if (y2 > max_y) max_y = y2;
    if (max_y < g_SubpixelMinY) {
        g_pLisaSubmeshPolygon += 11;
        return;
    }

    min_y = (y0 < y1) ? y0 : y1;
    if (y2 < min_y) min_y = y2;
    if (min_y > g_SubpixelMaxY) {
        g_pLisaSubmeshPolygon += 11;
        return;
    }

    total_z = z0 + z1 + z2;
    if (total_z <= 600) {
        g_pLisaSubmeshPolygon += 11;
        return;
    }

    cross = ((y2 >> 4) - (y1 >> 4)) * ((x0 >> 4) - (x1 >> 4)) +
            ((y1 >> 4) - (y0 >> 4)) * ((x2 >> 4) - (x1 >> 4));
    if ((cross ^ g_LisaBackfaceSign) <= 0) {
        g_pLisaSubmeshPolygon += 11;
        return;
    }

    depth = total_z >> 4;
    submesh_type = *(short *)(g_pLisaActiveSubmesh + 0x1e);

    if (depth < 0x2d1 &&
        (submesh_type < 100 || submesh_type == 200 || (submesh_type > 299 && submesh_type < 0x12f)) &&
        g_LisaCamera->shading_mode != 0 &&
        (char *)(poly[0] & 0xffff0000) != (char *)&g_LisaPerspectiveDepthTable) {
        int *cmd = (int *)*g_pLisaDrawCommandWritePtr;
        int **pwrite;
        int **buckets;

        *cmd = 0x15;
        if (depth < 0x1e1) {
            cmd[4] = z0;
            cmd[7] = z1;
            cmd[10] = z2;
        } else {
            int avg_z = total_z / 3;
            cmd[4] = (int)(double)z0 + avg_z;
            cmd[7] = (int)(double)z1 + avg_z;
            cmd[10] = (int)(double)z2 + avg_z;
        }

        cmd[1] = *(int *)(g_pLisaActiveMipTable - 5) + poly[10];
        cmd[2] = x0;
        cmd[3] = y0;
        cmd[5] = x1;
        cmd[6] = y1;
        cmd[8] = x2;
        cmd[9] = y2;
        cmd[11] = poly[4];
        cmd[12] = poly[5];
        cmd[13] = poly[6];
        cmd[14] = poly[7];
        cmd[15] = poly[8];
        cmd[16] = poly[9];
        cmd[17] = shd_table;

        cmd += 18;
        g_pLisaDrawCommandWritePtr[2] = (int)cmd;

        depth -= depth_bias;
        if (submesh_type > 99) {
            depth -= 0x5c;
        }
        if (depth < 0) depth = 0;
        if (depth > 5999) depth = 5999;

        pwrite = (int **)g_pLisaDrawCommandWritePtr;
        buckets = (int **)g_pLisaDepthBuckets;
        g_pLisaDrawCommandWritePtr = (int *)(pwrite + 2);
        pwrite[1] = buckets[depth];
        buckets[depth] = (int *)pwrite;
    } else {
        int *cmd = (int *)*g_pLisaDrawCommandWritePtr;
        int **pwrite;
        int **buckets;

        *cmd = 0x12;
        cmd[1] = x0;
        cmd[2] = y0;
        cmd[3] = x1;
        cmd[4] = y1;
        cmd[5] = x2;
        cmd[6] = y2;
        cmd[7] = (int)(poly + 4);
        cmd[8] = *(int *)(g_pLisaActiveMipTable - 5) + poly[10];
        cmd[9] = shd_table;

        cmd += 10;
        g_pLisaDrawCommandWritePtr[2] = (int)cmd;

        depth -= depth_bias;
        if (submesh_type > 99) {
            depth -= 0x5c;
        }
        if (depth < 0) depth = 0;
        if (depth > 5999) depth = 5999;

        pwrite = (int **)g_pLisaDrawCommandWritePtr;
        buckets = (int **)g_pLisaDepthBuckets;
        g_pLisaDrawCommandWritePtr = (int *)(pwrite + 2);
        pwrite[1] = buckets[depth];
        buckets[depth] = (int *)pwrite;
    }

    g_pLisaSubmeshPolygon += 11;
}

/**
 * @original Lisa_DrawTriangle_Op14 (MAINDOS_32BIT.EXE @ 0x0044cf00, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_DrawTriangle_Op14(void) {
    int v0 = ((MshPolygon*)g_pLisaSubmeshPolygon)->vi0;
    int v1 = ((MshPolygon*)g_pLisaSubmeshPolygon)->vi1;
    int v2 = ((MshPolygon*)g_pLisaSubmeshPolygon)->vi2;
    int *verts = (int *)g_LisaTransformedVertices;
    int x0 = verts[v0 * 3 + 0];
    int y0 = verts[v0 * 3 + 1];
    int x1 = verts[v1 * 3 + 0];
    int y1 = verts[v1 * 3 + 1];
    int x2 = verts[v2 * 3 + 0];
    int y2 = verts[v2 * 3 + 1];
    int max_x, min_x, max_y, min_y;
    int depth;
    int cross;

    max_x = (x1 > x0) ? x1 : x0;
    if (x2 > max_x) max_x = x2;
    if (max_x < g_SubpixelMinX) {
        g_pLisaSubmeshPolygon = (int *)((char *)g_pLisaSubmeshPolygon + 0x14);
        return;
    }

    min_x = (x0 < x1) ? x0 : x1;
    if (x2 < min_x) min_x = x2;
    if (min_x > g_SubpixelMaxX) {
        g_pLisaSubmeshPolygon = (int *)((char *)g_pLisaSubmeshPolygon + 0x14);
        return;
    }

    max_y = (y1 > y0) ? y1 : y0;
    if (y2 > max_y) max_y = y2;
    if (max_y < g_SubpixelMinY) {
        g_pLisaSubmeshPolygon = (int *)((char *)g_pLisaSubmeshPolygon + 0x14);
        return;
    }

    min_y = (y0 < y1) ? y0 : y1;
    if (y2 < min_y) min_y = y2;
    if (min_y > g_SubpixelMaxY) {
        g_pLisaSubmeshPolygon = (int *)((char *)g_pLisaSubmeshPolygon + 0x14);
        return;
    }

    depth = verts[v0 * 3 + 2] + verts[v1 * 3 + 2] + verts[v2 * 3 + 2];
    if (depth > 600) {
        cross = (y1 - y0) * (x2 - x1) + (y2 - y1) * (x0 - x1);
        if ((cross ^ g_LisaBackfaceSign) > 0) {
            int *cmd = (int *)*g_pLisaDrawCommandWritePtr;
            int **pwrite;
            int **buckets;

            cmd[0] = 0x13;
            cmd[1] = x0;
            cmd[2] = y0;
            cmd[3] = x1;
            cmd[4] = y1;
            cmd[5] = x2;
            cmd[6] = y2;
            cmd[7] = ((MshPolygon*)g_pLisaSubmeshPolygon)->tu0;
            cmd[8] = g_pActiveSHD;

            depth = depth >> 4;
            if (*(short *)(g_pLisaActiveSubmesh + 0x1e) > 99) {
                depth -= 0x4c;
                if (depth < 0) depth = 0;
            }
            if (depth > 5999) depth = 5999;

            g_pLisaDrawCommandWritePtr[2] = (int)(cmd + 9);
            pwrite = (int **)g_pLisaDrawCommandWritePtr;
            buckets = (int **)g_pLisaDepthBuckets;

            g_pLisaDrawCommandWritePtr = (int *)(pwrite + 2);
            pwrite[1] = buckets[depth];
            buckets[depth] = (int *)pwrite;
        }
    }

    g_pLisaSubmeshPolygon = (int *)((char *)g_pLisaSubmeshPolygon + 0x14);
}

/**
 * @original Lisa_DrawBillboard_Op07 (MAINDOS_32BIT.EXE @ 0x0044d0f0, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_DrawBillboard_Op07(void) {
    int *poly = (int *)g_pLisaSubmeshPolygon;
    int *verts = (int *)g_LisaTransformedVertices;
    int v_idx = poly[1];
    int z = verts[v_idx * 3 + 2];

    if (z > 200) {
        int *cmd = (int *)*g_pLisaDrawCommandWritePtr;
        int x = verts[v_idx * 3 + 0];
        int y = verts[v_idx * 3 + 1];
        int u0 = poly[2];
        int v0 = poly[3];
        int u1 = poly[4];
        int v1 = poly[5];
        int tex_offset = poly[6];
        float scale = 4.0f / (float)z;
        int depth;
        int **pwrite;
        int **buckets;

        cmd[0] = 7;
        cmd[1] = (int)(cmd + 5);
        cmd[2] = (int)(cmd + 13);
        cmd[3] = x;
        cmd[4] = y;
        cmd[5] = (u0 + u1) / 2 - u0;
        cmd[6] = (v0 + v1) / 2 - v0;
        cmd[7] = u0;
        cmd[8] = v0;
        cmd[9] = u1;
        cmd[10] = v1;
        cmd[11] = *(int *)(g_pLisaActiveMipTable - 5) + tex_offset;
        cmd[12] = g_pActiveSHD;
        cmd[13] = (int)(scale * (float)poly[7]);
        cmd[14] = 0;
        cmd[15] = 0;
        cmd[16] = (int)(scale * (float)poly[8]);

        depth = (int)((double)z * 2.8) >> 4;
        if (*(short *)(g_pLisaActiveSubmesh + 0x1e) > 99) {
            depth -= 0x5c;
            if (depth < 0) depth = 0;
        }
        if (depth > 5999) depth = 5999;

        g_pLisaDrawCommandWritePtr[2] = (int)(cmd + 17);
        pwrite = (int **)g_pLisaDrawCommandWritePtr;
        buckets = (int **)g_pLisaDepthBuckets;

        g_pLisaDrawCommandWritePtr = (int *)(pwrite + 2);
        pwrite[1] = buckets[depth];
        buckets[depth] = (int *)pwrite;
    }

    g_pLisaSubmeshPolygon = (int *)((char *)poly + 0x24);
}

/**
 * @original Lisa_DrawBillboard_Op08 (MAINDOS_32BIT.EXE @ 0x0044d230, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_DrawBillboard_Op08(void) {
    int *poly = (int *)g_pLisaSubmeshPolygon;
    int *verts = (int *)g_LisaTransformedVertices;
    int v_idx = poly[1];
    int z = verts[v_idx * 3 + 2];

    if (z > 200) {
        int *cmd = (int *)*g_pLisaDrawCommandWritePtr;
        int x = verts[v_idx * 3 + 0] & ~0xff;
        int y = verts[v_idx * 3 + 1] & ~0xff;
        int u0 = poly[2];
        int v0 = poly[3];
        int u1 = poly[4];
        int v1 = poly[5];
        int tex_offset = poly[6];
        int depth;
        int **pwrite;
        int **buckets;

        cmd[0] = 7;
        cmd[1] = (int)(cmd + 5);
        cmd[2] = (int)(cmd + 13);
        cmd[3] = x;
        cmd[4] = y;
        cmd[5] = (u0 + u1) / 2 - u0;
        cmd[6] = (v0 + v1) / 2 - v0;
        cmd[7] = u0;
        cmd[8] = v0;
        cmd[9] = u1;
        cmd[10] = v1;
        cmd[11] = *(int *)(g_pLisaActiveMipTable - 5) + tex_offset;
        cmd[12] = (int)g_pLisaTransparencyLUT;
        cmd[13] = (int)((double)poly[7] * 0.0039525693 * (double)g_LisaCamera->viewport_y);
        cmd[14] = 0;
        cmd[15] = 0;
        cmd[16] = (int)((double)poly[8] * 0.004820206304829848 * (double)g_LisaCamera->viewport_y);

        depth = (int)((double)z * 2.8) >> 4;
        if (*(short *)(g_pLisaActiveSubmesh + 0x1e) > 99) {
            depth -= 0x5c;
            if (depth < 0) depth = 0;
        }
        if (depth > 5999) depth = 5999;

        g_pLisaDrawCommandWritePtr[2] = (int)(cmd + 17);
        pwrite = (int **)g_pLisaDrawCommandWritePtr;
        buckets = (int **)g_pLisaDepthBuckets;

        g_pLisaDrawCommandWritePtr = (int *)(pwrite + 2);
        pwrite[1] = buckets[depth];
        buckets[depth] = (int *)pwrite;
    }

    g_pLisaSubmeshPolygon = (int *)((char *)poly + 0x24);
}

/**
 * @original Lisa_DrawTexturedTriangle_Op15 (MAINDOS_32BIT.EXE @ 0x0044d550, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_DrawTexturedTriangle_Op15(void) {
  int tmp_v2x_buckets;
  int tmp_v1x;
  int v2_z;
  int v1_z;
  int *cmd_node;
  int tmp_v0x_uv;
  int tmp_v0y_uv;
  int tmp_uv;
  int verts_ptr;
  int poly_ptr;
  int tmp_v2y;
  int sum_z;
  int tmp_area_v2uv;
  int poly_u0;
  int poly_v1;
  int poly_u1;
  int poly_v0;
  int poly_u2;
  int *queue_ptr;
  unsigned int area_tmp;
  unsigned int area_sign;
  int v0_idx_miptab;
  int *next_cmd;
  int tmp_v1y_depth;
  int z_offset_ftol;

  

  verts_ptr = g_LisaTransformedVertices;
  g_LisaObjMat_11 = g_LisaTransformedVertices;

  do {
    tmp_v1y_depth = ((MshPolygon*)g_pLisaSubmeshPolygon)->vi0;
    v0_idx_miptab = tmp_v1y_depth * 3;
    g_LisaSubmeshVertexStride = ((MshPolygon*)g_pLisaSubmeshPolygon)->vi1 * 3;
    g_LisaCameraFocalScale = ((MshPolygon*)g_pLisaSubmeshPolygon)->vi2 * 3;
    tmp_v2x_buckets = *(int *)(verts_ptr + 4 + tmp_v1y_depth * 0xc);
    tmp_v1y_depth = *(int *)(verts_ptr + tmp_v1y_depth * 0xc);

    if ((g_SubpixelMaxY - tmp_v2x_buckets | g_SubpixelMaxX - tmp_v1y_depth | tmp_v2x_buckets - g_SubpixelMinY | tmp_v1y_depth - g_SubpixelMinX
        ) < 0) {
      do {
        tmp_v1y_depth = *(int *)(verts_ptr + v0_idx_miptab * 4);
        tmp_v2x_buckets = *(int *)(verts_ptr + g_LisaSubmeshVertexStride * 4);
        g_LisaCameraMatrix_Y = tmp_v1y_depth;

        if (tmp_v1y_depth <= tmp_v2x_buckets) {
          g_LisaCameraMatrix_Y = tmp_v2x_buckets;
        }

        tmp_v1x = *(int *)(verts_ptr + g_LisaCameraFocalScale * 4);
        tmp_v2y = g_LisaCameraMatrix_Y;

        if (g_LisaCameraMatrix_Y <= tmp_v1x) {
          tmp_v2y = tmp_v1x;
        }

        if (g_SubpixelMinX <= tmp_v2y) {
          g_LisaCameraMatrix_Y = tmp_v1y_depth;

          if (tmp_v2x_buckets <= tmp_v1y_depth) {
            g_LisaCameraMatrix_Y = tmp_v2x_buckets;
          }

          tmp_v1y_depth = g_LisaCameraMatrix_Y;

          if (tmp_v1x <= g_LisaCameraMatrix_Y) {
            tmp_v1y_depth = tmp_v1x;
          }

          if (tmp_v1y_depth <= g_SubpixelMaxX) {
            tmp_v1y_depth = *(int *)(verts_ptr + 4 + v0_idx_miptab * 4);
            tmp_v2x_buckets = *(int *)(verts_ptr + 4 + g_LisaSubmeshVertexStride * 4);
            g_LisaCameraMatrix_Y = tmp_v1y_depth;

            if (tmp_v1y_depth <= tmp_v2x_buckets) {
              g_LisaCameraMatrix_Y = tmp_v2x_buckets;
            }

            tmp_v1x = *(int *)(verts_ptr + 4 + g_LisaCameraFocalScale * 4);
            tmp_v2y = g_LisaCameraMatrix_Y;

            if (g_LisaCameraMatrix_Y <= tmp_v1x) {
              tmp_v2y = tmp_v1x;
            }

            if (g_SubpixelMinY <= tmp_v2y) {
              g_LisaCameraMatrix_Y = tmp_v1y_depth;

              if (tmp_v2x_buckets <= tmp_v1y_depth) {
                g_LisaCameraMatrix_Y = tmp_v2x_buckets;
              }

              tmp_v1y_depth = g_LisaCameraMatrix_Y;

              if (tmp_v1x <= g_LisaCameraMatrix_Y) {
                tmp_v1y_depth = tmp_v1x;
              }

              if (tmp_v1y_depth <= g_SubpixelMaxY) break;
            }

          }

        }

        tmp_v1y_depth = g_pLisaSubmeshPolygon + 0x2c;

        if (*(char *)(g_pLisaSubmeshPolygon + 0x2c) != '\x15') {
          g_LisaObjMat_Tmp6 = v0_idx_miptab;
          g_pLisaSubmeshPolygon = tmp_v1y_depth;
          return;
        }

        if (g_LisaSubmeshPolyCount < 3) {
          g_LisaObjMat_Tmp6 = v0_idx_miptab;
          g_pLisaSubmeshPolygon = tmp_v1y_depth;
          return;
        }

        g_LisaSubmeshPolyCount = g_LisaSubmeshPolyCount + -1;
        v0_idx_miptab = *(int *)(g_pLisaSubmeshPolygon + 0x30) * 3;
        g_LisaCameraFocalScale = *(int *)(g_pLisaSubmeshPolygon + 0x38) * 3;
        g_LisaSubmeshVertexStride = *(int *)(g_pLisaSubmeshPolygon + 0x34) * 3;
        g_pLisaSubmeshPolygon = tmp_v1y_depth;
      } while( 1 );
    }

    poly_ptr = g_pLisaSubmeshPolygon;
    tmp_v1y_depth = *(int *)(verts_ptr + 4 + g_LisaSubmeshVertexStride * 4);
    tmp_v2x_buckets = *(int *)(verts_ptr + g_LisaCameraFocalScale * 4);
    tmp_v1x = *(int *)(verts_ptr + g_LisaSubmeshVertexStride * 4);
    tmp_v2y = *(int *)(verts_ptr + 4 + g_LisaCameraFocalScale * 4);

    g_LisaSubmeshCenterWorldY = ((tmp_v1y_depth >> 4) - (*(int *)(verts_ptr + 4 + v0_idx_miptab * 4) >> 4)) *

                   ((tmp_v2x_buckets >> 4) - (tmp_v1x >> 4)) +

                   ((tmp_v2y >> 4) - (tmp_v1y_depth >> 4)) *

                   ((*(int *)(verts_ptr + v0_idx_miptab * 4) >> 4) - (tmp_v1x >> 4)) ^ g_LisaBackfaceSign;
    v2_z = *(int *)(verts_ptr + 8 + g_LisaCameraFocalScale * 4);
    v1_z = *(int *)(verts_ptr + 8 + g_LisaSubmeshVertexStride * 4);
    sum_z = v2_z + v1_z + *(int *)(verts_ptr + 8 + v0_idx_miptab * 4);
    g_LisaObjMat_Tmp6 = v0_idx_miptab;

    if ((600 < sum_z) && (0 < (int)g_LisaSubmeshCenterWorldY)) {
      g_LisaCameraMatrix_X = sum_z >> 4;
      cmd_node = (int *)*g_pLisaDrawCommandWritePtr;

      if (g_LisaShadingEnabled == 1) {
        poly_v0 = *(int *)(verts_ptr + v0_idx_miptab * 4) >> 8;
        tmp_area_v2uv = *(int *)(verts_ptr + 4 + v0_idx_miptab * 4) >> 8;

        tmp_area_v2uv = (((tmp_v1y_depth >> 8) + tmp_area_v2uv) * ((tmp_v1x >> 8) - poly_v0) +

                  ((tmp_v2y >> 8) + tmp_area_v2uv) * (poly_v0 - (tmp_v2x_buckets >> 8)) +

                 ((tmp_v1y_depth >> 8) + (tmp_v2y >> 8)) * ((tmp_v2x_buckets >> 8) - (tmp_v1x >> 8))) * 3;
        area_tmp = tmp_area_v2uv >> 0x1f;
        g_LisaActiveLightingMode = (tmp_area_v2uv >> 1 ^ area_tmp) - area_tmp;
        tmp_area_v2uv = ((MshPolygon*)g_pLisaSubmeshPolygon)->tv2 >> 8;
        poly_v0 = ((MshPolygon*)g_pLisaSubmeshPolygon)->tv0 >> 8;
        poly_u2 = ((MshPolygon*)g_pLisaSubmeshPolygon)->tu2 >> 8;
        poly_u0 = ((MshPolygon*)g_pLisaSubmeshPolygon)->tu0 >> 8;
        poly_v1 = ((MshPolygon*)g_pLisaSubmeshPolygon)->tv1 >> 8;
        poly_u1 = ((MshPolygon*)g_pLisaSubmeshPolygon)->tu1 >> 8;

        area_tmp = (poly_v1 + poly_v0) * (poly_u1 - poly_u0) +

                 (poly_v0 + tmp_area_v2uv) * (poly_u0 - poly_u2) + (poly_v1 + tmp_area_v2uv) * (poly_u2 - poly_u1);
        area_sign = (int)area_tmp >> 0x1f;
        tmp_area_v2uv = (area_tmp ^ area_sign) - area_sign;
        if (tmp_area_v2uv < g_LisaActiveLightingMode) goto LAB_0044d8ef;
        g_LisaSubmeshBoundRadius = (g_LisaActiveLightingMode * 4 <= tmp_area_v2uv) - 4;
      }

      else {
LAB_0044d8ef:

        g_LisaSubmeshBoundRadius = -5;
      }

      if (((g_LisaCamera->shading_mode == 0) ||

          (g_LisaObjMat_20 = (int)*(short *)(g_pLisaActiveSubmesh + 0x1e), 0x2d0 < g_LisaCameraMatrix_X)) ||

         (((99 < g_LisaObjMat_20 && (g_LisaObjMat_20 != 200)) &&

          ((g_LisaObjMat_20 < 300 || (0x12e < g_LisaObjMat_20)))))) {
        tmp_v0x_uv = *(int *)(verts_ptr + v0_idx_miptab * 4);
        tmp_v0y_uv = *(int *)(verts_ptr + 4 + v0_idx_miptab * 4);
        g_LisaSubmeshTmp5 = cmd_node;
        *cmd_node = 0x11;
        cmd_node[1] = tmp_v0x_uv;
        cmd_node[2] = tmp_v0y_uv;
        cmd_node[3] = tmp_v1x;
        v0_idx_miptab = g_pLisaActiveMipTable;
        cmd_node[4] = tmp_v1y_depth;
        cmd_node[5] = tmp_v2x_buckets;
        tmp_v1y_depth = g_LisaSubmeshBoundRadius;
        cmd_node[6] = tmp_v2y;
        cmd_node[7] = g_pLisaSubmeshPolygon + 0x10;
        queue_ptr = g_pLisaDrawCommandWritePtr;
        next_cmd = cmd_node + 9;
        cmd_node[8] = *(int *)(v0_idx_miptab + tmp_v1y_depth * 4) + ((MshPolygon*)g_pLisaSubmeshPolygon)->extra;
      }

      else {
        if (g_LisaCameraMatrix_X < 0x1e1) {
          cmd_node[4] = *(int *)(verts_ptr + 8 + v0_idx_miptab * 4);
          cmd_node[7] = v1_z;
          cmd_node[10] = v2_z;
        }

        else {
          g_LisaSubmeshFlags = sum_z / 3;
          g_LisaSubmeshTmp4 = (float)(0x2d0 - g_LisaCameraMatrix_X) * g_Const_512;
          z_offset_ftol = __ftol();
          cmd_node[4] = (int)z_offset_ftol + g_LisaSubmeshFlags;
          z_offset_ftol = __ftol();
          cmd_node[7] = (int)z_offset_ftol + g_LisaSubmeshFlags;
          g_LisaSubmeshTmp1 = v2_z;
          z_offset_ftol = __ftol();
          cmd_node[10] = (int)z_offset_ftol + g_LisaSubmeshFlags;
        }

        tmp_v0x_uv = *(int *)(verts_ptr + v0_idx_miptab * 4);
        tmp_v0y_uv = *(int *)(verts_ptr + 4 + v0_idx_miptab * 4);
        g_LisaSubmeshTmp5 = cmd_node;
        *cmd_node = 0x14;
        cmd_node[2] = tmp_v0x_uv;
        cmd_node[3] = tmp_v0y_uv;
        cmd_node[5] = tmp_v1x;
        cmd_node[6] = tmp_v1y_depth;
        tmp_v0x_uv = ((MshPolygon*)poly_ptr)->tu0;
        cmd_node[8] = tmp_v2x_buckets;
        tmp_v0y_uv = ((MshPolygon*)poly_ptr)->tv0;
        cmd_node[9] = tmp_v2y;
        tmp_uv = ((MshPolygon*)poly_ptr)->tu1;
        cmd_node[0xb] = tmp_v0x_uv;
        tmp_v0x_uv = ((MshPolygon*)poly_ptr)->tv1;
        cmd_node[0xc] = tmp_v0y_uv;
        tmp_v0y_uv = ((MshPolygon*)poly_ptr)->tu2;
        cmd_node[0xd] = tmp_uv;
        tmp_uv = ((MshPolygon*)poly_ptr)->tv2;
        cmd_node[0xe] = tmp_v0x_uv;
        cmd_node[0xf] = tmp_v0y_uv;
        cmd_node[0x10] = tmp_uv;
        queue_ptr = g_pLisaDrawCommandWritePtr;
        next_cmd = cmd_node + 0x11;
        cmd_node[1] = *(int *)(g_pLisaActiveMipTable + g_LisaSubmeshBoundRadius * 4) + ((MshPolygon*)poly_ptr)->extra;
      }

      tmp_v1y_depth = g_pLisaActiveSubmesh;
      g_LisaCameraMatrix_X = g_LisaCameraMatrix_X + -0x50;
      queue_ptr[2] = next_cmd;
      tmp_v2x_buckets = g_pLisaDepthBuckets;
      cmd_node = g_pLisaDrawCommandWritePtr;

      if (99 < *(short *)(tmp_v1y_depth + 0x1e)) {
        if (*(short *)(tmp_v1y_depth + 0x1e) == 0xd2) {
          tmp_v1y_depth = -0x54;
        }

        else {
          tmp_v1y_depth = -0x5c;
        }

        g_LisaCameraMatrix_X = g_LisaCameraMatrix_X + tmp_v1y_depth;
      }

      if (g_LisaCameraMatrix_X < 0) {
        g_LisaCameraMatrix_X = 0;
      }

      if (5999 < g_LisaCameraMatrix_X) {
        g_LisaCameraMatrix_X = 5999;
      }

      tmp_v1y_depth = g_LisaCameraMatrix_X;
      next_cmd = g_pLisaDrawCommandWritePtr + 1;
      g_pLisaDrawCommandWritePtr = queue_ptr + 2;
      *next_cmd = *(int *)(g_pLisaDepthBuckets + g_LisaCameraMatrix_X * 4);
      *(int **)(tmp_v2x_buckets + tmp_v1y_depth * 4) = cmd_node;
    }

    g_pLisaSubmeshPolygon = poly_ptr + 0x2c;

    if ((*(char *)(poly_ptr + 0x2c) != '\x15') || (g_LisaSubmeshPolyCount < 3)) {
      return;
    }

    g_LisaSubmeshPolyCount = g_LisaSubmeshPolyCount + -1;
  } while( 1 );
}

/**
 * @original Lisa_DrawTexturedTriangle_Op15_Sub (MAINDOS_32BIT.EXE @ 0x0044e900, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_DrawTexturedTriangle_Op15_Sub(void) {
    const int32_t *verts = (const int32_t *)g_LisaTransformedVertices;
    uint8_t *prim = (uint8_t *)g_pLisaSubmeshPolygon;
    int count = g_LisaSubmeshPolyCount;
    int v0_idx, v1_idx, v2_idx;
    const int32_t *v0, *v1, *v2;
    int32_t v0_x, v0_y, v0_z;
    int32_t v1_x, v1_y, v1_z;
    int32_t v2_x, v2_y, v2_z;
    int max_x, min_x, max_y, min_y;
    int32_t cross, sum_z, depth;
    int32_t a0, a1, a2, a3, a4, a5, screen_area;
    int32_t u0, v0_uv, u1, v1_uv, u2, v2_uv, uv_area;
    int mip;
    uint32_t *node;
    uint32_t **next_queue;
    uint32_t *head;
    int32_t tex_idx, shift_u, shift_v;
    MshPolygon *poly;
    short submesh_flags;

    while (count > 0 && *prim == 0x11) {
        poly = (MshPolygon *)prim;
        v0_idx = poly->vi0 * 3;
        v1_idx = poly->vi1 * 3;
        v2_idx = poly->vi2 * 3;
        
        v0 = &verts[v0_idx];
        v1 = &verts[v1_idx];
        v2 = &verts[v2_idx];
        
        v0_x = v0[0]; v0_y = v0[1]; v0_z = v0[2];
        v1_x = v1[0]; v1_y = v1[1]; v1_z = v1[2];
        v2_x = v2[0]; v2_y = v2[1]; v2_z = v2[2];

        /* Screen boundary clipping check */
        if ((g_SubpixelMaxY - v0_y | g_SubpixelMaxX - v0_x | v0_y - g_SubpixelMinY | v0_x - g_SubpixelMinX) < 0) {
            max_x = v0_x > v1_x ? v0_x : v1_x;
            if (v2_x > max_x) max_x = v2_x;
            min_x = v0_x < v1_x ? v0_x : v1_x;
            if (v2_x < min_x) min_x = v2_x;
            max_y = v0_y > v1_y ? v0_y : v1_y;
            if (v2_y > max_y) max_y = v2_y;
            min_y = v0_y < v1_y ? v0_y : v1_y;
            if (v2_y < min_y) min_y = v2_y;

            if (max_x < g_SubpixelMinX || min_x > g_SubpixelMaxX ||
                max_y < g_SubpixelMinY || min_y > g_SubpixelMaxY) {
                prim += 0x2C;
                count--;
                continue;
            }
        }

        /* Backface culling: 2D cross product */
        cross = ((v0_y >> 4) - (v1_y >> 4)) * ((v2_x >> 4) - (v1_x >> 4)) +
                ((v1_y >> 4) - (v2_y >> 4)) * ((v2_x >> 4) - (v1_x >> 4));
        cross ^= g_LisaBackfaceSign;
        sum_z = v0_z + v1_z + v2_z;
        
        if (sum_z > 600 && cross > 0) {
            depth = sum_z >> 4;
            node = (uint32_t *)*g_pLisaDrawCommandWritePtr;

            if (g_LisaShadingEnabled == 1) {
                a0 = (v0_y >> 8) + (v2_y >> 8);
                a1 = (v0_x >> 8) - (v2_x >> 8);
                a2 = (v1_y >> 8) + (v2_y >> 8);
                a3 = (v2_x >> 8) - (v1_x >> 8);
                a4 = (v1_y >> 8) + (v0_y >> 8);
                a5 = (v1_x >> 8) - (v0_x >> 8);
                screen_area = (a0 * a1 + a2 * a3 + a4 * a5) * 3;
                if (screen_area < 0) screen_area = -screen_area;
                screen_area >>= 1;

                u0 = poly->tu0 >> 8;
                v0_uv = poly->tv0 >> 8;
                u1 = poly->tu1 >> 8;
                v1_uv = poly->tv1 >> 8;
                u2 = poly->tu2 >> 8;
                v2_uv = poly->tv2 >> 8;

                uv_area = (v0_uv + v2_uv) * (u0 - u2) +
                          (v1_uv + v2_uv) * (u2 - u1) +
                          (v1_uv + v0_uv) * (u1 - u0);
                if (uv_area < 0) uv_area = -uv_area;
                mip = -4;
                if (uv_area < screen_area) {
                    mip = -5;
                } else if (screen_area * 4 <= uv_area) {
                    mip = (screen_area * 16 <= uv_area) ? -2 : -3;
                }
            } else {
                mip = -4;
            }

            submesh_flags = *(short *)(g_pLisaActiveSubmesh + 0x1E);

            if (depth < 721 && ((submesh_flags < 100 || submesh_flags == 200) || (submesh_flags > 299 && submesh_flags < 303))) {
                if (depth < 481) {
                    node[4] = v0_z;
                    node[7] = v1_z;
                    node[10] = v2_z;
                } else {
                    g_LisaCurrentVertexIndex = sum_z / 3;
                    node[4] = (uint32_t)((720.0f - (float)depth) * 512.0f) + g_LisaCurrentVertexIndex;
                    node[7] = (uint32_t)((720.0f - (float)depth) * 512.0f) + g_LisaCurrentVertexIndex;
                    node[10] = (uint32_t)((720.0f - (float)depth) * 512.0f) + g_LisaCurrentVertexIndex;
                }

                node[0] = 0x14;
                node[2] = v0_x;
                node[3] = v0_y;
                node[5] = v1_x;
                node[6] = v1_y;
                node[8] = v2_x;
                node[9] = v2_y;

                if (mip == -5 && g_pLisaActiveMipTable[-5] != g_pLisaActiveMipTable[0]) {
                    node[11] = (poly->tu0 & 0x3fff) << 2;
                    node[12] = (poly->tv0 & 0x3fff) << 2;
                    node[13] = (poly->tu1 & 0x3fff) << 2;
                    node[14] = (poly->tv1 & 0x3fff) << 2;
                    node[15] = (poly->tu2 & 0x3fff) << 2;
                    node[16] = (poly->tv2 & 0x3fff) << 2;
                    shift_u = poly->tu0 >> 14;
                    shift_v = poly->tv0 >> 14;
                    tex_idx = poly->extra;
                    node[1] = g_pLisaActiveMipTable[shift_u * 4] + (tex_idx + shift_v * 0x4000) * 4;
                } else {
                    node[11] = poly->tu0;
                    node[12] = poly->tv0;
                    node[13] = poly->tu1;
                    node[14] = poly->tv1;
                    node[15] = poly->tu2;
                    node[16] = poly->tv2;
                    node[1] = g_pLisaActiveMipTable[mip] + poly->extra;
                }
                
                next_queue = (uint32_t **)node + 17;
            } else {
                node[1] = v0_x;
                node[2] = v0_y;
                node[3] = v1_x;
                node[4] = v1_y;
                node[5] = v2_x;
                node[6] = v2_y;
                node[7] = (uint32_t)(uintptr_t)(&poly->tu0);

                if (mip == -5 && g_pLisaActiveMipTable[-5] != g_pLisaActiveMipTable[0]) {
                    node[0] = 0x16;
                    shift_u = poly->tu0 >> 14;
                    shift_v = poly->tv0 >> 14;
                    tex_idx = poly->extra;
                    node[8] = g_pLisaActiveMipTable[shift_u * 4] + (tex_idx + shift_v * 0x4000) * 4;
                } else {
                    node[0] = 0x11;
                    node[8] = g_pLisaActiveMipTable[mip] + poly->extra;
                }
                
                next_queue = (uint32_t **)node + 9;
            }

            if (submesh_flags > 99) {
                if (submesh_flags == 210) {
                    depth -= 164;
                } else {
                    depth -= 172;
                }
            } else {
                depth -= 80;
            }

            if (depth < 0) depth = 0;
            if (depth > 5999) depth = 5999;

            head = (uint32_t *)g_pLisaDepthBuckets[depth];
            *next_queue = head;
            g_pLisaDepthBuckets[depth] = (uint32_t *)node;
            g_pLisaDrawCommandWritePtr = (int **)next_queue + 1;
        }

        prim += 0x2C;
        count--;
    }

    g_pLisaSubmeshPolygon = (int *)prim;
    g_LisaSubmeshPolyCount = count;
}

/**
 * @original Lisa_InitRasterizerTables (MAINDOS_32BIT.EXE @ 0x0044f070, lisa3d.c)
 * @fidelity ADAPTED
 */
LisaReturn64 Lisa_InitRasterizerTables(int screen_pitch,unsigned int flags) {
  int pitch_increment;
  short row_counter;
  int tmp_ebx;
  unsigned int idx;
  int accum_pitch;
  int tmp_esi;
  int *table_ptr;
  char **temp_uint_ptr_ptr;

  

  if ((pitch_increment < 0x579) && (tmp_ebx < 0x259)) {
    table_ptr = &g_LisaActiveTextureID;
    idx = 1;
    g_LisaScanlinePitch = pitch_increment;
    g_LisaAspectScale = tmp_ebx;

    do {
      *table_ptr = (int)(0x10000 / (unsigned int)idx) + -1;
      table_ptr = table_ptr + 1;
      idx = idx + 1;
    } while (idx != 0x3a9b);

    for (temp_uint_ptr_ptr = (char **)g_pLisaShutdownCallbacks; *temp_uint_ptr_ptr != (char *)0x0; temp_uint_ptr_ptr = temp_uint_ptr_ptr + 1) {
      (*(void (*)())*temp_uint_ptr_ptr)(temp_uint_ptr_ptr,tmp_esi,tmp_ebx);
    }

    table_ptr = &g_LisaScreenPitch;
    accum_pitch = 0;
    row_counter = 600;

    do {
      *table_ptr = accum_pitch;
      table_ptr = table_ptr + 1;
      accum_pitch = accum_pitch + pitch_increment;
      row_counter = row_counter + -1;
    } while (row_counter != 0);
    { LisaReturn64 _r; _r.edx = flags; _r.eax = 0; return _r; }
  }

  { LisaReturn64 _r; _r.edx = flags; _r.eax = 0xffffffff; return _r; }
}

/**
 * @original Lisa_ExecuteRasterizerCommands (MAINDOS_32BIT.EXE @ 0x0044f0e9, lisa3d.c)
 * @fidelity ADAPTED
 */
LisaReturn64 Lisa_ExecuteRasterizerCommands(int mode,unsigned int flags) {
  int *tmp_esi;
  LisaReturn64 ret_val;

  

  g_pLisaScanlineBuffer = tmp_esi;
  g_pLisaSpanBuffer = (int *)*tmp_esi;
  g_LisaCameraZoom = tmp_esi[1];
  g_pLisaActiveShading = tmp_esi[3];
  g_LisaClipLeft = tmp_esi[4];
  g_LisaClipRight = tmp_esi[5];
  g_LisaClipSubpixelLeft = g_LisaClipLeft;
  g_LisaViewportCenterX = g_LisaClipLeft << 8;
  g_LisaClipSubpixelRight = g_LisaClipRight;
  g_LisaViewportCenterY = g_LisaClipRight << 8;
  g_SubpixelMinX = g_LisaViewportCenterX;
  g_SubpixelMinY = g_LisaViewportCenterY;
  g_LisaClipTop = tmp_esi[6];
  g_LisaClipBottom = tmp_esi[7];
  g_LisaClipSubpixelTop = g_LisaClipTop + 1;
  g_LisaClipSubpixelBottom = g_LisaClipBottom + 1;
  g_SubpixelMaxX = g_LisaClipSubpixelTop * 0x100;
  g_LisaViewportWidth = g_SubpixelMaxX + -1;
  g_SubpixelMaxY = g_LisaClipSubpixelBottom * 0x100;
  g_LisaViewportHeight = g_SubpixelMaxY + -1;
  g_pLisaEdgeBuffer = (int *)*g_pLisaSpanBuffer;

  if (g_pLisaEdgeBuffer != (int *)0x0) {
    ret_val = (*(LisaReturn64 (*)())(((void **)g_LisaRasterizerJmpTable)[*g_pLisaEdgeBuffer]))();
    return ret_val;
  }

  { LisaReturn64 _r; _r.edx = flags; _r.eax = 0; return _r; }
}


/**
 * @original Car_UnpackMeshGeometry (MAINDOS_32BIT.EXE @ 0x0041d190, lisa3d.c)
 * @fidelity STUB
 */
void Car_UnpackMeshGeometry(void) {}

/**
 * @original Lisa_DrawPolygon_Op12 (MAINDOS_32BIT.EXE @ 0x0044caa0, lisa3d.c)
 * @fidelity STUB
 */
void Lisa_DrawPolygon_Op12(void) {}

/**
 * @original Lisa_DrawPolygon_Op13 (MAINDOS_32BIT.EXE @ 0x0044cac0, lisa3d.c)
 * @fidelity STUB
 */
void Lisa_DrawPolygon_Op13(void) {}

/**
 * @original Lisa_DrawPolygon_Op16 (MAINDOS_32BIT.EXE @ 0x0044cae0, lisa3d.c)
 * @fidelity STUB
 */
void Lisa_DrawPolygon_Op16(void) {}

/**
 * @original Lisa_DrawPolygon_Op17 (MAINDOS_32BIT.EXE @ 0x0044cb00, lisa3d.c)
 * @fidelity STUB
 */
void Lisa_DrawPolygon_Op17(void) {}

/**
 * @original Lisa_RenderTexturedTriangle_Op11 (MAINDOS_32BIT.EXE @ 0x00452800, lisa3d.c)
 * @fidelity STUB
 */
void Lisa_RenderTexturedTriangle_Op11(void) {}

/**
 * @original Lisa_DrawTexturedSpan_Op11 (MAINDOS_32BIT.EXE @ 0x004537dc, lisa3d.c)
 * @fidelity STUB
 */
void Lisa_DrawTexturedSpan_Op11(void) {}

/**
 * @original Video_SetPalette (MAINDOS_32BIT.EXE @ 0x00456c40, lisa3d.c)
 * @fidelity STUB
 */
void Video_SetPalette(void) {}
