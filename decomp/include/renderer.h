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
