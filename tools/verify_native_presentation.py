"""Paired native fullscreen presentation using shared authentic loader host."""
import hashlib,shutil,subprocess
from windows_target import ROOT,TARGET,verify_target

def native_surface_host(dll, helper_path, runtime_name, call_site, calls, expected):
    if not __debug__:raise RuntimeError("Verification requires assertions")
    runtime=ROOT/'build/runtime'/runtime_name;runtime.mkdir(parents=True,exist_ok=True)
    shutil.copyfile(TARGET,runtime/'IGN_WIN.EXE');verify_target(runtime/'IGN_WIN.EXE')
    shutil.copyfile(dll,runtime/'surface_open_validation.dll')
    source=(ROOT/'tools/native_surface_probe.c').read_text()
    marker='static void check_records(NativeRecord *primary, NativeRecord *bank1,'
    helper=(ROOT/helper_path).read_text()
    assert marker in source;source=source.replace(marker,helper+marker,1)
    assert call_site in source
    source=source.replace(call_site,calls+call_site,1)
    host=runtime/'native_surface_host.c';host.write_text(source,encoding='utf-8',newline='\n')
    groups={'KERNEL32':['CreateFileA@28','ReadFile@20','GetFileSize@8','CloseHandle@4','VirtualAlloc@16',
     'LoadLibraryA@4','GetProcAddress@8','GetStdHandle@4','WriteFile@20','ExitProcess@4','GetModuleHandleA@4'],
     'USER32':['LoadCursorA@8','LoadIconA@8','RegisterClassA@4','IsWindow@4','IsWindowVisible@4','GetClientRect@8',
     'GetWindowLongA@8','DestroyWindow@4','UnregisterClassA@8'],'GDI32':['GetStockObject@4']}
    libs=[]
    for module,exports in groups.items():
     definition=runtime/(module+'.def');lib=runtime/(module+'.lib')
     definition.write_text('LIBRARY '+module+'.dll\nEXPORTS\n'+'\n'.join(exports)+'\n',encoding='utf-8',newline='\n')
     subprocess.run([shutil.which('llvm-dlltool'),'-m','i386','-d',str(definition),'-l',str(lib),'--kill-at'],check=True)
     libs.append(str(lib))
    obj=runtime/'probe.obj';exe=runtime/'probe.exe'
    subprocess.run([shutil.which('gcc'),'-DSURFACE_TEST_SHUTDOWN','-m32','-std=c89','-pedantic-errors','-Wall','-Wextra',
     '-Werror','-Wno-cast-function-type','-O2','-ffreestanding','-fno-builtin','-mno-sse','-c',str(host),'-o',str(obj)],check=True)
    subprocess.run([shutil.which('lld-link'),'/entry:NativeEntry@0','/subsystem:console','/nodefaultlib','/machine:x86',
     '/safeseh:no','/base:0x20000000','/dynamicbase:no','/out:'+str(exe),str(obj)]+libs,check=True)
    result=subprocess.run([str(exe)],cwd=runtime,capture_output=True,text=True,timeout=45)
    (runtime/'result.log').write_text(result.stdout+result.stderr,encoding='utf-8',newline='\n')
    verify_target(runtime/'IGN_WIN.EXE')
    assert result.returncode==0 and all(marker in result.stdout for marker in expected),(result.returncode,result.stdout,result.stderr)
    assert hashlib.sha256(dll.read_bytes()).digest()==hashlib.sha256((runtime/'surface_open_validation.dll').read_bytes()).digest()
    return {'stdout':result.stdout,'exit_code':result.returncode,
        'host_sha256':hashlib.sha256(exe.read_bytes()).hexdigest()}


def native_presentation(dll):
    result=native_surface_host(dll,'tools/native_present_frames.c','present_frames_native',
        '    if (!UnregisterClassA("Ignition",instance))',
        '    probe_native_fullscreen(image,compiled,0);\n    probe_native_fullscreen(image,compiled,1);\n',
        ('PASS original fullscreen','PASS rebuilt fullscreen'))
    return result | {'variant':'fullscreen_frames','cases':1,'paired_executions':2,'flips':8,'primary_pixels_checked':2457600,
     'scope':'Actual paired fullscreen640x480x8 constructor/installer/Present body and entry. Four frames per side checked across all307200 primary pixels, real GetCaps/Flip and SDK test-owned fills. Test-owned display restoration and interface disposal.',
     'limitations':'One mode/one backbuffer/eight flips; original WndProc reference; SDK test-owned fills are not production renderer; no complete native startup/menu/input/audio/races or original toolchain proof.'}
