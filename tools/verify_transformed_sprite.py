# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Complete authentic transformed sprite preparation routine versus C89 renderer."""
import hashlib
import math
import random
import struct
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import (
    UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX, UC_X86_REG_EDX,
    UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_ESP,
    UC_X86_REG_EIP, UC_X86_REG_EFLAGS
)
from verify_texture_spans import build, validate_renderer_calls, STACK, STOP, ARENA, PRESERVED, INPUTS as SPAN_INPUTS
from windows_target import ROOT, BUILD, TARGET, verify_target, validation_symbols
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')

DLL = BUILD / 'transformed_sprite_validation.dll'

ROUTINE_NAME = 'Gfx_PrepareTransformedSprite'
ROUTINE_RVA = 0x68C70
ROUTINE_SIZE = 631
ROUTINE_SHA256 = 'c239f5eb8a12e6b59eb775b316f45e4d8c37ad81f2c0b66910cda5033f8f96b2'

TARGET_CALL_RVA = 0x5CA50  # Gfx_RasterizeTexturedTriangle

GLOBALS = {
    'g_transformedSpriteScratch': (0x4baf1c, 116)
}

INPUTS = SPAN_INPUTS + ['tools/verify_transformed_sprite.py']


def inspect_original(pe):
    code = pe.get_data(ROUTINE_RVA, ROUTINE_SIZE)
    assert hashlib.sha256(code).hexdigest() == ROUTINE_SHA256, 'Original routine hash mismatch'
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = list(md.disasm(code, 0x400000 + ROUTINE_RVA))
    assert sum(i.size for i in instructions) == ROUTINE_SIZE, 'Instruction size mismatch'
    relocs = [r.rva for entry in pe.DIRECTORY_ENTRY_BASERELOC for r in entry.entries
              if ROUTINE_RVA <= r.rva < ROUTINE_RVA + ROUTINE_SIZE]
    assert len(relocs) == 68, f'Expected 68 relocations, got {len(relocs)}'
    for r in relocs:
        target = struct.unpack('<I', pe.get_data(r, 4))[0]
        assert 0x4baf1c <= target <= 0x4baf54, f'Unexpected relocation target {target:#x}'
    calls = [i for i in instructions if i.mnemonic == 'call']
    assert len(calls) == 2, 'Expected 2 calls in original routine'
    for c in calls:
        target = int(c.op_str, 16)
        assert target == 0x400000 + TARGET_CALL_RVA, f'Unexpected call target {target:#x}'


def run_session(pe, is_orig, symbols, input_vals, prepared_vals, coeff_vals, seed):
    base = pe.OPTIONAL_HEADER.ImageBase
    size = (pe.OPTIONAL_HEADER.SizeOfImage + 0xfff) & ~0xfff
    uc = Uc(UC_ARCH_X86, UC_MODE_32)
    uc.mem_map(base, size)
    uc.mem_write(base, pe.get_memory_mapped_image())
    uc.mem_map(STACK, 0x10000)
    uc.mem_map(STOP, 0x1000)
    uc.mem_map(ARENA, 0x10000)

    scratch_addr = 0x4baf1c if is_orig else symbols['g_transformedSpriteScratch']
    entry = (0x400000 + ROUTINE_RVA) if is_orig else symbols[ROUTINE_NAME]
    target_call = (0x400000 + TARGET_CALL_RVA) if is_orig else symbols['Gfx_RasterizeTexturedTriangle']

    prep_addr = ARENA + 0x200
    coeff_addr = ARENA + 0x300
    input_addr = ARENA + 0x100

    uc.mem_write(prep_addr, struct.pack('<8I', *[v & 0xffffffff for v in prepared_vals]))
    uc.mem_write(coeff_addr, struct.pack('<4I', *[v & 0xffffffff for v in coeff_vals]))
    inp_words = [input_vals[0], prep_addr, coeff_addr, input_vals[3], input_vals[4]]
    uc.mem_write(input_addr, struct.pack('<5I', *[v & 0xffffffff for v in inp_words]))

    calls = []

    def call_hook(u, addr, sz, user_data=None):
        (void_addr, void_sz, void_ud) = (addr, sz, user_data)
        esp = u.reg_read(UC_X86_REG_ESP)
        ret = struct.unpack('<I', u.mem_read(esp, 4))[0]
        arg0 = struct.unpack('<I', u.mem_read(esp + 4, 4))[0]
        packet = bytes(u.mem_read(arg0, 60))
        scratch = bytes(u.mem_read(scratch_addr, 116))
        calls.append((arg0 - scratch_addr, packet, scratch))
        u.reg_write(UC_X86_REG_ESP, esp + 4)
        u.reg_write(UC_X86_REG_EIP, ret)

    uc.hook_add(UC_HOOK_CODE, call_hook, begin=target_call, end=target_call)

    rng = random.Random(seed)
    saved = [rng.getrandbits(32) for _ in PRESERVED]
    for r, v in zip(PRESERVED, saved):
        uc.reg_write(r, v)

    sp = STACK + 0x8000
    if is_orig:
        uc.mem_write(sp, struct.pack('<I', STOP))
        uc.reg_write(UC_X86_REG_ESI, input_addr)
    else:
        uc.mem_write(sp, struct.pack('<2I', STOP, input_addr))

    caller_stack = bytes(uc.mem_read(sp, STACK + 0x10000 - sp))
    uc.reg_write(UC_X86_REG_ESP, sp)
    uc.reg_write(UC_X86_REG_EFLAGS, 2)

    image_before = bytes(uc.mem_read(base, size))

    uc.emu_start(entry, STOP, count=100000)

    assert uc.reg_read(UC_X86_REG_EIP) == STOP
    assert uc.reg_read(UC_X86_REG_ESP) == sp + 4
    assert (uc.reg_read(UC_X86_REG_EFLAGS) & 0x400) == 0

    if is_orig:
        assert uc.reg_read(UC_X86_REG_EDI) == saved[2], 'Original EDI clobbered'
    else:
        assert [uc.reg_read(r) for r in PRESERVED] == saved, 'Compiled preserved registers clobbered'

    assert bytes(uc.mem_read(sp, STACK + 0x10000 - sp)) == caller_stack

    image_after = bytearray(uc.mem_read(base, size))
    # Scratch state is the only modified region in the image
    scratch_offset = scratch_addr - base
    image_after[scratch_offset:scratch_offset + 116] = image_before[scratch_offset:scratch_offset + 116]
    assert bytes(image_after) == image_before, 'Unrelated image write detected'

    final_scratch = bytes(uc.mem_read(scratch_addr, 116))
    return calls, final_scratch


def verify_transformed_sprite(record=False):
    target_data = verify_target()
    original_pe = pefile.PE(data=target_data)
    inspect_original(original_pe)

    commands = build(dll=DLL, exports=[ROUTINE_NAME, *GLOBALS.keys()])
    compiled_pe = pefile.PE(str(DLL))
    assert not getattr(compiled_pe, 'DIRECTORY_ENTRY_IMPORT', [])
    validate_renderer_calls(compiled_pe)
    symbols = validation_symbols(compiled_pe)

    trials = []

    # Identity transform
    trials.append((
        [0, 0, 0, 0, 0],
        [0, 0, 0, 0, 64, 64, 0x10000000, 0x20000000],
        [0x10000, 0, 0, 0x10000]
    ))
    # Offset with position
    trials.append((
        [0, 0, 0, 150, -80],
        [16, 24, 10, 20, 80, 100, 0x12345678, 0x9abcdef0],
        [0x10000, 0, 0, 0x10000]
    ))

    # Standard rotations (angles 0, 30, 45, 90, 180, 270)
    for deg in (0, 30, 45, 60, 90, 120, 135, 180, 225, 270, 315):
        rad = math.radians(deg)
        c = int(math.cos(rad) * 65536.0)
        s = int(math.sin(rad) * 65536.0)
        trials.append((
            [0, 0, 0, 320, 240],
            [32, 32, 0, 0, 64, 64, 0x55555555, 0x66666666],
            [c, s, -s, c]
        ))

    # Scaling with rotation
    for scale in (0.5, 1.5, 2.0, 0.25):
        rad = math.radians(45)
        c = int(math.cos(rad) * scale * 65536.0)
        s = int(math.sin(rad) * scale * 65536.0)
        trials.append((
            [0, 0, 0, -50, 400],
            [20, 15, 5, 12, 100, 80, 0x11111111, 0x22222222],
            [c, s, -s, c]
        ))

    # Randomized trials
    rng = random.Random(0x68C70)
    for _ in range(48):
        pos_x = rng.randint(-2000, 2000)
        pos_y = rng.randint(-2000, 2000)
        ox = rng.randint(-200, 200)
        oy = rng.randint(-200, 200)
        u0 = rng.randint(0, 255)
        v0 = rng.randint(0, 255)
        u1 = rng.randint(u0, 512)
        v1 = rng.randint(v0, 512)
        tex = rng.getrandbits(32) & 0xfffffffc
        lut = rng.getrandbits(32) & 0xfffffffc
        c0 = rng.randint(-0x20000, 0x20000)
        c1 = rng.randint(-0x20000, 0x20000)
        c2 = rng.randint(-0x20000, 0x20000)
        c3 = rng.randint(-0x20000, 0x20000)
        trials.append((
            [0, 0, 0, pos_x, pos_y],
            [ox, oy, u0, v0, u1, v1, tex, lut],
            [c0, c1, c2, c3]
        ))

    cases_count = len(trials)
    for seed, (inp, prep, coeff) in enumerate(trials):
        orig_calls, orig_scratch = run_session(original_pe, True, symbols, inp, prep, coeff, seed)
        comp_calls, comp_scratch = run_session(compiled_pe, False, symbols, inp, prep, coeff, seed)

        assert len(orig_calls) == 2, f'Expected 2 calls, got {len(orig_calls)}'
        assert orig_calls == comp_calls, f'Trial {seed}: Triangle rasterizer call packets mismatch'
        assert orig_scratch == comp_scratch, f'Trial {seed}: Final scratch buffer mismatch'

    coverage = {
        'trials': cases_count,
        'paired_triangle_calls': cases_count * 2,
        'verified_scratch_bytes': 116
    }

    if record:
        for kind in ('compilation', 'emulation'):
            record_run(ROUTINE_RVA, kind, 'pass', inputs=INPUTS,
                       artifact=DLL.relative_to(ROOT).as_posix(),
                       cases=cases_count if kind == 'emulation' else 0,
                       command='uv run tools/verify_transformed_sprite.py',
                       details={
                           'coverage': coverage,
                           'commands': commands,
                           'scope': 'Authentic transformed sprite quad preparation and triangle submission; differential testing against IGN_WIN.EXE with DEV-007 calling convention adaptation',
                           'limitations': 'DEV-007 input pointer register to stack adaptation; downstream software rasterization mocked via call hook'
                       })

    print(f'PASS transformed sprite preparation: {cases_count} trials, {cases_count * 2} verified triangle submissions', flush=True)
    return coverage


if __name__ == '__main__':
    verify_transformed_sprite(record=True)
