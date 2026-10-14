# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Complete authentic sprite packet rasterizer versus C89 renderer with an explicit transformed-call boundary."""
import hashlib
import math
import random
import struct
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_WRITE, UC_HOOK_MEM_READ, UC_HOOK_MEM_INVALID, UcError
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

DLL = BUILD / 'sprite_packet_validation.dll'

ROUTINE_NAME = 'Gfx_RasterizeSpritePacket'
ROUTINE_RVA = 0x65BB5
ROUTINE_SIZE = 474
ROUTINE_SHA256 = '2cdb543878a2ce01c034f50d8f679a881f1491d088a64746b18ce6273d464fbb'

TARGET_CALL_RVA = 0x68C70  # Gfx_PrepareTransformedSprite

GLOBALS = {
    'g_spriteBlitSource': (0x4bad28, 4),
    'g_spriteBlitWidth': (0x4bad2c, 4),
    'g_spriteBlitHeight': (0x4bad30, 4),
    'g_transformedSpriteInput': (0x4bad34, 20)
}

INPUTS = SPAN_INPUTS + ['tools/verify_sprite_packet.py']


def inspect_original(pe):
    code = pe.get_data(ROUTINE_RVA, ROUTINE_SIZE)
    assert hashlib.sha256(code).hexdigest() == ROUTINE_SHA256, 'Original routine hash mismatch'
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = list(md.disasm(code, 0x400000 + ROUTINE_RVA))
    assert sum(i.size for i in instructions) == ROUTINE_SIZE, 'Instruction size mismatch'
    relocs = [r.rva for entry in pe.DIRECTORY_ENTRY_BASERELOC for r in entry.entries
              if ROUTINE_RVA <= r.rva < ROUTINE_RVA + ROUTINE_SIZE]
    assert len(relocs) == 31, f'Expected 31 relocations, got {len(relocs)}'
    for r in relocs:
        target = struct.unpack('<I', pe.get_data(r, 4))[0]
        assert (0x4bad28 <= target <= 0x4bad44) or (0x63f2b4 <= target <= 0x63f2d4), f'Unexpected relocation target {target:#x}'
    calls = [i for i in instructions if i.mnemonic == 'call']
    assert len(calls) == 1, 'Expected 1 call in original routine'
    assert int(calls[0].op_str, 16) == 0x400000 + TARGET_CALL_RVA, 'Unexpected call target'


def run_session(pe, is_orig, symbols, pos_x, pos_y, prep_vals, coeff_vals, seed, options=None):
    options = options or {}
    base = pe.OPTIONAL_HEADER.ImageBase
    size = (pe.OPTIONAL_HEADER.SizeOfImage + 0xfff) & ~0xfff
    uc = Uc(UC_ARCH_X86, UC_MODE_32)
    uc.mem_map(base, size)
    uc.mem_write(base, pe.get_memory_mapped_image())
    uc.mem_map(STACK, 0x10000)
    uc.mem_map(STOP, 0x1000)
    uc.mem_map(ARENA, 0x80000)

    FB = ARENA + 0x20000
    TEX = ARENA + 0x40000
    LUT = ARENA + 0x60000

    # Deterministic pattern generation for FB, TEX, LUT from seed
    rng = random.Random(seed)
    fb_init = bytes(rng.randbytes(320 * 240))
    tex_init = bytes(rng.randbytes(0x10000))
    lut_init = bytes(rng.randbytes(0x10000))

    uc.mem_write(FB, fb_init)
    uc.mem_write(TEX, tex_init)
    uc.mem_write(LUT, lut_init)

    # Viewport state: width=256, clip_bottom_int=200, clip_top_int=10, clip_right_int=300, clip_left_int=20, row_stride=320
    viewport = [0, 200, 10, 300, 20, 0, 320, 0, 0]
    viewport_addr = 0x63f2b0 if is_orig else symbols['g_triangleViewportState']
    fb_ptr_addr = 0x63f2d4 if is_orig else symbols['g_triangleViewportFramebuffer']
    uc.mem_write(viewport_addr, struct.pack('<9i', *viewport))
    uc.mem_write(fb_ptr_addr, struct.pack('<I', FB))

    pos_addr = ARENA + 0x100
    prep_addr = ARENA + 0x200
    coeff_addr = (ARENA + 0x300) if coeff_vals else 0
    packet_addr = ARENA + 0x400

    uc.mem_write(pos_addr, struct.pack('<2i', pos_x, pos_y))
    prep_full = list(prep_vals)
    prep_full[6] = TEX
    prep_full[7] = LUT + options.get("lookup_offset", 0)
    if "source_overlap" in options:
        tex_offset = (prep_full[3] & 0xffffff00) | ((prep_full[2] >> 8) & 255)
        prep_full[6] = FB + (pos_y >> 8) * 320 + (pos_x >> 8) + options["source_overlap"] - tex_offset
    uc.mem_write(prep_addr, struct.pack('<8I', *[v & 0xffffffff for v in prep_full]))
    if coeff_vals:
        uc.mem_write(coeff_addr, struct.pack('<4i', *coeff_vals))

    uc.mem_write(packet_addr, struct.pack('<4I', 0, pos_addr, prep_addr, coeff_addr))

    entry = (0x400000 + ROUTINE_RVA) if is_orig else symbols[ROUTINE_NAME]
    target_call = (0x400000 + TARGET_CALL_RVA) if is_orig else symbols['Gfx_PrepareTransformedSprite']

    calls = []

    def call_hook(u, addr, sz, user_data=None):
        (void_addr, void_sz, void_ud) = (addr, sz, user_data)
        esp = u.reg_read(UC_X86_REG_ESP)
        ret = struct.unpack('<I', u.mem_read(esp, 4))[0]
        if is_orig:
            arg_input = u.reg_read(UC_X86_REG_ESI)
            input_bytes = bytes(u.mem_read(arg_input, 20))
            calls.append((arg_input, input_bytes))
        else:
            arg_input = struct.unpack('<I', u.mem_read(esp + 4, 4))[0]
            input_bytes = bytes(u.mem_read(arg_input, 20))
            calls.append((arg_input, input_bytes))
        u.reg_write(UC_X86_REG_ESP, esp + 4)
        u.reg_write(UC_X86_REG_EIP, ret)

    uc.hook_add(UC_HOOK_CODE, call_hook, begin=target_call, end=target_call)

    saved = [rng.getrandbits(32) for _ in PRESERVED]
    for r, v in zip(PRESERVED, saved):
        uc.reg_write(r, v)

    sp = STACK + 0x8000
    if is_orig:
        uc.mem_write(sp, struct.pack('<I', STOP))
        uc.reg_write(UC_X86_REG_ESI, packet_addr)
    else:
        uc.mem_write(sp, struct.pack('<2I', STOP, packet_addr))

    caller_stack = bytes(uc.mem_read(sp, STACK + 0x10000 - sp))
    uc.reg_write(UC_X86_REG_ESP, sp)
    uc.reg_write(UC_X86_REG_EFLAGS, 2)

    for name, (address, length) in GLOBALS.items():
        uc.mem_write(address if is_orig else symbols[name], rng.randbytes(length))
    image_before = bytes(uc.mem_read(base, size))

    writes = []
    reads = []
    faults = []
    def write_hook(u, access, address, length, value, unused):
        if ARENA <= address < ARENA + 0x80000:
            writes.append((address, length, value & ((1 << (8 * length)) - 1)))
    def read_hook(u, access, address, length, value, unused):
        if ARENA <= address < ARENA + 0x80000:
            reads.append((address, length, bytes(u.mem_read(address, length))))
    def invalid_hook(u, access, address, length, value, unused):
        faults.append((access, address, length))
        return False
    uc.hook_add(UC_HOOK_MEM_WRITE, write_hook)
    uc.hook_add(UC_HOOK_MEM_READ, read_hook)
    uc.hook_add(UC_HOOK_MEM_INVALID, invalid_hook)
    try:
        uc.emu_start(entry, STOP, count=2000000)
    except UcError:
        if not faults:
            raise
    if faults:
        return calls, bytes(uc.mem_read(ARENA, 0x80000)), tuple(
            bytes(uc.mem_read(a if is_orig else symbols[n], z))
            for n, (a, z) in GLOBALS.items()), writes, faults, reads

    assert uc.reg_read(UC_X86_REG_EIP) == STOP
    assert uc.reg_read(UC_X86_REG_ESP) == sp + 4
    assert (uc.reg_read(UC_X86_REG_EFLAGS) & 0x400) == 0

    if is_orig:
        assert [uc.reg_read(r) for r in (UC_X86_REG_EBX, UC_X86_REG_EDI, UC_X86_REG_EBP)] == [saved[0], saved[2], saved[3]], 'Original registers clobbered'
        assert uc.reg_read(UC_X86_REG_ESI) == packet_addr, 'Original ESI clobbered'
    else:
        assert [uc.reg_read(r) for r in PRESERVED] == saved, 'Compiled preserved registers clobbered'

    assert bytes(uc.mem_read(sp, STACK + 0x10000 - sp)) == caller_stack

    image_after = bytearray(uc.mem_read(base, size))
    for name, (orig_addr, sz) in GLOBALS.items():
        addr = orig_addr if is_orig else symbols[name]
        off = addr - base
        image_after[off:off + sz] = image_before[off:off + sz]
    assert bytes(image_after) == image_before, 'Unrelated image write detected'

    fb_out = bytes(uc.mem_read(ARENA, 0x80000))
    globals_out = tuple(bytes(uc.mem_read(orig_addr if is_orig else symbols[name], sz))
                        for name, (orig_addr, sz) in GLOBALS.items())
    return calls, fb_out, globals_out, writes, faults, reads


def verify_sprite_packet(record=False):
    target_data = verify_target()
    original_pe = pefile.PE(data=target_data)
    inspect_original(original_pe)

    commands = build(dll=DLL, exports=[ROUTINE_NAME, *GLOBALS.keys()])
    compiled_pe = pefile.PE(str(DLL))
    assert not getattr(compiled_pe, 'DIRECTORY_ENTRY_IMPORT', [])
    validate_renderer_calls(compiled_pe)
    symbols = validation_symbols(compiled_pe)

    trials = []

    # Transformed cases (delegate to Gfx_PrepareTransformedSprite)
    for deg in (0, 45, 90, 180, 270):
        rad = math.radians(deg)
        c = int(math.cos(rad) * 65536.0)
        s = int(math.sin(rad) * 65536.0)
        trials.append((
            100 << 8, 80 << 8,
            [16, 16, 0x1000, 0x2000, 0x5000, 0x6000, 0, 0],
            [c, s, -s, c]
        ))

    # Untransformed blits (coeff=None)
    # 1. Unclipped interior blits
    trials.append((50 << 8, 40 << 8, [0, 0, 0x1000, 0x2000, 0x5000, 0x6000, 0, 0], None))
    trials.append((100 << 8, 120 << 8, [10 << 8, 15 << 8, 0x2000, 0x1000, 0x6000, 0x4000, 0, 0], None))

    # 2. Viewport boundary clips
    trials.append((10 << 8, 5 << 8, [0, 0, 0x1000, 0x2000, 0x5000, 0x6000, 0, 0], None))   # clip top & left
    trials.append((280 << 8, 180 << 8, [0, 0, 0x1000, 0x2000, 0x5000, 0x6000, 0, 0], None)) # clip bottom & right
    trials.append((15 << 8, 50 << 8, [0, 0, 0x1000, 0x2000, 0x5000, 0x6000, 0, 0], None))  # clip left only
    trials.append((290 << 8, 50 << 8, [0, 0, 0x1000, 0x2000, 0x5000, 0x6000, 0, 0], None)) # clip right only
    trials.append((50 << 8, 5 << 8, [0, 0, 0x1000, 0x2000, 0x5000, 0x6000, 0, 0], None))   # clip top only
    trials.append((50 << 8, 190 << 8, [0, 0, 0x1000, 0x2000, 0x5000, 0x6000, 0, 0], None)) # clip bottom only

    # 3. Off-screen clips (completely clipped out)
    trials.append((-100 << 8, -100 << 8, [0, 0, 0x1000, 0x2000, 0x5000, 0x6000, 0, 0], None))
    trials.append((500 << 8, 500 << 8, [0, 0, 0x1000, 0x2000, 0x5000, 0x6000, 0, 0], None))
    trials.append((50 << 8, -80 << 8, [0, 0, 0x1000, 0x2000, 0x5000, 0x6000, 0, 0], None))
    trials.append((-80 << 8, 50 << 8, [0, 0, 0x1000, 0x2000, 0x5000, 0x6000, 0, 0], None))

    # 4. Small dimension sprites (tests small loop remainder handling)
    trials.append((50 << 8, 40 << 8, [0, 0, 0x1000, 0x2000, 0x1100, 0x2100, 0, 0], None)) # 1x1
    trials.append((50 << 8, 40 << 8, [0, 0, 0x1000, 0x2000, 0x1200, 0x2300, 0, 0], None)) # 2x3
    trials.append((50 << 8, 40 << 8, [0, 0, 0x1000, 0x2000, 0x1300, 0x2400, 0, 0], None)) # 3x4
    trials.append((50 << 8, 40 << 8, [0, 0, 0x1000, 0x2000, 0x1400, 0x2200, 0, 0], None)) # 4x2
    trials.append((50 << 8, 40 << 8, [0, 0, 0x1000, 0x2000, 0x1500, 0x2500, 0, 0], None)) # 5x5
    trials.append((50 << 8, 40 << 8, [0, 0, 0x1000, 0x2000, 0x1600, 0x2600, 0, 0], None)) # 6x6
    trials.append((50 << 8, 40 << 8, [0, 0, 0x1000, 0x2000, 0x1700, 0x2700, 0, 0], None)) # 7x7

    # 5. Randomized trials
    rng = random.Random(0x65BB5)
    for _ in range(40):
        pos_x = rng.randint(-50, 350) << 8
        pos_y = rng.randint(-50, 250) << 8
        ox = rng.randint(-20, 20) << 8
        oy = rng.randint(-20, 20) << 8
        u0 = rng.randint(0, 100) << 8
        v0 = rng.randint(0, 100) << 8
        w = rng.randint(1, 40) << 8
        h = rng.randint(1, 40) << 8
        u1 = u0 + w
        v1 = v0 + h
        is_trans = rng.choice([False, False, True])
        if is_trans:
            coeff = [rng.randint(-0x15000, 0x15000) for _ in range(4)]
        else:
            coeff = None
        trials.append((pos_x, pos_y, [ox, oy, u0, v0, u1, v1, 0, 0], coeff))

    # Retain all 64 initial cases and add independent full-word/alias/invalid
    # cases. Complete arena bytes and ordered store widths detect scalar-loop
    # substitutions even when final non-overlapping pixels happen to match.
    trials = [(*t, {}) for t in trials]
    for prep in ([0,0,0,0x120000,0x400,0x120400,0,0],
                 [0,0,0xffffff00,0,0x100,0x400,0,0]):
        trials.append((-100*256, -100*256, prep, None, {}))
    for width in range(1, 34):
        for overlap in (-5, -1, 0, 1, 5):
            trials.append((50*256,40*256,[0,0,0,0,width*256,3*256,0,0],None,
                           {'source_overlap':overlap}))
        for offset in (1, 255, 65535):
            trials.append((50*256,40*256,[0,0,0,0,width*256,3*256,0,0],None,
                           {'lookup_offset':offset}))
    for width in (-8, -1, 0, 1, 5, 6, 7):
        for height in (0, -1, 1, 3):
            if width <= 0 and height <= 0:
                continue  # DEC/JNE without pixel access is unbounded here.
            trials.append((50*256,40*256,[0,0,0,0,width*256,height*256,0,0],None,{}))
    cases_count = len(trials)
    fault_count = 0
    for seed, (pos_x, pos_y, prep, coeff, options) in enumerate(trials):
        orig_calls, orig_fb, orig_globs, orig_writes, orig_faults, orig_reads = run_session(original_pe, True, symbols, pos_x, pos_y, prep, coeff, seed, options)
        comp_calls, comp_fb, comp_globs, comp_writes, comp_faults, comp_reads = run_session(compiled_pe, False, symbols, pos_x, pos_y, prep, coeff, seed, options)
        assert orig_faults == comp_faults, (seed, orig_faults, comp_faults)
        fault_count += bool(orig_faults)

        if coeff is not None:
            assert len(orig_calls) == 1, f'Expected 1 call to Gfx_PrepareTransformedSprite, got {len(orig_calls)}'
            assert len(comp_calls) == 1, f'Expected 1 call to Gfx_PrepareTransformedSprite in compiled, got {len(comp_calls)}'
            assert orig_calls[0][1] == comp_calls[0][1], f'Trial {seed}: Gfx_PrepareTransformedSprite input struct mismatch'
            assert orig_fb == comp_fb and orig_globs == comp_globs
            assert orig_reads == comp_reads and orig_writes == comp_writes
        else:
            assert len(orig_calls) == 0
            assert len(comp_calls) == 0
            assert orig_fb == comp_fb, f'Trial {seed}: Complete arena mismatch'
            assert orig_writes == comp_writes, f'Trial {seed}: Ordered store mismatch'
            assert orig_reads == comp_reads, (seed, 'Ordered read mismatch', next(((i,a,b) for i,(a,b) in enumerate(zip(orig_reads,comp_reads)) if a!=b), (len(orig_reads),len(comp_reads))))
            assert orig_globs == comp_globs, f'Trial {seed}: Blit globals mismatch'

    coverage = {
        'trials': cases_count,
        'transformed_trials': sum(1 for t in trials if t[3] is not None),
        'untransformed_blits': sum(1 for t in trials if t[3] is None),
        'faults': fault_count
    }

    if record:
        for kind in ('compilation', 'emulation'):
            record_run(ROUTINE_RVA, kind, 'pass', inputs=INPUTS,
                       artifact=DLL.relative_to(ROOT).as_posix(),
                       cases=cases_count if kind == 'emulation' else 0,
                       command='uv run tools/verify_sprite_packet.py',
                       details={
                           'coverage': coverage,
                           'commands': commands,
                           'scope': 'Authentic sprite packet rasterization; transformed sprite delegation and unrolled translucent blit verified differentially against IGN_WIN.EXE',
                           'limitations': 'DEV-007 input pointer register to stack adaptation; downstream software rasterization mocked via call hook'
                       })

    print(f'PASS sprite packet rasterizer: {cases_count} trials ({coverage["transformed_trials"]} transformed, {coverage["untransformed_blits"]} untransformed blits; {fault_count} faults)', flush=True)
    return coverage


if __name__ == '__main__':
    verify_sprite_packet(record=True)
