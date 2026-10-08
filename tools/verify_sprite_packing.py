# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Packing allocation contracts; CRT heap is an explicit validation boundary."""
import hashlib
import random
import struct
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import (Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE,
    UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE, UC_HOOK_MEM_INVALID, UC_HOOK_INTR,
    UC_MEM_WRITE, UC_MEM_WRITE_UNMAPPED)
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX,
    UC_X86_REG_EIP, UC_X86_REG_ESP, UC_X86_REG_EFLAGS)
from verify_matching import PRESERVED, STACK, STOP
from build_decomp import build
from windows_target import ROOT, BUILD, TARGET, verify_target
from windows_tracking import record_run

if not __debug__:
    raise RuntimeError('Verification requires assertions')

MASK = 0xffffffff
ARENA, SIZE = 0x3000000, 0x100000
TEMPLATE, HEAD = 0x51fc00, 0x51fe40
DLL = BUILD / 'sprite_packing_validation.dll'
ROUTINES = {
    'Gfx_AddSpritePackingPage': (0x617e0,85,0,0,0x308,'3e028a97ebd2430f4181d83bd55f460261c9506827cddcee83adce2ea65650b9'),
    'Gfx_AllocBytes': (0x5f4a0,21,0,1,0,'2a7819df61b5e4eddd3b8f22a5a85e18f39ff17301f5ab6db1733e69913da9f6'),
    'Gfx_AllocAlignedBytes': (0x61840,47,0,2,0x20a,'02150c922745d67bfe77182db1c615fb172490b6f730092ffb3e6c460b3f3b34')}
INPUTS = ['decomp/src/geputget.c','decomp/include/geputget.h',
    'decomp/src/mem.c','decomp/include/mem.h','decomp/src/lisa3d.c',
    'decomp/include/lisa3d.h','decomp/target.json','tools/build_decomp.py',
    'tools/verify_sprite_packing.py','tools/verify_matching.py',
    'tools/windows_target.py','tools/windows_tracking.py']


def inspect(pe):
    debug = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    fpo = {r:(n,l,a,b) for r,n,l,a,b in struct.iter_unpack('<IIIHH',
        pe.__data__[debug.PointerToRawData:debug.PointerToRawData+debug.SizeOfData])}
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    reloc = {e.rva for block in pe.DIRECTORY_ENTRY_BASERELOC for e in block.entries if e.type == 3}
    for name,(r,n,l,a,b,digest) in ROUTINES.items():
        raw = pe.get_data(r,n)
        ins = list(md.disasm(raw,0x400000+r))
        assert fpo[r] == (n,l,a,b) and hashlib.sha256(raw).hexdigest() == digest
        assert sum(i.size for i in ins) == n and ins[-1].mnemonic == 'ret'
        assert not any(struct.unpack('<I',pe.get_data(e,4))[0] == r+0x400000 for e in reloc)
        expected_relocs = {0x617e4,0x617fe,0x6181c,0x61824,0x6182e} if r == 0x617e0 else set()
        assert {e for e in reloc if r <= e < r+n} == expected_relocs
        for e in expected_relocs:
            assert struct.unpack('<I',pe.get_data(e,4))[0] == (TEMPLATE if e == 0x617e4 else HEAD)
    assert [(i.address,i.op_str) for i in md.disasm(pe.get_data(0x5f4a0,21),0x45f4a0)
        if i.mnemonic == 'call'] == [(0x45f4ac,'0x469400')]
    assert [(i.address,i.op_str) for i in md.disasm(pe.get_data(0x61840,47),0x461840)
        if i.mnemonic == 'call'] == [(0x461850,'0x45f4a0')]
    refs = [(r,i.address) for r,(n,*_) in fpo.items()
        for i in md.disasm(pe.get_data(r,n),r+0x400000)
        if i.mnemonic in ('call','jmp') and i.op_str == '0x461840']
    assert refs == [(0x617e0,0x46180f)]
    assert pe.get_data(0x5f4b5,11) == b'\xcc'*11
    assert pe.get_data(0x6186f,1) == b'\xcc'
    assert pe.get_data(0x61835,11) == b'\xcc'*11
    assert [(r,i.address) for r,(n,*_) in fpo.items()
        for i in md.disasm(pe.get_data(r,n),r+0x400000)
        if i.mnemonic in ('call','jmp') and i.op_str == '0x4617e0'] == [(0x616c0,0x4616cf),(0x616c0,0x46174f)]
    assert hashlib.sha256(pe.get_data(0x616c0,273)).hexdigest() == '6da9083d32b7bd4186fb626b6fb5fe0fc099a6d769e332c98805b6d9686700ed'
    for site in (0x616cf,0x6174f):
        ins = list(md.disasm(pe.get_data(site,16),site+0x400000))
        assert [(i.mnemonic,i.op_str) for i in ins[:3]] == [('call','0x4617e0'),('mov','ebx, dword ptr [0x51fe40]'),('jmp','0x4616cb')]



class State:
    """Independent raw-address oracle, deliberately not based on production types."""
    def __init__(self, seed):
        rng = random.Random(seed)
        self.arena = bytearray(rng.randbytes(SIZE))
        self.template = bytearray(rng.randbytes(24))
        if seed in range(4):
            words = ([0]*6, [MASK]*6, [0x80000000]*6, list(range(6)))[seed]
            self.template[:] = struct.pack('<6I',*words)
        self.head = bytearray(4)
        self.events = []
        self.fault = None

    def region(self, address, size):
        for base,data in ((ARENA,self.arena),(TEMPLATE,self.template),(HEAD,self.head)):
            if base <= address and address+size <= base+len(data):
                return data,address-base
        return None,None

    def snapshot(self):
        return (bytes(self.arena),bytes(self.template),bytes(self.head))

    def read(self, address):
        data,offset = self.region(address,4)
        assert data is not None
        value = struct.unpack_from('<I',data,offset)[0]
        self.events.append(('read',address,4,value))
        return value

    def write(self, address, value):
        data,offset = self.region(address,4)
        if data is None:
            self.fault = ('write',address,4)
            return False
        self.events.append(('write',address,4,value))
        struct.pack_into('<I',data,offset,value)
        return True

    def allocate(self, size, replies):
        if size == 0:
            return 0
        self.events.append(('malloc',size,self.snapshot()))
        value,mutations = replies.pop(0)
        for address,word in mutations:
            data,offset = self.region(address,4)
            struct.pack_into('<I',data,offset,word)
        return value

    def aligned(self, size, alignment, replies):
        allocation = self.allocate((size+alignment+4)&MASK,replies)
        if alignment == 0:
            self.fault = ('divide',)
            return None
        # DIV EDX:EAX with EDX=0, not a signed or power-of-two operation.
        after_header = (allocation+4)&MASK
        residue = after_header % alignment
        header = (allocation+alignment-residue)&MASK
        if not self.write(header,allocation):
            return None
        return (header+4)&MASK


    def boundary(self, name, args):
        self.events.append(('entry',name,tuple(args),self.snapshot()))

    def page(self, replies, mode):
        self.boundary('Gfx_AllocBytes',(24,))
        node = self.allocate(24,replies)
        for offset in range(0,24,4):
            value = self.read(TEMPLATE+offset)
            if not self.write(node+offset,value):
                return None
        self.write(node+20,self.read(HEAD))
        self.boundary('Gfx_AllocAlignedBytes',(65536,65536))
        if mode == 'isolated':
            result,mutations = replies.pop(0)
            for address,word in mutations:
                data,offset = self.region(address,4)
                struct.pack_into('<I',data,offset,word)
        else:
            self.boundary('Gfx_AllocBytes',(131076,))
            result = self.aligned(65536,65536,replies)
            if self.fault is not None:
                return None
        self.write(node+8,result)
        if self.read(HEAD):
            result = self.read(HEAD)
            self.write(result+16,node)
        self.write(HEAD,node)
        return result


class Session:
    def __init__(self, pe, state, original):
        self.uc = Uc(UC_ARCH_X86,UC_MODE_32)
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.size = (pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095
        self.uc.mem_map(self.base,self.size)
        self.uc.mem_write(self.base,pe.get_memory_mapped_image())
        self.uc.mem_map(ARENA,SIZE)
        self.uc.mem_map(STACK,0x10000)
        self.uc.mem_map(STOP,0x1000)
        self.symbols = ({name:0x400000+data[0] for name,data in ROUTINES.items()} |
            {'malloc':0x469400,'g_spritePackingTemplate':TEMPLATE,'g_spritePackingPages':HEAD}) if original else {
            e.name.decode():self.base+e.address for e in pe.DIRECTORY_ENTRY_EXPORT.symbols if e.name}
        self.fields = [(TEMPLATE,self.symbols['g_spritePackingTemplate'],24),
            (HEAD,self.symbols['g_spritePackingPages'],4)]
        for address,data in ((ARENA,state.arena),(self.fields[0][1],state.template),(self.fields[1][1],state.head)):
            self.uc.mem_write(address,bytes(data))
        self.uc.hook_add(UC_HOOK_CODE,self.code)
        self.uc.hook_add(UC_HOOK_MEM_READ | UC_HOOK_MEM_WRITE,self.memory)
        self.uc.hook_add(UC_HOOK_MEM_INVALID,self.invalid)
        self.uc.hook_add(UC_HOOK_INTR,self.interrupt)

    def normalize(self, address):
        for logical,physical,n in self.fields:
            if physical <= address < physical+n:
                return logical+address-physical
        return address

    def physical(self, address):
        for logical,physical,n in self.fields:
            if logical <= address < logical+n:
                return physical+address-logical
        return address

    def snapshot(self):
        return (bytes(self.uc.mem_read(ARENA,SIZE)),
            bytes(self.uc.mem_read(self.fields[0][1],24)),
            bytes(self.uc.mem_read(self.fields[1][1],4)))

    def memory(self, uc, access, address, size, value, _):
        if STACK <= address < STACK+0x10000:
            return
        if access == UC_MEM_WRITE:
            # Unicorn reports attempted unmapped writes here as well.
            if not (ARENA <= address and address+size <= ARENA+SIZE or
                    any(p <= address and address+size <= p+n for _,p,n in self.fields)):
                return
            kind = 'write'
        else:
            kind = 'read'
            value = int.from_bytes(uc.mem_read(address,size),'little')
        self.events.append((kind,self.normalize(address),size,value))

    def invalid(self, uc, access, address, size, value, _):
        assert access == UC_MEM_WRITE_UNMAPPED, (access,hex(address))
        self.fault = ('write',address,size)
        return False

    def interrupt(self, uc, number, _):
        assert number == 0, number
        self.fault = ('divide',)
        uc.emu_stop()

    def code(self, uc, address, size, _):
        if self.page_mode:
            for name,argc in (('Gfx_AllocBytes',1),('Gfx_AllocAlignedBytes',2)):
                if address == self.symbols[name]:
                    sp = uc.reg_read(UC_X86_REG_ESP)
                    args = struct.unpack('<'+'I'*argc,uc.mem_read(sp+4,4*argc))
                    self.events.append(('entry',name,args,self.snapshot()))
                    if name == 'Gfx_AllocAlignedBytes' and self.page_mode == 'isolated':
                        value,mutations = self.replies.pop(0)
                        for dest,word in mutations:
                            uc.mem_write(self.physical(dest),struct.pack('<I',word))
                        ret = struct.unpack('<I',uc.mem_read(sp,4))[0]
                        uc.reg_write(UC_X86_REG_EAX,value)
                        uc.reg_write(UC_X86_REG_ECX,0xbadc0ffe)
                        uc.reg_write(UC_X86_REG_EDX,0x87654321)
                        uc.reg_write(UC_X86_REG_EFLAGS,0x8d7)
                        uc.reg_write(UC_X86_REG_ESP,sp+4)
                        uc.reg_write(UC_X86_REG_EIP,ret)
                        return
        if address == STOP:
            uc.emu_stop()
        elif address == self.symbols['malloc']:
            sp = uc.reg_read(UC_X86_REG_ESP)
            ret,arg = struct.unpack('<II',uc.mem_read(sp,8))
            self.events.append(('malloc',arg,self.snapshot()))
            value,mutations = self.replies.pop(0)
            for dest,word in mutations:
                uc.mem_write(self.physical(dest),struct.pack('<I',word))
            uc.reg_write(UC_X86_REG_EAX,value)
            uc.reg_write(UC_X86_REG_ECX,0xbadc0ffe)
            uc.reg_write(UC_X86_REG_EDX,0x87654321)
            uc.reg_write(UC_X86_REG_EFLAGS,0x8d7)
            uc.reg_write(UC_X86_REG_ESP,sp+4)
            uc.reg_write(UC_X86_REG_EIP,ret)

    def run(self, name, args, replies, expected, eax, seed, page_mode=None):
        self.page_mode = page_mode
        uc = self.uc
        self.events,self.fault,self.replies = [],None,list(replies)
        sp = STACK+0x8000
        uc.mem_write(sp,struct.pack('<'+'I'*(1+len(args)),STOP,*args))
        caller = bytes(uc.mem_read(sp,0x8000))
        saved = [random.Random(seed+i).getrandbits(32) for i in range(4)]
        for reg,value in zip(PRESERVED,saved):
            uc.reg_write(reg,value)
        uc.reg_write(UC_X86_REG_ESP,sp)
        uc.reg_write(UC_X86_REG_EFLAGS,2)
        before = bytearray(uc.mem_read(self.base,self.size))
        try:
            uc.emu_start(self.symbols[name],STOP,count=10000)
        except UcError:
            assert self.fault is not None
        assert self.fault == expected.fault, (name,self.fault,expected.fault)
        assert self.events == expected.events, (name,[(e[0],e[1:3]) for e in self.events],[(e[0],e[1:3]) for e in expected.events])
        assert self.snapshot() == expected.snapshot()
        assert not self.replies
        after = bytearray(uc.mem_read(self.base,self.size))
        for _,p,n in self.fields:
            before[p-self.base:p-self.base+n] = after[p-self.base:p-self.base+n]
        assert before == after, 'Unrelated image bytes changed'
        if expected.fault is None:
            assert uc.reg_read(UC_X86_REG_EIP) == STOP
            assert uc.reg_read(UC_X86_REG_EAX) == eax
            assert uc.reg_read(UC_X86_REG_ESP) == sp+4
            assert [uc.reg_read(r) for r in PRESERVED] == saved
            assert uc.reg_read(UC_X86_REG_EFLAGS)&0x400 == 0
            assert bytes(uc.mem_read(sp,0x8000)) == caller


def verify_sprite_packing():
    verify_target()
    original = pefile.PE(str(TARGET))
    inspect(original)
    build(dll=DLL)
    rebuilt = pefile.PE(str(DLL))
    assert rebuilt.FILE_HEADER.Machine == 0x14c and not hasattr(rebuilt,'DIRECTORY_ENTRY_IMPORT')
    # Clang uses a conditional tail transfer for the nonzero allocation path.
    symbols = {e.name.decode():rebuilt.OPTIONAL_HEADER.ImageBase+e.address for e in rebuilt.DIRECTORY_ENTRY_EXPORT.symbols if e.name}
    md = Cs(CS_ARCH_X86,CS_MODE_32)
    entry = symbols['Gfx_AllocBytes']
    instructions = list(md.disasm(rebuilt.get_data(entry-rebuilt.OPTIONAL_HEADER.ImageBase,14),entry))
    assert [(i.mnemonic,int(i.op_str,16)) for i in instructions if i.mnemonic.startswith('j')] == [('jne',symbols['malloc'])]
    counts = {name:0 for name in ROUTINES}
    faults = dict(counts)
    rng = random.Random(0x61840)
    cases = []
    for size in (0,1,24,65536,0x80000000,0xffffffff):
        for result in (0,ARENA+0x100,0x80000000,0xffffffff):
            cases.append(('Gfx_AllocBytes',(size,),[(result,[])] if size else []))
    for alignment in (1,2,3,4,7,16,255,256,65536):
        for offset in (0,1,3,4,15,255,65532):
            for size in (0,1,65536,0xffffffff):
                total = (size+alignment+4)&MASK
                cases.append(('Gfx_AllocAlignedBytes',(size,alignment),[(ARENA+0x10000+offset,[])] if total else []))
    for size,alignment,result in ((1,0,ARENA+16),(0,65536,0),(0,1,0),
            (0xfffffff8,4,0),(1,0x80000000,ARENA+16),(1,16,0xfffffffc),
            (0,0,0),(0xfffffffc,0,0)):
        cases.append(('Gfx_AllocAlignedBytes',(size,alignment),[(result,[])] if (size+alignment+4)&MASK else []))
    for name,args,replies in cases:
        state = State(rng.getrandbits(32))
        sessions = [Session(p,state,o) for p,o in ((original,True),(rebuilt,False))]
        for repeat in range(2):
            if repeat and state.fault is not None:
                break
            state.events,state.fault = [],None
            pending = list(replies)
            result = state.allocate(args[0],pending) if name == 'Gfx_AllocBytes' else state.aligned(*args,pending)
            assert not pending
            for session in sessions:
                session.run(name,args,replies,state,result,counts[name])
            counts[name] += 1
            faults[name] += int(state.fault is not None)
    page_counts = {'isolated':0,'real':0,'persistent_followups':0,'faults':0}
    def page_sequence(seed, head, calls):
        state = State(seed)
        struct.pack_into('<I',state.head,0,head)
        sessions = [Session(p,state,o) for p,o in ((original,True),(rebuilt,False))]
        for index,(mode,replies) in enumerate(calls):
            state.events,state.fault = [],None
            pending = list(replies)
            result = state.page(pending,mode)
            assert not pending
            for session in sessions:
                session.run('Gfx_AddSpritePackingPage',(),replies,state,result,seed+index,mode)
            page_counts[mode] += 1
            page_counts['persistent_followups'] += int(index != 0)
            page_counts['faults'] += int(state.fault is not None)
            if state.fault is not None:
                break
    node,old,other = ARENA+0x100,ARENA+0x200,ARENA+0x300
    # Exact node/head alias, arbitrary copied link words, independently varied
    # raw boundary returns; mutation occurs after saved-next but before repair.
    for seed in range(8):
        for head in (0,old,node):
            for pixel in (0,1,0xffffffff,0x80000000,node,ARENA+0x40000):
                for replacement in (None,0,old,other,node):
                    mutation = [] if replacement is None else [(HEAD,replacement)]
                    calls = [('isolated',[(node,[]),(pixel,mutation)])]*2
                    page_sequence(seed,head,calls)
    # First allocation can change both the template and the head before copying.
    # Second allocation can change the template, old/new links and live head.
    for seed in range(16):
        first = [(TEMPLATE+4*k,(seed*0x1020304+k)&MASK) for k in range(6)] + [(HEAD,other)]
        second = [(TEMPLATE,0xdeadbeef),(HEAD,old),(node+20,other),(old+4,0xabcdef)]
        page_sequence(1000+seed,old,[('real',[(node,first),(ARENA+0x40000+seed,second)])]*2)
    # Persistent client adds fresh pages without resetting CPU or image state.
    for seed in range(16):
        calls = [('real',[(node+k*24,[]),(ARENA+0x40000+k*0x10000+seed,[])]) for k in range(6)]
        page_sequence(2000+seed,old if seed%2 else 0,calls)
    # Node malloc failure faults on first copied dword, before pixel allocation.
    # Pixel malloc failure retains copied node/next and never publishes the head.
    for head in (0,old,node):
        page_sequence(3000+head,head,[('real',[(0,[])])])
        page_sequence(4000+head,head,[('real',[(node,[]),(0,[])])])
    counts['Gfx_AddSpritePackingPage'] = page_counts['isolated']+page_counts['real']
    faults['Gfx_AddSpritePackingPage'] = page_counts['faults']
    print('Page coverage:',page_counts,flush=True)
    for name,data in ROUTINES.items():
        details = {'scope':'Real allocator bodies; modeled CRT malloc; ordered accesses and CRT-entry arena/template/head snapshots; full state/image preservation; return ABI on normal completion; fault kind/address and preceding effects on failure',
            'persistent_repeats':page_counts['persistent_followups'] if name == 'Gfx_AddSpritePackingPage' else (counts[name]-faults[name])//2,'fault_cases':faults[name],
            'limitations':'No native heap/runtime parity, instruction equality, arbitrary reentry or original compiler/link layout; emulated fault comparison is not defined portable C behavior',
            'page_coverage':page_counts if name == 'Gfx_AddSpritePackingPage' else None,
            'compiler':'Provisional Clang/LLD; strict C89 PE32 extracted production bodies'}
        for kind in ('compilation','emulation'):
            record_run(data[0],kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
                cases=counts[name] if kind == 'emulation' else 0,
                command='uv run python tools/verify_sprite_packing.py',details=details)
    print('PASS packing allocation:',counts,'fault cases:',faults,flush=True)
    return counts


if __name__ == '__main__':
    verify_sprite_packing()
