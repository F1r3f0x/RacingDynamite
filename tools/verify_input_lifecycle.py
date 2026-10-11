# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Independent native keyboard setup/teardown oracle with explicit SDK boundaries."""
import copy,hashlib,random,struct
import pefile
from unicorn import Uc,UcError,UC_ARCH_X86,UC_MODE_32,UC_HOOK_CODE,UC_HOOK_MEM_READ,UC_HOOK_MEM_WRITE,UC_HOOK_MEM_INVALID,UC_MEM_WRITE
from unicorn.x86_const import UC_X86_REG_EAX,UC_X86_REG_ECX,UC_X86_REG_EDX,UC_X86_REG_ESP,UC_X86_REG_EIP,UC_X86_REG_EFLAGS
from verify_matching import STACK,STOP,PRESERVED
from verify_surface_lifecycle import Session as BaseSession,Fault,ARENA,SIZE
from windows_target import ROOT,BUILD,verify_target,validation_symbols,validate_platform_imports
from build_decomp import build
from windows_tracking import record_run
from verify_surface_open import native_surface

if not __debug__:raise RuntimeError('Verification requires assertions')
DLL=BUILD/'input_lifecycle_validation.dll'
INPUTS=['decomp/src/geputget.c','decomp/include/geputget.h','decomp/src/main.c','decomp/include/main.h',
 'decomp/src/mem.c','decomp/include/mem.h','decomp/src/lisa3d.c','decomp/include/lisa3d.h',
 'decomp/target.json','tools/build_decomp.py','tools/verify_matching.py','tools/verify_surface_lifecycle.py',
 'tools/verify_input_lifecycle.py','tools/windows_target.py','tools/windows_tracking.py',
 'tools/verify_surface_open.py','tools/native_surface_probe.c']
INPUT,KEYBOARD,ACQUIRED,TIMER,IDENTIFIER=0x50df60,0x50e26c,0x50e270,0x4ba65c,0x4ba658
INSTANCE,WINDOW,CLOCK,DELAY,INTERVAL,EPOCH=0x4c5398,0x4c539c,0x49373c,0x50df64,0x50e274,0x50e268
QUEUE,CURSOR,END,HELD,EVENTS,TABLEA,TABLEB,COUNTERS=0x50de18,0x50de10,0x50de58,0x50de60,0x50df68,0x50e068,0x50e168,0x50e278
FIELDS=[('g_native'+name,p,n) for name,p,n in (
 ('DirectInput',INPUT,4),('KeyboardDevice',KEYBOARD,4),('KeyboardAcquired',ACQUIRED,4),('InputTimerActive',TIMER,4),
 ('InputTimerId',IDENTIFIER,4),('WindowInstance',INSTANCE,4),('Window',WINDOW,4),('InputClock',CLOCK,4),
 ('RepeatDelay',DELAY,4),('RepeatInterval',INTERVAL,4),('RepeatEpoch',EPOCH,4),('CharacterQueue',QUEUE,64),
 ('CharacterQueueCursor',CURSOR,4),('CharacterQueueEnd',END,1),('KeyboardHeld',HELD,256),('KeyboardPressEvents',EVENTS,256),
 ('KeyboardTableA',TABLEA,256),('KeyboardTableB',TABLEB,256),('RepeatCounters',COUNTERS,1024))]
ARGC={'DirectInputCreateA':4,'CreateDevice':4,'SetDataFormat':2,'SetCooperativeLevel':3,'SetProperty':3,
 'Acquire':1,'Unacquire':1,'Release':1,'timeKillEvent':1}
FUNCTIONS={name:ARENA+0x3000+16*k for k,name in enumerate(ARGC)}
IO,KO,OTHER=ARENA+0x400,ARENA+0x420,ARENA+0x440
IVT,KVT=ARENA+0x1800,ARENA+0x1900
SYS_GUID=bytes.fromhex('612b1d6fa0d5cf11bfc7444553540000')
KEY_GUID=bytes.fromhex('208272553cd3cf11bfc7444553540000')
FORMAT=(24,16,2,256,256,tuple((KEY_GUID,k,0x8000000c|(k<<8),0) for k in range(256)))
PROPERTY=struct.pack('<5I',20,16,0,0,32)
ROUTINES={'Input_InitNative':(0x55ac0,406,(406,9,2,270),'cf2e4a6c7b8596487d881bf18519ca764fcdfc408e3b4411252c92e6a61aa174'),
 'Input_ShutdownNative':(0x55e20,117,(117,0,0,0),'1f060a91fa8308cad4a1f8cfe55b587770a24172b6d4801993012f8c9c74e39c')}

class State:
    def __init__(self,seed):
        rng=random.Random(seed);self.data={p:bytearray(rng.randbytes(n)) for _,p,n in FIELDS}
        self.data[ARENA]=bytearray(rng.randbytes(SIZE));self.events=[];self.fault=None;self.script={};self.seen={}
        for p,x in ((INPUT,IO),(KEYBOARD,KO),(ACQUIRED,1),(TIMER,1),(IDENTIFIER,123),
          (INSTANCE,0x19000100),(WINDOW,0x19000200),(CLOCK,0x12345678),(IO,IVT),(KO,KVT),(OTHER,KVT),
          (IVT+8,FUNCTIONS['Release']),(IVT+12,FUNCTIONS['CreateDevice'])):self.put(p,x)
        for offset,name in ((8,'Release'),(0x18,'SetProperty'),(0x1c,'Acquire'),(0x20,'Unacquire'),(0x2c,'SetDataFormat'),(0x34,'SetCooperativeLevel')):
            self.put(KVT+offset,FUNCTIONS[name])
    def region(self,a,n=4):
        for p,b in self.data.items():
            if p<=a and a+n<=p+len(b):return b,a-p
        self.fault=('read',a,n);raise Fault
    def put(self,a,x,n=4):
        b,o=self.region(a,n);b[o:o+n]=(x&((1<<(8*n))-1)).to_bytes(n,'little')
    def read(self,a,tracked=True):
        b,o=self.region(a);x=int.from_bytes(b[o:o+4],'little')
        if tracked:self.events.append(('read',a,4,x))
        return x
    def write(self,a,x,n=4):
        self.put(a,x,n);self.events.append(('write',a,n,x&((1<<(8*n))-1)))
    def snapshot(self):return tuple(bytes(self.data[p]) for p in [ARENA]+[p for _,p,_ in FIELDS])
    def response(self,name):
        k=self.seen.get(name,0);self.seen[name]=k+1
        default=(0,IO if name=='DirectInputCreateA' else KO if name=='CreateDevice' else None,[])
        reply,output,changes=self.script.get((name,k),default)
        for p,x in changes:self.put(p,x)
        return reply,output
    def api(self,name,args,output=None):
        self.events.append(('api',name,args,self.snapshot()));reply,value=self.response(name)
        if output is not None and value is not None:self.put(output,value)
        return reply
    def method(self,name,obj,args=(),output=None):
        vt=self.read(obj,False)
        offset={'CreateDevice':12,'SetDataFormat':0x2c,'SetCooperativeLevel':0x34,'SetProperty':0x18,
          'Acquire':0x1c,'Unacquire':0x20,'Release':8}[name]
        assert self.read(vt+offset,False)==FUNCTIONS[name]
        return self.api(name,(obj,)+args,output)
    def apply(self,name,args):
        self.events=[];self.fault=None;self.seen={}
        try:return self.initialize(args) if name=='Input_InitNative' else self.shutdown()
        except Fault:return None
    def initialize(self,args):
        instance=self.read(INSTANCE)
        if self.api('DirectInputCreateA',(instance,0x300,INPUT,0),INPUT):return 0
        if self.method('CreateDevice',self.read(INPUT),(SYS_GUID,KEYBOARD,0),KEYBOARD):return 0
        if self.method('SetDataFormat',self.read(KEYBOARD),(FORMAT,)):return 0
        window=self.read(WINDOW);keyboard=self.read(KEYBOARD)
        if self.method('SetCooperativeLevel',keyboard,(window,6)):return 0
        if self.method('SetProperty',self.read(KEYBOARD),(1,PROPERTY)):return 0
        result=self.method('Acquire',self.read(KEYBOARD));self.write(ACQUIRED,1)
        if result&0x80000000:self.write(ACQUIRED,0)
        self.write(CURSOR,QUEUE)
        for k in range(16):self.write(QUEUE+4*k,0xffffffff)
        self.write(END,0,1)
        for p in (EVENTS,HELD):
            for k in range(64):self.write(p+4*k,0)
        for k in range(256):
            self.write(TABLEA+k,0,1);self.write(COUNTERS+4*k,0);self.write(TABLEB+k,0,1)
        now=self.read(CLOCK);self.write(DELAY,args[0]);self.write(INTERVAL,args[1]);self.write(EPOCH,now)
        self.write(TIMER,0);self.write(IDENTIFIER,0);return 1
    def shutdown(self):
        if self.read(ACQUIRED)!=0:
            self.method('Unacquire',self.read(KEYBOARD));self.write(ACQUIRED,0)
        if self.read(KEYBOARD)!=0:
            self.method('Release',self.read(KEYBOARD));self.write(KEYBOARD,0)
        if self.read(INPUT)!=0:
            self.method('Release',self.read(INPUT));self.write(INPUT,0)
        if self.read(TIMER)!=0:self.api('timeKillEvent',(self.read(IDENTIFIER),))
        return 1

def format_graph(read,p):
    words=struct.unpack('<6I',read(p,24));assert words[:5]==(24,16,2,256,256)
    objects=[]
    for guid,offset,kind,flags in struct.iter_unpack('<4I',read(words[5],4096)):
        objects.append((bytes(read(guid,16)),offset,kind,flags))
    return words[:5]+(tuple(objects),)

class Session(BaseSession):
    def __init__(self,pe,state,original):
        self.uc=Uc(UC_ARCH_X86,UC_MODE_32);self.base=pe.OPTIONAL_HEADER.ImageBase
        self.size=(pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095
        self.uc.mem_map(self.base,self.size);self.uc.mem_write(self.base,pe.get_memory_mapped_image())
        self.uc.mem_map(ARENA,SIZE);self.uc.mem_map(STACK,0x10000);self.uc.mem_map(STOP,4096)
        self.symbols=({name:0x400000+r for name,(r,*_) in ROUTINES.items()}|{name:p for name,p,_ in FIELDS}) if original else validation_symbols(pe)
        self.fields=[(p,self.symbols[name],n) for name,p,n in FIELDS]
        for p,b in state.data.items():self.uc.mem_write(self.physical(p),bytes(b))
        for group in pe.DIRECTORY_ENTRY_IMPORT:
            for entry in group.imports:
                name=entry.name.decode() if entry.name else ''
                if name in FUNCTIONS:self.uc.mem_write(entry.address,struct.pack('<I',FUNCTIONS[name]))
        self.callbacks={p:name for name,p in FUNCTIONS.items()}
        self.uc.hook_add(UC_HOOK_CODE,self.code);self.uc.hook_add(UC_HOOK_MEM_READ|UC_HOOK_MEM_WRITE,self.memory)
        self.uc.hook_add(UC_HOOK_MEM_INVALID,self.invalid)
    def snapshot(self):
        values=[]
        for p,n in [(ARENA,SIZE)]+[(p,n) for _,p,n in FIELDS]:
            raw=bytes(self.uc.mem_read(self.physical(p),n))
            if p==CURSOR:raw=struct.pack('<I',self.normalize(int.from_bytes(raw,'little')))
            values.append(raw)
        return tuple(values)
    def memory(self,uc,access,a,n,x,data):
        length=len(self.events);super().memory(uc,access,a,n,x,data)
        if len(self.events)>length and self.normalize(a)==CURSOR:
            row=self.events[-1];self.events[-1]=row[:3]+(self.normalize(row[3]),)
    def code(self,uc,a,n,data):
        if a not in self.callbacks:return
        name=self.callbacks[a];sp=uc.reg_read(UC_X86_REG_ESP)
        raw=struct.unpack('<'+'I'*(ARGC[name]+1),uc.mem_read(sp,4*(ARGC[name]+1)))
        ret=raw[0];args=list(raw[1:]);canonical=[self.normalize(x) for x in args];output=None
        if name=='DirectInputCreateA':output=args[2]
        elif name=='CreateDevice':canonical[1]=bytes(uc.mem_read(args[1],16));output=args[2]
        elif name=='SetDataFormat':canonical[1]=format_graph(uc.mem_read,args[1])
        elif name=='SetProperty':canonical[2]=bytes(uc.mem_read(args[2],20))
        self.events.append(('api',name,tuple(canonical),self.snapshot()))
        reply,value=self.policy.response(name);index=self.policy.seen[name]-1
        for p,x in self.policy.script.get((name,index),(None,None,[]))[2]:uc.mem_write(self.physical(p),struct.pack('<I',x))
        if output is not None and value is not None:uc.mem_write(output,struct.pack('<I',value))
        for reg,x in ((UC_X86_REG_EAX,reply),(UC_X86_REG_ECX,0xdeadbeef),(UC_X86_REG_EDX,0x12345678),
          (UC_X86_REG_ESP,sp+4+4*ARGC[name]),(UC_X86_REG_EIP,ret),(UC_X86_REG_EFLAGS,0x8d7)):uc.reg_write(reg,x)
    def run(self,name,args,state,result,seed,policy):
        self.policy=policy;self.events=[];self.fault=None;uc=self.uc;sp=STACK+0x8000
        uc.mem_write(sp,struct.pack('<'+'I'*(len(args)+1),STOP,*args));caller=bytes(uc.mem_read(sp,0x8000))
        saved=[random.Random(seed+k).getrandbits(32) for k in range(4)]
        for reg,x in zip(PRESERVED,saved):uc.reg_write(reg,x)
        uc.reg_write(UC_X86_REG_ESP,sp);uc.reg_write(UC_X86_REG_EFLAGS,2)
        before=bytearray(uc.mem_read(self.base,self.size))
        try:uc.emu_start(self.symbols[name],STOP,count=25000)
        except UcError:assert self.fault is not None
        assert self.fault==state.fault,(seed,name,self.fault,state.fault)
        if self.events!=state.events:
            mismatch=next(((k,x[:3],y[:3]) for k,(x,y) in enumerate(zip(self.events,state.events)) if x!=y),(len(self.events),len(state.events)))
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

def inspect(original,compiled):
    debug=next(d.struct for d in original.DIRECTORY_ENTRY_DEBUG if d.struct.Type==3)
    fpo={r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',original.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    for r,n,row,digest in ROUTINES.values():
        assert fpo[r]==row and hashlib.sha256(original.get_data(r,n)).hexdigest()==digest
    assert format_graph(lambda p,n:original.get_data(p-0x400000,n),0x478760)==FORMAT
    symbols=validation_symbols(compiled)
    assert format_graph(lambda p,n:compiled.get_data(p-compiled.OPTIONAL_HEADER.ImageBase,n),symbols['g_nativeKeyboardFormat'])==FORMAT
    validate_platform_imports(compiled)

def verify_input_lifecycle():
    original=pefile.PE(data=verify_target());build(dll=DLL);compiled=pefile.PE(str(DLL));inspect(original,compiled)
    coverage={name:{'comparisons':0,'followups':0,'faults':0,'mutations':0} for name in ROUTINES}
    def sequence(state,names,args=(300,30)):
        sessions=[Session(pe,state,k==0) for k,pe in enumerate((original,compiled))]
        for k,name in enumerate(names):
            actual_args=args if name=='Input_InitNative' else ()
            policy=copy.deepcopy(state);result=state.apply(name,actual_args)
            for session in sessions:session.run(name,actual_args,state,result,coverage[name]['comparisons'],copy.deepcopy(policy))
            coverage[name]['comparisons']+=1;coverage[name]['followups']+=int(k>0);coverage[name]['faults']+=int(state.fault is not None)
            coverage[name]['mutations']+=int(any(changes and occurrence<state.seen.get(api,0) for (api,occurrence),(_,_,changes) in state.script.items()))
            if state.fault:break
    for seed in range(16):
        args=tuple(random.Random(seed+k).getrandbits(32) for k in range(2))
        sequence(State(seed),('Input_InitNative','Input_ShutdownNative','Input_ShutdownNative','Input_InitNative'),args)
    for name in ('DirectInputCreateA','CreateDevice','SetDataFormat','SetCooperativeLevel','SetProperty','Acquire'):
        for reply in (1,0x80004005,0x80000000,0xffffffff):
            state=State(31);state.script[name,0]=(reply,None,[])
            sequence(state,('Input_InitNative','Input_ShutdownNative','Input_InitNative'))
    for acquired in (0,1,2,0xffffffff):
        for timer in (0,1,2,0xffffffff):
            for input_live in (False,True):
                state=State(32);state.put(ACQUIRED,acquired);state.put(TIMER,timer);state.put(INPUT,IO if input_live else 0)
                sequence(state,('Input_ShutdownNative',)*3)
    for api,changes in (('Unacquire',[(KEYBOARD,OTHER)]),('Unacquire',[(INPUT,0)]),
      ('Release',[(TIMER,1),(IDENTIFIER,456)]),('Release',[(ACQUIRED,0x12345678)]),
      ('Acquire',[(WINDOW,0x19000300),(CLOCK,0x87654321)]),('CreateDevice',[(WINDOW,0x19000400)])):
        state=State(33);state.script[api,0]=(0,None,changes)
        sequence(state,('Input_InitNative','Input_ShutdownNative','Input_ShutdownNative'))
    for p in (KEYBOARD,INPUT,KO,IO):
        for bad in (0,0x80000000):
            state=State(34);state.put(p,bad);sequence(state,('Input_ShutdownNative',))
    for api in ('DirectInputCreateA','CreateDevice'):
        for output in (0,0x80000000):
            state=State(36);state.script[api,0]=(0,output,[])
            sequence(state,('Input_InitNative',))
    for api in ('SetDataFormat','SetCooperativeLevel','SetProperty','Acquire'):
        state=State(37);state.script[api,0]=(0,None,[(KEYBOARD,OTHER),(CLOCK,0xabcdef01)])
        sequence(state,('Input_InitNative','Input_ShutdownNative'))
    state=State(35);state.put(INPUT,KO);sequence(state,('Input_ShutdownNative',)*2)
    details={'scope':'Real C89 DirectInput setup and shutdown; SDK replies explicitly modeled in differential emulation',
      'coverage':coverage,'checks':'Authentic FPO/hash and complete256object SDKformat/Guids/property; normal/errors/Acquire sign distinction, live callback mutations, persistentcalls, faults/shared release; orderedglobals/fullimage/arena/callerbytes/cdecl/stdcall/nonvolatile/DF',
      'limitations':'No native input/game or original compiler/link/instruction equality claim yet. No-push startup/WndProc repeat timing arguments remain an authentic caller ABI question. Native asynchronous timing/reentry and two state-table meanings unresolved.'}
    for name,(r,*_) in ROUTINES.items():
        for kind in ('compilation','emulation'):
            record_run(r,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),cases=coverage[name]['comparisons'] if kind=='emulation' else 0,command='uv run tools/verify_input_lifecycle.py',details=details)
    print('PASS native input lifecycle differential:',coverage,flush=True)
    native=native_surface(DLL,input_test=True)
    for name,(r,*_) in ROUTINES.items():
        record_run(r,'native','pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),cases=3 if name=='Input_InitNative' else 4,
          command='uv run tools/verify_input_lifecycle.py',details=native|{'routine':name})
    print('PASS actual native input lifecycle:',native['stdout'],flush=True)
    return coverage

if __name__=='__main__':verify_input_lifecycle()
