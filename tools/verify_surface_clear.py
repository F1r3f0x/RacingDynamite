# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Authentic DWORD-scaled Clear through real access and dispatch dependencies."""
import copy
import hashlib
import struct
import pefile
from unicorn import UC_MEM_WRITE
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EFLAGS
import verify_surface_access as access
from verify_surface_lifecycle import ARENA, RECORDS, Fault
from verify_matching import STACK, STOP
from windows_target import ROOT, BUILD, verify_target, validation_symbols, validate_platform_imports
from windows_tracking import record_run
from build_decomp import build
from verify_surface_open import native_surface

if not __debug__:
    raise RuntimeError('Verification requires assertions')
DISPATCH=0x50eb84
ROUTINES={
 'Gfx_SurfaceClearNative':(0x5c3d0,210,(210,0,1,775),'dd3bd132e2b814763aab3ab5f58c6b162ddf69bb41a46f98e2cdd5a9b87df8d6'),
 'Gfx_ClearSurfaceNative':(0x56bb0,15,(15,0,1,0),'652dfcd5dbaf40c20c1d2f5745962ddee365aa37d367a97cba92f986ba2e2685')}
DLL=BUILD/'surface_clear_validation.dll'
INPUTS=access.INPUTS+['tools/verify_surface_clear.py']

def signed(x):return x-0x100000000 if x&0x80000000 else x

class State(access.State):
    def __init__(self,seed):
        super().__init__(seed)
        self.data[DISPATCH]=bytearray(struct.pack('<I',0x45c3d0))
    def snapshot(self):return super().snapshot()+(bytes(self.data[DISPATCH]),)
    def write(self,a,x):
        try:super().write(a,x)
        except Fault:
            self.fault=('write',a,4)
            raise
    def clear(self,p):
        if self.read(p)==0:return 0
        old=self.read(p+12)
        if old==0:
            if not self.lock(p,3):return 0
        elif old==1:
            if not self.unlock(p) or not self.lock(p,3):return 0
        pixels=self.read(p+20);row=0
        while row<signed(self.read(p+32)):
            column=0
            while column<signed(self.read(p+28)):
                offset=(self.read(p+24)*row+column)&0xffffffff
                self.write((pixels+offset*4)&0xffffffff,0);column+=1
            row+=1
        if self.read(p+12)!=old:
            if not self.unlock(p):return 0
            if old!=0 and not self.lock(p,old):return 0
        return 1
    def apply(self,name,args):
        self.events=[];self.fault=None;self.seen={}
        try:
            if name=='Gfx_ClearSurfaceNative':assert self.read(DISPATCH)==0x45c3d0
            return self.clear(args[0])
        except Fault:return None

class Session(access.Session):
    def __init__(self,pe,state,original):
        scratch=copy.copy(state);scratch.data=state.data.copy();del scratch.data[DISPATCH]
        super().__init__(pe,scratch,original)
        # Only the two verified pixel-pointer fields carry record relocations.
        for p in RECORDS:
            for offset in (16,20):
                value=state.read(p+offset,False)
                self.uc.mem_write(self.physical(p+offset),struct.pack('<I',self.physical(value)))
        if original:
            self.symbols.update({name:0x400000+row[0] for name,row in ROUTINES.items()})
            self.symbols['Gfx_InstallSurfaceDispatch']=0x45b690
            self.symbols['g_surfaceClear']=DISPATCH
        self.events=[];self.fault=None
        # Run the actual installer before initializing this case's contract.
        sp=STACK+0x6000;self.uc.mem_write(sp,struct.pack('<I',STOP))
        self.uc.reg_write(UC_X86_REG_ESP,sp);self.uc.reg_write(UC_X86_REG_EFLAGS,2)
        self.uc.emu_start(self.symbols['Gfx_InstallSurfaceDispatch'],STOP,count=1000)
        assert self.uc.reg_read(UC_X86_REG_EAX)==1 and self.uc.reg_read(UC_X86_REG_ESP)==sp+4
        target=struct.unpack('<I',self.uc.mem_read(self.symbols['g_surfaceClear'],4))[0]
        assert target==self.symbols['Gfx_SurfaceClearNative'],'Real installer did not bind the real Clear body'
        self.fields.append((DISPATCH,self.symbols['g_surfaceClear'],4))
    def snapshot(self):
        target=struct.unpack('<I',self.uc.mem_read(self.physical(DISPATCH),4))[0]
        if target==self.symbols['Gfx_SurfaceClearNative']:target=0x45c3d0
        result=list(super().snapshot())
        for k,(_,base,size) in enumerate(access.BANKS,1):
            bank=bytearray(result[k])
            for p in range(0,size,48):
                for offset in (16,20):
                    value=struct.unpack_from('<I',bank,p+offset)[0]
                    struct.pack_into('<I',bank,p+offset,self.normalize(value))
            result[k]=bytes(bank)
        return tuple(result)+(struct.pack('<I',target),)
    def memory(self,uc,kind,a,n,x,data):
        length=len(self.events);super().memory(uc,kind,a,n,x,data)
        if len(self.events)>length:
            row=self.events[-1]
            if n==4 and any(self.normalize(a)==p+offset for p in RECORDS for offset in (16,20)):
                self.events[-1]=row[:3]+(self.normalize(row[3]),)
        if len(self.events)>length and self.normalize(a)==DISPATCH:
            row=self.events[-1]
            if row[3]==self.symbols['Gfx_SurfaceClearNative']:self.events[-1]=row[:3]+(0x45c3d0,)
        if ARENA+0x2800<=a and a+n<=ARENA+0x3000:
            if kind==UC_MEM_WRITE:self.events.append(('write',a,n,x))

def verify_surface_clear(record=True):
    original=pefile.PE(data=verify_target());build(dll=DLL);compiled=pefile.PE(str(DLL));validate_platform_imports(compiled)
    debug=next(d.struct for d in original.DIRECTORY_ENTRY_DEBUG if d.struct.Type==3)
    fpo={r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',original.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    for r,n,row,digest in ROUTINES.values():assert fpo[r]==row and hashlib.sha256(original.get_data(r,n)).hexdigest()==digest
    coverage={name:{'comparisons':0,'followups':0,'faults':0,'mutations':0,'pixel_aliases':0} for name in ROUTINES}
    def sequence(state,names,args,alias=False):
        sessions=[Session(pe,state,k==0) for k,pe in enumerate((original,compiled))]
        for k,name in enumerate(names):
            policy=copy.deepcopy(state);policy.seen={};result=state.apply(name,args)
            for session in sessions:session.run(name,args,state,result,sum(c['comparisons'] for c in coverage.values()),copy.deepcopy(policy))
            c=coverage[name];c['comparisons']+=1;c['followups']+=int(k>0);c['faults']+=int(state.fault is not None);c['pixel_aliases']+=int(alias)
            c['mutations']+=int(any(changes and occurrence<state.seen.get(api,0) for (api,occurrence),(_,_,_,changes) in state.script.items()))
            if state.fault:break
    direct,entry=ROUTINES
    for p in RECORDS:
        for mode in (0,1,2,3,4,0xffffffff):
            for width,height in ((0,0),(0,4),(3,0),(3,4),(0xffffffff,3),(3,0xffffffff)):
                state=State(p);state.put(p+8,1 if mode else 0);state.put(p+12,mode);state.put(p+28,width);state.put(p+32,height)
                sequence(state,(direct,entry,direct),(p,))
    p=RECORDS[0]
    for api in access.FUNCTIONS:
        for reply in (0,1,0x80004005,access.LOST):
            for mode in (0,1,2,3):
                state=State(17);state.put(p+8,1 if mode else 0);state.put(p+12,mode)
                state.script[api,0]=(reply,access.PIXELS,8,[])
                sequence(state,(entry,direct),(p,))
    for address in (p,p+4,p+8,p+12,p+16,p+20,p+24,p+28,p+32,p+36,p+40,p+44):
        state=State(18);state.put(p+8,1);state.put(p+12,2);state.put(p+20,address);state.put(p+24,0)
        state.put(p+28,1);state.put(p+32,1)
        sequence(state,(direct,entry),(p,),True)
    for api in access.FUNCTIONS:
        state=State(19);state.put(p+12,1);state.put(p+8,1)
        if api=='Restore':state.script['IsLost',0]=(access.LOST,access.PIXELS,8,[])
        state.script[api,0]=(0,access.PIXELS,8,[(p+12,3),(p+28,2),(p+32,3),(p+24,7)])
        sequence(state,(entry,direct),(p,))
    for active in (0,1,2,0xffffffff):
        state=State(20);state.put(p,active);sequence(state,(entry,direct),(p,))
    for name in ROUTINES:
        for bad in (0,0xdead0000):
            state=State(21);state.put(p+8,1);state.put(p+12,2)
            state.put(p+20,bad);state.put(p+28,1);state.put(p+32,1)
            sequence(state,(name,),(p,))
        for pitch in (0,1,4,0xfffffffc):
            state=State(22);state.put(p+8,1);state.put(p+12,2)
            state.put(p+20,ARENA+0x2900);state.put(p+24,pitch)
            state.put(p+28,2);state.put(p+32,3)
            sequence(state,(name,name),(p,))
    if record:
        for name,(r,*_) in ROUTINES.items():
            for kind in ('compilation','emulation'):
                record_run(r,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),cases=coverage[name]['comparisons'] if kind=='emulation' else 0,
                    command='uv run tools/verify_surface_clear.py',details={'coverage':coverage[name],
                     'scope':'Real Clear/Lock/Unlock chain and real installed-table entry; SDK outputs modeled; exact DWORD-scaled pitch, pixel/record writes, state and ABI',
                     'limitations':'Native full-frame rendering, failed access without descriptor outputs, arbitrary aliases/reentry/native faults and original toolchain unverified'})
        native=native_surface(DLL,clear_test=True)
        for name,(r,*_) in ROUTINES.items():
            record_run(r,'native','pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),cases=2,
                command='uv run tools/verify_surface_clear.py',details=native|{'routine':name,'direct_calls':8,'entry_calls':8})
        print('PASS native Clear:',native['stdout'],flush=True)
    print('PASS surface Clear:',coverage,flush=True)
    return coverage

if __name__=='__main__':
    import sys
    verify_surface_clear(record='--no-record' not in sys.argv)
