#ifndef IGNITION_RENDERER_H
#define IGNITION_RENDERER_H

/* Reconstruction boundary; original source filename unknown. */
typedef struct GfxTriangleSpanEdgeRecord {
    unsigned int texture_v;
    unsigned int texture_u;
    int x;
} GfxTriangleSpanEdgeRecord;

extern volatile unsigned int g_triangleSpanByteStepStorage; /* VA 0x004BAD60 */
extern volatile unsigned int g_triangleSpanPackedStep; /* VA 0x004BAD64 */
extern volatile unsigned int g_triangleSpanTextureWord; /* VA 0x004BAD68 */
extern volatile unsigned int g_triangleSpanLookupWord; /* VA 0x004BAD6C */
extern volatile unsigned int g_triangleSpanLeftSampleStorage; /* VA 0x004BAD70 */

/* Distances are verified; intervening words are retained, not interpreted here. */
typedef struct GfxTriangleEdgeStepState {
    unsigned int initial_texture_u; /* VA 0x004BAEA4 */
    unsigned int initial_texture_v; /* VA 0x004BAEA8 */
    unsigned int texture_u_step; /* VA 0x004BAEAC */
    unsigned int texture_v_step; /* VA 0x004BAEB0 */
    unsigned int intervening_word; /* VA 0x004BAEB4, not consumed here */
    volatile GfxTriangleSpanEdgeRecord *advanced_corrected_output; /* VA 0x004BAEB8 */
    unsigned int initial_x; /* VA 0x004BAEBC */
    unsigned int x_step; /* VA 0x004BAEC0 */
} GfxTriangleEdgeStepState;

/* Explicit internal representation of original input/output registers. */
typedef struct GfxTriangleEdgeEmission {
    unsigned int texture_v; /* original EAX */
    unsigned int texture_u; /* original ESI */
    unsigned int x; /* original EDX */
    volatile GfxTriangleSpanEdgeRecord *output; /* original EBX */
    unsigned int count; /* original ECX */
} GfxTriangleEdgeEmission;

extern volatile GfxTriangleEdgeStepState g_triangleEdgeSteps;
extern volatile unsigned int g_triangleEdgeUCorrections[8]; /* VA 0x004BADB4 */
extern volatile unsigned int g_triangleEdgeVCorrections[8]; /* VA 0x004BADD4 */
void Gfx_EmitRawTriangleEdgeRecords(GfxTriangleEdgeEmission *emission);
void Gfx_EmitCorrectedTriangleEdgeRecords(GfxTriangleEdgeEmission *emission);
int Gfx_ComputeTriangleEdgeSlope(int x0, int y0, int x1, int y1);

typedef struct GfxTriangleEdgePacket {
    int x0;
    int y0;
    int x1;
    int y1;
    unsigned int texture_u0;
    unsigned int texture_v0;
    unsigned int texture_u1;
    unsigned int texture_v1;
} GfxTriangleEdgePacket;
typedef struct GfxTriangleTextureGradients {
    int texture_u; /* VA 0x004BADA4 */
    int texture_v; /* VA 0x004BADA8 */
} GfxTriangleTextureGradients;
extern volatile GfxTriangleTextureGradients g_triangleTextureGradients;
typedef struct GfxTriangleViewportState {
    int clip_top_fixed; /* VA 0x0063F2B0 */
    int clip_bottom_integer;
    int clip_top_integer;
    int clip_right_integer;
    int clip_left_integer;
    int clip_bottom_fixed;
    int row_stride;
    int clip_left_fixed;
    int clip_right_fixed; /* VA 0x0063F2D0 */
} GfxTriangleViewportState;
typedef struct GfxTriangleEdgeSplitState {
    unsigned int active; /* VA 0x004BAEC8 */
    int saved_x1;
    int saved_y1;
    unsigned int saved_texture_u1;
    unsigned int saved_texture_v1;
} GfxTriangleEdgeSplitState;
extern volatile GfxTriangleViewportState g_triangleViewportState;
extern volatile GfxTriangleEdgeSplitState g_triangleEdgeSplit;
#define g_triangleClipRightFixed (g_triangleViewportState.clip_right_fixed)
void Gfx_ClampTriangleEdgeRightEndpoint(volatile GfxTriangleEdgePacket *edge);
void Gfx_InitializeTriangleEdgeSteps(const volatile GfxTriangleEdgePacket *edge);
void Gfx_ClipAndSplitTriangleEdge(volatile GfxTriangleEdgePacket *edge);

typedef struct GfxTriangleEdgeBufferHeader {
    int start_y;
    int count;
} GfxTriangleEdgeBufferHeader;
typedef struct GfxTriangleEdgeContinuationState {
    int corrected_count; /* VA 0x004BAEDC */
    unsigned int intervening_word; /* VA 0x004BAEE0 */
    volatile GfxTriangleSpanEdgeRecord *raw_advanced_output; /* VA 0x004BAEE4 */
    int raw_count; /* VA 0x004BAEE8 */
} GfxTriangleEdgeContinuationState;
extern volatile GfxTriangleEdgeContinuationState g_triangleEdgeContinuation;
void Gfx_BuildClippedCorrectedTriangleEdge(volatile GfxTriangleEdgePacket *edge,
    volatile GfxTriangleEdgeBufferHeader *output);
void Gfx_BuildClippedRawTriangleEdge(volatile GfxTriangleEdgePacket *edge,
    volatile GfxTriangleEdgeBufferHeader *output);

void Gfx_SetTriangleSpanState(unsigned int gradient_v, unsigned int gradient_u,
    unsigned int texture_word, unsigned int lookup_word);
void Gfx_SetTriangleSpanCore(unsigned int gradient_v, unsigned int gradient_u,
    unsigned int texture_word, unsigned int lookup_word);
void Gfx_DrawTriangleSpan(unsigned char *destination, int count,
    unsigned int packed_uv, unsigned int integer_u, const GfxTriangleSpanEdgeRecord *left);
void Gfx_DrawTriangleSpanCore(unsigned char *destination, int count,
    unsigned int packed_uv, unsigned int integer_u, const GfxTriangleSpanEdgeRecord *left);
void Gfx_DrawClippedTriangleSpan(unsigned char *destination, int count,
    unsigned int packed_uv, unsigned int integer_u);
void Gfx_DrawClippedTriangleSpanCore(unsigned char *destination, int count,
    unsigned int packed_uv, unsigned int integer_u);

void Gfx_BuildCorrectedTriangleEdge(volatile GfxTriangleEdgePacket *edge,
    volatile GfxTriangleEdgeBufferHeader *output);
void Gfx_BuildRawTriangleEdge(volatile GfxTriangleEdgePacket *edge,
    volatile GfxTriangleEdgeBufferHeader *output);

/* Verified shared triangle input and gradient intermediates; original filenames unknown. */
typedef struct GfxTriangleVertexState {
    int x0; /* VA 0x004BAD74 */
    int y0;
    int x1;
    int y1;
    int x2;
    int y2;
    unsigned int texture_u0;
    unsigned int texture_v0;
    unsigned int texture_u1;
    unsigned int texture_v1;
    unsigned int texture_u2;
    unsigned int texture_v2; /* VA 0x004BADA0 */
} GfxTriangleVertexState;
typedef struct GfxTriangleGradientScratch {
    unsigned int height01; /* VA 0x004BADF4 */
    unsigned int height02;
    unsigned int denominator;
    unsigned int delta_x01;
    unsigned int interpolated_x;
    unsigned int interpolated_u;
    unsigned int interpolated_v;
    unsigned int ratio; /* VA 0x004BAE10 */
} GfxTriangleGradientScratch;
extern volatile GfxTriangleVertexState g_triangleVertices;
extern volatile GfxTriangleGradientScratch g_triangleGradientScratch;
void Gfx_ComputeTriangleGradients(void);

typedef struct GfxSpriteState {
    int width;
    int height;
    unsigned int option;
    int clip_left;
    int clip_top;
    int clip_right;
    int clip_bottom;
    int stride;
    int format;
    unsigned char *pixels;
} GfxSpriteState;
extern volatile unsigned char *volatile g_spritePixels; /* VA 0x004BA6F8 */
extern volatile int g_spriteClipLeftFixed; /* VA 0x0050EBF0 */
extern volatile int g_spriteStride; /* VA 0x0050EBF4 */
extern volatile int g_spriteClipRightFixed; /* VA 0x0050EBF8 */
extern volatile int g_spriteClipTopFixed; /* VA 0x0050EC00 */
extern volatile int g_spriteClipBottomFixed; /* VA 0x0050EC04 */
extern volatile int g_spriteClipLeft; /* VA 0x0050ED20 */
extern volatile int g_spriteClipTop; /* VA 0x0050ED24 */
extern volatile int g_spriteClipRight; /* VA 0x0050ED28 */
extern volatile int g_spriteWidth; /* VA 0x0050ED2C */
extern volatile int g_spriteClipBottom; /* VA 0x0050ED30 */
extern volatile unsigned int g_spriteConfigOption; /* VA 0x0050ED34 */
extern volatile int g_spriteHeight; /* VA 0x0050ED38 */
extern volatile int g_triangleViewportHeight; /* VA 0x0063F2AC */
extern volatile unsigned char *volatile g_triangleViewportFramebuffer; /* VA 0x0063F2D4 */

int Gfx_SpriteCloseNative(void);
int Gfx_SpriteOptionNative(unsigned int option);
int Gfx_SpriteConfigureNative(unsigned char *pixels, int stride, int width, int height, unsigned int option);
int Gfx_SpriteSetClipNative(int left, int top, int right, int bottom);
int Gfx_SpriteGetStateNative(GfxSpriteState *state);
int Gfx_SpriteSetStateNative(const GfxSpriteState *state);

/* RVA0x5CA50 cdecl input. Field0 is not consumed; original source/type names unknown. */
typedef struct GfxTexturedTrianglePacket {
    unsigned int unused_word;
    int x0;
    int y0;
    int x1;
    int y1;
    int x2;
    int y2;
    unsigned int texture_u0;
    unsigned int texture_v0;
    unsigned int texture_u1;
    unsigned int texture_v1;
    unsigned int texture_u2;
    unsigned int texture_v2;
    unsigned char *texture;
    unsigned char *lookup;
} GfxTexturedTrianglePacket;
typedef char GfxTrianglePacketMustBe60Bytes[(sizeof(GfxTexturedTrianglePacket) == 60) ? 1 : -1];
extern volatile GfxTriangleEdgePacket g_triangleOrdinaryPacket; /* VA0x0051A9A0 */
extern volatile GfxTriangleEdgePacket g_triangleClippedPacket; /* VA0x0051AA00 */
extern unsigned char *volatile g_triangleRasterTexture; /* VA0x0051A9C4 */
extern volatile unsigned int g_triangleRasterCursor; /* VA0x0051A9D0, wrapped pixel address */
extern volatile int g_triangleRasterMinimumX; /* VA0x0051A9D4 */
extern volatile int g_triangleRasterMaximumX; /* VA0x0051A9F8 */
void Gfx_RasterizeTexturedTriangle(const GfxTexturedTrianglePacket *packet);
void Gfx_RasterizeClippedTriangle(const GfxTexturedTrianglePacket *packet);

/* Verified 116-byte transformed sprite scratch state at VA 0x004BAF1C. */
typedef struct GfxTransformedSpriteScratch {
    int corner0_x; /* VA 0x004BAF1C */
    int corner0_y; /* VA 0x004BAF20 */
    int corner1_x; /* VA 0x004BAF24 */
    int corner1_y; /* VA 0x004BAF28 */
    int corner2_x; /* VA 0x004BAF2C */
    int corner2_y; /* VA 0x004BAF30 */
    int corner3_x; /* VA 0x004BAF34 */
    int corner3_y; /* VA 0x004BAF38 */
    unsigned int param_u0; /* VA 0x004BAF3C */
    unsigned int param_v0; /* VA 0x004BAF40 */
    unsigned int param_u1; /* VA 0x004BAF44 */
    unsigned int param_v1; /* VA 0x004BAF48 */
    unsigned char *lookup;  /* VA 0x004BAF4C */
    unsigned char *texture; /* VA 0x004BAF50 */
    GfxTexturedTrianglePacket packet; /* VA 0x004BAF54 */
} GfxTransformedSpriteScratch;
typedef char GfxTransformedSpriteScratchMustBe116Bytes[(sizeof(GfxTransformedSpriteScratch) == 116) ? 1 : -1];
extern volatile GfxTransformedSpriteScratch g_transformedSpriteScratch;

typedef struct GfxTransformedSpriteInput {
    unsigned int unused_0; /* VA 0x004BAD34 */
    const unsigned int *prepared; /* VA 0x004BAD38 */
    const int *coefficients; /* VA 0x004BAD3C */
    int position_x; /* VA 0x004BAD40 */
    int position_y; /* VA 0x004BAD44 */
} GfxTransformedSpriteInput;
typedef char GfxTransformedSpriteInputMustBe20Bytes[(sizeof(GfxTransformedSpriteInput) == 20) ? 1 : -1];

extern volatile unsigned int g_spriteBlitSource; /* VA 0x004BAD28 */
extern volatile int g_spriteBlitWidth; /* VA 0x004BAD2C */
extern volatile int g_spriteBlitHeight; /* VA 0x004BAD30 */
extern volatile GfxTransformedSpriteInput g_transformedSpriteInput; /* VA 0x004BAD34 */

void Gfx_PrepareTransformedSprite(const GfxTransformedSpriteInput *input);

typedef struct GfxPoint2D {
    int x;
    int y;
} GfxPoint2D;

typedef struct GfxSpriteRasterPacket {
    unsigned int type; /* 0x00 */
    const GfxPoint2D *position; /* 0x04 */
    const unsigned int *prepared; /* 0x08 */
    const int *coefficients; /* 0x0C */
} GfxSpriteRasterPacket;
typedef char GfxSpriteRasterPacketMustBe16Bytes[(sizeof(GfxSpriteRasterPacket) == 16) ? 1 : -1];

void Gfx_RasterizeSpritePacket(const GfxSpriteRasterPacket *packet);

extern volatile unsigned int g_spritePreparedStorage[8]; /* VA 0x004BA758 */
extern volatile GfxSpriteRasterPacket g_spriteRasterPacket; /* VA 0x004BA778 */
extern volatile unsigned int g_spriteRenderLookup; /* VA 0x0063F2D8 */

void Gfx_SubmitSpriteRequest(void);

extern volatile unsigned int g_triangleDrawListStride; /* VA 0x004BA704 */
extern volatile unsigned int g_triangleWorkspacePacket[14]; /* VA 0x004BA718 */

void Gfx_SpriteDrawListCallback(unsigned int header);
void Gfx_SubmitDrawListTriangleRequest(void);
int Gfx_SpriteDrawListNative(const unsigned int *list);
void Gfx_RasterizeWorkspaceATriangle(const GfxTexturedTrianglePacket *packet);

#endif
