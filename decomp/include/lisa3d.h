#ifndef LISA3D_H
#define LISA3D_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned int eax;
    unsigned int edx;
} LisaReturn64;

#pragma pack(push, 1)
typedef struct {
    uint8_t *file_data;         /* 0x00 */
    uint8_t *pixel_buffer;      /* 0x04 */
    int16_t frame_count;        /* 0x08 */
    int16_t loop_flag;          /* 0x0A */
    int16_t width;              /* 0x0C */
    int16_t height;             /* 0x0E */
    uint8_t *palette;           /* 0x10 */
    int16_t current_frame;      /* 0x14 */
    int32_t is_open;            /* 0x16 */
    uint8_t *frame_data_start;  /* 0x1A */
    uint8_t *cur_frame_ptr;     /* 0x1E */
} CdpFile;
#pragma pack(pop)

/* Lisa 3D Geometry & Pipeline structures */
typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
} LisaVertex;

typedef struct {
    int32_t min_x;
    int32_t min_y;
    int32_t max_x;
    int32_t max_y;
} LisaViewportClip;

#pragma pack(push, 1)
typedef struct LisaCamera {
    int32_t pad00[6];              /* 0x00: 24 bytes */
    double  rot_x;                 /* 0x18: Camera rotation pitch */
    double  rot_y;                 /* 0x20: Camera rotation yaw */
    double  rot_z;                 /* 0x28: Camera rotation roll */
    double  zoom;                  /* 0x30: Focal length / zoom factor */
    int32_t enable_sky;            /* 0x38: Sky backdrop enable flag */
    int32_t enable_frustum_cull;   /* 0x3C: Spatial grid frustum cull flag */
    int32_t enable_transform;      /* 0x40: Vertex transformation flag */
    int32_t pad44[3];              /* 0x44: 12 bytes */
    int32_t enable_submeshes;      /* 0x50: Submesh processing flag */
    int32_t enable_depth_sort;     /* 0x54: Depth bucket sorting flag */
    int32_t shading_mode;          /* 0x58: 0 = unshaded, 1 = shaded/alpha */
    int32_t vertex_counter;        /* 0x5C: Transformed vertex counter */
    int32_t visible_obj_count;     /* 0x60: Count of visible objects passed culling */
    int32_t submesh_count;         /* 0x64: Count of visible submeshes */
    int32_t active_draw_cmd;       /* 0x68: Active draw command index */
    int32_t pad6c[5];              /* 0x6C: 20 bytes */
    int32_t viewport_x;            /* 0x80: Viewport center X */
    int32_t viewport_y;            /* 0x84: Viewport center Y */
    int32_t viewport_width;        /* 0x88: Viewport screen width */
    int32_t pad8c[4];              /* 0x8C: 16 bytes */
    int32_t fov_x;                 /* 0x9C: Horizontal FOV scale (8.8 fixed) */
    int32_t fov_y;                 /* 0xA0: Vertical FOV scale (8.8 fixed) */
    int32_t projection_type;       /* 0xA4: 0 = perspective 3D, 1 = panorama/ortho */
} LisaCamera;

typedef struct MshPolygon {
    uint32_t header;               /* 0x00: Opcode in low byte (0x11..0x17), flags in high 24 bits */
    uint32_t vi0;                  /* 0x04: Index of vertex 0 in submesh vertex buffer */
    uint32_t vi1;                  /* 0x08: Index of vertex 1 in submesh vertex buffer */
    uint32_t vi2;                  /* 0x0C: Index of vertex 2 in submesh vertex buffer */
    int32_t  tu0;                  /* 0x10: Vertex 0 U (8.8 fixed) */
    int32_t  tv0;                  /* 0x14: Vertex 0 V (8.8 fixed) */
    int32_t  tu1;                  /* 0x18: Vertex 1 U (8.8 fixed) */
    int32_t  tv1;                  /* 0x1C: Vertex 1 V (8.8 fixed) */
    int32_t  tu2;                  /* 0x20: Vertex 2 U (8.8 fixed) */
    int32_t  tv2;                  /* 0x24: Vertex 2 V (8.8 fixed) */
    uint32_t extra;                /* 0x28: Texture page byte offset within .TEX */
} MshPolygon;

typedef struct {
    int32_t     vertex_count;  /* 0x00: Number of 3D vertices */
    int32_t     polygon_count; /* 0x04: Number of polygon records */
    /* Followed by vertex_count * 3 x int32_t */
    /* Followed by polygon_count * MshPolygon */
} MshSubmesh;

typedef struct LisaDynamicObject {
    struct LisaDynamicObject *self_ptr; /* 0x00 */
    MshSubmesh *mesh_data;              /* 0x04 */
    int32_t unknown_08;                 /* 0x08 */
    int32_t pos_x;                      /* 0x0C */
    int32_t pos_y;                      /* 0x10 */
    int32_t pos_z;                      /* 0x14 */
    int16_t rot_x;                      /* 0x18 */
    int16_t rot_y;                      /* 0x1A */
    int16_t rot_z;                      /* 0x1C */
    int16_t unknown_1e;                 /* 0x1E */
    int16_t unknown_20;                 /* 0x20 */
    int16_t unknown_22;                 /* 0x22 */
    int16_t unknown_24;                 /* 0x24 */
    struct LisaDynamicObject *next_in_cell; /* 0x26 */
} LisaDynamicObject;

typedef struct {
    LisaDynamicObject *dyn_obj;         /* 0x00 */
    int32_t pos_x;                      /* 0x04 */
    int32_t pos_y;                      /* 0x08 */
    int32_t pos_z;                      /* 0x0C */
    int32_t rot_x;                      /* 0x10 */
    int32_t rot_y;                      /* 0x14 */
    int32_t rot_z;                      /* 0x18 */
    int32_t in_grid;                    /* 0x1C */
} LisaEntityTransform;
#pragma pack(pop)

/* Global active panorama backdrop buffer (64KB, 256x256 8bpp) */
extern uint8_t *g_pActivePAN;
extern uint8_t *g_pLisaTransparencyLUT;
extern uint8_t *g_pVirtualFramebuffer;
extern int g_ScreenWidth;
extern int g_ScreenHeight;
extern float g_CameraYaw;
extern float g_CameraPitch;

/* Global state for Lisa 3D triangle rasterizer pipeline */
extern const LisaVertex *g_pLisaVertices;
extern uint8_t *g_pLisaDisplayList;
extern int g_LisaDisplayListCount;
extern uint32_t **g_pLisaDrawQueue;
extern uint32_t **g_pLisaDepthBuckets;
extern LisaViewportClip g_LisaViewport;
extern int32_t g_LisaCullFlag;
extern int32_t g_LisaMipmapTable[16];
extern LisaCamera *g_LisaCamera;
extern const MshPolygon *g_pLisaActivePolygon;

/* Function prototypes */
int Lisa_PrintVersion(void);
int Cdp_OpenFile(CdpFile *cdp);
int Cdp_DecodeFrame(CdpFile *cdp);
int Cdp_DecompressRLE(CdpFile *cdp, unsigned int unused);
void Lisa_RenderPanorama(void);
void Lisa_DrawTexturedTriangle_Op11_Unshaded(void);
void Lisa_DrawTexturedTriangle_Op11_Shaded(void);
int *Lisa_InitEngineMemory(void);
void Lisa_FreeEngineMemory(void);
int Lisa_RenderScene(void);
LisaReturn64 Lisa_ExecuteRasterizerCommands(void);
void Lisa_ResetRasterizerContext(void);

#endif /* LISA3D_H */
