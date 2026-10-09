# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Native ImageOp with coherent control and real resource dependencies."""
import hashlib
import random
import struct
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import UcError
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX,
    UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_EFLAGS)
from verify_matching import STACK, STOP, PRESERVED
from verify_sprite_packing import State as PackingState, GapStop, ARENA, SIZE, MASK, TEMPLATE, HEAD, TABLE
from verify_sprite_storage import State as StorageState, Session as StorageSession, inspect as inspect_storage
from verify_sprite_release import State as ReleaseState, inspect as inspect_release
from verify_gfx_string import State as StringState, inspect as inspect_string
from verify_gfx_table import State as TableState, inspect as inspect_table
from build_decomp import build
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')
NAME,RVA,LENGTH = 'Gfx_SpriteImageOpNative',0x61360,329
DIGEST = '81b591def47aa754c8a41dd4255ba978ad5ff82514ca2533537dd607077a2020'
CTL = 0x520380
S,RECORDS,R,NAME_SOURCE,NAME_COPY,PIXEL_SOURCE = [ARENA+x for x in (0x100,0x1000,0x6000,0xa0000,0xb0000,0xc0000)]
DLL = BUILD / 'sprite_imageop_validation.dll'
INPUTS = ['decomp/src/geputget.c','decomp/include/geputget.h',
    'decomp/src/mem.c','decomp/include/mem.h','decomp/src/lisa3d.c','decomp/include/lisa3d.h',
    'decomp/target.json','tools/build_decomp.py','tools/verify_matching.py',
    'tools/verify_sprite_imageop.py','tools/verify_sprite_storage.py','tools/verify_sprite_pixels.py',
    'tools/verify_sprite_packing.py','tools/verify_sprite_release.py','tools/verify_sprite_free.py',
    'tools/verify_gfx_string.py','tools/verify_gfx_table.py','tools/windows_target.py','tools/windows_tracking.py']
PARENT_ARGUMENTS = {'Gfx_GrowPointerTable':1,'Gfx_CopyAllocatedString':2,
    'Gfx_ReleaseSpritePackingStorage':1,'Gfx_AssignSpritePackingStorage':1}
PARENT_RVAS = {'Gfx_GrowPointerTable':0x5f560,'Gfx_CopyAllocatedString':0x60480,
    'Gfx_ReleaseSpritePackingStorage':0x618b0,'Gfx_AssignSpritePackingStorage':0x61530}


def signed(word):
    return word if word < 0x80000000 else word-0x100000000


class State(StorageState):
    """Parent oracle from instructions; independently recovered child oracles.

    Child resets affect only their local event bookkeeping. Parent read limits,
    malloc replies and actual CRT free schedules remain invocation-wide.
    """
    def __init__(self,seed):
        super().__init__(seed)
        self.control = bytearray(16)
        self.map_zero = False
        self.root_reads,self.root_limit = 0,None
        self.free_schedules,self.replies = [],[]

    def region(self,address,size):
        if CTL <= address and address+size <= CTL+16:
            return self.control,address-CTL
        return super().region(address,size)

    def snapshot(self):
        return super().snapshot()+(bytes(self.control),)

    def read(self,address):
        value = PackingState.read(self,address&MASK)
        self.root_reads += 1
        if self.root_limit is not None and self.root_reads == self.root_limit:
            self.fault = ('budget',self.root_limit)
            raise GapStop
        return value

    def byte(self,address,value=None):
        address &= MASK
        data,offset = self.region(address,1)
        kind = 'read' if value is None else 'write'
        if data is None:
            self.fault = (kind,address,1)
            raise GapStop
        if value is None:
            value = data[offset]
            self.root_reads += 1
        else:
            data[offset] = value
        self.events.append((kind,address,1,value))
        if kind == 'read' and self.root_limit is not None and self.root_reads == self.root_limit:
            self.fault = ('budget',self.root_limit)
            raise GapStop
        return value

    def free(self,address):
        if address:
            self.events.append(('free',address,self.snapshot()))
            mutations = self.free_schedules.pop(0) if self.free_schedules else []
            for destination,word in mutations:
                self.put(destination,[word])

    def child(self,name,args):
        self.boundary(name,args)
        prefix = self.events
        if name == 'Gfx_GrowPointerTable':
            mutations = self.free_schedules[0] if self.free_schedules else []
            self.replies = TableState.grow(self,CTL,self.replies,mutations)
            if any(e[0] == 'free' for e in self.events) and self.free_schedules:
                self.free_schedules.pop(0)
        elif name == 'Gfx_CopyAllocatedString':
            self.replies = StringState.copy_name(self,*args,self.replies)
        elif name == 'Gfx_ReleaseSpritePackingStorage':
            ReleaseState.release(self,*args)
        elif name == 'Gfx_AssignSpritePackingStorage':
            self.replies = StorageState.storage(self,*args,self.replies)
        else:
            raise AssertionError(name)
        self.events = prefix+self.events
        if self.fault:
            raise GapStop

    def imageop(self,source,image_id,replies,free_schedules=(),limit=None):
        self.events,self.fault,self.root_reads,self.root_limit = [],None,0,limit
        self.replies,self.free_schedules = list(replies),list(free_schedules)
        selected = image_id&MASK
        try:
            while True:
                if source == 0:
                    if signed(selected) <= 0 or signed(self.read(CTL+8)) <= signed(selected):
                        return 0
                    table = self.read(CTL+12)
                    record = self.read(table+selected*4)
                    if record:
                        self.free(self.read(record))
                        table = self.read(CTL+12)
                        record = self.read(table+selected*4)
                        self.child('Gfx_ReleaseSpritePackingStorage',(record,))
                    if signed(self.read(CTL)) > signed(selected):
                        self.write(CTL,selected)
                    table = self.read(CTL+12)
                    self.free(self.read(table+selected*4))
                    table = self.read(CTL+12)
                    self.write(table+selected*4,0)
                    return 0
                if selected:
                    if signed(selected) < 0 or signed(self.read(CTL+8)) <= signed(selected):
                        return 0
                    break
                cursor = self.read(CTL)
                if signed(self.read(CTL+8)) <= signed(cursor):
                    self.child('Gfx_GrowPointerTable',(CTL,))
                    continue
                selected = self.read(CTL)
                candidate = (selected+1)&MASK
                if signed(candidate) < signed(self.read(CTL+8)):
                    scan = (self.read(CTL+12)+candidate*4)&MASK
                    while self.read(scan):
                        scan = (scan+4)&MASK
                        candidate = (candidate+1)&MASK
                        if signed(candidate) >= signed(self.read(CTL+8)):
                            break
                self.write(CTL,candidate)
                break
            table = self.read(CTL+12)
            slot = (table+selected*4)&MASK
            record = self.read(slot)
            if record == 0:
                self.boundary('Gfx_AllocBytes',(64,))
                allocation = self.allocate(64,self.replies)
                self.write(slot,allocation)
            else:
                self.free(self.read(record))
                table = self.read(CTL+12)
                record = self.read(table+selected*4)
                self.child('Gfx_ReleaseSpritePackingStorage',(record,))
            table = self.read(CTL+12)
            record = self.read(table+selected*4)
            for index in range(16):
                self.write(record+4*index,self.read(source+4*index))
            name = self.read(record)
            self.child('Gfx_CopyAllocatedString',(name,record))
            self.child('Gfx_AssignSpritePackingStorage',(record,))
            return selected
        except GapStop:
            return None


class Session(StorageSession):
    def __init__(self,pe,state,original):
        super().__init__(pe,state,original)
        if original:
            self.symbols.update({name:0x400000+r for name,r in PARENT_RVAS.items()})
            self.symbols.update({NAME:0x400000+RVA,'free':0x4693b0,'g_nativeImageControl':CTL})
        self.control_address = self.symbols['g_nativeImageControl']
        self.fields.append((CTL,self.control_address,16))
        self.uc.mem_write(self.control_address,bytes(state.control))

    def snapshot(self):
        return super().snapshot()+(bytes(self.uc.mem_read(self.control_address,16)),)

    def code(self,uc,address,size,data):
        for name,argc in PARENT_ARGUMENTS.items():
            if address == self.symbols[name]:
                sp = uc.reg_read(UC_X86_REG_ESP)
                args = struct.unpack('<'+'I'*argc,uc.mem_read(sp+4,4*argc))
                self.events.append(('entry',name,tuple(self.normalize(a) for a in args),self.snapshot()))
        if address == self.symbols['free']:
            sp = uc.reg_read(UC_X86_REG_ESP)
            ret,arg = struct.unpack('<II',uc.mem_read(sp,8))
            self.events.append(('free',arg,self.snapshot()))
            mutations = self.free_schedules.pop(0) if self.free_schedules else []
            for dest,word in mutations:
                uc.mem_write(self.physical(dest),struct.pack('<I',word&MASK))
            for reg,value in ((UC_X86_REG_EAX,0xdeadbeef),(UC_X86_REG_ECX,0xbadc0ffe),
                    (UC_X86_REG_EDX,0x87654321),(UC_X86_REG_EFLAGS,0x8d7),
                    (UC_X86_REG_ESP,sp+4),(UC_X86_REG_EIP,ret)):
                uc.reg_write(reg,value)
            return
        super().code(uc,address,size,data)

    def run_imageop(self,source,image_id,replies,expected,result,seed,free_schedules=(),limit=None):
        self.events,self.fault,self.replies,self.free_schedules = [],None,list(replies),list(free_schedules)
        self.schedule,self.seen,self.limit,self.total = {},{},limit,0
        self.pending_mutations,self.seed = [],seed
        uc,sp = self.uc,STACK+0x8000
        uc.mem_write(sp,struct.pack('<III',STOP,source,image_id&MASK))
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
            assert uc.reg_read(UC_X86_REG_EAX) == result
            assert uc.reg_read(UC_X86_REG_ESP) == sp+4
            assert [uc.reg_read(r) for r in PRESERVED] == saved
            assert uc.reg_read(UC_X86_REG_EFLAGS)&0x400 == 0


def fixture(seed=0,next_id=1,capacity=4,table=RECORDS,name=0,width=3,height=0):
    state = State(seed)
    state.template[:] = bytes(24)
    state.table[:] = bytes(1028)
    state.put(CTL,[next_id,0x13579bdf,capacity,table])
    state.put(RECORDS,[0]*4000)
    state.put(S,[name,width,height,0xdeadbeef,PIXEL_SOURCE,8]+list(range(10)))
    state.put(R,[0,3,0,0,0,0]+[0]*10)
    state.arena[NAME_SOURCE-ARENA:NAME_SOURCE-ARENA+4] = b'abc\0'
    state.arena[PIXEL_SOURCE-ARENA:PIXEL_SOURCE-ARENA+16] = b'ABCDEFGHIJKLMNOP'
    return state


def inspect(pe):
    inspect_storage(pe)
    inspect_release(pe)
    inspect_string(pe)
    inspect_table(pe)
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    md = Cs(CS_ARCH_X86,CS_MODE_32)
    raw = pe.get_data(RVA,LENGTH)
    assert fpo[RVA] == (329,0,2,0x140c) and hashlib.sha256(raw).hexdigest() == DIGEST
    ins = list(md.disasm(raw,RVA+0x400000))
    assert sum(i.size for i in ins) == LENGTH
    assert [i.address for i in ins if i.mnemonic == 'ret'] == [0x4613fa,0x4614a1,0x4614a8]
    assert [(i.address,i.op_str) for i in ins if i.mnemonic == 'call'] == [
        (0x46138a,'0x45f560'),(0x4613af,'0x45f8b0'),(0x4613c0,'0x4618b0'),
        (0x4613df,'0x45f8b0'),(0x46144b,'0x45f4a0'),(0x46145a,'0x45f8b0'),
        (0x46146c,'0x4618b0'),(0x46148a,'0x460480'),(0x461493,'0x461530')]
    reloc = {e.rva:struct.unpack('<I',pe.get_data(e.rva,4))[0]
        for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries if e.type == 3}
    assert {r:v for r,v in reloc.items() if RVA <= r < RVA+LENGTH} == {
        0x61379:CTL,0x6137f:CTL+8,0x61386:CTL,0x6139a:CTL+8,
        0x613a1:CTL+12,0x613b8:CTL+12,0x613ca:CTL,0x613d2:CTL,
        0x613d7:CTL+12,0x613e9:CTL+12,0x613fd:CTL,0x61406:CTL+8,
        0x61415:CTL+12,0x61424:CTL+8,0x6142b:CTL,0x61435:CTL+8,
        0x6143c:CTL+12,0x61464:CTL+12,0x61475:CTL+12}
    assert [r for r,v in reloc.items() if v == 0x461360] == [0x56ec0]
    assert pe.get_data(RVA+LENGTH,7) == b'\xcc'*7
    assert [(r,i.address) for r,(n,*_) in fpo.items()
        for i in md.disasm(pe.get_data(r,n),r+0x400000)
        if i.mnemonic in ('call','jmp') and i.op_str == '0x461360'] == [
            (0x5c830,0x45c8ad),(0x5c830,0x45c8de),(0x5c900,0x45c90a),
            (0x63250,0x4632db),(0x633a0,0x46349d),(0x63cd0,0x463d9d),(0x63cd0,0x46486a)]
    for r,n,digest in ((0x611d0,53,'bb24fdcef5b8b3fdd31746c899ca6523d467479ebc00098f5be527e9e414d70a'),
            (0x612e0,114,'75f04bc300b71fce757262cbb88c22e365cd785b8fc1a87578179dba1c442858'),
            (0x5e610,326,'fd2abfef167ef5b92074ef41c17478dd955591025afb3aad714da050a29704a3')):
        assert fpo[r][0] == n and hashlib.sha256(pe.get_data(r,n)).hexdigest() == digest
    print('Authenticated complete ImageOp/control, nine calls, 19 global relocations, dispatch operand and seven direct callers.',flush=True)


def verify_sprite_imageop():
    verify_target()
    original = pefile.PE(str(TARGET))
    inspect(original)
    build(dll=DLL)
    rebuilt = pefile.PE(str(DLL))
    coverage = {'comparisons':0,'persistent_followups':0,'faults':0,
        'bounded_prefixes':0,'aliases':0,'mutation_calls':0,'packed_calls':0}
    def sequence(state,calls,alias=False,packed=False):
        sessions = [Session(p,state,o) for p,o in ((original,True),(rebuilt,False))]
        for repeat,(source,image_id,replies,free_schedules,limit,expected_result) in enumerate(calls):
            result = state.imageop(source,image_id,replies,free_schedules,limit)
            if expected_result is not None:
                assert result == expected_result&MASK, (image_id,result,expected_result,state.fault)
            for session in sessions:
                session.run_imageop(source,image_id,replies,state,result,coverage['comparisons'],free_schedules,limit)
            coverage['comparisons'] += 1
            coverage['persistent_followups'] += int(repeat != 0)
            coverage['faults'] += int(state.fault is not None and state.fault[0] != 'budget')
            coverage['bounded_prefixes'] += int(state.fault is not None and state.fault[0] == 'budget')
            coverage['aliases'] += int(alias)
            coverage['mutation_calls'] += int(any(m for _,m in replies) or any(free_schedules))
            coverage['packed_calls'] += int(packed)
            if state.fault:
                break
    def call(source=S,image_id=2,replies=(),free_schedules=(),limit=None,result=None):
        return (source,image_id,list(replies),list(free_schedules),limit,result)
    # Signed incoming-ID gates, with source validity irrelevant on rejection.
    for source in (0,S,0x80000000):
        for capacity in (0,1,4,0x7fffffff,0x80000000,MASK):
            for image_id in (0x80000000,MASK,0x7fffffff):
                sequence(fixture(capacity=capacity),[call(source,image_id,result=0)]*2)
    for image_id in (4,5,257):
        sequence(fixture(),[call(S,image_id,result=0),call(0,image_id,result=0)])
    for seed in range(16):
        # Existing and absent deletion, replacement, and explicit cursor retention.
        state = fixture(seed,next_id=3)
        sequence(state,[call(replies=[(R,[])],result=2),call(0,2,result=0),
            call(0,2,result=0),call(replies=[(R+0x100,[])],result=2),
            call(replies=[],result=2),call(0,2,result=0)])
        # Names are newly owned on every replacement and freed separately.
        state = fixture(seed,name=NAME_SOURCE,next_id=3)
        sequence(state,[call(replies=[(R,[]),(NAME_COPY,[])],result=2),
            call(replies=[(NAME_COPY+0x100,[])],result=2),call(0,2,result=0)])
    # Automatic slot scan, occupied prefixes, tail and exhaustion; chosen slot
    # is captured independently of the published next cursor.
    for capacity in (2,3,4,17,64):
        for occupied in (0,1,2,capacity):
            state = fixture(capacity=capacity)
            state.put(RECORDS+8,[R+0x100+64*k for k in range(min(occupied,max(0,capacity-2)))])
            sequence(state,[call(image_id=0,replies=[(R,[])],result=1),call(0,1,result=0)])
    for cursor in (0,1,999,1000,1001,1999,2001):
        state = fixture(next_id=cursor,capacity=0,table=0)
        grows = cursor//1000+1
        replies = [(RECORDS+0x20000*k,[]) for k in range(grows)]+[(R,[])]
        sequence(state,[call(image_id=0,replies=replies,result=cursor),call(0,cursor,result=0)])
    # Noncanonical negative automatic cursors bypass explicit-ID rejection.
    for cursor in (0x80000000,0x80000001,MASK):
        state = fixture(next_id=cursor)
        # Wrapped address of selected slot and scan reads is still mapped.
        sequence(state,[call(image_id=0,replies=[(R,[])])])
    # Real page/bucket/leaf allocation and pixel copying, then real cleanup.
    page,backing,container,bucket,leaf = [ARENA+x for x in (0x20000,0x30000,0x21000,0x22000,0x23000)]
    for width,height in ((1,1),(3,1),(7,3),(16,16),(1,256),(256,1)):
        state = fixture(width=width,height=height)
        replies = [(R,[]),(page,[]),(backing,[]),(container,[]),(bucket,[]),(leaf,[])]
        sequence(state,[call(image_id=0,replies=replies,result=1),call(0,1,result=0)],packed=True)
    # Replacing the singleton record releases the old packing hierarchy first.
    state = fixture(height=1,name=NAME_SOURCE)
    sequence(state,[call(image_id=0,replies=[(R,[]),(NAME_COPY,[]),(page,[]),(backing,[]),
        (container,[]),(bucket,[]),(leaf,[])],result=1),
        call(image_id=1,replies=[(NAME_COPY+0x100,[]),(page+0x100,[]),(backing+0x20000,[]),
            (container+0x100,[]),(bucket+0x100,[]),(leaf+0x100,[])],result=1),call(0,1,result=0)],packed=True)
    # Publication aliases, descriptor/record forward-copy aliases and live name
    # reads; no pre-copy descriptor snapshot or invented ownership correction.
    for allocation in (S,S+4,S-4,R,R+1):
        state = fixture()
        sequence(state,[call(replies=[(allocation,[])])],alias=True)
    for destination in (R,R+4,R-4):
        state = fixture()
        state.put(destination,[0,3,0,0,PIXEL_SOURCE,8]+[0]*10)
        state.put(RECORDS+8,[destination])
        sequence(state,[call(source=destination,image_id=2,result=2)],alias=True)
    # Malloc can retarget the global table while its old slot remains captured.
    for new_table in (RECORDS+0x10000,RECORDS):
        state = fixture()
        if new_table != RECORDS:
            state.put(new_table+8,[R+0x100])
        sequence(state,[call(replies=[(R,[(CTL+12,new_table)])])])
    state = fixture()
    sequence(state,[call(replies=[(R,[(CTL,999),(CTL+8,0),(S+4,99)])],result=2)])
    # Final record-free mutation retargets only the last slot clear.
    state = fixture(next_id=3)
    state.put(RECORDS+8,[R])
    state.put(RECORDS+0x10000+8,[R+0x100])
    sequence(state,[call(source=0,free_schedules=[[(CTL+12,RECORDS+0x10000)]],result=0)])
    # Freeing a name may replace the slot before release/copy/record freeing.
    for delete in (False,True):
        state = fixture(name=NAME_SOURCE,next_id=3)
        state.put(R,[NAME_COPY,3,0,0,PIXEL_SOURCE,8]+[0]*10)
        state.put(R+0x100,[0,3,0,0,PIXEL_SOURCE,8]+[0]*10)
        state.put(RECORDS+8,[R])
        sequence(state,[call(source=0 if delete else S,
            replies=[] if delete else [(NAME_COPY+0x100,[])],
            free_schedules=[[(RECORDS+8,R+0x100)]],result=0 if delete else 2)])
    # Concrete fully unmapped/partial aligned access faults and prior effects.
    bad = 0x80000000
    for source,image_id,allocation in ((bad,2,R),(S,2,0),(S,2,bad),
            (ARENA+SIZE-4,2,R),(S,2,ARENA+SIZE-4)):
        sequence(fixture(),[call(source,image_id,[(allocation,[])])])
    for table in (0,bad,ARENA+SIZE-4):
        sequence(fixture(table=table),[call(0,2)])
        sequence(fixture(table=table),[call(image_id=0)])
    for record in (bad,ARENA+SIZE-4,0):
        state = fixture();state.put(RECORDS+8,[record])
        sequence(state,[call(0,2)])
    state = fixture(name=bad)
    sequence(state,[call(replies=[(R,[])])])
    state = fixture(height=1);state.put(S+16,[bad])
    sequence(state,[call(image_id=0,replies=[(R,[]),(page,[]),(backing,[]),(container,[]),(bucket,[]),(leaf,[])])],packed=True)
    # Observation budgets do not add a production retry/scan limit.
    state = fixture(next_id=0x7fffffff,capacity=0,table=0)
    sequence(state,[call(image_id=0,replies=[(RECORDS,[])],limit=31)])
    details = {'scope':'Complete native ImageOp; coherent production control object; actual growth/string/storage/release/allocation/pixel/packing bodies execute; CRT malloc/free modeled only',
        'coverage':coverage,'checks':'independent instruction-derived parent/child oracles; exact ordered reads/writes, real dependency entries/arguments/snapshots and local pixel snapshot values; full arena/control/packing/low/high/unrelated-image bytes; persistent lifecycle/owned names/packed cleanup, callback mutations, aliases/wrapping/signed IDs, faults/retained effects and normal EAX/cdecl caller-stack/nonvolatile/DF',
        'compiler':'Provisional strict C89 Clang/LLD PE32; original compiler/link layout unresolved',
        'limitations':'No instruction equality or native heap/graphics/game/fault parity, whole handle/primitive/renderer integration, arbitrary reentry/concurrency, exhaustive/global/caller-stack aliases, native fault frames/atomicity or portable unchecked-fault guarantees; cross-page dword stores excluded; external budgets are observation limits'}
    for kind in ('compilation','emulation'):
        record_run(RVA,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
            cases=coverage['comparisons'] if kind == 'emulation' else 0,
            command='uv run python tools/verify_sprite_imageop.py',details=details)
    print('PASS complete native ImageOp:',coverage,flush=True)
    return coverage


if __name__ == '__main__':
    verify_sprite_imageop()
