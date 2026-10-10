# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Validate extracted production Font_DrawText against authenticated x86 code.

The original dispatch wrapper and _ftol execute. Renderer bodies are intercepted
at their recovered cdecl boundary; no renderer implementation is substituted.
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
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX,
    UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP,
    UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EFLAGS, UC_X86_REG_FPCW)
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run

RVA, SIZE = 0x56660, 978
DLL = BUILD / 'font_draw_validation.dll'
INPUTS = ['decomp/src/geputget.c', 'decomp/include/geputget.h',
          'decomp/include/mem.h', 'decomp/target.json', 'tools/verify_font_draw.py',
          'tools/verify_matching.py', 'tools/windows_target.py', 'tools/windows_tracking.py']
SAVED = [UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP]
STACK, STOP, TEXT = 0x7000000, 0x7100000, 0x7200000
RELOCS = [(0x56688,0x63f2e0),(0x56695,0x63f2e8),(0x566ac,0x63f2fa),
    (0x566bd,0x47aec8),(0x566c8,0x63f2ec),(0x566d5,0x63f2e4),
    (0x56707,0x63f2de),(0x56711,0x63f2fa),(0x56717,0x63f2f0),
    (0x5672b,0x63f2fa),(0x56731,0x63f2f0),(0x56750,0x63f2e4),
    (0x567bb,0x63f2de),(0x567c8,0x63f2fa),(0x567da,0x63f720),
    (0x5681a,0x63f360),(0x56830,0x63f2fa),(0x56836,0x63f2f0),
    (0x5686b,0x63f2e4),(0x568a7,0x63f2de),(0x568bd,0x63f720),
    (0x568c3,0x63f2f0),(0x568d0,0x63f2fa),(0x568e1,0x47aec8),
    (0x568e7,0x47aed0),(0x5690e,0x63f2e4),(0x56979,0x63f2de),
    (0x569a9,0x63f360),(0x569c5,0x63f720),(0x569cb,0x63f2f0),
    (0x569d8,0x63f2fa),(0x569e9,0x47aec8),(0x569ef,0x47aed0)]


def record(kind, outcome, cases=0, **details):
    record_run(RVA, kind, outcome, inputs=INPUTS,
        artifact=DLL.relative_to(ROOT).as_posix() if DLL.exists() else None,
        cases=cases, command='uv run tools/verify_font_draw.py' if __name__ == '__main__'
        else 'uv run tools/verify_matching.py', details=details)


def inspect_original(pe):
    code = pe.get_data(RVA, SIZE)
    assert hashlib.sha256(code).hexdigest() == 'f726ad45a7e5cf42d9fa835cdfcdaf008d57a7540556d9e8ff6c044d5e27d147'
    assert pe.get_data(RVA+SIZE, 14) == b'\xcc'*14
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    records = {a:(n,l,p,b) for a,n,l,p,b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    assert records[RVA] == (978,5,4,5135)
    actual = [(e.rva,struct.unpack('<I',pe.get_data(e.rva,4))[0])
        for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries
        if e.type == 3 and RVA <= e.rva < RVA+SIZE]
    assert actual == RELOCS
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    decoded = list(md.disasm(code,0x456660))
    assert sum(i.size for i in decoded) == SIZE
    assert [(i.address,i.op_str) for i in decoded if i.mnemonic == 'call'] == [
        (0x4566c1,'0x46950c'),(0x45681f,'0x456d20'),(0x4568eb,'0x46950c'),
        (0x4569ae,'0x456d20'),(0x4569f3,'0x46950c')]
    for r,n,digest in [(0x56d20,25,'e04def2288d23041976ef0a043cea2b296558f74cf82cce58476bf765e804623'),
        (0x571b0,145,'f2fe1b07e3a37fe080d3cfcaeadad8e3bae3d2ca326e16c4a99e6ac563e01dac')]:
        assert hashlib.sha256(pe.get_data(r,n)).hexdigest() == digest
        decoded += list(md.disasm(pe.get_data(r,n),r+0x400000))
    assert pe.get_data(0x56ea6,10) == bytes.fromhex('c705bceb5000b0714500')
    assert pe.get_data(0x7aec8,16) == struct.pack('<dd',0.35,1/256)
    for site in (0x402b6f,0x403c52,0x403c80,0x418348,0x4385fd,0x441314):
        i = next(md.disasm(pe.get_data(site-0x400000,5),site))
        assert i.mnemonic == 'call' and i.op_str == '0x456660'
    # The representative caller pushes y=90, x=160, font, text, then cleans 16.
    assert pe.get_data(0x3c49,7) == bytes.fromhex('6a5a68a0000000')
    assert pe.get_data(0x3c5b,3) == bytes.fromhex('83c410')
    helper = list(md.disasm(pe.get_data(0x6950c,39),0x46950c))
    assert helper[-1].mnemonic == 'ret' and sum(i.size for i in helper) == 39
    decoded += helper
    (BUILD/'font_draw_original.txt').write_text('\n'.join(
        f'{i.address:#010x}: {i.mnemonic} {i.op_str}' for i in decoded)+'\n',encoding='utf-8')


def build_font():
    BUILD.mkdir(parents=True,exist_ok=True)
    source = (ROOT/'decomp/src/geputget.c').read_text(encoding='utf-8-sig')
    start = source.index('int Font_DrawText(')
    end = source.index('{',start)+1
    depth = 1
    while depth:
        depth += (source[end]=='{')-(source[end]=='}')
        end += 1
    # Infinite-loop trap is intercepted by Unicorn before it executes. It has
    # no success behavior and is never linked into the production reconstruction.
    unit = ('#include "geputget.h"\nFontSlot g_fonts[MAX_FONTS];\nint g_fileErrorLine;\n'
        'int _fltused;\n'
        'typedef char slot_check[sizeof(FontSlot)==1600 ? 1 : -1];\n'
        'typedef char presence_check[offsetof(FontSlot,glyph_present)==30 ? 1 : -1];\n'
        'typedef char handle_check[offsetof(FontSlot,glyph_handles)==256 ? 1 : -1];\n'
        'typedef char width_check[offsetof(FontSlot,widths)==1152 ? 1 : -1];\n'
        'size_t strlen(const char *s) { const char *p=s; while (*p) p++; return (size_t)(p-s); }\n'
        +source[start:end]+'\n')
    path, obj = BUILD/'font_draw_unit.c', BUILD/'font_draw.obj'
    path.write_text(unit,encoding='utf-8')
    trap, trap_obj = BUILD/'font_draw_trap.c', BUILD/'font_draw_trap.obj'
    trap.write_text('#include "geputget.h"\n'
        'void Gfx_DrawSprite(void *h, Point2D *p, int t) {\n'
        '    (void)h; (void)p; (void)t; for (;;) {}\n}\n',encoding='utf-8')
    for product in (obj,trap_obj,DLL,DLL.with_suffix('.lib'),DLL.with_suffix('.exp')):
        product.unlink(missing_ok=True)
    cc,ld = shutil.which('clang'),shutil.which('lld-link')
    if not cc or not ld:
        raise RuntimeError('Clang/LLD required for focused font draw validation')
    commands = [[cc,'--target=i686-pc-windows-msvc','-std=c89','-pedantic-errors',
        '-Wall','-Wextra','-Werror','-O2','-ffreestanding','-fno-builtin',
        '-fno-vectorize','-fno-slp-vectorize','-mno-sse','-mno-sse2',
        '-I',str(ROOT/'decomp/include'),'-c',str(path),'-o',str(obj)],
        [cc,'--target=i686-pc-windows-msvc','-std=c89','-pedantic-errors',
         '-Wall','-Wextra','-Werror','-O2','-ffreestanding','-fno-builtin',
         '-I',str(ROOT/'decomp/include'),'-c',str(trap),'-o',str(trap_obj)],
        [ld,'/dll','/noentry','/nodefaultlib','/machine:x86','/base:0x10000000',
         '/out:'+str(DLL),str(obj),str(trap_obj)]+['/export:'+s for s in
         ('Font_DrawText','Gfx_DrawSprite','g_fonts','g_fileErrorLine')]]
    for command in commands:
        print(subprocess.list2cmdline(command),flush=True)
        subprocess.run(command,cwd=ROOT,check=True)
    return commands


def u32(value):
    return value & 0xffffffff


def s32(value):
    value = u32(value)
    return value if value < 0x80000000 else value-0x100000000


def trunc_half(value):
    return value//2 if value >= 0 else -((-value)//2)


def oracle(state, text, slot, x, y, cw, actions):
    """Instruction-derived observable model, including live reads after calls."""
    state, text = bytearray(state),bytearray(text+b'\0')
    o = slot*1600
    def d(off): return struct.unpack_from('<i',state,o+off)[0]
    def h(off): return struct.unpack_from('<h',state,o+off)[0]
    if slot >= 30 or not d(0): return 1040,[],bytes(state),bytes(text)
    if d(8): return 2,[],bytes(state),bytes(text)
    proportional = bool(d(12))
    def present(c): return state[o+c-2] == 1
    def advance(c):
        if present(c): return (h(1088+2*c) if proportional else h(26))+d(16)
        if c != 32: return 0
        if not proportional: return h(26)+d(16)
        return int(h(26)*0.35) if cw == 0x27f else int(h(26)*Fraction.from_float(0.35))
    cursor = u32(x)
    if d(4):
        for byte in text[:-1]:
            if not byte: break
            c = byte if byte < 128 else byte-256
            cursor = u32(cursor+advance(c))
        if d(4)==1: cursor = u32(x+trunc_half(s32(x-cursor)))
        elif d(4)==2: cursor = u32(x+x-cursor)
        else: cursor = u32(x)
    calls = []
    i = 0
    while i < text.index(0):
        byte = text[i]
        c = byte if byte < 128 else byte-256
        if present(c):
            inset = 0 if proportional else trunc_half(h(26)-h(1088+2*c))
            handle = struct.unpack_from('<I',state,o+128+4*c)[0]
            calls.append((handle,u32((cursor+inset)*256),u32(y*256),0,bytes(state),bytes(text)))
            apply_action(state,text,actions.get(len(calls)-1,{}))
            # The original never rechecks presence after rendering.
            cursor = u32(cursor+(h(1088+2*c) if proportional else h(26))+d(16))
        else:
            cursor = u32(cursor+advance(c))
        i += 1
    return 1,calls,bytes(state),bytes(text)


def apply_action(state,text,action):
    for off,data in action.get('state',[]): state[off:off+len(data)] = data
    for off,value in action.get('text',[]): text[off] = value


def execute(pe, entry, fonts, error, boundary, state, text, slot, x, y,
            seed, cw, actions, original=False, null_text=False):
    base = pe.OPTIONAL_HEADER.ImageBase
    size = (pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095
    cpu = Uc(UC_ARCH_X86,UC_MODE_32)
    cpu.mem_map(base,size)
    cpu.mem_write(base,pe.get_memory_mapped_image())
    cpu.mem_write(fonts,bytes(state))
    cpu.mem_write(error,struct.pack('<I',0x12345678))
    if original:
        cpu.mem_write(0x50ebbc,struct.pack('<I',boundary))
    for address in (STACK,STOP,TEXT): cpu.mem_map(address,0x10000)
    cpu.mem_write(TEXT,text+b'\0')
    sp = STACK+0x8000
    cpu.mem_write(sp,struct.pack('<5I',STOP,0 if null_text else TEXT,u32(slot),u32(x),u32(y)))
    caller_stack = bytes(cpu.mem_read(sp,0x8000))
    rng = random.Random(seed)
    saved = [rng.getrandbits(32) for _ in SAVED]
    for reg,value in zip(SAVED,saved): cpu.reg_write(reg,value)
    cpu.reg_write(UC_X86_REG_ESP,sp)
    cpu.reg_write(UC_X86_REG_EFLAGS,2)
    cpu.reg_write(UC_X86_REG_FPCW,cw)
    before = bytes(cpu.mem_read(base,size))
    calls, writes = [],[]
    modeled = False
    def hook(uc,address,length,unused):
        nonlocal modeled
        if address == boundary:
            current_sp = uc.reg_read(UC_X86_REG_ESP)
            ret,handle,point,transform = struct.unpack('<4I',uc.mem_read(current_sp,16))
            assert STACK <= point < sp-8 and transform == 0
            px,py = struct.unpack('<2I',uc.mem_read(point,8))
            calls.append((handle,px,py,transform,bytes(uc.mem_read(fonts,len(state))),
                          bytes(uc.mem_read(TEXT,len(text)+1))))
            action = actions.get(len(calls)-1,{})
            modeled = True
            for off,data in action.get('state',[]): uc.mem_write(fonts+off,data)
            for off,value in action.get('text',[]): uc.mem_write(TEXT+off,bytes([value]))
            # Exercise ignored return values and caller-volatile registers.
            for reg in (UC_X86_REG_EAX,UC_X86_REG_ECX,UC_X86_REG_EDX):
                uc.reg_write(reg,rng.getrandbits(32))
            uc.reg_write(UC_X86_REG_ESP,current_sp+4)
            uc.reg_write(UC_X86_REG_EIP,ret)
            modeled = False
            return
        if original:
            assert (0x456660 <= address < 0x456a32 or
                    0x456d20 <= address < 0x456d39 or 0x46950c <= address < 0x469533), hex(address)
        else:
            section = next(s for s in pe.sections if s.Name.rstrip(b'\0')==b'.text')
            assert base+section.VirtualAddress <= address < base+section.VirtualAddress+section.Misc_VirtualSize
    def write_hook(uc,access,address,length,value,unused):
        if not modeled and not STACK <= address < STACK+0x10000:
            writes.append((address,length,value))
    cpu.hook_add(UC_HOOK_CODE,hook)
    cpu.hook_add(UC_HOOK_MEM_WRITE,write_hook)
    cpu.emu_start(entry,STOP,count=2000000)
    assert cpu.reg_read(UC_X86_REG_EIP)==STOP and cpu.reg_read(UC_X86_REG_ESP)==sp+4
    assert [cpu.reg_read(r) for r in SAVED]==saved
    assert bytes(cpu.mem_read(sp,0x8000))==caller_stack
    assert cpu.reg_read(UC_X86_REG_FPCW)==cw and not cpu.reg_read(UC_X86_REG_EFLAGS)&0x400
    assert not writes, writes
    after = bytearray(cpu.mem_read(base,size))
    end_state = bytes(cpu.mem_read(fonts,len(state)))
    after[fonts-base:fonts-base+len(state)] = before[fonts-base:fonts-base+len(state)]
    assert after == before, 'Unexpected global change outside modeled font writes'
    return cpu.reg_read(UC_X86_REG_EAX),calls,end_state,bytes(cpu.mem_read(TEXT,len(text)+1))


def fixture(slot, height=20, spacing=2, proportional=0, alignment=0, flag=0, in_use=1):
    state = bytearray(48000)
    o = slot*1600
    struct.pack_into('<4Ii',state,o,in_use,alignment & 0xffffffff,flag,proportional,spacing)
    struct.pack_into('<h',state,o+26,height)
    for c in range(32,128):
        state[o+c-2] = 1
        struct.pack_into('<h',state,o+1088+2*c,c-53)
        struct.pack_into('<I',state,o+128+4*c,0x80000000+slot*256+c)
    state[o+30] = 0
    return state


def verify_font_draw():
    verify_target()
    phase = 'compilation'
    try:
        commands = build_font()
        original, rebuilt = pefile.PE(str(TARGET)),pefile.PE(str(DLL))
        inspect_original(original)
        assert not hasattr(rebuilt, 'DIRECTORY_ENTRY_IMPORT')
        symbols = {e.name.decode():rebuilt.OPTIONAL_HEADER.ImageBase+e.address
                   for e in rebuilt.DIRECTORY_ENTRY_EXPORT.symbols if e.name}
        details = {'scope':'extracted production C; original dispatch wrapper and _ftol; modeled renderer boundary',
            'commands':commands,'toolchain':{name:subprocess.check_output([shutil.which(name),'--version'],text=True).splitlines()[0]
                                           for name in ('clang','lld-link')},
            'limitations':'Renderer bodies/native visuals/full geputget.c and instruction equality unverified'}
        record('compilation','pass',**details)
        phase = 'emulation'
        cases = 0
        def compare(slot,state,text,x=101,y=-7,cw=0x37f,actions=None,expected_calls=None,null_text=False):
            nonlocal cases
            actions = actions or {}
            expected = oracle(state,text,slot,x,y,cw,actions)
            a = execute(original,0x456660,0x63f2e0,0x4bab34,0x4571b0,
                        state,text,slot,x,y,cases,cw,actions,True,null_text)
            b = execute(rebuilt,symbols['Font_DrawText'],symbols['g_fonts'],symbols['g_fileErrorLine'],
                        symbols['Gfx_DrawSprite'],state,text,slot,x,y,cases,cw,actions,False,null_text)
            assert a == b == expected, (slot,text,x,y,cw,a[:2],b[:2],expected[:2])
            if expected_calls is not None: assert [v[:4] for v in a[1]] == expected_calls
            cases += 1
        # Independently asserted positions, rather than only an identical model.
        compare(0,fixture(0),b'AB',expected_calls=[(0x80000041,105*256,u32(-7*256),0),(0x80000042,126*256,u32(-7*256),0)])
        for slot in range(30):
            for proportional in (0,1,2,0xffffffff):
                for alignment in (0,1,2,3,0xffffffff):
                    state = fixture(slot,proportional=proportional,alignment=alignment)
                    compare(slot,state,b'A B?')
                    compare(slot,state,b'')
        for in_use in (0,1,2,0xffffffff):
            for flag in (0,1,2,0xffffffff):
                compare(29,fixture(29,in_use=in_use,flag=flag),b'AB',null_text=not in_use or bool(flag))
        for slot in (30,31,0x7fffffff):
            compare(slot,fixture(0),b'AB',null_text=True)
        for cw in (0x27f,0x37f):
            for height in (-32768,-20,-1,0,1,20,32767):
                for prop in (0,1):
                    for alignment in (0,1,2):
                        compare(0,fixture(0,height=height,proportional=prop,alignment=alignment),b' A A',cw=cw)
        # Fixed inset uses truncation toward zero for both signs and odd values.
        for width in (-32768,-21,-1,0,19,20,21,23,32767):
            state = fixture(0)
            struct.pack_into('<h',state,1152+(65-32)*2,width)
            compare(0,state,b'AA')
        for presence in (0,1,2,255):
            for prop in (0,1):
                state = fixture(1,proportional=prop)
                state[1600+30] = presence
                state[1600+65-2] = presence
                compare(1,state,b' A A')
        rng = random.Random(0x456660)
        for index in range(180):
            slot = rng.randrange(1,30)
            state = fixture(slot,height=rng.randrange(-32768,32768),
                spacing=rng.choice([-2147483648,-1,0,2147483647]),
                proportional=rng.randrange(3),alignment=rng.choice([0,1,2,3,0xffffffff]))
            o = slot*1600
            header = bytes(state[o:o+30])
            for c in range(-128,128):
                state[o+c-2] = rng.choice([0,1,2,255])
                struct.pack_into('<I',state,o+128+4*c,rng.getrandbits(32))
                struct.pack_into('<h',state,o+1088+2*c,rng.randrange(-32768,32768))
            state[o:o+30] = header
            text = bytes(range(1,256)) if index==0 else bytes(rng.randrange(1,256) for _ in range(30))
            compare(slot,state,text,x=rng.choice([-2147483648,-1,0,101,2147483647]),
                    y=rng.choice([-2147483648,-1,0,2147483647]),cw=rng.choice([0x27f,0x37f]))
        # Rendering models intentionally mutate only explicit fixture state;
        # compare snapshots, later calls and final state to prove live rereads.
        for prop in (0,1):
            for alignment in (0,1,2):
                for action in (
                    {'state':[(26,struct.pack('<h',31)),(16,struct.pack('<i',-5))]},
                    {'state':[(1152+66,struct.pack('<h',-17))]},
                    {'state':[(66-2,b'\x02')]},
                    {'state':[(128+4*66,struct.pack('<I',0xdeadbeef))]},
                    {'state':[(12,struct.pack('<I',1-prop)),(8,struct.pack('<I',2))]},
                    {'text':[(1,0)]}, {'text':[(1,32)]}):
                    compare(0,fixture(0,proportional=prop,alignment=alignment),b'ABA',actions={0:action})
        # Negative slot has no lower guard. Surrounding original storage is not
        # the C table contract: preserve as separately labeled original-only evidence.
        original_only = 0
        for in_use,flag,result in ((0,0,1040),(1,1,2),(1,0,1)):
            state = fixture(0,in_use=in_use,flag=flag)
            got = execute(original,0x456660,0x63f2e0-1600,0x4bab34,0x4571b0,
                          state[:1600],b'',-1,0,0,999,0x37f,{},True)
            assert got[0]==result and not got[1]
            original_only += 1
        record('emulation','pass',cases,**details,original_only_cases=original_only,
               checks='ordered calls/arguments/point values, boundary snapshots/model mutations, EAX, image/text state, stack, saved registers, DF, x87 control word')
        print(f'PASS: Font_DrawText {cases} original-vs-C comparisons; {original_only} original-only negative-slot checks.')
        return cases
    except Exception as exc:
        record(phase,'fail',error=str(exc))
        raise


if __name__ == '__main__':
    if not __debug__: raise RuntimeError('Verification requires assertions')
    verify_font_draw()
