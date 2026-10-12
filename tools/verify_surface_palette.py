# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Authentic triplet palette copy, real SDK call and installed entry."""
import sys,copy,random,struct,hashlib
from pathlib import Path

import pefile
from unicorn import UC_MEM_WRITE
import verify_surface_access as access
from verify_surface_lifecycle import ARENA,VTABLE,Fault
from verify_matching import STACK
from windows_target import verify_target,BUILD,validate_platform_imports
from build_decomp import build
from windows_tracking import record_run
from windows_target import ROOT
from verify_native_presentation import native_surface_host
if not __debug__:raise RuntimeError('Verification requires assertions')
INPUTS=access.INPUTS+['tools/verify_surface_palette.py','tools/verify_native_presentation.py','tools/native_palette_probe.c']
DISPATCH=0x50eb98
ROUTINES={'Gfx_SurfaceSetPaletteNative':(0x5c6d0,83,(83,0,1,0),'cce59efd58c21d910a12512b15c5641b586e6beac7a4fd5daad80a56c991f5f0'),
 'Gfx_SetPaletteNative':(0x56c40,15,(15,0,1,0),'2f73ffb25305361147dd76513ce9b4125182485565ad60410532f9cc6f60f308')}
PAL=0x512448;ENTRIES=0x512450;FN=ARENA+0x2200;OBJ=ARENA+0x2300;PVT=ARENA+0x2400
class State(access.State):
 def __init__(self,seed):
  super().__init__(seed);self.data[PAL]=bytearray(struct.pack('<I',OBJ));self.data[ENTRIES]=bytearray(random.Random(seed+100).randbytes(1024))
  self.put(OBJ,PVT);self.put(PVT+0x18,FN);self.reply=0;self.read_trigger=None;self.read_changes=[]
  self.data[DISPATCH]=bytearray(struct.pack('<I',0x45c6d0))
  self.data[0x512048]=bytearray(random.Random(seed+200).randbytes(1024))
  self.data[0x512850]=bytearray(random.Random(seed+300).randbytes(1024))
 def snapshot(self):return super().snapshot()+(bytes(self.data[PAL]),bytes(self.data[ENTRIES]),bytes(self.data[DISPATCH]),bytes(self.data[0x512048]),bytes(self.data[0x512850]))
 def byte(self,p):
  if p==self.read_trigger:
   for a,x in self.read_changes:self.put(a,x)
  b,o=self.region(p,1);x=b[o];self.events.append(('read',p,1,x));return x
 def store(self,p,x):
  b,o=self.region(p,1);b[o]=x;self.events.append(('write',p,1,x))
 def apply(self,name,args):
  self.events=[];self.fault=None
  try:
   if name=='Gfx_SetPaletteNative':assert self.read(DISPATCH)==0x45c6d0
   if self.read(PAL)==0:return 0
   for k in range(256):
    for j in range(3):self.store(ENTRIES+4*k+j,self.byte(args[0]+3*k+j))
   obj=self.read(PAL);vt=self.read(obj,False);assert self.read(vt+0x18,False)==FN
   self.events.append(('api','SetEntries',(obj,0,0,256,ENTRIES),self.snapshot()))
   return int(self.reply==0)
  except Fault:return None
class Session(access.Session):
 def __init__(self,pe,state,original):
  scratch=copy.copy(state);scratch.data={p:b for p,b in state.data.items() if p not in (PAL,ENTRIES,DISPATCH,0x512048,0x512850)}
  super().__init__(pe,scratch,original)
  if original:
   self.symbols.update({name:0x400000+row[0] for name,row in ROUTINES.items()})
   self.symbols.update({'g_nativePalette':PAL,'g_nativeSetPaletteEntries':ENTRIES,'g_surfaceSetPalette':DISPATCH,
    'g_nativePaletteEntries':0x512048,'g_nativeRebuildPaletteEntries':0x512850,'Gfx_InstallSurfaceDispatch':0x45b690})
  for logical,name,size in ((PAL,'g_nativePalette',4),(ENTRIES,'g_nativeSetPaletteEntries',1024),
    (DISPATCH,'g_surfaceSetPalette',4),(0x512048,'g_nativePaletteEntries',1024),(0x512850,'g_nativeRebuildPaletteEntries',1024)):
   self.fields.append((logical,self.symbols[name],size))
   if logical!=DISPATCH:self.uc.mem_write(self.physical(logical),bytes(state.data[logical]))
  self.events=[];sp=STACK+0x6000;self.uc.mem_write(sp,struct.pack('<I',access.STOP))
  self.uc.reg_write(access.UC_X86_REG_ESP,sp);self.uc.reg_write(access.UC_X86_REG_EFLAGS,2)
  self.uc.emu_start(self.symbols['Gfx_InstallSurfaceDispatch'],access.STOP,count=1000)
  assert self.uc.reg_read(access.UC_X86_REG_EAX)==1 and self.uc.reg_read(access.UC_X86_REG_ESP)==sp+4
  assert struct.unpack('<I',self.uc.mem_read(self.physical(DISPATCH),4))[0]==self.symbols['Gfx_SurfaceSetPaletteNative']
 def snapshot(self):
  target=struct.unpack('<I',self.uc.mem_read(self.physical(DISPATCH),4))[0]
  if target==self.symbols['Gfx_SurfaceSetPaletteNative']:target=0x45c6d0
  return super().snapshot()+tuple(bytes(self.uc.mem_read(self.physical(p),n)) for p,n in ((PAL,4),(ENTRIES,1024)))+(struct.pack('<I',target),)+tuple(bytes(self.uc.mem_read(self.physical(p),1024)) for p in (0x512048,0x512850))
 def memory(self,uc,kind,a,n,x,data):
  if kind!=UC_MEM_WRITE and getattr(self,'policy',None) is not None and self.normalize(a)==self.policy.read_trigger:
   for p,x in self.policy.read_changes:uc.mem_write(self.physical(p),struct.pack('<I',x))
  k=len(self.events);super().memory(uc,kind,a,n,x,data)
  if len(self.events)>k and self.normalize(a)==DISPATCH:
   row=self.events[-1]
   if row[3]==self.symbols['Gfx_SurfaceSetPaletteNative']:self.events[-1]=row[:3]+(0x45c6d0,)
  if k==len(self.events) and ARENA+0x5000<=a and a+n<=ARENA+0x5400:
   self.events.append(('read',a,n,int.from_bytes(uc.mem_read(a,n),'little')))
 def code(self,uc,a,n,data):
  if a!=FN:return
  sp=uc.reg_read(access.UC_X86_REG_ESP);ret,*args=struct.unpack('<6I',uc.mem_read(sp,24))
  self.events.append(('api','SetEntries',tuple(self.normalize(x) for x in args),self.snapshot()))
  for r,x in ((access.UC_X86_REG_EAX,self.policy.reply),(access.UC_X86_REG_ECX,0xdeadbeef),(access.UC_X86_REG_EDX,0x12345678),(access.UC_X86_REG_ESP,sp+24),(access.UC_X86_REG_EIP,ret),(access.UC_X86_REG_EFLAGS,0x8d7)):uc.reg_write(r,x)

def verify_surface_palette(record=True,original_only=False):
 original=pefile.PE(data=verify_target())
 if not original_only:
  dll=BUILD/'surface_palette_validation.dll';build(dll=dll);compiled=pefile.PE(str(dll));validate_platform_imports(compiled)
 debug=next(d.struct for d in original.DIRECTORY_ENTRY_DEBUG if d.struct.Type==3)
 fpo={r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',original.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
 for r,n,row,digest in ROUTINES.values():assert fpo[r]==row and hashlib.sha256(original.get_data(r,n)).hexdigest()==digest
 coverage={n:{'comparisons':0,'followups':0,'faults':0,'aliases':0} for n in ROUTINES}
 for seed in range(8):
  for src in (ARENA+0x5000,ENTRIES,ENTRIES+1,ENTRIES+4,0x512048,0x512048+4,0x512850,0,0xdead0000):
   for reply in (0,1,0x80004005):
    for palette in (0,OBJ,0xdead0000):
     state=State(seed);state.reply=reply;state.put(PAL,palette)
     sessions=[Session(pe,state,k==0) for k,pe in enumerate((original,) if original_only else (original,compiled))]
     for k,name in enumerate(('Gfx_SurfaceSetPaletteNative','Gfx_SetPaletteNative','Gfx_SurfaceSetPaletteNative')):
      policy=copy.deepcopy(state);result=state.apply(name,(src,));count=sum(x['comparisons'] for x in coverage.values())
      for session in sessions:session.run(name,(src,),state,result,count,copy.deepcopy(policy))
      row=coverage[name];row['comparisons']+=1;row['followups']+=int(k>0);row['faults']+=int(state.fault is not None)
      row['aliases']+=int(ENTRIES<=src<ENTRIES+1024)
      if state.fault:break
 for name in ROUTINES:
  for target in (OBJ,OBJ+16,0,0xdead0000):
   state=State(23);state.put(OBJ+16,PVT);state.read_trigger=ARENA+0x5000+767;state.read_changes=[(PAL,target)]
   sessions=[Session(pe,state,k==0) for k,pe in enumerate((original,) if original_only else (original,compiled))]
   policy=copy.deepcopy(state);result=state.apply(name,(ARENA+0x5000,));count=sum(x['comparisons'] for x in coverage.values())
   for session in sessions:session.run(name,(ARENA+0x5000,),state,result,count,copy.deepcopy(policy))
   coverage[name]['comparisons']+=1;coverage[name]['faults']+=int(state.fault is not None)
 if record:
  for name,(r,*_) in ROUTINES.items():
   for kind in ('compilation','emulation'):
    record_run(r,kind,'pass',inputs=INPUTS,artifact=dll.relative_to(ROOT).as_posix(),cases=coverage[name]['comparisons'] if kind=='emulation' else 0,
     command='uv run tools/verify_surface_palette.py',details={'coverage':coverage[name],
      'scope':'Actual C89 triplet copy/flag retention/SetEntries and real installed entry; independent raw-offset oracle, SDK outputs modeled, postcopy palette pointer reload',
      'limitations':'Arbitrary aliases/reentry/native fault atomicity, full palette rendering/game and original toolchain unverified'})
  native=native_surface_host(dll,'tools/native_palette_probe.c','palette_native',
    '            installed->lpVtbl->Release(installed);','\n            probe_native_palette(image,compiled,side,*draws[side]);',
    ('PASS original real SDK palette','PASS rebuilt real SDK palette'))
  details=native|{'cases':3,'paired_executions':6,'updates':24,'null_guards':48,
   'scope':'Actual paired SDK8bit palette CreatePalette/SetEntries/GetEntries,256 RGB comparisons per update, retained flags0 and actual installer/body/entry plus null guards. Test-owned palette disposed/global restored; windowed0/1/4 constructors and real shutdown.',
   'limitations':'SDK palette object updates only; complete attached palette rendering, startup/menu/input/audio/races and original toolchain unverified'}
  for name,(r,*_) in ROUTINES.items():
   record_run(r,'native','pass',inputs=INPUTS,artifact=dll.relative_to(ROOT).as_posix(),cases=3,
    command='uv run tools/verify_surface_palette.py',details=details|{'routine':name})
  print('PASS native palette:',native['stdout'],flush=True)
 print('PASS palette:',coverage,flush=True)
 return coverage

if __name__=="__main__":
 import sys
 verify_surface_palette(record="--no-record" not in sys.argv)
