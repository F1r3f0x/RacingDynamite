# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Font_Parse: authentic PE versus extracted production C, modeled sprite boundary."""
import hashlib
import random
import shutil
import struct
import subprocess
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ESP, UC_X86_REG_EIP,
                              UC_X86_REG_EFLAGS)
from verify_font_cleanup import FontLifecycleCPU, LIFECYCLE_FIELDS, SAVED, STACK, STOP
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')
RVA, SIZE = 0x56270, 429
DLL = BUILD / 'font_parse_validation.dll'
BUFFER = 0x7400000
INPUTS = ['decomp/src/geputget.c', 'decomp/include/geputget.h', 'decomp/include/mem.h',
          'decomp/src/mem.c', 'decomp/target.json', 'tools/verify_font_parse.py',
          'tools/verify_font_cleanup.py', 'tools/verify_matching.py',
          'tools/windows_target.py', 'tools/windows_tracking.py']


def record(kind, outcome, cases=0, **details):
    record_run(RVA, kind, outcome, inputs=INPUTS,
               artifact=DLL.relative_to(ROOT).as_posix() if DLL.exists() else None,
               cases=cases, command=('uv run python tools/verify_font_parse.py' if __name__ == '__main__'
                        else 'uv run tools/verify_matching.py'), details=details)


def inspect_original(pe):
    code = pe.get_data(RVA, SIZE)
    assert hashlib.sha256(code).hexdigest() == 'cb425727822dfce52643cd3c6d418c7bb2495aed08be4ae0667f54ad6a67f549'
    assert pe.get_data(RVA + SIZE, 3) == b'\xcc' * 3
    assert pe.get_data(0xba6d4, 4) == b'LFT\0'
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {a: (n, local, params, bits) for a, n, local, params, bits in
           struct.iter_unpack('<IIIHH', pe.__data__[debug.PointerToRawData:
                                                     debug.PointerToRawData + debug.SizeOfData])}
    assert fpo[RVA] == (SIZE, 11, 2, 0x140e)
    instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(code, 0x456270))
    assert sum(i.size for i in instructions) == SIZE
    assert [(i.address, i.op_str) for i in instructions if i.mnemonic == 'call'] == [
        (0x456280, '0x456180'), (0x4563e2, '0x456d40')]
    relocs = [(e.rva, struct.unpack('<I', pe.get_data(e.rva, 4))[0])
              for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries
              if e.type == 3 and RVA <= e.rva < RVA + SIZE]
    assert relocs == [(0x56275, 0x4ba6c4), (0x56286, 0x4ba6d4),
        (0x5629f, 0x4bab34), (0x562c1, 0x4bab34), (0x562d3, 0x63f2e0),
        (0x562e3, 0x64ae60), (0x562f6, 0x4bab34), (0x56321, 0x63f2f8),
        (0x56334, 0x63f2fa), (0x5633e, 0x63f760), (0x56345, 0x63f2fc),
        (0x5636e, 0x63f3e0), (0x56389, 0x63f2fe), (0x563a0, 0x63f2fe),
        (0x5640f, 0x63f2e0)]


def build_parser(dll=DLL, include_loader=False):
    source = (ROOT / 'decomp/src/geputget.c').read_text(encoding='utf-8-sig')
    def extract(name):
        start = source.index('int ' + name + '(')
        opening = source.index('{', start)
        depth, end = 1, opening + 1
        while depth:
            depth += (source[end] == '{') - (source[end] == '}')
            end += 1
        return source[start:end]
    context = next(line for line in source.splitlines()
                   if line.startswith('static const char s_fontExitContext[]'))
    unit = '#include "geputget.h"\n' + (ROOT / 'decomp/src/mem.c').read_text(encoding='utf-8-sig')
    unit += '''
FontSlot g_fonts[MAX_FONTS];
int g_fontSystemInitialized, g_fontSubsystemHandle, g_fileErrorLine;
typedef char slot_size[sizeof(FontSlot) == 1600 ? 1 : -1];
typedef char width_offset[offsetof(FontSlot, widths) == 1152 ? 1 : -1];
typedef char presence_offset[offsetof(FontSlot, glyph_present) == 30 ? 1 : -1];
typedef char handle_offset[offsetof(FontSlot, glyph_handles) == 256 ? 1 : -1];
typedef char desc_size[sizeof(SpriteDesc) == 32 ? 1 : -1];
typedef char desc_width[offsetof(SpriteDesc, width) == 4 ? 1 : -1];
typedef char desc_pixels[offsetof(SpriteDesc, pixels) == 16 ? 1 : -1];
/* Ordinary immutable byte comparison; original parser uses inline REPE CMPSB. */
int memcmp(const void *a, const void *b, size_t n) {
    const unsigned char *x = a, *y = b;
    size_t i;
    for (i = 0; i < n; i++) if (x[i] != y[i]) return x[i] - y[i];
    return 0;
}
/* Execution intercepts this boundary and supplies explicit fixture handles. */
extern void *Gfx_SpriteOp(void *desc, int op);
'''
    unit += context + '\nconst char * const validation_fontExitContext = s_fontExitContext;\n'
    names = ['Font_Unload', 'Font_Shutdown', 'Font_InitSystem', 'Font_Parse']
    unit += '\n'.join(extract(n) for n in names)
    stem = dll.stem.replace('_validation', '')
    path, obj = BUILD / (stem + '_unit.c'), BUILD / (stem + '.obj')
    path.write_text(unit + '\n', encoding='utf-8')
    for p in [obj, dll, dll.with_suffix('.lib'), dll.with_suffix('.exp')]:
        p.unlink(missing_ok=True)
    stub = BUILD / (stem + '_boundary_stub.c')
    stub_obj = BUILD / (stem + '_boundary_stub.obj')
    stubs = 'void *Gfx_SpriteOp(void *desc, int op) { (void)desc; (void)op; return 0; }\n'
    if include_loader:
        # Deliberately cannot complete without the explicit CPU boundary model.
        stubs += 'void *File_LoadToMemory(const char *filename) { (void)filename; for (;;) {} }\n'
    stubs += 'void free(void *p) { (void)p; for (;;) {} }\nvoid *malloc(unsigned int n) { (void)n; for (;;) {} }\n'
    stub.write_text(stubs, encoding='utf-8')
    stub_obj.unlink(missing_ok=True)
    cc, ld = shutil.which('clang'), shutil.which('lld-link')
    if not cc or not ld:
        raise RuntimeError('Clang/LLD required')
    exports = [n for n, _, _ in LIFECYCLE_FIELDS] + ['Font_Parse', 'Font_InitSystem',
        'Font_Shutdown', 'Font_Unload', 'Mem_NextHandleId', 'Mem_RegisterHandle',
        'Mem_ReleaseHandleId', 'Mem_InitHandles', 'Gfx_SpriteOp',
        'validation_fontExitContext', 'g_fileErrorLine', 'g_memPools', 'Mem_Free', 'free']
    if include_loader:
        exports += ['Font_Load', 'File_LoadToMemory']
    commands = [[cc, '--target=i686-pc-windows-msvc', '-std=c89', '-pedantic-errors',
        '-Wall', '-Wextra', '-Werror', '-O2', '-ffreestanding', '-fno-builtin',
        '-fno-inline', '-mno-sse', '-mno-sse2', '-I', str(ROOT / 'decomp/include'),
        '-c', str(path), '-o', str(obj)],
        [ld, '/dll', '/noentry', '/nodefaultlib', '/machine:x86', '/base:0x10000000',
         '/out:' + str(dll), str(obj), str(stub_obj)] + ['/export:' + n for n in exports]]
    commands.insert(1, [cc, '--target=i686-pc-windows-msvc', '-std=c89', '-pedantic-errors',
        '-Wall', '-Wextra', '-Werror', '-O2', '-c', str(stub), '-o', str(stub_obj)])
    if include_loader:
        # Separate translation unit prevents dead-argument optimization of unused.
        loader_path, loader_obj = BUILD / (stem + '_wrapper.c'), BUILD / (stem + '_wrapper.obj')
        loader_path.write_text('#include \"geputget.h\"\nextern int g_fileErrorLine;\n'
            'extern void *File_LoadToMemory(const char *filename);\n'
            'extern int Mem_Free(int pool, void *buffer);\n' + extract('Font_Load') + '\n', encoding='utf-8')
        loader_obj.unlink(missing_ok=True)
        commands.insert(1, commands[0][:-4] + ['-c', str(loader_path), '-o', str(loader_obj)])
        commands[-1].append(str(loader_obj))
    for command in commands:
        subprocess.run(command, check=True)
    return commands


class ParserCPU(FontLifecycleCPU):
    def __init__(self, pe, symbols=None):
        super().__init__(pe, symbols)
        self.entries['Font_Parse'] = 0x456270 if self.original else symbols['Font_Parse']
        self.fields.append(('g_fileErrorLine', 0x4bab34 if self.original else symbols['g_fileErrorLine'], 4))
        self.cpu.mem_map(BUFFER, 0x10000)

    def hook(self, cpu, address, length, unused):
        if address != self.entries['Gfx_SpriteOp']:
            if address == self.entries['Font_InitSystem']:
                self.calls.append(('Font_InitSystem', ()))
            else:
                super().hook(cpu, address, length, unused)
            return
        sp = cpu.reg_read(UC_X86_REG_ESP)
        desc, op = struct.unpack('<II', cpu.mem_read(sp + 4, 8))
        assert desc != 0 and op == 0
        # Word zero is uninitialized in both binaries and not part of this contract.
        descriptor = struct.unpack('<7I', cpu.mem_read(desc + 4, 28))
        index = len(self.descriptors)
        assert index < len(self.expected_calls), 'Unexpected sprite creation'
        assert descriptor == self.expected_calls[index]
        assert self.state() == self.expected_snapshots[index], 'Wrong state at sprite boundary'
        self.descriptors.append(descriptor)
        self.calls.append(('Gfx_SpriteOp', (descriptor, op)))
        cpu.reg_write(UC_X86_REG_EAX, self.returns[index])
        cpu.reg_write(UC_X86_REG_EIP, struct.unpack('<I', cpu.mem_read(sp, 4))[0])
        cpu.reg_write(UC_X86_REG_ESP, sp + 4)

    def parse(self, blob, unused, seed, expected_calls, snapshots, returns):
        self.root_entry, self.calls, self.descriptors = 'Font_Parse', [], []
        self.expected_calls, self.expected_snapshots, self.returns = expected_calls, snapshots, returns
        cpu = self.cpu
        cpu.mem_write(BUFFER, blob)
        sp = STACK + 0x8000
        cpu.mem_write(sp, struct.pack('<III', STOP, BUFFER, unused))
        stack = bytes(cpu.mem_read(sp, 0x8000))
        rng = random.Random(seed)
        saved = [rng.getrandbits(32) for _ in SAVED]
        for reg, value in zip(SAVED, saved):
            cpu.reg_write(reg, value)
        cpu.reg_write(UC_X86_REG_ESP, sp)
        cpu.reg_write(UC_X86_REG_EFLAGS, 2)
        before = bytes(cpu.mem_read(self.base, self.size))
        cpu.emu_start(self.entries['Font_Parse'], STOP, count=1000000)
        assert cpu.reg_read(UC_X86_REG_EIP) == STOP
        assert cpu.reg_read(UC_X86_REG_ESP) == sp + 4
        assert [cpu.reg_read(r) for r in SAVED] == saved
        assert not cpu.reg_read(UC_X86_REG_EFLAGS) & 0x400
        assert bytes(cpu.mem_read(sp, 0x8000)) == stack
        assert bytes(cpu.mem_read(BUFFER, len(blob))) == blob
        assert len(self.descriptors) == len(expected_calls)
        after = bytearray(cpu.mem_read(self.base, self.size))
        for _, address, size in self.fields:
            after[address-self.base:address-self.base+size] = before[address-self.base:address-self.base+size]
        assert bytes(after) == before, 'Unrelated image mutation'
        return cpu.reg_read(UC_X86_REG_EAX), self.state(), self.calls


def oracle(old, blob, returns):
    state, calls = dict(old), []
    def put(name, value, index=0):
        data = bytearray(state[name]); struct.pack_into('<I', data, index*4, value)
        state[name] = bytes(data)
    if struct.unpack('<I', state['g_fontSystemInitialized'])[0] == 0:
        put('g_fontSystemInitialized', 1)
        enabled = struct.unpack('<I', state['g_memHandlesInitialized'])[0] != 0
        cursor = struct.unpack('<h', state['g_memHandleCursor'])[0]
        assert cursor >= 0
        handle = 0xffffffff
        if enabled and cursor + 1 < 200:
            handle = struct.unpack_from('<h', state['g_memHandleIds'], cursor*2)[0] & 0xffffffff
            state['g_memHandleCursor'] = struct.pack('<h', cursor+1)
        put('g_fontSubsystemHandle', handle)
        put('g_memPendingContext', 0x4ba6c8); put('g_memPendingCallback', 0x456210)
        put('g_memPendingParameter', 0)
        if enabled:
            free = next((i for i, (v,) in enumerate(struct.iter_unpack('<I', state['g_memHandleStatus'])) if v == 0), None)
            if free is not None:
                for name, value in [('g_memHandleStatus', 1), ('g_memHandleFlags', 0x10000),
                    ('g_memHandleContexts', 0x4ba6c8), ('g_memRegisteredHandleIds', handle),
                    ('g_memHandleParameters', 0), ('g_memHandleCallbacks', 0x456210)]:
                    put(name, value, free)
        fonts = bytearray(state['g_fonts'])
        for i in range(30): struct.pack_into('<6I', fonts, i*1600, 0, 0, 0, 0, 1, 1)
        state['g_fonts'] = bytes(fonts)
        calls = [('Font_InitSystem', ()), ('Mem_NextHandleId', ()), ('Mem_RegisterHandle', (handle,))]
    error = 1050 if blob[:4] != b'LFT\0' else 1060 if struct.unpack_from('<H', blob, 4)[0] != 100 else 0
    fonts = bytearray(state['g_fonts'])
    slot = next((i for i in range(30) if struct.unpack_from('<I', fonts, i*1600)[0] == 0), None)
    if not error and slot is None: error = 1030
    if error:
        put('g_fileErrorLine', error)
        return (0xffffffff, state, calls), [], []
    base = slot*1600
    fonts[base+24:base+30] = blob[6:12]
    fonts[base+1152:base+1600] = blob[0x68c:0x84c]
    descriptors, snapshots = [], []
    for i in range(224):
        offset = struct.unpack_from('<I', blob, 12+4*i)[0]
        present = offset != 0xffffffff
        fonts[base+30+i] = int(present)
        if present:
            width = struct.unpack_from('<h', blob, 0x68c+2*i)[0] & 0xffffffff
            height = struct.unpack_from('<h', blob, 10)[0] & 0xffffffff
            descriptor = (width, height, 0, (BUFFER+0x84c+offset)&0xffffffff, width, 0, 0)
            descriptors.append(descriptor)
            state['g_fonts'] = bytes(fonts); snapshots.append(dict(state))
            calls.append(('Gfx_SpriteOp', (descriptor, 0)))
            handle = returns[len(descriptors)-1]
        else: handle = 0
        struct.pack_into('<I', fonts, base+256+4*i, handle)
    struct.pack_into('<I', fonts, base, 1)
    state['g_fonts'] = bytes(fonts)
    return (slot, state, calls), descriptors, snapshots


def verify_font_parse():
    verify_target()
    phase = 'compilation'
    try:
        original = pefile.PE(str(TARGET)); inspect_original(original)
        commands = build_parser()
        record('compilation', 'pass', commands=commands, scope='Extracted production parser/font lifecycle and complete mem.c; focused DLL')
        rebuilt = pefile.PE(str(DLL))
        symbols = {s.name.decode(): rebuilt.OPTIONAL_HEADER.ImageBase+s.address
                   for s in rebuilt.DIRECTORY_ENTRY_EXPORT.symbols if s.name}
        prefix = rebuilt.get_data(symbols['Font_Parse']-rebuilt.OPTIONAL_HEADER.ImageBase, SIZE)
        record('raw_bytes', 'pass' if prefix == original.get_data(RVA, SIZE) else 'different',
               scope='Raw prefix diagnostic only; compiled extent and instruction equality unverified')
        phase = 'emulation'
        rng, cases = random.Random(RVA), 0
        def compare(flag=1, slot=0, pattern='sparse', magic=b'LFT\0', version=100,
                    enabled=1, cursor=0, registration=0, boundary=False):
            nonlocal cases
            state = {n: rng.randbytes(size) for n, _, size in LIFECYCLE_FIELDS}
            state['g_fileErrorLine'] = struct.pack('<I', 0x12345678)
            state['g_fontSystemInitialized'] = struct.pack('<I', flag)
            state['g_memHandlesInitialized'] = struct.pack('<I', enabled)
            state['g_memHandleCursor'] = struct.pack('<h', cursor)
            state['g_memHandleIds'] = struct.pack('<200h', *[rng.choice([1, -1, -32768, 32767, 0]) for _ in range(200)])
            statuses = [1]*200
            if registration is not None: statuses[registration] = 0
            state['g_memHandleStatus'] = struct.pack('<200I', *statuses)
            fonts = bytearray(state['g_fonts'])
            for i in range(30): struct.pack_into('<I', fonts, i*1600, rng.choice([1, 2, 0x80000000, 0xffffffff]))
            if slot is not None: struct.pack_into('<I', fonts, slot*1600, 0)
            if slot is not None and slot < 29: struct.pack_into('<I', fonts, 29*1600, 0)
            state['g_fonts'] = bytes(fonts)
            blob = bytearray(rng.randbytes(0x1000))
            blob[:4] = magic
            values = [0, 0x7fff, 0x8000, 0xffff]
            struct.pack_into('<4H', blob, 4, version, rng.choice(values), rng.choice(values), rng.choice(values))
            for i in range(224):
                present = pattern == 'full' or pattern == 'sparse' and i in (0, 113, 223) or pattern == 'random' and rng.randrange(2)
                offset = rng.choice([0, 1, 0xfffffffe, 0x80000000, 0x7fffffff]) if boundary else rng.randrange(0x700)
                struct.pack_into('<I', blob, 12+4*i, offset if present else 0xffffffff)
                struct.pack_into('<H', blob, 0x68c+2*i, rng.choice(values) if boundary else rng.randrange(64))
            returns = [rng.choice([0, 1, 0x80000000, 0xffffffff, rng.getrandbits(32)]) for _ in range(224)]
            expected, descriptors, snapshots = oracle(state, blob, returns)
            a, b = ParserCPU(original), ParserCPU(rebuilt, symbols)
            a.load(state); b.load(state)
            unused = rng.getrandbits(32)
            result_a = a.parse(bytes(blob), unused, cases, descriptors, snapshots, returns)
            result_b = b.parse(bytes(blob), unused, cases, descriptors, snapshots, returns)
            assert result_a == result_b == expected, (cases, result_a[0], result_b[0], expected[0])
            cases += 1
        for slot in range(30):
            for pattern in ['empty', 'sparse', 'full']: compare(slot=slot, pattern=pattern, boundary=True)
        for flag in [0, 1, 2, 0x80000000, 0xffffffff]:
            for bad_byte in range(4):
                magic = bytearray(b'LFT\0'); magic[bad_byte] ^= 0xff
                compare(flag=flag, magic=bytes(magic), version=0, slot=None)
            for version in [0, 99, 101, 0x8000, 0xffff]: compare(flag=flag, version=version, slot=None)
            compare(flag=flag, slot=None)
        for enabled in [0, 1, 2, 0xffffffff]:
            for cursor in [0, 198, 199, 200, 32767]:
                for registration in [0, 199, None]:
                    compare(flag=0, enabled=enabled, cursor=cursor, registration=registration, boundary=True)
        for _ in range(60): compare(flag=rng.choice([1, 2, 0xffffffff]), slot=rng.randrange(30), pattern='random', boundary=True)
        record('emulation', 'pass', cases=cases,
               scope='Full state, descriptor fields 4..31, boundary snapshots, lazy real handles, return and ABI; modeled Gfx_SpriteOp',
               limitations='Descriptor word zero uninitialized/excluded; no native sprite allocation, invalid buffers, negative handle cursors, instruction equality or native game parity')
        print(f'PASS: {cases} Font_Parse original-vs-C cases; state, descriptors, calls, lazy initialization and ABI equality.')
    except Exception as exc:
        record(phase, 'fail', error=str(exc))
        raise


if __name__ == '__main__':
    verify_font_parse()
