/* Included by generated validation host; actual SDK frame observations. */
static void probe_native_fullscreen(unsigned char *image, HMODULE compiled, int side) {
    typedef int (__cdecl *PresentFn)(NativeRecord *);
    union { unsigned char *bytes; FARPROC generic; PresentFn present; OpenFn open; } binding;
    volatile int32_t *mode, *width, *height, *depth, *count;
    OpenFn open, shutdown, install;
    PresentFn direct, entry;
    PresentFn *installed;
    LPDIRECTDRAW *draw;
    NativeRecord *primary, *back;
    HWND *window;
    DDSURFACEDESC desc;
    volatile unsigned char *pixels;
    unsigned char *bytes;
    int setup, success, pass, k, index;
    uint32_t row, column;
    unsigned char pattern;

    if (side==0) {
        draw=(LPDIRECTDRAW *)(image+0x112c50);window=(HWND *)(image+0xc539c);
        primary=(NativeRecord *)(image+0x10e778);back=(NativeRecord *)(image+0x10e688);
        mode=(volatile int32_t *)(image+0x93728);width=(volatile int32_t *)(image+0xba6e0);
        height=(volatile int32_t *)(image+0xba6e4);depth=(volatile int32_t *)(image+0xba6e8);count=(volatile int32_t *)(image+0xba6ec);
        binding.bytes=image+0x5b740;open=binding.open;
        binding.bytes=image+0x5c060;shutdown=binding.open;
        binding.bytes=image+0x5b690;install=binding.open;
        binding.bytes=image+0x5c4b0;direct=binding.present;
        binding.bytes=image+0x56c90;entry=binding.present;
        installed=(PresentFn *)(image+0x10eb88);
    } else {
        draw=(LPDIRECTDRAW *)data_symbol(compiled,"g_nativeDirectDraw");window=(HWND *)data_symbol(compiled,"g_nativeWindow");
        primary=(NativeRecord *)data_symbol(compiled,"g_nativePrimarySurface");back=(NativeRecord *)data_symbol(compiled,"g_nativeType1Surfaces");
        mode=(volatile int32_t *)data_symbol(compiled,"g_nativeFullscreen");width=(volatile int32_t *)data_symbol(compiled,"g_nativeSurfaceWidth");
        height=(volatile int32_t *)data_symbol(compiled,"g_nativeSurfaceHeight");depth=(volatile int32_t *)data_symbol(compiled,"g_nativeSurfaceBitDepth");count=(volatile int32_t *)data_symbol(compiled,"g_nativeBackbufferCount");
        binding.generic=symbol(compiled,"Gfx_SurfaceOpenNative");open=binding.open;
        binding.generic=symbol(compiled,"Gfx_SurfaceShutdownNative");shutdown=binding.open;
        binding.generic=symbol(compiled,"Gfx_InstallSurfaceDispatch");install=binding.open;
        binding.generic=symbol(compiled,"Gfx_SurfacePresentNative");direct=binding.present;
        binding.generic=symbol(compiled,"Gfx_PresentSurfaceNative");entry=binding.present;
        installed=(PresentFn *)data_symbol(compiled,"g_surfacePresent");
    }
    *mode=1;*width=640;*height=480;*depth=8;*count=1;*window=0;
    /* The original reference WndProc reads this original mode in both sides. */
    *(uint32_t *)(image+0x93728)=1;
    setup=open();success=setup==1 && primary->surface && back->surface && install()==1 && *installed==direct;
    if (success) {
        DDSCAPS caps;
        if (back->surface->lpVtbl->GetCaps(back->surface,&caps)!=DD_OK || !(caps.dwCaps&DDSCAPS_FLIP)) { success=0; }
    }
    for (pass=0; pass<4 && success; pass++) {
        bytes=(unsigned char *)&desc;
        for (k=0; k<(int)sizeof(desc); k++) { bytes[k]=0; }
        desc.dwSize=sizeof(desc);
        if (back->surface->lpVtbl->Lock(back->surface,0,&desc,DDLOCK_WAIT,0)!=DD_OK) { success=0;break; }
        if (desc.dwWidth!=640 || desc.dwHeight!=480 || desc.lPitch<640 || desc.ddpfPixelFormat.dwRGBBitCount!=8) {
            back->surface->lpVtbl->Unlock(back->surface,0);success=0;break;
        }
        pixels=(volatile unsigned char *)desc.lpSurface;pattern=(unsigned char)(17+pass);
        for (row=0; row<480; row++) { for (column=0; column<640; column++) { pixels[row*(uint32_t)desc.lPitch+column]=pattern; } }
        if (back->surface->lpVtbl->Unlock(back->surface,0)!=DD_OK || (pass%2 ? entry(back) : direct(back))!=1) { success=0;break; }
        for (k=0; k<(int)sizeof(desc); k++) { bytes[k]=0; }
        desc.dwSize=sizeof(desc);
        if (primary->surface->lpVtbl->Lock(primary->surface,0,&desc,DDLOCK_WAIT|DDLOCK_READONLY,0)!=DD_OK) { success=0;break; }
        pixels=(volatile unsigned char *)desc.lpSurface;
        if (desc.dwWidth!=640 || desc.dwHeight!=480 || desc.lPitch<640 || desc.ddpfPixelFormat.dwRGBBitCount!=8) { success=0; }
        for (row=0; row<480 && success; row++) {
            for (column=0; column<640; column++) { if (pixels[row*(uint32_t)desc.lPitch+column]!=pattern) { success=0;break; } }
        }
        if (primary->surface->lpVtbl->Unlock(primary->surface,0)!=DD_OK) { success=0; }
    }
    /* Test-owned disposal/restoration even after partial constructor failure. */
    for (index=0; index<5; index++) { if (back[index].surface) { back[index].surface->lpVtbl->Release(back[index].surface);back[index].surface=0; } }
    if (primary->surface) { primary->surface->lpVtbl->Release(primary->surface);primary->surface=0; }
    if (*draw) {
        if ((*draw)->lpVtbl->RestoreDisplayMode(*draw)!=DD_OK) { success=0; }
        if ((*draw)->lpVtbl->SetCooperativeLevel(*draw,*window,DDSCL_NORMAL)!=DD_OK) { success=0; }
    }
    if (shutdown()!=1) { success=0; }
    if (*window) { if (!DestroyWindow(*window)) { success=0; } *window=0; }
    *mode=0;*(uint32_t *)(image+0x93728)=0;
    if (!success) { fail("FAIL paired native fullscreen setup/flip/full primary observation\n",110); }
    output(side==0 ? "PASS original fullscreen640x480x8 four flips and primary pixel equality\n" :
        "PASS rebuilt fullscreen640x480x8 four flips and primary pixel equality\n");
}
