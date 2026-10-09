# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Surface bank initialization and real SDK COM restoration; modeled COM replies."""
import hashlib
import random
import struct
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE, UC_HOOK_MEM_INVALID, UC_MEM_WRITE, UC_MEM_READ_UNMAPPED
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EFLAGS
from verify_matching import STACK, STOP, PRESERVED
from build_decomp import build
from windows_target import ROOT, BUILD, verify_target, validation_symbols
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')
BANKS=[('g_nativePrimarySurface',0x50e778,48),('g_nativeType1Surfaces',0x50e688,240),('g_nativeType2Surfaces',0x50e7a8,960)]
RECORDS=[p+48*k for _,p,n in BANKS for k in range(n//48)]
ARENA,SIZE=0x3000000,0x4000
VTABLE=ARENA+0x1000
ISLOST,RESTORE=ARENA+0x2000,ARENA+0x2010
LOST=0x887601c2
ROUTINES={
 'Gfx_InitSurfaceRecords':(0x56a40,172,(172,0,0,0),'053404ce23bc0313a565a6807a6bff041db95d72b1231fda8e5fdbe5467f2d65'),
 'Gfx_SurfaceConfigureNative':(0x5b730,6,(6,0,4,0),'7140f35dee6220b79b12aecc27acf5105bf3b77d1588e89fce345de7c16c72b7'),
 'Gfx_SurfaceRestoreNative':(0x5c730,179,(179,0,0,0x308),'b5fbe03e6aab008cadf961db34a86c9d25525993c52b3a60dbe72a09591d6f71')}
DLL=BUILD/'surface_lifecycle_validation.dll'
INPUTS=['decomp/src/geputget.c','decomp/include/geputget.h','decomp/src/mem.c','decomp/include/mem.h','decomp/src/lisa3d.c','decomp/include/lisa3d.h','decomp/target.json','tools/build_decomp.py','tools/verify_surface_lifecycle.py','tools/verify_matching.py','tools/windows_target.py','tools/windows_tracking.py']

class Fault(Exception):
    pass

class State:
    def __init__(self,seed):
        rng=random.Random(seed)
        self.data={p:bytearray(rng.randbytes(n)) for _,p,n in BANKS}
        self.data[ARENA]=bytearray(rng.randbytes(SIZE))
        self.put(VTABLE+0x60,ISLOST);self.put(VTABLE+0x6c,RESTORE)
        self.policy={};self.events=[];self.fault=None
        for k,p in enumerate(RECORDS):
            obj=ARENA+16*k
            self.put(obj,VTABLE);self.put(p,0);self.put(p+44,obj)
            self.policy[('IsLost',obj)]=(0,[])
            self.policy[('Restore',obj)]=(0,[])

    def region(self,a,n=4):
        for p,b in self.data.items():
            if p<=a and a+n<=p+len(b):return b,a-p
        self.fault=('read',a,n);raise Fault

    def put(self,a,x):
        b,o=self.region(a);struct.pack_into('<I',b,o,x&0xffffffff)

    def read(self,a,tracked=True):
        b,o=self.region(a);x=struct.unpack_from('<I',b,o)[0]
        if tracked:self.events.append(('read',a,4,x))
        return x

    def write(self,a,x):
        self.put(a,x);self.events.append(('write',a,4,x&0xffffffff))

    def snapshot(self):return tuple(bytes(self.data[p]) for p in [ARENA]+[p for _,p,_ in BANKS])

    def com(self,name,obj):
        vt=self.read(obj,False);fn=self.read(vt+(0x60 if name=='IsLost' else 0x6c),False)
        assert fn==(ISLOST if name=='IsLost' else RESTORE)
        self.events.append((name,obj,self.snapshot()))
        reply,mutations=self.policy[name,obj]
        for p,x in mutations:self.put(p,x)
        return reply

    def apply(self,name):
        self.events=[];self.fault=None
        try:
            if name=='Gfx_SurfaceConfigureNative':return 2
            if name=='Gfx_InitSurfaceRecords':
                for kind,(_,p,n) in enumerate(BANKS):
                    for base in range(p,p+n,48):
                        for offset in range(0,48,4):self.write(base+offset,kind if offset==40 else 0)
                return 1
            restored=0
            for p in RECORDS:
                if self.read(p)==1:
                    obj=self.read(p+44)
                    if self.com('IsLost',obj)==LOST:
                        restored+=1
                        self.com('Restore',self.read(p+44))
                        self.write(p+4,1)
            return int(restored==0)
        except Fault:return None

class Session:
    def __init__(self,pe,state,original):
        self.uc=Uc(UC_ARCH_X86,UC_MODE_32);self.base=pe.OPTIONAL_HEADER.ImageBase
        self.size=(pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095
        self.uc.mem_map(self.base,self.size);self.uc.mem_write(self.base,pe.get_memory_mapped_image())
        self.uc.mem_map(ARENA,SIZE);self.uc.mem_map(STACK,0x10000);self.uc.mem_map(STOP,4096)
        self.symbols=({name:0x400000+r for name,(r,*_) in ROUTINES.items()}|{name:p for name,p,_ in BANKS}) if original else validation_symbols(pe)
        self.fields=[(p,self.symbols[name],n) for name,p,n in BANKS]
        for p,b in state.data.items():self.uc.mem_write(self.physical(p),bytes(b))
        self.uc.hook_add(UC_HOOK_CODE,self.code)
        self.uc.hook_add(UC_HOOK_MEM_READ|UC_HOOK_MEM_WRITE,self.memory)
        self.uc.hook_add(UC_HOOK_MEM_INVALID,self.invalid)

    def physical(self,a):
        for l,p,n in self.fields:
            if l<=a<l+n:return p+a-l
        return a

    def normalize(self,a):
        for l,p,n in self.fields:
            if p<=a<p+n:return l+a-p
        return a

    def snapshot(self):return tuple(bytes(self.uc.mem_read(self.physical(p),n)) for p,n in [(ARENA,SIZE)]+[(p,n) for _,p,n in BANKS])

    def memory(self,uc,access,a,n,x,data):
        if not any(p<=a and a+n<=p+z for _,p,z in self.fields):return
        if access!=UC_MEM_WRITE:x=int.from_bytes(uc.mem_read(a,n),'little')
        self.events.append(('write' if access==UC_MEM_WRITE else 'read',self.normalize(a),n,x))

    def invalid(self,uc,access,a,n,x,data):
        self.fault=('read' if access==UC_MEM_READ_UNMAPPED else 'write',self.normalize(a),n)
        return False

    def code(self,uc,a,n,data):
        if a not in (ISLOST,RESTORE):return
        name='IsLost' if a==ISLOST else 'Restore'
        sp=uc.reg_read(UC_X86_REG_ESP);ret,obj=struct.unpack('<II',uc.mem_read(sp,8))
        self.events.append((name,obj,self.snapshot()))
        reply,mutations=self.policy[name,obj]
        for p,x in mutations:uc.mem_write(self.physical(p),struct.pack('<I',x&0xffffffff))
        for reg,x in ((UC_X86_REG_EAX,reply),(UC_X86_REG_ECX,0xdeadbeef),(UC_X86_REG_EDX,0x12345678),(UC_X86_REG_ESP,sp+8),(UC_X86_REG_EIP,ret),(UC_X86_REG_EFLAGS,0x8d7)):
            uc.reg_write(reg,x)

    def run(self,name,args,state,result,seed):
        uc=self.uc;sp=STACK+0x8000
        self.events=[];self.fault=None;self.policy=state.policy
        uc.mem_write(sp,struct.pack('<'+'I'*(len(args)+1),STOP,*args))
        caller=bytes(uc.mem_read(sp,0x8000));saved=[random.Random(seed+i).getrandbits(32) for i in range(4)]
        for r,x in zip(PRESERVED,saved):uc.reg_write(r,x)
        uc.reg_write(UC_X86_REG_ESP,sp);uc.reg_write(UC_X86_REG_EFLAGS,2)
        before=bytearray(uc.mem_read(self.base,self.size))
        try:uc.emu_start(self.symbols[name],STOP,count=10000)
        except UcError:assert self.fault is not None
        assert self.fault==state.fault,(name,seed,self.fault,state.fault)
        assert self.events==state.events,(name,seed,next(((k,a[:3],b[:3]) for k,(a,b) in enumerate(zip(self.events,state.events)) if a!=b),(len(self.events),len(state.events))))
        assert self.snapshot()==state.snapshot(),(name,seed,'state')
        assert bytes(uc.mem_read(sp,0x8000))==caller
        after=bytearray(uc.mem_read(self.base,self.size))
        for _,p,n in self.fields:before[p-self.base:p-self.base+n]=after[p-self.base:p-self.base+n]
        assert before==after,'Unrelated image bytes changed'
        if self.fault is None:
            assert uc.reg_read(UC_X86_REG_EIP)==STOP and uc.reg_read(UC_X86_REG_ESP)==sp+4
            assert [uc.reg_read(r) for r in PRESERVED]==saved
            assert not uc.reg_read(UC_X86_REG_EFLAGS)&0x400
            assert uc.reg_read(UC_X86_REG_EAX)==result

def inspect(pe):
    debug=next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type==3)
    fpo={r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    for name,(r,n,row,digest) in ROUTINES.items():
        assert fpo[r]==row
        raw=pe.get_data(r,n);assert hashlib.sha256(raw).hexdigest()==digest
        assert sum(i.size for i in md.disasm(raw,0x400000+r))==n
    reloc={e.rva for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries if e.type==3}
    for r,value in ((0x5b696,0x45b730),(0x5b718,0x45c730)):
        assert r in reloc and struct.unpack('<I',pe.get_data(r,4))[0]==value

def verify_surface_lifecycle():
    original=pefile.PE(data=verify_target());inspect(original);build(dll=DLL);compiled=pefile.PE(str(DLL))
    coverage={name:{'comparisons':0,'persistent_followups':0,'faults':0,'mutations':0} for name in ROUTINES}
    def sequence(state,names,args=(),mutate=False):
        sessions=[Session(pe,state,k==0) for k,pe in enumerate((original,compiled))]
        for k,name in enumerate(names):
            result=state.apply(name)
            for session in sessions:session.run(name,args,state,result,sum(x['comparisons'] for x in coverage.values()))
            c=coverage[name];c['comparisons']+=1;c['persistent_followups']+=int(k>0);c['faults']+=int(state.fault is not None);c['mutations']+=int(mutate)
            if state.fault:break
    for seed in range(24):
        s=State(seed)
        for p in RECORDS:s.put(p,random.Random(seed+p).getrandbits(32))
        sequence(s,['Gfx_InitSurfaceRecords','Gfx_SurfaceRestoreNative','Gfx_InitSurfaceRecords','Gfx_SurfaceRestoreNative'])
        sequence(State(seed),['Gfx_SurfaceConfigureNative']*2,tuple(random.Random(seed+j).getrandbits(32) for j in range(4)))
    # Each individual slot, every bank, exact active equality and HRESULT.
    for k,p in enumerate(RECORDS):
        for active in (0,1,2,0xffffffff):
            for reply in (0,LOST,0x80004005,LOST+1):
                s=State(k);s.put(p,active);s.policy['IsLost',ARENA+16*k]=(reply,[])
                s.policy['Restore',ARENA+16*k]=(0x80004005,[])
                sequence(s,['Gfx_SurfaceRestoreNative']*2)
    for seed in range(32):
        rng=random.Random(seed);s=State(seed)
        for k,p in enumerate(RECORDS):
            s.put(p,rng.choice([0,1,2,1]));s.policy['IsLost',ARENA+16*k]=(rng.choice([LOST,0,LOST,0x80004005]),[])
        sequence(s,['Gfx_SurfaceRestoreNative']*3)
    for k,p in enumerate(RECORDS):
        s=State(k);s.put(p,1)
        # IsLost can replace this pointer, disable itself, activate a later slot.
        other=ARENA+16*((k+1)%26);later=RECORDS[(k+1)%26]
        s.policy['IsLost',ARENA+16*k]=(LOST,[(p+44,other),(p,0),(later,1),(p+4,0xaabbccdd)])
        s.policy['Restore',other]=(0x80004005,[(p+4,0x11223344),(RECORDS[-1],2)])
        sequence(s,['Gfx_SurfaceRestoreNative']*2,mutate=True)
        for bad in (0,0x80000000):
            s=State(k);s.put(p,1);s.put(p+44,bad)
            sequence(s,['Gfx_SurfaceRestoreNative'])
            s=State(k);s.put(p,1);s.policy['IsLost',ARENA+16*k]=(LOST,[(p+44,bad)])
            sequence(s,['Gfx_SurfaceRestoreNative'],mutate=True)
    # Several records may own the same COM surface; no deduplication is authentic.
    for seed in range(8):
        s=State(seed)
        for p in RECORDS:s.put(p,1);s.put(p+44,ARENA)
        s.policy['IsLost',ARENA]=(LOST,[])
        sequence(s,['Gfx_SurfaceRestoreNative']*3)
    # Unchecked object/vtable reads fail before any COM entry or marker write.
    for phase in ('IsLost','Restore'):
        for bad in (0,0x80000000):
            s=State(71);p=RECORDS[6];obj=ARENA+16*6;s.put(p,1)
            if phase=='IsLost':s.put(obj,bad)
            else:s.policy['IsLost',obj]=(LOST,[(obj,bad)])
            sequence(s,['Gfx_SurfaceRestoreNative'],mutate=phase=='Restore')
    details={'scope':'Real C89 surface bank initializer/configure and SDK IDirectDrawSurface restoration; COM IsLost/Restore are explicit stdcall modeled boundaries',
      'coverage':coverage,'checks':'Authenticated FPO/extents/hashes; independent raw-address oracle; ordered record reads/writes, full banks/arena/unrelated image, COM this/entry snapshots, HRESULT equality and ignored Restore failure, live callback pointer/active/marker mutations, persistent calls, faults, cdecl/nonvolatile/DF and stdcall stack cleanup',
      'limitations':'No native DirectDraw/window/game runtime certification, instruction equality or original compiler/link layout. Arbitrary reentry/concurrency, unknown adjacent aliases and native fault frames/atomicity unverified. Eight record words remain opaque.'}
    for name,(r,*_) in ROUTINES.items():
        for kind in ('compilation','emulation'):
            record_run(r,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),cases=coverage[name]['comparisons'] if kind=='emulation' else 0,command='uv run python tools/verify_surface_lifecycle.py',details=details|{'routine_coverage':coverage[name]})
    print('PASS surface lifecycle dependencies:',coverage,flush=True)
    return coverage

if __name__=='__main__':verify_surface_lifecycle()
