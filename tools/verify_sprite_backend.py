# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Native sprite publication: actual production C versus authenticated x86.

The zero-argument downstream boundary is modeled after independent original
adapter/lookup execution. No rasterizer or mathematical library is substituted.
"""
import hashlib
import random
import shutil
import struct
import subprocess

import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE, UC_HOOK_MEM_WRITE
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX,
    UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP,
    UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EFLAGS, UC_X86_REG_FPCW,
    UC_X86_REG_FPSW)
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run

RVA, SIZE = 0x571b0, 145
DLL = BUILD/'sprite_backend_validation.dll'
INPUTS = ['decomp/src/geputget.c', 'decomp/include/geputget.h',
    'decomp/include/mem.h', 'decomp/target.json', 'tools/verify_sprite_backend.py',
    'tools/verify_matching.py', 'tools/windows_target.py', 'tools/windows_tracking.py']
SAVED = [UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP]
STACK, STOP, DATA = 0x7000000, 0x7100000, 0x7200000
ORIGINAL_FIELDS = [('request',0x4ba708,12),('coefficients',0x50ec10,16),
                   ('active',0x50ebfc,4)]
RELOCS = [(0x571bc,0x4ba708),(0x571c2,0x4ba710),(0x571cd,0x4ba70c),
    (0x571dd,0x47aed8),(0x571fd,0x50ec10),(0x5720f,0x50ec14),
    (0x57214,0x50ec18),(0x5721a,0x4ba710),(0x5721e,0x50ec10),
    (0x57224,0x50ec1c),(0x5722a,0x50ebfc),(0x5722e,0x4ba708)]


def record(kind, outcome, cases=0, **details):
    record_run(RVA,kind,outcome,inputs=INPUTS,
        artifact=DLL.relative_to(ROOT).as_posix() if DLL.exists() else None,
        cases=cases,command='uv run tools/verify_sprite_backend.py' if __name__=='__main__'
        else 'uv run tools/verify_matching.py',details=details)


def inspect_original(pe):
    md = Cs(CS_ARCH_X86,CS_MODE_32)
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type==3)
    fpo = {a:(n,l,p,b) for a,n,l,p,b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    routines = [(RVA,SIZE,'f2fe1b07e3a37fe080d3cfcaeadad8e3bae3d2ca326e16c4a99e6ac563e01dac'),
        (0x56d20,25,'e04def2288d23041976ef0a043cea2b296558f74cf82cce58476bf765e804623'),
        (0x57370,167,'203cc8648be172994ad229e423e9f05c83ff3a5d541277de7e9864ac47ae79fe'),
        (0x612e0,114,'75f04bc300b71fce757262cbb88c22e365cd785b8fc1a87578179dba1c442858')]
    decoded = []
    for r,n,digest in routines:
        assert hashlib.sha256(pe.get_data(r,n)).hexdigest()==digest
        ins = list(md.disasm(pe.get_data(r,n),r+0x400000))
        assert sum(i.size for i in ins)==n and ins[-1].mnemonic=='ret'
        decoded += ins
    assert fpo[RVA]==(145,1,3,289)
    assert fpo[0x57370]==(167,16,0,267)
    assert fpo[0x612e0]==(114,0,2,520)
    assert pe.get_data(RVA+SIZE,15)==b'\xcc'*15
    actual = [(e.rva,struct.unpack('<I',pe.get_data(e.rva,4))[0])
        for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries
        if e.type==3 and RVA<=e.rva<RVA+SIZE]
    assert actual==RELOCS
    assert pe.get_data(0x7aed8,4)==struct.pack('<f',65536)
    assert pe.get_data(0x56ea6,10)==bytes.fromhex('c705bceb5000b0714500')
    reloc_sites = {e.rva for b in pe.DIRECTORY_ENTRY_BASERELOC for e in b.entries if e.type==3}
    assert {0x56ea8,0x56eac,0x56d31}<=reloc_sites
    init = list(md.disasm(pe.get_data(0x56e60,181),0x456e60))
    assert init[-1].mnemonic=='ret'
    assert pe.get_data(0x56af0,30).hex()==(
        '8b4c240485c97510e8934b0000e85e030000b801000000c3b802000000c3')
    assert [(i.address,i.op_str) for i in init if i.mnemonic=='call']==[
        (0x456f00,'0x45d840'),(0x456f05,'0x45c9f0'),(0x456f0a,'0x45c7f0')]
    assert [(i.address,i.op_str) for i in decoded[:len(list(md.disasm(pe.get_data(RVA,SIZE),0x4571b0)))]
        if i.mnemonic=='call']==[(0x4571e1,'0x46950c'),(0x4571f7,'0x46950c'),
                               (0x457209,'0x46950c'),(0x457232,'0x457370')]
    # Non-null caller packs scale and angle from raw stack float words, positions
    # from integers shifted eight, then pushes three pointer arguments.
    assert pe.get_data(0xb7d0,66).hex()==(
        '8b44240883ec14c1e008908b5424288b4c242489442408894c24008b442420'
        '8d4c2400c1e0085189442410895424088d44240c8b54241c5052e812b5040083c420c3')
    assert pe.get_data(0xba708,12)==bytes(12)
    assert pe.get_data(0xba778,16)==struct.pack('<4I',4,0,0x4ba758,0)
    assert 0xba780 in reloc_sites
    for va in (0x50ebbc,0x50ebfc,0x50ec10,0x51fb88,0x520388,0x63f2d8):
        section = next(s for s in pe.sections if s.VirtualAddress<=va-0x400000<s.VirtualAddress+s.Misc_VirtualSize)
        assert va-0x400000-section.VirtualAddress>=section.SizeOfRawData
    helper = list(md.disasm(pe.get_data(0x6950c,39),0x46950c))
    assert helper[-1].mnemonic=='ret' and not any(i.mnemonic=='call' for i in helper)
    rasterizer = list(md.disasm(pe.get_data(0x65bb5,437),0x465bb5))
    assert sum(i.size for i in rasterizer)==437 and rasterizer[-1].mnemonic=='ret'
    assert rasterizer[0].mnemonic=='pushal'
    assert sum(i.mnemonic=='popal' for i in rasterizer)==2
    assert [(i.address,i.op_str) for i in rasterizer if i.mnemonic=='call']==[(0x465be3,'0x468c70')]
    transformed = list(md.disasm(pe.get_data(0x68c70,631),0x468c70))
    assert sum(i.size for i in transformed)==631 and transformed[-1].mnemonic=='ret'
    assert [(i.address,i.op_str) for i in transformed if i.mnemonic=='call']==[
        (0x468e6b,'0x45ca50'),(0x468edf,'0x45ca50')]
    assert pe.get_data(0x68cb2,4)==bytes.fromhex('8b19f7eb')
    assert pe.get_data(0x68cc6,5)==bytes.fromhex('8b5908f7eb')
    (BUILD/'sprite_backend_original.txt').write_text('\n'.join(
        f'{i.address:#010x}: {i.mnemonic} {i.op_str}' for i in decoded+init+helper+rasterizer+transformed)+'\n',encoding='utf-8')


def build_backend():
    BUILD.mkdir(parents=True,exist_ok=True)
    source = (ROOT/'decomp/src/geputget.c').read_text(encoding='utf-8-sig')
    start = source.index('volatile GfxSpriteRequest g_nativeSpriteRequest =')
    opening = source.index('{',source.index('int Gfx_DrawSpriteNative(',start))
    end,depth = opening+1,1
    while depth:
        depth += (source[end]=='{')-(source[end]=='}')
        end += 1
    unit = '#include "geputget.h"\n'+source[start:end]+'\n'
    for name,condition in [('request_size','sizeof(GfxSpriteRequest)==12'),
        ('handle_prefix','sizeof(GfxSpriteHandle)==12'),('transform_size','sizeof(GfxSpriteTransform)==8'),
        ('coefficients_size','sizeof(GfxSpriteCoefficients)==16'),
        ('request_handle','offsetof(GfxSpriteRequest,handle)==4'),
        ('request_coefficients','offsetof(GfxSpriteRequest,coefficients)==8'),
        ('angle_offset','offsetof(GfxSpriteTransform,angle_radians)==4'),
        ('x87_storage','sizeof(long double)==12')]:
        unit += f'typedef char {name}_check[{condition} ? 1 : -1];\n'
    path,obj = BUILD/'sprite_backend_unit.c',BUILD/'sprite_backend.obj'
    path.write_text(unit,encoding='utf-8')
    trap,trap_obj = BUILD/'sprite_submit_trap.c',BUILD/'sprite_submit_trap.obj'
    trap.write_text('void Gfx_SubmitSpriteRequest(void) { for (;;) {} }\n',encoding='utf-8')
    for product in (obj,trap_obj,DLL,DLL.with_suffix('.lib'),DLL.with_suffix('.exp')):
        product.unlink(missing_ok=True)
    cc,ld = shutil.which('gcc'),shutil.which('lld-link')
    if not cc or not ld: raise RuntimeError('GCC x86/x87 and LLD required for sprite validation')
    flags = ['-m32','-std=c89','-pedantic-errors','-Wall','-Wextra','-Werror','-O2',
        '-mfpmath=387','-mno-sse','-mno-sse2','-fno-math-errno',
        '-funsafe-math-optimizations','-fno-associative-math','-fno-reciprocal-math',
        '-fsigned-zeros','-fno-finite-math-only','-fno-tree-vectorize',
        '-I',str(ROOT/'decomp/include')]
    commands = [[cc,*flags,'-c',str(path),'-o',str(obj)],
        [cc,*flags,'-c',str(trap),'-o',str(trap_obj)],
        [ld,'/dll','/noentry','/nodefaultlib','/safeseh:no','/machine:x86','/base:0x10000000',
         '/out:'+str(DLL),str(obj),str(trap_obj)]+['/export:'+s for s in
         ('Gfx_DrawSpriteNative','Gfx_SubmitSpriteRequest','g_nativeSpriteRequest',
          'g_nativeSpriteCoefficients','g_activeSpriteRequest')]]
    for command in commands:
        print(subprocess.list2cmdline(command),flush=True)
        subprocess.run(command,cwd=ROOT,check=True)
    return commands


def cpu_image(pe):
    cpu = Uc(UC_ARCH_X86,UC_MODE_32)
    base = pe.OPTIONAL_HEADER.ImageBase
    size = (pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095
    cpu.mem_map(base,size)
    cpu.mem_write(base,pe.get_memory_mapped_image())
    for addr in (STACK,STOP,DATA): cpu.mem_map(addr,0x10000)
    return cpu,base,size


def downstream_checks(pe):
    """Execute real 57370 + real 612E0, stop only at ESI rasterizer 465BB5.

    The separately derived expectation covers ALL adapter stores and lookup paths;
    it is not used as a substitute rasterizer in differential backend execution.
    """
    cases = 0
    rng = random.Random(0x457370)
    for image_id in (0,-1,-2147483648,1,2,3,2147483647):
        for coefficient_pointer in (0,0x50ec10):
            for empty_slot in (False,True):
                cpu,base,size = cpu_image(pe)
                descriptor = rng.randbytes(64)
                fallback = rng.randbytes(64)
                hx,hy = rng.getrandbits(32),rng.getrandbits(32)
                position = struct.pack('<2I',rng.getrandbits(32),rng.getrandbits(32))
                cpu.mem_write(DATA,struct.pack('<i2I',image_id,hx,hy))
                cpu.mem_write(DATA+32,position)
                cpu.mem_write(DATA+64,descriptor)
                cpu.mem_write(DATA+128,struct.pack('<3I',0,0 if empty_slot else DATA+64,DATA+64))
                cpu.mem_write(0x51fb88,fallback)
                cpu.mem_write(0x520388,struct.pack('<2I',3,DATA+128))
                cpu.mem_write(0x4ba708,struct.pack('<3I',DATA+32,DATA,coefficient_pointer))
                cpu.mem_write(0x50ebfc,struct.pack('<I',0x4ba708))
                cpu.mem_write(0x63f2d8,struct.pack('<I',0x12340000))
                sp = STACK+0x8000
                cpu.mem_write(sp,struct.pack('<I',STOP))
                cpu.reg_write(UC_X86_REG_ESP,sp)
                cpu.reg_write(UC_X86_REG_EFLAGS,2)
                saved = [rng.getrandbits(32) for _ in SAVED]
                for reg,value in zip(SAVED,saved): cpu.reg_write(reg,value)
                before = bytes(cpu.mem_read(base,size))
                writes,calls = [],[]
                chosen = descriptor if 0<image_id<3 and not (image_id==1 and empty_slot) else fallback
                d = struct.unpack('<16I',chosen)
                expected = [(0x4ba77c,DATA+32),(0x4ba784,coefficient_pointer),
                    (0x4ba758,hx),(0x4ba75c,hy),(0x4ba760,(d[4]&255)<<8),
                    (0x4ba764,d[4]&0xff00),(0x4ba770,d[4]&0xffff0000),
                    (0x4ba768,((d[1]<<8)+((d[4]&255)<<8))&0xffffffff),
                    (0x4ba76c,((d[2]<<8)+(d[4]&0xff00))&0xffffffff),
                    (0x4ba774,0x12340000)]
                def code(uc,address,length,unused):
                    if address==0x465bb5:
                        assert uc.reg_read(UC_X86_REG_ESI)==0x4ba778
                        calls.append(bytes(uc.mem_read(0x4ba758,48)))
                        ret_sp = uc.reg_read(UC_X86_REG_ESP)
                        uc.reg_write(UC_X86_REG_EIP,struct.unpack('<I',uc.mem_read(ret_sp,4))[0])
                        uc.reg_write(UC_X86_REG_ESP,ret_sp+4)
                    else:
                        assert 0x457370<=address<0x457417 or 0x4612e0<=address<0x461352
                def write(uc,access,address,length,value,unused):
                    if STACK<=address<STACK+0x10000: return
                    assert length==4
                    writes.append((address,value&0xffffffff))
                cpu.hook_add(UC_HOOK_CODE,code)
                cpu.hook_add(UC_HOOK_MEM_WRITE,write)
                cpu.emu_start(0x457370,STOP,count=10000)
                assert writes==expected and len(calls)==1
                assert cpu.reg_read(UC_X86_REG_EAX)==0x12340000
                assert cpu.reg_read(UC_X86_REG_EDX)==hy
                assert cpu.reg_read(UC_X86_REG_ESP)==sp+4
                assert [cpu.reg_read(r) for r in SAVED]==saved
                assert bytes(cpu.mem_read(DATA,12))==struct.pack('<i2I',image_id,hx,hy)
                assert bytes(cpu.mem_read(DATA+32,8))==position
                assert bytes(cpu.mem_read(DATA+64,64))==descriptor
                after = bytearray(cpu.mem_read(base,size))
                for addr,_ in expected: after[addr-base:addr-base+4]=before[addr-base:addr-base+4]
                assert after==before
                # Positive descriptor lookup clears +24/+28 only in its local copy.
                local_copy = bytearray(chosen)
                if chosen==descriptor: local_copy[24:32]=bytes(8)
                assert bytes(cpu.mem_read(sp-64,64))==local_copy
                cases += 1
    print(f'PASS: {cases} original-only downstream adapter/lookup executions; rasterizer intercepted.')
    return cases


def execute(pe,entry,boundary,fields,initial,transform,handle,point,cw,seed,mutation,original=False,wrapper=False):
    cpu,base,size = cpu_image(pe)
    for (_,addr,n),data in zip(fields,initial):
        assert len(data)==n
        cpu.mem_write(addr,data)
    cpu.mem_write(DATA+32,struct.pack('<2I',0x80000000,0x7fffffff))
    cpu.mem_write(DATA+64,struct.pack('<3I',1,0xffffffff,0x12345678))
    if transform is not None: cpu.mem_write(DATA,transform)
    if wrapper: cpu.mem_write(0x50ebbc,struct.pack('<I',0x4571b0))
    sp = STACK+0x8000
    args = struct.pack('<4I',STOP,handle,point,DATA if transform is not None else 0)
    cpu.mem_write(sp,args)
    stack_before = bytes(cpu.mem_read(sp,0x8000))
    saved = [random.Random(seed+r).getrandbits(32) for r in SAVED]
    for reg,value in zip(SAVED,saved): cpu.reg_write(reg,value)
    cpu.reg_write(UC_X86_REG_ESP,sp)
    cpu.reg_write(UC_X86_REG_EFLAGS,2)
    cpu.reg_write(UC_X86_REG_FPCW,cw)
    before = bytes(cpu.mem_read(base,size))
    input_before = bytes(cpu.mem_read(DATA,128))
    req,coeff,active = [a for _,a,_ in fields]
    events,calls = [],[]
    model_writes = []
    def pointer(value):
        return 'request' if value==req else 'coefficients' if value==coeff else value
    def normalized():
        request = list(struct.unpack('<3I',cpu.mem_read(req,12)))
        request[2] = pointer(request[2])
        return request,struct.unpack('<4I',cpu.mem_read(coeff,16)),pointer(struct.unpack('<I',cpu.mem_read(active,4))[0])
    def code(uc,address,length,unused):
        if address==boundary:
            snapshot = normalized()
            assert snapshot[0][0:2]==[point,handle] and snapshot[2]=='request'
            assert snapshot[0][2]==('coefficients' if transform is not None else 0)
            calls.append(snapshot)
            events.append(('call',snapshot))
            # 57370 has zero stack arguments. No additional request pointer is
            # pushed: the only dependency argument is the published global.
            ret_sp = uc.reg_read(UC_X86_REG_ESP)
            return_address = struct.unpack('<I',uc.mem_read(ret_sp,4))[0]
            if original: assert return_address==0x457237
            for index,off,data in mutation:
                addr = fields[index][1]+off
                uc.mem_write(addr,data)
                model_writes.append((addr,len(data)))
            uc.reg_write(UC_X86_REG_EAX,0xdeadbeef)
            uc.reg_write(UC_X86_REG_ECX,0x12345678)
            uc.reg_write(UC_X86_REG_EDX,0x87654321)
            uc.reg_write(UC_X86_REG_ESP,ret_sp+4)
            uc.reg_write(UC_X86_REG_EIP,return_address)
            return
        if original:
            assert 0x4571b0<=address<0x457241 or 0x46950c<=address<0x469533 or (wrapper and 0x456d20<=address<0x456d39)
        else:
            text = next(s for s in pe.sections if s.Name.rstrip(b'\0')==b'.text')
            assert base+text.VirtualAddress<=address<base+text.VirtualAddress+text.Misc_VirtualSize
    def write(uc,access,address,length,value,unused):
        if STACK<=address<STACK+0x10000: return
        for name,addr,n in fields:
            if addr<=address and address+length<=addr+n:
                assert length==4
                v = value&0xffffffff
                if name=='active' or (name=='request' and address-addr==8): v=pointer(v)
                events.append(('write',name,address-addr,v))
                return
        raise AssertionError(f'Unexpected state write {address:#x}/{length}')
    cpu.hook_add(UC_HOOK_CODE,code)
    cpu.hook_add(UC_HOOK_MEM_WRITE,write)
    cpu.emu_start(entry,STOP,count=10000)
    assert cpu.reg_read(UC_X86_REG_EIP)==STOP and cpu.reg_read(UC_X86_REG_ESP)==sp+4
    assert cpu.reg_read(UC_X86_REG_EAX)==1 and len(calls)==1
    assert [cpu.reg_read(r) for r in SAVED]==saved
    assert bytes(cpu.mem_read(sp,0x8000))==stack_before
    assert bytes(cpu.mem_read(DATA,128))==input_before
    assert not cpu.reg_read(UC_X86_REG_EFLAGS)&0x400
    assert cpu.reg_read(UC_X86_REG_FPCW)==cw
    assert (cpu.reg_read(UC_X86_REG_FPSW)>>11)&7==0, 'x87 stack leaked'
    after = bytearray(cpu.mem_read(base,size))
    for _,addr,n in fields: after[addr-base:addr-base+n]=before[addr-base:addr-base+n]
    assert after==before, 'Unexpected image mutation outside published state'
    return normalized(),events


def verify_sprite_backend():
    verify_target()
    phase = 'compilation'
    try:
        commands = build_backend()
        original,rebuilt = pefile.PE(str(TARGET)),pefile.PE(str(DLL))
        inspect_original(original)
        symbols = {e.name.decode():rebuilt.OPTIONAL_HEADER.ImageBase+e.address
            for e in rebuilt.DIRECTORY_ENTRY_EXPORT.symbols if e.name}
        assert not hasattr(rebuilt, 'DIRECTORY_ENTRY_IMPORT')
        text = next(s for s in rebuilt.sections if s.Name.rstrip(b'\0')==b'.text')
        decoded = list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(text.get_data(),rebuilt.OPTIONAL_HEADER.ImageBase+text.VirtualAddress))
        assert any(i.mnemonic=='fcos' for i in decoded)
        assert any(i.mnemonic=='fsin' for i in decoded)
        assert not any(i.mnemonic=='fsincos' for i in decoded)
        assert all(i.op_str==hex(symbols['Gfx_SubmitSpriteRequest']) for i in decoded if i.mnemonic=='call')
        fields = [('request',symbols['g_nativeSpriteRequest'],12),
                  ('coefficients',symbols['g_nativeSpriteCoefficients'],16),
                  ('active',symbols['g_activeSpriteRequest'],4)]
        details = {'scope':'extracted production C; native compiled x87; modeled zero-argument 57370 boundary',
            'commands':commands,'toolchain':{name:subprocess.check_output([shutil.which(name),'--version'],text=True).splitlines()[0]
                for name in ('gcc','lld-link')},
            'limitations':'No rasterizer/framebuffer/native execution or instruction equality; masked FP exceptions, CW precision/rounding variants only; long-double GCC intrinsics provisional'}
        record('compilation','pass',**details)
        phase = 'emulation'
        downstream_cases = downstream_checks(original)
        cases = 0
        rng = random.Random(RVA)
        def compare(transform,cw=0x37f,mutation=(),handle=DATA+64,point=DATA+32,expected=None,wrapper=False):
            nonlocal cases
            initial = [rng.randbytes(n) for _,_,n in fields]
            a = execute(original,0x456d20 if wrapper else 0x4571b0,0x457370,ORIGINAL_FIELDS,
                initial,transform,handle,point,cw,cases,mutation,True,wrapper)
            b = execute(rebuilt,symbols['Gfx_DrawSpriteNative'],symbols['Gfx_SubmitSpriteRequest'],fields,
                initial,transform,handle,point,cw,cases,mutation)
            assert a==b, (transform.hex() if transform else None,hex(cw),a,b)
            # Independently enforce publication sequence and untouched null state.
            writes = [e for e in a[1] if e[0]=='write']
            assert writes[:3]==[('write','request',0,point),('write','request',8,0),('write','request',4,handle)]
            assert writes[-1]==('write','active',0,'request')
            if transform is None:
                assert len(writes)==4 and a[1][-1][1][1]==struct.unpack('<4I',initial[1])
            else:
                assert [(v[1],v[2]) for v in writes]==[('request',0),('request',8),('request',4),
                    ('coefficients',0),('coefficients',4),('coefficients',8),('request',8),
                    ('coefficients',12),('active',0)]
                c,s,s2,c2 = a[1][-1][1][1]
                assert c==c2 and s==s2
                if expected is not None: assert (c,s)==expected,(c,s,expected)
            assert a[1][-1][0]=='call', 'Backend wrote after submission'
            cases += 1
        # Full null ABI: pointers are published, never dereferenced by the backend.
        for cw in (0x7f,0x27f,0x37f,0x77f,0xb7f,0xf7f):
            for handle in (0,DATA+64,0x80000000,0xffffffff):
                for point in (0,DATA+32,0x80000000,0xffffffff):
                    compare(None,cw,handle=handle,point=point,wrapper=True)
        cws = tuple(0x7f | precision | rounding for precision in (0,0x200,0x300) for rounding in (0,0x400,0x800,0xc00))
        scales = (-32768.0,-1.0,-1/65536,-0.0,0.0,1/131072,1/65536,0.99999994,1.0,32767.0,32768.0,65536.0,
                  2**47,2**48,-2**47,2**63,3.4028234663852886e38)
        angles = (-3.1415927410125732,-1.5707963705062866,-0.0,0.0,0.00001,1.0,1.5707963705062866,3.1415927410125732,10000.0,2**63,3.4028234663852886e38)
        for cw in cws:
            for scale in scales:
                for angle in angles:
                    compare(struct.pack('<2f',scale,angle),cw)
        # Adjacent float words around integer conversion and FCOS range thresholds.
        for word in (0x00000001,0x007fffff,0x00800000,0x3effffff,0x3f000000,0x3f7fffff,0x3f800001,
                     0x46ffffff,0x47000000,0x47000001,0x577fffff,0x57800000,0x57800001,
                     0x5effffff,0x5f000000,0x5f000001,0x7f7fffff,0x7f800000,0x7fc00000,0x7f800001):
            for sign in (0,0x80000000):
                compare(struct.pack('<2I',word|sign,0),expected=(0,0) if word>=0x57800000 else None)
                compare(struct.pack('<2I',0x3f800000,word|sign))
        compare(struct.pack('<2f',1,0),expected=(65536,0),wrapper=True)
        compare(struct.pack('<2f',-1,0),expected=(0xffff0000,0),wrapper=True)
        for _ in range(160):
            compare(rng.randbytes(8),rng.choice(cws))
        # The backend does not restore state overwritten by downstream work.
        for transform in (None,struct.pack('<2f',1,1)):
            for mutation in (((0,0,struct.pack('<3I',0,0xdeadbeef,0)),),
                             ((1,0,rng.randbytes(16)),),((2,0,struct.pack('<I',0)),)):
                compare(transform,mutation=mutation)
        record('emulation','pass',cases,**details,original_only_downstream_cases=downstream_cases,
            checks='ordered dword state stores, downstream snapshots/mutations, return/EAX, cdecl stack, callee saves, DF, input/image immutability, x87 CW and stack balance')
        record('raw_bytes','different',original_size=SIZE,
            original_sha256=hashlib.sha256(original.get_data(RVA,SIZE)).hexdigest(),
            scope='Compiler-generated C differs in instructions and extent; no instruction equality claimed')
        print(f'PASS: Gfx_DrawSpriteNative {cases} original-vs-C comparisons; {downstream_cases} original-only adapter cases.')
        return cases
    except Exception as exc:
        record(phase,'fail',error=str(exc))
        raise


if __name__=='__main__':
    if not __debug__: raise RuntimeError('Verification requires assertions')
    verify_sprite_backend()
