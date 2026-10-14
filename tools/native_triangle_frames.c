#include <windows.h>
#include <stdint.h>

#include "renderer.h"
typedef void (__cdecl *RenderFn)(const GfxTexturedTrianglePacket *);
typedef int (__cdecl *ConfigureFn)(unsigned char *,int,int,int,unsigned int);
typedef int (__cdecl *ClipFn)(int,int,int,int);
typedef struct NativeGlobal { const char *name; unsigned int rva; unsigned int size; } NativeGlobal;
static const NativeGlobal globals[] = {
    /* GENERATED_GLOBALS */
};
static const int shapes[][6] = {
    {4,4,20,4,4,20},{8,3,3,14,25,24},{8,3,25,14,3,24},
    {-3,-2,32,8,8,35},{-40,4,-20,8,-30,20},{4,4,4,4,4,4}
};
static unsigned char pixels[65536];
static unsigned char texture[65536];
static unsigned char lookup[65536];
static unsigned int edges[3][0x20d8/4];
static unsigned char expected_pixels[2][65536];
static unsigned char expected_edges[2][3][0x20d8];
static unsigned char expected_globals[2][32][48];

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
    RenderFn render_fn[2];
    RenderFn clipped_fn[2];
    ConfigureFn configure_fn[2];
    ClipFn clip_fn[2];
    unsigned int side;
    unsigned int phase;
    unsigned int g;
    unsigned int offset;
    unsigned int vertex;
    unsigned char *global;
    GfxTexturedTrianglePacket packet;
    union { unsigned char *bytes; FARPROC generic; RenderFn render; ConfigureFn configure; ClipFn clip; } binding;

    /* ORIGINAL_PE_MAPPING */

    compiled = LoadLibraryA("triangles_validation.dll");
    if (compiled == 0) { fail("FAIL native triangle DLL\n",20); }
    binding.bytes = image + 0x5ca50;
    render_fn[0] = binding.render;
    binding.generic = GetProcAddress(compiled,"Gfx_RasterizeTexturedTriangle");
    render_fn[1] = binding.render;
    if (render_fn[1] == 0) { fail("FAIL native triangle export\n",21); }
    binding.bytes = image + 0x5d170;
    clipped_fn[0] = binding.render;
    binding.generic = GetProcAddress(compiled,"Gfx_RasterizeClippedTriangle");
    clipped_fn[1] = binding.render;
    if (clipped_fn[1] == 0) { fail("FAIL native triangle export\n",21); }
    binding.bytes = image + 0x56f60;
    configure_fn[0] = binding.configure;
    binding.generic = GetProcAddress(compiled,"Gfx_SpriteConfigureNative");
    configure_fn[1] = binding.configure;
    if (configure_fn[1] == 0) { fail("FAIL native triangle export\n",21); }
    binding.bytes = image + 0x57020;
    clip_fn[0] = binding.clip;
    binding.generic = GetProcAddress(compiled,"Gfx_SpriteSetClipNative");
    clip_fn[1] = binding.clip;
    if (clip_fn[1] == 0) { fail("FAIL native triangle export\n",21); }
    for (n = 0; n < 24; n++) {
        for (side = 0; side < 2; side++) {
            for (g = 0; g < sizeof(globals)/sizeof(globals[0]); g++) {
                if (side == 0) { global = image + globals[g].rva; }
                else { binding.generic = GetProcAddress(compiled,globals[g].name); global = binding.bytes; }
                if (global == 0) { fail("FAIL native triangle global\n",22); }
                for (offset = 0; offset < globals[g].size; offset++) { global[offset] = 0; }
            }
            for (g = 0; g < 3; g++) {
                if (side == 0) { global = image + 0x11a9d8 + g*4; }
                else {
                    binding.generic = GetProcAddress(compiled, g == 0 ? "g_spriteWorkspaceBBuffer0" :
                        (g == 1 ? "g_spriteWorkspaceBBuffer1" : "g_spriteWorkspaceBBuffer2"));
                    global = binding.bytes;
                }
                *(unsigned int *)global = (unsigned int)(uintptr_t)edges[g];
            }
            if (configure_fn[side](pixels,64,32,32,0) != 1 || clip_fn[side](1,1,30,30) != 1) {
                fail("FAIL native triangle setup\n",23);
            }
            packet.unused_word = 0x1234u;
            vertex = n % 6;
            packet.x0 = (int)((unsigned int)shapes[vertex][0]*256u + n/6);
            packet.y0 = (int)((unsigned int)shapes[vertex][1]*256u + n/6);
            packet.x1 = (int)((unsigned int)shapes[vertex][2]*256u + n/6);
            packet.y1 = (int)((unsigned int)shapes[vertex][3]*256u + n/6);
            packet.x2 = (int)((unsigned int)shapes[vertex][4]*256u + n/6);
            packet.y2 = (int)((unsigned int)shapes[vertex][5]*256u + n/6);
            packet.texture_u0 = 512; packet.texture_v0 = 768;
            packet.texture_u1 = 4608; packet.texture_v1 = 1024;
            packet.texture_u2 = 1024; packet.texture_v2 = 4864;
            packet.texture = texture; packet.lookup = lookup;
            for (phase = 0; phase < 2; phase++) {
                for (j = 0; j < 65536; j++) {
                    pixels[j] = (unsigned char)(j+n);
                    texture[j] = (unsigned char)((j*37u+19u)^(j>>8));
                    lookup[j] = (unsigned char)((j*17u+29u)^(j>>4));
                }
                for (g = 0; g < 3; g++) {
                    for (j = 0; j < sizeof(edges[g])/sizeof(edges[g][0]); j++) { edges[g][j] = 0x5a5a5a5au; }
                }
                if (phase == 0) { render_fn[side](&packet); }
                else {
                    if (side == 0) { global = image + 0x11a9d0; }
                    else { binding.generic = GetProcAddress(compiled,"g_triangleRasterCursor"); global = binding.bytes; }
                    *(unsigned int *)global = (unsigned int)(uintptr_t)pixels;
                    clipped_fn[side](&packet);
                }
                if (side == 0) {
                    for (j = 0; j < 65536; j++) { expected_pixels[phase][j] = pixels[j]; }
                    for (g = 0; g < 3; g++) {
                        for (j = 0; j < sizeof(edges[g]); j++) { expected_edges[phase][g][j] = ((unsigned char *)edges[g])[j]; }
                    }
                    for (g = 0; g < sizeof(globals)/sizeof(globals[0]); g++) {
                        global = image + globals[g].rva;
                        for (j = 0; j < globals[g].size; j++) { expected_globals[phase][g][j] = global[j]; }
                    }
                } else {
                    for (j = 0; j < 65536; j++) {
                        if (expected_pixels[phase][j] != pixels[j]) { fail("FAIL native triangle frame bytes\n",24); }
                    }
                    for (g = 0; g < 3; g++) {
                        for (j = 0; j < sizeof(edges[g]); j++) {
                            if (expected_edges[phase][g][j] != ((unsigned char *)edges[g])[j]) { fail("FAIL native triangle edge bytes\n",25); }
                        }
                    }
                    for (g = 0; g < sizeof(globals)/sizeof(globals[0]); g++) {
                        binding.generic = GetProcAddress(compiled,globals[g].name); global = binding.bytes;
                        for (j = 0; j < globals[g].size; j++) {
                            if (expected_globals[phase][g][j] != global[j]) { fail("FAIL native triangle state bytes\n",26); }
                        }
                    }
                }
            }
        }
    }
    output("PASS native triangle frames: 48 paired parent calls; complete frame/edge/state bytes; real original/C89 renderer and actual configure/clip helpers. Caller-owned workspace fixtures; no native faults or game parity.\n");
    ExitProcess(0);
}
