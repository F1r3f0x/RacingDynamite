# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Buffered DirectInput execution against an independent raw-address oracle.

SDK replies and client callbacks are boundaries; all four production routines
execute together. No keyboard event or full game native claim follows from this.
"""
import copy
import hashlib
import random
import struct
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE, UC_HOOK_MEM_INVALID
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EFLAGS
from verify_matching import STACK, STOP, PRESERVED
from verify_surface_lifecycle import Session as BaseSession, ARENA, SIZE, Fault
from verify_input_lifecycle import State as LifeState, FIELDS as LIFE_FIELDS, KEYBOARD, ACQUIRED, CLOCK, DELAY, INTERVAL, EPOCH, QUEUE, CURSOR, HELD, EVENTS, TABLEA, TABLEB, COUNTERS, KO, KVT
from windows_target import ROOT, BUILD, verify_target, validation_symbols, validate_platform_imports
from windows_tracking import record_run
from build_decomp import build
from verify_surface_open import native_surface

if not __debug__:
    raise RuntimeError('Verification requires assertions')
DLL = BUILD / 'input_polling_validation.dll'
KEYCB, POLLCB, MAP = 0x50de14, 0x50e678, 0x4ba660
FIELDS = LIFE_FIELDS + [('g_inputKeyEventCallback', KEYCB, 4), ('g_inputPollCallback', POLLCB, 4)]
FUNCTIONS = {name: ARENA+0x3200+16*k for k,name in enumerate(('GetDeviceData','Acquire','KeyCallback','PollCallback'))}
TABLE = bytes.fromhex('ffff313233343536373839302d3dffff51574552545955494f507b7dffff4153444647484a4b4c3b27ffff5c5a584356424e4d2c2e2fff2aff20ffffffffffffffffffffffffff3738392d3435362b313233302e')
ROUTINES = {
 'Input_PollNative':(0x55ef0,376,(376,129,0,5150),'ac1bfc5738cf5a68eef9491b90049eb23453052f9350d545c82586248ec92f33'),
 'Input_DispatchKeyNative':(0x560c0,26,(26,0,2,0),'3b23ff815d34e56903eb4477a3cb227730eae66e254731b7902b62fed7d10a8b'),
 'Input_QueueCharacterNative':(0x560e0,59,(59,0,2,0),'12a599cf6a94f6b5f0e50c31159d04c6dab5785edbc0eadd208b3573c2139093'),
 'Input_TimerCallbackNative':(0x56160,19,(19,0,5,0),'2d1dd1ad615325f87eae45141b3a1d7f53a09f9d2646d1adcd17f93e16c76f58')}
INPUTS = ['decomp/src/geputget.c','decomp/include/geputget.h','decomp/src/main.c','decomp/include/main.h',
 'decomp/src/mem.c','decomp/include/mem.h','decomp/src/lisa3d.c','decomp/include/lisa3d.h','decomp/target.json',
 'tools/build_decomp.py','tools/verify_matching.py','tools/verify_input_polling.py','tools/verify_input_lifecycle.py',
 'tools/verify_surface_lifecycle.py','tools/windows_target.py','tools/windows_tracking.py',
 'tools/verify_surface_open.py','tools/native_surface_probe.c']

def signed(x):
    x &= 0xffffffff
    return x-0x100000000 if x&0x80000000 else x

class State(LifeState):
    def __init__(self, seed):
        super().__init__(seed)
        self.data[KEYCB]=bytearray(4);self.data[POLLCB]=bytearray(4)
        self.put(KEYCB,FUNCTIONS['KeyCallback']);self.put(POLLCB,FUNCTIONS['PollCallback'])
        self.put(KVT+0x28,FUNCTIONS['GetDeviceData']);self.put(KVT+0x1c,FUNCTIONS['Acquire'])
        self.put(CURSOR,QUEUE);self.put(DELAY,300);self.put(INTERVAL,30);self.put(EPOCH,100);self.put(CLOCK,120)
        self.data[HELD][:]=bytes(256);self.data[COUNTERS][:]=bytes(1024)
        self.records=[];self.count=0;self.reply=0;self.acquire_reply=0;self.mutations={};self.seen={};self.local_changes={}
    def snapshot(self):
        return tuple(bytes(self.data[p]) for p in [ARENA]+[p for _,p,_ in FIELDS])
    def read(self,a,tracked=True,n=4):
        b,o=self.region(a,n);x=int.from_bytes(b[o:o+n],'little')
        if tracked:self.events.append(('read',a,n,x))
        return x
    def boundary(self,name,args):
        self.events.append(('api',name,args,self.snapshot()))
        k=self.seen.get(name,0);self.seen[name]=k+1
        for a,x,n in self.mutations.get((name,k),[]):self.put(a,x,n)
        for index,word,value in self.local_changes.get((name,k),[]):
            if index==-1:self.count=value
            else:
                row=list(self.records[index]);row[word]=value;self.records[index]=tuple(row)
    def dispatch(self,args):
        if self.read(KEYCB):self.boundary('KeyCallback',tuple(args))
    def character(self,args):
        scan=signed(args[1])
        assert scan>=0,'Negative standalone index not certified'
        if scan>83:return
        value=TABLE[scan]
        self.write(self.read(CURSOR),value,1)
        self.write(self.read(CURSOR)+32,value,1)
        self.write(CURSOR,self.read(CURSOR)+1)
        if self.read(CURSOR)==QUEUE+32:self.write(CURSOR,QUEUE)
    def poll(self):
        for index in range(256):
            if self.read(HELD+index,n=1)==1:
                counter=self.read(COUNTERS+4*index);threshold=self.read(DELAY)
                counter-=self.read(EPOCH);threshold+=self.read(INTERVAL);counter+=self.read(CLOCK);counter&=0xffffffff
                self.write(COUNTERS+4*index,counter)
                if signed(counter)>=signed(threshold):
                    counter-=self.read(INTERVAL);self.write(EVENTS+index,1,1);self.write(COUNTERS+4*index,counter)
        keyboard=self.read(KEYBOARD);self.read(self.read(keyboard,False)+0x28,False)
        self.boundary('GetDeviceData',(keyboard,16,32,0))
        if self.reply==0x8007001e:
            self.write(ACQUIRED,0);keyboard=self.read(KEYBOARD);self.read(self.read(keyboard,False)+0x1c,False)
            self.boundary('Acquire',(keyboard,))
            if signed(self.acquire_reply)>=0:self.write(ACQUIRED,1)
            return
        if self.reply in (0,1):
            index=0
            while index<signed(self.count):
                offset,data,stamp,sequence=self.records[index]
                scan=offset&255
                if data&128:
                    self.write(HELD+scan,1,1)
                    if self.read(TABLEB+scan,n=1)==0:self.write(TABLEA+scan,1,1);self.write(TABLEB+scan,1,1)
                    if self.read(COUNTERS+4*scan)==0:self.write(EVENTS+scan,1,1)
                    self.dispatch((1,scan));self.character((1,self.records[index][0]&255))
                else:
                    for p in (HELD,TABLEA,TABLEB,EVENTS):self.write(p+scan,0,1)
                    self.write(COUNTERS+4*scan,0);self.dispatch((0,scan))
                index+=1
        self.write(EPOCH,self.read(CLOCK))
    def apply(self,name,args):
        self.events=[];self.fault=None;self.seen={}
        try:
            if name=='Input_DispatchKeyNative':self.dispatch(args)
            elif name=='Input_QueueCharacterNative':self.character(args)
            else:
                self.poll()
                if name=='Input_TimerCallbackNative' and self.read(POLLCB):self.boundary('PollCallback',())
        except Fault:pass

class Session(BaseSession):
    def __init__(self,pe,state,original):
        self.uc=Uc(UC_ARCH_X86,UC_MODE_32);self.base=pe.OPTIONAL_HEADER.ImageBase
        self.size=(pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095
        self.uc.mem_map(self.base,self.size);self.uc.mem_write(self.base,pe.get_memory_mapped_image())
        self.uc.mem_map(ARENA,SIZE);self.uc.mem_map(STACK,0x10000);self.uc.mem_map(STOP,4096)
        self.symbols=({name:0x400000+r for name,(r,*_) in ROUTINES.items()}|{name:p for name,p,_ in FIELDS}) if original else validation_symbols(pe)
        self.fields=[(p,self.symbols[name],n) for name,p,n in FIELDS]
        for p,b in state.data.items():
            raw=struct.pack('<I',self.physical(int.from_bytes(b,'little'))) if p==CURSOR else bytes(b)
            self.uc.mem_write(self.physical(p),raw)
        self.uc.hook_add(UC_HOOK_CODE,self.code);self.uc.hook_add(UC_HOOK_MEM_READ|UC_HOOK_MEM_WRITE,self.memory)
        self.uc.hook_add(UC_HOOK_MEM_INVALID,self.invalid)
    def snapshot(self):
        rows=[]
        for p,n in [(ARENA,SIZE)]+[(p,n) for _,p,n in FIELDS]:
            raw=bytes(self.uc.mem_read(self.physical(p),n))
            if p==CURSOR:raw=struct.pack('<I',self.normalize(int.from_bytes(raw,'little')))
            rows.append(raw)
        return tuple(rows)
    def memory(self,uc,access,a,n,x,data):
        length=len(self.events);super().memory(uc,access,a,n,x,data)
        if len(self.events)>length and self.normalize(a)==CURSOR:
            row=self.events[-1];self.events[-1]=row[:3]+(self.normalize(row[3]),)
    def code(self,uc,a,n,data):
        reverse={p:name for name,p in FUNCTIONS.items()}
        if a not in reverse:return
        name=reverse[a];argc={'GetDeviceData':5,'Acquire':1,'KeyCallback':2,'PollCallback':0}[name]
        sp=uc.reg_read(UC_X86_REG_ESP);raw=struct.unpack('<'+'I'*(argc+1),uc.mem_read(sp,4*(argc+1)))
        ret=raw[0];args=raw[1:]
        if name=='GetDeviceData':
            obj,width,buffer,count,flags=args
            assert width==16 and flags==0 and struct.unpack('<I',uc.mem_read(count,4))[0]==32
            assert STACK<=buffer<STACK+0x10000 and STACK<=count<STACK+0x10000
            if self.policy.records:uc.mem_write(buffer,b''.join(struct.pack('<4I',*row) for row in self.policy.records))
            uc.mem_write(count,struct.pack('<I',self.policy.count));self.buffer,self.count_address=buffer,count;args=(obj,width,32,flags)
        self.events.append(('api',name,tuple(args),self.snapshot()))
        k=self.policy.seen.get(name,0);self.policy.seen[name]=k+1
        for p,x,width in self.policy.mutations.get((name,k),[]):
            value=self.physical(x) if p==CURSOR else x
            uc.mem_write(self.physical(p),(value&((1<<(width*8))-1)).to_bytes(width,'little'))
        for index,word,value in self.policy.local_changes.get((name,k),[]):
            p=self.count_address if index==-1 else self.buffer+index*16+word*4
            uc.mem_write(p,struct.pack('<I',value))
        reply=self.policy.reply if name=='GetDeviceData' else self.policy.acquire_reply if name=='Acquire' else 0x12345678
        cleanup=argc*4 if name in ('GetDeviceData','Acquire') else 0
        for reg,x in ((UC_X86_REG_EAX,reply),(UC_X86_REG_ECX,0xdeadbeef),(UC_X86_REG_EDX,0x12345678),
          (UC_X86_REG_ESP,sp+4+cleanup),(UC_X86_REG_EIP,ret),(UC_X86_REG_EFLAGS,0x8d7)):uc.reg_write(reg,x)
    def run(self,name,args,state,seed,policy):
        self.policy=policy;self.events=[];self.fault=None;uc=self.uc;sp=STACK+0x8000
        uc.mem_write(sp,struct.pack('<'+'I'*(len(args)+1),STOP,*args));caller=bytes(uc.mem_read(sp,0x8000))
        saved=[random.Random(seed+k).getrandbits(32) for k in range(4)]
        for reg,x in zip(PRESERVED,saved):uc.reg_write(reg,x)
        uc.reg_write(UC_X86_REG_ESP,sp);uc.reg_write(UC_X86_REG_EFLAGS,2)
        before=bytearray(uc.mem_read(self.base,self.size))
        try:uc.emu_start(self.symbols[name],STOP,count=30000)
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
            assert uc.reg_read(UC_X86_REG_EIP)==STOP
            assert uc.reg_read(UC_X86_REG_ESP)==sp+4+(20 if name=='Input_TimerCallbackNative' else 0)
            assert [uc.reg_read(r) for r in PRESERVED]==saved and not uc.reg_read(UC_X86_REG_EFLAGS)&0x400

def verify_input_polling(record=True):
    original=pefile.PE(data=verify_target());build(dll=DLL);compiled=pefile.PE(str(DLL));validate_platform_imports(compiled)
    debug=next(d.struct for d in original.DIRECTORY_ENTRY_DEBUG if d.struct.Type==3)
    fpo={r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',original.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    for r,n,row,digest in ROUTINES.values():assert fpo[r]==row and hashlib.sha256(original.get_data(r,n)).hexdigest()==digest
    symbols=validation_symbols(compiled)
    assert original.get_data(MAP-0x400000,84)==TABLE==compiled.get_data(symbols['g_nativeCharacterMap']-compiled.OPTIONAL_HEADER.ImageBase,84)
    coverage={name:{'comparisons':0,'followups':0,'faults':0,'mutations':0} for name in ROUTINES}
    def sequence(state,names,args=()):
        sessions=[Session(pe,state,k==0) for k,pe in enumerate((original,compiled))]
        for k,name in enumerate(names):
            callargs=args if name in ('Input_DispatchKeyNative','Input_QueueCharacterNative') else (1,2,3,4,5) if name=='Input_TimerCallbackNative' else ()
            policy=copy.deepcopy(state);policy.seen={};state.apply(name,callargs)
            for session in sessions:session.run(name,callargs,state,coverage[name]['comparisons'],copy.deepcopy(policy))
            c=coverage[name];c['comparisons']+=1;c['followups']+=int(k>0);c['faults']+=int(state.fault is not None)
            c['mutations']+=int(any(rows and occurrence<state.seen.get(api,0)
                for changes in (state.mutations,state.local_changes) for (api,occurrence),rows in changes.items()))
            if state.fault:break
    for cursor in range(32):
        for scan in (0,1,2,15,16,28,30,57,71,83,84,255,0x7fffffff):
            state=State(cursor);state.put(CURSOR,QUEUE+cursor)
            sequence(state,('Input_QueueCharacterNative',)*2,(0x12345678,scan))
    for callback in (0,FUNCTIONS['KeyCallback']):
        for pressed,scan in ((0,0),(1,255),(0xffffffff,0x80000000),(7,83)):
            state=State(4);state.put(KEYCB,callback);sequence(state,('Input_DispatchKeyNative',)*2,(pressed,scan))
    for reply in (0,1,2,0x80004005,0x8007001e):
        for acquire in (0,1,0x80004005,0xffffffff):
            for n in (0,1,32,0xffffffff):
                state=State(n);state.reply=reply;state.acquire_reply=acquire;state.count=n
                state.records=[(k+0x100,128 if k%2 else 0,k,99) for k in range(32)]
                sequence(state,('Input_PollNative','Input_TimerCallbackNative','Input_PollNative'))
    for seed in range(16):
        state=State(seed);rng=random.Random(seed)
        for k in range(256):state.put(HELD+k,rng.choice((0,1,2,255)),1);state.put(COUNTERS+4*k,rng.getrandbits(32))
        for p in (DELAY,INTERVAL,EPOCH,CLOCK):state.put(p,rng.getrandbits(32))
        state.count=32;state.records=[(rng.getrandbits(32),rng.getrandbits(32),rng.getrandbits(32),rng.getrandbits(32)) for _ in range(32)]
        sequence(state,('Input_TimerCallbackNative','Input_PollNative','Input_TimerCallbackNative'))
    for api in FUNCTIONS:
        state=State(3);state.reply=0x8007001e if api=='Acquire' else 0;state.count=2
        state.records=[(30,128,0,0),(30,0,0,0)]
        state.mutations[api,0]=[(CLOCK,0xfedcba98,4),(CURSOR,QUEUE+31,4),(POLLCB,0,4),(KEYCB,0,4)]
        sequence(state,('Input_TimerCallbackNative',)*2)
    for bad in (0,0xdead0000):
        state=State(3);state.put(KEYBOARD,bad);sequence(state,('Input_PollNative',))
    for scan in (2,57,83,84,255,0x1234001e):
        state=State(5);state.count=2;state.records=[(30,128,0,0),(31,128,0,0)]
        state.local_changes['KeyCallback',0]=[(0,0,scan)]
        sequence(state,('Input_PollNative','Input_TimerCallbackNative'))
    for count in (0,1,2,0xffffffff):
        state=State(5);state.count=2;state.records=[(30,128,0,0),(31,128,0,0)]
        state.local_changes['KeyCallback',0]=[(-1,0,count)]
        sequence(state,('Input_PollNative','Input_TimerCallbackNative'))
    if record:
        for name,(r,*_) in ROUTINES.items():
            for kind in ('compilation','emulation'):
                record_run(r,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),cases=coverage[name]['comparisons'] if kind=='emulation' else 0,
                    command='uv run tools/verify_input_polling.py',
                    details={'scope':'Real four-routine chain, SDK/client boundaries modeled; no native event claim','coverage':coverage[name],
                     'limitations':'Negative character indices, SDK counts above32 and arbitrary reentry excluded; no instruction equality or native game proof'})
        native=native_surface(DLL,poll_test=True)
        for name,(r,*_) in ROUTINES.items():
            record_run(r,'native','pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),cases=3,
                command='uv run tools/verify_input_polling.py',details=native|{'routine':name})
        print('PASS native buffered input:',native['stdout'],flush=True)
    print('PASS buffered input:',coverage)
    return coverage

if __name__=='__main__':
    import sys
    verify_input_polling(record='--no-record' not in sys.argv)
