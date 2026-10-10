#include <windows.h>
#include <ddraw.h>
#include <stdint.h>

#ifdef SURFACE_TEST_ENTRY
#define SURFACE_TEST_REBUILD
#endif

typedef int (__cdecl *OpenFn)(void);
typedef struct NativeRecord {
    uint32_t active, marker, opaque[5], width, height, depth, kind;
    LPDIRECTDRAWSURFACE surface;
} NativeRecord;
typedef char record_size[(sizeof(NativeRecord)==48)?1:-1];

static void output(const char *text) {
    DWORD written, n;
    n = 0;
    while (text[n]) { n++; }
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), text, n, &written, 0);
}
static void fail(const char *text, DWORD code) {
    output(text);
    ExitProcess(code);
}
static int equal_text(const char *left, const char *right) {
    while (*left && *left==*right) { left++; right++; }
    return *left==*right;
}
static FARPROC symbol(HMODULE module, const char *name) {
    FARPROC address;
    address = GetProcAddress(module, name);
    if (!address) { fail("FAIL native export/import\n", 30); }
    return address;
}

static void *data_symbol(HMODULE module, const char *name) {
    union { FARPROC function; void *object; } binding;
    binding.function=symbol(module,name);
    return binding.object;
}

static void check_records(NativeRecord *primary, NativeRecord *bank1,
        NativeRecord *bank2, int count, int success, uint32_t depth) {
    NativeRecord *record;
    DDSURFACEDESC desc;
    int bank, index, n, k, active;
    for (bank=0; bank<3; bank++) {
        n = bank==0 ? 1 : bank==1 ? 5 : 20;
        for (index=0; index<n; index++) {
            record = bank==0 ? primary : bank==1 ? &bank1[index] : &bank2[index];
            active = success && (bank==0 || (bank==1 && index<count));
            if (record->kind!=(uint32_t)bank || record->active!=(uint32_t)active ||
                record->marker!=(uint32_t)active || (!!record->surface)!=active ||
                record->width!=(uint32_t)(active ? 640 : 0) ||
                record->height!=(uint32_t)(active ? 480 : 0) ||
                record->depth!=(active ? depth : 0)) {
                fail("FAIL native record state\n", 31);
            }
            for (k=0; k<5; k++) {
                if (record->opaque[k]!=0) { fail("FAIL native opaque words\n", 32); }
            }
            if (active) {
                unsigned char *bytes;
                bytes=(unsigned char *)&desc;
                for (k=0; k<(int)sizeof(desc); k++) { bytes[k]=0; }
                desc.dwSize=sizeof(desc);
                if (record->surface->lpVtbl->GetSurfaceDesc(record->surface,&desc)!=DD_OK) {
                    fail("FAIL native surface descriptor\n", 33);
                }
                if (bank==1 && (desc.dwWidth!=640 || desc.dwHeight!=480 ||
                    !(desc.ddsCaps.dwCaps&DDSCAPS_OFFSCREENPLAIN))) {
                    fail("FAIL native offscreen surface dimensions/caps\n", 34);
                }
                if (bank==0 && !(desc.ddsCaps.dwCaps&DDSCAPS_PRIMARYSURFACE)) {
                    fail("FAIL native primary surface caps\n", 35);
                }
            }
        }
    }
}

void WINAPI NativeEntry(void) {
    HANDLE file;
    DWORD size, read, delta, reloc_offset, reloc_end, target;
    unsigned char *raw, *image;
    IMAGE_DOS_HEADER *dos;
    IMAGE_NT_HEADERS32 *nt;
    IMAGE_SECTION_HEADER *section;
    IMAGE_BASE_RELOCATION *reloc;
    WORD *entries;
    uint32_t *word;
    unsigned int n, j;
    HMODULE compiled, module;
    IMAGE_IMPORT_DESCRIPTOR *import;
    IMAGE_THUNK_DATA32 *lookup, *iat;
    IMAGE_IMPORT_BY_NAME *entry;
    FARPROC procedure;
    WNDCLASSA window_class;
    HINSTANCE instance;
    HCURSOR cursor;
    OpenFn original_fn, rebuilt_fn;
#if defined(SURFACE_TEST_SHUTDOWN) || defined(SURFACE_TEST_REBUILD)
    OpenFn shutdown_fn[2];
#ifdef SURFACE_TEST_REBUILD
    OpenFn finalize_fn[2];
    LPDIRECTDRAWSURFACE retained_surfaces[6];
    LPDIRECTDRAW retained_draw;
#endif
    LPDIRECTDRAWCLIPPER retained_clipper;
    DDSURFACEDESC extra_desc;
#endif
    NativeRecord *records[2][3];
    volatile HINSTANCE *instances[2];
    volatile HWND *windows[2], *duplicates[2];
    volatile LPDIRECTDRAW *draws[2];
    volatile LPDIRECTDRAWCLIPPER *clippers[2];
    volatile LPDIRECTDRAWPALETTE *palettes[2];
    volatile int32_t *modes[2], *widths[2], *heights[2], *depths[2], *counts[2];
    int pass, side, bank, count, result, k;
#if !defined(SURFACE_TEST_SHUTDOWN) && !defined(SURFACE_TEST_REBUILD)
    int index;
#endif
    HWND window, clip_window;
    RECT client;
    LPDIRECTDRAWCLIPPER installed;
    LONG style;
    uint32_t *words;
    union { unsigned char *bytes; OpenFn open; WNDPROC window; FARPROC generic; } binding;
    file = CreateFileA("IGN_WIN.EXE", GENERIC_READ, FILE_SHARE_READ, 0,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file == INVALID_HANDLE_VALUE) {
        fail("FAIL authentic disposable input open\n", 1);
    }
    size = GetFileSize(file, 0);
    if (size != 915968) {
        fail("FAIL authentic size\n", 2);
    }
    raw = (unsigned char *)VirtualAlloc((void *)0x30000000, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (raw == 0 || !ReadFile(file, raw, size, &read, 0) || read != size) {
        fail("FAIL authentic read\n", 3);
    }
    CloseHandle(file);
    dos = (IMAGE_DOS_HEADER *)raw;
    nt = (IMAGE_NT_HEADERS32 *)(raw + dos->e_lfanew);
    if (dos->e_magic != 0x5a4d || nt->Signature != 0x4550 ||
        nt->FileHeader.Machine != 0x14c || nt->OptionalHeader.Magic != 0x10b ||
        nt->OptionalHeader.ImageBase != 0x400000) {
        fail("FAIL authentic PE32\n", 4);
    }
    image = (unsigned char *)VirtualAlloc((void *)0x400000,
        nt->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (image == 0) {
        image = (unsigned char *)VirtualAlloc(0, nt->OptionalHeader.SizeOfImage,
            MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    }
    if (image == 0) {
        fail("FAIL original native mapping\n", 5);
    }
    for (j = 0; j < nt->OptionalHeader.SizeOfHeaders; j++) {
        image[j] = raw[j];
    }
    section = IMAGE_FIRST_SECTION(nt);
    for (n = 0; n < nt->FileHeader.NumberOfSections; n++) {
        if (section[n].PointerToRawData + section[n].SizeOfRawData > size ||
            section[n].VirtualAddress + section[n].SizeOfRawData > nt->OptionalHeader.SizeOfImage) {
            fail("FAIL original section bounds\n", 6);
        }
        for (j = 0; j < section[n].SizeOfRawData; j++) {
            image[section[n].VirtualAddress + j] = raw[section[n].PointerToRawData + j];
        }
    }
    delta = (DWORD)image - nt->OptionalHeader.ImageBase;
    reloc_offset = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
    reloc_end = reloc_offset + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
    if (reloc_end > nt->OptionalHeader.SizeOfImage) {
        fail("FAIL relocation directory bounds\n", 13);
    }
    while (reloc_offset < reloc_end) {
        reloc = (IMAGE_BASE_RELOCATION *)(image + reloc_offset);
        if (reloc->SizeOfBlock < 8 || reloc_offset + reloc->SizeOfBlock > reloc_end) {
            fail("FAIL relocation block bounds\n", 14);
        }
        entries = (WORD *)(reloc + 1);
        for (n = 0; n < (reloc->SizeOfBlock - 8) / 2; n++) {
            if ((entries[n] >> 12) == IMAGE_REL_BASED_ABSOLUTE) {
                continue;
            }
            if ((entries[n] >> 12) != IMAGE_REL_BASED_HIGHLOW) {
                fail("FAIL unsupported PE32 relocation\n", 15);
            }
            target = reloc->VirtualAddress + (entries[n] & 0xfff);
            if (target + 4 > nt->OptionalHeader.SizeOfImage) {
                fail("FAIL relocation operand bounds\n", 16);
            }
            word = (uint32_t *)(image + target);
            *word += delta;
        }
        reloc_offset += reloc->SizeOfBlock;
    }
    /* Bind only actual platform imports used by the constructor/reference
     * WndProc. Game/input/CRT startup is not invoked or replaced by success. */
    import=(IMAGE_IMPORT_DESCRIPTOR *)(image+nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    while (import->Name) {
        const char *name;
        name=(const char *)(image+import->Name);
        if (equal_text(name,"USER32.dll") || equal_text(name,"GDI32.dll") || equal_text(name,"DDRAW.dll")) {
            module=LoadLibraryA(name);
            if (!module) { fail("FAIL native platform module\n",36); }
            lookup=(IMAGE_THUNK_DATA32 *)(image+import->OriginalFirstThunk);
            iat=(IMAGE_THUNK_DATA32 *)(image+import->FirstThunk);
            while (lookup->u1.AddressOfData) {
                if (lookup->u1.Ordinal&IMAGE_ORDINAL_FLAG32) { fail("FAIL unexpected platform ordinal\n",37); }
                entry=(IMAGE_IMPORT_BY_NAME *)(image+lookup->u1.AddressOfData);
                procedure=symbol(module,(const char *)entry->Name);
                iat->u1.Function=(DWORD)procedure;
                lookup++; iat++;
            }
        }
        import++;
    }
    compiled=LoadLibraryA("surface_open_validation.dll");
    if (!compiled) { fail("FAIL native production DLL load\n",38); }
#ifdef SURFACE_TEST_ENTRY
    /* Execute the real surface installer. Sprite installation also allocates
     * workspaces, outside this host's scope; bind only its authenticated two
     * lifecycle slots to their real original/rebuilt bodies. */
    binding.bytes=image+0x5b690;
    if (binding.open()!=1) { fail("FAIL native original surface installer\n",58); }
    binding.generic=symbol(compiled,"Gfx_InstallSurfaceDispatch");
    if (binding.open()!=1) { fail("FAIL native rebuilt surface installer\n",59); }
    binding.bytes=image+0x56f20;*(OpenFn volatile *)(image+0x10eba0)=binding.open;
    binding.bytes=image+0x56f30;*(OpenFn volatile *)(image+0x10eba4)=binding.open;
    binding.generic=symbol(compiled,"Gfx_SpriteOpenNative");
    *(OpenFn volatile *)data_symbol(compiled,"g_spriteOpen")=binding.open;
    binding.generic=symbol(compiled,"Gfx_SpriteShutdownNative");
    *(OpenFn volatile *)data_symbol(compiled,"g_spriteShutdown")=binding.open;
    binding.bytes=image+0x56bc0;original_fn=binding.open;
    binding.generic=symbol(compiled,"Gfx_Open");rebuilt_fn=binding.open;
#else
    binding.bytes=image+0x5b740;original_fn=binding.open;
    binding.generic=symbol(compiled,"Gfx_SurfaceOpenNative");rebuilt_fn=binding.open;
#endif
    records[0][0]=(NativeRecord *)(image+0x10e778);
    records[0][1]=(NativeRecord *)(image+0x10e688);
    records[0][2]=(NativeRecord *)(image+0x10e7a8);
    records[1][0]=(NativeRecord *)data_symbol(compiled,"g_nativePrimarySurface");
    records[1][1]=(NativeRecord *)data_symbol(compiled,"g_nativeType1Surfaces");
    records[1][2]=(NativeRecord *)data_symbol(compiled,"g_nativeType2Surfaces");
    instances[0]=(volatile HINSTANCE *)(image+0xc5398);
    windows[0]=(volatile HWND *)(image+0xc539c);duplicates[0]=(volatile HWND *)(image+0x93738);
    draws[0]=(volatile LPDIRECTDRAW *)(image+0x112c50);
    clippers[0]=(volatile LPDIRECTDRAWCLIPPER *)(image+0xbac8c);
    palettes[0]=(volatile LPDIRECTDRAWPALETTE *)(image+0x112448);
    modes[0]=(volatile int32_t *)(image+0x93728);
    widths[0]=(volatile int32_t *)(image+0xba6e0);heights[0]=(volatile int32_t *)(image+0xba6e4);
    depths[0]=(volatile int32_t *)(image+0xba6e8);counts[0]=(volatile int32_t *)(image+0xba6ec);
    instances[1]=(volatile HINSTANCE *)data_symbol(compiled,"g_nativeWindowInstance");
    windows[1]=(volatile HWND *)data_symbol(compiled,"g_nativeWindow");duplicates[1]=(volatile HWND *)data_symbol(compiled,"g_nativeSurfaceWindow");
    draws[1]=(volatile LPDIRECTDRAW *)data_symbol(compiled,"g_nativeDirectDraw");
    clippers[1]=(volatile LPDIRECTDRAWCLIPPER *)data_symbol(compiled,"g_nativeClipper");
    palettes[1]=(volatile LPDIRECTDRAWPALETTE *)data_symbol(compiled,"g_nativePalette");
    modes[1]=(volatile int32_t *)data_symbol(compiled,"g_nativeFullscreen");
    widths[1]=(volatile int32_t *)data_symbol(compiled,"g_nativeSurfaceWidth");
    heights[1]=(volatile int32_t *)data_symbol(compiled,"g_nativeSurfaceHeight");
    depths[1]=(volatile int32_t *)data_symbol(compiled,"g_nativeSurfaceBitDepth");
    counts[1]=(volatile int32_t *)data_symbol(compiled,"g_nativeBackbufferCount");
#if defined(SURFACE_TEST_SHUTDOWN) || defined(SURFACE_TEST_REBUILD)
#ifdef SURFACE_TEST_REBUILD
#ifdef SURFACE_TEST_ENTRY
    binding.bytes=image+0x56be0;shutdown_fn[0]=binding.open;
    binding.generic=symbol(compiled,"Gfx_Rebuild");shutdown_fn[1]=binding.open;
    binding.bytes=image+0x56bf0;finalize_fn[0]=binding.open;
    binding.generic=symbol(compiled,"Gfx_Shutdown");finalize_fn[1]=binding.open;
#else
    binding.bytes=image+0x5bd70;shutdown_fn[0]=binding.open;
    binding.generic=symbol(compiled,"Gfx_SurfaceRebuildNative");shutdown_fn[1]=binding.open;
    binding.bytes=image+0x5c060;finalize_fn[0]=binding.open;
    binding.generic=symbol(compiled,"Gfx_SurfaceShutdownNative");finalize_fn[1]=binding.open;
#endif
#else
    binding.bytes=image+0x5c060;shutdown_fn[0]=binding.open;
    binding.generic=symbol(compiled,"Gfx_SurfaceShutdownNative");shutdown_fn[1]=binding.open;
#endif
#endif
    instance=GetModuleHandleA(0);
    cursor=LoadCursorA(0,MAKEINTRESOURCEA(32512));
    *(HCURSOR *)(image+0xc5360)=cursor;
    /* Prevent reference WndProc from entering unreconstructed game shutdown;
     * no Alt-Enter/input path is sent. Native constructor scope is explicit. */
    *(uint32_t *)(image+0x9372c)=1;
    *(uint32_t *)(image+0xc5390)=0;
    for (pass=0; pass<4; pass++) {
        if (pass==1) {
            unsigned char *bytes;
            bytes=(unsigned char *)&window_class;
            for (k=0; k<(int)sizeof(window_class); k++) { bytes[k]=0; }
            window_class.style=8;
            binding.bytes=image+0x122d0;window_class.lpfnWndProc=binding.window;
            window_class.hInstance=instance;
            window_class.hIcon=LoadIconA(0,MAKEINTRESOURCEA(32512));
            window_class.hCursor=cursor;
            window_class.hbrBackground=(HBRUSH)GetStockObject(4);
            window_class.lpszClassName=(const char *)(image+0x93740);
            if (!RegisterClassA(&window_class)) { fail("FAIL native original class registration\n",39); }
        }
        count=pass<2 ? 0 : pass==2 ? 1 : 4;
        for (side=0; side<2; side++) {
            for (bank=0; bank<3; bank++) {
                words=(uint32_t *)records[side][bank];
                n=bank==0 ? 12 : bank==1 ? 60 : 240;
                for (j=0; j<n; j++) { words[j]=0xa5a50000u+j; }
            }
            *instances[side]=instance;*windows[side]=0;*duplicates[side]=0;
            *draws[side]=0;*clippers[side]=0;*palettes[side]=0;
            *modes[side]=0;*widths[side]=640;*heights[side]=480;*depths[side]=8;*counts[side]=count;
            /* Both constructors use the exact same authenticated reference
             * procedure/class. Its global mode must match the test caller. */
            *modes[0]=0;
            result=side==0 ? original_fn() : rebuilt_fn();
            if (result!=(pass!=0)) { fail("FAIL native constructor result\n",40); }
            check_records(records[side][0],records[side][1],records[side][2],count,pass!=0,8);
            if (pass==0) {
                if (*windows[side] || *duplicates[side] || *draws[side]) { fail("FAIL missing-class retained state\n",41); }
#ifdef SURFACE_TEST_SHUTDOWN
                if (shutdown_fn[side]()!=1 || *draws[side] || *clippers[side]) {
                    fail("FAIL empty native shutdown\n",50);
                }
#endif
                continue;
            }
            window=*windows[side];
            if (!window || *duplicates[side]!=window || !IsWindow(window) || !IsWindowVisible(window)) {
                fail("FAIL native HWND publication/show\n",42);
            }
            if (!GetClientRect(window,&client) || client.right-client.left!=640 || client.bottom-client.top!=480) {
                fail("FAIL native client geometry\n",43);
            }
            style=GetWindowLongA(window,GWL_STYLE);
            if ((style&WS_POPUP) || (style&0xc60000)!=0xc60000) { fail("FAIL native window style\n",44); }
            if (!*draws[side] || !*clippers[side] || *palettes[side]) { fail("FAIL native interface ownership\n",45); }
            if ((*clippers[side])->lpVtbl->GetHWnd(*clippers[side],&clip_window)!=DD_OK || clip_window!=window) {
                fail("FAIL native clipper HWND\n",46);
            }
            installed=0;
            if (records[side][0]->surface->lpVtbl->GetClipper(records[side][0]->surface,&installed)!=DD_OK || !installed) {
                fail("FAIL native installed clipper\n",47);
            }
            installed->lpVtbl->Release(installed);
#if defined(SURFACE_TEST_SHUTDOWN) || defined(SURFACE_TEST_REBUILD)
            {
                unsigned char *bytes;
                bytes=(unsigned char *)&extra_desc;
                for (k=0; k<(int)sizeof(extra_desc); k++) { bytes[k]=0; }
                extra_desc.dwSize=sizeof(extra_desc);
                extra_desc.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
                extra_desc.dwWidth=32;extra_desc.dwHeight=16;
                extra_desc.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN;
                for (j=0; j<20; j+=19) {
                    NativeRecord *record;
                    record=&records[side][2][j];
                    if ((*draws[side])->lpVtbl->CreateSurface(*draws[side],&extra_desc,&record->surface,0)!=DD_OK) {
                        fail("FAIL native type2 surface setup\n",53);
                    }
                    record->active=0;record->marker=0x12345678;
                    record->width=32;record->height=16;record->depth=8;
                }
            }
#ifdef SURFACE_TEST_REBUILD
            retained_draw=*draws[side];
            retained_surfaces[0]=records[side][0]->surface;
            for (j=0; j<(unsigned int)count; j++) { retained_surfaces[j+1]=records[side][1][j].surface; }
#endif
            retained_clipper=*clippers[side];
#ifdef SURFACE_TEST_REBUILD
            if (shutdown_fn[side]()!=1 || *draws[side]!=retained_draw || *clippers[side]!=retained_clipper ||
                    *windows[side]!=window || !IsWindowVisible(window)) {
                fail("FAIL native rebuild retained globals\n",54);
            }
            check_records(records[side][0],records[side][1],records[side][2],0,0,8);
            if (shutdown_fn[side]()!=1 || *draws[side]!=retained_draw) { fail("FAIL repeated native rebuild\n",55); }
            check_records(records[side][0],records[side][1],records[side][2],0,0,8);
            for (j=0; j<(unsigned int)count+1; j++) {
                unsigned char *bytes;
                bytes=(unsigned char *)&extra_desc;
                for (k=0; k<(int)sizeof(extra_desc); k++) { bytes[k]=0; }
                extra_desc.dwSize=sizeof(extra_desc);
                if (retained_surfaces[j]->lpVtbl->GetSurfaceDesc(retained_surfaces[j],&extra_desc)!=DD_OK) {
                    fail("FAIL native rebuild unexpectedly released retained surface\n",56);
                }
                retained_surfaces[j]->lpVtbl->Release(retained_surfaces[j]);
            }
            if (finalize_fn[side]()!=1 || *draws[side] || *clippers[side]!=retained_clipper ||
                    !IsWindow(window)) {
                fail("FAIL native rebuild/shutdown production sequence\n",57);
            }
#else
            if (shutdown_fn[side]()!=1 || *draws[side] || *clippers[side]!=retained_clipper ||
                    *windows[side]!=window || !IsWindow(window)) {
                fail("FAIL native shutdown retained globals\n",51);
            }
            for (bank=0; bank<3; bank++) {
                n=bank==0 ? 1 : bank==1 ? 5 : 20;
                for (j=0; j<n; j++) {
                    NativeRecord *record;
                    int prior_active, extra;
                    record=&records[side][bank][j];
                    prior_active=bank==0 || (bank==1 && (int)j<count);
                    extra=bank==2 && (j==0 || j==19);
                    if (record->active || record->surface || record->kind!=(uint32_t)bank ||
                            record->marker!=(extra ? 0x12345678u : (uint32_t)prior_active) ||
                            record->width!=(uint32_t)(extra ? 32 : prior_active ? 640 : 0) ||
                            record->height!=(uint32_t)(extra ? 16 : prior_active ? 480 : 0) ||
                            record->depth!=(uint32_t)(extra || prior_active ? 8 : 0)) {
                        fail("FAIL native shutdown record retention\n",52);
                    }
                }
            }
#endif
#else
            /* Test-owned cleanup. Rebuilt native shutdown remains a next
             * feature; this cleanup is not production behavior or evidence. */
            for (index=0; index<count; index++) {
                records[side][1][index].surface->lpVtbl->Release(records[side][1][index].surface);
            }
            records[side][0]->surface->lpVtbl->Release(records[side][0]->surface);
            (*clippers[side])->lpVtbl->Release(*clippers[side]);
            (*draws[side])->lpVtbl->Release(*draws[side]);
            *draws[side]=0;*clippers[side]=0;
#endif
            if (!DestroyWindow(window)) { fail("FAIL native test window disposal\n",48); }
        }
    }
    if (!UnregisterClassA("Ignition",instance)) { fail("FAIL native class disposal\n",49); }
#if defined(SURFACE_TEST_SHUTDOWN) || defined(SURFACE_TEST_REBUILD)
#ifdef SURFACE_TEST_REBUILD
#ifdef SURFACE_TEST_ENTRY
    output("PASS native Win32/DirectDraw original versus C89 entry: real surface installer, graphics open/repeated rebuild/shutdown and real sprite leaves; missing class and windowed 0/1/4 backbuffers; original WndProc reference boundary; no native game parity\n");
#else
    output("PASS native Win32/DirectDraw original versus C89 rebuild: windowed 0/1/4 backbuffers and type2 boundary releases; repeated real rebuild clears records while retaining primary/type1 interfaces; original WndProc reference boundary; no native game parity\n");
#endif
#else
    output("PASS native Win32/DirectDraw original versus C89 shutdown: missing class and 0/1/4 windowed backbuffers; real production releases and retained clipper/record metadata; original WndProc reference boundary; no native game parity\n");
#endif
#else
    output("PASS native Win32/DirectDraw original versus C89 constructor: missing class and 0/1/4 windowed backbuffers; authenticated original WndProc reference boundary; no native game parity\n");
#endif
    ExitProcess(0);
}
