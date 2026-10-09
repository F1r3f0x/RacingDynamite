# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Sprite pixel leaf: independent ordered byte-access oracle versus both x86 bodies."""
import hashlib
import random
import struct
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import (Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE,
    UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE, UC_HOOK_MEM_INVALID, UC_MEM_WRITE,
    UC_MEM_READ_UNMAPPED, UC_MEM_WRITE_UNMAPPED)
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ESP,
    UC_X86_REG_EIP, UC_X86_REG_EFLAGS)
from verify_matching import STACK, STOP, PRESERVED
from build_decomp import build
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')

NAME, RVA, LENGTH = 'Gfx_CopySpriteDescriptorPixels', 0x612a0, 61
DIGEST = '32a8a30b19b528630595c3efaab77bc2ff20798c4a133ae569091264cc784e5f'
DLL = BUILD / 'sprite_pixels_validation.dll'
MASK, A, SIZE = 0xffffffff, 0x3000000, 0x10000
S, D, P, Q = A+0x100, A+0x200, A+0x4000, A+0x8000
INPUTS = ['decomp/src/geputget.c', 'decomp/include/geputget.h',
    'decomp/src/mem.c', 'decomp/include/mem.h', 'decomp/src/lisa3d.c',
    'decomp/include/lisa3d.h', 'decomp/target.json', 'tools/build_decomp.py',
    'tools/verify_sprite_pixels.py', 'tools/verify_matching.py',
    'tools/windows_target.py', 'tools/windows_tracking.py']


def inspect(pe):
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    assert fpo[RVA] == (61,0,2,0x1411)
    raw = pe.get_data(RVA,LENGTH)
    assert hashlib.sha256(raw).hexdigest() == DIGEST
    md = Cs(CS_ARCH_X86,CS_MODE_32)
    ins = list(md.disasm(raw,0x400000+RVA))
    assert sum(i.size for i in ins) == LENGTH and ins[-1].address == 0x4612dc
    assert ins[-1].mnemonic == 'ret' and not any(i.mnemonic == 'call' for i in ins)
    assert pe.get_data(RVA+LENGTH,3) == b'\xcc'*3
    reloc = {e.rva for block in pe.DIRECTORY_ENTRY_BASERELOC for e in block.entries if e.type == 3}
    assert not any(RVA <= r < RVA+LENGTH for r in reloc)
    assert not any(struct.unpack('<I',pe.get_data(r,4))[0] == 0x4612a0 for r in reloc)
    assert [(r,i.address) for r,(n,*_) in fpo.items()
        for i in md.disasm(pe.get_data(r,n),0x400000+r)
        if i.mnemonic in ('call','jmp') and i.op_str == '0x4612a0'] == [
            (0x61210,0x461289),(0x61530,0x46167e)]
    # Caller extent is authenticated by FPO and bounded call-site checks.
    assert fpo[0x61210][0] == 143
    assert [(i.mnemonic,i.op_str) for i in md.disasm(pe.get_data(0x61287,10),0x461287)][:4] == [
        ('push','ebx'),('push','ebp'),('call','0x4612a0'),('add','esp, 8')]
    assert [(i.mnemonic,i.op_str) for i in md.disasm(pe.get_data(0x6167e,8),0x46167e)] == [
        ('call','0x4612a0'),('add','esp, 8')]
    print('Authenticated pixel leaf, FPO extent, padding, no relocations and both cdecl callers.',flush=True)


class Fault(Exception):
    pass


class State:
    """Raw bytes/offsets; no production field definitions or emulator output as expectations."""
    def __init__(self, seed):
        rng = random.Random(seed)
        self.regions = {0:bytearray(rng.randbytes(4096)),
            A:bytearray(rng.randbytes(SIZE)), 0xfffff000:bytearray(rng.randbytes(4096))}
        self.events, self.fault = [], None

    def locate(self, address, size):
        address &= MASK
        for base,data in self.regions.items():
            if base <= address and address+size <= base+len(data):
                return data,address-base
        raise Fault

    def put(self, address, raw):
        data,offset = self.locate(address,len(raw))
        data[offset:offset+len(raw)] = raw

    def descriptor(self, address, width, height, pixels, stride):
        self.put(address,struct.pack('<6I',0x11223344,width&MASK,height&MASK,
            0x55667788,pixels&MASK,stride&MASK))

    def snapshot(self):
        return tuple(bytes(data) for data in self.regions.values())

    def access(self, address, size, value=None):
        address &= MASK
        kind = 'read' if value is None else 'write'
        try:
            data,offset = self.locate(address,size)
        except Fault:
            self.fault = (kind,address,size)
            raise
        if value is None:
            value = int.from_bytes(data[offset:offset+size],'little')
        else:
            data[offset:offset+size] = value.to_bytes(size,'little')
        self.events.append((kind,address,size,value))
        return value

    def copy(self, source, destination):
        self.events,self.fault = [],None
        signed = lambda value: value if value < 0x80000000 else value-0x100000000
        try:
            src = self.access(source+16,4)
            dst = self.access(destination+16,4)
            row = 0
            while signed(self.access(source+8,4)) > row:
                column = 0
                while signed(self.access(source+4,4)) > column:
                    pixel = self.access(src+column,1)
                    self.access(dst+column,1,pixel)
                    column += 1
                    assert column < 10000, 'Unbounded fixture'
                src = (src+self.access(source+20,4))&MASK
                dst = (dst+self.access(destination+20,4))&MASK
                row += 1
                assert row < 10000, 'Unbounded fixture'
        except Fault:
            pass


class Session:
    def __init__(self, pe, state, original):
        self.uc = Uc(UC_ARCH_X86,UC_MODE_32)
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.size = (pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095
        self.entry = 0x4612a0 if original else next(self.base+e.address
            for e in pe.DIRECTORY_ENTRY_EXPORT.symbols if e.name == NAME.encode())
        self.uc.mem_map(self.base,self.size)
        self.uc.mem_write(self.base,pe.get_memory_mapped_image())
        for base,data in state.regions.items():
            self.uc.mem_map(base,len(data))
            self.uc.mem_write(base,bytes(data))
        self.regions = [(base,len(data)) for base,data in state.regions.items()]
        self.uc.mem_map(STACK,0x10000)
        self.uc.mem_map(STOP,0x1000)
        self.uc.hook_add(UC_HOOK_MEM_READ | UC_HOOK_MEM_WRITE,self.memory)
        self.uc.hook_add(UC_HOOK_MEM_INVALID,self.invalid)
        self.uc.hook_add(UC_HOOK_CODE,self.code)

    def memory(self, uc, access, address, size, value, _):
        if STACK <= address < STACK+0x10000:
            return
        mapped = any(base <= address and address+size <= base+n for base,n in self.regions)
        if not mapped:
            # Invalid accesses are recorded separately; valid unexpected image
            # reads remain visible and fail the ordered event comparison.
            if access == UC_MEM_WRITE or not (self.base <= address < self.base+self.size):
                return
        if access == UC_MEM_WRITE:
            kind = 'write'
        else:
            kind,value = 'read',int.from_bytes(uc.mem_read(address,size),'little')
        self.events.append((kind,address,size,value))

    def invalid(self, uc, access, address, size, value, _):
        assert access in (UC_MEM_READ_UNMAPPED,UC_MEM_WRITE_UNMAPPED)
        self.fault = ('read' if access == UC_MEM_READ_UNMAPPED else 'write',address,size)
        return False

    def code(self, uc, address, size, _):
        if address == STOP:
            uc.emu_stop()

    def run(self, source, destination, expected, seed):
        uc = self.uc
        self.events,self.fault = [],None
        sp = STACK+0x8000
        uc.mem_write(sp,struct.pack('<III',STOP,source,destination))
        caller = bytes(uc.mem_read(sp,0x8000))
        image = bytes(uc.mem_read(self.base,self.size))
        saved = [random.Random(seed+i).getrandbits(32) for i in range(4)]
        for reg,value in zip(PRESERVED,saved):
            uc.reg_write(reg,value)
        uc.reg_write(UC_X86_REG_EAX,seed&MASK)
        uc.reg_write(UC_X86_REG_ESP,sp)
        uc.reg_write(UC_X86_REG_EFLAGS,2)
        try:
            uc.emu_start(self.entry,STOP,count=200000)
        except UcError:
            assert self.fault is not None
        assert self.fault == expected.fault, (seed,self.fault,expected.fault)
        assert self.events == expected.events, (seed,self.events[:25],expected.events[:25])
        assert tuple(bytes(uc.mem_read(base,n)) for base,n in self.regions) == expected.snapshot(), seed
        assert bytes(uc.mem_read(self.base,self.size)) == image
        assert bytes(uc.mem_read(sp,0x8000)) == caller
        if self.fault is None:
            assert uc.reg_read(UC_X86_REG_EIP) == STOP
            assert uc.reg_read(UC_X86_REG_EAX) == source
            assert uc.reg_read(UC_X86_REG_ESP) == sp+4
            assert [uc.reg_read(reg) for reg in PRESERVED] == saved
            assert uc.reg_read(UC_X86_REG_EFLAGS)&0x400 == 0


def verify_sprite_pixels():
    verify_target()
    original = pefile.PE(str(TARGET))
    inspect(original)
    build(dll=DLL)
    rebuilt = pefile.PE(str(DLL))
    assert rebuilt.FILE_HEADER.Machine == 0x14c and not hasattr(rebuilt,'DIRECTORY_ENTRY_IMPORT')
    coverage = {'comparisons':0,'persistent_followups':0,'faults':0,'live_aliases':0,'wrapping':0}

    def sequence(state, source=S, destination=D, repeats=2, category=None):
        sessions = [Session(pe,state,is_original) for pe,is_original in ((original,True),(rebuilt,False))]
        for repeat in range(repeats):
            state.copy(source,destination)
            for session in sessions:
                session.run(source,destination,state,coverage['comparisons'])
            coverage['comparisons'] += 1
            coverage['persistent_followups'] += int(repeat != 0)
            coverage['faults'] += int(state.fault is not None)
            if category:
                coverage[category] += 1
            if state.fault is not None:
                break
        return state

    def fixture(width=3,height=2,src=P,dst=Q,ss=7,ds=11,seed=0):
        state = State(seed)
        state.descriptor(S,width,height,src,ss)
        state.descriptor(D,0x80000000,0x7fffffff,dst,ds)
        return state

    # Signed bounds, admitted empty rows, strides with distinct bit patterns;
    # no invented 256 upper bound. Nonpositive height still captures pointers.
    for width in (0,-1,-2147483648,1,2,5,257):
        for height in (0,-1,-2147483648,1,2,5):
            for ss,ds in ((300,301),(0,0),(-300,-301)):
                sequence(fixture(width,height,ss=ss,ds=ds,seed=coverage['comparisons']))
    for offset in (-5,-1,0,1,2,5):
        state = fixture(5,3,src=P,dst=P+offset,ss=8,ds=7)
        state.put(P,b'abcde123abcde456abcde789')
        sequence(state,repeats=4)
    for width in (0,1):
        sequence(fixture(width,257,ss=4,ds=5))
    state = fixture(5,1,src=P,dst=P+1)
    state.put(P,b'abcde!')
    sequence(state,repeats=1)
    data,offset = state.locate(P,6)
    assert data[offset:offset+6] == b'aaaaaa'
    # Identical and partially overlapping descriptors, with dimensions never
    # read from the destination. Pixels are captured once even if overwritten.
    sequence(fixture(),source=S,destination=S,repeats=4,category='live_aliases')
    for delta in (-8,-4,4,8,12,16,20):
        state = fixture(1,1)
        state.descriptor(S+delta,0,1,Q,0)
        state.put(S+4,struct.pack('<2I',1,1))
        state.put(S+16,struct.pack('<I',P))
        sequence(state,source=S,destination=S+delta,category='live_aliases')
    # Target each live source width/height/stride and both captured pointer
    # fields with byte writes. Zero payload terminates altered bounds safely.
    for field in (4,8,16,20):
        for initial in (1,2,5):
            state = fixture(initial,2,dst=S+field,ds=0)
            state.put(P,b'\0'*64)
            sequence(state,category='live_aliases')
    for field in (4,8,16,20):
        state = fixture(4,2,dst=D+field,ds=0)
        state.put(P,b'\0'*64)
        sequence(state,category='live_aliases')
    # First byte expands the width, third ends it: catches caching bounds.
    state = fixture(1,1,dst=S+4)
    state.put(P,b'\3\0\0')
    sequence(state,repeats=1,category='live_aliases')
    assert len([e for e in state.events if e[0] == 'write']) == 3
    state = fixture(2,1,dst=S+8,ds=0)
    state.put(P,b'\2\0'+b'\0'*64)
    sequence(state,repeats=1,category='live_aliases')
    assert len([e for e in state.events if e[0] == 'write']) == 4
    for field in (0,4,8,16,20):
        sequence(fixture(4,2,src=S+field,ss=0),category='live_aliases')
    # Crossing 2^32 occurs within a row and between rows in each direction.
    for src,dst,ss,ds in ((0xfffffffe,Q,4,8),(P,0xfffffffe,8,4),
            (0xfffffffe,0xfffffffd,4,5),(2,Q,-4,7),(P,2,7,-4)):
        sequence(fixture(5,2,src,dst,ss,ds),category='wrapping')
    # Faults at each ordered descriptor read and at first/later source/dest
    # bytes. Earlier writes survive; faults do not receive return-ABI claims.
    bad = 0x80000000
    for source,destination in ((bad,D),(S,bad),(A+SIZE-16,D),(S,A+SIZE-16)):
        sequence(fixture(),source,destination,repeats=1)
    # Pixel pointers mapped but earlier height/width dwords unmapped.
    state = fixture()
    state.put(A+4,struct.pack('<I',P))
    sequence(state,source=A-12,repeats=1)
    state = fixture()
    state.put(A,struct.pack('<I',1))
    state.put(A+8,struct.pack('<I',P))
    sequence(state,source=A-8,repeats=1)
    for src,dst,ss,ds in ((bad,Q,7,11),(P,bad,7,11),
            (A+SIZE-2,Q,7,11),(P,A+SIZE-2,7,11),
            (P,Q,bad,11),(P,Q,7,bad)):
        sequence(fixture(3,2,src,dst,ss,ds),repeats=1)
    # Nonpositive width still admits strides, and source stride precedes dest.
    for source,destination in ((A+SIZE-20,D),(S,A+SIZE-20)):
        state = fixture(0,2)
        state.put(source,struct.pack('<5I',0,0,2,0,P)) if source != S else None
        state.put(destination,struct.pack('<5I',0,0,2,0,Q)) if destination != D else None
        sequence(state,source,destination,repeats=1)
    for height in (0,-1):
        sequence(fixture(1,height,src=bad,dst=bad,ss=bad,ds=bad))
    rng = random.Random(RVA)
    for seed in range(96):
        width,height = rng.randrange(12),rng.randrange(6)
        ss,ds = rng.randrange(-20,21),rng.randrange(-20,21)
        sequence(fixture(width,height,ss=ss,ds=ds,seed=seed),repeats=3)

    details = {'scope':'Independent raw-offset signed-bound oracle; ordered pointer/bound/stride/byte accesses; complete mapped data and image preservation; persistent sessions; EAX/caller-stack/nonvolatile/DF ABI on return; ordered faults and retained writes',
        'coverage':coverage, 'compiler':'Provisional Clang/LLD strict C89 PE32, extracted production body',
        'limitations':'No instruction equality, original compiler/link layout, native graphics/game/fault parity, caller integration, arbitrary concurrent mutation or caller-stack/image pixel aliases; invalid access observations are provisional x86 behavior, not portable C guarantees'}
    for kind in ('compilation','emulation'):
        record_run(RVA,kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
            cases=coverage['comparisons'] if kind == 'emulation' else 0,
            command='uv run python tools/verify_sprite_pixels.py',details=details)
    print('PASS pixel leaf:',coverage,flush=True)
    return coverage


if __name__ == '__main__':
    verify_sprite_pixels()
