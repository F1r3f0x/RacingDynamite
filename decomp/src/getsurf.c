/*
 * getsurf.c - Track surface heightmap, spatial collision grid & raycasting
 * Original file: d:\projects\ignition\getsurf\getsurf.c
 * Target: MAINDOS.EXE (Watcom C/C++ 10.6, 32-bit flat protected mode)
 */

#include "getsurf.h"

/* Global collision variables matching MAINDOS_32BIT.EXE memory */
void *g_pActiveSRF = NULL;
SrfCell *g_pSRF_Grid = NULL;
int g_SRF_GridCellsX = 0;
int g_SRF_GridCellsZ = 0;
int g_SRF_CellSizeX = 512;
int g_SRF_CellSizeZ = 512;
int g_SRF_GridStrideX = 0;
int g_SRF_GridStrideZ = 0;

/**
 * Surface_FreeSRF (MAINDOS @ 0x0002002C, IGN_WIN @ 0x004127A0)
 * Frees the active surface buffer if allocated.
 */
void Surface_FreeSRF(void) {
    if (g_pActiveSRF != NULL) {
        free(g_pActiveSRF);
        g_pActiveSRF = NULL;
    }
}

/**
 * Surface_GetTriangleHeight (MAINDOS @ 0x00020BBC, IGN_WIN @ 0x00413380)
 * Evaluates the triangle plane height at the apex: ((-y0 - y1 - y2) / 3) + obj->pos_y
 */
int Surface_GetTriangleHeight(SurfaceHeightContext *ctx) {
    SurfaceObject *obj = ctx->obj;
    SurfacePolyTri *poly = ctx->poly;
    SurfaceVertex *vbuf = obj->vertex_buffer;

    int y0 = vbuf[poly->v0_idx].y;
    int y1 = vbuf[poly->v1_idx].y;
    int y2 = vbuf[poly->v2_idx].y;

    int avg_neg_y = ((-y0) + (-y1) + (-y2)) / 3;
    return avg_neg_y + obj->pos_y;
}
