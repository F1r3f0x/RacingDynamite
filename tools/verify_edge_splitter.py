# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Whole original splitter versus real C89 arithmetic and owned viewport/state."""
import hashlib
import random
import struct
import pefile
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
from unicorn import UcError,UC_ERR_EXCEPTION
from unicorn.x86_const import UC_X86_REG_ESI,UC_X86_REG_ESP,UC_X86_REG_EIP,UC_X86_REG_EFLAGS
from verify_texture_spans import Session,build,validate_renderer_calls,STACK,STOP,ARENA,PRESERVED,INPUTS as SPAN_INPUTS
from windows_target import ROOT,BUILD,verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')
NAME='Gfx_ClipAndSplitTriangleEdge'
RVA=0x24f270
DLL=BUILD/'edge_splitter_validation.dll'
GLOBALS={'g_triangleTextureGradients':(0x4bada4,8),
    'g_triangleViewportState':(0x63f2b0,36),'g_triangleEdgeSplit':(0x4baec8,20)}
INPUTS=SPAN_INPUTS+['tools/verify_edge_splitter.py','tools/verify_edge_clamp.py']


class SplitSession(Session):
    def __init__(self,pe,original,seed,packet,viewport,gradients,alias=False):
        super().__init__(pe,original,seed)
        if original:
            self.symbols.update({NAME:0x400000+RVA}|{n:a for n,(a,_) in GLOBALS.items()})
        self.uc.mem_write(self.symbols['g_triangleTextureGradients'],struct.pack('<2I',*[v&0xffffffff for v in gradients]))
        self.uc.mem_write(self.symbols['g_triangleViewportState'],struct.pack('<9I',*[v&0xffffffff for v in viewport]))
        self.uc.mem_write(self.symbols['g_triangleEdgeSplit'],random.Random(seed).randbytes(20))
        self.address=self.symbols['g_triangleViewportState'] if alias else ARENA+0x100
        self.uc.mem_write(self.address,struct.pack('<8I',*[v&0xffffffff for v in packet]))
        self.mutation=None
        self.mutated=False

    def canonical(self,address):
        for n,(original,size) in GLOBALS.items():
            offset=address-self.symbols[n]
            if 0<=offset<size:return original+offset
        return super().canonical(address)

    def write(self,uc,access,address,size,value,data):
        super().write(uc,access,address,size,value,data)
        if self.mutation and not self.mutated and address==self.address+self.mutation[0]:
            self.mutated=True
            uc.mem_write(self.symbols['g_triangleViewportState']+32,struct.pack('<I',self.mutation[1]))

    def call_split(self,mutation=None):
        self.reads,self.writes,self.fault=[],[],None
        self.mutation=mutation;self.mutated=False
        sp=STACK+0x8000
        self.uc.mem_write(sp,struct.pack('<I',STOP) if self.original else struct.pack('<II',STOP,self.address))
        before=bytes(self.uc.mem_read(self.base,self.size));caller=bytes(self.uc.mem_read(sp,0x8000))
        saved=[0x12345678+i for i in range(4)]
        if self.original:saved[1]=self.address
        for r,v in zip(PRESERVED,saved):self.uc.reg_write(r,v)
        self.uc.reg_write(UC_X86_REG_ESP,sp);self.uc.reg_write(UC_X86_REG_EFLAGS,2)
        exception=None
        try:
            self.uc.emu_start(self.symbols[NAME],STOP,count=20000)
        except UcError as error:
            if self.fault is not None:exception=self.fault
            else:
                assert error.errno==UC_ERR_EXCEPTION
                ip=self.uc.reg_read(UC_X86_REG_EIP)
                assert next(Cs(CS_ARCH_X86,CS_MODE_32).disasm(self.uc.mem_read(ip,16),ip)).mnemonic=='div'
                if self.original:assert ip in (0x64f28a,0x64f2d6,0x64f39e,0x64f453)
                exception='DIV #DE'
        if exception is None:
            assert self.uc.reg_read(UC_X86_REG_EIP)==STOP and self.uc.reg_read(UC_X86_REG_ESP)==sp+4
            assert [self.uc.reg_read(r) for r in PRESERVED]==saved
            assert self.uc.reg_read(UC_X86_REG_EFLAGS)&0x400==0
        assert bytes(self.uc.mem_read(sp,0x8000))==caller
        after=bytearray(self.uc.mem_read(self.base,self.size))
        for n,(_,size) in GLOBALS.items():
            offset=self.symbols[n]-self.base;after[offset:offset+size]=before[offset:offset+size]
        assert bytes(after)==before,'Unexpected image write'
        return self.reads,self.writes,exception,self.mutated,bytes(self.uc.mem_read(ARENA,0x30000)),tuple(
            bytes(self.uc.mem_read(self.symbols[n],size)) for n,(_,size) in GLOBALS.items())


def verify_edge_splitter(record=False):
    original=pefile.PE(data=verify_target())
    assert hashlib.sha256(original.get_data(RVA,537)).hexdigest()=='6c18e8828be874b0f51f9f8e808820be102209247fa3746bbccfae3fed7361c3'
    assert original.get_data(0xbaec8,20)==bytes(20)
    commands=build(dll=DLL,exports=[NAME,*GLOBALS])
    compiled=pefile.PE(str(DLL));assert not getattr(compiled,'DIRECTORY_ENTRY_IMPORT',[])
    validate_renderer_calls(compiled)
    viewport=[256,30,1,30,1,29*256+255,64,256,29*256+255]
    rng=random.Random(RVA)
    coverage={'comparisons':0,'followups':0,'faults':0,'aliases':0,'live_rechecks':0}
    for seed in range(256):
        packet=[rng.randint(-2000,12000),rng.randint(0,1500),rng.randint(-2000,12000),rng.randint(4000,10000),*[rng.getrandbits(32) for _ in range(4)]]
        if seed%13==0:packet[0]=viewport[8]
        if seed%17==0:packet[2]=viewport[8]
        gradients=[rng.getrandbits(32),rng.getrandbits(32)]
        for alias in (False,True):
            sessions=[SplitSession(p,i==0,seed,packet,viewport,gradients,alias) for i,p in enumerate((original,compiled))]
            results=[s.call_split() for s in sessions]
            assert results[0]==results[1],(seed,alias,results[0][:4],results[1][:4])
            coverage['comparisons']+=1;coverage['faults']+=int(results[0][2] is not None);coverage['aliases']+=int(alias)
            if results[0][2] is None:
                results=[s.call_split() for s in sessions]
                assert results[0]==results[1],(seed,alias,'followup',results[0][:4],results[1][:4])
                coverage['comparisons']+=1;coverage['followups']+=1;coverage['faults']+=int(results[0][2] is not None)
                coverage['aliases']+=int(alias)
    vp=[256,30,1,30,1,29*256+255,64,256,30*256]
    packet=[40*256,5*256,35*256,20*256,1000,2000,3000,4000]
    sessions=[SplitSession(p,i==0,991,packet,vp,[1234,5678]) for i,p in enumerate((original,compiled))]
    results=[s.call_split((16,25*256)) for s in sessions]
    assert results[0]==results[1] and results[0][3] and results[0][2] is None
    assert struct.unpack('<I',results[0][-1][-1][:4])[0]==1
    assert struct.unpack('<8I',sessions[0].uc.mem_read(sessions[0].address,32))==(6400,1280,6400,1281,952,1779,953,1780)
    coverage['comparisons']+=1;coverage['live_rechecks']+=1
    faults=[([0,0,0,128,0,0,0,0],viewport),
        ([0,256,0,8000,0,0,0,0],[256,30,1,30,1,256,64,256,viewport[8]])]
    for seed,(packet,vp) in enumerate(faults):
        sessions=[SplitSession(p,i==0,900+seed,packet,vp,[0,0]) for i,p in enumerate((original,compiled))]
        results=[s.call_split() for s in sessions]
        assert results[0]==results[1] and results[0][2]=='DIV #DE'
        coverage['comparisons']+=1;coverage['faults']+=1
    for seed,address in enumerate((0,ARENA+0x2fff0,ARENA+0x2ffeb)):
        packet=[10*256,5*256,35*256,20*256,1000,2000,3000,4000]
        sessions=[SplitSession(p,i==0,950+seed,packet,viewport,[1234,5678]) for i,p in enumerate((original,compiled))]
        for s in sessions:
            s.address=address
            if address:
                length=ARENA+0x30000-address
                s.uc.mem_write(address,struct.pack('<8I',*packet)[:length])
        results=[s.call_split() for s in sessions]
        assert results[0]==results[1] and isinstance(results[0][2],tuple),(address,results[0][:4],results[1][:4])
        coverage['comparisons']+=1;coverage['faults']+=1
    if record:
        for kind in ('compilation','emulation'):
            record_run(RVA,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
                cases=coverage['comparisons'] if kind=='emulation' else 0,
                command='uv run tools/verify_edge_splitter.py',details={'coverage':coverage,'commands':commands,
                'scope':'Whole original splitter versus complete C89 renderer with real viewport/splitstate, exact accesses/stores/full image/arena, aliases and live recheck/real DIV faults',
                'limitations':'DEV-007 typed packet/arithmetic; no native splitter/parent/flags/faultstack/game certification or arbitrary alias/SEH/original toolchain proof'})
    print('PASS edge splitter:',coverage,flush=True)
    return coverage


if __name__=='__main__':
    verify_edge_splitter()
