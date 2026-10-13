#include "renderer.h"
#include <stdint.h>

typedef char GfxSpanWordMustBe32Bits[(sizeof(unsigned int) == 4) ? 1 : -1];
typedef char GfxSpanEdgeMustBe12Bytes[(sizeof(GfxTriangleSpanEdgeRecord) == 12) ? 1 : -1];

volatile unsigned int g_triangleSpanByteStepStorage = 0;
volatile unsigned int g_triangleSpanPackedStep = 0;
volatile unsigned int g_triangleSpanTextureWord = 0;
volatile unsigned int g_triangleSpanLookupWord = 0;
volatile unsigned int g_triangleSpanLeftSampleStorage = 0;

volatile GfxTriangleEdgeStepState g_triangleEdgeSteps = {0, 0, {0, 0, 0}, 0};
volatile unsigned int g_triangleEdgeUCorrections[8] = {0, 0, 0, 0, 0, 0, 0, 0};
volatile unsigned int g_triangleEdgeVCorrections[8] = {0, 0, 0, 0, 0, 0, 0, 0};
typedef char GfxEdgeStepsMustBe24Bytes[(sizeof(GfxTriangleEdgeStepState) == 24) ? 1 : -1];
typedef char GfxEdgeEmissionMustBe20Bytes[(sizeof(GfxTriangleEdgeEmission) == 20) ? 1 : -1];

/* Deliberately wrapped addresses from the verified 32-bit Windows ABI. */
#define GFX_SPAN_BYTE(address) (*(volatile unsigned char *)(uintptr_t)(address))
#define GFX_EDGE_SAR(value, shift) (((value) >> (shift)) | ((0u - ((value) >> 31)) << (32 - (shift))))

/* @original Gfx_SetTriangleSpanCore (IGN_WIN.EXE @ 0x00467F1C, unknown original source)
 * @fidelity ADAPTED
 * @deviation DEV-007
 */
void Gfx_SetTriangleSpanCore(unsigned int gradient_v, unsigned int gradient_u,
        unsigned int texture_word, unsigned int lookup_word) {
    unsigned int negative_v;
    unsigned int negative_u;
    unsigned int rotated_u;
    g_triangleSpanLookupWord = lookup_word;
    negative_v = 0u - gradient_v;
    negative_u = 0u - gradient_u;
    negative_v >>= 8;
    g_triangleSpanTextureWord = texture_word;
    rotated_u = (negative_u >> 16) | (negative_u << 16);
    GFX_SPAN_BYTE(&g_triangleSpanByteStepStorage) = (unsigned char)rotated_u;
    g_triangleSpanPackedStep = (rotated_u & 0xffff0000u) | (negative_v & 0xffffu);
}

/* @original Gfx_SetTriangleSpanState (IGN_WIN.EXE @ 0x0045C990, unknown original source)
 * @fidelity EXACT
 */
void Gfx_SetTriangleSpanState(unsigned int gradient_v, unsigned int gradient_u,
        unsigned int texture_word, unsigned int lookup_word) {
    Gfx_SetTriangleSpanCore(gradient_v, gradient_u, texture_word, lookup_word);
}

/* @original Gfx_DrawTriangleSpanCore (IGN_WIN.EXE @ 0x00467F42, unknown original source)
 * @fidelity ADAPTED
 * @deviation DEV-007
 */
void Gfx_DrawTriangleSpanCore(unsigned char *destination, int count,
        unsigned int packed_uv, unsigned int integer_u,
        const GfxTriangleSpanEdgeRecord *left) {
    unsigned int texture;
    unsigned int lookup;
    unsigned int byte_step;
    unsigned int packed_step;
    unsigned int remaining;
    unsigned int previous_uv;
    unsigned int carry;
    unsigned int address;
    unsigned char source_byte;
    unsigned char destination_byte;
    unsigned char result;
    texture = g_triangleSpanTextureWord;
    texture = (texture & 0xffffff00u) | GFX_SPAN_BYTE((uintptr_t)left + 4u);
    texture = (texture & 0xffff00ffu) | ((unsigned int)GFX_SPAN_BYTE(left) << 8);
    source_byte = GFX_SPAN_BYTE(texture);
    GFX_SPAN_BYTE(&g_triangleSpanLeftSampleStorage) = source_byte;
    texture = (texture & 0xffff0000u) | (integer_u & 0xffffu);
    byte_step = GFX_SPAN_BYTE(&g_triangleSpanByteStepStorage);
    packed_step = g_triangleSpanPackedStep;
    lookup = g_triangleSpanLookupWord;
    remaining = (unsigned int)count - 1u;
    while ((remaining & 0x80000000u) == 0u) {
        previous_uv = packed_uv;
        packed_uv += packed_step;
        carry = packed_uv < previous_uv;
        texture = (texture & 0xffff00ffu) | (packed_uv & 0xff00u);
        texture = (texture & 0xffffff00u) | ((texture + byte_step + carry) & 0xffu);
        address = (unsigned int)(uintptr_t)destination + remaining;
        destination_byte = GFX_SPAN_BYTE(address);
        source_byte = GFX_SPAN_BYTE(texture);
        lookup = (lookup & 0xffff0000u) | ((unsigned int)source_byte << 8) | destination_byte;
        result = GFX_SPAN_BYTE(lookup);
        GFX_SPAN_BYTE(address) = result;
        remaining--;
    }
    if (remaining == 0xffffffffu) {
        source_byte = GFX_SPAN_BYTE(&g_triangleSpanLeftSampleStorage);
        address = (unsigned int)(uintptr_t)destination + remaining;
        destination_byte = GFX_SPAN_BYTE(address);
        lookup = (lookup & 0xffff0000u) | ((unsigned int)source_byte << 8) | destination_byte;
        result = GFX_SPAN_BYTE(lookup);
        GFX_SPAN_BYTE(address) = result;
    }
}

/* @original Gfx_DrawTriangleSpan (IGN_WIN.EXE @ 0x0045C9B0, unknown original source)
 * @fidelity EXACT
 */
void Gfx_DrawTriangleSpan(unsigned char *destination, int count,
        unsigned int packed_uv, unsigned int integer_u,
        const GfxTriangleSpanEdgeRecord *left) {
    Gfx_DrawTriangleSpanCore(destination, count, packed_uv, integer_u, left);
}

/* @original Gfx_DrawClippedTriangleSpanCore (IGN_WIN.EXE @ 0x00467F97, unknown original source)
 * @fidelity ADAPTED
 * @deviation DEV-007
 */
void Gfx_DrawClippedTriangleSpanCore(unsigned char *destination, int count,
        unsigned int packed_uv, unsigned int integer_u) {
    unsigned int texture;
    unsigned int lookup;
    unsigned int byte_step;
    unsigned int packed_step;
    unsigned int remaining;
    unsigned int previous_uv;
    unsigned int carry;
    unsigned int address;
    unsigned char source_byte;
    unsigned char destination_byte;
    unsigned char result;
    texture = (g_triangleSpanTextureWord & 0xffff0000u) | (integer_u & 0xffffu);
    byte_step = GFX_SPAN_BYTE(&g_triangleSpanByteStepStorage);
    packed_step = g_triangleSpanPackedStep;
    lookup = g_triangleSpanLookupWord;
    remaining = (unsigned int)count - 1u;
    while ((remaining & 0x80000000u) == 0u) {
        previous_uv = packed_uv;
        packed_uv += packed_step;
        carry = packed_uv < previous_uv;
        texture = (texture & 0xffff00ffu) | (packed_uv & 0xff00u);
        texture = (texture & 0xffffff00u) | ((texture + byte_step + carry) & 0xffu);
        address = (unsigned int)(uintptr_t)destination + remaining;
        source_byte = GFX_SPAN_BYTE(texture);
        destination_byte = GFX_SPAN_BYTE(address);
        lookup = (lookup & 0xffff0000u) | ((unsigned int)source_byte << 8) | destination_byte;
        result = GFX_SPAN_BYTE(lookup);
        GFX_SPAN_BYTE(address) = result;
        remaining--;
    }
}

/* @original Gfx_DrawClippedTriangleSpan (IGN_WIN.EXE @ 0x0045C9D0, unknown original source)
 * @fidelity EXACT
 */
void Gfx_DrawClippedTriangleSpan(unsigned char *destination, int count,
        unsigned int packed_uv, unsigned int integer_u) {
    Gfx_DrawClippedTriangleSpanCore(destination, count, packed_uv, integer_u);
}

/* @original Gfx_EmitRawTriangleEdgeRecords (IGN_WIN.EXE @ 0x0064F1E0, unknown original source)
 * @fidelity ADAPTED
 * @deviation DEV-007
 */
void Gfx_EmitRawTriangleEdgeRecords(GfxTriangleEdgeEmission *emission) {
    unsigned int texture_v;
    unsigned int texture_u;
    unsigned int x;
    unsigned int count;
    unsigned int previous_low_count;
    unsigned int x_step;
    volatile GfxTriangleSpanEdgeRecord *output;
    texture_v = emission->texture_v;
    texture_u = emission->texture_u;
    x = emission->x;
    output = emission->output;
    count = emission->count;
    do {
        output->texture_u = GFX_EDGE_SAR(texture_u, 16);
        texture_u += g_triangleEdgeSteps.texture_u_step;
        output->texture_v = GFX_EDGE_SAR(texture_v, 16);
        texture_v += g_triangleEdgeSteps.texture_v_step;
        x_step = g_triangleEdgeSteps.x_step;
        output->x = (int)GFX_EDGE_SAR(x, 16);
        x += x_step;
        output = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)
            ((unsigned int)(uintptr_t)output + 12u);
        previous_low_count = count & 0xffffu;
        count = (count & 0xffff0000u) | ((count - 1u) & 0xffffu);
        /* DEC CX/JG: use the signed mathematical decrement, including OF. */
    } while (previous_low_count > 1u && previous_low_count < 0x8000u);
    emission->texture_v = texture_v;
    emission->texture_u = texture_u;
    emission->x = x;
    emission->output = output;
    emission->count = count;
}

/* @original Gfx_EmitCorrectedTriangleEdgeRecords (IGN_WIN.EXE @ 0x0064F220, unknown original source)
 * @fidelity ADAPTED
 * @deviation DEV-007
 */
void Gfx_EmitCorrectedTriangleEdgeRecords(GfxTriangleEdgeEmission *emission) {
    unsigned int texture_v;
    unsigned int texture_u;
    unsigned int x;
    unsigned int count;
    unsigned int previous_count;
    unsigned int bucket;
    unsigned int corrected;
    unsigned int x_step;
    volatile GfxTriangleSpanEdgeRecord *output;
    texture_v = emission->texture_v;
    texture_u = emission->texture_u;
    x = emission->x;
    output = emission->output;
    count = emission->count;
    do {
        bucket = (x >> 13) & 7u;
        corrected = texture_u + g_triangleEdgeUCorrections[bucket];
        output->texture_u = GFX_EDGE_SAR(corrected, 8);
        texture_u += g_triangleEdgeSteps.texture_u_step;
        corrected = texture_v + g_triangleEdgeVCorrections[bucket];
        output->texture_v = GFX_EDGE_SAR(corrected, 8);
        texture_v += g_triangleEdgeSteps.texture_v_step;
        x_step = g_triangleEdgeSteps.x_step;
        output->x = (int)GFX_EDGE_SAR(x, 16);
        x += x_step;
        output = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)
            ((unsigned int)(uintptr_t)output + 12u);
        previous_count = count;
        count--;
        /* DEC ECX/JG rejects signed-negative input even when decrement wraps. */
    } while (previous_count > 1u && previous_count < 0x80000000u);
    emission->texture_v = texture_v;
    emission->texture_u = texture_u;
    emission->x = x;
    emission->output = output;
    emission->count = count;
}
