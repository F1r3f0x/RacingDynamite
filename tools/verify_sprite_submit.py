# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Complete authentic sprite submission adapter versus C89 renderer with DEV-007 rasterizer dispatch."""
import hashlib
import random
import shutil
import struct
import subprocess
from pathlib import Path

import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import (
    Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_WRITE, UC_HOOK_MEM_READ,
    UC_HOOK_MEM_INVALID, UcError
)
from unicorn.x86_const import (
    UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX, UC_X86_REG_EDX,
    UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_ESP,
    UC_X86_REG_EIP, UC_X86_REG_EFLAGS
)

from windows_target import ROOT, BUILD, TARGET, verify_target, validation_symbols
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')

DLL = BUILD / 'sprite_submit_validation.dll'
STACK = 0x7000000
STOP  = 0x7100000
ARENA = 0x7200000
DATA  = 0x7300000
FB    = ARENA + 0x20000
TEX   = ARENA + 0x40000
LUT   = ARENA + 0x60000

ROUTINE_NAME = 'Gfx_SubmitSpriteRequest'
ROUTINE_RVA = 0x57370
ROUTINE_SIZE = 167
ROUTINE_SHA256 = '203cc8648be172994ad229e423e9f05c83ff3a5d541277de7e9864ac47ae79fe'

TARGET_DESCRIPTOR_CALL = 0x4612E0
TARGET_RASTERIZER_CALL = 0x465BB5

PRESERVED = [UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP]

INPUTS = [
    'decomp/src/geputget.c',
    'decomp/include/geputget.h',
    'decomp/src/renderer.c',
    'decomp/include/renderer.h',
    'decomp/modules.json',
    'decomp/target.json',
    'tools/windows_target.py',
    'tools/windows_tracking.py',
    'tools/verify_sprite_submit.py'
]


def inspect_original(pe):
    code = pe.get_data(ROUTINE_RVA, ROUTINE_SIZE)
    assert hashlib.sha256(code).hexdigest() == ROUTINE_SHA256, 'Original routine hash mismatch'
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = list(md.disasm(code, 0x400000 + ROUTINE_RVA))
    assert sum(i.size for i in instructions) == ROUTINE_SIZE, 'Instruction size mismatch'

    expected_relocs = [
        (0x57374, 0x50EBFC), (0x5737D, 0x4BA77C), (0x57386, 0x4BA784),
        (0x573A2, 0x4BA758), (0x573B6, 0x4BA75C), (0x573BE, 0x4BA760),
        (0x573CA, 0x4BA764), (0x573DD, 0x4BA760), (0x573E3, 0x4BA770),
        (0x573E8, 0x4BA768), (0x573F5, 0x4BA764), (0x573FA, 0x4BA76C),
        (0x573FF, 0x63F2D8), (0x57404, 0x4BA774), (0x57409, 0x4BA778)
    ]
    relocs = [
        (r.rva, struct.unpack('<I', pe.get_data(r.rva, 4))[0])
        for entry in pe.DIRECTORY_ENTRY_BASERELOC for r in entry.entries
        if ROUTINE_RVA <= r.rva < ROUTINE_RVA + ROUTINE_SIZE
    ]
    assert relocs == expected_relocs, f'Relocation mismatch: {relocs} != {expected_relocs}'
    assert pe.get_data(0x57417, 9) == b'\xcc' * 9

    calls = [i for i in instructions if i.mnemonic == 'call']
    assert len(calls) == 2, 'Expected 2 calls in adapter'
    assert int(calls[0].op_str, 16) == TARGET_DESCRIPTOR_CALL
    assert int(calls[1].op_str, 16) == TARGET_RASTERIZER_CALL


def build():
    BUILD.mkdir(parents=True, exist_ok=True)
    gcc, linker = (shutil.which(n) for n in ('gcc', 'lld-link'))
    if not gcc or not linker:
        raise RuntimeError('Installed provisional GCC and LLD required')

    obj_renderer = BUILD / (DLL.stem + '_renderer.obj')
    commands = [
        [
            gcc, '-m32', '-std=c89', '-pedantic-errors', '-Wall', '-Wextra', '-Werror',
            '-O2', '-fno-inline', '-fno-optimize-sibling-calls',
            '-I', str(ROOT / 'decomp/include'),
            '-c', str(ROOT / 'decomp/src/renderer.c'),
            '-o', str(obj_renderer)
        ]
    ]

    legacy = (ROOT / 'decomp/src/geputget.c').read_text(encoding='utf-8')
    storage = legacy[
        legacy.index('volatile uint32_t g_spriteWorkspaceAAllocated'):
        legacy.index('/*\n * @original Gfx_InitSpriteWorkspaceA')
    ]
    desc_copy = legacy[
        legacy.index('/*\n * @original Gfx_CopySpriteDescriptor (IGN_WIN.EXE @ 0x004612E0'):
        legacy.index('/*\n * @original Gfx_InitSpriteHandles')
    ]

    helper_source = BUILD / (DLL.stem + '_helper.c')
    helper_obj = BUILD / (DLL.stem + '_helper.obj')
    helper_code = (
        '#include "geputget.h"\n'
        '#include "renderer.h"\n\n'
        + storage + '\n'
        'volatile GfxSpriteRequest *volatile g_activeSpriteRequest = 0;\n'
        'volatile GfxSpriteDescriptor g_nativeSpriteDefault = {{0}};\n'
        'volatile GfxPointerTable g_nativeImageControl = {{0, 0}, 0, (uint32_t *)0};\n\n'
        + desc_copy + '\n'
    )
    helper_source.write_text(helper_code, encoding='utf-8', newline='\n')

    commands.append([
        gcc, '-m32', '-std=c89', '-pedantic-errors', '-Wall', '-Wextra', '-Werror',
        '-O2', '-fno-inline', '-fno-optimize-sibling-calls',
        '-I', str(ROOT / 'decomp/include'),
        '-c', str(helper_source),
        '-o', str(helper_obj)
    ])

    exports = [
        'Gfx_SubmitSpriteRequest',
        'Gfx_RasterizeSpritePacket',
        'Gfx_PrepareTransformedSprite',
        'Gfx_CopySpriteDescriptor',
        'g_spritePreparedStorage',
        'g_spriteRasterPacket',
        'g_spriteRenderLookup',
        'g_activeSpriteRequest',
        'g_nativeSpriteDefault',
        'g_nativeImageControl',
        'g_spriteBlitSource',
        'g_spriteBlitWidth',
        'g_spriteBlitHeight',
        'g_transformedSpriteInput',
        'g_transformedSpriteScratch',
        'g_triangleViewportState',
        'g_triangleViewportFramebuffer'
    ]

    link_cmd = [
        linker, '/dll', '/noentry', '/nodefaultlib', '/machine:x86', '/safeseh:no',
        '/base:0x10000000', '/out:' + str(DLL),
        str(obj_renderer), str(helper_obj)
    ] + ['/export:' + n for n in sorted(set(exports))]

    commands.append(link_cmd)

    for command in commands:
        subprocess.run(command, cwd=ROOT, check=True)
    return commands


def run_session(pe, is_orig, symbols, count, image_id, empty, mutation, descriptor, fallback,
                pos_x, pos_y, hx, hy, render_word, coeff_vals, seed, stop_at_rasterizer=False):
    base = pe.OPTIONAL_HEADER.ImageBase
    size = (pe.OPTIONAL_HEADER.SizeOfImage + 0xfff) & ~0xfff
    uc = Uc(UC_ARCH_X86, UC_MODE_32)
    uc.mem_map(base, size)
    uc.mem_write(base, pe.get_memory_mapped_image())
    uc.mem_map(STACK, 0x10000)
    uc.mem_map(STOP, 0x1000)
    uc.mem_map(ARENA, 0x80000)
    uc.mem_map(DATA, 0x10000)

    FB = ARENA + 0x20000
    TEX = ARENA + 0x40000
    LUT = ARENA + 0x60000

    rng = random.Random(seed)
    fb_init = bytes(rng.randbytes(320 * 240))
    tex_init = bytes(rng.randbytes(0x10000))
    lut_init = bytes(rng.randbytes(0x10000))

    uc.mem_write(FB, fb_init)
    uc.mem_write(TEX, tex_init)
    uc.mem_write(LUT, lut_init)

    viewport = [0, 200, 10, 300, 20, 0, 320, 0, 0]
    viewport_addr = 0x63F2B0 if is_orig else symbols['g_triangleViewportState']
    fb_ptr_addr = 0x63F2D4 if is_orig else symbols['g_triangleViewportFramebuffer']
    uc.mem_write(viewport_addr, struct.pack('<9i', *viewport))
    uc.mem_write(fb_ptr_addr, struct.pack('<I', FB))

    pos_addr = DATA + 0x20
    handle_addr = DATA + 0x100
    request_addr = DATA + 0x200
    coeff_addr = (DATA + 0x300) if coeff_vals else 0
    table_addr = DATA + 0x400
    desc_addr = DATA + 0x500

    uc.mem_write(pos_addr, struct.pack('<2i', pos_x, pos_y))
    uc.mem_write(handle_addr, struct.pack('<i2I', image_id, hx, hy))
    if coeff_vals:
        uc.mem_write(coeff_addr, struct.pack('<4i', *coeff_vals))
    uc.mem_write(request_addr, struct.pack('<3I', pos_addr, handle_addr, coeff_addr))

    active_req_addr = 0x50EBFC if is_orig else symbols['g_activeSpriteRequest']
    uc.mem_write(active_req_addr, struct.pack('<I', request_addr))

    render_lookup_addr = 0x63F2D8 if is_orig else symbols['g_spriteRenderLookup']
    uc.mem_write(render_lookup_addr, struct.pack('<I', render_word))

    fallback_addr = 0x51FB88 if is_orig else symbols['g_nativeSpriteDefault']
    uc.mem_write(fallback_addr, fallback)

    uc.mem_write(desc_addr, descriptor)
    table_slot = 0 if empty else desc_addr
    uc.mem_write(table_addr, struct.pack('<3I', 0, table_slot, desc_addr))

    control_addr = 0x520380 if is_orig else symbols['g_nativeImageControl']
    uc.mem_write(control_addr + 8, struct.pack('<iI', count, table_addr))

    sp = STACK + 0x8000
    uc.mem_write(sp, struct.pack('<I', STOP))
    uc.reg_write(UC_X86_REG_ESP, sp)
    uc.reg_write(UC_X86_REG_EFLAGS, 0x202)

    saved_vals = [rng.getrandbits(32) for _ in PRESERVED]
    for reg, val in zip(PRESERVED, saved_vals):
        uc.reg_write(reg, val)

    entry = (0x400000 + ROUTINE_RVA) if is_orig else symbols['Gfx_SubmitSpriteRequest']

    events = []
    writes = []
    faults = []

    def code_hook(uc, address, size, user_data):
        desc_entry = TARGET_DESCRIPTOR_CALL if is_orig else symbols['Gfx_CopySpriteDescriptor']
        if address == desc_entry and mutation:
            new_hx, new_hy = hx ^ 0xFFFFFFFF, hy ^ 0x80000000
            uc.mem_write(handle_addr + 4, struct.pack('<2I', new_hx, new_hy))
            uc.mem_write(request_addr, struct.pack('<3I', 0, DATA + 0x800, 0))
            uc.mem_write(active_req_addr, struct.pack('<I', DATA + 0x800))

        if is_orig:
            if address == TARGET_RASTERIZER_CALL:
                esi = uc.reg_read(UC_X86_REG_ESI)
                events.append(('rasterizer_call', esi))
                if stop_at_rasterizer:
                    uc.reg_write(UC_X86_REG_EIP, STOP)
                    uc.emu_stop()
            elif address == 0x468C70:
                events.append(('transformed_call', uc.reg_read(UC_X86_REG_ESP)))
                uc.reg_write(UC_X86_REG_EIP, STOP)
                uc.emu_stop()
        else:
            if address == symbols['Gfx_RasterizeSpritePacket']:
                esp = uc.reg_read(UC_X86_REG_ESP)
                packet_arg = struct.unpack('<I', uc.mem_read(esp + 4, 4))[0]
                events.append(('rasterizer_call', packet_arg))
                if stop_at_rasterizer:
                    uc.reg_write(UC_X86_REG_EIP, STOP)
                    uc.emu_stop()
            elif address == symbols['Gfx_PrepareTransformedSprite']:
                events.append(('transformed_call', uc.reg_read(UC_X86_REG_ESP)))
                uc.reg_write(UC_X86_REG_EIP, STOP)
                uc.emu_stop()

    def write_hook(uc, access, address, size, value, user_data):
        if STACK <= address < STACK + 0x10000:
            return
        writes.append((address, size, value))

    def invalid_hook(uc, access, address, size, value, user_data):
        faults.append((access, address, size))
        uc.reg_write(UC_X86_REG_EIP, STOP)
        uc.emu_stop()
        return True

    uc.hook_add(UC_HOOK_CODE, code_hook)
    uc.hook_add(UC_HOOK_MEM_WRITE, write_hook)
    uc.hook_add(UC_HOOK_MEM_INVALID, invalid_hook)

    try:
        uc.emu_start(entry, STOP, count=200000)
    except UcError:
        pass

    reg_state = [uc.reg_read(r) for r in PRESERVED]
    esp_final = uc.reg_read(UC_X86_REG_ESP)

    packet_addr = 0x4BA778 if is_orig else symbols['g_spriteRasterPacket']
    storage_addr = 0x4BA758 if is_orig else symbols['g_spritePreparedStorage']
    packet_data = uc.mem_read(packet_addr, 16)
    storage_data = uc.mem_read(storage_addr, 32)
    fb_data = uc.mem_read(FB, 320 * 240)

    return {
        'events': events,
        'writes': writes,
        'faults': faults,
        'packet': struct.unpack('<4I', packet_data),
        'storage': struct.unpack('<8I', storage_data),
        'regs': reg_state,
        'esp': esp_final,
        'fb': hashlib.sha256(fb_data).hexdigest()
    }


def verify_sprite_submit(record=True):
    pe = pefile.PE(data=verify_target())
    inspect_original(pe)
    commands = build()

    dll_pe = pefile.PE(str(DLL))
    symbols = validation_symbols(dll_pe)

    cases_count = 0
    rng = random.Random(0x57370)

    # 1. 168 exhaustive adapter lookup trials (mirroring original analyze_sprite_adapter.py)
    for count in (-2147483648, -1, 0, 1, 2, 3):
        for image_id in (-2147483648, -1, 0, 1, 2, 3, 2147483647):
            for empty in (False, True):
                for mutation in (False, True):
                    descriptor = rng.randbytes(64)
                    fallback = rng.randbytes(64)
                    pos_x = rng.randint(-100, 400)
                    pos_y = rng.randint(-100, 300)
                    hx = rng.getrandbits(32)
                    hy = rng.getrandbits(32)
                    render_word = LUT
                    coeff = None

                    orig = run_session(pe, True, symbols, count, image_id, empty, mutation,
                                       descriptor, fallback, pos_x, pos_y, hx, hy, render_word,
                                       coeff, cases_count, stop_at_rasterizer=True)
                    comp = run_session(dll_pe, False, symbols, count, image_id, empty, mutation,
                                       descriptor, fallback, pos_x, pos_y, hx, hy, render_word,
                                       coeff, cases_count, stop_at_rasterizer=True)

                    assert len(orig['events']) == 1 and len(comp['events']) == 1
                    assert orig['events'][0][0] == 'rasterizer_call'
                    assert comp['events'][0][0] == 'rasterizer_call'
                    assert orig['events'][0][1] == 0x4BA778
                    assert comp['events'][0][1] == symbols['g_spriteRasterPacket']

                    # Prepared storage must match exactly
                    assert orig['storage'] == comp['storage'], f'Trial {cases_count}: prepared storage mismatch'
                    # Packet structure: type, position, prepared, coefficients
                    assert orig['packet'][0] == comp['packet'][0] == 4
                    assert orig['packet'][3] == comp['packet'][3] == 0

                    cases_count += 1

    # 2. 32 transformed sprite trials (coefficients present)
    for t in range(32):
        count = 10
        image_id = 1
        empty = False
        mutation = False
        descriptor = rng.randbytes(64)
        fallback = rng.randbytes(64)
        pos_x = rng.randint(-100, 400)
        pos_y = rng.randint(-100, 300)
        hx = rng.getrandbits(32)
        hy = rng.getrandbits(32)
        render_word = LUT
        coeff = [rng.randint(-0x10000, 0x10000) for _ in range(4)]

        orig = run_session(pe, True, symbols, count, image_id, empty, mutation,
                           descriptor, fallback, pos_x, pos_y, hx, hy, render_word,
                           coeff, cases_count)
        comp = run_session(dll_pe, False, symbols, count, image_id, empty, mutation,
                           descriptor, fallback, pos_x, pos_y, hx, hy, render_word,
                           coeff, cases_count)

        assert orig['events'][0][0] == 'rasterizer_call'
        assert comp['events'][0][0] == 'rasterizer_call'
        assert orig['events'][1][0] == 'transformed_call'
        assert comp['events'][1][0] == 'transformed_call'
        assert orig['storage'] == comp['storage'], f'Transformed trial {t}: storage mismatch'
        assert orig['packet'][0] == comp['packet'][0] == 4
        assert orig['packet'][3] == comp['packet'][3] != 0

        cases_count += 1

    # 3. 64 valid full-pipeline untransformed blits (valid mapped texture and dimensions)
    for t in range(64):
        count = 10
        image_id = 1
        empty = False
        mutation = False
        desc_words = [rng.getrandbits(32) for _ in range(16)]
        width = rng.randint(1, 32)
        height = rng.randint(1, 32)
        desc_words[1] = width
        desc_words[2] = height
        desc_words[4] = TEX
        descriptor = struct.pack('<16I', *desc_words)
        fallback = rng.randbytes(64)
        pos_x = rng.randint(30, 200) << 8
        pos_y = rng.randint(20, 150) << 8
        hx = 0
        hy = 0
        render_word = LUT
        coeff = None

        orig = run_session(pe, True, symbols, count, image_id, empty, mutation,
                           descriptor, fallback, pos_x, pos_y, hx, hy, render_word,
                           coeff, cases_count)
        comp = run_session(dll_pe, False, symbols, count, image_id, empty, mutation,
                           descriptor, fallback, pos_x, pos_y, hx, hy, render_word,
                           coeff, cases_count)

        assert len(orig['faults']) == 0 and len(comp['faults']) == 0, f'Valid blit {t} faulted'
        assert orig['regs'] == comp['regs'], f'Valid blit {t}: callee-saved registers mismatch'
        assert orig['esp'] == comp['esp'] == STACK + 0x8004, f'Valid blit {t}: ESP mismatch'
        assert orig['storage'] == comp['storage'], f'Valid blit {t}: storage mismatch'
        assert orig['packet'][0] == comp['packet'][0] == 4
        assert orig['packet'][1] == comp['packet'][1]
        assert orig['packet'][2] == 0x4BA758 and comp['packet'][2] == symbols['g_spritePreparedStorage']
        assert orig['packet'][3] == comp['packet'][3] == 0
        assert orig['fb'] == comp['fb'], f'Valid blit {t}: Framebuffer mismatch'

        cases_count += 1

    coverage = {
        'trials': cases_count,
        'adapter_lookup_trials': 168,
        'transformed_trials': 32,
        'full_pipeline_blits': 64
    }

    if record:
        for kind in ('compilation', 'emulation'):
            record_run(ROUTINE_RVA, kind, 'pass', inputs=INPUTS,
                       artifact=DLL.relative_to(ROOT).as_posix(),
                       cases=cases_count if kind == 'emulation' else 0,
                       command='uv run tools/verify_sprite_submit.py',
                       details={
                           'coverage': coverage,
                           'commands': commands,
                           'scope': 'Authentic sprite request submission; descriptor lookup, packet preparation and DEV-007 rasterizer dispatch verified differentially against IGN_WIN.EXE',
                           'limitations': 'DEV-007 rasterizer ESI register call adapted to C pointer parameter; full 3D scene compositing verified separately'
                       })

    print(f'PASS sprite submit adapter: {cases_count} trials ({coverage["adapter_lookup_trials"]} lookup/blits, {coverage["transformed_trials"]} transformed)', flush=True)
    return coverage


if __name__ == '__main__':
    verify_sprite_submit(record=True)
