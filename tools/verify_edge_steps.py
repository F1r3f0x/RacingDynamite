# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Complete original edge initialization versus runtime-free production C89."""
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
NAME='Gfx_InitializeTriangleEdgeSteps'
RVA=0x24f4d0
DLL=BUILD/'edge_steps_validation.dll'
GLOBAL='g_triangleEdgeSteps'
INPUTS=SPAN_INPUTS+['tools/verify_edge_steps.py','tools/verify_edge_emitters.py']


class StepSession(Session):
    def __init__(self,pe,original,seed,packet,alias=False):
        super().__init__(pe,original,seed)
        if original:
            self.symbols.update({NAME:0x400000+RVA,GLOBAL:0x4baea4})
        self.address=self.symbols[GLOBAL] if alias else ARENA+0x100
        rng=random.Random(seed+RVA)
        self.uc.mem_write(self.symbols[GLOBAL],rng.randbytes(32))
        self.uc.mem_write(self.address,struct.pack('<8I',*packet))

    def canonical(self,address):
        offset=address-self.symbols.get(GLOBAL,-0x10000000)
        if 0<=offset<32:
            return 0x4baea4+offset
        return super().canonical(address)

    def call_steps(self):
        self.reads,self.writes,self.fault=[],[],None
        sp=STACK+0x8000
        self.uc.mem_write(sp,struct.pack('<I',STOP) if self.original else struct.pack('<II',STOP,self.address))
        caller=bytes(self.uc.mem_read(sp,0x8000));before=bytes(self.uc.mem_read(self.base,self.size))
        saved=[0x12345678+i for i in range(4)]
        if self.original:saved[1]=self.address
        for r,v in zip(PRESERVED,saved):self.uc.reg_write(r,v)
        self.uc.reg_write(UC_X86_REG_ESP,sp);self.uc.reg_write(UC_X86_REG_EFLAGS,2)
        exception=None
        try:
            self.uc.emu_start(self.symbols[NAME],STOP,count=10000)
        except UcError as error:
            if self.fault is not None:
                exception=self.fault
            else:
                assert error.errno==UC_ERR_EXCEPTION
                ip=self.uc.reg_read(UC_X86_REG_EIP)
                assert next(Cs(CS_ARCH_X86,CS_MODE_32).disasm(self.uc.mem_read(ip,16),ip)).mnemonic=='idiv'
                if self.original:assert ip in (0x64f50d,0x64f522,0x64f536)
                exception='IDIV #DE'
        if exception is None:
            assert self.uc.reg_read(UC_X86_REG_EIP)==STOP and self.uc.reg_read(UC_X86_REG_ESP)==sp+4
            assert [self.uc.reg_read(r) for r in PRESERVED]==saved
            assert self.uc.reg_read(UC_X86_REG_EFLAGS)&0x400==0
        assert bytes(self.uc.mem_read(sp,0x8000))==caller
        after=bytearray(self.uc.mem_read(self.base,self.size))
        offset=self.symbols[GLOBAL]-self.base
        after[offset:offset+32]=before[offset:offset+32]
        assert bytes(after)==before,'Unexpected image write'
        return self.reads,self.writes,exception,bytes(self.uc.mem_read(ARENA,0x30000)),bytes(self.uc.mem_read(self.symbols[GLOBAL],32))


def verify_edge_steps(record=False):
    original=pefile.PE(data=verify_target())
    assert hashlib.sha256(original.get_data(RVA,199)).hexdigest()=='26ecf3f443aeac09a0baa2abe2104e6a883bf3b7d9c45d1fa3c20272dc944383'
    assert original.get_data(0xbaea4,32)==bytes(32)
    commands=build(dll=DLL,exports=[NAME,GLOBAL])
    compiled=pefile.PE(str(DLL));assert not getattr(compiled,'DIRECTORY_ENTRY_IMPORT',[])
    validate_renderer_calls(compiled)
    debug=next(d.struct for d in original.DIRECTORY_ENTRY_DEBUG if d.struct.Type==3)
    fpo={r for r,*_ in struct.iter_unpack('<IIIHH',
        original.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    assert RVA not in fpo
    rng=random.Random(RVA)
    coverage={'comparisons':0,'faults':0,'aliases':0,'followups':0,'stores':0}
    for seed in range(512):
        packet=[rng.getrandbits(32) for _ in range(8)]
        dy=(0,1,2,3,4,64,65,256,0xffffffff,0x80000000,rng.getrandbits(32))[seed%11]
        packet[3]=(packet[1]+dy)&0xffffffff
        if seed%3==0:packet[2]=(packet[0]+(0,1,65536,65537,0x80000000)[seed%5])&0xffffffff
        for alias in (False,True):
            sessions=[StepSession(p,i==0,seed,packet,alias) for i,p in enumerate((original,compiled))]
            results=[s.call_steps() for s in sessions]
            assert results[0]==results[1],(seed,alias,results[0][:3],results[1][:3])
            coverage['comparisons']+=1;coverage['faults']+=int(results[0][2] is not None)
            coverage['aliases']+=int(alias);coverage['stores']+=len(results[0][1])
            if results[0][2] is None:
                results=[s.call_steps() for s in sessions]
                assert results[0]==results[1],(seed,alias,'followup',results[0][:3],results[1][:3])
                coverage['comparisons']+=1;coverage['followups']+=1
                coverage['faults']+=int(results[0][2] is not None);coverage['aliases']+=int(alias)
                coverage['stores']+=len(results[0][1])
    for seed,address in enumerate((0,ARENA+0x2ffe8,ARENA+0x2fff4)):
        packet=[0,0,256,256,0,0,256,256]
        sessions=[StepSession(p,i==0,901+seed,packet) for i,p in enumerate((original,compiled))]
        for s in sessions:
            s.address=address
            if address:
                length=ARENA+0x30000-address
                s.uc.mem_write(address,struct.pack('<8I',*packet)[:length])
        results=[s.call_steps() for s in sessions]
        assert results[0]==results[1],(address,results[0][:3],results[1][:3])
        assert isinstance(results[0][2],tuple)
        coverage['comparisons']+=1;coverage['faults']+=1;coverage['stores']+=len(results[0][1])
    if record:
        for kind in ('compilation','emulation'):
            record_run(RVA,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
                cases=coverage['comparisons'] if kind=='emulation' else 0,
                command='uv run tools/verify_edge_steps.py',details={'coverage':coverage,'commands':commands,
                'scope':'Complete original register initializer versus whole C89 renderer, real paired-word division and IDIV failures, exact reads/stores and full state/image/arena, aliases and normal compiled ABI',
                'limitations':'DEV-007 typed pointer/internal arithmetic adaptation; native initializer/parent, flags and fault-stack/unwinding equality unverified; original compiler unknown'})
    print('PASS edge steps:',coverage,flush=True)
    return coverage


if __name__=='__main__':
    verify_edge_steps()
