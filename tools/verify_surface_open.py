# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Authentic native surface constructor: independent raw-address lifecycle oracle."""
import hashlib
import random
import struct
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE, UC_HOOK_MEM_INVALID
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EFLAGS
from verify_surface_lifecycle import State as SurfaceState, Session as SurfaceSession, BANKS, RECORDS, ARENA, SIZE, VTABLE, ISLOST, RESTORE, LOST, Fault
from verify_matching import STACK, STOP, PRESERVED
from windows_target import ROOT, BUILD, verify_target, validation_symbols
from build_decomp import build
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')
MASK=0xffffffff
INSTANCE,WINDOW,DUPLICATE,MODE=0x4c5398,0x4c539c,0x493738,0x493728
WIDTH,HEIGHT,DEPTH,COUNT,DESKTOP=0x4ba6e0,0x4ba6e4,0x4ba6e8,0x4ba6ec,0x4bac90
DRAW,CLIPPER,PALETTE,ENTRIES=0x512c50,0x4bac8c,0x512448,0x512048
CLASS,TITLE,LIMIT_TEXT,BACK_TEXT=0x493740,0x493778,0x4bacb4,0x4bac94
FIELDS=BANKS+[
 ('g_nativeWindowInstance',INSTANCE,4),('g_nativeWindow',WINDOW,4),('g_nativeSurfaceWindow',DUPLICATE,4),
 ('g_nativeFullscreen',MODE,4),('g_nativeSurfaceWidth',WIDTH,4),('g_nativeSurfaceHeight',HEIGHT,4),
 ('g_nativeSurfaceBitDepth',DEPTH,4),('g_nativeBackbufferCount',COUNT,4),('g_nativeDesktopBitDepth',DESKTOP,4),
 ('g_nativeDirectDraw',DRAW,4),('g_nativeClipper',CLIPPER,4),('g_nativePalette',PALETTE,4),
 ('g_nativePaletteEntries',ENTRIES,1024),('g_nativeWindowClassName',CLASS,9),('g_nativeWindowTitle',TITLE,9),
 ('g_nativeBackbufferLimitMessage',LIMIT_TEXT,46),('g_nativeBackbufferMessage',BACK_TEXT,32)]
DO,CO,PO,SO=ARENA+0x400,ARENA+0x410,ARENA+0x420,ARENA+0x500
API_ARGC={'GetSystemMetrics':1,'CreateWindowExA':12,'UpdateWindow':1,'SetFocus':1,
 'DirectDrawCreate':3,'GetDC':1,'GetDeviceCaps':2,'ReleaseDC':2,'GetWindowLongA':2,
 'SetWindowLongA':3,'SetRect':5,'GetMenu':1,'AdjustWindowRectEx':4,'SetWindowPos':7,
 'SystemParametersInfoA':4,'GetWindowRect':2,'MessageBoxA':4,'ShowWindow':2,
 'SetCooperativeLevel':3,'SetDisplayMode':4,'CreateSurface':4,'GetAttachedSurface':3,
 'CreateClipper':4,'SetHWnd':3,'SetClipper':2,'CreatePalette':5,'SetPalette':2,
 'Release':1,'IsLost':1,'Restore':1}
FUNCTIONS={name:ARENA+0x3000+16*k for k,name in enumerate(API_ARGC)}
VTABLES={'draw':ARENA+0x1800,'clipper':ARENA+0x1900,'palette':ARENA+0x1a00,'surface':VTABLE}
SLOTS={'draw':{8:'Release',0x10:'CreateClipper',0x14:'CreatePalette',0x18:'CreateSurface',0x50:'SetCooperativeLevel',0x54:'SetDisplayMode'},
 'clipper':{8:'Release',0x20:'SetHWnd'},'palette':{8:'Release'},
 'surface':{8:'Release',0x30:'GetAttachedSurface',0x60:'IsLost',0x6c:'Restore',0x70:'SetClipper',0x7c:'SetPalette'}}
DLL=BUILD/'surface_open_validation.dll'
INPUTS=['decomp/src/geputget.c','decomp/include/geputget.h','decomp/src/main.c','decomp/include/main.h',
 'decomp/src/mem.c','decomp/include/mem.h','decomp/src/lisa3d.c','decomp/include/lisa3d.h',
 'decomp/target.json','tools/build_decomp.py','tools/verify_matching.py','tools/verify_surface_lifecycle.py',
 'tools/verify_surface_open.py','tools/native_surface_probe.c','tools/windows_target.py','tools/windows_tracking.py']

def signed(x):return x-0x100000000 if x&0x80000000 else x

def descriptor(**words):
    raw=bytearray(108);struct.pack_into('<I',raw,0,108)
    for offset,value in words.items():struct.pack_into('<I',raw,int(offset[1:]),value&MASK)
    return bytes(raw)

def patch_desc(raw,offset,value):
    raw=bytearray(raw);struct.pack_into('<I',raw,offset,value&MASK);return bytes(raw)

class State(SurfaceState):
    def __init__(self,seed,mode=0,count=1):
        super().__init__(seed)
        rng=random.Random(seed+0x5b740)
        for _,p,n in FIELDS[len(BANKS):]:self.data[p]=bytearray(rng.randbytes(n))
        for p,x in ((INSTANCE,0x19000100),(WINDOW,0x19000200),(DUPLICATE,0x19000300),(MODE,mode),
          (WIDTH,640),(HEIGHT,480),(DEPTH,8),(COUNT,count),(DESKTOP,8),(DRAW,DO),(CLIPPER,CO),(PALETTE,PO)):
            self.put(p,x)
        for p,value in ((CLASS,b'Ignition\0'),(TITLE,b'Ignition\0'),
          (LIMIT_TEXT,b'The maximum amount of backbuffers is exceeded\0'),(BACK_TEXT,b"Backbuffer couldn't be obtained\0")):
            self.data[p][:]=value
        for kind,slots in SLOTS.items():
            for offset,name in slots.items():self.put(VTABLES[kind]+offset,FUNCTIONS[name])
        for obj,kind in ((DO,'draw'),(CO,'clipper'),(PO,'palette')):self.put(obj,VTABLES[kind])
        for k in range(26):self.put(SO+16*k,VTABLES['surface'])
        self.script={};self.seen={};self.surface_number=0
        self.window_style=0x80080000

    def snapshot(self):return tuple(bytes(self.data[p]) for p in [ARENA]+[p for _,p,_ in FIELDS])

    def response(self,name,args):
        index=self.seen.get(name,0);self.seen[name]=index+1
        if name=='GetSystemMetrics':reply=1080 if args[0]==1 else 1920
        elif name=='CreateWindowExA':reply=0x19000400
        elif name=='DirectDrawCreate':reply=0
        elif name=='GetDC':reply=0x19000500
        elif name=='GetDeviceCaps':reply=16 if args[1]==12 else 2
        elif name=='GetWindowLongA':reply=self.window_style if args[1]==0xfffffff0 else 0x40000
        elif name=='GetMenu':reply=0
        elif name in ('SetCooperativeLevel','SetDisplayMode','CreateSurface','GetAttachedSurface','CreateClipper','SetHWnd','SetClipper','CreatePalette','SetPalette','IsLost','Restore','Release'):reply=0
        else:reply=1
        if name=='CreateWindowExA':self.window_style=args[3]
        if name=='SetWindowLongA' and args[1]==0xfffffff0:self.window_style=args[2]
        output=None
        if name=='DirectDrawCreate':output=DO
        elif name=='CreateSurface':
            output=SO+16*self.surface_number;self.surface_number+=1
        elif name=='GetAttachedSurface':output=SO+16*(index+1)
        elif name=='CreateClipper':output=CO
        elif name=='CreatePalette':output=PO
        elif name=='SetRect':output=struct.pack('<4I',*args[1:])
        elif name=='AdjustWindowRectEx':output=struct.pack('<4i',-8,-31,648,488)
        elif name=='SystemParametersInfoA':output=struct.pack('<4i',10,20,1290,1044)
        elif name=='GetWindowRect':output=struct.pack('<4i',0,0,656,519)
        reply,output,mutations=self.script.get((name,index),(reply,output,[]))
        for p,x in mutations:self.put(p,x)
        return reply&MASK,output

    def api(self,name,args):
        self.events.append(('api',name,args,self.snapshot()))
        reply,output=self.response(name,args)
        if name in ('DirectDrawCreate','CreateClipper','CreatePalette'):self.put(args[-2],output) if output is not None else None
        elif name=='CreateSurface':self.put(args[2],output) if output is not None else None
        return reply,output

    def method(self,name,obj,args=()):
        table=self.read(obj,False)
        offset=next(offset for slots in SLOTS.values() for offset,label in slots.items() if label==name)
        assert self.read(table+offset,False)==FUNCTIONS[name]
        return self.api(name,(obj,)+args)

    def initialize(self):
        self.events.append(('entry','Gfx_InitSurfaceRecords',self.snapshot()))
        for kind,(_,p,n) in enumerate(BANKS):
            for base in range(p,p+n,48):
                for offset in range(0,48,4):self.write(base+offset,kind if offset==40 else 0)

    def restore(self):
        self.events.append(('entry','Gfx_SurfaceRestoreNative',self.snapshot()))
        for p in RECORDS:
            if self.read(p)==1:
                if self.method('IsLost',self.read(p+44))[0]==LOST:
                    self.method('Restore',self.read(p+44));self.write(p+4,1)

    def primary(self):
        width=self.read(WIDTH);height=self.read(HEIGHT)
        self.write(RECORDS[0],1);self.write(RECORDS[0]+4,1);self.write(RECORDS[0]+28,width)
        depth=self.read(DEPTH);self.write(RECORDS[0]+32,height);return depth

    def constructor(self):
        self.initialize();instance=self.read(INSTANCE)
        height=self.api('GetSystemMetrics',(1,))[0];width=self.api('GetSystemMetrics',(0,))[0]
        window=self.api('CreateWindowExA',(0x40000,CLASS,TITLE,0x80080000,0,0,width,height,0,0,instance,0))[0]
        self.write(WINDOW,window);self.write(DUPLICATE,window)
        if not window:return 0
        self.api('UpdateWindow',(self.read(WINDOW),));self.api('SetFocus',(self.read(WINDOW),))
        if self.api('DirectDrawCreate',(0,DRAW,0))[0]:return 0
        flags=0x53 if self.read(MODE) else 8
        window=self.read(WINDOW);draw=self.read(DRAW)
        if self.method('SetCooperativeLevel',draw,(window,flags))[0]:return 0
        if self.read(MODE):
            depth=self.read(DEPTH);height=self.read(HEIGHT);width=self.read(WIDTH);draw=self.read(DRAW)
            if self.method('SetDisplayMode',draw,(width,height,depth))[0]:return 0
        else:
            dc=self.api('GetDC',(0,))[0]
            depth=self.api('GetDeviceCaps',(dc,12))[0]*self.api('GetDeviceCaps',(dc,14))[0]&MASK
            self.write(DESKTOP,depth);self.api('ReleaseDC',(0,dc))
            style=self.api('GetWindowLongA',(self.read(WINDOW),0xfffffff0))[0]&0x7fffffff|0xc60000
            self.api('SetWindowLongA',(self.read(WINDOW),0xfffffff0,style))
            rectangle=self.api('SetRect',('rectangle',0,0,640,480))[1]
            extended=self.api('GetWindowLongA',(self.read(WINDOW),0xffffffec))[0]
            menu=int(self.api('GetMenu',(self.read(WINDOW),))[0]!=0)
            style=self.api('GetWindowLongA',(self.read(WINDOW),0xfffffff0))[0]
            rectangle=self.api('AdjustWindowRectEx',(rectangle,style,menu,extended))[1]
            left,top,right,bottom=struct.unpack('<4I',rectangle)
            self.api('SetWindowPos',(self.read(WINDOW),0,0,0,(right-left)&MASK,(bottom-top)&MASK,0x16))
            self.api('SetWindowPos',(self.read(WINDOW),0xfffffffe,0,0,0,0,0x13))
            work=self.api('SystemParametersInfoA',(0x30,0,'work_area',0))[1]
            rectangle=self.api('GetWindowRect',(self.read(WINDOW),'rectangle'))[1]
            left,top,_,_=struct.unpack('<4i',rectangle);wl,wt,_,_=struct.unpack('<4i',work)
            self.api('SetWindowPos',(self.read(WINDOW),0,max(left,wl)&MASK,max(top,wt)&MASK,0,0,0x15))
        desc=descriptor()
        if self.read(MODE):
            desc=descriptor(w4=0x21,w20=self.read(COUNT),w104=0x218);draw=self.read(DRAW)
            if self.method('CreateSurface',draw,(desc,RECORDS[0]+44,0))[0]:return 0
            depth=self.primary();bound=self.read(COUNT);self.write(RECORDS[0]+36,depth)
            if signed(bound)>=5:
                self.api('MessageBoxA',(self.read(WINDOW),LIMIT_TEXT,0,0));return 0
            if signed(self.read(COUNT))>0:
                chain=self.read(RECORDS[0]+44);index=0
                while True:
                    caps=4 if index==0 else 16
                    reply,chain=self.method('GetAttachedSurface',chain,(caps,'chain'))
                    if reply:
                        self.api('MessageBoxA',(self.read(WINDOW),BACK_TEXT,0,0));return 0
                    p=RECORDS[1+index];width=self.read(WIDTH);self.write(p+44,chain);height=self.read(HEIGHT)
                    self.write(p,1);depth=self.read(DEPTH);self.write(p+4,1)
                    self.write(p+28,width);self.write(p+32,height)
                    index+=1;bound=self.read(COUNT);self.write(p+36,depth)
                    if index>=signed(bound):break
        else:
            desc=descriptor(w4=1,w104=0x200);draw=self.read(DRAW)
            if self.method('CreateSurface',draw,(desc,RECORDS[0]+44,0))[0]:return 0
            depth=self.primary();desc=descriptor(w4=7,w8=480,w12=640,w104=64)
            bound=self.read(COUNT);self.write(RECORDS[0]+36,depth)
            if signed(bound)>=5:
                self.api('MessageBoxA',(self.read(WINDOW),LIMIT_TEXT,0,0));return 0
            if signed(self.read(COUNT))>0:
                index=0
                while True:
                    p=RECORDS[1+index];draw=self.read(DRAW)
                    if self.method('CreateSurface',draw,(desc,p+44,0))[0]:return 0
                    self.write(p,1);width=self.read(WIDTH);self.write(p+4,1);height=self.read(HEIGHT)
                    self.write(p+28,width);depth=self.read(DEPTH);self.write(p+32,height);self.write(p+36,depth)
                    index+=1
                    if index>=signed(self.read(COUNT)):break
            draw=self.read(DRAW)
            if self.method('CreateClipper',draw,(0,CLIPPER,0))[0]:return 0
            window=self.read(WINDOW);clip=self.read(CLIPPER)
            if self.method('SetHWnd',clip,(0,window))[0]:return 0
            clip=self.read(CLIPPER);surface=self.read(RECORDS[0]+44)
            if self.method('SetClipper',surface,(clip,))[0]:return 1
        if self.read(MODE):
            if self.read(PALETTE):
                palette=self.read(PALETTE);self.method('Release',palette);self.write(PALETTE,0)
            for k in range(256):
                for channel in range(3):
                    b,o=self.region(ENTRIES+4*k+channel,1);value=0 if k==0 else 255;b[o]=value
                    self.events.append(('write',ENTRIES+4*k+channel,1,value))
            draw=self.read(DRAW)
            if self.method('CreatePalette',draw,(68,ENTRIES,PALETTE,0))[0]:return 0
            palette=self.read(PALETTE);surface=self.read(RECORDS[0]+44)
            if self.method('SetPalette',surface,(palette,))[0]:return 0
        self.restore();self.api('ShowWindow',(self.read(WINDOW),5));return 1

    def apply(self):
        self.events=[];self.fault=None;self.seen={};self.surface_number=0
        try:return self.constructor()
        except Fault:return None

class Session(SurfaceSession):
    field_specs=FIELDS
    def __init__(self,pe,state,original):
        self.uc=Uc(UC_ARCH_X86,UC_MODE_32);self.base=pe.OPTIONAL_HEADER.ImageBase
        self.size=(pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095
        self.uc.mem_map(self.base,self.size);self.uc.mem_write(self.base,pe.get_memory_mapped_image())
        self.uc.mem_map(ARENA,SIZE);self.uc.mem_map(STACK,0x10000);self.uc.mem_map(STOP,4096)
        self.symbols=({name:p for name,p,_ in self.field_specs}|{'Gfx_SurfaceOpenNative':0x45b740,'Gfx_InitSurfaceRecords':0x456a40,'Gfx_SurfaceRestoreNative':0x45c730}) if original else validation_symbols(pe)
        self.fields=[(p,self.symbols[name],n) for name,p,n in self.field_specs]
        for p,b in state.data.items():self.uc.mem_write(self.physical(p),bytes(b))
        for group in pe.DIRECTORY_ENTRY_IMPORT:
            for entry in group.imports:
                name=entry.name.decode() if entry.name else ''
                if name in FUNCTIONS:self.uc.mem_write(entry.address,struct.pack('<I',FUNCTIONS[name]))
        self.uc.hook_add(UC_HOOK_CODE,self.code);self.uc.hook_add(UC_HOOK_MEM_READ|UC_HOOK_MEM_WRITE,self.memory)
        self.uc.hook_add(UC_HOOK_MEM_INVALID,self.invalid)
        self.callbacks={p:name for name,p in FUNCTIONS.items()}

    def snapshot(self):return tuple(bytes(self.uc.mem_read(self.physical(p),n)) for p,n in [(ARENA,SIZE)]+[(p,n) for _,p,n in self.field_specs])

    def code(self,uc,a,n,data):
        for name in ('Gfx_InitSurfaceRecords','Gfx_SurfaceRestoreNative'):
            if a==self.symbols[name]:self.events.append(('entry',name,self.snapshot()))
        if a not in self.callbacks:return
        name=self.callbacks[a];sp=uc.reg_read(UC_X86_REG_ESP)
        raw=struct.unpack('<'+'I'*(API_ARGC[name]+1),uc.mem_read(sp,4*(API_ARGC[name]+1)))
        ret=raw[0];args=list(raw[1:]);canonical=[self.normalize(x) for x in args]
        output_address=None
        if name=='CreateSurface':canonical[1]=bytes(uc.mem_read(args[1],108));output_address=args[2]
        elif name=='GetAttachedSurface':canonical[1]=struct.unpack('<I',uc.mem_read(args[1],4))[0];canonical[2]='chain';output_address=args[2]
        elif name in ('DirectDrawCreate','CreateClipper','CreatePalette'):output_address=args[-2]
        elif name=='SetRect':canonical[0]='rectangle';output_address=args[0]
        elif name=='AdjustWindowRectEx':canonical[0]=bytes(uc.mem_read(args[0],16));output_address=args[0]
        elif name=='SystemParametersInfoA':canonical[2]='work_area';output_address=args[2]
        elif name=='GetWindowRect':canonical[1]='rectangle';output_address=args[1]
        self.events.append(('api',name,tuple(canonical),self.snapshot()))
        reply,output=self.model.response(name,tuple(canonical))
        # response mutates a separate policy state; apply precisely that callback's writes.
        index=self.model.seen[name]-1
        mutations=self.model.script.get((name,index),(None,None,[]))[2]
        for p,x in mutations:uc.mem_write(self.physical(p),struct.pack('<I',x&MASK))
        if output_address is not None and output is not None:
            uc.mem_write(output_address,output if isinstance(output,bytes) else struct.pack('<I',output&MASK))
        for reg,x in ((UC_X86_REG_EAX,reply),(UC_X86_REG_ECX,0xdeadbeef),(UC_X86_REG_EDX,0x12345678),
          (UC_X86_REG_ESP,sp+4+4*API_ARGC[name]),(UC_X86_REG_EIP,ret),(UC_X86_REG_EFLAGS,0x8d7)):
            uc.reg_write(reg,x)

    def run(self,state,result,seed,policy):
        # The reply engine is source-independent and does not execute constructor code.
        self.model=policy;self.model.seen={};self.model.surface_number=0
        self.events=[];self.fault=None;uc=self.uc;sp=STACK+0x8000
        uc.mem_write(sp,struct.pack('<I',STOP));caller=bytes(uc.mem_read(sp,0x8000))
        saved=[random.Random(seed+i).getrandbits(32) for i in range(4)]
        for reg,x in zip(PRESERVED,saved):uc.reg_write(reg,x)
        uc.reg_write(UC_X86_REG_ESP,sp);uc.reg_write(UC_X86_REG_EFLAGS,2)
        before=bytearray(uc.mem_read(self.base,self.size))
        try:uc.emu_start(self.symbols['Gfx_SurfaceOpenNative'],STOP,count=100000)
        except UcError:assert self.fault is not None
        assert self.fault==state.fault,(seed,self.fault,state.fault)
        if self.events!=state.events:
            for k,(x,y) in enumerate(zip(self.events,state.events)):
                if x!=y:raise AssertionError((seed,k,x[:3],y[:3]))
            raise AssertionError((seed,len(self.events),len(state.events)))
        assert self.snapshot()==state.snapshot(),(seed,'state')
        assert bytes(uc.mem_read(sp,0x8000))==caller
        after=bytearray(uc.mem_read(self.base,self.size))
        for _,p,n in self.fields:before[p-self.base:p-self.base+n]=after[p-self.base:p-self.base+n]
        assert before==after,'Unrelated image bytes changed'
        if self.fault is None:
            assert uc.reg_read(UC_X86_REG_EIP)==STOP and uc.reg_read(UC_X86_REG_ESP)==sp+4
            assert [uc.reg_read(r) for r in PRESERVED]==saved
            assert not uc.reg_read(UC_X86_REG_EFLAGS)&0x400
            assert uc.reg_read(UC_X86_REG_EAX)==result

def native_surface(dll,shutdown=False,rebuild=False):
    import shutil
    import subprocess
    from windows_target import TARGET
    runtime=ROOT/'build/runtime/surface_native'
    runtime.mkdir(parents=True,exist_ok=True)
    copied=runtime/'IGN_WIN.EXE'
    shutil.copyfile(TARGET,copied);verify_target(copied)
    shutil.copyfile(dll,runtime/'surface_open_validation.dll')
    gcc,dlltool,linker=[shutil.which(name) for name in ('gcc','llvm-dlltool','lld-link')]
    if not all((gcc,dlltool,linker)):raise RuntimeError('Native SDK probe requires GCC, LLVM dlltool and LLD')
    groups={'KERNEL32':['CreateFileA@28','ReadFile@20','GetFileSize@8','CloseHandle@4',
        'VirtualAlloc@16','LoadLibraryA@4','GetProcAddress@8','GetStdHandle@4',
        'WriteFile@20','ExitProcess@4','GetModuleHandleA@4'],
      'USER32':['LoadCursorA@8','LoadIconA@8','RegisterClassA@4','IsWindow@4','IsWindowVisible@4',
        'GetClientRect@8','GetWindowLongA@8','DestroyWindow@4','UnregisterClassA@8'],
      'GDI32':['GetStockObject@4']}
    libraries=[];commands=[]
    for module,exports in groups.items():
        definition=runtime/(module.lower()+'.def');lib=runtime/(module.lower()+'.lib')
        definition.write_text('LIBRARY '+module+'.dll\nEXPORTS\n'+
            '\n'.join(exports)+'\n',encoding='utf-8',newline='\n')
        lib.unlink(missing_ok=True)
        command=[dlltool,'-m','i386','-d',str(definition),'-l',str(lib),'--kill-at']
        subprocess.run(command,cwd=ROOT,check=True);commands.append(command);libraries.append(str(lib))
    obj,exe=runtime/'native_surface_probe.obj',runtime/'native_surface_probe.exe'
    obj.unlink(missing_ok=True);exe.unlink(missing_ok=True)
    commands.extend([[gcc,'-m32','-std=c89','-pedantic-errors','-Wall','-Wextra','-Werror',
      '-Wno-cast-function-type','-O2','-ffreestanding','-fno-builtin','-mno-sse',
      '-c',str(ROOT/'tools/native_surface_probe.c'),'-o',str(obj)],
      [linker,'/entry:NativeEntry@0','/subsystem:console','/nodefaultlib','/machine:x86',
       '/safeseh:no','/base:0x20000000','/dynamicbase:no','/out:'+str(exe),str(obj)]+libraries])
    if rebuild:commands[-2].insert(1,'-DSURFACE_TEST_REBUILD')
    elif shutdown:commands[-2].insert(1,'-DSURFACE_TEST_SHUTDOWN')
    for command in commands[-2:]:subprocess.run(command,cwd=ROOT,check=True)
    result=subprocess.run([str(exe)],cwd=runtime,capture_output=True,text=True,timeout=45)
    (runtime/'result.log').write_text(result.stdout+result.stderr,encoding='utf-8',newline='\n')
    expected='PASS native Win32/DirectDraw original versus C89 '+('rebuild:' if rebuild else 'shutdown:' if shutdown else 'constructor:')
    if result.returncode or not result.stdout.startswith(expected):
        raise RuntimeError(('Native surface probe failed',result.returncode,result.stdout,result.stderr))
    verify_target(copied)
    assert hashlib.sha256(dll.read_bytes()).digest()==hashlib.sha256((runtime/'surface_open_validation.dll').read_bytes()).digest()
    return {'variant':'rebuild' if rebuild else 'shutdown' if shutdown else 'constructor','cases':3 if rebuild else 4,'paired_executions':6 if rebuild else 8,'lifecycle_invocations':12 if rebuild else 8 if shutdown else 0,'stdout':result.stdout,'exit_code':result.returncode,
      'commands':[subprocess.list2cmdline(command) for command in commands]+[str(exe)],
      'host_sha256':hashlib.sha256(exe.read_bytes()).hexdigest(),
      'scope':('Native original/rebuilt windowed rebuild after actual constructors: 0/1/4 backbuffers, real type2 boundary releases, repeated clearing while primary/type1 references remain alive outside records; actual SDK surface queries prove retention. Real production shutdown follows; test-owned cleanup only releases interfaces intentionally lost from records and disposes HWND. Original WndProc reference boundary.' if rebuild else 'Native constructor plus actual original/rebuilt SDK shutdown: missing-class and registered windowed 0/1/4 backbuffers; real production releases including actual offscreen type2 bank boundaries, retained clipper/markers/geometry, HWND remains until test-owned window disposal. Original WndProc reference boundary; game/input paths prevented.' if shutdown else 'Native Win32/DirectDraw constructor: missing-class failure and registered windowed 0/1/4 backbuffer lifecycles. Actual authenticated original WndProc is an explicit reference class boundary; game/input/shutdown paths prevented, test-owned SDK cleanup.'),
      'limitations':'No rebuilt WndProc/native game/startup/menu/render/input/audio/race parity, fullscreen/display switching or lost-device hardware validation. No instruction equality/original toolchain claim.'}


def inspect_constructor(original,compiled):
    debug=next(entry.struct for entry in original.DIRECTORY_ENTRY_DEBUG if entry.struct.Type==3)
    fpo={r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',original.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    assert fpo[0x5b740]==(1575,37,0,0x140a)
    assert fpo[0x122d0]==(400,0,4,775)
    assert hashlib.sha256(original.get_data(0x122d0,400)).hexdigest()=='461e5b4afef4dd9642c7ce031556749db188c41ea06480eab871723b337452e0'
    assert struct.unpack('<5I',original.get_data(0x1244c,20))==(0x4123d0,0x412319,0x412337,0x41242f,0x412337)
    assert original.get_data(0x7879c,6)==bytes.fromhex('ff259cc26400')
    imports={entry.address:(group.dll.decode(),entry.name.decode() if entry.name else '') for group in original.DIRECTORY_ENTRY_IMPORT for entry in group.imports}
    assert imports[0x64c29c]==('DDRAW.dll','DirectDrawCreate')
    assert imports[0x64c440]==('USER32.dll','CreateWindowExA')
    assert imports[0x64c2c0]==('GDI32.dll','GetDeviceCaps')
    assert original.get_data(0x5bd67,9)==b'\xcc'*9
    symbols=validation_symbols(compiled)
    defaults={'g_nativeFullscreen':1,'g_nativeSurfaceWidth':640,'g_nativeSurfaceHeight':480,
       'g_nativeSurfaceBitDepth':8,'g_nativeBackbufferCount':1,'g_nativeDesktopBitDepth':8}
    original_data=original.get_memory_mapped_image();compiled_data=compiled.get_memory_mapped_image()
    for name,p,n in FIELDS:
        original_bytes=original_data[p-0x400000:p-0x400000+n]
        address=symbols[name]-compiled.OPTIONAL_HEADER.ImageBase
        compiled_bytes=compiled_data[address:address+n]
        # Zero tail fields are materialized by both native/emulator loaders.
        compiled_bytes=compiled_bytes.ljust(n,b'\0')
        original_bytes=original_bytes.ljust(n,b'\0')
        assert compiled_bytes==original_bytes,(name,'initializer bytes')
        if name in defaults:assert int.from_bytes(compiled_bytes,'little')==defaults[name]


def verify_surface_open():
    original=pefile.PE(data=verify_target())
    assert hashlib.sha256(original.get_data(0x5b740,1575)).hexdigest()=='4626e6dbe7c24d021c37141e72e616e63bc43ebdd89cec16ea45012cf98acdc9'
    build(dll=DLL);compiled=pefile.PE(str(DLL));inspect_constructor(original,compiled)
    import copy
    coverage={'comparisons':0,'persistent_followups':0,'faults':0,'mutations':0}
    def sequence(state,repeats=1):
        sessions=[Session(pe,state,k==0) for k,pe in enumerate((original,compiled))]
        for repeat in range(repeats):
            policy=copy.deepcopy(state);result=state.apply()
            for session in sessions:session.run(state,result,coverage['comparisons'],copy.deepcopy(policy))
            coverage['comparisons']+=1;coverage['persistent_followups']+=int(repeat>0)
            coverage['faults']+=int(state.fault is not None)
            coverage['mutations']+=int(any(record[2] and index<state.seen.get(name,0) for (name,index),record in state.script.items()))
            if state.fault:break
    for mode in (0,1,2,MASK):
        for count in (0,1,2,4,5,MASK):sequence(State(coverage['comparisons'],mode,count),2)
    # Failures retain earlier HWND/interfaces/publications. Success-positive
    # HRESULT is still a failure to this constructor; boolean API returns ignored.
    for mode in (0,1):
        names=['CreateWindowExA','DirectDrawCreate','SetCooperativeLevel','CreateSurface']
        names+=['SetDisplayMode','GetAttachedSurface','CreatePalette','SetPalette'] if mode else ['CreateClipper','SetHWnd','SetClipper']
        for name in names:
            for reply in ([0] if name=='CreateWindowExA' else [1,0x80004005,LOST,MASK]):
                state=State(81,mode,4)
                output=DO if name=='DirectDrawCreate' else SO if name in ('CreateSurface','GetAttachedSurface') else CO if name=='CreateClipper' else PO if name=='CreatePalette' else None
                state.script[name,0]=(reply,output,[]);sequence(state,2)
        for index in range(1,4):
            name='GetAttachedSurface' if mode else 'CreateSurface'
            state=State(82,mode,4);state.script[name,index]=(0x80004005,SO+16*index,[]);sequence(state)
    # Live global/configuration and record mutations at actual external calls.
    schedules=[('UpdateWindow',0,[(WINDOW,0x19000600),(INSTANCE,0x19000700)]),
      ('SetFocus',0,[(MODE,1)]),('SetCooperativeLevel',0,[(MODE,0),(WIDTH,800),(HEIGHT,600),(DEPTH,32)]),
      ('SetDisplayMode',0,[(MODE,0)]),('CreateSurface',0,[(WIDTH,321),(HEIGHT,123),(DEPTH,16),(COUNT,2)]),
      ('CreateSurface',1,[(COUNT,1),(WIDTH,400),(HEIGHT,300)]),
      ('GetAttachedSurface',0,[(COUNT,1),(WIDTH,1024),(HEIGHT,768),(DEPTH,32)]),
      ('CreateClipper',0,[(WINDOW,0x19000600),(MODE,1)]),
      ('SetHWnd',0,[(MODE,1)]),('SetClipper',0,[(MODE,1)]),
      ('Release',0,[(PALETTE,PO+16)]),('CreatePalette',0,[(MODE,0),(RECORDS[0],2)]),
      ('SetPalette',0,[(WINDOW,0x19000600),(RECORDS[0]+4,0x11223344)]),
      ('IsLost',0,[(RECORDS[0]+44,SO+16),(RECORDS[1],1)]),
      ('Restore',0,[(WINDOW,0x19000600),(RECORDS[0]+4,0x55667788)])]
    for mode in (0,1):
        for name,index,changes in schedules:
            state=State(83,mode,4)
            args=(0,) if name=='GetSystemMetrics' else (DO,12) if name=='GetDeviceCaps' else (SO,) if name in ('IsLost','Restore','Release') else ()
            reply,output=state.response(name,args)
            state.seen={};state.surface_number=0
            if name=='IsLost':reply=LOST
            if name=='Restore':state.script['IsLost',0]=(LOST,None,[])
            state.script[name,index]=(reply,output,changes);sequence(state,2)
    # Window calculation wraps at 32 bits; geometry comparisons remain signed.
    for bits,planes in ((0,0),(32,1),(MASK,2),(0x80000000,4),(0x7fffffff,3)):
        state=State(84);state.script['GetDeviceCaps',0]=(bits,None,[]);state.script['GetDeviceCaps',1]=(planes,None,[]);sequence(state)
    for rectangle in ((-100,-200,400,300),(100,200,700,680),(0x7fffffff,-0x80000000,-1,0x7fffffff)):
        state=State(85);state.script['AdjustWindowRectEx',0]=(0,struct.pack('<4i',*rectangle),[])
        state.script['GetWindowRect',0]=(0,struct.pack('<4i',*rectangle),[]);sequence(state)
    for name in ('UpdateWindow','SetFocus','ReleaseDC','SetWindowLongA','SetWindowPos','MessageBoxA','ShowWindow'):
        state=State(86,0,5 if name=='MessageBoxA' else 1);state.script[name,0]=(0,None,[]);sequence(state)
    for boundary,output in (('DirectDrawCreate',0),('DirectDrawCreate',0x80000000),('CreateSurface',0),('CreateClipper',0),('CreatePalette',0)):
        state=State(87,int(boundary=='CreatePalette'),1);state.script[boundary,0]=(0,output,[]);sequence(state)
    print('PASS constructor contracts:',coverage,flush=True)
    native=native_surface(DLL)
    print('PASS constructor native:',native,flush=True)
    details={'scope':'Complete production C89 constructor with real record initializer/restoration and actual SDK API/COM imports; Win32/COM replies explicitly modeled in differential emulation',
      'coverage':coverage,'checks':'Authenticated FPO/extents/hashes, actual import thunk/name binding, WndProc/class provenance and source/DLL data initializers. Independent raw-address oracle; ordered reads/writes, COM/API arguments/descriptor bytes/callback snapshots, full banks/globals/palette/arena/unrelated image, caller stack/nonvolatile/DF and stdcall cleanup; persistent calls, ignored BOOL/Restore results, retained failures, SetClipper failure-success path, exact HRESULTs, callback mutations and 32-bit geometry wrapping.',
      'limitations':'No instruction equality/original compiler/link-layout or complete native game parity. Native scope below uses authenticated original WndProc as reference boundary, not rebuilt procedure/startup. Fullscreen/display switching/lost-device hardware, arbitrary reentry/concurrency/unknown adjacent aliases/out-of-bank live count growth and native fault frames/atomicity remain unverified.'}
    for kind in ('compilation','emulation'):
        record_run(0x5b740,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
          cases=coverage['comparisons'] if kind=='emulation' else 0,
          command='uv run python tools/verify_surface_open.py',details=details)
    record_run(0x5b740,'native','pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),cases=4,
      command='uv run python tools/verify_surface_open.py',details=native)
    return coverage

if __name__=='__main__':verify_surface_open()
