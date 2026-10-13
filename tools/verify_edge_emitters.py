# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Complete original edge emitters versus C cursor ABI, without modeled callees."""
import hashlib
import random
import struct
import pefile
from unicorn import UcError
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EBX, UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EFLAGS
from verify_texture_spans import Session, build, STACK, STOP, STATE, PRESERVED, INPUTS as SPAN_INPUTS
from windows_target import ROOT, BUILD, verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')
DLL = BUILD / 'edge_emitters_validation.dll'
OUTPUT,PACKET = 0x7400000,STACK+0x1000
ROUTINES = {
    'Gfx_EmitRawTriangleEdgeRecords':(0x24f1e0,51,'a236e05fa139724a52b65926d5407dead47076e422411d8a02528937d2c48811'),
    'Gfx_EmitCorrectedTriangleEdgeRecords':(0x24f220,70,'39c7e37061dbb0ba74b5e5735eee6081a8d152da7a6391807d978cc1e9c0bbe6')}
GLOBALS = {'g_triangleEdgeSteps':(0x4baeac,24),
    'g_triangleEdgeUCorrections':(0x4badb4,32),'g_triangleEdgeVCorrections':(0x4badd4,32)}
INPUTS = SPAN_INPUTS + ['tools/verify_edge_emitters.py']


class EdgeSession(Session):
    def __init__(self,pe,original,seed):
        super().__init__(pe,original,seed)
        if original:
            self.symbols.update({n:0x400000+r[0] for n,r in ROUTINES.items()})
            self.symbols.update({n:r[0] for n,r in GLOBALS.items()})
        self.uc.mem_map(OUTPUT,0x100000)
        rng = random.Random(seed+0x24f1e0)
        self.uc.mem_write(OUTPUT,rng.randbytes(0x100000))
        for n,(_,size) in GLOBALS.items():
            self.uc.mem_write(self.symbols[n],rng.randbytes(size))

    def canonical(self,address):
        for n,(original,size) in GLOBALS.items():
            offset = address-self.symbols[n]
            if 0<=offset<size:
                return original+offset
        return super().canonical(address)

    def call_edge(self,name,cursor):
        self.reads,self.writes,self.fault = [],[],None
        v,u,x,output,count = cursor
        self.uc.mem_write(PACKET,struct.pack('<5I',v,u,x,output,count))
        sp = STACK+0x8000
        self.uc.mem_write(sp,struct.pack('<I',STOP) if self.original else struct.pack('<II',STOP,PACKET))
        before = bytes(self.uc.mem_read(self.base,self.size))
        caller = bytes(self.uc.mem_read(sp,0x8000))
        saved = [0x12345678+i for i in range(4)]
        for r,value in zip(PRESERVED,saved):
            self.uc.reg_write(r,value)
        if self.original:
            for r,value in zip((UC_X86_REG_EAX,UC_X86_REG_ESI,UC_X86_REG_EDX,UC_X86_REG_EBX,UC_X86_REG_ECX),cursor):
                self.uc.reg_write(r,value)
        self.uc.reg_write(UC_X86_REG_ESP,sp)
        self.uc.reg_write(UC_X86_REG_EFLAGS,2)
        try:
            self.uc.emu_start(self.symbols[name],STOP,count=5000000)
        except UcError:
            assert self.fault is not None
        returned = None
        if self.fault is None:
            assert self.uc.reg_read(UC_X86_REG_EIP)==STOP,'Instruction bound reached without return'
            assert self.uc.reg_read(UC_X86_REG_ESP)==sp+4
            assert bytes(self.uc.mem_read(sp,0x8000))==caller
            if self.original:
                returned = tuple(self.uc.reg_read(r) for r in (UC_X86_REG_EAX,UC_X86_REG_ESI,UC_X86_REG_EDX,UC_X86_REG_EBX,UC_X86_REG_ECX))
            else:
                returned = struct.unpack('<5I',self.uc.mem_read(PACKET,20))
                assert [self.uc.reg_read(r) for r in PRESERVED]==saved
            assert self.uc.reg_read(UC_X86_REG_EFLAGS)&0x400==0
            # Pointers in the adapted cursor compare by advancement from input.
            returned = returned[:3]+((returned[3]-output)&0xffffffff,returned[4])
        after = bytearray(self.uc.mem_read(self.base,self.size))
        for n,(_,size) in GLOBALS.items():
            offset = self.symbols[n]-self.base
            after[offset:offset+size] = before[offset:offset+size]
        assert bytes(after)==before,'Unexpected image write'
        return (self.reads,self.writes,self.fault,returned,bytes(self.uc.mem_read(OUTPUT,0x100000)),
            tuple(bytes(self.uc.mem_read(self.symbols[n],size)) for n,(_,size) in GLOBALS.items()))


def verify_edge_emitters(record=False):
    original = pefile.PE(data=verify_target())
    for rva,size,digest in ROUTINES.values():
        assert hashlib.sha256(original.get_data(rva,size)).hexdigest()==digest
    assert original.get_data(0x24f213,13)==bytes.fromhex('87db'*6+'90')
    assert original.get_data(0x24f266,10)==bytes.fromhex('87db'*5)
    for address,size in GLOBALS.values():
        assert original.get_data(address-0x400000,size)==bytes(size)
    debug = next(d.struct for d in original.DIRECTORY_ENTRY_DEBUG if d.struct.Type==3)
    fpo = {r for r,*_ in struct.iter_unpack('<IIIHH',
        original.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    assert all(r not in fpo for r,_,_ in ROUTINES.values())
    commands = build(dll=DLL,exports=[*ROUTINES,*GLOBALS])
    compiled = pefile.PE(str(DLL))
    assert not getattr(compiled,'DIRECTORY_ENTRY_IMPORT',[])
    coverage = {n:{'comparisons':0,'followups':0,'faults':0,'aliases':0,'records':0} for n in ROUTINES}
    def compare(name,seed,count,alias=None,followup=False):
        rng = random.Random(seed)
        sessions = [EdgeSession(p,i==0,seed) for i,p in enumerate((original,compiled))]
        values = tuple(rng.getrandbits(32) for _ in range(3))
        def run(count,followup):
            results=[]
            for s in sessions:
                out = (s.symbols[alias[0]]+alias[1]) if alias else OUTPUT+0x100
                results.append(s.call_edge(name,(*values,out,count)))
            assert results[0]==results[1],(name,seed,count,alias,results[0][:4],results[1][:4])
            c = coverage[name];c['comparisons']+=1;c['followups']+=int(followup)
            c['faults']+=int(results[0][2] is not None);c['aliases']+=int(alias is not None)
            c['records']+=len([w for w in results[0][1] if w[1]==4])//3
        run(count,False)
        if followup:
            run(count,True)
    for name in ROUTINES:
        for seed in range(80):
            count = (0,1,2,3,7,17,64,0xffffffff)[seed%8]
            compare(name,seed,count,followup=True)
        for seed,count in enumerate((0x7fff,0x8000,0xffff,0x10000,0x10001,0x80000000,0x80000001)):
            compare(name,100+seed,count)
        for n,size in [('g_triangleEdgeSteps',24),('g_triangleEdgeUCorrections',32),('g_triangleEdgeVCorrections',32)]:
            for offset in range(0,size-11,4):
                compare(name,200+offset,1,(n,offset),True)
        # An unmapped destination faults on the first authentic U-word store.
        for seed,out in enumerate((0,OUTPUT+0xffff8,OUTPUT+0xffffc,OUTPUT+0xffffa,0xfffffffc)):
            sessions = [EdgeSession(p,i==0,901+seed) for i,p in enumerate((original,compiled))]
            results = [s.call_edge(name,(1,2,3,out,1)) for s in sessions]
            assert results[0]==results[1],(name,out,results[0][:4],results[1][:4])
            assert results[0][2] is not None
            coverage[name]['comparisons']+=1;coverage[name]['faults']+=1
    if record:
        for n,(rva,*_) in ROUTINES.items():
            for kind in ('compilation','emulation'):
                record_run(rva,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
                    cases=coverage[n]['comparisons'] if kind=='emulation' else 0,
                    command='uv run tools/verify_edge_emitters.py',details={'coverage':coverage[n],
                    'commands':commands,'scope':'Complete original register emitters versus whole production C89 renderer; typed register input/output cursor adaptation, ordered state/record accesses and stores, all image/buffer bytes, actual faults and ordinary compiled C ABI',
                    'limitations':'No native register-leaf shim or native emitter/game certification; adapted cursor must be disjoint from output; no flags/clobber equality, original toolchain or complete triangle integration'})
    print('PASS edge emitters:',coverage,flush=True)
    return coverage


if __name__=='__main__':
    verify_edge_emitters()
