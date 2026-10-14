# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Clipped wrappers execute real splitter/initializer/emitter/clamp in both images."""
import hashlib
import random
import struct
import pefile
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
from unicorn import UcError,UC_ERR_EXCEPTION
from unicorn.x86_const import UC_X86_REG_ESI,UC_X86_REG_EBX,UC_X86_REG_EBP,UC_X86_REG_ESP,UC_X86_REG_EIP,UC_X86_REG_EFLAGS
from verify_texture_spans import Session,build,validate_renderer_calls,STACK,STOP,ARENA,PRESERVED,INPUTS as SPAN_INPUTS
from windows_target import ROOT,BUILD,verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')
DLL=BUILD/'edge_wrappers_validation.dll'
ROUTINES={
    'Gfx_BuildClippedCorrectedTriangleEdge':(0x24efa0,282,'e10f563c0f702038ca423f24d915feeb909d74f16dad34b9eba4581dbe78209c'),
    'Gfx_BuildClippedRawTriangleEdge':(0x24f0c0,282,'481a1b8d63fbc07baff6bb008b9916fe07f4e5f7cc1c5a0cb7808e4f7c08fbb2')}
GLOBALS={'g_triangleTextureGradients':(0x4bada4,8),'g_triangleViewportState':(0x63f2b0,36),
    'g_triangleEdgeSteps':(0x4baea4,32),'g_triangleEdgeSplit':(0x4baec8,20),
    'g_triangleEdgeContinuation':(0x4baedc,16),'g_triangleEdgeUCorrections':(0x4badb4,32),
    'g_triangleEdgeVCorrections':(0x4badd4,32)}
INPUTS=SPAN_INPUTS+['tools/verify_edge_wrappers.py']


class WrapperSession(Session):
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
        self.uc.mem_write(self.address,struct.pack('<8I',*[v&0xffffffff for v in packet]))

    def canonical(self,address):
        for n,(original,size) in GLOBALS.items():
            offset=address-self.symbols[n]
            if 0<=offset<size:return original+offset
        return super().canonical(address)

    def call_wrapper(self,name):
        self.reads,self.writes,self.fault=[],[],None
        sp=STACK+0x8000
        self.uc.mem_write(sp,struct.pack('<I',STOP) if self.original else struct.pack('<3I',STOP,self.address,self.output))
        caller=bytes(self.uc.mem_read(sp,0x8000));before=bytes(self.uc.mem_read(self.base,self.size))
        saved=[0x12345678+i for i in range(4)]
        for r,v in zip(PRESERVED,saved):self.uc.reg_write(r,v)
        if self.original:
            self.uc.reg_write(UC_X86_REG_ESI,self.address);self.uc.reg_write(UC_X86_REG_EBX,self.output)
        self.uc.reg_write(UC_X86_REG_ESP,sp);self.uc.reg_write(UC_X86_REG_EFLAGS,2)
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
            if self.original:assert self.uc.reg_read(UC_X86_REG_EBP)==saved[3]
            else:assert [self.uc.reg_read(r) for r in PRESERVED]==saved
            assert self.uc.reg_read(UC_X86_REG_EFLAGS)&0x400==0
        assert bytes(self.uc.mem_read(sp,0x8000))==caller
        after=bytearray(self.uc.mem_read(self.base,self.size))
        for n,(_,size) in GLOBALS.items():
            offset=self.symbols[n]-self.base;after[offset:offset+size]=before[offset:offset+size]
        assert bytes(after)==before,'Unexpected image write'
        return self.reads,self.writes,exception,bytes(self.uc.mem_read(ARENA,0x30000)),tuple(
            bytes(self.uc.mem_read(self.symbols[n],size)) for n,(_,size) in GLOBALS.items())


def verify_edge_wrappers(record=False):
    original=pefile.PE(data=verify_target())
    for rva,size,digest in ROUTINES.values():assert hashlib.sha256(original.get_data(rva,size)).hexdigest()==digest
    assert original.get_data(0xbaedc,16)==bytes(16)
    commands=build(dll=DLL,exports=[*ROUTINES,*GLOBALS]);compiled=pefile.PE(str(DLL))
    assert not getattr(compiled,'DIRECTORY_ENTRY_IMPORT',[]);validate_renderer_calls(compiled)
    coverage={n:{'comparisons':0,'followups':0,'faults':0,'packet_output_aliases':0} for n in ROUTINES}
    rng=random.Random(0x24efa0)
    packets=[]
    for seed in range(64):
        packets.append([rng.randint(-512,10000),rng.randint(0,2000),rng.randint(-512,10000),rng.randint(5000,9500),*[rng.randint(-20000,20000) for _ in range(4)]])
    packets += [[0,20*256,0,10*256,0,0,0,0],[0,5*256,0,5*256,0,0,0,0],
        [0,0,0,128,0,0,0,0],[0,30*256,0,35*256,0,0,0,0],
        [40*256,5*256,10*256,20*256,1000,2000,3000,4000],
        [10*256,5*256,40*256,20*256,1000,2000,3000,4000],
        [29*256+255,5*256,40*256,20*256,1000,2000,3000,4000]]
    for name in ROUTINES:
        for seed,packet in enumerate(packets):
            for output in (0x1000,0x100,0x108,0x2fff8,0x2fffc,-ARENA):
                sessions=[WrapperSession(p,i==0,seed,packet,output) for i,p in enumerate((original,compiled))]
                results=[s.call_wrapper(name) for s in sessions]
                assert results[0]==results[1],(name,seed,output,'initial mismatch')
                c=coverage[name];c['comparisons']+=1;c['faults']+=int(results[0][2] is not None)
                c['packet_output_aliases']+=int(output in (0x100,0x108))
                if results[0][2] is None:
                    results=[s.call_wrapper(name) for s in sessions]
                    assert results[0]==results[1],(name,seed,output,'followup mismatch')
                    c['comparisons']+=1;c['followups']+=1;c['faults']+=int(results[0][2] is not None)
                    c['packet_output_aliases']+=int(output in (0x100,0x108))
    if record:
        for name,(rva,*_) in ROUTINES.items():
            for kind in ('compilation','emulation'):
                record_run(rva,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
                    cases=coverage[name]['comparisons'] if kind=='emulation' else 0,
                    command='uv run tools/verify_edge_wrappers.py',details={'coverage':coverage[name],'commands':commands,
                    'scope':'Complete original clipped wrappers and actual helper closure versus full production C89 real dependencies; exact state/records/read/store/fault prefixes and packet/output aliases',
                    'limitations':'DEV-007 ESI/EBX adapted to typed packet/header; no incidentalregister outputs/native wrapper/parent/flags/faultstack/game or original toolchain certification'})
    print('PASS edge wrappers:',coverage,flush=True)
    return coverage


if __name__=='__main__':
    verify_edge_wrappers()
