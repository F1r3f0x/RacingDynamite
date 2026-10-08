# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Mem_Free: complete production mem.c versus authentic instructions, CRT modeled."""
import hashlib
import random
import struct
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX,
    UC_X86_REG_EIP, UC_X86_REG_ESP, UC_X86_REG_EFLAGS)
from verify_font_cleanup import SAVED, STACK, STOP
from build_decomp import build
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')
RVA, SIZE = 0x5b000, 127
DLL = BUILD / 'mem_free_validation.dll'
ARENA, ARENA_SIZE = 0x7600000, 0x100000
POINTER = 0x7800000
INPUTS = ['decomp/src/lisa3d.c', 'decomp/include/lisa3d.h', 'decomp/src/geputget.c', 'decomp/include/geputget.h', 'decomp/src/mem.c', 'decomp/include/mem.h', 'decomp/target.json',
          'tools/verify_mem_free.py', 'tools/build_decomp.py', 'tools/verify_matching.py',
          'tools/verify_font_cleanup.py', 'tools/windows_target.py', 'tools/windows_tracking.py']


def record(kind, outcome, cases=0, **details):
    record_run(RVA, kind, outcome, inputs=INPUTS,
        artifact=DLL.relative_to(ROOT).as_posix() if DLL.exists() else None,
        cases=cases, command=('uv run python tools/verify_mem_free.py' if __name__ == '__main__'
                             else 'uv run tools/verify_matching.py'), details=details)


def inspect_original(pe):
    assert hashlib.sha256(pe.get_data(RVA, SIZE)).hexdigest() == '40c455ee2388ee3160f714b07885f4cc9f3fb5401ab86cd2bea5da8749acd471'
    assert pe.get_data(RVA+SIZE, 1) == b'\xcc'
    instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(pe.get_data(RVA, SIZE), 0x45b000))
    assert len(instructions) == 51 and sum(i.size for i in instructions) == SIZE
    assert [(i.address, i.op_str) for i in instructions if i.mnemonic == 'call'] == [(0x45b062, '0x4693b0')]
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {a: (n, local, params, bits) for a, n, local, params, bits in
        struct.iter_unpack('<IIIHH', pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    assert fpo[RVA] == (SIZE, 1, 2, 0x141d)
    relocs = [(e.rva, struct.unpack('<I', pe.get_data(e.rva, 4))[0])
        for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries if e.type == 3 and RVA <= e.rva < RVA+SIZE]
    assert relocs == [(0x5b015, 0x63c6a0)]
    for rva, size, digest in [
        (0x5ad50, 35, '4dde71fdc49b5480369eec8adf5b12a9a74ad3aeb97921330f412e22acc3b627'),
        (0x5ad80, 132, '8beca914f2291a89e320f0ad75559b562a336f5a0c020f9553de979bba923743'),
        (0x5ae10, 406, 'b272bd31d844358c51a80948c668ed37a98108dceff066c13a67328c9a964ed3'),
        (0x5b080, 182, '5d94bbebd22cf4806b9abdee9a916457043581b24f1fe69b907c320596cb7e62'),
        (0x5b140, 41, 'caa0affb689af0a88ea17b0af2829cec7f498ea2613b8ede871badc5afff2444'),
        (0x693b0, 79, '0b3a6390807c1232be0e96fec6d2e97c96f0c27607a2cd6685e14467d6f0c1af')]:
        assert fpo[rva][0] == size
        assert hashlib.sha256(pe.get_data(rva, size)).hexdigest() == digest
    section = next(s for s in pe.sections if s.VirtualAddress <= 0x23c6a0 < s.VirtualAddress+s.Misc_VirtualSize)
    assert 0x23c6a0 >= section.VirtualAddress+section.SizeOfRawData
    assert 0x23caa0 <= section.VirtualAddress+section.Misc_VirtualSize
    assert any(x.address == 0x64c318 and x.name == b'HeapFree' for d in pe.DIRECTORY_ENTRY_IMPORT for x in d.imports)
    for va in [0x41fcbb, 0x4208bc, 0x4127ac, 0x45645a]:
        raw = pe.get_data(va-0x400000, 5)
        assert raw[0] == 0xe8 and va+5+struct.unpack_from('<i', raw, 1)[0] == 0x45b000


class PoolFixture:
    """Concrete valid hierarchy at identical addresses in both emulators."""
    def __init__(self, dense=False):
        self.data = bytearray(b'\xcc' * ARENA_SIZE)
        self.data[64:320] = b'\0' * 256
        self.data[0x2000:0x2140] = b'\0' * 320  # Other pool: empty, distinct root.
        self.cursor, self.pages, self.blocks = 0x4000, {}, {}
        if dense:
            for p in range(64):
                for b in range(64): self.block(p, b)

    def alloc(self, n):
        address = self.cursor; self.cursor += n
        assert self.cursor <= ARENA_SIZE
        return address

    def word(self, offset, value):
        struct.pack_into('<I', self.data, offset, value)

    def block(self, p, b):
        if p not in self.pages:
            address = self.alloc(256)
            self.data[address:address+256] = b'\0' * 256
            self.pages[p] = address; self.word(64+4*p, ARENA+address)
        if (p, b) not in self.blocks:
            address = self.alloc(128); self.blocks[p, b] = address
            self.word(self.pages[p]+4*b, ARENA+address)
            for i in range(16):
                self.word(address+8*i, 0x80000000+address+8*i)
                self.word(address+8*i+4, 0x100+i)
        return self.blocks[p, b]

    def match(self, p, b, r, pointer=POINTER, size=123):
        offset = self.block(p, b)+8*r
        self.word(offset, pointer); self.word(offset+4, size)
        return offset


def oracle(data, pool_id, pointer, mutations):
    reads = [('table', pool_id*4, 4)]
    def word(offset):
        reads.append(('arena', offset, 4))
        return struct.unpack_from('<I', data, offset)[0]
    for p in range(64):
        page = word(64+4*p)
        if page:
            for b in range(64):
                records = word(page-ARENA+4*b)
                if records:
                    for r in range(16):
                        offset = records-ARENA+8*r
                        if word(offset) == pointer:
                            final = bytearray(data)
                            for off, value in mutations: struct.pack_into('<I', final, off, value)
                            struct.pack_into('<I', final, offset+4, 0)
                            return 1, bytes(final), offset, reads
    return 0, bytes(data), None, reads


class PoolCPU:
    def __init__(self, pe, symbols=None):
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.size = (pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095
        self.cpu = Uc(UC_ARCH_X86, UC_MODE_32)
        self.cpu.mem_map(self.base, self.size); self.cpu.mem_write(self.base, pe.get_memory_mapped_image())
        self.cpu.mem_map(ARENA, ARENA_SIZE)
        self.cpu.mem_map(STACK, 0x10000); self.cpu.mem_map(STOP, 0x1000)
        self.entry = 0x45b000 if symbols is None else symbols['Mem_Free']
        self.free = 0x4693b0 if symbols is None else symbols['free']
        self.table = 0x63c6a0 if symbols is None else symbols['g_memPools']
        self.cpu.hook_add(UC_HOOK_CODE, self.hook)
        self.cpu.hook_add(UC_HOOK_MEM_READ, self.read)
        self.cpu.hook_add(UC_HOOK_MEM_WRITE, self.write)

    def read(self, cpu, access, address, size, value, unused):
        if self.table <= address < self.table+1024:
            self.reads.append(('table', address-self.table, size))
        elif ARENA <= address < ARENA+ARENA_SIZE:
            self.reads.append(('arena', address-ARENA, size))

    def write(self, cpu, access, address, size, value, unused):
        if not STACK <= address < STACK+0x10000:
            self.events.append(('write', address, size, value))

    def hook(self, cpu, address, length, unused):
        if address != self.free: return
        sp = cpu.reg_read(UC_X86_REG_ESP)
        args = struct.unpack('<I', cpu.mem_read(sp+4, 4))
        assert args == (self.pointer,)
        assert bytes(cpu.mem_read(ARENA, ARENA_SIZE)) == self.before_data
        assert self.reads == self.expected_reads
        self.events.append(('free', self.pointer))
        for offset, value in self.mutations:
            cpu.mem_write(ARENA+offset, struct.pack('<I', value))
        cpu.reg_write(UC_X86_REG_EAX, 0xdeadbeef)
        cpu.reg_write(UC_X86_REG_ECX, 0xc1c1c1c1); cpu.reg_write(UC_X86_REG_EDX, 0xd2d2d2d2)
        cpu.reg_write(UC_X86_REG_EFLAGS, 0x43)
        cpu.reg_write(UC_X86_REG_EIP, struct.unpack('<I', cpu.mem_read(sp, 4))[0])
        cpu.reg_write(UC_X86_REG_ESP, sp+4)

    def invoke(self, data, pool_id, pointer, mutations=(), seed=0):
        expected, final, match, reads = oracle(data, pool_id, pointer, mutations)
        self.before_data, self.pointer, self.mutations = bytes(data), pointer, mutations
        self.expected_reads, self.reads, self.events = reads, [], []
        cpu = self.cpu; cpu.mem_write(ARENA, bytes(data))
        table = [ARENA+0x2000]*256; table[pool_id] = ARENA
        cpu.mem_write(self.table, struct.pack('<256I', *table))
        sp = STACK+0x8000
        cpu.mem_write(sp, struct.pack('<III', STOP, pool_id, pointer))
        stack = bytes(cpu.mem_read(sp, 0x8000))
        rng = random.Random(seed); saved = [rng.getrandbits(32) for _ in SAVED]
        for reg, value in zip(SAVED, saved): cpu.reg_write(reg, value)
        cpu.reg_write(UC_X86_REG_ESP, sp); cpu.reg_write(UC_X86_REG_EFLAGS, 2)
        before = bytes(cpu.mem_read(self.base, self.size))
        cpu.emu_start(self.entry, STOP, count=5000000)
        assert cpu.reg_read(UC_X86_REG_EIP) == STOP and cpu.reg_read(UC_X86_REG_ESP) == sp+4
        assert [cpu.reg_read(r) for r in SAVED] == saved
        assert not cpu.reg_read(UC_X86_REG_EFLAGS)&0x400
        assert bytes(cpu.mem_read(sp, 0x8000)) == stack
        assert bytes(cpu.mem_read(self.base, self.size)) == before
        assert self.reads == reads
        expected_events = [] if match is None else [('free', pointer), ('write', ARENA+match+4, 4, 0)]
        assert self.events == expected_events, self.events
        actual = cpu.reg_read(UC_X86_REG_EAX), bytes(cpu.mem_read(ARENA, ARENA_SIZE)), self.events
        assert actual == (expected, final, expected_events)
        return actual


def verify_mem_free():
    verify_target(); phase = 'compilation'
    try:
        original = pefile.PE(str(TARGET)); inspect_original(original)
        build(dll=DLL)
        record('compilation', 'pass', scope='Complete production mem.c and layout checks; explicit nonreturning CRT link boundary; focused DLL')
        rebuilt = pefile.PE(str(DLL))
        symbols = {s.name.decode():rebuilt.OPTIONAL_HEADER.ImageBase+s.address for s in rebuilt.DIRECTORY_ENTRY_EXPORT.symbols if s.name}
        prefix = rebuilt.get_data(symbols['Mem_Free']-rebuilt.OPTIONAL_HEADER.ImageBase, SIZE)
        record('raw_bytes', 'pass' if prefix == original.get_data(RVA, SIZE) else 'different', scope='Raw prefix diagnostic; instruction equality unverified')
        phase = 'emulation'; cases = 0; rng = random.Random(RVA)
        a, b = PoolCPU(original), PoolCPU(rebuilt, symbols)
        def compare(fixture, pool=0, pointer=POINTER, mutations=(), repeat=False):
            nonlocal cases
            args = (bytes(fixture.data), pool, pointer, mutations, cases)
            ra, rb = a.invoke(*args), b.invoke(*args)
            assert ra == rb
            cases += 1
            if repeat:
                assert a.invoke(ra[1], pool, pointer, seed=cases) == b.invoke(rb[1], pool, pointer, seed=cases)
                cases += 1
        for pool in range(256):
            f = PoolFixture(); f.match(pool%64, (pool//4)%64, pool%16, size=rng.getrandbits(32))
            compare(f, pool=pool)
        for index in range(64):
            for p, q, r in [(index,63,15),(63,index,15)]:
                f=PoolFixture(); f.match(p,q,r,size=0); compare(f)
        for index in range(16):
            f=PoolFixture(); f.match(63,63,index); compare(f)
        for pointer in [0,1,0x80000000,0xffffffff]:
            for size in [0,1,0x80000000,0xffffffff]:
                f=PoolFixture(); f.match(0,0,0,pointer,size); f.match(63,63,15,pointer,99)
                compare(f,pointer=pointer,repeat=True)
        for dense in [False,True]:
            f=PoolFixture(dense); compare(f)
            if dense:
                f.match(63,63,15); compare(f)
        for _ in range(80):
            f=PoolFixture()
            for n in range(rng.randrange(1,20)):
                f.block(rng.randrange(64),rng.randrange(64))
            f.match(rng.randrange(64),rng.randrange(64),rng.randrange(16),size=rng.getrandbits(32))
            compare(f,pool=rng.randrange(256))
        for p,q,r in [(0,0,0),(0,63,15),(63,0,0),(63,63,15)]:
            f=PoolFixture(); offset=f.match(p,q,r)
            # Change cached record pointer/size and detach the hierarchy at free.
            mutations=[(offset,0x12345678),(offset+4,0xffffffff),(64+4*p,0)]
            compare(f,mutations=mutations)
        record('emulation','pass',cases=cases,
            scope='Complete hierarchy/image/caller state; exact ordered reads and writes; CRT boundary snapshot/mutation; first match, repeat/null/stale pointers and ABI',
            limitations='CRT free modeled; valid pools/readable nonaliasing hierarchy only; native heap, invalid pool IDs, concurrency/reentry, instruction/linked equality and native game parity unverified')
        print(f'PASS: {cases} Mem_Free differential executions; hierarchy, ordered reads/writes, CRT boundary and ABI equality.')
    except Exception as exc:
        record(phase,'fail',error=str(exc)); raise


if __name__ == '__main__':
    verify_mem_free()
