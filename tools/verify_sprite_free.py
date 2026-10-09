# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Graphics free helpers: real production bodies, explicitly modeled CRT free."""
import hashlib
import random
import struct
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import UcError
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX,
    UC_X86_REG_EDX, UC_X86_REG_EIP, UC_X86_REG_ESP, UC_X86_REG_EFLAGS)
from verify_sprite_packing import State, Session as PackingSession, GapStop, ARENA, SIZE, MASK
from verify_matching import PRESERVED, STACK, STOP
from build_decomp import build
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')

ROUTINES = {
    'Gfx_FreeBytes': (0x5f8b0,18,0,1,0,'404196d294b1f0112a432fb80398ae7d09669d883e27df694804678cd1d91d94'),
    'Gfx_FreeAlignedBytes': (0x61a60,17,0,1,0,'396fe877c29ad0654d7555f73c4f499cb23e2069f5d64946b6ecb8ad14c6588b')}
DLL = BUILD / 'sprite_free_validation.dll'
INPUTS = ['decomp/src/geputget.c','decomp/include/geputget.h',
    'decomp/src/mem.c','decomp/include/mem.h','decomp/src/lisa3d.c',
    'decomp/include/lisa3d.h','decomp/target.json','tools/build_decomp.py',
    'tools/verify_sprite_free.py','tools/verify_sprite_packing.py',
    'tools/verify_matching.py','tools/windows_target.py','tools/windows_tracking.py']


def inspect(pe):
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    md = Cs(CS_ARCH_X86,CS_MODE_32)
    reloc = {e.rva for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries if e.type == 3}
    for name,(r,n,l,a,b,digest) in ROUTINES.items():
        assert fpo[r] == (n,l,a,b)
        assert hashlib.sha256(pe.get_data(r,n)).hexdigest() == digest
        instructions = list(md.disasm(pe.get_data(r,n),0x400000+r))
        assert sum(i.size for i in instructions) == n and instructions[-1].mnemonic == 'ret'
        assert not any(r <= x < r+n for x in reloc)
        assert not any(struct.unpack('<I',pe.get_data(x,4))[0] == r+0x400000 for x in reloc)
        assert [(i.address,i.op_str) for i in instructions if i.mnemonic == 'call'] == (
            [(0x45f8b9,'0x4693b0')] if name == 'Gfx_FreeBytes' else [(0x461a68,'0x45f8b0')])
    assert pe.get_data(0x5f8c2,14) == b'\xcc'*14
    assert pe.get_data(0x61a71,15) == b'\xcc'*15
    assert [(r,i.address) for r,(n,*_) in fpo.items()
        for i in md.disasm(pe.get_data(r,n),0x400000+r)
        if i.mnemonic in ('call','jmp') and i.op_str == '0x461a60'] == [(0x618b0,0x46199c)]
    assert fpo[0x618b0] == (419,1,1,0x1410)
    assert hashlib.sha256(pe.get_data(0x618b0,419)).hexdigest() == '7563059eb087b1961ce5aac8901e5ed29574c2f1a9354e62ed943d5842fb0072'
    print('Authenticated graphics free helper extents, hashes, ABI, calls and relocations.',flush=True)


class Session(PackingSession):
    def __init__(self, pe, state, original):
        super().__init__(pe,state,original)
        if original:
            self.symbols.update({name:0x400000+row[0] for name,row in ROUTINES.items()})
            self.symbols['free'] = 0x4693b0
        self.bucket_mode,self.page_mode,self.read_limit = False,None,None
        self.read_count = 0

    def code(self, uc, address, size, data):
        if address == self.symbols['free']:
            sp = uc.reg_read(UC_X86_REG_ESP)
            ret,arg = struct.unpack('<II',uc.mem_read(sp,8))
            self.events.append(('free',arg,self.snapshot()))
            for dest,word in self.mutations:
                uc.mem_write(self.physical(dest),struct.pack('<I',word&MASK))
            uc.reg_write(UC_X86_REG_EAX,self.reply)
            uc.reg_write(UC_X86_REG_ECX,0xbadc0ffe)
            uc.reg_write(UC_X86_REG_EDX,0x87654321)
            uc.reg_write(UC_X86_REG_EFLAGS,0x8d7)
            uc.reg_write(UC_X86_REG_ESP,sp+4)
            uc.reg_write(UC_X86_REG_EIP,ret)
        else:
            super().code(uc,address,size,data)

    def run_free(self,name,arg,expected,seed,mutations=(),reply=0):
        self.events,self.fault,self.mutations,self.reply = [],None,mutations,reply
        uc,sp = self.uc,STACK+0x8000
        uc.mem_write(sp,struct.pack('<II',STOP,arg))
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


def expected_free(state,name,arg,mutations=()):
    state.events,state.fault = [],None
    try:
        allocation = state.read((arg-4)&MASK) if name == 'Gfx_FreeAlignedBytes' else arg
        if allocation:
            state.events.append(('free',allocation,state.snapshot()))
            for dest,word in mutations:
                data,offset = state.region(dest,4)
                struct.pack_into('<I',data,offset,word&MASK)
    except GapStop:
        pass


def verify_sprite_free():
    verify_target()
    original = pefile.PE(str(TARGET))
    inspect(original)
    build(dll=DLL)
    rebuilt = pefile.PE(str(DLL))
    coverage = {name:{'comparisons':0,'persistent_followups':0,'faults':0,'crt_calls':0}
        for name in ROUTINES}
    def sequence(name,arg,header,seed,mutate=False):
        state = State(seed)
        if header is not None:
            data,offset = state.region((arg-4)&MASK,4)
            struct.pack_into('<I',data,offset,header)
        sessions = [Session(p,state,o) for p,o in ((original,True),(rebuilt,False))]
        for repeat in range(3):
            mutations = [(arg-4,0)] if mutate and repeat == 0 else []
            expected_free(state,name,arg,mutations)
            for session in sessions:
                session.run_free(name,arg,state,seed+repeat,mutations,0xdeadbeef^repeat)
            c = coverage[name]
            c['comparisons'] += 1
            c['persistent_followups'] += int(repeat != 0)
            c['faults'] += int(state.fault is not None)
            c['crt_calls'] += sum(e[0] == 'free' for e in state.events)
            if state.fault:
                break
    pointers = [0,1,4,0x7fffffff,0x80000000,MASK,ARENA,ARENA+SIZE]
    pointers += [random.Random(i+0x5f8b0).getrandbits(32) for i in range(64)]
    for seed,arg in enumerate(pointers):
        sequence('Gfx_FreeBytes',arg,None,seed)
    for seed,header in enumerate(pointers):
        # Unaligned dword loads and mapped boundary header locations.
        for arg in (ARENA+4,ARENA+5,ARENA+0x200,ARENA+SIZE):
            sequence('Gfx_FreeAlignedBytes',arg,header,seed)
    for seed in range(16):
        sequence('Gfx_FreeAlignedBytes',ARENA+0x300,0x12345678,1000+seed,True)
    for arg in (0,1,3,0x80000000,MASK,ARENA,ARENA+3,ARENA+SIZE+4):
        sequence('Gfx_FreeAlignedBytes',arg,None,2000+arg)
    for name,(rva,*_) in ROUTINES.items():
        details = {'coverage':coverage[name], 'boundary':'CRT free modeled with reply, volatile clobbers and header mutations; no native heap/game parity',
            'checks':'ordered header reads, free argument and pre-free snapshots, all arena/global/image bytes, persistent header poisoning, read faults, cdecl stack/caller/nonvolatile/DF; incidental EAX excluded',
            'compiler':'Clang/LLD provisional strict C89 PE32; no instruction equality or original compiler/link-layout claim'}
        for kind in ('compilation','emulation'):
            record_run(rva,kind,'pass',cases=coverage[name]['comparisons'] if kind == 'emulation' else 0,
                artifact=DLL.relative_to(ROOT).as_posix(),inputs=INPUTS,
                command='uv run python tools/verify_sprite_free.py',details=details)
    print('PASS graphics free:',coverage,flush=True)
    return coverage


if __name__ == '__main__':
    verify_sprite_free()
