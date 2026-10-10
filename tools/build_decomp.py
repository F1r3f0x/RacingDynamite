"""Compile only the verified Windows mem routines; link a validation DLL.

This replaces the DOS build entry point. Other decomp modules are unmigrated.
"""
import argparse
import shutil
import subprocess
from windows_target import ROOT, BUILD, DLL, verify_target

# Nonreturning validation-only boundaries: never fake production success.
STARTUP_BOUNDARY_SOURCE = (
    '#include <stdint.h>\n'
    'unsigned int validation_gfxDispatchWords[4];\n'
    'int Lisa_PrintVersion(void);\n'
    'int printf(const char *format, ...) { (void)format; for (;;) {} }\n'
    '')
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
    'int Gfx_SurfaceConfigureSurfaceNative(unsigned int option0, unsigned int option1, unsigned int option2, unsigned int option3, unsigned int option4) { (void)option0; (void)option1; (void)option2; (void)option3; (void)option4; for (;;) {} }\n'
    'int Gfx_SurfaceBlitNative(GfxSurfaceRecord *source, int x, int y, int width, int height, GfxSurfaceRecord *destination, int destination_y, int destination_x) { (void)source; (void)x; (void)y; (void)width; (void)height; (void)destination; (void)destination_y; (void)destination_x; for (;;) {} }\n'
    'int Gfx_SurfaceCopyPixelsNative(const unsigned char *pixels, int stride, int source_x, int source_y, int width, int height, GfxSurfaceRecord *destination, int destination_x, int destination_y) { (void)pixels; (void)stride; (void)source_x; (void)source_y; (void)width; (void)height; (void)destination; (void)destination_x; (void)destination_y; for (;;) {} }\n'
    'int Gfx_SurfaceClearNative(GfxSurfaceRecord *surface) { (void)surface; for (;;) {} }\n'
    'int Gfx_SurfacePresentNative(GfxSurfaceRecord *surface) { (void)surface; for (;;) {} }\n'
    'int Gfx_SurfaceReservedNative(void) { for (;;) {} }\n'
    'int Gfx_SurfaceLockNative(GfxSurfaceRecord *surface, int mode) { (void)surface; (void)mode; for (;;) {} }\n'
    'int Gfx_SurfaceUnlockNative(GfxSurfaceRecord *surface) { (void)surface; for (;;) {} }\n'
    'int Gfx_SurfaceSetPaletteNative(const unsigned char *rgb) { (void)rgb; for (;;) {} }\n'
)

# Extract the native banner body and its existing public prototype verbatim.
_lisa_header = (ROOT / 'decomp/include/lisa3d.h').read_text(encoding='utf-8-sig')
_lisa_source = (ROOT / 'decomp/src/lisa3d.c').read_text(encoding='utf-8-sig')
_lisa_start = _lisa_source.index('int Lisa_PrintVersion(void)')
_lisa_end = _lisa_source.index('\n}', _lisa_start) + 2
LISA_VERSION_SOURCE = (
    '#include <stdint.h>\n'
    'extern int printf(const char *format, ...);\n'
    + _lisa_header[_lisa_header.index('int Lisa_PrintVersion(void);'):
        _lisa_header.index('int Lisa_PrintVersion(void);') + len('int Lisa_PrintVersion(void);')]
    + '\n' + _lisa_source[_lisa_start:_lisa_end] + '\n')

# Like printf, the selector must not see its nonreturning fixtures in this TU.
_selector_start = _resource_source.index('int Gfx_SelectBackend(int backend)')
_selector_end = _resource_source.index('\n}', _selector_start) + 2
LISA_VERSION_SOURCE += (
    'extern void *malloc(unsigned int size);\n'
    'extern void free(void *pointer);\n'
    + _resource_header[_resource_header.index('int Gfx_SelectBackend(int backend);'):
        _resource_header.index('#define MAX_FONTS')]
    + _resource_source[_selector_start:_selector_end] + '\n'
    + _resource_source[_resource_source.index('/* Independently recovered Windows dispatch slots;'):
        _resource_source.index('/* Global font table matching')])

_surface_records_start = _resource_source.index('volatile GfxSurfaceRecord g_nativePrimarySurface;')
_surface_records_end = _resource_source.index('/* Independently recovered Windows dispatch slots;')
LISA_VERSION_SOURCE += _resource_source[_surface_records_start:_surface_records_end]

# Compile the real COM method calls using the local Win32 SDK declarations.
_restore_start = _resource_source.index('int Gfx_SurfaceRestoreNative(void)')
_restore_end = _resource_source.index('\n}', _restore_start) + 2
RESTORE_BODY = _resource_source[_restore_start:_restore_end]
LISA_VERSION_SOURCE = LISA_VERSION_SOURCE.replace(RESTORE_BODY, 'int Gfx_SurfaceRestoreNative(void);')
SURFACE_SOURCE = ('#include <ddraw.h>\n#include <stddef.h>\n#include "geputget.h"\n'
    'typedef char surface_record_size[(sizeof(GfxSurfaceRecord)==48)?1:-1];\n'
    'typedef char surface_width_offset[(offsetof(GfxSurfaceRecord,width)==28)?1:-1];\n'
    'typedef char surface_height_offset[(offsetof(GfxSurfaceRecord,height)==32)?1:-1];\n'
    'typedef char surface_depth_offset[(offsetof(GfxSurfaceRecord,bit_depth)==36)?1:-1];\n'
    'typedef char surface_descriptor_size[(sizeof(DDSURFACEDESC)==108)?1:-1];\n'
    'typedef char surface_pointer_offset[(offsetof(GfxSurfaceRecord,surface)==44)?1:-1];\n'
    'typedef char surface_is_lost_slot[(offsetof(IDirectDrawSurfaceVtbl,IsLost)==0x60)?1:-1];\n'
    'typedef char surface_restore_slot[(offsetof(IDirectDrawSurfaceVtbl,Restore)==0x6c)?1:-1];\n'
    + RESTORE_BODY + '\n')

# Standard C89 sin/cast body compiled separately with independently probed x87
# flags. This provisional GCC/Clang mix does not identify the original compiler.
_sine_start = _resource_source.index('int32_t Gfx_InitSineTable(void)')
_sine_end = _resource_source.index('\n}', _sine_start) + 2
SINE_BODY = _resource_source[_sine_start:_sine_end]
LISA_VERSION_SOURCE = LISA_VERSION_SOURCE.replace(SINE_BODY, 'int32_t Gfx_InitSineTable(void);')
SINE_SOURCE = '#include <math.h>\n#include "geputget.h"\n' + SINE_BODY + '\n'
_window_header = (ROOT/'decomp/include/main.h').read_text(encoding='utf-8-sig')
_window_source = (ROOT/'decomp/src/main.c').read_text(encoding='utf-8-sig')
_window_marker = '/* Native Windows platform backing, independently verified from WinMain/Open. */'
_window_end = '/* End native Windows platform backing. */'
WINDOW_HEADER = _window_header[_window_header.index(_window_marker):_window_header.index(_window_end)]
WINDOW_SOURCE = _window_source[_window_source.index(_window_marker):_window_source.index(_window_end)]
_open_marker = '/* Native surface constructor state; independently recovered Windows initializers. */'
_open_end = '/* End native surface constructor. */'
OPEN_SOURCE = _resource_source[_resource_source.index(_open_marker):_resource_source.index(_open_end)]
SURFACE_SOURCE += '#include <string.h>\n' + WINDOW_HEADER + WINDOW_SOURCE + OPEN_SOURCE.replace('#include "main.h"', '')
_shutdown_marker = '/* Native surface resource shutdown. */'
_shutdown_end = '/* End native surface resource shutdown. */'
SURFACE_SOURCE += _resource_source[_resource_source.index(_shutdown_marker):_resource_source.index(_shutdown_end)]
_rebuild_marker = '/* Native surface rebuild and its independently recovered palette backing. */'
_rebuild_end = '/* End native surface rebuild. */'
SURFACE_SOURCE += _resource_source[_resource_source.index(_rebuild_marker):_resource_source.index(_rebuild_end)]
SURFACE_EXPORTS_EXTRA = ['g_nativeRebuildPaletteEntries', 'g_nativeDirectDraw', 'g_nativeClipper', 'g_nativePalette',
    'g_nativePaletteEntries', 'g_nativeSurfaceWindow', 'g_nativeFullscreen',
    'g_nativeSurfaceWidth', 'g_nativeSurfaceHeight', 'g_nativeSurfaceBitDepth',
    'g_nativeBackbufferCount', 'g_nativeDesktopBitDepth', 'g_nativeWindowInstance',
    'g_nativeWindow', 'g_nativeWindowClassName', 'g_nativeWindowTitle',
    'g_nativeBackbufferMessage', 'g_nativeBackbufferLimitMessage']

PRIMITIVE_EXPORTS = ['Gfx_InitPairStorageControl', 'Gfx_InitGraphicsPairStorage',
    'Gfx_EnablePrimitiveControl', 'Gfx_InitSecondaryDefaultWords',
    'Gfx_InitPrimitiveLineControl', 'Gfx_InitNamedPrimitiveDefault',
    'g_nativeEmptyControl', 'g_nativePrimitiveControl', 'g_nativeNamedControl',
    'g_nativeSecondaryControl', 'g_nativeLineControl', 'g_nativePrimitiveRecordControl',
    'g_nativeAuxiliaryControl', 'g_nativeGraphicsPairStorage', 'g_nativeNamedPairStorage',
    'g_nativeNamedPrimitiveDefault', 'g_nativeSecondaryDefaultWords']
SINE_EXPORTS = ['Gfx_InitSineTable', 'g_nativeSineTable', 'g_nativeSineStep',
    'g_nativeSineFullCircle', 'g_nativeSineAmplitude']

SURFACE_EXPORTS = ['Gfx_InitSurfaceRecords', 'g_nativePrimarySurface',
    'g_nativeType1Surfaces', 'g_nativeType2Surfaces', 'g_surfaceConfigure', 'g_surfaceOpen', 'g_surfaceRebuild', 'g_surfaceShutdown', 'g_surfaceConfigureSurface', 'g_surfaceBlit', 'g_surfaceCopyPixels', 'g_surfaceClear', 'g_surfacePresent', 'g_surfaceReserved', 'g_surfaceLock', 'g_surfaceUnlock', 'g_surfaceSetPalette', 'g_surfaceRestore', 'Gfx_SurfaceConfigureNative', 'Gfx_SurfaceOpenNative', 'Gfx_SurfaceRebuildNative', 'Gfx_SurfaceShutdownNative', 'Gfx_SurfaceConfigureSurfaceNative', 'Gfx_SurfaceBlitNative', 'Gfx_SurfaceCopyPixelsNative', 'Gfx_SurfaceClearNative', 'Gfx_SurfacePresentNative', 'Gfx_SurfaceReservedNative', 'Gfx_SurfaceLockNative', 'Gfx_SurfaceUnlockNative', 'Gfx_SurfaceSetPaletteNative', 'Gfx_SurfaceRestoreNative'] + SURFACE_EXPORTS_EXTRA

# Validation fixtures live separately from the extracted production installer.
STARTUP_BOUNDARY_SOURCE += (
    'int Gfx_SpriteOpenNative(void) { for (;;) {} }\n'
    'int Gfx_SpriteResetNative(void) { for (;;) {} }\n'
    'int Gfx_SpriteCloseNative(void) { for (;;) {} }\n'
    'int Gfx_SpriteOptionNative(unsigned int option) { (void)option; for (;;) {} }\n'
    'int Gfx_SpriteConfigureNative(unsigned char *pixels, int stride, int width, int height, unsigned int option) { (void)pixels; (void)stride; (void)width; (void)height; (void)option; for (;;) {} }\n'
    'int Gfx_SpriteSetClipNative(int left, int top, int right, int bottom) { (void)left; (void)top; (void)right; (void)bottom; for (;;) {} }\n'
    'int Gfx_SpriteDrawListNative(const unsigned int *list) { (void)list; for (;;) {} }\n'
    'int Gfx_DrawSpriteNative(GfxSpriteHandle *handle, Point2D *position, const GfxSpriteTransform *transform) { (void)handle; (void)position; (void)transform; for (;;) {} }\n'
    'int Gfx_SpriteCreateDescriptorNative(GfxSpriteDescriptor *descriptor, int image_id) { (void)descriptor; (void)image_id; for (;;) {} }\n'
    'int Gfx_SpriteFreeDescriptorNative(GfxSpriteDescriptor *descriptor) { (void)descriptor; for (;;) {} }\n'
    'int Gfx_SpriteCopyDescriptorNative(GfxSpriteDescriptor *descriptor, int image_id) { (void)descriptor; (void)image_id; for (;;) {} }\n'
    'int Gfx_SpriteReservedNative(unsigned int option) { (void)option; for (;;) {} }\n'
    'int Gfx_SpriteGetStateNative(GfxSpriteState *state) { (void)state; for (;;) {} }\n'
    'int Gfx_SpriteSetStateNative(const GfxSpriteState *state) { (void)state; for (;;) {} }\n'
)
SPRITE_EXPORTS = ['Gfx_GrowPointerTable', 'Gfx_CopyAllocatedString', 'Gfx_ReleaseSpritePackingStorage', 'Gfx_FreeBytes', 'Gfx_FreeAlignedBytes', 'Gfx_AssignSpritePackingStorage', 'Gfx_CopySpriteDescriptorPixels', 'Gfx_AddSpritePackingBucket', 'Gfx_FindSpritePackingGap', 'Gfx_AddSpritePackingPage', 'Gfx_AllocBytes', 'Gfx_AllocAlignedBytes', 'Gfx_InitSpritePackingState', 'Gfx_LinkSpritePackingNode', 'g_spritePackingTemplate', 'g_spritePackingBuckets', 'g_spritePackingPages', 'Gfx_InitDefaultSpriteDescriptor', 'g_nativeSpriteDefaultName', 'g_nativeImageControl', 'g_spriteOpen', 'Gfx_SpriteOpenNative', 'g_spriteReset', 'Gfx_SpriteResetNative', 'g_spriteClose', 'Gfx_SpriteCloseNative', 'g_spriteOption', 'Gfx_SpriteOptionNative', 'g_spriteConfigure', 'Gfx_SpriteConfigureNative', 'g_spriteSetClip', 'Gfx_SpriteSetClipNative', 'g_spriteDrawList', 'Gfx_SpriteDrawListNative', 'g_spriteDraw', 'Gfx_DrawSpriteNative', 'g_spriteHandleOp', 'Gfx_SpriteHandleOpNative', 'g_spriteImageOp', 'Gfx_SpriteImageOpNative', 'g_spriteCreateDescriptor', 'Gfx_SpriteCreateDescriptorNative', 'g_spriteFreeDescriptor', 'Gfx_SpriteFreeDescriptorNative', 'g_spriteCopyDescriptor', 'Gfx_SpriteCopyDescriptorNative', 'g_spriteReserved', 'Gfx_SpriteReservedNative', 'g_spriteGetState', 'Gfx_SpriteGetStateNative', 'g_spriteSetState', 'Gfx_SpriteSetStateNative', 'Gfx_InitSpriteWorkspaceA', 'Gfx_InitSpriteWorkspaceB', 'Gfx_InitSpriteHandles', 'Gfx_CopySpriteDescriptor', 'g_nativeSpriteFreeList', 'g_nativeSpriteHandles', 'g_nativeSpriteFreeCursor', 'g_nativeSpriteScratch', 'g_nativeSpriteDefault']

WORKSPACE_EXPORTS = ['g_spriteWorkspace' + side + field for side in ('A', 'B')
    for field in ('Allocated', 'Count', 'Buffer0', 'Buffer1', 'Buffer2')]

EXPORTS = WORKSPACE_EXPORTS + ['validation_gfxDispatchWords', 'Lisa_PrintVersion', 'printf', 'Input_ResetCallbacks', 'g_inputKeyEventCallback', 'g_inputPollCallback', 'Mem_InitSystem', 'Mem_ShutdownSystem', 'Mem_DestroyPool', 'Mem_ShutdownPools', 'Mem_InitPools', 'Mem_CreatePool', 'Mem_InitHandles', 'Mem_NextHandleId', 'g_memHandlesInitialized', 'g_memHandleStatus',
           'g_memHandleIds', 'g_memHandleCursor', 'Mem_RegisterHandle', 'Mem_ShutdownHandles',
           'Mem_ReleaseHandleId', 'Mem_Free', 'Mem_Alloc', 'g_memPools', 'free', 'malloc',
           'g_memHandleContexts', 'g_memHandleParameters', 'g_memRegisteredHandleIds',
           'g_memHandleCallbacks', 'g_memHandleFlags', 'g_memPendingContext',
           'g_memPendingCallback', 'g_memPendingParameter'] + STARTUP_EXPORTS + SURFACE_EXPORTS + SPRITE_EXPORTS + SINE_EXPORTS + PRIMITIVE_EXPORTS

def compile_banner(flags, stem):
    # Separate TU: the production calls must not see the nonreturning printf
    # fixture, which otherwise permits argument removal and drops the second call.
    source, obj = BUILD / (stem + '_banner.c'), BUILD / (stem + '_banner.obj')
    source.write_text(LISA_VERSION_SOURCE, encoding='utf-8', newline='\n')
    obj.unlink(missing_ok=True)
    command = flags + ['-c', str(source), '-o', str(obj)]
    subprocess.run(command, cwd=ROOT, check=True)
    return obj, command

def compile_sine(stem):
    cc = shutil.which('gcc')
    if not cc:
        raise RuntimeError('Probed GCC i386 x87 math compiler required; no sin fixture fallback')
    source, obj = BUILD/(stem+'_sine.c'), BUILD/(stem+'_sine.obj')
    source.write_text(SINE_SOURCE, encoding='utf-8', newline='\n')
    obj.unlink(missing_ok=True)
    command = [cc, '-m32', '-std=c89', '-pedantic-errors', '-Wall', '-Wextra',
        '-Werror', '-O2', '-ffast-math', '-fno-associative-math',
        '-fexcess-precision=fast', '-mno-sse', '-mfpmath=387',
        '-I', str(ROOT/'decomp/include'), '-c', str(source), '-o', str(obj)]
    print(subprocess.list2cmdline(command), flush=True)
    subprocess.run(command, cwd=ROOT, check=True)
    return obj, command

def compile_surface(stem):
    cc = shutil.which('gcc')
    if not cc:
        raise RuntimeError('GCC i386 Win32 SDK compiler required')
    source, obj = BUILD/(stem+'_surface.c'), BUILD/(stem+'_surface.obj')
    source.write_text(SURFACE_SOURCE, encoding='utf-8', newline='\n')
    obj.unlink(missing_ok=True)
    command = [cc, '-m32', '-std=c89', '-pedantic-errors', '-Wall', '-Wextra',
        '-Werror', '-O2', '-I', str(ROOT/'decomp/include'),
        '-c', str(source), '-o', str(obj)]
    print(subprocess.list2cmdline(command), flush=True)
    subprocess.run(command, cwd=ROOT, check=True)
    return obj, command


def surface_import_libraries(stem):
    dlltool = shutil.which('llvm-dlltool')
    if not dlltool:
        raise RuntimeError('LLVM dlltool required for native i386 Win32 imports')
    groups = {'DDRAW':['DirectDrawCreate@12'],
        'USER32':['GetSystemMetrics@4','CreateWindowExA@48','UpdateWindow@4',
            'SetFocus@4','GetDC@4','ReleaseDC@8','GetWindowLongA@8',
            'SetWindowLongA@12','SetRect@20','GetMenu@4','AdjustWindowRectEx@16',
            'SetWindowPos@28','SystemParametersInfoA@16','GetWindowRect@8',
            'MessageBoxA@16','ShowWindow@8'],
        'GDI32':['GetDeviceCaps@8']}
    libraries = []
    for dll,exports in groups.items():
        definition = BUILD/(stem+'_'+dll.lower()+'.def')
        lib = BUILD/(stem+'_'+dll.lower()+'.lib')
        definition.write_text('LIBRARY '+dll+'.dll\nEXPORTS\n'+
            '\n'.join(exports)+'\n', encoding='utf-8', newline='\n')
        lib.unlink(missing_ok=True)
        subprocess.run([dlltool,'-m','i386','-d',str(definition),'-l',str(lib),
            '--kill-at'],cwd=ROOT,check=True)
        libraries.append(str(lib))
    return libraries


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
        [ld, '/dll', '/noentry', '/nodefaultlib', '/machine:x86', '/safeseh:no',
         '/base:0x10000000', '/out:'+str(dll), str(obj), str(stub_obj)] +
        ['/export:'+s for s in EXPORTS],
    ]
    commands.insert(1, commands[0][:-4] + ['-c', str(stub), '-o', str(stub_obj)])
    banner_obj, _ = compile_banner(commands[0][:-4], dll.stem)
    sine_obj, _ = compile_sine(dll.stem)
    surface_obj, _ = compile_surface(dll.stem)
    commands[-1].extend([str(banner_obj), str(sine_obj), str(surface_obj)] + surface_import_libraries(dll.stem))
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
