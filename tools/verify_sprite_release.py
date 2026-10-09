# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Storage release: independent instruction-derived oracle and real free helpers."""
import hashlib
import itertools
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from build_decomp import build
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run
if not __debug__:
    raise RuntimeError('Verification requires assertions')
DIGEST = '7563059eb087b1961ce5aac8901e5ed29574c2f1a9354e62ed943d5842fb0072'
DLL = BUILD / 'sprite_release_validation.dll'
INPUTS = ['decomp/src/geputget.c','decomp/include/geputget.h',
    'decomp/src/mem.c','decomp/include/mem.h','decomp/src/lisa3d.c',
    'decomp/include/lisa3d.h','decomp/target.json','tools/build_decomp.py',
    'tools/verify_sprite_free.py','tools/verify_sprite_packing.py',
    'tools/verify_sprite_release.py','tools/verify_matching.py',
    'tools/windows_target.py','tools/windows_tracking.py']
import struct
from verify_sprite_free import Session as FreeSession, inspect as inspect_free
from verify_sprite_packing import State as PackingState, GapStop, ARENA, SIZE, MASK, HEAD, TABLE
from unicorn.x86_const import UC_X86_REG_ESP

NAME = 'Gfx_ReleaseSpritePackingStorage'
RVA = 0x618b0
D, P, C, L, B = [ARENA+x for x in (0x100,0x1000,0x2000,0x3000,0x4000)]
PIXELS = ARENA+0x80000


class State(PackingState):
    def __init__(self,seed):
        super().__init__(seed)
        self.pending = []
        self.limit = None

    def put(self,address,words):
        data,offset = self.region(address,len(words)*4)
        assert data is not None
        struct.pack_into('<'+'I'*len(words),data,offset,*[x&MASK for x in words])

    def read(self,address):
        value = super().read(address)
        if self.limit and self.read_count == self.limit:
            self.fault = ('budget',self.limit)
            raise GapStop
        return value

    def write(self,address,value):
        if not super().write(address,value):
            raise GapStop
        return True

    def free(self,address):
        if address:
            self.events.append(('free',address,self.snapshot()))
            mutations = self.pending.pop(0) if self.pending else []
            for dest,word in mutations:
                self.put(dest,[word])

    def release(self,descriptor,schedules=(),limit=None):
        self.events,self.fault,self.read_count = [],None,0
        self.pending,self.limit = list(schedules),limit
        try:
            height = self.read(descriptor+8)
            def admitted(word):
                return 1 <= word <= 256
            if not admitted(height):
                return
            width = self.read(descriptor+4)
            if not admitted(width):
                return
            page = self.read(HEAD)
            pixels = self.read(descriptor+16)
            while page:
                if self.read(page+8) == pixels&0xffff0000:
                    break
                page = self.read(page+20)
            if not page:
                return
            container = self.read(page+12)
            # The first child is dereferenced even when null.
            while self.read(container+8) != pixels&0xffffff00:
                container = self.read(container+20)
                if not container:
                    return
            leaf = self.read(container+12)
            while self.read(leaf+8) != pixels:
                leaf = self.read(leaf+20)
                if not leaf:
                    return
            previous_bucket = 0
            bucket = self.read(TABLE+height*4)
            while self.read(bucket+4) != container:
                previous_bucket = bucket
                bucket = self.read(bucket)
                if not bucket:
                    return
            successor = self.read(leaf+20)
            if successor == 0 and self.read(leaf+16) == 0:
                successor = self.read(container+20)
                if successor == 0 and self.read(container+16) == 0:
                    base = self.read(page+8)
                    self.free(self.read((base-4)&MASK))
                    previous = self.read(page+16)
                    next_page = self.read(page+20)
                    self.write(previous+20 if previous else HEAD,next_page)
                    next_page = self.read(page+20)
                    if next_page:
                        self.write(next_page+16,self.read(page+16))
                    self.free(page)
                else:
                    previous = self.read(container+16)
                    self.write(previous+20 if previous else page+12,successor)
                    successor = self.read(container+20)
                    if successor:
                        self.write(successor+16,self.read(container+16))
                successor = self.read(bucket)
                if previous_bucket:
                    self.write(previous_bucket,successor)
                else:
                    live_height = self.read(descriptor+8)
                    self.write((TABLE+((live_height<<2)&MASK))&MASK,successor)
                self.free(bucket)
                self.free(container)
            else:
                previous = self.read(leaf+16)
                self.write(previous+20 if previous else container+12,successor)
                successor = self.read(leaf+20)
                if successor:
                    self.write(successor+16,self.read(leaf+16))
            self.free(leaf)
        except GapStop:
            return


class Session(FreeSession):
    def __init__(self,pe,state,original):
        super().__init__(pe,state,original)
        if original:
            self.symbols[NAME] = RVA+0x400000
        self.schedules = []

    def code(self,uc,address,size,data):
        if address == self.symbols['free']:
            self.mutations = self.schedules.pop(0) if self.schedules else []
        super().code(uc,address,size,data)


def fixture(seed=0,page_count=1,container_count=1,leaf_count=1,bucket_count=1,
        page_index=0,container_index=0,leaf_index=0,bucket_index=0):
    state = State(seed)
    state.put(D,[0,3,7,0,PIXELS+0x103,256]+[0]*10)
    state.put(HEAD,[P])
    state.put(TABLE+7*4,[B])
    state.put(PIXELS-4,[ARENA+0x50000])
    def linked(base,count,index,target_pixels,children):
        for k in range(count):
            address = base+k*24
            pixels = target_pixels if k == index else target_pixels+0x10000*(k+1)
            state.put(address,[k,k+1,pixels,children,
                address-24 if k else 0,address+24 if k+1<count else 0])
        return base+index*24
    leaf = linked(L,leaf_count,leaf_index,PIXELS+0x103,0)
    container = linked(C,container_count,container_index,PIXELS+0x100,L)
    page = linked(P,page_count,page_index,PIXELS,C)
    for k in range(bucket_count):
        state.put(B+8*k,[B+8*(k+1) if k+1<bucket_count else 0,
            container if k == bucket_index else C+0x800+24*k])
    return state,page,container,leaf,B+8*bucket_index


def inspect(pe):
    inspect_free(pe)
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    assert fpo[RVA] == (419,1,1,0x1410)
    assert hashlib.sha256(pe.get_data(RVA,419)).hexdigest() == DIGEST
    md = Cs(CS_ARCH_X86,CS_MODE_32)
    instructions = list(md.disasm(pe.get_data(RVA,419),RVA+0x400000))
    assert sum(i.size for i in instructions) == 419
    assert [i.address for i in instructions if i.mnemonic == 'ret'] == [
        0x461920,0x461937,0x46194e,0x461975,0x461a52]
    assert [(i.address,i.op_str) for i in instructions if i.mnemonic == 'call'] == [
        (0x46199c,'0x461a60'),(0x4619c9,'0x45f8b0'),(0x461a13,'0x45f8b0'),
        (0x461a1c,'0x45f8b0'),(0x461a43,'0x45f8b0')]
    reloc = {e.rva for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries if e.type == 3}
    assert {r:struct.unpack('<I',pe.get_data(r,4))[0] for r in reloc if RVA <= r < RVA+419} == {
        0x618ef:HEAD,0x6195a:TABLE,0x619b8:HEAD,0x61a0e:TABLE}
    assert not any(struct.unpack('<I',pe.get_data(r,4))[0] == RVA+0x400000 for r in reloc)
    assert pe.get_data(0x61a53,13) == b'\xcc'*13
    assert [(r,i.address) for r,(n,*_) in fpo.items()
        for i in md.disasm(pe.get_data(r,n),0x400000+r)
        if i.mnemonic in ('call','jmp') and i.op_str == '0x4618b0'] == [
            (0x61360,0x4613c0),(0x61360,0x46146c)]
    assert hashlib.sha256(pe.get_data(0x61360,329)).hexdigest() == '81b591def47aa754c8a41dd4255ba978ad5ff82514ca2533537dd607077a2020'
    print('Authenticated release full extent, FPO, returns, calls, globals and both caller sites.',flush=True)


def verify_sprite_release():
    verify_target()
    original = pefile.PE(str(TARGET))
    inspect(original)
    build(dll=DLL)
    rebuilt = pefile.PE(str(DLL))
    coverage = {'comparisons':0,'persistent_followups':0,'faults':0,
        'cycle_prefixes':0,'crt_calls':0,'aliases':0,'mutation_calls':0}
    def sequence(state,descriptor=D,schedules=(),limit=None,repeats=2,alias=False,free_order=None):
        sessions = [Session(p,state,o) for p,o in ((original,True),(rebuilt,False))]
        for repeat in range(repeats):
            state.release(descriptor,schedules,limit)
            if repeat == 0 and free_order is not None:
                assert [e[1] for e in state.events if e[0] == 'free'] == free_order
            for session in sessions:
                session.schedules = list(schedules)
                session.read_count = 0
                session.read_limit = limit
                session.run_free(NAME,descriptor,state,coverage['comparisons'],reply=0xdeadc0de)
                assert session.schedules == state.pending
            coverage['comparisons'] += 1
            coverage['persistent_followups'] += int(repeat != 0)
            coverage['faults'] += int(state.fault is not None and state.fault[0] != 'budget')
            coverage['cycle_prefixes'] += int(state.fault is not None and state.fault[0] == 'budget')
            coverage['crt_calls'] += sum(e[0] == 'free' for e in state.events)
            coverage['aliases'] += int(alias)
            coverage['mutation_calls'] += int(bool(schedules))
            if state.fault:
                break
    options = [(1,0),(2,0),(2,1),(3,1)]
    for seed,choices in enumerate(itertools.product(options,repeat=4)):
        pc,cc,lc,bc = [x[0] for x in choices]
        pi,ci,li,bi = [x[1] for x in choices]
        state,page,container,leaf,bucket = fixture(seed,pc,cc,lc,bc,pi,ci,li,bi)
        order = [leaf] if lc > 1 else ([bucket,container,leaf] if cc > 1 else
            [ARENA+0x50000,page,bucket,container,leaf])
        sequence(state,free_order=order)
    # Independently specified signed boundary rejections and four corners.
    for height,width in itertools.product((0,1,256,257,0x7fffffff,0x80000000,MASK),repeat=2):
        state,*_ = fixture()
        state.put(D+4,[width,height])
        if height not in (1,256) or width not in (1,256):
            sequence(state,free_order=[])
    for height,width in itertools.product((1,256),repeat=2):
        state,*_ = fixture()
        state.put(D+4,[width,height]);state.put(TABLE+height*4,[B])
        sequence(state,free_order=[ARENA+0x50000,P,B,C,L])
    # Search miss at every level, with no free calls or writes.
    for dest,value in ((HEAD,0),(P+8,0),(C+8,0),(L+8,0),(B+4,0)):
        state,*_ = fixture();state.put(dest,[value]);sequence(state,free_order=[])
    # Every retained/released extent index, regardless of dimensions used
    # for lookup: width is a gate only, captured height selects the bucket.
    for height in range(1,257):
        state,*_ = fixture(height);state.put(D+8,[height]);state.put(TABLE+height*4,[B])
        sequence(state,free_order=[ARENA+0x50000,P,B,C,L])
    mutations = [[(D+8,0)],[(D+8,256)],[(P+16,P+24),(P+20,P+48)],
        [(P+8,PIXELS+0x10000)],[(B,B+8)],[(C+12,0)],[(L+20,L),(L+16,L)]]
    for seed,change in enumerate(mutations):
        for phase in range(5):
            state,*_ = fixture(seed)
            state.put(P+24,[0,1,0,0,0,0]);state.put(P+48,[0,1,0,0,0,0])
            schedules = [[] for _ in range(5)];schedules[phase] = change
            sequence(state,schedules=schedules,repeats=1)
    # Header zero still reads it and skips just the first CRT free.
    state,*_ = fixture();state.put(PIXELS-4,[0])
    sequence(state,free_order=[P,B,C,L])
    bad = 0x80000000
    for dest,value in ((HEAD,bad),(P+12,0),(P+12,bad),(C+12,0),(C+12,bad),
            (TABLE+28,0),(TABLE+28,bad),(L+16,bad),(L+20,bad),(C+16,bad),
            (C+20,bad),(P+16,bad),(P+20,bad)):
        state,*_ = fixture();state.put(dest,[value]);sequence(state)
    for descriptor in (bad,ARENA+SIZE-8,ARENA+SIZE-4):
        state,*_ = fixture();sequence(state,descriptor=descriptor)
    # Ordered reads of partial mapped descriptor/node/header storage.
    for dest,value in ((HEAD,ARENA+SIZE-8),(P+12,ARENA+SIZE-8),
            (C+12,ARENA+SIZE-8),(TABLE+28,ARENA+SIZE-4)):
        state,*_ = fixture();state.put(dest,[value]);sequence(state)
    for previous,next in itertools.product((0,P,C,L,B,D),(0,P,C,L,B,D)):
        state,*_ = fixture();state.put(L+16,[previous,next]);sequence(state,alias=True)
    for descriptor in (L,C):
        state,*_ = fixture()
        state.put(P,[0,1,0,C,0,0])
        state.put(C,[0,1,256 if descriptor == C else 0,L,257 if descriptor == C else 0,0])
        state.put(L,[0,1,3 if descriptor == L else 257,0,3 if descriptor == L else 0,0])
        state.put(TABLE+(12 if descriptor == L else 256*4),[B])
        sequence(state,descriptor=descriptor,alias=True)
    for changes in ([(P+8,0),(P+20,P)],[(C+8,0),(C+20,C)],
            [(L+8,0),(L+20,L)],[(B+4,0),(B,B)]):
        state,*_ = fixture()
        for dest,word in changes:
            state.put(dest,[word])
        sequence(state,limit=31,repeats=1)
    details = {'scope':'Real release, aligned-free and byte-free bodies; independent ordered raw-address oracle; CRT free modeled with live mutations and volatile clobbers',
        'coverage':coverage,'checks':'all mapped arena/global/image bytes, exact ordered reads/writes and CRT arguments/pre-free snapshots; persistent followups, signed boundaries, masks, all bucket heights, head/interior/tail/singleton list removal, faults/retained effects, aliases and externally bounded cycles; cdecl stack/caller/nonvolatile/DF on return; incidental EAX excluded',
        'compiler':'Provisional Clang/LLD strict C89 PE32; original compiler/link layout unresolved',
        'limitations':'No instruction equality, whole ImageOp integration, native heap/graphics/game/fault parity, arbitrary reentry/concurrency, exhaustive aliases, caller-stack/global pointer aliases or out-of-table live retry-height parity; CRT free remains modeled'}
    for kind in ('compilation','emulation'):
        record_run(RVA,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
            cases=coverage['comparisons'] if kind == 'emulation' else 0,
            command='uv run python tools/verify_sprite_release.py',details=details)
    print('PASS packing release:',coverage,flush=True)
    return coverage


if __name__ == '__main__':
    verify_sprite_release()
