# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Complete authentic viewport/state bodies versus direct C89 renderer, real callees."""
import hashlib
import random
import struct
import json
import shutil
import subprocess
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import UcError
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EFLAGS
from verify_texture_spans import Session, build, validate_renderer_calls, STACK, STOP, ARENA, PRESERVED, INPUTS as RENDERER_INPUTS
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run
if not __debug__:
    raise RuntimeError('Assertions required')
DLL=BUILD/'viewport_setters_validation.dll'
ROUTINES = {
    'Gfx_SpriteCloseNative': (0x56f40, 6, '2db31f4e09597946e56e859813bfdad7046d457c32332c3a882654d1e3ffdf67'),
    'Gfx_SpriteOptionNative': (0x56f50, 3, '4bc724f3b1d0caf4fe369c18cba3102e6c4ea057f63fe1587e3973134a7f755e'),
    'Gfx_SpriteConfigureNative': (0x56f60, 187, 'b9bcf21802d579d076091d7908499681c374a5786d4b5c46ce5d981135950c93'),
    'Gfx_SpriteSetClipNative': (0x57020, 225, '223c2cd20c7f79c950d9fc173d170037f4bde1047b285142be51fe551173dbcb'),
    'Gfx_SpriteGetStateNative': (0x57110, 92, '9be413481f65ac603e7873f46b10cff7a09f8de06c3a0904b100e8b4f77a18ae'),
    'Gfx_SpriteSetStateNative': (0x57170, 63, '635cc8612df0e75ae8e7760faa6cd17b0730605c52c19d15184a89158f2092c7'),
}

EXPECTED_FPO = {
    0x56f40: (6, 0, 0, 0),
    0x56f50: (3, 0, 1, 0),
    0x56f60: (187, 0, 5, 5142),
    0x57020: (225, 0, 4, 5130),
    0x57110: (92, 0, 1, 0),
    0x57170: (63, 0, 1, 257),
}

GLOBALS = {
    'g_spritePixels': (0x4ba6f8, 4),
    'g_spriteStride': (0x50ebf4, 4),
    'g_spriteWidth': (0x50ed2c, 4),
    'g_spriteHeight': (0x50ed38, 4),
    'g_spriteConfigOption': (0x50ed34, 4),
    'g_spriteClipLeft': (0x50ed20, 4),
    'g_spriteClipLeftFixed': (0x50ebf0, 4),
    'g_spriteClipTop': (0x50ed24, 4),
    'g_spriteClipTopFixed': (0x50ec00, 4),
    'g_spriteClipRight': (0x50ed28, 4),
    'g_spriteClipRightFixed': (0x50ebf8, 4),
    'g_spriteClipBottom': (0x50ed30, 4),
    'g_spriteClipBottomFixed': (0x50ec04, 4),
    'g_triangleViewportHeight': (0x63f2ac, 4),
    'g_triangleViewportState': (0x63f2b0, 36),
    'g_triangleViewportFramebuffer': (0x63f2d4, 4),
}

def inspect_original(pe):
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {r: (n, l, a, b) for r, n, l, a, b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData + debug.SizeOfData])}
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    for name, (rva, size, digest) in ROUTINES.items():
        assert fpo[rva] == EXPECTED_FPO[rva], f"FPO mismatch for {name}: {fpo[rva]} vs {EXPECTED_FPO[rva]}"
        raw = pe.get_data(rva, size)
        assert hashlib.sha256(raw).hexdigest() == digest, f"Digest mismatch for {name}"
        ins = list(md.disasm(raw, 0x400000 + rva))
        assert sum(i.size for i in ins) == size
        assert ins[-1].mnemonic == 'ret'

    # Padding bytes
    assert pe.get_data(0x56f46, 10) == b'\xcc' * 10
    assert pe.get_data(0x56f53, 13) == b'\xcc' * 13
    assert pe.get_data(0x5701b, 5) == b'\xcc' * 5
    assert pe.get_data(0x57101, 15) == b'\xcc' * 15
    assert pe.get_data(0x5716c, 4) == b'\xcc' * 4
    assert pe.get_data(0x571af, 1) == b'\xcc' * 1

    # Relocation count across the 6 routines
    reloc_entries = [e.rva for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries
                     if e.type == 3 and 0x56f40 <= e.rva < 0x571b0]
    assert len(reloc_entries) == 51, f"Expected 51 relocations, got {len(reloc_entries)}"


INPUTS=RENDERER_INPUTS+['tools/verify_viewport_setters.py','tools/native_viewport_probe.c']

class ViewportSession(Session):
    def __init__(self,pe,original,seed):
        super().__init__(pe,original,seed)
        if original:
            self.symbols.update({n:0x400000+r[0] for n,r in ROUTINES.items()})
            self.symbols.update({n:a for n,(a,_) in GLOBALS.items()})
        rng=random.Random(seed)
        for n,(_,size) in GLOBALS.items():
            self.uc.mem_write(self.symbols[n],rng.randbytes(size))
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
        try:self.uc.emu_start(self.symbols[name],STOP,count=100000)
        except UcError:
            assert self.fault is not None,'Unexpected non-pointer exception'
            exception=self.fault
        if exception is None:
            assert self.uc.reg_read(UC_X86_REG_EIP)==STOP and self.uc.reg_read(UC_X86_REG_ESP)==sp+4
            assert [self.uc.reg_read(reg) for reg in PRESERVED]==saved
            assert self.uc.reg_read(UC_X86_REG_EFLAGS)&0x400==0
        assert bytes(self.uc.mem_read(sp,0x8000))==caller
        after=bytearray(self.uc.mem_read(self.base,self.size))
        for n,(_,size) in GLOBALS.items():
            offset=self.symbols[n]-self.base;after[offset:offset+size]=before[offset:offset+size]
        assert bytes(after)==before,'Unrelated image changed'
        return (self.reads,self.writes,exception,None if exception else self.uc.reg_read(UC_X86_REG_EAX),
            bytes(self.uc.mem_read(ARENA,0x30000)),tuple(bytes(self.uc.mem_read(self.symbols[n],size)) for n,(_,size) in GLOBALS.items()))

def native_viewport():
    runtime=ROOT/'build/runtime/viewport_setters_native'
    runtime.mkdir(parents=True,exist_ok=True)
    copied=runtime/'IGN_WIN.EXE'
    shutil.copyfile(TARGET,copied)
    verify_target(copied)
    copied_dll=runtime/DLL.name
    shutil.copyfile(DLL,copied_dll)
    mapper=(ROOT/'tools/native_sine_probe.c').read_text(encoding='utf-8')
    mapper=mapper[mapper.index('    file = CreateFileA'):mapper.index('    /* This isolated authentic routine')]
    host=(ROOT/'tools/native_viewport_probe.c').read_text(encoding='utf-8')
    generated=',\n'.join('    {\"'+name+'\", '+hex(va-0x400000)+'u, '+str(size)+'u}' for name,(va,size) in GLOBALS.items())
    assert host.count('    /* ORIGINAL_PE_MAPPING */')==1 and host.count('    /* GENERATED_GLOBALS */')==1
    source=runtime/'native_viewport_setters_host.c'
    source.write_text(host.replace('    /* ORIGINAL_PE_MAPPING */',mapper).replace(
        '    /* GENERATED_GLOBALS */',generated),encoding='utf-8',newline='\n')
    definition=runtime/'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+'\n'.join([
        'CreateFileA@28','ReadFile@20','GetFileSize@8','CloseHandle@4','VirtualAlloc@16',
        'LoadLibraryA@4','GetProcAddress@8','GetStdHandle@4','WriteFile@20','ExitProcess@4'])+'\n',
        encoding='utf-8',newline='\n')
    gcc,dlltool,linker=(shutil.which(n) for n in ('gcc','llvm-dlltool','lld-link'))
    if not all((gcc,dlltool,linker)):
        raise RuntimeError('Native slope requires installed provisional GCC/dlltool/LLD')
    lib,obj,exe=(runtime/n for n in ('kernel32.lib','native_viewport_setters.obj','native_viewport_setters.exe'))
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
    assert result.stdout.startswith('PASS native viewport: 448 paired cdecl calls;')
    verify_target(copied)
    assert hashlib.sha256(DLL.read_bytes()).digest()==hashlib.sha256(copied_dll.read_bytes()).digest()
    evidence={'cases':448,'paired_calls':896,'stdout':result.stdout,
        'exit_code':result.returncode,'commands':commands+[[str(exe)]],
        'host_sha256':hashlib.sha256(exe.read_bytes()).hexdigest(),
        'scope':'Actual mapped original cdecl viewport/state functions versus complete production C89 renderer; all16 state objects and40byte records compare; no original imports called',
        'limitations':'No native fault/SEH/unwinding/flags validation, complete triangle or game/startup/menu/input/audio/races/toolchain certification'}
    (runtime/'result.json').write_text(json.dumps(evidence,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(result.stdout,flush=True)
    return evidence


def verify_viewport_setters(record=False,native=True):
    original=pefile.PE(data=verify_target());inspect_original(original)
    commands=build(dll=DLL,exports=[*ROUTINES,*GLOBALS]);compiled=pefile.PE(str(DLL))
    assert not getattr(compiled,'DIRECTORY_ENTRY_IMPORT',[]);validate_renderer_calls(compiled)
    coverage={n:{'comparisons':0,'followups':0,'faults':0,'accepted':0,'rejected':0} for n in ROUTINES}
    def compare(sessions,name,args,followup=False):
        results=[s.invoke(name,args) for s in sessions]
        assert results[0]==results[1],(name,args,'state/access/fault/ABI mismatch')
        c=coverage[name];c['comparisons']+=1;c['followups']+=int(followup)
        c['faults']+=int(results[0][2] is not None)
        if results[0][2] is None:
            c['accepted']+=int(results[0][3]==1);c['rejected']+=int(results[0][3]==0)
        return results[0]
    rng=random.Random(0x56f60)
    for seed in range(128):
        sessions=[ViewportSession(p,i==0,seed) for i,p in enumerate((original,compiled))]
        compare(sessions,'Gfx_SpriteCloseNative',[])
        compare(sessions,'Gfx_SpriteOptionNative',[rng.getrandbits(32)])
        args=[rng.getrandbits(32) for _ in range(5)] if seed>=64 else [ARENA,64,32,24,seed]
        compare(sessions,'Gfx_SpriteConfigureNative',args)
        compare(sessions,'Gfx_SpriteConfigureNative',args,True)
        for clip in ((0,0,1,1),(1,1,32,24),(-1,0,1,1),(0,-1,1,1),
                (1,0,1,1),(0,1,1,1),(0,0,33,24),(0,0,32,25),tuple(rng.getrandbits(32) for _ in range(4))):
            compare(sessions,'Gfx_SpriteSetClipNative',clip,True)
        packet=ARENA+0x100
        compare(sessions,'Gfx_SpriteGetStateNative',[packet])
        compare(sessions,'Gfx_SpriteSetStateNative',[packet],True)
        compare(sessions,'Gfx_SpriteGetStateNative',[packet],True)
    for name in ('Gfx_SpriteGetStateNative','Gfx_SpriteSetStateNative'):
        for seed,address in enumerate((0,ARENA+0x30000,ARENA+0x2fffc,ARENA+0x2fff8,
                ARENA+0x2fff0,ARENA+0x2ffe0,ARENA+0x2ffdc,ARENA+0x2ffd8)):
            sessions=[ViewportSession(p,i==0,seed) for i,p in enumerate((original,compiled))]
            compare(sessions,name,[address])
    for seed in range(128):
        sessions=[ViewportSession(p,i==0,seed) for i,p in enumerate((original,compiled))]
        packet=[rng.getrandbits(32) for _ in range(10)] if seed>=64 else [32,24,seed,1,1,31,23,64,3,ARENA]
        for s in sessions:s.uc.mem_write(ARENA+0x100,struct.pack('<10I',*packet))
        compare(sessions,'Gfx_SpriteSetStateNative',[ARENA+0x100])
        compare(sessions,'Gfx_SpriteSetStateNative',[ARENA+0x100],True)
        compare(sessions,'Gfx_SpriteGetStateNative',[ARENA+0x100],True)
    native_result=native_viewport() if native else None
    if record:
        for n,(rva,*_) in ROUTINES.items():
            for kind in ('compilation','emulation'):
                record_run(rva,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
                    cases=coverage[n]['comparisons'] if kind=='emulation' else 0,
                    command='uv run tools/verify_viewport_setters.py',details={'coverage':coverage[n],
                    'commands':commands,'scope':'Whole original versus directly compiled production C89; real configure/clip callees, exact reads/stores/state/fault prefixes and normal ABI',
                    'limitations':'No native viewport/frame/game parity; no arbitrary image/stack aliases, concurrency, flags/fault-stack/SEH or original toolchain certification'})
            if native_result:
                record_run(rva,'native','pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
                    cases=128 if n=='Gfx_SpriteGetStateNative' else 64,
                    command='uv run tools/verify_viewport_setters.py',details=native_result)
    print('PASS viewport/state:',coverage,flush=True)
    return coverage

if __name__=='__main__':verify_viewport_setters()
