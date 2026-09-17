typedef unsigned char byte;
/*
 * lisa3d.c - Lisa 2 3D Rasterizer, Panorama Sky Renderer & CDP Animation System
 * Original file: lisa3d.c
 * Target: MAINDOS.EXE / IGN_WIN.EXE (Watcom C/C++ 10.6, 32-bit flat protected mode)
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

/* Global active panorama backdrop buffer (64KB, 256x256 8bpp) */
uint8_t *g_pActivePAN = NULL;
uint8_t *g_pLisaTransparencyLUT = NULL;
uint8_t *g_pVirtualFramebuffer = NULL;
LisaCamera *g_LisaCamera = NULL;
int g_ScreenWidth = 640;
int g_ScreenHeight = 480;
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

/**
 * @original Lisa_DrawTexturedTriangle_Op11_Unshaded (IGN_WIN.EXE @ 0x0044dc60, lisa3d.c)
 * @fidelity EXACT
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
 * @original Lisa_DrawTexturedTriangle_Op11_Shaded (IGN_WIN.EXE @ 0x0044e1b0, lisa3d.c)
 * @fidelity EXACT
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
int Lisa_InitSpatialGrid(int param_1,int param_2,size_t param_3);
long long Lisa_CreateDynamicObject(int param_1,int param_2,int *param_3,int *param_4,int param_5, short param_6,short param_7,short param_8,short param_9);
int Lisa_MoveDynamicObject(int *param_1);
int Lisa_UpdateObjectSpatialGrid(int *param_1);
long long Lisa_SetDynamicObjectMesh(int param_1,int param_2,int *param_3,int *param_4,int param_5, short param_6,short param_7,short param_8,short param_9);
int Lisa_DeleteDynamicObject(int *param_1);
unsigned long long Lisa_SetCameraViewport(void);
int FUN_00447280(unsigned int *param_1,int param_2,int param_3,int param_4,int param_5,int param_6 ,int param_7,int param_8,char *param_9);
void Lisa_GenerateTextureSpanTable(int param_1,int param_2,int param_3,int param_4,int *param_5);
void Lisa_DownsampleTextureMipmap(byte *param_1,byte *param_2,int param_3,int param_4,int param_5,int param_6);
void Lisa_FilterTextureBlock(byte *param_1,int param_2,int param_3,int param_4,int param_5,int param_6, int param_7,int param_8);
void Lisa_LoadOrCreateShadingTable(int param_1,int param_2);
unsigned int Lisa_FindClosestPaletteColor(int *param_1,int param_2);
unsigned long long Lisa_RenderSkyBackdrop(void);
int Lisa_CullObjectsOrthographic(void);
int Lisa_FrustumCullObjects(void);
int Lisa_CullObjects(void);
void Lisa_TransformVertices(void);
int Lisa_TransformVerticesPanorama(void);
int Lisa_ComputeObjectMatrix(int param_1,int param_2,int param_3,int param_4,int *param_5);
int FUN_0044ae20(int param_1,int param_2,int param_3,int param_4,int *param_5);
void Lisa_ComputeCameraRotationMatrix(int *param_1);
void Lisa_InitOpcodeTable(void);
int FUN_0044b570(void);
void FUN_0044b770(void);
void FUN_0044b980(void);
int Lisa_RenderSubmeshes(void);
void Lisa_DrawTriangle_OpcodeHelper(int param_1,int param_2);
void FUN_0044cf00(void);
void FUN_0044d0f0(void);
void FUN_0044d230(void);
void Lisa_DrawTexturedTriangle_Op15(void);
void Lisa_DrawTexturedTriangle_Op15_Sub(void);
long long Lisa_InitRasterizerTables(int param_1,unsigned int param_2);
long long Lisa_ExecuteRasterizerCommands(int param_1,unsigned int param_2);

/* --- Automatically extracted globals for Lisa 3D rendering pipeline --- */

extern int *DAT_0050dddc;
extern int *DAT_004cdc84;
extern int *g_pLisaActiveMipTable;
extern int *DAT_0063b600;
extern int *DAT_0063c5e8;
extern void *g_LisaOpcodeTable[];
extern void *PTR_DAT_004abe10[];
extern void *PTR_FUN_004abe6c[];
extern void *PTR_Lisa_DrawTexturedTriangle_Op15_0049c934;
extern void *PTR_LAB_0049c924;
extern void *PTR_DAT_004abe10[];
extern void *PTR_FUN_004abe6c[];
extern void *g_LisaOpcodeTable[];
extern void *PTR_LAB_0049c924;
extern void *PTR_Lisa_DrawTexturedTriangle_Op15_0049c934;
extern int DAT_0047c040;
extern int g_ViewportMinX;
extern int g_ViewportMinY;
extern int g_ViewportMaxX;
extern int g_ViewportMaxY;
extern int DAT_00499fa4;
extern int DAT_00499fa8;
extern int DAT_00499fac;
extern int DAT_0049a058;
extern int DAT_0049a05c;
extern int DAT_0049c9a8;
extern int DAT_0049c9ac;
extern int DAT_0049c9b4;
extern int DAT_0049c9bc;
extern int DAT_0049c9c0;
extern int DAT_0049c9c4;
extern int DAT_0049c9c8;
extern int DAT_0049c9d0;
extern int DAT_0049c9d8;
extern int g_SubpixelMinX;
extern int g_SubpixelMinY;
extern int g_SubpixelMaxX;
extern int g_SubpixelMaxY;
extern int DAT_0049ca3c;
extern int DAT_0049ca44;
extern int DAT_0049d3a8;
extern int DAT_004cdbbc;
extern int DAT_004cdbe8;
extern int DAT_004cdc20;
extern int DAT_004cdc24;
extern int DAT_004cdc38;
extern int DAT_004cdc44;
extern int DAT_004cdc58;
extern int DAT_004cdc60;
extern int DAT_004cdc64;
extern int DAT_004cdc68;
extern int DAT_004cdc6c;
extern int DAT_004cdc70;
extern int DAT_004cdc74;
extern int DAT_004cdc78;
extern int DAT_004cdc7c;
extern int DAT_004cdc80;
extern int DAT_004cdc88;
extern int DAT_004cdca0;
extern int DAT_004cdca4;
extern int DAT_004cdca8;
extern int DAT_004cdce0;
extern int DAT_004cdce4;
extern int DAT_004cdd18;
extern int DAT_004d0000;
extern int DAT_0050dd28;
extern int DAT_0050dd2c;
extern int DAT_0050dd54;
extern int DAT_0050dd64;
extern int DAT_0050dd68;
extern int DAT_0050dd6c;
extern int DAT_0050dd84;
extern int DAT_0050dd88;
extern int DAT_0050dd8c;
extern int DAT_0050dd9c;
extern int DAT_0050dda4;
extern int DAT_0050ddac;
extern int DAT_0050ddb8;
extern int DAT_0050ddc4;
extern int DAT_0050ddc8;
extern int DAT_0050ddcc;
extern int DAT_0050dde0;
extern int DAT_0050ddfc;
extern int DAT_0050de04;
extern int DAT_0063b5ec;
extern int DAT_0063b5f0;
extern int DAT_0063b5f4;
extern int DAT_0063b5f8;
extern int DAT_0063b608;
extern int DAT_0063b60c;
extern int DAT_0063b610;
extern int DAT_0063b614;
extern int DAT_0063c5b0;
extern int DAT_0063c5b8;
extern int g_LisaDrawCommands;
extern int DAT_0063c5c0;
extern int g_LisaVisibleObjects;
extern int DAT_0063c5d0;
extern int DAT_0063c5d4;
extern int DAT_0063c5d8;
extern int DAT_0063c5dc;
extern int DAT_0063c5ec;
extern int DAT_0063c5f4;
extern int g_LisaVisibleSubmeshes;
extern int DAT_0063c600;
extern int DAT_0063c608;
extern int DAT_0063c60c;
extern double _DAT_0047adc0;
extern double _DAT_0047adc8;
extern double _DAT_0047ae48;
extern double _DAT_0047ae50;
extern double _DAT_0047ae58;
extern double _DAT_0047ae64;
extern double _DAT_0047ae68;
extern double _DAT_0047ae70;
extern double _DAT_0047ae78;
extern double _DAT_0047ae88;
extern double _DAT_0047aea0;
extern int *_DAT_0049c9e4;
extern int *_DAT_0049c9e8;
extern int *_DAT_0049c9ec;
extern double _DAT_0049c9f0;
extern double _DAT_0049c9f8;
extern int _DAT_0049c9fc;
extern int _DAT_0049ca00;
extern int _DAT_0049ca04;
extern int _DAT_0049ca08;
extern int _DAT_0049ca0c;
extern int _DAT_0049ca10;
extern int _DAT_0049ca14;
extern int _DAT_0049ca18;
extern int _DAT_0049ca1c;
extern int _DAT_0049ca20;
extern int _DAT_0049ca24;
extern int _DAT_0049ca28;
extern double _DAT_0049ca40;
extern double _DAT_004cdbac;
extern double _DAT_004cdbcc;
extern int _DAT_004cdbdc;
extern double _DAT_004cdbf0;
extern double _DAT_004cdc3c;
extern double _DAT_004cdc4c;
extern double _DAT_004cdc50;
extern double _DAT_004cdc98;
extern double _DAT_004cdc9c;
extern double _DAT_004cdcd0;
extern double _DAT_004cdcd4;
extern double _DAT_004cdcd8;
extern double _DAT_004cdcdc;
extern double _DAT_004cdce8;
extern double _DAT_004cdcec;
extern double _DAT_004cdcf0;
extern double _DAT_004cdcf4;
extern double _DAT_004cdcf8;
extern double _DAT_004cdcfc;
extern double _DAT_004cdd00;
extern double _DAT_004cdd04;
extern double _DAT_004cdd08;
extern double _DAT_004cdd10;
extern double _DAT_0050dd20;
extern double _DAT_0050dd4c;
extern int _DAT_0050dd60;
extern double _DAT_0050dd98;
extern double _DAT_0050dde8;
extern int *_DAT_0050ddf0;
extern int *_DAT_0050ddf4;
extern double _DAT_0063b5e4;
extern double _DAT_0063c5f8;

/**
 * @original Lisa_RenderScene (IGN_WIN.EXE @ 0x004466d0, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_RenderScene(void) {
  int iVar1;
  int *puVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int *piVar6;
  int *puVar7;
  int iVar8;

  

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

  iVar8 = 0;

  if (g_LisaCamera->enable_depth_sort != 0) {
    iVar5 = 5999;
    piVar6 = (int *)(g_pLisaDepthBuckets + 0x5dbc);

    do {
      puVar7 = (int *)*piVar6;

      if (puVar7 != (int *)0x0) {
        *(int *)(g_LisaDrawCommands + iVar8 * 4) = *puVar7;
        iVar8 = iVar8 + 1;

        if (puVar7[1] != 0) {
          puVar2 = (int *)(g_LisaDrawCommands + iVar8 * 4);

          do {
            puVar7 = (int *)puVar7[1];
            iVar8 = iVar8 + 1;
            *puVar2 = *puVar7;
            puVar2 = puVar2 + 1;
          } while (puVar7[1] != 0);
        }

      }

      *piVar6 = 0;
      piVar6 = piVar6 + -1;
      iVar5 = iVar5 + -1;
    } while (-1 < iVar5);
    *(int *)(g_LisaDrawCommands + iVar8 * 4) = 0;
  }

  if (DAT_0049c9bc != 0) {
    if (DAT_0049c9bc < 10) {
      iVar8 = DAT_0063c5b0 / 2;

      if (0 < iVar8) {
        iVar5 = 0x28;

        do {
          iVar3 = rand();
          iVar3 = iVar3 % iVar8;
          iVar1 = *(int *)(&DAT_0063b610 + iVar3 * 8);
          iVar3 = *(int *)(&DAT_0063b614 + iVar3 * 8);
          iVar4 = rand();
          _DAT_0050dd4c = _DAT_0050dd4c + *(char *)((iVar4 << 10) % iVar3 + iVar1);
          iVar4 = rand();
          _DAT_0050dd4c = _DAT_0050dd4c + *(char *)((iVar4 << 10) % iVar3 + iVar1);
          iVar4 = rand();
          _DAT_0050dd4c = _DAT_0050dd4c + *(char *)((iVar4 << 10) % iVar3 + iVar1);
          iVar5 = iVar5 + -1;
        } while (iVar5 != 0);
      }

    }

    else {
      iVar8 = DAT_0063c5b0 / 2;

      if (0 < iVar8) {
        iVar5 = 2;

        do {
          iVar3 = rand();
          iVar3 = iVar3 % iVar8;
          iVar1 = *(int *)(&DAT_0063b610 + iVar3 * 8);
          iVar3 = *(int *)(&DAT_0063b614 + iVar3 * 8);
          iVar4 = rand();
          _DAT_0050dd4c = _DAT_0050dd4c + *(char *)((iVar4 << 10) % iVar3 + iVar1);
          iVar4 = rand();
          _DAT_0050dd4c = _DAT_0050dd4c + *(char *)((iVar4 << 10) % iVar3 + iVar1);
          iVar4 = rand();
          _DAT_0050dd4c = _DAT_0050dd4c + *(char *)((iVar4 << 10) % iVar3 + iVar1);
          iVar5 = iVar5 + -1;
        } while (iVar5 != 0);
      }

    }

    DAT_0049c9bc = DAT_0049c9bc + 1;
  }

  if (DAT_0049c9bc == 100) {
    DAT_0049c9bc = 0;
  }

  return g_LisaDrawCommands;
}

/**
 * @original Lisa_InitEngineMemory (IGN_WIN.EXE @ 0x004468d0, lisa3d.c)
 * @fidelity ADAPTED
 */
int * Lisa_InitEngineMemory(void) {
  void *pvVar1;
  void *pvVar2;
  void *pvVar3;
  void *pvVar4;
  int *puVar5;
  void *pvVar6;
  int iVar7;
  int iVar8;
  double fVar9;

  

  DAT_0063c5d0 = calloc(5000,4);
  DAT_0063c5d4 = 0;
  g_LisaVisibleObjects = calloc(0x5dc,4);
  g_LisaVisibleSubmeshes = calloc(3000,4);
  puVar5 = calloc(0x1838,4);
  iVar7 = DAT_0063c5b0 + 2;
  g_pLisaDepthBuckets = puVar5;
  DAT_0063c5b0 = iVar7;
  *(int **)(&DAT_0063b608 + iVar7 * 4) = puVar5;
  *(int *)(&DAT_0063b60c + iVar7 * 4) = 0x1a90;

  for (iVar7 = 6000; iVar7 != 0; iVar7 = iVar7 + -1) {
    *puVar5 = 0;
    puVar5 = puVar5 + 1;
  }

  g_LisaCamera = (LisaCamera *)calloc(1, sizeof(LisaCamera));
  DAT_0063c5ec = calloc(20000,0xc);
  DAT_0063b600 = calloc(14000,8);
  g_pLisaTextureSheets = calloc(1000,4);
  *(void **)(&DAT_0063b610 + DAT_0063c5b0 * 4) = g_pLisaTextureSheets;
  *(int *)(&DAT_0063b610 + (DAT_0063c5b0 + 1) * 4) = 4000;
  DAT_0063c5b0 = DAT_0063c5b0 + 2;
  g_LisaDrawCommands = DAT_0063c5ec;
  DAT_0063c600 = calloc(150000,4);
  pvVar6 = calloc(0x1fa4,4);
  pvVar3 = g_LisaVisibleSubmeshes;
  pvVar2 = DAT_0063c5d0;
  DAT_0063c5b0 = DAT_0063c5b0 + 2;
  DAT_0063b5f4 = pvVar6;
  *(void **)(&DAT_0063b608 + DAT_0063c5b0 * 4) = pvVar6;
  *(int *)(&DAT_0063b60c + DAT_0063c5b0 * 4) = 0x2a30;
  *(void **)((int)pvVar2 + (DAT_0063c5d4 + 3) * 4 + -0xc) = g_LisaVisibleObjects;
  pvVar1 = DAT_0063b600;
  puVar5 = g_pLisaDepthBuckets;
  *(void **)((int)pvVar2 + (DAT_0063c5d4 + 5) * 4 + -0x10) = pvVar3;
  pvVar3 = DAT_0063c5ec;
  *(int **)((int)pvVar2 + (DAT_0063c5d4 + 5) * 4 + -0xc) = puVar5;
  pvVar4 = DAT_0063c600;
  puVar5 = g_LisaCamera;
  *(int **)((int)pvVar2 + (DAT_0063c5d4 + 7) * 4 + -0x10) = g_LisaCamera;
  iVar7 = DAT_0063c5d4 + 9;
  DAT_0063c5d4 = iVar7;
  *(void **)((int)pvVar2 + iVar7 * 4 + -0x14) = pvVar3;
  *(void **)((int)pvVar2 + iVar7 * 4 + -0x10) = pvVar1;
  pvVar1 = g_pLisaTextureSheets;
  *(void **)((int)pvVar2 + iVar7 * 4 + -0xc) = g_pLisaTextureSheets;
  *(void **)((int)pvVar2 + iVar7 * 4 + -8) = pvVar4;
  *(void **)((int)pvVar2 + iVar7 * 4 + -4) = pvVar6;

  if (puVar5 == (int *)0x0) {
    return (int *)0x0;
  }

  DAT_0063b5f4 = pvVar6;

  if (pvVar3 == (void *)0x0) {
    return (int *)0x0;
  }

  if (DAT_0063b600 == (void *)0x0) {
    return (int *)0x0;
  }

  if (g_LisaVisibleObjects == (void *)0x0) {
    return (int *)0x0;
  }

  if (DAT_0063c600 == (void *)0x0) {
    return (int *)0x0;
  }

  if (g_pLisaDepthBuckets == (int *)0x0) {
    return (int *)0x0;
  }

  if (pvVar6 == (void *)0x0) {
    return (int *)0x0;
  }

  if (pvVar1 == (void *)0x0) {
    return (int *)0x0;
  }

  puVar5[0xe] = 1;
  puVar5[0xf] = 1;
  puVar5[0x10] = 1;
  puVar5[0x11] = 1;
  puVar5[0x12] = 1;
  puVar5[0x13] = 1;
  puVar5[0x14] = 1;
  puVar5[0x15] = 1;
  puVar5[0x16] = 1;
  puVar5[0x23] = 1;
  puVar5[0x24] = 1;
  puVar5[0x25] = 1;
  puVar5[0x20] = 0xfd;
  puVar5[0x21] = 0xcf;
  puVar5[0x22] = 0x2c;
  puVar5[0x26] = 10;
  puVar5[6] = 0;
  puVar5[8] = 0;
  puVar5[7] = 0;
  puVar5[9] = 0;
  puVar5[10] = 0;
  *puVar5 = 0;
  puVar5[0xb] = 0;
  puVar5[1] = 0;
  puVar5[2] = 0;
  puVar5[4] = 0;
  puVar5[3] = 0;
  puVar5[5] = 0;
  puVar5[0xc] = 0;
  puVar5[0x29] = 0;
  puVar5[0x27] = 0xa0;
  puVar5[0xd] = 0x3ff00000;
  puVar5[0x28] = 100;
  iVar7 = -0x708;

  while( 1 ) {
    iVar8 = iVar7 + 1;
    fVar9 = (double)fsin((double)iVar7 * (double)_DAT_0047adc0 * (double)_DAT_0047adc8);
    if (0x189b < iVar8) break;
    *(float *)((int)pvVar6 + iVar8 * 4 + 0x1c1c) = (float)fVar9;
    iVar7 = iVar8;
  }

  *(float *)((int)pvVar6 + iVar8 * 4 + 0x1c1c) = (float)fVar9;
  DAT_0063b5f8 = (int)pvVar6 + 0xe10;
  return puVar5;
}

/**
 * @original Lisa_FreeEngineMemory (IGN_WIN.EXE @ 0x00446c30, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_FreeEngineMemory(void) {
  void *_Memory;
  int iVar1;
  int iVar2;
  int iVar3;

  

  iVar2 = 0;

  if (DAT_0063c5d0 != (void *)0x0) {
    _Memory = DAT_0063c5d0;

    if (0 < DAT_0063c5d4) {
      iVar3 = 0;
      iVar1 = DAT_0063c5d4;

      do {
        if (*(void **)(iVar3 + (int)_Memory) != (void *)0x0) {
          _free(*(void **)(iVar3 + (int)_Memory));
          _Memory = DAT_0063c5d0;
          iVar1 = DAT_0063c5d4;
        }

        iVar3 = iVar3 + 4;
        iVar2 = iVar2 + 1;
      } while (iVar2 < iVar1);
    }

    _free(_Memory);
  }

  DAT_0063c5d0 = (void *)0x0;
  DAT_0063c5d4 = 0;
  DAT_0063c5b0 = 0;
  DAT_0049c9bc = 1;
  DAT_0049c9ac = 0;
  return;
}

/**
 * @original Lisa_InitSpatialGrid (IGN_WIN.EXE @ 0x00446ca0, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_InitSpatialGrid(int param_1,int param_2,size_t param_3) {
  int *puVar1;
  int iVar2;
  void *pvVar3;
  int *puVar4;
  int iVar5;
  size_t sVar6;
  int bVar7;

  

  DAT_0063c608 = param_1;
  DAT_0063c60c = param_2;
  DAT_0063c5b8 = (int)(param_1 + 0xff + (param_1 + 0xff >> 0x1f & 0xffU)) >> 8;

  DAT_0063b5ec = calloc(((int)(param_2 + 0xff + (param_2 + 0xff >> 0x1f & 0xffU)) >> 8) *

                         DAT_0063c5b8,4);
  DAT_0063c5c0 = calloc(param_3 + 5,4);
  pvVar3 = calloc(param_3,0x2a);
  iVar2 = DAT_0063c5d0;
  puVar1 = DAT_0063c5c0;
  iVar5 = DAT_0063c5d4 + 3;
  DAT_0063c5e8 = DAT_0063c5c0;
  bVar7 = DAT_0063b5ec != (void *)0x0;
  DAT_0063c5d4 = iVar5;
  DAT_0063c5dc = pvVar3;
  *(void **)(DAT_0063c5d0 + -0xc + iVar5 * 4) = DAT_0063b5ec;
  *(int **)(iVar2 + -8 + iVar5 * 4) = puVar1;
  *(void **)(iVar2 + -4 + iVar5 * 4) = pvVar3;

  if (((bVar7) && (pvVar3 != (void *)0x0)) && (puVar1 != (int *)0x0)) {
    puVar4 = puVar1;
    sVar6 = param_3;

    if (0 < (int)param_3) {
      do {
        *puVar4 = pvVar3;
        pvVar3 = (void *)((int)pvVar3 + 0x2a);
        sVar6 = sVar6 - 1;
        puVar4 = puVar4 + 1;
      } while (sVar6 != 0);
    }

    puVar1[param_3] = 0xffffffff;
    return 0;
  }

  return 0xffffffff;
}

/**
 * @original Lisa_CreateDynamicObject (IGN_WIN.EXE @ 0x00446d90, lisa3d.c)
 * @fidelity ADAPTED
 */
long long Lisa_CreateDynamicObject(int param_1,int param_2,int *param_3,int *param_4,int param_5, short param_6,short param_7,short param_8,short param_9) {
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  unsigned int uVar5;
  unsigned int uVar6;
  unsigned int uVar7;
  unsigned int *puVar8;
  long long lVar9;
  int local_8;
  int local_4;

  

  iVar1 = *DAT_0063c5e8;

  if (iVar1 == -1) {
    return (((long long)(param_2) << 32) | ((unsigned int)(0xffffffff)));
  }

  DAT_0063c5e8 = DAT_0063c5e8 + 1;
  *(int *)(iVar1 + 8) = param_5;
  *(int *)iVar1 = iVar1;
  *(int **)(iVar1 + 4) = param_4;
  iVar2 = param_3[2];
  iVar3 = param_3[3];
  *(int *)(iVar1 + 0xc) = param_3[1];
  iVar4 = param_3[4];
  *(int *)(iVar1 + 0x10) = iVar2;
  *(short *)(iVar1 + 0x18) = (short)iVar4;
  iVar2 = param_3[5];
  *(int *)(iVar1 + 0x14) = iVar3;
  *(short *)(iVar1 + 0x1a) = (short)iVar2;
  *(short *)(iVar1 + 0x1c) = (short)param_3[6];
  *(short *)(iVar1 + 0x1e) = param_6;
  *param_3 = iVar1;
  *(short *)(iVar1 + 0x20) = param_7;
  uVar5 = 0;
  *(short *)(iVar1 + 0x22) = param_8;
  *(int *)(iVar1 + 0x26) = 0;
  *(short *)(iVar1 + 0x24) = param_9;
  param_3[7] = 0;

  if (param_8 == -1) {
    local_8 = -1000;
    local_4 = *param_4;

    if (0 < local_4) {
      puVar8 = (unsigned int *)(param_4 + 2);

      do {
        uVar6 = (int)*puVar8 >> 0x1f;
        uVar7 = (int)puVar8[1] >> 0x1f;
        uVar5 = (int)puVar8[2] >> 0x1f;

        if (local_8 < (int)((((((*puVar8 ^ uVar6) - uVar6) - uVar7) + (puVar8[1] ^ uVar7)) - uVar5)

                           + (puVar8[2] ^ uVar5))) {
          lVar9 = __ftol();
          uVar5 = (unsigned int)((unsigned long long)lVar9 >> 0x20);

          if (local_8 < (int)lVar9) {
            local_8 = (int)lVar9;
          }

        }

        puVar8 = puVar8 + 3;
        local_4 = local_4 + -1;
      } while (local_4 != 0);
    }

    *(short *)(iVar1 + 0x22) = (short)local_8;
  }

  return (unsigned long long)uVar5 << 0x20;
}

/**
 * @original Lisa_MoveDynamicObject (IGN_WIN.EXE @ 0x00446eb0, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_MoveDynamicObject(int *param_1) {
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;

  

  if (param_1[7] != 1) {
    iVar1 = *param_1;
    iVar4 = param_1[1];
    iVar2 = param_1[2];
    *(short *)(iVar1 + 0x18) = (short)param_1[4];
    *(short *)(iVar1 + 0x1a) = (short)param_1[5];
    *(short *)(iVar1 + 0x1c) = (short)param_1[6];
    *(int *)(iVar1 + 0xc) = iVar4;
    *(int *)(iVar1 + 0x10) = iVar2;
    iVar2 = param_1[3];
    *(int *)(iVar1 + 0x14) = iVar2;
    iVar3 = DAT_0063b5ec;

    iVar4 = ((int)(iVar2 + (iVar2 >> 0x1f & 0xffU)) >> 8) * DAT_0063c5b8 +

            ((int)(iVar4 + (iVar4 >> 0x1f & 0xffU)) >> 8);
    param_1[7] = 1;
    *(int *)(iVar1 + 0x26) = *(int *)(iVar3 + iVar4 * 4);
    *(int *)(iVar3 + iVar4 * 4) = iVar1;
  }

  return 0;
}

/**
 * @original Lisa_UpdateObjectSpatialGrid (IGN_WIN.EXE @ 0x00446f30, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_UpdateObjectSpatialGrid(int *param_1) {
  int *piVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;

  

  iVar2 = *param_1;
  iVar7 = param_1[1];
  iVar8 = *(int *)(iVar2 + 0xc);
  iVar3 = *(int *)(iVar2 + 0x14);

  if ((((-1 < iVar7) && (iVar4 = param_1[3], -1 < iVar4)) && (iVar7 <= DAT_0063c608)) &&

     (iVar4 <= DAT_0063c60c)) {
    iVar5 = param_1[2];
    *(int *)(iVar2 + 0xc) = iVar7;
    *(int *)(iVar2 + 0x10) = iVar5;
    *(int *)(iVar2 + 0x14) = iVar4;
    *(short *)(iVar2 + 0x18) = (short)param_1[4];
    *(short *)(iVar2 + 0x1a) = (short)param_1[5];
    *(short *)(iVar2 + 0x1c) = (short)param_1[6];
    iVar6 = DAT_0063c5b8;
    iVar5 = DAT_0063b5ec;

    if (param_1[7] == 1) {
      iVar7 = (int)(iVar7 + (iVar7 >> 0x1f & 0xffU)) >> 8;
      iVar8 = (int)(iVar8 + (iVar8 >> 0x1f & 0xffU)) >> 8;

      if ((iVar7 != iVar8) ||

         ((int)(iVar4 + (iVar4 >> 0x1f & 0xffU)) >> 8 != (int)(iVar3 + (iVar3 >> 0x1f & 0xffU)) >> 8

         )) {
        iVar8 = ((int)(iVar3 + (iVar3 >> 0x1f & 0xffU)) >> 8) * DAT_0063c5b8 + iVar8;
        iVar3 = *(int *)(DAT_0063b5ec + iVar8 * 4);

        if (iVar3 == iVar2) {
          *(int *)(DAT_0063b5ec + iVar8 * 4) = *(int *)(iVar2 + 0x26);
        }

        else {
          iVar8 = *(int *)(iVar3 + 0x26);

          while (iVar8 != iVar2) {
            iVar3 = *(int *)(iVar3 + 0x26);
            iVar8 = *(int *)(iVar3 + 0x26);
          }

          *(int *)(iVar3 + 0x26) = *(int *)(iVar2 + 0x26);
        }

        piVar1 = (int *)(iVar5 + (((int)(iVar4 + (iVar4 >> 0x1f & 0xffU)) >> 8) * iVar6 + iVar7) * 4

                        );
        *(int *)(iVar2 + 0x26) = *piVar1;
        *piVar1 = iVar2;
      }

    }

    return 0;
  }

  return 0xffffffff;
}

/**
 * @original Lisa_SetDynamicObjectMesh (IGN_WIN.EXE @ 0x00447070, lisa3d.c)
 * @fidelity ADAPTED
 */
long long Lisa_SetDynamicObjectMesh(int param_1,int param_2,int *param_3,int *param_4,int param_5, short param_6,short param_7,short param_8,short param_9) {
  int iVar1;
  unsigned int uVar2;
  unsigned int uVar3;
  unsigned int uVar4;
  unsigned int *puVar5;
  long long lVar6;
  int local_8;
  int local_4;

  

  iVar1 = *param_3;
  *(short *)(iVar1 + 0x1e) = param_6;
  uVar4 = CONCAT22((short)((unsigned int)param_2 >> 0x10),param_8);
  *(short *)(iVar1 + 0x22) = param_8;
  *(int **)(iVar1 + 4) = param_4;
  *(int *)(iVar1 + 8) = param_5;
  *(short *)(iVar1 + 0x20) = param_7;
  *(short *)(iVar1 + 0x24) = param_9;

  if (param_8 == -1) {
    local_8 = -1000;
    local_4 = *param_4;

    if (0 < local_4) {
      puVar5 = (unsigned int *)(param_4 + 2);

      do {
        uVar2 = (int)*puVar5 >> 0x1f;
        uVar3 = (int)puVar5[1] >> 0x1f;
        uVar4 = (int)puVar5[2] >> 0x1f;

        if (local_8 < (int)((((((*puVar5 ^ uVar2) - uVar2) - uVar3) + (puVar5[1] ^ uVar3)) - uVar4)

                           + (puVar5[2] ^ uVar4))) {
          lVar6 = __ftol();
          uVar4 = (unsigned int)((unsigned long long)lVar6 >> 0x20);

          if (local_8 < (int)lVar6) {
            local_8 = (int)lVar6;
          }

        }

        puVar5 = puVar5 + 3;
        local_4 = local_4 + -1;
      } while (local_4 != 0);
    }

    *(short *)(iVar1 + 0x22) = (short)local_8;
  }

  return (unsigned long long)uVar4 << 0x20;
}

/**
 * @original Lisa_DeleteDynamicObject (IGN_WIN.EXE @ 0x00447150, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_DeleteDynamicObject(int *param_1) {
  int iVar1;
  int iVar2;
  int iVar3;

  

  if (param_1[7] == 1) {
    iVar1 = *param_1;

    iVar3 = ((int)(*(int *)(iVar1 + 0x14) + (*(int *)(iVar1 + 0x14) >> 0x1f & 0xffU)) >> 8) *

            DAT_0063c5b8 +

            ((int)(*(int *)(iVar1 + 0xc) + (*(int *)(iVar1 + 0xc) >> 0x1f & 0xffU)) >> 8);
    iVar2 = *(int *)(DAT_0063b5ec + iVar3 * 4);

    if (iVar1 == iVar2) {
      *(int *)(DAT_0063b5ec + iVar3 * 4) = *(int *)(iVar1 + 0x26);
    }

    else {
      iVar3 = *(int *)(iVar2 + 0x26);

      while (iVar3 != iVar1) {
        iVar2 = *(int *)(iVar2 + 0x26);
        iVar3 = *(int *)(iVar2 + 0x26);
      }

      *(int *)(iVar2 + 0x26) = *(int *)(iVar1 + 0x26);
    }

    param_1[7] = 0;
  }

  return 0;
}

/**
 * @original Lisa_SetCameraViewport (IGN_WIN.EXE @ 0x004471e0, lisa3d.c)
 * @fidelity ADAPTED
 */
unsigned long long Lisa_SetCameraViewport(void) {
  int iVar1;
  long long lVar2;
  unsigned long long uVar3;

  

  lVar2 = __ftol();
  iVar1 = g_LisaCamera;
  g_LisaCamera->viewport_x = (int)lVar2;
  lVar2 = __ftol();
  *(int *)(iVar1 + 0x84) = (int)lVar2;
  lVar2 = __ftol();
  *(int *)(iVar1 + 0x88) = (int)lVar2;
  lVar2 = __ftol();
  *(int *)(iVar1 + 0x9c) = (int)lVar2;
  uVar3 = __ftol();
  *(int *)(iVar1 + 0xa0) = (int)uVar3;
  return uVar3 & 0xffffffff00000000;
}

/**
 * @original FUN_00447280 (IGN_WIN.EXE @ 0x00447280, lisa3d.c)
 * @fidelity ADAPTED
 */
int FUN_00447280(unsigned int *param_1,int param_2,int param_3,int param_4,int param_5,int param_6 ,int param_7,int param_8,char *param_9) {
  char uVar1;
  char *puVar2;
  char *puVar3;
  unsigned int *puVar4;
  unsigned int *puVar5;
  void *pvVar6;
  byte *pbVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  byte *pbVar12;
  byte *pbVar13;
  unsigned int *puVar14;
  int bVar15;
  long long lVar16;
  byte *local_440;
  int local_438;
  byte *local_428;
  byte *local_424;
  int local_414;
  int local_40c;
  char local_400 [1024];

  

  puVar2 = local_400;

  do {
    uVar1 = param_9[1];
    *puVar2 = *param_9;
    puVar2[1] = uVar1;
    puVar3 = puVar2 + 4;
    puVar2[2] = param_9[2];
    puVar2 = puVar3;
    param_9 = param_9 + 3;
  } while (puVar3 < (unsigned int *)(local_400 + sizeof(local_400)));
  puVar4 = calloc(0x15,4);
  DAT_0063c5d4 = DAT_0063c5d4 + 1;
  *(unsigned int **)(DAT_0063c5d0 + -4 + DAT_0063c5d4 * 4) = puVar4;
  puVar5 = puVar4 + 5;
  lVar16 = __ftol();
  local_438 = 0;

  if (0 < (int)lVar16) {
    do {
      if (5999 < local_438) break;
      local_438 = local_438 + 1;
    } while (local_438 < (int)lVar16);
  }

  lVar16 = __ftol();
  iVar9 = (int)lVar16;
  local_440 = (byte *)*param_1;
  *puVar4 = (unsigned int)local_440;
  puVar4[1] = (unsigned int)local_440;
  puVar4[2] = (unsigned int)local_440;
  puVar4[3] = (unsigned int)local_440;
  puVar4[4] = (unsigned int)local_440;
  *puVar5 = (unsigned int)local_440;
  iVar10 = 0;

  if (0 < param_4 + -1) {
    iVar11 = iVar10;

    do {
      DAT_0049c9bc = 1;
      DAT_0049c9ac = 1;
      pvVar6 = calloc(param_7 * 0x100 + 0xffff,1);
      DAT_0063c5d4 = DAT_0063c5d4 + 1;
      *(void **)(DAT_0063c5d0 + -4 + DAT_0063c5d4 * 4) = pvVar6;
      pbVar7 = (byte *)((int)pvVar6 + 0xffffU & 0xffff0000);
      iVar8 = DAT_0063c5b0 + 2;
      DAT_0063c5b0 = iVar8;
      *(byte **)(&DAT_0063b608 + iVar8 * 4) = pbVar7;
      iVar10 = iVar11 + 1;
      *(int *)(&DAT_0063b60c + iVar8 * 4) = param_7 << 8;

      if (iVar10 < 5) {
        puVar4[iVar11 + 1] = (unsigned int)pbVar7;
      }

      iVar8 = 0;

      if (0 < iVar9) {
        do {
          if (5999 < local_438) break;
          iVar8 = iVar8 + 1;
          local_438 = local_438 + 1;
        } while (iVar8 < iVar9);
      }

      if (0 < param_7 / param_6) {
        local_424 = local_440;
        iVar9 = (int)(0x100 / (long long)param_5);
        local_428 = pbVar7;
        local_40c = param_7 / param_6;

        do {
          pbVar12 = local_428;
          pbVar13 = local_424;
          local_414 = iVar9;

          if (0 < iVar9) {
            do {
              if (iVar11 == 0) {
                Lisa_DownsampleTextureMipmap(pbVar13,pbVar12,param_5,param_6,0x100,(int)local_400);
              }

              else {
                Lisa_FilterTextureBlock(pbVar13,(int)pbVar12,param_5,param_6,0x100,(int)local_400,param_8,

                             iVar11);
              }

              local_414 = local_414 + -1;
              pbVar12 = pbVar12 + param_5;
              pbVar13 = pbVar13 + param_5;
            } while (local_414 != 0);
          }

          local_428 = local_428 + param_6 * 0x100;
          local_424 = local_424 + param_6 * 0x100;
          local_40c = local_40c + -1;
        } while (local_40c != 0);
      }

      lVar16 = __ftol();
      iVar9 = (int)lVar16;
      iVar11 = iVar10;
      local_440 = pbVar7;
    } while (iVar10 < param_4 + -1);
  }

  if (iVar10 < 4) {
    puVar14 = puVar4 + iVar10 + 1;

    for (iVar9 = 4 - iVar10; iVar9 != 0; iVar9 = iVar9 + -1) {
      *puVar14 = (unsigned int)local_440;
      puVar14 = puVar14 + 1;
    }

  }

  bVar15 = DAT_0049c9b4 == 0;
  *param_1 = (unsigned int)puVar5;

  if ((bVar15) && (1 < param_4)) {
    Lisa_GenerateTextureSpanTable(*puVar4,(int)local_400,param_8,param_7,puVar5);
  }

  return 0;
}

/**
 * @original Lisa_GenerateTextureSpanTable (IGN_WIN.EXE @ 0x004475c0, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_GenerateTextureSpanTable(int param_1,int param_2,int param_3,int param_4,int *param_5) {
  unsigned int uVar1;
  int iVar2;
  byte bVar3;
  unsigned int uVar4;
  unsigned int uVar5;
  int iVar6;
  void *pvVar7;
  unsigned int *puVar8;
  byte *pbVar9;
  unsigned int uVar10;
  byte bVar11;
  int *puVar12;
  int iVar13;
  unsigned int uVar14;
  byte bVar15;
  unsigned int *puVar16;
  unsigned int local_c4 [3];
  unsigned int local_b8;
  int local_b4;
  unsigned int *local_b0;
  unsigned int *local_ac;
  int local_a8 [3];
  int local_9c;
  int *local_98;
  int local_94;
  int local_90;
  int local_8c;
  int *local_88;
  int local_84;
  int local_80;
  int local_7c;
  int local_78;
  int local_74;
  int local_70;
  int local_6c;
  int local_68;
  int local_64;
  int local_60;
  int local_5c;
  int local_58;
  int local_54;
  int local_50;
  unsigned int *local_4c;
  int local_44;
  int local_40;
  unsigned int local_3c;
  unsigned int local_38;
  unsigned int local_34;
  unsigned int local_30;
  unsigned int local_2c;
  unsigned int local_28;
  unsigned int local_24;
  unsigned int local_20;
  unsigned int local_1c;
  unsigned int local_18;
  unsigned int local_14;
  unsigned int local_10;
  size_t local_c;
  int local_8;
  int local_4;

  

  local_88 = calloc(0x10000,0x14);
  *local_88 = 1;
  iVar6 = 0x10000;
  puVar12 = local_88;

  do {
    *puVar12 = 0;
    puVar12 = puVar12 + 5;
    iVar6 = iVar6 + -1;
  } while (iVar6 != 0);
  local_98 = param_5;
  local_c = param_4 * 0x400 + 0xffff;
  local_94 = 0;
  local_8 = param_4 << 10;

  do {
    pvVar7 = calloc(local_c,1);
    DAT_0063c5d4 = DAT_0063c5d4 + 1;
    *(void **)(DAT_0063c5d0 + -4 + DAT_0063c5d4 * 4) = pvVar7;
    puVar8 = (unsigned int *)((int)pvVar7 + 0xffffU & 0xffff0000);
    iVar6 = DAT_0063c5b0 + 2;
    puVar12 = local_98 + 4;
    DAT_0063c5b0 = iVar6;
    *(unsigned int **)(&DAT_0063b608 + iVar6 * 4) = puVar8;
    *(int *)(&DAT_0063b60c + iVar6 * 4) = local_8;
    *local_98 = puVar8;
    local_8c = 0;
    local_98[1] = puVar8;
    local_98[2] = puVar8;
    local_98[3] = puVar8;

    if (0 < param_4) {
      local_4 = param_4 + -1;
      local_90 = 0;
      local_4c = puVar8;

      do {
        local_a8[0] = 0;
        local_a8[1] = 0;
        local_a8[2] = 0;
        local_b4 = 0;
        local_ac = local_4c;

        do {
          bVar3 = *(byte *)(local_94 + local_b4 + param_1 + local_90);
          local_b8 = ((((unsigned int)((((unsigned int)(local_b8)) >> 8))) << 8) | ((unsigned char)(bVar3)));

          if (local_b4 < 0x3f) {
            pbVar9 = (byte *)(local_94 + local_b4 + param_1 + local_90);
            bVar15 = pbVar9[1];
          }

          else {
            pbVar9 = (byte *)(local_94 + local_b4 + param_1 + local_90);
            bVar15 = *pbVar9;
          }

          local_9c = ((((unsigned int)((((unsigned int)(local_9c)) >> 8))) << 8) | ((unsigned char)(bVar15)));

          if (local_8c < local_4) {
            bVar11 = pbVar9[0x100];

            if (local_b4 < 0x3f) {
              puVar8 = (unsigned int *)(unsigned int)pbVar9[0x101];
            }

            else {
LAB_00447785:

              puVar8 = (unsigned int *)(unsigned int)bVar11;
            }

          }

          else {
            bVar11 = *pbVar9;
            if (0x3e < local_b4) goto LAB_00447785;
            puVar8 = (unsigned int *)(unsigned int)pbVar9[1];
          }

          uVar10 = (unsigned int)bVar11;
          uVar14 = (unsigned int)bVar15;
          uVar4 = (unsigned int)bVar3;
          uVar1 = (((int)puVar8 * 0x100 + uVar10) * 0x100 + uVar14) * 0x100 + uVar4;
          puVar16 = local_88 + ((uVar1 >> 0x11) + uVar10 + uVar14 + uVar1 & 0xffff) * 5;
          local_b0 = puVar8;

          if (*puVar16 != uVar1) {
            *puVar16 = uVar1;
            local_b8 = (unsigned int)*(byte *)(param_2 + uVar4 * 4);
            uVar1 = local_b8;
            local_30 = local_b8;
            local_b8 = (unsigned int)*(byte *)(param_2 + 1 + uVar4 * 4);
            uVar5 = local_b8;
            local_2c = local_b8;
            local_b8 = (unsigned int)*(byte *)(param_2 + 2 + uVar4 * 4);
            uVar4 = local_b8;
            local_28 = local_b8;
            local_3c = (unsigned int)*(byte *)(param_2 + uVar14 * 4);
            local_38 = (unsigned int)*(byte *)(param_2 + 1 + uVar14 * 4);
            local_34 = (unsigned int)*(byte *)(param_2 + 2 + uVar14 * 4);
            local_18 = (unsigned int)*(byte *)(param_2 + uVar10 * 4);
            local_14 = (unsigned int)*(byte *)(param_2 + 1 + uVar10 * 4);
            local_10 = (unsigned int)*(byte *)(param_2 + 2 + uVar10 * 4);
            local_24 = (unsigned int)*(byte *)(param_2 + (int)puVar8 * 4);
            local_20 = (unsigned int)*(byte *)(param_2 + 1 + (int)puVar8 * 4);
            local_b0 = local_ac;
            local_9c = 0;
            local_74 = 0;
            local_1c = (unsigned int)*(byte *)(param_2 + 2 + (int)puVar8 * 4);
            local_6c = 0;
            local_78 = local_34 << 2;
            local_64 = 0;
            local_70 = local_38 << 2;
            local_68 = local_3c << 2;
            local_5c = 0;
            local_54 = 0;
            local_60 = local_b8 << 2;
            local_b8 = 4;
            local_58 = uVar5 * 4;
            local_50 = uVar1 << 2;
            puVar8 = puVar16;

            do {
              iVar6 = (int)(local_54 + local_50 + (local_54 + local_50 >> 0x1f & 3U)) >> 2;
              local_44 = (int)(local_5c + local_58 + (local_5c + local_58 >> 0x1f & 3U)) >> 2;
              local_40 = (int)(local_64 + local_60 + (local_64 + local_60 >> 0x1f & 3U)) >> 2;
              local_84 = (int)(local_6c + local_68 + (local_6c + local_68 >> 0x1f & 3U)) >> 2;
              local_80 = (int)(local_74 + local_70 + (local_74 + local_70 >> 0x1f & 3U)) >> 2;
              local_7c = (int)(local_9c + local_78 + (local_9c + local_78 >> 0x1f & 3U)) >> 2;
              local_c4[0] = iVar6 * 4;
              iVar13 = 0;
              local_c4[1] = local_44 << 2;
              local_c4[2] = local_40 << 2;

              do {
                iVar2 = *(int *)((int)local_a8 + iVar13) + 8 + *(int *)((int)local_c4 + iVar13);
                *(int *)((int)local_c4 + iVar13) = iVar2;

                if (0x3ff < iVar2) {
                  *(int *)((int)local_c4 + iVar13) = 0x3ff;
                }

                if (*(int *)((int)local_c4 + iVar13) < 0) {
                  *(int *)((int)local_c4 + iVar13) = 0;
                }

                iVar13 = iVar13 + 4;
              } while (iVar13 < 0xc);

              bVar3 = (&DAT_004cdd18)

                      [(local_c4[1] & 0x3f0) * 4 +

                       ((local_c4[2] & 0x3f0) >> 4) + (local_c4[0] & 0x3f0) * 0x100];
              uVar14 = (unsigned int)bVar3;
              *(byte *)(puVar8 + 1) = bVar3;
              bVar15 = *(byte *)(param_2 + uVar14 * 4);
              *(byte *)local_b0 = bVar3;
              local_a8[0] = (int)(local_c4[0] + (unsigned int)bVar15 * -4) / 2;
              local_a8[1] = (int)(local_c4[1] + (unsigned int)*(byte *)(param_2 + 1 + uVar14 * 4) * -4) / 2;
              local_a8[2] = (int)(local_c4[2] + (unsigned int)*(byte *)(param_2 + 2 + uVar14 * 4) * -4) / 2;
              local_c4[0] = iVar6 * 3 + local_84;
              local_c4[1] = local_44 * 3 + local_80;
              iVar13 = 0;
              local_c4[2] = local_40 * 3 + local_7c;

              do {
                iVar2 = *(int *)((int)local_a8 + iVar13) + 8 + *(int *)((int)local_c4 + iVar13);
                *(int *)((int)local_c4 + iVar13) = iVar2;

                if (0x3ff < iVar2) {
                  *(int *)((int)local_c4 + iVar13) = 0x3ff;
                }

                if (*(int *)((int)local_c4 + iVar13) < 0) {
                  *(int *)((int)local_c4 + iVar13) = 0;
                }

                iVar13 = iVar13 + 4;
              } while (iVar13 < 0xc);

              uVar14 = (unsigned int)(byte)(&DAT_004cdd18)

                                   [(local_c4[1] & 0x3f0) * 4 +

                                    ((local_c4[2] & 0x3f0) >> 4) + (local_c4[0] & 0x3f0) * 0x100];

              *(char *)((int)puVar8 + 5) =

                   (&DAT_004cdd18)

                   [(local_c4[1] & 0x3f0) * 4 +

                    ((local_c4[2] & 0x3f0) >> 4) + (local_c4[0] & 0x3f0) * 0x100];
              local_a8[0] = (int)(local_c4[0] + (unsigned int)*(byte *)(param_2 + uVar14 * 4) * -4) / 2;
              local_a8[1] = (int)(local_c4[1] + (unsigned int)*(byte *)(param_2 + 1 + uVar14 * 4) * -4) / 2;
              local_a8[2] = (int)(local_c4[2] + (unsigned int)*(byte *)(param_2 + 2 + uVar14 * 4) * -4) / 2;
              local_c4[0] = (local_84 + iVar6) * 2;
              local_c4[1] = (local_80 + local_44) * 2;
              iVar13 = 0;
              local_c4[2] = (local_7c + local_40) * 2;

              do {
                iVar2 = *(int *)((int)local_a8 + iVar13) + 8 + *(int *)((int)local_c4 + iVar13);
                *(int *)((int)local_c4 + iVar13) = iVar2;

                if (0x3ff < iVar2) {
                  *(int *)((int)local_c4 + iVar13) = 0x3ff;
                }

                if (*(int *)((int)local_c4 + iVar13) < 0) {
                  *(int *)((int)local_c4 + iVar13) = 0;
                }

                iVar13 = iVar13 + 4;
              } while (iVar13 < 0xc);

              uVar14 = (unsigned int)(byte)(&DAT_004cdd18)

                                   [(local_c4[1] & 0x3f0) * 4 +

                                    ((local_c4[2] & 0x3f0) >> 4) + (local_c4[0] & 0x3f0) * 0x100];

              *(char *)((int)puVar8 + 6) =

                   (&DAT_004cdd18)

                   [(local_c4[1] & 0x3f0) * 4 +

                    ((local_c4[2] & 0x3f0) >> 4) + (local_c4[0] & 0x3f0) * 0x100];
              local_a8[0] = (int)(local_c4[0] + (unsigned int)*(byte *)(param_2 + uVar14 * 4) * -4) / 2;
              local_a8[1] = (int)(local_c4[1] + (unsigned int)*(byte *)(param_2 + 1 + uVar14 * 4) * -4) / 2;
              local_a8[2] = (int)(local_c4[2] + (unsigned int)*(byte *)(param_2 + 2 + uVar14 * 4) * -4) / 2;
              local_c4[0] = local_84 * 3 + iVar6;
              iVar6 = 0;
              local_c4[1] = local_80 * 3 + local_44;
              local_c4[2] = local_7c * 3 + local_40;

              do {
                iVar13 = *(int *)((int)local_a8 + iVar6) + 8 + *(int *)((int)local_c4 + iVar6);
                *(int *)((int)local_c4 + iVar6) = iVar13;

                if (0x3ff < iVar13) {
                  *(int *)((int)local_c4 + iVar6) = 0x3ff;
                }

                if (*(int *)((int)local_c4 + iVar6) < 0) {
                  *(int *)((int)local_c4 + iVar6) = 0;
                }

                iVar6 = iVar6 + 4;
              } while (iVar6 < 0xc);

              uVar14 = (unsigned int)(byte)(&DAT_004cdd18)

                                   [(local_c4[1] & 0x3f0) * 4 +

                                    ((local_c4[2] & 0x3f0) >> 4) + (local_c4[0] & 0x3f0) * 0x100];

              *(char *)((int)puVar8 + 7) =

                   (&DAT_004cdd18)

                   [(local_c4[1] & 0x3f0) * 4 +

                    ((local_c4[2] & 0x3f0) >> 4) + (local_c4[0] & 0x3f0) * 0x100];
              local_a8[0] = (int)(local_c4[0] + (unsigned int)*(byte *)(param_2 + uVar14 * 4) * -4) / 2;
              local_a8[1] = (int)(local_c4[1] + (unsigned int)*(byte *)(param_2 + 1 + uVar14 * 4) * -4) / 2;
              local_b0 = local_b0 + 0x40;
              local_a8[2] = (int)(local_c4[2] + (unsigned int)*(byte *)(param_2 + 2 + uVar14 * 4) * -4) / 2;
              local_9c = local_9c + local_1c;
              local_78 = local_78 - local_34;
              local_74 = local_74 + local_20;
              local_70 = local_70 - local_38;
              local_6c = local_6c + local_24;
              local_68 = local_68 - local_3c;
              local_64 = local_64 + local_10;
              local_60 = local_60 - uVar4;
              local_5c = local_5c + local_14;
              local_58 = local_58 - uVar5;
              local_54 = local_54 + local_18;
              local_50 = local_50 - uVar1;
              local_b8 = local_b8 + -1;
              puVar8 = puVar8 + 1;
            } while (local_b8 != 0);
            local_a8[0] = 0;
            local_a8[1] = 0;
            local_a8[2] = 0;
            local_b8 = 0;
          }

          local_a8[2] = 0;
          local_a8[1] = 0;
          local_a8[0] = 0;
          uVar1 = puVar16[2];
          puVar8 = local_ac + 1;
          local_b4 = local_b4 + 1;
          *local_ac = puVar16[1];
          uVar14 = puVar16[3];
          local_ac[0x40] = uVar1;
          uVar1 = puVar16[4];
          local_ac[0x80] = uVar14;
          local_ac[0xc0] = uVar1;
          local_ac = puVar8;
        } while (local_b4 < 0x40);
        local_90 = local_90 + 0x100;
        local_4c = local_4c + 0x100;
        local_8c = local_8c + 1;
      } while (local_8c < param_4);
    }

    local_94 = local_94 + 0x40;
    local_98 = puVar12;

    if (0xff < local_94) {
      _free(local_88);
      return;
    }

  } while( 1 );
}

/**
 * @original Lisa_DownsampleTextureMipmap (IGN_WIN.EXE @ 0x00447fb0, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_DownsampleTextureMipmap(byte *param_1,byte *param_2,int param_3,int param_4,int param_5,int param_6) {
  int iVar1;
  byte bVar2;
  int iVar3;
  unsigned int *puVar4;
  unsigned int uVar5;
  unsigned int uVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int local_2c;
  int local_24;
  unsigned int local_1c [4];
  int local_c [3];

  

  local_24 = 0;

  if (0 < param_4) {
    do {
      local_2c = 0;
      local_c[0] = 0;
      local_c[1] = 0;
      local_c[2] = 0;

      if (0 < param_3) {
        iVar3 = param_3 + -1;

        do {
          iVar7 = 0;
          puVar4 = local_1c;
          bVar2 = *param_1;

          do {
            uVar6 = (unsigned int)*(byte *)((unsigned int)bVar2 * 4 + iVar7 + param_6);
            *puVar4 = uVar6;

            if (local_2c < iVar3) {
              uVar5 = *(byte *)(iVar7 + (unsigned int)param_1[1] * 4 + param_6) + uVar6;
            }

            else {
              uVar5 = uVar6 * 2;
            }

            *puVar4 = uVar5;

            if (local_24 < param_4 + -1) {
              uVar6 = (unsigned int)*(byte *)(iVar7 + (unsigned int)param_1[param_5] * 4 + param_6);
              uVar5 = *puVar4 + uVar6;
              *puVar4 = uVar5;

              if (local_2c < iVar3) {
                *puVar4 = *(byte *)(iVar7 + (unsigned int)param_1[param_5 + 1] * 4 + param_6) + uVar5;
              }

              else {
                *puVar4 = uVar6 + uVar5;
              }

            }

            else {
              uVar5 = *puVar4 + uVar6;
              *puVar4 = uVar5;

              if (local_2c < iVar3) {
                *puVar4 = *(byte *)(iVar7 + (unsigned int)param_1[1] * 4 + param_6) + uVar5;
              }

              else {
                *puVar4 = uVar5 + uVar6;
              }

            }

            puVar4 = puVar4 + 1;
            iVar7 = iVar7 + 1;
          } while (puVar4 < local_1c + 3);
          iVar7 = 0;

          do {
            iVar9 = *(int *)((int)local_c + iVar7) + 8 + *(int *)((int)local_1c + iVar7);
            *(int *)((int)local_1c + iVar7) = iVar9;

            if (0x3ff < iVar9) {
              *(int *)((int)local_1c + iVar7) = 0x3ff;
            }

            if (*(int *)((int)local_1c + iVar7) < 0) {
              *(int *)((int)local_1c + iVar7) = 0;
            }

            iVar7 = iVar7 + 4;
          } while (iVar7 < 0xc);
          iVar9 = 0;

          bVar2 = (&DAT_004cdd18)

                  [(local_1c[1] & 0x3f0) * 4 +

                   ((local_1c[2] & 0x3f0) >> 4) + (local_1c[0] & 0x3f0) * 0x100];
          *param_2 = bVar2;
          iVar7 = 0;

          do {
            iVar8 = iVar7 + 4;
            iVar1 = iVar9 + (unsigned int)bVar2 * 4;
            iVar9 = iVar9 + 1;

            *(int *)((int)local_c + iVar7) =

                 (int)(*(int *)((int)local_1c + iVar7) + (unsigned int)*(byte *)(iVar1 + param_6) * -4) / 2;
            iVar7 = iVar8;
          } while (iVar8 < 0xc);
          param_2 = param_2 + 1;
          param_1 = param_1 + 1;
          local_2c = local_2c + 1;
        } while (local_2c < param_3);
      }

      param_1 = param_1 + (param_5 - param_3);
      local_24 = local_24 + 1;
      param_2 = param_2 + (param_5 - param_3);
    } while (local_24 < param_4);
  }

  return;
}

/**
 * @original Lisa_FilterTextureBlock (IGN_WIN.EXE @ 0x004481f0, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_FilterTextureBlock(byte *param_1,int param_2,int param_3,int param_4,int param_5,int param_6, int param_7,int param_8) {
  byte bVar1;
  unsigned int uVar2;
  int iVar3;
  unsigned int *puVar4;
  int iVar5;
  int local_54;
  int local_4c;
  int local_48;
  byte *local_44;
  int local_40;
  byte *local_3c;
  int local_38;
  byte *local_34;
  int local_30;
  byte *local_2c;
  int local_28;
  unsigned int local_24 [6];
  int local_c [3];

  

  local_4c = 0;

  if (0 < param_4) {
    local_40 = param_8 * param_5;
    local_3c = param_1 + local_40;
    local_44 = param_1 + local_40 + param_3 + -1;
    local_38 = (param_5 + -1) * param_8;
    local_34 = param_1 + (param_3 - local_40) + -1;
    local_30 = -local_40;
    local_2c = param_1 + -local_40;
    local_28 = (-1 - param_5) * param_8;

    do {
      local_54 = 0;
      local_c[0] = 0;
      local_c[1] = 0;
      local_c[2] = 0;

      if (0 < param_3) {
        do {
          iVar3 = 0;
          puVar4 = local_24;

          do {
            if (local_4c - param_8 < 1) {
              if (local_54 - param_8 < 1) {
                bVar1 = *param_1;
              }

              else {
                bVar1 = param_1[local_54 - param_8];
              }

              *puVar4 = (unsigned int)*(byte *)(iVar3 + (unsigned int)bVar1 * 4 + param_6);

              if (local_54 + param_8 < param_3) {
                uVar2 = (unsigned int)param_1[local_54 + param_8];
              }

              else {
                uVar2 = (unsigned int)param_1[param_3 + -1];
              }

            }

            else {
              if (local_54 == param_8 || local_54 - param_8 < 0) {
                *puVar4 = (unsigned int)*(byte *)(iVar3 + (unsigned int)*local_2c * 4 + param_6);
              }

              else {
                *puVar4 = (unsigned int)*(byte *)(iVar3 + (unsigned int)param_1[local_28 + local_54] * 4 + param_6);
              }

              if (param_8 + local_54 < param_3) {
                uVar2 = (unsigned int)param_1[local_30 + param_8 + local_54];
              }

              else {
                uVar2 = (unsigned int)*local_34;
              }

            }

            *puVar4 = *puVar4 + (unsigned int)*(byte *)(iVar3 + uVar2 * 4 + param_6);

            if (local_4c + param_8 < param_4) {
              if (local_54 == param_8 || local_54 - param_8 < 0) {
                bVar1 = *local_3c;
              }

              else {
                bVar1 = param_1[local_38 + local_54];
              }

              *puVar4 = *puVar4 + (unsigned int)*(byte *)(iVar3 + (unsigned int)bVar1 * 4 + param_6);

              if (param_8 + local_54 < param_3) {
                uVar2 = (unsigned int)param_1[local_40 + param_8 + local_54];
                goto LAB_004484c0;
              }

              *puVar4 = *puVar4 + (unsigned int)*(byte *)(iVar3 + (unsigned int)*local_44 * 4 + param_6);
            }

            else {
              if (local_54 == param_8 || local_54 - param_8 < 0) {
                iVar5 = (param_4 + -1) * param_5;
                bVar1 = *(byte *)(iVar3 + (unsigned int)param_1[iVar5] * 4 + param_6);
              }

              else {
                iVar5 = (param_4 + -1) * param_5;

                bVar1 = *(byte *)(iVar3 + (unsigned int)param_1[iVar5 + (local_54 - param_8)] * 4 + param_6)

                ;
              }

              *puVar4 = *puVar4 + (unsigned int)bVar1;

              if (param_8 + local_54 < param_3) {
                uVar2 = (unsigned int)param_1[param_8 + local_54];
              }

              else {
                uVar2 = (unsigned int)param_1[iVar5 + param_3 + -1];
              }

LAB_004484c0:

              *puVar4 = *puVar4 + (unsigned int)*(byte *)(iVar3 + uVar2 * 4 + param_6);
            }

            puVar4 = puVar4 + 1;
            iVar3 = iVar3 + 1;
          } while (puVar4 < local_24 + 3);
          iVar3 = 0;

          do {
            iVar5 = *(int *)((int)local_c + iVar3) + 8 + *(int *)((int)local_24 + iVar3);
            *(int *)((int)local_24 + iVar3) = iVar5;

            if (0x3ff < iVar5) {
              *(int *)((int)local_24 + iVar3) = 0x3ff;
            }

            if (*(int *)((int)local_24 + iVar3) < 0) {
              *(int *)((int)local_24 + iVar3) = 0;
            }

            iVar3 = iVar3 + 4;
          } while (iVar3 < 0xc);
          local_48 = 0;

          bVar1 = (&DAT_004cdd18)

                  [(local_24[0] & 0x3f0) * 0x100 +

                   ((local_24[2] & 0x3f0) >> 4) + (local_24[1] & 0x3f0) * 4];
          *(byte *)(param_2 + local_54) = bVar1;
          iVar3 = 0;

          do {
            iVar5 = iVar3 + 4;

            *(int *)((int)local_c + iVar3) =

                 (int)(*(int *)((int)local_24 + iVar3) +

                      (unsigned int)*(byte *)(local_48 + param_6 + (unsigned int)bVar1 * 4) * -4) / 2;
            local_48 = local_48 + 1;
            iVar3 = iVar5;
          } while (iVar5 < 0xc);
          local_54 = local_54 + 1;
        } while (local_54 < param_3);
      }

      param_2 = param_2 + param_5;
      local_44 = local_44 + param_5;
      local_40 = local_40 + param_5;
      local_3c = local_3c + param_5;
      local_38 = local_38 + param_5;
      local_34 = local_34 + param_5;
      local_30 = local_30 + param_5;
      local_2c = local_2c + param_5;
      local_28 = local_28 + param_5;
      local_4c = local_4c + 1;
    } while (local_4c < param_4);
  }

  return;
}

/**
 * @original Lisa_LoadOrCreateShadingTable (IGN_WIN.EXE @ 0x00448620, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_LoadOrCreateShadingTable(int param_1,int param_2) {
  byte *pbVar1;
  char cVar2;
  unsigned int uVar3;
  int iVar4;
  FILE *pFVar5;
  unsigned int uVar6;
  int iVar7;
  unsigned int uVar8;
  int iVar9;
  size_t sVar10;
  char *pcVar11;
  int iVar12;
  unsigned int *puVar13;
  unsigned int *puVar14;
  char *pcVar15;
  char *pcVar16;
  int iVar17;
  unsigned int local_48;
  int local_38;
  unsigned int local_34 [5];
  char local_20 [32];

  

  uVar8 = 0;
  iVar4 = 0;

  do {
    pbVar1 = (byte *)(param_2 + iVar4);
    iVar4 = iVar4 + 1;
    iVar9 = uVar8 + *pbVar1;
    uVar6 = iVar9 * 2;
    uVar3 = (unsigned int)(iVar9 < 0);
    uVar8 = uVar6 | uVar3;
  } while (iVar4 < 0x300);
  sVar10 = 0;
  local_34[0] = 0xffffffff;
  local_20[0] = '\0';
  pcVar11 = &DAT_0049c9d8;

  do {
    pcVar16 = pcVar11;
    if (local_34[0] == 0) break;
    local_34[0] = local_34[0] - 1;
    pcVar16 = pcVar11 + 1;
    cVar2 = *pcVar11;
    pcVar11 = pcVar16;
  } while (cVar2 != '\0');
  local_34[0] = ~local_34[0];
  iVar4 = -1;
  pcVar11 = local_20;

  do {
    pcVar15 = pcVar11;
    if (iVar4 == 0) break;
    iVar4 = iVar4 + -1;
    pcVar15 = pcVar11 + 1;
    cVar2 = *pcVar11;
    pcVar11 = pcVar15;
  } while (cVar2 != '\0');
  pcVar11 = pcVar16 + -local_34[0];
  pcVar16 = pcVar15 + -1;

  for (uVar8 = local_34[0] >> 2; uVar8 != 0; uVar8 = uVar8 - 1) {
    *(int *)pcVar16 = *(int *)pcVar11;
    pcVar11 = pcVar11 + 4;
    pcVar16 = pcVar16 + 4;
  }

  for (uVar8 = local_34[0] & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
    *pcVar16 = *pcVar11;
    pcVar11 = pcVar11 + 1;
    pcVar16 = pcVar16 + 1;
  }

  __ultoa(uVar6 & 0xffff | uVar3,(char *)local_34,0x10);
  uVar8 = 0xffffffff;
  puVar13 = local_34;

  do {
    puVar14 = puVar13;
    if (uVar8 == 0) break;
    uVar8 = uVar8 - 1;
    puVar14 = (unsigned int *)((int)puVar13 + 1);
    uVar6 = *puVar13;
    puVar13 = puVar14;
  } while ((char)uVar6 != '\0');
  uVar8 = ~uVar8;
  iVar4 = -1;
  pcVar11 = local_20;

  do {
    pcVar16 = pcVar11;
    if (iVar4 == 0) break;
    iVar4 = iVar4 + -1;
    pcVar16 = pcVar11 + 1;
    cVar2 = *pcVar11;
    pcVar11 = pcVar16;
  } while (cVar2 != '\0');
  pcVar11 = (char *)((int)puVar14 - uVar8);
  pcVar16 = pcVar16 + -1;

  for (uVar6 = uVar8 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
    *(int *)pcVar16 = *(int *)pcVar11;
    pcVar11 = pcVar11 + 4;
    pcVar16 = pcVar16 + 4;
  }

  for (uVar8 = uVar8 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
    *pcVar16 = *pcVar11;
    pcVar11 = pcVar11 + 1;
    pcVar16 = pcVar16 + 1;
  }

  uVar8 = 0xffffffff;
  pcVar11 = (char *)&DAT_0049c9d0;

  do {
    pcVar16 = pcVar11;
    if (uVar8 == 0) break;
    uVar8 = uVar8 - 1;
    pcVar16 = pcVar11 + 1;
    cVar2 = *pcVar11;
    pcVar11 = pcVar16;
  } while (cVar2 != '\0');
  uVar8 = ~uVar8;
  iVar4 = -1;
  pcVar11 = local_20;

  do {
    pcVar15 = pcVar11;
    if (iVar4 == 0) break;
    iVar4 = iVar4 + -1;
    pcVar15 = pcVar11 + 1;
    cVar2 = *pcVar11;
    pcVar11 = pcVar15;
  } while (cVar2 != '\0');
  pcVar11 = pcVar16 + -uVar8;
  pcVar16 = pcVar15 + -1;

  for (uVar6 = uVar8 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
    *(int *)pcVar16 = *(int *)pcVar11;
    pcVar11 = pcVar11 + 4;
    pcVar16 = pcVar16 + 4;
  }

  for (uVar8 = uVar8 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
    *pcVar16 = *pcVar11;
    pcVar11 = pcVar11 + 1;
    pcVar16 = pcVar16 + 1;
  }

  uVar8 = 0xffffffff;
  pcVar11 = (char *)&DAT_0049c9c8;

  do {
    pcVar16 = pcVar11;
    if (uVar8 == 0) break;
    uVar8 = uVar8 - 1;
    pcVar16 = pcVar11 + 1;
    cVar2 = *pcVar11;
    pcVar11 = pcVar16;
  } while (cVar2 != '\0');
  uVar8 = ~uVar8;
  iVar4 = -1;
  pcVar11 = local_20;

  do {
    pcVar15 = pcVar11;
    if (iVar4 == 0) break;
    iVar4 = iVar4 + -1;
    pcVar15 = pcVar11 + 1;
    cVar2 = *pcVar11;
    pcVar11 = pcVar15;
  } while (cVar2 != '\0');
  pcVar11 = pcVar16 + -uVar8;
  pcVar16 = pcVar15 + -1;

  for (uVar6 = uVar8 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
    *(int *)pcVar16 = *(int *)pcVar11;
    pcVar11 = pcVar11 + 4;
    pcVar16 = pcVar16 + 4;
  }

  for (uVar8 = uVar8 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
    *pcVar16 = *pcVar11;
    pcVar11 = pcVar11 + 1;
    pcVar16 = pcVar16 + 1;
  }

  pFVar5 = (FILE *)FUN_00469390(local_20,&DAT_0047c040);

  if (pFVar5 != (FILE *)0x0) {
    sVar10 = _fread(&DAT_004cdd18,1,0x40000,pFVar5);
    _fclose(pFVar5);
  }

  if (sVar10 != 0x40000) {
    local_48 = 0;

    do {
      iVar7 = 0;
      iVar4 = 0x7f000000;
      iVar9 = 0;

      do {
        iVar12 = ((local_48 & 0x3f000) >> 10) - (unsigned int)*(byte *)(iVar7 + param_2);
        iVar12 = iVar12 * iVar12;

        if (((iVar12 < iVar4) &&

            (iVar17 = ((local_48 & 0xfc0) >> 4) - (unsigned int)*(byte *)(iVar7 + 1 + param_2),

            iVar12 = iVar12 + iVar17 * iVar17, iVar12 < iVar4)) &&

           (iVar17 = (local_48 & 0x3f) * 4 - (unsigned int)*(byte *)(iVar7 + 2 + param_2),

           iVar12 = iVar12 + iVar17 * iVar17, iVar12 < iVar4)) {
          iVar4 = iVar12;
          local_38 = iVar9;
        }

        iVar7 = iVar7 + 3;
        iVar9 = iVar9 + 1;
      } while (iVar9 < 0x100);
      uVar8 = local_48 + 1;
      ((int*)&(DAT_004cdd18))[local_48] = (char)local_38;
      local_48 = uVar8;
    } while ((int)uVar8 < 0x40000);
    pFVar5 = (FILE *)FUN_00469390(local_20,&DAT_0049c9c4);

    if (pFVar5 != (FILE *)0x0) {
      _fwrite(&DAT_004cdd18,1,0x40000,pFVar5);
      _fclose(pFVar5);
    }

  }

  return;
}

/**
 * @original Lisa_FindClosestPaletteColor (IGN_WIN.EXE @ 0x00448860, lisa3d.c)
 * @fidelity ADAPTED
 */
unsigned int Lisa_FindClosestPaletteColor(int *param_1,int param_2) {
  unsigned int uVar1;
  int iVar2;
  int iVar3;
  unsigned int local_c;
  int local_8;
  unsigned int local_4;

  

  local_8 = 0x7f000000;
  iVar2 = 0;
  local_4 = 0;
  uVar1 = local_c;

  do {
    iVar3 = *param_1 - (unsigned int)*(byte *)(param_2 + iVar2);
    iVar3 = iVar3 * iVar3;

    if (iVar3 < local_8) {
      local_c = (unsigned int)*(byte *)(param_2 + 1 + iVar2);
      iVar3 = iVar3 + (param_1[1] - local_c) * (param_1[1] - local_c);

      if (iVar3 < local_8) {
        local_c = (unsigned int)*(byte *)(param_2 + 2 + iVar2);
        iVar3 = iVar3 + (param_1[2] - local_c) * (param_1[2] - local_c);

        if (iVar3 < local_8) {
          uVar1 = local_4;
          local_8 = iVar3;
        }

      }

    }

    iVar2 = iVar2 + 3;
    local_4 = local_4 + 1;
  } while ((int)local_4 < 0x100);
  return uVar1;
}

/**
 * @original Lisa_RenderSkyBackdrop (IGN_WIN.EXE @ 0x00448990, lisa3d.c)
 * @fidelity ADAPTED
 */
unsigned long long Lisa_RenderSkyBackdrop(void) {
  float fVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  unsigned int uVar6;
  double extraout_ST0;
  double extraout_ST1;
  long long lVar7;
  unsigned long long uVar8;

  

  iVar2 = g_LisaCamera;
  lVar7 = __ftol();
  iVar3 = (int)lVar7;
  lVar7 = __ftol();
  iVar4 = (int)lVar7;
  lVar7 = __ftol();
  uVar6 = (unsigned int)((unsigned long long)lVar7 >> 0x20);
  iVar5 = (int)lVar7;
  fVar1 = (float)SQRT(extraout_ST1 * extraout_ST1 + extraout_ST0 * extraout_ST0);

  if ((iVar4 < 1) || ((int)fVar1 < 1)) {
    if ((iVar4 < 0) && (0 < (int)fVar1)) {
      fpatan((double)fVar1 / (double)-iVar4,(double)1);
      lVar7 = __ftol();
      *(double *)(iVar2 + 0x18) = (double)(int)lVar7;
    }

    else if (iVar4 == 0) {
      *(int *)(iVar2 + 0x18) = 0;
      *(int *)(iVar2 + 0x1c) = 0;
      lVar7 = (unsigned long long)uVar6 << 0x20;
    }

    else if ((iVar4 == 0) && (ABS(fVar1) == 0.0)) {
      *(int *)(iVar2 + 0x18) = 0;
      *(int *)(iVar2 + 0x1c) = 0;
      lVar7 = (unsigned long long)uVar6 << 0x20;
    }

    else if ((iVar4 < 0) && (ABS(fVar1) == 0.0)) {
      *(int *)(iVar2 + 0x18) = 0;
      *(int *)(iVar2 + 0x1c) = 0x40a51800;
      lVar7 = (((long long)(uVar6) << 32) | ((unsigned int)(fVar1)));
    }

    else {
      lVar7 = (unsigned long long)uVar6 << 0x20;

      if (0 < iVar4) {
        lVar7 = (((long long)(uVar6) << 32) | ((unsigned int)(fVar1)));

        if (ABS(fVar1) == 0.0) {
          *(int *)(iVar2 + 0x18) = 0;
          *(int *)(iVar2 + 0x1c) = 0x408c2000;
          lVar7 = (((long long)(uVar6) << 32) | ((unsigned int)(fVar1)));
        }

      }

    }

  }

  else {
    fpatan((double)iVar4 / (double)fVar1,(double)1);
    lVar7 = __ftol();
    *(double *)(iVar2 + 0x18) = (double)(int)lVar7;
  }

  uVar6 = (unsigned int)((unsigned long long)lVar7 >> 0x20);

  if ((iVar3 < 1) || (iVar5 < 1)) {
    if ((iVar5 < 0) && (0 < iVar3)) {
      fpatan((double)-iVar5 / (double)iVar3,(double)1);
      uVar8 = __ftol();
      *(double *)(iVar2 + 0x20) = (double)(int)uVar8;
    }

    else if ((iVar3 < 0) && (iVar5 < 0)) {
      fpatan((double)iVar3 / (double)iVar5,(double)1);
      uVar8 = __ftol();
      *(double *)(iVar2 + 0x20) = (double)(int)uVar8;
    }

    else if ((iVar5 < 1) || (-1 < iVar3)) {
      if (iVar3 == 0) {
        if (0 < iVar5) {
          *(int *)(iVar2 + 0x20) = 0;
          *(int *)(iVar2 + 0x24) = 0;
          uVar8 = (unsigned long long)uVar6 << 0x20;
          goto LAB_00448c11;
        }

        if (iVar5 < 0) {
          *(int *)(iVar2 + 0x20) = 0;
          *(int *)(iVar2 + 0x24) = 0x409c2000;
          uVar8 = (unsigned long long)uVar6 << 0x20;
          goto LAB_00448c11;
        }

      }

      if ((iVar5 == 0) && (0 < iVar3)) {
        *(int *)(iVar2 + 0x20) = 0;
        *(int *)(iVar2 + 0x24) = 0x408c2000;
        uVar8 = (unsigned long long)uVar6 << 0x20;
      }

      else {
        uVar8 = (unsigned long long)uVar6 << 0x20;

        if ((iVar5 == 0) && (uVar8 = (unsigned long long)uVar6 << 0x20, iVar3 < 0)) {
          *(int *)(iVar2 + 0x20) = 0;
          *(int *)(iVar2 + 0x24) = 0x40a51800;
          uVar8 = (unsigned long long)uVar6 << 0x20;
        }

      }

    }

    else {
      fpatan((double)iVar5 / (double)-iVar3,(double)1);
      uVar8 = __ftol();
      *(double *)(iVar2 + 0x20) = (double)(int)uVar8;
    }

  }

  else {
    fpatan((double)iVar3 / (double)iVar5,(double)1);
    uVar8 = __ftol();
    *(double *)(iVar2 + 0x20) = (double)(int)uVar8;
  }

LAB_00448c11:

  *(double *)(iVar2 + 0x28) = (double)*(int *)(iVar2 + 0x7c);
  return uVar8 & 0xffffffff00000000;
}

/**
 * @original Lisa_CullObjectsOrthographic (IGN_WIN.EXE @ 0x00448c30, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_CullObjectsOrthographic(void) {
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int *piVar5;
  int *puVar6;
  int iVar7;
  int iVar8;
  long long lVar9;
  long long lVar10;
  long long lVar11;
  int local_c;
  int local_4;

  

  iVar3 = g_LisaCamera;
  g_LisaCamera->visible_obj_count = 0;
  lVar9 = __ftol();
  lVar10 = __ftol();
  lVar11 = __ftol();
  iVar7 = ((int)lVar9 * 0x24 + (int)lVar10) * 8;

  iVar4 = (*(int *)(&DAT_0049a05c + iVar7) +

           ((int)((int)lVar11 + ((int)lVar11 >> 0x1f & 0xffU)) >> 8) + DAT_00499fa4) * DAT_0063c5b8;
  lVar9 = __ftol();
  iVar2 = g_LisaVisibleObjects;
  local_c = 3;

  iVar4 = DAT_0063b5ec +

          (iVar4 + ((int)((int)lVar9 + ((int)lVar9 >> 0x1f & 0xffU)) >> 8) +

           *(int *)(&DAT_0049a058 + iVar7) + DAT_00499fa8) * 4;
  iVar7 = DAT_00499fac;

  if (DAT_0063c5d8 == 0) {
    while (iVar7 != -5000) {
      iVar7 = *(int *)(local_c * 4 + 0x499fa0);

      if (0 < iVar7) {
        do {
          piVar5 = *(int **)(iVar4 + 4);
          iVar4 = iVar4 + 4;

          if ((piVar5 != (int *)0x0) && ((int *)*piVar5 == piVar5)) {
            iVar8 = *(int *)(iVar3 + 0x60) + 1;
            iVar1 = *(int *)((int)piVar5 + 0x26);
            *(int *)(iVar3 + 0x60) = iVar8;
            *(int **)(iVar2 + -4 + iVar8 * 4) = piVar5;

            if (iVar1 != 0) {
              puVar6 = (int *)(iVar2 + iVar8 * 4);

              do {
                piVar5 = *(int **)((int)piVar5 + 0x26);

                if ((int *)*piVar5 == piVar5) {
                  *puVar6 = piVar5;
                  puVar6 = puVar6 + 1;
                  *(int *)(iVar3 + 0x60) = *(int *)(iVar3 + 0x60) + 1;
                }

              } while (*(int *)((int)piVar5 + 0x26) != 0);
            }

          }

          iVar7 = iVar7 + -1;
        } while (iVar7 != 0);
      }

      iVar7 = local_c + 1;
      local_c = local_c + 2;
      iVar4 = iVar4 + (*(int *)(iVar7 * 4 + 0x499fa0) + DAT_0063c5b8) * 4;
      iVar7 = *(int *)(local_c * 4 + 0x499fa0);
    }

  }

  else {
    while (iVar7 != -5000) {
      local_4 = *(int *)(local_c * 4 + 0x499fa0);

      if (0 < local_4) {
        do {
          piVar5 = *(int **)(iVar4 + 4);
          iVar4 = iVar4 + 4;

          if ((piVar5 != (int *)0x0) && ((int *)*piVar5 == piVar5)) {
            if (((short)piVar5[9] == 0) || ((short)piVar5[9] == DAT_0063c5d8)) {
              iVar7 = *(int *)(iVar3 + 0x60) + 1;
              *(int *)(iVar3 + 0x60) = iVar7;
              *(int **)(iVar2 + -4 + iVar7 * 4) = piVar5;
            }

            if (*(int *)((int)piVar5 + 0x26) != 0) {
              puVar6 = (int *)(iVar2 + *(int *)(iVar3 + 0x60) * 4);

              do {
                piVar5 = *(int **)((int)piVar5 + 0x26);

                if (((int *)*piVar5 == piVar5) &&

                   (((short)piVar5[9] == 0 || ((short)piVar5[9] == DAT_0063c5d8)))) {
                  *puVar6 = piVar5;
                  puVar6 = puVar6 + 1;
                  *(int *)(iVar3 + 0x60) = *(int *)(iVar3 + 0x60) + 1;
                }

              } while (*(int *)((int)piVar5 + 0x26) != 0);
            }

          }

          local_4 = local_4 + -1;
        } while (local_4 != 0);
      }

      iVar7 = local_c + 1;
      local_c = local_c + 2;
      iVar4 = iVar4 + (*(int *)(iVar7 * 4 + 0x499fa0) + DAT_0063c5b8) * 4;
      iVar7 = *(int *)(local_c * 4 + 0x499fa0);
    }

  }

  *(int *)(iVar3 + 100) = *(int *)(iVar3 + 0x60);
  return 0;
}

/**
 * @original Lisa_FrustumCullObjects (IGN_WIN.EXE @ 0x00448e70, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_FrustumCullObjects(void) {
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int *piVar6;
  int *piVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int iVar12;
  int iVar13;
  int *piVar14;
  int iVar15;
  int iVar16;
  int iVar17;
  int iVar18;
  int *piVar19;
  int iVar20;
  int iVar21;
  int iVar22;
  double extraout_ST1;
  long long lVar23;
  long long lVar24;
  long long lVar25;
  int local_64;
  int local_60;
  int local_5c;
  int *local_48;
  int *local_44;
  int local_38;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  int local_4;

  

  iVar2 = g_LisaCamera;
  g_LisaCamera->visible_obj_count = 0;

  if (*(int *)(iVar2 + 0xa4) == 0) {
    lVar23 = __ftol();
    lVar24 = __ftol();
    lVar25 = __ftol();
    iVar4 = ((int)lVar23 * 0x24 + (int)lVar24) * 8;

    iVar5 = (*(int *)(&DAT_0049a05c + iVar4) +

             ((int)((int)lVar25 + ((int)lVar25 >> 0x1f & 0xffU)) >> 8) + DAT_00499fa4) *

            DAT_0063c5b8;
    lVar23 = __ftol();
    iVar17 = DAT_0063c5d8;
    piVar14 = g_LisaVisibleObjects;
    local_64 = 3;

    iVar5 = DAT_0063b5ec +

            (iVar5 + ((int)((int)lVar23 + ((int)lVar23 >> 0x1f & 0xffU)) >> 8) +

             *(int *)(&DAT_0049a058 + iVar4) + DAT_00499fa8) * 4;
    iVar4 = DAT_00499fac;

    if (DAT_0063c5d8 == 0) {
      while (iVar4 != -5000) {
        iVar4 = *(int *)(local_64 * 4 + 0x499fa0);

        if (0 < iVar4) {
          do {
            piVar19 = *(int **)(iVar5 + 4);
            iVar5 = iVar5 + 4;

            if ((piVar19 != (int *)0x0) && ((int *)*piVar19 == piVar19)) {
              iVar17 = *(int *)(iVar2 + 0x60);
              iVar18 = iVar17 + 1;
              iVar1 = *(int *)((int)piVar19 + 0x26);
              *(int *)(iVar2 + 0x60) = iVar18;
              piVar14[iVar17] = (int)piVar19;

              if (iVar1 != 0) {
                piVar7 = piVar14 + iVar18;

                do {
                  piVar19 = *(int **)((int)piVar19 + 0x26);

                  if ((int *)*piVar19 == piVar19) {
                    *piVar7 = (int)piVar19;
                    piVar7 = piVar7 + 1;
                    *(int *)(iVar2 + 0x60) = *(int *)(iVar2 + 0x60) + 1;
                  }

                } while (*(int *)((int)piVar19 + 0x26) != 0);
              }

            }

            iVar4 = iVar4 + -1;
          } while (iVar4 != 0);
        }

        iVar4 = local_64 + 1;
        local_64 = local_64 + 2;
        iVar5 = iVar5 + (*(int *)(iVar4 * 4 + 0x499fa0) + DAT_0063c5b8) * 4;
        iVar4 = *(int *)(local_64 * 4 + 0x499fa0);
      }

    }

    else {
      while (iVar4 != -5000) {
        local_60 = *(int *)(local_64 * 4 + 0x499fa0);

        if (0 < local_60) {
          do {
            piVar19 = *(int **)(iVar5 + 4);
            iVar5 = iVar5 + 4;

            if ((piVar19 != (int *)0x0) && ((int *)*piVar19 == piVar19)) {
              if (((short)piVar19[9] == 0) || ((short)piVar19[9] == iVar17)) {
                iVar4 = *(int *)(iVar2 + 0x60);
                *(int *)(iVar2 + 0x60) = iVar4 + 1;
                piVar14[iVar4] = (int)piVar19;
              }

              if (*(int *)((int)piVar19 + 0x26) != 0) {
                piVar7 = piVar14 + *(int *)(iVar2 + 0x60);

                do {
                  piVar19 = *(int **)((int)piVar19 + 0x26);

                  if (((int *)*piVar19 == piVar19) &&

                     (((short)piVar19[9] == 0 || ((short)piVar19[9] == iVar17)))) {
                    *piVar7 = (int)piVar19;
                    piVar7 = piVar7 + 1;
                    *(int *)(iVar2 + 0x60) = *(int *)(iVar2 + 0x60) + 1;
                  }

                } while (*(int *)((int)piVar19 + 0x26) != 0);
              }

            }

            local_60 = local_60 + -1;
          } while (local_60 != 0);
        }

        iVar4 = local_64 + 1;
        local_64 = local_64 + 2;
        iVar5 = iVar5 + (*(int *)(iVar4 * 4 + 0x499fa0) + DAT_0063c5b8) * 4;
        iVar4 = *(int *)(local_64 * 4 + 0x499fa0);
      }

    }

  }

  else {
    fcos((double)*(double *)(iVar2 + 0x20) * (double)_DAT_0047ae48);
    iVar4 = *(int *)(iVar2 + 0xa4) / 2;
    lVar23 = __ftol();
    lVar24 = __ftol();
    fsin(extraout_ST1);

    iVar17 = (((int)lVar23 + ((int)((int)lVar24 + ((int)lVar24 >> 0x1f & 0xffU)) >> 8)) - iVar4) *

             DAT_0063c5b8;
    lVar23 = __ftol();
    lVar24 = __ftol();
    iVar5 = DAT_0063c5d8;
    piVar19 = g_LisaVisibleObjects;

    piVar14 = (int *)(DAT_0063b5ec +

                     ((iVar17 + (int)lVar23 +

                      ((int)((int)lVar24 + ((int)lVar24 >> 0x1f & 0xffU)) >> 8)) - iVar4) * 4);

    if (DAT_0063c5d8 == 0) {
      if (0 < *(int *)(iVar2 + 0xa4)) {
        local_64 = *(int *)(iVar2 + 0xa4);
        iVar5 = DAT_0063c5b8 - local_64;

        do {
          iVar4 = *(int *)(iVar2 + 0xa4);

          if (0 < iVar4) {
            do {
              piVar7 = (int *)*piVar14;

              if ((piVar7 != (int *)0x0) && ((int *)*piVar7 == piVar7)) {
                iVar17 = *(int *)(iVar2 + 0x60);
                iVar18 = iVar17 + 1;
                iVar1 = *(int *)((int)piVar7 + 0x26);
                *(int *)(iVar2 + 0x60) = iVar18;
                piVar19[iVar17] = (int)piVar7;

                if (iVar1 != 0) {
                  piVar6 = piVar19 + iVar18;

                  do {
                    piVar7 = *(int **)((int)piVar7 + 0x26);

                    if ((int *)*piVar7 == piVar7) {
                      *piVar6 = (int)piVar7;
                      piVar6 = piVar6 + 1;
                      *(int *)(iVar2 + 0x60) = *(int *)(iVar2 + 0x60) + 1;
                    }

                  } while (*(int *)((int)piVar7 + 0x26) != 0);
                }

              }

              piVar14 = piVar14 + 1;
              iVar4 = iVar4 + -1;
            } while (iVar4 != 0);
          }

          piVar14 = piVar14 + iVar5;
          local_64 = local_64 + -1;
        } while (local_64 != 0);
      }

    }

    else if (0 < *(int *)(iVar2 + 0xa4)) {
      local_5c = *(int *)(iVar2 + 0xa4);
      iVar4 = DAT_0063c5b8 - local_5c;

      do {
        local_64 = *(int *)(iVar2 + 0xa4);

        if (0 < local_64) {
          do {
            piVar7 = (int *)*piVar14;

            if ((piVar7 != (int *)0x0) && ((int *)*piVar7 == piVar7)) {
              if (((short)piVar7[9] == 0) || ((short)piVar7[9] == iVar5)) {
                iVar17 = *(int *)(iVar2 + 0x60);
                *(int *)(iVar2 + 0x60) = iVar17 + 1;
                piVar19[iVar17] = (int)piVar7;
              }

              if (*(int *)((int)piVar7 + 0x26) != 0) {
                piVar6 = piVar19 + *(int *)(iVar2 + 0x60);

                do {
                  piVar7 = *(int **)((int)piVar7 + 0x26);

                  if (((int *)*piVar7 == piVar7) &&

                     (((short)piVar7[9] == 0 || ((short)piVar7[9] == iVar5)))) {
                    *piVar6 = (int)piVar7;
                    piVar6 = piVar6 + 1;
                    *(int *)(iVar2 + 0x60) = *(int *)(iVar2 + 0x60) + 1;
                  }

                } while (*(int *)((int)piVar7 + 0x26) != 0);
              }

            }

            piVar14 = piVar14 + 1;
            local_64 = local_64 + -1;
          } while (local_64 != 0);
        }

        piVar14 = piVar14 + iVar4;
        local_5c = local_5c + -1;
      } while (local_5c != 0);
    }

  }

  iVar5 = *(int *)(iVar2 + 0x80);
  iVar4 = *(int *)(iVar2 + 0x88);
  iVar17 = *(int *)(iVar2 + 0x9c);
  iVar1 = *(int *)(iVar2 + 0x84);
  iVar2 = *(int *)(iVar2 + 0xa0);
  iVar8 = g_SubpixelMinX >> 8;
  iVar9 = g_SubpixelMaxX >> 8;
  iVar10 = g_SubpixelMinY >> 8;
  iVar11 = g_SubpixelMaxY >> 8;
  Lisa_ComputeCameraRotationMatrix(&local_24);
  iVar18 = g_LisaCamera;
  local_38 = g_LisaCamera->visible_obj_count;
  g_LisaCamera->submesh_count = local_38;
  *(int *)(iVar18 + 0x60) = 0;

  if (0 < local_38) {
    local_48 = g_LisaVisibleObjects;
    local_44 = g_LisaVisibleObjects;

    do {
      iVar3 = *local_44;
      lVar23 = __ftol();
      iVar21 = (int)lVar23;
      lVar23 = __ftol();
      iVar12 = (int)lVar23;
      lVar23 = __ftol();
      iVar13 = (int)lVar23;
      iVar16 = (int)*(short *)(iVar3 + 0x22);
      iVar15 = (iVar13 * local_4 + iVar12 * local_8 + iVar21 * local_c >> 0xf) + iVar4;

      if (-1 < iVar15 + iVar16) {
        if (iVar15 < iVar4) {
          iVar15 = iVar4;
        }

        iVar20 = iVar17 + ((iVar13 * local_1c + iVar12 * local_20 + iVar21 * local_24 >> 0xf) *

                          -iVar5) / iVar15;
        iVar22 = 2 - (-iVar5 * iVar16) / iVar15;

        if ((iVar8 <= iVar22 + iVar20) || (iVar20 - iVar22 <= iVar9)) {
          iVar21 = iVar2 + ((iVar13 * local_10 + iVar12 * local_14 + iVar21 * local_18 >> 0xf) *

                           -iVar1) / iVar15;
          iVar15 = 2 - (-iVar1 * iVar16) / iVar15;

          if ((iVar10 < iVar21 + iVar15) && (iVar21 - iVar15 < iVar11)) {
            *(int *)(iVar18 + 0x60) = *(int *)(iVar18 + 0x60) + 1;
            *local_48 = iVar3;
            local_48 = local_48 + 1;
          }

        }

      }

      local_44 = local_44 + 1;
      local_38 = local_38 + -1;
    } while (local_38 != 0);
  }

  return 0;
}

/**
 * @original Lisa_CullObjects (IGN_WIN.EXE @ 0x00449470, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_CullObjects(void) {
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  int iVar9;
  int *piVar10;
  int *piVar11;
  unsigned int uVar12;
  unsigned int uVar13;
  int *piVar14;
  int *piVar15;
  float *pfVar16;
  float *pfVar17;
  int iVar18;
  int iVar19;
  int *piVar20;
  int iVar21;
  int iVar22;
  int *puVar23;
  double fVar24;
  double fVar25;
  double fVar26;
  double fVar27;
  double fVar28;
  double fVar29;
  double extraout_ST1;
  long long lVar30;
  long long lVar31;
  long long lVar32;
  int local_70;
  int *local_6c;
  int local_68;
  int *local_64;
  double *local_58;
  float local_50 [19];
  float local_4;

  

  iVar22 = g_LisaCamera;
  iVar19 = g_LisaCamera->projection_type;

  if (iVar19 == 0) {
    g_LisaCamera->visible_obj_count = 0;
    piVar15 = (int *)(iVar22 + 0x60);
    local_58 = &g_LisaCamera->rot_y;
    lVar30 = __ftol();
    lVar31 = __ftol();
    iVar19 = ((int)lVar30 * 0x24 + (int)lVar31) * 8;
    lVar30 = __ftol();

    iVar9 = (*(int *)(&DAT_0049a05c + iVar19) +

             ((int)((int)lVar30 + ((int)lVar30 >> 0x1f & 0xffU)) >> 8) + DAT_00499fa4) *

            DAT_0063c5b8;
    lVar30 = __ftol();
    iVar18 = DAT_0063c5d8;
    piVar20 = g_LisaVisibleObjects;
    local_70 = 3;

    iVar19 = DAT_0063b5ec +

             (iVar9 + ((int)((int)lVar30 + ((int)lVar30 >> 0x1f & 0xffU)) >> 8) +

              *(int *)(&DAT_0049a058 + iVar19) + DAT_00499fa8) * 4;
    iVar9 = DAT_00499fac;

    if (DAT_0063c5d8 == 0) {
      while (iVar9 != -5000) {
        iVar9 = *(int *)(local_70 * 4 + 0x499fa0);

        if (0 < iVar9) {
          do {
            piVar14 = *(int **)(iVar19 + 4);
            iVar19 = iVar19 + 4;

            if ((piVar14 != (int *)0x0) && ((int *)*piVar14 == piVar14)) {
              iVar18 = *piVar15;
              piVar20[iVar18] = (int)piVar14;
              iVar18 = iVar18 + 1;
              *piVar15 = iVar18;

              if (*(int *)((int)piVar14 + 0x26) != 0) {
                piVar11 = piVar20 + iVar18;

                do {
                  piVar14 = *(int **)((int)piVar14 + 0x26);

                  if ((int *)*piVar14 == piVar14) {
                    *piVar11 = (int)piVar14;
                    piVar11 = piVar11 + 1;
                    *piVar15 = *piVar15 + 1;
                  }

                } while (*(int *)((int)piVar14 + 0x26) != 0);
              }

            }

            iVar9 = iVar9 + -1;
          } while (iVar9 != 0);
        }

        iVar9 = local_70 + 1;
        local_70 = local_70 + 2;
        iVar19 = iVar19 + (*(int *)(iVar9 * 4 + 0x499fa0) + DAT_0063c5b8) * 4;
        iVar9 = *(int *)(local_70 * 4 + 0x499fa0);
      }

    }

    else {
      while (iVar9 != -5000) {
        local_50[0] = *(float *)(local_70 * 4 + 0x499fa0);

        if (0 < (int)local_50[0]) {
          do {
            piVar14 = *(int **)(iVar19 + 4);
            iVar19 = iVar19 + 4;

            if ((piVar14 != (int *)0x0) && ((int *)*piVar14 == piVar14)) {
              if (((short)piVar14[9] == 0) || ((short)piVar14[9] == iVar18)) {
                iVar9 = *piVar15;
                piVar20[iVar9] = (int)piVar14;
                *piVar15 = iVar9 + 1;
              }

              if (*(int *)((int)piVar14 + 0x26) != 0) {
                piVar11 = piVar20 + *piVar15;

                do {
                  piVar14 = *(int **)((int)piVar14 + 0x26);

                  if (((int *)*piVar14 == piVar14) &&

                     (((short)piVar14[9] == 0 || ((short)piVar14[9] == iVar18)))) {
                    *piVar11 = (int)piVar14;
                    piVar11 = piVar11 + 1;
                    *piVar15 = *piVar15 + 1;
                  }

                } while (*(int *)((int)piVar14 + 0x26) != 0);
              }

            }

            local_50[0] = (float)((int)local_50[0] + -1);
          } while (local_50[0] != 0.0);
        }

        iVar9 = local_70 + 1;
        local_70 = local_70 + 2;
        iVar19 = iVar19 + (*(int *)(iVar9 * 4 + 0x499fa0) + DAT_0063c5b8) * 4;
        iVar9 = *(int *)(local_70 * 4 + 0x499fa0);
      }

    }

  }

  else {
    g_LisaCamera->visible_obj_count = 0;
    piVar14 = (int *)(iVar22 + 0x60);
    local_58 = &g_LisaCamera->rot_y;
    local_50[0] = (float)(iVar19 / 3);
    fcos((double)*local_58 * (double)_DAT_0047ae48);
    lVar30 = __ftol();
    lVar31 = __ftol();
    fsin(extraout_ST1);

    iVar18 = (((int)lVar30 + ((int)((int)lVar31 + ((int)lVar31 >> 0x1f & 0xffU)) >> 8)) - iVar19 / 2

             ) * DAT_0063c5b8;
    lVar30 = __ftol();
    lVar31 = __ftol();
    iVar9 = DAT_0063c5d8;
    piVar20 = g_LisaVisibleObjects;

    piVar15 = (int *)(DAT_0063b5ec +

                     ((iVar18 + (int)lVar30 +

                      ((int)((int)lVar31 + ((int)lVar31 >> 0x1f & 0xffU)) >> 8)) - iVar19 / 2) * 4);
    local_68 = iVar19;

    if (DAT_0063c5d8 == 0) {
      if (0 < iVar19) {
        iVar9 = DAT_0063c5b8 - iVar19;

        do {
          iVar18 = iVar19;

          if (0 < iVar19) {
            do {
              piVar11 = (int *)*piVar15;

              if ((piVar11 != (int *)0x0) && ((int *)*piVar11 == piVar11)) {
                iVar21 = *piVar14;
                piVar20[iVar21] = (int)piVar11;
                iVar21 = iVar21 + 1;
                *piVar14 = iVar21;

                if (*(int *)((int)piVar11 + 0x26) != 0) {
                  piVar10 = piVar20 + iVar21;

                  do {
                    piVar11 = *(int **)((int)piVar11 + 0x26);

                    if ((int *)*piVar11 == piVar11) {
                      *piVar10 = (int)piVar11;
                      piVar10 = piVar10 + 1;
                      *piVar14 = *piVar14 + 1;
                    }

                  } while (*(int *)((int)piVar11 + 0x26) != 0);
                }

              }

              piVar15 = piVar15 + 1;
              iVar18 = iVar18 + -1;
            } while (iVar18 != 0);
          }

          piVar15 = piVar15 + iVar9;
          local_68 = local_68 + -1;
        } while (local_68 != 0);
      }

    }

    else if (0 < iVar19) {
      iVar18 = DAT_0063c5b8 - iVar19;

      do {
        local_64 = (int *)iVar19;

        if (0 < iVar19) {
          do {
            piVar11 = (int *)*piVar15;

            if ((piVar11 != (int *)0x0) && ((int *)*piVar11 == piVar11)) {
              if (((short)piVar11[9] == 0) || ((short)piVar11[9] == iVar9)) {
                iVar21 = *piVar14;
                piVar20[iVar21] = (int)piVar11;
                *piVar14 = iVar21 + 1;
              }

              if (*(int *)((int)piVar11 + 0x26) != 0) {
                piVar10 = piVar20 + *piVar14;

                do {
                  piVar11 = *(int **)((int)piVar11 + 0x26);

                  if (((int *)*piVar11 == piVar11) &&

                     (((short)piVar11[9] == 0 || ((short)piVar11[9] == iVar9)))) {
                    *piVar10 = (int)piVar11;
                    piVar10 = piVar10 + 1;
                    *piVar14 = *piVar14 + 1;
                  }

                } while (*(int *)((int)piVar11 + 0x26) != 0);
              }

            }

            piVar15 = piVar15 + 1;
            local_64 = (int *)((int)local_64 + -1);
          } while (local_64 != (int *)0x0);
        }

        piVar15 = piVar15 + iVar18;
        local_68 = local_68 + -1;
      } while (local_68 != 0);
    }

  }

  local_6c = (int *)(iVar22 + 0x60);
  iVar19 = g_LisaCamera->fov_x;
  iVar22 = g_SubpixelMinX + iVar19 * -0x100;

  if ((((DAT_0050dd88 != iVar22) || (g_SubpixelMaxX + iVar19 * -0x100 != DAT_0050dd68)) ||

      (g_SubpixelMinY + g_LisaCamera->fov_y * -0x100 != DAT_0050dd2c)) ||

     (((g_SubpixelMaxY + g_LisaCamera->fov_y * -0x100 != DAT_004cdc24 ||

       (g_LisaCamera->viewport_x != DAT_0050dda4)) ||

      ((g_LisaCamera->viewport_y != DAT_004cdc20 || (DAT_0049c9c0 == 1)))))) {
    uVar13 = 0;
    DAT_0049c9c0 = 0;
    DAT_0050dd68 = g_SubpixelMaxX + iVar19 * -0x100;
    iVar9 = g_LisaCamera->fov_y;
    DAT_0050dd2c = g_SubpixelMinY + iVar9 * -0x100;
    DAT_004cdc24 = g_SubpixelMaxY + iVar9 * -0x100;
    DAT_0050dda4 = g_LisaCamera->viewport_x;
    DAT_004cdc20 = g_LisaCamera->viewport_y;

    local_50[0] = ((float)((-1 - iVar19) * 0x100 + g_SubpixelMinX) * (float)_DAT_0047ae58) /

                  ((float)DAT_0050dda4 * (float)_DAT_0047ae50);

    local_50[3] = ((float)((1 - iVar19) * 0x100 + g_SubpixelMaxX) * (float)_DAT_0047ae58) /

                  ((float)DAT_0050dda4 * (float)_DAT_0047ae50);

    local_50[1] = ((float)((-1 - iVar9) * 0x100 + g_SubpixelMinY) * (float)_DAT_0047ae58) /

                  ((float)DAT_004cdc20 * (float)_DAT_0047ae50);

    local_50[7] = ((float)((1 - iVar9) * 0x100 + g_SubpixelMaxY) * (float)_DAT_0047ae58) /

                  ((float)DAT_004cdc20 * (float)_DAT_0047ae50);
    local_50[2] = 1000.0;
    local_50[4] = local_50[1];
    local_50[5] = 1000.0;
    local_50[6] = local_50[3];
    local_50[8] = 1000.0;
    local_50[9] = local_50[0];
    local_50[10] = local_50[7];
    local_50[0xb] = 1000.0;
    iVar19 = 0;
    DAT_0050dd88 = iVar22;

    while( 1 ) {
      uVar13 = uVar13 + 1;
      uVar12 = uVar13 & 3;
      fVar1 = local_50[uVar12 * 3];
      fVar2 = *(float *)((int)local_50 + iVar19 + 8);
      fVar3 = local_50[uVar12 * 3 + 2];
      fVar4 = *(float *)((int)local_50 + iVar19);
      fVar5 = local_50[uVar12 * 3 + 1];
      fVar6 = *(float *)((int)local_50 + iVar19);

      *(float *)((int)&DAT_004cdca0 + iVar19) =

           local_50[uVar12 * 3 + 2] * *(float *)((int)local_50 + iVar19 + 4) -

           local_50[uVar12 * 3 + 1] * *(float *)((int)local_50 + iVar19 + 8);
      fVar7 = local_50[uVar12 * 3];
      fVar8 = *(float *)((int)local_50 + iVar19 + 4);
      *(float *)((int)&DAT_004cdca4 + iVar19) = fVar1 * fVar2 - fVar3 * fVar4;
      *(float *)((int)&DAT_004cdca8 + iVar19) = fVar5 * fVar6 - fVar7 * fVar8;

      fVar1 = SQRT(*(float *)((int)&DAT_004cdca0 + iVar19) * *(float *)((int)&DAT_004cdca0 + iVar19)

                   + *(float *)((int)&DAT_004cdca4 + iVar19) *

                     *(float *)((int)&DAT_004cdca4 + iVar19) +

                     *(float *)((int)&DAT_004cdca8 + iVar19) *

                     *(float *)((int)&DAT_004cdca8 + iVar19));

      *(float *)((int)&DAT_004cdca0 + iVar19) =

           (*(float *)((int)&DAT_004cdca0 + iVar19) / fVar1) * _DAT_0047ae64;

      *(float *)((int)&DAT_004cdca4 + iVar19) =

           (*(float *)((int)&DAT_004cdca4 + iVar19) / fVar1) * _DAT_0047ae64;
      fVar1 = (*(float *)((int)&DAT_004cdca8 + iVar19) / fVar1) * _DAT_0047ae64;
      if (0x2f < iVar19 + 0xc) break;
      *(float *)((int)&DAT_004cdca8 + iVar19) = fVar1;
      iVar19 = iVar19 + 0xc;
    }

    *(float *)((int)&DAT_004cdca8 + iVar19) = fVar1;
  }

  fVar24 = (double)g_LisaCamera->rot_x * (double)_DAT_0047ae48;
  fVar25 = (double)fcos(fVar24);
  fVar26 = (double)fcos((double)*local_58 * (double)_DAT_0047ae48);
  fVar24 = (double)fsin(fVar24);
  fVar27 = (double)g_LisaCamera->rot_z * (double)_DAT_0047ae48;
  fVar28 = (double)fsin((double)*local_58 * (double)_DAT_0047ae48);
  fVar29 = (double)fcos(fVar27);
  local_50[0] = (float)fVar29;
  fVar27 = (double)fsin(fVar27);
  _DAT_004cdcf0 = (float)(fVar24 * fVar28);
  _DAT_004cdce8 = (float)((double)_DAT_004cdcf0 * fVar27 + (double)local_50[0] * fVar26);
  _DAT_004cdcec = (float)((double)local_50[0] * (double)_DAT_004cdcf0 - fVar26 * fVar27);
  _DAT_004cdcf4 = (float)(fVar25 * fVar27);
  _DAT_004cdcf8 = (float)((double)local_50[0] * fVar25);
  _DAT_004cdcfc = (float)-fVar24;
  _DAT_004cdd00 = (float)(fVar27 * fVar24 * fVar26 - (double)local_50[0] * fVar28);
  _DAT_004cdcd0 = 0;
  _DAT_004cdcd4 = 0;
  puVar23 = &DAT_004cdca8;
  _DAT_004cdd04 = (float)((double)local_50[0] * fVar24 * fVar26 + fVar28 * fVar27);
  _DAT_004cdcd8 = 0;
  _DAT_004cdd08 = (float)(fVar25 * fVar26);
  pfVar17 = local_50;

  do {
    puVar23 = puVar23 + 3;
    lVar30 = __ftol();
    *pfVar17 = (float)lVar30;
    lVar30 = __ftol();
    pfVar17[1] = (float)lVar30;
    lVar30 = __ftol();
    pfVar17[2] = (float)lVar30;
    pfVar17 = pfVar17 + 4;
  } while (puVar23 < &DAT_004cdce4);
  lVar30 = __ftol();
  lVar31 = __ftol();
  lVar32 = __ftol();
  local_70 = *local_6c;
  g_LisaCamera->submesh_count = local_70;
  *local_6c = 0;
  pfVar17 = local_50 + 3;

  do {
    pfVar16 = pfVar17 + 4;

    *pfVar17 = (float)-((int)pfVar17[-1] * (int)lVar32 + (int)pfVar17[-2] * (int)lVar31 +

                       (int)pfVar17[-3] * (int)lVar30);
    pfVar17 = pfVar16;
  } while (pfVar16 < &local_4);
  local_64 = piVar20;

  if (0 < local_70) {
    do {
      iVar19 = *piVar20;
      uVar13 = 0;
      pfVar17 = local_50 + 2;

      do {
        uVar13 = uVar13 | (int)pfVar17[-1] * *(int *)(iVar19 + 0x10) +

                          (int)pfVar17[-2] * *(int *)(iVar19 + 0xc) +

                          (int)*pfVar17 * *(int *)(iVar19 + 0x14) + (int)pfVar17[1] +

                          *(short *)(iVar19 + 0x22) * 0x40000;
        if ((int)uVar13 < 0) break;
        pfVar17 = pfVar17 + 4;
      } while (pfVar17 < local_50 + 0x12);

      if (0 < (int)uVar13) {
        *local_64 = iVar19;
        *local_6c = *local_6c + 1;
        local_64 = local_64 + 1;
      }

      piVar20 = piVar20 + 1;
      local_70 = local_70 + -1;
    } while (local_70 != 0);
  }

  return 0;
}

/**
 * @original Lisa_TransformVertices (IGN_WIN.EXE @ 0x00449e70, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_TransformVertices(void) {
  int *piVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  double *pdVar10;
  int *piVar11;
  int iVar12;
  int iVar13;
  int iVar14;
  int iVar15;
  double fVar16;
  double fVar17;
  double fVar18;
  double fVar19;
  double fVar20;
  double fVar21;
  long long lVar22;
  long long lVar23;
  long long lVar24;
  long long lVar25;
  int local_1c;
  int local_18;
  int local_10;
  int local_c;

  

                    

  iVar6 = g_LisaCamera;
  pdVar10 = &g_LisaCamera->zoom;
  DAT_0049c9a8 = 0;

  if (g_LisaCamera->zoom <= 0.0) {
    DAT_0049c9a8 = 0xffffffff;
  }

  piVar11 = &g_LisaCamera->viewport_x;

  if (300 < *piVar11) {
    Lisa_TransformVerticesPanorama();
    return;
  }

  piVar1 = &g_LisaCamera->viewport_width;
  g_LisaCamera->vertex_counter = 0;
  DAT_0050ddfc = *piVar1 << 2;
  fVar16 = (double)*(double *)(iVar6 + 0x18) * (double)_DAT_0047ae68;
  DAT_004cdbbc = *(int *)(iVar6 + 0x9c) << 8;
  fVar17 = (double)fcos(fVar16);
  DAT_004cdbe8 = *(int *)(iVar6 + 0xa0) << 8;
  fVar18 = (double)*(double *)(iVar6 + 0x20) * (double)_DAT_0047ae68;
  fVar19 = (double)fcos(fVar18);
  fVar16 = (double)fsin(fVar16);
  fVar20 = (double)*(double *)(iVar6 + 0x28) * (double)_DAT_0047ae68;
  fVar18 = (double)fsin(fVar18);
  fVar21 = (double)fcos(fVar20);
  fVar20 = (double)fsin(fVar20);
  fVar2 = (float)fVar21;
  fVar3 = (float)-*piVar11 * (float)*pdVar10 * (float)_DAT_0047ae70;
  fVar4 = (float)-*(int *)(iVar6 + 0x84) * (float)_DAT_0047ae70;

  _DAT_004cdce8 =

       (float)(((double)fVar2 * fVar19 - (double)(float)(fVar16 * fVar18) * fVar20) *

              (double)fVar3);
  _DAT_004cdcec = (float)-((double)fVar3 * fVar17 * fVar20);
  _DAT_004cdcf0 = (float)((fVar16 * fVar19 * fVar20 + (double)fVar2 * fVar18) * (double)fVar3);

  _DAT_004cdcf4 =

       (float)(((double)(float)(fVar16 * fVar18) * (double)fVar2 + fVar19 * fVar20) *

              (double)fVar4);
  _DAT_004cdcf8 = (float)((double)fVar4 * fVar17 * (double)fVar2);
  _DAT_004cdd00 = (float)(-(fVar18 * fVar17) * (double)_DAT_0047ae78);
  _DAT_004cdd04 = (float)(fVar16 * (double)_DAT_0047ae78);

  _DAT_004cdcfc =

       (float)((fVar18 * fVar20 - (double)(float)(fVar16 * fVar19) * (double)fVar2) *

              (double)fVar4);
  _DAT_004cdd08 = (float)(fVar17 * fVar19 * (double)_DAT_0047ae78);
  lVar22 = __ftol();
  DAT_004cdc60 = (int)lVar22;
  lVar22 = __ftol();
  DAT_004cdc64 = (int)lVar22;
  lVar22 = __ftol();
  DAT_004cdc68 = (int)lVar22;
  lVar22 = __ftol();
  DAT_004cdc6c = (int)lVar22;
  lVar22 = __ftol();
  DAT_004cdc70 = (int)lVar22;
  lVar22 = __ftol();
  DAT_004cdc74 = (int)lVar22;
  lVar22 = __ftol();
  DAT_004cdc78 = (int)lVar22;
  lVar22 = __ftol();
  DAT_004cdc7c = (int)lVar22;
  lVar22 = __ftol();
  DAT_004cdc80 = (int)lVar22;
  lVar22 = __ftol();
  lVar23 = __ftol();
  iVar6 = (int)lVar23;
  lVar23 = __ftol();
  lVar24 = __ftol();
  iVar15 = (int)lVar24;
  lVar24 = __ftol();
  lVar25 = __ftol();
  iVar7 = (int)lVar25;
  iVar5 = DAT_004cdc64 * iVar15 + iVar6 * DAT_004cdc60 + DAT_004cdc68 * iVar7;
  DAT_0050ddc8 = (int)(iVar5 + (iVar5 >> 0x1f & 0xfffU)) >> 0xc;
  iVar5 = DAT_004cdc70 * iVar15 + iVar6 * DAT_004cdc6c + DAT_004cdc74 * iVar7;
  DAT_0050dde0 = (int)(iVar5 + (iVar5 >> 0x1f & 0xfffU)) >> 0xc;
  iVar6 = DAT_004cdc7c * iVar15 + iVar6 * DAT_004cdc78 + DAT_004cdc80 * iVar7;
  local_c = 0;
  DAT_0050de04 = (int)(iVar6 + (iVar6 >> 0x1f & 0xfffU)) >> 0xc;

  if (0 < g_LisaCamera->visible_obj_count) {
    local_18 = 0;
    local_10 = 0;

    do {
      iVar5 = g_LisaVisibleSubmeshes;
      iVar7 = DAT_0063c5ec;
      iVar6 = *(int *)(g_LisaVisibleObjects + local_10);
      piVar1 = *(int **)(iVar6 + 4);
      iVar15 = g_LisaCamera->vertex_counter;
      *(int **)(g_LisaVisibleSubmeshes + local_18) = piVar1;
      piVar11 = (int *)(iVar7 + iVar15 * 0xc);
      iVar7 = *(int *)(iVar6 + 0xc) - (int)lVar22;
      *(int **)(iVar5 + 4 + local_18) = piVar11;
      iVar5 = *(int *)(iVar6 + 0x10) - (int)lVar23;
      iVar8 = *(int *)(iVar6 + 0x14) - (int)lVar24;

      if ((*(short *)(iVar6 + 0x18) == 0 && *(short *)(iVar6 + 0x1a) == 0) &&

          *(short *)(iVar6 + 0x1c) == 0) {
        iVar6 = 2;
        local_1c = *piVar1;

        if (0 < local_1c) {
          g_LisaCamera->vertex_counter = iVar15 + local_1c;

          do {
            iVar12 = piVar1[iVar6] + iVar7;
            iVar14 = iVar5 - piVar1[iVar6 + 1];
            iVar15 = iVar6 + 2;
            iVar6 = iVar6 + 3;
            iVar9 = iVar8 + piVar1[iVar15];

            iVar13 = (iVar12 * DAT_004cdc6c + DAT_004cdc74 * iVar9 + DAT_004cdc70 * iVar14) -

                     DAT_0050dde0;

            iVar15 = DAT_0050ddfc +

                     ((iVar12 * DAT_004cdc78 + DAT_004cdc80 * iVar9 + DAT_004cdc7c * iVar14) -

                      DAT_0050de04 >> 0x10);

            if (iVar15 < DAT_0050ddfc) {
              iVar15 = DAT_0050ddfc;
            }

            *piVar11 = DAT_004cdbbc +

                       ((iVar12 * DAT_004cdc60 + DAT_004cdc68 * iVar9 + DAT_004cdc64 * iVar14) -

                       DAT_0050ddc8) / iVar15;
            iVar9 = DAT_004cdbe8;
            piVar11[2] = iVar15;
            local_1c = local_1c + -1;
            piVar11[1] = iVar9 + iVar13 / iVar15;
            piVar11 = piVar11 + 3;
          } while (local_1c != 0);
        }

      }

      else {
        Lisa_ComputeObjectMatrix(iVar7,iVar5,iVar8,iVar6,piVar1);
      }

      local_18 = local_18 + 8;
      local_10 = local_10 + 4;
      local_c = local_c + 1;
    } while (local_c < g_LisaCamera->visible_obj_count);
  }

  return;
}

/**
 * @original Lisa_TransformVerticesPanorama (IGN_WIN.EXE @ 0x0044a3d0, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_TransformVerticesPanorama(void) {
  float fVar1;
  int *piVar2;
  float fVar3;
  float fVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int *piVar11;
  int iVar12;
  int iVar13;
  int iVar14;
  double fVar15;
  double fVar16;
  double fVar17;
  double fVar18;
  double fVar19;
  double fVar20;
  long long lVar21;
  long long lVar22;
  long long lVar23;
  long long lVar24;
  int local_20;
  int local_1c;
  int local_10;
  int local_c;

  

  iVar6 = g_LisaCamera;
  DAT_0050ddfc = g_LisaCamera->viewport_width << 2;
  g_LisaCamera->vertex_counter = 0;
  fVar15 = (double)*(double *)(iVar6 + 0x18) * (double)_DAT_0047ae68;
  DAT_004cdbbc = *(int *)(iVar6 + 0x9c) << 8;
  fVar16 = (double)fcos(fVar15);
  DAT_004cdbe8 = *(int *)(iVar6 + 0xa0) << 8;
  fVar17 = (double)*(double *)(iVar6 + 0x20) * (double)_DAT_0047ae68;
  fVar18 = (double)fcos(fVar17);
  fVar15 = (double)fsin(fVar15);
  fVar19 = (double)*(double *)(iVar6 + 0x28) * (double)_DAT_0047ae68;
  fVar17 = (double)fsin(fVar17);
  fVar20 = (double)fcos(fVar19);
  fVar19 = (double)fsin(fVar19);
  fVar1 = (float)fVar20;
  fVar3 = (float)-*(int *)(iVar6 + 0x80) * (float)*(double *)(iVar6 + 0x30) * (float)_DAT_0047ae88;
  fVar4 = (float)-*(int *)(iVar6 + 0x84) * (float)_DAT_0047ae88;

  _DAT_004cdce8 =

       (float)(((double)fVar1 * fVar18 - (double)(float)(fVar15 * fVar17) * fVar19) *

              (double)fVar3);
  _DAT_004cdcec = (float)-((double)fVar3 * fVar16 * fVar19);

  _DAT_004cdcf0 =

       (float)(((double)(float)(fVar15 * fVar18) * fVar19 + (double)fVar1 * fVar17) *

              (double)fVar3);

  _DAT_004cdcf4 =

       (float)(((double)fVar1 * (double)(float)(fVar15 * fVar17) + fVar18 * fVar19) *

              (double)fVar4);
  _DAT_004cdcf8 = (float)((double)fVar4 * fVar16 * (double)fVar1);
  _DAT_004cdd04 = (float)(fVar15 * (double)_DAT_0047ae78);
  _DAT_004cdd00 = (float)(-(fVar16 * fVar17) * (double)_DAT_0047ae78);

  _DAT_004cdcfc =

       (float)((fVar17 * fVar19 - (double)fVar1 * (double)(float)(fVar15 * fVar18)) *

              (double)fVar4);
  _DAT_004cdd08 = (float)(fVar18 * fVar16 * (double)_DAT_0047ae78);
  lVar21 = __ftol();
  DAT_004cdc60 = (int)lVar21;
  lVar21 = __ftol();
  DAT_004cdc64 = (int)lVar21;
  lVar21 = __ftol();
  DAT_004cdc68 = (int)lVar21;
  lVar21 = __ftol();
  DAT_004cdc6c = (int)lVar21;
  lVar21 = __ftol();
  DAT_004cdc70 = (int)lVar21;
  lVar21 = __ftol();
  DAT_004cdc74 = (int)lVar21;
  lVar21 = __ftol();
  DAT_004cdc78 = (int)lVar21;
  lVar21 = __ftol();
  DAT_004cdc7c = (int)lVar21;
  lVar21 = __ftol();
  DAT_004cdc80 = (int)lVar21;
  lVar21 = __ftol();
  lVar22 = __ftol();
  iVar6 = (int)lVar22;
  lVar22 = __ftol();
  lVar23 = __ftol();
  iVar14 = (int)lVar23;
  lVar23 = __ftol();
  lVar24 = __ftol();
  iVar7 = (int)lVar24;
  iVar5 = DAT_004cdc60 * iVar6 + DAT_004cdc68 * iVar7 + DAT_004cdc64 * iVar14;
  DAT_0050ddc8 = (int)(iVar5 + (iVar5 >> 0x1f & 0xfffU)) >> 0xc;
  iVar5 = DAT_004cdc6c * iVar6 + DAT_004cdc74 * iVar7 + DAT_004cdc70 * iVar14;
  DAT_0050dde0 = (int)(iVar5 + (iVar5 >> 0x1f & 0xfffU)) >> 0xc;
  iVar6 = DAT_004cdc78 * iVar6 + DAT_004cdc80 * iVar7 + DAT_004cdc7c * iVar14;
  local_c = 0;
  DAT_0050de04 = (int)(iVar6 + (iVar6 >> 0x1f & 0xfffU)) >> 0xc;

  if (0 < g_LisaCamera->visible_obj_count) {
    local_1c = 0;
    local_10 = 0;

    do {
      iVar14 = g_LisaCamera;
      iVar6 = *(int *)(g_LisaVisibleObjects + local_10);
      piVar2 = *(int **)(iVar6 + 4);
      *(int **)(g_LisaVisibleSubmeshes + local_1c) = piVar2;
      iVar14 = *(int *)(iVar14 + 0x5c);
      piVar11 = (int *)(DAT_0063c5ec + iVar14 * 0xc);
      *(int **)(g_LisaVisibleSubmeshes + 4 + local_1c) = piVar11;
      iVar7 = *(int *)(iVar6 + 0xc) - (int)lVar21;
      iVar5 = *(int *)(iVar6 + 0x10) - (int)lVar22;
      iVar8 = *(int *)(iVar6 + 0x14) - (int)lVar23;

      if ((*(short *)(iVar6 + 0x18) == 0 && *(short *)(iVar6 + 0x1a) == 0) &&

          *(short *)(iVar6 + 0x1c) == 0) {
        local_20 = *piVar2;

        if (0 < local_20) {
          g_LisaCamera->vertex_counter = iVar14 + local_20;
          iVar6 = 2;

          do {
            iVar9 = iVar7 + piVar2[iVar6];
            iVar12 = iVar5 - piVar2[iVar6 + 1];
            iVar10 = iVar8 + piVar2[iVar6 + 2];

            iVar13 = (DAT_004cdc74 * iVar10 + DAT_004cdc70 * iVar12 + DAT_004cdc6c * iVar9) -

                     DAT_0050dde0;

            iVar14 = ((DAT_004cdc80 * iVar10 + DAT_004cdc7c * iVar12 + DAT_004cdc78 * iVar9) -

                      DAT_0050de04 >> 0x10) + DAT_0050ddfc;

            if (iVar14 < DAT_0050ddfc) {
              iVar14 = DAT_0050ddfc;
            }

            *piVar11 = DAT_004cdbbc +

                       (((DAT_004cdc68 * iVar10 + DAT_004cdc64 * iVar12 + DAT_004cdc60 * iVar9) -

                        DAT_0050ddc8) / iVar14) * 4;
            local_20 = local_20 + -1;
            piVar11[2] = iVar14;
            piVar11[1] = DAT_004cdbe8 + (iVar13 / iVar14) * 4;
            piVar11 = piVar11 + 3;
            iVar6 = iVar6 + 3;
          } while (local_20 != 0);
        }

      }

      else {
        FUN_0044ae20(iVar7,iVar5,iVar8,iVar6,piVar2);
      }

      local_1c = local_1c + 8;
      local_10 = local_10 + 4;
      local_c = local_c + 1;
    } while (local_c < g_LisaCamera->visible_obj_count);
  }

  return 0;
}

/**
 * @original Lisa_ComputeObjectMatrix (IGN_WIN.EXE @ 0x0044a900, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_ComputeObjectMatrix(int param_1,int param_2,int param_3,int param_4,int *param_5) {
  ushort uVar1;
  ushort uVar2;
  ushort uVar3;
  short sVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int *piVar10;
  int iVar11;
  int iVar12;
  int iVar13;
  int iVar14;
  long long lVar15;
  long long lVar16;
  long long lVar17;
  long long lVar18;
  long long lVar19;
  long long lVar20;
  long long lVar21;
  long long lVar22;
  long long lVar23;
  int local_58;

  

  iVar11 = (DAT_004cdc68 * param_3 + DAT_004cdc64 * param_2 + DAT_004cdc60 * param_1) - DAT_0050ddc8

  ;

  iVar12 = (DAT_004cdc74 * param_3 + DAT_004cdc70 * param_2 + DAT_004cdc6c * param_1) - DAT_0050dde0

  ;

  iVar13 = (DAT_004cdc80 * param_3 + DAT_004cdc7c * param_2 + DAT_004cdc78 * param_1) - DAT_0050de04

  ;
  uVar1 = *(ushort *)(param_4 + 0x1a);
  uVar2 = *(ushort *)(param_4 + 0x18);
  uVar3 = *(ushort *)(param_4 + 0x1c);

  if (0xe10 < (ushort)(uVar2 | uVar1 | uVar3)) {
    if ((short)uVar2 < 0) {
      *(ushort *)(param_4 + 0x18) = ((ushort)(0xe0f - uVar2) / 0xe10) * 0xe10 + uVar2;
    }

    if ((short)uVar1 < 0) {
      *(ushort *)(param_4 + 0x1a) = ((ushort)(0xe0f - uVar1) / 0xe10) * 0xe10 + uVar1;
    }

    if ((short)uVar3 < 0) {
      *(ushort *)(param_4 + 0x1c) = ((ushort)(0xe0f - uVar3) / 0xe10) * 0xe10 + uVar3;
    }

    sVar4 = *(short *)(param_4 + 0x18);

    if (0xe10 < sVar4) {
      *(ushort *)(param_4 + 0x18) = ((ushort)(sVar4 - 1U) / 0xe10) * -0xe10 + sVar4;
    }

    sVar4 = *(short *)(param_4 + 0x1a);

    if (0xe10 < sVar4) {
      *(ushort *)(param_4 + 0x1a) = ((ushort)(sVar4 - 1U) / 0xe10) * -0xe10 + sVar4;
    }

    sVar4 = *(short *)(param_4 + 0x1c);

    if (0xe10 < sVar4) {
      *(ushort *)(param_4 + 0x1c) = ((ushort)(sVar4 - 1U) / 0xe10) * -0xe10 + sVar4;
    }

  }

  lVar15 = __ftol();
  lVar16 = __ftol();
  lVar17 = __ftol();
  lVar18 = __ftol();
  lVar19 = __ftol();
  lVar20 = __ftol();
  lVar21 = __ftol();
  lVar22 = __ftol();
  lVar23 = __ftol();
  iVar6 = DAT_0063c5ec;
  iVar9 = 2;
  local_58 = *param_5;

  if (0 < local_58) {
    iVar5 = g_LisaCamera->vertex_counter;
    g_LisaCamera->vertex_counter = iVar5 + local_58;
    piVar10 = (int *)(iVar6 + iVar5 * 0xc);

    do {
      iVar6 = param_5[iVar9];
      iVar5 = param_5[iVar9 + 1];
      iVar7 = param_5[iVar9 + 2];
      iVar9 = iVar9 + 3;

      iVar14 = DAT_0050ddfc +

               (iVar7 * (int)lVar23 + iVar5 * (int)lVar22 + iVar6 * (int)lVar21 + iVar13 >> 0x10);

      if (iVar14 < DAT_0050ddfc) {
        iVar14 = DAT_0050ddfc;
      }

      *piVar10 = DAT_004cdbbc +

                 (iVar11 + iVar7 * (int)lVar17 + iVar5 * (int)lVar16 + iVar6 * (int)lVar15) / iVar14

      ;
      iVar8 = DAT_004cdbe8;
      piVar10[2] = iVar14;
      local_58 = local_58 + -1;

      piVar10[1] = iVar8 + (iVar12 + iVar7 * (int)lVar20 + iVar5 * (int)lVar19 + iVar6 * (int)lVar18

                           ) / iVar14;
      piVar10 = piVar10 + 3;
    } while (local_58 != 0);
  }

  return 0;
}

/**
 * @original FUN_0044ae20 (IGN_WIN.EXE @ 0x0044ae20, lisa3d.c)
 * @fidelity ADAPTED
 */
int FUN_0044ae20(int param_1,int param_2,int param_3,int param_4,int *param_5) {
  ushort uVar1;
  ushort uVar2;
  ushort uVar3;
  short sVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int *piVar10;
  int iVar11;
  int iVar12;
  int iVar13;
  long long lVar14;
  long long lVar15;
  long long lVar16;
  long long lVar17;
  long long lVar18;
  long long lVar19;
  long long lVar20;
  long long lVar21;
  long long lVar22;
  int local_58;

  

  iVar11 = (DAT_004cdc68 * param_3 + DAT_004cdc64 * param_2 + DAT_004cdc60 * param_1) - DAT_0050ddc8

  ;

  iVar12 = (DAT_004cdc74 * param_3 + DAT_004cdc70 * param_2 + DAT_004cdc6c * param_1) - DAT_0050dde0

  ;

  iVar13 = (DAT_004cdc80 * param_3 + DAT_004cdc7c * param_2 + DAT_004cdc78 * param_1) - DAT_0050de04

  ;
  uVar1 = *(ushort *)(param_4 + 0x1a);
  uVar2 = *(ushort *)(param_4 + 0x18);
  uVar3 = *(ushort *)(param_4 + 0x1c);

  if (0xe10 < (ushort)(uVar2 | uVar1 | uVar3)) {
    if ((short)uVar2 < 0) {
      *(ushort *)(param_4 + 0x18) = ((ushort)(0xe0f - uVar2) / 0xe10) * 0xe10 + uVar2;
    }

    if ((short)uVar1 < 0) {
      *(ushort *)(param_4 + 0x1a) = ((ushort)(0xe0f - uVar1) / 0xe10) * 0xe10 + uVar1;
    }

    if ((short)uVar3 < 0) {
      *(ushort *)(param_4 + 0x1c) = ((ushort)(0xe0f - uVar3) / 0xe10) * 0xe10 + uVar3;
    }

    sVar4 = *(short *)(param_4 + 0x18);

    if (0xe10 < sVar4) {
      *(ushort *)(param_4 + 0x18) = ((ushort)(sVar4 - 1U) / 0xe10) * -0xe10 + sVar4;
    }

    sVar4 = *(short *)(param_4 + 0x1a);

    if (0xe10 < sVar4) {
      *(ushort *)(param_4 + 0x1a) = ((ushort)(sVar4 - 1U) / 0xe10) * -0xe10 + sVar4;
    }

    sVar4 = *(short *)(param_4 + 0x1c);

    if (0xe10 < sVar4) {
      *(ushort *)(param_4 + 0x1c) = ((ushort)(sVar4 - 1U) / 0xe10) * -0xe10 + sVar4;
    }

  }

  lVar14 = __ftol();
  lVar15 = __ftol();
  lVar16 = __ftol();
  lVar17 = __ftol();
  lVar18 = __ftol();
  lVar19 = __ftol();
  lVar20 = __ftol();
  lVar21 = __ftol();
  lVar22 = __ftol();
  iVar6 = DAT_0063c5ec;
  local_58 = *param_5;

  if (0 < local_58) {
    iVar5 = g_LisaCamera->vertex_counter;
    g_LisaCamera->vertex_counter = iVar5 + local_58;
    iVar8 = 2;
    piVar10 = (int *)(iVar6 + iVar5 * 0xc);

    do {
      iVar6 = param_5[iVar8];
      iVar5 = param_5[iVar8 + 1];
      iVar7 = param_5[iVar8 + 2];

      iVar9 = (iVar5 * (int)lVar21 + iVar7 * (int)lVar22 + iVar6 * (int)lVar20 + iVar13 >> 0x10) +

              DAT_0050ddfc;

      if (iVar9 < DAT_0050ddfc) {
        iVar9 = DAT_0050ddfc;
      }

      *piVar10 = DAT_004cdbbc +

                 ((iVar11 + iVar5 * (int)lVar15 + iVar7 * (int)lVar16 + iVar6 * (int)lVar14) / iVar9

                 ) * 4;
      local_58 = local_58 + -1;
      piVar10[2] = iVar9;

      piVar10[1] = DAT_004cdbe8 +

                   ((iVar12 + iVar7 * (int)lVar19 + iVar5 * (int)lVar18 + iVar6 * (int)lVar17) /

                   iVar9) * 4;
      iVar8 = iVar8 + 3;
      piVar10 = piVar10 + 3;
    } while (local_58 != 0);
  }

  return 0;
}

/**
 * @original Lisa_ComputeCameraRotationMatrix (IGN_WIN.EXE @ 0x0044b340, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_ComputeCameraRotationMatrix(int *param_1) {
  double fVar1;
  double fVar2;
  long long lVar3;

  

  fVar1 = (double)g_LisaCamera->rot_x * (double)_DAT_0047ae68;
  fcos(fVar1);
  fVar2 = (double)g_LisaCamera->rot_y * (double)_DAT_0047ae68;
  fcos(fVar2);
  fsin(fVar1);
  fVar1 = (double)g_LisaCamera->rot_z * (double)_DAT_0047ae68;
  fsin(fVar2);
  fcos(fVar1);
  fsin(fVar1);
  lVar3 = __ftol();
  *param_1 = (int)lVar3;
  lVar3 = __ftol();
  param_1[1] = (int)lVar3;
  lVar3 = __ftol();
  param_1[2] = (int)lVar3;
  lVar3 = __ftol();
  param_1[3] = (int)lVar3;
  lVar3 = __ftol();
  param_1[4] = (int)lVar3;
  lVar3 = __ftol();
  param_1[5] = (int)lVar3;
  lVar3 = __ftol();
  param_1[6] = (int)lVar3;
  lVar3 = __ftol();
  param_1[7] = (int)lVar3;
  lVar3 = __ftol();
  param_1[8] = (int)lVar3;
  return;
}

/**
 * @original Lisa_InitOpcodeTable (IGN_WIN.EXE @ 0x0044b480, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_InitOpcodeTable(void) {
  int iVar1;

  

  iVar1 = g_LisaCamera;
  _DAT_0063b5e4 = (g_LisaCamera->viewport_width >> 2) + 1;
  _DAT_0063c5f8 = g_LisaCamera->viewport_width - _DAT_0063b5e4;

  if (DAT_0049c9b4 != 0) {
    if (g_LisaCamera->shading_mode == 0) {
      if (DAT_0049c9ac == 0) {
        PTR_LAB_0049c924 = ((void *)0x0044c2e0);
      }

      else {
        PTR_LAB_0049c924 = ((void *)0x0044c610);
      }

    }

    else {
      PTR_LAB_0049c924 = ((void *)0x0044bb90);
    }

    PTR_Lisa_DrawTexturedTriangle_Op15_0049c934 = Lisa_DrawTexturedTriangle_Op15;
    g_LisaCamera->active_draw_cmd = 0;
    *(int *)(iVar1 + 0x6c) = 0;
    return;
  }

  if (g_LisaCamera->shading_mode == 0) {
    if (DAT_0049c9ac == 0) {
      PTR_LAB_0049c924 = ((void *)0x0044c2e0);
    }

    else {
      PTR_LAB_0049c924 = Lisa_DrawTexturedTriangle_Op11_Unshaded;
    }

  }

  else {
    PTR_LAB_0049c924 = ((void *)0x0044bb90);

    if (DAT_0049c9ac != 0) {
      PTR_LAB_0049c924 = Lisa_DrawTexturedTriangle_Op11_Shaded;
    }

  }

  if (DAT_0049c9ac == 0) {
    PTR_Lisa_DrawTexturedTriangle_Op15_0049c934 = Lisa_DrawTexturedTriangle_Op15;
    g_LisaCamera->active_draw_cmd = 0;
    *(int *)(iVar1 + 0x6c) = 0;
    return;
  }

  PTR_Lisa_DrawTexturedTriangle_Op15_0049c934 = Lisa_DrawTexturedTriangle_Op15_Sub;
  g_LisaCamera->active_draw_cmd = 0;
  *(int *)(iVar1 + 0x6c) = 0;
  return;
}

/**
 * @original FUN_0044b570 (IGN_WIN.EXE @ 0x0044b570, lisa3d.c)
 * @fidelity ADAPTED
 */
int FUN_0044b570(void) {
  int iVar1;
  int iVar2;
  int *puVar3;
  int *piVar4;
  int iVar5;
  int *puVar6;
  int local_4;

  

  iVar2 = g_LisaDrawCommands;
  piVar4 = (int *)(g_pLisaDepthBuckets + 0x5dbc);
  local_4 = 5999;
  iVar5 = 0;

  do {
    puVar6 = (int *)*piVar4;

    if (puVar6 != (int *)0x0) {
      iVar5 = iVar5 + 1;
      iVar1 = puVar6[1];
      *(int *)(iVar2 + -4 + iVar5 * 4) = *puVar6;

      if (iVar1 != 0) {
        puVar3 = (int *)(iVar2 + iVar5 * 4);

        do {
          puVar6 = (int *)puVar6[1];
          iVar5 = iVar5 + 1;
          *puVar3 = *puVar6;
          puVar3 = puVar3 + 1;
        } while (puVar6[1] != 0);
      }

    }

    *piVar4 = 0;
    piVar4 = piVar4 + -1;
    local_4 = local_4 + -1;
  } while (-1 < local_4);
  *(int *)(iVar2 + iVar5 * 4) = 0;
  return 0;
}

/**
 * @original FUN_0044b770 (IGN_WIN.EXE @ 0x0044b770, lisa3d.c)
 * @fidelity ADAPTED
 */
void FUN_0044b770(void) {
  int *piVar1;
  short sVar2;
  unsigned int uVar3;
  unsigned int uVar4;
  unsigned int uVar5;
  unsigned int uVar6;
  unsigned int uVar7;
  unsigned int *puVar8;
  int *piVar9;
  int *piVar10;
  int iVar11;
  unsigned int *puVar12;
  unsigned int uVar13;
  unsigned int uVar14;
  unsigned int uVar15;
  int iVar16;
  unsigned int uVar17;

  

  iVar11 = DAT_0050dd8c;
  uVar3 = DAT_0050dddc[1];
  uVar4 = DAT_0050dddc[2];
  uVar5 = DAT_0050dddc[3];
  uVar6 = *(unsigned int *)(DAT_0050dd8c + uVar3 * 0xc);
  uVar7 = *(unsigned int *)(DAT_0050dd8c + uVar4 * 0xc);
  uVar13 = uVar7;

  if ((int)uVar7 <= (int)uVar6) {
    uVar13 = uVar6;
  }

  uVar14 = *(unsigned int *)(DAT_0050dd8c + uVar5 * 0xc);

  if ((int)uVar13 <= (int)uVar14) {
    uVar13 = uVar14;
  }

  if (g_SubpixelMinX <= (int)uVar13) {
    uVar13 = uVar7;

    if ((int)uVar6 <= (int)uVar7) {
      uVar13 = uVar6;
    }

    uVar14 = *(unsigned int *)(DAT_0050dd8c + uVar5 * 0xc);

    if ((int)uVar13 < (int)uVar14) {
      uVar14 = uVar13;
    }

    if ((int)uVar14 <= g_SubpixelMaxX) {
      uVar13 = *(unsigned int *)(DAT_0050dd8c + 4 + uVar4 * 0xc);
      uVar14 = *(unsigned int *)(DAT_0050dd8c + 4 + uVar3 * 0xc);
      uVar15 = uVar13;

      if ((int)uVar13 <= (int)uVar14) {
        uVar15 = uVar14;
      }

      uVar17 = *(unsigned int *)(DAT_0050dd8c + 4 + uVar5 * 0xc);

      if ((int)uVar15 <= (int)uVar17) {
        uVar15 = uVar17;
      }

      if (g_SubpixelMinY <= (int)uVar15) {
        uVar15 = uVar13;

        if ((int)uVar14 <= (int)uVar13) {
          uVar15 = uVar14;
        }

        uVar17 = *(unsigned int *)(DAT_0050dd8c + 4 + uVar5 * 0xc);

        if ((int)uVar15 < (int)uVar17) {
          uVar17 = uVar15;
        }

        if ((int)uVar17 <= g_SubpixelMaxY) {
          iVar16 = *(int *)(DAT_0050dd8c + 8 + uVar5 * 0xc) +

                   *(int *)(DAT_0050dd8c + 8 + uVar4 * 0xc) +

                   *(int *)(DAT_0050dd8c + 8 + uVar3 * 0xc);

          if ((600 < iVar16) &&

             (0 < (int)((*(int *)(DAT_0050dd8c + 4 + uVar5 * 0xc) - uVar13) * (uVar6 - uVar7) +

                        (uVar13 - uVar14) * (*(int *)(DAT_0050dd8c + uVar5 * 0xc) - uVar7) ^

                       DAT_0049c9a8))) {
            puVar8 = (unsigned int *)*DAT_004cdc84;
            uVar3 = *DAT_0050dddc;
            puVar8[1] = uVar6;
            *puVar8 = uVar3 & 0xffff;
            puVar8[2] = uVar14;
            uVar3 = *(unsigned int *)(iVar11 + uVar5 * 0xc);
            puVar8[4] = uVar7;
            puVar8[5] = uVar13;
            uVar4 = *(unsigned int *)(iVar11 + 4 + uVar5 * 0xc);
            puVar8[7] = uVar3;
            puVar12 = DAT_0050dddc;
            puVar8[8] = uVar4;
            puVar8[0xb] = 0;
            sVar2 = *(short *)(DAT_0050dd28 + 0x1e);
            puVar8[10] = puVar12[4];
            piVar9 = DAT_004cdc84;
            iVar16 = iVar16 >> 4;

            if ((99 < sVar2) && (iVar16 = iVar16 + -0x5c, iVar16 < 0)) {
              iVar16 = 0;
            }

            DAT_004cdc84[2] = (int)(puVar8 + 0xc);
            iVar11 = g_pLisaDepthBuckets;
            piVar10 = DAT_004cdc84;

            if (5999 < iVar16) {
              iVar16 = 5999;
            }

            piVar1 = DAT_004cdc84 + 1;
            DAT_004cdc84 = piVar9 + 2;
            *piVar1 = *(int *)(g_pLisaDepthBuckets + iVar16 * 4);
            *(int **)(iVar11 + iVar16 * 4) = piVar10;
          }

          DAT_0050dddc = DAT_0050dddc + 5;
          return;
        }

      }

    }

  }

  DAT_0050dddc = DAT_0050dddc + 5;
  return;
}

/**
 * @original FUN_0044b980 (IGN_WIN.EXE @ 0x0044b980, lisa3d.c)
 * @fidelity ADAPTED
 */
void FUN_0044b980(void) {
  int *piVar1;
  short sVar2;
  unsigned int uVar3;
  unsigned int uVar4;
  unsigned int uVar5;
  unsigned int uVar6;
  unsigned int uVar7;
  unsigned int *puVar8;
  int *piVar9;
  int *piVar10;
  int iVar11;
  unsigned int *puVar12;
  unsigned int uVar13;
  unsigned int uVar14;
  unsigned int uVar15;
  int iVar16;
  unsigned int uVar17;

  

  iVar11 = DAT_0050dd8c;
  uVar3 = DAT_0050dddc[1];
  uVar4 = DAT_0050dddc[2];
  uVar5 = DAT_0050dddc[3];
  uVar6 = *(unsigned int *)(DAT_0050dd8c + uVar3 * 0xc);
  uVar7 = *(unsigned int *)(DAT_0050dd8c + uVar4 * 0xc);
  uVar13 = uVar7;

  if ((int)uVar7 <= (int)uVar6) {
    uVar13 = uVar6;
  }

  uVar14 = *(unsigned int *)(DAT_0050dd8c + uVar5 * 0xc);

  if ((int)uVar13 <= (int)uVar14) {
    uVar13 = uVar14;
  }

  if (g_SubpixelMinX <= (int)uVar13) {
    uVar13 = uVar7;

    if ((int)uVar6 <= (int)uVar7) {
      uVar13 = uVar6;
    }

    uVar14 = *(unsigned int *)(DAT_0050dd8c + uVar5 * 0xc);

    if ((int)uVar13 < (int)uVar14) {
      uVar14 = uVar13;
    }

    if ((int)uVar14 <= g_SubpixelMaxX) {
      uVar13 = *(unsigned int *)(DAT_0050dd8c + 4 + uVar4 * 0xc);
      uVar14 = *(unsigned int *)(DAT_0050dd8c + 4 + uVar3 * 0xc);
      uVar15 = uVar13;

      if ((int)uVar13 <= (int)uVar14) {
        uVar15 = uVar14;
      }

      uVar17 = *(unsigned int *)(DAT_0050dd8c + 4 + uVar5 * 0xc);

      if ((int)uVar15 <= (int)uVar17) {
        uVar15 = uVar17;
      }

      if (g_SubpixelMinY <= (int)uVar15) {
        uVar15 = uVar13;

        if ((int)uVar14 <= (int)uVar13) {
          uVar15 = uVar14;
        }

        uVar17 = *(unsigned int *)(DAT_0050dd8c + 4 + uVar5 * 0xc);

        if ((int)uVar15 < (int)uVar17) {
          uVar17 = uVar15;
        }

        if ((int)uVar17 <= g_SubpixelMaxY) {
          iVar16 = *(int *)(DAT_0050dd8c + 8 + uVar5 * 0xc) +

                   *(int *)(DAT_0050dd8c + 8 + uVar4 * 0xc) +

                   *(int *)(DAT_0050dd8c + 8 + uVar3 * 0xc);

          if ((600 < iVar16) &&

             (0 < (int)((*(int *)(DAT_0050dd8c + 4 + uVar5 * 0xc) - uVar13) * (uVar6 - uVar7) +

                        (uVar13 - uVar14) * (*(int *)(DAT_0050dd8c + uVar5 * 0xc) - uVar7) ^

                       DAT_0049c9a8))) {
            puVar8 = (unsigned int *)*DAT_004cdc84;
            uVar3 = *DAT_0050dddc;
            puVar8[1] = uVar6;
            *puVar8 = uVar3 & 0xffff;
            puVar8[2] = uVar14;
            uVar3 = *(unsigned int *)(iVar11 + uVar5 * 0xc);
            puVar8[5] = uVar7;
            puVar8[6] = uVar13;
            uVar4 = *(unsigned int *)(iVar11 + 4 + uVar5 * 0xc);
            puVar8[9] = uVar3;
            puVar12 = DAT_0050dddc;
            puVar8[10] = uVar4;
            puVar8[4] = 0;
            iVar11 = DAT_0050dd28;
            uVar3 = puVar12[4];
            puVar8[8] = 0;
            puVar8[0xc] = 0;
            sVar2 = *(short *)(iVar11 + 0x1e);
            puVar8[0xd] = uVar3;
            piVar9 = DAT_004cdc84;
            iVar16 = iVar16 >> 4;

            if ((99 < sVar2) && (iVar16 = iVar16 + -0x5c, iVar16 < 0)) {
              iVar16 = 0;
            }

            DAT_004cdc84[2] = (int)(puVar8 + 0xe);
            iVar11 = g_pLisaDepthBuckets;
            piVar10 = DAT_004cdc84;

            if (5999 < iVar16) {
              iVar16 = 5999;
            }

            piVar1 = DAT_004cdc84 + 1;
            DAT_004cdc84 = piVar9 + 2;
            *piVar1 = *(int *)(g_pLisaDepthBuckets + iVar16 * 4);
            *(int **)(iVar11 + iVar16 * 4) = piVar10;
          }

          DAT_0050dddc = DAT_0050dddc + 5;
          return;
        }

      }

    }

  }

  DAT_0050dddc = DAT_0050dddc + 5;
  return;
}

/**
 * @original Lisa_RenderSubmeshes (IGN_WIN.EXE @ 0x0044c1f0, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_RenderSubmeshes(void) {
  int *piVar1;
  int iVar2;
  int iVar3;
  int iVar4;

  

                    

  iVar2 = 0;
  Lisa_InitOpcodeTable();
  DAT_004cdc84 = DAT_0063b600;
  *DAT_0063b600 = DAT_0063c600;

  if (0 < g_LisaCamera->visible_obj_count) {
    iVar4 = 0;
    iVar3 = 0;

    do {
      DAT_0050dd28 = *(int *)(g_LisaVisibleObjects + iVar3);
      piVar1 = *(int **)(g_LisaVisibleSubmeshes + iVar4);
      g_pLisaActiveMipTable = *(int *)(g_pLisaTextureSheets + *(int *)(DAT_0050dd28 + 8) * 4);
      DAT_0050dddc = piVar1 + *piVar1 * 3 + 2;
      DAT_0050dd8c = ((int *)(g_LisaVisibleSubmeshes + iVar4))[1];

      for (DAT_0050dd6c = piVar1[1]; 0 < DAT_0050dd6c; DAT_0050dd6c = DAT_0050dd6c + -1) {
        (*(void (*)(void))(&g_LisaOpcodeTable)[(char)*DAT_0050dddc])();
      }

      iVar2 = iVar2 + 1;
      iVar4 = iVar4 + 8;
      iVar3 = iVar3 + 4;
    } while (iVar2 < g_LisaCamera->visible_obj_count);
  }

  g_LisaCamera->active_draw_cmd =

       (int)(((int)DAT_004cdc84 - (int)DAT_0063b600) +

            ((int)DAT_004cdc84 - (int)DAT_0063b600 >> 0x1f & 7U)) >> 3;
  *DAT_004cdc84 = 0;
  return 0;
}

/**
 * @original Lisa_DrawTriangle_OpcodeHelper (IGN_WIN.EXE @ 0x0044cb20, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_DrawTriangle_OpcodeHelper(int param_1,int param_2) {
  int *puVar1;
  short sVar2;
  unsigned int uVar3;
  unsigned int uVar4;
  unsigned int uVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int uVar9;
  int uVar10;
  int iVar11;
  unsigned int *puVar12;
  int iVar13;
  int iVar14;
  int iVar15;
  int iVar16;
  int iVar17;
  int iVar18;
  int *puVar19;
  int *puVar20;
  long long lVar21;
  int local_24;

  

  puVar12 = DAT_0050dddc;
  iVar11 = DAT_0050dd8c;
  uVar3 = DAT_0050dddc[2];
  uVar4 = DAT_0050dddc[1];
  uVar5 = DAT_0050dddc[3];
  iVar6 = *(int *)(DAT_0050dd8c + uVar4 * 0xc);
  iVar13 = *(int *)(DAT_0050dd8c + uVar3 * 0xc);

  if (iVar13 <= iVar6) {
    iVar13 = iVar6;
  }

  iVar7 = *(int *)(DAT_0050dd8c + uVar5 * 0xc);

  if (iVar13 <= iVar7) {
    iVar13 = iVar7;
  }

  if (g_SubpixelMinX <= iVar13) {
    iVar13 = *(int *)(DAT_0050dd8c + uVar3 * 0xc);

    if (iVar6 <= iVar13) {
      iVar13 = iVar6;
    }

    iVar14 = iVar7;

    if (iVar13 < iVar7) {
      iVar14 = iVar13;
    }

    if (iVar14 <= g_SubpixelMaxX) {
      iVar13 = *(int *)(DAT_0050dd8c + 4 + uVar4 * 0xc);
      iVar14 = *(int *)(DAT_0050dd8c + 4 + uVar3 * 0xc);

      if (iVar14 <= iVar13) {
        iVar14 = iVar13;
      }

      iVar8 = *(int *)(DAT_0050dd8c + 4 + uVar5 * 0xc);

      if (iVar14 <= iVar8) {
        iVar14 = iVar8;
      }

      if (g_SubpixelMinY <= iVar14) {
        iVar14 = *(int *)(DAT_0050dd8c + 4 + uVar3 * 0xc);

        if (iVar13 <= iVar14) {
          iVar14 = iVar13;
        }

        iVar17 = iVar8;

        if (iVar14 < iVar8) {
          iVar17 = iVar14;
        }

        if (iVar17 <= g_SubpixelMaxY) {
          iVar18 = *(int *)(DAT_0050dd8c + 4 + uVar3 * 0xc) >> 4;
          iVar15 = *(int *)(DAT_0050dd8c + uVar3 * 0xc) >> 4;
          iVar14 = *(int *)(DAT_0050dd8c + 8 + uVar5 * 0xc);
          iVar17 = *(int *)(DAT_0050dd8c + 8 + uVar4 * 0xc);
          iVar16 = *(int *)(DAT_0050dd8c + 8 + uVar3 * 0xc) + iVar14 + iVar17;

          if ((600 < iVar16) &&

             (0 < (int)(((iVar8 >> 4) - iVar18) * ((iVar6 >> 4) - iVar15) +

                        (iVar18 - (iVar13 >> 4)) * ((iVar7 >> 4) - iVar15) ^ DAT_0049c9a8))) {
            puVar20 = (int *)*DAT_004cdc84;
            local_24 = iVar16 >> 4;
            sVar2 = *(short *)(DAT_0050dd28 + 0x1e);

            if ((local_24 < 0x2d1) &&

               ((((sVar2 < 100 || (sVar2 == 200)) || ((299 < sVar2 && (sVar2 < 0x12f)))) &&

                ((g_LisaCamera->shading_mode != 0 &&

                 ((char *)(*DAT_0050dddc & 0xffff0000) != &DAT_004d0000)))))) {
              if (local_24 < 0x1e1) {
                uVar9 = *(int *)(DAT_0050dd8c + 8 + uVar3 * 0xc);
                puVar20[4] = iVar17;
                puVar20[7] = uVar9;
                puVar20[10] = iVar14;
              }

              else {
                iVar16 = iVar16 / 3;
                lVar21 = __ftol();
                puVar20[4] = (int)lVar21 + iVar16;
                lVar21 = __ftol();
                puVar20[7] = (int)lVar21 + iVar16;
                lVar21 = __ftol();
                puVar20[10] = (int)lVar21 + iVar16;
              }

              uVar9 = *(int *)(iVar11 + uVar3 * 0xc);
              *puVar20 = 0x15;
              puVar20[2] = iVar6;
              puVar20[3] = iVar13;
              uVar10 = *(int *)(iVar11 + 4 + uVar3 * 0xc);
              puVar20[5] = uVar9;
              puVar20[6] = uVar10;
              uVar3 = puVar12[4];
              puVar20[8] = iVar7;
              puVar20[9] = iVar8;
              uVar4 = puVar12[5];
              uVar5 = puVar12[6];
              puVar20[0xb] = uVar3;
              puVar20[0xc] = uVar4;
              uVar3 = puVar12[7];
              uVar4 = puVar12[8];
              puVar20[0xd] = uVar5;
              puVar20[0xe] = uVar3;
              uVar3 = puVar12[9];
              uVar5 = puVar12[10];
              puVar20[0xf] = uVar4;
              iVar6 = g_pLisaActiveMipTable;
              puVar20[0x10] = uVar3;
              puVar20[0x11] = param_1;
              puVar19 = DAT_004cdc84;
              puVar20[1] = *(int *)(iVar6 + -0x14) + uVar5;
              puVar20 = puVar20 + 0x12;
            }

            else {
              uVar9 = *(int *)(DAT_0050dd8c + uVar3 * 0xc);
              *puVar20 = 0x12;
              puVar20[1] = iVar6;
              puVar20[2] = iVar13;
              uVar10 = *(int *)(iVar11 + 4 + uVar3 * 0xc);
              puVar20[3] = uVar9;
              puVar20[4] = uVar10;
              puVar20[5] = iVar7;
              puVar20[6] = iVar8;
              iVar6 = g_pLisaActiveMipTable;
              uVar3 = DAT_0050dddc[10];
              puVar20[7] = DAT_0050dddc + 4;
              puVar20[9] = param_1;
              puVar19 = DAT_004cdc84;
              puVar20[8] = *(int *)(iVar6 + -0x14) + uVar3;
              puVar20 = puVar20 + 10;
            }

            iVar6 = DAT_0050dd28;
            local_24 = local_24 - param_2;
            puVar19[2] = puVar20;
            iVar13 = g_pLisaDepthBuckets;
            puVar20 = DAT_004cdc84;

            if (99 < *(short *)(iVar6 + 0x1e)) {
              local_24 = local_24 + -0x5c;
            }

            if (local_24 < 0) {
              local_24 = 0;
            }

            if (5999 < local_24) {
              local_24 = 5999;
            }

            puVar1 = DAT_004cdc84 + 1;
            DAT_004cdc84 = puVar19 + 2;
            *puVar1 = *(int *)(g_pLisaDepthBuckets + local_24 * 4);
            *(int **)(iVar13 + local_24 * 4) = puVar20;
          }

          DAT_0050dddc = puVar12 + 0xb;
          return;
        }

      }

    }

  }

  DAT_0050dddc = DAT_0050dddc + 0xb;
  return;
}

/**
 * @original FUN_0044cf00 (IGN_WIN.EXE @ 0x0044cf00, lisa3d.c)
 * @fidelity ADAPTED
 */
void FUN_0044cf00(void) {
  int *puVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int *puVar5;
  int uVar6;
  int uVar7;
  int *puVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int iVar12;
  int iVar13;

  

  iVar9 = DAT_0050dd8c;
  iVar11 = *(int *)(DAT_0050dddc + 4);
  iVar2 = *(int *)(DAT_0050dddc + 8);
  iVar3 = *(int *)(DAT_0050dd8c + iVar11 * 0xc);
  iVar4 = *(int *)(DAT_0050dddc + 0xc);
  iVar10 = *(int *)(DAT_0050dd8c + iVar2 * 0xc);

  if (iVar10 <= iVar3) {
    iVar10 = iVar3;
  }

  iVar12 = *(int *)(DAT_0050dd8c + iVar4 * 0xc);

  if (iVar10 <= iVar12) {
    iVar10 = iVar12;
  }

  if (g_SubpixelMinX <= iVar10) {
    iVar10 = *(int *)(DAT_0050dd8c + iVar2 * 0xc);

    if (iVar3 <= iVar10) {
      iVar10 = iVar3;
    }

    iVar12 = *(int *)(DAT_0050dd8c + iVar4 * 0xc);

    if (iVar10 < iVar12) {
      iVar12 = iVar10;
    }

    if (iVar12 <= g_SubpixelMaxX) {
      iVar10 = *(int *)(DAT_0050dd8c + 4 + iVar2 * 0xc);
      iVar12 = *(int *)(DAT_0050dd8c + 4 + iVar11 * 0xc);

      if (iVar10 <= iVar12) {
        iVar10 = iVar12;
      }

      iVar13 = *(int *)(DAT_0050dd8c + 4 + iVar4 * 0xc);

      if (iVar10 <= iVar13) {
        iVar10 = iVar13;
      }

      if (g_SubpixelMinY <= iVar10) {
        iVar10 = *(int *)(DAT_0050dd8c + 4 + iVar2 * 0xc);

        if (iVar12 <= iVar10) {
          iVar10 = iVar12;
        }

        iVar13 = *(int *)(DAT_0050dd8c + 4 + iVar4 * 0xc);

        if (iVar10 < iVar13) {
          iVar13 = iVar10;
        }

        if (iVar13 <= g_SubpixelMaxY) {
          iVar11 = *(int *)(DAT_0050dd8c + 8 + iVar4 * 0xc) +

                   *(int *)(DAT_0050dd8c + 8 + iVar2 * 0xc) +

                   *(int *)(DAT_0050dd8c + 8 + iVar11 * 0xc);

          if ((600 < iVar11) &&

             (0 < (int)((*(int *)(DAT_0050dd8c + 4 + iVar2 * 0xc) - iVar12) *

                        (*(int *)(DAT_0050dd8c + iVar4 * 0xc) - *(int *)(DAT_0050dd8c + iVar2 * 0xc)

                        ) + (*(int *)(DAT_0050dd8c + 4 + iVar4 * 0xc) -

                            *(int *)(DAT_0050dd8c + 4 + iVar2 * 0xc)) *

                            (iVar3 - *(int *)(DAT_0050dd8c + iVar2 * 0xc)) ^ DAT_0049c9a8))) {
            puVar5 = (int *)*DAT_004cdc84;
            uVar6 = *(int *)(DAT_0050dd8c + iVar2 * 0xc);
            uVar7 = *(int *)(DAT_0050dd8c + 4 + iVar2 * 0xc);
            puVar5[1] = iVar3;
            *puVar5 = 0x13;
            puVar5[3] = uVar6;
            puVar5[2] = iVar12;
            puVar5[4] = uVar7;
            iVar2 = DAT_0050dddc;
            uVar6 = *(int *)(iVar9 + 4 + iVar4 * 0xc);
            puVar5[5] = *(int *)(iVar9 + iVar4 * 0xc);
            puVar5[6] = uVar6;
            uVar6 = *(int *)(iVar2 + 0x10);
            puVar5[8] = DAT_0063b5f0;
            iVar2 = DAT_0050dd28;
            puVar5[7] = uVar6;
            puVar8 = DAT_004cdc84;
            iVar11 = iVar11 >> 4;

            if ((99 < *(short *)(iVar2 + 0x1e)) && (iVar11 = iVar11 + -0x4c, iVar11 < 0)) {
              iVar11 = 0;
            }

            DAT_004cdc84[2] = puVar5 + 9;
            iVar2 = g_pLisaDepthBuckets;
            puVar5 = DAT_004cdc84;

            if (5999 < iVar11) {
              iVar11 = 5999;
            }

            puVar1 = DAT_004cdc84 + 1;
            DAT_004cdc84 = puVar8 + 2;
            *puVar1 = *(int *)(g_pLisaDepthBuckets + iVar11 * 4);
            *(int **)(iVar2 + iVar11 * 4) = puVar5;
          }

          DAT_0050dddc = DAT_0050dddc + 0x14;
          return;
        }

      }

    }

  }

  DAT_0050dddc = DAT_0050dddc + 0x14;
  return;
}

/**
 * @original FUN_0044d0f0 (IGN_WIN.EXE @ 0x0044d0f0, lisa3d.c)
 * @fidelity ADAPTED
 */
void FUN_0044d0f0(void) {
  int *puVar1;
  int uVar2;
  int uVar3;
  int iVar4;
  int iVar5;
  int *puVar6;
  int iVar7;
  int iVar8;
  long long lVar9;

  

  iVar7 = DAT_0050dddc;
  iVar4 = DAT_0050dd8c;
  puVar6 = DAT_004cdc84;
  iVar8 = *(int *)(DAT_0050dddc + 4);

  if (200 < *(int *)(DAT_0050dd8c + 8 + iVar8 * 0xc)) {
    puVar1 = (int *)*DAT_004cdc84;
    *puVar1 = 7;
    puVar1[1] = puVar1 + 5;
    uVar2 = *(int *)(iVar4 + iVar8 * 0xc);
    puVar1[2] = puVar1 + 0xd;
    uVar3 = *(int *)(iVar4 + 4 + iVar8 * 0xc);
    puVar1[3] = uVar2;
    iVar8 = *(int *)(iVar7 + 8);
    puVar1[4] = uVar3;
    iVar4 = *(int *)(iVar7 + 0x14);
    puVar1[5] = (iVar8 + *(int *)(iVar7 + 0x10)) / 2 - *(int *)(iVar7 + 8);
    iVar8 = *(int *)(iVar7 + 0xc);
    iVar5 = *(int *)(iVar7 + 0xc);
    puVar1[8] = iVar5;
    uVar2 = *(int *)(iVar7 + 0x14);
    puVar1[6] = (iVar8 + iVar4) / 2 - iVar5;
    puVar1[10] = uVar2;
    puVar1[7] = *(int *)(iVar7 + 8);
    puVar1[9] = *(int *)(iVar7 + 0x10);
    puVar1[0xc] = DAT_0063b5f0;
    lVar9 = __ftol();
    puVar1[0xd] = (int)lVar9;
    puVar1[0xe] = 0;
    puVar1[0xf] = 0;
    lVar9 = __ftol();
    puVar1[0x10] = (int)lVar9;
    lVar9 = __ftol();
    iVar8 = (int)lVar9 >> 4;
    puVar1[0xb] = *(int *)(g_pLisaActiveMipTable + -0x14) + *(int *)(iVar7 + 0x18);

    if ((99 < *(short *)(DAT_0050dd28 + 0x1e)) && (iVar8 = iVar8 + -0x5c, iVar8 < 0)) {
      iVar8 = 0;
    }

    DAT_004cdc84 = puVar6 + 2;
    *DAT_004cdc84 = puVar1 + 0x11;
    iVar4 = g_pLisaDepthBuckets;

    if (5999 < iVar8) {
      iVar8 = 5999;
    }

    puVar6[1] = *(int *)(g_pLisaDepthBuckets + iVar8 * 4);
    *(int **)(iVar4 + iVar8 * 4) = puVar6;
  }

  DAT_0050dddc = iVar7 + 0x24;
  return;
}

/**
 * @original FUN_0044d230 (IGN_WIN.EXE @ 0x0044d230, lisa3d.c)
 * @fidelity ADAPTED
 */
void FUN_0044d230(void) {
  int *puVar1;
  short sVar2;
  int *puVar3;
  int iVar4;
  int iVar5;
  int uVar6;
  int *puVar7;
  int iVar8;
  int uVar9;
  int iVar10;
  long long lVar11;

  

  iVar8 = DAT_0050dddc;
  iVar5 = DAT_0050dd8c;
  iVar10 = *(int *)(DAT_0050dddc + 4);

  if (200 < *(int *)(DAT_0050dd8c + 8 + iVar10 * 0xc)) {
    puVar3 = (int *)*DAT_004cdc84;
    *puVar3 = 7;
    puVar3[1] = puVar3 + 5;
    puVar3[2] = puVar3 + 0xd;
    iVar4 = *(int *)(iVar8 + 0x10);
    puVar3[3] = *(unsigned int *)(iVar5 + iVar10 * 0xc) & 0xffffff00;
    puVar3[4] = *(unsigned int *)(iVar5 + 4 + iVar10 * 0xc) & 0xffffff00;
    puVar3[5] = (*(int *)(iVar8 + 8) + iVar4) / 2 - *(int *)(iVar8 + 8);
    iVar10 = *(int *)(iVar8 + 0xc);
    iVar5 = *(int *)(iVar8 + 0x14);
    puVar3[8] = iVar10;
    puVar3[7] = *(int *)(iVar8 + 8);
    uVar9 = DAT_0063c5f4;
    uVar6 = *(int *)(iVar8 + 0x10);
    puVar3[6] = (iVar5 + iVar10) / 2 - iVar10;
    puVar3[9] = uVar6;
    puVar3[10] = *(int *)(iVar8 + 0x14);
    puVar3[0xc] = uVar9;
    lVar11 = __ftol();
    puVar3[0xe] = 0;
    puVar3[0xf] = 0;
    puVar3[0xd] = (int)lVar11;
    lVar11 = __ftol();
    puVar3[0x10] = (int)lVar11;
    lVar11 = __ftol();
    iVar10 = (int)lVar11 >> 4;
    sVar2 = *(short *)(DAT_0050dd28 + 0x1e);
    puVar3[0xb] = *(int *)(g_pLisaActiveMipTable + -0x14) + *(int *)(iVar8 + 0x18);
    puVar7 = DAT_004cdc84;

    if ((99 < sVar2) && (iVar10 = iVar10 + -0x5c, iVar10 < 0)) {
      iVar10 = 0;
    }

    DAT_004cdc84[2] = puVar3 + 0x11;
    iVar5 = g_pLisaDepthBuckets;
    puVar3 = DAT_004cdc84;

    if (5999 < iVar10) {
      iVar10 = 5999;
    }

    puVar1 = DAT_004cdc84 + 1;
    DAT_004cdc84 = puVar7 + 2;
    *puVar1 = *(int *)(g_pLisaDepthBuckets + iVar10 * 4);
    *(int **)(iVar5 + iVar10 * 4) = puVar3;
  }

  DAT_0050dddc = iVar8 + 0x24;
  return;
}

/**
 * @original Lisa_DrawTexturedTriangle_Op15 (IGN_WIN.EXE @ 0x0044d550, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_DrawTexturedTriangle_Op15(void) {
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int *puVar5;
  int uVar6;
  int uVar7;
  int uVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int iVar12;
  int iVar13;
  int iVar14;
  int iVar15;
  int iVar16;
  int iVar17;
  int iVar18;
  int *puVar19;
  unsigned int uVar20;
  unsigned int uVar21;
  int iVar22;
  int *puVar23;
  int iVar24;
  long long lVar25;

  

  iVar9 = DAT_0050dd8c;
  _DAT_004cdc3c = DAT_0050dd8c;

  do {
    iVar24 = *(int *)(DAT_0050dddc + 4);
    iVar22 = iVar24 * 3;
    DAT_0050dd9c = *(int *)(DAT_0050dddc + 8) * 3;
    DAT_004cdc88 = *(int *)(DAT_0050dddc + 0xc) * 3;
    iVar1 = *(int *)(iVar9 + 4 + iVar24 * 0xc);
    iVar24 = *(int *)(iVar9 + iVar24 * 0xc);

    if ((g_SubpixelMaxY - iVar1 | g_SubpixelMaxX - iVar24 | iVar1 - g_SubpixelMinY | iVar24 - g_SubpixelMinX

        ) < 0) {
      do {
        iVar24 = *(int *)(iVar9 + iVar22 * 4);
        iVar1 = *(int *)(iVar9 + DAT_0050dd9c * 4);
        _DAT_004cdc4c = iVar24;

        if (iVar24 <= iVar1) {
          _DAT_004cdc4c = iVar1;
        }

        iVar2 = *(int *)(iVar9 + DAT_004cdc88 * 4);
        iVar11 = _DAT_004cdc4c;

        if (_DAT_004cdc4c <= iVar2) {
          iVar11 = iVar2;
        }

        if (g_SubpixelMinX <= iVar11) {
          _DAT_004cdc4c = iVar24;

          if (iVar1 <= iVar24) {
            _DAT_004cdc4c = iVar1;
          }

          iVar24 = _DAT_004cdc4c;

          if (iVar2 <= _DAT_004cdc4c) {
            iVar24 = iVar2;
          }

          if (iVar24 <= g_SubpixelMaxX) {
            iVar24 = *(int *)(iVar9 + 4 + iVar22 * 4);
            iVar1 = *(int *)(iVar9 + 4 + DAT_0050dd9c * 4);
            _DAT_004cdc4c = iVar24;

            if (iVar24 <= iVar1) {
              _DAT_004cdc4c = iVar1;
            }

            iVar2 = *(int *)(iVar9 + 4 + DAT_004cdc88 * 4);
            iVar11 = _DAT_004cdc4c;

            if (_DAT_004cdc4c <= iVar2) {
              iVar11 = iVar2;
            }

            if (g_SubpixelMinY <= iVar11) {
              _DAT_004cdc4c = iVar24;

              if (iVar1 <= iVar24) {
                _DAT_004cdc4c = iVar1;
              }

              iVar24 = _DAT_004cdc4c;

              if (iVar2 <= _DAT_004cdc4c) {
                iVar24 = iVar2;
              }

              if (iVar24 <= g_SubpixelMaxY) break;
            }

          }

        }

        iVar24 = DAT_0050dddc + 0x2c;

        if (*(char *)(DAT_0050dddc + 0x2c) != '\x15') {
          _DAT_004cdd10 = iVar22;
          DAT_0050dddc = iVar24;
          return;
        }

        if (DAT_0050dd6c < 3) {
          _DAT_004cdd10 = iVar22;
          DAT_0050dddc = iVar24;
          return;
        }

        DAT_0050dd6c = DAT_0050dd6c + -1;
        iVar22 = *(int *)(DAT_0050dddc + 0x30) * 3;
        DAT_004cdc88 = *(int *)(DAT_0050dddc + 0x38) * 3;
        DAT_0050dd9c = *(int *)(DAT_0050dddc + 0x34) * 3;
        DAT_0050dddc = iVar24;
      } while( 1 );
    }

    iVar10 = DAT_0050dddc;
    iVar24 = *(int *)(iVar9 + 4 + DAT_0050dd9c * 4);
    iVar1 = *(int *)(iVar9 + DAT_004cdc88 * 4);
    iVar2 = *(int *)(iVar9 + DAT_0050dd9c * 4);
    iVar11 = *(int *)(iVar9 + 4 + DAT_004cdc88 * 4);

    DAT_0050ddcc = ((iVar24 >> 4) - (*(int *)(iVar9 + 4 + iVar22 * 4) >> 4)) *

                   ((iVar1 >> 4) - (iVar2 >> 4)) +

                   ((iVar11 >> 4) - (iVar24 >> 4)) *

                   ((*(int *)(iVar9 + iVar22 * 4) >> 4) - (iVar2 >> 4)) ^ DAT_0049c9a8;
    iVar3 = *(int *)(iVar9 + 8 + DAT_004cdc88 * 4);
    iVar4 = *(int *)(iVar9 + 8 + DAT_0050dd9c * 4);
    iVar12 = iVar3 + iVar4 + *(int *)(iVar9 + 8 + iVar22 * 4);
    _DAT_004cdd10 = iVar22;

    if ((600 < iVar12) && (0 < (int)DAT_0050ddcc)) {
      DAT_004cdc44 = iVar12 >> 4;
      puVar5 = (int *)*DAT_004cdc84;

      if (DAT_0049c9ac == 1) {
        iVar17 = *(int *)(iVar9 + iVar22 * 4) >> 8;
        iVar13 = *(int *)(iVar9 + 4 + iVar22 * 4) >> 8;

        iVar13 = (((iVar24 >> 8) + iVar13) * ((iVar2 >> 8) - iVar17) +

                  ((iVar11 >> 8) + iVar13) * (iVar17 - (iVar1 >> 8)) +

                 ((iVar24 >> 8) + (iVar11 >> 8)) * ((iVar1 >> 8) - (iVar2 >> 8))) * 3;
        uVar20 = iVar13 >> 0x1f;
        DAT_0050dd84 = (iVar13 >> 1 ^ uVar20) - uVar20;
        iVar13 = *(int *)(DAT_0050dddc + 0x24) >> 8;
        iVar17 = *(int *)(DAT_0050dddc + 0x14) >> 8;
        iVar18 = *(int *)(DAT_0050dddc + 0x20) >> 8;
        iVar14 = *(int *)(DAT_0050dddc + 0x10) >> 8;
        iVar15 = *(int *)(DAT_0050dddc + 0x1c) >> 8;
        iVar16 = *(int *)(DAT_0050dddc + 0x18) >> 8;

        uVar20 = (iVar15 + iVar17) * (iVar16 - iVar14) +

                 (iVar17 + iVar13) * (iVar14 - iVar18) + (iVar15 + iVar13) * (iVar18 - iVar16);
        uVar21 = (int)uVar20 >> 0x1f;
        iVar13 = (uVar20 ^ uVar21) - uVar21;
        if (iVar13 < DAT_0050dd84) goto LAB_0044d8ef;
        DAT_0050ddac = (DAT_0050dd84 * 4 <= iVar13) - 4;
      }

      else {
LAB_0044d8ef:

        DAT_0050ddac = -5;
      }

      if (((g_LisaCamera->shading_mode == 0) ||

          (_DAT_004cdc98 = (int)*(short *)(DAT_0050dd28 + 0x1e), 0x2d0 < DAT_004cdc44)) ||

         (((99 < _DAT_004cdc98 && (_DAT_004cdc98 != 200)) &&

          ((_DAT_004cdc98 < 300 || (0x12e < _DAT_004cdc98)))))) {
        uVar6 = *(int *)(iVar9 + iVar22 * 4);
        uVar7 = *(int *)(iVar9 + 4 + iVar22 * 4);
        _DAT_0050ddf0 = puVar5;
        *puVar5 = 0x11;
        puVar5[1] = uVar6;
        puVar5[2] = uVar7;
        puVar5[3] = iVar2;
        iVar22 = g_pLisaActiveMipTable;
        puVar5[4] = iVar24;
        puVar5[5] = iVar1;
        iVar24 = DAT_0050ddac;
        puVar5[6] = iVar11;
        puVar5[7] = DAT_0050dddc + 0x10;
        puVar19 = DAT_004cdc84;
        puVar23 = puVar5 + 9;
        puVar5[8] = *(int *)(iVar22 + iVar24 * 4) + *(int *)(DAT_0050dddc + 0x28);
      }

      else {
        if (DAT_004cdc44 < 0x1e1) {
          puVar5[4] = *(int *)(iVar9 + 8 + iVar22 * 4);
          puVar5[7] = iVar4;
          puVar5[10] = iVar3;
        }

        else {
          DAT_0050dd64 = iVar12 / 3;
          _DAT_0050dde8 = (float)(0x2d0 - DAT_004cdc44) * _DAT_0047aea0;
          lVar25 = __ftol();
          puVar5[4] = (int)lVar25 + DAT_0050dd64;
          lVar25 = __ftol();
          puVar5[7] = (int)lVar25 + DAT_0050dd64;
          _DAT_0050dd20 = iVar3;
          lVar25 = __ftol();
          puVar5[10] = (int)lVar25 + DAT_0050dd64;
        }

        uVar6 = *(int *)(iVar9 + iVar22 * 4);
        uVar7 = *(int *)(iVar9 + 4 + iVar22 * 4);
        _DAT_0050ddf0 = puVar5;
        *puVar5 = 0x14;
        puVar5[2] = uVar6;
        puVar5[3] = uVar7;
        puVar5[5] = iVar2;
        puVar5[6] = iVar24;
        uVar6 = *(int *)(iVar10 + 0x10);
        puVar5[8] = iVar1;
        uVar7 = *(int *)(iVar10 + 0x14);
        puVar5[9] = iVar11;
        uVar8 = *(int *)(iVar10 + 0x18);
        puVar5[0xb] = uVar6;
        uVar6 = *(int *)(iVar10 + 0x1c);
        puVar5[0xc] = uVar7;
        uVar7 = *(int *)(iVar10 + 0x20);
        puVar5[0xd] = uVar8;
        uVar8 = *(int *)(iVar10 + 0x24);
        puVar5[0xe] = uVar6;
        puVar5[0xf] = uVar7;
        puVar5[0x10] = uVar8;
        puVar19 = DAT_004cdc84;
        puVar23 = puVar5 + 0x11;
        puVar5[1] = *(int *)(g_pLisaActiveMipTable + DAT_0050ddac * 4) + *(int *)(iVar10 + 0x28);
      }

      iVar24 = DAT_0050dd28;
      DAT_004cdc44 = DAT_004cdc44 + -0x50;
      puVar19[2] = puVar23;
      iVar1 = g_pLisaDepthBuckets;
      puVar5 = DAT_004cdc84;

      if (99 < *(short *)(iVar24 + 0x1e)) {
        if (*(short *)(iVar24 + 0x1e) == 0xd2) {
          iVar24 = -0x54;
        }

        else {
          iVar24 = -0x5c;
        }

        DAT_004cdc44 = DAT_004cdc44 + iVar24;
      }

      if (DAT_004cdc44 < 0) {
        DAT_004cdc44 = 0;
      }

      if (5999 < DAT_004cdc44) {
        DAT_004cdc44 = 5999;
      }

      iVar24 = DAT_004cdc44;
      puVar23 = DAT_004cdc84 + 1;
      DAT_004cdc84 = puVar19 + 2;
      *puVar23 = *(int *)(g_pLisaDepthBuckets + DAT_004cdc44 * 4);
      *(int **)(iVar1 + iVar24 * 4) = puVar5;
    }

    DAT_0050dddc = iVar10 + 0x2c;

    if ((*(char *)(iVar10 + 0x2c) != '\x15') || (DAT_0050dd6c < 3)) {
      return;
    }

    DAT_0050dd6c = DAT_0050dd6c + -1;
  } while( 1 );
}

/**
 * @original Lisa_DrawTexturedTriangle_Op15_Sub (IGN_WIN.EXE @ 0x0044e900, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_DrawTexturedTriangle_Op15_Sub(void) {
  char *pcVar1;
  short sVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int *puVar6;
  int uVar7;
  int uVar8;
  int uVar9;
  int *piVar10;
  int iVar11;
  int iVar12;
  int iVar13;
  int iVar14;
  int iVar15;
  int iVar16;
  int iVar17;
  int iVar18;
  int iVar19;
  unsigned int uVar20;
  int iVar21;
  unsigned int uVar22;
  int iVar23;
  int *puVar24;
  int iVar25;
  int iVar26;
  int *puVar27;
  long long lVar28;

  

  iVar11 = DAT_0050dd8c;
  _DAT_004cdbcc = DAT_0050dd8c;
  iVar19 = DAT_0050dd6c;

  do {
    DAT_0050dd6c = iVar19;
    iVar19 = *(int *)(DAT_0050dddc + 4);
    iVar25 = iVar19 * 3;
    DAT_004cdce0 = *(int *)(DAT_0050dddc + 8) * 3;
    DAT_0050ddc4 = *(int *)(DAT_0050dddc + 0xc) * 3;
    iVar23 = *(int *)(iVar11 + 4 + iVar19 * 0xc);
    iVar19 = *(int *)(iVar11 + iVar19 * 0xc);

    if ((g_SubpixelMaxY - iVar23 | g_SubpixelMaxX - iVar19 | iVar23 - g_SubpixelMinY |

        iVar19 - g_SubpixelMinX) < 0) {
      do {
        iVar4 = DAT_0050dddc;
        iVar19 = *(int *)(iVar11 + iVar25 * 4);
        iVar23 = *(int *)(iVar11 + DAT_004cdce0 * 4);
        _DAT_004cdc9c = iVar23;

        if (iVar23 <= iVar19) {
          _DAT_004cdc9c = iVar19;
        }

        iVar3 = *(int *)(iVar11 + DAT_0050ddc4 * 4);
        iVar26 = _DAT_004cdc9c;

        if (_DAT_004cdc9c <= iVar3) {
          iVar26 = iVar3;
        }

        if (g_SubpixelMinX <= iVar26) {
          _DAT_004cdc9c = iVar23;

          if (iVar19 <= iVar23) {
            _DAT_004cdc9c = iVar19;
          }

          iVar19 = _DAT_004cdc9c;

          if (iVar3 <= _DAT_004cdc9c) {
            iVar19 = iVar3;
          }

          if (iVar19 <= g_SubpixelMaxX) {
            iVar19 = *(int *)(iVar11 + 4 + DAT_004cdce0 * 4);
            iVar23 = *(int *)(iVar11 + 4 + iVar25 * 4);
            _DAT_004cdc9c = iVar19;

            if (iVar19 <= iVar23) {
              _DAT_004cdc9c = iVar23;
            }

            iVar3 = *(int *)(iVar11 + 4 + DAT_0050ddc4 * 4);
            iVar26 = _DAT_004cdc9c;

            if (_DAT_004cdc9c <= iVar3) {
              iVar26 = iVar3;
            }

            if (g_SubpixelMinY <= iVar26) {
              _DAT_004cdc9c = iVar19;

              if (iVar23 <= iVar19) {
                _DAT_004cdc9c = iVar23;
              }

              iVar19 = _DAT_004cdc9c;

              if (iVar3 <= _DAT_004cdc9c) {
                iVar19 = iVar3;
              }

              if (iVar19 <= g_SubpixelMaxY) break;
            }

          }

        }

        _DAT_004cdc50 = iVar25;
        pcVar1 = (char *)(DAT_0050dddc + 0x2c);
        DAT_0050dddc = DAT_0050dddc + 0x2c;

        if ((*pcVar1 != '\x11') || (DAT_0050dd6c + -1 < 1)) {
          return;
        }

        iVar25 = *(int *)(iVar4 + 0x30) * 3;
        DAT_004cdce0 = *(int *)(iVar4 + 0x34) * 3;
        DAT_0050ddc4 = *(int *)(iVar4 + 0x38) * 3;
        DAT_0050dd6c = DAT_0050dd6c + -1;
      } while( 1 );
    }

    iVar12 = DAT_0050dddc;
    iVar19 = *(int *)(iVar11 + 4 + DAT_0050ddc4 * 4);
    iVar23 = *(int *)(iVar11 + 4 + DAT_004cdce0 * 4);
    iVar4 = *(int *)(iVar11 + DAT_004cdce0 * 4);
    iVar3 = *(int *)(iVar11 + DAT_0050ddc4 * 4);

    _DAT_004cdbac =

         ((iVar19 >> 4) - (iVar23 >> 4)) * ((*(int *)(iVar11 + iVar25 * 4) >> 4) - (iVar4 >> 4)) +

         ((iVar23 >> 4) - (*(int *)(iVar11 + 4 + iVar25 * 4) >> 4)) * ((iVar3 >> 4) - (iVar4 >> 4))

         ^ DAT_0049c9a8;
    iVar26 = *(int *)(iVar11 + 8 + DAT_004cdce0 * 4);
    iVar5 = *(int *)(iVar11 + 8 + DAT_0050ddc4 * 4);
    iVar13 = iVar26 + iVar5 + *(int *)(iVar11 + 8 + iVar25 * 4);
    _DAT_004cdc50 = iVar25;

    if ((600 < iVar13) && (0 < (int)_DAT_004cdbac)) {
      DAT_0050ddb8 = iVar13 >> 4;
      puVar6 = (int *)*DAT_004cdc84;

      if (DAT_0049c9ac == 1) {
        iVar14 = *(int *)(iVar11 + 4 + iVar25 * 4) >> 8;
        iVar15 = *(int *)(iVar11 + iVar25 * 4) >> 8;

        iVar14 = (((iVar19 >> 8) + (iVar23 >> 8)) * ((iVar3 >> 8) - (iVar4 >> 8)) +

                  (iVar14 + (iVar23 >> 8)) * ((iVar4 >> 8) - iVar15) +

                 (iVar14 + (iVar19 >> 8)) * (iVar15 - (iVar3 >> 8))) * 3;
        uVar20 = iVar14 >> 0x1f;
        DAT_004cdc58 = (iVar14 >> 1 ^ uVar20) - uVar20;
        iVar14 = *(int *)(DAT_0050dddc + 0x24) >> 8;
        iVar15 = *(int *)(DAT_0050dddc + 0x14) >> 8;
        iVar18 = *(int *)(DAT_0050dddc + 0x20) >> 8;
        iVar16 = *(int *)(DAT_0050dddc + 0x10) >> 8;
        iVar17 = *(int *)(DAT_0050dddc + 0x1c) >> 8;
        iVar21 = *(int *)(DAT_0050dddc + 0x18) >> 8;

        uVar20 = (iVar14 + iVar15) * (iVar16 - iVar18) + (iVar14 + iVar17) * (iVar18 - iVar21) +

                 (iVar17 + iVar15) * (iVar21 - iVar16);
        uVar22 = (int)uVar20 >> 0x1f;
        iVar14 = (uVar20 ^ uVar22) - uVar22;

        if (iVar14 < DAT_004cdc58) {
          DAT_004cdc38 = -5;
        }

        else {
          if (iVar14 < DAT_004cdc58 * 4) goto LAB_0044ecb6;
          DAT_004cdc38 = (DAT_004cdc58 * 0x10 <= iVar14) - 3;
        }

      }

      else {
LAB_0044ecb6:

        DAT_004cdc38 = -4;
      }

      sVar2 = *(short *)(DAT_0050dd28 + 0x1e);
      _DAT_004cdcdc = (int)sVar2;

      if ((DAT_0050ddb8 < 0x2d1) &&

         (((_DAT_004cdcdc < 100 || (_DAT_004cdcdc == 200)) ||

          ((299 < _DAT_004cdcdc && (_DAT_004cdcdc < 0x12f)))))) {
        if (DAT_0050ddb8 < 0x1e1) {
          puVar6[4] = *(int *)(iVar11 + 8 + iVar25 * 4);
          puVar6[7] = iVar26;
          puVar6[10] = iVar5;
        }

        else {
          DAT_0050dd54 = iVar13 / 3;
          _DAT_0050dd98 = (float)(0x2d0 - DAT_0050ddb8) * _DAT_0047aea0;
          lVar28 = __ftol();
          puVar6[4] = (int)lVar28 + DAT_0050dd54;
          lVar28 = __ftol();
          puVar6[7] = (int)lVar28 + DAT_0050dd54;
          _DAT_004cdbf0 = iVar5;
          lVar28 = __ftol();
          puVar6[10] = (int)lVar28 + DAT_0050dd54;
        }

        uVar7 = *(int *)(iVar11 + iVar25 * 4);
        uVar8 = *(int *)(iVar11 + 4 + iVar25 * 4);
        *puVar6 = 0x14;
        puVar6[2] = uVar7;
        puVar6[3] = uVar8;
        iVar25 = DAT_004cdc38;
        puVar6[5] = iVar4;
        puVar6[6] = iVar23;
        puVar6[8] = iVar3;
        puVar6[9] = iVar19;

        if ((iVar25 == -5) && (g_pLisaActiveMipTable[-5] != *g_pLisaActiveMipTable)) {
          uVar20 = *(unsigned int *)(iVar12 + 0x10);
          uVar22 = *(unsigned int *)(iVar12 + 0x14);
          puVar6[0xb] = (uVar20 & 0x3fff) << 2;
          puVar6[0xc] = (uVar22 & 0x3fff) << 2;
          puVar6[0xd] = (*(unsigned int *)(iVar12 + 0x18) & 0x3fff) << 2;
          puVar6[0xe] = (*(unsigned int *)(iVar12 + 0x1c) & 0x3fff) << 2;
          iVar19 = (int)uVar20 >> 0xe;
          puVar6[0xf] = (*(unsigned int *)(iVar12 + 0x20) & 0x3fff) << 2;
          _DAT_004cdbdc = (int)uVar22 >> 0xe;
          iVar23 = _DAT_004cdbdc * 0x4000;
          _DAT_0050dd60 = iVar19;
          puVar6[0x10] = (*(unsigned int *)(iVar12 + 0x24) & 0x3fff) << 2;
          iVar19 = g_pLisaActiveMipTable[iVar19 * 4] + (*(int *)(iVar12 + 0x28) + iVar23) * 4;
        }

        else {
          uVar7 = *(int *)(iVar12 + 0x14);
          uVar8 = *(int *)(iVar12 + 0x18);
          puVar6[0xb] = *(int *)(iVar12 + 0x10);
          uVar9 = *(int *)(iVar12 + 0x1c);
          puVar6[0xc] = uVar7;
          piVar10 = g_pLisaActiveMipTable;
          uVar7 = *(int *)(iVar12 + 0x20);
          puVar6[0xd] = uVar8;
          puVar6[0xe] = uVar9;
          iVar19 = DAT_004cdc38;
          uVar8 = *(int *)(iVar12 + 0x24);
          puVar6[0xf] = uVar7;
          puVar6[0x10] = uVar8;
          iVar19 = piVar10[iVar19] + *(int *)(iVar12 + 0x28);
        }

        puVar24 = DAT_004cdc84;
        puVar6[1] = iVar19;
        puVar27 = puVar6 + 0x11;
      }

      else {
        uVar7 = *(int *)(iVar11 + 4 + iVar25 * 4);
        puVar6[1] = *(int *)(iVar11 + iVar25 * 4);
        puVar6[2] = uVar7;
        puVar6[3] = iVar4;
        iVar25 = DAT_004cdc38;
        puVar6[4] = iVar23;
        puVar6[5] = iVar3;
        puVar6[6] = iVar19;
        puVar6[7] = (int *)(DAT_0050dddc + 0x10);
        iVar19 = DAT_004cdc38;
        piVar10 = g_pLisaActiveMipTable;

        if ((iVar25 == -5) && (g_pLisaActiveMipTable[-5] != *g_pLisaActiveMipTable)) {
          _DAT_0050dd60 = *(int *)(DAT_0050dddc + 0x10);
          *puVar6 = 0x16;
          _DAT_0050dd60 = _DAT_0050dd60 >> 0xe;
          _DAT_004cdbdc = *(int *)(DAT_0050dddc + 0x14) >> 0xe;

          puVar6[8] = piVar10[_DAT_0050dd60 * 4] +

                      (*(int *)(DAT_0050dddc + 0x28) + _DAT_004cdbdc * 0x4000) * 4;
        }

        else {
          iVar23 = *(int *)(DAT_0050dddc + 0x28);
          *puVar6 = 0x11;
          puVar6[8] = piVar10[iVar19] + iVar23;
        }

        puVar27 = puVar6 + 9;
        puVar24 = DAT_004cdc84;
      }

      _DAT_0050ddf4 = puVar6;
      puVar24[2] = puVar27;
      iVar19 = g_pLisaDepthBuckets;
      puVar6 = DAT_004cdc84;
      iVar23 = DAT_0050ddb8 + -0x50;

      if (99 < sVar2) {
        if (sVar2 == 0xd2) {
          iVar23 = DAT_0050ddb8 + -0xa4;
        }

        else {
          iVar23 = DAT_0050ddb8 + -0xac;
        }

      }

      DAT_0050ddb8 = iVar23;

      if (DAT_0050ddb8 < 0) {
        DAT_0050ddb8 = 0;
      }

      if (5999 < DAT_0050ddb8) {
        DAT_0050ddb8 = 5999;
      }

      iVar23 = DAT_0050ddb8;
      puVar27 = DAT_004cdc84 + 1;
      DAT_004cdc84 = puVar24 + 2;
      *puVar27 = *(int *)(g_pLisaDepthBuckets + DAT_0050ddb8 * 4);
      *(int **)(iVar19 + iVar23 * 4) = puVar6;
    }

    DAT_0050dddc = iVar12 + 0x2c;

    if ((*(char *)(iVar12 + 0x2c) != '\x11') || (iVar19 = DAT_0050dd6c + -1, DAT_0050dd6c + -1 < 1))

    {
      return;
    }

  } while( 1 );
}

/**
 * @original Lisa_InitRasterizerTables (IGN_WIN.EXE @ 0x0044f070, lisa3d.c)
 * @fidelity ADAPTED
 */
long long Lisa_InitRasterizerTables(int param_1,unsigned int param_2) {
  int in_EAX;
  short sVar1;
  int unaff_EBX;
  unsigned int uVar2;
  int iVar3;
  int unaff_ESI;
  int *piVar4;
  char **ppuVar5;

  

  if ((in_EAX < 0x579) && (unaff_EBX < 0x259)) {
    piVar4 = &DAT_0049d3a8;
    uVar2 = 1;
    DAT_0049ca3c = in_EAX;
    _DAT_0049ca40 = unaff_EBX;

    do {
      *piVar4 = (int)(0x10000 / (unsigned long long)uVar2) + -1;
      piVar4 = piVar4 + 1;
      uVar2 = uVar2 + 1;
    } while (uVar2 != 0x3a9b);

    for (ppuVar5 = (char **)PTR_FUN_004abe6c; *ppuVar5 != (char *)0x0; ppuVar5 = ppuVar5 + 1) {
      (*(void (*)())*ppuVar5)(ppuVar5,unaff_ESI,unaff_EBX);
    }

    piVar4 = &DAT_0049ca44;
    iVar3 = 0;
    sVar1 = 600;

    do {
      *piVar4 = iVar3;
      piVar4 = piVar4 + 1;
      iVar3 = iVar3 + in_EAX;
      sVar1 = sVar1 + -1;
    } while (sVar1 != 0);
    return (unsigned long long)param_2 << 0x20;
  }

  return (((long long)(param_2) << 32) | ((unsigned int)(0xffffffff)));
}

/**
 * @original Lisa_ExecuteRasterizerCommands (IGN_WIN.EXE @ 0x0044f0e9, lisa3d.c)
 * @fidelity ADAPTED
 */
long long Lisa_ExecuteRasterizerCommands(int param_1,unsigned int param_2) {
  int *unaff_ESI;
  long long lVar1;

  

  _DAT_0049c9e4 = unaff_ESI;
  _DAT_0049c9e8 = (int *)*unaff_ESI;
  _DAT_0049c9f0 = unaff_ESI[1];
  _DAT_0049c9f8 = unaff_ESI[3];
  _DAT_0049c9fc = unaff_ESI[4];
  _DAT_0049ca00 = unaff_ESI[5];
  _DAT_0049ca0c = _DAT_0049c9fc;
  _DAT_0049ca1c = _DAT_0049c9fc << 8;
  _DAT_0049ca10 = _DAT_0049ca00;
  _DAT_0049ca20 = _DAT_0049ca00 << 8;
  g_SubpixelMinX = _DAT_0049ca1c;
  g_SubpixelMinY = _DAT_0049ca20;
  _DAT_0049ca04 = unaff_ESI[6];
  _DAT_0049ca08 = unaff_ESI[7];
  _DAT_0049ca14 = _DAT_0049ca04 + 1;
  _DAT_0049ca18 = _DAT_0049ca08 + 1;
  g_SubpixelMaxX = _DAT_0049ca14 * 0x100;
  _DAT_0049ca24 = g_SubpixelMaxX + -1;
  g_SubpixelMaxY = _DAT_0049ca18 * 0x100;
  _DAT_0049ca28 = g_SubpixelMaxY + -1;
  _DAT_0049c9ec = (int *)*_DAT_0049c9e8;

  if (_DAT_0049c9ec != (int *)0x0) {
    lVar1 = (*(long long (*)())(((void **)PTR_DAT_004abe10)[*_DAT_0049c9ec]))();
    return lVar1;
  }

  return (unsigned long long)param_2 << 0x20;
}
