# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Bounded original-instruction versus extracted production C font cleanup validation.

Validates Font_InitSystem (VA 0x00456180 / RVA 0x56180), Font_Unload (VA 0x00456470 / RVA 0x56470) and Font_Shutdown
(VA 0x00456210 / RVA 0x56210) against authentic PE execution in x86 emulation.
Invoked by verify_matching.py in its pinned pefile/capstone/Unicorn environment.
"""
import hashlib
import random
import shutil
import struct
import subprocess

import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ESI,
    UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_ESP, UC_X86_REG_EIP,
    UC_X86_REG_EFLAGS)

from build_decomp import STARTUP_BOUNDARY_SOURCE, compile_banner
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions; disable -O/PYTHONOPTIMIZE')

RVA_INIT = 0x56180
SIZE_INIT = 134
SHA_INIT = '32c69eb459a5bc9c8e74254334262750ef7db85caf4f5a70437ebc28738f2c3d'

RVA_SHUTDOWN = 0x56210
RVA_UNLOAD = 0x56470
SIZE_SHUTDOWN = 83
SIZE_UNLOAD = 88
SHA_SHUTDOWN = '93eb65fc8f1076b5a1ec14a6b4e1b603893a3eb1bc30f17017e5ab54809a604c'
SHA_UNLOAD = 'd398726038e727ae65ae4041981d9970b581fa2078b75d4e946c1dc0147a4009'

DLL = BUILD / 'font_cleanup_validation.dll'
INPUTS = ['decomp/src/lisa3d.c', 'decomp/include/lisa3d.h',
    'tools/build_decomp.py',
    'decomp/src/geputget.c', 'decomp/include/geputget.h', 'decomp/include/mem.h',
    'decomp/src/mem.c', 'decomp/target.json', 'tools/verify_font_cleanup.py',
    'tools/verify_matching.py', 'tools/windows_target.py', 'tools/windows_tracking.py'
]
SAVED = [UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP]
STACK, STOP = 0x7000000, 0x7100000
MOCK_SPRITE_OP = 0x7300000


def record(rva, kind, outcome, cases=0, **details):
    record_run(rva, kind, outcome, inputs=INPUTS,
               artifact=DLL.relative_to(ROOT).as_posix() if DLL.exists() else None,
               cases=cases, command=('uv run tools/verify_font_cleanup.py' if __name__ == '__main__'
                                     else 'uv run tools/verify_matching.py'), details=details)


def inspect_original(pe):
    code = pe.get_data(RVA_INIT, SIZE_INIT)
    assert hashlib.sha256(code).hexdigest() == SHA_INIT
    assert pe.get_data(RVA_INIT + SIZE_INIT, 10) == b'\xcc' * 10
    assert pe.get_data(0xba6c4, 4) == b'\0' * 4
    assert pe.get_data(0xba6c8, 11) == b'fontExit()\0'
    relocs = [(e.rva, struct.unpack('<I', pe.get_data(e.rva, 4))[0])
              for block in pe.DIRECTORY_ENTRY_BASERELOC for e in block.entries
              if e.type == 3 and RVA_INIT <= e.rva < RVA_INIT + SIZE_INIT]
    assert relocs == [(0x56182, 0x4ba6c4), (0x5619c, 0x4ba6c4),
        (0x561a7, 0x50e680), (0x561ad, 0x63c690), (0x561b1, 0x4ba6c8),
        (0x561b7, 0x63c694), (0x561bb, 0x456210), (0x561c1, 0x63c698),
        (0x561ce, 0x63f2e0), (0x561da, 0x64ae60)]
    decoded = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(code, 0x456180))
    assert len(decoded) == 33 and sum(i.size for i in decoded) == SIZE_INIT
    assert [(i.address, i.op_str) for i in decoded if i.mnemonic == 'call'] == [
        (0x4561a0, '0x45b1b0'), (0x4561c5, '0x45b360')]

    code_unload = pe.get_data(RVA_UNLOAD, SIZE_UNLOAD)
    assert hashlib.sha256(code_unload).hexdigest() == SHA_UNLOAD
    assert pe.get_data(RVA_UNLOAD + SIZE_UNLOAD, 8) == b'\xcc' * 8

    code_shutdown = pe.get_data(RVA_SHUTDOWN, SIZE_SHUTDOWN)
    assert hashlib.sha256(code_shutdown).hexdigest() == SHA_SHUTDOWN
    assert pe.get_data(RVA_SHUTDOWN + SIZE_SHUTDOWN, 13) == b'\xcc' * 13

    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {a: (n, local, params, bits) for a, n, local, params, bits in
           struct.iter_unpack('<IIIHH', pe.__data__[debug.PointerToRawData:
                                                     debug.PointerToRawData + debug.SizeOfData])}
    assert fpo[RVA_INIT] == (SIZE_INIT, 0, 0, 0x209)
    assert fpo[RVA_UNLOAD] == (SIZE_UNLOAD, 0, 1, 0x140a)
    assert fpo[RVA_SHUTDOWN] == (SIZE_SHUTDOWN, 0, 0, 0x209)

    actual_unload_relocs = [(e.rva, struct.unpack('<I', pe.get_data(e.rva, 4))[0])
                           for block in pe.DIRECTORY_ENTRY_BASERELOC for e in block.entries
                           if e.type == 3 and RVA_UNLOAD <= e.rva < RVA_UNLOAD + SIZE_UNLOAD]
    assert actual_unload_relocs == [
        (0x5648b, 0x63f2e0), (0x56492, 0x63f2fe), (0x564a6, 0x63f3e0)
    ]

    actual_shutdown_relocs = [(e.rva, struct.unpack('<I', pe.get_data(e.rva, 4))[0])
                             for block in pe.DIRECTORY_ENTRY_BASERELOC for e in block.entries
                             if e.type == 3 and RVA_SHUTDOWN <= e.rva < RVA_SHUTDOWN + SIZE_SHUTDOWN]
    assert actual_shutdown_relocs == [
        (0x56212, 0x4ba6c4), (0x56224, 0x50e680), (0x5622c, 0x63f2e0),
        (0x5623a, 0x4ba6c4), (0x56255, 0x64ae60)
    ]

    md = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions_unload = list(md.disasm(code_unload, 0x456470))
    assert sum(i.size for i in instructions_unload) == SIZE_UNLOAD
    assert [(i.address, i.op_str) for i in instructions_unload if i.mnemonic == 'call'] == [
        (0x4564ad, '0x456d40')
    ]

    instructions_shutdown = list(md.disasm(code_shutdown, 0x456210))
    assert sum(i.size for i in instructions_shutdown) == SIZE_SHUTDOWN
    assert [(i.address, i.op_str) for i in instructions_shutdown if i.mnemonic == 'call'] == [
        (0x456230, '0x45b410'), (0x456244, '0x456470')
    ]


def build_dll():
    BUILD.mkdir(parents=True, exist_ok=True)
    geputget_src = (ROOT / 'decomp/src/geputget.c').read_text(encoding='utf-8-sig')
    mem_src = (ROOT / 'decomp/src/mem.c').read_text(encoding='utf-8-sig')

    start = geputget_src.index('int Font_Shutdown(void)')
    opening = geputget_src.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (geputget_src[end] == '{') - (geputget_src[end] == '}')
        end += 1
    shutdown_code = geputget_src[start:end]

    start = geputget_src.index('int Font_Unload(int font_id)')
    opening = geputget_src.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (geputget_src[end] == '{') - (geputget_src[end] == '}')
        end += 1
    unload_code = geputget_src[start:end]

    start = geputget_src.index('int Font_InitSystem(void)')
    opening = geputget_src.index('{', start)
    depth, end = 1, opening + 1
    while depth:
        depth += (geputget_src[end] == '{') - (geputget_src[end] == '}')
        end += 1
    init_code = geputget_src[start:end]
    context_line = next(line for line in geputget_src.splitlines()
                        if line.startswith('static const char s_fontExitContext[]'))

    unit = f"""#include "geputget.h"
{mem_src}
FontSlot g_fonts[MAX_FONTS];
int g_fontSystemInitialized;
int g_fontSubsystemHandle;
{context_line}
const char * const validation_fontExitContext = s_fontExitContext;
typedef char Font_SizeCheck[(sizeof(FontSlot) == 1600) ? 1 : -1];
typedef char Font_HeaderCheck[(offsetof(FontSlot, field_18) == 24) ? 1 : -1];
typedef char Font_PresenceCheck[(offsetof(FontSlot, glyph_present) == 30) ? 1 : -1];
typedef char Font_HandleCheck[(offsetof(FontSlot, glyph_handles) == 256) ? 1 : -1];
extern void *Gfx_SpriteOp(void *desc, int op);
{unload_code}
{shutdown_code}
{init_code}
"""
    unit_path = BUILD / 'font_cleanup_unit.c'
    obj_path = BUILD / 'font_cleanup.obj'
    unit_path.write_text(unit, encoding='utf-8')

    stub_c = """void *Gfx_SpriteOp(void *desc, int op) {
    (void)desc; (void)op;
    return 0;
}
"""
    stub_c += STARTUP_BOUNDARY_SOURCE + 'void free(void *p) { (void)p; for (;;) {} }\nvoid *malloc(unsigned int n) { (void)n; for (;;) {} }\n'
    stub_path = BUILD / 'sprite_op_stub.c'
    stub_obj = BUILD / 'sprite_op_stub.obj'
    stub_path.write_text(stub_c, encoding='utf-8')

    for p in [obj_path, stub_obj, DLL, DLL.with_suffix('.lib'), DLL.with_suffix('.exp')]:
        p.unlink(missing_ok=True)

    cc = shutil.which('clang')
    ld = shutil.which('lld-link')
    if not cc or not ld:
        raise RuntimeError('Clang and LLD required for font cleanup validation')

    compile_cmd = [cc, '--target=i686-pc-windows-msvc', '-std=c89', '-pedantic-errors',
                   '-Wall', '-Wextra', '-Werror', '-O2', '-ffreestanding', '-fno-builtin',
                   '-fno-inline', '-mno-sse', '-mno-sse2', '-I', str(ROOT / 'decomp/include'),
                   '-c', str(unit_path), '-o', str(obj_path)]
    subprocess.run(compile_cmd, check=True)

    compile_stub_cmd = [cc, '--target=i686-pc-windows-msvc', '-std=c89', '-pedantic-errors',
                        '-Wall', '-Wextra', '-Werror', '-O2', '-ffreestanding', '-fno-builtin',
                        '-fno-inline', '-mno-sse', '-mno-sse2', '-I', str(ROOT / 'decomp/include'),
                        '-c', str(stub_path), '-o', str(stub_obj)]
    subprocess.run(compile_stub_cmd, check=True)

    exports = [
        'Font_InitSystem', 'Font_Shutdown', 'Font_Unload', 'Mem_ReleaseHandleId', 'Gfx_SpriteOp',
        'Mem_NextHandleId', 'Mem_RegisterHandle', 'Mem_InitHandles', 'Mem_ShutdownHandles',
        'validation_fontExitContext', 'g_memHandleIds', 'g_memHandleCursor',
        'g_memHandleContexts', 'g_memHandleParameters', 'g_memHandleCallbacks',
        'g_memHandleFlags', 'g_memPendingContext', 'g_memPendingCallback', 'g_memPendingParameter',
        'g_fonts', 'g_fontSystemInitialized', 'g_fontSubsystemHandle',
        'g_memHandlesInitialized', 'g_memHandleStatus', 'g_memRegisteredHandleIds'
    ]
    link_cmd = [ld, '/dll', '/noentry', '/nodefaultlib', '/machine:x86',
                '/base:0x10000000', '/out:' + str(DLL), str(obj_path), str(stub_obj)] + [
                    '/export:' + e for e in exports
                ]
    banner_obj, banner_cmd = compile_banner(compile_cmd[:-4], DLL.stem)
    link_cmd.append(str(banner_obj))
    subprocess.run(link_cmd, check=True)
    assert obj_path.stat().st_size and DLL.stat().st_size
    return [compile_cmd, compile_stub_cmd, banner_cmd, link_cmd]


def execute_unload(pe, entry, fonts_addr, sprite_op_addr, font_id, fonts_data, seed, original=False):
    base = pe.OPTIONAL_HEADER.ImageBase
    size = (pe.OPTIONAL_HEADER.SizeOfImage + 4095) & ~4095
    cpu = Uc(UC_ARCH_X86, UC_MODE_32)
    cpu.mem_map(base, size)
    cpu.mem_write(base, pe.get_memory_mapped_image())

    cpu.mem_write(fonts_addr, bytes(fonts_data))

    cpu.mem_map(STACK, 0x10000)
    cpu.mem_map(STOP, 0x1000)
    cpu.mem_map(MOCK_SPRITE_OP, 0x1000)
    cpu.mem_write(MOCK_SPRITE_OP, b'\xc3')

    if original:
        cpu.mem_write(0x50ebc0, struct.pack('<I', MOCK_SPRITE_OP))

    sp = STACK + 0x8000
    cpu.mem_write(sp, struct.pack('<II', STOP, font_id & 0xffffffff))
    stack_before = bytes(cpu.mem_read(sp, 0x8000))

    rng = random.Random(seed)
    registers = [rng.getrandbits(32) for _ in SAVED]
    for reg, value in zip(SAVED, registers):
        cpu.reg_write(reg, value)
    cpu.reg_write(UC_X86_REG_ESP, sp)
    cpu.reg_write(UC_X86_REG_EFLAGS, 2)

    before = bytes(cpu.mem_read(base, size))
    sprite_op_calls = []

    def code_hook(uc, address, length, unused):
        if original and address == MOCK_SPRITE_OP:
            cur_sp = uc.reg_read(UC_X86_REG_ESP)
            desc = struct.unpack('<I', uc.mem_read(cur_sp + 4, 4))[0]
            op = struct.unpack('<I', uc.mem_read(cur_sp + 8, 4))[0]
            sprite_op_calls.append((desc, op))
        elif not original and address == sprite_op_addr:
            cur_sp = uc.reg_read(UC_X86_REG_ESP)
            desc = struct.unpack('<I', uc.mem_read(cur_sp + 4, 4))[0]
            op = struct.unpack('<I', uc.mem_read(cur_sp + 8, 4))[0]
            sprite_op_calls.append((desc, op))

    cpu.hook_add(UC_HOOK_CODE, code_hook)
    cpu.emu_start(entry, STOP, count=1000000)

    assert cpu.reg_read(UC_X86_REG_EIP) == STOP
    assert cpu.reg_read(UC_X86_REG_ESP) == sp + 4
    assert [cpu.reg_read(r) for r in SAVED] == registers
    assert not (cpu.reg_read(UC_X86_REG_EFLAGS) & 0x400)
    assert bytes(cpu.mem_read(sp, 0x8000)) == stack_before

    after = bytearray(cpu.mem_read(base, size))
    result_fonts = bytes(after[fonts_addr - base : fonts_addr - base + 48000])
    after[fonts_addr - base : fonts_addr - base + 48000] = before[fonts_addr - base : fonts_addr - base + 48000]
    if original:
        after[0x50ebc0 - base : 0x50ebc0 - base + 4] = before[0x50ebc0 - base : 0x50ebc0 - base + 4]
    assert bytes(after) == before, "Unrelated memory modified!"

    return cpu.reg_read(UC_X86_REG_EAX), result_fonts, sprite_op_calls


def execute_shutdown(pe, entry, fonts_addr, sys_init_addr, handle_addr,
                     mem_init_addr, mem_status_addr, mem_reg_ids_addr,
                     sprite_op_addr, sys_init_val, handle_val, fonts_data,
                     mem_init_val, mem_status_list, mem_reg_ids_list,
                     seed, original=False):
    base = pe.OPTIONAL_HEADER.ImageBase
    size = (pe.OPTIONAL_HEADER.SizeOfImage + 4095) & ~4095
    cpu = Uc(UC_ARCH_X86, UC_MODE_32)
    cpu.mem_map(base, size)
    cpu.mem_write(base, pe.get_memory_mapped_image())

    cpu.mem_write(fonts_addr, bytes(fonts_data))
    cpu.mem_write(sys_init_addr, struct.pack('<I', sys_init_val))
    cpu.mem_write(handle_addr, struct.pack('<I', handle_val))
    cpu.mem_write(mem_init_addr, struct.pack('<I', mem_init_val))
    cpu.mem_write(mem_status_addr, struct.pack('<200I', *mem_status_list))
    cpu.mem_write(mem_reg_ids_addr, struct.pack('<200I', *mem_reg_ids_list))

    cpu.mem_map(STACK, 0x10000)
    cpu.mem_map(STOP, 0x1000)
    cpu.mem_map(MOCK_SPRITE_OP, 0x1000)
    cpu.mem_write(MOCK_SPRITE_OP, b'\xc3')

    if original:
        cpu.mem_write(0x50ebc0, struct.pack('<I', MOCK_SPRITE_OP))

    sp = STACK + 0x8000
    cpu.mem_write(sp, struct.pack('<I', STOP))
    stack_before = bytes(cpu.mem_read(sp, 0x8000))

    rng = random.Random(seed)
    registers = [rng.getrandbits(32) for _ in SAVED]
    for reg, value in zip(SAVED, registers):
        cpu.reg_write(reg, value)
    cpu.reg_write(UC_X86_REG_ESP, sp)
    cpu.reg_write(UC_X86_REG_EFLAGS, 2)

    sprite_op_calls = []

    def code_hook(uc, address, length, unused):
        if original and address == MOCK_SPRITE_OP:
            cur_sp = uc.reg_read(UC_X86_REG_ESP)
            desc = struct.unpack('<I', uc.mem_read(cur_sp + 4, 4))[0]
            op = struct.unpack('<I', uc.mem_read(cur_sp + 8, 4))[0]
            sprite_op_calls.append((desc, op))
        elif not original and address == sprite_op_addr:
            cur_sp = uc.reg_read(UC_X86_REG_ESP)
            desc = struct.unpack('<I', uc.mem_read(cur_sp + 4, 4))[0]
            op = struct.unpack('<I', uc.mem_read(cur_sp + 8, 4))[0]
            sprite_op_calls.append((desc, op))

    cpu.hook_add(UC_HOOK_CODE, code_hook)
    cpu.emu_start(entry, STOP, count=10000000)

    assert cpu.reg_read(UC_X86_REG_EIP) == STOP
    assert cpu.reg_read(UC_X86_REG_ESP) == sp + 4
    assert [cpu.reg_read(r) for r in SAVED] == registers
    assert not (cpu.reg_read(UC_X86_REG_EFLAGS) & 0x400)
    assert bytes(cpu.mem_read(sp, 0x8000)) == stack_before

    eax = cpu.reg_read(UC_X86_REG_EAX)
    after = bytearray(cpu.mem_read(base, size))
    result_fonts = bytes(after[fonts_addr - base : fonts_addr - base + 48000])
    result_sys_init = struct.unpack('<I', after[sys_init_addr - base : sys_init_addr - base + 4])[0]
    result_handle = struct.unpack('<I', after[handle_addr - base : handle_addr - base + 4])[0]
    result_mem_init = struct.unpack('<I', after[mem_init_addr - base : mem_init_addr - base + 4])[0]
    result_mem_status = list(struct.unpack('<200I', after[mem_status_addr - base : mem_status_addr - base + 800]))
    result_mem_ids = list(struct.unpack('<200I', after[mem_reg_ids_addr - base : mem_reg_ids_addr - base + 800]))

    return (eax, result_sys_init, result_handle, result_fonts,
            result_mem_init, result_mem_status, result_mem_ids, sprite_op_calls)


# Original preferred VAs. Pointer relocation is normalized only for the verified
# fontExit string and Font_Shutdown callback in their context/callback fields.
LIFECYCLE_FIELDS = [
    ('g_memHandlesInitialized', 0x4bab38, 4),
    ('g_memHandleStatus', 0x5116e0, 800),
    ('g_memHandleIds', 0x511230, 400),
    ('g_memHandleCursor', 0x512040, 2),
    ('g_memHandleContexts', 0x510bf0, 800),
    ('g_memHandleParameters', 0x510f10, 800),
    ('g_memRegisteredHandleIds', 0x5113c0, 800),
    ('g_memHandleCallbacks', 0x511a00, 800),
    ('g_memHandleFlags', 0x511d20, 800),
    ('g_memPendingContext', 0x63c690, 4),
    ('g_memPendingCallback', 0x63c694, 4),
    ('g_memPendingParameter', 0x63c698, 4),
    ('g_fontSystemInitialized', 0x4ba6c4, 4),
    ('g_fontSubsystemHandle', 0x50e680, 4),
    ('g_fonts', 0x63f2e0, 48000)]


class FontLifecycleCPU:
    """Persistent CPU: all handle/font bodies execute; only sprite release is modeled."""
    def __init__(self, pe, symbols=None):
        self.original = symbols is None
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.size = (pe.OPTIONAL_HEADER.SizeOfImage + 4095) & ~4095
        self.cpu = Uc(UC_ARCH_X86, UC_MODE_32)
        self.cpu.mem_map(self.base, self.size)
        self.cpu.mem_write(self.base, pe.get_memory_mapped_image())
        self.cpu.mem_map(STACK, 0x10000)
        self.cpu.mem_map(STOP, 0x1000)
        self.cpu.mem_map(MOCK_SPRITE_OP, 0x1000)
        self.cpu.mem_write(MOCK_SPRITE_OP, b'\xc3')
        self.fields = [(name, address if self.original else symbols[name], size)
                       for name, address, size in LIFECYCLE_FIELDS]
        self.addresses = {name: address for name, address, _ in self.fields}
        if self.original:
            self.entries = {'Font_InitSystem': 0x456180, 'Font_Shutdown': 0x456210,
                'Font_Unload': 0x456470, 'Mem_NextHandleId': 0x45b1b0,
                'Mem_RegisterHandle': 0x45b360, 'Mem_ReleaseHandleId': 0x45b410,
                'Mem_InitHandles': 0x45b1f0, 'Gfx_SpriteOp': MOCK_SPRITE_OP}
            self.context = 0x4ba6c8
            self.cpu.mem_write(0x50ebc0, struct.pack('<I', MOCK_SPRITE_OP))
        else:
            self.entries = {name: symbols[name] for name in [
                'Font_InitSystem', 'Font_Shutdown', 'Font_Unload', 'Mem_NextHandleId',
                'Mem_RegisterHandle', 'Mem_ReleaseHandleId', 'Mem_InitHandles', 'Gfx_SpriteOp']}
            self.context = struct.unpack('<I', self.cpu.mem_read(
                symbols['validation_fontExitContext'], 4))[0]
        assert bytes(self.cpu.mem_read(self.context, 11)) == b'fontExit()\0'
        self.cpu.hook_add(UC_HOOK_CODE, self.hook)

    def pointer(self, name, value, encode):
        pairs = []
        if name in ('g_memPendingContext', 'g_memHandleContexts'):
            pairs = [(0x4ba6c8, self.context)]
        if name in ('g_memPendingCallback', 'g_memHandleCallbacks'):
            pairs = [(0x456210, self.entries['Font_Shutdown'])]
        for canonical, actual in pairs:
            if value == (canonical if encode else actual):
                return actual if encode else canonical
        return value

    def encode(self, name, data, encode):
        if name not in ('g_memPendingContext', 'g_memHandleContexts',
                        'g_memPendingCallback', 'g_memHandleCallbacks'):
            return data
        return b''.join(struct.pack('<I', self.pointer(name, value, encode))
                        for (value,) in struct.iter_unpack('<I', data))

    def load(self, state):
        for name, address, size in self.fields:
            assert len(state[name]) == size
            self.cpu.mem_write(address, self.encode(name, state[name], True))

    def state(self):
        return {name: self.encode(name, bytes(self.cpu.mem_read(address, size)), False)
                for name, address, size in self.fields}

    def hook(self, cpu, address, length, unused):
        name = next((name for name, entry in self.entries.items() if entry == address), None)
        if name is None or name == self.root_entry:
            return
        sp = cpu.reg_read(UC_X86_REG_ESP)
        if name == 'Mem_NextHandleId':
            args = ()
            # Observe the initialized flag before allocation.
            assert self.state()['g_fontSystemInitialized'] == struct.pack('<I', 1)
        elif name == 'Gfx_SpriteOp':
            args = struct.unpack('<II', cpu.mem_read(sp + 4, 8))
        else:
            args = struct.unpack('<I', cpu.mem_read(sp + 4, 4))
        if name == 'Mem_RegisterHandle':
            state = self.state()
            assert state['g_memPendingContext'] == struct.pack('<I', 0x4ba6c8)
            assert state['g_memPendingCallback'] == struct.pack('<I', 0x456210)
            assert state['g_memPendingParameter'] == struct.pack('<I', 0)
            assert state['g_fontSubsystemHandle'] == struct.pack('<I', args[0])
        self.calls.append((name, args))

    def invoke(self, name, seed):
        self.root_entry, self.calls = name, []
        sp = STACK + 0x8000
        self.cpu.mem_write(sp, struct.pack('<I', STOP))
        stack = bytes(self.cpu.mem_read(sp, 0x8000))
        rng = random.Random(seed)
        registers = [rng.getrandbits(32) for _ in SAVED]
        for reg, value in zip(SAVED, registers):
            self.cpu.reg_write(reg, value)
        self.cpu.reg_write(UC_X86_REG_ESP, sp)
        self.cpu.reg_write(UC_X86_REG_EFLAGS, 2)
        before = bytes(self.cpu.mem_read(self.base, self.size))
        self.cpu.emu_start(self.entries[name], STOP, count=10000000)
        assert self.cpu.reg_read(UC_X86_REG_EIP) == STOP
        assert self.cpu.reg_read(UC_X86_REG_ESP) == sp + 4
        assert [self.cpu.reg_read(r) for r in SAVED] == registers
        assert self.cpu.reg_read(UC_X86_REG_EFLAGS) & 0x400 == 0
        assert bytes(self.cpu.mem_read(sp, 0x8000)) == stack
        after = bytearray(self.cpu.mem_read(self.base, self.size))
        for _, address, size in self.fields:
            offset = address - self.base
            after[offset:offset + size] = before[offset:offset + size]
        assert bytes(after) == before, 'Write outside lifecycle state'
        return self.cpu.reg_read(UC_X86_REG_EAX), self.state(), list(self.calls)


def verify_font_init(original, rebuilt, symbols, details):
    rng = random.Random(0x456180)
    cases = 0

    def word(state, field, index=0):
        return struct.unpack_from('<I', state[field], index * 4)[0]

    def store(state, field, value, index=0):
        data = bytearray(state[field])
        struct.pack_into('<I', data, index * 4, value & 0xffffffff)
        state[field] = bytes(data)

    def expected_init(old):
        state = dict(old)
        if word(state, 'g_fontSystemInitialized') == 1:
            return 1010, state, []
        store(state, 'g_fontSystemInitialized', 1)
        enabled = word(state, 'g_memHandlesInitialized') != 0
        cursor = struct.unpack('<h', state['g_memHandleCursor'])[0]
        # Negative cursors need surrounding global layout recovery; excluded here.
        assert cursor >= 0
        handle = 0xffffffff
        if enabled and cursor + 1 < 200:
            handle = struct.unpack_from('<h', state['g_memHandleIds'], cursor * 2)[0] & 0xffffffff
            state['g_memHandleCursor'] = struct.pack('<h', cursor + 1)
        store(state, 'g_fontSubsystemHandle', handle)
        store(state, 'g_memPendingContext', 0x4ba6c8)
        store(state, 'g_memPendingCallback', 0x456210)
        store(state, 'g_memPendingParameter', 0)
        if enabled:
            status = struct.unpack('<200I', state['g_memHandleStatus'])
            slot = next((i for i, value in enumerate(status) if value == 0), None)
            if slot is not None:
                for field, value in [('g_memHandleStatus', 1), ('g_memHandleFlags', 0x10000),
                    ('g_memHandleContexts', 0x4ba6c8), ('g_memRegisteredHandleIds', handle),
                    ('g_memHandleParameters', 0), ('g_memHandleCallbacks', 0x456210)]:
                    store(state, field, value, slot)
        fonts = bytearray(state['g_fonts'])
        for slot in range(30):
            struct.pack_into('<6I', fonts, slot * 1600, 0, 0, 0, 0, 1, 1)
        state['g_fonts'] = bytes(fonts)
        return 1, state, [('Mem_NextHandleId', ()), ('Mem_RegisterHandle', (handle,))]

    def fresh(flag, enabled, cursor, free_slot):
        state = {name: rng.randbytes(size) for name, _, size in LIFECYCLE_FIELDS}
        store(state, 'g_fontSystemInitialized', flag)
        store(state, 'g_memHandlesInitialized', enabled)
        state['g_memHandleCursor'] = struct.pack('<h', cursor)
        state['g_memHandleIds'] = struct.pack('<200h', *[
            rng.choice([0, 1, 32767, -1, -32768]) for _ in range(200)])
        statuses = [rng.choice([1, 2, 0x80000000, 0xffffffff]) for _ in range(200)]
        if free_slot is not None:
            statuses[free_slot:] = [0] * (200 - free_slot)
        state['g_memHandleStatus'] = struct.pack('<200I', *statuses)
        return state

    def compare(old):
        nonlocal cases
        a, b = FontLifecycleCPU(original), FontLifecycleCPU(rebuilt, symbols)
        a.load(old); b.load(old)
        expected = expected_init(old)
        actual_a = a.invoke('Font_InitSystem', cases)
        actual_b = b.invoke('Font_InitSystem', cases)
        assert actual_a == actual_b == expected, (cases,
            actual_a[0], actual_b[0], expected[0], actual_a[2], actual_b[2], expected[2],
            [(name, actual_a[1][name][:32].hex(), actual_b[1][name][:32].hex(),
              expected[1][name][:32].hex()) for name in old
             if not actual_a[1][name] == actual_b[1][name] == expected[1][name]])
        cases += 1
        return a, b

    # Every registration index and first-zero selection, with arbitrary old fonts.
    for slot in range(200):
        compare(fresh(0, 1, slot % 199, slot))
    # Strict flag guard, disabled handles, cursor exhaustion, noncanonical flags,
    # signed IDs, full registration table; failures are ignored by Font_InitSystem.
    for flag in [0, 1, 2, 0x80000000, 0xffffffff]:
        for enabled in [0, 1, 2, 0xffffffff]:
            for cursor in [0, 198, 199, 200, 32767]:
                for slot in [0, 199, None]:
                    compare(fresh(flag, enabled, cursor, slot))

    lifecycle_steps = 0
    for active in [[], [0], [29], [0, 5, 29], list(range(30))]:
        old = fresh(0, 0, 0, None)
        a, b = FontLifecycleCPU(original), FontLifecycleCPU(rebuilt, symbols)
        a.load(old); b.load(old)
        result_a = a.invoke('Mem_InitHandles', cases)
        result_b = b.invoke('Mem_InitHandles', cases)
        assert result_a == result_b and result_a[0] == 1
        assert word(result_a[1], 'g_memHandlesInitialized') == 1
        assert result_a[1]['g_memHandleIds'] == struct.pack('<200h', *range(1, 201))
        assert result_a[1]['g_memHandleStatus'] == b'\0' * 800
        assert result_a[1]['g_memHandleCursor'] == b'\0' * 2
        state = result_a[1]
        assert a.invoke('Font_InitSystem', cases) == b.invoke('Font_InitSystem', cases) == expected_init(state)
        cases += 1; lifecycle_steps += 2
        state = a.state()
        assert word(state, 'g_fontSubsystemHandle') == 1
        # Populate font records directly; Font_Parse and native allocation aren't claimed.
        fonts = bytearray(state['g_fonts'])
        for slot in range(30):
            fonts[slot * 1600 + 30:slot * 1600 + 254] = b'\0' * 224
        calls = [('Mem_ReleaseHandleId', (1,))]
        for slot in active:
            struct.pack_into('<I', fonts, slot * 1600, 1)
            calls.append(('Font_Unload', (slot,)))
            for glyph in [0, 113, 223]:
                fonts[slot * 1600 + 30 + glyph] = 1
                handle = 0x80000000 + slot * 224 + glyph
                struct.pack_into('<I', fonts, slot * 1600 + 256 + 4 * glyph, handle)
                calls.append(('Gfx_SpriteOp', (0, handle)))
        state['g_fonts'] = bytes(fonts)
        a.load(state); b.load(state)
        expected = dict(state)
        store(expected, 'g_fontSystemInitialized', 0)
        store(expected, 'g_memHandleStatus', 0, 0)
        for slot in active:
            struct.pack_into('<I', fonts, slot * 1600, 0)
        expected['g_fonts'] = bytes(fonts)
        assert a.invoke('Font_Shutdown', cases) == b.invoke('Font_Shutdown', cases) == (1, expected, calls)
        lifecycle_steps += 1
        assert a.invoke('Font_InitSystem', cases) == b.invoke('Font_InitSystem', cases) == expected_init(expected)
        cases += 1; lifecycle_steps += 1
        state = a.state()
        assert word(state, 'g_fontSubsystemHandle') == 2
        assert word(state, 'g_memHandleStatus') == 1
        assert state['g_memHandleCursor'] == struct.pack('<h', 2)
    record(RVA_INIT, 'emulation', 'pass', cases=cases,
           lifecycle_steps=lifecycle_steps, **details)
    print(f'PASS: {cases} Font_InitSystem differential executions; {lifecycle_steps} persistent lifecycle steps; state/calls/ABI equality.')


def verify_font_cleanup():
    verify_target()
    commands = build_dll()
    original = pefile.PE(str(TARGET))
    rebuilt = pefile.PE(str(DLL))

    inspect_original(original)
    assert not hasattr(rebuilt, 'DIRECTORY_ENTRY_IMPORT')

    symbols = {e.name.decode(): rebuilt.OPTIONAL_HEADER.ImageBase + e.address
               for e in rebuilt.DIRECTORY_ENTRY_EXPORT.symbols if e.name}

    details = {
        'scope': 'extracted production Font_InitSystem, Font_Unload, Font_Shutdown and complete mem.c; focused PE32 DLL; modeled sprite-release boundary',
        'commands': [[subprocess.list2cmdline(c) for c in commands]],
        'toolchain': {
            name: subprocess.check_output([shutil.which(name), '--version'], text=True).splitlines()[0]
            for name in ('clang', 'lld-link')
        },
        'limitations': 'Full geputget.c and native game blocked by legacy dependencies; instruction equality unclaimed'
    }

    # Record compilation
    record(RVA_INIT, 'compilation', 'pass', **details)
    code = original.get_data(RVA_INIT, SIZE_INIT)
    prefix = rebuilt.get_data(symbols['Font_InitSystem'] - rebuilt.OPTIONAL_HEADER.ImageBase, SIZE_INIT)
    record(RVA_INIT, 'raw_bytes', 'pass' if code == prefix else 'different',
           details={'original_bytes': SIZE_INIT, 'compiled_prefix_bytes': SIZE_INIT,
                    'scope': 'diagnostic prefix only; instruction equality unclaimed'})
    verify_font_init(original, rebuilt, symbols, details)
    record(RVA_UNLOAD, 'compilation', 'pass', **details)
    record(RVA_SHUTDOWN, 'compilation', 'pass', **details)

    # Diagnostic raw code prefix equality
    code_unload_orig = original.get_data(RVA_UNLOAD, SIZE_UNLOAD)
    code_unload_rebuilt = rebuilt.get_data(symbols['Font_Unload'] - rebuilt.OPTIONAL_HEADER.ImageBase, SIZE_UNLOAD)
    record(RVA_UNLOAD, 'raw_bytes', 'pass' if code_unload_orig == code_unload_rebuilt else 'different',
           details={'original_bytes': SIZE_UNLOAD, 'compiled_prefix_bytes': SIZE_UNLOAD,
                    'scope': 'diagnostic prefix only; instruction equality unclaimed'})

    code_shutdown_orig = original.get_data(RVA_SHUTDOWN, SIZE_SHUTDOWN)
    code_shutdown_rebuilt = rebuilt.get_data(symbols['Font_Shutdown'] - rebuilt.OPTIONAL_HEADER.ImageBase, SIZE_SHUTDOWN)
    record(RVA_SHUTDOWN, 'raw_bytes', 'pass' if code_shutdown_orig == code_shutdown_rebuilt else 'different',
           details={'original_bytes': SIZE_SHUTDOWN, 'compiled_prefix_bytes': SIZE_SHUTDOWN,
                    'scope': 'diagnostic prefix only; instruction equality unclaimed'})

    # 1. Font_Unload verification
    unload_cases = 0
    rng = random.Random(0x456470)

    # Test all 30 font slots with sparse glyphs
    for slot in range(30):
        fonts_state = bytearray(48000)
        struct.pack_into('<I', fonts_state, slot * 1600, 1)
        active_glyphs = [5, 12, 65, 128, 200]
        for g in active_glyphs:
            fonts_state[slot * 1600 + 30 + g] = 1
            struct.pack_into('<I', fonts_state, slot * 1600 + 256 + g * 4, 0x1000 * (g + 1) + slot)
        # Test non-1 glyph present values to verify strict check
        fonts_state[slot * 1600 + 30 + 1] = 2
        fonts_state[slot * 1600 + 30 + 2] = 0xff

        a = execute_unload(original, 0x456470, 0x63f2e0, 0, slot, fonts_state, unload_cases, original=True)
        b = execute_unload(rebuilt, symbols['Font_Unload'], symbols['g_fonts'], symbols['Gfx_SpriteOp'],
                           slot, fonts_state, unload_cases, original=False)
        assert a == b, f"Font_Unload mismatch at slot {slot}"
        assert a[0] == 1
        expected_calls = [(0, 0x1000 * (g + 1) + slot) for g in active_glyphs]
        assert a[2] == expected_calls
        assert struct.unpack_from('<I', a[1], slot * 1600)[0] == 0
        unload_cases += 1

    # Empty font (0 glyphs present) across all slots
    for slot in range(30):
        fonts_state = bytearray(48000)
        struct.pack_into('<I', fonts_state, slot * 1600, 1)
        a = execute_unload(original, 0x456470, 0x63f2e0, 0, slot, fonts_state, unload_cases, original=True)
        b = execute_unload(rebuilt, symbols['Font_Unload'], symbols['g_fonts'], symbols['Gfx_SpriteOp'],
                           slot, fonts_state, unload_cases, original=False)
        assert a == b
        assert a[0] == 1
        assert a[2] == []
        assert struct.unpack_from('<I', a[1], slot * 1600)[0] == 0
        unload_cases += 1

    # Full font (all 224 glyphs present)
    for slot in [0, 14, 29]:
        fonts_state = bytearray(48000)
        struct.pack_into('<I', fonts_state, slot * 1600, 1)
        for g in range(224):
            fonts_state[slot * 1600 + 30 + g] = 1
            struct.pack_into('<I', fonts_state, slot * 1600 + 256 + g * 4, 0x5000 + g)
        a = execute_unload(original, 0x456470, 0x63f2e0, 0, slot, fonts_state, unload_cases, original=True)
        b = execute_unload(rebuilt, symbols['Font_Unload'], symbols['g_fonts'], symbols['Gfx_SpriteOp'],
                           slot, fonts_state, unload_cases, original=False)
        assert a == b
        assert a[0] == 1
        assert a[2] == [(0, 0x5000 + g) for g in range(224)]
        assert struct.unpack_from('<I', a[1], slot * 1600)[0] == 0
        unload_cases += 1

    # Randomized glyph patterns
    for _ in range(60):
        slot = rng.randrange(30)
        fonts_state = bytearray(rng.randbytes(48000))
        struct.pack_into('<I', fonts_state, slot * 1600, 1)
        expected_calls = []
        for g in range(224):
            present = rng.choice([0, 1, 2, 0xff])
            fonts_state[slot * 1600 + 30 + g] = present
            handle = rng.getrandbits(32)
            struct.pack_into('<I', fonts_state, slot * 1600 + 256 + g * 4, handle)
            if present == 1:
                expected_calls.append((0, handle))
        a = execute_unload(original, 0x456470, 0x63f2e0, 0, slot, fonts_state, unload_cases, original=True)
        b = execute_unload(rebuilt, symbols['Font_Unload'], symbols['g_fonts'], symbols['Gfx_SpriteOp'],
                           slot, fonts_state, unload_cases, original=False)
        assert a == b
        assert a[0] == 1
        assert a[2] == expected_calls
        assert struct.unpack_from('<I', a[1], slot * 1600)[0] == 0
        unload_cases += 1

    record(RVA_UNLOAD, 'emulation', 'pass', cases=unload_cases, **details)
    print(f"PASS: {unload_cases} Font_Unload differential emulation executions.")

    # 2. Font_Shutdown verification
    shutdown_cases = 0
    rng = random.Random(0x456210)

    # Uninitialized state: g_fontSystemInitialized == 0 -> returns 1020
    for sys_init_val in [0]:
        for handle_val in [0, 42, 0x12345678, 0xffffffff]:
            fonts_state = bytearray(rng.randbytes(48000))
            status_list = [rng.choice([0, 1, 2]) for _ in range(200)]
            id_list = [rng.getrandbits(32) for _ in range(200)]
            a = execute_shutdown(original, 0x456210, 0x63f2e0, 0x4ba6c4, 0x50e680,
                                 0x4bab38, 0x5116e0, 0x5113c0, 0,
                                 sys_init_val, handle_val, fonts_state, 1, status_list, id_list,
                                 shutdown_cases, original=True)
            b = execute_shutdown(rebuilt, symbols['Font_Shutdown'], symbols['g_fonts'],
                                 symbols['g_fontSystemInitialized'], symbols['g_fontSubsystemHandle'],
                                 symbols['g_memHandlesInitialized'], symbols['g_memHandleStatus'],
                                 symbols['g_memRegisteredHandleIds'], symbols['Gfx_SpriteOp'],
                                 sys_init_val, handle_val, fonts_state, 1, status_list, id_list,
                                 shutdown_cases, original=False)
            assert a == b
            assert a[0] == 1020
            assert a[1] == sys_init_val  # unchanged
            assert a[2] == handle_val    # unchanged
            assert a[3] == bytes(fonts_state)  # unchanged
            assert a[5] == status_list   # unchanged
            assert a[-1] == []           # no sprite calls
            shutdown_cases += 1

    # Initialized state: 0 active fonts
    for slot_to_release in [0, 10, 199, None]:
        fonts_state = bytearray(48000)  # all in_use == 0
        status_list = [1 if i == slot_to_release else 0 for i in range(200)]
        id_list = [42 if i == slot_to_release else 0 for i in range(200)]
        a = execute_shutdown(original, 0x456210, 0x63f2e0, 0x4ba6c4, 0x50e680,
                             0x4bab38, 0x5116e0, 0x5113c0, 0,
                             1, 42, fonts_state, 1, status_list, id_list,
                             shutdown_cases, original=True)
        b = execute_shutdown(rebuilt, symbols['Font_Shutdown'], symbols['g_fonts'],
                             symbols['g_fontSystemInitialized'], symbols['g_fontSubsystemHandle'],
                             symbols['g_memHandlesInitialized'], symbols['g_memHandleStatus'],
                             symbols['g_memRegisteredHandleIds'], symbols['Gfx_SpriteOp'],
                             1, 42, fonts_state, 1, status_list, id_list,
                             shutdown_cases, original=False)
        assert a == b
        assert a[0] == 1
        assert a[1] == 0  # sys_init reset to 0
        assert a[5] == [0] * 200  # released if present
        assert a[-1] == []  # no font unloads
        shutdown_cases += 1

    # Initialized state: all 30 active fonts
    fonts_state = bytearray(48000)
    expected_calls = []
    for slot in range(30):
        struct.pack_into('<I', fonts_state, slot * 1600, 1)
        fonts_state[slot * 1600 + 30 + 10] = 1
        handle = 0x8000 + slot
        struct.pack_into('<I', fonts_state, slot * 1600 + 256 + 10 * 4, handle)
        expected_calls.append((0, handle))
    status_list = [0] * 200
    status_list[50] = 1
    id_list = [0] * 200
    id_list[50] = 99
    a = execute_shutdown(original, 0x456210, 0x63f2e0, 0x4ba6c4, 0x50e680,
                         0x4bab38, 0x5116e0, 0x5113c0, 0,
                         1, 99, fonts_state, 1, status_list, id_list,
                         shutdown_cases, original=True)
    b = execute_shutdown(rebuilt, symbols['Font_Shutdown'], symbols['g_fonts'],
                         symbols['g_fontSystemInitialized'], symbols['g_fontSubsystemHandle'],
                         symbols['g_memHandlesInitialized'], symbols['g_memHandleStatus'],
                         symbols['g_memRegisteredHandleIds'], symbols['Gfx_SpriteOp'],
                         1, 99, fonts_state, 1, status_list, id_list,
                         shutdown_cases, original=False)
    assert a == b
    assert a[0] == 1
    assert a[1] == 0
    assert a[5][50] == 0
    assert a[-1] == expected_calls
    # All font in_use cleared
    for slot in range(30):
        assert struct.unpack_from('<I', a[3], slot * 1600)[0] == 0
    shutdown_cases += 1

    # Initialized state: randomized active font sets and glyphs
    for _ in range(60):
        fonts_state = bytearray(48000)
        expected_calls = []
        active_slots = [s for s in range(30) if rng.choice([True, False])]
        for slot in range(30):
            in_use = 1 if slot in active_slots else rng.choice([0, 2, 0xff])
            struct.pack_into('<I', fonts_state, slot * 1600, in_use)
            for g in range(rng.randrange(5)):
                g_idx = rng.randrange(224)
                fonts_state[slot * 1600 + 30 + g_idx] = 1
                h = rng.getrandbits(32)
                struct.pack_into('<I', fonts_state, slot * 1600 + 256 + g_idx * 4, h)
                if in_use == 1:
                    expected_calls.append((0, h))

        target_handle = rng.getrandbits(32)
        status_list = [rng.choice([0, 1, 2]) for _ in range(200)]
        id_list = [target_handle if rng.randrange(5) == 0 else rng.getrandbits(32) for _ in range(200)]
        mem_init_val = rng.choice([0, 1, 2])

        a = execute_shutdown(original, 0x456210, 0x63f2e0, 0x4ba6c4, 0x50e680,
                             0x4bab38, 0x5116e0, 0x5113c0, 0,
                             1, target_handle, fonts_state, mem_init_val, status_list, id_list,
                             shutdown_cases, original=True)
        b = execute_shutdown(rebuilt, symbols['Font_Shutdown'], symbols['g_fonts'],
                             symbols['g_fontSystemInitialized'], symbols['g_fontSubsystemHandle'],
                             symbols['g_memHandlesInitialized'], symbols['g_memHandleStatus'],
                             symbols['g_memRegisteredHandleIds'], symbols['Gfx_SpriteOp'],
                             1, target_handle, fonts_state, mem_init_val, status_list, id_list,
                             shutdown_cases, original=False)
        assert a == b
        assert a[0] == 1
        assert a[1] == 0
        shutdown_cases += 1

    # Repeat call idempotency: calling Font_Shutdown again after shutdown
    # First call resets g_fontSystemInitialized to 0; second call returns 1020
    first_res = execute_shutdown(original, 0x456210, 0x63f2e0, 0x4ba6c4, 0x50e680,
                                 0x4bab38, 0x5116e0, 0x5113c0, 0,
                                 1, 42, fonts_state, 1, [1]*200, [42]*200,
                                 shutdown_cases, original=True)
    second_res = execute_shutdown(original, 0x456210, 0x63f2e0, 0x4ba6c4, 0x50e680,
                                  0x4bab38, 0x5116e0, 0x5113c0, 0,
                                  first_res[1], first_res[2], first_res[3], first_res[4], first_res[5], first_res[6],
                                  shutdown_cases + 1, original=True)
    c_first_res = execute_shutdown(rebuilt, symbols['Font_Shutdown'], symbols['g_fonts'],
                                   symbols['g_fontSystemInitialized'], symbols['g_fontSubsystemHandle'],
                                   symbols['g_memHandlesInitialized'], symbols['g_memHandleStatus'],
                                   symbols['g_memRegisteredHandleIds'], symbols['Gfx_SpriteOp'],
                                   1, 42, fonts_state, 1, [1]*200, [42]*200,
                                   shutdown_cases, original=False)
    c_second_res = execute_shutdown(rebuilt, symbols['Font_Shutdown'], symbols['g_fonts'],
                                    symbols['g_fontSystemInitialized'], symbols['g_fontSubsystemHandle'],
                                    symbols['g_memHandlesInitialized'], symbols['g_memHandleStatus'],
                                    symbols['g_memRegisteredHandleIds'], symbols['Gfx_SpriteOp'],
                                    c_first_res[1], c_first_res[2], c_first_res[3], c_first_res[4], c_first_res[5], c_first_res[6],
                                    shutdown_cases + 1, original=False)
    assert first_res == c_first_res and first_res[0] == 1
    assert second_res == c_second_res and second_res[0] == 1020
    shutdown_cases += 2

    record(RVA_SHUTDOWN, 'emulation', 'pass', cases=shutdown_cases, **details)
    print(f"PASS: {shutdown_cases} Font_Shutdown differential emulation executions.")
    print("PASS: Font cleanup validation complete.")


if __name__ == '__main__':
    verify_font_cleanup()
