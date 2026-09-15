#ifndef LISA3D_H
#define LISA3D_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
extern uint32_t *g_pLisaDepthBuckets[6000];
extern LisaViewportClip g_LisaViewport;
extern int32_t g_LisaCullFlag;
extern int32_t g_LisaMipmapTable[16];

/* Function prototypes */
int Lisa_PrintVersion(void);
int Cdp_OpenFile(CdpFile *cdp);
int Cdp_DecodeFrame(CdpFile *cdp);
int Cdp_DecompressRLE(CdpFile *cdp, unsigned int unused);
void Lisa_RenderPanorama(void);
void Lisa_DrawTexturedTriangle_Op11_Unshaded(void);
void Lisa_DrawTexturedTriangle_Op11_Shaded(void);

#endif /* LISA3D_H */
