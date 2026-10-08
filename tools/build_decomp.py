"""Compile only the verified Windows mem routines; link a validation DLL.

This replaces the DOS build entry point. Other decomp modules are unmigrated.
"""
import argparse
import shutil
import subprocess
from windows_target import ROOT, BUILD, DLL, verify_target

# Nonreturning validation-only boundaries: never fake production success.
STARTUP_BOUNDARY_SOURCE = (
    'unsigned int validation_gfxDispatchWords[20];\n'
    'int Lisa_PrintVersion(void);\n'
    'int printf(const char *format, ...) { (void)format; for (;;) {} }\n'
    'void Gfx_InitPrimitiveState(void) { for (;;) {} }\n'
    'int Gfx_InstallSpriteDispatch(void) { for (;;) {} }\n')
STARTUP_EXPORTS = ['Gfx_InitPrimitiveState', 'Gfx_SelectBackend',
    'Gfx_InstallSurfaceDispatch', 'Gfx_InstallSpriteDispatch']

# Extract these verified production declarations/body, rather than duplicating C.
_resource_header = (ROOT / 'decomp/include/geputget.h').read_text(encoding='utf-8-sig')
_resource_source = (ROOT / 'decomp/src/geputget.c').read_text(encoding='utf-8-sig')
STARTUP_BOUNDARY_SOURCE += (
    _resource_header[_resource_header.index('typedef void (*InputKeyEventCallback)'):
        _resource_header.index('#define MAX_FONTS')]
    + _resource_source[_resource_source.index('InputKeyEventCallback volatile g_inputKeyEventCallback ='):
        _resource_source.index('/*\n * @original Gfx_SelectBackend')])

SURFACE_HEADER = _resource_header[_resource_header.index('/* Windows surface dispatch.'):_resource_header.index('#define MAX_FONTS')]
STARTUP_BOUNDARY_SOURCE += (
    'int Gfx_SurfaceConfigureNative(unsigned int option0, unsigned int option1, unsigned int option2, unsigned int option3) { (void)option0; (void)option1; (void)option2; (void)option3; for (;;) {} }\n'
    'int Gfx_SurfaceOpenNative(void) { for (;;) {} }\n'
    'int Gfx_SurfaceCloseNative(void) { for (;;) {} }\n'
    'int Gfx_SurfaceResetNative(void) { for (;;) {} }\n'
    'int Gfx_SurfaceConfigureSurfaceNative(unsigned int option0, unsigned int option1, unsigned int option2, unsigned int option3, unsigned int option4) { (void)option0; (void)option1; (void)option2; (void)option3; (void)option4; for (;;) {} }\n'
    'int Gfx_SurfaceBlitNative(GfxSurfaceRecord *source, int x, int y, int width, int height, GfxSurfaceRecord *destination, int destination_y, int destination_x) { (void)source; (void)x; (void)y; (void)width; (void)height; (void)destination; (void)destination_y; (void)destination_x; for (;;) {} }\n'
    'int Gfx_SurfaceCopyPixelsNative(const unsigned char *pixels, int stride, int source_x, int source_y, int width, int height, GfxSurfaceRecord *destination, int destination_x, int destination_y) { (void)pixels; (void)stride; (void)source_x; (void)source_y; (void)width; (void)height; (void)destination; (void)destination_x; (void)destination_y; for (;;) {} }\n'
    'int Gfx_SurfaceClearNative(GfxSurfaceRecord *surface) { (void)surface; for (;;) {} }\n'
    'int Gfx_SurfacePresentNative(GfxSurfaceRecord *surface) { (void)surface; for (;;) {} }\n'
    'int Gfx_SurfaceReservedNative(void) { for (;;) {} }\n'
    'int Gfx_SurfaceLockNative(GfxSurfaceRecord *surface, int mode) { (void)surface; (void)mode; for (;;) {} }\n'
    'int Gfx_SurfaceUnlockNative(GfxSurfaceRecord *surface) { (void)surface; for (;;) {} }\n'
    'int Gfx_SurfaceSetPaletteNative(const unsigned char *rgb) { (void)rgb; for (;;) {} }\n'
    'int Gfx_SurfaceRestoreNative(void) { for (;;) {} }\n'
)

# Extract the native banner body and its existing public prototype verbatim.
_lisa_header = (ROOT / 'decomp/include/lisa3d.h').read_text(encoding='utf-8-sig')
_lisa_source = (ROOT / 'decomp/src/lisa3d.c').read_text(encoding='utf-8-sig')
_lisa_start = _lisa_source.index('int Lisa_PrintVersion(void)')
_lisa_end = _lisa_source.index('\n}', _lisa_start) + 2
LISA_VERSION_SOURCE = (
    'extern int printf(const char *format, ...);\n'
    + _lisa_header[_lisa_header.index('int Lisa_PrintVersion(void);'):
        _lisa_header.index('int Lisa_PrintVersion(void);') + len('int Lisa_PrintVersion(void);')]
    + '\n' + _lisa_source[_lisa_start:_lisa_end] + '\n')

# Like printf, the selector must not see its nonreturning fixtures in this TU.
_selector_start = _resource_source.index('int Gfx_SelectBackend(int backend)')
_selector_end = _resource_source.index('\n}', _selector_start) + 2
LISA_VERSION_SOURCE += (
    _resource_header[_resource_header.index('int Gfx_SelectBackend(int backend);'):
        _resource_header.index('#define MAX_FONTS')]
    + _resource_source[_selector_start:_selector_end] + '\n'
    + _resource_source[_resource_source.index('/* Independently recovered Windows dispatch slots;'):
        _resource_source.index('/* Global font table matching')])

SURFACE_EXPORTS = ['g_surfaceConfigure', 'g_surfaceOpen', 'g_surfaceClose', 'g_surfaceReset', 'g_surfaceConfigureSurface', 'g_surfaceBlit', 'g_surfaceCopyPixels', 'g_surfaceClear', 'g_surfacePresent', 'g_surfaceReserved', 'g_surfaceLock', 'g_surfaceUnlock', 'g_surfaceSetPalette', 'g_surfaceRestore', 'Gfx_SurfaceConfigureNative', 'Gfx_SurfaceOpenNative', 'Gfx_SurfaceCloseNative', 'Gfx_SurfaceResetNative', 'Gfx_SurfaceConfigureSurfaceNative', 'Gfx_SurfaceBlitNative', 'Gfx_SurfaceCopyPixelsNative', 'Gfx_SurfaceClearNative', 'Gfx_SurfacePresentNative', 'Gfx_SurfaceReservedNative', 'Gfx_SurfaceLockNative', 'Gfx_SurfaceUnlockNative', 'Gfx_SurfaceSetPaletteNative', 'Gfx_SurfaceRestoreNative']

EXPORTS = ['validation_gfxDispatchWords', 'Lisa_PrintVersion', 'printf', 'Input_ResetCallbacks', 'g_inputKeyEventCallback', 'g_inputPollCallback', 'Mem_InitSystem', 'Mem_ShutdownSystem', 'Mem_DestroyPool', 'Mem_ShutdownPools', 'Mem_InitPools', 'Mem_CreatePool', 'Mem_InitHandles', 'Mem_NextHandleId', 'g_memHandlesInitialized', 'g_memHandleStatus',
           'g_memHandleIds', 'g_memHandleCursor', 'Mem_RegisterHandle', 'Mem_ShutdownHandles',
           'Mem_ReleaseHandleId', 'Mem_Free', 'Mem_Alloc', 'g_memPools', 'free', 'malloc',
           'g_memHandleContexts', 'g_memHandleParameters', 'g_memRegisteredHandleIds',
           'g_memHandleCallbacks', 'g_memHandleFlags', 'g_memPendingContext',
           'g_memPendingCallback', 'g_memPendingParameter'] + STARTUP_EXPORTS + SURFACE_EXPORTS

def compile_banner(flags, stem):
    # Separate TU: the production calls must not see the nonreturning printf
    # fixture, which otherwise permits argument removal and drops the second call.
    source, obj = BUILD / (stem + '_banner.c'), BUILD / (stem + '_banner.obj')
    source.write_text(LISA_VERSION_SOURCE, encoding='utf-8', newline='\n')
    obj.unlink(missing_ok=True)
    command = flags + ['-c', str(source), '-o', str(obj)]
    subprocess.run(command, cwd=ROOT, check=True)
    return obj, command

def build(compiler='clang', linker='lld-link', dll=DLL):
    verify_target()
    cc, ld = shutil.which(compiler), shutil.which(linker)
    if not cc or not ld:
        raise RuntimeError('clang and lld-link are required; no DOS fallback')
    BUILD.mkdir(parents=True, exist_ok=True)
    obj = BUILD / 'mem.obj'
    stub, stub_obj = BUILD / 'mem_crt_boundary.c', BUILD / 'mem_crt_boundary.obj'
    stub.write_text(STARTUP_BOUNDARY_SOURCE + 'void free(void *p) { (void)p; for (;;) {} }\nvoid *malloc(unsigned int n) { (void)n; for (;;) {} }\n', encoding='utf-8')
    # Only these explicit fresh products are inputs/outputs; never glob old objects.
    for path in [obj, stub_obj, dll, dll.with_suffix('.lib'), dll.with_suffix('.exp')]:
        path.unlink(missing_ok=True)
    commands = [
        [cc, '--target=i686-pc-windows-msvc', '-std=c89', '-pedantic-errors',
         '-Wall', '-Wextra', '-Werror', '-O2', '-ffreestanding', '-fno-builtin',
         '-fno-inline-functions', '-fno-unroll-loops', '-fno-vectorize', '-fno-slp-vectorize', '-mno-sse', '-mno-sse2',
         '-I', str(ROOT/'decomp/include'), '-c', str(ROOT/'decomp/src/mem.c'),
         '-o', str(obj)],
        [ld, '/dll', '/noentry', '/nodefaultlib', '/machine:x86',
         '/base:0x10000000', '/out:'+str(dll), str(obj), str(stub_obj)] +
        ['/export:'+s for s in EXPORTS],
    ]
    commands.insert(1, commands[0][:-4] + ['-c', str(stub), '-o', str(stub_obj)])
    banner_obj, _ = compile_banner(commands[0][:-4], dll.stem)
    commands[-1].append(str(banner_obj))
    for command in commands:
        print(subprocess.list2cmdline(command), flush=True)
        subprocess.run(command, cwd=ROOT, check=True)
    if any(not path.is_file() or path.stat().st_size == 0 for path in [obj, dll]):
        raise RuntimeError('Fresh Windows object and DLL required')
    print('Compiled/linked focused PE32 validation DLL; not a playable game.')
    return dll

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', default='clang')
    parser.add_argument('--linker', default='lld-link')
    args = parser.parse_args()
    build(args.compiler, args.linker)
