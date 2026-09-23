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
LisaReturn64 Lisa_CreateDynamicObject(int arg_1, int object_id, LisaEntityTransform *entity, MshSubmesh *mesh, int arg_5, short arg_6, short arg_7, short base_elevation, short arg_9);
int Lisa_MoveDynamicObject(LisaEntityTransform *entity);
int Lisa_UpdateObjectSpatialGrid(LisaEntityTransform *entity);
LisaReturn64 Lisa_SetDynamicObjectMesh(int arg_1, int arg_2, LisaEntityTransform *entity, MshSubmesh *mesh, int arg_5, short arg_6, short arg_7, short base_elevation, short arg_9);
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
  int local_i1;
  int *local_pu2;
  int local_i3;
  int local_i4;
  int local_i5;
  int *local_pi6;
  int *local_pu7;
  int local_i8;

  

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

  local_i8 = 0;

  if (g_LisaCamera->enable_depth_sort != 0) {
    local_i5 = 5999;
    local_pi6 = (int *)(g_pLisaDepthBuckets + 0x5dbc);

    do {
      local_pu7 = (int *)*local_pi6;

      if (local_pu7 != (int *)0x0) {
        *(int *)(g_LisaDrawCommands + local_i8 * 4) = *local_pu7;
        local_i8 = local_i8 + 1;

        if (local_pu7[1] != 0) {
          local_pu2 = (int *)(g_LisaDrawCommands + local_i8 * 4);

          do {
            local_pu7 = (int *)local_pu7[1];
            local_i8 = local_i8 + 1;
            *local_pu2 = *local_pu7;
            local_pu2 = local_pu2 + 1;
          } while (local_pu7[1] != 0);
        }

      }

      *local_pi6 = 0;
      local_pi6 = local_pi6 + -1;
      local_i5 = local_i5 + -1;
    } while (-1 < local_i5);
    *(int *)(g_LisaDrawCommands + local_i8 * 4) = 0;
  }

  if (g_LisaEnableMipmaps != 0) {
    if (g_LisaEnableMipmaps < 10) {
      local_i8 = g_LisaActivePageCount / 2;

      if (0 < local_i8) {
        local_i5 = 0x28;

        do {
          local_i3 = rand();
          local_i3 = local_i3 % local_i8;
          local_i1 = *(int *)(&g_pLisaTexturePageTable1 + local_i3 * 8);
          local_i3 = *(int *)(&g_pLisaTexturePageTable2 + local_i3 * 8);
          local_i4 = rand();
          g_LisaRasterizerAccumulator = g_LisaRasterizerAccumulator + *(char *)((local_i4 << 10) % local_i3 + local_i1);
          local_i4 = rand();
          g_LisaRasterizerAccumulator = g_LisaRasterizerAccumulator + *(char *)((local_i4 << 10) % local_i3 + local_i1);
          local_i4 = rand();
          g_LisaRasterizerAccumulator = g_LisaRasterizerAccumulator + *(char *)((local_i4 << 10) % local_i3 + local_i1);
          local_i5 = local_i5 + -1;
        } while (local_i5 != 0);
      }

    }

    else {
      local_i8 = g_LisaActivePageCount / 2;

      if (0 < local_i8) {
        local_i5 = 2;

        do {
          local_i3 = rand();
          local_i3 = local_i3 % local_i8;
          local_i1 = *(int *)(&g_pLisaTexturePageTable1 + local_i3 * 8);
          local_i3 = *(int *)(&g_pLisaTexturePageTable2 + local_i3 * 8);
          local_i4 = rand();
          g_LisaRasterizerAccumulator = g_LisaRasterizerAccumulator + *(char *)((local_i4 << 10) % local_i3 + local_i1);
          local_i4 = rand();
          g_LisaRasterizerAccumulator = g_LisaRasterizerAccumulator + *(char *)((local_i4 << 10) % local_i3 + local_i1);
          local_i4 = rand();
          g_LisaRasterizerAccumulator = g_LisaRasterizerAccumulator + *(char *)((local_i4 << 10) % local_i3 + local_i1);
          local_i5 = local_i5 + -1;
        } while (local_i5 != 0);
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
  int *local_pu5;
  void *pvVar6;
  int local_i7;
  int local_i8;
  double local_f9;

  

  g_pLisaAllocatedBuffers = calloc(5000,4);
  g_LisaAllocatedBufferCount = 0;
  g_LisaVisibleObjects = calloc(0x5dc,4);
  g_LisaVisibleSubmeshes = calloc(3000,4);
  local_pu5 = calloc(0x1838,4);
  local_i7 = g_LisaActivePageCount + 2;
  g_pLisaDepthBuckets = local_pu5;
  g_LisaActivePageCount = local_i7;
  *(int **)(&g_pLisaTexturePagePointers + local_i7 * 4) = local_pu5;
  *(int *)(&g_LisaTexturePageSizes + local_i7 * 4) = 0x1a90;

  for (local_i7 = 6000; local_i7 != 0; local_i7 = local_i7 + -1) {
    *local_pu5 = 0;
    local_pu5 = local_pu5 + 1;
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
  local_pu5 = g_pLisaDepthBuckets;
  *(void **)((int)pvVar2 + (g_LisaAllocatedBufferCount + 5) * 4 + -0x10) = pvVar3;
  pvVar3 = g_LisaTransformedVertices;
  *(int **)((int)pvVar2 + (g_LisaAllocatedBufferCount + 5) * 4 + -0xc) = local_pu5;
  pvVar4 = g_pLisaDrawCommandTail;
  local_pu5 = g_LisaCamera;
  *(int **)((int)pvVar2 + (g_LisaAllocatedBufferCount + 7) * 4 + -0x10) = g_LisaCamera;
  local_i7 = g_LisaAllocatedBufferCount + 9;
  g_LisaAllocatedBufferCount = local_i7;
  *(void **)((int)pvVar2 + local_i7 * 4 + -0x14) = pvVar3;
  *(void **)((int)pvVar2 + local_i7 * 4 + -0x10) = pvVar1;
  pvVar1 = g_pLisaTextureSheets;
  *(void **)((int)pvVar2 + local_i7 * 4 + -0xc) = g_pLisaTextureSheets;
  *(void **)((int)pvVar2 + local_i7 * 4 + -8) = pvVar4;
  *(void **)((int)pvVar2 + local_i7 * 4 + -4) = pvVar6;

  if (local_pu5 == (int *)0x0) {
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

  local_pu5[0xe] = 1;
  local_pu5[0xf] = 1;
  local_pu5[0x10] = 1;
  local_pu5[0x11] = 1;
  local_pu5[0x12] = 1;
  local_pu5[0x13] = 1;
  local_pu5[0x14] = 1;
  local_pu5[0x15] = 1;
  local_pu5[0x16] = 1;
  local_pu5[0x23] = 1;
  local_pu5[0x24] = 1;
  local_pu5[0x25] = 1;
  local_pu5[0x20] = 0xfd;
  local_pu5[0x21] = 0xcf;
  local_pu5[0x22] = 0x2c;
  local_pu5[0x26] = 10;
  local_pu5[6] = 0;
  local_pu5[8] = 0;
  local_pu5[7] = 0;
  local_pu5[9] = 0;
  local_pu5[10] = 0;
  *local_pu5 = 0;
  local_pu5[0xb] = 0;
  local_pu5[1] = 0;
  local_pu5[2] = 0;
  local_pu5[4] = 0;
  local_pu5[3] = 0;
  local_pu5[5] = 0;
  local_pu5[0xc] = 0;
  local_pu5[0x29] = 0;
  local_pu5[0x27] = 0xa0;
  local_pu5[0xd] = 0x3ff00000;
  local_pu5[0x28] = 100;
  local_i7 = -0x708;

  while( 1 ) {
    local_i8 = local_i7 + 1;
    local_f9 = (double)fsin((double)local_i7 * (double)g_Const_0_1 * (double)g_Const_DegToRad);
    if (0x189b < local_i8) break;
    *(float *)((int)pvVar6 + local_i8 * 4 + 0x1c1c) = (float)local_f9;
    local_i7 = local_i8;
  }

  *(float *)((int)pvVar6 + local_i8 * 4 + 0x1c1c) = (float)local_f9;
  g_LisaActiveTabSize = (int)pvVar6 + 0xe10;
  return local_pu5;
}

/**
 * @original Lisa_FreeEngineMemory (IGN_WIN.EXE @ 0x00446c30, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_FreeEngineMemory(void) {
  void *_Memory;
  int local_i1;
  int local_i2;
  int local_i3;

  

  local_i2 = 0;

  if (g_pLisaAllocatedBuffers != (void *)0x0) {
    _Memory = g_pLisaAllocatedBuffers;

    if (0 < g_LisaAllocatedBufferCount) {
      local_i3 = 0;
      local_i1 = g_LisaAllocatedBufferCount;

      do {
        if (*(void **)(local_i3 + (int)_Memory) != (void *)0x0) {
          _free(*(void **)(local_i3 + (int)_Memory));
          _Memory = g_pLisaAllocatedBuffers;
          local_i1 = g_LisaAllocatedBufferCount;
        }

        local_i3 = local_i3 + 4;
        local_i2 = local_i2 + 1;
      } while (local_i2 < local_i1);
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
LisaReturn64 Lisa_CreateDynamicObject(int arg_1, int object_id, LisaEntityTransform *entity, MshSubmesh *mesh, int arg_5, short arg_6, short arg_7, short base_elevation, short arg_9) {
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
  obj->unknown_08 = arg_5;
  obj->pos_x = entity->pos_x;
  obj->pos_y = entity->pos_y;
  obj->pos_z = entity->pos_z;
  obj->rot_x = (short)entity->rot_x;
  obj->rot_y = (short)entity->rot_y;
  obj->rot_z = (short)entity->rot_z;
  obj->unknown_1e = arg_6;
  obj->unknown_20 = arg_7;
  obj->unknown_22 = base_elevation;
  obj->unknown_24 = arg_9;
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
LisaReturn64 Lisa_SetDynamicObjectMesh(int arg_1, int arg_2, LisaEntityTransform *entity, MshSubmesh *mesh, int arg_5, short arg_6, short arg_7, short base_elevation, short arg_9) {
  LisaDynamicObject *obj;
  int max_dist;
  int i;
  int *vertices;
  float fx, fy, fz;
  float dist;
  unsigned int ret_val;

  obj = entity->dyn_obj;
  
  obj->unknown_1e = arg_6;
  obj->unknown_22 = base_elevation;
  obj->mesh_data = mesh;
  obj->unknown_08 = arg_5;
  obj->unknown_20 = arg_7;
  obj->unknown_24 = arg_9;
  
  ret_val = ((unsigned int)arg_2 >> 16) | ((unsigned int)base_elevation << 16);

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
  int local_i1;
  int local_l2;
  unsigned int local_u3;

  

  local_l2 = __ftol();
  local_i1 = g_LisaCamera;
  g_LisaCamera->viewport_x = (int)local_l2;
  local_l2 = __ftol();
  *(int *)(local_i1 + 0x84) = (int)local_l2;
  local_l2 = __ftol();
  *(int *)(local_i1 + 0x88) = (int)local_l2;
  local_l2 = __ftol();
  *(int *)(local_i1 + 0x9c) = (int)local_l2;
  local_u3 = __ftol();
  *(int *)(local_i1 + 0xa0) = (int)local_u3;
  { LisaReturn64 _r; _r.edx = local_u3; _r.eax = 0; return _r; }
}

/**
 * @original Lisa_GenerateMipmaps (IGN_WIN.EXE @ 0x00447280, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_GenerateMipmaps(unsigned int *arg_1,int arg_2,int arg_3,int arg_4,int arg_5,int arg_6 ,int arg_7,int arg_8,char *arg_9) {
  char local_u1;
  char *local_pu2;
  char *local_pu3;
  unsigned int *local_pu4;
  unsigned int *local_pu5;
  void *pvVar6;
  byte *pbVar7;
  int local_i8;
  int local_i9;
  int local_i10;
  int local_i11;
  byte *pbVar12;
  byte *pbVar13;
  unsigned int *local_pu14;
  int local_b15;
  int local_l16;
  byte *local_440;
  int local_438;
  byte *local_428;
  byte *local_424;
  int local_414;
  int local_40c;
  char local_400 [1024];

  

  local_pu2 = local_400;

  do {
    local_u1 = arg_9[1];
    *local_pu2 = *arg_9;
    local_pu2[1] = local_u1;
    local_pu3 = local_pu2 + 4;
    local_pu2[2] = arg_9[2];
    local_pu2 = local_pu3;
    arg_9 = arg_9 + 3;
  } while (local_pu3 < (unsigned int *)(local_400 + sizeof(local_400)));
  local_pu4 = calloc(0x15,4);
  g_LisaAllocatedBufferCount = g_LisaAllocatedBufferCount + 1;
  *(unsigned int **)(g_pLisaAllocatedBuffers + -4 + g_LisaAllocatedBufferCount * 4) = local_pu4;
  local_pu5 = local_pu4 + 5;
  local_l16 = __ftol();
  local_438 = 0;

  if (0 < (int)local_l16) {
    do {
      if (5999 < local_438) break;
      local_438 = local_438 + 1;
    } while (local_438 < (int)local_l16);
  }

  local_l16 = __ftol();
  local_i9 = (int)local_l16;
  local_440 = (byte *)*arg_1;
  *local_pu4 = (unsigned int)local_440;
  local_pu4[1] = (unsigned int)local_440;
  local_pu4[2] = (unsigned int)local_440;
  local_pu4[3] = (unsigned int)local_440;
  local_pu4[4] = (unsigned int)local_440;
  *local_pu5 = (unsigned int)local_440;
  local_i10 = 0;

  if (0 < arg_4 + -1) {
    local_i11 = local_i10;

    do {
      g_LisaEnableMipmaps = 1;
      g_LisaShadingEnabled = 1;
      pvVar6 = calloc(arg_7 * 0x100 + 0xffff,1);
      g_LisaAllocatedBufferCount = g_LisaAllocatedBufferCount + 1;
      *(void **)(g_pLisaAllocatedBuffers + -4 + g_LisaAllocatedBufferCount * 4) = pvVar6;
      pbVar7 = (byte *)((int)pvVar6 + 0xffffU & 0xffff0000);
      local_i8 = g_LisaActivePageCount + 2;
      g_LisaActivePageCount = local_i8;
      *(byte **)(&g_pLisaTexturePagePointers + local_i8 * 4) = pbVar7;
      local_i10 = local_i11 + 1;
      *(int *)(&g_LisaTexturePageSizes + local_i8 * 4) = arg_7 << 8;

      if (local_i10 < 5) {
        local_pu4[local_i11 + 1] = (unsigned int)pbVar7;
      }

      local_i8 = 0;

      if (0 < local_i9) {
        do {
          if (5999 < local_438) break;
          local_i8 = local_i8 + 1;
          local_438 = local_438 + 1;
        } while (local_i8 < local_i9);
      }

      if (0 < arg_7 / arg_6) {
        local_424 = local_440;
        local_i9 = (int)(0x100 / (int)arg_5);
        local_428 = pbVar7;
        local_40c = arg_7 / arg_6;

        do {
          pbVar12 = local_428;
          pbVar13 = local_424;
          local_414 = local_i9;

          if (0 < local_i9) {
            do {
              if (local_i11 == 0) {
                Lisa_DownsampleTextureMipmap(pbVar13,pbVar12,arg_5,arg_6,0x100,(int)local_400);
              }

              else {
                Lisa_FilterTextureBlock(pbVar13,(int)pbVar12,arg_5,arg_6,0x100,(int)local_400,arg_8,

                             local_i11);
              }

              local_414 = local_414 + -1;
              pbVar12 = pbVar12 + arg_5;
              pbVar13 = pbVar13 + arg_5;
            } while (local_414 != 0);
          }

          local_428 = local_428 + arg_6 * 0x100;
          local_424 = local_424 + arg_6 * 0x100;
          local_40c = local_40c + -1;
        } while (local_40c != 0);
      }

      local_l16 = __ftol();
      local_i9 = (int)local_l16;
      local_i11 = local_i10;
      local_440 = pbVar7;
    } while (local_i10 < arg_4 + -1);
  }

  if (local_i10 < 4) {
    local_pu14 = local_pu4 + local_i10 + 1;

    for (local_i9 = 4 - local_i10; local_i9 != 0; local_i9 = local_i9 + -1) {
      *local_pu14 = (unsigned int)local_440;
      local_pu14 = local_pu14 + 1;
    }

  }

  local_b15 = g_LisaDisableFiltering == 0;
  *arg_1 = (unsigned int)local_pu5;

  if ((local_b15) && (1 < arg_4)) {
    Lisa_GenerateTextureSpanTable(*local_pu4,(int)local_400,arg_8,arg_7,local_pu5);
  }

  return 0;
}

/**
 * @original Lisa_GenerateTextureSpanTable (IGN_WIN.EXE @ 0x004475c0, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_GenerateTextureSpanTable(int arg_1,int arg_2,int arg_3,int arg_4,int *arg_5) {
  unsigned int local_u1;
  int local_i2;
  byte local_b3;
  unsigned int local_u4;
  unsigned int local_u5;
  int local_i6;
  void *pvVar7;
  unsigned int *local_pu8;
  byte *pbVar9;
  unsigned int local_u10;
  byte local_b11;
  int *local_pu12;
  int local_i13;
  unsigned int local_u14;
  byte local_b15;
  unsigned int *local_pu16;
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
  local_i6 = 0x10000;
  local_pu12 = local_88;

  do {
    *local_pu12 = 0;
    local_pu12 = local_pu12 + 5;
    local_i6 = local_i6 + -1;
  } while (local_i6 != 0);
  local_98 = arg_5;
  local_c = arg_4 * 0x400 + 0xffff;
  local_94 = 0;
  local_8 = arg_4 << 10;

  do {
    pvVar7 = calloc(local_c,1);
    g_LisaAllocatedBufferCount = g_LisaAllocatedBufferCount + 1;
    *(void **)(g_pLisaAllocatedBuffers + -4 + g_LisaAllocatedBufferCount * 4) = pvVar7;
    local_pu8 = (unsigned int *)((int)pvVar7 + 0xffffU & 0xffff0000);
    local_i6 = g_LisaActivePageCount + 2;
    local_pu12 = local_98 + 4;
    g_LisaActivePageCount = local_i6;
    *(unsigned int **)(&g_pLisaTexturePagePointers + local_i6 * 4) = local_pu8;
    *(int *)(&g_LisaTexturePageSizes + local_i6 * 4) = local_8;
    *local_98 = local_pu8;
    local_8c = 0;
    local_98[1] = local_pu8;
    local_98[2] = local_pu8;
    local_98[3] = local_pu8;

    if (0 < arg_4) {
      local_4 = arg_4 + -1;
      local_90 = 0;
      local_4c = local_pu8;

      do {
        local_a8[0] = 0;
        local_a8[1] = 0;
        local_a8[2] = 0;
        local_b4 = 0;
        local_ac = local_4c;

        do {
          local_b3 = *(byte *)(local_94 + local_b4 + arg_1 + local_90);
          local_b8 = ((((unsigned int)((((unsigned int)(local_b8)) >> 8))) << 8) | ((unsigned char)(local_b3)));

          if (local_b4 < 0x3f) {
            pbVar9 = (byte *)(local_94 + local_b4 + arg_1 + local_90);
            local_b15 = pbVar9[1];
          }

          else {
            pbVar9 = (byte *)(local_94 + local_b4 + arg_1 + local_90);
            local_b15 = *pbVar9;
          }

          local_9c = ((((unsigned int)((((unsigned int)(local_9c)) >> 8))) << 8) | ((unsigned char)(local_b15)));

          if (local_8c < local_4) {
            local_b11 = pbVar9[0x100];

            if (local_b4 < 0x3f) {
              local_pu8 = (unsigned int *)(unsigned int)pbVar9[0x101];
            }

            else {
LAB_00447785:

              local_pu8 = (unsigned int *)(unsigned int)local_b11;
            }

          }

          else {
            local_b11 = *pbVar9;
            if (0x3e < local_b4) goto LAB_00447785;
            local_pu8 = (unsigned int *)(unsigned int)pbVar9[1];
          }

          local_u10 = (unsigned int)local_b11;
          local_u14 = (unsigned int)local_b15;
          local_u4 = (unsigned int)local_b3;
          local_u1 = (((int)local_pu8 * 0x100 + local_u10) * 0x100 + local_u14) * 0x100 + local_u4;
          local_pu16 = local_88 + ((local_u1 >> 0x11) + local_u10 + local_u14 + local_u1 & 0xffff) * 5;
          local_b0 = local_pu8;

          if (*local_pu16 != local_u1) {
            *local_pu16 = local_u1;
            local_b8 = (unsigned int)*(byte *)(arg_2 + local_u4 * 4);
            local_u1 = local_b8;
            local_30 = local_b8;
            local_b8 = (unsigned int)*(byte *)(arg_2 + 1 + local_u4 * 4);
            local_u5 = local_b8;
            local_2c = local_b8;
            local_b8 = (unsigned int)*(byte *)(arg_2 + 2 + local_u4 * 4);
            local_u4 = local_b8;
            local_28 = local_b8;
            local_3c = (unsigned int)*(byte *)(arg_2 + local_u14 * 4);
            local_38 = (unsigned int)*(byte *)(arg_2 + 1 + local_u14 * 4);
            local_34 = (unsigned int)*(byte *)(arg_2 + 2 + local_u14 * 4);
            local_18 = (unsigned int)*(byte *)(arg_2 + local_u10 * 4);
            local_14 = (unsigned int)*(byte *)(arg_2 + 1 + local_u10 * 4);
            local_10 = (unsigned int)*(byte *)(arg_2 + 2 + local_u10 * 4);
            local_24 = (unsigned int)*(byte *)(arg_2 + (int)local_pu8 * 4);
            local_20 = (unsigned int)*(byte *)(arg_2 + 1 + (int)local_pu8 * 4);
            local_b0 = local_ac;
            local_9c = 0;
            local_74 = 0;
            local_1c = (unsigned int)*(byte *)(arg_2 + 2 + (int)local_pu8 * 4);
            local_6c = 0;
            local_78 = local_34 << 2;
            local_64 = 0;
            local_70 = local_38 << 2;
            local_68 = local_3c << 2;
            local_5c = 0;
            local_54 = 0;
            local_60 = local_b8 << 2;
            local_b8 = 4;
            local_58 = local_u5 * 4;
            local_50 = local_u1 << 2;
            local_pu8 = local_pu16;

            do {
              local_i6 = (int)(local_54 + local_50 + (local_54 + local_50 >> 0x1f & 3U)) >> 2;
              local_44 = (int)(local_5c + local_58 + (local_5c + local_58 >> 0x1f & 3U)) >> 2;
              local_40 = (int)(local_64 + local_60 + (local_64 + local_60 >> 0x1f & 3U)) >> 2;
              local_84 = (int)(local_6c + local_68 + (local_6c + local_68 >> 0x1f & 3U)) >> 2;
              local_80 = (int)(local_74 + local_70 + (local_74 + local_70 >> 0x1f & 3U)) >> 2;
              local_7c = (int)(local_9c + local_78 + (local_9c + local_78 >> 0x1f & 3U)) >> 2;
              local_c4[0] = local_i6 * 4;
              local_i13 = 0;
              local_c4[1] = local_44 << 2;
              local_c4[2] = local_40 << 2;

              do {
                local_i2 = *(int *)((int)local_a8 + local_i13) + 8 + *(int *)((int)local_c4 + local_i13);
                *(int *)((int)local_c4 + local_i13) = local_i2;

                if (0x3ff < local_i2) {
                  *(int *)((int)local_c4 + local_i13) = 0x3ff;
                }

                if (*(int *)((int)local_c4 + local_i13) < 0) {
                  *(int *)((int)local_c4 + local_i13) = 0;
                }

                local_i13 = local_i13 + 4;
              } while (local_i13 < 0xc);

              local_b3 = (&g_LisaObjectMatrix_22)

                      [(local_c4[1] & 0x3f0) * 4 +

                       ((local_c4[2] & 0x3f0) >> 4) + (local_c4[0] & 0x3f0) * 0x100];
              local_u14 = (unsigned int)local_b3;
              *(byte *)(local_pu8 + 1) = local_b3;
              local_b15 = *(byte *)(arg_2 + local_u14 * 4);
              *(byte *)local_b0 = local_b3;
              local_a8[0] = (int)(local_c4[0] + (unsigned int)local_b15 * -4) / 2;
              local_a8[1] = (int)(local_c4[1] + (unsigned int)*(byte *)(arg_2 + 1 + local_u14 * 4) * -4) / 2;
              local_a8[2] = (int)(local_c4[2] + (unsigned int)*(byte *)(arg_2 + 2 + local_u14 * 4) * -4) / 2;
              local_c4[0] = local_i6 * 3 + local_84;
              local_c4[1] = local_44 * 3 + local_80;
              local_i13 = 0;
              local_c4[2] = local_40 * 3 + local_7c;

              do {
                local_i2 = *(int *)((int)local_a8 + local_i13) + 8 + *(int *)((int)local_c4 + local_i13);
                *(int *)((int)local_c4 + local_i13) = local_i2;

                if (0x3ff < local_i2) {
                  *(int *)((int)local_c4 + local_i13) = 0x3ff;
                }

                if (*(int *)((int)local_c4 + local_i13) < 0) {
                  *(int *)((int)local_c4 + local_i13) = 0;
                }

                local_i13 = local_i13 + 4;
              } while (local_i13 < 0xc);

              local_u14 = (unsigned int)(byte)(&g_LisaObjectMatrix_22)

                                   [(local_c4[1] & 0x3f0) * 4 +

                                    ((local_c4[2] & 0x3f0) >> 4) + (local_c4[0] & 0x3f0) * 0x100];

              *(char *)((int)local_pu8 + 5) =

                   (&g_LisaObjectMatrix_22)

                   [(local_c4[1] & 0x3f0) * 4 +

                    ((local_c4[2] & 0x3f0) >> 4) + (local_c4[0] & 0x3f0) * 0x100];
              local_a8[0] = (int)(local_c4[0] + (unsigned int)*(byte *)(arg_2 + local_u14 * 4) * -4) / 2;
              local_a8[1] = (int)(local_c4[1] + (unsigned int)*(byte *)(arg_2 + 1 + local_u14 * 4) * -4) / 2;
              local_a8[2] = (int)(local_c4[2] + (unsigned int)*(byte *)(arg_2 + 2 + local_u14 * 4) * -4) / 2;
              local_c4[0] = (local_84 + local_i6) * 2;
              local_c4[1] = (local_80 + local_44) * 2;
              local_i13 = 0;
              local_c4[2] = (local_7c + local_40) * 2;

              do {
                local_i2 = *(int *)((int)local_a8 + local_i13) + 8 + *(int *)((int)local_c4 + local_i13);
                *(int *)((int)local_c4 + local_i13) = local_i2;

                if (0x3ff < local_i2) {
                  *(int *)((int)local_c4 + local_i13) = 0x3ff;
                }

                if (*(int *)((int)local_c4 + local_i13) < 0) {
                  *(int *)((int)local_c4 + local_i13) = 0;
                }

                local_i13 = local_i13 + 4;
              } while (local_i13 < 0xc);

              local_u14 = (unsigned int)(byte)(&g_LisaObjectMatrix_22)

                                   [(local_c4[1] & 0x3f0) * 4 +

                                    ((local_c4[2] & 0x3f0) >> 4) + (local_c4[0] & 0x3f0) * 0x100];

              *(char *)((int)local_pu8 + 6) =

                   (&g_LisaObjectMatrix_22)

                   [(local_c4[1] & 0x3f0) * 4 +

                    ((local_c4[2] & 0x3f0) >> 4) + (local_c4[0] & 0x3f0) * 0x100];
              local_a8[0] = (int)(local_c4[0] + (unsigned int)*(byte *)(arg_2 + local_u14 * 4) * -4) / 2;
              local_a8[1] = (int)(local_c4[1] + (unsigned int)*(byte *)(arg_2 + 1 + local_u14 * 4) * -4) / 2;
              local_a8[2] = (int)(local_c4[2] + (unsigned int)*(byte *)(arg_2 + 2 + local_u14 * 4) * -4) / 2;
              local_c4[0] = local_84 * 3 + local_i6;
              local_i6 = 0;
              local_c4[1] = local_80 * 3 + local_44;
              local_c4[2] = local_7c * 3 + local_40;

              do {
                local_i13 = *(int *)((int)local_a8 + local_i6) + 8 + *(int *)((int)local_c4 + local_i6);
                *(int *)((int)local_c4 + local_i6) = local_i13;

                if (0x3ff < local_i13) {
                  *(int *)((int)local_c4 + local_i6) = 0x3ff;
                }

                if (*(int *)((int)local_c4 + local_i6) < 0) {
                  *(int *)((int)local_c4 + local_i6) = 0;
                }

                local_i6 = local_i6 + 4;
              } while (local_i6 < 0xc);

              local_u14 = (unsigned int)(byte)(&g_LisaObjectMatrix_22)

                                   [(local_c4[1] & 0x3f0) * 4 +

                                    ((local_c4[2] & 0x3f0) >> 4) + (local_c4[0] & 0x3f0) * 0x100];

              *(char *)((int)local_pu8 + 7) =

                   (&g_LisaObjectMatrix_22)

                   [(local_c4[1] & 0x3f0) * 4 +

                    ((local_c4[2] & 0x3f0) >> 4) + (local_c4[0] & 0x3f0) * 0x100];
              local_a8[0] = (int)(local_c4[0] + (unsigned int)*(byte *)(arg_2 + local_u14 * 4) * -4) / 2;
              local_a8[1] = (int)(local_c4[1] + (unsigned int)*(byte *)(arg_2 + 1 + local_u14 * 4) * -4) / 2;
              local_b0 = local_b0 + 0x40;
              local_a8[2] = (int)(local_c4[2] + (unsigned int)*(byte *)(arg_2 + 2 + local_u14 * 4) * -4) / 2;
              local_9c = local_9c + local_1c;
              local_78 = local_78 - local_34;
              local_74 = local_74 + local_20;
              local_70 = local_70 - local_38;
              local_6c = local_6c + local_24;
              local_68 = local_68 - local_3c;
              local_64 = local_64 + local_10;
              local_60 = local_60 - local_u4;
              local_5c = local_5c + local_14;
              local_58 = local_58 - local_u5;
              local_54 = local_54 + local_18;
              local_50 = local_50 - local_u1;
              local_b8 = local_b8 + -1;
              local_pu8 = local_pu8 + 1;
            } while (local_b8 != 0);
            local_a8[0] = 0;
            local_a8[1] = 0;
            local_a8[2] = 0;
            local_b8 = 0;
          }

          local_a8[2] = 0;
          local_a8[1] = 0;
          local_a8[0] = 0;
          local_u1 = local_pu16[2];
          local_pu8 = local_ac + 1;
          local_b4 = local_b4 + 1;
          *local_ac = local_pu16[1];
          local_u14 = local_pu16[3];
          local_ac[0x40] = local_u1;
          local_u1 = local_pu16[4];
          local_ac[0x80] = local_u14;
          local_ac[0xc0] = local_u1;
          local_ac = local_pu8;
        } while (local_b4 < 0x40);
        local_90 = local_90 + 0x100;
        local_4c = local_4c + 0x100;
        local_8c = local_8c + 1;
      } while (local_8c < arg_4);
    }

    local_94 = local_94 + 0x40;
    local_98 = local_pu12;

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
void Lisa_DownsampleTextureMipmap(byte *arg_1,byte *arg_2,int arg_3,int arg_4,int arg_5,int arg_6) {
  int local_i1;
  byte local_b2;
  int local_i3;
  unsigned int *local_pu4;
  unsigned int local_u5;
  unsigned int local_u6;
  int local_i7;
  int local_i8;
  int local_i9;
  int local_2c;
  int local_24;
  unsigned int local_1c [4];
  int local_c [3];

  

  local_24 = 0;

  if (0 < arg_4) {
    do {
      local_2c = 0;
      local_c[0] = 0;
      local_c[1] = 0;
      local_c[2] = 0;

      if (0 < arg_3) {
        local_i3 = arg_3 + -1;

        do {
          local_i7 = 0;
          local_pu4 = local_1c;
          local_b2 = *arg_1;

          do {
            local_u6 = (unsigned int)*(byte *)((unsigned int)local_b2 * 4 + local_i7 + arg_6);
            *local_pu4 = local_u6;

            if (local_2c < local_i3) {
              local_u5 = *(byte *)(local_i7 + (unsigned int)arg_1[1] * 4 + arg_6) + local_u6;
            }

            else {
              local_u5 = local_u6 * 2;
            }

            *local_pu4 = local_u5;

            if (local_24 < arg_4 + -1) {
              local_u6 = (unsigned int)*(byte *)(local_i7 + (unsigned int)arg_1[arg_5] * 4 + arg_6);
              local_u5 = *local_pu4 + local_u6;
              *local_pu4 = local_u5;

              if (local_2c < local_i3) {
                *local_pu4 = *(byte *)(local_i7 + (unsigned int)arg_1[arg_5 + 1] * 4 + arg_6) + local_u5;
              }

              else {
                *local_pu4 = local_u6 + local_u5;
              }

            }

            else {
              local_u5 = *local_pu4 + local_u6;
              *local_pu4 = local_u5;

              if (local_2c < local_i3) {
                *local_pu4 = *(byte *)(local_i7 + (unsigned int)arg_1[1] * 4 + arg_6) + local_u5;
              }

              else {
                *local_pu4 = local_u5 + local_u6;
              }

            }

            local_pu4 = local_pu4 + 1;
            local_i7 = local_i7 + 1;
          } while (local_pu4 < local_1c + 3);
          local_i7 = 0;

          do {
            local_i9 = *(int *)((int)local_c + local_i7) + 8 + *(int *)((int)local_1c + local_i7);
            *(int *)((int)local_1c + local_i7) = local_i9;

            if (0x3ff < local_i9) {
              *(int *)((int)local_1c + local_i7) = 0x3ff;
            }

            if (*(int *)((int)local_1c + local_i7) < 0) {
              *(int *)((int)local_1c + local_i7) = 0;
            }

            local_i7 = local_i7 + 4;
          } while (local_i7 < 0xc);
          local_i9 = 0;

          local_b2 = (&g_LisaObjectMatrix_22)

                  [(local_1c[1] & 0x3f0) * 4 +

                   ((local_1c[2] & 0x3f0) >> 4) + (local_1c[0] & 0x3f0) * 0x100];
          *arg_2 = local_b2;
          local_i7 = 0;

          do {
            local_i8 = local_i7 + 4;
            local_i1 = local_i9 + (unsigned int)local_b2 * 4;
            local_i9 = local_i9 + 1;

            *(int *)((int)local_c + local_i7) =

                 (int)(*(int *)((int)local_1c + local_i7) + (unsigned int)*(byte *)(local_i1 + arg_6) * -4) / 2;
            local_i7 = local_i8;
          } while (local_i8 < 0xc);
          arg_2 = arg_2 + 1;
          arg_1 = arg_1 + 1;
          local_2c = local_2c + 1;
        } while (local_2c < arg_3);
      }

      arg_1 = arg_1 + (arg_5 - arg_3);
      local_24 = local_24 + 1;
      arg_2 = arg_2 + (arg_5 - arg_3);
    } while (local_24 < arg_4);
  }

  return;
}

/**
 * @original Lisa_FilterTextureBlock (IGN_WIN.EXE @ 0x004481f0, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_FilterTextureBlock(byte *arg_1,int arg_2,int arg_3,int arg_4,int arg_5,int arg_6, int arg_7,int arg_8) {
  byte local_b1;
  unsigned int local_u2;
  int local_i3;
  unsigned int *local_pu4;
  int local_i5;
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

  if (0 < arg_4) {
    local_40 = arg_8 * arg_5;
    local_3c = arg_1 + local_40;
    local_44 = arg_1 + local_40 + arg_3 + -1;
    local_38 = (arg_5 + -1) * arg_8;
    local_34 = arg_1 + (arg_3 - local_40) + -1;
    local_30 = -local_40;
    local_2c = arg_1 + -local_40;
    local_28 = (-1 - arg_5) * arg_8;

    do {
      local_54 = 0;
      local_c[0] = 0;
      local_c[1] = 0;
      local_c[2] = 0;

      if (0 < arg_3) {
        do {
          local_i3 = 0;
          local_pu4 = local_24;

          do {
            if (local_4c - arg_8 < 1) {
              if (local_54 - arg_8 < 1) {
                local_b1 = *arg_1;
              }

              else {
                local_b1 = arg_1[local_54 - arg_8];
              }

              *local_pu4 = (unsigned int)*(byte *)(local_i3 + (unsigned int)local_b1 * 4 + arg_6);

              if (local_54 + arg_8 < arg_3) {
                local_u2 = (unsigned int)arg_1[local_54 + arg_8];
              }

              else {
                local_u2 = (unsigned int)arg_1[arg_3 + -1];
              }

            }

            else {
              if (local_54 == arg_8 || local_54 - arg_8 < 0) {
                *local_pu4 = (unsigned int)*(byte *)(local_i3 + (unsigned int)*local_2c * 4 + arg_6);
              }

              else {
                *local_pu4 = (unsigned int)*(byte *)(local_i3 + (unsigned int)arg_1[local_28 + local_54] * 4 + arg_6);
              }

              if (arg_8 + local_54 < arg_3) {
                local_u2 = (unsigned int)arg_1[local_30 + arg_8 + local_54];
              }

              else {
                local_u2 = (unsigned int)*local_34;
              }

            }

            *local_pu4 = *local_pu4 + (unsigned int)*(byte *)(local_i3 + local_u2 * 4 + arg_6);

            if (local_4c + arg_8 < arg_4) {
              if (local_54 == arg_8 || local_54 - arg_8 < 0) {
                local_b1 = *local_3c;
              }

              else {
                local_b1 = arg_1[local_38 + local_54];
              }

              *local_pu4 = *local_pu4 + (unsigned int)*(byte *)(local_i3 + (unsigned int)local_b1 * 4 + arg_6);

              if (arg_8 + local_54 < arg_3) {
                local_u2 = (unsigned int)arg_1[local_40 + arg_8 + local_54];
                goto LAB_004484c0;
              }

              *local_pu4 = *local_pu4 + (unsigned int)*(byte *)(local_i3 + (unsigned int)*local_44 * 4 + arg_6);
            }

            else {
              if (local_54 == arg_8 || local_54 - arg_8 < 0) {
                local_i5 = (arg_4 + -1) * arg_5;
                local_b1 = *(byte *)(local_i3 + (unsigned int)arg_1[local_i5] * 4 + arg_6);
              }

              else {
                local_i5 = (arg_4 + -1) * arg_5;

                local_b1 = *(byte *)(local_i3 + (unsigned int)arg_1[local_i5 + (local_54 - arg_8)] * 4 + arg_6)

                ;
              }

              *local_pu4 = *local_pu4 + (unsigned int)local_b1;

              if (arg_8 + local_54 < arg_3) {
                local_u2 = (unsigned int)arg_1[arg_8 + local_54];
              }

              else {
                local_u2 = (unsigned int)arg_1[local_i5 + arg_3 + -1];
              }

LAB_004484c0:

              *local_pu4 = *local_pu4 + (unsigned int)*(byte *)(local_i3 + local_u2 * 4 + arg_6);
            }

            local_pu4 = local_pu4 + 1;
            local_i3 = local_i3 + 1;
          } while (local_pu4 < local_24 + 3);
          local_i3 = 0;

          do {
            local_i5 = *(int *)((int)local_c + local_i3) + 8 + *(int *)((int)local_24 + local_i3);
            *(int *)((int)local_24 + local_i3) = local_i5;

            if (0x3ff < local_i5) {
              *(int *)((int)local_24 + local_i3) = 0x3ff;
            }

            if (*(int *)((int)local_24 + local_i3) < 0) {
              *(int *)((int)local_24 + local_i3) = 0;
            }

            local_i3 = local_i3 + 4;
          } while (local_i3 < 0xc);
          local_48 = 0;

          local_b1 = (&g_LisaObjectMatrix_22)

                  [(local_24[0] & 0x3f0) * 0x100 +

                   ((local_24[2] & 0x3f0) >> 4) + (local_24[1] & 0x3f0) * 4];
          *(byte *)(arg_2 + local_54) = local_b1;
          local_i3 = 0;

          do {
            local_i5 = local_i3 + 4;

            *(int *)((int)local_c + local_i3) =

                 (int)(*(int *)((int)local_24 + local_i3) +

                      (unsigned int)*(byte *)(local_48 + arg_6 + (unsigned int)local_b1 * 4) * -4) / 2;
            local_48 = local_48 + 1;
            local_i3 = local_i5;
          } while (local_i5 < 0xc);
          local_54 = local_54 + 1;
        } while (local_54 < arg_3);
      }

      arg_2 = arg_2 + arg_5;
      local_44 = local_44 + arg_5;
      local_40 = local_40 + arg_5;
      local_3c = local_3c + arg_5;
      local_38 = local_38 + arg_5;
      local_34 = local_34 + arg_5;
      local_30 = local_30 + arg_5;
      local_2c = local_2c + arg_5;
      local_28 = local_28 + arg_5;
      local_4c = local_4c + 1;
    } while (local_4c < arg_4);
  }

  return;
}

/**
 * @original Lisa_LoadOrCreateShadingTable (IGN_WIN.EXE @ 0x00448620, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_LoadOrCreateShadingTable(int arg_1,int arg_2) {
  byte *pbVar1;
  char cVar2;
  unsigned int local_u3;
  int local_i4;
  FILE *pFVar5;
  unsigned int local_u6;
  int local_i7;
  unsigned int local_u8;
  int local_i9;
  size_t sVar10;
  char *local_pc11;
  int local_i12;
  unsigned int *local_pu13;
  unsigned int *local_pu14;
  char *local_pc15;
  char *local_pc16;
  int local_i17;
  unsigned int local_48;
  int local_38;
  unsigned int local_34 [5];
  char local_20 [32];

  

  local_u8 = 0;
  local_i4 = 0;

  do {
    pbVar1 = (byte *)(arg_2 + local_i4);
    local_i4 = local_i4 + 1;
    local_i9 = local_u8 + *pbVar1;
    local_u6 = local_i9 * 2;
    local_u3 = (unsigned int)(local_i9 < 0);
    local_u8 = local_u6 | local_u3;
  } while (local_i4 < 0x300);
  sVar10 = 0;
  local_34[0] = 0xffffffff;
  local_20[0] = '\0';
  local_pc11 = &s_pal_chk_str2;

  do {
    local_pc16 = local_pc11;
    if (local_34[0] == 0) break;
    local_34[0] = local_34[0] - 1;
    local_pc16 = local_pc11 + 1;
    cVar2 = *local_pc11;
    local_pc11 = local_pc16;
  } while (cVar2 != '\0');
  local_34[0] = ~local_34[0];
  local_i4 = -1;
  local_pc11 = local_20;

  do {
    local_pc15 = local_pc11;
    if (local_i4 == 0) break;
    local_i4 = local_i4 + -1;
    local_pc15 = local_pc11 + 1;
    cVar2 = *local_pc11;
    local_pc11 = local_pc15;
  } while (cVar2 != '\0');
  local_pc11 = local_pc16 + -local_34[0];
  local_pc16 = local_pc15 + -1;

  for (local_u8 = local_34[0] >> 2; local_u8 != 0; local_u8 = local_u8 - 1) {
    *(int *)local_pc16 = *(int *)local_pc11;
    local_pc11 = local_pc11 + 4;
    local_pc16 = local_pc16 + 4;
  }

  for (local_u8 = local_34[0] & 3; local_u8 != 0; local_u8 = local_u8 - 1) {
    *local_pc16 = *local_pc11;
    local_pc11 = local_pc11 + 1;
    local_pc16 = local_pc16 + 1;
  }

  __ultoa(local_u6 & 0xffff | local_u3,(char *)local_34,0x10);
  local_u8 = 0xffffffff;
  local_pu13 = local_34;

  do {
    local_pu14 = local_pu13;
    if (local_u8 == 0) break;
    local_u8 = local_u8 - 1;
    local_pu14 = (unsigned int *)((int)local_pu13 + 1);
    local_u6 = *local_pu13;
    local_pu13 = local_pu14;
  } while ((char)local_u6 != '\0');
  local_u8 = ~local_u8;
  local_i4 = -1;
  local_pc11 = local_20;

  do {
    local_pc16 = local_pc11;
    if (local_i4 == 0) break;
    local_i4 = local_i4 + -1;
    local_pc16 = local_pc11 + 1;
    cVar2 = *local_pc11;
    local_pc11 = local_pc16;
  } while (cVar2 != '\0');
  local_pc11 = (char *)((int)local_pu14 - local_u8);
  local_pc16 = local_pc16 + -1;

  for (local_u6 = local_u8 >> 2; local_u6 != 0; local_u6 = local_u6 - 1) {
    *(int *)local_pc16 = *(int *)local_pc11;
    local_pc11 = local_pc11 + 4;
    local_pc16 = local_pc16 + 4;
  }

  for (local_u8 = local_u8 & 3; local_u8 != 0; local_u8 = local_u8 - 1) {
    *local_pc16 = *local_pc11;
    local_pc11 = local_pc11 + 1;
    local_pc16 = local_pc16 + 1;
  }

  local_u8 = 0xffffffff;
  local_pc11 = (char *)&s_pal_chk_str1;

  do {
    local_pc16 = local_pc11;
    if (local_u8 == 0) break;
    local_u8 = local_u8 - 1;
    local_pc16 = local_pc11 + 1;
    cVar2 = *local_pc11;
    local_pc11 = local_pc16;
  } while (cVar2 != '\0');
  local_u8 = ~local_u8;
  local_i4 = -1;
  local_pc11 = local_20;

  do {
    local_pc15 = local_pc11;
    if (local_i4 == 0) break;
    local_i4 = local_i4 + -1;
    local_pc15 = local_pc11 + 1;
    cVar2 = *local_pc11;
    local_pc11 = local_pc15;
  } while (cVar2 != '\0');
  local_pc11 = local_pc16 + -local_u8;
  local_pc16 = local_pc15 + -1;

  for (local_u6 = local_u8 >> 2; local_u6 != 0; local_u6 = local_u6 - 1) {
    *(int *)local_pc16 = *(int *)local_pc11;
    local_pc11 = local_pc11 + 4;
    local_pc16 = local_pc16 + 4;
  }

  for (local_u8 = local_u8 & 3; local_u8 != 0; local_u8 = local_u8 - 1) {
    *local_pc16 = *local_pc11;
    local_pc11 = local_pc11 + 1;
    local_pc16 = local_pc16 + 1;
  }

  local_u8 = 0xffffffff;
  local_pc11 = (char *)&s_pal_checksum_fmt;

  do {
    local_pc16 = local_pc11;
    if (local_u8 == 0) break;
    local_u8 = local_u8 - 1;
    local_pc16 = local_pc11 + 1;
    cVar2 = *local_pc11;
    local_pc11 = local_pc16;
  } while (cVar2 != '\0');
  local_u8 = ~local_u8;
  local_i4 = -1;
  local_pc11 = local_20;

  do {
    local_pc15 = local_pc11;
    if (local_i4 == 0) break;
    local_i4 = local_i4 + -1;
    local_pc15 = local_pc11 + 1;
    cVar2 = *local_pc11;
    local_pc11 = local_pc15;
  } while (cVar2 != '\0');
  local_pc11 = local_pc16 + -local_u8;
  local_pc16 = local_pc15 + -1;

  for (local_u6 = local_u8 >> 2; local_u6 != 0; local_u6 = local_u6 - 1) {
    *(int *)local_pc16 = *(int *)local_pc11;
    local_pc11 = local_pc11 + 4;
    local_pc16 = local_pc16 + 4;
  }

  for (local_u8 = local_u8 & 3; local_u8 != 0; local_u8 = local_u8 - 1) {
    *local_pc16 = *local_pc11;
    local_pc11 = local_pc11 + 1;
    local_pc16 = local_pc16 + 1;
  }

  pFVar5 = (FILE *)fopen(local_20,(const char *)&s_rb);

  if (pFVar5 != (FILE *)0x0) {
    sVar10 = _fread(&g_LisaObjectMatrix_22,1,0x40000,pFVar5);
    _fclose(pFVar5);
  }

  if (sVar10 != 0x40000) {
    local_48 = 0;

    do {
      local_i7 = 0;
      local_i4 = 0x7f000000;
      local_i9 = 0;

      do {
        local_i12 = ((local_48 & 0x3f000) >> 10) - (unsigned int)*(byte *)(local_i7 + arg_2);
        local_i12 = local_i12 * local_i12;

        if (((local_i12 < local_i4) &&

            (local_i17 = ((local_48 & 0xfc0) >> 4) - (unsigned int)*(byte *)(local_i7 + 1 + arg_2),

            local_i12 = local_i12 + local_i17 * local_i17, local_i12 < local_i4)) &&

           (local_i17 = (local_48 & 0x3f) * 4 - (unsigned int)*(byte *)(local_i7 + 2 + arg_2),

           local_i12 = local_i12 + local_i17 * local_i17, local_i12 < local_i4)) {
          local_i4 = local_i12;
          local_38 = local_i9;
        }

        local_i7 = local_i7 + 3;
        local_i9 = local_i9 + 1;
      } while (local_i9 < 0x100);
      local_u8 = local_48 + 1;
      ((int*)&(g_LisaObjectMatrix_22))[local_48] = (char)local_38;
      local_48 = local_u8;
    } while ((int)local_u8 < 0x40000);
    pFVar5 = (FILE *)fopen(local_20,(const char *)&s_tab_tab);

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
unsigned int Lisa_FindClosestPaletteColor(int *arg_1,int arg_2) {
  unsigned int local_u1;
  int local_i2;
  int local_i3;
  unsigned int local_c;
  int local_8;
  unsigned int local_4;

  

  local_8 = 0x7f000000;
  local_i2 = 0;
  local_4 = 0;
  local_u1 = local_c;

  do {
    local_i3 = *arg_1 - (unsigned int)*(byte *)(arg_2 + local_i2);
    local_i3 = local_i3 * local_i3;

    if (local_i3 < local_8) {
      local_c = (unsigned int)*(byte *)(arg_2 + 1 + local_i2);
      local_i3 = local_i3 + (arg_1[1] - local_c) * (arg_1[1] - local_c);

      if (local_i3 < local_8) {
        local_c = (unsigned int)*(byte *)(arg_2 + 2 + local_i2);
        local_i3 = local_i3 + (arg_1[2] - local_c) * (arg_1[2] - local_c);

        if (local_i3 < local_8) {
          local_u1 = local_4;
          local_8 = local_i3;
        }

      }

    }

    local_i2 = local_i2 + 3;
    local_4 = local_4 + 1;
  } while ((int)local_4 < 0x100);
  return local_u1;
}

/**
 * @original Lisa_RenderSkyBackdrop (IGN_WIN.EXE @ 0x00448990, lisa3d.c)
 * @fidelity ADAPTED
 */
LisaReturn64 Lisa_RenderSkyBackdrop(void) {
  float local_f1;
  int local_i2;
  int local_i3;
  int local_i4;
  int local_i5;
  unsigned int local_u6;
  double extraout_ST0;
  double extraout_ST1;
  int local_l7;
  unsigned int local_u8;

  

  local_i2 = g_LisaCamera;
  local_l7 = __ftol();
  local_i3 = (int)local_l7;
  local_l7 = __ftol();
  local_i4 = (int)local_l7;
  local_l7 = __ftol();
  local_u6 = (unsigned int)((unsigned int)local_l7 >> 0);
  local_i5 = (int)local_l7;
  local_f1 = (float)SQRT(extraout_ST1 * extraout_ST1 + extraout_ST0 * extraout_ST0);

  if ((local_i4 < 1) || ((int)local_f1 < 1)) {
    if ((local_i4 < 0) && (0 < (int)local_f1)) {
      fpatan((double)local_f1 / (double)-local_i4,(double)1);
      local_l7 = __ftol();
      *(double *)(local_i2 + 0x18) = (double)(int)local_l7;
    }

    else if (local_i4 == 0) {
      *(int *)(local_i2 + 0x18) = 0;
      *(int *)(local_i2 + 0x1c) = 0;
      local_l7 = (unsigned int)local_u6 << 0;
    }

    else if ((local_i4 == 0) && (ABS(local_f1) == 0.0)) {
      *(int *)(local_i2 + 0x18) = 0;
      *(int *)(local_i2 + 0x1c) = 0;
      local_l7 = (unsigned int)local_u6 << 0;
    }

    else if ((local_i4 < 0) && (ABS(local_f1) == 0.0)) {
      *(int *)(local_i2 + 0x18) = 0;
      *(int *)(local_i2 + 0x1c) = 0x40a51800;
      local_l7 = (((int)(local_u6) << 0) | ((unsigned int)(local_f1)));
    }

    else {
      local_l7 = (unsigned int)local_u6 << 0;

      if (0 < local_i4) {
        local_l7 = (((int)(local_u6) << 0) | ((unsigned int)(local_f1)));

        if (ABS(local_f1) == 0.0) {
          *(int *)(local_i2 + 0x18) = 0;
          *(int *)(local_i2 + 0x1c) = 0x408c2000;
          local_l7 = (((int)(local_u6) << 0) | ((unsigned int)(local_f1)));
        }

      }

    }

  }

  else {
    fpatan((double)local_i4 / (double)local_f1,(double)1);
    local_l7 = __ftol();
    *(double *)(local_i2 + 0x18) = (double)(int)local_l7;
  }

  local_u6 = (unsigned int)((unsigned int)local_l7 >> 0);

  if ((local_i3 < 1) || (local_i5 < 1)) {
    if ((local_i5 < 0) && (0 < local_i3)) {
      fpatan((double)-local_i5 / (double)local_i3,(double)1);
      local_u8 = __ftol();
      *(double *)(local_i2 + 0x20) = (double)(int)local_u8;
    }

    else if ((local_i3 < 0) && (local_i5 < 0)) {
      fpatan((double)local_i3 / (double)local_i5,(double)1);
      local_u8 = __ftol();
      *(double *)(local_i2 + 0x20) = (double)(int)local_u8;
    }

    else if ((local_i5 < 1) || (-1 < local_i3)) {
      if (local_i3 == 0) {
        if (0 < local_i5) {
          *(int *)(local_i2 + 0x20) = 0;
          *(int *)(local_i2 + 0x24) = 0;
          local_u8 = (unsigned int)local_u6 << 0;
          goto LAB_00448c11;
        }

        if (local_i5 < 0) {
          *(int *)(local_i2 + 0x20) = 0;
          *(int *)(local_i2 + 0x24) = 0x409c2000;
          local_u8 = (unsigned int)local_u6 << 0;
          goto LAB_00448c11;
        }

      }

      if ((local_i5 == 0) && (0 < local_i3)) {
        *(int *)(local_i2 + 0x20) = 0;
        *(int *)(local_i2 + 0x24) = 0x408c2000;
        local_u8 = (unsigned int)local_u6 << 0;
      }

      else {
        local_u8 = (unsigned int)local_u6 << 0;

        if ((local_i5 == 0) && (local_u8 = (unsigned int)local_u6 << 0, local_i3 < 0)) {
          *(int *)(local_i2 + 0x20) = 0;
          *(int *)(local_i2 + 0x24) = 0x40a51800;
          local_u8 = (unsigned int)local_u6 << 0;
        }

      }

    }

    else {
      fpatan((double)local_i5 / (double)-local_i3,(double)1);
      local_u8 = __ftol();
      *(double *)(local_i2 + 0x20) = (double)(int)local_u8;
    }

  }

  else {
    fpatan((double)local_i3 / (double)local_i5,(double)1);
    local_u8 = __ftol();
    *(double *)(local_i2 + 0x20) = (double)(int)local_u8;
  }

LAB_00448c11:

  *(double *)(local_i2 + 0x28) = (double)*(int *)(local_i2 + 0x7c);
  return local_u8 & 0xffffffff00000000;
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
    double extraout_ST1;

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
        fsin(extraout_ST1);

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
            
            // Matrix multiply: rot_matrix[6, 7, 8] are local_4, local_8, local_c
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
  float local_f1;
  float local_f2;
  float local_f3;
  float local_f4;
  float local_f5;
  float local_f6;
  float local_f7;
  float local_f8;
  int local_i9;
  int *local_pi10;
  int *local_pi11;
  unsigned int local_u12;
  unsigned int local_u13;
  int *local_pi14;
  int *local_pi15;
  float *pfVar16;
  float *pfVar17;
  int local_i18;
  int local_i19;
  int *local_pi20;
  int local_i21;
  int local_i22;
  int *local_pu23;
  double local_f24;
  double local_f25;
  double local_f26;
  double local_f27;
  double local_f28;
  double local_f29;
  double extraout_ST1;
  int local_l30;
  int local_l31;
  int local_l32;
  int local_70;
  int *local_6c;
  int local_68;
  int *local_64;
  double *local_58;
  float local_50 [19];
  float local_4;

  

  local_i22 = g_LisaCamera;
  local_i19 = g_LisaCamera->projection_type;

  if (local_i19 == 0) {
    g_LisaCamera->visible_obj_count = 0;
    local_pi15 = (int *)(local_i22 + 0x60);
    local_58 = &g_LisaCamera->rot_y;
    local_l30 = __ftol();
    local_l31 = __ftol();
    local_i19 = ((int)local_l30 * 0x24 + (int)local_l31) * 8;
    local_l30 = __ftol();

    local_i9 = (*(int *)(&g_LisaDefaultScale_Y + local_i19) +

             ((int)((int)local_l30 + ((int)local_l30 >> 0x1f & 0xffU)) >> 8) + g_LisaDefaultOffset_X) *

            g_LisaGridCellsX;
    local_l30 = __ftol();
    local_i18 = g_pLisaAllocatedBuffersEnd;
    local_pi20 = g_LisaVisibleObjects;
    local_70 = 3;

    local_i19 = (int)g_pLisaGridCells +

             (local_i9 + ((int)((int)local_l30 + ((int)local_l30 >> 0x1f & 0xffU)) >> 8) +

              *(int *)(&g_LisaDefaultScale_X + local_i19) + g_LisaDefaultOffset_Y) * 4;
    local_i9 = g_LisaDefaultOffset_Z;

    if (g_pLisaAllocatedBuffersEnd == 0) {
      while (local_i9 != -5000) {
        local_i9 = *(int *)(local_70 * 4 + 0x499fa0);

        if (0 < local_i9) {
          do {
            local_pi14 = *(int **)(local_i19 + 4);
            local_i19 = local_i19 + 4;

            if ((local_pi14 != (int *)0x0) && ((int *)*local_pi14 == local_pi14)) {
              local_i18 = *local_pi15;
              local_pi20[local_i18] = (int)local_pi14;
              local_i18 = local_i18 + 1;
              *local_pi15 = local_i18;

              if (*(int *)((int)local_pi14 + 0x26) != 0) {
                local_pi11 = local_pi20 + local_i18;

                do {
                  local_pi14 = *(int **)((int)local_pi14 + 0x26);

                  if ((int *)*local_pi14 == local_pi14) {
                    *local_pi11 = (int)local_pi14;
                    local_pi11 = local_pi11 + 1;
                    *local_pi15 = *local_pi15 + 1;
                  }

                } while (*(int *)((int)local_pi14 + 0x26) != 0);
              }

            }

            local_i9 = local_i9 + -1;
          } while (local_i9 != 0);
        }

        local_i9 = local_70 + 1;
        local_70 = local_70 + 2;
        local_i19 = local_i19 + (*(int *)(local_i9 * 4 + 0x499fa0) + g_LisaGridCellsX) * 4;
        local_i9 = *(int *)(local_70 * 4 + 0x499fa0);
      }

    }

    else {
      while (local_i9 != -5000) {
        local_50[0] = *(float *)(local_70 * 4 + 0x499fa0);

        if (0 < (int)local_50[0]) {
          do {
            local_pi14 = *(int **)(local_i19 + 4);
            local_i19 = local_i19 + 4;

            if ((local_pi14 != (int *)0x0) && ((int *)*local_pi14 == local_pi14)) {
              if (((short)local_pi14[9] == 0) || ((short)local_pi14[9] == local_i18)) {
                local_i9 = *local_pi15;
                local_pi20[local_i9] = (int)local_pi14;
                *local_pi15 = local_i9 + 1;
              }

              if (*(int *)((int)local_pi14 + 0x26) != 0) {
                local_pi11 = local_pi20 + *local_pi15;

                do {
                  local_pi14 = *(int **)((int)local_pi14 + 0x26);

                  if (((int *)*local_pi14 == local_pi14) &&

                     (((short)local_pi14[9] == 0 || ((short)local_pi14[9] == local_i18)))) {
                    *local_pi11 = (int)local_pi14;
                    local_pi11 = local_pi11 + 1;
                    *local_pi15 = *local_pi15 + 1;
                  }

                } while (*(int *)((int)local_pi14 + 0x26) != 0);
              }

            }

            local_50[0] = (float)((int)local_50[0] + -1);
          } while (local_50[0] != 0.0);
        }

        local_i9 = local_70 + 1;
        local_70 = local_70 + 2;
        local_i19 = local_i19 + (*(int *)(local_i9 * 4 + 0x499fa0) + g_LisaGridCellsX) * 4;
        local_i9 = *(int *)(local_70 * 4 + 0x499fa0);
      }

    }

  }

  else {
    g_LisaCamera->visible_obj_count = 0;
    local_pi14 = (int *)(local_i22 + 0x60);
    local_58 = &g_LisaCamera->rot_y;
    local_50[0] = (float)(local_i19 / 3);
    fcos((double)*local_58 * (double)g_Const_TenthDegToRad);
    local_l30 = __ftol();
    local_l31 = __ftol();
    fsin(extraout_ST1);

    local_i18 = (((int)local_l30 + ((int)((int)local_l31 + ((int)local_l31 >> 0x1f & 0xffU)) >> 8)) - local_i19 / 2

             ) * g_LisaGridCellsX;
    local_l30 = __ftol();
    local_l31 = __ftol();
    local_i9 = g_pLisaAllocatedBuffersEnd;
    local_pi20 = g_LisaVisibleObjects;

    local_pi15 = (int *)((int)g_pLisaGridCells +

                     ((local_i18 + (int)local_l30 +

                      ((int)((int)local_l31 + ((int)local_l31 >> 0x1f & 0xffU)) >> 8)) - local_i19 / 2) * 4);
    local_68 = local_i19;

    if (g_pLisaAllocatedBuffersEnd == 0) {
      if (0 < local_i19) {
        local_i9 = g_LisaGridCellsX - local_i19;

        do {
          local_i18 = local_i19;

          if (0 < local_i19) {
            do {
              local_pi11 = (int *)*local_pi15;

              if ((local_pi11 != (int *)0x0) && ((int *)*local_pi11 == local_pi11)) {
                local_i21 = *local_pi14;
                local_pi20[local_i21] = (int)local_pi11;
                local_i21 = local_i21 + 1;
                *local_pi14 = local_i21;

                if (*(int *)((int)local_pi11 + 0x26) != 0) {
                  local_pi10 = local_pi20 + local_i21;

                  do {
                    local_pi11 = *(int **)((int)local_pi11 + 0x26);

                    if ((int *)*local_pi11 == local_pi11) {
                      *local_pi10 = (int)local_pi11;
                      local_pi10 = local_pi10 + 1;
                      *local_pi14 = *local_pi14 + 1;
                    }

                  } while (*(int *)((int)local_pi11 + 0x26) != 0);
                }

              }

              local_pi15 = local_pi15 + 1;
              local_i18 = local_i18 + -1;
            } while (local_i18 != 0);
          }

          local_pi15 = local_pi15 + local_i9;
          local_68 = local_68 + -1;
        } while (local_68 != 0);
      }

    }

    else if (0 < local_i19) {
      local_i18 = g_LisaGridCellsX - local_i19;

      do {
        local_64 = (int *)local_i19;

        if (0 < local_i19) {
          do {
            local_pi11 = (int *)*local_pi15;

            if ((local_pi11 != (int *)0x0) && ((int *)*local_pi11 == local_pi11)) {
              if (((short)local_pi11[9] == 0) || ((short)local_pi11[9] == local_i9)) {
                local_i21 = *local_pi14;
                local_pi20[local_i21] = (int)local_pi11;
                *local_pi14 = local_i21 + 1;
              }

              if (*(int *)((int)local_pi11 + 0x26) != 0) {
                local_pi10 = local_pi20 + *local_pi14;

                do {
                  local_pi11 = *(int **)((int)local_pi11 + 0x26);

                  if (((int *)*local_pi11 == local_pi11) &&

                     (((short)local_pi11[9] == 0 || ((short)local_pi11[9] == local_i9)))) {
                    *local_pi10 = (int)local_pi11;
                    local_pi10 = local_pi10 + 1;
                    *local_pi14 = *local_pi14 + 1;
                  }

                } while (*(int *)((int)local_pi11 + 0x26) != 0);
              }

            }

            local_pi15 = local_pi15 + 1;
            local_64 = (int *)((int)local_64 + -1);
          } while (local_64 != (int *)0x0);
        }

        local_pi15 = local_pi15 + local_i18;
        local_68 = local_68 + -1;
      } while (local_68 != 0);
    }

  }

  local_6c = (int *)(local_i22 + 0x60);
  local_i19 = g_LisaCamera->fov_x;
  local_i22 = g_SubpixelMinX + local_i19 * -0x100;

  if ((((g_LisaActiveMaterial != local_i22) || (g_SubpixelMaxX + local_i19 * -0x100 != g_LisaSubmeshLodLevel)) ||

      (g_SubpixelMinY + g_LisaCamera->fov_y * -0x100 != g_LisaActiveSubmeshFlags)) ||

     (((g_SubpixelMaxY + g_LisaCamera->fov_y * -0x100 != g_LisaCameraPitch ||

       (g_LisaCamera->viewport_x != g_LisaSubmeshClipMask)) ||

      ((g_LisaCamera->viewport_y != g_LisaCameraDistance || (g_LisaMipmapQuality == 1)))))) {
    local_u13 = 0;
    g_LisaMipmapQuality = 0;
    g_LisaSubmeshLodLevel = g_SubpixelMaxX + local_i19 * -0x100;
    local_i9 = g_LisaCamera->fov_y;
    g_LisaActiveSubmeshFlags = g_SubpixelMinY + local_i9 * -0x100;
    g_LisaCameraPitch = g_SubpixelMaxY + local_i9 * -0x100;
    g_LisaSubmeshClipMask = g_LisaCamera->viewport_x;
    g_LisaCameraDistance = g_LisaCamera->viewport_y;

    local_50[0] = ((float)((-1 - local_i19) * 0x100 + g_SubpixelMinX) * (float)g_Const_1000) /

                  ((float)g_LisaSubmeshClipMask * (float)g_Const_Neg256);

    local_50[3] = ((float)((1 - local_i19) * 0x100 + g_SubpixelMaxX) * (float)g_Const_1000) /

                  ((float)g_LisaSubmeshClipMask * (float)g_Const_Neg256);

    local_50[1] = ((float)((-1 - local_i9) * 0x100 + g_SubpixelMinY) * (float)g_Const_1000) /

                  ((float)g_LisaCameraDistance * (float)g_Const_Neg256);

    local_50[7] = ((float)((1 - local_i9) * 0x100 + g_SubpixelMaxY) * (float)g_Const_1000) /

                  ((float)g_LisaCameraDistance * (float)g_Const_Neg256);
    local_50[2] = 1000.0;
    local_50[4] = local_50[1];
    local_50[5] = 1000.0;
    local_50[6] = local_50[3];
    local_50[8] = 1000.0;
    local_50[9] = local_50[0];
    local_50[10] = local_50[7];
    local_50[0xb] = 1000.0;
    local_i19 = 0;
    g_LisaActiveMaterial = local_i22;

    while( 1 ) {
      local_u13 = local_u13 + 1;
      local_u12 = local_u13 & 3;
      local_f1 = local_50[local_u12 * 3];
      local_f2 = *(float *)((int)local_50 + local_i19 + 8);
      local_f3 = local_50[local_u12 * 3 + 2];
      local_f4 = *(float *)((int)local_50 + local_i19);
      local_f5 = local_50[local_u12 * 3 + 1];
      local_f6 = *(float *)((int)local_50 + local_i19);

      *(float *)((int)&g_LisaFrustumPlaneLeft + local_i19) =

           local_50[local_u12 * 3 + 2] * *(float *)((int)local_50 + local_i19 + 4) -

           local_50[local_u12 * 3 + 1] * *(float *)((int)local_50 + local_i19 + 8);
      local_f7 = local_50[local_u12 * 3];
      local_f8 = *(float *)((int)local_50 + local_i19 + 4);
      *(float *)((int)&g_LisaFrustumPlaneRight + local_i19) = local_f1 * local_f2 - local_f3 * local_f4;
      *(float *)((int)&g_LisaFrustumPlaneTop + local_i19) = local_f5 * local_f6 - local_f7 * local_f8;

      local_f1 = SQRT(*(float *)((int)&g_LisaFrustumPlaneLeft + local_i19) * *(float *)((int)&g_LisaFrustumPlaneLeft + local_i19)

                   + *(float *)((int)&g_LisaFrustumPlaneRight + local_i19) *

                     *(float *)((int)&g_LisaFrustumPlaneRight + local_i19) +

                     *(float *)((int)&g_LisaFrustumPlaneTop + local_i19) *

                     *(float *)((int)&g_LisaFrustumPlaneTop + local_i19));

      *(float *)((int)&g_LisaFrustumPlaneLeft + local_i19) =

           (*(float *)((int)&g_LisaFrustumPlaneLeft + local_i19) / local_f1) * g_Const_TenthDegToRadFloat;

      *(float *)((int)&g_LisaFrustumPlaneRight + local_i19) =

           (*(float *)((int)&g_LisaFrustumPlaneRight + local_i19) / local_f1) * g_Const_TenthDegToRadFloat;
      local_f1 = (*(float *)((int)&g_LisaFrustumPlaneTop + local_i19) / local_f1) * g_Const_TenthDegToRadFloat;
      if (0x2f < local_i19 + 0xc) break;
      *(float *)((int)&g_LisaFrustumPlaneTop + local_i19) = local_f1;
      local_i19 = local_i19 + 0xc;
    }

    *(float *)((int)&g_LisaFrustumPlaneTop + local_i19) = local_f1;
  }

  local_f24 = (double)g_LisaCamera->rot_x * (double)g_Const_TenthDegToRad;
  local_f25 = (double)fcos(local_f24);
  local_f26 = (double)fcos((double)*local_58 * (double)g_Const_TenthDegToRad);
  local_f24 = (double)fsin(local_f24);
  local_f27 = (double)g_LisaCamera->rot_z * (double)g_Const_TenthDegToRad;
  local_f28 = (double)fsin((double)*local_58 * (double)g_Const_TenthDegToRad);
  local_f29 = (double)fcos(local_f27);
  local_50[0] = (float)local_f29;
  local_f27 = (double)fsin(local_f27);
  g_LisaObjMat_CosRoll = (float)(local_f24 * local_f28);
  g_LisaObjMat_CosPitch = (float)((double)g_LisaObjMat_CosRoll * local_f27 + (double)local_50[0] * local_f26);
  g_LisaObjMat_SinPitch = (float)((double)local_50[0] * (double)g_LisaObjMat_CosRoll - local_f26 * local_f27);
  g_LisaObjMat_SinRoll = (float)(local_f25 * local_f27);
  g_LisaObjMat_Tmp1 = (float)((double)local_50[0] * local_f25);
  g_LisaObjMat_Tmp2 = (float)-local_f24;
  g_LisaObjMat_Tmp3 = (float)(local_f27 * local_f24 * local_f26 - (double)local_50[0] * local_f28);
  g_LisaObjMat_21 = 0;
  g_LisaObjMat_Scale = 0;
  local_pu23 = &g_LisaFrustumPlaneTop;
  g_LisaObjMat_Tmp4 = (float)((double)local_50[0] * local_f24 * local_f26 + local_f28 * local_f27);
  g_LisaObjMat_CosYaw = 0;
  g_LisaObjMat_Tmp5 = (float)(local_f25 * local_f26);
  pfVar17 = local_50;

  do {
    local_pu23 = local_pu23 + 3;
    local_l30 = __ftol();
    *pfVar17 = (float)local_l30;
    local_l30 = __ftol();
    pfVar17[1] = (float)local_l30;
    local_l30 = __ftol();
    pfVar17[2] = (float)local_l30;
    pfVar17 = pfVar17 + 4;
  } while (local_pu23 < &g_LisaFrustumNear);
  local_l30 = __ftol();
  local_l31 = __ftol();
  local_l32 = __ftol();
  local_70 = *local_6c;
  g_LisaCamera->submesh_count = local_70;
  *local_6c = 0;
  pfVar17 = local_50 + 3;

  do {
    pfVar16 = pfVar17 + 4;

    *pfVar17 = (float)-((int)pfVar17[-1] * (int)local_l32 + (int)pfVar17[-2] * (int)local_l31 +

                       (int)pfVar17[-3] * (int)local_l30);
    pfVar17 = pfVar16;
  } while (pfVar16 < &local_4);
  local_64 = local_pi20;

  if (0 < local_70) {
    do {
      local_i19 = *local_pi20;
      local_u13 = 0;
      pfVar17 = local_50 + 2;

      do {
        local_u13 = local_u13 | (int)pfVar17[-1] * *(int *)(local_i19 + 0x10) +

                          (int)pfVar17[-2] * *(int *)(local_i19 + 0xc) +

                          (int)*pfVar17 * *(int *)(local_i19 + 0x14) + (int)pfVar17[1] +

                          *(short *)(local_i19 + 0x22) * 0x40000;
        if ((int)local_u13 < 0) break;
        pfVar17 = pfVar17 + 4;
      } while (pfVar17 < local_50 + 0x12);

      if (0 < (int)local_u13) {
        *local_64 = local_i19;
        *local_6c = *local_6c + 1;
        local_64 = local_64 + 1;
      }

      local_pi20 = local_pi20 + 1;
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
  /* Refactored and semantically cleaned */
  int *mesh_ptr;
  float roll_cos_f;
  float scale_x;
  float scale_y;
  int temp_x;
  int camera_int;
  int temp_z;
  int obj_y;
  int vert_z;
  double *zoom_ptr;
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
  int visible_idx;
  int obj_idx;
  int i;

  

                    

  LisaCamera *camera = g_LisaCamera;
  zoom_ptr = &g_LisaCamera->zoom;
  g_LisaBackfaceSign = 0;

  if (g_LisaCamera->zoom <= 0.0) {
    g_LisaBackfaceSign = 0xffffffff;
  }

  dst_vert_ptr = &g_LisaCamera->viewport_x;

  if (300 < *dst_vert_ptr) {
    Lisa_TransformVerticesPanorama();
    return;
  }

  mesh_ptr = &g_LisaCamera->viewport_width;
  g_LisaCamera->vertex_counter = 0;
  g_LisaCameraFocalLength = *mesh_ptr << 2;
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
  scale_x = (float)-*dst_vert_ptr * (float)*zoom_ptr * (float)g_Const_1024;
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

  if (0 < g_LisaCamera->visible_obj_count) {
    visible_idx = 0;
    obj_idx = 0;

    do {
      temp_x = g_LisaVisibleSubmeshes;
      temp_z = g_LisaTransformedVertices;
      LisaDynamicObject *obj = g_LisaVisibleObjects[obj_idx >> 2];
      mesh_ptr = (int *)obj->mesh_data;
      trans_z = g_LisaCamera->vertex_counter;
      *(int **)(g_LisaVisibleSubmeshes + visible_idx) = mesh_ptr;
      dst_vert_ptr = (int *)(temp_z + trans_z * 0xc);
      temp_z = obj->pos_x - (int)cam_pos_x;
      *(int **)(temp_x + 4 + visible_idx) = dst_vert_ptr;
      temp_x = obj->pos_y - (int)cam_pos_y;
      obj_y = obj->pos_z - (int)cam_pos_z_1;

      if ((obj->rot_x == 0 && obj->rot_y == 0) &&

          obj->rot_z == 0) {
        int vert_stride = 2;
        vert_idx = *mesh_ptr;

        if (0 < vert_idx) {
          g_LisaCamera->vertex_counter = trans_z + vert_idx;

          do {
            vert_x = mesh_ptr[vert_stride] + temp_z;
            vert_y = temp_x - mesh_ptr[vert_stride + 1];
            trans_z = vert_stride + 2;
            vert_stride = vert_stride + 3;
            vert_z = obj_y + mesh_ptr[trans_z];

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
        Lisa_ComputeObjectMatrix(temp_z,temp_x,obj_y,(int)obj,mesh_ptr);
      }

      visible_idx = visible_idx + 8;
      obj_idx = obj_idx + 4;
      i = i + 1;
    } while (i < g_LisaCamera->visible_obj_count);
  }

  return;
}

/**
 * @original Lisa_TransformVerticesPanorama (IGN_WIN.EXE @ 0x0044a3d0, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_TransformVerticesPanorama(void) {
  float local_f1;
  int *local_pi2;
  float local_f3;
  float local_f4;
  int local_i5;
  int local_i6;
  int local_i7;
  int local_i8;
  int local_i9;
  int local_i10;
  int *local_pi11;
  int local_i12;
  int local_i13;
  int local_i14;
  double local_f15;
  double local_f16;
  double local_f17;
  double local_f18;
  double local_f19;
  double local_f20;
  int local_l21;
  int local_l22;
  int local_l23;
  int local_l24;
  int local_20;
  int local_1c;
  int local_10;
  int local_c;

  

  local_i6 = g_LisaCamera;
  g_LisaCameraFocalLength = g_LisaCamera->viewport_width << 2;
  g_LisaCamera->vertex_counter = 0;
  local_f15 = (double)*(double *)(local_i6 + 0x18) * (double)g_Const_NegTenthDegToRad;
  g_LisaCameraOffsetX = *(int *)(local_i6 + 0x9c) << 8;
  local_f16 = (double)fcos(local_f15);
  g_LisaCameraOffsetY = *(int *)(local_i6 + 0xa0) << 8;
  local_f17 = (double)*(double *)(local_i6 + 0x20) * (double)g_Const_NegTenthDegToRad;
  local_f18 = (double)fcos(local_f17);
  local_f15 = (double)fsin(local_f15);
  local_f19 = (double)*(double *)(local_i6 + 0x28) * (double)g_Const_NegTenthDegToRad;
  local_f17 = (double)fsin(local_f17);
  local_f20 = (double)fcos(local_f19);
  local_f19 = (double)fsin(local_f19);
  local_f1 = (float)local_f20;
  local_f3 = (float)-*(int *)(local_i6 + 0x80) * (float)*(double *)(local_i6 + 0x30) * (float)g_Const_256;
  local_f4 = (float)-*(int *)(local_i6 + 0x84) * (float)g_Const_256;

  g_LisaObjMat_CosPitch =

       (float)(((double)local_f1 * local_f18 - (double)(float)(local_f15 * local_f17) * local_f19) *

              (double)local_f3);
  g_LisaObjMat_SinPitch = (float)-((double)local_f3 * local_f16 * local_f19);

  g_LisaObjMat_CosRoll =

       (float)(((double)(float)(local_f15 * local_f18) * local_f19 + (double)local_f1 * local_f17) *

              (double)local_f3);

  g_LisaObjMat_SinRoll =

       (float)(((double)local_f1 * (double)(float)(local_f15 * local_f17) + local_f18 * local_f19) *

              (double)local_f4);
  g_LisaObjMat_Tmp1 = (float)((double)local_f4 * local_f16 * (double)local_f1);
  g_LisaObjMat_Tmp4 = (float)(local_f15 * (double)g_Const_262144);
  g_LisaObjMat_Tmp3 = (float)(-(local_f16 * local_f17) * (double)g_Const_262144);

  g_LisaObjMat_Tmp2 =

       (float)((local_f17 * local_f19 - (double)local_f1 * (double)(float)(local_f15 * local_f18)) *

              (double)local_f4);
  g_LisaObjMat_Tmp5 = (float)(local_f18 * local_f16 * (double)g_Const_262144);
  local_l21 = __ftol();
  g_LisaCameraMatrix_00 = (int)local_l21;
  local_l21 = __ftol();
  g_LisaCameraMatrix_01 = (int)local_l21;
  local_l21 = __ftol();
  g_LisaCameraMatrix_02 = (int)local_l21;
  local_l21 = __ftol();
  g_LisaCameraMatrix_10 = (int)local_l21;
  local_l21 = __ftol();
  g_LisaCameraMatrix_11 = (int)local_l21;
  local_l21 = __ftol();
  g_LisaCameraMatrix_12 = (int)local_l21;
  local_l21 = __ftol();
  g_LisaCameraMatrix_20 = (int)local_l21;
  local_l21 = __ftol();
  g_LisaCameraMatrix_21 = (int)local_l21;
  local_l21 = __ftol();
  g_LisaCameraMatrix_22 = (int)local_l21;
  local_l21 = __ftol();
  local_l22 = __ftol();
  local_i6 = (int)local_l22;
  local_l22 = __ftol();
  local_l23 = __ftol();
  local_i14 = (int)local_l23;
  local_l23 = __ftol();
  local_l24 = __ftol();
  local_i7 = (int)local_l24;
  local_i5 = g_LisaCameraMatrix_00 * local_i6 + g_LisaCameraMatrix_02 * local_i7 + g_LisaCameraMatrix_01 * local_i14;
  g_LisaSubmeshCenterWorldX = (int)(local_i5 + (local_i5 >> 0x1f & 0xfffU)) >> 0xc;
  local_i5 = g_LisaCameraMatrix_10 * local_i6 + g_LisaCameraMatrix_12 * local_i7 + g_LisaCameraMatrix_11 * local_i14;
  g_LisaSubmeshCenterWorldZ = (int)(local_i5 + (local_i5 >> 0x1f & 0xfffU)) >> 0xc;
  local_i6 = g_LisaCameraMatrix_20 * local_i6 + g_LisaCameraMatrix_22 * local_i7 + g_LisaCameraMatrix_21 * local_i14;
  local_c = 0;
  g_LisaSubmeshDepthOffset = (int)(local_i6 + (local_i6 >> 0x1f & 0xfffU)) >> 0xc;

  if (0 < g_LisaCamera->visible_obj_count) {
    local_1c = 0;
    local_10 = 0;

    do {
      local_i14 = g_LisaCamera;
      local_i6 = *(int *)(g_LisaVisibleObjects + local_10);
      local_pi2 = *(int **)(local_i6 + 4);
      *(int **)(g_LisaVisibleSubmeshes + local_1c) = local_pi2;
      local_i14 = *(int *)(local_i14 + 0x5c);
      local_pi11 = (int *)(g_LisaTransformedVertices + local_i14 * 0xc);
      *(int **)(g_LisaVisibleSubmeshes + 4 + local_1c) = local_pi11;
      local_i7 = *(int *)(local_i6 + 0xc) - (int)local_l21;
      local_i5 = *(int *)(local_i6 + 0x10) - (int)local_l22;
      local_i8 = *(int *)(local_i6 + 0x14) - (int)local_l23;

      if ((*(short *)(local_i6 + 0x18) == 0 && *(short *)(local_i6 + 0x1a) == 0) &&

          *(short *)(local_i6 + 0x1c) == 0) {
        local_20 = *local_pi2;

        if (0 < local_20) {
          g_LisaCamera->vertex_counter = local_i14 + local_20;
          local_i6 = 2;

          do {
            local_i9 = local_i7 + local_pi2[local_i6];
            local_i12 = local_i5 - local_pi2[local_i6 + 1];
            local_i10 = local_i8 + local_pi2[local_i6 + 2];

            local_i13 = (g_LisaCameraMatrix_12 * local_i10 + g_LisaCameraMatrix_11 * local_i12 + g_LisaCameraMatrix_10 * local_i9) -

                     g_LisaSubmeshCenterWorldZ;

            local_i14 = ((g_LisaCameraMatrix_22 * local_i10 + g_LisaCameraMatrix_21 * local_i12 + g_LisaCameraMatrix_20 * local_i9) -

                      g_LisaSubmeshDepthOffset >> 0x10) + g_LisaCameraFocalLength;

            if (local_i14 < g_LisaCameraFocalLength) {
              local_i14 = g_LisaCameraFocalLength;
            }

            *local_pi11 = g_LisaCameraOffsetX +

                       (((g_LisaCameraMatrix_02 * local_i10 + g_LisaCameraMatrix_01 * local_i12 + g_LisaCameraMatrix_00 * local_i9) -

                        g_LisaSubmeshCenterWorldX) / local_i14) * 4;
            local_20 = local_20 + -1;
            local_pi11[2] = local_i14;
            local_pi11[1] = g_LisaCameraOffsetY + (local_i13 / local_i14) * 4;
            local_pi11 = local_pi11 + 3;
            local_i6 = local_i6 + 3;
          } while (local_20 != 0);
        }

      }

      else {
        Lisa_TransformSubmeshVerticesPanorama(local_i7,local_i5,local_i8,local_i6,local_pi2);
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
int Lisa_ComputeObjectMatrix(int arg_1,int arg_2,int arg_3,int arg_4,int *arg_5) {
  ushort local_u1;
  ushort local_u2;
  ushort local_u3;
  short sVar4;
  int local_i5;
  int local_i6;
  int local_i7;
  int local_i8;
  int local_i9;
  int *local_pi10;
  int local_i11;
  int local_i12;
  int local_i13;
  int local_i14;
  int local_l15;
  int local_l16;
  int local_l17;
  int local_l18;
  int local_l19;
  int local_l20;
  int local_l21;
  int local_l22;
  int local_l23;
  int local_58;

  

  local_i11 = (g_LisaCameraMatrix_02 * arg_3 + g_LisaCameraMatrix_01 * arg_2 + g_LisaCameraMatrix_00 * arg_1) - g_LisaSubmeshCenterWorldX

  ;

  local_i12 = (g_LisaCameraMatrix_12 * arg_3 + g_LisaCameraMatrix_11 * arg_2 + g_LisaCameraMatrix_10 * arg_1) - g_LisaSubmeshCenterWorldZ

  ;

  local_i13 = (g_LisaCameraMatrix_22 * arg_3 + g_LisaCameraMatrix_21 * arg_2 + g_LisaCameraMatrix_20 * arg_1) - g_LisaSubmeshDepthOffset

  ;
  local_u1 = *(ushort *)(arg_4 + 0x1a);
  local_u2 = *(ushort *)(arg_4 + 0x18);
  local_u3 = *(ushort *)(arg_4 + 0x1c);

  if (0xe10 < (ushort)(local_u2 | local_u1 | local_u3)) {
    if ((short)local_u2 < 0) {
      *(ushort *)(arg_4 + 0x18) = ((ushort)(0xe0f - local_u2) / 0xe10) * 0xe10 + local_u2;
    }

    if ((short)local_u1 < 0) {
      *(ushort *)(arg_4 + 0x1a) = ((ushort)(0xe0f - local_u1) / 0xe10) * 0xe10 + local_u1;
    }

    if ((short)local_u3 < 0) {
      *(ushort *)(arg_4 + 0x1c) = ((ushort)(0xe0f - local_u3) / 0xe10) * 0xe10 + local_u3;
    }

    sVar4 = *(short *)(arg_4 + 0x18);

    if (0xe10 < sVar4) {
      *(ushort *)(arg_4 + 0x18) = ((ushort)(sVar4 - 1U) / 0xe10) * -0xe10 + sVar4;
    }

    sVar4 = *(short *)(arg_4 + 0x1a);

    if (0xe10 < sVar4) {
      *(ushort *)(arg_4 + 0x1a) = ((ushort)(sVar4 - 1U) / 0xe10) * -0xe10 + sVar4;
    }

    sVar4 = *(short *)(arg_4 + 0x1c);

    if (0xe10 < sVar4) {
      *(ushort *)(arg_4 + 0x1c) = ((ushort)(sVar4 - 1U) / 0xe10) * -0xe10 + sVar4;
    }

  }

  local_l15 = __ftol();
  local_l16 = __ftol();
  local_l17 = __ftol();
  local_l18 = __ftol();
  local_l19 = __ftol();
  local_l20 = __ftol();
  local_l21 = __ftol();
  local_l22 = __ftol();
  local_l23 = __ftol();
  local_i6 = g_LisaTransformedVertices;
  local_i9 = 2;
  local_58 = *arg_5;

  if (0 < local_58) {
    local_i5 = g_LisaCamera->vertex_counter;
    g_LisaCamera->vertex_counter = local_i5 + local_58;
    local_pi10 = (int *)(local_i6 + local_i5 * 0xc);

    do {
      local_i6 = arg_5[local_i9];
      local_i5 = arg_5[local_i9 + 1];
      local_i7 = arg_5[local_i9 + 2];
      local_i9 = local_i9 + 3;

      local_i14 = g_LisaCameraFocalLength +

               (local_i7 * (int)local_l23 + local_i5 * (int)local_l22 + local_i6 * (int)local_l21 + local_i13 >> 0x10);

      if (local_i14 < g_LisaCameraFocalLength) {
        local_i14 = g_LisaCameraFocalLength;
      }

      *local_pi10 = g_LisaCameraOffsetX +

                 (local_i11 + local_i7 * (int)local_l17 + local_i5 * (int)local_l16 + local_i6 * (int)local_l15) / local_i14

      ;
      local_i8 = g_LisaCameraOffsetY;
      local_pi10[2] = local_i14;
      local_58 = local_58 + -1;

      local_pi10[1] = local_i8 + (local_i12 + local_i7 * (int)local_l20 + local_i5 * (int)local_l19 + local_i6 * (int)local_l18

                           ) / local_i14;
      local_pi10 = local_pi10 + 3;
    } while (local_58 != 0);
  }

  return 0;
}

/**
 * @original Lisa_TransformSubmeshVerticesPanorama (IGN_WIN.EXE @ 0x0044ae20, lisa3d.c)
 * @fidelity ADAPTED
 */
int Lisa_TransformSubmeshVerticesPanorama(int arg_1,int arg_2,int arg_3,int arg_4,int *arg_5) {
  ushort local_u1;
  ushort local_u2;
  ushort local_u3;
  short sVar4;
  int local_i5;
  int local_i6;
  int local_i7;
  int local_i8;
  int local_i9;
  int *local_pi10;
  int local_i11;
  int local_i12;
  int local_i13;
  int local_l14;
  int local_l15;
  int local_l16;
  int local_l17;
  int local_l18;
  int local_l19;
  int local_l20;
  int local_l21;
  int local_l22;
  int local_58;

  

  local_i11 = (g_LisaCameraMatrix_02 * arg_3 + g_LisaCameraMatrix_01 * arg_2 + g_LisaCameraMatrix_00 * arg_1) - g_LisaSubmeshCenterWorldX

  ;

  local_i12 = (g_LisaCameraMatrix_12 * arg_3 + g_LisaCameraMatrix_11 * arg_2 + g_LisaCameraMatrix_10 * arg_1) - g_LisaSubmeshCenterWorldZ

  ;

  local_i13 = (g_LisaCameraMatrix_22 * arg_3 + g_LisaCameraMatrix_21 * arg_2 + g_LisaCameraMatrix_20 * arg_1) - g_LisaSubmeshDepthOffset

  ;
  local_u1 = *(ushort *)(arg_4 + 0x1a);
  local_u2 = *(ushort *)(arg_4 + 0x18);
  local_u3 = *(ushort *)(arg_4 + 0x1c);

  if (0xe10 < (ushort)(local_u2 | local_u1 | local_u3)) {
    if ((short)local_u2 < 0) {
      *(ushort *)(arg_4 + 0x18) = ((ushort)(0xe0f - local_u2) / 0xe10) * 0xe10 + local_u2;
    }

    if ((short)local_u1 < 0) {
      *(ushort *)(arg_4 + 0x1a) = ((ushort)(0xe0f - local_u1) / 0xe10) * 0xe10 + local_u1;
    }

    if ((short)local_u3 < 0) {
      *(ushort *)(arg_4 + 0x1c) = ((ushort)(0xe0f - local_u3) / 0xe10) * 0xe10 + local_u3;
    }

    sVar4 = *(short *)(arg_4 + 0x18);

    if (0xe10 < sVar4) {
      *(ushort *)(arg_4 + 0x18) = ((ushort)(sVar4 - 1U) / 0xe10) * -0xe10 + sVar4;
    }

    sVar4 = *(short *)(arg_4 + 0x1a);

    if (0xe10 < sVar4) {
      *(ushort *)(arg_4 + 0x1a) = ((ushort)(sVar4 - 1U) / 0xe10) * -0xe10 + sVar4;
    }

    sVar4 = *(short *)(arg_4 + 0x1c);

    if (0xe10 < sVar4) {
      *(ushort *)(arg_4 + 0x1c) = ((ushort)(sVar4 - 1U) / 0xe10) * -0xe10 + sVar4;
    }

  }

  local_l14 = __ftol();
  local_l15 = __ftol();
  local_l16 = __ftol();
  local_l17 = __ftol();
  local_l18 = __ftol();
  local_l19 = __ftol();
  local_l20 = __ftol();
  local_l21 = __ftol();
  local_l22 = __ftol();
  local_i6 = g_LisaTransformedVertices;
  local_58 = *arg_5;

  if (0 < local_58) {
    local_i5 = g_LisaCamera->vertex_counter;
    g_LisaCamera->vertex_counter = local_i5 + local_58;
    local_i8 = 2;
    local_pi10 = (int *)(local_i6 + local_i5 * 0xc);

    do {
      local_i6 = arg_5[local_i8];
      local_i5 = arg_5[local_i8 + 1];
      local_i7 = arg_5[local_i8 + 2];

      local_i9 = (local_i5 * (int)local_l21 + local_i7 * (int)local_l22 + local_i6 * (int)local_l20 + local_i13 >> 0x10) +

              g_LisaCameraFocalLength;

      if (local_i9 < g_LisaCameraFocalLength) {
        local_i9 = g_LisaCameraFocalLength;
      }

      *local_pi10 = g_LisaCameraOffsetX +

                 ((local_i11 + local_i5 * (int)local_l15 + local_i7 * (int)local_l16 + local_i6 * (int)local_l14) / local_i9

                 ) * 4;
      local_58 = local_58 + -1;
      local_pi10[2] = local_i9;

      local_pi10[1] = g_LisaCameraOffsetY +

                   ((local_i12 + local_i7 * (int)local_l19 + local_i5 * (int)local_l18 + local_i6 * (int)local_l17) /

                   local_i9) * 4;
      local_i8 = local_i8 + 3;
      local_pi10 = local_pi10 + 3;
    } while (local_58 != 0);
  }

  return 0;
}

/**
 * @original Lisa_ComputeCameraRotationMatrix (IGN_WIN.EXE @ 0x0044b340, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_ComputeCameraRotationMatrix(int *arg_1) {
  double local_f1;
  double local_f2;
  int local_l3;

  

  local_f1 = (double)g_LisaCamera->rot_x * (double)g_Const_NegTenthDegToRad;
  fcos(local_f1);
  local_f2 = (double)g_LisaCamera->rot_y * (double)g_Const_NegTenthDegToRad;
  fcos(local_f2);
  fsin(local_f1);
  local_f1 = (double)g_LisaCamera->rot_z * (double)g_Const_NegTenthDegToRad;
  fsin(local_f2);
  fcos(local_f1);
  fsin(local_f1);
  local_l3 = __ftol();
  *arg_1 = (int)local_l3;
  local_l3 = __ftol();
  arg_1[1] = (int)local_l3;
  local_l3 = __ftol();
  arg_1[2] = (int)local_l3;
  local_l3 = __ftol();
  arg_1[3] = (int)local_l3;
  local_l3 = __ftol();
  arg_1[4] = (int)local_l3;
  local_l3 = __ftol();
  arg_1[5] = (int)local_l3;
  local_l3 = __ftol();
  arg_1[6] = (int)local_l3;
  local_l3 = __ftol();
  arg_1[7] = (int)local_l3;
  local_l3 = __ftol();
  arg_1[8] = (int)local_l3;
  return;
}

/**
 * @original Lisa_InitOpcodeTable (IGN_WIN.EXE @ 0x0044b480, lisa3d.c)
 * @fidelity ADAPTED
 */
void Lisa_InitOpcodeTable(void) {
  int local_i1;

  

  local_i1 = g_LisaCamera;
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
    *(int *)(local_i1 + 0x6c) = 0;
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
    *(int *)(local_i1 + 0x6c) = 0;
    return;
  }

  PTR_Lisa_DrawTexturedTriangle_Op15_0049c934 = Lisa_DrawTexturedTriangle_Op15_Sub;
  g_LisaCamera->active_draw_cmd = 0;
  *(int *)(local_i1 + 0x6c) = 0;
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
    int v0 = *(int *)(g_pLisaSubmeshPolygon + 4);
    int v1 = *(int *)(g_pLisaSubmeshPolygon + 8);
    int v2 = *(int *)(g_pLisaSubmeshPolygon + 0xc);
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
            cmd[7] = *(int *)(g_pLisaSubmeshPolygon + 0x10);
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
  int local_i1;
  int local_i2;
  int local_i3;
  int local_i4;
  int *local_pu5;
  int local_u6;
  int local_u7;
  int local_u8;
  int local_i9;
  int local_i10;
  int local_i11;
  int local_i12;
  int local_i13;
  int local_i14;
  int local_i15;
  int local_i16;
  int local_i17;
  int local_i18;
  int *local_pu19;
  unsigned int local_u20;
  unsigned int local_u21;
  int local_i22;
  int *local_pu23;
  int local_i24;
  int local_l25;

  

  local_i9 = g_LisaTransformedVertices;
  g_LisaObjMat_11 = g_LisaTransformedVertices;

  do {
    local_i24 = *(int *)(g_pLisaSubmeshPolygon + 4);
    local_i22 = local_i24 * 3;
    g_LisaSubmeshVertexStride = *(int *)(g_pLisaSubmeshPolygon + 8) * 3;
    g_LisaCameraFocalScale = *(int *)(g_pLisaSubmeshPolygon + 0xc) * 3;
    local_i1 = *(int *)(local_i9 + 4 + local_i24 * 0xc);
    local_i24 = *(int *)(local_i9 + local_i24 * 0xc);

    if ((g_SubpixelMaxY - local_i1 | g_SubpixelMaxX - local_i24 | local_i1 - g_SubpixelMinY | local_i24 - g_SubpixelMinX

        ) < 0) {
      do {
        local_i24 = *(int *)(local_i9 + local_i22 * 4);
        local_i1 = *(int *)(local_i9 + g_LisaSubmeshVertexStride * 4);
        g_LisaCameraMatrix_Y = local_i24;

        if (local_i24 <= local_i1) {
          g_LisaCameraMatrix_Y = local_i1;
        }

        local_i2 = *(int *)(local_i9 + g_LisaCameraFocalScale * 4);
        local_i11 = g_LisaCameraMatrix_Y;

        if (g_LisaCameraMatrix_Y <= local_i2) {
          local_i11 = local_i2;
        }

        if (g_SubpixelMinX <= local_i11) {
          g_LisaCameraMatrix_Y = local_i24;

          if (local_i1 <= local_i24) {
            g_LisaCameraMatrix_Y = local_i1;
          }

          local_i24 = g_LisaCameraMatrix_Y;

          if (local_i2 <= g_LisaCameraMatrix_Y) {
            local_i24 = local_i2;
          }

          if (local_i24 <= g_SubpixelMaxX) {
            local_i24 = *(int *)(local_i9 + 4 + local_i22 * 4);
            local_i1 = *(int *)(local_i9 + 4 + g_LisaSubmeshVertexStride * 4);
            g_LisaCameraMatrix_Y = local_i24;

            if (local_i24 <= local_i1) {
              g_LisaCameraMatrix_Y = local_i1;
            }

            local_i2 = *(int *)(local_i9 + 4 + g_LisaCameraFocalScale * 4);
            local_i11 = g_LisaCameraMatrix_Y;

            if (g_LisaCameraMatrix_Y <= local_i2) {
              local_i11 = local_i2;
            }

            if (g_SubpixelMinY <= local_i11) {
              g_LisaCameraMatrix_Y = local_i24;

              if (local_i1 <= local_i24) {
                g_LisaCameraMatrix_Y = local_i1;
              }

              local_i24 = g_LisaCameraMatrix_Y;

              if (local_i2 <= g_LisaCameraMatrix_Y) {
                local_i24 = local_i2;
              }

              if (local_i24 <= g_SubpixelMaxY) break;
            }

          }

        }

        local_i24 = g_pLisaSubmeshPolygon + 0x2c;

        if (*(char *)(g_pLisaSubmeshPolygon + 0x2c) != '\x15') {
          g_LisaObjMat_Tmp6 = local_i22;
          g_pLisaSubmeshPolygon = local_i24;
          return;
        }

        if (g_LisaSubmeshPolyCount < 3) {
          g_LisaObjMat_Tmp6 = local_i22;
          g_pLisaSubmeshPolygon = local_i24;
          return;
        }

        g_LisaSubmeshPolyCount = g_LisaSubmeshPolyCount + -1;
        local_i22 = *(int *)(g_pLisaSubmeshPolygon + 0x30) * 3;
        g_LisaCameraFocalScale = *(int *)(g_pLisaSubmeshPolygon + 0x38) * 3;
        g_LisaSubmeshVertexStride = *(int *)(g_pLisaSubmeshPolygon + 0x34) * 3;
        g_pLisaSubmeshPolygon = local_i24;
      } while( 1 );
    }

    local_i10 = g_pLisaSubmeshPolygon;
    local_i24 = *(int *)(local_i9 + 4 + g_LisaSubmeshVertexStride * 4);
    local_i1 = *(int *)(local_i9 + g_LisaCameraFocalScale * 4);
    local_i2 = *(int *)(local_i9 + g_LisaSubmeshVertexStride * 4);
    local_i11 = *(int *)(local_i9 + 4 + g_LisaCameraFocalScale * 4);

    g_LisaSubmeshCenterWorldY = ((local_i24 >> 4) - (*(int *)(local_i9 + 4 + local_i22 * 4) >> 4)) *

                   ((local_i1 >> 4) - (local_i2 >> 4)) +

                   ((local_i11 >> 4) - (local_i24 >> 4)) *

                   ((*(int *)(local_i9 + local_i22 * 4) >> 4) - (local_i2 >> 4)) ^ g_LisaBackfaceSign;
    local_i3 = *(int *)(local_i9 + 8 + g_LisaCameraFocalScale * 4);
    local_i4 = *(int *)(local_i9 + 8 + g_LisaSubmeshVertexStride * 4);
    local_i12 = local_i3 + local_i4 + *(int *)(local_i9 + 8 + local_i22 * 4);
    g_LisaObjMat_Tmp6 = local_i22;

    if ((600 < local_i12) && (0 < (int)g_LisaSubmeshCenterWorldY)) {
      g_LisaCameraMatrix_X = local_i12 >> 4;
      local_pu5 = (int *)*g_pLisaDrawCommandWritePtr;

      if (g_LisaShadingEnabled == 1) {
        local_i17 = *(int *)(local_i9 + local_i22 * 4) >> 8;
        local_i13 = *(int *)(local_i9 + 4 + local_i22 * 4) >> 8;

        local_i13 = (((local_i24 >> 8) + local_i13) * ((local_i2 >> 8) - local_i17) +

                  ((local_i11 >> 8) + local_i13) * (local_i17 - (local_i1 >> 8)) +

                 ((local_i24 >> 8) + (local_i11 >> 8)) * ((local_i1 >> 8) - (local_i2 >> 8))) * 3;
        local_u20 = local_i13 >> 0x1f;
        g_LisaActiveLightingMode = (local_i13 >> 1 ^ local_u20) - local_u20;
        local_i13 = *(int *)(g_pLisaSubmeshPolygon + 0x24) >> 8;
        local_i17 = *(int *)(g_pLisaSubmeshPolygon + 0x14) >> 8;
        local_i18 = *(int *)(g_pLisaSubmeshPolygon + 0x20) >> 8;
        local_i14 = *(int *)(g_pLisaSubmeshPolygon + 0x10) >> 8;
        local_i15 = *(int *)(g_pLisaSubmeshPolygon + 0x1c) >> 8;
        local_i16 = *(int *)(g_pLisaSubmeshPolygon + 0x18) >> 8;

        local_u20 = (local_i15 + local_i17) * (local_i16 - local_i14) +

                 (local_i17 + local_i13) * (local_i14 - local_i18) + (local_i15 + local_i13) * (local_i18 - local_i16);
        local_u21 = (int)local_u20 >> 0x1f;
        local_i13 = (local_u20 ^ local_u21) - local_u21;
        if (local_i13 < g_LisaActiveLightingMode) goto LAB_0044d8ef;
        g_LisaSubmeshBoundRadius = (g_LisaActiveLightingMode * 4 <= local_i13) - 4;
      }

      else {
LAB_0044d8ef:

        g_LisaSubmeshBoundRadius = -5;
      }

      if (((g_LisaCamera->shading_mode == 0) ||

          (g_LisaObjMat_20 = (int)*(short *)(g_pLisaActiveSubmesh + 0x1e), 0x2d0 < g_LisaCameraMatrix_X)) ||

         (((99 < g_LisaObjMat_20 && (g_LisaObjMat_20 != 200)) &&

          ((g_LisaObjMat_20 < 300 || (0x12e < g_LisaObjMat_20)))))) {
        local_u6 = *(int *)(local_i9 + local_i22 * 4);
        local_u7 = *(int *)(local_i9 + 4 + local_i22 * 4);
        g_LisaSubmeshTmp5 = local_pu5;
        *local_pu5 = 0x11;
        local_pu5[1] = local_u6;
        local_pu5[2] = local_u7;
        local_pu5[3] = local_i2;
        local_i22 = g_pLisaActiveMipTable;
        local_pu5[4] = local_i24;
        local_pu5[5] = local_i1;
        local_i24 = g_LisaSubmeshBoundRadius;
        local_pu5[6] = local_i11;
        local_pu5[7] = g_pLisaSubmeshPolygon + 0x10;
        local_pu19 = g_pLisaDrawCommandWritePtr;
        local_pu23 = local_pu5 + 9;
        local_pu5[8] = *(int *)(local_i22 + local_i24 * 4) + *(int *)(g_pLisaSubmeshPolygon + 0x28);
      }

      else {
        if (g_LisaCameraMatrix_X < 0x1e1) {
          local_pu5[4] = *(int *)(local_i9 + 8 + local_i22 * 4);
          local_pu5[7] = local_i4;
          local_pu5[10] = local_i3;
        }

        else {
          g_LisaSubmeshFlags = local_i12 / 3;
          g_LisaSubmeshTmp4 = (float)(0x2d0 - g_LisaCameraMatrix_X) * g_Const_512;
          local_l25 = __ftol();
          local_pu5[4] = (int)local_l25 + g_LisaSubmeshFlags;
          local_l25 = __ftol();
          local_pu5[7] = (int)local_l25 + g_LisaSubmeshFlags;
          g_LisaSubmeshTmp1 = local_i3;
          local_l25 = __ftol();
          local_pu5[10] = (int)local_l25 + g_LisaSubmeshFlags;
        }

        local_u6 = *(int *)(local_i9 + local_i22 * 4);
        local_u7 = *(int *)(local_i9 + 4 + local_i22 * 4);
        g_LisaSubmeshTmp5 = local_pu5;
        *local_pu5 = 0x14;
        local_pu5[2] = local_u6;
        local_pu5[3] = local_u7;
        local_pu5[5] = local_i2;
        local_pu5[6] = local_i24;
        local_u6 = *(int *)(local_i10 + 0x10);
        local_pu5[8] = local_i1;
        local_u7 = *(int *)(local_i10 + 0x14);
        local_pu5[9] = local_i11;
        local_u8 = *(int *)(local_i10 + 0x18);
        local_pu5[0xb] = local_u6;
        local_u6 = *(int *)(local_i10 + 0x1c);
        local_pu5[0xc] = local_u7;
        local_u7 = *(int *)(local_i10 + 0x20);
        local_pu5[0xd] = local_u8;
        local_u8 = *(int *)(local_i10 + 0x24);
        local_pu5[0xe] = local_u6;
        local_pu5[0xf] = local_u7;
        local_pu5[0x10] = local_u8;
        local_pu19 = g_pLisaDrawCommandWritePtr;
        local_pu23 = local_pu5 + 0x11;
        local_pu5[1] = *(int *)(g_pLisaActiveMipTable + g_LisaSubmeshBoundRadius * 4) + *(int *)(local_i10 + 0x28);
      }

      local_i24 = g_pLisaActiveSubmesh;
      g_LisaCameraMatrix_X = g_LisaCameraMatrix_X + -0x50;
      local_pu19[2] = local_pu23;
      local_i1 = g_pLisaDepthBuckets;
      local_pu5 = g_pLisaDrawCommandWritePtr;

      if (99 < *(short *)(local_i24 + 0x1e)) {
        if (*(short *)(local_i24 + 0x1e) == 0xd2) {
          local_i24 = -0x54;
        }

        else {
          local_i24 = -0x5c;
        }

        g_LisaCameraMatrix_X = g_LisaCameraMatrix_X + local_i24;
      }

      if (g_LisaCameraMatrix_X < 0) {
        g_LisaCameraMatrix_X = 0;
      }

      if (5999 < g_LisaCameraMatrix_X) {
        g_LisaCameraMatrix_X = 5999;
      }

      local_i24 = g_LisaCameraMatrix_X;
      local_pu23 = g_pLisaDrawCommandWritePtr + 1;
      g_pLisaDrawCommandWritePtr = local_pu19 + 2;
      *local_pu23 = *(int *)(g_pLisaDepthBuckets + g_LisaCameraMatrix_X * 4);
      *(int **)(local_i1 + local_i24 * 4) = local_pu5;
    }

    g_pLisaSubmeshPolygon = local_i10 + 0x2c;

    if ((*(char *)(local_i10 + 0x2c) != '\x15') || (g_LisaSubmeshPolyCount < 3)) {
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
  char *local_pc1;
  short sVar2;
  int local_i3;
  int local_i4;
  int local_i5;
  int *local_pu6;
  int local_u7;
  int local_u8;
  int local_u9;
  int *local_pi10;
  int local_i11;
  int local_i12;
  int local_i13;
  int local_i14;
  int local_i15;
  int local_i16;
  int local_i17;
  int local_i18;
  int local_i19;
  unsigned int local_u20;
  int local_i21;
  unsigned int local_u22;
  int local_i23;
  int *local_pu24;
  int local_i25;
  int local_i26;
  int *local_pu27;
  int local_l28;

  

  local_i11 = g_LisaTransformedVertices;
  g_LisaObjMat_01 = g_LisaTransformedVertices;
  local_i19 = g_LisaSubmeshPolyCount;

  do {
    g_LisaSubmeshPolyCount = local_i19;
    local_i19 = *(int *)(g_pLisaSubmeshPolygon + 4);
    local_i25 = local_i19 * 3;
    g_LisaFrustumPlaneBottom = *(int *)(g_pLisaSubmeshPolygon + 8) * 3;
    g_LisaSubmeshPolyStride = *(int *)(g_pLisaSubmeshPolygon + 0xc) * 3;
    local_i23 = *(int *)(local_i11 + 4 + local_i19 * 0xc);
    local_i19 = *(int *)(local_i11 + local_i19 * 0xc);

    if ((g_SubpixelMaxY - local_i23 | g_SubpixelMaxX - local_i19 | local_i23 - g_SubpixelMinY |

        local_i19 - g_SubpixelMinX) < 0) {
      do {
        local_i4 = g_pLisaSubmeshPolygon;
        local_i19 = *(int *)(local_i11 + local_i25 * 4);
        local_i23 = *(int *)(local_i11 + g_LisaFrustumPlaneBottom * 4);
        g_LisaCameraMatrix_Z = local_i23;

        if (local_i23 <= local_i19) {
          g_LisaCameraMatrix_Z = local_i19;
        }

        local_i3 = *(int *)(local_i11 + g_LisaSubmeshPolyStride * 4);
        local_i26 = g_LisaCameraMatrix_Z;

        if (g_LisaCameraMatrix_Z <= local_i3) {
          local_i26 = local_i3;
        }

        if (g_SubpixelMinX <= local_i26) {
          g_LisaCameraMatrix_Z = local_i23;

          if (local_i19 <= local_i23) {
            g_LisaCameraMatrix_Z = local_i19;
          }

          local_i19 = g_LisaCameraMatrix_Z;

          if (local_i3 <= g_LisaCameraMatrix_Z) {
            local_i19 = local_i3;
          }

          if (local_i19 <= g_SubpixelMaxX) {
            local_i19 = *(int *)(local_i11 + 4 + g_LisaFrustumPlaneBottom * 4);
            local_i23 = *(int *)(local_i11 + 4 + local_i25 * 4);
            g_LisaCameraMatrix_Z = local_i19;

            if (local_i19 <= local_i23) {
              g_LisaCameraMatrix_Z = local_i23;
            }

            local_i3 = *(int *)(local_i11 + 4 + g_LisaSubmeshPolyStride * 4);
            local_i26 = g_LisaCameraMatrix_Z;

            if (g_LisaCameraMatrix_Z <= local_i3) {
              local_i26 = local_i3;
            }

            if (g_SubpixelMinY <= local_i26) {
              g_LisaCameraMatrix_Z = local_i19;

              if (local_i23 <= local_i19) {
                g_LisaCameraMatrix_Z = local_i23;
              }

              local_i19 = g_LisaCameraMatrix_Z;

              if (local_i3 <= g_LisaCameraMatrix_Z) {
                local_i19 = local_i3;
              }

              if (local_i19 <= g_SubpixelMaxY) break;
            }

          }

        }

        g_LisaObjMat_12 = local_i25;
        local_pc1 = (char *)(g_pLisaSubmeshPolygon + 0x2c);
        g_pLisaSubmeshPolygon = g_pLisaSubmeshPolygon + 0x2c;

        if ((*local_pc1 != '\x11') || (g_LisaSubmeshPolyCount + -1 < 1)) {
          return;
        }

        local_i25 = *(int *)(local_i4 + 0x30) * 3;
        g_LisaFrustumPlaneBottom = *(int *)(local_i4 + 0x34) * 3;
        g_LisaSubmeshPolyStride = *(int *)(local_i4 + 0x38) * 3;
        g_LisaSubmeshPolyCount = g_LisaSubmeshPolyCount + -1;
      } while( 1 );
    }

    local_i12 = g_pLisaSubmeshPolygon;
    local_i19 = *(int *)(local_i11 + 4 + g_LisaSubmeshPolyStride * 4);
    local_i23 = *(int *)(local_i11 + 4 + g_LisaFrustumPlaneBottom * 4);
    local_i4 = *(int *)(local_i11 + g_LisaFrustumPlaneBottom * 4);
    local_i3 = *(int *)(local_i11 + g_LisaSubmeshPolyStride * 4);

    g_LisaObjMat_00 =

         ((local_i19 >> 4) - (local_i23 >> 4)) * ((*(int *)(local_i11 + local_i25 * 4) >> 4) - (local_i4 >> 4)) +

         ((local_i23 >> 4) - (*(int *)(local_i11 + 4 + local_i25 * 4) >> 4)) * ((local_i3 >> 4) - (local_i4 >> 4))

         ^ g_LisaBackfaceSign;
    local_i26 = *(int *)(local_i11 + 8 + g_LisaFrustumPlaneBottom * 4);
    local_i5 = *(int *)(local_i11 + 8 + g_LisaSubmeshPolyStride * 4);
    local_i13 = local_i26 + local_i5 + *(int *)(local_i11 + 8 + local_i25 * 4);
    g_LisaObjMat_12 = local_i25;

    if ((600 < local_i13) && (0 < (int)g_LisaObjMat_00)) {
      g_LisaSubmeshVertexCount = local_i13 >> 4;
      local_pu6 = (int *)*g_pLisaDrawCommandWritePtr;

      if (g_LisaShadingEnabled == 1) {
        local_i14 = *(int *)(local_i11 + 4 + local_i25 * 4) >> 8;
        local_i15 = *(int *)(local_i11 + local_i25 * 4) >> 8;

        local_i14 = (((local_i19 >> 8) + (local_i23 >> 8)) * ((local_i3 >> 8) - (local_i4 >> 8)) +

                  (local_i14 + (local_i23 >> 8)) * ((local_i4 >> 8) - local_i15) +

                 (local_i14 + (local_i19 >> 8)) * (local_i15 - (local_i3 >> 8))) * 3;
        local_u20 = local_i14 >> 0x1f;
        g_LisaCameraRoll = (local_i14 >> 1 ^ local_u20) - local_u20;
        local_i14 = *(int *)(g_pLisaSubmeshPolygon + 0x24) >> 8;
        local_i15 = *(int *)(g_pLisaSubmeshPolygon + 0x14) >> 8;
        local_i18 = *(int *)(g_pLisaSubmeshPolygon + 0x20) >> 8;
        local_i16 = *(int *)(g_pLisaSubmeshPolygon + 0x10) >> 8;
        local_i17 = *(int *)(g_pLisaSubmeshPolygon + 0x1c) >> 8;
        local_i21 = *(int *)(g_pLisaSubmeshPolygon + 0x18) >> 8;

        local_u20 = (local_i14 + local_i15) * (local_i16 - local_i18) + (local_i14 + local_i17) * (local_i18 - local_i21) +

                 (local_i17 + local_i15) * (local_i21 - local_i16);
        local_u22 = (int)local_u20 >> 0x1f;
        local_i14 = (local_u20 ^ local_u22) - local_u22;

        if (local_i14 < g_LisaCameraRoll) {
          g_LisaCameraYaw = -5;
        }

        else {
          if (local_i14 < g_LisaCameraRoll * 4) goto LAB_0044ecb6;
          g_LisaCameraYaw = (g_LisaCameraRoll * 0x10 <= local_i14) - 3;
        }

      }

      else {
LAB_0044ecb6:

        g_LisaCameraYaw = -4;
      }

      sVar2 = *(short *)(g_pLisaActiveSubmesh + 0x1e);
      g_LisaObjMat_SinYaw = (int)sVar2;

      if ((g_LisaSubmeshVertexCount < 0x2d1) &&

         (((g_LisaObjMat_SinYaw < 100 || (g_LisaObjMat_SinYaw == 200)) ||

          ((299 < g_LisaObjMat_SinYaw && (g_LisaObjMat_SinYaw < 0x12f)))))) {
        if (g_LisaSubmeshVertexCount < 0x1e1) {
          local_pu6[4] = *(int *)(local_i11 + 8 + local_i25 * 4);
          local_pu6[7] = local_i26;
          local_pu6[10] = local_i5;
        }

        else {
          g_LisaCurrentVertexIndex = local_i13 / 3;
          g_LisaSubmeshTmp3 = (float)(0x2d0 - g_LisaSubmeshVertexCount) * g_Const_512;
          local_l28 = __ftol();
          local_pu6[4] = (int)local_l28 + g_LisaCurrentVertexIndex;
          local_l28 = __ftol();
          local_pu6[7] = (int)local_l28 + g_LisaCurrentVertexIndex;
          g_LisaObjMat_10 = local_i5;
          local_l28 = __ftol();
          local_pu6[10] = (int)local_l28 + g_LisaCurrentVertexIndex;
        }

        local_u7 = *(int *)(local_i11 + local_i25 * 4);
        local_u8 = *(int *)(local_i11 + 4 + local_i25 * 4);
        *local_pu6 = 0x14;
        local_pu6[2] = local_u7;
        local_pu6[3] = local_u8;
        local_i25 = g_LisaCameraYaw;
        local_pu6[5] = local_i4;
        local_pu6[6] = local_i23;
        local_pu6[8] = local_i3;
        local_pu6[9] = local_i19;

        if ((local_i25 == -5) && (g_pLisaActiveMipTable[-5] != *g_pLisaActiveMipTable)) {
          local_u20 = *(unsigned int *)(local_i12 + 0x10);
          local_u22 = *(unsigned int *)(local_i12 + 0x14);
          local_pu6[0xb] = (local_u20 & 0x3fff) << 2;
          local_pu6[0xc] = (local_u22 & 0x3fff) << 2;
          local_pu6[0xd] = (*(unsigned int *)(local_i12 + 0x18) & 0x3fff) << 2;
          local_pu6[0xe] = (*(unsigned int *)(local_i12 + 0x1c) & 0x3fff) << 2;
          local_i19 = (int)local_u20 >> 0xe;
          local_pu6[0xf] = (*(unsigned int *)(local_i12 + 0x20) & 0x3fff) << 2;
          g_LisaObjMat_02 = (int)local_u22 >> 0xe;
          local_i23 = g_LisaObjMat_02 * 0x4000;
          g_LisaSubmeshTmp2 = local_i19;
          local_pu6[0x10] = (*(unsigned int *)(local_i12 + 0x24) & 0x3fff) << 2;
          local_i19 = g_pLisaActiveMipTable[local_i19 * 4] + (*(int *)(local_i12 + 0x28) + local_i23) * 4;
        }

        else {
          local_u7 = *(int *)(local_i12 + 0x14);
          local_u8 = *(int *)(local_i12 + 0x18);
          local_pu6[0xb] = *(int *)(local_i12 + 0x10);
          local_u9 = *(int *)(local_i12 + 0x1c);
          local_pu6[0xc] = local_u7;
          local_pi10 = g_pLisaActiveMipTable;
          local_u7 = *(int *)(local_i12 + 0x20);
          local_pu6[0xd] = local_u8;
          local_pu6[0xe] = local_u9;
          local_i19 = g_LisaCameraYaw;
          local_u8 = *(int *)(local_i12 + 0x24);
          local_pu6[0xf] = local_u7;
          local_pu6[0x10] = local_u8;
          local_i19 = local_pi10[local_i19] + *(int *)(local_i12 + 0x28);
        }

        local_pu24 = g_pLisaDrawCommandWritePtr;
        local_pu6[1] = local_i19;
        local_pu27 = local_pu6 + 0x11;
      }

      else {
        local_u7 = *(int *)(local_i11 + 4 + local_i25 * 4);
        local_pu6[1] = *(int *)(local_i11 + local_i25 * 4);
        local_pu6[2] = local_u7;
        local_pu6[3] = local_i4;
        local_i25 = g_LisaCameraYaw;
        local_pu6[4] = local_i23;
        local_pu6[5] = local_i3;
        local_pu6[6] = local_i19;
        local_pu6[7] = (int *)(g_pLisaSubmeshPolygon + 0x10);
        local_i19 = g_LisaCameraYaw;
        local_pi10 = g_pLisaActiveMipTable;

        if ((local_i25 == -5) && (g_pLisaActiveMipTable[-5] != *g_pLisaActiveMipTable)) {
          g_LisaSubmeshTmp2 = *(int *)(g_pLisaSubmeshPolygon + 0x10);
          *local_pu6 = 0x16;
          g_LisaSubmeshTmp2 = g_LisaSubmeshTmp2 >> 0xe;
          g_LisaObjMat_02 = *(int *)(g_pLisaSubmeshPolygon + 0x14) >> 0xe;

          local_pu6[8] = local_pi10[g_LisaSubmeshTmp2 * 4] +

                      (*(int *)(g_pLisaSubmeshPolygon + 0x28) + g_LisaObjMat_02 * 0x4000) * 4;
        }

        else {
          local_i23 = *(int *)(g_pLisaSubmeshPolygon + 0x28);
          *local_pu6 = 0x11;
          local_pu6[8] = local_pi10[local_i19] + local_i23;
        }

        local_pu27 = local_pu6 + 9;
        local_pu24 = g_pLisaDrawCommandWritePtr;
      }

      g_LisaSubmeshTmp6 = local_pu6;
      local_pu24[2] = local_pu27;
      local_i19 = g_pLisaDepthBuckets;
      local_pu6 = g_pLisaDrawCommandWritePtr;
      local_i23 = g_LisaSubmeshVertexCount + -0x50;

      if (99 < sVar2) {
        if (sVar2 == 0xd2) {
          local_i23 = g_LisaSubmeshVertexCount + -0xa4;
        }

        else {
          local_i23 = g_LisaSubmeshVertexCount + -0xac;
        }

      }

      g_LisaSubmeshVertexCount = local_i23;

      if (g_LisaSubmeshVertexCount < 0) {
        g_LisaSubmeshVertexCount = 0;
      }

      if (5999 < g_LisaSubmeshVertexCount) {
        g_LisaSubmeshVertexCount = 5999;
      }

      local_i23 = g_LisaSubmeshVertexCount;
      local_pu27 = g_pLisaDrawCommandWritePtr + 1;
      g_pLisaDrawCommandWritePtr = local_pu24 + 2;
      *local_pu27 = *(int *)(g_pLisaDepthBuckets + g_LisaSubmeshVertexCount * 4);
      *(int **)(local_i19 + local_i23 * 4) = local_pu6;
    }

    g_pLisaSubmeshPolygon = local_i12 + 0x2c;

    if ((*(char *)(local_i12 + 0x2c) != '\x11') || (local_i19 = g_LisaSubmeshPolyCount + -1, g_LisaSubmeshPolyCount + -1 < 1))

    {
      return;
    }

  } while( 1 );
}

/**
 * @original Lisa_InitRasterizerTables (IGN_WIN.EXE @ 0x0044f070, lisa3d.c)
 * @fidelity ADAPTED
 */
LisaReturn64 Lisa_InitRasterizerTables(int arg_1,unsigned int arg_2) {
  int in_EAX;
  short sVar1;
  int unaff_EBX;
  unsigned int local_u2;
  int local_i3;
  int unaff_ESI;
  int *local_pi4;
  char **ppuVar5;

  

  if ((in_EAX < 0x579) && (unaff_EBX < 0x259)) {
    local_pi4 = &g_LisaActiveTextureID;
    local_u2 = 1;
    g_LisaScanlinePitch = in_EAX;
    g_LisaAspectScale = unaff_EBX;

    do {
      *local_pi4 = (int)(0x10000 / (unsigned int)local_u2) + -1;
      local_pi4 = local_pi4 + 1;
      local_u2 = local_u2 + 1;
    } while (local_u2 != 0x3a9b);

    for (ppuVar5 = (char **)g_pLisaShutdownCallbacks; *ppuVar5 != (char *)0x0; ppuVar5 = ppuVar5 + 1) {
      (*(void (*)())*ppuVar5)(ppuVar5,unaff_ESI,unaff_EBX);
    }

    local_pi4 = &g_LisaScreenPitch;
    local_i3 = 0;
    sVar1 = 600;

    do {
      *local_pi4 = local_i3;
      local_pi4 = local_pi4 + 1;
      local_i3 = local_i3 + in_EAX;
      sVar1 = sVar1 + -1;
    } while (sVar1 != 0);
    { LisaReturn64 _r; _r.edx = arg_2; _r.eax = 0; return _r; }
  }

  { LisaReturn64 _r; _r.edx = arg_2; _r.eax = 0xffffffff; return _r; }
}

/**
 * @original Lisa_ExecuteRasterizerCommands (IGN_WIN.EXE @ 0x0044f0e9, lisa3d.c)
 * @fidelity ADAPTED
 */
LisaReturn64 Lisa_ExecuteRasterizerCommands(int arg_1,unsigned int arg_2) {
  int *unaff_ESI;
  LisaReturn64 local_l1;

  

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
    local_l1 = (*(LisaReturn64 (*)())(((void **)g_LisaRasterizerJmpTable)[*g_pLisaEdgeBuffer]))();
    return local_l1;
  }

  { LisaReturn64 _r; _r.edx = arg_2; _r.eax = 0; return _r; }
}
