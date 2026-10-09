# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Complete native handle operation with real ImageOp resource descendants."""
import hashlib
import random
import struct
import pefile
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
from unicorn import UcError
from unicorn.x86_const import UC_X86_REG_ESP,UC_X86_REG_EAX,UC_X86_REG_EIP,UC_X86_REG_EFLAGS
from verify_matching import STACK,STOP,PRESERVED
from verify_sprite_imageop import State as ImageState,Session as ImageSession,inspect as inspect_image,fixture as image_fixture,S,R,RECORDS,CTL,NAME_SOURCE,NAME_COPY,PIXEL_SOURCE,INPUTS as IMAGE_INPUTS
from verify_sprite_packing import GapStop,ARENA,SIZE,MASK
from build_decomp import build
from windows_target import ROOT,BUILD,verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')
NAME,RVA,LENGTH = 'Gfx_SpriteHandleOpNative',0x5c830,207
DIGEST = '77a47f976fb496d9826b327cdf81fa38f527ee1a1fb3ce7085124b6ab39a0ec7'
DLL = BUILD/'sprite_handleop_validation.dll'
FL,SCRATCH,POOL,CURSOR = 0x512c58,0x514b98,0x514bd8,0x51a998
FIELDS = [('g_nativeSpriteFreeList',FL,8000),('g_nativeSpriteScratch',SCRATCH,64),
    ('g_nativeSpriteHandles',POOL,24000),('g_nativeSpriteFreeCursor',CURSOR,4)]
INPUTS = list(dict.fromkeys(IMAGE_INPUTS+['tools/verify_sprite_handleop.py']))


class State(ImageState):
    def __init__(self,seed):
        super().__init__(seed)
        self.handle_fields = [bytearray(n) for _,_,n in FIELDS]

    def region(self,address,size):
        for (_,base,n),data in zip(FIELDS,self.handle_fields):
            if base <= address and address+size <= base+n:
                return data,address-base
        return super().region(address,size)

    def snapshot(self):
        return super().snapshot()+tuple(bytes(x) for x in self.handle_fields)

    def handleop(self,source,handle,replies,free_schedules=(),limit=None):
        self.events,self.fault = [],None
        self.replies,self.free_schedules = list(replies),list(free_schedules)
        self.root_reads,self.root_limit = 0,limit
        selected = handle
        try:
            if selected == 0:
                if source == 0:
                    return 0
                if self.read(CURSOR) == FL:
                    return 0
                self.write(CURSOR,self.read(CURSOR)-4)
                cursor = self.read(CURSOR)
                selected = self.read(cursor)
                self.write(selected,0)
            else:
                image_id = self.read(selected)
                if image_id == 0:
                    return 0
                if source == 0:
                    self.invoke_image(0,image_id)
                    self.write(selected,0)
                    cursor = self.read(CURSOR)
                    self.write(cursor,selected)
                    self.write(CURSOR,self.read(CURSOR)+4)
                    return 0
            self.write(SCRATCH,0)
            for index in range(1,6):
                self.write(SCRATCH+index*4,self.read(source+index*4))
            image_id = self.read(selected)
            result = self.invoke_image(SCRATCH,image_id)
            self.write(selected,result)
            self.write(selected+4,self.read(source+24))
            self.write(selected+8,self.read(source+28))
            return selected
        except GapStop:
            return None

    def invoke_image(self,source,image_id):
        self.boundary('Gfx_SpriteImageOpNative',(source,image_id))
        prefix = self.events
        result = ImageState.imageop(self,source,image_id,self.replies,self.free_schedules)
        self.events = prefix+self.events
        if self.fault:
            raise GapStop
        return result


class Session(ImageSession):
    def __init__(self,pe,state,original):
        super().__init__(pe,state,original)
        self.handle_fields = []
        self.original = original
        self.initial_arena = bytes(state.arena)
        if original:
            self.symbols[NAME] = 0x400000+RVA
            self.symbols.update({name:base for name,base,_ in FIELDS})
        for (name,logical,n),data in zip(FIELDS,state.handle_fields):
            physical = self.symbols[name]
            self.fields.append((logical,physical,n))
            self.handle_fields.append((logical,physical,n))
        for (_,physical,n),data in zip(self.handle_fields,state.handle_fields):
            words = struct.unpack('<'+'I'*(n//4),data)
            self.uc.mem_write(physical,struct.pack('<'+'I'*len(words),*[self.physical(x) for x in words]))

    def snapshot(self):
        parts = super().snapshot()
        # Relocate newly stored declared static-pointer words in the arena.
        # Preserve untouched random bytes even if they numerically collide with
        # the compiled address range. Fixture scalar fields avoid such collisions.
        if not self.original:
            arena = parts[0]
            changed = None
            pos = arena.find(b'\x10',3)
            while pos >= 0:
                offset = pos-3
                if offset%4 == 0 and arena[offset:offset+4] != self.initial_arena[offset:offset+4]:
                    value = struct.unpack_from('<I',arena,offset)[0]
                    logical = self.normalize(value)
                    if value != logical:
                        if changed is None:
                            changed = bytearray(arena)
                        struct.pack_into('<I',changed,offset,logical)
                pos = arena.find(b'\x10',pos+1)
            if changed is not None:
                parts = (bytes(changed),)+parts[1:]
        for _,physical,n in self.handle_fields:
            words = struct.unpack('<'+'I'*(n//4),self.uc.mem_read(physical,n))
            parts += (struct.pack('<'+'I'*len(words),*[self.normalize(x) for x in words]),)
        return parts

    def memory(self,uc,access,address,size,value,data):
        start = len(self.events)
        super().memory(uc,access,address,size,value,data)
        if len(self.events) > start and size == 4:
            event = self.events[-1]
            self.events[-1] = event[:3]+(self.normalize(event[3]),)

    def code(self,uc,address,size,data):
        if address == self.symbols['Gfx_SpriteImageOpNative']:
            sp = uc.reg_read(UC_X86_REG_ESP)
            args = struct.unpack('<II',uc.mem_read(sp+4,8))
            self.events.append(('entry','Gfx_SpriteImageOpNative',tuple(self.normalize(x) for x in args),self.snapshot()))
        super().code(uc,address,size,data)

    def run_handleop(self,source,handle,replies,expected,result,seed,free_schedules=(),limit=None):
        self.events,self.fault = [],None
        self.replies = [(self.physical(address),
            [(dest,self.physical(word)) for dest,word in mutations])
            for address,mutations in replies]
        self.free_schedules = [[(dest,self.physical(word)) for dest,word in mutations]
            for mutations in free_schedules]
        self.schedule,self.seen,self.limit,self.total = {},{},limit,0
        self.pending_mutations,self.seed = [],seed
        uc,sp = self.uc,STACK+0x8000
        uc.mem_write(sp,struct.pack('<III',STOP,self.physical(source),self.physical(handle)))
        caller = bytes(uc.mem_read(sp,0x8000))
        saved = [random.Random(seed+i).getrandbits(32) for i in range(4)]
        for reg,value in zip(PRESERVED,saved):
            uc.reg_write(reg,value)
        uc.reg_write(UC_X86_REG_ESP,sp)
        uc.reg_write(UC_X86_REG_EFLAGS,2)
        before = bytearray(uc.mem_read(self.base,self.size))
        try:
            uc.emu_start(self.symbols[NAME],STOP,count=1000000)
        except UcError:
            assert self.fault is not None
        assert self.fault == expected.fault, (seed,self.fault,expected.fault)
        if self.events != expected.events:
            for k,(a,b) in enumerate(zip(self.events,expected.events)):
                if a != b:
                    raise AssertionError((seed,k,a[:3],b[:3],len(self.events),len(expected.events)))
            raise AssertionError((seed,len(self.events),len(expected.events)))
        assert self.snapshot() == expected.snapshot(), seed
        assert self.replies == expected.replies and self.free_schedules == expected.free_schedules
        after = bytearray(uc.mem_read(self.base,self.size))
        for _,p,n in self.fields:
            before[p-self.base:p-self.base+n] = after[p-self.base:p-self.base+n]
        assert before == after, 'Unrelated image bytes changed'
        assert bytes(uc.mem_read(sp,0x8000)) == caller
        if expected.fault is None:
            assert uc.reg_read(UC_X86_REG_EIP) == STOP
            assert self.normalize(uc.reg_read(UC_X86_REG_EAX)) == result
            assert uc.reg_read(UC_X86_REG_ESP) == sp+4
            assert [uc.reg_read(r) for r in PRESERVED] == saved
            assert uc.reg_read(UC_X86_REG_EFLAGS)&0x400 == 0


def fixture(seed=0,cursor=2000):
    base = image_fixture(seed)
    state = State(seed)
    for attr in ('arena','template','head','table','low','high','control'):
        setattr(state,attr,getattr(base,attr))
    state.put(FL,[POOL+12*k for k in range(2000)])
    state.put(CURSOR,[FL+cursor*4])
    state.put(SCRATCH,[0]*16)
    state.put(S+24,[0x13579bdf,0x87654321])
    return state


def inspect(pe):
    inspect_image(pe)
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    raw = pe.get_data(RVA,LENGTH)
    assert fpo[RVA] == (207,0,2,0x202) and hashlib.sha256(raw).hexdigest() == DIGEST
    md = Cs(CS_ARCH_X86,CS_MODE_32);ins = list(md.disasm(raw,0x400000+RVA))
    assert sum(i.size for i in ins) == LENGTH
    assert [i.address for i in ins if i.mnemonic == 'ret'] == [0x45c84a,0x45c85b,0x45c8c7,0x45c8d2,0x45c8fe]
    assert [(i.address,i.op_str) for i in ins if i.mnemonic == 'call'] == [(0x45c8ad,'0x461360'),(0x45c8de,'0x461360')]
    reloc = {e.rva:struct.unpack('<I',pe.get_data(e.rva,4))[0]
        for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries if e.type == 3}
    expected = {0x5c84d:CURSOR,0x5c851:FL,0x5c85e:CURSOR,0x5c864:CURSOR,
        0x5c872:SCRATCH,0x5c87e:SCRATCH+4,0x5c887:SCRATCH+8,0x5c890:SCRATCH+12,
        0x5c898:SCRATCH+16,0x5c8a1:SCRATCH+20,0x5c8a9:SCRATCH,0x5c8ed:CURSOR,0x5c8f8:CURSOR}
    assert {r:v for r,v in reloc.items() if RVA <= r < RVA+LENGTH} == expected
    assert [r for r,v in reloc.items() if v == 0x400000+RVA] == [0x56eb6]
    assert pe.get_data(RVA+LENGTH,1) == b'\xcc'
    assert not [i.address for r,(n,*_) in fpo.items() for i in md.disasm(pe.get_data(r,n),0x400000+r)
        if i.mnemonic == 'call' and i.op_str == '0x45c830']
    # Independently verified two-word indirect dispatch consumer.
    assert fpo[0x56d40] == (20,0,2,0)
    assert hashlib.sha256(pe.get_data(0x56d40,20)).hexdigest() == 'f76b5ff746f37d91b851c1a25161196baa69ffe94b4bfe1250a607da04aa97bc'
    print('Authenticated HandleOp extent, calls, relocations, dispatch and consumer.',flush=True)


def verify_sprite_handleop():
    original = pefile.PE(data=verify_target());inspect(original)
    build(dll=DLL);rebuilt = pefile.PE(str(DLL))
    coverage = {'comparisons':0,'persistent_followups':0,'faults':0,'aliases':0,'mutation_calls':0,'packed_calls':0}
    def sequence(state,calls,alias=False,packed=False):
        sessions = [Session(pe,state,k == 0) for k,pe in enumerate((original,rebuilt))]
        for index,args in enumerate(calls):
            result = state.handleop(**args)
            for session in sessions:
                session.run_handleop(expected=state,result=result,seed=coverage['comparisons'],**args)
            coverage['comparisons'] += 1
            coverage['persistent_followups'] += int(index > 0)
            coverage['faults'] += int(state.fault is not None)
            coverage['aliases'] += int(alias)
            coverage['packed_calls'] += int(packed)
            coverage['mutation_calls'] += int(any(x[1] for x in args.get('replies',[])) or bool(args.get('free_schedules')))
    def call(source=S,handle=0,replies=(),free_schedules=()):
        return dict(source=source,handle=handle,replies=list(replies),free_schedules=list(free_schedules))
    for seed in range(4):
        state=fixture(seed)
        sequence(state,[call(0),call(handle=POOL),call(handle=POOL+23988)])
        state=fixture(seed,0);sequence(state,[call()])
        state=fixture(seed);h=POOL+23988
        sequence(state,[call(replies=[(R,[])]),call(handle=h),call(0,h),call(0,h)])
    # Existing nonzero IDs still return the handle when ImageOp rejects the ID.
    for image_id in (1,2,3,4,0xffffffff,0x80000000,0x7fffffff):
        state=fixture(cursor=1999);state.put(POOL,[image_id,11,22])
        replies=[(R,[])] if 0 < image_id < 4 else []
        sequence(state,[call(handle=POOL,replies=replies),call(0,POOL),call(0,POOL)])
    # Representative positions include both pool bounds and internal gaps.
    for cursor in (1,2,3,17,63,511,999,1000,1998,1999,2000):
        state=fixture(cursor=cursor);h=POOL+12*(cursor-1)
        sequence(state,[call(replies=[(R,[])]),call(handle=h),call(0,h)])
    # Three simultaneous live handles, middle deletion and reuse.
    state=fixture();h0,h1,h2=POOL+23988,POOL+23976,POOL+23964
    sequence(state,[call(replies=[(R,[])]),call(replies=[(R+64,[])]),
        call(replies=[(R+128,[])]),call(0,h1),call(replies=[(R+192,[])]),
        call(0,h0),call(0,h2),call(0,h1)])
    for next_id in (0,1,999,1000,1001):
        state=fixture();state.put(CTL,[next_id,0x13579bdf,0,0])
        grows=next_id//1000+1
        replies=[(RECORDS+0x20000*k,[]) for k in range(grows)]+[(R,[])]
        sequence(state,[call(replies=replies)])
    # Scratch words 6..15 survive; only source 1..5 are copied by HandleOp.
    for seed in range(8):
        state=fixture(seed);state.put(SCRATCH+24,[0x210000+k+seed for k in range(10)])
        state.put(S,[0xdeadbeef,3,0,0x11223344,PIXEL_SOURCE,8,101,202]+list(range(8)))
        sequence(state,[call(replies=[(R,[])])])
    # Both heaps and record cleanup can mutate later source/cursor/handle reads.
    state=fixture();h=POOL+23988
    sequence(state,[call(replies=[(R,[(S+24,1024),(S+28,512)])]),call(0,h)])
    state=fixture();h=POOL+23988
    sequence(state,[call(replies=[(R,[(SCRATCH,NAME_SOURCE)]),(NAME_COPY,[])]),
        call(0,h,free_schedules=[[(CURSOR,FL+8),(h,0x1234)],[]])])
    state=fixture();h=POOL+23988
    sequence(state,[call(replies=[(R,[])]),call(0,h,free_schedules=[[(CURSOR,FL+8)]])])
    # Whole static-object aliases use verified relocation bindings. Bit patterns
    # used as scalar fixture values do not coincide with compiled object addresses.
    for source,handle in ((SCRATCH,0),(POOL+23988-24,POOL+23988),
            (POOL+96,POOL+96),(S,SCRATCH),(FL,0),(S,FL+8)):
        state=fixture();state.put(SCRATCH,[0,3,0,0,PIXEL_SOURCE,8,0,0]+[0]*8)
        if handle:
            state.put(handle,[1,3,0])
        if source==POOL+23988-24:
            state.put(source+4,[3,0,0,PIXEL_SOURCE,8])
            state.put(handle,[1,111,222])
        sequence(state,[call(source,handle,[(R,[])])],alias=True)
    # Packed creation, replacement and deletion execute all real child bodies.
    page,backing,container,bucket,leaf=[ARENA+x for x in (0x20000,0x30000,0x21000,0x22000,0x23000)]
    for width,height in ((1,1),(3,1),(7,3),(16,16),(1,256),(256,1)):
        state=fixture();state.put(S+4,[width,height]);h=POOL+23988
        replies=[(R,[]),(page,[]),(backing,[]),(container,[]),(bucket,[]),(leaf,[])]
        sequence(state,[call(replies=replies),call(0,h)],packed=True)
    state=fixture();state.put(S+8,[1]);h=POOL+23988
    sequence(state,[call(replies=[(R,[]),(page,[]),(backing,[]),(container,[]),(bucket,[]),(leaf,[])]),
        call(handle=h,replies=[(page+0x100,[]),(backing+0x20000,[]),(container+0x100,[]),(bucket+0x100,[]),(leaf+0x100,[])]),
        call(0,h)],packed=True)
    # Concrete aligned/full-unmapped faults retain pop/clear/scratch/child effects.
    bad=0x80000000
    for source in (bad,ARENA+SIZE-4,ARENA+SIZE-16):
        sequence(fixture(),[call(source)])
    for cursor in (bad,0):
        state=fixture();state.put(CURSOR,[cursor]);sequence(state,[call()])
    for selected in (bad,ARENA+SIZE-4,0):
        state=fixture();state.put(FL+7996,[selected]);sequence(state,[call(replies=[(R,[])] if selected==ARENA+SIZE-4 else [])])
    for allocation in (0,bad,ARENA+SIZE-4):
        sequence(fixture(),[call(replies=[(allocation,[])])])
    for handle in (bad,ARENA+SIZE-4):
        state=fixture();sequence(state,[call(handle=handle)])
    state=fixture();state.put(POOL,[1,11,22]);sequence(state,[call(bad,POOL)])
    state=fixture();state.put(SCRATCH+24,[bad,0]);sequence(state,[call(replies=[(R,[])]),call(0,POOL+23988)])
    details={'scope':'Complete HandleOp with real ImageOp and all resource/packing descendants; CRT malloc/free modeled',
        'coverage':coverage,'checks':'independent instruction-derived parent/child oracles; exact ordered external reads/writes and dependency entries/arguments/snapshots; full arena/static pool/freelist/scratch/cursor/control/packing and unrelated image bytes; persistent lifecycle, scratch-tail preservation, signed/wrapping IDs, live origins after CRT mutation, aliases, fault prefixes and normal EAX/cdecl stack/nonvolatile/DF',
        'compiler':'Provisional strict C89 Clang/LLD PE32; original compiler/link layout unresolved',
        'limitations':'No native game/heap/fault validation or instruction equality; primitive/renderer integration, arbitrary reentry/concurrency, exhaustive aliases and scalar/address collisions unverified. Declared fixture pointers normalize by verified static-object relocations; untouched random arena bytes preserved. Cross-page dword fault atomicity and fault-time registers/frames excluded.'}
    for kind in ('compilation','emulation'):
        record_run(RVA,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
            cases=coverage['comparisons'] if kind=='emulation' else 0,
            command='uv run python tools/verify_sprite_handleop.py',details=details)
    print('PASS complete HandleOp:',coverage,flush=True)
    return coverage


if __name__ == '__main__':
    verify_sprite_handleop()
