static void blit_probe_number(const char *label, unsigned int value) {
    char digits[12];unsigned int length,index;char temporary;
    output(label);length=0;
    do { digits[length++]=(char)('0'+value%10);value/=10; } while(value);
    for(index=0;index<length/2;index++) { temporary=digits[index];digits[index]=digits[length-1-index];digits[length-1-index]=temporary; }
    digits[length++]='\n';digits[length]=0;output(digits);
}
static void zero_blit_buffer(void *target, unsigned int count) {
    volatile unsigned char *bytes;
    bytes=(volatile unsigned char *)target;
    while (count) { *bytes++=0;count--; }
}
/* Paired original/C89 SDK pixel contract; all surfaces and seeds are test-owned. */
static void probe_native_blit(unsigned char *image, HMODULE compiled, int side, LPDIRECTDRAW draw) {
    typedef int (__cdecl *BlitFn)(NativeRecord *, int, int, int, int, NativeRecord *, int, int);
    union { unsigned char *bytes; FARPROC generic; BlitFn blit; OpenFn install; } binding;
    DDSURFACEDESC description, source_lock, target_lock;
    LPDIRECTDRAWSURFACE source, target;
    NativeRecord source_record, target_record;
    static unsigned char expected[64 * 64 * 4];
    unsigned char *row;
    unsigned int bytes, x, y, component, pass;
    DWORD pixel_mask;
    BlitFn direct, entry;
    OpenFn install;
    BlitFn *installed;

    zero_blit_buffer(&description, sizeof(description));
    description.dwSize = sizeof(description);
    description.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    description.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    description.dwWidth = 64;
    description.dwHeight = 64;
    source = 0;
    target = 0;
    if (draw->lpVtbl->CreateSurface(draw, &description, &source, 0) != DD_OK ||
            draw->lpVtbl->CreateSurface(draw, &description, &target, 0) != DD_OK) {
        fail("FAIL paired blit owned SDK surfaces\n", 100);
    }
    zero_blit_buffer(&source_record, sizeof(source_record));
    zero_blit_buffer(&target_record, sizeof(target_record));
    source_record.active = 1;
    source_record.width = source_record.height = 64;
    source_record.kind = 2;
    source_record.surface = source;
    target_record = source_record;
    target_record.surface = target;
    if (side==0) {
        binding.bytes=image+0x5c160;direct=binding.blit;
        binding.bytes=image+0x56b30;entry=binding.blit;
        binding.bytes=image+0x5b690;install=binding.install;
        installed=(BlitFn *)(image+0x10eb7c);
    } else {
        binding.generic=symbol(compiled,"Gfx_SurfaceBlitNative");direct=binding.blit;
        binding.generic=symbol(compiled,"Gfx_SurfaceBlitDispatch");entry=binding.blit;
        binding.generic=symbol(compiled,"Gfx_InstallSurfaceDispatch");install=binding.install;
        installed=(BlitFn *)data_symbol(compiled,"g_surfaceBlit");
    }
    if (install() != 1 || *installed != direct) {
        fail("FAIL paired blit installer\n", 101);
    }
    for (pass = 0; pass < 4; pass++) {
        source_record.active=pass==1 ? 2u : pass==2 ? 0xffffffffu : 1u;
        target_record.active=pass==1 ? 0xffffffffu : pass==2 ? 2u : 1u;
        zero_blit_buffer(&source_lock, sizeof(source_lock));
        source_lock.dwSize = sizeof(source_lock);
        zero_blit_buffer(&target_lock, sizeof(target_lock));
        target_lock.dwSize = sizeof(target_lock);
        if (source->lpVtbl->Lock(source, 0, &source_lock, DDLOCK_WAIT, 0) != DD_OK ||
                target->lpVtbl->Lock(target, 0, &target_lock, DDLOCK_WAIT, 0) != DD_OK) {
            fail("FAIL paired blit seed locks\n", 102);
        }
        bytes = source_lock.ddpfPixelFormat.dwRGBBitCount / 8;
        if (!bytes || bytes > 4 || source_lock.ddpfPixelFormat.dwRGBBitCount % 8 ||
                target_lock.ddpfPixelFormat.dwRGBBitCount != 8 * bytes ||
                source_lock.lPitch < (LONG)(64 * bytes) || target_lock.lPitch < (LONG)(64 * bytes)) {
            fail("FAIL paired blit actual SDK format/pitch\n", 103);
        }
        pixel_mask=source_lock.ddpfPixelFormat.dwRBitMask | source_lock.ddpfPixelFormat.dwGBitMask | source_lock.ddpfPixelFormat.dwBBitMask | source_lock.ddpfPixelFormat.dwRGBAlphaBitMask;
        if (bytes==1) { pixel_mask=0xff; }
        for (y = 0; y < 64; y++) {
            row = (unsigned char *)source_lock.lpSurface + y * source_lock.lPitch;
            for (x = 0; x < 64; x++) {
                for (component = 0; component < bytes; component++) {
                    row[x * bytes + component] = (unsigned char)((17 * x + 31 * y + 53 * component + pass) & (pixel_mask >> (8 * component)));
                }
            }
            row = (unsigned char *)target_lock.lpSurface + y * target_lock.lPitch;
            for (x = 0; x < 64 * bytes; x++) {
                row[x] = 0xaa;
                expected[(y * 64 * bytes) + x] = 0xaa;
            }
        }
        for (y = 0; y < 4; y++) {
            for (x = 0; x < 6; x++) {
                for (component = 0; component < bytes; component++) {
                    expected[((y + 7) * 64 + x + 13) * bytes + component] =
                        (unsigned char)((17 * (x + 3) + 31 * (y + 5) + 53 * component + pass) & (pixel_mask >> (8 * component)));
                }
            }
        }
        if (source->lpVtbl->Unlock(source, source_lock.lpSurface) != DD_OK ||
                target->lpVtbl->Unlock(target, target_lock.lpSurface) != DD_OK) {
            fail("FAIL paired blit seed unlocks\n", 104);
        }
        if ((pass % 2 ? entry : direct)(&source_record, 3, 5, 6, 4, &target_record, 13, 7) != 1) {
            fail("FAIL paired actual SDK BltFast\n", 105);
        }
        zero_blit_buffer(&target_lock, sizeof(target_lock));
        target_lock.dwSize = sizeof(target_lock);
        if (target->lpVtbl->Lock(target, 0, &target_lock, DDLOCK_WAIT | DDLOCK_READONLY, 0) != DD_OK) {
            fail("FAIL paired blit readback lock\n", 106);
        }
        for (y = 0; y < 64; y++) {
            row = (unsigned char *)target_lock.lpSurface + y * target_lock.lPitch;
            for (x = 0; x < 64 * bytes; x++) {
                if (row[x] != expected[y * 64 * bytes + x]) {
                    blit_probe_number("bytes=",bytes);blit_probe_number("y=",y);blit_probe_number("byte_x=",x);blit_probe_number("actual=",row[x]);blit_probe_number("expected=",expected[y * 64 * bytes + x]);
                    fail("FAIL paired asymmetric blit pixels/outside preservation\n", 107);
                }
            }
        }
        if (target->lpVtbl->Unlock(target, target_lock.lpSurface) != DD_OK) {
            fail("FAIL paired blit readback unlock\n", 108);
        }
        source_record.active=0;
        if (direct(&source_record,3,5,6,4,0,13,7)!=0 ||
                entry(&source_record,3,5,6,4,0,13,7)!=0) {
            fail("FAIL paired inactive source before null destination\n",109);
        }
        source_record.active=1;target_record.active=0;
        if (direct(&source_record,3,5,6,4,&target_record,13,7)!=0 ||
                entry(&source_record,3,5,6,4,&target_record,13,7)!=0) {
            fail("FAIL paired inactive destination\n",110);
        }
        target_record.active=1;
    }
    target->lpVtbl->Release(target);
    source->lpVtbl->Release(source);
    output(side==0 ? "PASS original SDK BltFast direct/installed entry, destination x13/y7, exact rectangle and outside pixels\n" : "PASS rebuilt SDK BltFast direct/installed entry, destination x13/y7, exact rectangle and outside pixels\n");
}
