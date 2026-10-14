# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Complete original gradients versus production C89 with actual shared tables and scratch."""
import hashlib
import random
import itertools
import struct
import pefile
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
from unicorn import UcError,UC_ERR_EXCEPTION
from unicorn.x86_const import UC_X86_REG_ECX,UC_X86_REG_EDI,UC_X86_REG_ESI,UC_X86_REG_EBX,UC_X86_REG_EBP,UC_X86_REG_ESP,UC_X86_REG_EIP,UC_X86_REG_EFLAGS
from verify_texture_spans import Session,build,validate_renderer_calls,STACK,STOP,ARENA,PRESERVED,INPUTS as SPAN_INPUTS
from windows_target import ROOT,BUILD,verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')
DLL=BUILD/'triangle_gradients_validation.dll'
ROUTINES={'Gfx_ComputeTriangleGradients':(0x24ea80,380,'83c0abc6600657871b1ca05baeb39fc3977b6e154ebcfe7578ce339495cfb7aa')}
GLOBALS={'g_triangleTextureGradients':(0x4bada4,8),'g_triangleViewportState':(0x63f2b0,36),
    'g_triangleEdgeSteps':(0x4baea4,32),'g_triangleEdgeSplit':(0x4baec8,20),
    'g_triangleEdgeContinuation':(0x4baedc,16),'g_triangleEdgeUCorrections':(0x4badb4,32),
    'g_triangleEdgeVCorrections':(0x4badd4,32),'g_triangleVertices':(0x4bad74,48),
    'g_triangleGradientScratch':(0x4badf4,32)}
INPUTS=SPAN_INPUTS+['tools/verify_triangle_gradients.py']


class GradientSession(Session):
    def __init__(self,pe,original,seed,packet,output_offset=0x1000):
        super().__init__(pe,original,seed)
        if original:
            self.symbols.update({n:0x400000+r[0] for n,r in ROUTINES.items()})
            self.symbols.update({n:a for n,(a,_) in GLOBALS.items()})
        rng=random.Random(seed)
        for n,(_,size) in GLOBALS.items():self.uc.mem_write(self.symbols[n],rng.randbytes(size))
        viewport=[256,30,1,30,1,29*256+255,64,256,29*256+255]
        self.uc.mem_write(self.symbols['g_triangleViewportState'],struct.pack('<9I',*viewport))
        self.uc.mem_write(self.symbols['g_triangleTextureGradients'],struct.pack('<2I',1234,(-5678)&0xffffffff))
        for j,n in enumerate(('g_triangleEdgeUCorrections','g_triangleEdgeVCorrections')):
            gradient=(1234,-5678)[j];negative=(-gradient)&0xffffffff
            signed=(negative&0x7fffffff)-(negative&0x80000000)
            self.uc.mem_write(self.symbols[n],struct.pack('<8I',*[(i*(signed>>3))&0xffffffff for i in range(8)]))
        self.address=ARENA+0x100
        self.output=ARENA+output_offset
        self.uc.mem_write(self.symbols['g_triangleVertices'],struct.pack('<12I',*[v&0xffffffff for v in packet]))

    def canonical(self,address):
        for n,(original,size) in GLOBALS.items():
            offset=address-self.symbols[n]
            if 0<=offset<size:return original+offset
        return super().canonical(address)

    def call_gradient(self,name):
        self.reads,self.writes,self.fault=[],[],None
        sp=STACK+0x8000
        self.uc.mem_write(sp,struct.pack('<I',STOP))
        caller=bytes(self.uc.mem_read(sp,0x8000));before=bytes(self.uc.mem_read(self.base,self.size))
        saved=[0x12345678+i for i in range(4)]
        for r,v in zip(PRESERVED,saved):self.uc.reg_write(r,v)
        if self.original:
            self.uc.reg_write(UC_X86_REG_ESI,self.address);self.uc.reg_write(UC_X86_REG_EBX,self.output)
        self.uc.reg_write(UC_X86_REG_ESP,sp);self.uc.reg_write(UC_X86_REG_EFLAGS,2)
        original_preserved=[self.uc.reg_read(r) for r in (UC_X86_REG_ECX,UC_X86_REG_ESI,UC_X86_REG_EDI)]
        exception=None
        try:
            self.uc.emu_start(self.symbols[name],STOP,count=300000)
        except UcError as error:
            if self.fault is not None:exception=self.fault
            else:
                assert error.errno==UC_ERR_EXCEPTION
                ip=self.uc.reg_read(UC_X86_REG_EIP)
                mnemonic=next(Cs(CS_ARCH_X86,CS_MODE_32).disasm(self.uc.mem_read(ip,16),ip)).mnemonic
                assert mnemonic in ('div','idiv')
                exception=mnemonic+' #DE'
        if exception is None:
            assert self.uc.reg_read(UC_X86_REG_EIP)==STOP and self.uc.reg_read(UC_X86_REG_ESP)==sp+4
            if self.original:
                assert self.uc.reg_read(UC_X86_REG_EBP)==saved[3]
                assert [self.uc.reg_read(r) for r in (UC_X86_REG_ECX,UC_X86_REG_ESI,UC_X86_REG_EDI)]==original_preserved
            else:assert [self.uc.reg_read(r) for r in PRESERVED]==saved
            assert self.uc.reg_read(UC_X86_REG_EFLAGS)&0x400==0
        assert bytes(self.uc.mem_read(sp,0x8000))==caller
        after=bytearray(self.uc.mem_read(self.base,self.size))
        for n,(_,size) in GLOBALS.items():
            offset=self.symbols[n]-self.base;after[offset:offset+size]=before[offset:offset+size]
        assert bytes(after)==before,'Unexpected image write'
        return self.reads,self.writes,exception,bytes(self.uc.mem_read(ARENA,0x30000)),tuple(
            bytes(self.uc.mem_read(self.symbols[n],size)) for n,(_,size) in GLOBALS.items())


def verify_triangle_gradients(record=False):
    original=pefile.PE(data=verify_target())
    for rva,size,digest in ROUTINES.values():assert hashlib.sha256(original.get_data(rva,size)).hexdigest()==digest
    assert original.get_data(0xbad74,48)==bytes(48)
    assert original.get_data(0xbadf4,32)==bytes(32)
    commands=build(dll=DLL,exports=[*ROUTINES,*GLOBALS]);compiled=pefile.PE(str(DLL))
    assert not getattr(compiled,'DIRECTORY_ENTRY_IMPORT',[]);validate_renderer_calls(compiled)
    coverage={n:{'comparisons':0,'followups':0,'faults':0,'input_cases':1388} for n in ROUTINES}
    rng=random.Random(0x24ea80)
    packets=[]
    for seed in range(256):
        ys=sorted(rng.sample(range(0,5000),3))
        xs=[rng.randint(-5000,5000) for _ in range(3)]
        us=[rng.randint(-5000,5000) for _ in range(3)]
        vs=[rng.randint(-5000,5000) for _ in range(3)]
        packets.append([xs[0],ys[0],xs[1],ys[1],xs[2],ys[2],us[0],vs[0],us[1],vs[1],us[2],vs[2]])
    for ys,xs,uv in itertools.product(
            ((0,0,0),(0,1,1),(0,1,0),(0,2,1),(0,-1,1),(0,0,1)),
            ((0,0,0),(0,1,1),(0,-1,1),(0,2,2),(0,-2,-2),(0,0,65536)),
            ((0,0,0),(0,65536,0),(0,-65536,0))):
        packets.append([xs[0],ys[0],xs[1],ys[1],xs[2],ys[2],uv[0],uv[0],uv[1],uv[1],uv[2],uv[2]])
    wide_rng=random.Random(0x4bad74)
    for seed in range(1024):
        packet=[wide_rng.getrandbits(32) for _ in range(12)]
        if seed%4==0:packet[5]=packet[3]
        if seed%7==0:packet[5]=packet[1]
        packets.append(packet)
    for name in ROUTINES:
        for seed,packet in enumerate(packets):
            sessions=[GradientSession(p,i==0,seed,packet) for i,p in enumerate((original,compiled))]
            results=[s.call_gradient(name) for s in sessions]
            assert results[0]==results[1],(name,seed,'initial mismatch')
            c=coverage[name];c['comparisons']+=1;c['faults']+=int(results[0][2] is not None)
            if results[0][2] is None:
                results=[s.call_gradient(name) for s in sessions]
                assert results[0]==results[1],(name,seed,'followup mismatch')
                c['comparisons']+=1;c['followups']+=1;c['faults']+=int(results[0][2] is not None)
    if record:
        for name,(rva,*_) in ROUTINES.items():
            for kind in ('compilation','emulation'):
                record_run(rva,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
                    cases=coverage[name]['comparisons'] if kind=='emulation' else 0,
                    command='uv run tools/verify_triangle_gradients.py',details={'coverage':coverage[name],'commands':commands,
                    'scope':'Complete original gradient and actual shared input/scratch/table backing versus full production C89 real dependencies; exact gradients/tables/scratch/read/store/fault prefixes and persistent shared gradient/table outputs',
                    'limitations':'DEV-007 internal C89 register ABI adaptation; no incidental register outputs/native gradient/parent/flags/fault stack/game or original toolchain certification'})
    print('PASS triangle gradients:',coverage,flush=True)
    return coverage


if __name__=='__main__':
    verify_triangle_gradients()
