# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Allocated-string copying: independent byte oracle and real allocation wrapper."""
import hashlib
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from build_decomp import build
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run
if not __debug__:
    raise RuntimeError('Verification requires assertions')
DIGEST = 'c74c6d2d995e97623501ae2f7f259573637233d61412dbf2db008c793eaa2e5d'
DLL = BUILD / 'gfx_string_validation.dll'
INPUTS = ['decomp/src/geputget.c','decomp/include/geputget.h',
    'decomp/src/mem.c','decomp/include/mem.h','decomp/src/lisa3d.c',
    'decomp/include/lisa3d.h','decomp/target.json','tools/build_decomp.py',
    'tools/verify_sprite_free.py','tools/verify_sprite_packing.py',
    'tools/verify_gfx_string.py','tools/verify_matching.py',
    'tools/windows_target.py','tools/windows_tracking.py']
import struct
import random
from unicorn import UcError
from unicorn.x86_const import UC_X86_REG_EIP, UC_X86_REG_ESP, UC_X86_REG_EFLAGS
from verify_matching import PRESERVED, STACK, STOP
from verify_sprite_packing import State as PackingState, GapStop, ARENA, SIZE, MASK, inspect as inspect_dependencies
from verify_sprite_free import Session as FreeSession

NAME,RVA = 'Gfx_CopyAllocatedString',0x60480
S,O,A = ARENA+0x100,ARENA+0x400,ARENA+0x800


class State(PackingState):
    def put(self,address,data):
        region,offset = self.region(address,len(data))
        assert region is not None
        region[offset:offset+len(data)] = data

    def byte(self,address,value=None):
        address &= MASK
        region,offset = self.region(address,1)
        if region is None:
            self.fault = ('read' if value is None else 'write',address,1)
            raise GapStop
        if value is None:
            value = region[offset]
            self.events.append(('read',address,1,value))
        else:
            region[offset] = value
            self.events.append(('write',address,1,value))
        return value

    def copy_name(self,source,destination,replies):
        self.events,self.fault = [],None
        pending = list(replies)
        try:
            if source == 0:
                if not self.write(destination,0):
                    raise GapStop
                return pending
            count = 0
            while self.byte((source+count)&MASK) != 0:
                count = (count+1)&MASK
            count = (count+1)&MASK
            self.boundary('Gfx_AllocBytes',(count,))
            allocation = self.allocate(count,pending)
            if not self.write(destination,allocation):
                raise GapStop
            if count & 0x80000000:
                return pending
            for index in range(count):
                value = self.byte((source+index)&MASK)
                self.byte((allocation+index)&MASK,value)
        except GapStop:
            return pending
        return pending


class Session(FreeSession):
    def __init__(self,pe,state,original):
        super().__init__(pe,state,original)
        if original:
            self.symbols[NAME] = 0x400000+RVA
        self.page_mode = 'real'

    def run_name(self,source,destination,replies,expected,remaining,seed):
        name = NAME
        self.events,self.fault,self.replies = [],None,list(replies)
        uc,sp = self.uc,STACK+0x8000
        uc.mem_write(sp,struct.pack('<III',STOP,source,destination))
        caller = bytes(uc.mem_read(sp,0x8000))
        saved = [random.Random(seed+i).getrandbits(32) for i in range(4)]
        for reg,value in zip(PRESERVED,saved):
            uc.reg_write(reg,value)
        uc.reg_write(UC_X86_REG_ESP,sp)
        uc.reg_write(UC_X86_REG_EFLAGS,2)
        before = bytearray(uc.mem_read(self.base,self.size))
        try:
            uc.emu_start(self.symbols[name],STOP,count=10000)
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


def inspect(pe):
    inspect_dependencies(pe)
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    assert fpo[RVA] == (72,0,2,0x202)
    assert hashlib.sha256(pe.get_data(RVA,72)).hexdigest() == DIGEST
    md = Cs(CS_ARCH_X86,CS_MODE_32)
    instructions = list(md.disasm(pe.get_data(RVA,72),RVA+0x400000))
    assert sum(i.size for i in instructions) == 72
    assert [i.address for i in instructions if i.mnemonic == 'ret'] == [0x460494,0x4604c7]
    assert [(i.address,i.op_str) for i in instructions if i.mnemonic == 'call'] == [(0x4604a3,'0x45f4a0')]
    reloc = {e.rva for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries if e.type == 3}
    assert not any(RVA <= r < RVA+72 for r in reloc)
    assert not any(struct.unpack('<I',pe.get_data(r,4))[0] == RVA+0x400000 for r in reloc)
    assert pe.get_data(RVA+72,8) == b'\xcc'*8
    assert [(r,i.address) for r,(n,*_) in fpo.items()
        for i in md.disasm(pe.get_data(r,n),r+0x400000)
        if i.mnemonic in ('call','jmp') and i.op_str == '0x460480'] == [
            (0x5fe30,0x45ff29),(0x60150,0x460275),(0x60cb0,0x460ccd),
            (0x60cf0,0x460d32),(0x60de0,0x460f42),(0x60f90,0x461181),
            (0x61210,0x461261),(0x61360,0x46148a)]
    assert hashlib.sha256(pe.get_data(0x61360,329)).hexdigest() == '81b591def47aa754c8a41dd4255ba978ad5ff82514ca2533537dd607077a2020'
    print('Authenticated allocated-string copy, real allocation wrapper and all eight direct callers.',flush=True)


def verify_gfx_string():
    verify_target()
    original = pefile.PE(str(TARGET))
    inspect(original)
    build(dll=DLL)
    rebuilt = pefile.PE(str(DLL))
    coverage = {'comparisons':0,'persistent_followups':0,'faults':0,
        'aliases':0,'mutation_calls':0,'malloc_calls':0}
    def fixture(data=b'hello',seed=0,source=S):
        state = State(seed)
        state.put(source,data+b'\0'+bytes(32))
        return state
    def sequence(state,source=S,destination=O,allocation=A,mutations=(),repeats=2,alias=False,expected_bytes=None):
        sessions = [Session(p,state,o) for p,o in ((original,True),(rebuilt,False))]
        for repeat in range(repeats):
            replies = [(allocation,mutations)] if source else []
            remaining = state.copy_name(source,destination,replies)
            if repeat == 0 and expected_bytes is not None:
                region,offset = state.region(allocation,len(expected_bytes))
                assert bytes(region[offset:offset+len(expected_bytes)]) == expected_bytes
            for session in sessions:
                session.run_name(source,destination,replies,state,remaining,coverage['comparisons'])
            coverage['comparisons'] += 1
            coverage['persistent_followups'] += int(repeat != 0)
            coverage['faults'] += int(state.fault is not None)
            coverage['aliases'] += int(alias)
            coverage['mutation_calls'] += int(bool(mutations))
            coverage['malloc_calls'] += sum(e[0] == 'malloc' for e in state.events)
            if state.fault:
                break
    for length in range(257):
        data = bytes(1+(k*73)%255 for k in range(length))
        sequence(fixture(data,seed=length),expected_bytes=data+b'\0')
    for seed in range(32):
        sequence(fixture(seed=seed),source=0)
        sequence(fixture(b'',seed=seed),expected_bytes=b'\0')
    # Independent hand-derived forward propagation and post-allocation bytes.
    sequence(fixture(b'abcde'),allocation=S+1,alias=True,expected_bytes=b'aaaaaa')
    sequence(fixture(b'abc'),mutations=[(S,0x5a595857)],repeats=1,expected_bytes=b'WXYZ')
    sequence(fixture(b'ABCDE'),destination=S,alias=True,
        expected_bytes=struct.pack('<I',A)+b'E\0')
    for offset in range(-8,9):
        for length in (0,1,3,8,16):
            sequence(fixture(b'X'*length,seed=length),allocation=S+offset,alias=True)
    for offset in range(-4,9):
        sequence(fixture(b'abcdefghijk'),destination=S+offset,alias=True)
        sequence(fixture(b'abcdefghijk'),destination=A+offset,alias=True)
    for seed in range(32):
        sequence(fixture(b'abcd',seed),mutations=[(S,seed*0x01010101&MASK)],repeats=1)
        sequence(fixture(b'abcd',seed),mutations=[(O,0xffffffff)],repeats=1)
    bad = 0x80000000
    for source,destination,allocation in ((bad,O,A),(S,bad,A),(0,bad,A),
            (S,O,0),(S,O,bad),(S,O,ARENA+SIZE-2)):
        sequence(fixture(),source,destination,allocation,repeats=1)
    state = State(1000);state.put(ARENA+SIZE-4,b'abcd')
    sequence(state,source=ARENA+SIZE-4,repeats=1)
    state = State(1001);state.put(ARENA+SIZE-4,b'abc\0')
    sequence(state,source=ARENA+SIZE-4,repeats=1,expected_bytes=b'abc\0')
    details = {'scope':'Instruction-derived byte oracle and real production string/allocation bodies; only CRT malloc modeled with explicit replies/mutations and volatile clobbers',
        'coverage':coverage,'checks':'exact scan and copy byte order, published pointer timing, malloc size/entry/pre-malloc snapshots, all arena/global/unrelated-image bytes, persistent effects, forward/source/destination aliases, empty/null/boundary/high-bit bytes, fault retained effects and normal cdecl caller-stack/nonvolatile/DF; incidental EAX excluded',
        'compiler':'Provisional strict C89 Clang/LLD PE32; no original compiler/codegen/layout claim',
        'limitations':'No instruction equality, native heap/game/fault parity, whole caller integration, 32-bit scan/count overflow, arbitrary reentry/concurrency, caller-stack/global pointer aliases or portable unchecked-fault guarantees; scheduled heap replies are not heap lifecycle validation; cross-page dword publication excluded because Unicorn partially commits mapped bytes before reporting byte faults'}
    for kind in ('compilation','emulation'):
        record_run(RVA,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
            cases=coverage['comparisons'] if kind == 'emulation' else 0,
            command='uv run python tools/verify_gfx_string.py',details=details)
    print('PASS graphics string copy:',coverage,flush=True)
    return coverage


if __name__ == '__main__':
    verify_gfx_string()
