# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Execute original Windows instructions versus compiled C in x86 emulation.

Instruction equality and native game runtime are reported separately.
"""
import hashlib
import random
import struct
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_WRITE
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ESI,
    UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_ESP, UC_X86_REG_EFLAGS,
    UC_X86_REG_EIP)
from build_decomp import build
from windows_target import TARGET, DLL, ROUTINE_SHA256, verify_target

if not __debug__:
    raise RuntimeError('Verification requires assertions; disable -O/PYTHONOPTIMIZE')

FIELDS = [('g_memHandlesInitialized', 0x4bab38, 4),
          ('g_memHandleStatus', 0x5116e0, 800),
          ('g_memHandleIds', 0x511230, 400),
          ('g_memHandleCursor', 0x512040, 2)]
STACK, STOP = 0x7000000, 0x7100000
PRESERVED = [UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP]

def execute(pe, entry, fields, values, seed, extent):
    base = pe.OPTIONAL_HEADER.ImageBase
    size = (pe.OPTIONAL_HEADER.SizeOfImage + 4095) & ~4095
    uc = Uc(UC_ARCH_X86, UC_MODE_32)
    uc.mem_map(base, size)
    uc.mem_write(base, pe.get_memory_mapped_image())
    # Surrounding writable bytes expose overruns; neither leaf uses import slots.
    for section in pe.sections:
        if section.Characteristics & 0x80000000:
            uc.mem_write(base+section.VirtualAddress,
                         bytes([0xa5]) * section.Misc_VirtualSize)
    for (_, addr, length), value in zip(fields, values):
        assert len(value) == length
        uc.mem_write(addr, value)
    uc.mem_map(STACK, 0x10000)
    uc.mem_map(STOP, 0x1000)
    sp = STACK + 0x8000
    uc.mem_write(sp, struct.pack('<I', STOP))
    rng = random.Random(seed)
    registers = [rng.getrandbits(32) for _ in PRESERVED]
    for reg, value in zip(PRESERVED, registers):
        uc.reg_write(reg, value)
    uc.reg_write(UC_X86_REG_ESP, sp)
    uc.reg_write(UC_X86_REG_EFLAGS, 2)  # Windows x86 ABI: direction flag clear.
    before = bytes(uc.mem_read(base, size))
    writes = []
    def code_hook(cpu, address, length, unused):
        if not (extent[0] <= address < extent[1]):
            raise AssertionError('Execution escaped bounded routine')
    def write_hook(cpu, access, address, length, value, unused):
        if STACK <= address and address+length <= STACK+0x10000:
            return
        if not any(addr <= address and address+length <= addr+n for _,addr,n in fields):
            raise AssertionError(f'Unexpected write at {address:#x}, length {length}')
        writes.append((address, length, value))
    uc.hook_add(UC_HOOK_CODE, code_hook)
    uc.hook_add(UC_HOOK_MEM_WRITE, write_hook)
    uc.emu_start(entry, STOP, count=10000)
    assert uc.reg_read(UC_X86_REG_EIP) == STOP, 'Did not return within instruction limit'
    assert uc.reg_read(UC_X86_REG_EAX) == 1
    assert uc.reg_read(UC_X86_REG_ESP) == sp+4
    assert [uc.reg_read(r) for r in PRESERVED] == registers
    assert not (uc.reg_read(UC_X86_REG_EFLAGS) & 0x400)
    after = bytearray(uc.mem_read(base, size))
    result = [bytes(uc.mem_read(addr,n)) for _,addr,n in fields]
    for _,addr,n in fields:
        after[addr-base:addr-base+n] = before[addr-base:addr-base+n]
    assert bytes(after) == before, 'Changed unrelated image memory'
    return result, writes

def verify():
    verify_target()
    build()  # Always fresh; no --skip-build and no stale DOS inputs.
    original, rebuilt = pefile.PE(str(TARGET)), pefile.PE(str(DLL))
    assert rebuilt.FILE_HEADER.Machine == 0x14c
    assert not hasattr(rebuilt, 'DIRECTORY_ENTRY_IMPORT'), 'Unexpected dependencies'
    symbols = {e.name.decode(): rebuilt.OPTIONAL_HEADER.ImageBase+e.address
               for e in rebuilt.DIRECTORY_ENTRY_EXPORT.symbols if e.name}
    fields = [(name, symbols[name], n) for name,_,n in FIELDS]
    original_code = original.get_data(0x5b1f0, 76)
    assert hashlib.sha256(original_code).hexdigest() == ROUTINE_SHA256
    text = next(s for s in rebuilt.sections if s.Name.rstrip(b'\0') == b'.text')
    lo = rebuilt.OPTIONAL_HEADER.ImageBase+text.VirtualAddress
    extent = (lo, lo+text.Misc_VirtualSize)
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = list(md.disasm(original_code, 0x45b1f0))
    assert sum(i.size for i in instructions) == 76
    generated_code = text.get_data()[:text.Misc_VirtualSize]
    generated = list(md.disasm(generated_code, lo))
    assert not any(i.mnemonic == 'call' for i in generated), 'Helper is no longer a leaf'
    print(f'Original: 76 bytes, {len(instructions)} instructions; compiled .text: {text.Misc_VirtualSize} bytes.')
    print(f'Raw code-byte equality: {original_code == generated_code}; relocation-aware equality not evaluated.')
    print('Instruction equality: not claimed (modern provisional Clang code generation).')
    rng = random.Random(0x45b1f0)
    cases = 0
    for flag in [0, 1, 2, 0xffffffff, 0x80000000]:
        for pattern in [0, 0xff, None]:
            def data(n):
                return bytes([pattern])*n if pattern is not None else rng.randbytes(n)
            values = [struct.pack('<I',flag), data(800), data(400), data(2)]
            for repeat in range(2):
                old = list(values)
                values, owrites = execute(original, 0x45b1f0, FIELDS, old, cases,
                                         (0x45b1f0,0x45b23c))
                actual, cwrites = execute(rebuilt, symbols['Mem_InitHandles'], fields,
                                         old, cases, extent)
                assert actual == values, f'Original/C divergence: flag={flag:#x}, pattern={pattern}, repeat={repeat}'
                if struct.unpack('<I',old[0])[0] == 1:
                    assert values == old and not owrites and not cwrites
                else:
                    # Derived from REP STOSD count and 16-bit stores in original assembly.
                    assert values == [struct.pack('<I',1), bytes(800),
                                      struct.pack('<200H',*range(1,201)), bytes(2)]
                    assert owrites[0] == (0x4bab38,4,1)
                    assert owrites[-1] == (0x512040,2,0)
                cases += 1
    print(f'PASS: {cases} original-vs-C executions; full state, boundaries, skip/repeat, writes and ABI checked.')
    print('Native DLL/game execution and startup/gameplay parity: unverified by this harness.')

if __name__ == '__main__':
    verify()
