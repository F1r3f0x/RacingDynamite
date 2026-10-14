#include "renderer.h"
#include <stdint.h>

typedef char GfxSpanWordMustBe32Bits[(sizeof(unsigned int) == 4) ? 1 : -1];
typedef char GfxSpanEdgeMustBe12Bytes[(sizeof(GfxTriangleSpanEdgeRecord) == 12) ? 1 : -1];

volatile unsigned int g_triangleSpanByteStepStorage = 0;
volatile unsigned int g_triangleSpanPackedStep = 0;
volatile unsigned int g_triangleSpanTextureWord = 0;
volatile unsigned int g_triangleSpanLookupWord = 0;
volatile unsigned int g_triangleSpanLeftSampleStorage = 0;

volatile GfxTriangleEdgeStepState g_triangleEdgeSteps = {0, 0, 0, 0, 0, 0, 0, 0};
volatile unsigned int g_triangleEdgeUCorrections[8] = {0, 0, 0, 0, 0, 0, 0, 0};
volatile unsigned int g_triangleEdgeVCorrections[8] = {0, 0, 0, 0, 0, 0, 0, 0};
typedef char GfxEdgeStepsMustBe32Bytes[(sizeof(GfxTriangleEdgeStepState) == 32) ? 1 : -1];
typedef char GfxEdgeEmissionMustBe20Bytes[(sizeof(GfxTriangleEdgeEmission) == 20) ? 1 : -1];
volatile GfxTriangleTextureGradients g_triangleTextureGradients = {0, 0};
volatile GfxTriangleViewportState g_triangleViewportState = {0, 0, 0, 0, 0, 0, 0, 0, 0};
volatile GfxTriangleEdgeSplitState g_triangleEdgeSplit = {0, 0, 0, 0, 0};
typedef char GfxViewportMustBe36Bytes[(sizeof(GfxTriangleViewportState) == 36) ? 1 : -1];
typedef char GfxSplitMustBe20Bytes[(sizeof(GfxTriangleEdgeSplitState) == 20) ? 1 : -1];
volatile GfxTriangleEdgeContinuationState g_triangleEdgeContinuation = {0, 0, 0, 0};
typedef char GfxEdgeHeaderMustBe8Bytes[(sizeof(GfxTriangleEdgeBufferHeader) == 8) ? 1 : -1];
typedef char GfxEdgeContinuationMustBe16Bytes[(sizeof(GfxTriangleEdgeContinuationState) == 16) ? 1 : -1];
typedef char GfxEdgePacketMustBe32Bytes[(sizeof(GfxTriangleEdgePacket) == 32) ? 1 : -1];
typedef char GfxTextureGradientsMustBe8Bytes[(sizeof(GfxTriangleTextureGradients) == 8) ? 1 : -1];

volatile GfxTriangleVertexState g_triangleVertices = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
volatile GfxTriangleGradientScratch g_triangleGradientScratch = {0, 0, 0, 0, 0, 0, 0, 0};
typedef char GfxTriangleVerticesMustBe48Bytes[(sizeof(GfxTriangleVertexState) == 48) ? 1 : -1];
typedef char GfxTriangleGradientScratchMustBe32Bytes[(sizeof(GfxTriangleGradientScratch) == 32) ? 1 : -1];

/* Deliberately wrapped addresses from the verified 32-bit Windows ABI. */
#define GFX_SPAN_BYTE(address) (*(volatile unsigned char *)(uintptr_t)(address))
#define GFX_EDGE_SAR(value, shift) (((value) >> (shift)) | ((0u - ((value) >> 31)) << (32 - (shift))))
/* Signed 32x32 product bits 16..47, expressed in C89 unsigned halfwords.
 * Inputs are captured local words; sign corrections subtract multiples of 2^32.
 */
#define GFX_EDGE_MUL16(a, b) \
    (((((a) & 0xffffu) * ((b) & 0xffffu)) >> 16) \
    + ((a) & 0xffffu) * ((b) >> 16) \
    + ((a) >> 16) * ((b) & 0xffffu) \
    + ((((a) >> 16) * ((b) >> 16)) << 16) \
    - (((0u - ((a) >> 31)) & (b)) << 16) \
    - (((0u - ((b) >> 31)) & (a)) << 16))

/* Signed paired-word numerator / signed word, returning a signed-word bit pattern.
 * Restoring division avoids a missing wide runtime. Invalid quotients deliberately
 * reach an actual i386 IDIV fault under the verified provisional compiler.
 */
#define GFX_EDGE_IDIV_WORDS(result, numerator_high, numerator_low, divisor_word) \
    do { \
        unsigned int divide_high = (numerator_high); \
        unsigned int divide_low = (numerator_low); \
        unsigned int divide_denominator = (divisor_word); \
        unsigned int numerator_negative = divide_high >> 31; \
        unsigned int quotient_negative = numerator_negative ^ (divide_denominator >> 31); \
        unsigned int divide_invalid; \
        unsigned int divide_remainder; \
        unsigned int divide_quotient = 0; \
        unsigned int divide_carry; \
        unsigned int divide_bit; \
        volatile int fault_numerator; \
        volatile int fault_denominator; \
        if (numerator_negative != 0) { \
            divide_high = 0u - divide_high - (divide_low != 0); \
            divide_low = 0u - divide_low; \
        } \
        if ((divide_denominator & 0x80000000u) != 0) { \
            divide_denominator = 0u - divide_denominator; \
        } \
        divide_invalid = divide_denominator == 0 || divide_high >= divide_denominator; \
        divide_remainder = divide_high; \
        if (divide_invalid == 0) { \
            for (divide_bit = 0; divide_bit < 32; divide_bit++) { \
                divide_carry = divide_remainder >> 31; \
                divide_remainder = (divide_remainder << 1) | (divide_low >> 31); \
                divide_low <<= 1; \
                divide_quotient <<= 1; \
                if (divide_carry != 0 || divide_remainder >= divide_denominator) { \
                    divide_remainder -= divide_denominator; \
                    divide_quotient |= 1u; \
                } \
            } \
            divide_invalid = divide_quotient > \
                (quotient_negative != 0 ? 0x80000000u : 0x7fffffffu); \
        } \
        if (divide_invalid != 0) { \
            fault_numerator = (int)divide_high; \
            fault_denominator = 0; \
            (result) = (unsigned int)(fault_numerator / fault_denominator); \
        } else { \
            (result) = quotient_negative != 0 ? 0u - divide_quotient : divide_quotient; \
        } \
    } while (0)

#define GFX_EDGE_UDIV_WORDS(result, numerator_high, numerator_low, divisor_word) \
    do { \
        unsigned int unsigned_high = (numerator_high); \
        unsigned int unsigned_low = (numerator_low); \
        unsigned int unsigned_denominator = (divisor_word); \
        unsigned int unsigned_quotient = 0; \
        unsigned int unsigned_carry; \
        unsigned int unsigned_bit; \
        volatile unsigned int fault_numerator; \
        volatile unsigned int fault_denominator; \
        if (unsigned_denominator == 0 || unsigned_high >= unsigned_denominator) { \
            fault_numerator = unsigned_high; \
            fault_denominator = 0; \
            (result) = fault_numerator / fault_denominator; \
        } else { \
            for (unsigned_bit = 0; unsigned_bit < 32; unsigned_bit++) { \
                unsigned_carry = unsigned_high >> 31; \
                unsigned_high = (unsigned_high << 1) | (unsigned_low >> 31); \
                unsigned_low <<= 1; \
                unsigned_quotient <<= 1; \
                if (unsigned_carry != 0 || unsigned_high >= unsigned_denominator) { \
                    unsigned_high -= unsigned_denominator; \
                    unsigned_quotient |= 1u; \
                } \
            } \
            (result) = unsigned_quotient; \
        } \
    } while (0)

#define GFX_EDGE_PRODUCT_HIGH(result, product_a, product_b) \
    do { \
        unsigned int a_word = (product_a); \
        unsigned int b_word = (product_b); \
        unsigned int a_low = a_word & 65535u; \
        unsigned int a_high = a_word >> 16; \
        unsigned int b_low = b_word & 65535u; \
        unsigned int b_high = b_word >> 16; \
        unsigned int low_product = a_low * b_low; \
        unsigned int cross_a = a_low * b_high; \
        unsigned int cross_b = a_high * b_low; \
        unsigned int product_carry = (low_product >> 16) \
            + (cross_a & 65535u) + (cross_b & 65535u); \
        (result) = a_high * b_high + (cross_a >> 16) + (cross_b >> 16) \
            + (product_carry >> 16) - ((0u - (a_word >> 31)) & b_word) \
            - ((0u - (b_word >> 31)) & a_word); \
    } while (0)

/* Inputs are volatile fields: capture end, start, then reread the destination. */
#define GFX_EDGE_INTERPOLATE(destination, start, end, ratio, subtract) \
    do { \
        volatile unsigned int interpolation_delta = (unsigned int)(end); \
        unsigned int interpolation_high; \
        volatile unsigned int interpolation_old; \
        interpolation_delta -= (unsigned int)(start); \
        GFX_EDGE_PRODUCT_HIGH(interpolation_high, interpolation_delta << 1, (ratio)); \
        interpolation_old = (unsigned int)(destination); \
        (destination) = (subtract) ? interpolation_old - interpolation_high \
            : interpolation_old + interpolation_high; \
    } while (0)

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

/* @original Gfx_ComputeTriangleEdgeSlope (IGN_WIN.EXE @ 0x0045D7A0, unknown original source)
 * @fidelity EXACT
 */
int Gfx_ComputeTriangleEdgeSlope(int x0, int y0, int x1, int y1) {
    unsigned int numerator_word;
    unsigned int denominator_word;
    if (y0 == y1) {
        if (x1 < x0) {
            return (int)0x80010000u;
        }
        return 0x7fff0000;
    }
    numerator_word = ((unsigned int)x0 - (unsigned int)x1) << 8;
    denominator_word = (unsigned int)y0 - (unsigned int)y1;
    /* Verified i386 casts/IDIV retain original signed quotient and #DE behavior. */
    return (int)numerator_word / (int)denominator_word;
}

/* @original Gfx_ClampTriangleEdgeRightEndpoint (IGN_WIN.EXE @ 0x0064F489, unknown original source)
 * @fidelity ADAPTED
 * @deviation DEV-007
 */
void Gfx_ClampTriangleEdgeRightEndpoint(volatile GfxTriangleEdgePacket *edge) {
    volatile int endpoint_x;
    volatile int clip_right;
    unsigned int distance;
    unsigned int gradient_word;
    unsigned int correction;
    unsigned int endpoint_texture;
    endpoint_x = edge->x1;
    clip_right = g_triangleClipRightFixed;
    if (clip_right >= endpoint_x) {
        return;
    }
    distance = (unsigned int)edge->x1;
    distance -= (unsigned int)g_triangleClipRightFixed;
    gradient_word = (unsigned int)g_triangleTextureGradients.texture_u;
    correction = GFX_EDGE_MUL16(distance, gradient_word);
    endpoint_texture = edge->texture_u1;
    edge->texture_u1 = endpoint_texture - correction;
    gradient_word = (unsigned int)g_triangleTextureGradients.texture_v;
    correction = GFX_EDGE_MUL16(distance, gradient_word);
    endpoint_texture = edge->texture_v1;
    edge->texture_v1 = endpoint_texture - correction;
    clip_right = g_triangleClipRightFixed;
    edge->x1 = clip_right;
}

/* @original Gfx_InitializeTriangleEdgeSteps (IGN_WIN.EXE @ 0x0064F4D0, unknown original source)
 * @fidelity ADAPTED
 * @deviation DEV-007
 */
void Gfx_InitializeTriangleEdgeSteps(const volatile GfxTriangleEdgePacket *edge) {
    volatile unsigned int denominator;
    volatile unsigned int delta;
    unsigned int magnitude;
    unsigned int numerator_high;
    unsigned int numerator_low;
    unsigned int quotient;
    unsigned int fraction;
    volatile unsigned int coordinate;
    unsigned int step;
    unsigned int adjustment;
    denominator = (unsigned int)edge->y1;
    denominator -= (unsigned int)edge->y0;
    if (denominator <= 64u) {
        delta = (unsigned int)edge->x1;
        delta -= (unsigned int)edge->x0;
        magnitude = (delta & 0x80000000u) != 0 ? 0u - delta : delta;
        if (magnitude <= 65536u) {
            denominator = 65u;
        } else if (denominator <= 3u) {
            denominator = 3u;
        }
    }
    delta = edge->texture_v1;
    delta -= edge->texture_v0;
    numerator_high = 0u - (delta >> 31);
    numerator_low = delta << 16;
    GFX_EDGE_IDIV_WORDS(quotient, numerator_high, numerator_low, denominator);
    g_triangleEdgeSteps.texture_v_step = quotient;
    delta = edge->texture_u1;
    delta -= edge->texture_u0;
    numerator_high = 0u - (delta >> 31);
    numerator_low = delta << 16;
    GFX_EDGE_IDIV_WORDS(quotient, numerator_high, numerator_low, denominator);
    g_triangleEdgeSteps.texture_u_step = quotient;
    delta = (unsigned int)edge->x1;
    delta -= (unsigned int)edge->x0;
    numerator_high = GFX_EDGE_SAR(delta, 16);
    numerator_low = delta << 16;
    GFX_EDGE_IDIV_WORDS(quotient, numerator_high, numerator_low, denominator);
    g_triangleEdgeSteps.x_step = quotient;
    fraction = 256u - ((unsigned int)edge->y0 & 255u);
    coordinate = edge->texture_u0 << 8;
    step = g_triangleEdgeSteps.texture_u_step;
    adjustment = fraction * step;
    g_triangleEdgeSteps.initial_texture_u = coordinate + GFX_EDGE_SAR(adjustment, 8);
    coordinate = edge->texture_v0 << 8;
    step = g_triangleEdgeSteps.texture_v_step;
    adjustment = fraction * step;
    g_triangleEdgeSteps.initial_texture_v = coordinate + GFX_EDGE_SAR(adjustment, 8);
    coordinate = (unsigned int)edge->x0 << 8;
    step = g_triangleEdgeSteps.x_step;
    g_triangleEdgeSteps.initial_x = coordinate + fraction * GFX_EDGE_SAR(step, 8);
}

/* @original Gfx_ClipAndSplitTriangleEdge (IGN_WIN.EXE @ 0x0064F270, unknown original source)
 * @fidelity ADAPTED
 * @deviation DEV-007
 */
void Gfx_ClipAndSplitTriangleEdge(volatile GfxTriangleEdgePacket *edge) {
    volatile int coordinate;
    volatile int bound;
    volatile unsigned int denominator;
    volatile unsigned int distance;
    unsigned int ratio;
    unsigned int correction;
    unsigned int gradient_word;
    volatile unsigned int old_texture;
    coordinate = edge->y0;
    bound = g_triangleViewportState.clip_top_fixed;
    if (bound > coordinate) {
        denominator = (unsigned int)edge->y1;
        denominator -= (unsigned int)coordinate;
        distance = (unsigned int)g_triangleViewportState.clip_top_fixed;
        distance -= (unsigned int)coordinate;
        GFX_EDGE_UDIV_WORDS(ratio, distance, 0, denominator);
        ratio >>= 1;
        GFX_EDGE_INTERPOLATE(edge->texture_v0, edge->texture_v0, edge->texture_v1, ratio, 0);
        GFX_EDGE_INTERPOLATE(edge->texture_u0, edge->texture_u0, edge->texture_u1, ratio, 0);
        GFX_EDGE_INTERPOLATE(edge->x0, edge->x0, edge->x1, ratio, 0);
        bound = g_triangleViewportState.clip_top_fixed;
        edge->y0 = bound;
    }
    coordinate = edge->y1;
    bound = g_triangleViewportState.clip_bottom_fixed;
    if (bound < coordinate) {
        denominator = (unsigned int)coordinate;
        denominator -= (unsigned int)edge->y0;
        distance = (unsigned int)coordinate;
        distance -= (unsigned int)g_triangleViewportState.clip_bottom_fixed;
        GFX_EDGE_UDIV_WORDS(ratio, distance, 0, denominator);
        ratio >>= 1;
        GFX_EDGE_INTERPOLATE(edge->texture_v1, edge->texture_v0, edge->texture_v1, ratio, 1);
        GFX_EDGE_INTERPOLATE(edge->texture_u1, edge->texture_u0, edge->texture_u1, ratio, 1);
        GFX_EDGE_INTERPOLATE(edge->x1, edge->x0, edge->x1, ratio, 1);
        bound = g_triangleViewportState.clip_bottom_fixed;
        edge->y1 = bound;
    }
    g_triangleEdgeSplit.active = 0;
    coordinate = edge->x0;
    bound = g_triangleClipRightFixed;
    if (bound < coordinate) {
        coordinate = edge->x1;
        bound = g_triangleClipRightFixed;
        if (bound <= coordinate) {
            distance = (unsigned int)edge->x1;
            distance -= (unsigned int)g_triangleClipRightFixed;
            gradient_word = (unsigned int)g_triangleTextureGradients.texture_u;
            correction = GFX_EDGE_MUL16(distance, gradient_word);
            old_texture = edge->texture_u1;
            edge->texture_u1 = old_texture - correction;
            gradient_word = (unsigned int)g_triangleTextureGradients.texture_v;
            correction = GFX_EDGE_MUL16(distance, gradient_word);
            old_texture = edge->texture_v1;
            edge->texture_v1 = old_texture - correction;
            bound = g_triangleClipRightFixed;
            edge->x1 = bound;
        } else {
            g_triangleEdgeSplit.active = 1;
            coordinate = edge->x1;
            g_triangleEdgeSplit.saved_x1 = coordinate;
            coordinate = edge->y1;
            g_triangleEdgeSplit.saved_y1 = coordinate;
            old_texture = edge->texture_u1;
            g_triangleEdgeSplit.saved_texture_u1 = old_texture;
            old_texture = edge->texture_v1;
            g_triangleEdgeSplit.saved_texture_v1 = old_texture;
            denominator = (unsigned int)edge->x0;
            distance = (unsigned int)g_triangleClipRightFixed;
            coordinate = edge->x1;
            denominator -= (unsigned int)coordinate;
            distance -= (unsigned int)coordinate;
            GFX_EDGE_UDIV_WORDS(ratio, distance, 0, denominator);
            ratio >>= 1;
            GFX_EDGE_INTERPOLATE(edge->texture_u1, edge->texture_u0, edge->texture_u1, ratio, 1);
            GFX_EDGE_INTERPOLATE(edge->texture_v1, edge->texture_v0, edge->texture_v1, ratio, 1);
            GFX_EDGE_INTERPOLATE(edge->y1, edge->y0, edge->y1, ratio, 1);
            bound = g_triangleClipRightFixed;
            edge->x1 = bound;
        }
        distance = (unsigned int)edge->x0;
        distance -= (unsigned int)g_triangleClipRightFixed;
        gradient_word = (unsigned int)g_triangleTextureGradients.texture_u;
        correction = GFX_EDGE_MUL16(distance, gradient_word);
        old_texture = edge->texture_u0;
        edge->texture_u0 = old_texture - correction;
        gradient_word = (unsigned int)g_triangleTextureGradients.texture_v;
        correction = GFX_EDGE_MUL16(distance, gradient_word);
        old_texture = edge->texture_v0;
        edge->texture_v0 = old_texture - correction;
        bound = g_triangleClipRightFixed;
        edge->x0 = bound;
    }
    /* Original outside-right path falls through into this live second check. */
    coordinate = edge->x1;
    bound = g_triangleClipRightFixed;
    if (bound < coordinate) {
        g_triangleEdgeSplit.active = 1;
        coordinate = edge->x1;
        g_triangleEdgeSplit.saved_x1 = coordinate;
        coordinate = edge->y1;
        g_triangleEdgeSplit.saved_y1 = coordinate;
        old_texture = edge->texture_u1;
        g_triangleEdgeSplit.saved_texture_u1 = old_texture;
        old_texture = edge->texture_v1;
        g_triangleEdgeSplit.saved_texture_v1 = old_texture;
        coordinate = edge->x1;
        distance = (unsigned int)coordinate;
        denominator = (unsigned int)coordinate;
        distance -= (unsigned int)g_triangleClipRightFixed;
        denominator -= (unsigned int)edge->x0;
        if (distance == denominator) {
            ratio = 0xffffffffu;
        } else {
            GFX_EDGE_UDIV_WORDS(ratio, distance, 0, denominator);
        }
        ratio >>= 1;
        GFX_EDGE_INTERPOLATE(edge->texture_u1, edge->texture_u0, edge->texture_u1, ratio, 1);
        GFX_EDGE_INTERPOLATE(edge->texture_v1, edge->texture_v0, edge->texture_v1, ratio, 1);
        GFX_EDGE_INTERPOLATE(edge->y1, edge->y0, edge->y1, ratio, 1);
        bound = g_triangleClipRightFixed;
        edge->x1 = bound;
    }
}

/* @original Gfx_BuildClippedCorrectedTriangleEdge (IGN_WIN.EXE @ 0x0064EFA0, unknown original source)
 * @fidelity ADAPTED
 * @deviation DEV-007
 */
void Gfx_BuildClippedCorrectedTriangleEdge(volatile GfxTriangleEdgePacket *edge,
        volatile GfxTriangleEdgeBufferHeader *output) {
    volatile int endpoint_y;
    volatile int bound;
    unsigned int start_y;
    unsigned int end_y;
    volatile unsigned int count;
    volatile unsigned int texture;
    volatile int coordinate;
    GfxTriangleEdgeEmission emission;
    endpoint_y = edge->y1;
    bound = g_triangleViewportState.clip_top_fixed;
    if (bound >= endpoint_y) {
        output->count = 0;
        bound = g_triangleViewportState.clip_top_integer;
        output->start_y = bound;
        return;
    }
    endpoint_y = edge->y0;
    bound = g_triangleViewportState.clip_bottom_fixed;
    if (bound <= endpoint_y) {
        output->count = 0;
        bound = g_triangleViewportState.clip_bottom_integer;
        output->start_y = bound;
        return;
    }
    Gfx_ClipAndSplitTriangleEdge(edge);
    g_triangleEdgeSteps.advanced_corrected_output = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)
        ((unsigned int)(uintptr_t)output + 8u);
    start_y = (unsigned int)edge->y0;
    start_y = GFX_EDGE_SAR(start_y, 8);
    output->start_y = (int)start_y;
    end_y = (unsigned int)edge->y1;
    end_y = GFX_EDGE_SAR(end_y, 8);
    count = end_y - start_y;
    output->count = (int)count;
    if (count != 0) {
        Gfx_InitializeTriangleEdgeSteps(edge);
        emission.count = (unsigned int)output->count;
        emission.texture_v = g_triangleEdgeSteps.initial_texture_v;
        emission.x = g_triangleEdgeSteps.initial_x;
        emission.texture_u = g_triangleEdgeSteps.initial_texture_u;
        emission.output = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)
            ((unsigned int)(uintptr_t)output + 8u);
        Gfx_EmitCorrectedTriangleEdgeRecords(&emission);
        g_triangleEdgeSteps.advanced_corrected_output = emission.output;
    }
    if (g_triangleEdgeSplit.active != 1u) {
        return;
    }
    coordinate = edge->x1;
    edge->x0 = coordinate;
    coordinate = edge->y1;
    edge->y0 = coordinate;
    texture = edge->texture_u1;
    edge->texture_u0 = texture;
    texture = edge->texture_v1;
    edge->texture_v0 = texture;
    coordinate = g_triangleEdgeSplit.saved_x1;
    edge->x1 = coordinate;
    coordinate = g_triangleEdgeSplit.saved_y1;
    edge->y1 = coordinate;
    texture = g_triangleEdgeSplit.saved_texture_u1;
    edge->texture_u1 = texture;
    texture = g_triangleEdgeSplit.saved_texture_v1;
    edge->texture_v1 = texture;
    Gfx_ClampTriangleEdgeRightEndpoint(edge);
    end_y = (unsigned int)edge->y1;
    start_y = (unsigned int)edge->y0;
    end_y = GFX_EDGE_SAR(end_y, 8);
    start_y = GFX_EDGE_SAR(start_y, 8);
    count = end_y - start_y;
    g_triangleEdgeContinuation.corrected_count = (int)count;
    if (count != 0) {
        output->count = (int)((unsigned int)output->count + count);
        Gfx_InitializeTriangleEdgeSteps(edge);
        emission.count = (unsigned int)g_triangleEdgeContinuation.corrected_count;
        emission.texture_v = g_triangleEdgeSteps.initial_texture_v;
        emission.x = g_triangleEdgeSteps.initial_x;
        emission.texture_u = g_triangleEdgeSteps.initial_texture_u;
        emission.output = g_triangleEdgeSteps.advanced_corrected_output;
        Gfx_EmitCorrectedTriangleEdgeRecords(&emission);
    }
}

/* @original Gfx_BuildClippedRawTriangleEdge (IGN_WIN.EXE @ 0x0064F0C0, unknown original source)
 * @fidelity ADAPTED
 * @deviation DEV-007
 */
void Gfx_BuildClippedRawTriangleEdge(volatile GfxTriangleEdgePacket *edge,
        volatile GfxTriangleEdgeBufferHeader *output) {
    volatile int endpoint_y;
    volatile int bound;
    unsigned int start_y;
    unsigned int end_y;
    volatile unsigned int count;
    volatile unsigned int texture;
    volatile int coordinate;
    GfxTriangleEdgeEmission emission;
    endpoint_y = edge->y1;
    bound = g_triangleViewportState.clip_top_fixed;
    if (bound >= endpoint_y) {
        output->count = 0;
        bound = g_triangleViewportState.clip_top_integer;
        output->start_y = bound;
        return;
    }
    endpoint_y = edge->y0;
    bound = g_triangleViewportState.clip_bottom_fixed;
    if (bound <= endpoint_y) {
        output->count = 0;
        bound = g_triangleViewportState.clip_bottom_integer;
        output->start_y = bound;
        return;
    }
    Gfx_ClipAndSplitTriangleEdge(edge);
    g_triangleEdgeContinuation.raw_advanced_output = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)
        ((unsigned int)(uintptr_t)output + 8u);
    start_y = (unsigned int)edge->y0;
    start_y = GFX_EDGE_SAR(start_y, 8);
    output->start_y = (int)start_y;
    end_y = (unsigned int)edge->y1;
    end_y = GFX_EDGE_SAR(end_y, 8);
    count = end_y - start_y;
    output->count = (int)count;
    if (count != 0) {
        Gfx_InitializeTriangleEdgeSteps(edge);
        emission.count = (unsigned int)output->count;
        emission.texture_v = g_triangleEdgeSteps.initial_texture_v;
        emission.x = g_triangleEdgeSteps.initial_x;
        emission.texture_u = g_triangleEdgeSteps.initial_texture_u;
        emission.output = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)
            ((unsigned int)(uintptr_t)output + 8u);
        Gfx_EmitRawTriangleEdgeRecords(&emission);
        g_triangleEdgeContinuation.raw_advanced_output = emission.output;
    }
    if (g_triangleEdgeSplit.active != 1u) {
        return;
    }
    coordinate = edge->x1;
    edge->x0 = coordinate;
    coordinate = edge->y1;
    edge->y0 = coordinate;
    texture = edge->texture_u1;
    edge->texture_u0 = texture;
    texture = edge->texture_v1;
    edge->texture_v0 = texture;
    coordinate = g_triangleEdgeSplit.saved_x1;
    edge->x1 = coordinate;
    coordinate = g_triangleEdgeSplit.saved_y1;
    edge->y1 = coordinate;
    texture = g_triangleEdgeSplit.saved_texture_u1;
    edge->texture_u1 = texture;
    texture = g_triangleEdgeSplit.saved_texture_v1;
    edge->texture_v1 = texture;
    Gfx_ClampTriangleEdgeRightEndpoint(edge);
    end_y = (unsigned int)edge->y1;
    start_y = (unsigned int)edge->y0;
    end_y = GFX_EDGE_SAR(end_y, 8);
    start_y = GFX_EDGE_SAR(start_y, 8);
    count = end_y - start_y;
    g_triangleEdgeContinuation.raw_count = (int)count;
    if (count != 0) {
        output->count = (int)((unsigned int)output->count + count);
        Gfx_InitializeTriangleEdgeSteps(edge);
        emission.count = (unsigned int)g_triangleEdgeContinuation.raw_count;
        emission.texture_v = g_triangleEdgeSteps.initial_texture_v;
        emission.x = g_triangleEdgeSteps.initial_x;
        emission.texture_u = g_triangleEdgeSteps.initial_texture_u;
        emission.output = g_triangleEdgeContinuation.raw_advanced_output;
        Gfx_EmitRawTriangleEdgeRecords(&emission);
    }
}

/* @original Gfx_BuildCorrectedTriangleEdge (IGN_WIN.EXE @ 0x0064EC00, unknown original source)
 * @fidelity ADAPTED
 * @deviation DEV-007
 */
void Gfx_BuildCorrectedTriangleEdge(volatile GfxTriangleEdgePacket *edge,
        volatile GfxTriangleEdgeBufferHeader *output) {
    unsigned int start_y;
    unsigned int end_y;
    unsigned int count;
    GfxTriangleEdgeEmission emission;
    start_y = (unsigned int)edge->y0;
    start_y = GFX_EDGE_SAR(start_y, 8);
    output->start_y = (int)start_y;
    end_y = (unsigned int)edge->y1;
    end_y = GFX_EDGE_SAR(end_y, 8);
    count = end_y - start_y;
    output->count = (int)count;
    if (count != 0) {
        Gfx_InitializeTriangleEdgeSteps(edge);
        emission.count = (unsigned int)output->count;
        emission.texture_v = g_triangleEdgeSteps.initial_texture_v;
        emission.x = g_triangleEdgeSteps.initial_x;
        emission.texture_u = g_triangleEdgeSteps.initial_texture_u;
        emission.output = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)
            ((unsigned int)(uintptr_t)output + 8u);
        Gfx_EmitCorrectedTriangleEdgeRecords(&emission);
    }
}

/* @original Gfx_BuildRawTriangleEdge (IGN_WIN.EXE @ 0x0064EC40, unknown original source)
 * @fidelity ADAPTED
 * @deviation DEV-007
 */
void Gfx_BuildRawTriangleEdge(volatile GfxTriangleEdgePacket *edge,
        volatile GfxTriangleEdgeBufferHeader *output) {
    unsigned int start_y;
    unsigned int end_y;
    unsigned int count;
    GfxTriangleEdgeEmission emission;
    start_y = (unsigned int)edge->y0;
    start_y = GFX_EDGE_SAR(start_y, 8);
    output->start_y = (int)start_y;
    end_y = (unsigned int)edge->y1;
    end_y = GFX_EDGE_SAR(end_y, 8);
    count = end_y - start_y;
    output->count = (int)count;
    if (count != 0) {
        Gfx_InitializeTriangleEdgeSteps(edge);
        emission.count = (unsigned int)output->count;
        emission.texture_v = g_triangleEdgeSteps.initial_texture_v;
        emission.x = g_triangleEdgeSteps.initial_x;
        emission.texture_u = g_triangleEdgeSteps.initial_texture_u;
        emission.output = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)
            ((unsigned int)(uintptr_t)output + 8u);
        Gfx_EmitRawTriangleEdgeRecords(&emission);
    }
}

/* @original Gfx_ComputeTriangleGradients (IGN_WIN.EXE @ 0x0064EA80, unknown original source)
 * @fidelity ADAPTED
 * @deviation DEV-007
 */
void Gfx_ComputeTriangleGradients(void) {
    unsigned int value;
    unsigned int height;
    unsigned int original_height;
    unsigned int denominator;
    unsigned int ratio;
    unsigned int high;
    unsigned int step;
    unsigned int table_word;
    unsigned int i;
    value = (unsigned int)g_triangleVertices.y1;
    value -= (unsigned int)g_triangleVertices.y0;
    g_triangleGradientScratch.height01 = value;
    original_height = (unsigned int)g_triangleVertices.y2;
    original_height -= (unsigned int)g_triangleVertices.y0;
    g_triangleGradientScratch.height02 = original_height;
    if (original_height == 0) {
        g_triangleGradientScratch.height02 = 1;
    }
    height = g_triangleGradientScratch.height01;
    if (original_height == height) {
        ratio = 0x7fffffffu;
    } else {
        denominator = g_triangleGradientScratch.height02;
        GFX_EDGE_UDIV_WORDS(ratio, height, 0, denominator);
        ratio >>= 1;
    }
    g_triangleGradientScratch.ratio = ratio;
    value = (unsigned int)g_triangleVertices.x1;
    value -= (unsigned int)g_triangleVertices.x0;
    g_triangleGradientScratch.delta_x01 = value;
    value = (unsigned int)g_triangleVertices.x2;
    value -= (unsigned int)g_triangleVertices.x0;
    value += value;
    ratio = g_triangleGradientScratch.ratio;
    GFX_EDGE_PRODUCT_HIGH(high, value, ratio);
    g_triangleGradientScratch.interpolated_x = high;
    value = (unsigned int)g_triangleVertices.texture_u2;
    value -= (unsigned int)g_triangleVertices.texture_u0;
    value += value;
    ratio = g_triangleGradientScratch.ratio;
    GFX_EDGE_PRODUCT_HIGH(high, value, ratio);
    g_triangleGradientScratch.interpolated_u = high;
    value = (unsigned int)g_triangleVertices.texture_v2;
    value -= (unsigned int)g_triangleVertices.texture_v0;
    value += value;
    ratio = g_triangleGradientScratch.ratio;
    GFX_EDGE_PRODUCT_HIGH(high, value, ratio);
    g_triangleGradientScratch.interpolated_v = high;
    value = g_triangleGradientScratch.interpolated_x;
    value -= g_triangleGradientScratch.delta_x01;
    if ((int)value >= -2 && (int)value <= 2) {
        value = (GFX_EDGE_SAR(value, 2) | 1u) << 2;
    }
    g_triangleGradientScratch.denominator = value;
    value = g_triangleGradientScratch.interpolated_u;
    value -= g_triangleVertices.texture_u1;
    value += g_triangleVertices.texture_u0;
    high = GFX_EDGE_SAR(value, 16);
    value <<= 16;
    denominator = g_triangleGradientScratch.denominator;
    GFX_EDGE_IDIV_WORDS(value, high, value, denominator);
    g_triangleTextureGradients.texture_u = (int)value;
    step = 0u - value;
    step = GFX_EDGE_SAR(step, 3);
    table_word = step;
    g_triangleEdgeUCorrections[0] = 0;
    g_triangleEdgeUCorrections[1] = table_word;
    for (i = 2; i < 8; i++) {
        table_word += step;
        g_triangleEdgeUCorrections[i] = table_word;
    }
    value = g_triangleGradientScratch.interpolated_v;
    value -= g_triangleVertices.texture_v1;
    value += g_triangleVertices.texture_v0;
    high = GFX_EDGE_SAR(value, 16);
    value <<= 16;
    denominator = g_triangleGradientScratch.denominator;
    GFX_EDGE_IDIV_WORDS(value, high, value, denominator);
    g_triangleTextureGradients.texture_v = (int)value;
    step = 0u - value;
    step = GFX_EDGE_SAR(step, 3);
    table_word = step;
    g_triangleEdgeVCorrections[0] = 0;
    g_triangleEdgeVCorrections[1] = table_word;
    for (i = 2; i < 8; i++) {
        table_word += step;
        g_triangleEdgeVCorrections[i] = table_word;
    }
}
