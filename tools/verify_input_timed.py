# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Real timed setup/reinitialization and key consumers versus raw Windows oracle."""
import copy
import hashlib
import random
import struct
import pefile
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE, UC_HOOK_MEM_INVALID
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_EIP, UC_X86_REG_EFLAGS
import verify_input_lifecycle as life
from verify_surface_lifecycle import ARENA, SIZE, Fault
from verify_matching import STACK, STOP
from windows_target import ROOT, BUILD, verify_target, validation_symbols, validate_platform_imports
from windows_tracking import record_run
from build_decomp import build
from verify_surface_open import native_surface

if not __debug__:
    raise RuntimeError('Verification requires assertions')
PERIOD=0x50e67c
FIELDS=life.FIELDS+[('g_nativeInputPeriod',PERIOD,4)]
FUNCTIONS=life.FUNCTIONS|{'timeSetEvent':ARENA+0x3300}
ROUTINES={
 'Input_InitTimedNative':(0x55c60,436,(436,9,3,270),'6ade6c95c919cb9ea6d550e552623c057dbefcdef7b325d552d883a7ad782efe'),
 'Input_ShutdownNativeEntry':(0x55ea0,11,(11,0,0,0),'9600756fc2217d7a1487cb0390abec17132d31aeafb82b2141817396a6727d2c'),
 'Input_ReinitializeNative':(0x55eb0,57,(57,0,0,0),'f6c11630f036e58381a9b2998e2a447b4089fbaa272abc80f77b81bdc2acdc2d'),
 'Input_ReadHeldNative':(0x56070,15,(15,0,1,0),'4be4b0c8433812e5d7a3abe390a16531f5998e56ae6a2e26b95da51a0eba564a'),
 'Input_ConsumePressNative':(0x56080,28,(28,0,1,0),'3ee48544758501dafd75914c3f6fbc8822648eb2536d881b73754074af371684'),
 'Input_ConsumeRepeatNative':(0x560a0,28,(28,0,1,0),'5fec67455369bb53eb5d47621fac91218a140bc0df6dfe9827f458bc03745e42')}
ALL_ROUTINES=ROUTINES|life.ROUTINES|{'Input_TimerCallbackNative':(0x56160,)}
DLL=BUILD/'input_timed_validation.dll'
INPUTS=life.INPUTS+['tools/verify_input_timed.py']

class State(life.State):
    def __init__(self,seed):
        super().__init__(seed)
        self.data[PERIOD]=bytearray(random.Random(seed).randbytes(4))
        self.script['timeSetEvent',0]=(7,None,[])
    def snapshot(self):
        return tuple(bytes(self.data[p]) for p in [ARENA]+[p for _,p,_ in FIELDS])
    def timed(self,args):
        old_id=self.read(life.IDENTIFIER,False)
        result=self.initialize(args[1:])
        if not result:return 0
        # The identical SDK prefix has already been independently authenticated.
        # Ordinary initialization's six-event ending differs from timed setup.
        tail=self.events[-6:]
        assert tail[0][:3]==('read',life.CLOCK,4)
        self.events=self.events[:-6];self.put(life.IDENTIFIER,old_id)
        self.events.append(tail[0]);self.write(life.DELAY,args[1]);self.write(life.INTERVAL,args[2]);self.write(PERIOD,args[0])
        self.write(life.EPOCH,tail[0][3]);self.write(life.TIMER,1)
        timer=self.api('timeSetEvent',(args[0],args[0],0x456160,0,1));self.write(life.IDENTIFIER,timer)
        return int(timer!=0)
    def apply(self,name,args):
        self.events=[];self.fault=None;self.seen={}
        try:
            if name=='Input_InitTimedNative':return self.timed(args)
            if name=='Input_InitNative':return self.initialize(args)
            if name in ('Input_ShutdownNativeEntry','Input_ShutdownNative'):return self.shutdown()
            if name=='Input_ReinitializeNative':
                interval=self.read(life.INTERVAL);delay=self.read(life.DELAY)
                if self.read(life.TIMER)!=0:self.timed((self.read(PERIOD),delay,interval))
                else:self.initialize((delay,interval))
                return 1
            base={'Input_ReadHeldNative':life.HELD,'Input_ConsumePressNative':life.TABLEA,'Input_ConsumeRepeatNative':life.EVENTS}[name]
            address=base+(args[0]&255);b,o=self.region(address,1);value=b[o]
            self.events.append(('read',address,1,value))
            if name=='Input_ReadHeldNative':return value
            if value:self.write(address,0,1)
            return int(value!=0)
        except Fault:return None

class Session(life.Session):
    def __init__(self,pe,state,original):
        self.uc=Uc(UC_ARCH_X86,UC_MODE_32);self.base=pe.OPTIONAL_HEADER.ImageBase
        self.size=(pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095
        self.uc.mem_map(self.base,self.size);self.uc.mem_write(self.base,pe.get_memory_mapped_image())
        self.uc.mem_map(ARENA,SIZE);self.uc.mem_map(STACK,0x10000);self.uc.mem_map(STOP,4096)
        self.symbols=({name:0x400000+row[0] for name,row in ALL_ROUTINES.items()}|{name:p for name,p,_ in FIELDS}) if original else validation_symbols(pe)
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
        rows=[]
        for p,n in [(ARENA,SIZE)]+[(p,n) for _,p,n in FIELDS]:
            raw=bytes(self.uc.mem_read(self.physical(p),n))
            if p==life.CURSOR:raw=struct.pack('<I',self.normalize(int.from_bytes(raw,'little')))
            rows.append(raw)
        return tuple(rows)
    def code(self,uc,a,n,data):
        if a!=FUNCTIONS['timeSetEvent']:return super().code(uc,a,n,data)
        sp=uc.reg_read(UC_X86_REG_ESP);ret,*args=struct.unpack('<6I',uc.mem_read(sp,24))
        assert args[2]==self.symbols['Input_TimerCallbackNative']
        args[2]=0x456160
        self.events.append(('api','timeSetEvent',tuple(args),self.snapshot()))
        reply,_=self.policy.response('timeSetEvent');occurrence=self.policy.seen['timeSetEvent']-1
        for p,x in self.policy.script.get(('timeSetEvent',occurrence),(None,None,[]))[2]:uc.mem_write(self.physical(p),struct.pack('<I',x))
        for r,x in ((UC_X86_REG_EAX,reply),(UC_X86_REG_ECX,0xdeadbeef),(UC_X86_REG_EDX,0x12345678),
          (UC_X86_REG_ESP,sp+24),(UC_X86_REG_EIP,ret),(UC_X86_REG_EFLAGS,0x8d7)):uc.reg_write(r,x)

def verify_input_timed(record=True):
    original=pefile.PE(data=verify_target());build(dll=DLL);compiled=pefile.PE(str(DLL));validate_platform_imports(compiled)
    life.inspect(original,compiled)
    debug=next(d.struct for d in original.DIRECTORY_ENTRY_DEBUG if d.struct.Type==3)
    fpo={r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',original.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    for r,n,row,digest in ROUTINES.values():assert fpo[r]==row and hashlib.sha256(original.get_data(r,n)).hexdigest()==digest
    data_section=next(section for section in original.sections if section.Name.rstrip(b'\0')==b'.data')
    assert data_section.VirtualAddress+data_section.SizeOfRawData<=PERIOD-0x400000
    assert PERIOD-0x400000+4<=data_section.VirtualAddress+data_section.Misc_VirtualSize
    assert original.get_memory_mapped_image()[PERIOD-0x400000:PERIOD-0x400000+4]==b'\0'*4
    coverage={name:{'comparisons':0,'followups':0,'faults':0,'mutations':0} for name in ROUTINES}
    def sequence(state,names,args=(10,300,30)):
        sessions=[Session(pe,state,k==0) for k,pe in enumerate((original,compiled))]
        for k,name in enumerate(names):
            actual=args if name=='Input_InitTimedNative' or name.startswith(('Input_Read','Input_Consume')) else args[1:] if name=='Input_InitNative' else ()
            policy=copy.deepcopy(state);policy.seen={};result=state.apply(name,actual)
            for session in sessions:session.run(name,actual,state,result,sum(c['comparisons'] for c in coverage.values()),copy.deepcopy(policy))
            if name in coverage:
                c=coverage[name];c['comparisons']+=1;c['followups']+=int(k>0);c['faults']+=int(state.fault is not None)
                c['mutations']+=int(any(changes and occurrence<state.seen.get(api,0) for (api,occurrence),(_,_,changes) in state.script.items()))
            if state.fault:break
    for seed in range(12):
        for timer in (0,1,0xffffffff):
            state=State(seed);state.script['timeSetEvent',0]=(timer,None,[])
            rng=random.Random(seed);args=tuple(rng.getrandbits(32) for _ in range(3))
            sequence(state,('Input_InitTimedNative','Input_ShutdownNativeEntry','Input_ReinitializeNative','Input_ShutdownNativeEntry'),args)
    for flag in (0,1,2,0xffffffff):
        for api in ('DirectInputCreateA','CreateDevice','SetDataFormat','SetCooperativeLevel','SetProperty','Acquire'):
            for reply in (1,0x80004005):
                state=State(99);state.put(life.TIMER,flag);state.script[api,0]=(reply,None,[])
                sequence(state,('Input_ReinitializeNative','Input_ShutdownNativeEntry'))
    for api in ('DirectInputCreateA','CreateDevice','SetDataFormat','SetCooperativeLevel','SetProperty','Acquire'):
        for reply in (1,0x80004005):
            state=State(101);state.script[api,0]=(reply,None,[])
            sequence(state,('Input_InitTimedNative','Input_ShutdownNativeEntry'))
    for api in ('DirectInputCreateA','CreateDevice'):
        state=State(102);state.script[api,0]=(0,0,[])
        sequence(state,('Input_InitTimedNative',))
    for api in FUNCTIONS:
        state=State(100);state.script[api,0]=(7 if api=='timeSetEvent' else 0,None,[(life.CLOCK,555),(life.TIMER,0),(PERIOD,999),(life.ACQUIRED,77)])
        sequence(state,('Input_InitTimedNative','Input_ShutdownNativeEntry','Input_ReinitializeNative','Input_ShutdownNativeEntry'))
    for name,base in (('Input_ReadHeldNative',life.HELD),('Input_ConsumePressNative',life.TABLEA),('Input_ConsumeRepeatNative',life.EVENTS)):
        for index in range(256):
            for value in (0,1,2,255):
                state=State(index);state.put(base+index,value,1);sequence(state,(name,name),(0xabcdef00|index,))
    if record:
        for name,(r,*_) in ROUTINES.items():
            for kind in ('compilation','emulation'):
                record_run(r,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
                    cases=coverage[name]['comparisons'] if kind=='emulation' else 0,command='uv run tools/verify_input_timed.py',
                    details={'coverage':coverage[name],'scope':'Real timed/ordinary setup/shutdown/reinit dependency chain and key consumers; SDK replies modeled',
                     'limitations':'Native asynchronous timer/game and original toolchain not certified; arbitrary reentry/concurrency outside contract'})
        native=native_surface(DLL,timed_test=True)
        for name,(r,*_) in ROUTINES.items():
            record_run(r,'native','pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),cases=3,
                command='uv run tools/verify_input_timed.py',details=native|{'routine':name})
        print('PASS native timed input:',native['stdout'],flush=True)
    print('PASS timed input differential:',coverage,flush=True)
    return coverage

if __name__=='__main__':
    import sys
    verify_input_timed(record='--no-record' not in sys.argv)
