# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Bounded original-instruction versus extracted production C font-width validation.

Invoked by verify_matching.py in its pinned pefile/capstone/Unicorn environment.
No copy of the reconstruction is maintained in this harness.
"""
import hashlib
import random
import shutil
import struct
import subprocess
from fractions import Fraction

import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_WRITE
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ESI,
    UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_ESP, UC_X86_REG_EIP,
    UC_X86_REG_EFLAGS, UC_X86_REG_FPCW)

from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run

RVA = 0x564d0
FONT_DLL = BUILD / 'font_width_validation.dll'
INPUTS = ['decomp/src/geputget.c', 'decomp/include/geputget.h',
          'decomp/include/mem.h', 'decomp/target.json', 'tools/verify_font_width.py',
          'tools/verify_matching.py', 'tools/windows_target.py', 'tools/windows_tracking.py']
SAVED = [UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP]
STACK, STOP, TEXT = 0x7000000, 0x7100000, 0x7200000


def record(kind, outcome, cases=0, **details):
    record_run(RVA, kind, outcome, inputs=INPUTS,
               artifact=FONT_DLL.relative_to(ROOT).as_posix() if FONT_DLL.exists() else None,
               cases=cases, command=('uv run tools/verify_font_width.py' if __name__ == '__main__'
                                     else 'uv run tools/verify_matching.py'), details=details)


def inspect_original(pe):
    code = pe.get_data(RVA, 397)
    assert hashlib.sha256(code).hexdigest() == 'f69343cc05e2d5113fe34847fd7ab817a81c430ebe0b59cd7320ca7193dbf066'
    assert pe.get_data(RVA+397, 3) == b'\xcc'*3
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {a: (n, local, params, bits) for a, n, local, params, bits in
           struct.iter_unpack('<IIIHH', pe.__data__[debug.PointerToRawData:
                                                     debug.PointerToRawData+debug.SizeOfData])}
    assert fpo[RVA] == (397, 2, 2, 5129)
    expected = [(0x564ee, 0x63f2e0), (0x564f6, 0x4bab34), (0x5650a, 0x63f2e8),
        (0x56521, 0x63f2fa), (0x56532, 0x47aec8), (0x5653d, 0x63f2ec),
        (0x5656b, 0x63f2de), (0x56575, 0x63f2fa), (0x5657b, 0x63f2f0),
        (0x5658f, 0x63f2fa), (0x56595, 0x63f2f0), (0x565ed, 0x63f2de),
        (0x56604, 0x63f720), (0x5660a, 0x63f2f0), (0x56617, 0x63f2fa),
        (0x56628, 0x47aec8), (0x5662e, 0x47aed0)]
    actual = [(e.rva, struct.unpack('<I', pe.get_data(e.rva, 4))[0])
              for block in pe.DIRECTORY_ENTRY_BASERELOC for e in block.entries
              if e.type == 3 and RVA <= e.rva < RVA+397]
    assert actual == expected
    assert pe.get_data(0x7aec8, 16) == struct.pack('<dd', 0.35, 1.0/256)
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = list(md.disasm(code, 0x4564d0))
    assert sum(i.size for i in instructions) == 397
    assert [(i.address, i.op_str) for i in instructions if i.mnemonic == 'call'] == [
        (0x456536, '0x46950c'), (0x456632, '0x46950c')]
    for site in (0x402b3e, 0x409ac1, 0x40a8ce, 0x40a9ff, 0x40b000, 0x40b027,
                 0x40b082, 0x4187c3, 0x4188dd, 0x418ab4):
        instruction = next(md.disasm(pe.get_data(site-0x400000, 5), site))
        assert instruction.mnemonic == 'call' and instruction.op_str == '0x4564d0'
    # Execute the real CRT conversion helper, including control-word restore.
    helper = list(md.disasm(pe.get_data(0x6950c, 64), 0x46950c))
    end = next(i.address+i.size for i in helper if i.mnemonic == 'ret')
    assert end == 0x469533
    assert not any(i.mnemonic == 'call' for i in helper if i.address < end)
    (BUILD/'font_width_original.txt').write_text('\n'.join(
        f'{i.address:#010x}: {i.mnemonic} {i.op_str}' for i in instructions+[
            i for i in helper if i.address < end])+'\n', encoding='utf-8')


def build_font():
    BUILD.mkdir(parents=True, exist_ok=True)
    source = (ROOT/'decomp/src/geputget.c').read_text(encoding='utf-8-sig')
    start = source.index('int Font_GetTextWidth(')
    opening = source.index('{', start)
    depth = 1
    end = opening+1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    # Declaration and layout come from the production header. strlen is a C
    # harness dependency with ordinary null-terminated, immutable-string semantics.
    unit = ('#include "geputget.h"\n'
            'FontSlot g_fonts[MAX_FONTS];\nint g_fileErrorLine;\n'
            'int _fltused = 0; /* MSVC ABI floating-point linker marker, not a helper. */\n'
            'typedef char slot_size_check[sizeof(FontSlot) == 1600 ? 1 : -1];\n'
            'typedef char glyph_offset_check[offsetof(FontSlot, glyph_present) == 30 ? 1 : -1];\n'
            'typedef char width_offset_check[offsetof(FontSlot, widths) == 1152 ? 1 : -1];\n'
            'size_t strlen(const char *s) { const char *p = s; while (*p) p++; return (size_t)(p-s); }\n'
            + source[start:end]+'\n')
    path, obj = BUILD/'font_width_unit.c', BUILD/'font_width.obj'
    path.write_text(unit, encoding='utf-8')
    for product in [obj, FONT_DLL, FONT_DLL.with_suffix('.lib'), FONT_DLL.with_suffix('.exp')]:
        product.unlink(missing_ok=True)
    cc, ld = shutil.which('clang'), shutil.which('lld-link')
    if not cc or not ld:
        raise RuntimeError('Clang/LLD required for bounded font validation')
    commands = [[cc, '--target=i686-pc-windows-msvc', '-std=c89', '-pedantic-errors',
        '-Wall', '-Wextra', '-Werror', '-O2', '-ffreestanding', '-fno-builtin',
        '-fno-vectorize', '-fno-slp-vectorize', '-mno-sse', '-mno-sse2',
        '-I', str(ROOT/'decomp/include'), '-c', str(path), '-o', str(obj)],
        [ld, '/dll', '/noentry', '/nodefaultlib', '/machine:x86', '/base:0x10000000',
         '/out:'+str(FONT_DLL), str(obj), '/export:Font_GetTextWidth',
         '/export:g_fonts', '/export:g_fileErrorLine']]
    for command in commands:
        print(subprocess.list2cmdline(command), flush=True)
        subprocess.run(command, cwd=ROOT, check=True)
    assert obj.stat().st_size and FONT_DLL.stat().st_size
    return commands


def execute(pe, entry, fonts, error, state, text, slot, seed, original=False, control_word=0x37f):
    base = pe.OPTIONAL_HEADER.ImageBase
    size = (pe.OPTIONAL_HEADER.SizeOfImage+4095) & ~4095
    cpu = Uc(UC_ARCH_X86, UC_MODE_32)
    cpu.mem_map(base, size)
    cpu.mem_write(base, pe.get_memory_mapped_image())
    cpu.mem_write(fonts, bytes(state))
    cpu.mem_write(error, struct.pack('<I', 0x12345678))
    for address in (STACK, STOP, TEXT):
        cpu.mem_map(address, 0x10000)
    cpu.mem_write(TEXT, text+b'\0')
    sp = STACK+0x8000
    cpu.mem_write(sp, struct.pack('<III', STOP, TEXT, slot & 0xffffffff))
    stack_before = bytes(cpu.mem_read(sp, 0x8000))
    rng = random.Random(seed)
    registers = [rng.getrandbits(32) for _ in SAVED]
    for reg, value in zip(SAVED, registers):
        cpu.reg_write(reg, value)
    cpu.reg_write(UC_X86_REG_ESP, sp)
    cpu.reg_write(UC_X86_REG_EFLAGS, 2)
    cpu.reg_write(UC_X86_REG_FPCW, control_word)
    before = bytes(cpu.mem_read(base, size))
    section = next(s for s in pe.sections if s.Name.rstrip(b'\0') == b'.text')
    bounds = (base+section.VirtualAddress, base+section.VirtualAddress+section.Misc_VirtualSize)
    writes = []
    def code_hook(uc, address, length, unused):
        if original:
            assert 0x4564d0 <= address < 0x45665d or 0x46950c <= address < 0x469533
        else:
            assert bounds[0] <= address < bounds[1]
    def write_hook(uc, access, address, length, value, unused):
        if STACK <= address and address+length <= STACK+0x10000:
            return
        assert address == error and length == 4, (hex(address), length)
        writes.append(value & 0xffffffff)
    cpu.hook_add(UC_HOOK_CODE, code_hook)
    cpu.hook_add(UC_HOOK_MEM_WRITE, write_hook)
    cpu.emu_start(entry, STOP, count=1000000)
    assert cpu.reg_read(UC_X86_REG_EIP) == STOP
    assert cpu.reg_read(UC_X86_REG_ESP) == sp+4
    assert [cpu.reg_read(r) for r in SAVED] == registers
    assert bytes(cpu.mem_read(sp, 0x8000)) == stack_before
    assert not cpu.reg_read(UC_X86_REG_EFLAGS) & 0x400
    assert cpu.reg_read(UC_X86_REG_FPCW) == control_word
    after = bytearray(cpu.mem_read(base, size))
    error_value = struct.unpack('<I', after[error-base:error-base+4])[0]
    after[error-base:error-base+4] = before[error-base:error-base+4]
    assert after == before, 'Unexpected global/table change'
    assert bytes(cpu.mem_read(TEXT, len(text)+1)) == text+b'\0'
    return cpu.reg_read(UC_X86_REG_EAX), error_value, writes


def verify_font_width():
    verify_target()
    phase = 'compilation'
    try:
        commands = build_font()
        phase = 'emulation'
        original, rebuilt = pefile.PE(str(TARGET)), pefile.PE(str(FONT_DLL))
        inspect_original(original)
        assert not hasattr(rebuilt, 'DIRECTORY_ENTRY_IMPORT')
        symbols = {e.name.decode(): rebuilt.OPTIONAL_HEADER.ImageBase+e.address
                   for e in rebuilt.DIRECTORY_ENTRY_EXPORT.symbols if e.name}
        details = {'scope': 'extracted production Font_GetTextWidth only; real original _ftol; C strlen harness',
                   'commands': commands, 'toolchain': {
                       name: subprocess.check_output([shutil.which(name), '--version'], text=True).splitlines()[0]
                       for name in ('clang', 'lld-link')},
                   'limitations': 'Full geputget.c and native game blocked by legacy dependencies; instruction equality unclaimed'}
        record('compilation', 'pass', **details)
        cases = 0
        def compare(slot, state, text, expected=None, control_word=0x37f):
            nonlocal cases
            a = execute(original, 0x4564d0, 0x63f2e0, 0x4bab34,
                        state, text, slot, cases, True, control_word)
            b = execute(rebuilt, symbols['Font_GetTextWidth'], symbols['g_fonts'],
                        symbols['g_fileErrorLine'], state, text, slot, cases, control_word=control_word)
            assert a == b, (slot, text, a, b)
            if expected is not None:
                assert a[0] == expected & 0xffffffff, (text, a, expected)
            in_use = struct.unpack_from('<I', state, slot*1600)[0]
            assert a[1:] == ((0x12345678, []) if in_use else (1040, [1040]))
            cases += 1
        def fixture(slot, height=20, spacing=2, proportional=0, flag=0, in_use=1):
            state = bytearray(48000)
            offset = slot*1600
            struct.pack_into('<I', state, offset, in_use)
            struct.pack_into('<IIi', state, offset+8, flag, proportional, spacing)
            struct.pack_into('<H', state, offset+26, height & 0xffff)
            for c in range(32, 128):
                state[offset+30+c-32] = 1
                struct.pack_into('<h', state, offset+1152+(c-32)*2, c-53)
            return state
        # Independently asserted normal arithmetic and skip precedence.
        for slot in range(30):
            for proportional in (0, 1, 2, 0xffffffff):
                state = fixture(slot, proportional=proportional)
                compare(slot, state, b'AB', 29 if proportional else 44)
                state[slot*1600+30] = 0
                compare(slot, state, b'A B', 35 if proportional else 66)
                state[slot*1600+30+66-32] = 2
                compare(slot, state, b'AB', 14 if proportional else 22)
                compare(slot, state, b'', 0)
        for height in (-32768, -1, 0, 1, 2, 3, 20, 32767):
            for proportional in (0, 1):
                state = fixture(0, height=height, proportional=proportional)
                state[30] = 0
                compare(0, state, b' ', int(height*Fraction.from_float(0.35)) if proportional else height+2)
        for slot in (0, 1, 29):
            for in_use in (0, 1, 2, 0xffffffff):
                for flag in (1, 2, 0xffffffff):
                    compare(slot, fixture(slot, flag=flag, in_use=in_use), b'', 2)
        # Rounding depends on the caller's x87 precision control. Binary64 0.35
        # is below 7/20; extended precision gives 6 for height 20, double gives 7.
        for control_word in (0x27f, 0x37f):
            for height in (-32768, -20, -1, 0, 1, 2, 20, 32767):
                state = fixture(0, height=height, proportional=1)
                state[30] = 0
                expected = int(height*0.35) if control_word == 0x27f else int(height*Fraction.from_float(0.35))
                compare(0, state, b' ', expected, control_word)
        rng = random.Random(0x4564d0)
        for index in range(160):
            slot = rng.randrange(1, 30)
            state = fixture(slot, height=rng.randrange(-32768, 32768),
                            spacing=rng.choice([-2147483648, -1, 0, 2147483647]),
                            proportional=rng.randrange(3), in_use=rng.randrange(3))
            header = bytes(state[slot*1600:slot*1600+30])
            # All signed character offsets stay inside the containing g_fonts table.
            for c in range(-128, 128):
                if c:
                    state[slot*1600+30+c-32] = rng.choice([0, 1, 2, 255])
            for c in range(-128, 128):
                struct.pack_into('<h', state, slot*1600+1152+(c-32)*2,
                                 rng.randrange(-32768, 32768))
            # Low character presence addresses alias header fields. Keep those
            # fields intentional so mixed-byte cases exercise the measurement path.
            state[slot*1600:slot*1600+30] = header
            text = bytes(range(1, 256)) if index == 0 else bytes(rng.randrange(1, 256) for _ in range(48))
            compare(slot, state, text)
        # Outside the C table contract: observe original continuation after errors.
        original_only = 0
        for slot in (-1, 30):
            base = 0x63f2e0+slot*1600
            state = fixture(0, flag=1, in_use=0)
            result = execute(original, 0x4564d0, base, 0x4bab34,
                             state[:1600], b'', slot, 999, True)
            assert result == (2, 1040, [1040])
            original_only += 1
        record('emulation', 'pass', cases, **details, original_only_cases=original_only,
               checks='EAX, error writes, whole image/text immutability, stack, saved registers, DF, x87 control word')
        print(f'PASS: Font_GetTextWidth {cases} original-vs-C cases; {original_only} original-only invalid slots.')
    except Exception as exc:
        record(phase, 'fail', error=str(exc))
        raise


if __name__ == '__main__':
    if not __debug__:
        raise RuntimeError('Verification requires assertions')
    verify_font_width()
