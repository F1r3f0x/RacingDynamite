# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Real native file wrappers versus complete production file.c; modeled CRT I/O."""
import hashlib
import random
import shutil
import struct
import subprocess

import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX,
    UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EFLAGS)
from verify_font_cleanup import SAVED, STACK, STOP
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')
ROUTINES = {
    'File_GetStreamSize': (0x575f0,60,0x307,'2524b5e97bba2f220082804901c8494382563e7a8207947bd60b8524424f4647'),
    'File_GetSize': (0x57630,47,0x206,'74b545b88eaa717b908b4d812074e28954eb30ea5fc0560f0810492515a5d19f'),
    'File_CheckReadable': (0x576b0,43,0,'3dde60cb96d06429d62e1e2826c17dc7e2c779dc5e2b39d5608b06f47485d7a3')}
CRT = {'fopen':(0x69390,21,2,0,'5f531b4e8d09786e07982b951f0259e58816996f5cdc227fe6438bf553e2bb17'),
    'fclose':(0x69100,112,1,0x202,'77ed7a98cac6e47994360f0f3625796c2cb18ce4adc98cea5d1cd30c973d6af2'),
    'ftell':(0x69de0,431,1,0x140b,'226f6ea54e9cf412aff7099c437890a19ccc0698054ebf69cc7d3dd5990e21f4'),
    'fseek':(0x69d40,153,3,0x307,'ae8af7be1dcb0f972852f6e90cdab27def44f6825d8b60d1c0cd08854a5ca833')}
DLL = BUILD/'file_helpers_validation.dll'
DATA, DATA_SIZE = 0x7400000, 4096
INPUTS = ['decomp/src/file.c','decomp/include/file.h','decomp/target.json',
    'tools/verify_file_helpers.py','tools/verify_matching.py','tools/verify_font_cleanup.py',
    'tools/windows_target.py','tools/windows_tracking.py']


def record(name,kind,outcome,cases=0,**details):
    record_run(ROUTINES[name][0],kind,outcome,inputs=INPUTS,
        artifact=DLL.relative_to(ROOT).as_posix() if DLL.exists() else None,
        cases=cases,command=('uv run python tools/verify_file_helpers.py' if __name__=='__main__'
        else 'uv run tools/verify_matching.py'),details=details)


def inspect_original(pe):
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type==3)
    fpo = {a:(n,l,p,b) for a,n,l,p,b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    expected_calls = {
        'File_GetStreamSize':[(0x4575f8,0x469de0),(0x457607,0x469d40),(0x457610,0x469de0),(0x45761e,0x469d40)],
        'File_GetSize':[(0x45763c,0x469390),(0x457647,0x4575f0),(0x457652,0x469100)],
        'File_CheckReadable':[(0x4576ba,0x469390),(0x4576cd,0x469100)]}
    for name,(rva,size,flags,digest) in ROUTINES.items():
        raw=pe.get_data(rva,size)
        assert fpo[rva]==(size,0,1,flags)
        assert hashlib.sha256(raw).hexdigest()==digest
        ins=list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(raw,0x400000+rva))
        assert sum(i.size for i in ins)==size
        assert [(i.address,int(i.op_str,16)) for i in ins if i.mnemonic=='call']==expected_calls[name]
        relocs=[(e.rva,struct.unpack('<I',pe.get_data(e.rva,4))[0])
            for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries
            if e.type==3 and rva<=e.rva<rva+size]
        assert relocs==([] if name=='File_GetStreamSize' else
            [(0x57637 if name=='File_GetSize' else 0x576b5,0x4ba794)])
    for rva,size,params,flags,digest in CRT.values():
        raw=pe.get_data(rva,size)
        assert fpo[rva]==(size,5 if rva==0x69de0 else 0,params,flags)
        assert hashlib.sha256(raw).hexdigest()==digest
        assert sum(i.size for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(raw,0x400000+rva))==size
    # fopen forwards filename, mode, policy 0x40 to the native common opener.
    assert pe.get_data(0x69390,21)==bytes.fromhex('8b4424086a408b4c24085051e8bfffffff83c40cc3')
    assert pe.get_data(0xba794,2)==b'r\0'
    assert pe.get_data(0xbab34,4)==b'\0'*4
    for va,target in [(0x4574ab,0x4576b0),(0x4574cc,0x457630),(0x457647,0x4575f0)]:
        raw=pe.get_data(va-0x400000,5)
        assert raw[0]==0xe8 and va+5+struct.unpack_from('<i',raw,1)[0]==target


def build_helpers():
    cc,ld=shutil.which('clang'),shutil.which('lld-link')
    if not cc or not ld: raise RuntimeError('Clang/LLD required; no fallback')
    BUILD.mkdir(parents=True,exist_ok=True)
    obj,stub,sobj=BUILD/'file.obj',BUILD/'file_crt_boundary.c',BUILD/'file_crt_boundary.obj'
    stub.write_text('void *fopen(const char *f,const char *m) { (void)f; (void)m; for (;;) {} }\n'
        'int fclose(void *s) { (void)s; for (;;) {} }\n'
        'long ftell(void *s) { (void)s; for (;;) {} }\n'
        'int fseek(void *s,long p,int o) { (void)s; (void)p; (void)o; for (;;) {} }\n',encoding='utf-8')
    for path in [obj,sobj,DLL,DLL.with_suffix('.lib'),DLL.with_suffix('.exp')]: path.unlink(missing_ok=True)
    flags=[cc,'--target=i686-pc-windows-msvc','-std=c89','-pedantic-errors','-Wall','-Wextra',
        '-Werror','-O2','-ffreestanding','-fno-builtin','-fno-inline','-fno-vectorize',
        '-fno-slp-vectorize','-mno-sse','-mno-sse2','-I',str(ROOT/'decomp/include')]
    commands=[flags+['-c',str(ROOT/'decomp/src/file.c'),'-o',str(obj)],
        flags+['-c',str(stub),'-o',str(sobj)],
        [ld,'/dll','/noentry','/nodefaultlib','/machine:x86','/base:0x10000000',
         '/out:'+str(DLL),str(obj),str(sobj)]+['/export:'+n for n in [*ROUTINES,*CRT]]]
    for command in commands:
        print(subprocess.list2cmdline(command),flush=True)
        subprocess.run(command,cwd=ROOT,check=True)
    assert all(p.is_file() and p.stat().st_size for p in [obj,sobj,DLL])


def inspect_rebuilt(pe,symbols):
    text=next(s for s in pe.sections if s.Name.startswith(b'.text'))
    ins=Cs(CS_ARCH_X86,CS_MODE_32).disasm(
        pe.get_data(text.VirtualAddress,text.Misc_VirtualSize),
        pe.OPTIONAL_HEADER.ImageBase+text.VirtualAddress)
    expected={
        'File_GetStreamSize':[symbols['ftell'],symbols['fseek'],symbols['ftell'],symbols['fseek']],
        'File_GetSize':[symbols['fopen'],symbols['File_GetStreamSize'],symbols['fclose']],
        'File_CheckReadable':[symbols['fopen'],symbols['fclose']]}
    actual={name:[] for name in expected}
    entries=sorted((address,name) for name,address in symbols.items())
    for i in ins:
        if i.mnemonic!='call': continue
        owner=next(name for address,name in reversed(entries) if address<=i.address)
        assert owner in actual,'Unexpected linked dependency owner'
        assert i.op_str.startswith('0x'),'Unexpected indirect linked call'
        actual[owner].append(int(i.op_str,16))
    assert actual==expected,'Linked call identities differ from the bounded contract'


def contract(name,arg,stream,tells,seeks,close):
    calls=[]
    if name!='File_GetStreamSize': calls.append(('fopen',(arg,b'r\0'),stream))
    if name=='File_CheckReadable':
        if stream: calls.append(('fclose',(stream,),close))
        return (1 if stream else 2031),calls
    if name=='File_GetStreamSize': stream=arg
    calls.extend([('ftell',(stream,),tells[0]),('fseek',(stream,0,2),seeks[0]),
        ('ftell',(stream,),tells[1]),('fseek',(stream,tells[0],0),seeks[1])])
    if name=='File_GetSize': calls.append(('fclose',(stream,),close))
    return tells[1],calls


class FileCPU:
    def __init__(self,pe,symbols=None):
        self.base=pe.OPTIONAL_HEADER.ImageBase
        self.n=(pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095
        self.cpu=Uc(UC_ARCH_X86,UC_MODE_32)
        self.cpu.mem_map(self.base,self.n)
        self.cpu.mem_write(self.base,pe.get_memory_mapped_image())
        self.cpu.mem_map(DATA,DATA_SIZE)
        self.cpu.mem_map(STACK,0x10000);self.cpu.mem_map(STOP,0x1000)
        self.entries={name:0x400000+info[0] if symbols is None else symbols[name]
            for name,info in ROUTINES.items()}
        self.boundaries={0x400000+info[0] if symbols is None else symbols[name]:name
            for name,info in CRT.items()}
        self.error=0x4bab34 if symbols is None else DATA+0xf00
        if symbols is None:
            self.allowed=[(0x400000+r,0x400000+r+s) for r,s,_,_ in ROUTINES.values()]
        else:
            text=next(s for s in pe.sections if s.Name.startswith(b'.text'))
            self.allowed=[(self.base+text.VirtualAddress,self.base+text.VirtualAddress+text.Misc_VirtualSize)]
        self.cpu.hook_add(UC_HOOK_CODE,self.hook)

    def hook(self,cpu,address,n,unused):
        if address in self.entries.values(): self.visited.add(address)
        if address not in self.boundaries:
            assert any(a<=address<b for a,b in self.allowed),'Escaped helper code'
            return
        sp=cpu.reg_read(UC_X86_REG_ESP)
        name=self.boundaries[address]
        assert self.index<len(self.calls),'Extra CRT invocation'
        expected,args,result=self.calls[self.index]
        assert name==expected
        words=struct.unpack('<'+'I'*(2 if name=='fopen' else 3 if name=='fseek' else 1),
            cpu.mem_read(sp+4,8 if name=='fopen' else 12 if name=='fseek' else 4))
        if name=='fopen':
            actual=(words[0],bytes(cpu.mem_read(words[1],2)))
        else: actual=words
        assert actual==args,(name,actual,args)
        # Full mapped image and external fixture state before every CRT call.
        assert bytes(cpu.mem_read(self.base,self.n))==bytes(self.image)
        assert bytes(cpu.mem_read(DATA,DATA_SIZE))==bytes(self.data)
        if self.mutate:
            # Explicit CRT fixture mutation of stream storage, filename storage
            # and error word. Wrappers must cache their pointers/tell results.
            value=(0xabc00000+self.index)&0xffffffff
            cpu.mem_write(DATA+0x100+4*self.index,struct.pack('<I',value))
            struct.pack_into('<I',self.data,0x100+4*self.index,value)
            cpu.mem_write(self.error,struct.pack('<I',value))
            if self.base<=self.error<self.base+self.n:
                struct.pack_into('<I',self.image,self.error-self.base,value)
            else: struct.pack_into('<I',self.data,self.error-DATA,value)
        self.index+=1
        cpu.reg_write(UC_X86_REG_EAX,result)
        cpu.reg_write(UC_X86_REG_ECX,0xc1c1c1c1);cpu.reg_write(UC_X86_REG_EDX,0xd2d2d2d2)
        cpu.reg_write(UC_X86_REG_EFLAGS,0x43)
        cpu.reg_write(UC_X86_REG_EIP,struct.unpack('<I',cpu.mem_read(sp,4))[0])
        cpu.reg_write(UC_X86_REG_ESP,sp+4)

    def invoke(self,name,arg,stream,tells,seeks,close,mutate,seed):
        expected,self.calls=contract(name,arg,stream,tells,seeks,close)
        cpu=self.cpu
        self.index,self.mutate,self.visited=0,mutate,set()
        self.data=bytearray((i*19+seed)%256 for i in range(DATA_SIZE))
        self.data[0x100:0x10c]=b'fixture.dat\0'
        cpu.mem_write(DATA,bytes(self.data))
        cpu.mem_write(self.error,struct.pack('<I',0x12345678))
        self.image=bytearray(cpu.mem_read(self.base,self.n))
        self.data=bytearray(cpu.mem_read(DATA,DATA_SIZE))
        sp=STACK+0x8000
        stack=bytes((i+seed)%256 for i in range(64))
        cpu.mem_write(sp,struct.pack('<II',STOP,arg)+stack)
        for i,reg in enumerate(SAVED): cpu.reg_write(reg,0x11220000+seed+i)
        cpu.reg_write(UC_X86_REG_ESP,sp);cpu.reg_write(UC_X86_REG_EFLAGS,2)
        cpu.emu_start(self.entries[name],STOP,count=10000)
        assert cpu.reg_read(UC_X86_REG_EIP)==STOP
        assert cpu.reg_read(UC_X86_REG_EAX)==expected
        assert cpu.reg_read(UC_X86_REG_ESP)==sp+4
        assert bytes(cpu.mem_read(sp,72))==struct.pack('<II',STOP,arg)+stack
        assert [cpu.reg_read(r) for r in SAVED]==[0x11220000+seed+i for i in range(4)]
        assert not cpu.reg_read(UC_X86_REG_EFLAGS)&0x400
        assert self.index==len(self.calls)
        assert bytes(cpu.mem_read(self.base,self.n))==bytes(self.image)
        assert bytes(cpu.mem_read(DATA,DATA_SIZE))==bytes(self.data)
        if name=='File_GetSize': assert self.entries['File_GetStreamSize'] in self.visited
        return expected,self.calls,bytes(cpu.mem_read(self.error,4)),bytes(cpu.mem_read(DATA,0xf00))


def verify_file_helpers():
    verify_target();phase='compilation'
    try:
        original=pefile.PE(str(TARGET));inspect_original(original);build_helpers()
        rebuilt=pefile.PE(str(DLL))
        symbols={s.name.decode():rebuilt.OPTIONAL_HEADER.ImageBase+s.address
            for s in rebuilt.DIRECTORY_ENTRY_EXPORT.symbols if s.name}
        inspect_rebuilt(rebuilt,symbols)
        for name,(_,size,_,_) in ROUTINES.items():
            record(name,'compilation','pass',scope='Complete production C89 file.c; no-inline real helper; nonreturning CRT link fixtures',
                compiler='Clang/LLD 19.1.1 provisional i686-pc-windows-msvc')
            raw=rebuilt.get_data(symbols[name]-rebuilt.OPTIONAL_HEADER.ImageBase,size)
            record(name,'raw_bytes','pass' if raw==original.get_data(ROUTINES[name][0],size) else 'different',
                scope='Prefix diagnostic only; instruction equality unverified')
        phase='emulation';a,b=FileCPU(original),FileCPU(rebuilt,symbols)
        counts={name:0 for name in ROUTINES};rng=random.Random(0x575f0)
        def compare(name,arg,stream,tells,seeks,close,mutate=False):
            seed=sum(counts.values())
            args=(name,arg,stream,tells,seeks,close,mutate,seed)
            assert a.invoke(*args)==b.invoke(*args)
            counts[name]+=1
        words=[0,1,0x7fffffff,0x80000000,0xffffffff]
        for name in ROUTINES:
            for stream in [0,DATA+0x200,0xffffffff]:
                for arg in [0,DATA+0x100,0xffffffff]:
                    for first in words:
                        for second in words:
                            if name=='File_CheckReadable' and (first!=0 or second!=0): continue
                            compare(name,arg,stream,(first,second),
                                (words[first%5],words[second%5]),words[(first+second)%5])
            for i in range(80):
                compare(name,DATA+0x100,rng.choice([0,DATA+0x200,0x80000000,0xffffffff]),
                    (rng.getrandbits(32),rng.getrandbits(32)),
                    (rng.getrandbits(32),rng.getrandbits(32)),rng.getrandbits(32),mutate=bool(i%2))
            # Every ignored close/seek return, including noncanonical raw words.
            for word in words:
                compare(name,DATA+0x100,DATA+0x200,(0xffffffff,0x80000000),
                    (word,word),word,mutate=True)
        for name,count in counts.items():
            record(name,'emulation','pass',cases=count,
                scope='Exact ordered CRT arguments/results, complete image/fixture state at every boundary and return, cached words, error preservation/mutation, caller stack and ABI; real GetStreamSize executes through GetSize',
                limitations='CRT I/O explicitly modeled; null/raw stream forwarding does not establish safe native calls; no native filesystem, CRT FILE layout, concurrency/reentry, instruction equality, loader or game parity')
        print('PASS: file helper differential executions '+str(counts))
        return counts
    except Exception as exc:
        for name in ROUTINES: record(name,phase,'fail',error=str(exc))
        raise


if __name__=='__main__':
    verify_file_helpers()
