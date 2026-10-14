# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Complete authentic triangle parents versus direct C89 renderer with real dependencies."""
import hashlib
import itertools
import random
import struct
import json
import shutil
import subprocess
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import UcError, UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EFLAGS
from verify_texture_spans import Session, build, validate_renderer_calls, STACK, STOP, ARENA, PRESERVED, INPUTS as RENDERER_INPUTS
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run
if not __debug__:
    raise RuntimeError('Assertions required')
DLL=BUILD/'triangles_validation.dll'
ROUTINES={'Gfx_RasterizeTexturedTriangle':(0x5ca50,1813,'4458bb7690590a3289e6ce7d99b3a80338382fe2afc3b67a25891b2af0e5f8bf'),
    'Gfx_RasterizeClippedTriangle':(0x5d170,1569,'d1d8d33f6f947f949ae7160d07da0fcc3e66d4a67168ad30c5eb80fa62f175f7')}
GLOBALS={
 'g_triangleSpanByteStepStorage':(0x4bad60,4),'g_triangleSpanPackedStep':(0x4bad64,4),
 'g_triangleSpanTextureWord':(0x4bad68,4),'g_triangleSpanLookupWord':(0x4bad6c,4),
 'g_triangleSpanLeftSampleStorage':(0x4bad70,4),'g_triangleVertices':(0x4bad74,48),
 'g_triangleTextureGradients':(0x4bada4,8),'g_triangleEdgeUCorrections':(0x4badb4,32),
 'g_triangleEdgeVCorrections':(0x4badd4,32),'g_triangleGradientScratch':(0x4badf4,32),
 'g_triangleEdgeSteps':(0x4baea4,32),'g_triangleEdgeSplit':(0x4baec8,20),
 'g_triangleEdgeContinuation':(0x4baedc,16),'g_triangleViewportState':(0x63f2b0,36),
 'g_triangleViewportFramebuffer':(0x63f2d4,4),
 'g_triangleOrdinaryPacket':(0x51a9a0,32),'g_triangleClippedPacket':(0x51aa00,32),
 'g_triangleRasterTexture':(0x51a9c4,4),'g_triangleRasterCursor':(0x51a9d0,4),
 'g_triangleRasterMinimumX':(0x51a9d4,4),'g_triangleRasterMaximumX':(0x51a9f8,4),
 'g_spriteWorkspaceBBuffer0':(0x51a9d8,4),'g_spriteWorkspaceBBuffer1':(0x51a9dc,4),
 'g_spriteWorkspaceBBuffer2':(0x51a9e0,4)}
INPUTS=RENDERER_INPUTS+['tools/verify_triangles.py','tools/native_triangle_frames.c']
TEXTURE,FRAMEBUFFER,TABLE=ARENA+0x40000,ARENA+0x50000,ARENA+0x60000
class TriangleSession(Session):
    def __init__(self,pe,original,seed):
        super().__init__(pe,original,seed)
        if original:
            self.symbols.update({n:0x400000+r[0] for n,r in ROUTINES.items()})
            self.symbols.update({n:a for n,(a,_) in GLOBALS.items()})
        self.entries={name:0 for name in ROUTINES}
        for name in ROUTINES:
            self.uc.hook_add(UC_HOOK_CODE,self.entry,user_data=name,
                begin=self.symbols[name],end=self.symbols[name])
        rng=random.Random(seed)
        for n,(_,size) in GLOBALS.items():
            self.uc.mem_write(self.symbols[n],rng.randbytes(size))
        self.uc.mem_map(TEXTURE,0x30000)
        self.uc.mem_write(TEXTURE,rng.randbytes(0x30000))
        viewport=[256,30,1,30,1,29*256+255,64,256,29*256+255]
        self.uc.mem_write(self.symbols['g_triangleViewportState'],struct.pack('<9I',*viewport))
        self.uc.mem_write(self.symbols['g_triangleViewportFramebuffer'],struct.pack('<I',FRAMEBUFFER))
        for i in range(3):self.uc.mem_write(self.symbols['g_spriteWorkspaceBBuffer'+str(i)],struct.pack('<I',ARENA+0x3000+i*0x3000))
    def entry(self,uc,address,size,name):
        self.entries[name]+=1
    def canonical(self,address):
        for n,(va,size) in GLOBALS.items():
            offset=address-self.symbols[n]
            if 0<=offset<size:return va+offset
        return super().canonical(address)
    def invoke(self,name,args):
        self.reads,self.writes,self.fault=[],[],None
        sp=STACK+0x8000
        self.uc.mem_write(sp,struct.pack('<'+'I'*(len(args)+1),STOP,*[v&0xffffffff for v in args]))
        caller=bytes(self.uc.mem_read(sp,0x8000))
        before=bytes(self.uc.mem_read(self.base,self.size))
        saved=[0x12345678+i for i in range(4)]
        for reg,value in zip(PRESERVED,saved):self.uc.reg_write(reg,value)
        self.uc.reg_write(UC_X86_REG_ESP,sp);self.uc.reg_write(UC_X86_REG_EFLAGS,2)
        exception=None
        try:self.uc.emu_start(self.symbols[name],STOP,count=1000000)
        except UcError as error:
            if self.fault is not None:exception=self.fault
            else:
                from unicorn import UC_ERR_EXCEPTION
                assert error.errno==UC_ERR_EXCEPTION
                ip=self.uc.reg_read(UC_X86_REG_EIP)
                op=next(Cs(CS_ARCH_X86,CS_MODE_32).disasm(self.uc.mem_read(ip,16),ip)).mnemonic
                assert op in ('div','idiv')
                exception=op+' #DE'
        if exception is None:
            assert self.uc.reg_read(UC_X86_REG_EIP)==STOP and self.uc.reg_read(UC_X86_REG_ESP)==sp+4
            assert [self.uc.reg_read(reg) for reg in PRESERVED]==saved
            assert self.uc.reg_read(UC_X86_REG_EFLAGS)&0x400==0
        assert bytes(self.uc.mem_read(sp,0x8000))==caller
        after=bytearray(self.uc.mem_read(self.base,self.size))
        for n,(_,size) in GLOBALS.items():
            offset=self.symbols[n]-self.base;after[offset:offset+size]=before[offset:offset+size]
        assert bytes(after)==before,'Unrelated image changed'
        return (self.reads,self.writes,exception,None,
            bytes(self.uc.mem_read(ARENA,0x30000))+bytes(self.uc.mem_read(TEXTURE,0x30000)),tuple(bytes(self.uc.mem_read(self.symbols[n],size)) for n,(_,size) in GLOBALS.items()))


def native_frames():
    runtime=ROOT/'build/runtime/triangle_frames_native'
    runtime.mkdir(parents=True,exist_ok=True)
    copied=runtime/'IGN_WIN.EXE'
    shutil.copyfile(TARGET,copied)
    verify_target(copied)
    copied_dll=runtime/DLL.name
    shutil.copyfile(DLL,copied_dll)
    mapper=(ROOT/'tools/native_sine_probe.c').read_text(encoding='utf-8')
    mapper=mapper[mapper.index('    file = CreateFileA'):mapper.index('    /* This isolated authentic routine')]
    host=(ROOT/'tools/native_triangle_frames.c').read_text(encoding='utf-8')
    generated=',\n'.join('    {\"'+name+'\", '+hex(va-0x400000)+'u, '+str(size)+'u}' for name,(va,size) in GLOBALS.items())
    assert host.count('    /* ORIGINAL_PE_MAPPING */')==1 and host.count('    /* GENERATED_GLOBALS */')==1
    source=runtime/'native_triangle_frames_host.c'
    source.write_text(host.replace('    /* ORIGINAL_PE_MAPPING */',mapper).replace(
        '    /* GENERATED_GLOBALS */',generated),encoding='utf-8',newline='\n')
    definition=runtime/'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+'\n'.join([
        'CreateFileA@28','ReadFile@20','GetFileSize@8','CloseHandle@4','VirtualAlloc@16',
        'LoadLibraryA@4','GetProcAddress@8','GetStdHandle@4','WriteFile@20','ExitProcess@4'])+'\n',
        encoding='utf-8',newline='\n')
    gcc,dlltool,linker=(shutil.which(n) for n in ('gcc','llvm-dlltool','lld-link'))
    if not all((gcc,dlltool,linker)):
        raise RuntimeError('Native triangle frames require installed provisional GCC/dlltool/LLD')
    lib,obj,exe=(runtime/n for n in ('kernel32.lib','native_triangle_frames.obj','native_triangle_frames.exe'))
    commands=[[dlltool,'-m','i386','-d',str(definition),'-l',str(lib),'--kill-at'],
        [gcc,'-m32','-std=c89','-pedantic-errors','-Wall','-Wextra','-Werror','-O2',
         '-ffreestanding','-fno-builtin','-I',str(ROOT/'decomp/include'),'-c',str(source),'-o',str(obj)],
        [linker,'/entry:NativeEntry@0','/subsystem:console','/nodefaultlib','/machine:x86',
         '/safeseh:no','/base:0x20000000','/dynamicbase:no','/out:'+str(exe),str(obj),str(lib)]]
    for command in commands:
        subprocess.run(command,cwd=ROOT,check=True)
    result=subprocess.run([str(exe)],cwd=runtime,capture_output=True,text=True,timeout=60)
    (runtime/'result.log').write_text(result.stdout+result.stderr,encoding='utf-8',newline='\n')
    assert result.returncode==0,(result.returncode,result.stdout,result.stderr)
    assert result.stdout.startswith('PASS native triangle frames: 48 paired parent calls;')
    verify_target(copied)
    assert hashlib.sha256(DLL.read_bytes()).digest()==hashlib.sha256(copied_dll.read_bytes()).digest()
    evidence={'cases':48,'paired_calls':96,'stdout':result.stdout,
        'exit_code':result.returncode,'commands':commands+[[str(exe)]],
        'host_sha256':hashlib.sha256(exe.read_bytes()).hexdigest(),
        'scope':'Actual mapped original whole triangle parents versus complete production renderer;48paired parent calls compare entire frame/edge/shared state bytes; caller-owned workspace fixtures and real configure/clip helpers',
        'limitations':'No native fault/SEH/unwinding/flags, transformed sprites, game/startup/menu/input/audio/races or original toolchain certification; mapped original NX/loader adaptation and caller-owned workspace fixtures'}
    (runtime/'result.json').write_text(json.dumps(evidence,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(result.stdout,flush=True)
    return evidence


def compare_results(original,compiled,label):
    if original==compiled:return
    for kind,a,b in zip(('reads','stores','fault','return','arena','globals'),original,compiled):
        if a!=b:
            if kind in ('reads','stores'):
                i=next((i for i,(x,y) in enumerate(zip(a,b)) if x!=y),min(len(a),len(b)))
                raise AssertionError((label,kind,i,a[max(0,i-3):i+3],b[max(0,i-3):i+3]))
            raise AssertionError((label,kind,'mismatch'))

def verify_triangles(record=False,native=True):
    original=pefile.PE(data=verify_target())
    for rva,size,digest in ROUTINES.values():assert hashlib.sha256(original.get_data(rva,size)).hexdigest()==digest
    commands=build(dll=DLL,exports=[*ROUTINES,*GLOBALS]);compiled=pefile.PE(str(DLL))
    assert not getattr(compiled,'DIRECTORY_ENTRY_IMPORT',[]);validate_renderer_calls(compiled)
    rng=random.Random(0x5ca50)
    rows=[]
    shapes=[[(4*256,4*256),(20*256,8*256),(8*256,20*256)],
        [(3*256,5*256),(9*256,20*256),(24*256,12*256)],
        [(4*256,5*256),(20*256,5*256),(8*256,20*256)],
        [(-3*256,-2*256),(32*256,8*256),(8*256,35*256)],
        [(2*256,2*256),(4*256,2*256),(8*256,2*256)]]
    for shape in shapes:
        for order in itertools.permutations(range(3)):
            vertices=[shape[i] for i in order]
            uv=[(i*300-200,i*170+80) for i in order]
            rows.append((vertices,uv))
    for seed in range(128):
        rows.append(([(rng.randint(-1000,9000),rng.randint(-1000,9000)) for _ in range(3)],
            [(rng.randint(-2000,2000),rng.randint(-2000,2000)) for _ in range(3)]))
    # Preserve all312 normal and60 clipped fixtures from the existing original analysis.
    legacy_shapes=[(4,4,20,4,4,20),(4,4,4,20,20,20),(8,3,3,14,25,24),
        (8,3,25,14,3,24),(4,4,5,4,4,12),(4,4,5,12,4,20),
        (4,4,20,4,12,4),(4,4,4,4,4,4),(8,3,12,13,16,23)]
    uv0=[(512,768),(4608,1024),(1024,4864)]
    for shape in legacy_shapes:
        for order in itertools.permutations(range(3)):
            for fraction in (0,1,127,255):
                rows.append(([(shape[2*i]*256+fraction,shape[2*i+1]*256+fraction) for i in order],[uv0[i] for i in order]))
    old_rng=random.Random(0x24ea80)
    for _ in range(96):
        rows.append(([(old_rng.randrange(512,7168),old_rng.randrange(512,7168)) for _ in range(3)],
            [(old_rng.randrange(-2048,8192),old_rng.randrange(-2048,8192)) for _ in range(3)]))
    for dx,dy in ((-8,0),(20,0),(0,-8),(0,20),(-8,-8),(20,20),(-40,0),(40,0),(0,-40),(0,40)):
        for order in itertools.permutations(range(3)):
            shape=legacy_shapes[2]
            rows.append(([((shape[2*i]+dx)*256,(shape[2*i+1]+dy)*256) for i in order],[uv0[i] for i in order]))
    rows += [([(1024,1024),(1025,1025),(1026,1026)],[(0,0),(2147483647,0),(0,0)])]
    coverage={'comparisons':0,'followups':0,'faults':0,'frame_stores':0}
    visits={name:0 for name in ROUTINES}
    for seed,(vertices,uv) in enumerate(rows):
        sessions=[TriangleSession(p,i==0,seed) for i,p in enumerate((original,compiled))]
        packet=[0,*sum((list(v) for v in vertices),[]),*sum((list(v) for v in uv),[]),TEXTURE,TABLE]
        for q in sessions:q.uc.mem_write(ARENA+0x100,struct.pack('<15I',*[v&0xffffffff for v in packet]))
        results=[q.invoke('Gfx_RasterizeTexturedTriangle',[ARENA+0x100]) for q in sessions]
        compare_results(*results,('outer',seed,vertices,uv))
        coverage['comparisons']+=1;coverage['faults']+=int(results[0][2] is not None)
        assert sessions[0].entries==sessions[1].entries
        for name in visits:visits[name]+=sessions[0].entries[name]
        for q in sessions:q.entries={name:0 for name in ROUTINES}
        coverage['frame_stores']+=sum(FRAMEBUFFER<=a<FRAMEBUFFER+0x10000 for a,size,value in results[0][1])
        if results[0][2] is None:
            results=[q.invoke('Gfx_RasterizeTexturedTriangle',[ARENA+0x100]) for q in sessions]
            compare_results(*results,('followup',seed))
            assert sessions[0].entries==sessions[1].entries
            for name in visits:visits[name]+=sessions[0].entries[name]
            coverage['comparisons']+=1;coverage['followups']+=1;coverage['faults']+=int(results[0][2] is not None)
    # Direct clipped entry consumes real prepared/sorted scratch, not an invented decoder.
    for seed,(vertices,uv) in enumerate(rows[:30]+rows[30:70]):
        ordered=[[*v,*t] for v,t in zip(vertices,uv)]
        if ordered[0][1]>ordered[2][1]:ordered[0],ordered[2]=ordered[2],ordered[0]
        if ordered[0][1]>ordered[1][1]:ordered[0],ordered[1]=ordered[1],ordered[0]
        elif ordered[2][1]<ordered[1][1]:ordered[1],ordered[2]=ordered[2],ordered[1]
        values=[*sum((v[:2] for v in ordered),[]),*sum((v[2:] for v in ordered),[])]
        sessions=[TriangleSession(p,i==0,seed) for i,p in enumerate((original,compiled))]
        packet=[0,*sum((list(v) for v in vertices),[]),*sum((list(v) for v in uv),[]),TEXTURE,TABLE]
        for q in sessions:
            q.uc.mem_write(ARENA+0x100,struct.pack('<15I',*[v&0xffffffff for v in packet]))
            q.uc.mem_write(q.symbols['g_triangleVertices'],struct.pack('<12I',*[v&0xffffffff for v in values]))
            for name,value in [('g_triangleRasterTexture',TEXTURE),('g_triangleRasterCursor',FRAMEBUFFER),
                    ('g_triangleRasterMinimumX',min(v[0] for v in vertices)),('g_triangleRasterMaximumX',max(v[0] for v in vertices))]:
                q.uc.mem_write(q.symbols[name],struct.pack('<I',value&0xffffffff))
        results=[q.invoke('Gfx_RasterizeClippedTriangle',[ARENA+0x100]) for q in sessions]
        compare_results(*results,('direct clipped',seed))
        coverage['comparisons']+=1;coverage['faults']+=int(results[0][2] is not None)
        assert sessions[0].entries==sessions[1].entries
        for name in visits:visits[name]+=sessions[0].entries[name]
        for q in sessions:q.entries={name:0 for name in ROUTINES}
        coverage['frame_stores']+=sum(FRAMEBUFFER<=a<FRAMEBUFFER+0x10000 for a,size,value in results[0][1])
    for seed,(address,bad_buffer,bad_frame) in enumerate([
            (0,None,False),(ARENA+0x2fffc,None,False),(ARENA+0x2ffe0,None,False),
            (ARENA+0x2ffc8,None,False),(ARENA+0x100,0,False),
            (ARENA+0x100,1,False),(ARENA+0x100,2,False),(ARENA+0x100,None,True)]):
        sessions=[TriangleSession(p,i==0,seed) for i,p in enumerate((original,compiled))]
        packet=[0,1024,1024,5120,2048,2048,5120,0,0,200,100,400,200,TEXTURE,TABLE]
        for q in sessions:
            q.uc.mem_write(ARENA+0x100,struct.pack('<15I',*packet))
            if ARENA<=address<ARENA+0x30000:
                available=min(60,ARENA+0x30000-address)
                q.uc.mem_write(address,struct.pack('<15I',*packet)[:available])
            if bad_buffer is not None:q.uc.mem_write(q.symbols['g_spriteWorkspaceBBuffer'+str(bad_buffer)],bytes(4))
            if bad_frame:q.uc.mem_write(q.symbols['g_triangleViewportFramebuffer'],bytes(4))
        results=[q.invoke('Gfx_RasterizeTexturedTriangle',[address]) for q in sessions]
        compare_results(*results,('pointer fault',seed))
        assert results[0][2] is not None,(seed,address,bad_buffer,bad_frame)
        assert sessions[0].entries==sessions[1].entries
        for name in visits:visits[name]+=sessions[0].entries[name]
        coverage['comparisons']+=1;coverage['faults']+=1
    native_result=native_frames() if native else None
    coverage['routine_visits']=visits
    if record:
        for name,(rva,*_) in ROUTINES.items():
            for kind in ('compilation','emulation'):
                record_run(rva,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
                    cases=visits[name] if kind=='emulation' else 0,
                    command='uv run tools/verify_triangles.py',details={'coverage':coverage,'commands':commands,
                    'scope':'Complete original versus production C89 parents/real helpers/storage; exact accesses/state/framebuffers/fault prefixes and normal cdecl ABI',
                    'limitations':'DEV-007 internal helper register adaptation; no arbitrary image/stack aliases/concurrency/nativefaults or game/originaltoolchain parity'})
            if native_result:
                record_run(rva,'native','pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
                    cases=24,command='uv run tools/verify_triangles.py',details=native_result)
    print('PASS whole triangles:' ,coverage,flush=True)
    return coverage

if __name__=='__main__':verify_triangles()
