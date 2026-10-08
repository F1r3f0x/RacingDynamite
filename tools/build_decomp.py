"""Compile only the verified Windows mem routines; link a validation DLL.

This replaces the DOS build entry point. Other decomp modules are unmigrated.
"""
import argparse
import shutil
import subprocess
from windows_target import ROOT, BUILD, DLL, verify_target

# Nonreturning validation-only boundaries: never fake production success.
STARTUP_BOUNDARY_SOURCE = (
    'int Lisa_PrintVersion(void);\n'
    'int printf(const char *format, ...) { (void)format; for (;;) {} }\n'
    'void Gfx_InitPrimitiveState(void) { for (;;) {} }\n'
    'int Gfx_SelectBackend(int backend) { (void)backend; for (;;) {} }\n')
STARTUP_EXPORTS = ['Gfx_InitPrimitiveState', 'Gfx_SelectBackend']

# Extract these verified production declarations/body, rather than duplicating C.
_resource_header = (ROOT / 'decomp/include/geputget.h').read_text(encoding='utf-8-sig')
_resource_source = (ROOT / 'decomp/src/geputget.c').read_text(encoding='utf-8-sig')
STARTUP_BOUNDARY_SOURCE += (
    _resource_header[_resource_header.index('typedef void (*InputKeyEventCallback)'):
        _resource_header.index('#define MAX_FONTS')]
    + _resource_source[_resource_source.index('InputKeyEventCallback volatile g_inputKeyEventCallback ='):
        _resource_source.index('/* Global font table matching')])

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

EXPORTS = ['Lisa_PrintVersion', 'printf', 'Input_ResetCallbacks', 'g_inputKeyEventCallback', 'g_inputPollCallback', 'Mem_InitSystem', 'Mem_ShutdownSystem', 'Mem_DestroyPool', 'Mem_ShutdownPools', 'Mem_InitPools', 'Mem_CreatePool', 'Mem_InitHandles', 'Mem_NextHandleId', 'g_memHandlesInitialized', 'g_memHandleStatus',
           'g_memHandleIds', 'g_memHandleCursor', 'Mem_RegisterHandle', 'Mem_ShutdownHandles',
           'Mem_ReleaseHandleId', 'Mem_Free', 'Mem_Alloc', 'g_memPools', 'free', 'malloc',
           'g_memHandleContexts', 'g_memHandleParameters', 'g_memRegisteredHandleIds',
           'g_memHandleCallbacks', 'g_memHandleFlags', 'g_memPendingContext',
           'g_memPendingCallback', 'g_memPendingParameter'] + STARTUP_EXPORTS

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
