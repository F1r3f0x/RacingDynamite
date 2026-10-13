static void probe_native_surface_constants(unsigned char *image, HMODULE compiled, int side) {
    typedef int (__cdecl *ConfigureFn)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int);
    union { unsigned char *bytes; FARPROC generic; ConfigureFn configure; OpenFn simple; } binding;
    ConfigureFn direct,entry;OpenFn reserved,install;
    unsigned int values[4],index;
    if(side==0) {
        binding.bytes=image+0x5c150;direct=binding.configure;
        binding.bytes=image+0x56c60;entry=binding.configure;
        binding.bytes=image+0x5c520;reserved=binding.simple;
        binding.bytes=image+0x5b690;install=binding.simple;
    } else {
        binding.generic=symbol(compiled,"Gfx_SurfaceConfigureSurfaceNative");direct=binding.configure;
        binding.generic=symbol(compiled,"Gfx_SurfaceConfigureSurfaceDispatch");entry=binding.configure;
        binding.generic=symbol(compiled,"Gfx_SurfaceReservedNative");reserved=binding.simple;
        binding.generic=symbol(compiled,"Gfx_InstallSurfaceDispatch");install=binding.simple;
    }
    if(install()!=1) { fail("FAIL paired constant-slot installer\n",120); }
    values[0]=0;values[1]=1;values[2]=0x80000000u;values[3]=0xffffffffu;
    for(index=0;index<4;index++) {
        if(direct(values[index],0x12345678,0xffffffffu,0,17)!=2 ||
                entry(values[index],0x12345678,0xffffffffu,0,17)!=2 || reserved()!=2) {
            fail("FAIL paired surface constant returns\n",121);
        }
    }
    output(side==0 ? "PASS original constant2 surface bodies and real installed configuration entry\n" : "PASS rebuilt constant2 surface bodies and real installed configuration entry\n");
}
