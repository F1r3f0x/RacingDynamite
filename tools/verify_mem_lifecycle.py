# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Lifecycle wrappers: isolated dependency contracts and real memory integration."""
import hashlib
import random
import struct
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX,
    UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EFLAGS)
from verify_mem_destroy import DestroySession, inspect_original as inspect_destroy
from verify_mem_free import PoolFixture, ARENA, ARENA_SIZE
from verify_mem_alloc import oracle as alloc_oracle
from verify_matching import BOOKKEEPING_FIELDS, PRESERVED, STACK, STOP
from build_decomp import build, STARTUP_EXPORTS
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')
ROUTINES = {
    'Input_ResetCallbacks': (0x55ab0,13,'c301c37754413d115bac22b922a9ed480b4080011c559a4485b7ea044270fd59'),
    'Mem_InitSystem': (0x5b170,41,'e4026b0791ef97b56f8fa7013dd568dfd4772d64e2638d850a217c9d338ce2b8'),
    'Mem_ShutdownSystem': (0x5b1a0,16,'75f6e3f589d50b5cb564113e1270c4f2a5df39c0614d41516b520d0d3ee064a3')}
DEPENDENCIES = {
    'Lisa_PrintVersion': (0x5b4f0,34,'719ae03224b3f4a4ab08fa52985cd65454730bd8de870f8afe632d9216951802'),
    'Gfx_InitPrimitiveState': (0x5e610,326,'fd2abfef167ef5b92074ef41c17478dd955591025afb3aad714da050a29704a3'),
    'Gfx_SelectBackend': (0x56af0,30,'d1fcb40ea924f27f8afef2a4b1e804ee70ce3fed2ff4f9e74a376877e7c3c4cd')}
MEMORY = {'Mem_InitHandles':(0x5b1f0,76), 'Mem_ShutdownHandles':(0x5b240,155),
    'Mem_NextHandleId':(0x5b1b0,58), 'Mem_RegisterHandle':(0x5b360,120),
    'Mem_ReleaseHandleId':(0x5b410,62)}
ARGUMENTS = {'Gfx_SelectBackend':1, 'Mem_CreatePool':1, 'Mem_DestroyPool':1,
    'Mem_RegisterHandle':1, 'Mem_ReleaseHandleId':1, 'Mem_Alloc':2, 'Mem_Free':2,
    'malloc':1, 'free':1, 'callback':1}
CALLBACK = 0x7200000
DLL = BUILD / 'mem_lifecycle_validation.dll'
RESOURCE_FIELDS = [('g_inputKeyEventCallback',0x50de14,4), ('g_inputPollCallback',0x50e678,4)]
STATE_FIELDS = BOOKKEEPING_FIELDS + RESOURCE_FIELDS
INPUTS = ['decomp/src/geputget.c','decomp/include/geputget.h','decomp/src/mem.c','decomp/include/mem.h','decomp/target.json',
    'tools/verify_mem_lifecycle.py','tools/verify_mem_destroy.py','tools/verify_mem_pools.py',
    'tools/verify_mem_alloc.py','tools/verify_mem_free.py','tools/verify_matching.py',
    'tools/build_decomp.py','tools/verify_font_cleanup.py','tools/windows_target.py',
    'tools/windows_tracking.py']


def record(name,kind,outcome,cases=0,**details):
    record_run(ROUTINES[name][0],kind,outcome,inputs=INPUTS,
        artifact=DLL.relative_to(ROOT).as_posix() if DLL.exists() else None,
        cases=cases,command=('uv run python tools/verify_mem_lifecycle.py'
            if __name__=='__main__' else 'uv run tools/verify_matching.py'),details=details)


def inspect_original(pe):
    inspect_destroy(pe)
    debug=next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type==3)
    fpo={r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    for name,(r,n,digest) in (ROUTINES|DEPENDENCIES).items():
        raw=pe.get_data(r,n);ins=list(md.disasm(raw,0x400000+r))
        assert hashlib.sha256(raw).hexdigest()==digest
        assert sum(i.size for i in ins)==n
        assert fpo[r]==(n,0,1 if name=='Gfx_SelectBackend' else 0,0)
        assert ins[-1].mnemonic==('jmp' if name=='Gfx_InitPrimitiveState' else 'ret')
    for name,(r,n,_) in ROUTINES.items():
        if name=='Input_ResetCallbacks':continue
        ins=list(md.disasm(pe.get_data(r,n),0x400000+r))
        expected=[(0x45b170,0x45b4f0),(0x45b175,0x45ad50),(0x45b17a,0x45b1f0),
            (0x45b17f,0x455ab0),(0x45b184,0x45e610),(0x45b18b,0x456af0)] if name=='Mem_InitSystem' else [(0x45b1a0,0x45b240),(0x45b1a5,0x45b140)]
        assert [(i.address,int(i.op_str,16)) for i in ins if i.mnemonic=='call']==expected
        assert len(ins)==(10 if name=='Mem_InitSystem' else 4)
        assert not any(e.type==3 and r<=e.rva<r+n for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries)
    # Independently authenticate consumer ABI/ownership, not inherited names.
    for r,n,digest in [(0x560c0,26,'3b23ff815d34e56903eb4477a3cb227730eae66e254731b7902b62fed7d10a8b'),
            (0x56160,19,'2d1dd1ad615325f87eae45141b3a1d7f53a09f9d2646d1adcd17f93e16c76f58'),
            (0x55ef0,376,'ac1bfc5738cf5a68eef9491b90049eb23453052f9350d545c82586248ec92f33'),
            (0x55c60,436,'6ade6c95c919cb9ea6d550e552623c057dbefcdef7b325d552d883a7ad782efe')]:
        assert hashlib.sha256(pe.get_data(r,n)).hexdigest()==digest and fpo[r][0]==n
    assert fpo[0x560c0]==(26,0,2,0) and fpo[0x56160]==(19,0,5,0)
    for va,expected in [(0x50de14,[0x55ab3,0x560c2]),
            (0x50e678,[0x55ab8,0x55f4a,0x56166])]:
        assert [e.rva for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries
            if e.type==3 and struct.unpack('<I',pe.get_data(e.rva,4))[0]==va]==expected
    assert any(s.dll==b'WINMM.dll' and i.name==b'timeSetEvent' and i.address==0x64c474
        for s in pe.DIRECTORY_ENTRY_IMPORT for i in s.imports)
    assert pe.get_data(0x55abd,3)==b'\xcc'*3
    relocs=[(e.rva,struct.unpack('<I',pe.get_data(e.rva,4))[0])
        for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries
        if e.type==3 and 0x55ab0<=e.rva<0x55abd]
    assert relocs==[(0x55ab3,0x50de14),(0x55ab8,0x50e678)]
    data=next(s for s in pe.sections if s.Name.rstrip(b'\0')==b'.data')
    for _,va,n in RESOURCE_FIELDS:
        assert data.VirtualAddress+data.SizeOfRawData<=va-0x400000
        assert va-0x400000+n<=data.VirtualAddress+data.Misc_VirtualSize
    assert pe.get_data(0x5b199,7)==b'\xcc'*7
    assert pe.get_data(0x5b1b0,1)==b'\x83'  # Next entry directly follows shutdown RET.
    for site,target in [(0x412170,0x45b170),(0x4122a7,0x45b1a0),(0x412502,0x456af0)]:
        raw=pe.get_data(site-0x400000,5)
        assert raw[0]==0xe8 and site+5+struct.unpack_from('<i',raw,1)[0]==target
    for target,site in [(0x45b170,0x412170),(0x45b1a0,0x4122a7)]:
        assert not any(e.type==3 and struct.unpack('<I',pe.get_data(e.rva,4))[0]==target
            for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries)
        calls=[i.address for r,(n,_,_,_) in fpo.items()
            for i in md.disasm(pe.get_data(r,n),0x400000+r)
            if i.mnemonic=='call' and i.op_str==hex(target)]
        assert calls==[site]  # Bounded FPO scan, not a whole-program absence proof.
    for r,n in MEMORY.values():assert fpo[r][0]==n


class Oracle:
    """Independent instruction-derived effects; explicit arbitrary boundary models."""
    def __init__(self,state,isolated=False,actions=None,returns=None):
        self.state={k:bytearray(v) for k,v in state.items()}
        self.isolated=isolated;self.actions=actions or {};self.returns=returns or {}
        self.events=[];self.boundaries=[];self.indices={};self.top=None

    def snapshot(self):return {k:bytes(v) for k,v in self.state.items()}

    def read(self,name,off=0,n=4):
        self.events.append(('read',name,off,n))
        return int.from_bytes(self.state[name][off:off+n],'little')

    def write(self,name,off,value,n=4):
        self.events.append(('write',name,off,n,value))
        self.state[name][off:off+n]=value.to_bytes(n,'little')

    def boundary(self,name,args):
        self.boundaries.append((name,args,self.snapshot()))
        self.events.append(('boundary',name,args))
        index=self.indices.get(name,0);self.indices[name]=index+1
        for field,off,n,value in self.actions.get((name,index),()):
            self.state[field][off:off+n]=value.to_bytes(n,'little')
        values=self.returns.get(name,())
        return values[index] if index<len(values) else 0xdeadbeef

    def call(self,name,args=()):
        if name!=self.top:self.events.append(('call',name,args))
        if name in STARTUP_EXPORTS or (self.isolated and name in ('Input_ResetCallbacks',
                'Mem_InitPools','Mem_InitHandles','Mem_ShutdownHandles','Mem_ShutdownPools')):
            return self.boundary(name,args)
        if name=='Input_ResetCallbacks':
            self.write('g_inputKeyEventCallback',0,0);self.write('g_inputPollCallback',0,0);return None
        if name=='Mem_InitSystem':
            for dep in ['Lisa_PrintVersion','Mem_InitPools','Mem_InitHandles',
                        'Input_ResetCallbacks','Gfx_InitPrimitiveState','Gfx_SelectBackend']:
                self.call(dep,(0,) if dep=='Gfx_SelectBackend' else ())
            return 1
        if name=='Mem_ShutdownSystem':
            self.call('Mem_ShutdownHandles');self.call('Mem_ShutdownPools');return 1
        if name=='Mem_InitPools':
            for i in range(256):self.write('table',4*i,0)
            self.call('Mem_CreatePool',('DEFAULT',));return 1
        if name=='Mem_CreatePool':
            slot=next((i for i in range(256) if self.read('table',4*i)==0),256)
            if slot==256:return 0xffffffff
            pointer=self.call('malloc',(320,))
            if pointer==0:return 0xffffffff
            self.write('table',4*slot,pointer);off=pointer-ARENA
            for i,value in enumerate(b'DEFAULT'):
                self.events += [('read','name',i,1),('read','name',i,1)]
                self.write('arena',off+i,value,1)
            self.events.append(('read','name',7,1));self.write('arena',off+7,0,1)
            for i in range(64):self.write('arena',off+64+4*i,0)
            return slot
        if name=='Mem_InitHandles':
            flag=self.read('g_memHandlesInitialized')
            if flag==1:return 1
            self.write('g_memHandlesInitialized',0,1)
            for i in range(200):self.write('g_memHandleStatus',4*i,0)
            for i in range(200):self.write('g_memHandleIds',2*i,i+1,2)
            self.write('g_memHandleCursor',0,0,2);return 1
        if name=='Mem_ShutdownHandles':
            if self.read('g_memHandlesInitialized')==0:return 1
            self.write('g_memHandlesInitialized',0,0)
            for flag in (0x10000,0x20000):
                for i in range(200):
                    if self.read('g_memHandleStatus',4*i)==1 and self.read('g_memHandleFlags',4*i)==flag:
                        self.write('g_memHandleStatus',4*i,0)
                        param=self.read('g_memHandleParameters',4*i)
                        assert self.read('g_memHandleCallbacks',4*i)==CALLBACK
                        self.call('callback',(param,))
            return 1
        if name=='Mem_ShutdownPools':
            for i in range(256):
                if self.read('table',4*i):self.call('Mem_DestroyPool',(i,))
            return 1
        if name=='Mem_DestroyPool':
            slot=args[0];pool=self.read('table',4*slot)-ARENA
            for p in range(64):
                page=self.read('arena',pool+64+4*p)
                if not page:continue
                for b in range(64):
                    block=self.read('arena',page-ARENA+4*b)
                    if not block:continue
                    for r in range(16):
                        off=block-ARENA+8*r
                        if self.read('arena',off+4):self.call('free',(self.read('arena',off),))
                    self.call('free',(block,))
                self.call('free',(page,))
            self.call('free',(ARENA+pool,));self.write('table',4*slot,0);return 1
        if name=='Mem_RegisterHandle':
            if self.read('g_memHandlesInitialized')==0:return 0
            for i in range(200):
                if self.read('g_memHandleStatus',4*i)==0:
                    context=self.read('g_memPendingContext')
                    self.write('g_memHandleStatus',4*i,1);self.write('g_memHandleFlags',4*i,0x10000)
                    self.write('g_memHandleContexts',4*i,context);self.write('g_memRegisteredHandleIds',4*i,args[0])
                    self.write('g_memHandleParameters',4*i,self.read('g_memPendingParameter'))
                    self.write('g_memHandleCallbacks',4*i,self.read('g_memPendingCallback'));return i
            return 0xffffffff
        if name=='Mem_Alloc':
            values=self.returns.get('malloc',())
            result,final,events,snapshots,requests=alloc_oracle(self.state['arena'],args[0],args[1],values,{})
            for event in events:
                kind,*rest=event
                if kind=='table':self.read('table',rest[0],rest[1])
                elif kind=='read':self.read('arena',*rest)
                elif kind=='write':
                    off,n,value=rest;self.write('arena',off,value,n)
                elif kind=='malloc':
                    size,pointer=rest;assert self.call('malloc',(size,))==pointer
                else:raise AssertionError(event)
            assert bytes(self.state['arena'])==final;return result
        if name in ('malloc','free','callback'):return self.boundary(name,args)
        raise AssertionError('Unmodeled routine '+name)

    def invoke(self,name,args=()):
        self.top=name;result=self.call(name,args)
        return result,self.snapshot(),self.events,self.boundaries


class LifecycleSession(DestroySession):
    def __init__(self,pe,symbols=None):
        super().__init__(pe,symbols)
        self.fields=[(name,va if symbols is None else symbols[name],n) for name,va,n in STATE_FIELDS]
        self.entries.update({name:0x400000+r if symbols is None else symbols[name]
            for name,(r,_,_) in (ROUTINES|DEPENDENCIES).items()})
        self.entries.update({name:0x400000+r if symbols is None else symbols[name] for name,(r,n) in MEMORY.items()})
        self.entries.update(malloc=self.malloc,free=self.free,callback=CALLBACK)
        if symbols is None:self.ranges.extend((0x400000+r,n) for r,n in MEMORY.values())
        self.ranges.extend((0x400000+r,n) for r,n,_ in ROUTINES.values()) if symbols is None else None
        self.cpu.mem_map(CALLBACK,0x1000)
        self.by_address={addr:name for name,addr in self.entries.items() if name in
            set(ROUTINES)|set(DEPENDENCIES)|set(MEMORY)|{'Mem_InitPools','Mem_CreatePool',
            'Mem_ShutdownPools','Mem_DestroyPool','Mem_Alloc','Mem_Free','malloc','free','callback'}}

    def state(self):
        return {'table':bytes(self.cpu.mem_read(self.table,1024)),
            'arena':bytes(self.cpu.mem_read(ARENA,ARENA_SIZE)),
            **{name:bytes(self.cpu.mem_read(addr,n)) for name,addr,n in self.fields}}

    def reset_state(self,state):
        self.reset(state['table'],state['arena'])
        for name,addr,n in self.fields:self.cpu.mem_write(addr,state[name])

    def region(self,address,n):
        for name,addr,length in self.fields:
            if addr<=address and address+n<=addr+length:return name,address-addr
        if self.default<=address and address+n<=self.default+8:return 'name',address-self.default
        if self.table<=address and address+n<=self.table+1024:return 'table',address-self.table
        if ARENA<=address and address+n<=ARENA+ARENA_SIZE:return 'arena',address-ARENA
        if STACK<=address and address+n<=STACK+0x10000:return None
        raise AssertionError('Untracked data access %x'%address)

    def read(self,cpu,access,address,n,value,unused):
        region=self.region(address,n)
        if region:self.events.append(('read',*region,n))

    def write(self,cpu,access,address,n,value,unused):
        region=self.region(address,n)
        if region:self.events.append(('write',*region,n,value))

    def hook(self,cpu,address,n,unused):
        name=self.by_address.get(address)
        if name:
            sp=cpu.reg_read(UC_X86_REG_ESP);argc=ARGUMENTS.get(name,0)
            args=struct.unpack('<'+'I'*argc,cpu.mem_read(sp+4,argc*4)) if argc else ()
            if name=='Mem_CreatePool':
                assert args==(self.default,);args=('DEFAULT',)
            if name!=self.top:self.events.append(('call',name,args))
            modeled=name in STARTUP_EXPORTS or name in ('malloc','free','callback') or (
                self.isolated and name in ('Input_ResetCallbacks','Mem_InitPools','Mem_InitHandles','Mem_ShutdownHandles','Mem_ShutdownPools'))
            if modeled:
                assert self.boundary_index<len(self.expected_boundaries),'Unexpected boundary'
                expected_name,expected_args,state=self.expected_boundaries[self.boundary_index]
                assert (name,args)==(expected_name,expected_args)
                assert self.state()==state,'Boundary-entry state mismatch '+name
                self.boundary_index+=1;self.events.append(('boundary',name,args))
                index=self.indices.get(name,0);self.indices[name]=index+1
                addresses={key:addr for key,addr,length in self.fields}|{'table':self.table,'arena':ARENA}
                for field,off,length,value in self.actions.get((name,index),()):
                    cpu.mem_write(addresses[field]+off,value.to_bytes(length,'little'))
                values=self.returns.get(name,());result=values[index] if index<len(values) else 0xdeadbeef
                cpu.reg_write(UC_X86_REG_EAX,result);cpu.reg_write(UC_X86_REG_ECX,0xc1c1c1c1)
                cpu.reg_write(UC_X86_REG_EDX,0xd2d2d2d2);cpu.reg_write(UC_X86_REG_EFLAGS,0x43)
                cpu.reg_write(UC_X86_REG_EIP,struct.unpack('<I',cpu.mem_read(sp,4))[0])
                cpu.reg_write(UC_X86_REG_ESP,sp+4);return
        assert any(a<=address<a+length for a,length in self.ranges),'Escaped authenticated memory bodies'

    def invoke_case(self,name,args,expected,isolated=False,actions=None,returns=None,seed=0):
        eax,state,events,self.expected_boundaries=expected
        self.top=name;self.isolated=isolated;self.actions=actions or {};self.returns=returns or {}
        self.events=[];self.indices={};self.boundary_index=0
        cpu=self.cpu;sp=STACK+0x8000;cpu.mem_write(sp,struct.pack('<'+'I'*(1+len(args)),STOP,*args))
        stack=bytes(cpu.mem_read(sp,0x8000));rng=random.Random(seed)
        saved=[rng.getrandbits(32) for _ in PRESERVED]
        for reg,value in zip(PRESERVED,saved):cpu.reg_write(reg,value)
        cpu.reg_write(UC_X86_REG_EAX,rng.getrandbits(32));cpu.reg_write(UC_X86_REG_ESP,sp)
        cpu.reg_write(UC_X86_REG_EFLAGS,2);before=bytes(cpu.mem_read(self.base,self.image_size))
        cpu.emu_start(self.entries[name],STOP,count=5000000)
        assert cpu.reg_read(UC_X86_REG_EIP)==STOP and cpu.reg_read(UC_X86_REG_ESP)==sp+4
        assert [cpu.reg_read(r) for r in PRESERVED]==saved
        assert bytes(cpu.mem_read(sp,0x8000))==stack and not cpu.reg_read(UC_X86_REG_EFLAGS)&0x400
        after=bytearray(cpu.mem_read(self.base,self.image_size))
        for key,addr,length in self.fields+[('table',self.table,1024)]:
            off=addr-self.base;after[off:off+length]=before[off:off+length]
        assert bytes(after)==before,'Unrelated image changed'
        if self.events!=events:
            k=next((i for i,(a,b) in enumerate(zip(self.events,events)) if a!=b),min(len(self.events),len(events)))
            raise AssertionError('Event mismatch %d: %s != %s; counts %d/%d'%(k,self.events[k:k+3],events[k:k+3],len(self.events),len(events)))
        assert self.boundary_index==len(self.expected_boundaries)
        assert self.state()==state
        if eax is not None:assert cpu.reg_read(UC_X86_REG_EAX)==eax
        return state


def verify_mem_lifecycle():
    verify_target();phase='compilation';counts={name:0 for name in ROUTINES}
    try:
        original=pefile.PE(str(TARGET));inspect_original(original);build(dll=DLL)
        for name in ROUTINES:record(name,'compilation','pass',scope='Complete production mem.c plus extracted production Input_ResetCallbacks declarations/body; strict C89; provisional Clang/LLD; explicit nonreturning startup and CRT fixtures')
        rebuilt=pefile.PE(str(DLL));assert not hasattr(rebuilt,'DIRECTORY_ENTRY_IMPORT')
        symbols={s.name.decode():rebuilt.OPTIONAL_HEADER.ImageBase+s.address for s in rebuilt.DIRECTORY_ENTRY_EXPORT.symbols if s.name}
        for name,(r,n,_) in ROUTINES.items():
            equal=rebuilt.get_data(symbols[name]-rebuilt.OPTIONAL_HEADER.ImageBase,n)==original.get_data(r,n)
            record(name,'raw_bytes','pass' if equal else 'different',scope='Raw prefix diagnostic only; instruction equality unverified')
        a,b=LifecycleSession(original),LifecycleSession(rebuilt,symbols);phase='emulation'
        rng=random.Random(0x5b170);isolated_counts={name:0 for name in ROUTINES};persistent=0
        def fresh(flag=0):
            state={name:rng.randbytes(n) for name,_,n in STATE_FIELDS}
            state.update(table=bytes(1024),arena=bytes(PoolFixture().data))
            state['g_memHandlesInitialized']=struct.pack('<I',flag)
            state['g_memHandleStatus']=bytes(800)
            return state
        def compare(name,state,args=(),isolated=False,actions=None,returns=None,reset=True):
            expected=Oracle(state,isolated,actions,returns).invoke(name,args)
            if reset:
                for s in (a,b):s.reset_state(state)
            outputs=[s.invoke_case(name,args,expected,isolated,actions,returns,sum(counts.values())) for s in (a,b)]
            assert outputs[0]==outputs[1]
            if name in counts:counts[name]+=1
            if isolated:isolated_counts[name]+=1
            return outputs[0]
        # Standalone reset covers arbitrary raw pointer words and repeated clearing.
        for value in [0,1,2,0x80000000,0xffffffff]+[rng.getrandbits(32) for _ in range(128)]:
            state=fresh();state['g_inputKeyEventCallback']=struct.pack('<I',value)
            state['g_inputPollCallback']=struct.pack('<I',value^0xffffffff)
            state=compare('Input_ResetCallbacks',state)
            compare('Input_ResetCallbacks',state,reset=False)
        # Every dependency receives all representative failure/high-bit EAX values.
        # Models mutate tracked state independently; no helper result short-circuits.
        for name,deps in [('Mem_InitSystem',['Lisa_PrintVersion','Mem_InitPools','Mem_InitHandles',
                'Input_ResetCallbacks','Gfx_InitPrimitiveState','Gfx_SelectBackend']),
                ('Mem_ShutdownSystem',['Mem_ShutdownHandles','Mem_ShutdownPools'])]:
            for dep in deps:
                for value in [0,1,2,0x80000000,0xffffffff]:
                    actions={(d,0):[('g_memPendingContext',0,4,rng.getrandbits(32)),
                        ('arena',0xff000+i*4,4,rng.getrandbits(32))] for i,d in enumerate(deps)}
                    compare(name,fresh(rng.getrandbits(32)),isolated=True,actions=actions,returns={dep:[value]})
            for _ in range(64):
                compare(name,fresh(rng.getrandbits(32)),isolated=True,
                    returns={d:[rng.getrandbits(32)] for d in deps})
        # Real init: flag exactly one skips handle reset; other values reset.
        for flag in [0,1,2,0x80000000,0xffffffff]:
            for pointer in [0,ARENA]:
                state=fresh(flag);state['table']=rng.randbytes(1024)
                compare('Mem_InitSystem',state,returns={'malloc':[pointer]})
        # Changes made by early/late startup boundaries are observed by real helpers.
        for flag in [0,1,2,0xffffffff]:
            actions={
                ('Lisa_PrintVersion',0):[('g_memHandlesInitialized',0,4,flag),
                    ('g_inputKeyEventCallback',0,4,0xffffffff),('g_inputPollCallback',0,4,0x12345678)],
                ('Gfx_InitPrimitiveState',0):[('g_memPendingCallback',0,4,CALLBACK)],
                ('Gfx_SelectBackend',0):[('g_memHandlesInitialized',0,4,0)]}
            compare('Mem_InitSystem',fresh(2),actions=actions,returns={'malloc':[ARENA]})
        # Every handle slot in both passes; callbacks precede any pool destruction.
        for i in range(200):
            for flag in [0x10000,0x20000]:
                state=fresh(1);f=PoolFixture();block=f.block(63,63)
                for r in range(16):f.word(block+8*r+4,0)
                f.match(63,63,15,pointer=0xffffffff,size=1);state['arena']=bytes(f.data)
                table=bytearray(1024);struct.pack_into('<I',table,(i%256)*4,ARENA);state['table']=bytes(table)
                for key,value in [('g_memHandleStatus',1),('g_memHandleFlags',flag),
                        ('g_memHandleCallbacks',CALLBACK),('g_memHandleParameters',i)]:
                    data=bytearray(state[key]);struct.pack_into('<I',data,4*i,value);state[key]=bytes(data)
                compare('Mem_ShutdownSystem',state)
        for flag in [0,1,2,0x80000000,0xffffffff]:
            for _ in range(8):
                state=fresh(flag);status=[rng.choice([0,1,2,0xffffffff]) for i in range(200)]
                state['g_memHandleStatus']=struct.pack('<200I',*status)
                state['g_memHandleFlags']=struct.pack('<200I',*[rng.choice([0,0x10000,0x20000,0x30000]) for i in range(200)])
                state['g_memHandleCallbacks']=struct.pack('<200I',*([CALLBACK]*200))
                compare('Mem_ShutdownSystem',state)
        # Cross-phase mutations: callback inserts/removes pools, schedules a later
        # second-pass callback, and CRT changes handles after their scan has ended.
        state=fresh(1);state['table']=struct.pack('<256I',ARENA,*([0]*255))
        for key,value in [('g_memHandleStatus',1),('g_memHandleFlags',0x10000),('g_memHandleCallbacks',CALLBACK)]:
            data=bytearray(state[key]);struct.pack_into('<I',data,0,value);state[key]=bytes(data)
        actions={('callback',0):[('table',0,4,0),('table',1020,4,ARENA+0x2000),
            ('g_memHandleStatus',796,4,1),('g_memHandleFlags',796,4,0x20000),
            ('g_memHandleCallbacks',796,4,CALLBACK)],
            ('free',0):[('g_memHandlesInitialized',0,4,1),('g_memHandleStatus',0,4,1)]}
        compare('Mem_ShutdownSystem',state,actions=actions)
        # Persistent startup, real allocation/registration, callback -> pool teardown,
        # repeated shutdown, default failure, and repeated startup orphaning.
        state=fresh(0)
        for s in (a,b):s.reset_state(state)
        def step(name,args=(),actions=None,returns=None):
            nonlocal state,persistent
            state=compare(name,state,args,actions=actions,returns=returns,reset=False);persistent+=1
        step('Mem_InitSystem',returns={'malloc':[ARENA]})
        step('Mem_Alloc',(0,123),returns={'malloc':[ARENA+0xc0000,ARENA+0xc1000,ARENA+0xc2000]})
        for key,value in [('g_memPendingCallback',CALLBACK),('g_memPendingParameter',0xffffffff)]:
            state[key]=struct.pack('<I',value)
        for s in (a,b):s.reset_state(state)  # Explicit client setup, same in both CPUs.
        step('Mem_RegisterHandle',(42,))
        step('Mem_ShutdownSystem',actions={('callback',0):[('g_memPendingContext',0,4,0x12345678)]})
        step('Mem_ShutdownSystem')
        step('Mem_InitSystem',returns={'malloc':[0]})
        step('Mem_ShutdownSystem')
        step('Mem_InitSystem',returns={'malloc':[ARENA+0x2000]})
        step('Mem_InitSystem',returns={'malloc':[ARENA]})
        step('Mem_ShutdownSystem')
        for name in ROUTINES:
            record(name,'emulation','pass',cases=counts[name],isolated_cases=isolated_counts[name],
                persistent_invocations=persistent if name!='Input_ResetCallbacks' else 0,
                integrated_startups=18 if name=='Input_ResetCallbacks' else None,
                scope=('266 standalone ordered two-dword clearing calls, repeated persistent clearing; 18 integrated startups; complete tracked state and boundary snapshots; nonvolatile registers/stack/DF/unrelated-image checks; void EAX excluded' if name=='Input_ResetCallbacks' else 'Exact ordered calls/arguments and accesses; complete handles, roots, one-MiB heap and boundary-entry snapshots; ABI/stack/unrelated-image checks; real pool/handle initialization and teardown; 400 callback slot/pass cases, cross-phase mutation and persistent real allocation/registration'),
                limitations='Three startup dependencies, CRT heap and callbacks modeled; no instruction equality, original linked layout, native startup/graphics/heap/game parity, invalid/aliased storage, concurrency or general reentry validation')
        print('PASS: %d Input_ResetCallbacks; %d Mem_InitSystem and %d Mem_ShutdownSystem differential invocations; isolated %s; %d persistent invocations.'%(counts['Input_ResetCallbacks'],counts['Mem_InitSystem'],counts['Mem_ShutdownSystem'],isolated_counts,persistent))
        return counts
    except Exception as exc:
        for name in ROUTINES:record(name,phase,'fail',error=str(exc))
        raise


if __name__=='__main__':
    verify_mem_lifecycle()
