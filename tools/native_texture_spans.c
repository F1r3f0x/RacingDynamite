#include <windows.h>
#include <stdint.h>

typedef void (__cdecl *SetupFn)(unsigned int,unsigned int,unsigned int,unsigned int);
typedef void (__cdecl *SpanFn)(unsigned char *,int,unsigned int,unsigned int,const unsigned int *);
typedef void (__cdecl *ClippedFn)(unsigned char *,int,unsigned int,unsigned int);

static void output(const char *text) {
    DWORD written;
    DWORD length;
    length = 0;
    while (text[length] != 0) {
        length++;
    }
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),text,length,&written,0);
}

static void fail(const char *text,unsigned int code) {
    output(text);
    ExitProcess(code);
}

void WINAPI NativeEntry(void) {
    HANDLE file;
    DWORD size;
    DWORD read;
    unsigned char *raw;
    unsigned char *image;
    IMAGE_DOS_HEADER *dos;
    IMAGE_NT_HEADERS32 *nt;
    IMAGE_SECTION_HEADER *section;
    unsigned int n;
    unsigned int j;
    uint32_t *word;
    IMAGE_BASE_RELOCATION *reloc;
    WORD *entries;
    DWORD reloc_offset;
    DWORD reloc_end;
    DWORD delta;
    DWORD target;
    HMODULE compiled;
    SetupFn setup[2];
    SpanFn span[2];
    ClippedFn clipped[2];
    volatile unsigned int *state[2][5];
    unsigned char *buffers;
    unsigned char *texture[2];
    unsigned char *lookup[2];
    unsigned char *framebuffer[2];
    unsigned char *destination;
    unsigned int left[3];
    unsigned int gradients[4][2];
    int counts[8];
    unsigned int mode;
    unsigned int c;
    unsigned int g;
    unsigned int side;
    unsigned int case_number;
    const char *names[5];
    union {
        unsigned char *bytes;
        FARPROC generic;
        SetupFn setup;
        SpanFn span;
        ClippedFn clipped;
        volatile unsigned int *state;
    } binding;

    /* ORIGINAL_PE_MAPPING: generated host inserts the checked existing mapper. */

    binding.bytes = image + 0x5c990;
    setup[0] = binding.setup;
    binding.bytes = image + 0x5c9b0;
    span[0] = binding.span;
    binding.bytes = image + 0x5c9d0;
    clipped[0] = binding.clipped;
    compiled = LoadLibraryA("texture_spans_validation.dll");
    if (compiled == 0) {
        fail("FAIL native renderer DLL\n",20);
    }
    binding.generic = GetProcAddress(compiled,"Gfx_SetTriangleSpanState");
    setup[1] = binding.setup;
    binding.generic = GetProcAddress(compiled,"Gfx_DrawTriangleSpan");
    span[1] = binding.span;
    binding.generic = GetProcAddress(compiled,"Gfx_DrawClippedTriangleSpan");
    clipped[1] = binding.clipped;
    names[0] = "g_triangleSpanByteStepStorage";
    names[1] = "g_triangleSpanPackedStep";
    names[2] = "g_triangleSpanTextureWord";
    names[3] = "g_triangleSpanLookupWord";
    names[4] = "g_triangleSpanLeftSampleStorage";
    for (n = 0; n < 5; n++) {
        state[0][n] = (volatile unsigned int *)(image + 0xbad60 + n * 4);
        binding.generic = GetProcAddress(compiled,names[n]);
        state[1][n] = binding.state;
        if (state[1][n] == 0) {
            fail("FAIL native span state export\n",21);
        }
    }
    if (setup[1] == 0 || span[1] == 0 || clipped[1] == 0) {
        fail("FAIL native span code exports\n",22);
    }
    buffers = (unsigned char *)VirtualAlloc(0,6 * 65536,MEM_COMMIT | MEM_RESERVE,PAGE_READWRITE);
    if (buffers == 0 || ((unsigned int)(uintptr_t)buffers & 65535u) != 0) {
        fail("FAIL native aligned span buffers\n",23);
    }
    for (side = 0; side < 2; side++) {
        texture[side] = buffers + side * 65536;
        lookup[side] = buffers + (side + 2) * 65536;
        framebuffer[side] = buffers + (side + 4) * 65536;
    }
    gradients[0][0] = 0;
    gradients[0][1] = 0;
    gradients[1][0] = 0x10000;
    gradients[1][1] = 0x10000;
    gradients[2][0] = 0x23456;
    gradients[2][1] = 0xfffedcbb;
    gradients[3][0] = 0x7fffffff;
    gradients[3][1] = 0x80000000;
    counts[0] = -3;
    counts[1] = -1;
    counts[2] = 0;
    counts[3] = 1;
    counts[4] = 2;
    counts[5] = 5;
    counts[6] = 17;
    counts[7] = 64;
    left[0] = 0x12345678;
    left[1] = 0x9abcde34;
    left[2] = 0;
    case_number = 0;
    for (mode = 0; mode < 2; mode++) {
        for (c = 0; c < 8; c++) {
            for (g = 0; g < 4; g++) {
                for (side = 0; side < 2; side++) {
                    for (n = 0; n < 65536; n++) {
                        texture[side][n] = (unsigned char)(n * 73 + case_number * 3);
                        lookup[side][n] = (unsigned char)(n * 17 + (n >> 8));
                        framebuffer[side][n] = (unsigned char)(n * 29 + case_number);
                    }
                    *state[side][0] = 0x123456ab;
                    *state[side][4] = 0x87654321;
                    setup[side](gradients[g][0],gradients[g][1],
                        (unsigned int)(uintptr_t)texture[side] + 0x4567,
                        (unsigned int)(uintptr_t)lookup[side] + 0xabcd);
                    destination = (case_number % 3 == 1 ? texture[side] :
                        case_number % 3 == 2 ? lookup[side] : framebuffer[side]) + 0x2000;
                    if (mode == 0) {
                        span[side](destination,counts[c],0xffee1234,0x12345678,left);
                    } else {
                        clipped[side](destination,counts[c],0xffee1234,0x12345678);
                    }
                }
                for (n = 0; n < 65536; n++) {
                    if (texture[0][n] != texture[1][n] || lookup[0][n] != lookup[1][n] ||
                            framebuffer[0][n] != framebuffer[1][n]) {
                        fail("FAIL native paired span buffer comparison\n",24);
                    }
                }
                if (*state[0][0] != *state[1][0] || *state[0][1] != *state[1][1] ||
                        *state[0][4] != *state[1][4] ||
                        (*state[0][0] & 0xffffff00u) != 0x12345600 ||
                        (*state[0][4] & 0xffffff00u) != 0x87654300) {
                    fail("FAIL native span scratch or upper-byte preservation\n",25);
                }
                for (side = 0; side < 2; side++) {
                    if (*state[side][2] != (unsigned int)(uintptr_t)texture[side] + 0x4567 ||
                            *state[side][3] != (unsigned int)(uintptr_t)lookup[side] + 0xabcd) {
                        fail("FAIL native captured texture/table words\n",26);
                    }
                }
                case_number++;
            }
        }
    }
    output("PASS native texture spans: 64 paired calls, 12582912 buffer bytes, real original wrappers/leaves versus complete C89 renderer; zero/negative counts and overlapping render buffers. No native game parity.\n");
    ExitProcess(0);
}
