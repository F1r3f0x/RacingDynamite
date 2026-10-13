# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Real original cdecl slope versus complete production renderer, including #DE."""
import hashlib
import json
import random
import shutil
import struct
import subprocess
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import UcError, UC_ERR_EXCEPTION
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EFLAGS
from verify_texture_spans import Session, build, STACK, STOP, PRESERVED, INPUTS as SPAN_INPUTS
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')
NAME = 'Gfx_ComputeTriangleEdgeSlope'
RVA = 0x5d7a0
DLL = BUILD / 'triangle_slope_validation.dll'
INPUTS = SPAN_INPUTS + ['tools/verify_triangle_slope.py','tools/native_triangle_slope.c']


def cases():
    rows=[]
    for x0,x1 in ((0,0),(0,1),(1,0),(0x80000000,0x7fffffff),(0x7fffffff,0x80000000)):
        for y in (0,1,0x80000000,0xffffffff):
            rows.append((x0,y,x1,y))
    rows += [(0x800000,0,0,1),(0,0,0x800000,0xffffffff),(0x800000,1,0,0),
        (0x7fffffff,0x80000000,0x80000000,0x7fffffff),(0x80000000,0x7fffffff,0x7fffffff,0x80000000)]
    rng=random.Random(RVA)
    rows += [tuple(rng.getrandbits(32) for _ in range(4)) for _ in range(1000)]
    return rows


class SlopeSession(Session):
    def __init__(self,pe,original):
        super().__init__(pe,original,RVA)
        if original:
            self.symbols[NAME]=0x400000+RVA

    def call_slope(self,args):
        self.reads,self.writes,self.fault=[],[],None
        sp=STACK+0x8000
        self.uc.mem_write(sp,struct.pack('<5I',STOP,*args))
        caller=bytes(self.uc.mem_read(sp,0x8000))
        before=bytes(self.uc.mem_read(self.base,self.size))
        saved=[0x12345678+i for i in range(4)]
        for r,v in zip(PRESERVED,saved):
            self.uc.reg_write(r,v)
        self.uc.reg_write(UC_X86_REG_ESP,sp)
        self.uc.reg_write(UC_X86_REG_EFLAGS,2)
        exception=None
        try:
            self.uc.emu_start(self.symbols[NAME],STOP,count=1000)
        except UcError as error:
            assert error.errno==UC_ERR_EXCEPTION and self.fault is None
            eip=self.uc.reg_read(UC_X86_REG_EIP)
            instruction=next(Cs(CS_ARCH_X86,CS_MODE_32).disasm(self.uc.mem_read(eip,16),eip))
            assert instruction.mnemonic=='idiv'
            if self.original:
                assert eip==0x45d7d1
            exception='IDIV #DE'
        assert self.reads==[] and self.writes==[]
        assert bytes(self.uc.mem_read(self.base,self.size))==before
        assert bytes(self.uc.mem_read(sp,0x8000))==caller
        if exception is None:
            assert self.uc.reg_read(UC_X86_REG_EIP)==STOP and self.uc.reg_read(UC_X86_REG_ESP)==sp+4
            assert [self.uc.reg_read(r) for r in PRESERVED]==saved
            assert self.uc.reg_read(UC_X86_REG_EFLAGS)&0x400==0
        return exception,None if exception else self.uc.reg_read(UC_X86_REG_EAX)


def expected_slope(args):
    def signed(value):
        return (value&0x7fffffff)-(value&0x80000000)
    x0,y0,x1,y1=args
    if y0==y1:
        return 0x80010000 if signed(x1)<signed(x0) else 0x7fff0000
    numerator=signed(((x0-x1)<<8)&0xffffffff)
    denominator=signed((y0-y1)&0xffffffff)
    quotient=abs(numerator)//abs(denominator)
    if (numerator<0)!=(denominator<0):
        quotient=-quotient
    if not -0x80000000<=quotient<=0x7fffffff:
        raise OverflowError('Authentic IDIV quotient overflow')
    return quotient&0xffffffff


def native_slope():
    runtime=ROOT/'build/runtime/triangle_slope_native'
    runtime.mkdir(parents=True,exist_ok=True)
    copied=runtime/'IGN_WIN.EXE'
    shutil.copyfile(TARGET,copied)
    verify_target(copied)
    copied_dll=runtime/DLL.name
    shutil.copyfile(DLL,copied_dll)
    mapper=(ROOT/'tools/native_sine_probe.c').read_text(encoding='utf-8')
    mapper=mapper[mapper.index('    file = CreateFileA'):mapper.index('    /* This isolated authentic routine')]
    host=(ROOT/'tools/native_triangle_slope.c').read_text(encoding='utf-8')
    rows=cases()[:20]+cases()[-64:]
    assert len(rows)==84
    generated=',\n'.join('    {'+', '.join(hex(v)+'u' for v in (*args,expected_slope(args)))+'}' for args in rows)
    assert host.count('    /* ORIGINAL_PE_MAPPING */')==1 and host.count('    /* GENERATED_SLOPE_CASES */')==1
    source=runtime/'native_triangle_slope_host.c'
    source.write_text(host.replace('    /* ORIGINAL_PE_MAPPING */',mapper).replace(
        '    /* GENERATED_SLOPE_CASES */',generated),encoding='utf-8',newline='\n')
    definition=runtime/'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+'\n'.join([
        'CreateFileA@28','ReadFile@20','GetFileSize@8','CloseHandle@4','VirtualAlloc@16',
        'LoadLibraryA@4','GetProcAddress@8','GetStdHandle@4','WriteFile@20','ExitProcess@4'])+'\n',
        encoding='utf-8',newline='\n')
    gcc,dlltool,linker=(shutil.which(n) for n in ('gcc','llvm-dlltool','lld-link'))
    if not all((gcc,dlltool,linker)):
        raise RuntimeError('Native slope requires installed provisional GCC/dlltool/LLD')
    lib,obj,exe=(runtime/n for n in ('kernel32.lib','native_triangle_slope.obj','native_triangle_slope.exe'))
    commands=[[dlltool,'-m','i386','-d',str(definition),'-l',str(lib),'--kill-at'],
        [gcc,'-m32','-std=c89','-pedantic-errors','-Wall','-Wextra','-Werror','-O2',
         '-ffreestanding','-fno-builtin','-c',str(source),'-o',str(obj)],
        [linker,'/entry:NativeEntry@0','/subsystem:console','/nodefaultlib','/machine:x86',
         '/safeseh:no','/base:0x20000000','/dynamicbase:no','/out:'+str(exe),str(obj),str(lib)]]
    for command in commands:
        subprocess.run(command,cwd=ROOT,check=True)
    result=subprocess.run([str(exe)],cwd=runtime,capture_output=True,text=True,timeout=60)
    (runtime/'result.log').write_text(result.stdout+result.stderr,encoding='utf-8',newline='\n')
    assert result.returncode==0,(result.returncode,result.stdout,result.stderr)
    assert result.stdout.startswith('PASS native triangle slope: 84 paired cdecl calls,')
    verify_target(copied)
    assert hashlib.sha256(DLL.read_bytes()).digest()==hashlib.sha256(copied_dll.read_bytes()).digest()
    evidence={'cases':84,'paired_calls':168,'sentinels':20,'stdout':result.stdout,
        'exit_code':result.returncode,'commands':commands+[[str(exe)]],
        'host_sha256':hashlib.sha256(exe.read_bytes()).hexdigest(),
        'scope':'Actual mapped original cdecl slope and complete production C89 renderer with independently computed expected normal quotients/sentinels; no original imports called',
        'limitations':'No native fault/SEH/unwinding/flags validation, complete triangle or game/startup/menu/input/audio/races/toolchain certification'}
    (runtime/'result.json').write_text(json.dumps(evidence,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(result.stdout,flush=True)
    return evidence


def verify_triangle_slope(record=False,native=True):
    original=pefile.PE(data=verify_target())
    assert hashlib.sha256(original.get_data(RVA,53)).hexdigest()=='fee5ce3932998c6905816c67a42acfbc80612edfa7abc7d724cc90fec87f3bb2'
    assert original.get_data(RVA+53,11)==b'\xcc'*11
    debug=next(d.struct for d in original.DIRECTORY_ENTRY_DEBUG if d.struct.Type==3)
    fpo={r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',
        original.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    assert fpo[RVA]==(53,0,4,261)
    commands=build(dll=DLL,exports=[NAME])
    compiled=pefile.PE(str(DLL))
    assert not getattr(compiled,'DIRECTORY_ENTRY_IMPORT',[])
    sessions=[SlopeSession(p,i==0) for i,p in enumerate((original,compiled))]
    faults=0
    for args in cases():
        results=[s.call_slope(args) for s in sessions]
        assert results[0]==results[1],(args,results)
        try:
            expected=expected_slope(args)
            assert results[0]==(None,expected)
        except OverflowError:
            assert results[0]==('IDIV #DE',None)
        faults+=int(results[0][0] is not None)
    assert faults==1
    coverage={'comparisons':len(cases()),'faults':faults,'sentinels':20}
    native_result=native_slope() if native else None
    if record:
        for kind in ('compilation','emulation'):
            record_run(RVA,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
                cases=coverage['comparisons'] if kind=='emulation' else 0,
                command='uv run tools/verify_triangle_slope.py',details={'coverage':coverage,'commands':commands,
                'scope':'Actual original cdecl slope versus full production C89 renderer; bitwise quotient/sentinels, unchanged state/arguments, ordinary ABI, real IDIV CPU exception',
                'limitations':'Provisional i386 casts and signed-division lowering; no fault stack/unwinding/flags equality or complete triangle/game certification'})
        if native_result:
            record_run(RVA,'native','pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
                cases=84,command='uv run tools/verify_triangle_slope.py',details=native_result)
    print('PASS triangle slope:',coverage,flush=True)
    return coverage


if __name__=='__main__':
    verify_triangle_slope()
