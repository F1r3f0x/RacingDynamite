#include "renderer.h"
#include "geputget.h" /* Verified workspaceB storage remains owned by graphics_legacy. */
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

volatile unsigned char *volatile g_spritePixels = 0;
volatile int g_spriteClipLeftFixed = 0;
volatile int g_spriteStride = 0;
volatile int g_spriteClipRightFixed = 0;
volatile int g_spriteClipTopFixed = 0;
volatile int g_spriteClipBottomFixed = 0;
volatile int g_spriteClipLeft = 0;
volatile int g_spriteClipTop = 0;
volatile int g_spriteClipRight = 0;
volatile int g_spriteWidth = 0;
volatile int g_spriteClipBottom = 0;
volatile unsigned int g_spriteConfigOption = 0;
volatile int g_spriteHeight = 0;
volatile int g_triangleViewportHeight = 0;
volatile unsigned char *volatile g_triangleViewportFramebuffer = 0;

typedef char GfxSpriteStateMustBe40Bytes[(sizeof(GfxSpriteState) == 40) ? 1 : -1];

volatile GfxTriangleEdgePacket g_triangleOrdinaryPacket = {0, 0, 0, 0, 0, 0, 0, 0};
volatile GfxTriangleEdgePacket g_triangleClippedPacket = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char *volatile g_triangleRasterTexture = 0;
volatile unsigned int g_triangleRasterCursor = 0;
volatile int g_triangleRasterMinimumX = 0;
volatile int g_triangleRasterMaximumX = 0;

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

/*
 * @original Gfx_SpriteCloseNative (IGN_WIN.EXE @ 0x00456F40, unknown original source)
 * @fidelity EXACT
 */
int Gfx_SpriteCloseNative(void) {
    return 1;
}

/*
 * @original Gfx_SpriteOptionNative (IGN_WIN.EXE @ 0x00456F50, unknown original source)
 * @fidelity EXACT
 */
int Gfx_SpriteOptionNative(unsigned int option) {
    (void)option;
    return 0;
}

/*
 * @original Gfx_SpriteConfigureNative (IGN_WIN.EXE @ 0x00456F60, unknown original source)
 * @fidelity EXACT
 */
int Gfx_SpriteConfigureNative(unsigned char *pixels, int stride, int width, int height, unsigned int option) {
    g_spritePixels = pixels;
    g_spriteStride = stride;
    g_spriteWidth = width;
    g_spriteHeight = height;
    g_spriteConfigOption = option;
    g_spriteClipLeft = 0;
    g_spriteClipLeftFixed = 0;
    g_spriteClipTop = 0;
    g_spriteClipTopFixed = 0;
    g_spriteClipRight = width;
    g_spriteClipRightFixed = (int)((unsigned int)width << 8);
    g_spriteClipBottom = height;
    g_spriteClipBottomFixed = (int)((unsigned int)height << 8);

    g_triangleViewportState.clip_left_integer = 0;
    g_triangleViewportState.clip_top_integer = 0;
    g_triangleViewportState.clip_right_integer = width;
    g_triangleViewportState.clip_bottom_integer = height;
    g_triangleViewportState.clip_left_fixed = 0;
    g_triangleViewportState.clip_top_fixed = 0;
    g_triangleViewportState.clip_right_fixed = (int)((unsigned int)width << 8);
    g_triangleViewportState.clip_bottom_fixed = (int)((unsigned int)height << 8);
    g_triangleViewportFramebuffer = pixels;
    g_triangleViewportState.row_stride = stride;
    g_triangleViewportHeight = height;
    return 1;
}

/*
 * @original Gfx_SpriteSetClipNative (IGN_WIN.EXE @ 0x00457020, unknown original source)
 * @fidelity EXACT
 */
int Gfx_SpriteSetClipNative(int left, int top, int right, int bottom) {
    if (left < 0 || top < 0 || right <= left || bottom <= top ||
        right > g_spriteWidth || bottom > g_spriteHeight) {
        return 0;
    }

    g_spriteClipLeft = left;
    g_spriteClipLeftFixed = (int)((unsigned int)left << 8);
    g_spriteClipTop = top;
    g_spriteClipTopFixed = (int)((unsigned int)top << 8);
    g_spriteClipRight = right;
    g_spriteClipRightFixed = (int)((unsigned int)right << 8);
    g_spriteClipBottom = bottom;
    g_spriteClipBottomFixed = (int)((unsigned int)bottom << 8);

    g_triangleViewportState.clip_left_integer = left;
    g_triangleViewportState.clip_top_integer = top;
    g_triangleViewportState.clip_right_integer = right;
    g_triangleViewportState.clip_bottom_integer = bottom;
    g_triangleViewportState.clip_left_fixed = (int)((unsigned int)left << 8);
    g_triangleViewportState.clip_top_fixed = (int)((unsigned int)top << 8);
    g_triangleViewportState.clip_right_fixed = (int)((unsigned int)right << 8);
    g_triangleViewportState.clip_bottom_fixed = (int)((unsigned int)bottom << 8);
    return 1;
}

/*
 * @original Gfx_SpriteGetStateNative (IGN_WIN.EXE @ 0x00457110, unknown original source)
 * @fidelity EXACT
 */
int Gfx_SpriteGetStateNative(GfxSpriteState *state) {
    volatile GfxSpriteState *output = state;
    int stride;
    output->width = g_spriteWidth;
    output->height = g_spriteHeight;
    output->option = g_spriteConfigOption;
    stride = g_spriteStride;
    output->format = 3;
    output->stride = stride;
    output->pixels = (unsigned char *)g_spritePixels;
    output->clip_left = g_spriteClipLeft;
    output->clip_top = g_spriteClipTop;
    output->clip_right = g_spriteClipRight;
    output->clip_bottom = g_spriteClipBottom;
    return 1;
}

/*
 * @original Gfx_SpriteSetStateNative (IGN_WIN.EXE @ 0x00457170, unknown original source)
 * @fidelity EXACT
 */
int Gfx_SpriteSetStateNative(const GfxSpriteState *state) {
    const volatile GfxSpriteState *input = state;
    unsigned int option;
    int height;
    int width;
    int stride;
    unsigned char *pixels;
    int bottom;
    int right;
    int top;
    int left;
    option = input->option;
    height = input->height;
    width = input->width;
    stride = input->stride;
    pixels = input->pixels;
    Gfx_SpriteConfigureNative(pixels, stride, width, height, option);
    bottom = input->clip_bottom;
    right = input->clip_right;
    top = input->clip_top;
    left = input->clip_left;
    Gfx_SpriteSetClipNative(left, top, right, bottom);
    return 1;
}

/* @original Gfx_RasterizeTexturedTriangle (IGN_WIN.EXE @ 0x0045CA50, unknown original source)
 * @fidelity ADAPTED
 * @deviation DEV-007
 */
void Gfx_RasterizeTexturedTriangle(const GfxTexturedTrianglePacket *packet) {
    const volatile GfxTexturedTrianglePacket *input = packet;
    int x_start;
    int y_start;
    int x_end;
    int y_end;
    volatile int comparison;
    volatile int bound;
    unsigned int u_start;
    unsigned int v_start;
    unsigned int u_end;
    unsigned int v_end;
    unsigned int texture_word;
    unsigned int lookup_word;
    unsigned int gradient_u;
    unsigned int gradient_v;
    unsigned int frame_word;
    int long_slope;
    int short_slope;
    volatile GfxTriangleEdgeBufferHeader *output;
    volatile GfxTriangleSpanEdgeRecord *left_record;
    volatile GfxTriangleSpanEdgeRecord *right_record;
    unsigned int left_base;
    unsigned int right_base;
    unsigned int left_offset;
    unsigned int right_offset;
    unsigned int processed_rows;
    int count;
    int left_x;
    int right_x;
    unsigned int texture_u;
    unsigned int texture_v;
    unsigned int integer_u;
    unsigned int packed_uv;
    unsigned int span_count;
    unsigned int destination;
    unsigned int stride;
    g_triangleVertices.x0 = input->x0;
    g_triangleVertices.y0 = input->y0;
    g_triangleVertices.x1 = input->x1;
    g_triangleVertices.y1 = input->y1;
    g_triangleVertices.x2 = input->x2;
    g_triangleVertices.y2 = input->y2;
    g_triangleVertices.texture_u0 = input->texture_u0;
    g_triangleVertices.texture_v0 = input->texture_v0;
    g_triangleVertices.texture_u1 = input->texture_u1;
    g_triangleVertices.texture_v1 = input->texture_v1;
    g_triangleVertices.texture_u2 = input->texture_u2;
    frame_word = (unsigned int)(uintptr_t)g_triangleViewportFramebuffer;
    g_triangleVertices.texture_v2 = input->texture_v2;
    x_end = g_triangleVertices.x1;
    texture_word = (unsigned int)(uintptr_t)input->texture;
    comparison = g_triangleVertices.x0;
    g_triangleRasterTexture = (unsigned char *)(uintptr_t)texture_word;
    g_triangleRasterCursor = frame_word;
    if (comparison > x_end) {
        x_start = g_triangleVertices.x0;
        g_triangleRasterMinimumX = x_end;
        g_triangleRasterMaximumX = x_start;
    } else {
        x_end = g_triangleVertices.x1;
        x_start = g_triangleVertices.x0;
        g_triangleRasterMaximumX = x_end;
        g_triangleRasterMinimumX = x_start;
    }
    bound = g_triangleRasterMaximumX;
    comparison = g_triangleVertices.x2;
    x_end = g_triangleVertices.x2;
    if (comparison > bound) {
        g_triangleRasterMaximumX = x_end;
    } else {
        comparison = g_triangleRasterMinimumX;
        if (comparison > x_end) {
            g_triangleRasterMinimumX = x_end;
        }
    }
    y_end = g_triangleVertices.y2;
    comparison = g_triangleVertices.y0;
    if (comparison > y_end) {
        y_start = g_triangleVertices.y0;
        v_end = g_triangleVertices.texture_v2;
        x_end = g_triangleVertices.x2;
        g_triangleVertices.y2 = y_start;
        x_start = g_triangleVertices.x0;
        g_triangleVertices.y0 = y_end;
        v_start = g_triangleVertices.texture_v0;
        g_triangleVertices.x2 = x_start;
        g_triangleVertices.texture_v2 = v_start;
        g_triangleVertices.texture_v0 = v_end;
        u_end = g_triangleVertices.texture_u2;
        u_start = g_triangleVertices.texture_u0;
        g_triangleVertices.x0 = x_end;
        g_triangleVertices.texture_u2 = u_start;
        g_triangleVertices.texture_u0 = u_end;
    }
    y_end = g_triangleVertices.y1;
    comparison = g_triangleVertices.y0;
    if (comparison > y_end) {
        y_start = g_triangleVertices.y0;
        v_end = g_triangleVertices.texture_v1;
        x_end = g_triangleVertices.x1;
        g_triangleVertices.y1 = y_start;
        x_start = g_triangleVertices.x0;
        g_triangleVertices.y0 = y_end;
        v_start = g_triangleVertices.texture_v0;
        g_triangleVertices.x1 = x_start;
        g_triangleVertices.texture_v1 = v_start;
        g_triangleVertices.texture_v0 = v_end;
        u_end = g_triangleVertices.texture_u1;
        u_start = g_triangleVertices.texture_u0;
        g_triangleVertices.x0 = x_end;
        g_triangleVertices.texture_u1 = u_start;
        g_triangleVertices.texture_u0 = u_end;
    } else {
        y_end = g_triangleVertices.y1;
        comparison = g_triangleVertices.y2;
        if (comparison < y_end) {
            y_start = g_triangleVertices.y2;
            v_end = g_triangleVertices.texture_v1;
            x_end = g_triangleVertices.x1;
            g_triangleVertices.y1 = y_start;
            x_start = g_triangleVertices.x2;
            g_triangleVertices.y2 = y_end;
            v_start = g_triangleVertices.texture_v2;
            g_triangleVertices.x1 = x_start;
            g_triangleVertices.texture_v1 = v_start;
            g_triangleVertices.texture_v2 = v_end;
            u_end = g_triangleVertices.texture_u1;
            u_start = g_triangleVertices.texture_u2;
            g_triangleVertices.x2 = x_end;
            g_triangleVertices.texture_u1 = u_start;
            g_triangleVertices.texture_u2 = u_end;
        }
    }
    bound = g_triangleViewportState.clip_top_fixed;
    comparison = g_triangleVertices.y0;
    if (comparison < bound) {
        Gfx_RasterizeClippedTriangle(packet);
        return;
    }
    bound = g_triangleViewportState.clip_bottom_fixed;
    comparison = g_triangleVertices.y2;
    if (comparison > bound) {
        Gfx_RasterizeClippedTriangle(packet);
        return;
    }
    bound = g_triangleViewportState.clip_right_fixed;
    comparison = g_triangleRasterMaximumX;
    if (comparison > bound) {
        Gfx_RasterizeClippedTriangle(packet);
        return;
    }
    bound = g_triangleViewportState.clip_left_fixed;
    comparison = g_triangleRasterMinimumX;
    if (comparison < bound) {
        Gfx_RasterizeClippedTriangle(packet);
        return;
    }
    Gfx_ComputeTriangleGradients();
    lookup_word = (unsigned int)(uintptr_t)input->lookup;
    texture_word = (unsigned int)(uintptr_t)g_triangleRasterTexture;
    gradient_u = (unsigned int)g_triangleTextureGradients.texture_u;
    gradient_v = (unsigned int)g_triangleTextureGradients.texture_v;
    Gfx_SetTriangleSpanState(gradient_v, gradient_u, texture_word, lookup_word);
    y_end = g_triangleVertices.y2;
    x_end = g_triangleVertices.x2;
    y_start = g_triangleVertices.y0;
    x_start = g_triangleVertices.x0;
    long_slope = Gfx_ComputeTriangleEdgeSlope(x_start, y_start, x_end, y_end);
    y_end = g_triangleVertices.y1;
    x_end = g_triangleVertices.x1;
    y_start = g_triangleVertices.y0;
    x_start = g_triangleVertices.x0;
    short_slope = Gfx_ComputeTriangleEdgeSlope(x_start, y_start, x_end, y_end);
    y_start = g_triangleVertices.y0;
    x_end = g_triangleVertices.x1;
    x_start = g_triangleVertices.x0;
    g_triangleOrdinaryPacket.y0 = y_start;
    u_start = g_triangleVertices.texture_u0;
    g_triangleOrdinaryPacket.x0 = x_start;
    y_end = g_triangleVertices.y1;
    g_triangleOrdinaryPacket.x1 = x_end;
    v_start = g_triangleVertices.texture_v0;
    g_triangleOrdinaryPacket.y1 = y_end;
    u_end = g_triangleVertices.texture_u1;
    g_triangleOrdinaryPacket.texture_u0 = u_start;
    v_end = g_triangleVertices.texture_v1;
    g_triangleOrdinaryPacket.texture_v0 = v_start;
    g_triangleOrdinaryPacket.texture_u1 = u_end;
    g_triangleOrdinaryPacket.texture_v1 = v_end;
    if (long_slope > short_slope) {
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer0;
        Gfx_BuildRawTriangleEdge(&g_triangleOrdinaryPacket, output);
        x_start = g_triangleVertices.x0;
        y_start = g_triangleVertices.y0;
        x_end = g_triangleVertices.x2;
        g_triangleOrdinaryPacket.x0 = x_start;
        y_end = g_triangleVertices.y2;
        g_triangleOrdinaryPacket.y0 = y_start;
        u_start = g_triangleVertices.texture_u0;
        g_triangleOrdinaryPacket.x1 = x_end;
        v_start = g_triangleVertices.texture_v0;
        g_triangleOrdinaryPacket.y1 = y_end;
        u_end = g_triangleVertices.texture_u2;
        g_triangleOrdinaryPacket.texture_u0 = u_start;
        v_end = g_triangleVertices.texture_v2;
        g_triangleOrdinaryPacket.texture_v0 = v_start;
        g_triangleOrdinaryPacket.texture_u1 = u_end;
        g_triangleOrdinaryPacket.texture_v1 = v_end;
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer1;
        Gfx_BuildCorrectedTriangleEdge(&g_triangleOrdinaryPacket, output);
        x_start = g_triangleVertices.x1;
        y_start = g_triangleVertices.y1;
        x_end = g_triangleVertices.x2;
        g_triangleOrdinaryPacket.x0 = x_start;
        y_end = g_triangleVertices.y2;
        g_triangleOrdinaryPacket.y0 = y_start;
        u_start = g_triangleVertices.texture_u1;
        g_triangleOrdinaryPacket.x1 = x_end;
        v_start = g_triangleVertices.texture_v1;
        g_triangleOrdinaryPacket.y1 = y_end;
        u_end = g_triangleVertices.texture_u2;
        g_triangleOrdinaryPacket.texture_u0 = u_start;
        v_end = g_triangleVertices.texture_v2;
        g_triangleOrdinaryPacket.texture_v0 = v_start;
        g_triangleOrdinaryPacket.texture_u1 = u_end;
        g_triangleOrdinaryPacket.texture_v1 = v_end;
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer2;
        Gfx_BuildRawTriangleEdge(&g_triangleOrdinaryPacket, output);
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer0;
        y_start = output->start_y;
        stride = (unsigned int)g_triangleViewportState.row_stride;
        g_triangleRasterCursor += (unsigned int)y_start * stride;
        processed_rows = 0;
        left_offset = 0;
        right_offset = 0;
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer0;
        count = output->count;
        if (count > 0) {
            do {
                left_base = (unsigned int)(uintptr_t)g_spriteWorkspaceBBuffer0;
                right_base = (unsigned int)(uintptr_t)g_spriteWorkspaceBBuffer1;
                left_record = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)(left_base + left_offset + 8u);
                right_record = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)(right_base + right_offset + 8u);
                texture_u = right_record->texture_u;
                left_x = left_record->x;
                integer_u = GFX_EDGE_SAR(texture_u, 8);
                texture_v = right_record->texture_v;
                packed_uv = (texture_v & 65535u) + (texture_u << 24);
                right_x = right_record->x;
                span_count = (unsigned int)right_x - (unsigned int)left_x - 1u;
                destination = g_triangleRasterCursor;
                destination += (unsigned int)left_x + 1u;
                Gfx_DrawTriangleSpan((unsigned char *)(uintptr_t)destination, (int)span_count,
                    packed_uv, integer_u, (const GfxTriangleSpanEdgeRecord *)left_record);
                stride = (unsigned int)g_triangleViewportState.row_stride;
                output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer0;
                g_triangleRasterCursor += stride;
                left_offset += 12u;
                right_offset += 12u;
                processed_rows++;
                count = output->count;
            } while (count > (int)processed_rows);
        }
        right_offset = processed_rows * 12u;
        left_offset = 0;
        processed_rows = 0;
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer2;
        count = output->count;
        if (count > 0) {
            do {
                left_base = (unsigned int)(uintptr_t)g_spriteWorkspaceBBuffer2;
                right_base = (unsigned int)(uintptr_t)g_spriteWorkspaceBBuffer1;
                left_record = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)(left_base + left_offset + 8u);
                right_record = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)(right_base + right_offset + 8u);
                texture_u = right_record->texture_u;
                left_x = left_record->x;
                integer_u = GFX_EDGE_SAR(texture_u, 8);
                texture_v = right_record->texture_v;
                packed_uv = (texture_v & 65535u) + (texture_u << 24);
                right_x = right_record->x;
                span_count = (unsigned int)right_x - (unsigned int)left_x - 1u;
                destination = g_triangleRasterCursor;
                destination += (unsigned int)left_x + 1u;
                Gfx_DrawTriangleSpan((unsigned char *)(uintptr_t)destination, (int)span_count,
                    packed_uv, integer_u, (const GfxTriangleSpanEdgeRecord *)left_record);
                stride = (unsigned int)g_triangleViewportState.row_stride;
                output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer2;
                g_triangleRasterCursor += stride;
                left_offset += 12u;
                right_offset += 12u;
                processed_rows++;
                count = output->count;
            } while (count > (int)processed_rows);
        }
    } else {
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer0;
        Gfx_BuildCorrectedTriangleEdge(&g_triangleOrdinaryPacket, output);
        x_start = g_triangleVertices.x0;
        y_start = g_triangleVertices.y0;
        x_end = g_triangleVertices.x2;
        g_triangleOrdinaryPacket.x0 = x_start;
        y_end = g_triangleVertices.y2;
        g_triangleOrdinaryPacket.y0 = y_start;
        u_start = g_triangleVertices.texture_u0;
        g_triangleOrdinaryPacket.x1 = x_end;
        v_start = g_triangleVertices.texture_v0;
        g_triangleOrdinaryPacket.y1 = y_end;
        u_end = g_triangleVertices.texture_u2;
        g_triangleOrdinaryPacket.texture_u0 = u_start;
        v_end = g_triangleVertices.texture_v2;
        g_triangleOrdinaryPacket.texture_v0 = v_start;
        g_triangleOrdinaryPacket.texture_u1 = u_end;
        g_triangleOrdinaryPacket.texture_v1 = v_end;
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer1;
        Gfx_BuildRawTriangleEdge(&g_triangleOrdinaryPacket, output);
        x_start = g_triangleVertices.x1;
        y_start = g_triangleVertices.y1;
        x_end = g_triangleVertices.x2;
        g_triangleOrdinaryPacket.x0 = x_start;
        y_end = g_triangleVertices.y2;
        g_triangleOrdinaryPacket.y0 = y_start;
        u_start = g_triangleVertices.texture_u1;
        g_triangleOrdinaryPacket.x1 = x_end;
        v_start = g_triangleVertices.texture_v1;
        g_triangleOrdinaryPacket.y1 = y_end;
        u_end = g_triangleVertices.texture_u2;
        g_triangleOrdinaryPacket.texture_u0 = u_start;
        v_end = g_triangleVertices.texture_v2;
        g_triangleOrdinaryPacket.texture_v0 = v_start;
        g_triangleOrdinaryPacket.texture_u1 = u_end;
        g_triangleOrdinaryPacket.texture_v1 = v_end;
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer2;
        Gfx_BuildCorrectedTriangleEdge(&g_triangleOrdinaryPacket, output);
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer1;
        y_start = output->start_y;
        stride = (unsigned int)g_triangleViewportState.row_stride;
        g_triangleRasterCursor += (unsigned int)y_start * stride;
        processed_rows = 0;
        left_offset = 0;
        right_offset = 0;
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer0;
        count = output->count;
        if (count > 0) {
            do {
                left_base = (unsigned int)(uintptr_t)g_spriteWorkspaceBBuffer1;
                right_base = (unsigned int)(uintptr_t)g_spriteWorkspaceBBuffer0;
                left_record = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)(left_base + left_offset + 8u);
                right_record = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)(right_base + right_offset + 8u);
                texture_u = right_record->texture_u;
                left_x = left_record->x;
                integer_u = GFX_EDGE_SAR(texture_u, 8);
                texture_v = right_record->texture_v;
                packed_uv = (texture_v & 65535u) + (texture_u << 24);
                right_x = right_record->x;
                span_count = (unsigned int)right_x - (unsigned int)left_x - 1u;
                destination = g_triangleRasterCursor;
                destination += (unsigned int)left_x + 1u;
                Gfx_DrawTriangleSpan((unsigned char *)(uintptr_t)destination, (int)span_count,
                    packed_uv, integer_u, (const GfxTriangleSpanEdgeRecord *)left_record);
                stride = (unsigned int)g_triangleViewportState.row_stride;
                output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer0;
                g_triangleRasterCursor += stride;
                left_offset += 12u;
                right_offset += 12u;
                processed_rows++;
                count = output->count;
            } while (count > (int)processed_rows);
        }
        left_offset = processed_rows * 12u;
        right_offset = 0;
        processed_rows = 0;
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer2;
        count = output->count;
        if (count > 0) {
            do {
                left_base = (unsigned int)(uintptr_t)g_spriteWorkspaceBBuffer1;
                right_base = (unsigned int)(uintptr_t)g_spriteWorkspaceBBuffer2;
                left_record = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)(left_base + left_offset + 8u);
                right_record = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)(right_base + right_offset + 8u);
                texture_u = right_record->texture_u;
                left_x = left_record->x;
                integer_u = GFX_EDGE_SAR(texture_u, 8);
                texture_v = right_record->texture_v;
                packed_uv = (texture_v & 65535u) + (texture_u << 24);
                right_x = right_record->x;
                span_count = (unsigned int)right_x - (unsigned int)left_x - 1u;
                destination = g_triangleRasterCursor;
                destination += (unsigned int)left_x + 1u;
                Gfx_DrawTriangleSpan((unsigned char *)(uintptr_t)destination, (int)span_count,
                    packed_uv, integer_u, (const GfxTriangleSpanEdgeRecord *)left_record);
                stride = (unsigned int)g_triangleViewportState.row_stride;
                output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer2;
                g_triangleRasterCursor += stride;
                left_offset += 12u;
                right_offset += 12u;
                processed_rows++;
                count = output->count;
            } while (count > (int)processed_rows);
        }
    }
}

/* @original Gfx_RasterizeClippedTriangle (IGN_WIN.EXE @ 0x0045D170, unknown original source)
 * @fidelity ADAPTED
 * @deviation DEV-007
 */
void Gfx_RasterizeClippedTriangle(const GfxTexturedTrianglePacket *packet) {
    const volatile GfxTexturedTrianglePacket *input = packet;
    int x_start;
    int y_start;
    int x_end;
    int y_end;
    volatile int comparison;
    volatile int bound;
    unsigned int u_start;
    unsigned int v_start;
    unsigned int u_end;
    unsigned int v_end;
    unsigned int texture_word;
    unsigned int lookup_word;
    unsigned int gradient_u;
    unsigned int gradient_v;
    int long_slope;
    int short_slope;
    volatile GfxTriangleEdgeBufferHeader *output;
    volatile GfxTriangleSpanEdgeRecord *left_record;
    volatile GfxTriangleSpanEdgeRecord *right_record;
    unsigned int left_base;
    unsigned int right_base;
    unsigned int left_offset;
    unsigned int right_offset;
    unsigned int processed_rows;
    int count;
    int left_x;
    int right_x;
    unsigned int texture_u;
    unsigned int texture_v;
    unsigned int integer_u;
    unsigned int packed_uv;
    unsigned int span_count;
    unsigned int destination;
    unsigned int stride;
    int clip_left;
    bound = g_triangleViewportState.clip_top_fixed;
    comparison = g_triangleVertices.y2;
    if (comparison < bound) {
        return;
    }
    bound = g_triangleViewportState.clip_bottom_fixed;
    comparison = g_triangleVertices.y0;
    if (comparison > bound) {
        return;
    }
    bound = g_triangleViewportState.clip_right_fixed;
    comparison = g_triangleRasterMinimumX;
    if (comparison > bound) {
        return;
    }
    bound = g_triangleViewportState.clip_left_fixed;
    comparison = g_triangleRasterMaximumX;
    if (comparison < bound) {
        return;
    }
    Gfx_ComputeTriangleGradients();
    texture_word = (unsigned int)(uintptr_t)g_triangleRasterTexture;
    lookup_word = (unsigned int)(uintptr_t)input->lookup;
    gradient_u = (unsigned int)g_triangleTextureGradients.texture_u;
    gradient_v = (unsigned int)g_triangleTextureGradients.texture_v;
    Gfx_SetTriangleSpanState(gradient_v, gradient_u, texture_word, lookup_word);
    y_end = g_triangleVertices.y2;
    x_end = g_triangleVertices.x2;
    y_start = g_triangleVertices.y1;
    x_start = g_triangleVertices.x1;
    long_slope = Gfx_ComputeTriangleEdgeSlope(x_start, y_start, x_end, y_end);
    y_end = g_triangleVertices.y1;
    x_end = g_triangleVertices.x1;
    y_start = g_triangleVertices.y0;
    x_start = g_triangleVertices.x0;
    short_slope = Gfx_ComputeTriangleEdgeSlope(x_start, y_start, x_end, y_end);
    y_start = g_triangleVertices.y0;
    x_end = g_triangleVertices.x1;
    x_start = g_triangleVertices.x0;
    g_triangleClippedPacket.y0 = y_start;
    u_start = g_triangleVertices.texture_u0;
    g_triangleClippedPacket.x0 = x_start;
    y_end = g_triangleVertices.y1;
    g_triangleClippedPacket.x1 = x_end;
    v_start = g_triangleVertices.texture_v0;
    g_triangleClippedPacket.y1 = y_end;
    u_end = g_triangleVertices.texture_u1;
    g_triangleClippedPacket.texture_u0 = u_start;
    v_end = g_triangleVertices.texture_v1;
    g_triangleClippedPacket.texture_v0 = v_start;
    g_triangleClippedPacket.texture_u1 = u_end;
    g_triangleClippedPacket.texture_v1 = v_end;
    if (long_slope > short_slope) {
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer0;
        Gfx_BuildClippedRawTriangleEdge(&g_triangleClippedPacket, output);
        x_start = g_triangleVertices.x0;
        y_start = g_triangleVertices.y0;
        x_end = g_triangleVertices.x2;
        g_triangleClippedPacket.x0 = x_start;
        y_end = g_triangleVertices.y2;
        g_triangleClippedPacket.y0 = y_start;
        u_start = g_triangleVertices.texture_u0;
        g_triangleClippedPacket.x1 = x_end;
        v_start = g_triangleVertices.texture_v0;
        g_triangleClippedPacket.y1 = y_end;
        u_end = g_triangleVertices.texture_u2;
        g_triangleClippedPacket.texture_u0 = u_start;
        v_end = g_triangleVertices.texture_v2;
        g_triangleClippedPacket.texture_v0 = v_start;
        g_triangleClippedPacket.texture_u1 = u_end;
        g_triangleClippedPacket.texture_v1 = v_end;
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer1;
        Gfx_BuildClippedCorrectedTriangleEdge(&g_triangleClippedPacket, output);
        x_start = g_triangleVertices.x1;
        y_start = g_triangleVertices.y1;
        x_end = g_triangleVertices.x2;
        g_triangleClippedPacket.x0 = x_start;
        y_end = g_triangleVertices.y2;
        g_triangleClippedPacket.y0 = y_start;
        u_start = g_triangleVertices.texture_u1;
        g_triangleClippedPacket.x1 = x_end;
        v_start = g_triangleVertices.texture_v1;
        g_triangleClippedPacket.y1 = y_end;
        u_end = g_triangleVertices.texture_u2;
        g_triangleClippedPacket.texture_u0 = u_start;
        v_end = g_triangleVertices.texture_v2;
        g_triangleClippedPacket.texture_v0 = v_start;
        g_triangleClippedPacket.texture_u1 = u_end;
        g_triangleClippedPacket.texture_v1 = v_end;
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer2;
        Gfx_BuildClippedRawTriangleEdge(&g_triangleClippedPacket, output);
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer0;
        y_start = output->start_y;
        stride = (unsigned int)g_triangleViewportState.row_stride;
        g_triangleRasterCursor += (unsigned int)y_start * stride;
        processed_rows = 0;
        left_offset = 0;
        right_offset = 0;
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer0;
        count = output->count;
        if (count > 0) {
            do {
                left_base = (unsigned int)(uintptr_t)g_spriteWorkspaceBBuffer0;
                left_record = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)(left_base + left_offset + 8u);
                left_x = left_record->x;
                clip_left = g_triangleViewportState.clip_left_integer;
                if (clip_left > left_x) {
                    right_base = (unsigned int)(uintptr_t)g_spriteWorkspaceBBuffer1;
                    right_record = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)(right_base + right_offset + 8u);
                    texture_u = right_record->texture_u;
                    integer_u = GFX_EDGE_SAR(texture_u, 8);
                    texture_v = right_record->texture_v;
                    packed_uv = (texture_v & 65535u) + (texture_u << 24);
                    right_x = right_record->x;
                    clip_left = g_triangleViewportState.clip_left_integer;
                    span_count = (unsigned int)right_x - (unsigned int)clip_left;
                    clip_left = g_triangleViewportState.clip_left_integer;
                    destination = (unsigned int)clip_left + g_triangleRasterCursor;
                    Gfx_DrawClippedTriangleSpan((unsigned char *)(uintptr_t)destination, (int)span_count,
                        packed_uv, integer_u);
                } else {
                    right_base = (unsigned int)(uintptr_t)g_spriteWorkspaceBBuffer1;
                    right_record = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)(right_base + right_offset + 8u);
                    texture_u = right_record->texture_u;
                    integer_u = GFX_EDGE_SAR(texture_u, 8);
                    texture_v = right_record->texture_v;
                    packed_uv = (texture_v & 65535u) + (texture_u << 24);
                    right_x = right_record->x;
                    span_count = (unsigned int)right_x - (unsigned int)left_x - 1u;
                    destination = g_triangleRasterCursor;
                    destination += (unsigned int)left_x + 1u;
                    Gfx_DrawTriangleSpan((unsigned char *)(uintptr_t)destination, (int)span_count,
                    packed_uv, integer_u, (const GfxTriangleSpanEdgeRecord *)left_record);
                }
                stride = (unsigned int)g_triangleViewportState.row_stride;
                left_offset += 12u;
                right_offset += 12u;
                processed_rows++;
                g_triangleRasterCursor += stride;
                output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer0;
                count = output->count;
            } while (count > (int)processed_rows);
        }
        right_offset = processed_rows * 12u;
        left_offset = 0;
        processed_rows = 0;
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer2;
        count = output->count;
        if (count > 0) {
            do {
                left_base = (unsigned int)(uintptr_t)g_spriteWorkspaceBBuffer2;
                left_record = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)(left_base + left_offset + 8u);
                left_x = left_record->x;
                clip_left = g_triangleViewportState.clip_left_integer;
                if (clip_left > left_x) {
                    right_base = (unsigned int)(uintptr_t)g_spriteWorkspaceBBuffer1;
                    right_record = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)(right_base + right_offset + 8u);
                    texture_u = right_record->texture_u;
                    integer_u = GFX_EDGE_SAR(texture_u, 8);
                    texture_v = right_record->texture_v;
                    packed_uv = (texture_v & 65535u) + (texture_u << 24);
                    right_x = right_record->x;
                    clip_left = g_triangleViewportState.clip_left_integer;
                    span_count = (unsigned int)right_x - (unsigned int)clip_left;
                    clip_left = g_triangleViewportState.clip_left_integer;
                    destination = (unsigned int)clip_left + g_triangleRasterCursor;
                    Gfx_DrawClippedTriangleSpan((unsigned char *)(uintptr_t)destination, (int)span_count,
                        packed_uv, integer_u);
                } else {
                    right_base = (unsigned int)(uintptr_t)g_spriteWorkspaceBBuffer1;
                    right_record = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)(right_base + right_offset + 8u);
                    texture_u = right_record->texture_u;
                    integer_u = GFX_EDGE_SAR(texture_u, 8);
                    texture_v = right_record->texture_v;
                    packed_uv = (texture_v & 65535u) + (texture_u << 24);
                    right_x = right_record->x;
                    span_count = (unsigned int)right_x - (unsigned int)left_x - 1u;
                    destination = g_triangleRasterCursor;
                    destination += (unsigned int)left_x + 1u;
                    Gfx_DrawTriangleSpan((unsigned char *)(uintptr_t)destination, (int)span_count,
                    packed_uv, integer_u, (const GfxTriangleSpanEdgeRecord *)left_record);
                }
                stride = (unsigned int)g_triangleViewportState.row_stride;
                left_offset += 12u;
                right_offset += 12u;
                processed_rows++;
                g_triangleRasterCursor += stride;
                output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer2;
                count = output->count;
            } while (count > (int)processed_rows);
        }
    } else {
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer0;
        Gfx_BuildClippedCorrectedTriangleEdge(&g_triangleClippedPacket, output);
        x_start = g_triangleVertices.x0;
        y_start = g_triangleVertices.y0;
        x_end = g_triangleVertices.x2;
        g_triangleClippedPacket.x0 = x_start;
        y_end = g_triangleVertices.y2;
        g_triangleClippedPacket.y0 = y_start;
        u_start = g_triangleVertices.texture_u0;
        g_triangleClippedPacket.x1 = x_end;
        v_start = g_triangleVertices.texture_v0;
        g_triangleClippedPacket.y1 = y_end;
        u_end = g_triangleVertices.texture_u2;
        g_triangleClippedPacket.texture_u0 = u_start;
        v_end = g_triangleVertices.texture_v2;
        g_triangleClippedPacket.texture_v0 = v_start;
        g_triangleClippedPacket.texture_u1 = u_end;
        g_triangleClippedPacket.texture_v1 = v_end;
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer1;
        Gfx_BuildClippedRawTriangleEdge(&g_triangleClippedPacket, output);
        x_start = g_triangleVertices.x1;
        y_start = g_triangleVertices.y1;
        x_end = g_triangleVertices.x2;
        g_triangleClippedPacket.x0 = x_start;
        y_end = g_triangleVertices.y2;
        g_triangleClippedPacket.y0 = y_start;
        u_start = g_triangleVertices.texture_u1;
        g_triangleClippedPacket.x1 = x_end;
        v_start = g_triangleVertices.texture_v1;
        g_triangleClippedPacket.y1 = y_end;
        u_end = g_triangleVertices.texture_u2;
        g_triangleClippedPacket.texture_u0 = u_start;
        v_end = g_triangleVertices.texture_v2;
        g_triangleClippedPacket.texture_v0 = v_start;
        g_triangleClippedPacket.texture_u1 = u_end;
        g_triangleClippedPacket.texture_v1 = v_end;
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer2;
        Gfx_BuildClippedCorrectedTriangleEdge(&g_triangleClippedPacket, output);
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer1;
        y_start = output->start_y;
        stride = (unsigned int)g_triangleViewportState.row_stride;
        g_triangleRasterCursor += (unsigned int)y_start * stride;
        processed_rows = 0;
        left_offset = 0;
        right_offset = 0;
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer0;
        count = output->count;
        if (count > 0) {
            do {
                left_base = (unsigned int)(uintptr_t)g_spriteWorkspaceBBuffer1;
                left_record = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)(left_base + left_offset + 8u);
                left_x = left_record->x;
                clip_left = g_triangleViewportState.clip_left_integer;
                if (clip_left > left_x) {
                    right_base = (unsigned int)(uintptr_t)g_spriteWorkspaceBBuffer0;
                    right_record = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)(right_base + right_offset + 8u);
                    texture_u = right_record->texture_u;
                    integer_u = GFX_EDGE_SAR(texture_u, 8);
                    texture_v = right_record->texture_v;
                    packed_uv = (texture_v & 65535u) + (texture_u << 24);
                    right_x = right_record->x;
                    clip_left = g_triangleViewportState.clip_left_integer;
                    span_count = (unsigned int)right_x - (unsigned int)clip_left;
                    clip_left = g_triangleViewportState.clip_left_integer;
                    destination = (unsigned int)clip_left + g_triangleRasterCursor;
                    Gfx_DrawClippedTriangleSpan((unsigned char *)(uintptr_t)destination, (int)span_count,
                        packed_uv, integer_u);
                } else {
                    right_base = (unsigned int)(uintptr_t)g_spriteWorkspaceBBuffer0;
                    right_record = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)(right_base + right_offset + 8u);
                    texture_u = right_record->texture_u;
                    integer_u = GFX_EDGE_SAR(texture_u, 8);
                    texture_v = right_record->texture_v;
                    packed_uv = (texture_v & 65535u) + (texture_u << 24);
                    right_x = right_record->x;
                    span_count = (unsigned int)right_x - (unsigned int)left_x - 1u;
                    destination = g_triangleRasterCursor;
                    destination += (unsigned int)left_x + 1u;
                    Gfx_DrawTriangleSpan((unsigned char *)(uintptr_t)destination, (int)span_count,
                    packed_uv, integer_u, (const GfxTriangleSpanEdgeRecord *)left_record);
                }
                stride = (unsigned int)g_triangleViewportState.row_stride;
                left_offset += 12u;
                right_offset += 12u;
                processed_rows++;
                g_triangleRasterCursor += stride;
                output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer0;
                count = output->count;
            } while (count > (int)processed_rows);
        }
        left_offset = processed_rows * 12u;
        right_offset = 0;
        processed_rows = 0;
        output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer2;
        count = output->count;
        if (count > 0) {
            do {
                left_base = (unsigned int)(uintptr_t)g_spriteWorkspaceBBuffer1;
                left_record = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)(left_base + left_offset + 8u);
                left_x = left_record->x;
                clip_left = g_triangleViewportState.clip_left_integer;
                if (clip_left > left_x) {
                    right_base = (unsigned int)(uintptr_t)g_spriteWorkspaceBBuffer2;
                    right_record = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)(right_base + right_offset + 8u);
                    texture_u = right_record->texture_u;
                    integer_u = GFX_EDGE_SAR(texture_u, 8);
                    texture_v = right_record->texture_v;
                    packed_uv = (texture_v & 65535u) + (texture_u << 24);
                    right_x = right_record->x;
                    clip_left = g_triangleViewportState.clip_left_integer;
                    span_count = (unsigned int)right_x - (unsigned int)clip_left;
                    clip_left = g_triangleViewportState.clip_left_integer;
                    destination = (unsigned int)clip_left + g_triangleRasterCursor;
                    Gfx_DrawClippedTriangleSpan((unsigned char *)(uintptr_t)destination, (int)span_count,
                        packed_uv, integer_u);
                } else {
                    right_base = (unsigned int)(uintptr_t)g_spriteWorkspaceBBuffer2;
                    right_record = (volatile GfxTriangleSpanEdgeRecord *)(uintptr_t)(right_base + right_offset + 8u);
                    texture_u = right_record->texture_u;
                    integer_u = GFX_EDGE_SAR(texture_u, 8);
                    texture_v = right_record->texture_v;
                    packed_uv = (texture_v & 65535u) + (texture_u << 24);
                    right_x = right_record->x;
                    span_count = (unsigned int)right_x - (unsigned int)left_x - 1u;
                    destination = g_triangleRasterCursor;
                    destination += (unsigned int)left_x + 1u;
                    Gfx_DrawTriangleSpan((unsigned char *)(uintptr_t)destination, (int)span_count,
                    packed_uv, integer_u, (const GfxTriangleSpanEdgeRecord *)left_record);
                }
                stride = (unsigned int)g_triangleViewportState.row_stride;
                left_offset += 12u;
                g_triangleRasterCursor += stride;
                output = (volatile GfxTriangleEdgeBufferHeader *)g_spriteWorkspaceBBuffer2;
                right_offset += 12u;
                processed_rows++;
                count = output->count;
            } while (count > (int)processed_rows);
        }
    }
}

volatile GfxTransformedSpriteScratch g_transformedSpriteScratch; /* VA 0x004BAF1C */

/*
 * @original Gfx_PrepareTransformedSprite (IGN_WIN.EXE @ 0x00468C70, unknown original source)
 * @fidelity ADAPTED
 * @deviation DEV-007
 */
void Gfx_PrepareTransformedSprite(const GfxTransformedSpriteInput *input) {
    const unsigned int *prepared = input->prepared;
    const int *coefficients = input->coefficients;
    int position_x = input->position_x;
    int position_y = input->position_y;
    int ox;
    int oy;
    int c0;
    int c1;
    int c2;
    int c3;
    int dx_span;
    int dy_span;
    int corner0_x;
    int corner0_y;
    int dx_rot_x;
    int dx_rot_y;
    int dy_rot_x;
    int dy_rot_y;

    g_transformedSpriteScratch.param_u0 = prepared[3];
    g_transformedSpriteScratch.param_v0 = prepared[5] - 0x10u;
    g_transformedSpriteScratch.param_u1 = prepared[2];
    g_transformedSpriteScratch.param_v1 = prepared[4] - 0x10u;
    g_transformedSpriteScratch.lookup = (unsigned char *)(uintptr_t)prepared[7];
    g_transformedSpriteScratch.texture = (unsigned char *)(uintptr_t)prepared[6];

    c0 = coefficients[0];
    c1 = coefficients[1];
    c2 = coefficients[2];
    c3 = coefficients[3];
    ox = (int)prepared[0];
    oy = (int)prepared[1];

    corner0_x = (int)(GFX_EDGE_MUL16((unsigned int)-ox, (unsigned int)c0)
              + GFX_EDGE_MUL16((unsigned int)-oy, (unsigned int)c2));
    corner0_y = (int)(GFX_EDGE_MUL16((unsigned int)-ox, (unsigned int)c1)
              + GFX_EDGE_MUL16((unsigned int)-oy, (unsigned int)c3));

    dx_span = (int)prepared[4] - 0x10 - (int)prepared[2];
    dx_rot_y = (int)GFX_EDGE_MUL16((unsigned int)dx_span, (unsigned int)c1);
    dx_rot_x = (int)GFX_EDGE_MUL16((unsigned int)dx_span, (unsigned int)c0);

    dy_span = (int)prepared[5] - 0x10 - (int)prepared[3];
    dy_rot_y = (int)GFX_EDGE_MUL16((unsigned int)dy_span, (unsigned int)c3);
    dy_rot_x = (int)GFX_EDGE_MUL16((unsigned int)dy_span, (unsigned int)c2);

    g_transformedSpriteScratch.corner0_x = corner0_x + position_x;
    g_transformedSpriteScratch.corner0_y = corner0_y + position_y;
    g_transformedSpriteScratch.corner1_x = corner0_x + dx_rot_x + position_x;
    g_transformedSpriteScratch.corner1_y = corner0_y + dx_rot_y + position_y;
    g_transformedSpriteScratch.corner2_x = corner0_x + dy_rot_x + position_x;
    g_transformedSpriteScratch.corner2_y = corner0_y + dy_rot_y + position_y;
    g_transformedSpriteScratch.corner3_x = corner0_x + dy_rot_x + dx_rot_x + position_x;
    g_transformedSpriteScratch.corner3_y = corner0_y + dy_rot_y + dx_rot_y + position_y;

    /* Triangle 1: corners 0, 1, 2 */
    g_transformedSpriteScratch.packet.x0 = g_transformedSpriteScratch.corner0_x;
    g_transformedSpriteScratch.packet.y0 = g_transformedSpriteScratch.corner0_y;
    g_transformedSpriteScratch.packet.x1 = g_transformedSpriteScratch.corner1_x;
    g_transformedSpriteScratch.packet.y1 = g_transformedSpriteScratch.corner1_y;
    g_transformedSpriteScratch.packet.x2 = g_transformedSpriteScratch.corner2_x;
    g_transformedSpriteScratch.packet.y2 = g_transformedSpriteScratch.corner2_y;
    g_transformedSpriteScratch.packet.texture_u0 = g_transformedSpriteScratch.param_u1;
    g_transformedSpriteScratch.packet.texture_v0 = g_transformedSpriteScratch.param_u0;
    g_transformedSpriteScratch.packet.texture_u1 = g_transformedSpriteScratch.param_v1;
    g_transformedSpriteScratch.packet.texture_v1 = g_transformedSpriteScratch.param_u0;
    g_transformedSpriteScratch.packet.texture_u2 = g_transformedSpriteScratch.param_u1;
    g_transformedSpriteScratch.packet.texture_v2 = g_transformedSpriteScratch.param_v0;
    g_transformedSpriteScratch.packet.texture = g_transformedSpriteScratch.texture;
    g_transformedSpriteScratch.packet.lookup = g_transformedSpriteScratch.lookup;

    Gfx_RasterizeTexturedTriangle((const GfxTexturedTrianglePacket *)&g_transformedSpriteScratch.packet);

    /* Triangle 2: corners 1, 2, 3 */
    g_transformedSpriteScratch.packet.x0 = g_transformedSpriteScratch.corner1_x;
    g_transformedSpriteScratch.packet.y0 = g_transformedSpriteScratch.corner1_y;
    g_transformedSpriteScratch.packet.x1 = g_transformedSpriteScratch.corner2_x;
    g_transformedSpriteScratch.packet.y1 = g_transformedSpriteScratch.corner2_y;
    g_transformedSpriteScratch.packet.x2 = g_transformedSpriteScratch.corner3_x;
    g_transformedSpriteScratch.packet.y2 = g_transformedSpriteScratch.corner3_y;
    g_transformedSpriteScratch.packet.texture_u0 = g_transformedSpriteScratch.param_v1;
    g_transformedSpriteScratch.packet.texture_v0 = g_transformedSpriteScratch.param_u0;
    g_transformedSpriteScratch.packet.texture_u1 = g_transformedSpriteScratch.param_u1;
    g_transformedSpriteScratch.packet.texture_v1 = g_transformedSpriteScratch.param_v0;
    g_transformedSpriteScratch.packet.texture_u2 = g_transformedSpriteScratch.param_v1;
    g_transformedSpriteScratch.packet.texture_v2 = g_transformedSpriteScratch.param_v0;
    g_transformedSpriteScratch.packet.texture = g_transformedSpriteScratch.texture;
    g_transformedSpriteScratch.packet.lookup = g_transformedSpriteScratch.lookup;

    Gfx_RasterizeTexturedTriangle((const GfxTexturedTrianglePacket *)&g_transformedSpriteScratch.packet);
}

volatile unsigned int g_spriteBlitSource = 0; /* VA 0x004BAD28 */
volatile int g_spriteBlitWidth = 0; /* VA 0x004BAD2C */
volatile int g_spriteBlitHeight = 0; /* VA 0x004BAD30 */
volatile GfxTransformedSpriteInput g_transformedSpriteInput = {0, 0, 0, 0, 0}; /* VA 0x004BAD34 */

/*
 * @original Gfx_RasterizeSpritePacket (IGN_WIN.EXE @ 0x00465BB5, unknown original source)
 * @fidelity ADAPTED
 * @deviation DEV-007
 */
void Gfx_RasterizeSpritePacket(const GfxSpriteRasterPacket *packet)
{
    const volatile GfxSpriteRasterPacket *live_packet = packet;
    const volatile unsigned int *prepared;
    const volatile GfxPoint2D *position;
    unsigned int screen_x, screen_y, delta, previous;
    unsigned int source, destination, rows, remaining;
    unsigned int lookup_a, lookup_b, colors;

    if (live_packet->coefficients != (const int *)0) {
        position = live_packet->position;
        g_transformedSpriteInput.position_x = position->x;
        g_transformedSpriteInput.position_y = position->y;
        g_transformedSpriteInput.prepared = live_packet->prepared;
        g_transformedSpriteInput.coefficients = live_packet->coefficients;
        Gfx_PrepareTransformedSprite((const GfxTransformedSpriteInput *)&g_transformedSpriteInput);
        return;
    }

    prepared = live_packet->prepared;
    source = prepared[3] & 0xffffff00u;
    source |= *((const volatile unsigned char *)prepared + 9);
    source += prepared[6];
    g_spriteBlitSource = source;
    previous = (unsigned int)((int)prepared[4] >> 8);
    previous -= (unsigned int)((int)prepared[2] >> 8);
    g_spriteBlitWidth = (int)previous;
    previous = (unsigned int)((int)prepared[5] >> 8);
    previous -= (unsigned int)((int)prepared[3] >> 8);
    g_spriteBlitHeight = (int)previous;
    position = live_packet->position;
    screen_x = (unsigned int)position->x;
    screen_y = (unsigned int)position->y;
    screen_x -= prepared[0];
    screen_y -= prepared[1];
    screen_x = (unsigned int)((int)screen_x >> 8);
    screen_y = (unsigned int)((int)screen_y >> 8);
    if ((int)screen_y < g_triangleViewportState.clip_top_integer) {
        delta = (unsigned int)g_triangleViewportState.clip_top_integer - screen_y;
        previous = (unsigned int)g_spriteBlitHeight;
        g_spriteBlitHeight = (int)(previous - delta);
        if ((int)previous <= (int)delta) return;
        g_spriteBlitSource += delta << 8;
        screen_y = (unsigned int)g_triangleViewportState.clip_top_integer;
    }
    previous = screen_y + (unsigned int)g_spriteBlitHeight;
    delta = (unsigned int)g_triangleViewportState.clip_bottom_integer;
    if ((int)previous > (int)delta) {
        delta = previous - delta;
        previous = (unsigned int)g_spriteBlitHeight;
        g_spriteBlitHeight = (int)(previous - delta);
        if ((int)previous <= (int)delta) return;
    }
    if ((int)screen_x < g_triangleViewportState.clip_left_integer) {
        delta = (unsigned int)g_triangleViewportState.clip_left_integer - screen_x;
        previous = (unsigned int)g_spriteBlitWidth;
        g_spriteBlitWidth = (int)(previous - delta);
        if ((int)previous <= (int)delta) return;
        g_spriteBlitSource += delta;
        screen_x = (unsigned int)g_triangleViewportState.clip_left_integer;
    }
    previous = screen_x + (unsigned int)g_spriteBlitWidth;
    delta = (unsigned int)g_triangleViewportState.clip_right_integer;
    if ((int)previous > (int)delta) {
        delta = previous - delta;
        previous = (unsigned int)g_spriteBlitWidth;
        g_spriteBlitWidth = (int)(previous - delta);
        if ((int)previous <= (int)delta) return;
    }
    destination = screen_y * (unsigned int)g_triangleViewportState.row_stride;
    prepared = live_packet->prepared;
    destination += screen_x;
    destination += (unsigned int)(uintptr_t)g_triangleViewportFramebuffer;
    lookup_a = prepared[7];
    rows = (unsigned int)g_spriteBlitHeight;
    source = g_spriteBlitSource;
    remaining = (unsigned int)g_spriteBlitWidth;
    lookup_b = lookup_a;
    /* Two prefetched lookup addresses feed descending four-byte stores.
     * Keep their read/write order: source, lookup and frame may overlap.
     * Lookup indexing replaces the low WORD of the supplied table address.
     */
#define SPRITE_BYTE(address) (*(volatile unsigned char *)(uintptr_t)(address))
#define SPRITE_LOW(address, value) (((address) & 0xffffff00u) | (value))
#define SPRITE_HIGH(address, value) (((address) & 0xffff00ffu) | ((unsigned int)(value) << 8))
    do {
        previous = remaining;
        remaining -= 6u;
        if ((int)previous < 6) {
            remaining += 4u;
            if ((int)remaining < 0) {
                remaining += 1u;
                if ((int)remaining < 0) goto sprite_next_row;
                lookup_b = SPRITE_LOW(lookup_b, SPRITE_BYTE(destination));
                lookup_b = SPRITE_HIGH(lookup_b, SPRITE_BYTE(source));
                colors = SPRITE_BYTE(lookup_b);
                goto sprite_first_pixel;
            }
            lookup_a = SPRITE_LOW(lookup_a, SPRITE_BYTE(destination + remaining + 1u));
            lookup_b = SPRITE_HIGH(lookup_b, SPRITE_BYTE(source + remaining));
            lookup_a = SPRITE_HIGH(lookup_a, SPRITE_BYTE(source + remaining + 1u));
            lookup_b = SPRITE_LOW(lookup_b, SPRITE_BYTE(destination + remaining));
            remaining -= 4u;
            colors = (unsigned int)SPRITE_BYTE(lookup_a) << 8;
        } else {
            lookup_a = SPRITE_LOW(lookup_a, SPRITE_BYTE(destination + remaining + 5u));
            lookup_b = SPRITE_HIGH(lookup_b, SPRITE_BYTE(source + remaining + 4u));
            lookup_a = SPRITE_HIGH(lookup_a, SPRITE_BYTE(source + remaining + 5u));
            lookup_b = SPRITE_LOW(lookup_b, SPRITE_BYTE(destination + remaining + 4u));
            colors = (unsigned int)SPRITE_BYTE(lookup_a) << 8;
            do {
                lookup_a = SPRITE_LOW(lookup_a, SPRITE_BYTE(destination + remaining + 3u));
                colors = (colors & 0xffffff00u) | SPRITE_BYTE(lookup_b);
                lookup_a = SPRITE_HIGH(lookup_a, SPRITE_BYTE(source + remaining + 3u));
                lookup_b = SPRITE_LOW(lookup_b, SPRITE_BYTE(destination + remaining + 2u));
                colors <<= 16;
                lookup_b = SPRITE_HIGH(lookup_b, SPRITE_BYTE(source + remaining + 2u));
                colors = SPRITE_HIGH(colors, SPRITE_BYTE(lookup_a));
                lookup_a = SPRITE_LOW(lookup_a, SPRITE_BYTE(destination + remaining + 1u));
                colors = SPRITE_LOW(colors, SPRITE_BYTE(lookup_b));
                lookup_b = SPRITE_LOW(lookup_b, SPRITE_BYTE(destination + remaining));
                lookup_a = SPRITE_HIGH(lookup_a, SPRITE_BYTE(source + remaining + 1u));
                *(volatile unsigned int *)(uintptr_t)(destination + remaining + 2u) = colors;
                lookup_b = SPRITE_HIGH(lookup_b, SPRITE_BYTE(source + remaining));
                remaining -= 4u;
                colors = SPRITE_HIGH(colors, SPRITE_BYTE(lookup_a));
            } while ((int)remaining >= 0);
        }
        SPRITE_BYTE(destination + remaining + 5u) = (unsigned char)(colors >> 8);
        remaining += 3u;
        colors = SPRITE_BYTE(lookup_b);
        if ((int)remaining >= 0) {
            do {
                lookup_b = SPRITE_HIGH(lookup_b, SPRITE_BYTE(source + remaining));
                SPRITE_BYTE(destination + remaining + 1u) = (unsigned char)colors;
                lookup_b = SPRITE_LOW(lookup_b, SPRITE_BYTE(destination + remaining));
                remaining -= 1u;
                colors = SPRITE_BYTE(lookup_b);
            } while ((int)remaining >= 0);
        }
    sprite_first_pixel:
        SPRITE_BYTE(destination) = (unsigned char)colors;
    sprite_next_row:
        delta = (unsigned int)g_triangleViewportState.row_stride;
        source += 256u;
        destination += delta;
        rows -= 1u;
        remaining = (unsigned int)g_spriteBlitWidth;
    } while (rows != 0);
#undef SPRITE_HIGH
#undef SPRITE_LOW
#undef SPRITE_BYTE
}

volatile unsigned int g_spritePreparedStorage[8] = {0, 0, 0, 0, 0, 0, 0, 0}; /* VA 0x004BA758 */
volatile GfxSpriteRasterPacket g_spriteRasterPacket = {
    4u,
    (const GfxPoint2D *)0,
    (const unsigned int *)g_spritePreparedStorage,
    (const int *)0
}; /* VA 0x004BA778 */
volatile unsigned int g_spriteRenderLookup = 0; /* VA 0x0063F2D8 */

/*
 * @original Gfx_SubmitSpriteRequest (IGN_WIN.EXE @ 0x00457370, unknown original source)
 * @fidelity ADAPTED
 * @deviation DEV-007
 */
void Gfx_SubmitSpriteRequest(void)
{
    GfxSpriteDescriptor local_descriptor;
    volatile GfxSpriteRequest *request = g_activeSpriteRequest;
    GfxSpriteHandle *handle;
    unsigned int word4;

    g_spriteRasterPacket.position = (const GfxPoint2D *)request->position;
    g_spriteRasterPacket.coefficients = (const int *)request->coefficients;
    handle = request->handle;

    Gfx_CopySpriteDescriptor(&local_descriptor, handle->image_id);

    g_spritePreparedStorage[0] = (unsigned int)handle->origin_x;
    word4 = local_descriptor.words[4];
    g_spritePreparedStorage[1] = (unsigned int)handle->origin_y;
    g_spritePreparedStorage[2] = (word4 & 0xFFu) << 8;
    g_spritePreparedStorage[3] = word4 & 0xFF00u;
    g_spritePreparedStorage[6] = word4 & 0xFFFF0000u;
    g_spritePreparedStorage[4] = (local_descriptor.words[1] << 8) + g_spritePreparedStorage[2];
    g_spritePreparedStorage[5] = (local_descriptor.words[2] << 8) + g_spritePreparedStorage[3];
    g_spritePreparedStorage[7] = g_spriteRenderLookup;

    Gfx_RasterizeSpritePacket((const GfxSpriteRasterPacket *)&g_spriteRasterPacket);
}

volatile unsigned int g_triangleDrawListStride = 4; /* VA 0x004BA704 */
volatile unsigned int g_triangleWorkspacePacket[14] = {18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; /* VA 0x004BA718 */

/*
 * @original Gfx_SpriteDrawListCallback (IGN_WIN.EXE @ 0x004572B0, unknown original source)
 * @fidelity EXACT
 */
void Gfx_SpriteDrawListCallback(unsigned int header)
{
    volatile unsigned int local_header = header;
    (void)local_header;
}

/*
 * @original Gfx_SubmitDrawListTriangleRequest (IGN_WIN.EXE @ 0x004572C0, unknown original source)
 * @fidelity EXACT
 */
void Gfx_SubmitDrawListTriangleRequest(void)
{
    const unsigned int *const *request = (const unsigned int *const *)g_activeSpriteRequest;
    const unsigned int *vertex0 = request[0];
    const unsigned int *vertex1 = request[1];
    const unsigned int *vertex2 = request[2];
    const unsigned int *attrs = request[3];

    g_triangleWorkspacePacket[1] = vertex0[0];
    g_triangleWorkspacePacket[2] = vertex0[1];
    g_triangleWorkspacePacket[3] = vertex1[0];
    g_triangleWorkspacePacket[4] = vertex1[1];
    g_triangleWorkspacePacket[5] = vertex2[0];
    g_triangleWorkspacePacket[6] = vertex2[1];
    g_triangleWorkspacePacket[7] = attrs[2];
    g_triangleWorkspacePacket[8] = attrs[3];
    g_triangleWorkspacePacket[9] = attrs[4];
    g_triangleWorkspacePacket[10] = attrs[5];
    g_triangleWorkspacePacket[11] = attrs[6];
    g_triangleWorkspacePacket[12] = attrs[7];
    g_triangleWorkspacePacket[13] = attrs[0];

    Gfx_RasterizeWorkspaceATriangle((const GfxTexturedTrianglePacket *)g_triangleWorkspacePacket);
}

/*
 * @original Gfx_SpriteDrawListNative (IGN_WIN.EXE @ 0x00457250, unknown original source)
 * @fidelity EXACT
 */
int Gfx_SpriteDrawListNative(const unsigned int *list)
{
    while (*list != 0) {
        unsigned int header = *list++;
        Gfx_SpriteDrawListCallback(header);
        while (*list != 0) {
            const unsigned int *batch = (const unsigned int *)(uintptr_t)(*list++);
            g_activeSpriteRequest = (volatile GfxSpriteRequest *)batch;
            if (*batch != 0) {
                do {
                    Gfx_SubmitDrawListTriangleRequest();
                    g_activeSpriteRequest = (volatile GfxSpriteRequest *)(
                        (const char *)g_activeSpriteRequest + (g_triangleDrawListStride << 2)
                    );
                } while (*(const unsigned int *)g_activeSpriteRequest != 0);
            }
        }
        list++;
    }
    return 1;
}

volatile unsigned int g_triangleSpanWorkspaceAByteStep = 0;
volatile unsigned int g_triangleSpanWorkspaceAPackedStep = 0;
volatile unsigned int g_triangleSpanWorkspaceATextureWord = 0;
volatile unsigned int g_triangleSpanWorkspaceALeftSample = 0;

/* @original Gfx_SetWorkspaceATriangleSpanCore (IGN_WIN.EXE @ 0x00468030, unknown original source)
 * @fidelity ADAPTED
 * @deviation DEV-007
 */
void Gfx_SetWorkspaceATriangleSpanCore(unsigned int gradient_v, unsigned int gradient_u,
        unsigned int texture_word) {
    unsigned int negative_v;
    unsigned int negative_u;
    unsigned int rotated_u;
    negative_v = 0u - gradient_v;
    negative_u = 0u - gradient_u;
    negative_v >>= 8;
    g_triangleSpanWorkspaceATextureWord = texture_word;
    rotated_u = (negative_u >> 16) | (negative_u << 16);
    GFX_SPAN_BYTE(&g_triangleSpanWorkspaceAByteStep) = (unsigned char)rotated_u;
    g_triangleSpanWorkspaceAPackedStep = (rotated_u & 0xffff0000u) | (negative_v & 0xffffu);
}

/* @original Gfx_SetWorkspaceATriangleSpanState (IGN_WIN.EXE @ 0x0045D7E0, unknown original source)
 * @fidelity EXACT
 */
void Gfx_SetWorkspaceATriangleSpanState(unsigned int gradient_v, unsigned int gradient_u,
        unsigned int texture_word) {
    Gfx_SetWorkspaceATriangleSpanCore(gradient_v, gradient_u, texture_word);
}

/* @original Gfx_DrawWorkspaceATriangleSpanCore (IGN_WIN.EXE @ 0x00468050, unknown original source)
 * @fidelity ADAPTED
 * @deviation DEV-007
 */
void Gfx_DrawWorkspaceATriangleSpanCore(unsigned char *destination, int count,
        unsigned int packed_uv, unsigned int integer_u,
        const GfxTriangleSpanEdgeRecord *left) {
    unsigned int texture;
    unsigned int byte_step;
    unsigned int packed_step;
    unsigned int remaining;
    unsigned int previous_uv;
    unsigned int carry;
    unsigned int address;
    unsigned char source_byte;
    texture = g_triangleSpanWorkspaceATextureWord;
    texture = (texture & 0xffffff00u) | GFX_SPAN_BYTE((uintptr_t)left + 4u);
    texture = (texture & 0xffff00ffu) | ((unsigned int)GFX_SPAN_BYTE(left) << 8);
    source_byte = GFX_SPAN_BYTE(texture);
    GFX_SPAN_BYTE(&g_triangleSpanWorkspaceALeftSample) = source_byte;
    texture = (texture & 0xffff0000u) | (integer_u & 0xffffu);
    byte_step = GFX_SPAN_BYTE(&g_triangleSpanWorkspaceAByteStep);
    packed_step = g_triangleSpanWorkspaceAPackedStep;
    remaining = (unsigned int)count - 1u;
    while ((remaining & 0x80000000u) == 0u) {
        previous_uv = packed_uv;
        packed_uv += packed_step;
        carry = packed_uv < previous_uv;
        texture = (texture & 0xffff00ffu) | (packed_uv & 0xff00u);
        texture = (texture & 0xffffff00u) | ((texture + byte_step + carry) & 0xffu);
        address = (unsigned int)(uintptr_t)destination + remaining;
        source_byte = GFX_SPAN_BYTE(texture);
        GFX_SPAN_BYTE(address) = source_byte;
        remaining--;
    }
    if (remaining == 0xffffffffu) {
        source_byte = GFX_SPAN_BYTE(&g_triangleSpanWorkspaceALeftSample);
        address = (unsigned int)(uintptr_t)destination + remaining;
        GFX_SPAN_BYTE(address) = source_byte;
    }
}

/* @original Gfx_DrawWorkspaceATriangleSpan (IGN_WIN.EXE @ 0x0045D800, unknown original source)
 * @fidelity EXACT
 */
void Gfx_DrawWorkspaceATriangleSpan(unsigned char *destination, int count,
        unsigned int packed_uv, unsigned int integer_u,
        const GfxTriangleSpanEdgeRecord *left) {
    Gfx_DrawWorkspaceATriangleSpanCore(destination, count, packed_uv, integer_u, left);
}

/* @original Gfx_DrawWorkspaceAClippedTriangleSpanCore (IGN_WIN.EXE @ 0x004680C6, unknown original source)
 * @fidelity ADAPTED
 * @deviation DEV-007
 */
void Gfx_DrawWorkspaceAClippedTriangleSpanCore(unsigned char *destination, int count,
        unsigned int packed_uv, unsigned int integer_u) {
    unsigned int texture;
    unsigned int byte_step;
    unsigned int packed_step;
    unsigned int remaining;
    unsigned int previous_uv;
    unsigned int carry;
    unsigned int address;
    unsigned char source_byte;
    texture = (g_triangleSpanWorkspaceATextureWord & 0xffff0000u) | (integer_u & 0xffffu);
    byte_step = GFX_SPAN_BYTE(&g_triangleSpanWorkspaceAByteStep);
    packed_step = g_triangleSpanWorkspaceAPackedStep;
    remaining = (unsigned int)count - 1u;
    while ((remaining & 0x80000000u) == 0u) {
        previous_uv = packed_uv;
        packed_uv += packed_step;
        carry = packed_uv < previous_uv;
        texture = (texture & 0xffff00ffu) | (packed_uv & 0xff00u);
        texture = (texture & 0xffffff00u) | ((texture + byte_step + carry) & 0xffu);
        address = (unsigned int)(uintptr_t)destination + remaining;
        source_byte = GFX_SPAN_BYTE(texture);
        GFX_SPAN_BYTE(address) = source_byte;
        remaining--;
    }
}

/* @original Gfx_DrawWorkspaceAClippedTriangleSpan (IGN_WIN.EXE @ 0x0045D820, unknown original source)
 * @fidelity EXACT
 */
void Gfx_DrawWorkspaceAClippedTriangleSpan(unsigned char *destination, int count,
        unsigned int packed_uv, unsigned int integer_u) {
    Gfx_DrawWorkspaceAClippedTriangleSpanCore(destination, count, packed_uv, integer_u);
}

/* @original Gfx_ComputeWorkspaceATriangleSlope (IGN_WIN.EXE @ 0x0045E5D0, unknown original source)
 * @fidelity EXACT
 */
int Gfx_ComputeWorkspaceATriangleSlope(int x0, int y0, int x1, int y1) {
    unsigned int numerator_word;
    unsigned int denominator_word;
    if (y0 == y1) {
        if (x0 > x1) {
            return (int)0x80010000u;
        }
        return 0x7fff0000;
    }
    numerator_word = ((unsigned int)x0 - (unsigned int)x1) << 8;
    denominator_word = (unsigned int)y0 - (unsigned int)y1;
    return (int)numerator_word / (int)denominator_word;
}
