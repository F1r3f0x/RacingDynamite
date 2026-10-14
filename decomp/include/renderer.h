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

#endif
