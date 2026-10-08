"""Compile only the verified Windows mem routines; link a validation DLL.

This replaces the DOS build entry point. Other decomp modules are unmigrated.
"""
import argparse
import shutil
import subprocess
from windows_target import ROOT, BUILD, DLL, verify_target

# Nonreturning validation-only boundaries: never fake production success.
STARTUP_BOUNDARY_SOURCE = (
    'int Lisa_PrintVersion(void) { for (;;) {} }\n'
    'void Gfx_InitPrimitiveState(void) { for (;;) {} }\n'
    'int Gfx_SelectBackend(int backend) { (void)backend; for (;;) {} }\n')
STARTUP_EXPORTS = ['Lisa_PrintVersion',
                   'Gfx_InitPrimitiveState', 'Gfx_SelectBackend']

# Extract these verified production declarations/body, rather than duplicating C.
_resource_header = (ROOT / 'decomp/include/geputget.h').read_text(encoding='utf-8-sig')
_resource_source = (ROOT / 'decomp/src/geputget.c').read_text(encoding='utf-8-sig')
STARTUP_BOUNDARY_SOURCE += (
    _resource_header[_resource_header.index('typedef void (*InputKeyEventCallback)'):
        _resource_header.index('#define MAX_FONTS')]
    + _resource_source[_resource_source.index('InputKeyEventCallback volatile g_inputKeyEventCallback ='):
        _resource_source.index('/* Global font table matching')])

EXPORTS = ['Input_ResetCallbacks', 'g_inputKeyEventCallback', 'g_inputPollCallback', 'Mem_InitSystem', 'Mem_ShutdownSystem', 'Mem_DestroyPool', 'Mem_ShutdownPools', 'Mem_InitPools', 'Mem_CreatePool', 'Mem_InitHandles', 'Mem_NextHandleId', 'g_memHandlesInitialized', 'g_memHandleStatus',
           'g_memHandleIds', 'g_memHandleCursor', 'Mem_RegisterHandle', 'Mem_ShutdownHandles',
           'Mem_ReleaseHandleId', 'Mem_Free', 'Mem_Alloc', 'g_memPools', 'free', 'malloc',
           'g_memHandleContexts', 'g_memHandleParameters', 'g_memRegisteredHandleIds',
           'g_memHandleCallbacks', 'g_memHandleFlags', 'g_memPendingContext',
           'g_memPendingCallback', 'g_memPendingParameter'] + STARTUP_EXPORTS

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
