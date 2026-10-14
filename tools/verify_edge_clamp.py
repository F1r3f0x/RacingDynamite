# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Actual original right clamp versus C89 halfword product and volatile state."""
import hashlib
import random
import struct
import pefile
from unicorn import UcError
from unicorn.x86_const import UC_X86_REG_ESI, UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EFLAGS
from verify_texture_spans import Session, build, STACK, STOP, ARENA, PRESERVED, INPUTS as SPAN_INPUTS
from windows_target import ROOT, BUILD, verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')
NAME='Gfx_ClampTriangleEdgeRightEndpoint'
RVA=0x24f489
DLL=BUILD/'edge_clamp_validation.dll'
GLOBALS={'g_triangleTextureGradients':(0x4bada4,8),'g_triangleViewportState':(0x63f2b0,36)}
INPUTS=SPAN_INPUTS+['tools/verify_edge_clamp.py']


class ClampSession(Session):
    def __init__(self,pe,original,seed,packet,right,du,dv,address=ARENA+0x100):
        super().__init__(pe,original,seed)
        if original:
            self.symbols[NAME]=0x400000+RVA
            self.symbols.update({n:a for n,(a,_) in GLOBALS.items()})
            self.symbols['g_triangleClipRightFixed']=0x63f2d0
        self.address=address
        if address!=0:
            available=max(0,min(32,ARENA+0x30000-address))
            self.uc.mem_write(address,struct.pack('<8I',*packet)[:available])
        self.uc.mem_write(self.symbols['g_triangleTextureGradients'],struct.pack('<2I',du,dv))
        self.uc.mem_write(self.symbols['g_triangleClipRightFixed'],struct.pack('<I',right))
        self.mutation=None

    def canonical(self,address):
        for n,(original,size) in GLOBALS.items():
            offset=address-self.symbols[n]
            if 0<=offset<size:
                return original+offset
        return super().canonical(address)

    def write(self,uc,access,address,size,value,data):
        super().write(uc,access,address,size,value,data)
        if address==self.address+24 and self.mutation:
            dv,right=self.mutation
            uc.mem_write(self.symbols['g_triangleTextureGradients']+4,struct.pack('<I',dv))
            uc.mem_write(self.symbols['g_triangleClipRightFixed'],struct.pack('<I',right))

    def call_clamp(self,mutation=None):
        self.reads,self.writes,self.fault=[],[],None
        self.mutation=mutation
        sp=STACK+0x8000
        self.uc.mem_write(sp,struct.pack('<I',STOP) if self.original else struct.pack('<II',STOP,self.address))
        before=bytes(self.uc.mem_read(self.base,self.size))
        caller=bytes(self.uc.mem_read(sp,0x8000))
        saved=[0x12345678+i for i in range(4)]
        if self.original:
            saved[1]=self.address
        for r,v in zip(PRESERVED,saved):
            self.uc.reg_write(r,v)
        self.uc.reg_write(UC_X86_REG_ESP,sp)
        self.uc.reg_write(UC_X86_REG_EFLAGS,2)
        try:
            self.uc.emu_start(self.symbols[NAME],STOP,count=1000)
        except UcError:
            assert self.fault is not None
        if self.fault is None:
            assert self.uc.reg_read(UC_X86_REG_EIP)==STOP and self.uc.reg_read(UC_X86_REG_ESP)==sp+4
            assert [self.uc.reg_read(r) for r in PRESERVED]==saved
            assert bytes(self.uc.mem_read(sp,0x8000))==caller
            assert self.uc.reg_read(UC_X86_REG_EFLAGS)&0x400==0
        after=bytearray(self.uc.mem_read(self.base,self.size))
        for n,(_,size) in GLOBALS.items():
            offset=self.symbols[n]-self.base
            after[offset:offset+size]=before[offset:offset+size]
        assert bytes(after)==before,'Unexpected image write'
        return self.reads,self.writes,self.fault,bytes(self.uc.mem_read(ARENA,0x30000)),tuple(
            bytes(self.uc.mem_read(self.symbols[n],size)) for n,(_,size) in GLOBALS.items())


def verify_edge_clamp(record=False):
    original=pefile.PE(data=verify_target())
    assert hashlib.sha256(original.get_data(RVA,63)).hexdigest()=='65b2cb3d21d75ff2be58a259eb79e9430f6b01539cb992e6722b2c680aeb27f0'
    assert original.get_data(0xbada4,8)==bytes(8)
    section=original.get_section_by_rva(0x23f2d0)
    assert 0x23f2d0-section.VirtualAddress>=section.SizeOfRawData
    commands=build(dll=DLL,exports=[NAME,*GLOBALS])
    compiled=pefile.PE(str(DLL))
    assert not getattr(compiled,'DIRECTORY_ENTRY_IMPORT',[])
    rng=random.Random(RVA)
    coverage={'comparisons':0,'followups':0,'clamped':0,'mutations':0,'faults':0}
    for seed in range(512):
        packet=[rng.getrandbits(32) for _ in range(8)]
        right=(0,1,0xffffffff,0x80000000,0x7fffffff,rng.getrandbits(32))[seed%6]
        du,dv=rng.getrandbits(32),rng.getrandbits(32)
        if seed%7==0:
            packet[2]=right
        sessions=[ClampSession(p,i==0,seed,packet,right,du,dv) for i,p in enumerate((original,compiled))]
        mutation=((dv^0xabcdef01)&0xffffffff,(right+17)&0xffffffff) if seed%5==0 else None
        for followup in (False,True):
            results=[s.call_clamp(mutation) for s in sessions]
            assert results[0]==results[1],(seed,followup,results[0][:3],results[1][:3])
            coverage['comparisons']+=1;coverage['followups']+=int(followup)
            coverage['clamped']+=int(bool(results[0][1]));coverage['mutations']+=int(bool(results[0][1]) and mutation is not None)
    for seed,address in enumerate((0,ARENA+0x2fff0,ARENA+0x2ffec,ARENA+0x2ffeb)):
        packet=[0,0,0x7fffffff,0,1,2,3,4]
        sessions=[ClampSession(p,i==0,seed,packet,0,0x12345678,0x87654321,address)
            for i,p in enumerate((original,compiled))]
        results=[s.call_clamp() for s in sessions]
        assert results[0]==results[1],(address,results[0][:3],results[1][:3])
        assert results[0][2] is not None
        coverage['comparisons']+=1;coverage['faults']+=1
    if record:
        for kind in ('compilation','emulation'):
            record_run(RVA,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
                cases=coverage['comparisons'] if kind=='emulation' else 0,
                command='uv run tools/verify_edge_clamp.py',details={'coverage':coverage,'commands':commands,
                'scope':'Complete original ESI clamp versus actual C89 typed packet; signed/wrapped halfword product, ordered accesses/stores, live diagnostic mutations, full arena/state/image and ABI/fault prefixes',
                'limitations':'Internal ABI adapted DEV-007; no incidental scratch/flags or native clamp/parent/game certification; arbitrary image/state aliases and original toolchain unresolved'})
    print('PASS edge clamp:',coverage,flush=True)
    return coverage


if __name__=='__main__':
    verify_edge_clamp()
