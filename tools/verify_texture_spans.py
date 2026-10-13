# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Real original span instructions versus whole C89 renderer; no render stubs."""
import hashlib
import json
import random
import shutil
import struct
import subprocess
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE, UC_HOOK_MEM_INVALID
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EFLAGS
from windows_target import ROOT, BUILD, TARGET, verify_target, validation_symbols
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')
DLL = BUILD / 'texture_spans_validation.dll'
ARENA, STACK, STOP = 0x7200000, 0x7000000, 0x7100000
STATE = ['g_triangleSpanByteStepStorage', 'g_triangleSpanPackedStep',
    'g_triangleSpanTextureWord', 'g_triangleSpanLookupWord', 'g_triangleSpanLeftSampleStorage']
ROUTINES = {
    'Gfx_SetTriangleSpanState': (0x5c990,24,'bc5e0b937897780cefd8d87d3b95fff67e75ef98d1a91d60e0d2ceb368f0b9b8'),
    'Gfx_SetTriangleSpanCore': (0x67f1c,38,'d7b77c5b6446f23ae6be7ab2cb3e5034585cfe71362cf792222dd3003f3ed05c'),
    'Gfx_DrawTriangleSpan': (0x5c9b0,32,'e3dbc55544ab3db0f77135a83fac72ff4ba1293984655e31b72723eb1be5bf0d'),
    'Gfx_DrawTriangleSpanCore': (0x67f42,85,'2d40bc4ff2b77b85bdba551897c8fc3ad8cf7437670fa2bff0afaa59aa827309'),
    'Gfx_DrawClippedTriangleSpan': (0x5c9d0,28,'fa88078708ae1fc6bd627bf2fd66459e95f5595bcc828ec82e108d4d7b2bf31e'),
    'Gfx_DrawClippedTriangleSpanCore': (0x67f97,53,'b5ca9a8fa2762fafec0f0b81c28c95a7136d797e931800496e82aa6d7607fbab')}
INPUTS = ['decomp/src/renderer.c','decomp/include/renderer.h','decomp/modules.json',
    'decomp/target.json','tools/verify_texture_spans.py','tools/windows_target.py','tools/windows_tracking.py',
    'tools/native_texture_spans.c','tools/native_sine_probe.c']
PRESERVED = (UC_X86_REG_EBX,UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBP)


def build():
    BUILD.mkdir(parents=True,exist_ok=True)
    gcc,linker = (shutil.which(n) for n in ('gcc','lld-link'))
    if not gcc or not linker:
        raise RuntimeError('Installed provisional GCC and LLD required')
    obj = BUILD / 'texture_spans_renderer.obj'
    commands = [[gcc,'-m32','-std=c89','-pedantic-errors','-Wall','-Wextra','-Werror',
        '-O2','-fno-inline','-fno-optimize-sibling-calls','-I',str(ROOT/'decomp/include'),
        '-c',str(ROOT/'decomp/src/renderer.c'),'-o',str(obj)],
        [linker,'/dll','/noentry','/nodefaultlib','/machine:x86','/safeseh:no',
         '/base:0x10000000','/out:'+str(DLL),str(obj)] + ['/export:'+n for n in [*ROUTINES,*STATE]]]
    for command in commands:
        subprocess.run(command,cwd=ROOT,check=True)
    return commands


class Session:
    def __init__(self,pe,original,seed):
        self.original = original
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.size = (pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095
        self.uc = Uc(UC_ARCH_X86,UC_MODE_32)
        self.uc.mem_map(self.base,self.size)
        self.uc.mem_write(self.base,pe.get_memory_mapped_image())
        self.uc.mem_map(ARENA,0x30000)
        self.uc.mem_map(STACK,0x10000)
        self.uc.mem_map(STOP,0x1000)
        rng = random.Random(seed)
        self.uc.mem_write(ARENA,rng.randbytes(0x30000))
        self.symbols = ({n:0x400000+r[0] for n,r in ROUTINES.items()} |
            {n:0x4bad60+4*i for i,n in enumerate(STATE)}) if original else validation_symbols(pe)
        for n in STATE:
            self.uc.mem_write(self.symbols[n],struct.pack('<I',rng.getrandbits(32)))
        self.uc.hook_add(UC_HOOK_MEM_READ,self.read)
        self.uc.hook_add(UC_HOOK_MEM_WRITE,self.write)
        self.uc.hook_add(UC_HOOK_MEM_INVALID,self.invalid)

    def canonical(self,address):
        for i,n in enumerate(STATE):
            offset = address-self.symbols[n]
            if 0<=offset<4:
                return 0x4bad60+i*4+offset
        return address

    def read(self,uc,access,address,size,value,data):
        if STACK<=address<STACK+0x10000:
            return
        self.reads.append((self.canonical(address),size))

    def write(self,uc,access,address,size,value,data):
        if STACK<=address<STACK+0x10000:
            return
        self.writes.append((self.canonical(address),size,value&((1<<(8*size))-1)))

    def invalid(self,uc,access,address,size,value,data):
        self.fault = (access,self.canonical(address),size)
        return False

    def call(self,name,args):
        self.reads,self.writes,self.fault = [],[],None
        sp = STACK+0x8000
        register_leaf = self.original and name.endswith('Core')
        words = [STOP] if register_leaf else [STOP,*[v&0xffffffff for v in args]]
        self.uc.mem_write(sp,struct.pack('<'+'I'*len(words),*words))
        caller = bytes(self.uc.mem_read(sp,0x8000))
        before = bytes(self.uc.mem_read(self.base,self.size))
        saved = [0x13579bdf+i for i in range(4)]
        for r,v in zip(PRESERVED,saved):
            self.uc.reg_write(r,v)
        if register_leaf:
            regs = ((UC_X86_REG_EAX,UC_X86_REG_EBX,UC_X86_REG_ECX,UC_X86_REG_EDX)
                if name.startswith('Gfx_Set') else
                (UC_X86_REG_EDI,UC_X86_REG_ESI,UC_X86_REG_ECX,UC_X86_REG_EBX,UC_X86_REG_EAX))
            for r,v in zip(regs,args):
                self.uc.reg_write(r,v&0xffffffff)
        self.uc.reg_write(UC_X86_REG_ESP,sp)
        self.uc.reg_write(UC_X86_REG_EFLAGS,2)
        try:
            self.uc.emu_start(self.symbols[name],STOP,count=100000)
        except UcError:
            assert self.fault is not None
        if self.fault is None:
            assert self.uc.reg_read(UC_X86_REG_EIP)==STOP
            assert self.uc.reg_read(UC_X86_REG_ESP)==sp+4
            assert bytes(self.uc.mem_read(sp,0x8000))==caller
            if not register_leaf:
                assert [self.uc.reg_read(r) for r in PRESERVED]==saved
            else:
                assert self.uc.reg_read(UC_X86_REG_EBP)==saved[3]
            assert self.uc.reg_read(UC_X86_REG_EFLAGS)&0x400==0
        after = bytearray(self.uc.mem_read(self.base,self.size))
        for n in STATE:
            offset = self.symbols[n]-self.base
            after[offset:offset+4] = before[offset:offset+4]
        assert bytes(after)==before,'Unexpected image write'
        return (self.reads,self.writes,self.fault,bytes(self.uc.mem_read(ARENA,0x30000)),
            tuple(bytes(self.uc.mem_read(self.symbols[n],4)) for n in STATE))


def native_spans():
    runtime = ROOT / 'build/runtime/texture_spans_native'
    runtime.mkdir(parents=True,exist_ok=True)
    copied = runtime / 'IGN_WIN.EXE'
    shutil.copyfile(TARGET,copied)
    verify_target(copied)
    copied_dll = runtime / DLL.name
    shutil.copyfile(DLL,copied_dll)
    mapper = (ROOT/'tools/native_sine_probe.c').read_text(encoding='utf-8')
    mapper = mapper[mapper.index('    file = CreateFileA'):mapper.index('    /* This isolated authentic routine')]
    host = (ROOT/'tools/native_texture_spans.c').read_text(encoding='utf-8')
    marker = '    /* ORIGINAL_PE_MAPPING: generated host inserts the checked existing mapper. */'
    assert host.count(marker)==1
    source = runtime / 'native_texture_spans_host.c'
    source.write_text(host.replace(marker,mapper),encoding='utf-8',newline='\n')
    definition = runtime / 'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+ '\n'.join([
        'CreateFileA@28','ReadFile@20','GetFileSize@8','CloseHandle@4','VirtualAlloc@16',
        'LoadLibraryA@4','GetProcAddress@8','GetStdHandle@4','WriteFile@20','ExitProcess@4'])+'\n',
        encoding='utf-8',newline='\n')
    gcc,dlltool,linker = (shutil.which(n) for n in ('gcc','llvm-dlltool','lld-link'))
    if not all((gcc,dlltool,linker)):
        raise RuntimeError('Native span validation requires installed GCC/dlltool/LLD')
    lib,obj,exe = (runtime/n for n in ('kernel32.lib','native_texture_spans.obj','native_texture_spans.exe'))
    commands = [[dlltool,'-m','i386','-d',str(definition),'-l',str(lib),'--kill-at'],
        [gcc,'-m32','-std=c89','-pedantic-errors','-Wall','-Wextra','-Werror','-O2',
         '-ffreestanding','-fno-builtin','-mno-sse','-mfpmath=387','-c',str(source),'-o',str(obj)],
        [linker,'/entry:NativeEntry@0','/subsystem:console','/nodefaultlib','/machine:x86',
         '/safeseh:no','/base:0x20000000','/dynamicbase:no','/out:'+str(exe),str(obj),str(lib)]]
    for command in commands:
        subprocess.run(command,cwd=ROOT,check=True)
    result = subprocess.run([str(exe)],cwd=runtime,capture_output=True,text=True,timeout=60)
    (runtime/'result.log').write_text(result.stdout+result.stderr,encoding='utf-8',newline='\n')
    assert result.returncode==0,(result.returncode,result.stdout,result.stderr)
    assert result.stdout.startswith('PASS native texture spans: 64 paired calls, 12582912 buffer bytes,')
    verify_target(copied)
    assert hashlib.sha256(DLL.read_bytes()).digest()==hashlib.sha256(copied_dll.read_bytes()).digest()
    evidence = {'cases':64,'paired_span_calls':128,'compared_buffer_bytes':12582912,
        'stdout':result.stdout,'exit_code':result.returncode,'commands':commands+[[str(exe)]],
        'host_sha256':hashlib.sha256(exe.read_bytes()).hexdigest(),
        'scope':'Actual original cdecl setup/span wrappers and real register leaves versus complete C89 renderer; real Win32 mapped PE and owned buffers; no imports called by original render routines',
        'limitations':'Bounded geometry/buffer cases; no native fault/flags/register equality or complete game/render integration'}
    (runtime/'result.json').write_text(json.dumps(evidence,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(result.stdout,flush=True)
    return evidence


def verify_texture_spans(record=False,native=True):
    original = pefile.PE(data=verify_target())
    for rva,size,digest in ROUTINES.values():
        assert hashlib.sha256(original.get_data(rva,size)).hexdigest()==digest
    debug = next(d.struct for d in original.DIRECTORY_ENTRY_DEBUG if d.struct.Type==3)
    fpo = {r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',
        original.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    assert fpo[0x5c990]==(24,0,4,257)
    assert fpo[0x5c9b0]==(32,0,5,771) and fpo[0x5c9d0]==(28,0,4,771)
    assert all(r not in fpo for r in (0x67f1c,0x67f42,0x67f97))
    assert original.get_data(0xbad60,20)==bytes(20),'Unexpected initial span scratch'
    commands = build()
    compiled = pefile.PE(str(DLL))
    assert not getattr(compiled,'DIRECTORY_ENTRY_IMPORT',[]),'Unexpected render dependency'
    symbols = validation_symbols(compiled)
    section = next(s for s in compiled.sections if s.Name.rstrip(b'\0')==b'.text')
    instructions = list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(
        section.get_data()[:section.Misc_VirtualSize],compiled.OPTIONAL_HEADER.ImageBase+section.VirtualAddress))
    calls = [int(i.op_str,16) for i in instructions if i.mnemonic=='call']
    assert len(calls)==3 and set(calls)=={symbols[n] for n in ROUTINES if n.endswith('Core')},calls
    coverage = {n:{'comparisons':0,'faults':0,'followups':0} for n in ROUTINES}
    for seed in range(80):
        rng = random.Random(seed)
        sessions = [Session(p,i==0,seed) for i,p in enumerate((original,compiled))]
        texture,lookup = ARENA+0x10000+(seed*773&65535),ARENA+0x20000+(seed*937&65535)
        # Some real buffers overlap: reads must observe earlier writes.
        destination = (ARENA+0x10000+0x100 if seed%5==0 else
            ARENA+0x20000+0x100 if seed%5==1 else ARENA+0x1000)
        setup = (rng.getrandbits(32),rng.getrandbits(32),texture,lookup)
        def compare(name,args,followup=False):
            results = [s.call(name,args) for s in sessions]
            assert results[0]==results[1],(name,seed,args,results[0][:3],results[1][:3])
            c = coverage[name]
            c['comparisons']+=1;c['faults']+=int(results[0][2] is not None);c['followups']+=int(followup)
        for n in ('Gfx_SetTriangleSpanState','Gfx_SetTriangleSpanCore'):
            compare(n,setup)
        count = (-3,-1,0,1,2,5,17,64)[seed%8]
        args = (destination,count,rng.getrandbits(32),rng.getrandbits(32))
        for n in ('Gfx_DrawTriangleSpan','Gfx_DrawTriangleSpanCore',
                'Gfx_DrawClippedTriangleSpan','Gfx_DrawClippedTriangleSpanCore'):
            compare(n,args+((ARENA+0x100,) if 'Clipped' not in n else ()))
            compare(n,args+((ARENA+0x100,) if 'Clipped' not in n else ()),True)
    # Invalid pointers and wrapped INT_MIN counts must fault at authentic accesses.
    for seed in range(8):
        sessions = [Session(p,i==0,seed) for i,p in enumerate((original,compiled))]
        for s in sessions:
            s.call('Gfx_SetTriangleSpanState',(0,0,ARENA+0x10000,ARENA+0x20000))
        for name in list(ROUTINES)[2:]:
            args = (0 if seed%2==0 else ARENA+0x1000,
                0x80000000 if seed%2 else 1,0,0)
            if 'Clipped' not in name:
                args += (0 if seed%3==0 else ARENA+0x100,)
            results = [s.call(name,args) for s in sessions]
            assert results[0]==results[1],(name,seed,results[0][:3],results[1][:3])
            coverage[name]['comparisons']+=1;coverage[name]['faults']+=int(results[0][2] is not None)
    # Unmapped texture/table and ignored pointers on empty clipped spans.
    for seed in range(16):
        sessions = [Session(p,i==0,seed) for i,p in enumerate((original,compiled))]
        for s in sessions:
            s.call('Gfx_SetTriangleSpanState',(0,0,
                0x4567 if seed%4==0 else ARENA+0x10000,
                0xabcd if seed%4==1 else ARENA+0x20000))
        for name in list(ROUTINES)[2:]:
            args = (0 if seed%4==3 else ARENA+0x1000,
                (-1,0,1,0x80000000)[seed//4],0,0)
            if 'Clipped' not in name:
                args += (0 if seed%4==2 else ARENA+0x100,)
            results = [s.call(name,args) for s in sessions]
            assert results[0]==results[1],(name,seed,results[0][:3],results[1][:3])
            coverage[name]['comparisons']+=1;coverage[name]['faults']+=int(results[0][2] is not None)
    native_result = native_spans() if native else None
    if record:
        for n,(rva,*_) in ROUTINES.items():
            for kind in ('compilation','emulation'):
                record_run(rva,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
                    cases=coverage[n]['comparisons'] if kind=='emulation' else 0,
                    command='uv run tools/verify_texture_spans.py',details={'coverage':coverage[n],
                    'commands':commands,'scope':'Whole production renderer with actual original leaves/wrappers; ordered accesses, full arena/state, faults and ordinary cdecl ABI; internal leaf ABI adapted',
                    'limitations':'No incidental EAX/flags equality, native game, triangle integration or original toolchain certification'})
            if native_result:
                record_run(rva,'native','pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
                    cases=64,command='uv run tools/verify_texture_spans.py',details=native_result)
    print('PASS texture spans:',coverage,flush=True)
    return coverage


if __name__=='__main__':
    verify_texture_spans()
