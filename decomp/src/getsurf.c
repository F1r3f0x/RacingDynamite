/*
 * getsurf.c - Track surface heightmap, spatial collision grid & raycasting
 * Original file: d:\projects\ignition\getsurf\getsurf.c
 * Target: MAINDOS.EXE (Watcom C/C++ 10.6, 32-bit flat protected mode)
 */

#include "getsurf.h"

/* Global collision variables matching MAINDOS_32BIT.EXE memory */
void *g_pActiveSRF = NULL;
SrfCell *g_pSRF_Grid = NULL;
SrfTriangle *g_pSRF_Triangles = NULL;
SrfTriangle **g_pSRF_Table1 = NULL;
SrfTriangle **g_pSRF_Table2 = NULL;
int g_SRF_GridCellsX = 0;
int g_SRF_GridCellsZ = 0;
int g_SRF_CellSizeX = 512;
int g_SRF_CellSizeZ = 512;
int g_SRF_GridStrideX = 0;
int g_SRF_GridStrideZ = 0;

/**
 * Surface_LoadSRF (MAINDOS @ 0x0001FEE0, IGN_WIN @ 0x00412670)
 * Loads .SRF track collision surface from disk and converts relative offsets to pointers.
 */
int Surface_LoadSRF(const char *filename, void *scene_objects) {
    SrfHeader *hdr;
    int total_cells;
    int tri_count;
    int t1_count;
    int t2_count;
    int total_tables_bytes;
    int i;
    char *triangles;
    char *table1;
    char *table2;
    SrfCell *cell;

    g_pActiveSRF = File_LoadToMemory(filename);
    hdr = (SrfHeader *)g_pActiveSRF;

    g_SRF_GridCellsX = hdr->grid_cells_x;
    g_SRF_GridCellsZ = hdr->grid_cells_z;
    g_SRF_CellSizeZ = hdr->cell_size_z;
    g_SRF_CellSizeX = hdr->cell_size_x;
    g_SRF_GridStrideX = hdr->grid_stride_x;
    g_SRF_GridStrideZ = hdr->grid_stride_z;

    tri_count = hdr->triangle_count;
    t1_count = hdr->table1_count;
    t2_count = hdr->table2_count;

    g_pSRF_Grid = (SrfCell *)((char *)hdr + sizeof(SrfHeader));
    total_cells = g_SRF_GridStrideX * g_SRF_GridStrideZ;

    triangles = (char *)g_pSRF_Grid + total_cells * sizeof(SrfCell);
    table1 = triangles + tri_count * sizeof(SrfTriangle);
    table2 = table1 + t1_count * sizeof(int);
    g_pSRF_Table2 = (SrfTriangle **)table2;

    for (i = 0; i < total_cells; i++) {
        cell = &g_pSRF_Grid[i];
        cell->table2_offset += (int)table2;
        cell->table1_offset += (int)table1;
    }

    if (tri_count > 0) {
        int total_tri_bytes = tri_count * sizeof(SrfTriangle);
        for (i = 0; i < total_tri_bytes; i += sizeof(SrfTriangle)) {
            SrfTriangle *tri = (SrfTriangle *)(triangles + i);
            tri->v_ptr += (int)scene_objects;
        }
    }

    total_tables_bytes = (t2_count + t1_count) * sizeof(int);
    for (i = 0; i < total_tables_bytes; i += sizeof(int)) {
        int *entry = (int *)(table1 + i);
        *entry += (int)triangles;
    }

    g_pSRF_Table1 = (SrfTriangle **)table1;
    g_pSRF_Triangles = (SrfTriangle *)triangles;
    return 1;
}

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

/**
 * Surface_TestTrianglePositiveDZ (MAINDOS @ 0x00020C18, IGN_WIN @ 0x00446578)
 * 2D trapezoidal slope span test for table2 triangles (dz >= 0).
 */
/*
void *Surface_TestTrianglePositiveDZ(int count, int qx, int qz, SrfTriangle **table, void *out_hits) {
...
}

void *Surface_TestTriangleNegativeDZ(int count, int qx, int qz, SrfTriangle **table, void *out_hits) {
...
}
*/

/* Static hit buffer and scratch variables for raycasting (0x000eb900 - 0x000ebb54) */
static SurfaceHeightContext s_hits[56];
static SurfaceHeightContext *s_cur_hit;
static SurfaceHeightContext *s_best_hit;
static void *s_hits_end;
static int s_saved_ebp;
static SrfCell *s_cur_cell;
static SurfaceRaycastResult s_raycast_result;

static int s_scratch_mat;
static int s_scratch_pos_x;
static int s_scratch_pos_y;
static int s_scratch_pos_z;
static int s_scratch_v0_x;
static int s_scratch_v0_y;
static int s_scratch_v0_z;
static int s_scratch_v1_x;
static int s_scratch_v1_y;
static int s_scratch_v1_z;
static int s_scratch_v2_x;
static int s_scratch_v2_y;
static int s_scratch_v2_z;
static int s_scratch_obj_field0;
static int s_scratch_obj_field20;
static int s_scratch_obj_field1e;

/**
 * Surface_Raycast (MAINDOS @ 0x00020814, IGN_WIN @ 0x00412fc0)
 * Evaluates spatial grid cell, queries triangle spans, selects best surface,
 * computes face normal via cross product, and transforms triangle vertices to world space.
 */
SurfaceRaycastResult *Surface_Raycast(int qx, int qy, int qz) {
    int cell_z = qz / g_SRF_CellSizeZ;
    int cell_x = qx / g_SRF_CellSizeX;
    int cell_idx = cell_z * g_SRF_GridStrideX + cell_x;
    s_cur_cell = &g_pSRF_Grid[cell_idx];

    s_hits_end = Surface_TestTrianglePositiveDZ(
        s_cur_cell->table2_count, qx, qz,
        (SrfTriangle **)s_cur_cell->table2_offset,
        s_hits
    );
    s_hits_end = Surface_TestTriangleNegativeDZ(
        s_cur_cell->table1_count, qx, qz,
        (SrfTriangle **)s_cur_cell->table1_offset,
        s_hits_end
    );

    if (s_hits_end == s_hits) {
        s_raycast_result.material = -1;
        return &s_raycast_result;
    }

    s_best_hit = s_hits;
    if ((SurfaceHeightContext *)s_hits_end > &s_hits[1]) {
        int best_mat = 0x4f;
        int best_dist = 1000000000;
        for (s_cur_hit = s_hits; (void *)s_cur_hit < s_hits_end; s_cur_hit++) {
            int tri_y = Surface_GetTriangleHeight(s_cur_hit);
            int dy = tri_y - qy;
            int mat;
            if (dy < 0) {
                dy = -dy;
            } else {
                dy = dy * dy;
            }
            mat = s_cur_hit->poly->flags_material >> 16;
            if ((dy < best_dist && (mat < 0x28 || mat > 0x4f)) ||
                ((dy < best_dist || mat < 0x28 || mat > 0x4f) && (best_mat >= 0x28 && best_mat <= 0x4f))) {
                best_mat = mat;
                best_dist = dy;
                s_best_hit = s_cur_hit;
            }
        }
    }

    {
        SurfaceObject *obj = s_best_hit->obj;
        SurfacePolyTri *poly = s_best_hit->poly;
        SurfaceVertex *vbuf = (SurfaceVertex *)((char *)obj->vertex_buffer + 8);

        s_scratch_mat = poly->flags_material >> 16;
        s_scratch_pos_x = obj->pos_x;
        s_scratch_pos_y = obj->pos_y;
        s_scratch_pos_z = obj->pos_z;

        s_scratch_v0_x = vbuf[poly->v0_idx].x;
        s_scratch_v0_y = vbuf[poly->v0_idx].y;
        s_scratch_v0_z = vbuf[poly->v0_idx].z;

        s_scratch_v1_x = vbuf[poly->v1_idx].x;
        s_scratch_v1_y = vbuf[poly->v1_idx].y;
        s_scratch_v1_z = vbuf[poly->v1_idx].z;

        s_scratch_v2_x = vbuf[poly->v2_idx].x;
        s_scratch_v2_y = vbuf[poly->v2_idx].y;
        s_scratch_v2_z = vbuf[poly->v2_idx].z;

        s_scratch_obj_field0 = obj->field0;
        s_scratch_obj_field20 = obj->field_20;
        s_scratch_obj_field1e = obj->field_1e;

        {
            int ax = s_scratch_v0_x - s_scratch_v1_x;
            int ay = s_scratch_v0_y - s_scratch_v1_y;
            int az = s_scratch_v0_z - s_scratch_v1_z;

            int bx = s_scratch_v2_x - s_scratch_v1_x;
            int by = s_scratch_v2_y - s_scratch_v1_y;
            int bz = s_scratch_v2_z - s_scratch_v1_z;

            int nx = ay * bz - az * by;
            int ny = az * bx - ax * bz;
            int nz = ax * by - ay * bx;

            s_raycast_result.material = s_scratch_mat;
            s_raycast_result.normal_x = nx;
            s_raycast_result.normal_y = ny;
            s_raycast_result.normal_z = nz;

            s_raycast_result.v0_world_x = s_scratch_v0_x + s_scratch_pos_x;
            s_raycast_result.v0_world_y = s_scratch_v0_y - s_scratch_pos_y;
            s_raycast_result.v0_world_z = s_scratch_v0_z + s_scratch_pos_z;

            s_raycast_result.v1_world_x = s_scratch_v1_x + s_scratch_pos_x;
            s_raycast_result.v1_world_y = s_scratch_v1_y - s_scratch_pos_y;
            s_raycast_result.v1_world_z = s_scratch_v1_z + s_scratch_pos_z;

            s_raycast_result.v2_world_x = s_scratch_v2_x + s_scratch_pos_x;
            s_raycast_result.v2_world_y = s_scratch_v2_y - s_scratch_pos_y;
            s_raycast_result.v2_world_z = s_scratch_v2_z + s_scratch_pos_z;

            s_raycast_result.obj_field0 = s_scratch_obj_field0;
            s_raycast_result.obj_field20 = s_scratch_obj_field20;
            s_raycast_result.obj_field1e = s_scratch_obj_field1e;
        }
    }

    return &s_raycast_result;
}


