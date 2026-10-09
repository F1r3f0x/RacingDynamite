# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Complete real primitive initialization; raw-address oracle and native sine."""
import hashlib
import json
import random
import shutil
import struct
import subprocess
import pefile
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
from unicorn import Uc,UcError,UC_ARCH_X86,UC_MODE_32,UC_HOOK_CODE,UC_HOOK_MEM_READ,UC_HOOK_MEM_WRITE,UC_HOOK_MEM_INVALID,UC_MEM_WRITE,UC_MEM_READ_UNMAPPED,UC_MEM_WRITE_UNMAPPED
from unicorn.x86_const import UC_X86_REG_ESP,UC_X86_REG_EIP,UC_X86_REG_EAX,UC_X86_REG_ECX,UC_X86_REG_EDX,UC_X86_REG_EFLAGS,UC_X86_REG_FPCW,UC_X86_REG_FPSW
from verify_matching import STACK,STOP,PRESERVED
from windows_target import ROOT,BUILD,TARGET,verify_target,validation_symbols
from windows_tracking import record_run
from build_decomp import build

if not __debug__:
    raise RuntimeError('Verification requires assertions')
MASK=0xffffffff
ARENA,SIZE=0x3000000,0x10000
EMPTY,PRIMARY,NAMED,SECONDARY,LINE,RECORD,AUX,IMAGE=0x51fd00,0x51fe98,0x5203b8,0x520360,0x51fe30,0x51fc88,0x5203a0,0x520380
PAIR,NAMED_PAIR,DEFAULT,WORDS,SINE,SPRITE,PACK,BUCKETS,HEAD,LITERAL=0x51fc20,0x520370,0x51aae0,0x51fbc8,0x51bb60,0x51fb88,0x51fc00,0x51ff58,0x51fe40,0x4bacf8
CONSTANTS=[(0x47af48,0.000244140625),(0x47af50,6.283192),(0x47af58,2147418112.0)]
FIELDS=[('g_nativeEmptyControl',EMPTY,16),('g_nativePrimitiveControl',PRIMARY,16),
 ('g_nativeNamedControl',NAMED,16),('g_nativeSecondaryControl',SECONDARY,16),
 ('g_nativeLineControl',LINE,16),('g_nativePrimitiveRecordControl',RECORD,16),
 ('g_nativeAuxiliaryControl',AUX,16),('g_nativeImageControl',IMAGE,16),
 ('g_nativeGraphicsPairStorage',PAIR,16),('g_nativeNamedPairStorage',NAMED_PAIR,16),
 ('g_nativeNamedPrimitiveDefault',DEFAULT,37),('g_nativeSecondaryDefaultWords',WORDS,44),
 ('g_nativeSineTable',SINE,16384),('g_nativeSpriteDefault',SPRITE,64),
 ('g_spritePackingTemplate',PACK,24),('g_spritePackingBuckets',BUCKETS,1028),
 ('g_spritePackingPages',HEAD,4),('g_nativeSpriteDefaultName',LITERAL,8),
 ('g_nativeSineStep',CONSTANTS[0][0],8),('g_nativeSineFullCircle',CONSTANTS[1][0],8),('g_nativeSineAmplitude',CONSTANTS[2][0],8)]
POINTERS={p+12 for _,p,n in FIELDS if n==16}|{DEFAULT,DEFAULT+33,SPRITE,PACK+8,PACK+12,PACK+16,PACK+20,HEAD}|set(range(BUCKETS,BUCKETS+1028,4))
ROUTINES={
 'Gfx_InitPrimitiveState':(0x5e610,326,'fd2abfef167ef5b92074ef41c17478dd955591025afb3aad714da050a29704a3',0),
 'Gfx_InitGraphicsPairStorage':(0x5f490,14,'a1a3b1021d987df71e310755f2adbd67749fa172443f0c85a583158118583f38',0),
 'Gfx_InitSineTable':(0x60740,60,'d674e3df6ab18f1b8a699a6d69fdc82cf9ec6549914e46e23fe819ca7a6ca6f6',0),
 'Gfx_EnablePrimitiveControl':(0x607c0,11,'8c8710963f05a7f5a62074f2d9d1903f56311a193ee2af96aaa569f1d781e89d',0),
 'Gfx_InitSecondaryDefaultWords':(0x5f640,68,'c3b41d7b766be5723dc8350191a818ff5badf66130e2700298ed2b4a49286c6f',0),
 'Gfx_InitPrimitiveLineControl':(0x5fa50,11,'ac413b1076087447011efe2202591091027f7595c3cfe38b06960dffc90a86d2',0),
 'Gfx_InitNamedPrimitiveDefault':(0x60b50,130,'2fc72c837750bc901796befe065be5c30a194bdd09a26ff4c50f9cc21dfcd7d7',0),
 'Gfx_InitPairStorageControl':(0x63ab0,72,'771c561d8b04a3859a6ac714e9ef6660465b32fd1c776a2aefc97f62fe509d61',1)}
CHILDREN={'Gfx_AllocBytes':(0x5f4a0,1),'Gfx_InitDefaultSpriteDescriptor':(0x611d0,0),'Gfx_InitSpritePackingState':(0x614d0,0)}
DLL=BUILD/'primitive_validation.dll'
INPUTS=['decomp/src/geputget.c','decomp/include/geputget.h','decomp/src/mem.c','decomp/include/mem.h','decomp/src/lisa3d.c','decomp/include/lisa3d.h','decomp/target.json','tools/build_decomp.py','tools/verify_matching.py','tools/verify_primitive_init.py','tools/native_sine_probe.c','tools/windows_target.py','tools/windows_tracking.py']


class Fault(Exception):
    pass


class State:
    def __init__(self,seed):
        rng=random.Random(seed)
        self.data={p:bytearray(rng.randbytes(n)) for _,p,n in FIELDS}
        self.data[ARENA]=bytearray(rng.randbytes(SIZE))
        self.pointers=set(POINTERS)
        for p in POINTERS:
            self.put(p,ARENA+0x7000)
        self.data[LITERAL][:]=b'default\0'
        for p,x in CONSTANTS:
            self.data[p][:]=struct.pack('<d',x)
        self.events,self.fault,self.replies=[],None,[]
        self.schedule,self.seen={},{}

    def region(self,a,n):
        a &= MASK
        for p,b in self.data.items():
            if p<=a and a+n<=p+len(b):
                return b,a-p
        return None,None

    def put(self,a,x,n=4):
        b,o=self.region(a,n)
        assert b is not None,(hex(a),n)
        b[o:o+n]=(x&((1<<(8*n))-1)).to_bytes(n,'little')

    def snapshot(self):
        return tuple(bytes(self.data[p]) for p in [ARENA]+[p for _,p,_ in FIELDS])

    def read(self,a,n=4):
        a &= MASK;b,o=self.region(a,n)
        if b is None:
            self.fault=('read',a,n);raise Fault
        x=int.from_bytes(b[o:o+n],'little')
        self.events.append(('read',a,n,x))
        self.seen[a]=self.seen.get(a,0)+1
        for dest,word in self.schedule.get((a,self.seen[a]),()):
            self.put(dest,word)
        return x

    def write(self,a,x,n=4):
        a &= MASK;b,o=self.region(a,n)
        if b is None:
            self.fault=('write',a,n);raise Fault
        self.put(a,x,n);self.events.append(('write',a,n,x&((1<<(8*n))-1)))

    def entry(self,name,args=()):
        self.events.append(('entry',name,args,self.snapshot()))

    def allocate(self,n):
        self.entry('Gfx_AllocBytes',(n,))
        self.events.append(('malloc',n,self.snapshot()))
        a,mutations=self.replies.pop(0)
        for dest,word in mutations:
            self.put(dest,word)
        return a

    def call(self,name,args=(),top=False):
        if not top:
            self.entry(name,args)
        if name=='Gfx_InitPairStorageControl':
            p=args[0];self.pointers.add((p+12)&MASK)
            self.write(p+12,self.allocate(128))
            for offset in range(0,128,8):
                a=self.read(p+12);self.write(a+offset,0)
                a=self.read(p+12);self.write(a+offset+4,0)
            self.write(p+8,15);self.write(p+4,0);self.write(p,16)
            return a
        if name=='Gfx_InitGraphicsPairStorage':
            return self.call('Gfx_InitPairStorageControl',(PAIR,))
        if name=='Gfx_InitSineTable':
            values=self.sine_values
            for k,x in enumerate(values):
                for a,_ in CONSTANTS:
                    self.read(a,8)
                self.write(SINE+4*k,x)
            return values[-1]
        if name=='Gfx_EnablePrimitiveControl':
            self.write(PRIMARY,1);return None
        if name=='Gfx_InitPrimitiveLineControl':
            self.write(LINE,8);return None
        if name=='Gfx_InitSecondaryDefaultWords':
            for o in (0,8,4,12,16,24,20,28,32,36):
                self.write(WORDS+o,0)
            self.write(SECONDARY,1);self.write(WORDS+40,0);return 0
        if name=='Gfx_InitNamedPrimitiveDefault':
            self.call('Gfx_InitPairStorageControl',(NAMED_PAIR,))
            for a,x,n in ((DEFAULT,LITERAL,4),(DEFAULT+12,0x3f800000,4),
                (DEFAULT+28,0,4),(DEFAULT+4,0,4),(DEFAULT+32,0,1),
                (DEFAULT+24,0x3ecccccd,4),(DEFAULT+20,0x3f000000,4),(DEFAULT+16,0x3dcccccd,4)):
                self.write(a,x,n)
            a=self.allocate(16);self.write(DEFAULT+33,a)
            for o,x in ((0,12),(4,0),(8,4095),(12,0)):
                self.write(a+o,x)
            self.write(NAMED,1);return a
        if name=='Gfx_InitDefaultSpriteDescriptor':
            self.write(SPRITE,LITERAL)
            for o in range(4,28,4):self.write(SPRITE+o,0)
            self.write(IMAGE,1);return 0
        if name=='Gfx_InitSpritePackingState':
            for o in (16,20,12,0,4,8):self.write(PACK+o,0)
            for o in range(0,1028,4):self.write(BUCKETS+o,0)
            self.write(HEAD,0);return 0
        assert name=='Gfx_InitPrimitiveState'
        for o in range(0,16,4):self.write(EMPTY+o,0)
        for p in (PRIMARY,NAMED,SECONDARY,LINE,RECORD,AUX):
            self.write(p,self.read(EMPTY));self.write(p+4,self.read(EMPTY+4))
            x=self.read(EMPTY+8);y=self.read(EMPTY+12)
            self.write(p+8,x)
            if p==AUX:next_word=self.read(EMPTY)
            self.write(p+12,y)
        self.write(IMAGE,next_word);self.write(IMAGE+4,self.read(EMPTY+4))
        x=self.read(EMPTY+8);y=self.read(EMPTY+12)
        self.write(IMAGE+8,x);self.write(IMAGE+12,y)
        for child in ('Gfx_InitGraphicsPairStorage','Gfx_InitSineTable',
            'Gfx_InitDefaultSpriteDescriptor','Gfx_InitSpritePackingState',
            'Gfx_EnablePrimitiveControl','Gfx_InitSecondaryDefaultWords','Gfx_InitPrimitiveLineControl'):
            self.call(child)
        return self.call('Gfx_InitNamedPrimitiveDefault')

    def apply(self,name,args,replies,values,schedule):
        self.events,self.fault,self.replies=[],None,list(replies)
        self.sine_values,self.schedule,self.seen=values,schedule or {},{}
        if args:self.pointers.add((args[0]+12)&MASK)
        try:return self.call(name,args,True)
        except Fault:return None


class Session:
    def __init__(self,pe,state,original):
        self.uc=Uc(UC_ARCH_X86,UC_MODE_32);self.base=pe.OPTIONAL_HEADER.ImageBase
        self.size=(pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095
        self.uc.mem_map(self.base,self.size);self.uc.mem_write(self.base,pe.get_memory_mapped_image())
        self.uc.mem_map(ARENA,SIZE);self.uc.mem_map(STACK,0x10000);self.uc.mem_map(STOP,4096)
        self.symbols=({name:0x400000+r for name,(r,*_) in ROUTINES.items()}|
            {name:0x400000+r for name,(r,_) in CHILDREN.items()}|
            {name:p for name,p,_ in FIELDS}|{'malloc':0x469400}) if original else validation_symbols(pe)
        self.fields=[(p,self.symbols[name],n) for name,p,n in FIELDS]
        self.pointers=set(state.pointers)
        self.original=original
        for logical,b in state.data.items():
            raw=bytearray(b)
            for p in self.pointers:
                if logical<=p and p+4<=logical+len(raw):
                    x=struct.unpack_from('<I',raw,p-logical)[0]
                    struct.pack_into('<I',raw,p-logical,self.physical(x))
            self.uc.mem_write(self.physical(logical),bytes(raw))
        self.uc.hook_add(UC_HOOK_CODE,self.code)
        self.uc.hook_add(UC_HOOK_MEM_READ|UC_HOOK_MEM_WRITE,self.memory)
        self.uc.hook_add(UC_HOOK_MEM_INVALID,self.invalid)
        self.entries={self.symbols[name]:(name,argc) for name,(*_,argc) in ROUTINES.items()}
        self.entries.update({self.symbols[name]:(name,argc) for name,(_,argc) in CHILDREN.items()})

    def physical(self,a):
        for l,p,n in self.fields:
            if l<=a<l+n:return p+a-l
        return a&MASK

    def normalize(self,a):
        for l,p,n in self.fields:
            if p<=a<p+n:return l+a-p
        return a&MASK

    def snapshot(self):
        pieces=[]
        for l,p,n in [(ARENA,ARENA,SIZE)]+self.fields:
            b=bytearray(self.uc.mem_read(p,n))
            for a in self.pointers:
                if l<=a and a+4<=l+n:
                    x=struct.unpack_from('<I',b,a-l)[0]
                    struct.pack_into('<I',b,a-l,self.normalize(x))
            pieces.append(bytes(b))
        return tuple(pieces)

    def memory(self,uc,access,a,n,x,data):
        if STACK<=a<STACK+0x10000:return
        valid=(ARENA<=a and a+n<=ARENA+SIZE or any(p<=a and a+n<=p+z for _,p,z in self.fields))
        if not valid:
            if access==UC_MEM_WRITE or not self.base<=a<self.base+self.size:return
        logical=self.normalize(a)
        kind='write' if access==UC_MEM_WRITE else 'read'
        if kind=='read':x=int.from_bytes(uc.mem_read(a,n),'little')
        if n==4 and logical in self.pointers:x=self.normalize(x)
        self.events.append((kind,logical,n,x&((1<<(n*8))-1)))
        if kind=='read':
            self.seen[logical]=self.seen.get(logical,0)+1
            self.pending.extend(self.schedule.get((logical,self.seen[logical]),()))

    def invalid(self,uc,access,a,n,x,data):
        assert access in (UC_MEM_READ_UNMAPPED,UC_MEM_WRITE_UNMAPPED)
        self.fault=('read' if access==UC_MEM_READ_UNMAPPED else 'write',self.normalize(a),n)
        return False

    def code(self,uc,a,n,data):
        for dest,word in self.pending:
            value=self.physical(word) if dest in self.pointers else word
            uc.mem_write(self.physical(dest),struct.pack('<I',value&MASK))
        self.pending=[]
        if a in self.entries and a!=self.top:
            name,argc=self.entries[a];sp=uc.reg_read(UC_X86_REG_ESP)
            args=tuple(self.normalize(x) for x in struct.unpack('<'+'I'*argc,uc.mem_read(sp+4,4*argc))) if argc else ()
            if name=='Gfx_InitPairStorageControl':self.pointers.add((args[0]+12)&MASK)
            self.events.append(('entry',name,args,self.snapshot()))
        if a==self.symbols['malloc']:
            sp=uc.reg_read(UC_X86_REG_ESP);ret,size=struct.unpack('<II',uc.mem_read(sp,8))
            self.events.append(('malloc',size,self.snapshot()))
            allocation,mutations=self.replies.pop(0)
            for dest,word in mutations:
                value=self.physical(word) if dest in self.pointers else word
                uc.mem_write(self.physical(dest),struct.pack('<I',value&MASK))
            for reg,value in ((UC_X86_REG_EAX,self.physical(allocation)),(UC_X86_REG_ECX,0xbadc0ffe),
                (UC_X86_REG_EDX,0x87654321),(UC_X86_REG_EFLAGS,0x8d7),(UC_X86_REG_ESP,sp+4),(UC_X86_REG_EIP,ret)):
                uc.reg_write(reg,value)

    def run(self,name,args,replies,expected,result,cw,seed,schedule=None):
        uc=self.uc;sp=STACK+0x8000
        self.top=self.symbols[name];self.events,self.fault,self.replies=[],None,list(replies)
        self.schedule,self.seen,self.pending=schedule or {},{},[]
        if args:self.pointers.add((args[0]+12)&MASK)
        uc.mem_write(sp,struct.pack('<'+'I'*(len(args)+1),STOP,*[self.physical(x) for x in args]))
        caller=bytes(uc.mem_read(sp,0x8000));saved=[random.Random(seed+k).getrandbits(32) for k in range(4)]
        for reg,x in zip(PRESERVED,saved):uc.reg_write(reg,x)
        uc.reg_write(UC_X86_REG_ESP,sp);uc.reg_write(UC_X86_REG_EFLAGS,2);uc.reg_write(UC_X86_REG_FPCW,cw)
        before=bytearray(uc.mem_read(self.base,self.size))
        try:uc.emu_start(self.top,STOP,count=1000000)
        except UcError:assert self.fault is not None
        assert self.fault==expected.fault,(name,seed,self.fault,expected.fault)
        if self.events!=expected.events:
            for k,(a,b) in enumerate(zip(self.events,expected.events)):
                if a!=b:raise AssertionError((name,seed,k,a[:3],b[:3]))
            raise AssertionError((name,seed,len(self.events),len(expected.events)))
        assert self.snapshot()==expected.snapshot(),(name,seed,'state')
        assert self.replies==expected.replies
        assert bytes(uc.mem_read(sp,0x8000))==caller
        after=bytearray(uc.mem_read(self.base,self.size))
        for _,p,n in self.fields:before[p-self.base:p-self.base+n]=after[p-self.base:p-self.base+n]
        assert before==after,'Unrelated image bytes changed'
        if self.fault is None:
            assert uc.reg_read(UC_X86_REG_EIP)==STOP and uc.reg_read(UC_X86_REG_ESP)==sp+4
            assert [uc.reg_read(r) for r in PRESERVED]==saved
            assert not uc.reg_read(UC_X86_REG_EFLAGS)&0x400
            assert uc.reg_read(UC_X86_REG_FPCW)==cw
            assert (uc.reg_read(UC_X86_REG_FPSW)>>11)&7==0
            if result is not None:
                got=uc.reg_read(UC_X86_REG_EAX)
                if name not in ('Gfx_InitSineTable','Gfx_InitSecondaryDefaultWords'):got=self.normalize(got)
                assert got==result,(name,hex(got),hex(result))


def sine_reference(pe,cw):
    # Primary executable executes its actual x87 instructions/private converter.
    # This source-independent result supplies values to the structural oracle.
    uc=Uc(UC_ARCH_X86,UC_MODE_32);base=pe.OPTIONAL_HEADER.ImageBase
    uc.mem_map(base,(pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095);uc.mem_write(base,pe.get_memory_mapped_image())
    uc.mem_map(STACK,0x10000);uc.mem_map(STOP,4096);sp=STACK+0x8000
    uc.mem_write(sp,struct.pack('<I',STOP));uc.reg_write(UC_X86_REG_ESP,sp);uc.reg_write(UC_X86_REG_FPCW,cw)
    uc.emu_start(0x460740,STOP,count=1000000)
    assert uc.reg_read(UC_X86_REG_EIP)==STOP and uc.reg_read(UC_X86_REG_FPCW)==cw
    return struct.unpack('<4096I',uc.mem_read(SINE,16384))


def inspect(pe):
    debug=next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type==3)
    fpo={r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    for name,(r,n,digest,argc) in ROUTINES.items():
        raw=pe.get_data(r,n);assert hashlib.sha256(raw).hexdigest()==digest
        assert fpo[r][0]==n and fpo[r][2]==argc
        assert sum(i.size for i in md.disasm(raw,r+0x400000))==n
    calls=[(i.address,i.op_str) for i in md.disasm(pe.get_data(0x5e610,326),0x45e610) if i.mnemonic in ('call','jmp')]
    assert calls==[(0x45e72e,'0x45f490'),(0x45e733,'0x460740'),(0x45e738,'0x4611d0'),(0x45e73d,'0x4614d0'),(0x45e742,'0x4607c0'),(0x45e747,'0x45f640'),(0x45e74c,'0x45fa50'),(0x45e751,'0x460b50')]
    for p,x in CONSTANTS:assert pe.get_data(p-0x400000,8)==struct.pack('<d',x)
    assert pe.get_data(LITERAL-0x400000,8)==b'default\0'
    assert hashlib.sha256(pe.get_data(0x6950c,39)).hexdigest()=='d0a2aca68648a0ae9d5cbfae1b276d4a64d4f797fc0c60f28b64096cdfa3148c'
    print('Authenticated complete primitive root/eight extents, calls, constant/layout accesses and real private converter.',flush=True)


def native_sine(dll):
    runtime=ROOT/'build/runtime/primitive_native'
    runtime.mkdir(parents=True,exist_ok=True)
    copied=runtime/'IGN_WIN.EXE'
    shutil.copyfile(TARGET,copied)
    verify_target(copied)
    shutil.copyfile(dll,runtime/'sine_validation.dll')
    definition=runtime/'kernel32.def'
    definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+
        '\n'.join(['CreateFileA@28','ReadFile@20','GetFileSize@8','CloseHandle@4',
        'VirtualAlloc@16','LoadLibraryA@4','GetProcAddress@8','GetStdHandle@4',
        'WriteFile@20','ExitProcess@4'])+'\n',encoding='utf-8',newline='\n')
    gcc,dlltool,linker=[shutil.which(x) for x in ('gcc','llvm-dlltool','lld-link')]
    if not all((gcc,dlltool,linker)):
        raise RuntimeError('Native Win32 sine verification requires installed GCC, LLVM dlltool and LLD')
    lib,obj,exe=runtime/'kernel32.lib',runtime/'native_sine_probe.obj',runtime/'native_sine_probe.exe'
    for path in (lib,obj,exe):path.unlink(missing_ok=True)
    commands=[
        [dlltool,'-m','i386','-d',str(definition),'-l',str(lib),'--kill-at'],
        [gcc,'-m32','-std=c89','-pedantic-errors','-Wall','-Wextra','-Werror',
         '-Wno-cast-function-type','-O2','-ffreestanding','-fno-builtin','-mno-sse','-mfpmath=387',
         '-c',str(ROOT/'tools/native_sine_probe.c'),'-o',str(obj)],
        [linker,'/entry:NativeEntry@0','/subsystem:console','/nodefaultlib','/machine:x86',
         '/safeseh:no','/base:0x20000000','/dynamicbase:no','/out:'+str(exe),str(obj),str(lib)]]
    for command in commands:subprocess.run(command,cwd=ROOT,check=True)
    result=subprocess.run([str(exe)],cwd=runtime,capture_output=True,text=True,check=True)
    (runtime/'result.log').write_text(result.stdout+result.stderr,encoding='utf-8',newline='\n')
    assert result.stdout.startswith('PASS native Win32 original versus pure C89 sine probe: 12 masked control modes, 4096 entries each;')
    verify_target(copied)
    assert hashlib.sha256(dll.read_bytes()).digest()==hashlib.sha256((runtime/'sine_validation.dll').read_bytes()).digest()
    return {'cases':12,'entries_per_case':4096,'exit_code':result.returncode,
        'stdout':result.stdout,'host_sha256':hashlib.sha256(exe.read_bytes()).hexdigest(),
        'commands':[subprocess.list2cmdline(x) for x in commands]+[str(exe)],
        'scope':'Native Win32 isolated sine only, actual original sine/private ftol execute in mapped existing-HIGHLOW-relocated runtime PE; production import-free DLL sine executes; real Win32 API/msvcrt control/clear host; no game startup/import resolution',
        'limitations':'Empty x87 stack, masked12 precision/round modes, finite inputs; no unmasked exceptions, FPSW/nonempty stack, native heap, complete startup/menu/render/input/audio/race or original compiler/link layout certification'}


def verify_primitive_init():
    original=pefile.PE(data=verify_target());inspect(original);build(dll=DLL);compiled=pefile.PE(str(DLL))
    values={cw:sine_reference(original,cw) for pc in (0,0x200,0x300) for rounding in (0,0x400,0x800,0xc00) for cw in [0x7f|pc|rounding]}
    coverage={name:{'comparisons':0,'persistent_followups':0,'faults':0,'aliases':0,'mutations':0} for name in ROUTINES}
    def sequence(state,name,calls,cw=0x37f,alias=False):
        sessions=[Session(pe,state,k==0) for k,pe in enumerate((original,compiled))]
        for k,(args,replies,schedule) in enumerate(calls):
            result=state.apply(name,args,replies,values[cw],schedule)
            for session in sessions:session.run(name,args,replies,state,result,cw,sum(x['comparisons'] for x in coverage.values()),schedule)
            c=coverage[name];c['comparisons']+=1;c['persistent_followups']+=int(k>0);c['faults']+=int(state.fault is not None);c['aliases']+=int(alias);c['mutations']+=int(bool(schedule) or any(x[1] for x in replies))
    for cw in values:
        sequence(State(cw),'Gfx_InitSineTable',[((),[],{}),((),[],{})],cw)
    p=ARENA+0x100;buffer=ARENA+0x1000
    for seed in range(12):
        state=State(seed);state.put(p+12,ARENA+0x7000)
        sequence(state,'Gfx_InitPairStorageControl',[((p,),[(buffer,[])],{}),((p,),[(buffer+256,[])],{})])
        for name in ('Gfx_EnablePrimitiveControl','Gfx_InitSecondaryDefaultWords','Gfx_InitPrimitiveLineControl'):
            sequence(State(seed),name,[((),[],{}),((),[],{})])
        sequence(State(seed),'Gfx_InitGraphicsPairStorage',[((),[(buffer,[])],{}),((),[(buffer+256,[])],{})])
        sequence(State(seed),'Gfx_InitNamedPrimitiveDefault',[((),[(buffer,[]),(buffer+256,[])],{}),((),[(buffer+512,[]),(buffer+768,[])],{})])
    # Real full chain and repeated initialization: three allocations per invocation.
    for cw in values:
        sequence(State(cw),'Gfx_InitPrimitiveState',[((),[(buffer,[]),(buffer+256,[]),(buffer+512,[])],{}),((),[(buffer+1024,[]),(buffer+1280,[]),(buffer+1536,[])],{})],cw)
    # Generic clear publication/read order and self-corruption aliases.
    for allocation in (p-116,p-112,p-4,p,p+4,p+8,p+12,PAIR,PAIR+4,NAMED_PAIR):
        packet=PAIR if allocation in (PAIR,PAIR+4) else NAMED_PAIR if allocation==NAMED_PAIR else p
        state=State(44);state.put(packet+12,buffer)
        sequence(state,'Gfx_InitPairStorageControl',[((packet,),[(allocation,[])],{})],alias=True)
    for packet in (PAIR,NAMED_PAIR,DEFAULT,DEFAULT+4):
        sequence(State(45),'Gfx_InitPairStorageControl',[((packet,),[(buffer,[])],{})],alias=True)
    # Captured control pointer, publication overwrites malloc mutations, then
    # every clear reloads the live pointer independently.
    state=State(46);state.put(p+12,buffer)
    sequence(state,'Gfx_InitPairStorageControl',[((p,),[(buffer,[(p,0x11223344),(p+8,0x98765432),(p+12,buffer+512)])],{})])
    for occurrence in (1,2,3,16,31,32):
        state=State(47);state.put(p+12,buffer)
        sequence(state,'Gfx_InitPairStorageControl',[((p,),[(buffer,[])],{(p+12,occurrence):[(p+12,buffer+512)]})])
    for allocation in (DEFAULT,DEFAULT+4,DEFAULT+16,buffer):
        sequence(State(48),'Gfx_InitNamedPrimitiveDefault',[((),[(buffer+1024,[]),(allocation,[])],{})],alias=True)
    sequence(State(49),'Gfx_InitNamedPrimitiveDefault',[((),[(buffer,[]),(buffer+512,[(DEFAULT+8,0x99887766),(DEFAULT+32,9),(NAMED,123)])],{})])
    # Template mutation after reads must preserve captured words and affect later
    # live reads; the source remains a real globally owned packet.
    for address,occurrence,destination,value in ((EMPTY,3,EMPTY+4,7),
            (EMPTY+12,6,EMPTY,123),(EMPTY+8,2,EMPTY+12,buffer+4096)):
        sequence(State(50),'Gfx_InitPrimitiveState',[((),[(buffer,[]),(buffer+256,[]),(buffer+512,[])],{(address,occurrence):[(destination,value)]})])
    # Allocation-boundary changes can retarget metadata/default words. They are
    # retained or overwritten in instruction order; no fake initializer is used.
    sequence(State(51),'Gfx_InitPrimitiveState',[((),[(buffer,[(NAMED+4,17)]),(buffer+256,[(DEFAULT+8,0x12345678)]),(buffer+512,[(NAMED+8,0x80000000)])],{})])
    bad=0x80000000
    for packet in (0,bad,ARENA+SIZE-4,ARENA+SIZE-16):
        sequence(State(52),'Gfx_InitPairStorageControl',[((packet,),[(buffer,[])],{})])
    for allocation in (0,bad,ARENA+SIZE-4,ARENA+SIZE-124):
        sequence(State(53),'Gfx_InitPairStorageControl',[((p,),[(allocation,[])],{})])
        sequence(State(54),'Gfx_InitGraphicsPairStorage',[((),[(allocation,[])],{})])
    for allocation in (0,bad,ARENA+SIZE-4,ARENA+SIZE-12):
        sequence(State(55),'Gfx_InitNamedPrimitiveDefault',[((),[(buffer,[]),(allocation,[])],{})])
    for phase in range(3):
        for allocation in (0,bad,ARENA+SIZE-4):
            replies=[(buffer,[]),(buffer+256,[]),(buffer+512,[])]
            replies[phase]=(allocation,[])
            sequence(State(56),'Gfx_InitPrimitiveState',[((),replies,{})])
    native=native_sine(DLL)
    details={'scope':'Whole real primitive initialization and all eight production bodies; real existing default sprite/packing/byte allocation helpers; only CRT malloc modeled in emulation',
        'coverage':coverage,'checks':'Independent raw-address instruction-derived structural oracle; sine numeric reference executes authenticated original x87/private converter. Ordered external reads/writes/dependency entries/arguments/malloc snapshots, full known BSS/arena/packed37-byte view and unrelated image bytes, caller stack, nonvolatile regs/DF/CW/x87 stack top, persistent initialization, live-pointer/scratch mutations, aliases and retained faults',
        'compiler':'Provisional strict C89 Clang/LLD plus isolated GCC i386 ordered x87 FSIN; /safeseh:no. Original compiler/link layout unresolved',
        'limitations':'No instruction equality or whole native game/heap/startup parity. Lifecycle old primitive boundary coverage retained separately. Unmasked/nonempty-x87/FPSW details, arbitrary reentry/concurrency/exhaustive aliases/scalar-address collisions and native fault frames/atomicity unverified; unknown adjacent BSS aliases excluded.'}
    for name,(r,*_) in ROUTINES.items():
        for kind in ('compilation','emulation'):
            record_run(r,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
                cases=coverage[name]['comparisons'] if kind=='emulation' else 0,
                command='uv run python tools/verify_primitive_init.py',details=details|{'routine_coverage':coverage[name]})
    record_run(0x60740,'native','pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
        cases=12,command='uv run python tools/verify_primitive_init.py',details=native)
    print('PASS complete primitive contracts:',coverage,flush=True)
    print('PASS native sine:',native,flush=True)
    return coverage


if __name__=='__main__':verify_primitive_init()
