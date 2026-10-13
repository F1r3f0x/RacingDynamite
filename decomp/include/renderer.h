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
    unsigned int texture_u_step; /* VA 0x004BAEAC */
    unsigned int texture_v_step; /* VA 0x004BAEB0 */
    unsigned int intervening_words[3]; /* VA 0x004BAEB4..BC */
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

#endif
