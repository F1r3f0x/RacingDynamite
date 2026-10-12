# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Authentic capability-gated presentation and finite SDK busy retries."""
import copy,struct,hashlib
import pefile
from unicorn.x86_const import UC_X86_REG_EAX,UC_X86_REG_ECX,UC_X86_REG_EDX,UC_X86_REG_EIP,UC_X86_REG_ESP,UC_X86_REG_EFLAGS
import verify_surface_access as access
from verify_surface_lifecycle import ARENA,RECORDS,VTABLE,Fault
from verify_matching import STACK,STOP
from windows_target import BUILD,verify_target,validate_platform_imports
from build_decomp import build
from windows_tracking import record_run
from verify_surface_open import native_surface
from verify_native_presentation import native_presentation
from windows_target import ROOT
if not __debug__:raise RuntimeError('Verification requires assertions')
INPUTS=access.INPUTS+['tools/verify_surface_present.py','tools/verify_native_presentation.py','tools/native_present_frames.c']
BUSY=0x8876021c
DISPATCH=0x50eb88
FUNCTIONS={'GetCaps':ARENA+0x2100,'Flip':ARENA+0x2110}
ROUTINES={'Gfx_SurfacePresentNative':(0x5c4b0,98,(98,1,1,260),'0a2b21a1bf12a224948977d5410f15eee668a81d3245a9d644f8f0846af6412e'),
 'Gfx_PresentSurfaceNative':(0x56c90,15,(15,0,1,0),'90586f74ec900ac8213237cc025768b3c50c8782ddbb0130505452bcea742f01')}
class State(access.State):
 def __init__(self,seed):
  super().__init__(seed);self.data[DISPATCH]=bytearray(struct.pack('<I',0x45c4b0))
  self.put(VTABLE+0x38,FUNCTIONS['GetCaps']);self.put(VTABLE+0x2c,FUNCTIONS['Flip'])
 def snapshot(self):return super().snapshot()+(bytes(self.data[DISPATCH]),)
 def response(self,name):
  k=self.seen.get(name,0);self.seen[name]=k+1
  hr,caps,changes=self.script.get((name,k),(0,0x10,[]))
  for p,x in changes:self.put(p,x)
  return hr,caps
 def method(self,name,obj,args):
  vt=self.read(obj,False);assert self.read(vt+(0x38 if name=='GetCaps' else 0x2c),False)==FUNCTIONS[name]
  self.events.append(('api',name,(obj,)+args,self.snapshot()));return self.response(name)
 def apply(self,name,args):
  self.events=[];self.fault=None;self.seen={};p=args[0]
  try:
   if name=='Gfx_PresentSurfaceNative':assert self.read(DISPATCH)==0x45c4b0
   if self.read(p+40)!=1 or self.read(p)==0:return 0
   _,caps=self.method('GetCaps',self.read(p+44),(4,))
   if not caps&0x10:return 0
   while True:
    target=self.read(p+44);primary=self.read(0x50e7a4)
    hr,_=self.method('Flip',primary,(target,0))
    if hr!=BUSY:return int(hr==0)
  except Fault:return None
class Session(access.Session):
 def __init__(self,pe,state,original):
  scratch=copy.copy(state);scratch.data=state.data.copy();del scratch.data[DISPATCH]
  super().__init__(pe,scratch,original)
  if original:
   self.symbols.update({n:0x400000+x[0] for n,x in ROUTINES.items()})
   self.symbols['Gfx_InstallSurfaceDispatch']=0x45b690;self.symbols['g_surfacePresent']=DISPATCH
  self.events=[];sp=STACK+0x6000;self.uc.mem_write(sp,struct.pack('<I',STOP))
  self.uc.reg_write(UC_X86_REG_ESP,sp);self.uc.reg_write(UC_X86_REG_EFLAGS,2)
  self.uc.emu_start(self.symbols['Gfx_InstallSurfaceDispatch'],STOP,count=1000)
  assert self.uc.reg_read(UC_X86_REG_EAX)==1 and self.uc.reg_read(UC_X86_REG_ESP)==sp+4
  assert struct.unpack('<I',self.uc.mem_read(self.symbols['g_surfacePresent'],4))[0]==self.symbols['Gfx_SurfacePresentNative']
  self.fields.append((DISPATCH,self.symbols['g_surfacePresent'],4))
 def snapshot(self):
  target=struct.unpack('<I',self.uc.mem_read(self.physical(DISPATCH),4))[0]
  if target==self.symbols['Gfx_SurfacePresentNative']:target=0x45c4b0
  return super().snapshot()+(struct.pack('<I',target),)
 def memory(self,uc,kind,a,n,x,data):
  k=len(self.events);super().memory(uc,kind,a,n,x,data)
  if len(self.events)>k and self.normalize(a)==DISPATCH:
   row=self.events[-1]
   if row[3]==self.symbols['Gfx_SurfacePresentNative']:self.events[-1]=row[:3]+(0x45c4b0,)
 def code(self,uc,a,n,data):
  reverse={v:k for k,v in FUNCTIONS.items()}
  if a not in reverse:return
  name=reverse[a];argc=2 if name=='GetCaps' else 3;sp=uc.reg_read(UC_X86_REG_ESP)
  ret,*raw=struct.unpack('<'+'I'*(argc+1),uc.mem_read(sp,4*(argc+1)))
  args=[self.normalize(x) for x in raw]
  if name=='GetCaps':assert STACK<=raw[1]<STACK+0x10000;args[1]=4
  self.events.append(('api',name,tuple(args),self.snapshot()))
  result,caps=self.policy.response(name);k=self.policy.seen[name]-1
  for p,x in self.policy.script.get((name,k),(None,None,[]))[2]:uc.mem_write(self.physical(p),struct.pack('<I',x))
  if name=='GetCaps':uc.mem_write(raw[1],struct.pack('<I',caps))
  for reg,x in ((UC_X86_REG_EAX,result),(UC_X86_REG_ECX,0xdeadbeef),(UC_X86_REG_EDX,0x12345678),(UC_X86_REG_EIP,ret),(UC_X86_REG_ESP,sp+4+4*argc),(UC_X86_REG_EFLAGS,0x8d7)):uc.reg_write(reg,x)
def verify_surface_present(record=True):
 original=pefile.PE(data=verify_target());dll=BUILD/'surface_present_validation.dll';build(dll=dll);compiled=pefile.PE(str(dll));validate_platform_imports(compiled)
 debug=next(d.struct for d in original.DIRECTORY_ENTRY_DEBUG if d.struct.Type==3)
 fpo={r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',original.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
 for r,n,row,digest in ROUTINES.values():assert fpo[r]==row and hashlib.sha256(original.get_data(r,n)).hexdigest()==digest
 coverage={n:{'comparisons':0,'followups':0,'faults':0,'mutations':0,'busy_retries':0} for n in ROUTINES}
 def sequence(state,names,args):
  sessions=[Session(pe,state,k==0) for k,pe in enumerate((original,compiled))]
  for k,name in enumerate(names):
   policy=copy.deepcopy(state);policy.seen={};result=state.apply(name,args)
   for session in sessions:session.run(name,args,state,result,sum(x['comparisons'] for x in coverage.values()),copy.deepcopy(policy))
   c=coverage[name];c['comparisons']+=1;c['followups']+=int(k>0);c['faults']+=int(state.fault is not None)
   c['mutations']+=int(any(changes and j<state.seen.get(api,0) for (api,j),(_,_,changes) in state.script.items()))
   c['busy_retries']+=max(0,state.seen.get('Flip',0)-1)
   if state.fault:break
 direct,entry=ROUTINES
 for p in RECORDS:
  for kind in (0,1,2,0xffffffff):
   for active in (0,1,2,0xffffffff):
    for caps in (0,0x10,0xffffffff):
     state=State(p);state.put(p+40,kind);state.put(p,active);state.script['GetCaps',0]=(0,caps,[])
     sequence(state,(direct,entry,direct),(p,))
 p=RECORDS[1]
 for caps in (0,0x10,0xffffffff):
  for hr in (0,1,0x80004005,access.LOST):
   for final in (0,1,0x80004005,access.LOST):
    state=State(3);state.put(p+40,1);state.script['GetCaps',0]=(hr,caps,[]);state.script['Flip',0]=(final,0,[])
    sequence(state,(entry,direct),(p,))
 for count in (1,2,8,32):
  state=State(4);state.put(p+40,1)
  for k in range(count):state.script['Flip',k]=(BUSY,0,[])
  sequence(state,(direct,entry),(p,))
 for api in FUNCTIONS:
  state=State(5);state.put(p+40,1)
  state.script[api,0]=(BUSY if api=='Flip' else 0,0x10,[(p+44,ARENA+16*3),(0x50e7a4,ARENA+16*4),(p+40,2),(p,0)])
  sequence(state,(entry,direct),(p,))
 for name in ROUTINES:
  for bad in (0,0xdead0000):
   for address in (p+44,0x50e7a4):
    state=State(6);state.put(p+40,1);state.put(address,bad);sequence(state,(name,),(p,))
 if record:
  for name,(r,*_) in ROUTINES.items():
   for kind in ('compilation','emulation'):
    record_run(r,kind,'pass',inputs=INPUTS,artifact=dll.relative_to(ROOT).as_posix(),cases=coverage[name]['comparisons'] if kind=='emulation' else 0,
     command='uv run tools/verify_surface_present.py',details={'coverage':coverage[name],
      'scope':'Real SDK C89 GetCaps/Flip and actual installed-table entry; SDK outputs and finite busy retries modeled; ordered state/ABI and image equality',
      'limitations':'GetCaps without outputs exposes uninitialized original stack; infinite busy loop, arbitrary reentry and full native game/toolchain unverified'})
  native=native_surface(dll,present_test=True)
  frames=native_presentation(dll)
  for name,(r,*_) in ROUTINES.items():
   record_run(r,'native','pass',inputs=INPUTS,artifact=dll.relative_to(ROOT).as_posix(),cases=3,
    command='uv run tools/verify_surface_present.py',details=native|{'routine':name,'fullscreen_frames':frames})
  print('PASS native Present:',native['stdout']+frames['stdout'],flush=True)
 print('PASS Present:',coverage,flush=True)
 return coverage

if __name__=="__main__":
 import sys
 verify_surface_present(record="--no-record" not in sys.argv)
