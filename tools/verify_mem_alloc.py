# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Mem_Alloc: complete production C versus native instructions; CRT heap modeled."""
import hashlib
import random
import struct
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX,
    UC_X86_REG_EIP, UC_X86_REG_ESP, UC_X86_REG_EFLAGS)
from verify_font_cleanup import SAVED, STACK, STOP
from verify_mem_free import PoolFixture, PoolCPU, ARENA, ARENA_SIZE
from build_decomp import build
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')
RVA, SIZE = 0x5ae10, 406
DLL = BUILD / 'mem_alloc_validation.dll'
INPUTS = ['decomp/src/mem.c', 'decomp/include/mem.h', 'decomp/target.json',
    'tools/verify_mem_alloc.py', 'tools/verify_mem_free.py', 'tools/build_decomp.py',
    'tools/verify_matching.py', 'tools/verify_font_cleanup.py',
    'tools/windows_target.py', 'tools/windows_tracking.py']
NEW_PAGE, NEW_BLOCK, PAYLOAD = ARENA+0xc0000, ARENA+0xc1000, ARENA+0xc2000


def record(kind, outcome, cases=0, **details):
    record_run(RVA, kind, outcome, inputs=INPUTS,
        artifact=DLL.relative_to(ROOT).as_posix() if DLL.exists() else None,
        cases=cases, command=('uv run python tools/verify_mem_alloc.py'
        if __name__ == '__main__' else 'uv run tools/verify_matching.py'), details=details)


def inspect_original(pe):
    raw = pe.get_data(RVA, SIZE)
    assert hashlib.sha256(raw).hexdigest() == 'b272bd31d844358c51a80948c668ed37a98108dceff066c13a67328c9a964ed3'
    assert pe.get_data(RVA+SIZE, 10) == b'\xcc'*10
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {r:(n,l,p,b) for r,n,l,p,b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    assert fpo[RVA] == (SIZE, 2, 2, 0x141b)
    ins = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(raw, 0x45ae10))
    assert sum(i.size for i in ins) == SIZE
    assert [(i.address, i.op_str) for i in ins if i.mnemonic == 'call'] == [
        (v, '0x469400') for v in [0x45ae94,0x45aebf,0x45aef7,0x45af1f,0x45af4f,0x45af84]]
    relocs = [(e.rva, struct.unpack('<I', pe.get_data(e.rva,4))[0])
        for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries
        if e.type == 3 and RVA <= e.rva < RVA+SIZE]
    assert relocs == [(0x5ae1b,0x63c6a0)]
    assert fpo[0x69400] == (20,0,1,0)
    assert hashlib.sha256(pe.get_data(0x69400,20)).hexdigest() == '7d8692fed5df9efac5f7c0937ec31305b20b064cc084472afa53c39c7333fcfc'
    # Malloc entry forwards size and the new-handler policy to __nh_malloc.
    assert pe.get_data(0xbb400,4) == b'\0'*4
    assert any(x.address == 0x64c314 and x.name == b'HeapAlloc'
               for d in pe.DIRECTORY_ENTRY_IMPORT for x in d.imports)
    for va in [0x4574f0,0x45afc0,0x4100c9,0x41aca3]:
        code = pe.get_data(va-0x400000,5)
        assert code[0] == 0xe8 and va+5+struct.unpack_from('<i',code,1)[0] == 0x45ae10


def oracle(initial, pool, size, returns, mutations):
    """Instruction-derived oracle including the three pre-bound lookahead reads."""
    data = bytearray(initial)
    events, snapshots, requests = [('table',pool*4,4)], [], []
    def read(offset):
        events.append(('read',offset,4))
        return struct.unpack_from('<I',data,offset)[0]
    def write(offset,value):
        events.append(('write',offset,4,value))
        struct.pack_into('<I',data,offset,value)
    def alloc(n):
        k = len(requests)
        assert k < len(returns), 'Unexpected malloc'
        value = returns[k]
        requests.append(n); snapshots.append(bytes(data))
        events.append(('malloc',n,value))
        for off,word in mutations.get(k,()): struct.pack_into('<I',data,off,word)
        return value
    def finish(value):
        assert len(requests) == len(returns)
        return value,bytes(data),events,snapshots,requests
    p = 0
    while read(64+4*p) != 0 and p < 64:
        page = read(64+4*p)-ARENA
        b = 0
        while read(page+4*b) != 0 and b < 64:
            block = read(page+4*b)-ARENA
            r = 0
            while read(block+8*r+4) != 0 and r < 16: r += 1
            if r != 16:
                value = alloc(size)
                if value:
                    write(block+8*r,value); write(block+8*r+4,size)
                return finish(value)
            b += 1
        if b != 64:
            block = alloc(128)
            if not block: return finish(0)
            write(page+4*b,block); block -= ARENA
            for r in range(1,16): write(block+8*r+4,0)
            value = alloc(size)
            if value:
                write(block,value); write(block+4,size)
            return finish(value)
        p += 1
    if p == 64: return finish(0)
    page = alloc(256)
    if not page: return finish(0)
    write(64+4*p,page); page -= ARENA
    for b in range(64): write(page+4*b,0)
    block = alloc(128)
    if not block: return finish(0)
    write(page,block); block -= ARENA
    for r in range(16): write(block+8*r+4,0)
    value = alloc(size)
    if value:
        write(block,value); write(block+4,size)
    return finish(value)


class AllocCPU:
    def __init__(self,pe,symbols=None):
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.image_size = (pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095
        self.cpu = Uc(UC_ARCH_X86,UC_MODE_32)
        self.cpu.mem_map(self.base,self.image_size)
        self.cpu.mem_write(self.base,pe.get_memory_mapped_image())
        self.cpu.mem_map(ARENA,ARENA_SIZE)
        self.cpu.mem_map(STACK,0x10000); self.cpu.mem_map(STOP,0x1000)
        self.entry = 0x45ae10 if symbols is None else symbols['Mem_Alloc']
        self.malloc = 0x469400 if symbols is None else symbols['malloc']
        self.table = 0x63c6a0 if symbols is None else symbols['g_memPools']
        self.extent = (0x45ae10,0x45afa6) if symbols is None else (
            self.base+next(s.VirtualAddress for s in pe.sections if s.Name.startswith(b'.text')),
            self.base+next(s.VirtualAddress+s.Misc_VirtualSize for s in pe.sections if s.Name.startswith(b'.text')))
        self.cpu.hook_add(UC_HOOK_CODE,self.hook)
        self.cpu.hook_add(UC_HOOK_MEM_READ,self.read)
        self.cpu.hook_add(UC_HOOK_MEM_WRITE,self.write)

    def read(self,cpu,access,address,n,value,unused):
        if self.table <= address < self.table+1024:
            self.events.append(('table',address-self.table,n))
        elif ARENA <= address < ARENA+ARENA_SIZE:
            self.events.append(('read',address-ARENA,n))
        elif not STACK <= address < STACK+0x10000:
            raise AssertionError('Unexpected data read %x'%address)

    def write(self,cpu,access,address,n,value,unused):
        if ARENA <= address < ARENA+ARENA_SIZE:
            self.events.append(('write',address-ARENA,n,value))
        elif not STACK <= address < STACK+0x10000:
            raise AssertionError('Unexpected data write %x'%address)

    def hook(self,cpu,address,n,unused):
        if address != self.malloc:
            assert self.extent[0] <= address < self.extent[1], 'Escaped allocation routine'
            return
        sp = cpu.reg_read(UC_X86_REG_ESP)
        k = self.call_index; self.call_index += 1
        assert k < len(self.returns), 'Unexpected malloc call'
        request = struct.unpack('<I',cpu.mem_read(sp+4,4))[0]
        assert request == self.requests[k]
        assert bytes(cpu.mem_read(ARENA,ARENA_SIZE)) == self.snapshots[k], 'CRT-entry state mismatch'
        self.events.append(('malloc',request,self.returns[k]))
        for off,word in self.mutations.get(k,()): cpu.mem_write(ARENA+off,struct.pack('<I',word))
        cpu.reg_write(UC_X86_REG_EAX,self.returns[k])
        cpu.reg_write(UC_X86_REG_ECX,0xc1c1c1c1); cpu.reg_write(UC_X86_REG_EDX,0xd2d2d2d2)
        cpu.reg_write(UC_X86_REG_EFLAGS,0x43)
        cpu.reg_write(UC_X86_REG_EIP,struct.unpack('<I',cpu.mem_read(sp,4))[0])
        cpu.reg_write(UC_X86_REG_ESP,sp+4)

    def invoke(self,data,pool,size,returns,mutations=None,seed=0):
        mutations = mutations or {}
        expected,final,events,snapshots,requests = oracle(data,pool,size,returns,mutations)
        self.events,self.call_index = [],0
        self.returns,self.mutations,self.snapshots,self.requests = returns,mutations,snapshots,requests
        cpu = self.cpu; cpu.mem_write(ARENA,bytes(data))
        table = [ARENA+0x2000]*256; table[pool] = ARENA
        cpu.mem_write(self.table,struct.pack('<256I',*table))
        sp = STACK+0x8000; cpu.mem_write(sp,struct.pack('<III',STOP,pool,size))
        stack = bytes(cpu.mem_read(sp,0x8000))
        rng = random.Random(seed); saved = [rng.getrandbits(32) for _ in SAVED]
        for reg,word in zip(SAVED,saved): cpu.reg_write(reg,word)
        cpu.reg_write(UC_X86_REG_ESP,sp); cpu.reg_write(UC_X86_REG_EFLAGS,2)
        image = bytes(cpu.mem_read(self.base,self.image_size))
        cpu.emu_start(self.entry,STOP,count=3000000)
        assert cpu.reg_read(UC_X86_REG_EIP) == STOP and cpu.reg_read(UC_X86_REG_ESP) == sp+4
        assert [cpu.reg_read(r) for r in SAVED] == saved
        assert not cpu.reg_read(UC_X86_REG_EFLAGS)&0x400
        assert bytes(cpu.mem_read(sp,0x8000)) == stack
        assert bytes(cpu.mem_read(self.base,self.image_size)) == image
        if self.events != events:
            k = next((i for i,(a,b) in enumerate(zip(self.events,events)) if a != b),min(len(self.events),len(events)))
            raise AssertionError('Event mismatch %d: %s versus %s'%(k,self.events[k:k+3],events[k:k+3]))
        assert self.call_index == len(returns)
        result = cpu.reg_read(UC_X86_REG_EAX),bytes(cpu.mem_read(ARENA,ARENA_SIZE))
        assert result == (expected,final)
        return result


def fixture(p=0,b=0,r=0,branch='record'):
    f = PoolFixture()
    for pi in range(p+1):
        limit = 64 if pi < p else (b+1 if branch == 'record' else b if branch == 'block' else 0)
        if pi == p and branch == 'page': break
        for bi in range(limit): f.block(pi,bi)
    if branch == 'block' and b == 0:
        f.block(p,0)
        f.word(f.pages[p],0)
    if branch == 'record': f.word(f.blocks[p,b]+8*r+4,0)
    return f


def verify_mem_alloc():
    verify_target(); phase = 'compilation'
    try:
        original = pefile.PE(str(TARGET)); inspect_original(original)
        build(dll=DLL)
        record('compilation','pass',scope='Complete production mem.c; strict C89; provisional Clang/LLD; nonreturning malloc/free link boundaries')
        rebuilt = pefile.PE(str(DLL))
        symbols = {s.name.decode():rebuilt.OPTIONAL_HEADER.ImageBase+s.address for s in rebuilt.DIRECTORY_ENTRY_EXPORT.symbols if s.name}
        prefix = rebuilt.get_data(symbols['Mem_Alloc']-rebuilt.OPTIONAL_HEADER.ImageBase,SIZE)
        record('raw_bytes','pass' if prefix == original.get_data(RVA,SIZE) else 'different',scope='Raw prefix only; instruction equality unverified')
        phase = 'emulation'; cases = 0
        a,b = AllocCPU(original),AllocCPU(rebuilt,symbols)
        def compare(f,pool=0,size=123,returns=(PAYLOAD,),mutations=None):
            nonlocal cases
            args = (bytes(f.data),pool,size,returns,mutations,cases)
            actual = a.invoke(*args)
            assert b.invoke(*args) == actual
            cases += 1
            return actual
        # Every pool; each record; unsigned request and return boundaries.
        for pool in range(256):
            compare(fixture(r=pool%16),pool,size=[0,1,128,256,0x80000000,0xffffffff][pool%6])
        for r in range(16):
            for size in [0,1,0x80000000,0xffffffff]:
                for value in [0,PAYLOAD,0xffffffff]: compare(fixture(r=r),size=size,returns=(value,))
        # Every page/block insertion and existing-record position, including complete preceding scans.
        for p in range(64):
            compare(fixture(p=p,branch='page'),returns=(NEW_PAGE,NEW_BLOCK,PAYLOAD))
            compare(fixture(p=p,r=15))
        for bi in range(64):
            compare(fixture(b=bi,branch='block'),returns=(NEW_BLOCK,PAYLOAD))
            compare(fixture(b=bi,r=15))
        # Every allocation failure point; partial attachments must persist.
        for branch,success in [('record',(PAYLOAD,)),('block',(NEW_BLOCK,PAYLOAD)),
                               ('page',(NEW_PAGE,NEW_BLOCK,PAYLOAD))]:
            for k in range(len(success)):
                compare(fixture(branch=branch),returns=success[:k]+(0,))
        # Exhausted arrays read their trailing word even if it is zero.
        for sentinel in [0,0xffffffff]:
            f = PoolFixture(dense=True)
            f.word(320,sentinel)
            compare(f,returns=())
            f = fixture(b=63,r=15)
            f.word(f.blocks[0,63]+124,99)
            f.word(f.blocks[0,63]+132,sentinel)
            compare(f,returns=(NEW_PAGE,NEW_BLOCK,PAYLOAD))
            f = fixture(p=1,branch='page')
            f.word(f.pages[0]+256,sentinel)
            compare(f,returns=(NEW_PAGE,NEW_BLOCK,PAYLOAD))
        # Cached destination survives hierarchy detachment at each modeled boundary.
        f = fixture(r=7); off = f.blocks[0,0]+56
        compare(f,mutations={0:[(64,0),(f.pages[0],0),(off,0x11223344),(off+4,99)]})
        f = fixture(b=1,branch='block')
        compare(f,returns=(NEW_BLOCK,PAYLOAD),mutations={0:[(64,0)],1:[(f.pages[0]+4,0)]})
        f = fixture(branch='page')
        compare(f,returns=(NEW_PAGE,NEW_BLOCK,PAYLOAD),mutations={1:[(64,0)],2:[(NEW_PAGE-ARENA,0)]})
        # Existing-page failure leaves record zero's allocator-pattern size untouched;
        # subsequent allocation skips it, while zero-size payloads remain reusable.
        f = fixture(b=1,branch='block')
        _,data = compare(f,returns=(NEW_BLOCK,0))
        f.data[:] = data
        compare(f)
        f = fixture()
        _,data = compare(f,size=0); f.data[:] = data
        compare(f,size=0xffffffff,returns=(0xffffffff,))
        # A later reusable record cannot bypass the first missing page/block.
        f = fixture(branch='page'); f.match(1,0,0,size=0)
        compare(f,returns=(NEW_PAGE,NEW_BLOCK,PAYLOAD))
        f = fixture(b=1,branch='block'); f.match(0,2,0,size=0)
        compare(f,returns=(NEW_BLOCK,PAYLOAD))
        f = PoolFixture(dense=True); f.word(f.blocks[63,63]+124,0)
        compare(f)
        rng = random.Random(RVA)
        for k in range(64):
            f = fixture(p=rng.randrange(4),b=rng.randrange(6),r=rng.randrange(16))
            for off in f.blocks.values():
                for r in range(16):
                    if struct.unpack_from('<I',f.data,off+8*r+4)[0]:
                        f.word(off+8*r+4,rng.choice([1,2,0x80000000,0xffffffff]))
            compare(f,pool=rng.randrange(256),size=rng.getrandbits(32),
                    returns=(0 if k%4 == 0 else rng.getrandbits(32) or 1,))
        # Retry each new-page partial failure with the retained hierarchy.
        for failed in [(NEW_PAGE,0),(NEW_PAGE,NEW_BLOCK,0)]:
            f = fixture(branch='page'); _,data = compare(f,returns=failed)
            f.data[:] = data
            compare(f,returns=(NEW_BLOCK,PAYLOAD) if len(failed)==2 else (PAYLOAD,))
        # Allocation/free round trips execute both real routines, then real reuse.
        fa,fb = PoolCPU(original),PoolCPU(rebuilt,symbols)
        for branch,success in [('record',(PAYLOAD,)),('block',(NEW_BLOCK,PAYLOAD)),
                               ('page',(NEW_PAGE,NEW_BLOCK,PAYLOAD))]:
            f = fixture(branch=branch)
            _,data = compare(f,returns=success)
            released = fa.invoke(data,0,PAYLOAD,seed=cases)
            assert fb.invoke(data,0,PAYLOAD,seed=cases) == released
            f.data[:] = released[1]
            compare(f,size=987)
        record('emulation','pass',cases=cases,scope='Original versus complete production C; CRT malloc modeled; exact ordered reads/writes including lookahead; complete image/arena/CRT-entry state; failure persistence, reuse, mutations, ABI; three real Mem_Free round trips',
            limitations='Readable trailing words required; invalid pools/hierarchies, aliasing, concurrency, general reentry, native heap/game parity and instruction equality unverified')
        print('PASS: %d Mem_Alloc differential cases and three real Mem_Free round trips.'%cases)
        return cases
    except Exception as exc:
        record(phase,'fail',error=str(exc)); raise


if __name__ == '__main__':
    verify_mem_alloc()
