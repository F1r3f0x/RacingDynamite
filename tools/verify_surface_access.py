# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Surface Lock/Unlock: independent raw-record oracle, real SDK production calls."""
import copy
import hashlib
import random
import struct
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE, UC_HOOK_MEM_INVALID
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EFLAGS
from verify_surface_lifecycle import State as BaseState, Session as BaseSession, BANKS, RECORDS, ARENA, SIZE, VTABLE, LOST, Fault
from verify_matching import STACK, STOP, PRESERVED
from build_decomp import build
from windows_target import ROOT, BUILD, verify_target, validation_symbols, validate_platform_imports
from windows_tracking import record_run
from verify_surface_open import native_surface

if not __debug__:
    raise RuntimeError('Verification requires assertions')
FUNCTIONS={name:ARENA+0x2000+16*k for k,name in enumerate(('IsLost','Restore','Lock','Unlock'))}
OFFSETS={'IsLost':0x60,'Restore':0x6c,'Lock':0x64,'Unlock':0x80}
PIXELS=ARENA+0x2800
DLL=BUILD/'surface_access_validation.dll'
ROUTINES={
 'Gfx_SurfaceLockNative':(0x5c530,326,(326,27,2,778),'35592a3ed0bfaa7af2ea2a5d0ca5116f8f7f3f225a78705c31cc9ddfa8b3b512'),
 'Gfx_SurfaceUnlockNative':(0x5c680,71,(71,0,1,257),'0e6fdf8dbb9b1279859342662207e6693a69b37cf823335fd050d2c6aae1c7af')}
INPUTS=['decomp/src/geputget.c','decomp/include/geputget.h','decomp/src/main.c','decomp/include/main.h',
 'decomp/src/mem.c','decomp/include/mem.h','decomp/src/lisa3d.c','decomp/include/lisa3d.h',
 'decomp/target.json','tools/build_decomp.py','tools/verify_matching.py','tools/verify_surface_access.py',
 'tools/verify_surface_lifecycle.py','tools/windows_target.py','tools/windows_tracking.py',
 'tools/verify_surface_open.py','tools/native_surface_probe.c']

class State(BaseState):
    def __init__(self,seed):
        super().__init__(seed)
        self.script={};self.seen={}
        for name,offset in OFFSETS.items():self.put(VTABLE+offset,FUNCTIONS[name])
        for p in RECORDS:
            for offset,value in ((0,1),(8,0),(12,0),(16,PIXELS),(20,PIXELS),(24,16),(28,4),(32,4)):
                self.put(p+offset,value)
    def response(self,name):
        k=self.seen.get(name,0);self.seen[name]=k+1
        result,pixels,pitch,changes=self.script.get((name,k),(0,PIXELS,16,[]))
        for p,x in changes:self.put(p,x)
        return result,pixels,pitch
    def method(self,name,obj,args=()):
        vt=self.read(obj,False);assert self.read(vt+OFFSETS[name],False)==FUNCTIONS[name]
        self.events.append(('api',name,(obj,)+args,self.snapshot()))
        return self.response(name)
    def lock(self,p,mode):
        if self.read(p)==0:return 0
        if self.read(p+8)==1:
            if self.read(p+12)==mode:return 1
            result,_,_=self.method('Unlock',self.read(p+44),(p+44,))
            if result:return 0
            self.write(p+12,0)
        result,_,_=self.method('IsLost',self.read(p+44))
        if result==LOST:
            self.method('Restore',self.read(p+44));self.write(p+4,1)
        if mode not in (1,2,3):return 0
        result,pixels,pitch=self.method('Lock',self.read(p+44),(0,108,{1:0x11,2:1,3:0x21}[mode],0))
        self.write(p+16,0 if mode==3 else pixels);self.write(p+20,0 if mode==1 else pixels)
        if result:
            self.write(p+16,0);self.write(p+20,0);self.write(p+8,0);return 0
        self.write(p+12,mode);self.write(p+24,pitch);self.write(p+8,1);return 1
    def unlock(self,p):
        if self.read(p)==0:return 0
        if self.read(p+8)==0:return 1
        result,_,_=self.method('Unlock',self.read(p+44),(p+44,))
        if result:return 0
        self.write(p+8,0);self.write(p+12,0);return 1
    def apply(self,name,args):
        self.events=[];self.fault=None;self.seen={}
        try:return self.lock(*args) if name=='Gfx_SurfaceLockNative' else self.unlock(args[0])
        except Fault:return None

class Session(BaseSession):
    def __init__(self,pe,state,original):
        self.uc=Uc(UC_ARCH_X86,UC_MODE_32);self.base=pe.OPTIONAL_HEADER.ImageBase
        self.size=(pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095
        self.uc.mem_map(self.base,self.size);self.uc.mem_write(self.base,pe.get_memory_mapped_image())
        self.uc.mem_map(ARENA,SIZE);self.uc.mem_map(STACK,0x10000);self.uc.mem_map(STOP,4096)
        self.symbols=({name:0x400000+row[0] for name,row in ROUTINES.items()}|{name:p for name,p,_ in BANKS}) if original else validation_symbols(pe)
        self.fields=[(p,self.symbols[name],n) for name,p,n in BANKS]
        for p,b in state.data.items():self.uc.mem_write(self.physical(p),bytes(b))
        self.uc.hook_add(UC_HOOK_CODE,self.code);self.uc.hook_add(UC_HOOK_MEM_READ|UC_HOOK_MEM_WRITE,self.memory)
        self.uc.hook_add(UC_HOOK_MEM_INVALID,self.invalid)
    def code(self,uc,a,n,data):
        reverse={p:name for name,p in FUNCTIONS.items()}
        if a not in reverse:return
        name=reverse[a];argc={'IsLost':1,'Restore':1,'Lock':5,'Unlock':2}[name]
        sp=uc.reg_read(UC_X86_REG_ESP);ret,*raw=struct.unpack('<'+'I'*(argc+1),uc.mem_read(sp,4*(argc+1)))
        args=[self.normalize(x) for x in raw]
        if name=='Lock':
            assert struct.unpack('<I',uc.mem_read(raw[2],4))[0]==108
            assert STACK<=raw[2] and raw[2]+108<STACK+0x10000
            args[2]=108
        self.events.append(('api',name,tuple(args),self.snapshot()))
        result,pixels,pitch=self.policy.response(name);occurrence=self.policy.seen[name]-1
        for p,x in self.policy.script.get((name,occurrence),(None,None,None,[]))[3]:uc.mem_write(self.physical(p),struct.pack('<I',x))
        if name=='Lock':
            uc.mem_write(raw[2]+16,struct.pack('<I',pitch&0xffffffff));uc.mem_write(raw[2]+36,struct.pack('<I',pixels))
        for reg,x in ((UC_X86_REG_EAX,result),(UC_X86_REG_ECX,0xdeadbeef),(UC_X86_REG_EDX,0x12345678),
          (UC_X86_REG_ESP,sp+4+4*argc),(UC_X86_REG_EIP,ret),(UC_X86_REG_EFLAGS,0x8d7)):uc.reg_write(reg,x)
    def run(self,name,args,state,result,seed,policy):
        self.policy=policy;self.events=[];self.fault=None;uc=self.uc;sp=STACK+0x8000
        physical=(self.physical(args[0]),)+tuple(args[1:])
        uc.mem_write(sp,struct.pack('<'+'I'*(len(args)+1),STOP,*physical));caller=bytes(uc.mem_read(sp,0x8000))
        saved=[random.Random(seed+k).getrandbits(32) for k in range(4)]
        for reg,x in zip(PRESERVED,saved):uc.reg_write(reg,x)
        uc.reg_write(UC_X86_REG_ESP,sp);uc.reg_write(UC_X86_REG_EFLAGS,2)
        before=bytearray(uc.mem_read(self.base,self.size))
        try:uc.emu_start(self.symbols[name],STOP,count=4000)
        except UcError:assert self.fault is not None
        assert self.fault==state.fault,(seed,name,self.fault,state.fault)
        if self.events!=state.events:
            mismatch=next(((k,x,y) for k,(x,y) in enumerate(zip(self.events,state.events)) if x!=y),(len(self.events),len(state.events)))
            raise AssertionError((seed,name,mismatch))
        assert self.snapshot()==state.snapshot(),(seed,name,'state')
        assert bytes(uc.mem_read(sp,0x8000))==caller
        after=bytearray(uc.mem_read(self.base,self.size))
        for _,p,n in self.fields:before[p-self.base:p-self.base+n]=after[p-self.base:p-self.base+n]
        assert before==after,'Unrelated image changed'
        if not self.fault:
            assert uc.reg_read(UC_X86_REG_EAX)==result and uc.reg_read(UC_X86_REG_EIP)==STOP
            assert uc.reg_read(UC_X86_REG_ESP)==sp+4 and [uc.reg_read(r) for r in PRESERVED]==saved
            assert not uc.reg_read(UC_X86_REG_EFLAGS)&0x400

def verify_surface_access(record=True):
    original=pefile.PE(data=verify_target());build(dll=DLL);compiled=pefile.PE(str(DLL));validate_platform_imports(compiled)
    debug=next(d.struct for d in original.DIRECTORY_ENTRY_DEBUG if d.struct.Type==3)
    fpo={r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',original.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    for r,n,row,digest in ROUTINES.values():assert fpo[r]==row and hashlib.sha256(original.get_data(r,n)).hexdigest()==digest
    coverage={name:{'comparisons':0,'followups':0,'faults':0,'mutations':0} for name in ROUTINES}
    def sequence(state,steps):
        sessions=[Session(pe,state,k==0) for k,pe in enumerate((original,compiled))]
        for k,(name,args) in enumerate(steps):
            policy=copy.deepcopy(state);policy.seen={};result=state.apply(name,args)
            for session in sessions:session.run(name,args,state,result,sum(c['comparisons'] for c in coverage.values()),copy.deepcopy(policy))
            c=coverage[name];c['comparisons']+=1;c['followups']+=int(k>0);c['faults']+=int(state.fault is not None)
            c['mutations']+=int(any(changes and occurrence<state.seen.get(api,0) for (api,occurrence),(_,_,_,changes) in state.script.items()))
            if state.fault:break
    lock,unlock=ROUTINES
    for p in RECORDS:
        for mode in (0,1,2,3,4,0xffffffff):
            state=State(p);sequence(state,[(lock,(p,mode)),(lock,(p,mode)),(unlock,(p,)),(unlock,(p,))])
    p=RECORDS[0]
    for active in (0,1,2,0xffffffff):
        for flag in (0,1,2,0xffffffff):
            for previous in (0,1,2,3,4,0xffffffff):
                for mode in (0,1,2,3,4,0xffffffff):
                    state=State(5);state.put(p,active);state.put(p+8,flag);state.put(p+12,previous)
                    sequence(state,[(lock,(p,mode)),(unlock,(p,))])
    for api in FUNCTIONS:
        for reply in (0,1,0x80004005,LOST):
            for mode in (1,2,3,4):
                state=State(8);state.put(p+8,1);state.put(p+12,0);state.script[api,0]=(reply,PIXELS,0xfffffff0,[])
                sequence(state,[(lock,(p,mode)),(unlock,(p,))])
    for api in FUNCTIONS:
        state=State(9);state.put(p+8,1);state.put(p+12,0)
        if api=='Restore':state.script['IsLost',0]=(LOST,PIXELS,16,[])
        state.script[api,0]=(0,PIXELS,32,[(p+4,7),(p+8,2),(p+12,99),(p+24,0xabcdef12)])
        sequence(state,[(lock,(p,3)),(unlock,(p,)),(lock,(p,2))])
    for bad in (0,0xdead0000):
        state=State(10);state.put(p+44,bad);sequence(state,[(lock,(p,2))])
        state=State(10);state.put(p+8,1);state.put(p+44,bad);sequence(state,[(unlock,(p,))])
    if record:
        for name,(r,*_) in ROUTINES.items():
            for kind in ('compilation','emulation'):
                record_run(r,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
                    cases=coverage[name]['comparisons'] if kind=='emulation' else 0,command='uv run tools/verify_surface_access.py',
                    details={'coverage':coverage[name],'scope':'Real C89 SDK bodies; deterministic SDK replies/output fields explicitly modeled',
                     'limitations':'Failed Lock without descriptor outputs/uninitialized stack transients, arbitrary aliases/reentry/native fault atomicity and full native game remain unverified'})
        native=native_surface(DLL,access_test=True)
        for name,(r,*_) in ROUTINES.items():
            record_run(r,'native','pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),cases=3,
                command='uv run tools/verify_surface_access.py',details=native|{'routine':name})
        print('PASS native surface access:',native['stdout'],flush=True)
    print('PASS surface access:',coverage,flush=True)
    return coverage

if __name__=='__main__':
    import sys
    verify_surface_access(record='--no-record' not in sys.argv)
