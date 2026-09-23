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
 * @fidelity ADAPTED
 * @notes Prints Lisa 2 Development System version and UDS copyright header.
 */
int Lisa_PrintVersion(void) {
    printf("Lisa 2 Development System: %s\n", "Compilation 0.91.0");
    printf("Copyright (c) UDS, 1995-1996\n");
    return 0;
}

/**
 * @original Cdp_OpenFile (IGN_WIN.EXE @ 0x00412580, lisa3d.c)
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
 * @original Cdp_DecompressRLE (IGN_WIN.EXE @ 0x00499abc, lisa3d.c)
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
 * @original Cdp_DecodeFrame (IGN_WIN.EXE @ 0x00412610, lisa3d.c)
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
 * @original Lisa_DrawTexturedTriangle_Op11_Shaded (IGN_WIN.EXE @ 0x0044e1b0, lisa3d.c)
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
LisaReturn64 Lisa_CreateDynamicObject(int param_1, int object_id, LisaEntityTransform *entity, MshSubmesh *mesh, int param_5, short param_6, short param_7, short base_elevation, short param_9);
int Lisa_MoveDynamicObject(LisaEntityTransform *entity);
int Lisa_UpdateObjectSpatialGrid(LisaEntityTransform *entity);
LisaReturn64 Lisa_SetDynamicObjectMesh(int param_1, int param_2, LisaEntityTransform *entity, MshSubmesh *mesh, int param_5, short param_6, short param_7, short base_elevation, short param_9);
int Lisa_DeleteDynamicObject(LisaEntityTransform *entity);
LisaReturn64 Lisa_SetCameraViewport(void);
int Lisa_GenerateMipmaps(unsigned int *texture_data, int width, int height, int page_w, int page_h, int mip_count, int flag1, int flag2, char *name);
void Lisa_GenerateTextureSpanTable(int src_w, int src_h, int dst_w, int dst_h, int *span_table);
void Lisa_DownsampleTextureMipmap(byte *src, byte *dst, int src_w, int src_h, int dst_w, int dst_h);
void Lisa_FilterTextureBlock(byte *src, int src_w, int src_h, int dst_w, int dst_h, int filter_mode, int stride, int flags);
void Lisa_LoadOrCreateShadingTable(int shade_level, int lighting_mode);
unsigned int Lisa_FindClosestPaletteColor(int *rgb, int palette_offset);
LisaReturn64 Lisa_RenderSkyBackdrop(void);
int Lisa_CullObjectsOrthographic(void);
int Lisa_FrustumCullObjects(void);
int Lisa_CullObjects(void);
void Lisa_TransformVertices(void);
int Lisa_TransformVerticesPanorama(void);
int Lisa_ComputeObjectMatrix(int pos_x, int pos_y, int pos_z, int rot_y, int *out_matrix);
int Lisa_TransformSubmeshVerticesPanorama(int pos_x, int pos_y, int pos_z, int rot_y, int *out_matrix);
void Lisa_ComputeCameraRotationMatrix(int *out_matrix);
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
extern void *PTR_Lisa_DrawTexturedTriangle_Op15_0049c934;
extern void *PTR_LAB_0049c924;
extern void *g_LisaRasterizerJmpTable[];
extern void *g_pLisaShutdownCallbacks[];
extern void *g_LisaOpcodeTable[];
extern void *PTR_LAB_0049c924;
extern void *PTR_Lisa_DrawTexturedTriangle_Op15_0049c934;
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
 * @original Lisa_RenderScene (IGN_WIN.EXE @ 0x004466d0, lisa3d.c)
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
 * @original Lisa_InitEngineMemory (IGN_WIN.EXE @ 0x004468d0, lisa3d.c)
 * @fidelity ADAPTED
 */
int * Lisa_InitEngineMemory(void) {
  void *pvVar1;
  void *pvVar2;
  void *pvVar3;
  void *pvVar4;
  int *var_pu5;
  void *pvVar6;
  int var_i7;
  int var_i8;
  double var_f9;

  

  g_pLisaAllocatedBuffers = calloc(5000,4);
  g_LisaAllocatedBufferCount = 0;
  g_LisaVisibleObjects = calloc(0x5dc,4);
  g_LisaVisibleSubmeshes = calloc(3000,4);
  var_pu5 = calloc(0x1838,4);
  var_i7 = g_LisaActivePageCount + 2;
  g_pLisaDepthBuckets = var_pu5;
  g_LisaActivePageCount = var_i7;
  *(int **)(&g_pLisaTexturePagePointers + var_i7 * 4) = var_pu5;
  *(int *)(&g_LisaTexturePageSizes + var_i7 * 4) = 0x1a90;

  for (var_i7 = 6000; var_i7 != 0; var_i7 = var_i7 + -1) {
    *var_pu5 = 0;
    var_pu5 = var_pu5 + 1;
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
  pvVar6 = calloc(0x1fa4,4);
  pvVar3 = g_LisaVisibleSubmeshes;
  pvVar2 = g_pLisaAllocatedBuffers;
  g_LisaActivePageCount = g_LisaActivePageCount + 2;
  g_LisaActiveShdSize = pvVar6;
  *(void **)(&g_pLisaTexturePagePointers + g_LisaActivePageCount * 4) = pvVar6;
  *(int *)(&g_LisaTexturePageSizes + g_LisaActivePageCount * 4) = 0x2a30;
  *(void **)((int)pvVar2 + (g_LisaAllocatedBufferCount + 3) * 4 + -0xc) = g_LisaVisibleObjects;
  pvVar1 = g_LisaDrawCommandBuffer;
  var_pu5 = g_pLisaDepthBuckets;
  *(void **)((int)pvVar2 + (g_LisaAllocatedBufferCount + 5) * 4 + -0x10) = pvVar3;
  pvVar3 = g_LisaTransformedVertices;
  *(int **)((int)pvVar2 + (g_LisaAllocatedBufferCount + 5) * 4 + -0xc) = var_pu5;
  pvVar4 = g_pLisaDrawCommandTail;
  var_pu5 = g_LisaCamera;
  *(int **)((int)pvVar2 + (g_LisaAllocatedBufferCount + 7) * 4 + -0x10) = g_LisaCamera;
  var_i7 = g_LisaAllocatedBufferCount + 9;
  g_LisaAllocatedBufferCount = var_i7;
  *(void **)((int)pvVar2 + var_i7 * 4 + -0x14) = pvVar3;
  *(void **)((int)pvVar2 + var_i7 * 4 + -0x10) = pvVar1;
  pvVar1 = g_pLisaTextureSheets;
  *(void **)((int)pvVar2 + var_i7 * 4 + -0xc) = g_pLisaTextureSheets;
  *(void **)((int)pvVar2 + var_i7 * 4 + -8) = pvVar4;
  *(void **)((int)pvVar2 + var_i7 * 4 + -4) = pvVar6;

  if (var_pu5 == (int *)0x0) {
    return (int *)0x0;
  }

  g_LisaActiveShdSize = pvVar6;

  if (pvVar3 == (void *)0x0) {
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

  if (pvVar6 == (void *)0x0) {
    return (int *)0x0;
  }

  if (pvVar1 == (void *)0x0) {
    return (int *)0x0;
  }

  var_pu5[0xe] = 1;
  var_pu5[0xf] = 1;
  var_pu5[0x10] = 1;
  var_pu5[0x11] = 1;
  var_pu5[0x12] = 1;
  var_pu5[0x13] = 1;
  var_pu5[0x14] = 1;
  var_pu5[0x15] = 1;
  var_pu5[0x16] = 1;
  var_pu5[0x23] = 1;
  var_pu5[0x24] = 1;
  var_pu5[0x25] = 1;
  var_pu5[0x20] = 0xfd;
  var_pu5[0x21] = 0xcf;
  var_pu5[0x22] = 0x2c;
  var_pu5[0x26] = 10;
  var_pu5[6] = 0;
  var_pu5[8] = 0;
  var_pu5[7] = 0;
  var_pu5[9] = 0;
  var_pu5[10] = 0;
  *var_pu5 = 0;
  var_pu5[0xb] = 0;
  var_pu5[1] = 0;
  var_pu5[2] = 0;
  var_pu5[4] = 0;
  var_pu5[3] = 0;
  var_pu5[5] = 0;
  var_pu5[0xc] = 0;
  var_pu5[0x29] = 0;
  var_pu5[0x27] = 0xa0;
  var_pu5[0xd] = 0x3ff00000;
  var_pu5[0x28] = 100;
  var_i7 = -0x708;

  while( 1 ) {
    var_i8 = var_i7 + 1;
    var_f9 = (double)fsin((double)var_i7 * (double)g_Const_0_1 * (double)g_Const_DegToRad);
    if (0x189b < var_i8) break;
    *(float *)((int)pvVar6 + var_i8 * 4 + 0x1c1c) = (float)var_f9;
    var_i7 = var_i8;
  }

  *(float *)((int)pvVar6 + var_i8 * 4 + 0x1c1c) = (float)var_f9;
  g_LisaActiveTabSize = (int)pvVar6 + 0xe10;
  return var_pu5;
}

/**
 * @original Lisa_FreeEngineMemory (IGN_WIN.EXE @ 0x00446c30, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_FreeEngineMemory(void) {
  void *_Memory;
  int var_i1;
  int var_i2;
  int var_i3;

  

  var_i2 = 0;

  if (g_pLisaAllocatedBuffers != (void *)0x0) {
    _Memory = g_pLisaAllocatedBuffers;

    if (0 < g_LisaAllocatedBufferCount) {
      var_i3 = 0;
      var_i1 = g_LisaAllocatedBufferCount;

      do {
        if (*(void **)(var_i3 + (int)_Memory) != (void *)0x0) {
          _free(*(void **)(var_i3 + (int)_Memory));
          _Memory = g_pLisaAllocatedBuffers;
          var_i1 = g_LisaAllocatedBufferCount;
        }

        var_i3 = var_i3 + 4;
        var_i2 = var_i2 + 1;
      } while (var_i2 < var_i1);
    }

    _free(_Memory);
  }

  g_pLisaAllocatedBuffers = (void *)0x0;
  g_LisaAllocatedBufferCount = 0;
  g_LisaActivePageCount = 0;
  g_LisaEnableMipmaps = 1;
  g_LisaShadingEnabled = 0;
  return;
}

/**
 * @original Lisa_InitSpatialGrid (IGN_WIN.EXE @ 0x00446ca0, lisa3d.c)
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
 * @original Lisa_CreateDynamicObject (IGN_WIN.EXE @ 0x00446d90, lisa3d.c)
 * @fidelity ADAPTED
 */
LisaReturn64 Lisa_CreateDynamicObject(int param_1, int object_id, LisaEntityTransform *entity, MshSubmesh *mesh, int param_5, short param_6, short param_7, short base_elevation, short param_9) {
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
  obj->unknown_08 = param_5;
  obj->pos_x = entity->pos_x;
  obj->pos_y = entity->pos_y;
  obj->pos_z = entity->pos_z;
  obj->rot_x = (short)entity->rot_x;
  obj->rot_y = (short)entity->rot_y;
  obj->rot_z = (short)entity->rot_z;
  obj->unknown_1e = param_6;
  obj->unknown_20 = param_7;
  obj->unknown_22 = base_elevation;
  obj->unknown_24 = param_9;
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
 * @original Lisa_MoveDynamicObject (IGN_WIN.EXE @ 0x00446eb0, lisa3d.c)
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
 * @original Lisa_UpdateObjectSpatialGrid (IGN_WIN.EXE @ 0x00446f30, lisa3d.c)
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
 * @original Lisa_SetDynamicObjectMesh (IGN_WIN.EXE @ 0x00447070, lisa3d.c)
 * @fidelity ADAPTED
 */
LisaReturn64 Lisa_SetDynamicObjectMesh(int param_1, int param_2, LisaEntityTransform *entity, MshSubmesh *mesh, int param_5, short param_6, short param_7, short base_elevation, short param_9) {
  LisaDynamicObject *obj;
  int max_dist;
  int i;
  int *vertices;
  float fx, fy, fz;
  float dist;
  unsigned int ret_val;

  obj = entity->dyn_obj;
  
  obj->unknown_1e = param_6;
  obj->unknown_22 = base_elevation;
  obj->mesh_data = mesh;
  obj->unknown_08 = param_5;
  obj->unknown_20 = param_7;
  obj->unknown_24 = param_9;
  
  ret_val = ((unsigned int)param_2 >> 16) | ((unsigned int)base_elevation << 16);

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
 * @original Lisa_DeleteDynamicObject (IGN_WIN.EXE @ 0x00447150, lisa3d.c)
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
 * @original Lisa_SetCameraViewport (IGN_WIN.EXE @ 0x004471e0, lisa3d.c)
 * @fidelity ADAPTED
 */
LisaReturn64 Lisa_SetCameraViewport(void) {
  int var_i1;
  int var_l2;
  unsigned int var_u3;

  

  var_l2 = __ftol();
  camera = g_LisaCamera;
  g_LisaCamera->viewport_x = (int)var_l2;
  var_l2 = __ftol();
  camera->viewport_y = (int)var_l2;
  var_l2 = __ftol();
  camera->viewport_width = (int)var_l2;
  var_l2 = __ftol();
  camera->fov_x = (int)var_l2;
  var_u3 = __ftol();
  camera->fov_y = (int)var_u3;
  { LisaReturn64 _r; _r.edx = var_u3; _r.eax = 0; return _r; }
}

/**
 * @original Lisa_GenerateMipmaps (IGN_WIN.EXE @ 0x00447280, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_GenerateMipmaps(unsigned int *param_1,int param_2,int param_3,int param_4,int param_5,int param_6 ,int param_7,int param_8,char *param_9) {
  char var_u1;
  char *var_pu2;
  char *var_pu3;
  unsigned int *var_pu4;
  unsigned int *var_pu5;
  void *pvVar6;
  byte *pbVar7;
  int var_i8;
  int var_i9;
  int var_i10;
  int var_i11;
  byte *pbVar12;
  byte *pbVar13;
  unsigned int *var_pu14;
  int var_b15;
  int var_l16;
  byte *var_440;
  int var_438;
  byte *var_428;
  byte *var_424;
  int var_414;
  int var_40c;
  char var_400 [1024];

  

  var_pu2 = var_400;

  do {
    var_u1 = param_9[1];
    *var_pu2 = *param_9;
    var_pu2[1] = var_u1;
    var_pu3 = var_pu2 + 4;
    var_pu2[2] = param_9[2];
    var_pu2 = var_pu3;
    param_9 = param_9 + 3;
  } while (var_pu3 < (unsigned int *)(var_400 + sizeof(var_400)));
  var_pu4 = calloc(0x15,4);
  g_LisaAllocatedBufferCount = g_LisaAllocatedBufferCount + 1;
  *(unsigned int **)(g_pLisaAllocatedBuffers + -4 + g_LisaAllocatedBufferCount * 4) = var_pu4;
  var_pu5 = var_pu4 + 5;
  var_l16 = __ftol();
  var_438 = 0;

  if (0 < (int)var_l16) {
    do {
      if (5999 < var_438) break;
      var_438 = var_438 + 1;
    } while (var_438 < (int)var_l16);
  }

  var_l16 = __ftol();
  var_i9 = (int)var_l16;
  var_440 = (byte *)*param_1;
  *var_pu4 = (unsigned int)var_440;
  var_pu4[1] = (unsigned int)var_440;
  var_pu4[2] = (unsigned int)var_440;
  var_pu4[3] = (unsigned int)var_440;
  var_pu4[4] = (unsigned int)var_440;
  *var_pu5 = (unsigned int)var_440;
  var_i10 = 0;

  if (0 < param_4 + -1) {
    var_i11 = var_i10;

    do {
      g_LisaEnableMipmaps = 1;
      g_LisaShadingEnabled = 1;
      pvVar6 = calloc(param_7 * 0x100 + 0xffff,1);
      g_LisaAllocatedBufferCount = g_LisaAllocatedBufferCount + 1;
      *(void **)(g_pLisaAllocatedBuffers + -4 + g_LisaAllocatedBufferCount * 4) = pvVar6;
      pbVar7 = (byte *)((int)pvVar6 + 0xffffU & 0xffff0000);
      var_i8 = g_LisaActivePageCount + 2;
      g_LisaActivePageCount = var_i8;
      *(byte **)(&g_pLisaTexturePagePointers + var_i8 * 4) = pbVar7;
      var_i10 = var_i11 + 1;
      *(int *)(&g_LisaTexturePageSizes + var_i8 * 4) = param_7 << 8;

      if (var_i10 < 5) {
        var_pu4[var_i11 + 1] = (unsigned int)pbVar7;
      }

      var_i8 = 0;

      if (0 < var_i9) {
        do {
          if (5999 < var_438) break;
          var_i8 = var_i8 + 1;
          var_438 = var_438 + 1;
        } while (var_i8 < var_i9);
      }

      if (0 < param_7 / param_6) {
        var_424 = var_440;
        var_i9 = (int)(0x100 / (int)param_5);
        var_428 = pbVar7;
        var_40c = param_7 / param_6;

        do {
          pbVar12 = var_428;
          pbVar13 = var_424;
          var_414 = var_i9;

          if (0 < var_i9) {
            do {
              if (var_i11 == 0) {
                Lisa_DownsampleTextureMipmap(pbVar13,pbVar12,param_5,param_6,0x100,(int)var_400);
              }

              else {
                Lisa_FilterTextureBlock(pbVar13,(int)pbVar12,param_5,param_6,0x100,(int)var_400,param_8,

                             var_i11);
              }

              var_414 = var_414 + -1;
              pbVar12 = pbVar12 + param_5;
              pbVar13 = pbVar13 + param_5;
            } while (var_414 != 0);
          }

          var_428 = var_428 + param_6 * 0x100;
          var_424 = var_424 + param_6 * 0x100;
          var_40c = var_40c + -1;
        } while (var_40c != 0);
      }

      var_l16 = __ftol();
      var_i9 = (int)var_l16;
      var_i11 = var_i10;
      var_440 = pbVar7;
    } while (var_i10 < param_4 + -1);
  }

  if (var_i10 < 4) {
    var_pu14 = var_pu4 + var_i10 + 1;

    for (var_i9 = 4 - var_i10; var_i9 != 0; var_i9 = var_i9 + -1) {
      *var_pu14 = (unsigned int)var_440;
      var_pu14 = var_pu14 + 1;
    }

  }

  var_b15 = g_LisaDisableFiltering == 0;
  *param_1 = (unsigned int)var_pu5;

  if ((var_b15) && (1 < param_4)) {
    Lisa_GenerateTextureSpanTable(*var_pu4,(int)var_400,param_8,param_7,var_pu5);
  }

  return 0;
}

/**
 * @original Lisa_GenerateTextureSpanTable (IGN_WIN.EXE @ 0x004475c0, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_GenerateTextureSpanTable(int param_1,int param_2,int param_3,int param_4,int *param_5) {
  unsigned int var_u1;
  int var_i2;
  byte var_b3;
  unsigned int var_u4;
  unsigned int var_u5;
  int var_i6;
  void *pvVar7;
  unsigned int *var_pu8;
  byte *pbVar9;
  unsigned int var_u10;
  byte var_b11;
  int *var_pu12;
  int var_i13;
  unsigned int var_u14;
  byte var_b15;
  unsigned int *var_pu16;
  unsigned int var_c4 [3];
  unsigned int var_b8;
  int var_b4;
  unsigned int *var_b0;
  unsigned int *var_ac;
  int var_a8 [3];
  int var_9c;
  int *var_98;
  int var_94;
  int var_90;
  int var_8c;
  int *var_88;
  int var_84;
  int var_80;
  int var_7c;
  int var_78;
  int var_74;
  int var_70;
  int var_6c;
  int var_68;
  int var_64;
  int var_60;
  int var_5c;
  int var_58;
  int var_54;
  int var_50;
  unsigned int *var_4c;
  int var_44;
  int var_40;
  unsigned int var_3c;
  unsigned int var_38;
  unsigned int var_34;
  unsigned int var_30;
  unsigned int var_2c;
  unsigned int var_28;
  unsigned int var_24;
  unsigned int var_20;
  unsigned int var_1c;
  unsigned int var_18;
  unsigned int var_14;
  unsigned int var_10;
  size_t var_c;
  int var_8;
  int var_4;

  

  var_88 = calloc(0x10000,0x14);
  *var_88 = 1;
  var_i6 = 0x10000;
  var_pu12 = var_88;

  do {
    *var_pu12 = 0;
    var_pu12 = var_pu12 + 5;
    var_i6 = var_i6 + -1;
  } while (var_i6 != 0);
  var_98 = param_5;
  var_c = param_4 * 0x400 + 0xffff;
  var_94 = 0;
  var_8 = param_4 << 10;

  do {
    pvVar7 = calloc(var_c,1);
    g_LisaAllocatedBufferCount = g_LisaAllocatedBufferCount + 1;
    *(void **)(g_pLisaAllocatedBuffers + -4 + g_LisaAllocatedBufferCount * 4) = pvVar7;
    var_pu8 = (unsigned int *)((int)pvVar7 + 0xffffU & 0xffff0000);
    var_i6 = g_LisaActivePageCount + 2;
    var_pu12 = var_98 + 4;
    g_LisaActivePageCount = var_i6;
    *(unsigned int **)(&g_pLisaTexturePagePointers + var_i6 * 4) = var_pu8;
    *(int *)(&g_LisaTexturePageSizes + var_i6 * 4) = var_8;
    *var_98 = var_pu8;
    var_8c = 0;
    var_98[1] = var_pu8;
    var_98[2] = var_pu8;
    var_98[3] = var_pu8;

    if (0 < param_4) {
      var_4 = param_4 + -1;
      var_90 = 0;
      var_4c = var_pu8;

      do {
        var_a8[0] = 0;
        var_a8[1] = 0;
        var_a8[2] = 0;
        var_b4 = 0;
        var_ac = var_4c;

        do {
          var_b3 = *(byte *)(var_94 + var_b4 + param_1 + var_90);
          var_b8 = ((((unsigned int)((((unsigned int)(var_b8)) >> 8))) << 8) | ((unsigned char)(var_b3)));

          if (var_b4 < 0x3f) {
            pbVar9 = (byte *)(var_94 + var_b4 + param_1 + var_90);
            var_b15 = pbVar9[1];
          }

          else {
            pbVar9 = (byte *)(var_94 + var_b4 + param_1 + var_90);
            var_b15 = *pbVar9;
          }

          var_9c = ((((unsigned int)((((unsigned int)(var_9c)) >> 8))) << 8) | ((unsigned char)(var_b15)));

          if (var_8c < var_4) {
            var_b11 = pbVar9[0x100];

            if (var_b4 < 0x3f) {
              var_pu8 = (unsigned int *)(unsigned int)pbVar9[0x101];
            }

            else {
LAB_00447785:

              var_pu8 = (unsigned int *)(unsigned int)var_b11;
            }

          }

          else {
            var_b11 = *pbVar9;
            if (0x3e < var_b4) goto LAB_00447785;
            var_pu8 = (unsigned int *)(unsigned int)pbVar9[1];
          }

          var_u10 = (unsigned int)var_b11;
          var_u14 = (unsigned int)var_b15;
          var_u4 = (unsigned int)var_b3;
          var_u1 = (((int)var_pu8 * 0x100 + var_u10) * 0x100 + var_u14) * 0x100 + var_u4;
          var_pu16 = var_88 + ((var_u1 >> 0x11) + var_u10 + var_u14 + var_u1 & 0xffff) * 5;
          var_b0 = var_pu8;

          if (*var_pu16 != var_u1) {
            *var_pu16 = var_u1;
            var_b8 = (unsigned int)*(byte *)(param_2 + var_u4 * 4);
            var_u1 = var_b8;
            var_30 = var_b8;
            var_b8 = (unsigned int)*(byte *)(param_2 + 1 + var_u4 * 4);
            var_u5 = var_b8;
            var_2c = var_b8;
            var_b8 = (unsigned int)*(byte *)(param_2 + 2 + var_u4 * 4);
            var_u4 = var_b8;
            var_28 = var_b8;
            var_3c = (unsigned int)*(byte *)(param_2 + var_u14 * 4);
            var_38 = (unsigned int)*(byte *)(param_2 + 1 + var_u14 * 4);
            var_34 = (unsigned int)*(byte *)(param_2 + 2 + var_u14 * 4);
            var_18 = (unsigned int)*(byte *)(param_2 + var_u10 * 4);
            var_14 = (unsigned int)*(byte *)(param_2 + 1 + var_u10 * 4);
            var_10 = (unsigned int)*(byte *)(param_2 + 2 + var_u10 * 4);
            var_24 = (unsigned int)*(byte *)(param_2 + (int)var_pu8 * 4);
            var_20 = (unsigned int)*(byte *)(param_2 + 1 + (int)var_pu8 * 4);
            var_b0 = var_ac;
            var_9c = 0;
            var_74 = 0;
            var_1c = (unsigned int)*(byte *)(param_2 + 2 + (int)var_pu8 * 4);
            var_6c = 0;
            var_78 = var_34 << 2;
            var_64 = 0;
            var_70 = var_38 << 2;
            var_68 = var_3c << 2;
            var_5c = 0;
            var_54 = 0;
            var_60 = var_b8 << 2;
            var_b8 = 4;
            var_58 = var_u5 * 4;
            var_50 = var_u1 << 2;
            var_pu8 = var_pu16;

            do {
              var_i6 = (int)(var_54 + var_50 + (var_54 + var_50 >> 0x1f & 3U)) >> 2;
              var_44 = (int)(var_5c + var_58 + (var_5c + var_58 >> 0x1f & 3U)) >> 2;
              var_40 = (int)(var_64 + var_60 + (var_64 + var_60 >> 0x1f & 3U)) >> 2;
              var_84 = (int)(var_6c + var_68 + (var_6c + var_68 >> 0x1f & 3U)) >> 2;
              var_80 = (int)(var_74 + var_70 + (var_74 + var_70 >> 0x1f & 3U)) >> 2;
              var_7c = (int)(var_9c + var_78 + (var_9c + var_78 >> 0x1f & 3U)) >> 2;
              var_c4[0] = var_i6 * 4;
              var_i13 = 0;
              var_c4[1] = var_44 << 2;
              var_c4[2] = var_40 << 2;

              do {
                var_i2 = *(int *)((int)var_a8 + var_i13) + 8 + *(int *)((int)var_c4 + var_i13);
                *(int *)((int)var_c4 + var_i13) = var_i2;

                if (0x3ff < var_i2) {
                  *(int *)((int)var_c4 + var_i13) = 0x3ff;
                }

                if (*(int *)((int)var_c4 + var_i13) < 0) {
                  *(int *)((int)var_c4 + var_i13) = 0;
                }

                var_i13 = var_i13 + 4;
              } while (var_i13 < 0xc);

              var_b3 = (&g_LisaObjectMatrix_22)

                      [(var_c4[1] & 0x3f0) * 4 +

                       ((var_c4[2] & 0x3f0) >> 4) + (var_c4[0] & 0x3f0) * 0x100];
              var_u14 = (unsigned int)var_b3;
              *(byte *)(var_pu8 + 1) = var_b3;
              var_b15 = *(byte *)(param_2 + var_u14 * 4);
              *(byte *)var_b0 = var_b3;
              var_a8[0] = (int)(var_c4[0] + (unsigned int)var_b15 * -4) / 2;
              var_a8[1] = (int)(var_c4[1] + (unsigned int)*(byte *)(param_2 + 1 + var_u14 * 4) * -4) / 2;
              var_a8[2] = (int)(var_c4[2] + (unsigned int)*(byte *)(param_2 + 2 + var_u14 * 4) * -4) / 2;
              var_c4[0] = var_i6 * 3 + var_84;
              var_c4[1] = var_44 * 3 + var_80;
              var_i13 = 0;
              var_c4[2] = var_40 * 3 + var_7c;

              do {
                var_i2 = *(int *)((int)var_a8 + var_i13) + 8 + *(int *)((int)var_c4 + var_i13);
                *(int *)((int)var_c4 + var_i13) = var_i2;

                if (0x3ff < var_i2) {
                  *(int *)((int)var_c4 + var_i13) = 0x3ff;
                }

                if (*(int *)((int)var_c4 + var_i13) < 0) {
                  *(int *)((int)var_c4 + var_i13) = 0;
                }

                var_i13 = var_i13 + 4;
              } while (var_i13 < 0xc);

              var_u14 = (unsigned int)(byte)(&g_LisaObjectMatrix_22)

                                   [(var_c4[1] & 0x3f0) * 4 +

                                    ((var_c4[2] & 0x3f0) >> 4) + (var_c4[0] & 0x3f0) * 0x100];

              *(char *)((int)var_pu8 + 5) =

                   (&g_LisaObjectMatrix_22)

                   [(var_c4[1] & 0x3f0) * 4 +

                    ((var_c4[2] & 0x3f0) >> 4) + (var_c4[0] & 0x3f0) * 0x100];
              var_a8[0] = (int)(var_c4[0] + (unsigned int)*(byte *)(param_2 + var_u14 * 4) * -4) / 2;
              var_a8[1] = (int)(var_c4[1] + (unsigned int)*(byte *)(param_2 + 1 + var_u14 * 4) * -4) / 2;
              var_a8[2] = (int)(var_c4[2] + (unsigned int)*(byte *)(param_2 + 2 + var_u14 * 4) * -4) / 2;
              var_c4[0] = (var_84 + var_i6) * 2;
              var_c4[1] = (var_80 + var_44) * 2;
              var_i13 = 0;
              var_c4[2] = (var_7c + var_40) * 2;

              do {
                var_i2 = *(int *)((int)var_a8 + var_i13) + 8 + *(int *)((int)var_c4 + var_i13);
                *(int *)((int)var_c4 + var_i13) = var_i2;

                if (0x3ff < var_i2) {
                  *(int *)((int)var_c4 + var_i13) = 0x3ff;
                }

                if (*(int *)((int)var_c4 + var_i13) < 0) {
                  *(int *)((int)var_c4 + var_i13) = 0;
                }

                var_i13 = var_i13 + 4;
              } while (var_i13 < 0xc);

              var_u14 = (unsigned int)(byte)(&g_LisaObjectMatrix_22)

                                   [(var_c4[1] & 0x3f0) * 4 +

                                    ((var_c4[2] & 0x3f0) >> 4) + (var_c4[0] & 0x3f0) * 0x100];

              *(char *)((int)var_pu8 + 6) =

                   (&g_LisaObjectMatrix_22)

                   [(var_c4[1] & 0x3f0) * 4 +

                    ((var_c4[2] & 0x3f0) >> 4) + (var_c4[0] & 0x3f0) * 0x100];
              var_a8[0] = (int)(var_c4[0] + (unsigned int)*(byte *)(param_2 + var_u14 * 4) * -4) / 2;
              var_a8[1] = (int)(var_c4[1] + (unsigned int)*(byte *)(param_2 + 1 + var_u14 * 4) * -4) / 2;
              var_a8[2] = (int)(var_c4[2] + (unsigned int)*(byte *)(param_2 + 2 + var_u14 * 4) * -4) / 2;
              var_c4[0] = var_84 * 3 + var_i6;
              var_i6 = 0;
              var_c4[1] = var_80 * 3 + var_44;
              var_c4[2] = var_7c * 3 + var_40;

              do {
                var_i13 = *(int *)((int)var_a8 + var_i6) + 8 + *(int *)((int)var_c4 + var_i6);
                *(int *)((int)var_c4 + var_i6) = var_i13;

                if (0x3ff < var_i13) {
                  *(int *)((int)var_c4 + var_i6) = 0x3ff;
                }

                if (*(int *)((int)var_c4 + var_i6) < 0) {
                  *(int *)((int)var_c4 + var_i6) = 0;
                }

                var_i6 = var_i6 + 4;
              } while (var_i6 < 0xc);

              var_u14 = (unsigned int)(byte)(&g_LisaObjectMatrix_22)

                                   [(var_c4[1] & 0x3f0) * 4 +

                                    ((var_c4[2] & 0x3f0) >> 4) + (var_c4[0] & 0x3f0) * 0x100];

              *(char *)((int)var_pu8 + 7) =

                   (&g_LisaObjectMatrix_22)

                   [(var_c4[1] & 0x3f0) * 4 +

                    ((var_c4[2] & 0x3f0) >> 4) + (var_c4[0] & 0x3f0) * 0x100];
              var_a8[0] = (int)(var_c4[0] + (unsigned int)*(byte *)(param_2 + var_u14 * 4) * -4) / 2;
              var_a8[1] = (int)(var_c4[1] + (unsigned int)*(byte *)(param_2 + 1 + var_u14 * 4) * -4) / 2;
              var_b0 = var_b0 + 0x40;
              var_a8[2] = (int)(var_c4[2] + (unsigned int)*(byte *)(param_2 + 2 + var_u14 * 4) * -4) / 2;
              var_9c = var_9c + var_1c;
              var_78 = var_78 - var_34;
              var_74 = var_74 + var_20;
              var_70 = var_70 - var_38;
              var_6c = var_6c + var_24;
              var_68 = var_68 - var_3c;
              var_64 = var_64 + var_10;
              var_60 = var_60 - var_u4;
              var_5c = var_5c + var_14;
              var_58 = var_58 - var_u5;
              var_54 = var_54 + var_18;
              var_50 = var_50 - var_u1;
              var_b8 = var_b8 + -1;
              var_pu8 = var_pu8 + 1;
            } while (var_b8 != 0);
            var_a8[0] = 0;
            var_a8[1] = 0;
            var_a8[2] = 0;
            var_b8 = 0;
          }

          var_a8[2] = 0;
          var_a8[1] = 0;
          var_a8[0] = 0;
          var_u1 = var_pu16[2];
          var_pu8 = var_ac + 1;
          var_b4 = var_b4 + 1;
          *var_ac = var_pu16[1];
          var_u14 = var_pu16[3];
          var_ac[0x40] = var_u1;
          var_u1 = var_pu16[4];
          var_ac[0x80] = var_u14;
          var_ac[0xc0] = var_u1;
          var_ac = var_pu8;
        } while (var_b4 < 0x40);
        var_90 = var_90 + 0x100;
        var_4c = var_4c + 0x100;
        var_8c = var_8c + 1;
      } while (var_8c < param_4);
    }

    var_94 = var_94 + 0x40;
    var_98 = var_pu12;

    if (0xff < var_94) {
      _free(var_88);
      return;
    }

  } while( 1 );
}

/**
 * @original Lisa_DownsampleTextureMipmap (IGN_WIN.EXE @ 0x00447fb0, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_DownsampleTextureMipmap(byte *param_1,byte *param_2,int param_3,int param_4,int param_5,int param_6) {
  int var_i1;
  byte var_b2;
  int var_i3;
  unsigned int *var_pu4;
  unsigned int var_u5;
  unsigned int var_u6;
  int var_i7;
  int var_i8;
  int var_i9;
  int var_2c;
  int var_24;
  unsigned int var_1c [4];
  int var_c [3];

  

  var_24 = 0;

  if (0 < param_4) {
    do {
      var_2c = 0;
      var_c[0] = 0;
      var_c[1] = 0;
      var_c[2] = 0;

      if (0 < param_3) {
        var_i3 = param_3 + -1;

        do {
          var_i7 = 0;
          var_pu4 = var_1c;
          var_b2 = *param_1;

          do {
            var_u6 = (unsigned int)*(byte *)((unsigned int)var_b2 * 4 + var_i7 + param_6);
            *var_pu4 = var_u6;

            if (var_2c < var_i3) {
              var_u5 = *(byte *)(var_i7 + (unsigned int)param_1[1] * 4 + param_6) + var_u6;
            }

            else {
              var_u5 = var_u6 * 2;
            }

            *var_pu4 = var_u5;

            if (var_24 < param_4 + -1) {
              var_u6 = (unsigned int)*(byte *)(var_i7 + (unsigned int)param_1[param_5] * 4 + param_6);
              var_u5 = *var_pu4 + var_u6;
              *var_pu4 = var_u5;

              if (var_2c < var_i3) {
                *var_pu4 = *(byte *)(var_i7 + (unsigned int)param_1[param_5 + 1] * 4 + param_6) + var_u5;
              }

              else {
                *var_pu4 = var_u6 + var_u5;
              }

            }

            else {
              var_u5 = *var_pu4 + var_u6;
              *var_pu4 = var_u5;

              if (var_2c < var_i3) {
                *var_pu4 = *(byte *)(var_i7 + (unsigned int)param_1[1] * 4 + param_6) + var_u5;
              }

              else {
                *var_pu4 = var_u5 + var_u6;
              }

            }

            var_pu4 = var_pu4 + 1;
            var_i7 = var_i7 + 1;
          } while (var_pu4 < var_1c + 3);
          var_i7 = 0;

          do {
            var_i9 = *(int *)((int)var_c + var_i7) + 8 + *(int *)((int)var_1c + var_i7);
            *(int *)((int)var_1c + var_i7) = var_i9;

            if (0x3ff < var_i9) {
              *(int *)((int)var_1c + var_i7) = 0x3ff;
            }

            if (*(int *)((int)var_1c + var_i7) < 0) {
              *(int *)((int)var_1c + var_i7) = 0;
            }

            var_i7 = var_i7 + 4;
          } while (var_i7 < 0xc);
          var_i9 = 0;

          var_b2 = (&g_LisaObjectMatrix_22)

                  [(var_1c[1] & 0x3f0) * 4 +

                   ((var_1c[2] & 0x3f0) >> 4) + (var_1c[0] & 0x3f0) * 0x100];
          *param_2 = var_b2;
          var_i7 = 0;

          do {
            var_i8 = var_i7 + 4;
            var_i1 = var_i9 + (unsigned int)var_b2 * 4;
            var_i9 = var_i9 + 1;

            *(int *)((int)var_c + var_i7) =

                 (int)(*(int *)((int)var_1c + var_i7) + (unsigned int)*(byte *)(var_i1 + param_6) * -4) / 2;
            var_i7 = var_i8;
          } while (var_i8 < 0xc);
          param_2 = param_2 + 1;
          param_1 = param_1 + 1;
          var_2c = var_2c + 1;
        } while (var_2c < param_3);
      }

      param_1 = param_1 + (param_5 - param_3);
      var_24 = var_24 + 1;
      param_2 = param_2 + (param_5 - param_3);
    } while (var_24 < param_4);
  }

  return;
}

/**
 * @original Lisa_FilterTextureBlock (IGN_WIN.EXE @ 0x004481f0, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_FilterTextureBlock(byte *param_1,int param_2,int param_3,int param_4,int param_5,int param_6, int param_7,int param_8) {
  byte var_b1;
  unsigned int var_u2;
  int var_i3;
  unsigned int *var_pu4;
  int var_i5;
  int var_54;
  int var_4c;
  int var_48;
  byte *var_44;
  int var_40;
  byte *var_3c;
  int var_38;
  byte *var_34;
  int var_30;
  byte *var_2c;
  int var_28;
  unsigned int var_24 [6];
  int var_c [3];

  

  var_4c = 0;

  if (0 < param_4) {
    var_40 = param_8 * param_5;
    var_3c = param_1 + var_40;
    var_44 = param_1 + var_40 + param_3 + -1;
    var_38 = (param_5 + -1) * param_8;
    var_34 = param_1 + (param_3 - var_40) + -1;
    var_30 = -var_40;
    var_2c = param_1 + -var_40;
    var_28 = (-1 - param_5) * param_8;

    do {
      var_54 = 0;
      var_c[0] = 0;
      var_c[1] = 0;
      var_c[2] = 0;

      if (0 < param_3) {
        do {
          var_i3 = 0;
          var_pu4 = var_24;

          do {
            if (var_4c - param_8 < 1) {
              if (var_54 - param_8 < 1) {
                var_b1 = *param_1;
              }

              else {
                var_b1 = param_1[var_54 - param_8];
              }

              *var_pu4 = (unsigned int)*(byte *)(var_i3 + (unsigned int)var_b1 * 4 + param_6);

              if (var_54 + param_8 < param_3) {
                var_u2 = (unsigned int)param_1[var_54 + param_8];
              }

              else {
                var_u2 = (unsigned int)param_1[param_3 + -1];
              }

            }

            else {
              if (var_54 == param_8 || var_54 - param_8 < 0) {
                *var_pu4 = (unsigned int)*(byte *)(var_i3 + (unsigned int)*var_2c * 4 + param_6);
              }

              else {
                *var_pu4 = (unsigned int)*(byte *)(var_i3 + (unsigned int)param_1[var_28 + var_54] * 4 + param_6);
              }

              if (param_8 + var_54 < param_3) {
                var_u2 = (unsigned int)param_1[var_30 + param_8 + var_54];
              }

              else {
                var_u2 = (unsigned int)*var_34;
              }

            }

            *var_pu4 = *var_pu4 + (unsigned int)*(byte *)(var_i3 + var_u2 * 4 + param_6);

            if (var_4c + param_8 < param_4) {
              if (var_54 == param_8 || var_54 - param_8 < 0) {
                var_b1 = *var_3c;
              }

              else {
                var_b1 = param_1[var_38 + var_54];
              }

              *var_pu4 = *var_pu4 + (unsigned int)*(byte *)(var_i3 + (unsigned int)var_b1 * 4 + param_6);

              if (param_8 + var_54 < param_3) {
                var_u2 = (unsigned int)param_1[var_40 + param_8 + var_54];
                goto LAB_004484c0;
              }

              *var_pu4 = *var_pu4 + (unsigned int)*(byte *)(var_i3 + (unsigned int)*var_44 * 4 + param_6);
            }

            else {
              if (var_54 == param_8 || var_54 - param_8 < 0) {
                var_i5 = (param_4 + -1) * param_5;
                var_b1 = *(byte *)(var_i3 + (unsigned int)param_1[var_i5] * 4 + param_6);
              }

              else {
                var_i5 = (param_4 + -1) * param_5;

                var_b1 = *(byte *)(var_i3 + (unsigned int)param_1[var_i5 + (var_54 - param_8)] * 4 + param_6)

                ;
              }

              *var_pu4 = *var_pu4 + (unsigned int)var_b1;

              if (param_8 + var_54 < param_3) {
                var_u2 = (unsigned int)param_1[param_8 + var_54];
              }

              else {
                var_u2 = (unsigned int)param_1[var_i5 + param_3 + -1];
              }

LAB_004484c0:

              *var_pu4 = *var_pu4 + (unsigned int)*(byte *)(var_i3 + var_u2 * 4 + param_6);
            }

            var_pu4 = var_pu4 + 1;
            var_i3 = var_i3 + 1;
          } while (var_pu4 < var_24 + 3);
          var_i3 = 0;

          do {
            var_i5 = *(int *)((int)var_c + var_i3) + 8 + *(int *)((int)var_24 + var_i3);
            *(int *)((int)var_24 + var_i3) = var_i5;

            if (0x3ff < var_i5) {
              *(int *)((int)var_24 + var_i3) = 0x3ff;
            }

            if (*(int *)((int)var_24 + var_i3) < 0) {
              *(int *)((int)var_24 + var_i3) = 0;
            }

            var_i3 = var_i3 + 4;
          } while (var_i3 < 0xc);
          var_48 = 0;

          var_b1 = (&g_LisaObjectMatrix_22)

                  [(var_24[0] & 0x3f0) * 0x100 +

                   ((var_24[2] & 0x3f0) >> 4) + (var_24[1] & 0x3f0) * 4];
          *(byte *)(param_2 + var_54) = var_b1;
          var_i3 = 0;

          do {
            var_i5 = var_i3 + 4;

            *(int *)((int)var_c + var_i3) =

                 (int)(*(int *)((int)var_24 + var_i3) +

                      (unsigned int)*(byte *)(var_48 + param_6 + (unsigned int)var_b1 * 4) * -4) / 2;
            var_48 = var_48 + 1;
            var_i3 = var_i5;
          } while (var_i5 < 0xc);
          var_54 = var_54 + 1;
        } while (var_54 < param_3);
      }

      param_2 = param_2 + param_5;
      var_44 = var_44 + param_5;
      var_40 = var_40 + param_5;
      var_3c = var_3c + param_5;
      var_38 = var_38 + param_5;
      var_34 = var_34 + param_5;
      var_30 = var_30 + param_5;
      var_2c = var_2c + param_5;
      var_28 = var_28 + param_5;
      var_4c = var_4c + 1;
    } while (var_4c < param_4);
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
  unsigned int var_u3;
  int var_i4;
  FILE *pFVar5;
  unsigned int var_u6;
  int var_i7;
  unsigned int var_u8;
  int var_i9;
  size_t sVar10;
  char *var_pc11;
  int var_i12;
  unsigned int *var_pu13;
  unsigned int *var_pu14;
  char *var_pc15;
  char *var_pc16;
  int var_i17;
  unsigned int var_48;
  int var_38;
  unsigned int var_34 [5];
  char var_20 [32];

  

  var_u8 = 0;
  var_i4 = 0;

  do {
    pbVar1 = (byte *)(param_2 + var_i4);
    var_i4 = var_i4 + 1;
    var_i9 = var_u8 + *pbVar1;
    var_u6 = var_i9 * 2;
    var_u3 = (unsigned int)(var_i9 < 0);
    var_u8 = var_u6 | var_u3;
  } while (var_i4 < 0x300);
  sVar10 = 0;
  var_34[0] = 0xffffffff;
  var_20[0] = '\0';
  var_pc11 = &s_pal_chk_str2;

  do {
    var_pc16 = var_pc11;
    if (var_34[0] == 0) break;
    var_34[0] = var_34[0] - 1;
    var_pc16 = var_pc11 + 1;
    cVar2 = *var_pc11;
    var_pc11 = var_pc16;
  } while (cVar2 != '\0');
  var_34[0] = ~var_34[0];
  var_i4 = -1;
  var_pc11 = var_20;

  do {
    var_pc15 = var_pc11;
    if (var_i4 == 0) break;
    var_i4 = var_i4 + -1;
    var_pc15 = var_pc11 + 1;
    cVar2 = *var_pc11;
    var_pc11 = var_pc15;
  } while (cVar2 != '\0');
  var_pc11 = var_pc16 + -var_34[0];
  var_pc16 = var_pc15 + -1;

  for (var_u8 = var_34[0] >> 2; var_u8 != 0; var_u8 = var_u8 - 1) {
    *(int *)var_pc16 = *(int *)var_pc11;
    var_pc11 = var_pc11 + 4;
    var_pc16 = var_pc16 + 4;
  }

  for (var_u8 = var_34[0] & 3; var_u8 != 0; var_u8 = var_u8 - 1) {
    *var_pc16 = *var_pc11;
    var_pc11 = var_pc11 + 1;
    var_pc16 = var_pc16 + 1;
  }

  __ultoa(var_u6 & 0xffff | var_u3,(char *)var_34,0x10);
  var_u8 = 0xffffffff;
  var_pu13 = var_34;

  do {
    var_pu14 = var_pu13;
    if (var_u8 == 0) break;
    var_u8 = var_u8 - 1;
    var_pu14 = (unsigned int *)((int)var_pu13 + 1);
    var_u6 = *var_pu13;
    var_pu13 = var_pu14;
  } while ((char)var_u6 != '\0');
  var_u8 = ~var_u8;
  var_i4 = -1;
  var_pc11 = var_20;

  do {
    var_pc16 = var_pc11;
    if (var_i4 == 0) break;
    var_i4 = var_i4 + -1;
    var_pc16 = var_pc11 + 1;
    cVar2 = *var_pc11;
    var_pc11 = var_pc16;
  } while (cVar2 != '\0');
  var_pc11 = (char *)((int)var_pu14 - var_u8);
  var_pc16 = var_pc16 + -1;

  for (var_u6 = var_u8 >> 2; var_u6 != 0; var_u6 = var_u6 - 1) {
    *(int *)var_pc16 = *(int *)var_pc11;
    var_pc11 = var_pc11 + 4;
    var_pc16 = var_pc16 + 4;
  }

  for (var_u8 = var_u8 & 3; var_u8 != 0; var_u8 = var_u8 - 1) {
    *var_pc16 = *var_pc11;
    var_pc11 = var_pc11 + 1;
    var_pc16 = var_pc16 + 1;
  }

  var_u8 = 0xffffffff;
  var_pc11 = (char *)&s_pal_chk_str1;

  do {
    var_pc16 = var_pc11;
    if (var_u8 == 0) break;
    var_u8 = var_u8 - 1;
    var_pc16 = var_pc11 + 1;
    cVar2 = *var_pc11;
    var_pc11 = var_pc16;
  } while (cVar2 != '\0');
  var_u8 = ~var_u8;
  var_i4 = -1;
  var_pc11 = var_20;

  do {
    var_pc15 = var_pc11;
    if (var_i4 == 0) break;
    var_i4 = var_i4 + -1;
    var_pc15 = var_pc11 + 1;
    cVar2 = *var_pc11;
    var_pc11 = var_pc15;
  } while (cVar2 != '\0');
  var_pc11 = var_pc16 + -var_u8;
  var_pc16 = var_pc15 + -1;

  for (var_u6 = var_u8 >> 2; var_u6 != 0; var_u6 = var_u6 - 1) {
    *(int *)var_pc16 = *(int *)var_pc11;
    var_pc11 = var_pc11 + 4;
    var_pc16 = var_pc16 + 4;
  }

  for (var_u8 = var_u8 & 3; var_u8 != 0; var_u8 = var_u8 - 1) {
    *var_pc16 = *var_pc11;
    var_pc11 = var_pc11 + 1;
    var_pc16 = var_pc16 + 1;
  }

  var_u8 = 0xffffffff;
  var_pc11 = (char *)&s_pal_checksum_fmt;

  do {
    var_pc16 = var_pc11;
    if (var_u8 == 0) break;
    var_u8 = var_u8 - 1;
    var_pc16 = var_pc11 + 1;
    cVar2 = *var_pc11;
    var_pc11 = var_pc16;
  } while (cVar2 != '\0');
  var_u8 = ~var_u8;
  var_i4 = -1;
  var_pc11 = var_20;

  do {
    var_pc15 = var_pc11;
    if (var_i4 == 0) break;
    var_i4 = var_i4 + -1;
    var_pc15 = var_pc11 + 1;
    cVar2 = *var_pc11;
    var_pc11 = var_pc15;
  } while (cVar2 != '\0');
  var_pc11 = var_pc16 + -var_u8;
  var_pc16 = var_pc15 + -1;

  for (var_u6 = var_u8 >> 2; var_u6 != 0; var_u6 = var_u6 - 1) {
    *(int *)var_pc16 = *(int *)var_pc11;
    var_pc11 = var_pc11 + 4;
    var_pc16 = var_pc16 + 4;
  }

  for (var_u8 = var_u8 & 3; var_u8 != 0; var_u8 = var_u8 - 1) {
    *var_pc16 = *var_pc11;
    var_pc11 = var_pc11 + 1;
    var_pc16 = var_pc16 + 1;
  }

  pFVar5 = (FILE *)fopen(var_20,(const char *)&s_rb);

  if (pFVar5 != (FILE *)0x0) {
    sVar10 = _fread(&g_LisaObjectMatrix_22,1,0x40000,pFVar5);
    _fclose(pFVar5);
  }

  if (sVar10 != 0x40000) {
    var_48 = 0;

    do {
      var_i7 = 0;
      var_i4 = 0x7f000000;
      var_i9 = 0;

      do {
        var_i12 = ((var_48 & 0x3f000) >> 10) - (unsigned int)*(byte *)(var_i7 + param_2);
        var_i12 = var_i12 * var_i12;

        if (((var_i12 < var_i4) &&

            (var_i17 = ((var_48 & 0xfc0) >> 4) - (unsigned int)*(byte *)(var_i7 + 1 + param_2),

            var_i12 = var_i12 + var_i17 * var_i17, var_i12 < var_i4)) &&

           (var_i17 = (var_48 & 0x3f) * 4 - (unsigned int)*(byte *)(var_i7 + 2 + param_2),

           var_i12 = var_i12 + var_i17 * var_i17, var_i12 < var_i4)) {
          var_i4 = var_i12;
          var_38 = var_i9;
        }

        var_i7 = var_i7 + 3;
        var_i9 = var_i9 + 1;
      } while (var_i9 < 0x100);
      var_u8 = var_48 + 1;
      ((int*)&(g_LisaObjectMatrix_22))[var_48] = (char)var_38;
      var_48 = var_u8;
    } while ((int)var_u8 < 0x40000);
    pFVar5 = (FILE *)fopen(var_20,(const char *)&s_tab_tab);

    if (pFVar5 != (FILE *)0x0) {
      _fwrite(&g_LisaObjectMatrix_22,1,0x40000,pFVar5);
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
  unsigned int var_u1;
  int var_i2;
  int var_i3;
  unsigned int var_c;
  int var_8;
  unsigned int var_4;

  

  var_8 = 0x7f000000;
  var_i2 = 0;
  var_4 = 0;
  var_u1 = var_c;

  do {
    var_i3 = *param_1 - (unsigned int)*(byte *)(param_2 + var_i2);
    var_i3 = var_i3 * var_i3;

    if (var_i3 < var_8) {
      var_c = (unsigned int)*(byte *)(param_2 + 1 + var_i2);
      var_i3 = var_i3 + (param_1[1] - var_c) * (param_1[1] - var_c);

      if (var_i3 < var_8) {
        var_c = (unsigned int)*(byte *)(param_2 + 2 + var_i2);
        var_i3 = var_i3 + (param_1[2] - var_c) * (param_1[2] - var_c);

        if (var_i3 < var_8) {
          var_u1 = var_4;
          var_8 = var_i3;
        }

      }

    }

    var_i2 = var_i2 + 3;
    var_4 = var_4 + 1;
  } while ((int)var_4 < 0x100);
  return var_u1;
}

/**
 * @original Lisa_RenderSkyBackdrop (IGN_WIN.EXE @ 0x00448990, lisa3d.c)
 * @fidelity ADAPTED
 */
LisaReturn64 Lisa_RenderSkyBackdrop(void) {
  float var_f1;
  int var_i2;
  int var_i3;
  int var_i4;
  int var_i5;
  unsigned int var_u6;
  double out_ST0;
  double out_ST1;
  int var_l7;
  unsigned int var_u8;

  

  var_i2 = g_LisaCamera;
  var_l7 = __ftol();
  var_i3 = (int)var_l7;
  var_l7 = __ftol();
  var_i4 = (int)var_l7;
  var_l7 = __ftol();
  var_u6 = (unsigned int)((unsigned int)var_l7 >> 0);
  var_i5 = (int)var_l7;
  var_f1 = (float)SQRT(out_ST1 * out_ST1 + out_ST0 * out_ST0);

  if ((var_i4 < 1) || ((int)var_f1 < 1)) {
    if ((var_i4 < 0) && (0 < (int)var_f1)) {
      fpatan((double)var_f1 / (double)-var_i4,(double)1);
      var_l7 = __ftol();
      *(double *)(var_i2 + 0x18) = (double)(int)var_l7;
    }

    else if (var_i4 == 0) {
      *(int *)(var_i2 + 0x18) = 0;
      *(int *)(var_i2 + 0x1c) = 0;
      var_l7 = (unsigned int)var_u6 << 0;
    }

    else if ((var_i4 == 0) && (ABS(var_f1) == 0.0)) {
      *(int *)(var_i2 + 0x18) = 0;
      *(int *)(var_i2 + 0x1c) = 0;
      var_l7 = (unsigned int)var_u6 << 0;
    }

    else if ((var_i4 < 0) && (ABS(var_f1) == 0.0)) {
      *(int *)(var_i2 + 0x18) = 0;
      *(int *)(var_i2 + 0x1c) = 0x40a51800;
      var_l7 = (((int)(var_u6) << 0) | ((unsigned int)(var_f1)));
    }

    else {
      var_l7 = (unsigned int)var_u6 << 0;

      if (0 < var_i4) {
        var_l7 = (((int)(var_u6) << 0) | ((unsigned int)(var_f1)));

        if (ABS(var_f1) == 0.0) {
          *(int *)(var_i2 + 0x18) = 0;
          *(int *)(var_i2 + 0x1c) = 0x408c2000;
          var_l7 = (((int)(var_u6) << 0) | ((unsigned int)(var_f1)));
        }

      }

    }

  }

  else {
    fpatan((double)var_i4 / (double)var_f1,(double)1);
    var_l7 = __ftol();
    *(double *)(var_i2 + 0x18) = (double)(int)var_l7;
  }

  var_u6 = (unsigned int)((unsigned int)var_l7 >> 0);

  if ((var_i3 < 1) || (var_i5 < 1)) {
    if ((var_i5 < 0) && (0 < var_i3)) {
      fpatan((double)-var_i5 / (double)var_i3,(double)1);
      var_u8 = __ftol();
      *(double *)(var_i2 + 0x20) = (double)(int)var_u8;
    }

    else if ((var_i3 < 0) && (var_i5 < 0)) {
      fpatan((double)var_i3 / (double)var_i5,(double)1);
      var_u8 = __ftol();
      *(double *)(var_i2 + 0x20) = (double)(int)var_u8;
    }

    else if ((var_i5 < 1) || (-1 < var_i3)) {
      if (var_i3 == 0) {
        if (0 < var_i5) {
          *(int *)(var_i2 + 0x20) = 0;
          *(int *)(var_i2 + 0x24) = 0;
          var_u8 = (unsigned int)var_u6 << 0;
          goto LAB_00448c11;
        }

        if (var_i5 < 0) {
          *(int *)(var_i2 + 0x20) = 0;
          *(int *)(var_i2 + 0x24) = 0x409c2000;
          var_u8 = (unsigned int)var_u6 << 0;
          goto LAB_00448c11;
        }

      }

      if ((var_i5 == 0) && (0 < var_i3)) {
        *(int *)(var_i2 + 0x20) = 0;
        *(int *)(var_i2 + 0x24) = 0x408c2000;
        var_u8 = (unsigned int)var_u6 << 0;
      }

      else {
        var_u8 = (unsigned int)var_u6 << 0;

        if ((var_i5 == 0) && (var_u8 = (unsigned int)var_u6 << 0, var_i3 < 0)) {
          *(int *)(var_i2 + 0x20) = 0;
          *(int *)(var_i2 + 0x24) = 0x40a51800;
          var_u8 = (unsigned int)var_u6 << 0;
        }

      }

    }

    else {
      fpatan((double)var_i5 / (double)-var_i3,(double)1);
      var_u8 = __ftol();
      *(double *)(var_i2 + 0x20) = (double)(int)var_u8;
    }

  }

  else {
    fpatan((double)var_i3 / (double)var_i5,(double)1);
    var_u8 = __ftol();
    *(double *)(var_i2 + 0x20) = (double)(int)var_u8;
  }

LAB_00448c11:

  *(double *)(var_i2 + 0x28) = (double)*(int *)(var_i2 + 0x7c);
  return var_u8 & 0xffffffff00000000;
}

/**
 * @original Lisa_CullObjectsOrthographic (IGN_WIN.EXE @ 0x00448c30, lisa3d.c)
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
    int param_idx;
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
    param_idx = 3;

    // Final grid pointer offset
    grid_index = (int)g_pLisaGridCells +
            (cell_index + ((int)((int)cam_z_int + ((int)cam_z_int >> 31 & 0xffU)) >> 8) +
             *(int *)((char *)&g_LisaDefaultScale_X + array_index) + g_LisaDefaultOffset_Y) * 4;
             
    grid_offset_z = g_LisaDefaultOffset_Z;

    if (g_pLisaAllocatedBuffersEnd == 0) {
        while (grid_offset_z != -5000) {
            grid_offset_z = *(int *)(param_idx * 4 + 0x499fa0);

            if (0 < grid_offset_z) {
                do {
                    obj_ptr = *(LisaDynamicObject **)(grid_index + 4);
                    grid_index += 4;

                    if ((obj_ptr != NULL) && (obj_ptr->self_ptr == obj_ptr)) {
                        obj_count = g_LisaCamera->visible_obj_count + 1;
                        g_LisaCamera->visible_obj_count = obj_count;
                        visible_array[obj_count - 1] = obj_ptr;

                        if (obj_ptr->next_in_cell != NULL) {
                            LisaDynamicObject **out_ptr = &visible_array[obj_count];

                            do {
                                obj_ptr = obj_ptr->next_in_cell;

                                if (obj_ptr->self_ptr == obj_ptr) {
                                    *out_ptr = obj_ptr;
                                    out_ptr++;
                                    g_LisaCamera->visible_obj_count++;
                                }

                            } while (obj_ptr->next_in_cell != NULL);
                        }
                    }
                    grid_offset_z--;
                } while (grid_offset_z != 0);
            }

            grid_offset_z = param_idx + 1;
            param_idx += 2;
            grid_index += (*(int *)(grid_offset_z * 4 + 0x499fa0) + g_LisaGridCellsX) * 4;
            grid_offset_z = *(int *)(param_idx * 4 + 0x499fa0);
        }
    } else {
        while (grid_offset_z != -5000) {
            count = *(int *)(param_idx * 4 + 0x499fa0);

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
                            LisaDynamicObject **out_ptr = &visible_array[g_LisaCamera->visible_obj_count];

                            do {
                                obj_ptr = obj_ptr->next_in_cell;

                                if ((obj_ptr->self_ptr == obj_ptr) &&
                                   ((obj_ptr->unknown_24 == 0 || obj_ptr->unknown_24 == g_pLisaAllocatedBuffersEnd))) {
                                    *out_ptr = obj_ptr;
                                    out_ptr++;
                                    g_LisaCamera->visible_obj_count++;
                                }

                            } while (obj_ptr->next_in_cell != NULL);
                        }
                    }
                    count--;
                } while (count != 0);
            }

            grid_offset_z = param_idx + 1;
            param_idx += 2;
            grid_index += (*(int *)(grid_offset_z * 4 + 0x499fa0) + g_LisaGridCellsX) * 4;
            grid_offset_z = *(int *)(param_idx * 4 + 0x499fa0);
        }
    }

    g_LisaCamera->submesh_count = g_LisaCamera->visible_obj_count;
    return 0;
}

/**
 * @original Lisa_FrustumCullObjects (IGN_WIN.EXE @ 0x00448e70, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_FrustumCullObjects(void) {
    LisaCamera *camera;
    LisaDynamicObject *obj_ptr;
    LisaDynamicObject **visible_array;
    LisaDynamicObject **out_ptr;
    
    int obj_count;
    int next_obj_count;
    int grid_offset_z;
    int cell_index;
    int grid_index;
    int param_idx;
    int count;
    int layer_id;
    int grid_stride;
    int submesh_count;
    
    int cam_val1, cam_val2, cam_val3;
    double out_ST1;

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
        param_idx = 3;

        grid_index = (int)g_pLisaGridCells +
                (cell_index + ((int)((int)cam_val1 + ((int)cam_val1 >> 31 & 0xffU)) >> 8) +
                 *(int *)((char *)&g_LisaDefaultScale_X + grid_stride) + g_LisaDefaultOffset_Y) * 4;
                 
        grid_offset_z = g_LisaDefaultOffset_Z;

        if (g_pLisaAllocatedBuffersEnd == 0) {
            while (grid_offset_z != -5000) {
                grid_offset_z = *(int *)(param_idx * 4 + 0x499fa0);

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
                                out_ptr = visible_array + next_obj_count;
                                do {
                                    obj_ptr = obj_ptr->next_in_cell;
                                    if (obj_ptr->self_ptr == obj_ptr) {
                                        *out_ptr = obj_ptr;
                                        out_ptr++;
                                        camera->visible_obj_count++;
                                    }
                                } while (obj_ptr->next_in_cell != NULL);
                            }
                        }
                        grid_offset_z--;
                    } while (grid_offset_z != 0);
                }

                grid_offset_z = param_idx + 1;
                param_idx += 2;
                grid_index += (*(int *)(grid_offset_z * 4 + 0x499fa0) + g_LisaGridCellsX) * 4;
                grid_offset_z = *(int *)(param_idx * 4 + 0x499fa0);
            }
        } else {
            while (grid_offset_z != -5000) {
                count = *(int *)(param_idx * 4 + 0x499fa0);

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
                                out_ptr = visible_array + camera->visible_obj_count;
                                do {
                                    obj_ptr = obj_ptr->next_in_cell;
                                    if ((obj_ptr->self_ptr == obj_ptr) &&
                                       ((obj_ptr->unknown_24 == 0 || obj_ptr->unknown_24 == layer_id))) {
                                        *out_ptr = obj_ptr;
                                        out_ptr++;
                                        camera->visible_obj_count++;
                                    }
                                } while (obj_ptr->next_in_cell != NULL);
                            }
                        }
                        count--;
                    } while (count != 0);
                }

                grid_offset_z = param_idx + 1;
                param_idx += 2;
                grid_index += (*(int *)(grid_offset_z * 4 + 0x499fa0) + g_LisaGridCellsX) * 4;
                grid_offset_z = *(int *)(param_idx * 4 + 0x499fa0);
            }
        }
    } else {
        // Projection calculation
        fcos(camera->rot_y * 0.017453292519943295); // g_Const_TenthDegToRad approx
        grid_stride = camera->projection_type / 2;
        cam_val1 = __ftol();
        cam_val2 = __ftol();
        fsin(out_ST1);

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
                param_idx = camera->projection_type;
                cell_index = g_LisaGridCellsX - param_idx;

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
                                    out_ptr = visible_array + next_obj_count;
                                    do {
                                        obj_ptr = obj_ptr->next_in_cell;
                                        if (obj_ptr->self_ptr == obj_ptr) {
                                            *out_ptr = obj_ptr;
                                            out_ptr++;
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
                    param_idx--;
                } while (param_idx != 0);
            }
        } else if (0 < camera->projection_type) {
            count = camera->projection_type;
            grid_stride = g_LisaGridCellsX - count;

            do {
                param_idx = camera->projection_type;
                if (0 < param_idx) {
                    do {
                        obj_ptr = *grid_cells;
                        if ((obj_ptr != NULL) && (obj_ptr->self_ptr == obj_ptr)) {
                            if ((obj_ptr->unknown_24 == 0) || (obj_ptr->unknown_24 == cell_index)) {
                                obj_count = camera->visible_obj_count;
                                camera->visible_obj_count = obj_count + 1;
                                visible_array[obj_count] = obj_ptr;
                            }

                            if (obj_ptr->next_in_cell != NULL) {
                                out_ptr = visible_array + camera->visible_obj_count;
                                do {
                                    obj_ptr = obj_ptr->next_in_cell;
                                    if ((obj_ptr->self_ptr == obj_ptr) &&
                                       ((obj_ptr->unknown_24 == 0 || obj_ptr->unknown_24 == cell_index))) {
                                        *out_ptr = obj_ptr;
                                        out_ptr++;
                                        camera->visible_obj_count++;
                                    }
                                } while (obj_ptr->next_in_cell != NULL);
                            }
                        }
                        grid_cells++;
                        param_idx--;
                    } while (param_idx != 0);
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
            
            // Matrix multiply: rot_matrix[6, 7, 8] are var_4, var_8, var_c
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
 * @original Lisa_CullObjects (IGN_WIN.EXE @ 0x00449470, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_CullObjects(void) {
  float var_f1;
  float var_f2;
  float var_f3;
  float var_f4;
  float var_f5;
  float var_f6;
  float var_f7;
  float var_f8;
  int var_i9;
  int *var_pi10;
  int *var_pi11;
  unsigned int var_u12;
  unsigned int var_u13;
  int *var_pi14;
  int *var_pi15;
  float *pfVar16;
  float *pfVar17;
  int var_i18;
  int var_i19;
  int *var_pi20;
  int var_i21;
  int var_i22;
  int *var_pu23;
  double var_f24;
  double var_f25;
  double var_f26;
  double var_f27;
  double var_f28;
  double var_f29;
  double out_ST1;
  int var_l30;
  int var_l31;
  int var_l32;
  int var_70;
  int *var_6c;
  int var_68;
  int *var_64;
  double *var_58;
  float var_50 [19];
  float var_4;

  

  var_i22 = g_LisaCamera;
  var_i19 = g_LisaCamera->projection_type;

  if (var_i19 == 0) {
    g_LisaCamera->visible_obj_count = 0;
    var_pi15 = (int *)(var_i22 + 0x60);
    var_58 = &g_LisaCamera->rot_y;
    var_l30 = __ftol();
    var_l31 = __ftol();
    var_i19 = ((int)var_l30 * 0x24 + (int)var_l31) * 8;
    var_l30 = __ftol();

    var_i9 = (*(int *)(&g_LisaDefaultScale_Y + var_i19) +

             ((int)((int)var_l30 + ((int)var_l30 >> 0x1f & 0xffU)) >> 8) + g_LisaDefaultOffset_X) *

            g_LisaGridCellsX;
    var_l30 = __ftol();
    var_i18 = g_pLisaAllocatedBuffersEnd;
    var_pi20 = g_LisaVisibleObjects;
    var_70 = 3;

    var_i19 = (int)g_pLisaGridCells +

             (var_i9 + ((int)((int)var_l30 + ((int)var_l30 >> 0x1f & 0xffU)) >> 8) +

              *(int *)(&g_LisaDefaultScale_X + var_i19) + g_LisaDefaultOffset_Y) * 4;
    var_i9 = g_LisaDefaultOffset_Z;

    if (g_pLisaAllocatedBuffersEnd == 0) {
      while (var_i9 != -5000) {
        var_i9 = *(int *)(var_70 * 4 + 0x499fa0);

        if (0 < var_i9) {
          do {
            var_pi14 = *(int **)(var_i19 + 4);
            var_i19 = var_i19 + 4;

            if ((var_pi14 != (int *)0x0) && ((int *)*var_pi14 == var_pi14)) {
              var_i18 = *var_pi15;
              var_pi20[var_i18] = (int)var_pi14;
              var_i18 = var_i18 + 1;
              *var_pi15 = var_i18;

              if (*(int *)((int)var_pi14 + 0x26) != 0) {
                var_pi11 = var_pi20 + var_i18;

                do {
                  var_pi14 = *(int **)((int)var_pi14 + 0x26);

                  if ((int *)*var_pi14 == var_pi14) {
                    *var_pi11 = (int)var_pi14;
                    var_pi11 = var_pi11 + 1;
                    *var_pi15 = *var_pi15 + 1;
                  }

                } while (*(int *)((int)var_pi14 + 0x26) != 0);
              }

            }

            var_i9 = var_i9 + -1;
          } while (var_i9 != 0);
        }

        var_i9 = var_70 + 1;
        var_70 = var_70 + 2;
        var_i19 = var_i19 + (*(int *)(var_i9 * 4 + 0x499fa0) + g_LisaGridCellsX) * 4;
        var_i9 = *(int *)(var_70 * 4 + 0x499fa0);
      }

    }

    else {
      while (var_i9 != -5000) {
        var_50[0] = *(float *)(var_70 * 4 + 0x499fa0);

        if (0 < (int)var_50[0]) {
          do {
            var_pi14 = *(int **)(var_i19 + 4);
            var_i19 = var_i19 + 4;

            if ((var_pi14 != (int *)0x0) && ((int *)*var_pi14 == var_pi14)) {
              if (((short)var_pi14[9] == 0) || ((short)var_pi14[9] == var_i18)) {
                var_i9 = *var_pi15;
                var_pi20[var_i9] = (int)var_pi14;
                *var_pi15 = var_i9 + 1;
              }

              if (*(int *)((int)var_pi14 + 0x26) != 0) {
                var_pi11 = var_pi20 + *var_pi15;

                do {
                  var_pi14 = *(int **)((int)var_pi14 + 0x26);

                  if (((int *)*var_pi14 == var_pi14) &&

                     (((short)var_pi14[9] == 0 || ((short)var_pi14[9] == var_i18)))) {
                    *var_pi11 = (int)var_pi14;
                    var_pi11 = var_pi11 + 1;
                    *var_pi15 = *var_pi15 + 1;
                  }

                } while (*(int *)((int)var_pi14 + 0x26) != 0);
              }

            }

            var_50[0] = (float)((int)var_50[0] + -1);
          } while (var_50[0] != 0.0);
        }

        var_i9 = var_70 + 1;
        var_70 = var_70 + 2;
        var_i19 = var_i19 + (*(int *)(var_i9 * 4 + 0x499fa0) + g_LisaGridCellsX) * 4;
        var_i9 = *(int *)(var_70 * 4 + 0x499fa0);
      }

    }

  }

  else {
    g_LisaCamera->visible_obj_count = 0;
    var_pi14 = (int *)(var_i22 + 0x60);
    var_58 = &g_LisaCamera->rot_y;
    var_50[0] = (float)(var_i19 / 3);
    fcos((double)*var_58 * (double)g_Const_TenthDegToRad);
    var_l30 = __ftol();
    var_l31 = __ftol();
    fsin(out_ST1);

    var_i18 = (((int)var_l30 + ((int)((int)var_l31 + ((int)var_l31 >> 0x1f & 0xffU)) >> 8)) - var_i19 / 2

             ) * g_LisaGridCellsX;
    var_l30 = __ftol();
    var_l31 = __ftol();
    var_i9 = g_pLisaAllocatedBuffersEnd;
    var_pi20 = g_LisaVisibleObjects;

    var_pi15 = (int *)((int)g_pLisaGridCells +

                     ((var_i18 + (int)var_l30 +

                      ((int)((int)var_l31 + ((int)var_l31 >> 0x1f & 0xffU)) >> 8)) - var_i19 / 2) * 4);
    var_68 = var_i19;

    if (g_pLisaAllocatedBuffersEnd == 0) {
      if (0 < var_i19) {
        var_i9 = g_LisaGridCellsX - var_i19;

        do {
          var_i18 = var_i19;

          if (0 < var_i19) {
            do {
              var_pi11 = (int *)*var_pi15;

              if ((var_pi11 != (int *)0x0) && ((int *)*var_pi11 == var_pi11)) {
                var_i21 = *var_pi14;
                var_pi20[var_i21] = (int)var_pi11;
                var_i21 = var_i21 + 1;
                *var_pi14 = var_i21;

                if (*(int *)((int)var_pi11 + 0x26) != 0) {
                  var_pi10 = var_pi20 + var_i21;

                  do {
                    var_pi11 = *(int **)((int)var_pi11 + 0x26);

                    if ((int *)*var_pi11 == var_pi11) {
                      *var_pi10 = (int)var_pi11;
                      var_pi10 = var_pi10 + 1;
                      *var_pi14 = *var_pi14 + 1;
                    }

                  } while (*(int *)((int)var_pi11 + 0x26) != 0);
                }

              }

              var_pi15 = var_pi15 + 1;
              var_i18 = var_i18 + -1;
            } while (var_i18 != 0);
          }

          var_pi15 = var_pi15 + var_i9;
          var_68 = var_68 + -1;
        } while (var_68 != 0);
      }

    }

    else if (0 < var_i19) {
      var_i18 = g_LisaGridCellsX - var_i19;

      do {
        var_64 = (int *)var_i19;

        if (0 < var_i19) {
          do {
            var_pi11 = (int *)*var_pi15;

            if ((var_pi11 != (int *)0x0) && ((int *)*var_pi11 == var_pi11)) {
              if (((short)var_pi11[9] == 0) || ((short)var_pi11[9] == var_i9)) {
                var_i21 = *var_pi14;
                var_pi20[var_i21] = (int)var_pi11;
                *var_pi14 = var_i21 + 1;
              }

              if (*(int *)((int)var_pi11 + 0x26) != 0) {
                var_pi10 = var_pi20 + *var_pi14;

                do {
                  var_pi11 = *(int **)((int)var_pi11 + 0x26);

                  if (((int *)*var_pi11 == var_pi11) &&

                     (((short)var_pi11[9] == 0 || ((short)var_pi11[9] == var_i9)))) {
                    *var_pi10 = (int)var_pi11;
                    var_pi10 = var_pi10 + 1;
                    *var_pi14 = *var_pi14 + 1;
                  }

                } while (*(int *)((int)var_pi11 + 0x26) != 0);
              }

            }

            var_pi15 = var_pi15 + 1;
            var_64 = (int *)((int)var_64 + -1);
          } while (var_64 != (int *)0x0);
        }

        var_pi15 = var_pi15 + var_i18;
        var_68 = var_68 + -1;
      } while (var_68 != 0);
    }

  }

  var_6c = (int *)(var_i22 + 0x60);
  var_i19 = g_LisaCamera->fov_x;
  var_i22 = g_SubpixelMinX + var_i19 * -0x100;

  if ((((g_LisaActiveMaterial != var_i22) || (g_SubpixelMaxX + var_i19 * -0x100 != g_LisaSubmeshLodLevel)) ||

      (g_SubpixelMinY + g_LisaCamera->fov_y * -0x100 != g_LisaActiveSubmeshFlags)) ||

     (((g_SubpixelMaxY + g_LisaCamera->fov_y * -0x100 != g_LisaCameraPitch ||

       (g_LisaCamera->viewport_x != g_LisaSubmeshClipMask)) ||

      ((g_LisaCamera->viewport_y != g_LisaCameraDistance || (g_LisaMipmapQuality == 1)))))) {
    var_u13 = 0;
    g_LisaMipmapQuality = 0;
    g_LisaSubmeshLodLevel = g_SubpixelMaxX + var_i19 * -0x100;
    var_i9 = g_LisaCamera->fov_y;
    g_LisaActiveSubmeshFlags = g_SubpixelMinY + var_i9 * -0x100;
    g_LisaCameraPitch = g_SubpixelMaxY + var_i9 * -0x100;
    g_LisaSubmeshClipMask = g_LisaCamera->viewport_x;
    g_LisaCameraDistance = g_LisaCamera->viewport_y;

    var_50[0] = ((float)((-1 - var_i19) * 0x100 + g_SubpixelMinX) * (float)g_Const_1000) /

                  ((float)g_LisaSubmeshClipMask * (float)g_Const_Neg256);

    var_50[3] = ((float)((1 - var_i19) * 0x100 + g_SubpixelMaxX) * (float)g_Const_1000) /

                  ((float)g_LisaSubmeshClipMask * (float)g_Const_Neg256);

    var_50[1] = ((float)((-1 - var_i9) * 0x100 + g_SubpixelMinY) * (float)g_Const_1000) /

                  ((float)g_LisaCameraDistance * (float)g_Const_Neg256);

    var_50[7] = ((float)((1 - var_i9) * 0x100 + g_SubpixelMaxY) * (float)g_Const_1000) /

                  ((float)g_LisaCameraDistance * (float)g_Const_Neg256);
    var_50[2] = 1000.0;
    var_50[4] = var_50[1];
    var_50[5] = 1000.0;
    var_50[6] = var_50[3];
    var_50[8] = 1000.0;
    var_50[9] = var_50[0];
    var_50[10] = var_50[7];
    var_50[0xb] = 1000.0;
    var_i19 = 0;
    g_LisaActiveMaterial = var_i22;

    while( 1 ) {
      var_u13 = var_u13 + 1;
      var_u12 = var_u13 & 3;
      var_f1 = var_50[var_u12 * 3];
      var_f2 = *(float *)((int)var_50 + var_i19 + 8);
      var_f3 = var_50[var_u12 * 3 + 2];
      var_f4 = *(float *)((int)var_50 + var_i19);
      var_f5 = var_50[var_u12 * 3 + 1];
      var_f6 = *(float *)((int)var_50 + var_i19);

      *(float *)((int)&g_LisaFrustumPlaneLeft + var_i19) =

           var_50[var_u12 * 3 + 2] * *(float *)((int)var_50 + var_i19 + 4) -

           var_50[var_u12 * 3 + 1] * *(float *)((int)var_50 + var_i19 + 8);
      var_f7 = var_50[var_u12 * 3];
      var_f8 = *(float *)((int)var_50 + var_i19 + 4);
      *(float *)((int)&g_LisaFrustumPlaneRight + var_i19) = var_f1 * var_f2 - var_f3 * var_f4;
      *(float *)((int)&g_LisaFrustumPlaneTop + var_i19) = var_f5 * var_f6 - var_f7 * var_f8;

      var_f1 = SQRT(*(float *)((int)&g_LisaFrustumPlaneLeft + var_i19) * *(float *)((int)&g_LisaFrustumPlaneLeft + var_i19)

                   + *(float *)((int)&g_LisaFrustumPlaneRight + var_i19) *

                     *(float *)((int)&g_LisaFrustumPlaneRight + var_i19) +

                     *(float *)((int)&g_LisaFrustumPlaneTop + var_i19) *

                     *(float *)((int)&g_LisaFrustumPlaneTop + var_i19));

      *(float *)((int)&g_LisaFrustumPlaneLeft + var_i19) =

           (*(float *)((int)&g_LisaFrustumPlaneLeft + var_i19) / var_f1) * g_Const_TenthDegToRadFloat;

      *(float *)((int)&g_LisaFrustumPlaneRight + var_i19) =

           (*(float *)((int)&g_LisaFrustumPlaneRight + var_i19) / var_f1) * g_Const_TenthDegToRadFloat;
      var_f1 = (*(float *)((int)&g_LisaFrustumPlaneTop + var_i19) / var_f1) * g_Const_TenthDegToRadFloat;
      if (0x2f < var_i19 + 0xc) break;
      *(float *)((int)&g_LisaFrustumPlaneTop + var_i19) = var_f1;
      var_i19 = var_i19 + 0xc;
    }

    *(float *)((int)&g_LisaFrustumPlaneTop + var_i19) = var_f1;
  }

  var_f24 = (double)g_LisaCamera->rot_x * (double)g_Const_TenthDegToRad;
  var_f25 = (double)fcos(var_f24);
  var_f26 = (double)fcos((double)*var_58 * (double)g_Const_TenthDegToRad);
  var_f24 = (double)fsin(var_f24);
  var_f27 = (double)g_LisaCamera->rot_z * (double)g_Const_TenthDegToRad;
  var_f28 = (double)fsin((double)*var_58 * (double)g_Const_TenthDegToRad);
  var_f29 = (double)fcos(var_f27);
  var_50[0] = (float)var_f29;
  var_f27 = (double)fsin(var_f27);
  g_LisaObjMat_CosRoll = (float)(var_f24 * var_f28);
  g_LisaObjMat_CosPitch = (float)((double)g_LisaObjMat_CosRoll * var_f27 + (double)var_50[0] * var_f26);
  g_LisaObjMat_SinPitch = (float)((double)var_50[0] * (double)g_LisaObjMat_CosRoll - var_f26 * var_f27);
  g_LisaObjMat_SinRoll = (float)(var_f25 * var_f27);
  g_LisaObjMat_Tmp1 = (float)((double)var_50[0] * var_f25);
  g_LisaObjMat_Tmp2 = (float)-var_f24;
  g_LisaObjMat_Tmp3 = (float)(var_f27 * var_f24 * var_f26 - (double)var_50[0] * var_f28);
  g_LisaObjMat_21 = 0;
  g_LisaObjMat_Scale = 0;
  var_pu23 = &g_LisaFrustumPlaneTop;
  g_LisaObjMat_Tmp4 = (float)((double)var_50[0] * var_f24 * var_f26 + var_f28 * var_f27);
  g_LisaObjMat_CosYaw = 0;
  g_LisaObjMat_Tmp5 = (float)(var_f25 * var_f26);
  pfVar17 = var_50;

  do {
    var_pu23 = var_pu23 + 3;
    var_l30 = __ftol();
    *pfVar17 = (float)var_l30;
    var_l30 = __ftol();
    pfVar17[1] = (float)var_l30;
    var_l30 = __ftol();
    pfVar17[2] = (float)var_l30;
    pfVar17 = pfVar17 + 4;
  } while (var_pu23 < &g_LisaFrustumNear);
  var_l30 = __ftol();
  var_l31 = __ftol();
  var_l32 = __ftol();
  var_70 = *var_6c;
  g_LisaCamera->submesh_count = var_70;
  *var_6c = 0;
  pfVar17 = var_50 + 3;

  do {
    pfVar16 = pfVar17 + 4;

    *pfVar17 = (float)-((int)pfVar17[-1] * (int)var_l32 + (int)pfVar17[-2] * (int)var_l31 +

                       (int)pfVar17[-3] * (int)var_l30);
    pfVar17 = pfVar16;
  } while (pfVar16 < &var_4);
  var_64 = var_pi20;

  if (0 < var_70) {
    do {
      var_i19 = *var_pi20;
      var_u13 = 0;
      pfVar17 = var_50 + 2;

      do {
        var_u13 = var_u13 | (int)pfVar17[-1] * *(int *)(var_i19 + 0x10) +

                          (int)pfVar17[-2] * *(int *)(var_i19 + 0xc) +

                          (int)*pfVar17 * *(int *)(var_i19 + 0x14) + (int)pfVar17[1] +

                          *(short *)(var_i19 + 0x22) * 0x40000;
        if ((int)var_u13 < 0) break;
        pfVar17 = pfVar17 + 4;
      } while (pfVar17 < var_50 + 0x12);

      if (0 < (int)var_u13) {
        *var_64 = var_i19;
        *var_6c = *var_6c + 1;
        var_64 = var_64 + 1;
      }

      var_pi20 = var_pi20 + 1;
      var_70 = var_70 + -1;
    } while (var_70 != 0);
  }

  return 0;
}

/**
 * @original Lisa_TransformVertices (IGN_WIN.EXE @ 0x00449e70, lisa3d.c)
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
      LisaDynamicObject *obj = visible_objs[i];
      MshSubmesh *mesh = obj->mesh_data;
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
}
  return;
}

/**
 * @original Lisa_TransformVerticesPanorama (IGN_WIN.EXE @ 0x0044a3d0, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_TransformVerticesPanorama(void) {
  float var_f1;
  int *var_pi2;
  float var_f3;
  float var_f4;
  int var_i5;
  int var_i6;
  int var_i7;
  int var_i8;
  int var_i9;
  int var_i10;
  int *var_pi11;
  int var_i12;
  int var_i13;
  int var_i14;
  double var_f15;
  double var_f16;
  double var_f17;
  double var_f18;
  double var_f19;
  double var_f20;
  int var_l21;
  int var_l22;
  int var_l23;
  int var_l24;
  int var_20;
  int var_1c;
  int var_10;
  int var_c;

  

  var_i6 = g_LisaCamera;
  g_LisaCameraFocalLength = g_LisaCamera->viewport_width << 2;
  g_LisaCamera->vertex_counter = 0;
  var_f15 = (double)*(double *)(var_i6 + 0x18) * (double)g_Const_NegTenthDegToRad;
  g_LisaCameraOffsetX = var_i6->fov_x << 8;
  var_f16 = (double)fcos(var_f15);
  g_LisaCameraOffsetY = var_i6->fov_y << 8;
  var_f17 = (double)*(double *)(var_i6 + 0x20) * (double)g_Const_NegTenthDegToRad;
  var_f18 = (double)fcos(var_f17);
  var_f15 = (double)fsin(var_f15);
  var_f19 = (double)*(double *)(var_i6 + 0x28) * (double)g_Const_NegTenthDegToRad;
  var_f17 = (double)fsin(var_f17);
  var_f20 = (double)fcos(var_f19);
  var_f19 = (double)fsin(var_f19);
  var_f1 = (float)var_f20;
  var_f3 = (float)-*(int *)(var_i6 + 0x80) * (float)*(double *)(var_i6 + 0x30) * (float)g_Const_256;
  var_f4 = (float)-var_i6->viewport_y * (float)g_Const_256;

  g_LisaObjMat_CosPitch =

       (float)(((double)var_f1 * var_f18 - (double)(float)(var_f15 * var_f17) * var_f19) *

              (double)var_f3);
  g_LisaObjMat_SinPitch = (float)-((double)var_f3 * var_f16 * var_f19);

  g_LisaObjMat_CosRoll =

       (float)(((double)(float)(var_f15 * var_f18) * var_f19 + (double)var_f1 * var_f17) *

              (double)var_f3);

  g_LisaObjMat_SinRoll =

       (float)(((double)var_f1 * (double)(float)(var_f15 * var_f17) + var_f18 * var_f19) *

              (double)var_f4);
  g_LisaObjMat_Tmp1 = (float)((double)var_f4 * var_f16 * (double)var_f1);
  g_LisaObjMat_Tmp4 = (float)(var_f15 * (double)g_Const_262144);
  g_LisaObjMat_Tmp3 = (float)(-(var_f16 * var_f17) * (double)g_Const_262144);

  g_LisaObjMat_Tmp2 =

       (float)((var_f17 * var_f19 - (double)var_f1 * (double)(float)(var_f15 * var_f18)) *

              (double)var_f4);
  g_LisaObjMat_Tmp5 = (float)(var_f18 * var_f16 * (double)g_Const_262144);
  var_l21 = __ftol();
  g_LisaCameraMatrix_00 = (int)var_l21;
  var_l21 = __ftol();
  g_LisaCameraMatrix_01 = (int)var_l21;
  var_l21 = __ftol();
  g_LisaCameraMatrix_02 = (int)var_l21;
  var_l21 = __ftol();
  g_LisaCameraMatrix_10 = (int)var_l21;
  var_l21 = __ftol();
  g_LisaCameraMatrix_11 = (int)var_l21;
  var_l21 = __ftol();
  g_LisaCameraMatrix_12 = (int)var_l21;
  var_l21 = __ftol();
  g_LisaCameraMatrix_20 = (int)var_l21;
  var_l21 = __ftol();
  g_LisaCameraMatrix_21 = (int)var_l21;
  var_l21 = __ftol();
  g_LisaCameraMatrix_22 = (int)var_l21;
  var_l21 = __ftol();
  var_l22 = __ftol();
  var_i6 = (int)var_l22;
  var_l22 = __ftol();
  var_l23 = __ftol();
  var_i14 = (int)var_l23;
  var_l23 = __ftol();
  var_l24 = __ftol();
  var_i7 = (int)var_l24;
  var_i5 = g_LisaCameraMatrix_00 * var_i6 + g_LisaCameraMatrix_02 * var_i7 + g_LisaCameraMatrix_01 * var_i14;
  g_LisaSubmeshCenterWorldX = (int)(var_i5 + (var_i5 >> 0x1f & 0xfffU)) >> 0xc;
  var_i5 = g_LisaCameraMatrix_10 * var_i6 + g_LisaCameraMatrix_12 * var_i7 + g_LisaCameraMatrix_11 * var_i14;
  g_LisaSubmeshCenterWorldZ = (int)(var_i5 + (var_i5 >> 0x1f & 0xfffU)) >> 0xc;
  var_i6 = g_LisaCameraMatrix_20 * var_i6 + g_LisaCameraMatrix_22 * var_i7 + g_LisaCameraMatrix_21 * var_i14;
  var_c = 0;
  g_LisaSubmeshDepthOffset = (int)(var_i6 + (var_i6 >> 0x1f & 0xfffU)) >> 0xc;

  if (0 < g_LisaCamera->visible_obj_count) {
    var_1c = 0;
    var_10 = 0;

    do {
      var_i14 = g_LisaCamera;
      var_i6 = *(int *)(g_LisaVisibleObjects + var_10);
      var_pi2 = *(int **)(var_i6 + 4);
      *(int **)(g_LisaVisibleSubmeshes + var_1c) = var_pi2;
      var_i14 = *(int *)(var_i14 + 0x5c);
      var_pi11 = (int *)(g_LisaTransformedVertices + var_i14 * 0xc);
      *(int **)(g_LisaVisibleSubmeshes + 4 + var_1c) = var_pi11;
      var_i7 = *(int *)(var_i6 + 0xc) - (int)var_l21;
      var_i5 = *(int *)(var_i6 + 0x10) - (int)var_l22;
      var_i8 = *(int *)(var_i6 + 0x14) - (int)var_l23;

      if ((*(short *)(var_i6 + 0x18) == 0 && *(short *)(var_i6 + 0x1a) == 0) &&

          *(short *)(var_i6 + 0x1c) == 0) {
        var_20 = *var_pi2;

        if (0 < var_20) {
          g_LisaCamera->vertex_counter = var_i14 + var_20;
          var_i6 = 2;

          do {
            var_i9 = var_i7 + var_pi2[var_i6];
            var_i12 = var_i5 - var_pi2[var_i6 + 1];
            var_i10 = var_i8 + var_pi2[var_i6 + 2];

            var_i13 = (g_LisaCameraMatrix_12 * var_i10 + g_LisaCameraMatrix_11 * var_i12 + g_LisaCameraMatrix_10 * var_i9) -

                     g_LisaSubmeshCenterWorldZ;

            var_i14 = ((g_LisaCameraMatrix_22 * var_i10 + g_LisaCameraMatrix_21 * var_i12 + g_LisaCameraMatrix_20 * var_i9) -

                      g_LisaSubmeshDepthOffset >> 0x10) + g_LisaCameraFocalLength;

            if (var_i14 < g_LisaCameraFocalLength) {
              var_i14 = g_LisaCameraFocalLength;
            }

            *var_pi11 = g_LisaCameraOffsetX +

                       (((g_LisaCameraMatrix_02 * var_i10 + g_LisaCameraMatrix_01 * var_i12 + g_LisaCameraMatrix_00 * var_i9) -

                        g_LisaSubmeshCenterWorldX) / var_i14) * 4;
            var_20 = var_20 + -1;
            var_pi11[2] = var_i14;
            var_pi11[1] = g_LisaCameraOffsetY + (var_i13 / var_i14) * 4;
            var_pi11 = var_pi11 + 3;
            var_i6 = var_i6 + 3;
          } while (var_20 != 0);
        }

      }

      else {
        Lisa_TransformSubmeshVerticesPanorama(var_i7,var_i5,var_i8,var_i6,var_pi2);
      }

      var_1c = var_1c + 8;
      var_10 = var_10 + 4;
      var_c = var_c + 1;
    } while (var_c < g_LisaCamera->visible_obj_count);
  }

  return 0;
}

/**
 * @original Lisa_ComputeObjectMatrix (IGN_WIN.EXE @ 0x0044a900, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_ComputeObjectMatrix(int param_1,int param_2,int param_3,int param_4,int *param_5) {
  ushort var_u1;
  ushort var_u2;
  ushort var_u3;
  short sVar4;
  int var_i5;
  int var_i6;
  int var_i7;
  int var_i8;
  int var_i9;
  int *var_pi10;
  int var_i11;
  int var_i12;
  int var_i13;
  int var_i14;
  int var_l15;
  int var_l16;
  int var_l17;
  int var_l18;
  int var_l19;
  int var_l20;
  int var_l21;
  int var_l22;
  int var_l23;
  int var_58;

  

  var_i11 = (g_LisaCameraMatrix_02 * param_3 + g_LisaCameraMatrix_01 * param_2 + g_LisaCameraMatrix_00 * param_1) - g_LisaSubmeshCenterWorldX

  ;

  var_i12 = (g_LisaCameraMatrix_12 * param_3 + g_LisaCameraMatrix_11 * param_2 + g_LisaCameraMatrix_10 * param_1) - g_LisaSubmeshCenterWorldZ

  ;

  var_i13 = (g_LisaCameraMatrix_22 * param_3 + g_LisaCameraMatrix_21 * param_2 + g_LisaCameraMatrix_20 * param_1) - g_LisaSubmeshDepthOffset

  ;
  var_u1 = *(ushort *)(param_4 + 0x1a);
  var_u2 = *(ushort *)(param_4 + 0x18);
  var_u3 = *(ushort *)(param_4 + 0x1c);

  if (0xe10 < (ushort)(var_u2 | var_u1 | var_u3)) {
    if ((short)var_u2 < 0) {
      *(ushort *)(param_4 + 0x18) = ((ushort)(0xe0f - var_u2) / 0xe10) * 0xe10 + var_u2;
    }

    if ((short)var_u1 < 0) {
      *(ushort *)(param_4 + 0x1a) = ((ushort)(0xe0f - var_u1) / 0xe10) * 0xe10 + var_u1;
    }

    if ((short)var_u3 < 0) {
      *(ushort *)(param_4 + 0x1c) = ((ushort)(0xe0f - var_u3) / 0xe10) * 0xe10 + var_u3;
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

  var_l15 = __ftol();
  var_l16 = __ftol();
  var_l17 = __ftol();
  var_l18 = __ftol();
  var_l19 = __ftol();
  var_l20 = __ftol();
  var_l21 = __ftol();
  var_l22 = __ftol();
  var_l23 = __ftol();
  var_i6 = g_LisaTransformedVertices;
  var_i9 = 2;
  var_58 = *param_5;

  if (0 < var_58) {
    var_i5 = g_LisaCamera->vertex_counter;
    g_LisaCamera->vertex_counter = var_i5 + var_58;
    var_pi10 = (int *)(var_i6 + var_i5 * 0xc);

    do {
      var_i6 = param_5[var_i9];
      var_i5 = param_5[var_i9 + 1];
      var_i7 = param_5[var_i9 + 2];
      var_i9 = var_i9 + 3;

      var_i14 = g_LisaCameraFocalLength +

               (var_i7 * (int)var_l23 + var_i5 * (int)var_l22 + var_i6 * (int)var_l21 + var_i13 >> 0x10);

      if (var_i14 < g_LisaCameraFocalLength) {
        var_i14 = g_LisaCameraFocalLength;
      }

      *var_pi10 = g_LisaCameraOffsetX +

                 (var_i11 + var_i7 * (int)var_l17 + var_i5 * (int)var_l16 + var_i6 * (int)var_l15) / var_i14

      ;
      var_i8 = g_LisaCameraOffsetY;
      var_pi10[2] = var_i14;
      var_58 = var_58 + -1;

      var_pi10[1] = var_i8 + (var_i12 + var_i7 * (int)var_l20 + var_i5 * (int)var_l19 + var_i6 * (int)var_l18

                           ) / var_i14;
      var_pi10 = var_pi10 + 3;
    } while (var_58 != 0);
  }

  return 0;
}

/**
 * @original Lisa_TransformSubmeshVerticesPanorama (IGN_WIN.EXE @ 0x0044ae20, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_TransformSubmeshVerticesPanorama(int param_1,int param_2,int param_3,int param_4,int *param_5) {
  ushort var_u1;
  ushort var_u2;
  ushort var_u3;
  short sVar4;
  int var_i5;
  int var_i6;
  int var_i7;
  int var_i8;
  int var_i9;
  int *var_pi10;
  int var_i11;
  int var_i12;
  int var_i13;
  int var_l14;
  int var_l15;
  int var_l16;
  int var_l17;
  int var_l18;
  int var_l19;
  int var_l20;
  int var_l21;
  int var_l22;
  int var_58;

  

  var_i11 = (g_LisaCameraMatrix_02 * param_3 + g_LisaCameraMatrix_01 * param_2 + g_LisaCameraMatrix_00 * param_1) - g_LisaSubmeshCenterWorldX

  ;

  var_i12 = (g_LisaCameraMatrix_12 * param_3 + g_LisaCameraMatrix_11 * param_2 + g_LisaCameraMatrix_10 * param_1) - g_LisaSubmeshCenterWorldZ

  ;

  var_i13 = (g_LisaCameraMatrix_22 * param_3 + g_LisaCameraMatrix_21 * param_2 + g_LisaCameraMatrix_20 * param_1) - g_LisaSubmeshDepthOffset

  ;
  var_u1 = *(ushort *)(param_4 + 0x1a);
  var_u2 = *(ushort *)(param_4 + 0x18);
  var_u3 = *(ushort *)(param_4 + 0x1c);

  if (0xe10 < (ushort)(var_u2 | var_u1 | var_u3)) {
    if ((short)var_u2 < 0) {
      *(ushort *)(param_4 + 0x18) = ((ushort)(0xe0f - var_u2) / 0xe10) * 0xe10 + var_u2;
    }

    if ((short)var_u1 < 0) {
      *(ushort *)(param_4 + 0x1a) = ((ushort)(0xe0f - var_u1) / 0xe10) * 0xe10 + var_u1;
    }

    if ((short)var_u3 < 0) {
      *(ushort *)(param_4 + 0x1c) = ((ushort)(0xe0f - var_u3) / 0xe10) * 0xe10 + var_u3;
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

  var_l14 = __ftol();
  var_l15 = __ftol();
  var_l16 = __ftol();
  var_l17 = __ftol();
  var_l18 = __ftol();
  var_l19 = __ftol();
  var_l20 = __ftol();
  var_l21 = __ftol();
  var_l22 = __ftol();
  var_i6 = g_LisaTransformedVertices;
  var_58 = *param_5;

  if (0 < var_58) {
    var_i5 = g_LisaCamera->vertex_counter;
    g_LisaCamera->vertex_counter = var_i5 + var_58;
    var_i8 = 2;
    var_pi10 = (int *)(var_i6 + var_i5 * 0xc);

    do {
      var_i6 = param_5[var_i8];
      var_i5 = param_5[var_i8 + 1];
      var_i7 = param_5[var_i8 + 2];

      var_i9 = (var_i5 * (int)var_l21 + var_i7 * (int)var_l22 + var_i6 * (int)var_l20 + var_i13 >> 0x10) +

              g_LisaCameraFocalLength;

      if (var_i9 < g_LisaCameraFocalLength) {
        var_i9 = g_LisaCameraFocalLength;
      }

      *var_pi10 = g_LisaCameraOffsetX +

                 ((var_i11 + var_i5 * (int)var_l15 + var_i7 * (int)var_l16 + var_i6 * (int)var_l14) / var_i9

                 ) * 4;
      var_58 = var_58 + -1;
      var_pi10[2] = var_i9;

      var_pi10[1] = g_LisaCameraOffsetY +

                   ((var_i12 + var_i7 * (int)var_l19 + var_i5 * (int)var_l18 + var_i6 * (int)var_l17) /

                   var_i9) * 4;
      var_i8 = var_i8 + 3;
      var_pi10 = var_pi10 + 3;
    } while (var_58 != 0);
  }

  return 0;
}

/**
 * @original Lisa_ComputeCameraRotationMatrix (IGN_WIN.EXE @ 0x0044b340, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_ComputeCameraRotationMatrix(int *param_1) {
  double var_f1;
  double var_f2;
  int var_l3;

  

  var_f1 = (double)g_LisaCamera->rot_x * (double)g_Const_NegTenthDegToRad;
  fcos(var_f1);
  var_f2 = (double)g_LisaCamera->rot_y * (double)g_Const_NegTenthDegToRad;
  fcos(var_f2);
  fsin(var_f1);
  var_f1 = (double)g_LisaCamera->rot_z * (double)g_Const_NegTenthDegToRad;
  fsin(var_f2);
  fcos(var_f1);
  fsin(var_f1);
  var_l3 = __ftol();
  *param_1 = (int)var_l3;
  var_l3 = __ftol();
  param_1[1] = (int)var_l3;
  var_l3 = __ftol();
  param_1[2] = (int)var_l3;
  var_l3 = __ftol();
  param_1[3] = (int)var_l3;
  var_l3 = __ftol();
  param_1[4] = (int)var_l3;
  var_l3 = __ftol();
  param_1[5] = (int)var_l3;
  var_l3 = __ftol();
  param_1[6] = (int)var_l3;
  var_l3 = __ftol();
  param_1[7] = (int)var_l3;
  var_l3 = __ftol();
  param_1[8] = (int)var_l3;
  return;
}

/**
 * @original Lisa_InitOpcodeTable (IGN_WIN.EXE @ 0x0044b480, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_InitOpcodeTable(void) {
  int var_i1;

  

  camera = g_LisaCamera;
  g_LisaViewportQuarter = (g_LisaCamera->viewport_width >> 2) + 1;
  g_LisaViewportRemaining = g_LisaCamera->viewport_width - g_LisaViewportQuarter;

  if (g_LisaDisableFiltering != 0) {
    if (g_LisaCamera->shading_mode == 0) {
      if (g_LisaShadingEnabled == 0) {
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
    *(int *)(var_i1 + 0x6c) = 0;
    return;
  }

  if (g_LisaCamera->shading_mode == 0) {
    if (g_LisaShadingEnabled == 0) {
      PTR_LAB_0049c924 = ((void *)0x0044c2e0);
    }

    else {
      PTR_LAB_0049c924 = Lisa_DrawTexturedTriangle_Op11_Unshaded;
    }

  }

  else {
    PTR_LAB_0049c924 = ((void *)0x0044bb90);

    if (g_LisaShadingEnabled != 0) {
      PTR_LAB_0049c924 = Lisa_DrawTexturedTriangle_Op11_Shaded;
    }

  }

  if (g_LisaShadingEnabled == 0) {
    PTR_Lisa_DrawTexturedTriangle_Op15_0049c934 = Lisa_DrawTexturedTriangle_Op15;
    g_LisaCamera->active_draw_cmd = 0;
    *(int *)(var_i1 + 0x6c) = 0;
    return;
  }

  PTR_Lisa_DrawTexturedTriangle_Op15_0049c934 = Lisa_DrawTexturedTriangle_Op15_Sub;
  g_LisaCamera->active_draw_cmd = 0;
  *(int *)(var_i1 + 0x6c) = 0;
  return;
}

/**
 * @original Lisa_SortDepthBuckets (IGN_WIN.EXE @ 0x0044b570, lisa3d.c)
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
 * @original Lisa_DrawTriangle_Op0F (IGN_WIN.EXE @ 0x0044b770, lisa3d.c)
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
 * @original Lisa_DrawTriangle_Op10 (IGN_WIN.EXE @ 0x0044b980, lisa3d.c)
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
 * @original Lisa_RenderSubmeshes (IGN_WIN.EXE @ 0x0044c1f0, lisa3d.c)
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
                (*(void (*)(void))(&g_LisaOpcodeTable)[(char)*g_pLisaSubmeshPolygon])();
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
 * @original Lisa_DrawTriangle_OpcodeHelper (IGN_WIN.EXE @ 0x0044cb20, lisa3d.c)
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
 * @original Lisa_DrawTriangle_Op14 (IGN_WIN.EXE @ 0x0044cf00, lisa3d.c)
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
 * @original Lisa_DrawBillboard_Op07 (IGN_WIN.EXE @ 0x0044d0f0, lisa3d.c)
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
 * @original Lisa_DrawBillboard_Op08 (IGN_WIN.EXE @ 0x0044d230, lisa3d.c)
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
 * @original Lisa_DrawTexturedTriangle_Op15 (IGN_WIN.EXE @ 0x0044d550, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_DrawTexturedTriangle_Op15(void) {
  int var_i1;
  int var_i2;
  int var_i3;
  int var_i4;
  int *var_pu5;
  int var_u6;
  int var_u7;
  int var_u8;
  int var_i9;
  int var_i10;
  int var_i11;
  int var_i12;
  int var_i13;
  int var_i14;
  int var_i15;
  int var_i16;
  int var_i17;
  int var_i18;
  int *var_pu19;
  unsigned int var_u20;
  unsigned int var_u21;
  int var_i22;
  int *var_pu23;
  int var_i24;
  int var_l25;

  

  var_i9 = g_LisaTransformedVertices;
  g_LisaObjMat_11 = g_LisaTransformedVertices;

  do {
    var_i24 = ((MshPolygon*)g_pLisaSubmeshPolygon)->vi0;
    var_i22 = var_i24 * 3;
    g_LisaSubmeshVertexStride = ((MshPolygon*)g_pLisaSubmeshPolygon)->vi1 * 3;
    g_LisaCameraFocalScale = ((MshPolygon*)g_pLisaSubmeshPolygon)->vi2 * 3;
    var_i1 = *(int *)(var_i9 + 4 + var_i24 * 0xc);
    var_i24 = *(int *)(var_i9 + var_i24 * 0xc);

    if ((g_SubpixelMaxY - var_i1 | g_SubpixelMaxX - var_i24 | var_i1 - g_SubpixelMinY | var_i24 - g_SubpixelMinX

        ) < 0) {
      do {
        var_i24 = *(int *)(var_i9 + var_i22 * 4);
        var_i1 = *(int *)(var_i9 + g_LisaSubmeshVertexStride * 4);
        g_LisaCameraMatrix_Y = var_i24;

        if (var_i24 <= var_i1) {
          g_LisaCameraMatrix_Y = var_i1;
        }

        var_i2 = *(int *)(var_i9 + g_LisaCameraFocalScale * 4);
        var_i11 = g_LisaCameraMatrix_Y;

        if (g_LisaCameraMatrix_Y <= var_i2) {
          var_i11 = var_i2;
        }

        if (g_SubpixelMinX <= var_i11) {
          g_LisaCameraMatrix_Y = var_i24;

          if (var_i1 <= var_i24) {
            g_LisaCameraMatrix_Y = var_i1;
          }

          var_i24 = g_LisaCameraMatrix_Y;

          if (var_i2 <= g_LisaCameraMatrix_Y) {
            var_i24 = var_i2;
          }

          if (var_i24 <= g_SubpixelMaxX) {
            var_i24 = *(int *)(var_i9 + 4 + var_i22 * 4);
            var_i1 = *(int *)(var_i9 + 4 + g_LisaSubmeshVertexStride * 4);
            g_LisaCameraMatrix_Y = var_i24;

            if (var_i24 <= var_i1) {
              g_LisaCameraMatrix_Y = var_i1;
            }

            var_i2 = *(int *)(var_i9 + 4 + g_LisaCameraFocalScale * 4);
            var_i11 = g_LisaCameraMatrix_Y;

            if (g_LisaCameraMatrix_Y <= var_i2) {
              var_i11 = var_i2;
            }

            if (g_SubpixelMinY <= var_i11) {
              g_LisaCameraMatrix_Y = var_i24;

              if (var_i1 <= var_i24) {
                g_LisaCameraMatrix_Y = var_i1;
              }

              var_i24 = g_LisaCameraMatrix_Y;

              if (var_i2 <= g_LisaCameraMatrix_Y) {
                var_i24 = var_i2;
              }

              if (var_i24 <= g_SubpixelMaxY) break;
            }

          }

        }

        var_i24 = g_pLisaSubmeshPolygon + 0x2c;

        if (*(char *)(g_pLisaSubmeshPolygon + 0x2c) != '\x15') {
          g_LisaObjMat_Tmp6 = var_i22;
          g_pLisaSubmeshPolygon = var_i24;
          return;
        }

        if (g_LisaSubmeshPolyCount < 3) {
          g_LisaObjMat_Tmp6 = var_i22;
          g_pLisaSubmeshPolygon = var_i24;
          return;
        }

        g_LisaSubmeshPolyCount = g_LisaSubmeshPolyCount + -1;
        var_i22 = *(int *)(g_pLisaSubmeshPolygon + 0x30) * 3;
        g_LisaCameraFocalScale = *(int *)(g_pLisaSubmeshPolygon + 0x38) * 3;
        g_LisaSubmeshVertexStride = *(int *)(g_pLisaSubmeshPolygon + 0x34) * 3;
        g_pLisaSubmeshPolygon = var_i24;
      } while( 1 );
    }

    var_i10 = g_pLisaSubmeshPolygon;
    var_i24 = *(int *)(var_i9 + 4 + g_LisaSubmeshVertexStride * 4);
    var_i1 = *(int *)(var_i9 + g_LisaCameraFocalScale * 4);
    var_i2 = *(int *)(var_i9 + g_LisaSubmeshVertexStride * 4);
    var_i11 = *(int *)(var_i9 + 4 + g_LisaCameraFocalScale * 4);

    g_LisaSubmeshCenterWorldY = ((var_i24 >> 4) - (*(int *)(var_i9 + 4 + var_i22 * 4) >> 4)) *

                   ((var_i1 >> 4) - (var_i2 >> 4)) +

                   ((var_i11 >> 4) - (var_i24 >> 4)) *

                   ((*(int *)(var_i9 + var_i22 * 4) >> 4) - (var_i2 >> 4)) ^ g_LisaBackfaceSign;
    var_i3 = *(int *)(var_i9 + 8 + g_LisaCameraFocalScale * 4);
    var_i4 = *(int *)(var_i9 + 8 + g_LisaSubmeshVertexStride * 4);
    var_i12 = var_i3 + var_i4 + *(int *)(var_i9 + 8 + var_i22 * 4);
    g_LisaObjMat_Tmp6 = var_i22;

    if ((600 < var_i12) && (0 < (int)g_LisaSubmeshCenterWorldY)) {
      g_LisaCameraMatrix_X = var_i12 >> 4;
      var_pu5 = (int *)*g_pLisaDrawCommandWritePtr;

      if (g_LisaShadingEnabled == 1) {
        var_i17 = *(int *)(var_i9 + var_i22 * 4) >> 8;
        var_i13 = *(int *)(var_i9 + 4 + var_i22 * 4) >> 8;

        var_i13 = (((var_i24 >> 8) + var_i13) * ((var_i2 >> 8) - var_i17) +

                  ((var_i11 >> 8) + var_i13) * (var_i17 - (var_i1 >> 8)) +

                 ((var_i24 >> 8) + (var_i11 >> 8)) * ((var_i1 >> 8) - (var_i2 >> 8))) * 3;
        var_u20 = var_i13 >> 0x1f;
        g_LisaActiveLightingMode = (var_i13 >> 1 ^ var_u20) - var_u20;
        var_i13 = ((MshPolygon*)g_pLisaSubmeshPolygon)->tv2 >> 8;
        var_i17 = ((MshPolygon*)g_pLisaSubmeshPolygon)->tv0 >> 8;
        var_i18 = ((MshPolygon*)g_pLisaSubmeshPolygon)->tu2 >> 8;
        var_i14 = ((MshPolygon*)g_pLisaSubmeshPolygon)->tu0 >> 8;
        var_i15 = ((MshPolygon*)g_pLisaSubmeshPolygon)->tv1 >> 8;
        var_i16 = ((MshPolygon*)g_pLisaSubmeshPolygon)->tu1 >> 8;

        var_u20 = (var_i15 + var_i17) * (var_i16 - var_i14) +

                 (var_i17 + var_i13) * (var_i14 - var_i18) + (var_i15 + var_i13) * (var_i18 - var_i16);
        var_u21 = (int)var_u20 >> 0x1f;
        var_i13 = (var_u20 ^ var_u21) - var_u21;
        if (var_i13 < g_LisaActiveLightingMode) goto LAB_0044d8ef;
        g_LisaSubmeshBoundRadius = (g_LisaActiveLightingMode * 4 <= var_i13) - 4;
      }

      else {
LAB_0044d8ef:

        g_LisaSubmeshBoundRadius = -5;
      }

      if (((g_LisaCamera->shading_mode == 0) ||

          (g_LisaObjMat_20 = (int)*(short *)(g_pLisaActiveSubmesh + 0x1e), 0x2d0 < g_LisaCameraMatrix_X)) ||

         (((99 < g_LisaObjMat_20 && (g_LisaObjMat_20 != 200)) &&

          ((g_LisaObjMat_20 < 300 || (0x12e < g_LisaObjMat_20)))))) {
        var_u6 = *(int *)(var_i9 + var_i22 * 4);
        var_u7 = *(int *)(var_i9 + 4 + var_i22 * 4);
        g_LisaSubmeshTmp5 = var_pu5;
        *var_pu5 = 0x11;
        var_pu5[1] = var_u6;
        var_pu5[2] = var_u7;
        var_pu5[3] = var_i2;
        var_i22 = g_pLisaActiveMipTable;
        var_pu5[4] = var_i24;
        var_pu5[5] = var_i1;
        var_i24 = g_LisaSubmeshBoundRadius;
        var_pu5[6] = var_i11;
        var_pu5[7] = g_pLisaSubmeshPolygon + 0x10;
        var_pu19 = g_pLisaDrawCommandWritePtr;
        var_pu23 = var_pu5 + 9;
        var_pu5[8] = *(int *)(var_i22 + var_i24 * 4) + ((MshPolygon*)g_pLisaSubmeshPolygon)->extra;
      }

      else {
        if (g_LisaCameraMatrix_X < 0x1e1) {
          var_pu5[4] = *(int *)(var_i9 + 8 + var_i22 * 4);
          var_pu5[7] = var_i4;
          var_pu5[10] = var_i3;
        }

        else {
          g_LisaSubmeshFlags = var_i12 / 3;
          g_LisaSubmeshTmp4 = (float)(0x2d0 - g_LisaCameraMatrix_X) * g_Const_512;
          var_l25 = __ftol();
          var_pu5[4] = (int)var_l25 + g_LisaSubmeshFlags;
          var_l25 = __ftol();
          var_pu5[7] = (int)var_l25 + g_LisaSubmeshFlags;
          g_LisaSubmeshTmp1 = var_i3;
          var_l25 = __ftol();
          var_pu5[10] = (int)var_l25 + g_LisaSubmeshFlags;
        }

        var_u6 = *(int *)(var_i9 + var_i22 * 4);
        var_u7 = *(int *)(var_i9 + 4 + var_i22 * 4);
        g_LisaSubmeshTmp5 = var_pu5;
        *var_pu5 = 0x14;
        var_pu5[2] = var_u6;
        var_pu5[3] = var_u7;
        var_pu5[5] = var_i2;
        var_pu5[6] = var_i24;
        var_u6 = ((MshPolygon*)var_i10)->tu0;
        var_pu5[8] = var_i1;
        var_u7 = ((MshPolygon*)var_i10)->tv0;
        var_pu5[9] = var_i11;
        var_u8 = ((MshPolygon*)var_i10)->tu1;
        var_pu5[0xb] = var_u6;
        var_u6 = ((MshPolygon*)var_i10)->tv1;
        var_pu5[0xc] = var_u7;
        var_u7 = ((MshPolygon*)var_i10)->tu2;
        var_pu5[0xd] = var_u8;
        var_u8 = ((MshPolygon*)var_i10)->tv2;
        var_pu5[0xe] = var_u6;
        var_pu5[0xf] = var_u7;
        var_pu5[0x10] = var_u8;
        var_pu19 = g_pLisaDrawCommandWritePtr;
        var_pu23 = var_pu5 + 0x11;
        var_pu5[1] = *(int *)(g_pLisaActiveMipTable + g_LisaSubmeshBoundRadius * 4) + ((MshPolygon*)var_i10)->extra;
      }

      var_i24 = g_pLisaActiveSubmesh;
      g_LisaCameraMatrix_X = g_LisaCameraMatrix_X + -0x50;
      var_pu19[2] = var_pu23;
      var_i1 = g_pLisaDepthBuckets;
      var_pu5 = g_pLisaDrawCommandWritePtr;

      if (99 < *(short *)(var_i24 + 0x1e)) {
        if (*(short *)(var_i24 + 0x1e) == 0xd2) {
          var_i24 = -0x54;
        }

        else {
          var_i24 = -0x5c;
        }

        g_LisaCameraMatrix_X = g_LisaCameraMatrix_X + var_i24;
      }

      if (g_LisaCameraMatrix_X < 0) {
        g_LisaCameraMatrix_X = 0;
      }

      if (5999 < g_LisaCameraMatrix_X) {
        g_LisaCameraMatrix_X = 5999;
      }

      var_i24 = g_LisaCameraMatrix_X;
      var_pu23 = g_pLisaDrawCommandWritePtr + 1;
      g_pLisaDrawCommandWritePtr = var_pu19 + 2;
      *var_pu23 = *(int *)(g_pLisaDepthBuckets + g_LisaCameraMatrix_X * 4);
      *(int **)(var_i1 + var_i24 * 4) = var_pu5;
    }

    g_pLisaSubmeshPolygon = var_i10 + 0x2c;

    if ((*(char *)(var_i10 + 0x2c) != '\x15') || (g_LisaSubmeshPolyCount < 3)) {
      return;
    }

    g_LisaSubmeshPolyCount = g_LisaSubmeshPolyCount + -1;
  } while( 1 );
}

/**
 * @original Lisa_DrawTexturedTriangle_Op15_Sub (IGN_WIN.EXE @ 0x0044e900, lisa3d.c)
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
 * @original Lisa_InitRasterizerTables (IGN_WIN.EXE @ 0x0044f070, lisa3d.c)
 * @fidelity ADAPTED
 */
LisaReturn64 Lisa_InitRasterizerTables(int param_1,unsigned int param_2) {
  int in_EAX;
  short sVar1;
  int unaff_EBX;
  unsigned int var_u2;
  int var_i3;
  int unaff_ESI;
  int *var_pi4;
  char **ppuVar5;

  

  if ((in_EAX < 0x579) && (unaff_EBX < 0x259)) {
    var_pi4 = &g_LisaActiveTextureID;
    var_u2 = 1;
    g_LisaScanlinePitch = in_EAX;
    g_LisaAspectScale = unaff_EBX;

    do {
      *var_pi4 = (int)(0x10000 / (unsigned int)var_u2) + -1;
      var_pi4 = var_pi4 + 1;
      var_u2 = var_u2 + 1;
    } while (var_u2 != 0x3a9b);

    for (ppuVar5 = (char **)g_pLisaShutdownCallbacks; *ppuVar5 != (char *)0x0; ppuVar5 = ppuVar5 + 1) {
      (*(void (*)())*ppuVar5)(ppuVar5,unaff_ESI,unaff_EBX);
    }

    var_pi4 = &g_LisaScreenPitch;
    var_i3 = 0;
    sVar1 = 600;

    do {
      *var_pi4 = var_i3;
      var_pi4 = var_pi4 + 1;
      var_i3 = var_i3 + in_EAX;
      sVar1 = sVar1 + -1;
    } while (sVar1 != 0);
    { LisaReturn64 _r; _r.edx = param_2; _r.eax = 0; return _r; }
  }

  { LisaReturn64 _r; _r.edx = param_2; _r.eax = 0xffffffff; return _r; }
}

/**
 * @original Lisa_ExecuteRasterizerCommands (IGN_WIN.EXE @ 0x0044f0e9, lisa3d.c)
 * @fidelity ADAPTED
 */
LisaReturn64 Lisa_ExecuteRasterizerCommands(int param_1,unsigned int param_2) {
  int *unaff_ESI;
  LisaReturn64 var_l1;

  

  g_pLisaScanlineBuffer = unaff_ESI;
  g_pLisaSpanBuffer = (int *)*unaff_ESI;
  g_LisaCameraZoom = unaff_ESI[1];
  g_pLisaActiveShading = unaff_ESI[3];
  g_LisaClipLeft = unaff_ESI[4];
  g_LisaClipRight = unaff_ESI[5];
  g_LisaClipSubpixelLeft = g_LisaClipLeft;
  g_LisaViewportCenterX = g_LisaClipLeft << 8;
  g_LisaClipSubpixelRight = g_LisaClipRight;
  g_LisaViewportCenterY = g_LisaClipRight << 8;
  g_SubpixelMinX = g_LisaViewportCenterX;
  g_SubpixelMinY = g_LisaViewportCenterY;
  g_LisaClipTop = unaff_ESI[6];
  g_LisaClipBottom = unaff_ESI[7];
  g_LisaClipSubpixelTop = g_LisaClipTop + 1;
  g_LisaClipSubpixelBottom = g_LisaClipBottom + 1;
  g_SubpixelMaxX = g_LisaClipSubpixelTop * 0x100;
  g_LisaViewportWidth = g_SubpixelMaxX + -1;
  g_SubpixelMaxY = g_LisaClipSubpixelBottom * 0x100;
  g_LisaViewportHeight = g_SubpixelMaxY + -1;
  g_pLisaEdgeBuffer = (int *)*g_pLisaSpanBuffer;

  if (g_pLisaEdgeBuffer != (int *)0x0) {
    var_l1 = (*(LisaReturn64 (*)())(((void **)g_LisaRasterizerJmpTable)[*g_pLisaEdgeBuffer]))();
    return var_l1;
  }

  { LisaReturn64 _r; _r.edx = param_2; _r.eax = 0; return _r; }
}
