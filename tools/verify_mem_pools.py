# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Pool initialization/creation: real production C and real alloc/free integration."""
import hashlib
import random
import struct
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX,
    UC_X86_REG_EIP, UC_X86_REG_ESP, UC_X86_REG_EFLAGS)
from verify_font_cleanup import SAVED, STACK, STOP
from verify_mem_free import ARENA, ARENA_SIZE, oracle as free_oracle, inspect_original as inspect_free
from verify_mem_alloc import oracle as alloc_oracle, inspect_original as inspect_alloc
from build_decomp import build
from windows_target import validate_platform_imports
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')
ROUTINES = {'Mem_InitPools': (0x5ad50, 35,
    '4dde71fdc49b5480369eec8adf5b12a9a74ad3aeb97921330f412e22acc3b627'),
    'Mem_CreatePool': (0x5ad80, 132,
    '8beca914f2291a89e320f0ad75559b562a336f5a0c020f9553de979bba923743')}
DLL = BUILD / 'mem_pools_validation.dll'
NAME = ARENA + 0x80000
INPUTS = ['decomp/src/lisa3d.c', 'decomp/include/lisa3d.h', 'decomp/src/geputget.c', 'decomp/include/geputget.h', 'decomp/src/mem.c', 'decomp/include/mem.h', 'decomp/target.json',
    'tools/verify_mem_pools.py', 'tools/verify_mem_alloc.py', 'tools/verify_mem_free.py',
    'tools/build_decomp.py', 'tools/verify_matching.py', 'tools/verify_font_cleanup.py',
    'tools/windows_target.py', 'tools/windows_tracking.py']


def record(name, kind, outcome, cases=0, **details):
    record_run(ROUTINES[name][0], kind, outcome, inputs=INPUTS,
        artifact=DLL.relative_to(ROOT).as_posix() if DLL.exists() else None,
        cases=cases, command=('uv run python tools/verify_mem_pools.py'
            if __name__ == '__main__' else 'uv run tools/verify_matching.py'), details=details)


def inspect_original(pe):
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {r:(n,l,p,b) for r,n,l,p,b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    for name,(rva,n,digest) in ROUTINES.items():
        raw = pe.get_data(rva,n)
        assert hashlib.sha256(raw).hexdigest() == digest
        ins = list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(raw,0x400000+rva))
        assert sum(i.size for i in ins) == n and ins[-1].mnemonic == 'ret'
        assert len(ins) == (11 if name == 'Mem_InitPools' else 50)
        expected = (n,0,0,0x103) if name == 'Mem_InitPools' else (n,0,1,0x209)
        assert fpo[rva] == expected
        assert pe.get_data(rva+n,13 if name == 'Mem_InitPools' else 12) == b'\xcc'*(13 if name == 'Mem_InitPools' else 12)
        relocs = [(e.rva,struct.unpack('<I',pe.get_data(e.rva,4))[0])
            for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries
            if e.type == 3 and rva <= e.rva < rva+n]
        assert relocs == ([(0x5ad54,0x63c6a0),(0x5ad60,0x491f6c)]
            if name == 'Mem_InitPools' else [(0x5ad82,0x63c6a0),(0x5ad93,0x63caa0),(0x5adc9,0x63c6a0)])
        assert [(i.address,i.op_str) for i in ins if i.mnemonic == 'call'] == (
            [(0x45ad64,'0x45ad80')] if name == 'Mem_InitPools' else [(0x45adae,'0x469400')])
    assert pe.get_data(0x91f6c,8) == b'DEFAULT\0'
    section = next(s for s in pe.sections if s.VirtualAddress <= 0x23c6a0 < s.VirtualAddress+s.Misc_VirtualSize)
    assert section.Name.rstrip(b'\0') == b'.data'
    assert section.VirtualAddress+section.SizeOfRawData <= 0x23c6a0
    assert 0x23caa0 <= section.VirtualAddress+section.Misc_VirtualSize
    inspect_alloc(pe); inspect_free(pe)
    for va,target in [(0x45b175,0x45ad50)] + [(va,0x45ad80) for va in
            [0x40ffeb,0x417eef,0x418b7c,0x420249,0x4223f1,0x446049,0x45ad64]]:
        raw = pe.get_data(va-0x400000,5)
        assert raw[0] == 0xe8 and va+5+struct.unpack_from('<i',raw,1)[0] == target


def pool_oracle(table, arena, name, returns, mutations=(), initialize=False):
    """Literal native scan/copy/store order; heap mutations are explicit fixtures."""
    table,arena = bytearray(table),bytearray(arena)
    events,calls = [],[]
    if initialize:
        for i in range(256):
            struct.pack_into('<I',table,4*i,0)
            events.append(('write','table',4*i,4,0))
        name = b'DEFAULT\0'
    slot = 0
    while slot < 256:
        events.append(('read','table',slot*4,4))
        if struct.unpack_from('<I',table,slot*4)[0] == 0: break
        slot += 1
    if slot == 256:
        assert not returns
        return (1 if initialize else 0xffffffff),bytes(table),bytes(arena),events,calls
    assert len(returns) == 1
    pointer = returns[0]
    calls.append(('malloc',320,pointer,bytes(table),bytes(arena)))
    events.append(('malloc',320,pointer))
    for region,off,value in mutations:
        struct.pack_into('<I',table if region == 'table' else arena,off,value)
    if not pointer:
        return (1 if initialize else 0xffffffff),bytes(table),bytes(arena),events,calls
    events.append(('write','table',slot*4,4,pointer));struct.pack_into('<I',table,slot*4,pointer)
    destination = pointer-ARENA
    index = 0
    if name is not None:
        # Compare source[index], then copy from a second byte read; byte 63 is
        # compared even for an overlong name. Null names cause no source reads.
        while True:
            events.append(('read','name',index,1))
            if name[index] == 0 or index >= 63: break
            events.append(('read','name',index,1))
            events.append(('write','arena',destination+index,1,name[index]))
            arena[destination+index] = name[index];index += 1
    events.append(('write','arena',destination+index,1,0));arena[destination+index] = 0
    for i in range(64):
        off=destination+64+4*i
        events.append(('write','arena',off,4,0));struct.pack_into('<I',arena,off,0)
    return (1 if initialize else slot),bytes(table),bytes(arena),events,calls


class PoolSession:
    def __init__(self,pe,symbols=None):
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.image_size = (pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095
        self.cpu = Uc(UC_ARCH_X86,UC_MODE_32)
        self.cpu.mem_map(self.base,self.image_size);self.cpu.mem_write(self.base,pe.get_memory_mapped_image())
        self.cpu.mem_map(ARENA,ARENA_SIZE);self.cpu.mem_map(STACK,0x10000);self.cpu.mem_map(STOP,0x1000)
        self.entries = {name:0x400000+rva for name,(rva,_,_) in ROUTINES.items()} if symbols is None else symbols
        self.entries = dict(self.entries)
        if symbols is None: self.entries.update(Mem_Alloc=0x45ae10,Mem_Free=0x45b000)
        self.malloc = 0x469400 if symbols is None else symbols['malloc']
        self.free = 0x4693b0 if symbols is None else symbols['free']
        self.table = 0x63c6a0 if symbols is None else symbols['g_memPools']
        if symbols is None:
            self.ranges = [(0x400000+r,n) for r,n in [(0x5ad50,35),(0x5ad80,132),(0x5ae10,406),(0x5b000,127)]]
            self.default = 0x491f6c
        else:
            text = next(s for s in pe.sections if s.Name.rstrip(b'\0') == b'.text')
            self.ranges = [(self.base+text.VirtualAddress,text.Misc_VirtualSize)]
            image = pe.get_memory_mapped_image()
            matches=[i for i in range(len(image)) if image.startswith(b'DEFAULT\0',i)]
            assert len(matches) == 1
            self.default = self.base+matches[0]
        assert bytes(self.cpu.mem_read(self.default,8)) == b'DEFAULT\0'
        self.cpu.hook_add(UC_HOOK_CODE,self.hook)
        self.cpu.hook_add(UC_HOOK_MEM_READ,self.read)
        self.cpu.hook_add(UC_HOOK_MEM_WRITE,self.write)

    def read(self,cpu,access,address,n,value,unused):
        if self.name_address and self.name_address <= address < self.name_address+64:
            self.events.append(('read','name',address-self.name_address,n))
        elif self.table <= address < self.table+1024:
            self.events.append(('read','table',address-self.table,n))
        elif ARENA <= address < ARENA+ARENA_SIZE:
            self.events.append(('read','arena',address-ARENA,n))
        elif not STACK <= address < STACK+0x10000:
            raise AssertionError('Unexpected data read %x'%address)

    def write(self,cpu,access,address,n,value,unused):
        if self.table <= address < self.table+1024:
            self.events.append(('write','table',address-self.table,n,value))
        elif ARENA <= address < ARENA+ARENA_SIZE:
            self.events.append(('write','arena',address-ARENA,n,value))
        elif not STACK <= address < STACK+0x10000:
            raise AssertionError('Unexpected write %x'%address)

    def hook(self,cpu,address,n,unused):
        if address not in (self.malloc,self.free):
            assert any(a <= address < a+length for a,length in self.ranges), 'Escaped authenticated bodies'
            return
        assert self.call_index < len(self.calls), 'Unexpected CRT call'
        kind,arg,result,table,arena = self.calls[self.call_index];self.call_index += 1
        assert address == (self.malloc if kind == 'malloc' else self.free)
        sp=cpu.reg_read(UC_X86_REG_ESP)
        assert struct.unpack('<I',cpu.mem_read(sp+4,4))[0] == arg
        assert bytes(cpu.mem_read(self.table,1024)) == table, 'CRT-entry root table'
        assert bytes(cpu.mem_read(ARENA,ARENA_SIZE)) == arena, 'CRT-entry full heap'
        self.events.append(('malloc',arg,result) if kind == 'malloc' else ('free',arg))
        for region,off,word in self.mutations:
            cpu.mem_write((self.table if region == 'table' else ARENA)+off,struct.pack('<I',word))
        cpu.reg_write(UC_X86_REG_EAX,result)
        cpu.reg_write(UC_X86_REG_ECX,0xc1c1c1c1);cpu.reg_write(UC_X86_REG_EDX,0xd2d2d2d2)
        cpu.reg_write(UC_X86_REG_EFLAGS,0x43)
        cpu.reg_write(UC_X86_REG_EIP,struct.unpack('<I',cpu.mem_read(sp,4))[0]);cpu.reg_write(UC_X86_REG_ESP,sp+4)

    def reset(self,table,arena):
        self.cpu.mem_write(self.table,table);self.cpu.mem_write(ARENA,arena)

    def invoke(self,routine,args,expected,name_address=0,mutations=(),seed=0):
        eax,table,arena,events,self.calls = expected
        self.events,self.call_index,self.name_address,self.mutations = [],0,name_address,mutations
        cpu=self.cpu;sp=STACK+0x8000
        cpu.mem_write(sp,struct.pack('<'+'I'*(len(args)+1),STOP,*args))
        stack=bytes(cpu.mem_read(sp,0x8000))
        rng=random.Random(seed);saved=[rng.getrandbits(32) for _ in SAVED]
        for reg,value in zip(SAVED,saved):cpu.reg_write(reg,value)
        cpu.reg_write(UC_X86_REG_ESP,sp);cpu.reg_write(UC_X86_REG_EFLAGS,2)
        before=bytes(cpu.mem_read(self.base,self.image_size))
        cpu.emu_start(self.entries[routine],STOP,count=5000000)
        assert cpu.reg_read(UC_X86_REG_EIP) == STOP and cpu.reg_read(UC_X86_REG_ESP) == sp+4
        assert [cpu.reg_read(reg) for reg in SAVED] == saved
        assert not cpu.reg_read(UC_X86_REG_EFLAGS)&0x400
        assert bytes(cpu.mem_read(sp,0x8000)) == stack
        after=bytearray(cpu.mem_read(self.base,self.image_size))
        off=self.table-self.base;after[off:off+1024]=before[off:off+1024]
        assert bytes(after) == before, 'Unrelated image state changed'
        if self.events != events:
            k=next((i for i,(a,b) in enumerate(zip(self.events,events)) if a!=b),min(len(self.events),len(events)))
            raise AssertionError('Event mismatch %d: %s versus %s'%(k,self.events[k:k+3],events[k:k+3]))
        assert self.call_index == len(self.calls)
        result=cpu.reg_read(UC_X86_REG_EAX),bytes(cpu.mem_read(self.table,1024)),bytes(cpu.mem_read(ARENA,ARENA_SIZE))
        assert result == (eax,table,arena), 'Return/table/heap mismatch'
        return result


def verify_mem_pools():
    verify_target();phase='compilation';counts={name:0 for name in ROUTINES}
    try:
        original=pefile.PE(str(TARGET));inspect_original(original);build(dll=DLL)
        for name in ROUTINES:
            record(name,'compilation','pass',scope='Complete production mem.c, strict C89; Clang/LLD provisional, nonreturning CRT boundaries')
        rebuilt=pefile.PE(str(DLL))
        validate_platform_imports(rebuilt)
        symbols={s.name.decode():rebuilt.OPTIONAL_HEADER.ImageBase+s.address for s in rebuilt.DIRECTORY_ENTRY_EXPORT.symbols if s.name}
        for name,(rva,n,_) in ROUTINES.items():
            prefix=rebuilt.get_data(symbols[name]-rebuilt.OPTIONAL_HEADER.ImageBase,n)
            record(name,'raw_bytes','pass' if prefix==original.get_data(rva,n) else 'different',scope='Raw prefix diagnostic only; instruction equality unverified')
        a,b=PoolSession(original),PoolSession(rebuilt,symbols);phase='emulation'
        rng=random.Random(0x5ad80)
        def fresh():return bytearray(b'\xa5'*ARENA_SIZE)
        def compare(table,name=b'test\0',pointer=ARENA,initialize=False,mutations=(),arena=None,unreadable=False):
            arena=fresh() if arena is None else bytearray(arena)
            if name is not None and not initialize:arena[NAME-ARENA:NAME-ARENA+len(name)]=name
            routine='Mem_InitPools' if initialize else 'Mem_CreatePool'
            returns=() if not initialize and all(struct.unpack('<256I',table)) else (pointer,)
            expected=pool_oracle(table,arena,name,returns,mutations,initialize)
            for session in (a,b):session.reset(bytes(table),bytes(arena))
            args=() if initialize else (0 if name is None else 0xffffffff if unreadable else NAME,)
            results=[session.invoke(routine,args,expected,
                session.default if initialize else 0 if name is None or unreadable else NAME,
                mutations,counts[routine]) for session in (a,b)]
            assert results[0] == results[1]
            counts[routine]+=1
            return results[0]
        # Every first-hole index, alternating successful creation and malloc failure.
        for slot in range(256):
            words=[0x80000000+i*4 for i in range(256)]
            words[slot]=0
            if slot < 255:words[255]=0
            table=struct.pack('<256I',*words)
            compare(table,name=None if slot%4==0 else b'slot\0',pointer=ARENA)
            compare(table,name=b'ignored\0',pointer=0,unreadable=True)
        full=struct.pack('<256I',*[0x80000000+i for i in range(256)])
        compare(full,unreadable=True)
        zero=b'\0'*1024
        # Every terminating index, both zero/nonzero byte 63 and long inputs;
        # unsigned char values are copied literally. Unwritten heap bytes persist.
        for length in range(81):
            for pattern in (0x41,0xff):
                compare(zero,name=bytes([pattern])*length+b'\0',pointer=ARENA+0x2000)
        compare(zero,name=None)
        for _ in range(64):
            slot=rng.randrange(256);words=[rng.getrandbits(32) or 1 for _ in range(slot)]+[0]*(256-slot)
            n=rng.randrange(100)
            compare(struct.pack('<256I',*words),name=bytes(rng.randrange(1,256) for _ in range(n))+b'\0',
                pointer=ARENA+0x4000)
        # Cached slot survives CRT changing the selected/table words. Source
        # mutation is excluded: this oracle keeps the input bytes independent.
        table=struct.pack('<256I',1,0,*([0]*254))
        for pointer in (0,ARENA):
            compare(table,mutations=(('table',0,0),('table',4,0xdeadbeef),('table',1020,0x12345678)),pointer=pointer)
        # Initializer always resets (including populated roots), never frees;
        # real nested creator receives DEFAULT and sees the already-cleared table.
        for pattern in (0,0xffffffff,None):
            for repeat in range(8):
                table=struct.pack('<256I',*[rng.getrandbits(32) if pattern is None else pattern for _ in range(256)])
                for pointer in (0,ARENA,ARENA+0x2000):compare(table,initialize=True,pointer=pointer)
        # Persistent sequence: reset twice (orphan old roots), create after
        # failure, then more creators. No table/heap reset between calls.
        persistent=0
        def step(routine,args,expected,name_address=0):
            nonlocal persistent
            outputs=[s.invoke(routine,args,expected,s.default if name_address == -1 else name_address,
                seed=10000+persistent) for s in (a,b)]
            assert outputs[0] == outputs[1];persistent+=1
            if routine in counts:counts[routine]+=1
            return outputs[0]
        table=full;arena=fresh()
        for s in (a,b):s.reset(table,bytes(arena))
        for pointer in (ARENA,0,ARENA):
            result=step('Mem_InitPools',(),pool_oracle(table,arena,None,(pointer,),initialize=True),-1)
            _,table,arena=result
        # Real pool 0 now exists. Both binaries use these exact persistent
        # created roots through production Mem_Alloc/Mem_Free, then record reuse.
        integration=0
        for size,returns in [(123,(ARENA+0xc0000,ARENA+0xc1000,ARENA+0xc2000)),
                             (987,(ARENA+0xc3000,)),(0,(ARENA+0xc4000,))]:
            value,final,events,snapshots,requests=alloc_oracle(arena,0,size,returns,{})
            converted=[]
            for event in events:
                kind,*rest=event
                converted.append(('read','table',*rest) if kind=='table' else
                    (kind,'arena',*rest) if kind in ('read','write') else event)
            calls=[('malloc',request,pointer,table,snapshot) for request,pointer,snapshot in zip(requests,returns,snapshots)]
            _,table,arena=step('Mem_Alloc',(0,size),(value,table,final,converted,calls));integration+=1
            expected,final,match,reads=free_oracle(arena,0,value,())
            converted=[('read',*event) for event in reads]+[('free',value),('write','arena',match+4,4,0)]
            calls=[('free',value,0xdeadbeef,table,arena)]
            _,table,arena=step('Mem_Free',(0,value),(expected,table,final,converted,calls));integration+=1
        # Persistent creators use existing pool zero and choose the next slot;
        # first failure then success must leave the same slot available.
        for pointer in (0,ARENA+0x2000,ARENA+0x4000):
            _,table,arena=step('Mem_CreatePool',(0,),pool_oracle(table,arena,None,(pointer,)))
        for name in ROUTINES:
            record(name,'emulation','pass',cases=counts[name],scope='Complete table/heap/image plus caller-preserved state/stack; ordered scan/name reads/stores and CRT calls; CRT-entry snapshots, failure, truncation, retained name tails, all 256 slots, real nested creator; persistent reset/create and six real alloc/free integration invocations',
                persistent_invocations=persistent,alloc_free_invocations=integration,
                limitations='CRT malloc/free modeled; readable nonaliasing name/heap inputs, DF clear; native heap/game parity, instruction equality, original link layout, arbitrary aliasing, invalid storage, concurrency/reentry unverified')
        print('PASS: %d Mem_InitPools and %d Mem_CreatePool differential invocations; %d persistent invocations include %d real Mem_Alloc/Mem_Free calls.'%(counts['Mem_InitPools'],counts['Mem_CreatePool'],persistent,integration))
        return counts
    except Exception as exc:
        for name in ROUTINES:record(name,phase,'fail',error=str(exc))
        raise


if __name__ == '__main__':
    verify_mem_pools()
