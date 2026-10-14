#include <windows.h>
#include <stdint.h>

#include "renderer.h"
typedef int (__cdecl *CloseFn)(void);
typedef int (__cdecl *OptionFn)(unsigned int);
typedef int (__cdecl *ConfigureFn)(unsigned char *,int,int,int,unsigned int);
typedef int (__cdecl *ClipFn)(int,int,int,int);
typedef int (__cdecl *GetFn)(GfxSpriteState *);
typedef int (__cdecl *SetFn)(const GfxSpriteState *);
static unsigned char pixels[2048];
typedef struct NativeGlobal {
    const char *name;
    unsigned int rva;
    unsigned int size;
} NativeGlobal;
static const NativeGlobal globals[] = {
    /* GENERATED_GLOBALS */
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
    CloseFn close_fn[2];
    OptionFn option_fn[2];
    ConfigureFn configure_fn[2];
    ClipFn clip_fn[2];
    GetFn get_fn[2];
    SetFn set_fn[2];
    GfxSpriteState states[2];
    GfxSpriteState input;
    unsigned int side;
    unsigned int k;
    int left;
    int right;
    int original_result;
    int compiled_result;
    unsigned char *original_global;
    unsigned char *compiled_global;
    union {
        unsigned char *bytes;
        FARPROC generic;
        CloseFn close;
        OptionFn option;
        ConfigureFn configure;
        ClipFn clip;
        GetFn get;
        SetFn set;
    } binding;

    /* ORIGINAL_PE_MAPPING */

    compiled = LoadLibraryA("viewport_setters_validation.dll");
    if (compiled == 0) {
        fail("FAIL native viewport DLL\n",20);
    }
    binding.bytes = image + 0x56f40;
    close_fn[0] = binding.close;
    binding.generic = GetProcAddress(compiled,"Gfx_SpriteCloseNative");
    close_fn[1] = binding.close;
    if (close_fn[1] == 0) { fail("FAIL native viewport export\n",21); }
    binding.bytes = image + 0x56f50;
    option_fn[0] = binding.option;
    binding.generic = GetProcAddress(compiled,"Gfx_SpriteOptionNative");
    option_fn[1] = binding.option;
    if (option_fn[1] == 0) { fail("FAIL native viewport export\n",21); }
    binding.bytes = image + 0x56f60;
    configure_fn[0] = binding.configure;
    binding.generic = GetProcAddress(compiled,"Gfx_SpriteConfigureNative");
    configure_fn[1] = binding.configure;
    if (configure_fn[1] == 0) { fail("FAIL native viewport export\n",21); }
    binding.bytes = image + 0x57020;
    clip_fn[0] = binding.clip;
    binding.generic = GetProcAddress(compiled,"Gfx_SpriteSetClipNative");
    clip_fn[1] = binding.clip;
    if (clip_fn[1] == 0) { fail("FAIL native viewport export\n",21); }
    binding.bytes = image + 0x57110;
    get_fn[0] = binding.get;
    binding.generic = GetProcAddress(compiled,"Gfx_SpriteGetStateNative");
    get_fn[1] = binding.get;
    if (get_fn[1] == 0) { fail("FAIL native viewport export\n",21); }
    binding.bytes = image + 0x57170;
    set_fn[0] = binding.set;
    binding.generic = GetProcAddress(compiled,"Gfx_SpriteSetStateNative");
    set_fn[1] = binding.set;
    if (set_fn[1] == 0) { fail("FAIL native viewport export\n",21); }
    for (n = 0; n < 64; n++) {
        for (side = 0; side < 2; side++) {
            if (close_fn[side]() != 1 || option_fn[side](n) != 0) {
                fail("FAIL native close/option\n",22);
            }
            if (configure_fn[side](pixels,64,n % 8 == 0 ? -7 : 32,24,n) != 1) {
                fail("FAIL native configure\n",23);
            }
        }
        left = n % 4 == 0 ? -1 : (int)(n % 4);
        right = n % 4 == 1 ? 33 : 31;
        original_result = clip_fn[0](left,0,right,23);
        compiled_result = clip_fn[1](left,0,right,23);
        if (original_result != compiled_result) {
            fail("FAIL native clip return\n",24);
        }
        for (k = 0; k < 2; k++) {
            for (side = 0; side < 2; side++) {
                if (get_fn[side](&states[side]) != 1) {
                    fail("FAIL native getter return\n",25);
                }
            }
            for (j = 0; j < sizeof(GfxSpriteState); j++) {
                if (((unsigned char *)&states[0])[j] != ((unsigned char *)&states[1])[j]) {
                    fail("FAIL native state bytes\n",26);
                }
            }
            for (j = 0; j < sizeof(globals) / sizeof(globals[0]); j++) {
                original_global = image + globals[j].rva;
                binding.generic = GetProcAddress(compiled,globals[j].name);
                compiled_global = binding.bytes;
                if (compiled_global == 0) { fail("FAIL native global export\n",27); }
                for (side = 0; side < globals[j].size; side++) {
                    if (original_global[side] != compiled_global[side]) {
                        fail("FAIL native viewport/global bytes\n",28);
                    }
                }
            }
            if (k == 0) {
                input.width = 32;
                input.height = 24;
                input.option = n ^ 0x1234u;
                input.clip_left = 0;
                input.clip_top = 0;
                input.clip_right = n % 2 != 0 ? 33 : 31;
                input.clip_bottom = 23;
                input.stride = 64;
                input.format = 71;
                input.pixels = pixels;
                for (side = 0; side < 2; side++) {
                    if (set_fn[side](&input) != 1 || input.format != 71) {
                        fail("FAIL native setter return/input\n",29);
                    }
                }
            }
        }
    }
    output("PASS native viewport: 448 paired cdecl calls; actual original/C89 returns, state records and all16 shared objects. No native faults, rendered frames or game parity.\n");
    ExitProcess(0);
}
