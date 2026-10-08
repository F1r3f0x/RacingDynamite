# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Pool destruction/shutdown: production C versus authenticated PE instructions."""
import hashlib
import random
import struct
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from verify_mem_pools import PoolSession, pool_oracle, inspect_original as inspect_pools
from verify_mem_alloc import oracle as alloc_oracle
from verify_mem_free import PoolFixture, ARENA, ARENA_SIZE, oracle as free_oracle
from build_decomp import build
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')
ROUTINES = {
    'Mem_DestroyPool': (0x5b080,182,'5d94bbebd22cf4806b9abdee9a916457043581b24f1fe69b907c320596cb7e62'),
    'Mem_ShutdownPools': (0x5b140,41,'caa0affb689af0a88ea17b0af2829cec7f498ea2613b8ede871badc5afff2444')}
DLL = BUILD / 'mem_destroy_validation.dll'
INPUTS = ['decomp/src/lisa3d.c', 'decomp/include/lisa3d.h', 'decomp/src/geputget.c', 'decomp/include/geputget.h', 'decomp/src/mem.c','decomp/include/mem.h','decomp/target.json',
    'tools/verify_mem_destroy.py','tools/verify_mem_pools.py','tools/verify_mem_alloc.py',
    'tools/verify_mem_free.py','tools/build_decomp.py','tools/verify_matching.py',
    'tools/verify_font_cleanup.py','tools/windows_target.py','tools/windows_tracking.py']


def record(name,kind,outcome,cases=0,**details):
    record_run(ROUTINES[name][0],kind,outcome,inputs=INPUTS,
        artifact=DLL.relative_to(ROOT).as_posix() if DLL.exists() else None,
        cases=cases,command=('uv run python tools/verify_mem_destroy.py'
            if __name__ == '__main__' else 'uv run tools/verify_matching.py'),details=details)


def inspect_original(pe):
    inspect_pools(pe)
    debug=next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type==3)
    fpo={r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    for name,(rva,n,digest) in ROUTINES.items():
        raw=pe.get_data(rva,n)
        assert hashlib.sha256(raw).hexdigest()==digest
        ins=list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(raw,0x400000+rva))
        assert len(ins)==(58 if name=='Mem_DestroyPool' else 16)
        assert sum(i.size for i in ins)==n and ins[-1].mnemonic=='ret'
        assert fpo[rva]==((182,5,1,0x141d) if name=='Mem_DestroyPool' else (41,0,0,0x202))
        padding=10 if name=='Mem_DestroyPool' else 7
        assert pe.get_data(rva+n,padding)==b'\xcc'*padding
        relocs=[(e.rva,struct.unpack('<I',pe.get_data(e.rva,4))[0])
            for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries
            if e.type==3 and rva<=e.rva<rva+n]
        assert relocs==([(0x5b08b,0x63c6a0),(0x5b121,0x63c6a0)]
            if name=='Mem_DestroyPool' else [(0x5b149,0x63c6a0)])
        assert [(i.address,i.op_str) for i in ins if i.mnemonic=='call']==(
            [(v,'0x4693b0') for v in (0x45b0d5,0x45b0e4,0x45b0fa,0x45b112)]
            if name=='Mem_DestroyPool' else [(0x45b150,'0x45b080')])
    for site,target in [(v,0x45b080) for v in
            (0x410926,0x420853,0x420977,0x420be8,0x422513,0x44616e,0x45b150)]+[(0x45b1a5,0x45b140)]:
        raw=pe.get_data(site-0x400000,5)
        assert raw[0]==0xe8 and site+5+struct.unpack_from('<i',raw,1)[0]==target


def destroy_oracle(table,arena,pool_id=0,shutdown=False,plan=None):
    """Literal size-first traversal, cached object bases and live later-slot loads."""
    plan=plan or {};table=bytearray(table);arena=bytearray(arena)
    events,calls=[],[]
    table_snapshot,arena_snapshot=bytes(table),bytes(arena)
    def read(region,off):
        events.append(('read',region,off,4))
        return struct.unpack_from('<I',table if region=='table' else arena,off)[0]
    def release(pointer):
        nonlocal table_snapshot,arena_snapshot
        k=len(calls)
        calls.append(('free',pointer,0xdeadbeef,table_snapshot,arena_snapshot))
        events.append(('free',pointer))
        for region,off,word in plan.get(k,()):
            struct.pack_into('<I',table if region=='table' else arena,off,word)
        if plan.get(k):table_snapshot,arena_snapshot=bytes(table),bytes(arena)
    def destroy(slot):
        nonlocal table_snapshot
        pool=read('table',4*slot)-ARENA
        for p in range(64):
            page=read('arena',pool+64+4*p)
            if not page:continue
            for b in range(64):
                block=read('arena',page-ARENA+4*b)
                if not block:continue
                for r in range(16):
                    off=block-ARENA+8*r
                    if read('arena',off+4):release(read('arena',off))
                release(block)
            release(page)
        release(ARENA+pool)
        events.append(('write','table',4*slot,4,0));struct.pack_into('<I',table,4*slot,0)
        table_snapshot=bytes(table)
    if shutdown:
        for slot in range(256):
            if read('table',4*slot):destroy(slot)
    else:destroy(pool_id)
    assert all(k<len(calls) for k in plan), 'Unused boundary mutation'
    return 1,bytes(table),bytes(arena),events,calls


class DestroySession(PoolSession):
    def __init__(self,pe,symbols=None):
        self.plan={}
        super().__init__(pe,symbols)
        if symbols is None:
            self.entries.update({name:0x400000+r for name,(r,_,_) in ROUTINES.items()})
            self.ranges.extend((0x400000+r,n) for r,n,_ in ROUTINES.values())

    def hook(self,cpu,address,n,unused):
        if address in (self.malloc,self.free):
            self.mutations=self.plan.get(self.call_index,())
        super().hook(cpu,address,n,unused)


def verify_mem_destroy():
    verify_target();phase='compilation';counts={name:0 for name in ROUTINES}
    try:
        original=pefile.PE(str(TARGET));inspect_original(original);build(dll=DLL)
        for name in ROUTINES:record(name,'compilation','pass',scope='Complete production mem.c, strict C89; provisional Clang/LLD, nonreturning CRT fixtures')
        rebuilt=pefile.PE(str(DLL));assert not hasattr(rebuilt,'DIRECTORY_ENTRY_IMPORT')
        symbols={s.name.decode():rebuilt.OPTIONAL_HEADER.ImageBase+s.address for s in rebuilt.DIRECTORY_ENTRY_EXPORT.symbols if s.name}
        for name,(r,n,_) in ROUTINES.items():
            equal=rebuilt.get_data(symbols[name]-rebuilt.OPTIONAL_HEADER.ImageBase,n)==original.get_data(r,n)
            record(name,'raw_bytes','pass' if equal else 'different',scope='Raw prefix only; instruction equality unverified')
        a,b=DestroySession(original),DestroySession(rebuilt,symbols);phase='emulation'
        rng=random.Random(0x5b080)
        def compare(table,arena,slot=0,shutdown=False,plan=None):
            routine='Mem_ShutdownPools' if shutdown else 'Mem_DestroyPool'
            expected=destroy_oracle(table,arena,slot,shutdown,plan)
            for s in (a,b):s.reset(bytes(table),bytes(arena));s.plan=plan or {}
            outputs=[s.invoke(routine,() if shutdown else (slot,),expected,seed=sum(counts.values())) for s in (a,b)]
            assert outputs[0]==outputs[1];counts[routine]+=1
            return outputs[0]
        def roots(slot=0):
            table=bytearray(1024);struct.pack_into('<I',table,4*slot,ARENA);return table
        # Every root, including last; other roots contain distinct unreadable values
        # in direct-destroy tests, proving no unrelated roots are consulted.
        for slot in range(256):
            f=PoolFixture();table=bytearray(struct.pack('<256I',*([0x80000000]*256)))
            struct.pack_into('<I',table,4*slot,ARENA)
            compare(table,f.data,slot)
            compare(roots(slot),f.data,shutdown=True)
        for index in range(64):
            for p,q in [(index,63),(63,index)]:
                f=PoolFixture();off=f.block(p,q)
                for r in range(16):f.word(off+8*r+4,0)
                f.match(p,q,index%16,pointer=0xffffffff,size=0x80000000)
                compare(roots(),f.data)
        for r in range(16):
            for pointer in (0,1,0x80000000,0xffffffff):
                f=PoolFixture();off=f.block(63,63)
                for i in range(16):f.word(off+8*i+4,0)
                f.match(63,63,r,pointer,size=0xffffffff)
                compare(roots(),f.data)
        # Zero-sized records keep arbitrary stale pointers; a completely dense
        # hierarchy still frees every block/page but never reads payload pointers.
        f=PoolFixture(dense=True)
        for off in f.blocks.values():
            for r in range(16):f.word(off+8*r+4,0)
        compare(roots(),f.data)
        f.match(63,63,15,pointer=0,size=1);compare(roots(),f.data)
        compare(bytes(1024),PoolFixture().data,shutdown=True)
        for _ in range(48):
            f=PoolFixture()
            for k in range(rng.randrange(1,8)):
                off=f.block(rng.randrange(64),rng.randrange(64))
                for r in range(16):
                    f.word(off+8*r,rng.getrandbits(32))
                    f.word(off+8*r+4,rng.choice([0,0,1,0x80000000,0xffffffff]))
            compare(roots(),f.data)
        # A free mutates the current linkage and root, next record and later
        # block/page slots. Current objects are cached; future slots are live.
        for p,q,r in [(0,0,0),(0,63,15),(63,0,0),(63,63,15)]:
            f=PoolFixture();off=f.block(p,q)
            for i in range(16):f.word(off+8*i+4,0)
            f.match(p,q,r,pointer=1,size=1)
            action=[('table',0,ARENA+0x2000),('arena',64+4*p,0),
                ('arena',f.pages[p]+4*q,0),('arena',off+8*r,0x11223344)]
            if r<15:action += [('arena',off+8*(r+1)+4,1),('arena',off+8*(r+1),0xffffffff)]
            compare(roots(),f.data,plan={0:action})
        f=PoolFixture();off=f.block(0,0);f.block(0,1);f.block(1,0)
        compare(roots(),f.data,plan={0:[('arena',f.pages[0]+4,0),('arena',68,0)]})
        # Live shutdown observes future insertion/removal, never revisits an
        # earlier slot, and clears selected roots only after the root free.
        f=PoolFixture();table=roots(2);struct.pack_into('<I',table,4*9,ARENA+0x2000)
        compare(table,f.data,shutdown=True,plan={0:[('table',4*9,0),('table',4*255,ARENA+0x2000)]})
        table=roots(10)
        compare(table,f.data,shutdown=True,plan={0:[('table',0,ARENA+0x2000),('table',40,ARENA+0x2000)]})
        # Persistent lifecycle uses actual initialization, creation, allocation,
        # free, destruction and shutdown without resetting state between calls.
        persistent=0;integration=0
        def step(routine,args,expected,name_address=0):
            nonlocal persistent,integration
            for s in (a,b):s.plan={}
            outputs=[s.invoke(routine,args,expected,s.default if name_address==-1 else name_address,
                seed=10000+persistent) for s in (a,b)]
            assert outputs[0]==outputs[1];persistent+=1
            if routine in counts:counts[routine]+=1
            else:integration+=1
            return outputs[0][1:]
        def allocate(table,arena,size,returns):
            value,final,events,snapshots,requests=alloc_oracle(arena,0,size,returns,{})
            converted=[('read','table',*e[1:]) if e[0]=='table' else
                (e[0],'arena',*e[1:]) if e[0] in ('read','write') else e for e in events]
            calls=[('malloc',n,v,table,image) for n,v,image in zip(requests,returns,snapshots)]
            table,arena=step('Mem_Alloc',(0,size),(value,table,final,converted,calls))
            return table,arena,value
        def release(table,arena,value):
            result,final,match,reads=free_oracle(arena,0,value,())
            events=[('read',*e) for e in reads]+[('free',value),('write','arena',match+4,4,0)]
            return step('Mem_Free',(0,value),(result,table,final,events,[('free',value,0xdeadbeef,table,arena)]))
        table=bytes(1024);arena=bytes(PoolFixture().data)
        for s in (a,b):s.reset(table,arena)
        table,arena=step('Mem_InitPools',(),pool_oracle(table,arena,None,(ARENA,),initialize=True),-1)
        # Null-name creator yields a second real pool, then successful allocation,
        # individual free, zero-size allocation and overwrite leakage.
        table,arena=step('Mem_CreatePool',(0,),pool_oracle(table,arena,None,(ARENA+0x2000,)))
        table,arena,value=allocate(table,arena,123,(ARENA+0xc0000,ARENA+0xc1000,ARENA+0xc2000))
        table,arena=release(table,arena,value)
        table,arena,value=allocate(table,arena,0,(ARENA+0xc3000,))
        table,arena,value=allocate(table,arena,99,(ARENA+0xc4000,))
        table,arena=step('Mem_DestroyPool',(0,),destroy_oracle(table,arena))
        table,arena=step('Mem_ShutdownPools',(),destroy_oracle(table,arena,shutdown=True))
        table,arena=step('Mem_ShutdownPools',(),destroy_oracle(table,arena,shutdown=True))
        # Each partial allocation failure is retained and subsequently destroyed.
        for returns in [(0,),(ARENA+0xc0000,0),(ARENA+0xc0000,ARENA+0xc1000,0)]:
            table,arena=step('Mem_CreatePool',(0,),pool_oracle(table,arena,None,(ARENA,)))
            table,arena,value=allocate(table,arena,77,returns)
            table,arena=step('Mem_ShutdownPools',(),destroy_oracle(table,arena,shutdown=True))
        # Existing-page block failure leaves uninitialized record-zero size:
        # destruction follows that nonzero word and forwards the stale pointer.
        table,arena=step('Mem_InitPools',(),pool_oracle(table,arena,None,(ARENA,),initialize=True),-1)
        table,arena,value=allocate(table,arena,1,(ARENA+0xc0000,ARENA+0xc1000,ARENA+0xc2000))
        for i in range(15):table,arena,value=allocate(table,arena,1,(ARENA+0xc2100+16*i,))
        table,arena,value=allocate(table,arena,88,(ARENA+0xc5000,0))
        table,arena=step('Mem_ShutdownPools',(),destroy_oracle(table,arena,shutdown=True))
        for name in ROUTINES:
            record(name,'emulation','pass',cases=counts[name],persistent_invocations=persistent,
                prerequisite_invocations=integration,
                scope='Exact ordered size/pointer/hierarchy reads, frees and root clear; complete heap/root/image/caller state, CRT-entry snapshots, ABI, all indices, stale/null pointers, live/cached boundary mutations, persistent actual six-body lifecycle and partial failures',
                limitations='CRT malloc/free modeled; invalid/unmapped or aliased storage, concurrency/reentry, native heap/game parity, original compiler/link layout and instruction equality unverified')
        print('PASS: %d Mem_DestroyPool and %d Mem_ShutdownPools differential invocations; %d persistent invocations (%d real prerequisite calls).'%(counts['Mem_DestroyPool'],counts['Mem_ShutdownPools'],persistent,integration))
        return counts
    except Exception as exc:
        for name in ROUTINES:record(name,phase,'fail',error=str(exc))
        raise


if __name__=='__main__':
    verify_mem_destroy()
