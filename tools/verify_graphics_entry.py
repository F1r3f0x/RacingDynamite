# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Authentic graphics entry dispatch plus real surface dependency execution."""
import copy
import hashlib
import random
import struct
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_INVALID
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EFLAGS
from verify_matching import STACK, STOP, PRESERVED
from verify_surface_open import Session as OpenSession, native_surface, Fault
from verify_surface_rebuild import State as RebuildState, FIELDS as REBUILD_FIELDS, INPUTS as REBUILD_INPUTS
from verify_surface_shutdown import State as ShutdownState
from windows_target import ROOT, BUILD, verify_target, validation_symbols
from build_decomp import build
from windows_tracking import record_run

if not __debug__:raise RuntimeError('Verification requires assertions')
DLL=BUILD/'graphics_entry_validation.dll'
INPUTS=REBUILD_INPUTS+['tools/verify_surface_shutdown.py','tools/verify_graphics_entry.py']
ROUTINES={
 'Gfx_Open':(0x56bc0,19,'52be6802ced83d2ed0f4d075befb6cb8a27c422283385d5948efbf9807fee826'),
 'Gfx_Rebuild':(0x56be0,6,'38113c56bd5b8bad3c416585d5886f408e4c6e8aa5712adc29541c69099eaac4'),
 'Gfx_Shutdown':(0x56bf0,19,'12f76bae9f55a6539d23baf465bf52e0b2450cf8cc41121eb16f100448679412'),
 'Gfx_SpriteOpenNative':(0x56f20,6,'2db31f4e09597946e56e859813bfdad7046d457c32332c3a882654d1e3ffdf67'),
 'Gfx_SpriteShutdownNative':(0x56f30,6,'2db31f4e09597946e56e859813bfdad7046d457c32332c3a882654d1e3ffdf67')}
SLOTS=[('g_surfaceOpen',0x50eb6c,4),('g_surfaceRebuild',0x50eb70,4),('g_surfaceShutdown',0x50eb74,4),
 ('g_spriteOpen',0x50eba0,4),('g_spriteShutdown',0x50eba4,4)]
TARGETS={'Gfx_SurfaceOpenNative':0x45b740,'Gfx_SurfaceRebuildNative':0x45bd70,'Gfx_SurfaceShutdownNative':0x45c060,
 'Gfx_SpriteOpenNative':0x456f20,'Gfx_SpriteShutdownNative':0x456f30}
FIELDS=REBUILD_FIELDS+SLOTS

def inspect(pe):
    debug=next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type==3)
    fpo={r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    for r,n,digest in ROUTINES.values():
        assert fpo[r]==(n,0,0,0)
        assert hashlib.sha256(pe.get_data(r,n)).hexdigest()==digest

def standalone(pe,original,name,reply,tail_reply,change=False,bad=None):
    uc=Uc(UC_ARCH_X86,UC_MODE_32);base=pe.OPTIONAL_HEADER.ImageBase
    size=(pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095
    uc.mem_map(base,size);uc.mem_write(base,pe.get_memory_mapped_image())
    uc.mem_map(STACK,0x10000);uc.mem_map(STOP,4096)
    symbols={n:0x400000+r for n,(r,*_) in ROUTINES.items()}|{n:p for n,p,_ in SLOTS} if original else validation_symbols(pe)
    entry=symbols[name]
    first,second={'Gfx_Open':('g_surfaceOpen','g_spriteOpen'),'Gfx_Rebuild':('g_surfaceRebuild',None),
      'Gfx_Shutdown':('g_surfaceShutdown','g_spriteShutdown')}.get(name,(None,None))
    def put(p,x):uc.mem_write(p,struct.pack('<I',x))
    if first:put(symbols[first],bad if bad is not None else STOP+16)
    if second:put(symbols[second],STOP+32)
    events=[];fault=[]
    def callback(uc,a,n,data):
        if a not in (STOP+16,STOP+32,STOP+48):return
        sp=uc.reg_read(UC_X86_REG_ESP);ret=struct.unpack('<I',uc.mem_read(sp,4))[0]
        events.append(a-STOP)
        if a==STOP+16 and second and change:put(symbols[second],STOP+48 if change==1 else 0x80000000)
        for reg,x in ((UC_X86_REG_EAX,reply if a==STOP+16 else tail_reply),(UC_X86_REG_ECX,0xdeadbeef),
          (UC_X86_REG_EDX,0x12345678),(UC_X86_REG_ESP,sp+4),(UC_X86_REG_EIP,ret),(UC_X86_REG_EFLAGS,0x8d7)):uc.reg_write(reg,x)
    def invalid(uc,access,a,n,x,data):fault.append((access,a,n));return False
    uc.hook_add(UC_HOOK_CODE,callback);uc.hook_add(UC_HOOK_MEM_INVALID,invalid)
    sp=STACK+0x8000;put(sp,STOP);caller=bytes(uc.mem_read(sp,0x8000))
    saved=[random.Random(reply+k).getrandbits(32) for k in range(4)]
    for reg,x in zip(PRESERVED,saved):uc.reg_write(reg,x)
    uc.reg_write(UC_X86_REG_ESP,sp);uc.reg_write(UC_X86_REG_EFLAGS,2)
    before=bytearray(uc.mem_read(base,size))
    try:uc.emu_start(entry,STOP,count=100)
    except UcError:assert fault
    expected=([16]+([48 if change else 32] if second and reply and change!=2 else [])) if first and bad is None else []
    assert events==expected,(name,events,expected)
    assert bool(fault)==bool(bad is not None or second and reply and change==2)
    after=bytearray(uc.mem_read(base,size))
    if second:before[symbols[second]-base:symbols[second]-base+4]=after[symbols[second]-base:symbols[second]-base+4]
    assert before==after and bytes(uc.mem_read(sp,0x8000))==caller
    if not fault:
        expected_result=1 if not first else tail_reply if second and reply else reply if not second else 0
        assert uc.reg_read(UC_X86_REG_EAX)==expected_result
        assert uc.reg_read(UC_X86_REG_ESP)==sp+4 and uc.reg_read(UC_X86_REG_EIP)==STOP
        assert [uc.reg_read(r) for r in PRESERVED]==saved and not uc.reg_read(UC_X86_REG_EFLAGS)&0x400
    return bool(fault)

class State(RebuildState):
    def __init__(self,seed,mode=0,count=1):
        super().__init__(seed,mode,count)
        for (_,p,_),x in zip(SLOTS,TARGETS.values()):self.data[p]=bytearray(struct.pack('<I',x))
    def snapshot(self):return tuple(bytes(self.data[p]) for p in [0x3000000]+[p for _,p,_ in FIELDS])
    def apply_entry(self,name):
        self.events=[];self.fault=None;self.seen={};self.surface_number=0
        try:return self.entry(name)
        except Fault:return None
    def entry(self,name):
        if name=='Gfx_Open':
            assert self.read(0x50eb6c)==0x45b740
            result=self.constructor()
            if result:assert self.read(0x50eba0)==0x456f20
            return int(bool(result))
        if name=='Gfx_Rebuild':
            assert self.read(0x50eb70)==0x45bd70
            return self.rebuild()
        assert self.read(0x50eb74)==0x45c060
        prefix=self.events[:];result=ShutdownState.apply(self);self.events=prefix+self.events
        if result:assert self.read(0x50eba4)==0x456f30
        return result

class Session(OpenSession):
    field_specs=FIELDS
    def __init__(self,pe,state,original):
        super().__init__(pe,state,original)
        if original:self.symbols.update(TARGETS|{n:0x400000+r for n,(r,*_) in ROUTINES.items()})
        self.logical_code={p:self.symbols[name] for name,p in TARGETS.items()}
        self.reverse_code={v:k for k,v in self.logical_code.items()}
        for _,p,_ in SLOTS:
            value=state.read(p,False);self.uc.mem_write(self.physical(p),struct.pack('<I',self.logical_code[value]))
    def snapshot(self):
        result=list(super().snapshot())
        for k in range(len(result)-5,len(result)):
            value=struct.unpack('<I',result[k])[0];result[k]=struct.pack('<I',self.reverse_code.get(value,value))
        return tuple(result)
    def memory(self,uc,access,a,n,x,data):
        count=len(self.events);super().memory(uc,access,a,n,x,data)
        if len(self.events)>count and any(self.physical(p)==a for _,p,_ in SLOTS):
            row=self.events[-1];self.events[-1]=row[:3]+(self.reverse_code.get(row[3],row[3]),)
    def run_entry(self,name,state,result,seed,policy):
        saved=self.symbols['Gfx_SurfaceOpenNative'];self.symbols['Gfx_SurfaceOpenNative']=self.symbols[name]
        try:super().run(state,result,seed,policy)
        finally:self.symbols['Gfx_SurfaceOpenNative']=saved

def verify_graphics_entry():
    original=pefile.PE(data=verify_target());inspect(original);build(dll=DLL);compiled=pefile.PE(str(DLL))
    coverage={name:{'standalone':0,'faults':0,'integration':0,'followups':0} for name in ROUTINES}
    for name in ROUTINES:
        for reply in (0,1,2,0x80000000,0xffffffff):
            for tail in (0,1,0x12345678,0xffffffff):
                for change in (False,1,2):
                    faults=[standalone(pe,k==0,name,reply,tail,change) for k,pe in enumerate((original,compiled))]
                    assert faults[0]==faults[1];coverage[name]['standalone']+=1;coverage[name]['faults']+=int(faults[0])
        if name.startswith('Gfx_Sprite'):continue
        for bad in (0,0x80000000,0xdeadbeef):
            for k,pe in enumerate((original,compiled)):assert standalone(pe,k==0,name,1,1,bad=bad)
            coverage[name]['standalone']+=1;coverage[name]['faults']+=1
    def sequence(state,names):
        sessions=[Session(pe,state,k==0) for k,pe in enumerate((original,compiled))]
        for k,name in enumerate(names):
            policy=copy.deepcopy(state);result=state.apply_entry(name)
            for session in sessions:session.run_entry(name,state,result,k,copy.deepcopy(policy))
            coverage[name]['integration']+=1;coverage[name]['followups']+=int(k>0)
            coverage[name]['faults']+=int(state.fault is not None)
            if state.fault:break
    for mode in (0,1,2,0xffffffff):
        for count in (0,1,2,4,5,0xffffffff):
            sequence(State(count,mode,count),('Gfx_Open','Gfx_Rebuild','Gfx_Rebuild','Gfx_Shutdown','Gfx_Shutdown'))
        for api in ('CreateWindowExA','DirectDrawCreate','SetCooperativeLevel','SetDisplayMode',
          'CreateSurface','GetAttachedSurface','CreateClipper','SetHWnd','SetClipper','CreatePalette','SetPalette'):
            state=State(91,mode,4);state.script[api,0]=(0 if api=='CreateWindowExA' else 0x80004005,None,[])
            sequence(state,('Gfx_Open','Gfx_Shutdown'))
        for bad in (0,0x80000000):
            state=State(92,mode,1);state.put(0x512c50,bad)
            sequence(state,('Gfx_Rebuild',))
    details={'scope':'Real C89 lifecycle dispatch and sprite leaves; standalone callback contracts plus real surface constructor/rebuild/shutdown integration, SDK replies modeled in emulation',
      'coverage':coverage,'checks':'Authentic hashes/FPO, arbitrary EAX replies, zero short circuit, live tail mutation/fetch faults, ordered effects, full state/image/caller bytes and cdecl/nonvolatile/DF; persistent real dependency lifecycle',
      'limitations':'No instruction equality/original toolchain/full game certification. Native windowed scope recorded separately with original WndProc reference boundary.'}
    for name,(r,*_) in ROUTINES.items():
        for kind in ('compilation','emulation'):
            record_run(r,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),cases=sum(coverage[name][x] for x in ('standalone','integration')) if kind=='emulation' else 0,command='uv run tools/verify_graphics_entry.py',details=details)
    print('PASS graphics entry differential:',coverage,flush=True)
    native=native_surface(DLL,entry=True)
    for name,(r,*_) in ROUTINES.items():
        cases=4 if name=='Gfx_Open' else 3
        record_run(r,'native','pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),cases=cases,
          command='uv run tools/verify_graphics_entry.py',details=native|{'routine':name,'executions':native['routine_executions'][name]})
    print('PASS native graphics entry:',native['stdout'],flush=True)
    return coverage

if __name__=='__main__':verify_graphics_entry()
