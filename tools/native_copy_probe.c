static void copy_probe_number(const char *label, unsigned int value) {
    char digits[12];unsigned int length,index;char temporary;
    output(label);length=0;
    do { digits[length++]=(char)('0'+value%10);value/=10; } while(value);
    for(index=0;index<length/2;index++) { temporary=digits[index];digits[index]=digits[length-1-index];digits[length-1-index]=temporary; }
    digits[length++]='\n';digits[length]=0;output(digits);
}
static void zero_copy_buffer(void *target, unsigned int count) {
    volatile unsigned char *bytes;
    bytes=(volatile unsigned char *)target;
    while (count) { *bytes++=0;count--; }
}
/* Paired original/C89 byte-copy observations; owned SDK surfaces and seeds. */
static void probe_native_copy(unsigned char *image, HMODULE compiled, int side, LPDIRECTDRAW draw) {
    typedef int (__cdecl *CopyFn)(const unsigned char *, int, int, int, int, int, NativeRecord *, int, int);
    typedef int (__cdecl *LockFn)(NativeRecord *, int);
    typedef int (__cdecl *UnlockFn)(NativeRecord *);
    union { unsigned char *bytes; FARPROC generic; CopyFn copy; OpenFn install; LockFn lock; UnlockFn unlock; } binding;
    LockFn lock;UnlockFn unlock;
    DDSURFACEDESC description, source_lock, target_lock;
    LPDIRECTDRAWSURFACE source, target;
    NativeRecord source_record, target_record;
    static unsigned char expected[64 * 64 * 4];
    unsigned char *row;
    unsigned int bytes, x, y, component, pass;
    DWORD pixel_mask;
    static unsigned char pixels[1024];
    unsigned int stride, source_x, source_y, width, height, destination_x, destination_y, index, mode;
    CopyFn direct, entry;
    OpenFn install;
    CopyFn *installed;

    zero_copy_buffer(&description, sizeof(description));
    description.dwSize = sizeof(description);
    description.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    description.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    description.dwWidth = 64;
    description.dwHeight = 64;
    source = 0;
    target = 0;
    if (draw->lpVtbl->CreateSurface(draw, &description, &source, 0) != DD_OK ||
            draw->lpVtbl->CreateSurface(draw, &description, &target, 0) != DD_OK) {
        fail("FAIL paired copy owned SDK surfaces\n", 100);
    }
    zero_copy_buffer(&source_record, sizeof(source_record));
    zero_copy_buffer(&target_record, sizeof(target_record));
    source_record.active = 1;
    source_record.width = source_record.height = 64;
    source_record.kind = 2;
    source_record.surface = source;
    target_record = source_record;
    target_record.surface = target;
    if (side==0) {
        binding.bytes=image+0x5c1d0;direct=binding.copy;
        binding.bytes=image+0x56b70;entry=binding.copy;
        binding.bytes=image+0x5b690;install=binding.install;
        installed=(CopyFn *)(image+0x10eb80);
        binding.bytes=image+0x5c530;lock=binding.lock;
        binding.bytes=image+0x5c680;unlock=binding.unlock;
    } else {
        binding.generic=symbol(compiled,"Gfx_SurfaceCopyPixelsNative");direct=binding.copy;
        binding.generic=symbol(compiled,"Gfx_SurfaceCopyPixelsDispatch");entry=binding.copy;
        binding.generic=symbol(compiled,"Gfx_InstallSurfaceDispatch");install=binding.install;
        installed=(CopyFn *)data_symbol(compiled,"g_surfaceCopyPixels");
        binding.generic=symbol(compiled,"Gfx_SurfaceLockNative");lock=binding.lock;
        binding.generic=symbol(compiled,"Gfx_SurfaceUnlockNative");unlock=binding.unlock;
    }
    if (install() != 1 || *installed != direct) {
        fail("FAIL paired copy installer\n", 101);
    }
    for (pass = 0; pass < 8; pass++) {
        zero_copy_buffer(&source_lock, sizeof(source_lock));
        source_lock.dwSize = sizeof(source_lock);
        zero_copy_buffer(&target_lock, sizeof(target_lock));
        target_lock.dwSize = sizeof(target_lock);
        if (source->lpVtbl->Lock(source, 0, &source_lock, DDLOCK_WAIT, 0) != DD_OK ||
                target->lpVtbl->Lock(target, 0, &target_lock, DDLOCK_WAIT, 0) != DD_OK) {
            fail("FAIL paired copy seed locks\n", 102);
        }
        bytes = source_lock.ddpfPixelFormat.dwRGBBitCount / 8;
        if (!bytes || bytes > 4 || source_lock.ddpfPixelFormat.dwRGBBitCount % 8 ||
                target_lock.ddpfPixelFormat.dwRGBBitCount != 8 * bytes ||
                source_lock.lPitch < (LONG)(64 * bytes) || target_lock.lPitch < (LONG)(64 * bytes)) {
            fail("FAIL paired copy actual SDK format/pitch\n", 103);
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
        mode=pass%4;
        stride=pass<4 ? 160 : 64;source_x=pass<4 ? 0 : 4;source_y=pass<4 ? 0 : 2;
        width=pass<4 ? 160 : 48;height=pass<4 ? 2 : 3;
        destination_x=pass<4 ? 16 : 8;destination_y=pass<4 ? 4 : 3;
        if (bytes!=4 || ((uintptr_t)pixels&3)) { fail("FAIL paired byte-copy fixture format/alignment\n",109); }
        for (index=0;index<sizeof(pixels);index++) {
            pixels[index]=(unsigned char)((17*index+31*pass)&(pixel_mask>>(8*(index%bytes))));
        }
        for (y=0;y<height;y++) {
            for(x=0;x<width;x++) {
                expected[(destination_y+y)*64*bytes+destination_x+x]=pixels[(source_y+y)*stride+source_x+x];
            }
        }
        if (source->lpVtbl->Unlock(source, source_lock.lpSurface) != DD_OK ||
                target->lpVtbl->Unlock(target, target_lock.lpSurface) != DD_OK) {
            fail("FAIL paired copy seed unlocks\n", 104);
        }
        if (mode && (lock(&target_record,(int)mode)!=1 || target_record.opaque[1]!=mode)) { fail("FAIL paired copy initial lock mode\n",110); }
        if ((pass % 2 ? entry : direct)(pixels,stride,source_x,source_y,width,height,&target_record,destination_x,destination_y) != 1) {
            fail("FAIL paired CopyPixels actual SDK Lock/Unlock\n", 105);
        }
        if (target_record.opaque[0]!=(mode ? 1u : 0u) || target_record.opaque[1]!=mode) { fail("FAIL paired copy restored lock mode\n",111); }
        if (mode && unlock(&target_record)!=1) { fail("FAIL paired copy owned readback unlock\n",112); }
        zero_copy_buffer(&target_lock, sizeof(target_lock));
        target_lock.dwSize = sizeof(target_lock);
        if (target->lpVtbl->Lock(target, 0, &target_lock, DDLOCK_WAIT | DDLOCK_READONLY, 0) != DD_OK) {
            fail("FAIL paired copy readback lock\n", 106);
        }
        for (y = 0; y < 64; y++) {
            row = (unsigned char *)target_lock.lpSurface + y * target_lock.lPitch;
            for (x = 0; x < 64 * bytes; x++) {
                if (row[x] != expected[y * 64 * bytes + x]) {
                    copy_probe_number("bytes=",bytes);copy_probe_number("y=",y);copy_probe_number("byte_x=",x);copy_probe_number("actual=",row[x]);copy_probe_number("expected=",expected[y * 64 * bytes + x]);
                    fail("FAIL paired byte copy/outside preservation\n", 107);
                }
            }
        }
        if (target->lpVtbl->Unlock(target, target_lock.lpSurface) != DD_OK) {
            fail("FAIL paired copy readback unlock\n", 108);
        }
    }
    /* Forward overlap is observable; replacing it with memmove would differ. */
    if (lock(&target_record,2)!=1 || !target_record.opaque[3]) {
        fail("FAIL paired copy alias lock\n",113);
    }
    row=(unsigned char *)(uintptr_t)target_record.opaque[3];
    for(index=0;index<64*bytes;index++) {
        row[index]=(unsigned char)((17*index+3)&(pixel_mask>>(8*(index%bytes))));
        expected[index]=row[index];
    }
    for(index=0;index<8;index++) {
        for(component=0;component<4;component++) {
            expected[4+4*index+component]=expected[4*index+component];
        }
    }
    if (direct(row,(int)target_record.opaque[4],0,0,32,1,&target_record,4,0)!=1 ||
            target_record.opaque[0]!=1 || target_record.opaque[1]!=2 || unlock(&target_record)!=1) {
        fail("FAIL paired forward overlapping copy/mode\n",114);
    }
    target_record.active=0;
    if (direct(0,160,0,0,32,1,&target_record,0,0)!=0 ||
            entry(0,160,0,0,32,1,&target_record,0,0)!=0) {
        fail("FAIL paired inactive byte-copy guard\n",115);
    }
    target_record.active=1;
    if (direct(0,17,1,0,32,0,&target_record,0,0)!=1 ||
            entry(0,17,1,0,32,0,&target_record,0,0)!=1 ||
            target_record.opaque[0]!=0 || target_record.opaque[1]!=0) {
        fail("FAIL paired generic zero-height access/restoration\n",116);
    }
    zero_copy_buffer(&target_lock,sizeof(target_lock));target_lock.dwSize=sizeof(target_lock);
    if (target->lpVtbl->Lock(target,0,&target_lock,DDLOCK_WAIT|DDLOCK_READONLY,0)!=DD_OK) {
        fail("FAIL paired alias readback lock\n",117);
    }
    for(y=0;y<64;y++) {
        row=(unsigned char *)target_lock.lpSurface+y*target_lock.lPitch;
        for(x=0;x<64*bytes;x++) {
            if(row[x]!=expected[y*64*bytes+x]) { fail("FAIL paired alias/empty-height pixels\n",118); }
        }
    }
    if(target->lpVtbl->Unlock(target,target_lock.lpSurface)!=DD_OK) { fail("FAIL paired alias readback unlock\n",119); }
    target->lpVtbl->Release(target);
    source->lpVtbl->Release(source);
    output(side==0 ? "PASS original CopyPixels direct/installed entry, byte units, fast/generic branches, four restored modes and exact outside bytes\n" : "PASS rebuilt CopyPixels direct/installed entry, byte units, fast/generic branches, four restored modes and exact outside bytes\n");
}
