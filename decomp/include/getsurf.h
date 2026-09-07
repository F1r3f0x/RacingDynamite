#ifndef GETSURF_H
#define GETSURF_H

#include <stddef.h>
#include <stdlib.h>

#pragma pack(push, 1)

/* .SRF File Header (36 bytes / 9 x 32-bit int) */
typedef struct SrfHeader {
    int grid_cells_x;     /* 0x00: Grid cells along X */
    int grid_cells_z;     /* 0x04: Grid cells along Z */
    int cell_size_z;      /* 0x08: World dimension of cell Z (512) */
    int cell_size_x;      /* 0x0C: World dimension of cell X (512) */
    int grid_stride_x;    /* 0x10: Spatial stride X */
    int grid_stride_z;    /* 0x14: Spatial stride Z */
    int triangle_count;   /* 0x18: Total collision triangles */
    int table1_count;     /* 0x1C: Primary index buffer count */
    int table2_count;     /* 0x20: Secondary index buffer count */
} SrfHeader;

/* 12-byte Spatial Grid Cell */
typedef struct SrfCell {
    int table2_offset;    /* 0x00: Byte offset into table2 */
    int table1_offset;    /* 0x04: Byte offset into table1 */
    short table1_count;   /* 0x08: Number of triangles intersecting cell */
    short table2_count;   /* 0x0A: Number of secondary entities */
} SrfCell;

/* 24-byte Collision Triangle Record */
typedef struct SrfTriangle {
    int x_base;           /* 0x00: Base apex X coordinate */
    int z_base;           /* 0x04: Base apex Z coordinate */
    int slope1;           /* 0x08: 16.16 fixed-point slope dx1/dz */
    int slope2;           /* 0x0C: 16.16 fixed-point slope dx2/dz */
    short flags_dz;       /* 0x10: Low 16 bits = dz (signed) */
    short poly_offset;    /* 0x12: High 16 bits = polygon offset */
    int v_ptr;            /* 0x14: Byte offset into .PLC placed object array */
} SrfTriangle;

/* Object representation matching placement / vertex structures */
typedef struct SurfaceVertex {
    int x;
    int y;
    int z;
} SurfaceVertex;

typedef struct SurfaceObject {
    int field0;                   /* 0x00 */
    SurfaceVertex *vertex_buffer; /* 0x04 */
    int dummy8;                   /* 0x08 */
    int pos_x;                    /* 0x0C */
    int pos_y;                    /* 0x10 */
    int pos_z;                    /* 0x14 */
    short dummy18;                /* 0x18 */
    short dummy1a;                /* 0x1A */
    short dummy1c;                /* 0x1C */
    short field_1e;               /* 0x1E */
    short field_20;               /* 0x20 */
    short dummy22;                /* 0x22 */
} SurfaceObject;

typedef struct SurfacePolyTri {
    int flags_material;           /* 0x00 */
    int v0_idx;                   /* 0x04 */
    int v1_idx;                   /* 0x08 */
    int v2_idx;                   /* 0x0C */
} SurfacePolyTri;

typedef struct SurfaceHeightContext {
    SurfaceObject *obj;           /* 0x00 */
    SurfacePolyTri *poly;         /* 0x04 */
} SurfaceHeightContext;

/* 64-byte Surface Raycast Result (16 x 32-bit int) */
typedef struct SurfaceRaycastResult {
    int material;                 /* 0x00: Material ID (-1 if no hit) */
    int normal_x;                 /* 0x04: Face normal X */
    int normal_y;                 /* 0x08: Face normal Y */
    int normal_z;                 /* 0x0C: Face normal Z */
    int v0_world_x;               /* 0x10: Vertex 0 world X */
    int v0_world_y;               /* 0x14: Vertex 0 world Y */
    int v0_world_z;               /* 0x18: Vertex 0 world Z */
    int v1_world_x;               /* 0x1C: Vertex 1 world X */
    int v1_world_y;               /* 0x20: Vertex 1 world Y */
    int v1_world_z;               /* 0x24: Vertex 1 world Z */
    int v2_world_x;               /* 0x28: Vertex 2 world X */
    int v2_world_y;               /* 0x2C: Vertex 2 world Y */
    int v2_world_z;               /* 0x30: Vertex 2 world Z */
    int obj_field0;               /* 0x34: Object header field 0 */
    int obj_field20;              /* 0x38: Object field at offset 0x20 */
    int obj_field1e;              /* 0x3C: Object field at offset 0x1E */
} SurfaceRaycastResult;

#pragma pack(pop)

/* Global surface pointers matching authentic memory layout */
extern void *g_pActiveSRF;
extern SrfCell *g_pSRF_Grid;
extern int g_SRF_GridCellsX;
extern int g_SRF_GridCellsZ;
extern int g_SRF_CellSizeX;
extern int g_SRF_CellSizeZ;
extern int g_SRF_GridStrideX;
extern int g_SRF_GridStrideZ;

/* Function prototypes */
void Surface_FreeSRF(void);
int Surface_GetTriangleHeight(SurfaceHeightContext *ctx);

void *Surface_TestTrianglePositiveDZ(int count, int qx, int qz, SrfTriangle **table, void *out_hits);
#pragma aux Surface_TestTrianglePositiveDZ parm [ecx] [edx] [ebx] [esi] [edi] value [edi] modify [eax ebx edx esi];

void *Surface_TestTriangleNegativeDZ(int count, int qx, int qz, SrfTriangle **table, void *out_hits);
#pragma aux Surface_TestTriangleNegativeDZ parm [ecx] [edx] [ebx] [esi] [edi] value [edi] modify [eax ebx edx esi];

SurfaceRaycastResult *Surface_Raycast(int qx, int qy, int qz);

#endif /* GETSURF_H */
