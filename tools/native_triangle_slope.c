#include <windows.h>
#include <stdint.h>

typedef int (__cdecl *SlopeFn)(int,int,int,int);
static const unsigned int cases[][5] = {
    /* GENERATED_SLOPE_CASES */
};

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
    SlopeFn original_slope;
    SlopeFn compiled_slope;
    union {
        unsigned char *bytes;
        FARPROC generic;
        SlopeFn slope;
    } binding;

    /* ORIGINAL_PE_MAPPING */

    binding.bytes = image + 0x5d7a0;
    original_slope = binding.slope;
    compiled = LoadLibraryA("triangle_slope_validation.dll");
    if (compiled == 0) {
        fail("FAIL native slope DLL\n",20);
    }
    binding.generic = GetProcAddress(compiled,"Gfx_ComputeTriangleEdgeSlope");
    compiled_slope = binding.slope;
    if (compiled_slope == 0) {
        fail("FAIL native slope export\n",21);
    }
    for (n = 0; n < sizeof(cases) / sizeof(cases[0]); n++) {
        if ((unsigned int)original_slope((int)cases[n][0],(int)cases[n][1],
                (int)cases[n][2],(int)cases[n][3]) != cases[n][4]) {
            fail("FAIL original native slope result\n",22);
        }
        if ((unsigned int)compiled_slope((int)cases[n][0],(int)cases[n][1],
                (int)cases[n][2],(int)cases[n][3]) != cases[n][4]) {
            fail("FAIL rebuilt native slope result\n",23);
        }
    }
    output("PASS native triangle slope: 84 paired cdecl calls, 20 equal-Y sentinels and 64 signed/wrapped normal quotients; real original and C89 renderer. No native fault or game parity.\n");
    ExitProcess(0);
}
