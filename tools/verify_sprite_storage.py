# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Storage assignment: instruction-derived oracle and real packing/pixel bodies."""
import hashlib
import random
import struct
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import UcError, UC_MEM_WRITE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP,
    UC_X86_REG_EFLAGS, UC_X86_REG_EAX)
from verify_sprite_packing import (State as PackingState, Session as PackingSession,
    GapStop, MASK, ARENA, SIZE, TEMPLATE, HEAD, TABLE, ROUTINES,
    inspect as inspect_dependencies)
from verify_sprite_pixels import inspect as inspect_pixels
from verify_matching import STACK, STOP, PRESERVED
from build_decomp import build
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')
NAME, RVA, LENGTH = 'Gfx_AssignSpritePackingStorage', 0x61530, 350
DIGEST = '028b7432fafcd6c13a8ea0d675bda862a30ca19e99fbd6a3ec3e9a4344f9c394'
DLL = BUILD / 'sprite_storage_validation.dll'
INPUTS = ['decomp/src/geputget.c','decomp/include/geputget.h',
    'decomp/src/mem.c','decomp/include/mem.h','decomp/src/lisa3d.c',
    'decomp/include/lisa3d.h','decomp/target.json','tools/build_decomp.py',
    'tools/verify_sprite_storage.py','tools/verify_sprite_packing.py',
    'tools/verify_sprite_pixels.py','tools/verify_matching.py',
    'tools/windows_target.py','tools/windows_tracking.py']
D, PAGE, CONTAINER, CHILD, NEXT, NODE, BUCKET, SOURCE, PIXELS = [
    ARENA+x for x in (0x100,0x1000,0x1100,0x1200,0x1280,0x1400,0x1800,0x90000,0x50000)]
ARGUMENTS = {'Gfx_AddSpritePackingBucket':2,'Gfx_AddSpritePackingPage':0,
    'Gfx_FindSpritePackingGap':2,'Gfx_LinkSpritePackingNode':3,
    'Gfx_AllocBytes':1,'Gfx_AllocAlignedBytes':2}

def signed(word):
    return word if word < 0x80000000 else word-0x100000000


def inspect(pe):
    inspect_dependencies(pe)
    inspect_pixels(pe)
    raw = pe.get_data(RVA,LENGTH)
    assert hashlib.sha256(raw).hexdigest() == DIGEST
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    assert fpo[RVA] == (350,18,1,0x140b)
    md = Cs(CS_ARCH_X86,CS_MODE_32)
    instructions = list(md.disasm(raw,0x400000+RVA))
    assert sum(i.size for i in instructions) == LENGTH
    assert [i.address for i in instructions if i.mnemonic == 'ret'] == [0x46160f,0x46168d]
    assert pe.get_data(RVA+LENGTH,2) == b'\xcc'*2
    assert [(i.address,i.op_str) for i in instructions if i.mnemonic == 'call'] == [
        (0x46158d,'0x461870'),(0x4615af,'0x45f4a0'),(0x4615f6,'0x4616c0'),
        (0x461615,'0x45f4a0'),(0x461652,'0x461690'),(0x46167e,'0x4612a0')]
    reloc = {e.rva for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries if e.type == 3}
    operands = {r:struct.unpack('<I',pe.get_data(r,4))[0]
        for r in reloc if RVA <= r < RVA+LENGTH}
    assert operands == {0x6156d:TABLE,0x61576:HEAD,0x615ab:TEMPLATE,0x615ed:HEAD,0x61611:TEMPLATE}
    assert not any(struct.unpack('<I',pe.get_data(r,4))[0] == 0x461530 for r in reloc)
    assert [(r,i.address) for r,(n,*_) in fpo.items()
        for i in md.disasm(pe.get_data(r,n),0x400000+r)
        if i.mnemonic in ('call','jmp') and i.op_str == '0x461530'] == [(0x61360,0x461493)]
    assert fpo[0x61360] == (329,0,2,0x140c)
    assert hashlib.sha256(pe.get_data(0x61360,329)).hexdigest() == '81b591def47aa754c8a41dd4255ba978ad5ff82514ca2533537dd607077a2020'
    assert [(i.mnemonic,i.op_str) for i in md.disasm(pe.get_data(0x61492,11),0x461492)] == [
        ('push','ebp'),('call','0x461530'),('add','esp, 4'),('mov','eax, ebx')]
    # These globals are in the loader-zeroed .data virtual tail, not raw bytes.
    for address,n in ((TEMPLATE,24),(HEAD,4),(TABLE,1028)):
        rva = address-0x400000
        section = next(s for s in pe.sections
            if s.VirtualAddress <= rva and rva+n <= s.VirtualAddress+s.Misc_VirtualSize)
        assert section.Name.rstrip(b'\0') == b'.data'
        assert rva >= section.VirtualAddress+section.SizeOfRawData
    print('Authenticated storage extent, calls, relocations, loader-zeroed globals and sole cdecl caller.',flush=True)


class State(PackingState):
    """Raw offsets; dependency oracles were independently derived from instructions.

    The local snapshot has compiler-dependent placement. Its sixteen values are
    checked at real pixel entry; private stack accesses and incidental EAX are
    excluded. All external pixel and descriptor accesses remain ordered.
    """
    def __init__(self, seed):
        super().__init__(seed)
        self.low,self.high = bytearray(4096),bytearray(4096)
        self.map_zero = True
        self.schedule,self.seen,self.limit,self.total = {},{},None,0

    def region(self,address,size):
        for base,data in ((0,self.low),(0xfffff000,self.high)):
            if base == 0 and not self.map_zero:
                continue
            if base <= address and address+size <= base+len(data):
                return data,address-base
        return super().region(address,size)

    def snapshot(self):
        return super().snapshot()+(bytes(self.low),bytes(self.high))

    def put(self,address,values):
        data,offset = self.region(address,4*len(values))
        assert data is not None
        struct.pack_into('<'+'I'*len(values),data,offset,*[v&MASK for v in values])

    def mutate(self,address):
        self.seen[address] = self.seen.get(address,0)+1
        for dest,word in self.schedule.get((address,self.seen[address]),()):
            self.put(dest,[word])

    def read(self,address):
        value = super().read(address)
        self.total += 1
        self.mutate(address&MASK)
        if self.limit is not None and self.total == self.limit:
            self.fault = ('budget',self.limit)
            raise GapStop
        return value

    def write(self,address,value):
        if not super().write(address,value&MASK):
            raise GapStop
        return True

    def byte(self,address,value=None):
        address &= MASK
        data,offset = self.region(address,1)
        kind = 'read' if value is None else 'write'
        if data is None:
            self.fault = (kind,address,1)
            raise GapStop
        if value is None:
            value = data[offset]
        else:
            data[offset] = value
        self.events.append((kind,address,1,value))
        if kind == 'read':
            self.total += 1
            if self.limit is not None and self.total == self.limit:
                self.fault = ('budget',self.limit)
                raise GapStop
        return value

    def storage(self,descriptor,replies,schedule=None,limit=None):
        self.events,self.fault = [],None
        self.schedule,self.seen,self.limit,self.total = schedule or {},{},limit,0
        pending = list(replies)
        try:
            if signed(self.read(descriptor+8)) <= 0:
                return pending
            while True:
                height = self.read(descriptor+8)
                if signed(height) > 256:
                    return pending
                width = self.read(descriptor+4)
                if not 0 < signed(width) <= 256:
                    return pending
                bucket = self.read(TABLE+((height*4)&MASK))
                if bucket == 0:
                    head = self.read(HEAD)
                else:
                    while bucket:
                        container = self.read(bucket+4)
                        width = self.read(descriptor+4)
                        children = self.read(container+12)
                        self.boundary('Gfx_FindSpritePackingGap',(width,children))
                        previous = self.gap(width,children)
                        if self.fault:
                            raise GapStop
                        if previous:
                            break
                        bucket = self.read(bucket)
                    if bucket:
                        break
                    head = self.read(HEAD)
                    height = self.read(descriptor+8)
                self.boundary('Gfx_AddSpritePackingBucket',(height,head))
                self.bucket(height,head,pending)
                if self.fault:
                    raise GapStop
                if signed(self.read(descriptor+8)) <= 0:
                    return pending
            self.boundary('Gfx_AllocBytes',(24,))
            node = self.allocate(24,pending)
            for offset in range(0,24,4):
                self.write(node+offset,self.read(TEMPLATE+offset))
            if previous == MASK:
                self.write(node+4,self.read(descriptor+4))
                pixels = self.read(container+8)
                start = self.read(node)
                self.write(node+8,pixels+start)
                following = self.read(container+12)
                self.write(node+20,following)
                if following:
                    self.write(following+16,node)
                self.write(container+12,node)
            else:
                self.write(node,self.read(previous+4))
                width = self.read(descriptor+4)
                end = self.read(previous+4)
                self.write(node+4,width+end)
                pixels = self.read(container+8)
                start = self.read(node)
                self.write(node+8,pixels+start)
                following = self.read(previous+20)
                self.boundary('Gfx_LinkSpritePackingNode',(previous,node,following))
                self.write(node+20,following)
                self.write(node+16,previous)
                self.write(previous+20,node)
                if following:
                    self.write(following+16,node)
            snapshot = tuple(self.read(descriptor+4*k) for k in range(16))
            pixels = self.read(node+8)
            self.write(descriptor+16,pixels)
            self.write(descriptor+20,256)
            self.events.append(('pixels',snapshot,descriptor,self.snapshot()))
            source = snapshot[4]
            destination = self.read(descriptor+16)
            for row in range(max(0,signed(snapshot[2]))):
                assert row < 1024, 'Unbounded fixture'
                for column in range(max(0,signed(snapshot[1]))):
                    assert column < 1024, 'Unbounded fixture'
                    value = self.byte(source+column)
                    self.byte(destination+column,value)
                source = (source+snapshot[5])&MASK
                destination = (destination+self.read(descriptor+20))&MASK
        except GapStop:
            pass
        return pending


class Session(PackingSession):
    def __init__(self,pe,state,original):
        super().__init__(pe,state,original)
        if original:
            self.symbols[NAME] = 0x461530
            self.symbols['Gfx_CopySpriteDescriptorPixels'] = 0x4612a0
        self.map_zero = state.map_zero
        if self.map_zero:
            self.uc.mem_map(0,4096)
            self.uc.mem_write(0,bytes(state.low))
        self.uc.mem_map(0xfffff000,4096)
        self.uc.mem_write(0xfffff000,bytes(state.high))
        self.bucket_mode,self.page_mode = False,None

    def snapshot(self):
        return super().snapshot()+(bytes(self.uc.mem_read(0,4096)) if self.map_zero else bytes(4096),bytes(self.uc.mem_read(0xfffff000,4096)))

    def memory(self,uc,access,address,size,value,_):
        if STACK <= address < STACK+0x10000:
            return
        kind = 'write' if access == UC_MEM_WRITE else 'read'
        valid = (ARENA <= address and address+size <= ARENA+SIZE or
            self.map_zero and 0 <= address and address+size <= 4096 or
            0xfffff000 <= address and address+size <= 0x100000000 or
            any(p <= address and address+size <= p+n for _,p,n in self.fields))
        if not valid:
            # Unexpected mapped image accesses are retained and fail the oracle.
            if kind == 'write' or not self.base <= address < self.base+self.size:
                return
        if kind == 'read':
            value = int.from_bytes(uc.mem_read(address,size),'little')
        logical = self.normalize(address)
        self.events.append((kind,logical,size,value))
        if kind == 'read':
            self.total += 1
            self.seen[logical] = self.seen.get(logical,0)+1
            for dest,word in self.schedule.get((logical,self.seen[logical]),()):
                self.pending_mutations.append((dest,word))
            if self.limit is not None and self.total == self.limit:
                self.fault = ('budget',self.limit)
                uc.emu_stop()

    def code(self,uc,address,size,_):
        for dest,word in self.pending_mutations:
            uc.mem_write(self.physical(dest),struct.pack('<I',word&MASK))
        self.pending_mutations = []
        for name,argc in ARGUMENTS.items():
            if address == self.symbols[name]:
                sp = uc.reg_read(UC_X86_REG_ESP)
                args = struct.unpack('<'+'I'*argc,uc.mem_read(sp+4,4*argc)) if argc else ()
                self.events.append(('entry',name,args,self.snapshot()))
        if address == self.symbols['Gfx_CopySpriteDescriptorPixels']:
            sp = uc.reg_read(UC_X86_REG_ESP)
            source,destination = struct.unpack('<II',uc.mem_read(sp+4,8))
            assert STACK <= source and source+64 <= STACK+0x8000
            snapshot = struct.unpack('<16I',uc.mem_read(source,64))
            self.events.append(('pixels',snapshot,destination,self.snapshot()))
        if address == self.symbols['malloc']:
            assert self.replies, ('Unexpected malloc',self.seed,[(e[0],e[1] if e[0] == 'entry' else e[1:3]) for e in self.events[-4:]])
        super().code(uc,address,size,_)

    def run_storage(self,descriptor,replies,expected,remaining,seed,schedule=None,limit=None):
        self.schedule,self.seen,self.limit,self.total = schedule or {},{},limit,0
        self.events,self.fault,self.replies = [],None,list(replies)
        self.pending_mutations = []
        self.seed = seed
        uc = self.uc
        sp = STACK+0x8000
        uc.mem_write(sp,struct.pack('<II',STOP,descriptor))
        caller = bytes(uc.mem_read(sp,0x8000))
        saved = [random.Random(seed+i).getrandbits(32) for i in range(4)]
        for reg,value in zip(PRESERVED,saved):
            uc.reg_write(reg,value)
        uc.reg_write(UC_X86_REG_ESP,sp)
        uc.reg_write(UC_X86_REG_EAX,0x31415926)
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
        assert self.replies == remaining, seed
        after = bytearray(uc.mem_read(self.base,self.size))
        for _,p,n in self.fields:
            before[p-self.base:p-self.base+n] = after[p-self.base:p-self.base+n]
        assert before == after, 'Unrelated image bytes changed'
        assert bytes(uc.mem_read(sp,0x8000)) == caller
        if expected.fault is None:
            assert uc.reg_read(UC_X86_REG_EIP) == STOP
            assert uc.reg_read(UC_X86_REG_ESP) == sp+4
            assert [uc.reg_read(reg) for reg in PRESERVED] == saved
            assert uc.reg_read(UC_X86_REG_EFLAGS)&0x400 == 0
        # EAX is intentionally unspecified for this void semantic contract.


def fixture(width=16,height=4,seed=0):
    state = State(seed)
    state.template[:] = b'\0'*24
    state.table[:] = b'\0'*1028
    state.put(HEAD,[PAGE])
    state.put(D,[0x11111111,width,height,0x33333333,SOURCE,259,
        0x66666666,0x77777777]+list(range(8,16)))
    state.put(PAGE,[0,0,PIXELS,0,0,0])
    state.put(CONTAINER,[0,height,PIXELS,0,0,0])
    state.put(BUCKET,[0,CONTAINER])
    if 0 < signed(height&MASK) <= 256:
        state.put(TABLE+4*height,[BUCKET])
    return state


def verify_sprite_storage():
    verify_target()
    original = pefile.PE(str(TARGET))
    inspect(original)
    build(dll=DLL)
    rebuilt = pefile.PE(str(DLL))
    assert rebuilt.FILE_HEADER.Machine == 0x14c and not hasattr(rebuilt,'DIRECTORY_ENTRY_IMPORT')
    coverage = {'comparisons':0,'persistent_followups':0,'faults':0,'cycle_prefixes':0,
        'aliases':0,'wrapping':0,'read_mutations':0,'pixel_entries':0}
    bodies = {name:0 for name in ARGUMENTS}

    def sequence(state,plans,descriptor=D,category=None,limit=None,schedule=None):
        sessions = [Session(pe,state,o) for pe,o in ((original,True),(rebuilt,False))]
        for repeat,replies in enumerate(plans):
            remaining = state.storage(descriptor,replies,schedule,limit)
            for session in sessions:
                session.run_storage(descriptor,replies,state,remaining,coverage['comparisons'],schedule,limit)
            coverage['comparisons'] += 1
            coverage['persistent_followups'] += int(repeat != 0)
            coverage['faults'] += int(state.fault is not None and state.fault[0] != 'budget')
            coverage['cycle_prefixes'] += int(state.fault is not None and state.fault[0] == 'budget')
            if category:
                coverage[category] += 1
            for event in state.events:
                if event[0] == 'entry':
                    bodies[event[1]] += 1
                coverage['pixel_entries'] += int(event[0] == 'pixels')
            if state.fault:
                break
        return state

    # Signed rejection and the four full 1/256 corners; opaque tail survives.
    bounds = (0,1,2,255,256,257,0x7fffffff,0x80000000,0xffffffff)
    for width in bounds:
        for height in bounds:
            state = fixture(width,height,seed=width^height)
            admitted = 0 < signed(width) <= 256 and 0 < signed(height) <= 256
            sequence(state,[[(NODE,[])] if admitted else []])
    # Every admitted table index and its persistent interior insertion.
    for height in range(1,257):
        sequence(fixture(1,height),[[(NODE,[])],[(NODE+64,[])]])
    # Persistent insertion chains exercise real gap/link bodies and real pixels.
    for width in (1,2,7,16,31,63):
        for height in (1,2,4,16):
            plans = [[(NODE+0x40*k,[])] for k in range(4)]
            sequence(fixture(width,height),plans)
    # Prefix versus interior insertion, with/without a successor; template
    # fields are copied verbatim, including arbitrary previous/children values.
    for path in ('prefix','tail','middle','later-bucket'):
        for width in (1,16,64):
            state = fixture(width,2)
            state.put(TEMPLATE,[3,0xeeeeeeee,0xdddddddd,0xcccccccc,0xbbbbbbbb,0xaaaaaaaa])
            state.put(CONTAINER+12,[CHILD])
            if path == 'prefix':
                state.put(CHILD,[128,160,PIXELS,0,0,0])
            else:
                state.put(CHILD,[0,16,PIXELS,0,0,NEXT if path == 'middle' else 0])
                state.put(NEXT,[128,160,PIXELS+128,0,CHILD,0])
            if path == 'later-bucket':
                state.put(CHILD+4,[256])
                state.put(BUCKET,[BUCKET+8,CONTAINER])
                state.put(BUCKET+8,[0,CONTAINER+64])
                state.put(CONTAINER+64,[0,2,PIXELS,0,0,0])
            sequence(state,[[(NODE,[])]])
    # Empty table, exhausted bucket, page creation and live exit on retry.
    for empty_page in (False,True):
        for exhausted in (False,True):
            for changed_height in (None,0,257,MASK):
                state = fixture()
                state.put(TABLE+16,[BUCKET if exhausted else 0])
                if exhausted:
                    state.put(CONTAINER+12,[CHILD])
                    state.put(CHILD,[0,256,PIXELS,0,0,0])
                if empty_page:
                    state.put(HEAD,[0])
                mutations = [] if changed_height is None else [(D+8,changed_height)]
                replies = ([(PAGE+64,[]),(ARENA+0x30003,[])] if empty_page else [])
                replies += [(CONTAINER+64,mutations),(BUCKET+8,[])]
                if changed_height is None:
                    replies += [(NODE,[])]
                sequence(state,[replies])
    # Heap boundary mutation happens after gap selection and before template
    # reads. Captured node addresses and live descriptor/range words differ.
    for interior in (False,True):
        for field,value in ((D+4,0),(D+4,MASK),(D+4,257),(D+8,0),(D+8,2),
                (D+16,SOURCE+3),(D+20,0),(D+20,MASK),(CONTAINER+8,PIXELS+7),
                (CONTAINER+12,NEXT),(CHILD+4,32),(CHILD+20,NEXT),
                (BUCKET+4,CONTAINER+64),(TEMPLATE,5),(TEMPLATE+12,0x12345678)):
            state = fixture(4,1)
            if interior:
                state.put(CONTAINER+12,[CHILD])
                state.put(CHILD,[0,16,PIXELS,0,0,0])
            state.put(NEXT,[128,160,PIXELS,0,0,0])
            sequence(state,[[(NODE,[(field,value)])]])
    # Exact and partial arena aliases: allocation overwrites descriptors and
    # captured containers/predecessors before the final snapshot is taken.
    for interior in (False,True):
        for allocation in (D,D+4,D+8,D+16,CONTAINER,CONTAINER+4,CHILD,NEXT,BUCKET):
            state = fixture(1,1)
            state.put(TEMPLATE,[0,1,PIXELS,0,0,0])
            if D <= allocation < D+64:
                state.put(CONTAINER+8,[2])
            state.put(NEXT,[128,160,PIXELS,0,0,0])
            if interior:
                state.put(CONTAINER+12,[CHILD])
                state.put(CHILD,[0,16,PIXELS,0,0,0])
            sequence(state,[[(allocation,[])]],category='aliases')
    for target in (D,D+4,D+8,D+16,D+20,CONTAINER,CHILD,NODE,BUCKET,SOURCE+1):
        state = fixture(4,2)
        state.put(CONTAINER+8,[target])
        sequence(state,[[(NODE,[])]],category='aliases')
    # The descriptor itself can also be a captured container or predecessor.
    state = fixture(1,1)
    state.put(CONTAINER,[0,1,1,0,SOURCE,0]+list(range(6,16)))
    sequence(state,[[(NODE,[])]],CONTAINER,category='aliases')
    state = fixture(16,1)
    state.put(CONTAINER+12,[CHILD])
    state.put(CHILD,[0,16,1,0,SOURCE,0]+list(range(6,16)))
    sequence(state,[[(NODE,[])]],CHILD,category='aliases')
    # Wrapping interval addition has no pixel shift in this horizontal path.
    state = fixture(32,1)
    state.put(CONTAINER+12,[CHILD]);state.put(CONTAINER+8,[PIXELS+16])
    state.put(CHILD,[0x80000000,0xfffffff0,PIXELS,0,0,0])
    sequence(state,[[(NODE,[])]],category='wrapping')
    assert struct.unpack_from('<I',state.arena,NODE-ARENA+4)[0] == 16
    # Copying the snapshot must retain source dimensions and pointer despite
    # destination writes into the live descriptor. Forward overlap propagates.
    state = fixture(5,1)
    data,offset = state.region(SOURCE,6);data[offset:offset+6] = b'abcdef'
    state.put(CONTAINER+8,[SOURCE+1])
    sequence(state,[[(NODE,[])]],category='aliases')
    assert state.arena[SOURCE-ARENA:SOURCE-ARENA+6] == b'aaaaaa'
    for pixels,start in ((0xfffffffe,0),(MASK,3),(PIXELS,(-PIXELS+2)&MASK)):
        state = fixture(3,2)
        state.put(CONTAINER+8,[pixels]);state.put(TEMPLATE,[start,0,0,0,0,0])
        sequence(state,[[(NODE,[])]],category='wrapping')
    # Deterministic external read-boundary mutation separates the first and
    # second height reads, cached empty-slot height and live exhaustion height.
    for second in (0,1,2,257):
        state = fixture(1,1)
        state.put(TABLE,[BUCKET]);state.put(TABLE+8,[BUCKET])
        schedule = {(D+8,1):[(D+8,second)]}
        replies = [] if second == 257 else [(NODE,[])]
        sequence(state,[replies],schedule=schedule,category='read_mutations')
    for exhausted in (False,True):
        state = fixture(1,1)
        if exhausted:
            state.put(CONTAINER+12,[CHILD]);state.put(CHILD,[0,256,PIXELS,0,0,0])
        else:
            state.put(TABLE+4,[0])
        schedule = {(HEAD,1):[(D+8,2)]}
        # Empty slot still allocates height 1, then height 2 on retry.
        replies = [(CONTAINER+64,[]),(BUCKET+8,[])]
        if not exhausted:
            replies += [(CONTAINER+128,[]),(BUCKET+16,[])]
        replies += [(NODE,[])]
        sequence(state,[replies],schedule=schedule,category='read_mutations')
    # Null allocation is mapped to observe continued unchecked effects;
    # unmapped/partial node, source/destination, bucket/node/snapshot faults.
    state = fixture(1,1);state.map_zero = False
    sequence(state,[[(0,[])]])
    for interior in (False,True):
        state = fixture(1,1)
        if interior:
            state.put(CONTAINER+12,[CHILD]);state.put(CHILD,[0,16,PIXELS,0,0,0])
        dest = CHILD+20 if interior else CONTAINER+12
        sequence(state,[[(NODE,[(dest,0x80000000)])]])
    for allocation in (0,0x80000000,ARENA+SIZE-4,ARENA+SIZE-16):
        sequence(fixture(1,1),[[(allocation,[])]])
    for field,bad in ((D+16,0x80000000),(D+16,ARENA+SIZE-1),
            (CONTAINER+8,0x80000000),(CONTAINER+8,ARENA+SIZE-1),
            (D+20,0x80000000)):
        state = fixture(3,2)
        state.put(field,[bad])
        sequence(state,[[(NODE,[])]])
    for bad in (0x80000000,ARENA+SIZE-4):
        state = fixture();state.put(TABLE+16,[bad])
        sequence(state,[[]])
        state = fixture();state.put(BUCKET+4,[bad])
        sequence(state,[[]])
        state = fixture();state.put(CONTAINER+12,[bad])
        sequence(state,[[]])
    for descriptor in (0x80000000,ARENA+SIZE-8,ARENA+SIZE-12,ARENA+SIZE-60):
        state = fixture(1,1)
        if descriptor == ARENA+SIZE-12:
            state.put(descriptor,[0,1,1])
        if descriptor == ARENA+SIZE-60:
            state.put(descriptor,[0,1,1,0,SOURCE,259]+[0]*9)
        sequence(state,[[(NODE,[])]],descriptor)
    # Faults in real page/bucket dependencies, with state preserved before exit.
    for replies in ([(0x80000000,[])],[(PAGE+64,[]),(0,[])],
            [(PAGE+64,[]),(ARENA+0x30003,[]),(0x80000000,[])],
            [(PAGE+64,[]),(ARENA+0x30003,[]),(CONTAINER+64,[]),(0x80000000,[])]):
        state = fixture();state.put(TABLE+16,[0]);state.put(HEAD,[0])
        sequence(state,[replies])
    # Nonproductive cycles are bounded by the observer, never by production C.
    state = fixture();state.put(CONTAINER+12,[CHILD]);state.put(CHILD,[0,256,PIXELS,0,0,0]);state.put(BUCKET,[BUCKET,CONTAINER])
    sequence(state,[[]],limit=40)
    state = fixture();state.put(CONTAINER+12,[CHILD]);state.put(CHILD,[0,256,PIXELS,0,0,CHILD])
    sequence(state,[[]],limit=40)
    rng = random.Random(RVA)
    for seed in range(48):
        width,height = rng.randrange(1,32),rng.randrange(1,8)
        sequence(fixture(width,height,seed),[[(NODE+0x40*k,[])] for k in range(3)])
    details = {'scope':'Independent raw-offset storage oracle, ordered external accesses, sixteen-word local snapshot at real pixel entry, complete data/image/caller-stack state, cdecl nonvolatile/DF return ABI; EAX unspecified',
        'coverage':coverage,'real_dependency_entries_per_binary':bodies,
        'compiler':'Provisional Clang/LLD strict C89 PE32 extracted production C; only CRT malloc modeled',
        'limitations':'No instruction equality, original compiler/link layout, native graphics/heap/game/fault parity or whole ImageOp integration; out-of-table negative retry heights, template/global/caller-stack pointer aliases, exhaustive partial overlap, arbitrary reentry/concurrency and portable unchecked-fault guarantees excluded; read mutations are deterministic observer schedules'}
    for kind in ('compilation','emulation'):
        record_run(RVA,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
            cases=coverage['comparisons'] if kind == 'emulation' else 0,
            command='uv run python tools/verify_sprite_storage.py',details=details)
    print('PASS storage:',coverage,'real bodies:',bodies,flush=True)
    return coverage


if __name__ == '__main__':
    verify_sprite_storage()
