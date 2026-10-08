# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Authenticated original triangle analysis, without substituted rendering calls."""
import hashlib
import json
import random
import struct
import itertools

import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import UC_HOOK_CODE, UC_HOOK_MEM_WRITE, UC_HOOK_MEM_READ, UcError
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EFLAGS,
    UC_X86_REG_ESI, UC_X86_REG_EBX, UC_X86_REG_EBP)
from verify_sprite_backend import BUILD, DATA, STACK, STOP, cpu_image, verify_target
from analyze_sprite_rasterizer import put, s32, u32, TABLE, REGISTERS, inspect as inspect_sprite

TEXTURE, FRAMEBUFFER = DATA+0x10000, DATA+0x20000
EDGES = (DATA+0x3000, DATA+0x6000, DATA+0x9000)
CODE_STARTS = set()


def fixture(pe):
    cpu, base, size = cpu_image(pe)
    cpu.mem_map(TEXTURE, 0x10000)
    cpu.mem_map(FRAMEBUFFER, 0x10000)
    cpu.mem_map(TABLE, 0x10000)
    rng = random.Random(0x5ca50)
    texture, framebuffer, table = (rng.randbytes(65536) for _ in range(3))
    cpu.mem_write(TEXTURE, texture)
    cpu.mem_write(FRAMEBUFFER, framebuffer)
    cpu.mem_write(TABLE, table)
    put(cpu, 0x51a9d8, EDGES)
    put(cpu, 0x63f2b0, [256, 30, 1, 30, 1, 29*256+255, 64, 256, 29*256+255, FRAMEBUFFER])
    return cpu, (texture, framebuffer, table)


def divide(numerator, denominator):
    quotient = abs(numerator)//abs(denominator)
    return -quotient if (numerator < 0) != (denominator < 0) else quotient


def fast_expectation(vertices, uv, fixtures):
    """Independent expressions for the unclipped path; no emulator substitution."""
    v = [list(p)+list(t) for p, t in zip(vertices, uv)]
    # Preserve the instruction's compare/swap order, including equal-Y ties.
    if s32(v[0][1]) > s32(v[2][1]): v[0], v[2] = v[2], v[0]
    if s32(v[0][1]) > s32(v[1][1]): v[0], v[1] = v[1], v[0]
    elif s32(v[2][1]) < s32(v[1][1]): v[1], v[2] = v[2], v[1]
    a, b, c = v
    h1, h2 = u32(b[1]-a[1]), u32(c[1]-a[1])
    ratio = 0x7fffffff if h1 == h2 else ((h1 << 32)//(h2 or 1)) >> 1
    assert ratio <= 0x7fffffff
    def interpolate(index):
        return u32((s32(u32(2*(c[index]-a[index])))*s32(ratio)) >> 32)
    denominator = s32(u32(interpolate(0)-(b[0]-a[0])))
    if -2 <= denominator <= 2: denominator = ((denominator >> 2) | 1) << 2
    gu, gv = [u32(divide(s32(u32(interpolate(i)-b[i]+a[i])) << 16, denominator))
        for i in (2,3)]
    gradients = (gu, gv)
    corrections = [[u32(i*(s32(u32(-g)) >> 3)) for i in range(8)] for g in gradients]
    def slope(p, q):
        if p[1] == q[1]: return -2147418112 if s32(q[0]) < s32(p[0]) else 2147418112
        return divide(s32(u32((p[0]-q[0]) << 8)), s32(u32(p[1]-q[1])))
    right_short = slope(a, c) <= slope(a, b)
    edge_records = {}
    def edge(p, q, corrected, address):
        count = (s32(q[1]) >> 8)-(s32(p[1]) >> 8)
        records = []
        if count:
            dy = u32(q[1]-p[1])
            dx = s32(u32(q[0]-p[0]))
            if dy <= 64:
                if abs(dx) <= 65536: dy = 65
                elif dy <= 3: dy = 3
            def texture_slope(i):
                delta = s32(u32(q[i]-p[i]))
                # The original high dword is only the sign, NOT delta >> 16.
                n = (-1 if delta < 0 else 0)*(1 << 32)+u32(delta << 16)
                return u32(divide(n, s32(dy)))
            tu, tv = texture_slope(2), texture_slope(3)
            tx = u32(divide(dx << 16, s32(dy)))
            fraction = 256-(p[1] & 255)
            accum = [u32((p[i] << 8)+(s32(u32(fraction*g)) >> 8))
                for i, g in ((2,tu),(3,tv))]
            x = u32((p[0] << 8)+fraction*(s32(tx) >> 8))
            for _ in range(count):
                bucket = (x >> 13) & 7
                if corrected:
                    rec = (s32(u32(accum[1]+corrections[1][bucket])) >> 8,
                        s32(u32(accum[0]+corrections[0][bucket])) >> 8, s32(x) >> 16)
                else: rec = (s32(accum[1]) >> 16, s32(accum[0]) >> 16, s32(x) >> 16)
                records.append(tuple(u32(t) for t in rec))
                accum = [u32(accum[0]+tu), u32(accum[1]+tv)]
                x = u32(x+tx)
        edge_records[address] = ((s32(p[1]) >> 8), count, records)
        return records
    ab = edge(a,b,right_short,EDGES[0])
    ac = edge(a,c,not right_short,EDGES[1])
    bc = edge(b,c,right_short,EDGES[2])
    left, right = (ac, ab+bc) if right_short else (ab+bc, ac)
    texture, framebuffer, table = fixtures
    framebuffer = bytearray(framebuffer)
    writes, accesses = [], []
    packed_step = (u32(-gu) & 0xffff) << 16 | ((u32(-gv) >> 8) & 0xffff)
    byte_step = (u32(-gu) >> 16) & 255
    for row, (l, r) in enumerate(zip(left, right)):
        lx, rx = s32(l[2]), s32(r[2])
        start, count = (s32(a[1]) >> 8)+row, rx-lx-1
        # The span samples the left edge BEFORE testing the span length.
        idx = ((l[0] & 255) << 8) | (l[1] & 255)
        accesses.append(('texture', TEXTURE+idx))
        ecx, bl = u32((r[0] & 0xffff)+(r[1] << 24)), (s32(r[1]) >> 8) & 255
        for column in range(count-1, -1, -1):
            total = ecx+packed_step
            ecx = u32(total)
            bl = (bl+byte_step+(total >> 32)) & 255
            idx = ((ecx >> 8) & 255)*256+bl
            destination = start*64+lx+1+column
            accesses.extend([('framebuffer', FRAMEBUFFER+destination), ('texture', TEXTURE+idx)])
            table_index = texture[idx]*256+framebuffer[destination]
            accesses.append(('table', TABLE+table_index))
            framebuffer[destination] = table[table_index]
            writes.append((FRAMEBUFFER+destination, 1, framebuffer[destination]))
        if count >= 0:
            destination = start*64+lx
            accesses.extend([('framebuffer', FRAMEBUFFER+destination),
                ('table', TABLE+texture[((l[0] & 255) << 8) | (l[1] & 255)]*256+framebuffer[destination])])
            framebuffer[destination] = table[texture[((l[0] & 255) << 8) | (l[1] & 255)]*256+framebuffer[destination]]
            writes.append((FRAMEBUFFER+destination, 1, framebuffer[destination]))
    scratch = {0x4bada4: gu, 0x4bada8: gv, 0x4badf4:h1, 0x4badf8:h2 or 1,
        0x4bae10:ratio, 0x4bae00:u32(b[0]-a[0]), 0x4bae04:interpolate(0),
        0x4bae08:interpolate(2), 0x4bae0c:interpolate(3), 0x4badfc:u32(denominator),
        0x4bad64:packed_step, 0x4bad60:byte_step, 0x4bad68:TEXTURE, 0x4bad6c:TABLE,
        0x51a9d0:FRAMEBUFFER+((s32(c[1]) >> 8)*64)}
    for i, p in enumerate(v):
        scratch[0x4bad74+i*8], scratch[0x4bad78+i*8] = map(u32, p[:2])
        scratch[0x4bad8c+i*8], scratch[0x4bad90+i*8] = map(u32, p[2:])
    for j in range(2):
        for i in range(8): scratch[0x4badb4+j*32+i*4] = corrections[j][i]
    return bytes(framebuffer), writes, scratch, edge_records, accesses


def mul16(a, b):
    p = s32(a) * s32(b)
    return u32((p >> 16) & 0xffffffff)


def interp_step(start, end, ratio):
    delta = s32(u32(2 * (end - start)))
    prod = s32(delta) * s32(ratio)
    hi = s32(prod >> 32)
    return u32(start + hi)


def interp_back(start, end, ratio):
    delta = s32(u32(2 * (end - start)))
    prod = s32(delta) * s32(ratio)
    hi = s32(prod >> 32)
    return u32(end - hi)


def expected_splitter(packet, viewport, gradients):
    clip_top, clip_bottom, clip_right = viewport[0], viewport[5], viewport[8]
    du, dv = gradients[0], gradients[1]
    p = list(packet)
    writes = []
    if s32(p[1]) < s32(clip_top):
        dy = u32(p[3] - p[1])
        dist = u32(clip_top - p[1])
        ratio = ((dist << 32) // dy) >> 1
        p[5] = interp_step(p[5], p[7], ratio)
        writes.append(('packet', 0x14, p[5]))
        p[4] = interp_step(p[4], p[6], ratio)
        writes.append(('packet', 0x10, p[4]))
        p[0] = interp_step(p[0], p[2], ratio)
        writes.append(('packet', 0x00, p[0]))
        p[1] = clip_top
        writes.append(('packet', 0x04, p[1]))
    if s32(p[3]) > s32(clip_bottom):
        dy = u32(p[3] - p[1])
        dist = u32(p[3] - clip_bottom)
        ratio = ((dist << 32) // dy) >> 1
        p[7] = interp_back(p[5], p[7], ratio)
        writes.append(('packet', 0x1c, p[7]))
        p[6] = interp_back(p[4], p[6], ratio)
        writes.append(('packet', 0x18, p[6]))
        p[2] = interp_back(p[0], p[2], ratio)
        writes.append(('packet', 0x08, p[2]))
        p[3] = clip_bottom
        writes.append(('packet', 0x0c, p[3]))
    split_flag = 0
    writes.append(('scratch', 0x4baec8, 0))
    saved = None
    if s32(p[0]) > s32(clip_right):
        if s32(p[2]) < s32(clip_right):
            split_flag = 1
            writes.append(('scratch', 0x4baec8, 1))
            saved = (p[2], p[3], p[6], p[7])
            writes.extend([('scratch', 0x4baecc, p[2]), ('scratch', 0x4baed0, p[3]),
                           ('scratch', 0x4baed4, p[6]), ('scratch', 0x4baed8, p[7])])
            dx = u32(p[0] - p[2])
            dist = u32(clip_right - p[2])
            ratio = ((dist << 32) // dx) >> 1
            p[6] = interp_back(p[4], p[6], ratio)
            writes.append(('packet', 0x18, p[6]))
            p[7] = interp_back(p[5], p[7], ratio)
            writes.append(('packet', 0x1c, p[7]))
            p[3] = interp_back(p[1], p[3], ratio)
            writes.append(('packet', 0x0c, p[3]))
            p[2] = clip_right
            writes.append(('packet', 0x08, p[2]))
            dx0 = u32(p[0] - clip_right)
            p[4] = u32(p[4] - mul16(dx0, du))
            writes.append(('packet', 0x10, p[4]))
            p[5] = u32(p[5] - mul16(dx0, dv))
            writes.append(('packet', 0x14, p[5]))
            p[0] = clip_right
            writes.append(('packet', 0x00, p[0]))
        else:
            dx1 = u32(p[2] - clip_right)
            p[6] = u32(p[6] - mul16(dx1, du))
            writes.append(('packet', 0x18, p[6]))
            p[7] = u32(p[7] - mul16(dx1, dv))
            writes.append(('packet', 0x1c, p[7]))
            p[2] = clip_right
            writes.append(('packet', 0x08, p[2]))
            dx0 = u32(p[0] - clip_right)
            p[4] = u32(p[4] - mul16(dx0, du))
            writes.append(('packet', 0x10, p[4]))
            p[5] = u32(p[5] - mul16(dx0, dv))
            writes.append(('packet', 0x14, p[5]))
            p[0] = clip_right
            writes.append(('packet', 0x00, p[0]))
    else:
        if s32(p[2]) > s32(clip_right):
            split_flag = 1
            writes.append(('scratch', 0x4baec8, 1))
            saved = (p[2], p[3], p[6], p[7])
            writes.extend([('scratch', 0x4baecc, p[2]), ('scratch', 0x4baed0, p[3]),
                           ('scratch', 0x4baed4, p[6]), ('scratch', 0x4baed8, p[7])])
            dx = u32(p[2] - p[0])
            dist = u32(p[2] - clip_right)
            ratio = 0x7fffffff if dist == dx else (((dist << 32) // dx) >> 1)
            p[6] = interp_back(p[4], p[6], ratio)
            writes.append(('packet', 0x18, p[6]))
            p[7] = interp_back(p[5], p[7], ratio)
            writes.append(('packet', 0x1c, p[7]))
            p[3] = interp_back(p[1], p[3], ratio)
            writes.append(('packet', 0x0c, p[3]))
            p[2] = clip_right
            writes.append(('packet', 0x08, p[2]))
    return p, split_flag, saved, writes


def expected_wrapper(packet, viewport, gradients, is_corrected):
    clip_top, clip_bottom_int, clip_top_int = viewport[0], viewport[1], viewport[2]
    clip_bottom, clip_right = viewport[5], viewport[8]
    du, dv = gradients[0], gradients[1]
    corrections = [[u32(i*(s32(u32(-g)) >> 3)) for i in range(8)] for g in (du, dv)]
    p = list(packet)
    if s32(clip_top) >= s32(p[3]):
        return {'header': (clip_top_int, 0), 'records': []}
    if s32(clip_bottom) <= s32(p[1]):
        return {'header': (clip_bottom_int, 0), 'records': []}
    p_split, split_flag, saved, _ = expected_splitter(p, viewport, gradients)
    start_y = s32(p_split[1]) >> 8
    count1 = (s32(p_split[3]) >> 8) - start_y
    def emit_segment_records(seg_p, count):
        if count <= 0: return []
        dy = u32(seg_p[3] - seg_p[1])
        dx = s32(u32(seg_p[2] - seg_p[0]))
        if dy <= 64:
            if abs(dx) <= 65536: dy = 65
            elif dy <= 3: dy = 3
        def texture_slope(v0, v1):
            delta = s32(u32(v1 - v0))
            n = (-1 if delta < 0 else 0)*(1 << 32) + u32(delta << 16)
            return u32(divide(n, s32(dy)))
        tu, tv = texture_slope(seg_p[4], seg_p[6]), texture_slope(seg_p[5], seg_p[7])
        tx = u32(divide(dx << 16, s32(dy)))
        fraction = 256 - (seg_p[1] & 255)
        accum = [u32((seg_p[4] << 8) + (s32(u32(fraction * tu)) >> 8)),
                 u32((seg_p[5] << 8) + (s32(u32(fraction * tv)) >> 8))]
        x = u32((seg_p[0] << 8) + fraction * (s32(tx) >> 8))
        recs = []
        for _ in range(count):
            bucket = (x >> 13) & 7
            if is_corrected:
                rec = (s32(u32(accum[1] + corrections[1][bucket])) >> 8,
                       s32(u32(accum[0] + corrections[0][bucket])) >> 8, s32(x) >> 16)
            else:
                rec = (s32(accum[1]) >> 16, s32(accum[0]) >> 16, s32(x) >> 16)
            recs.append(tuple(u32(t) for t in rec))
            accum = [u32(accum[0] + tu), u32(accum[1] + tv)]
            x = u32(x + tx)
        return recs
    records = emit_segment_records(p_split, count1)
    total_count = max(0, count1)
    if split_flag == 1:
        p2 = [p_split[2], p_split[3], saved[0], saved[1],
              p_split[6], p_split[7], saved[2], saved[3]]
        if s32(p2[2]) > s32(clip_right):
            dx1 = u32(p2[2] - clip_right)
            p2[6] = u32(p2[6] - mul16(dx1, du))
            p2[7] = u32(p2[7] - mul16(dx1, dv))
            p2[2] = clip_right
        count2 = (s32(p2[3]) >> 8) - (s32(p2[1]) >> 8)
        if count2 > 0:
            total_count += count2
            records.extend(emit_segment_records(p2, count2))
    return {'header': (start_y, total_count), 'records': records}


def clipped_expectation(vertices, uv, fixtures, viewport=(256, 30, 1, 30, 1, 29*256+255, 64, 256, 29*256+255)):
    clip_top, clip_bottom_int, clip_top_int = viewport[0], viewport[1], viewport[2]
    clip_right_int, clip_left_int, clip_bottom = viewport[3], viewport[4], viewport[5]
    stride, clip_left, clip_right = viewport[6], viewport[7], viewport[8]
    v = [list(p)+list(t) for p, t in zip(vertices, uv)]
    if s32(v[0][1]) > s32(v[2][1]): v[0], v[2] = v[2], v[0]
    if s32(v[0][1]) > s32(v[1][1]): v[0], v[1] = v[1], v[0]
    elif s32(v[2][1]) < s32(v[1][1]): v[1], v[2] = v[2], v[1]
    a, b, c = v
    if s32(c[1]) < s32(clip_top) or s32(a[1]) > s32(clip_bottom):
        return fixtures[1], [], []
    min_x, max_x = min(s32(p[0]) for p in v), max(s32(p[0]) for p in v)
    if min_x > s32(clip_right) or max_x < s32(clip_left):
        return fixtures[1], [], []
    h1, h2 = u32(b[1]-a[1]), u32(c[1]-a[1])
    ratio = 0x7fffffff if h1 == h2 else ((h1 << 32)//(h2 or 1)) >> 1
    def interpolate(index):
        return u32((s32(u32(2*(c[index]-a[index])))*s32(ratio)) >> 32)
    denominator = s32(u32(interpolate(0)-(b[0]-a[0])))
    if -2 <= denominator <= 2: denominator = ((denominator >> 2) | 1) << 2
    gu, gv = [u32(divide(s32(u32(interpolate(i)-b[i]+a[i])) << 16, denominator)) for i in (2,3)]
    gradients = (gu, gv)
    def slope(p, q):
        if p[1] == q[1]: return -2147418112 if s32(q[0]) < s32(p[0]) else 2147418112
        return divide(s32(u32((p[0]-q[0]) << 8)), s32(u32(p[1]-q[1])))
    right_short = slope(a, c) <= slope(a, b)
    pkt_ab = [a[0], a[1], b[0], b[1], a[2], a[3], b[2], b[3]]
    pkt_ac = [a[0], a[1], c[0], c[1], a[2], a[3], c[2], c[3]]
    pkt_bc = [b[0], b[1], c[0], c[1], b[2], b[3], c[2], c[3]]
    vp_list = [clip_top, clip_bottom_int, clip_top_int, clip_right_int, clip_left_int, clip_bottom, stride, clip_left, clip_right, 0]
    if right_short:
        wrap_ab = expected_wrapper(pkt_ab, vp_list, gradients, True)
        wrap_ac = expected_wrapper(pkt_ac, vp_list, gradients, False)
        wrap_bc = expected_wrapper(pkt_bc, vp_list, gradients, True)
        left = wrap_ac['records']
        right = wrap_ab['records'] + wrap_bc['records']
        start_y = wrap_ac['header'][0]
    else:
        wrap_ab = expected_wrapper(pkt_ab, vp_list, gradients, False)
        wrap_ac = expected_wrapper(pkt_ac, vp_list, gradients, True)
        wrap_bc = expected_wrapper(pkt_bc, vp_list, gradients, False)
        left = wrap_ab['records'] + wrap_bc['records']
        right = wrap_ac['records']
        start_y = wrap_ab['header'][0]
    texture, framebuffer, table = fixtures
    framebuffer = bytearray(framebuffer)
    writes, accesses = [], []
    packed_step = (u32(-gu) & 0xffff) << 16 | ((u32(-gv) >> 8) & 0xffff)
    byte_step = (u32(-gu) >> 16) & 255
    cursor = start_y * stride
    for row, (l, r) in enumerate(zip(left, right)):
        lx, rx = s32(l[2]), s32(r[2])
        if lx < clip_left_int:
            count = rx - clip_left_int
            dest = cursor + clip_left_int
            ecx, bl = u32((r[0] & 0xffff)+(r[1] << 24)), (s32(r[1]) >> 8) & 255
            for col in range(count - 1, -1, -1):
                total = ecx + packed_step
                ecx = u32(total)
                bl = (bl + byte_step + (total >> 32)) & 255
                idx = ((ecx >> 8) & 255)*256 + bl
                destination = dest + col
                accesses.extend([('texture', TEXTURE+idx), ('framebuffer', FRAMEBUFFER+destination)])
                table_index = texture[idx]*256 + framebuffer[destination]
                accesses.append(('table', TABLE+table_index))
                framebuffer[destination] = table[table_index]
                writes.append((FRAMEBUFFER+destination, 1, framebuffer[destination]))
        else:
            idx_left = ((l[0] & 255) << 8) | (l[1] & 255)
            accesses.append(('texture', TEXTURE+idx_left))
            count = rx - lx - 1
            dest = cursor + lx + 1
            ecx, bl = u32((r[0] & 0xffff)+(r[1] << 24)), (s32(r[1]) >> 8) & 255
            for col in range(count - 1, -1, -1):
                total = ecx + packed_step
                ecx = u32(total)
                bl = (bl + byte_step + (total >> 32)) & 255
                idx = ((ecx >> 8) & 255)*256 + bl
                destination = dest + col
                accesses.extend([('framebuffer', FRAMEBUFFER+destination), ('texture', TEXTURE+idx)])
                table_index = texture[idx]*256 + framebuffer[destination]
                accesses.append(('table', TABLE+table_index))
                framebuffer[destination] = table[table_index]
                writes.append((FRAMEBUFFER+destination, 1, framebuffer[destination]))
            if count >= 0:
                destination = cursor + lx
                accesses.extend([('framebuffer', FRAMEBUFFER+destination),
                    ('table', TABLE+texture[idx_left]*256+framebuffer[destination])])
                framebuffer[destination] = table[texture[idx_left]*256+framebuffer[destination]]
                writes.append((FRAMEBUFFER+destination, 1, framebuffer[destination]))
        cursor += stride
    return bytes(framebuffer), writes, accesses


def execute(pe, vertices, uv, transformed=False, coefficients=(65536, 0, 0, 65536)):
    cpu, fixtures = fixture(pe)
    if transformed:
        put(cpu, DATA, [4, DATA+64, DATA+96, DATA+160])
        put(cpu, DATA+64, vertices)
        put(cpu, DATA+96, uv)
        put(cpu, DATA+160, coefficients)
    else:
        put(cpu, DATA, [0, *sum((list(v) for v in vertices), []),
            *sum((list(v) for v in uv), []), TEXTURE, TABLE])
    sp = STACK+0x8000
    put(cpu, sp, [STOP] if transformed else [STOP, DATA])
    cpu.reg_write(UC_X86_REG_ESP, sp)
    saved = [0x12345678+i*0x111111 for i in range(7)]
    if transformed: saved[4] = DATA
    for reg, value in zip(REGISTERS, saved): cpu.reg_write(reg, value)
    cpu.reg_write(UC_X86_REG_EFLAGS, 2)
    writes, calls, path, accesses, shared, call_store_counts = [], [], set(), [], [], []
    call_store_start = 0
    input_before = bytes(cpu.mem_read(DATA, 256))
    image_before = bytes(cpu.mem_read(0x400000, pe.OPTIONAL_HEADER.SizeOfImage))
    def code(uc, address, length, unused):
        nonlocal call_store_start
        assert address in CODE_STARTS, hex(address)
        path.add(address)
        if address == 0x45ca50:
            arg = struct.unpack('<I', uc.mem_read(uc.reg_read(UC_X86_REG_ESP)+4, 4))[0]
            calls.append(list(struct.unpack('<15I', uc.mem_read(arg, 60))))
            shared.append(bytes(uc.mem_read(0x4baf1c, 116)))
            call_store_start = sum(FRAMEBUFFER <= a < FRAMEBUFFER+65536 for a,n,v in writes)
        if transformed and address == 0x468e70:
            assert bytes(uc.mem_read(0x4baf1c, 116)) == shared[0]
        if transformed and address in (0x468e70,0x468ee4):
            call_store_counts.append(sum(FRAMEBUFFER <= a < FRAMEBUFFER+65536 for a,n,v in writes)-call_store_start)
    def write(uc, access, address, length, value, unused):
        if STACK <= address < STACK+0x10000: return
        allowed = [(FRAMEBUFFER, FRAMEBUFFER+65536), *[(a,a+0x20d8) for a in EDGES],
            (0x4bad60,0x4baef0), (0x51a9a0,0x51aa20)]
        if transformed: allowed += [(0x4bad34,0x4bad48),(0x4baf1c,0x4baf90)]
        assert any(a <= address and address+length <= b for a,b in allowed), hex(address)
        writes.append((address, length, value & ((1 << (8*length))-1)))
    def read(uc, access, address, length, value, unused):
        for label, start in (('texture',TEXTURE), ('framebuffer',FRAMEBUFFER), ('table',TABLE)):
            if start <= address < start+65536:
                assert length == 1
                accesses.append((label, address))
    cpu.hook_add(UC_HOOK_CODE, code)
    cpu.hook_add(UC_HOOK_MEM_WRITE, write)
    cpu.hook_add(UC_HOOK_MEM_READ, read)
    fault = None
    try:
        cpu.emu_start(0x465bb5 if transformed else 0x45ca50, STOP, count=200000)
    except UcError as exc:
        fault = str(exc)
    result = {'fault': fault, 'eip': hex(cpu.reg_read(UC_X86_REG_EIP)),
        'calls': calls, 'path': [hex(a) for a in sorted(path)],
        'framebuffer_writes': [w for w in writes if FRAMEBUFFER <= w[0] < FRAMEBUFFER+65536],
        'writes': writes, 'accesses': accesses, 'triangle_store_counts':call_store_counts}
    if fault is None:
        assert cpu.reg_read(UC_X86_REG_EIP) == STOP, result
        assert cpu.reg_read(UC_X86_REG_ESP) == sp+4
        for i in range(7) if transformed else (1,4,5,6):
            assert cpu.reg_read(REGISTERS[i]) == saved[i]
        assert bytes(cpu.mem_read(TEXTURE, 65536)) == fixtures[0]
        assert bytes(cpu.mem_read(TABLE, 65536)) == fixtures[2]
        assert bytes(cpu.mem_read(DATA, 256)) == input_before
        # Every image mutation must have an observed write; no hidden clobbers.
        image_after = bytearray(cpu.mem_read(0x400000, pe.OPTIONAL_HEADER.SizeOfImage))
        for address, length, value in writes:
            if 0x400000 <= address < 0x400000+len(image_after):
                image_after[address-0x400000:address-0x400000+length] = image_before[address-0x400000:address-0x400000+length]
        assert image_after == image_before
    return result, cpu, fixtures


def check_fast(result, cpu, vertices, uv, fixtures):
    expected, stores, scratch, edges, accesses = fast_expectation(vertices, uv, fixtures)
    assert result['fault'] is None, result['fault']
    assert result['framebuffer_writes'] == stores, (result['framebuffer_writes'][:8], stores[:8])
    assert result['accesses'] == accesses, (result['accesses'][:8], accesses[:8])
    assert bytes(cpu.mem_read(FRAMEBUFFER, 65536)) == expected
    for address, value in scratch.items():
        length = 1 if address == 0x4bad60 else 4
        assert int.from_bytes(cpu.mem_read(address, length), 'little') == value, hex(address)
    for address, (y, count, records) in edges.items():
        assert bytes(cpu.mem_read(address, 8)) == struct.pack('<2I', u32(y), count)
        assert bytes(cpu.mem_read(address+8, len(records)*12)) == b''.join(struct.pack('<3I', *r) for r in records)


def direct_splitter_checks(pe):
    viewport = [256, 30, 1, 30, 1, 29*256+255, 64, 256, 29*256+255, 0x60000]
    gradients = (1234, -5678)
    test_packets = [
        [10*256, 10*256, 20*256, 20*256, 1000, 2000, 3000, 4000],
        [10*256, 0, 20*256, 20*256, 1000, 2000, 3000, 4000],
        [10*256, 10*256, 20*256, 35*256, 1000, 2000, 3000, 4000],
        [10*256, 0, 20*256, 35*256, 1000, 2000, 3000, 4000],
        [10*256, 10*256, 35*256, 20*256, 1000, 2000, 3000, 4000],
        [viewport[8], 10*256, 35*256, 20*256, 1000, 2000, 3000, 4000],
        [35*256, 10*256, 10*256, 20*256, 1000, 2000, 3000, 4000],
        [35*256, 10*256, 32*256, 20*256, 1000, 2000, 3000, 4000],
        [35*256, 10*256, viewport[8], 20*256, 1000, 2000, 3000, 4000],
        [viewport[8], 10*256, viewport[8], 20*256, 1000, 2000, 3000, 4000],
        [10*256, 0, 35*256, 35*256, 1000, 2000, 3000, 4000],
        [35*256, 0, 10*256, 35*256, 1000, 2000, 3000, 4000],
    ]
    rng = random.Random(0x24f270)
    for _ in range(50):
        test_packets.append([
            rng.randint(-1000, 10000), rng.randint(256, 3000),
            rng.randint(-1000, 10000), rng.randint(3001, 8000),
            rng.randint(-50000, 50000), rng.randint(-50000, 50000),
            rng.randint(-50000, 50000), rng.randint(-50000, 50000)
        ])
    cases, paths = 0, set()
    for raw_pkt in test_packets:
        cpu, base, size = cpu_image(pe)
        put(cpu, 0x63f2b0, viewport)
        put(cpu, 0x4bada4, gradients)
        pkt_addr = DATA + 0x100
        put(cpu, pkt_addr, raw_pkt)
        sp = STACK + 0x8000
        put(cpu, sp, [STOP])
        cpu.reg_write(UC_X86_REG_ESP, sp)
        cpu.reg_write(UC_X86_REG_ESI, pkt_addr)
        cpu.reg_write(UC_X86_REG_EFLAGS, 2)
        writes = []
        def write(uc, access, address, length, value, unused):
            if STACK <= address < STACK + 0x10000: return
            writes.append((address, length, value & ((1 << (8*length)) - 1)))
        def code(uc, address, length, unused):
            assert address in CODE_STARTS
            paths.add(hex(address))
        cpu.hook_add(UC_HOOK_MEM_WRITE, write)
        cpu.hook_add(UC_HOOK_CODE, code)
        cpu.emu_start(0x64f270, STOP, count=10000)
        exp_pkt, exp_split, exp_saved, exp_writes = expected_splitter(raw_pkt, viewport, gradients)
        res_pkt = list(struct.unpack('<8i', cpu.mem_read(pkt_addr, 32)))
        res_split = struct.unpack('<I', cpu.mem_read(0x4baec8, 4))[0]
        if exp_saved:
            exp_saved = tuple(s32(x) for x in exp_saved)
            res_saved = struct.unpack('<4i', cpu.mem_read(0x4baecc, 16))
            assert res_saved == exp_saved
        exp_pkt = [s32(x) for x in exp_pkt]
        assert res_split == exp_split
        assert res_pkt == exp_pkt
        mapped_exp_writes = []
        for kind, offset_or_addr, val in exp_writes:
            addr = (pkt_addr + offset_or_addr) if kind == 'packet' else offset_or_addr
            mapped_exp_writes.append((addr, 4, u32(val)))
        assert writes == mapped_exp_writes
        assert cpu.reg_read(UC_X86_REG_EIP) == STOP
        cases += 1
    return cases, paths


def direct_wrapper_checks(pe):
    viewport = [256, 30, 1, 30, 1, 29*256+255, 64, 256, 29*256+255, 0x60000]
    gradients = (1234, -5678)
    corrections = [[u32(i*(s32(u32(-g)) >> 3)) for i in range(8)] for g in gradients]
    test_packets = [
        [10*256, 5*256, 20*256, 25*256, 1000, 2000, 3000, 4000],
        [10*256, 0, 20*256, 250, 1000, 2000, 3000, 4000],
        [10*256, 30*256, 20*256, 35*256, 1000, 2000, 3000, 4000],
        [10*256, 0, 20*256, 20*256, 1000, 2000, 3000, 4000],
        [10*256, 10*256, 20*256, 35*256, 1000, 2000, 3000, 4000],
        [35*256, 5*256, 10*256, 25*256, 1000, 2000, 3000, 4000],
        [10*256, 5*256, 35*256, 25*256, 1000, 2000, 3000, 4000],
        [35*256, 5*256, 32*256, 25*256, 1000, 2000, 3000, 4000],
    ]
    cases, paths = 0, set()
    for is_corrected in (True, False):
        entry_rva = 0x24efa0 if is_corrected else 0x24f0c0
        for raw_pkt in test_packets:
            cpu, base, size = cpu_image(pe)
            put(cpu, 0x63f2b0, viewport)
            put(cpu, 0x4bada4, gradients)
            for j in range(2):
                for i in range(8):
                    put(cpu, 0x4badb4 + j*32 + i*4, [corrections[j][i]])
            pkt_addr = DATA + 0x100
            buf_addr = DATA + 0x1000
            put(cpu, pkt_addr, raw_pkt)
            cpu.mem_write(buf_addr, bytes(0x2000))
            sp = STACK + 0x8000
            put(cpu, sp, [STOP])
            cpu.reg_write(UC_X86_REG_ESP, sp)
            cpu.reg_write(UC_X86_REG_ESI, pkt_addr)
            cpu.reg_write(UC_X86_REG_EBX, buf_addr)
            cpu.reg_write(UC_X86_REG_EFLAGS, 2)
            saved_ebp = 0x87654321
            cpu.reg_write(UC_X86_REG_EBP, saved_ebp)
            def code(uc, address, length, unused):
                assert address in CODE_STARTS
                paths.add(hex(address))
            cpu.hook_add(UC_HOOK_CODE, code)
            cpu.emu_start(0x400000 + entry_rva, STOP, count=100000)
            assert cpu.reg_read(UC_X86_REG_EBP) == saved_ebp
            assert cpu.reg_read(UC_X86_REG_EIP) == STOP
            hdr = struct.unpack('<2i', cpu.mem_read(buf_addr, 8))
            count = hdr[1]
            recs = [struct.unpack('<3I', cpu.mem_read(buf_addr + 8 + k*12, 12)) for k in range(count)]
            exp = expected_wrapper(raw_pkt, viewport, gradients, is_corrected)
            assert hdr == exp['header']
            assert recs == exp['records']
            cases += 1
    return cases, paths


def experiments(pe):
    report = {'fast_cases': 0, 'clipped_cases': 0, 'transformed_cases': 0,
        'fault_cases': [], 'paths': set(), 'framebuffer_stores': 0}
    shapes = [(4,4,20,4,4,20), (4,4,4,20,20,20), (8,3,3,14,25,24),
        (8,3,25,14,3,24), (4,4,5,4,4,12), (4,4,5,12,4,20),
        (4,4,20,4,12,4), (4,4,4,4,4,4), (8,3,12,13,16,23)]
    uv0 = [(2*256,3*256), (18*256,4*256), (4*256,19*256)]
    for shape in shapes:
        for permutation in itertools.permutations(range(3)):
            for fraction in (0, 1, 127, 255):
                vertices = [(shape[2*i]*256+fraction,shape[2*i+1]*256+fraction) for i in permutation]
                uv = [uv0[i] for i in permutation]
                result, cpu, fixtures = execute(pe, vertices, uv)
                check_fast(result, cpu, vertices, uv, fixtures)
                report['paths'].update(result['path'])
                report['framebuffer_stores'] += len(result['framebuffer_writes'])
                report['fast_cases'] += 1
    rng = random.Random(0x24ea80)
    for _ in range(96):
        vertices = [(rng.randrange(512,7168),rng.randrange(512,7168)) for _ in range(3)]
        uv = [(rng.randrange(-2048,8192),rng.randrange(-2048,8192)) for _ in range(3)]
        result, cpu, fixtures = execute(pe,vertices,uv)
        check_fast(result,cpu,vertices,uv,fixtures)
        report['paths'].update(result['path'])
        report['framebuffer_stores'] += len(result['framebuffer_writes'])
        report['fast_cases'] += 1
    # Real clipped paths: repeatability, confinement, and full independent clipped-rendering oracle.
    for dx, dy in ((-8,0),(20,0),(0,-8),(0,20),(-8,-8),(20,20),(-40,0),(40,0),(0,-40),(0,40)):
        for permutation in itertools.permutations(range(3)):
            shape = shapes[2]
            vertices = [((shape[2*i]+dx)*256,(shape[2*i+1]+dy)*256) for i in permutation]
            uv = [uv0[i] for i in permutation]
            first, cpu, fixtures = execute(pe, vertices, uv)
            second, _, _ = execute(pe, vertices, uv)
            assert first == second and first['fault'] is None
            assert all(FRAMEBUFFER+64 <= w[0] < FRAMEBUFFER+30*64 and
                1 <= (w[0]-FRAMEBUFFER)%64 < 30 for w in first['framebuffer_writes'])
            exp_fb, exp_ws, exp_rs = clipped_expectation(vertices, uv, fixtures)
            assert bytes(cpu.mem_read(FRAMEBUFFER, 65536)) == exp_fb
            assert first['framebuffer_writes'] == exp_ws
            assert first['accesses'] == exp_rs
            report['paths'].update(first['path'])
            report['framebuffer_stores'] += len(first['framebuffer_writes'])
            report['clipped_cases'] += 1
            report.setdefault('clipped_oracle_cases', 0)
            report['clipped_oracle_cases'] += 1
    for coefficients in ((65536,0,0,65536),(-65536,0,0,-65536),
            (0,65536,-65536,0),(46340,46340,-46340,46340),
            (65536,16384,16384,65536),(0,0,0,0)):
        for px, py in ((12,12),(4,4),(28,28),(-4,12)):
            descriptor = [2*256,2*256,2*256,3*256,10*256,11*256,TEXTURE,TABLE]
            result, cpu, fixtures = execute(pe, [px*256,py*256], descriptor, True, coefficients)
            assert result['fault'] is None and len(result['calls']) == 2
            assert len(result['triangle_store_counts']) == 2
            if all(result['triangle_store_counts']):
                report.setdefault('transformed_both_triangles_store_cases',0)
                report['transformed_both_triangles_store_cases'] += 1
            # Recover packets independently from mul16 corner expressions.
            def mul(a,b): return u32((s32(a)*s32(b)) >> 16)
            def vector(x,y): return [u32(mul(x,coefficients[j])+mul(y,coefficients[j+2])) for j in (0,1)]
            a = vector(u32(-descriptor[0]),u32(-descriptor[1]))
            h = vector(descriptor[4]-16-descriptor[2],0)
            v = vector(0,descriptor[5]-16-descriptor[3])
            corners = [[u32(a[j]+(h[j] if k & 1 else 0)+(v[j] if k & 2 else 0)+[px*256,py*256][j]) for j in (0,1)] for k in range(4)]
            uv_corners = [(descriptor[2],descriptor[3]),(descriptor[4]-16,descriptor[3]),
                (descriptor[2],descriptor[5]-16),(descriptor[4]-16,descriptor[5]-16)]
            for packet, order in zip(result['calls'], ((0,1,2),(1,2,3))):
                expected = [0, *sum((corners[k] for k in order), []),
                    *sum((list(uv_corners[k]) for k in order), []), TEXTURE, TABLE]
                assert packet == expected
            # All transformed cases compare both real framebuffer calls to the unified oracle.
            current = fixtures
            stores, reads = [], []
            for packet in result['calls']:
                vertices = list(zip(packet[1:7:2],packet[2:7:2]))
                uv = list(zip(packet[7:13:2],packet[8:13:2]))
                expected_fb, ws, rs = clipped_expectation(vertices,uv,current)
                current = (current[0],expected_fb,current[2])
                stores.extend(ws)
                reads.extend(rs)
            assert result['framebuffer_writes'] == stores
            assert result['accesses'] == reads
            assert bytes(cpu.mem_read(FRAMEBUFFER,65536)) == current[1]
            report.setdefault('transformed_oracle_cases',0)
            report['transformed_oracle_cases'] += 1
            report['paths'].update(result['path'])
            report['transformed_cases'] += 1
    splitter_cases, splitter_paths = direct_splitter_checks(pe)
    report['splitter_cases'] = splitter_cases
    report['paths'].update(splitter_paths)
    wrapper_cases, wrapper_paths = direct_wrapper_checks(pe)
    report['wrapper_cases'] = wrapper_cases
    report['paths'].update(wrapper_paths)
    for vertices, uv in [([(1024,1024),(1025,1025),(1026,1026)],
            [(0,0),(0x7fffffff,0),(0,0)]),
            ([(1024,1024),(1024,1024),(1024,1024)],[(0,0)]*3)]:
        result, _, _ = execute(pe,vertices,uv)
        report['fault_cases'].append({'vertices':vertices, 'uv':uv,'fault':result['fault'],'eip':result['eip']})
    assert report['fault_cases'][0]['fault'] == 'Unhandled CPU exception (UC_ERR_EXCEPTION)'
    assert report['fault_cases'][0]['eip'] == '0x64eb44'
    assert report['fault_cases'][1]['fault'] is None
    report['paths'] = sorted(report['paths'])
    report['triangle_return_sites'] = sorted(set(report['paths']) &
        {'0x45cc72','0x45cc8d','0x45cca8','0x45ccc3','0x45cf7d','0x45d164'})
    assert len(report['triangle_return_sites']) == 6
    report['scope'] = 'Original-only expressions, unified clipped rendering oracle, and direct splitter/wrapper contracts; no production C or native validation'
    return report


def inspect(pe):
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {a: n for a, n, l, c, b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    pins = {
        0x5ca50:'4458bb7690590a3289e6ce7d99b3a80338382fe2afc3b67a25891b2af0e5f8bf',
        0x24ea80:'83c0abc6600657871b1ca05baeb39fc3977b6e154ebcfe7578ce339495cfb7aa',
        0x24ec00:'3d7cf4d9c890809aee52eff06fe958abf5987150fee36ea62311c57118177390',
        0x24ec40:'31d786c6ae0152819d172f452e335972f98e2cf739cfc0821da8743c0ca791d3',
        0x5c990:'bc5e0b937897780cefd8d87d3b95fff67e75ef98d1a91d60e0d2ceb368f0b9b8',
        0x5c9b0:'e3dbc55544ab3db0f77135a83fac72ff4ba1293984655e31b72723eb1be5bf0d',
        0x5d7a0:'fee5ce3932998c6905816c67a42acfbc80612edfa7abc7d724cc90fec87f3bb2',
        0x5d170:'d1d8d33f6f947f949ae7160d07da0fcc3e66d4a67168ad30c5eb80fa62f175f7',
        0x5c9f0:'922d88eb43e5d8b7ede3d71df0d81a543dc389bfad921e3faa13815d0ac5468e',
        0x67f1c:'d7b77c5b6446f23ae6be7ab2cb3e5034585cfe71362cf792222dd3003f3ed05c',
        0x67f42:'2d40bc4ff2b77b85bdba551897c8fc3ad8cf7437670fa2bff0afaa59aa827309',
        0x67f97:'b5ca9a8fa2762fafec0f0b81c28c95a7136d797e931800496e82aa6d7607fbab',
        0x24f4d0:'26ecf3f443aeac09a0baa2abe2104e6a883bf3b7d9c45d1fa3c20272dc944383',
        0x24f1e0:'a236e05fa139724a52b65926d5407dead47076e422411d8a02528937d2c48811',
        0x24f220:'39c7e37061dbb0ba74b5e5735eee6081a8d152da7a6391807d978cc1e9c0bbe6',
        0x24efa0:'e10f563c0f702038ca423f24d915feeb909d74f16dad34b9eba4581dbe78209c',
        0x24f0c0:'481a1b8d63fbc07baff6bb008b9916fe07f4e5f7cc1c5a0cb7808e4f7c08fbb2',
        0x24f270:'6c18e8828be874b0f51f9f8e808820be102209247fa3746bbccfae3fed7361c3',
        0x24f489:'65b2cb3d21d75ff2be58a259eb79e9430f6b01539cb992e6722b2c680aeb27f0',
        0x5c9d0:'fa88078708ae1fc6bd627bf2fd66459e95f5595bcc828ec82e108d4d7b2bf31e',
    }
    results = []
    for rva, size in [(0x5ca50, 1813), (0x24ea80, 380), (0x24ec00, 59),
            (0x24ec40, 59), *[(r, fpo[r]) for r in
            (0x5c990, 0x5c9b0, 0x5d7a0, 0x5d170, 0x5c9f0)],
            (0x67f1c, 38), (0x67f42,85), (0x67f97,53),
            (0x24f4d0, 199), (0x24f1e0, 51),
            (0x24f220, 70), (0x24efa0, 282), (0x24f0c0, 282),
            (0x24f270, 537), (0x24f489,63), (0x5c9d0,fpo[0x5c9d0])]:
        raw = pe.get_data(rva, size)
        instructions = list(md.disasm(raw, rva+0x400000))
        digest = hashlib.sha256(raw).hexdigest()
        assert digest == pins[rva]
        assert sum(i.size for i in instructions) == size and instructions[-1].mnemonic == 'ret'
        starts = {i.address for i in instructions}
        CODE_STARTS.update(starts)
        assert all(int(i.op_str,16) in starts for i in instructions if i.mnemonic.startswith('j'))
        if rva in fpo: assert fpo[rva] == size
        results.append({'rva':hex(rva),'size':size,'sha256':digest,
            'calls':[(hex(i.address),i.op_str) for i in instructions if i.mnemonic == 'call']})
        (BUILD/f'triangle_dep_{rva:x}.txt').write_text('\n'.join(
            f'{i.address:08x}: {i.mnemonic} {i.op_str}' for i in instructions)+'\n',
            encoding='utf-8')
    assert pe.get_data(0xbad60,0x190) == bytes(0x190)
    for va in (0x51a9a0,0x51aa00,0x63f2b0,0x63f2d4):
        section = next(s for s in pe.sections if s.VirtualAddress <= va-0x400000 < s.VirtualAddress+s.Misc_VirtualSize)
        assert va-0x400000-section.VirtualAddress >= section.SizeOfRawData
    for row in inspect_sprite(pe)[:2]:
        rva, size = int(row['rva'],16), row['size']
        CODE_STARTS.update(i.address for i in md.disasm(pe.get_data(rva,size),rva+0x400000))
        results.append(row)
    return results


if __name__ == '__main__':
    verify_target()
    BUILD.mkdir(parents=True, exist_ok=True)
    pe = pefile.PE('Ignition/Ignition/IGN_WIN.EXE')
    extents = inspect(pe)
    report = experiments(pe)
    report['extents'] = extents
    report['target_sha256'] = hashlib.sha256(pe.__data__).hexdigest()
    report['input_sha256'] = {path:hashlib.sha256(open(path,'rb').read()).hexdigest() for path in
        ('tools/analyze_triangle.py','tools/analyze_sprite_rasterizer.py','tools/verify_sprite_backend.py','decomp/target.json')}
    (BUILD/'triangle_analysis.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps({k:v for k,v in report.items() if k not in ('paths','extents')}, indent=2))
