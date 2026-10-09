# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Pointer-table growth: independent raw-address oracle, modeled CRT heap only."""
import hashlib
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from build_decomp import build
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run
if not __debug__:
    raise RuntimeError('Verification requires assertions')
DIGEST = 'c70cf873d3217422a169e38c5ba292d11bad39e619c1873c813d5830be610d82'
DLL = BUILD / 'gfx_table_validation.dll'
INPUTS = ['decomp/src/geputget.c','decomp/include/geputget.h',
    'decomp/src/mem.c','decomp/include/mem.h','decomp/src/lisa3d.c',
    'decomp/include/lisa3d.h','decomp/target.json','tools/build_decomp.py',
    'tools/verify_sprite_free.py','tools/verify_sprite_packing.py',
    'tools/verify_gfx_table.py','tools/verify_matching.py',
    'tools/windows_target.py','tools/windows_tracking.py']
import struct
import random
from unicorn import UcError
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EFLAGS
from verify_matching import PRESERVED, STACK, STOP
from verify_sprite_packing import State as PackingState, GapStop, ARENA, SIZE, MASK
from verify_sprite_free import Session as FreeSession
from unicorn import UC_MEM_WRITE

NAME,RVA = 'Gfx_GrowPointerTable',0x5f560
D,OLD,NEW = ARENA+0x100,ARENA+0x20000,ARENA+0x60000


def signed(word):
    return word if word < 0x80000000 else word-0x100000000


class State(PackingState):
    def put(self,address,words):
        region,offset = self.region(address,4*len(words))
        assert region is not None
        struct.pack_into('<'+'I'*len(words),region,offset,*[x&MASK for x in words])

    def read(self,address):
        value = super().read(address)
        if self.limit is not None and self.read_count == self.limit:
            self.fault = ('budget',self.limit)
            raise GapStop
        return value

    def write(self,address,value):
        if not super().write(address&MASK,value&MASK):
            raise GapStop
        return True

    def grow(self,packet,replies,free_mutations=(),limit=None):
        self.events,self.fault,self.read_count,self.limit = [],None,0,limit
        pending = list(replies)
        try:
            old = self.read(packet+12)
            count = self.read(packet+8)
            size = (count*4+4000)&MASK
            # Direct CRT malloc, including a zero-size request.
            self.events.append(('malloc',size,self.snapshot()))
            allocation,mutations = pending.pop(0)
            for address,word in mutations:
                self.put(address,[word])
            self.write(packet+12,allocation)
            index = 0
            while signed(self.read(packet+8)) > signed(index):
                word = self.read((old+index*4)&MASK)
                base = self.read(packet+12)
                self.write((base+index*4)&MASK,word)
                index = (index+1)&MASK
            index = self.read(packet+8)
            if signed((index+1000)&MASK) > signed(index):
                while True:
                    base = self.read(packet+12)
                    self.write((base+index*4)&MASK,0)
                    index = (index+1)&MASK
                    upper = (self.read(packet+8)+1000)&MASK
                    if signed(upper) <= signed(index):
                        break
            self.write(packet+8,(self.read(packet+8)+1000)&MASK)
            # Captured old pointer is freed unconditionally, even null.
            self.events.append(('free',old,self.snapshot()))
            for address,word in free_mutations:
                self.put(address,[word])
        except GapStop:
            return pending
        return pending


class Session(FreeSession):
    def __init__(self,pe,state,original):
        super().__init__(pe,state,original)
        if original:
            self.symbols[NAME] = RVA+0x400000

    def memory(self,uc,access,address,size,value,data):
        limit = self.read_limit
        self.read_limit = None
        super().memory(uc,access,address,size,value,data)
        self.read_limit = limit
        if access != UC_MEM_WRITE and limit is not None and self.read_count == limit:
            self.fault = ('budget',limit)
            uc.emu_stop()


    def run_growth(self,packet,replies,expected,remaining,seed,free_mutations=(),limit=None):
        name = NAME
        self.events,self.fault,self.mutations,self.reply = [],None,free_mutations,0xdeadbeef
        self.replies,self.read_count,self.read_limit = list(replies),0,limit
        uc,sp = self.uc,STACK+0x8000
        uc.mem_write(sp,struct.pack('<II',STOP,packet))
        caller = bytes(uc.mem_read(sp,0x8000))
        saved = [random.Random(seed+i).getrandbits(32) for i in range(4)]
        for reg,value in zip(PRESERVED,saved):
            uc.reg_write(reg,value)
        uc.reg_write(UC_X86_REG_ESP,sp)
        uc.reg_write(UC_X86_REG_EFLAGS,2)
        before = bytearray(uc.mem_read(self.base,self.size))
        try:
            uc.emu_start(self.symbols[name],STOP,count=1000000)
        except UcError:
            assert self.fault is not None
        assert self.fault == expected.fault, (name,self.fault,expected.fault)
        assert self.events == expected.events, (name,self.events,expected.events)
        assert self.snapshot() == expected.snapshot()
        assert self.replies == remaining
        after = bytearray(uc.mem_read(self.base,self.size))
        for _,p,n in self.fields:
            before[p-self.base:p-self.base+n] = after[p-self.base:p-self.base+n]
        assert before == after, 'Unrelated image bytes changed'
        if self.fault is None:
            assert uc.reg_read(UC_X86_REG_EIP) == STOP
            assert uc.reg_read(UC_X86_REG_ESP) == sp+4
            assert [uc.reg_read(r) for r in PRESERVED] == saved
            assert uc.reg_read(UC_X86_REG_EFLAGS)&0x400 == 0
            assert bytes(uc.mem_read(sp,0x8000)) == caller


def fixture(count=0,seed=0,old=OLD):
    state = State(seed)
    state.put(D,[0x12345678,0x87654321,count,old])
    return state


CALLERS = [
    (0x5f4c0,151,0x45f4e4,0x51fc88,'eb1b0a07ea378bab1a4d2239fcf07a71a725fe445bcdaf762b63cc3635caa663'),
    (0x5f7b0,245,0x45f7da,0x520360,'e2bc725199e7c67bfa58483239fba8d777b8db55951726a1190e3f7933bfe4c2'),
    (0x60150,419,0x46017b,0x51fe30,'1818a9307d8d0f964619a2442a3c7b903673c8bb7ead3cb8775459e36f09e493'),
    (0x60860,646,0x46089d,0x51fe98,'1eb84b4cc5448f726e623b47e611860499a6f47b763d7148d91fc22f74035e4d'),
    (0x60de0,419,0x460e0e,0x5203b8,'fdff044c6897fa75554edde628d92840899d25f9d31c864a79936f31f1eb1260'),
    (0x60f90,539,0x46102d,0x5203b8,'3b4153830d297745e0c123de5ef4249589bd58964d8be9d4993497a033167a75'),
    (0x61360,329,0x46138a,0x520380,'81b591def47aa754c8a41dd4255ba978ad5ff82514ca2533537dd607077a2020')]


def inspect(pe):
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    assert fpo[RVA] == (133,0,1,0x1408)
    assert hashlib.sha256(pe.get_data(RVA,133)).hexdigest() == DIGEST
    md = Cs(CS_ARCH_X86,CS_MODE_32)
    instructions = list(md.disasm(pe.get_data(RVA,133),RVA+0x400000))
    assert sum(i.size for i in instructions) == 133
    assert [(i.address,i.op_str) for i in instructions if i.mnemonic == 'ret'] == [(0x45f5e4,'')]
    assert [(i.address,i.op_str) for i in instructions if i.mnemonic == 'call'] == [
        (0x45f576,'0x469400'),(0x45f5d8,'0x4693b0')]
    reloc = {e.rva for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries if e.type == 3}
    assert not any(RVA <= r < RVA+133 for r in reloc)
    assert not any(struct.unpack('<I',pe.get_data(r,4))[0] == RVA+0x400000 for r in reloc)
    assert pe.get_data(RVA+133,11) == b'\xcc'*11
    assert [(r,i.address) for r,(n,*_) in fpo.items()
        for i in md.disasm(pe.get_data(r,n),r+0x400000)
        if i.mnemonic in ('call','jmp') and i.op_str == '0x45f560'] == [
            (r,site) for r,_,site,_,_ in CALLERS]
    for r,n,site,control,digest in CALLERS:
        assert fpo[r][0] == n and hashlib.sha256(pe.get_data(r,n)).hexdigest() == digest
        ins = list(md.disasm(pe.get_data(r,n),r+0x400000))
        push = next(i for i in ins if site-7 <= i.address < site and i.mnemonic == 'push')
        assert push.op_str == hex(control)
        assert any(i.address > site and i.address <= site+16 and i.mnemonic == 'add' and i.op_str == 'esp, 4' for i in ins)
        cr = control-0x400000
        section = next(s for s in pe.sections if s.VirtualAddress <= cr and cr+16 <= s.VirtualAddress+s.Misc_VirtualSize)
        assert section.Name.rstrip(b'\0') == b'.data' and cr >= section.VirtualAddress+section.SizeOfRawData
    print('Authenticated growth, direct CRT calls, all seven caller/control views and loader-zeroed control storage.',flush=True)


def verify_gfx_table():
    verify_target()
    original = pefile.PE(str(TARGET))
    inspect(original)
    build(dll=DLL)
    rebuilt = pefile.PE(str(DLL))
    coverage = {'comparisons':0,'persistent_followups':0,'faults':0,
        'bounded_prefixes':0,'aliases':0,'mutation_calls':0,'zero_size_malloc':0}
    def sequence(state,calls,packet=D,alias=False):
        sessions = [Session(p,state,o) for p,o in ((original,True),(rebuilt,False))]
        for repeat,(allocation,mutations,free_mutations,limit) in enumerate(calls):
            replies = [(allocation,mutations)]
            remaining = state.grow(packet,replies,free_mutations,limit)
            for session in sessions:
                session.run_growth(packet,replies,state,remaining,coverage['comparisons'],free_mutations,limit)
            coverage['comparisons'] += 1
            coverage['persistent_followups'] += int(repeat != 0)
            coverage['faults'] += int(state.fault is not None and state.fault[0] != 'budget')
            coverage['bounded_prefixes'] += int(state.fault is not None and state.fault[0] == 'budget')
            coverage['aliases'] += int(alias)
            coverage['mutation_calls'] += int(bool(mutations) or bool(free_mutations))
            coverage['zero_size_malloc'] += sum(e[0] == 'malloc' and e[1] == 0 for e in state.events)
            if state.fault:
                break
    def calls(repeats=2,allocation=NEW,mutations=(),free_mutations=(),limit=None):
        return [(allocation+k*0x10000,mutations,free_mutations,limit) for k in range(repeats)]
    # Real persistent growth with independent initial retained-word expectations.
    for count in list(range(65))+[255,256,257,999,1000,1001,2000,4000]:
        state = fixture(count,count,old=0 if count == 0 else OLD)
        if count:
            state.put(OLD,[0x10203040+k for k in range(count)])
        sequence(state,calls(3))
    # Negative and high-bit raw capacities preserve original wrapping clears.
    for count in (0xffffffff,0xfffffffe,0xfffffc18,0xfffffc17,
            0x80000000,0x80000001,0x800003e8):
        sequence(fixture(count,count),calls())
    # Independently specified copied words, zero slots, cap increment and free.
    for count in (0,1,4,1000):
        state = fixture(count,500+count,old=0 if count == 0 else OLD)
        if count:
            state.put(OLD,[0x10203040+k for k in range(count)])
        expected = fixture(count,500+count,old=0 if count == 0 else OLD)
        if count:
            expected.put(OLD,[0x10203040+k for k in range(count)])
        expected.grow(D,[(NEW,[])])
        assert [e[1] for e in expected.events if e[0] == 'malloc'] == [count*4+4000]
        assert [e[1] for e in expected.events if e[0] == 'free'] == [0 if count == 0 else OLD]
        region,offset = expected.region(NEW,(count+1000)*4)
        assert bytes(region[offset:offset+count*4]) == struct.pack('<'+'I'*count,*[0x10203040+k for k in range(count)])
        assert bytes(region[offset+count*4:offset+(count+1000)*4]) == bytes(4000)
        assert struct.unpack_from('<I',expected.arena,D-ARENA+8)[0] == count+1000
        sequence(state,calls(1))
    # Exact and partial control/source/destination aliases and forward copies.
    for count in (0,1,2,3,7,31):
        for allocation in (D-4,D,D+4,D+8,D+12,OLD-4,OLD,OLD+4,OLD+1):
            sequence(fixture(count,1000+count),calls(1,allocation),alias=True)
        for old in (D,D+4,D+8,D+12,NEW,NEW+4):
            sequence(fixture(count,2000+count,old=old),calls(1),alias=True)
    for delta in (1,2,3):
        state = fixture(3,3000+delta)
        state.put(D+delta,[0x12345678,0x87654321,3,OLD])
        sequence(state,calls(1),packet=D+delta,alias=True)
    for count,mutation in ((7,0),(7,2),(7,31),(0x7fffffff,0),
            (0,0x80000000),(7,0x7ffffc18),(0,0x7fffffff)):
        sequence(fixture(count,4000+mutation),calls(1,mutations=[(D+8,mutation)],
            limit=31 if mutation in (0x7ffffc18,0x7fffffff) else None))
    # Saved old pointer and live destination semantics at both CRT boundaries.
    for count in (0,1,7,1000):
        sequence(fixture(count,5000+count),calls(1,mutations=[(D+12,0x80000000)],
            free_mutations=[(D+8,0x87654321),(D+12,OLD)]))
        sequence(fixture(count,5100+count),calls(1,mutations=[(OLD,0xdeadbeef)]))
    bad = 0x80000000
    for packet in (bad,ARENA+SIZE-12,ARENA+SIZE-8,ARENA+SIZE-4):
        sequence(fixture(),calls(1),packet=packet)
    for count,old,allocation in ((1,bad,NEW),(1,0,NEW),(0,OLD,0),
            (1,OLD,0),(0,OLD,bad),(1,OLD,bad),(0xffffffff,OLD,4),
            (0,OLD,ARENA+SIZE),(0,OLD,ARENA+SIZE-4),
            (2,ARENA+SIZE-4,NEW),(3,OLD,ARENA+SIZE-4)):
        sequence(fixture(count,6000+count,old=old),calls(1,allocation))
    # No fabricated capacity/cycle guard: bound genuinely large copy prefixes.
    for count in (0x7fffffff,0x7ffffc18,0x7ffffc17):
        sequence(fixture(count,count),calls(1,limit=31))
    details = {'scope':'Instruction-derived raw-offset oracle, production generic table body; direct CRT malloc/free modeled with explicit replies/mutations and volatile clobbers; no success stubs',
        'coverage':coverage,'checks':'exact reads/writes, source capture and immediate publication, live copy/clear pointer and bounds, direct zero-size malloc, unconditional captured-old free including null, full arena/global/unrelated-image bytes, persistent growth, signed/wrapping sizes and indices, aliases/fault retained effects/bounded prefixes and normal cdecl caller-stack/nonvolatile/DF; incidental EAX excluded',
        'compiler':'Provisional Clang/LLD strict C89 PE32; original compiler/link layout unresolved',
        'limitations':'No instruction equality, native heap/graphics/game/fault parity, whole caller/global-control integration, arbitrary reentry/concurrency, exhaustive aliases or portable unchecked-fault guarantees; fault-time registers/stack and cross-page dword stores excluded; external prefix limits are not termination guards'}
    for kind in ('compilation','emulation'):
        record_run(RVA,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
            cases=coverage['comparisons'] if kind == 'emulation' else 0,
            command='uv run python tools/verify_gfx_table.py',details=details)
    print('PASS generic pointer-table growth:',coverage,flush=True)
    return coverage


if __name__ == '__main__':
    verify_gfx_table()
