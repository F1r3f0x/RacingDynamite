/* Actual SDK palette observations; included in the shared native-loader host. */
static void probe_native_palette(unsigned char *image, HMODULE compiled, int side, LPDIRECTDRAW draw) {
    typedef int (__cdecl *PaletteFn)(const unsigned char *);
    union { unsigned char *bytes; FARPROC generic; PaletteFn palette; OpenFn install; } binding;
    PaletteFn *installed;
    OpenFn install;
    PaletteFn direct, entry;
    LPDIRECTDRAWPALETTE palette;
    LPDIRECTDRAWPALETTE *global;
    PALETTEENTRY initial[256], actual[256];
    volatile PALETTEENTRY *bank;
    unsigned char rgb[768];
    unsigned int k, pass;

    for (k=0; k<256; k++) {
        initial[k].peRed=0;initial[k].peGreen=0;initial[k].peBlue=0;initial[k].peFlags=0;
    }
    palette=0;
    if (draw->lpVtbl->CreatePalette(draw,DDPCAPS_8BIT|DDPCAPS_ALLOW256,initial,&palette,0)!=DD_OK || !palette) {
        fail("FAIL paired native actual SDK8bit palette creation\n",90);
    }
    if (side==0) {
        global=(LPDIRECTDRAWPALETTE *)(image+0x112448);bank=(volatile PALETTEENTRY *)(image+0x112450);
        binding.bytes=image+0x5b690;install=binding.install;
        binding.bytes=image+0x5c6d0;direct=binding.palette;
        binding.bytes=image+0x56c40;entry=binding.palette;
        installed=(PaletteFn *)(image+0x10eb98);
    } else {
        global=(LPDIRECTDRAWPALETTE *)data_symbol(compiled,"g_nativePalette");
        bank=(volatile PALETTEENTRY *)data_symbol(compiled,"g_nativeSetPaletteEntries");
        binding.generic=symbol(compiled,"Gfx_InstallSurfaceDispatch");install=binding.install;
        binding.generic=symbol(compiled,"Gfx_SurfaceSetPaletteNative");direct=binding.palette;
        binding.generic=symbol(compiled,"Gfx_SetPaletteNative");entry=binding.palette;
        installed=(PaletteFn *)data_symbol(compiled,"g_surfaceSetPalette");
    }
    if (*global) { palette->lpVtbl->Release(palette);fail("FAIL palette boundary occupied\n",91); }
    if (install()!=1 || *installed!=direct) { palette->lpVtbl->Release(palette);fail("FAIL palette actual installer\n",92); }
    for (k=0; k<256; k++) { bank[k].peFlags=0; }
    for (pass=0; pass<4; pass++) {
        for (k=0; k<256; k++) {
            rgb[3*k]=(unsigned char)(17*k+pass);rgb[3*k+1]=(unsigned char)(5*k+3*pass);
            rgb[3*k+2]=(unsigned char)(255-k+pass);
        }
        *global=palette;
        if ((pass%2 ? entry(rgb) : direct(rgb))!=1 ||
                palette->lpVtbl->GetEntries(palette,0,0,256,actual)!=DD_OK) {
            *global=0;palette->lpVtbl->Release(palette);fail("FAIL paired native actual SetEntries/GetEntries\n",93);
        }
        for (k=0; k<256; k++) {
            if (actual[k].peRed!=rgb[3*k] || actual[k].peGreen!=rgb[3*k+1] || actual[k].peBlue!=rgb[3*k+2] ||
                    bank[k].peRed!=rgb[3*k] || bank[k].peGreen!=rgb[3*k+1] || bank[k].peBlue!=rgb[3*k+2] || bank[k].peFlags!=0) {
                *global=0;palette->lpVtbl->Release(palette);fail("FAIL paired native SDK palette RGB/flags\n",94);
            }
        }
        *global=0;
        if (direct(0)!=0 || entry(0)!=0) { palette->lpVtbl->Release(palette);fail("FAIL paired native null palette guard\n",95); }
    }
    palette->lpVtbl->Release(palette);
    output(side==0 ? "PASS original real SDK palette RGB, flags, repeated direct/installed entry and null guard\n" : "PASS rebuilt real SDK palette RGB, flags, repeated direct/installed entry and null guard\n");
}
