# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7", "unicorn==2.1.4"]
# ///
"""Packing bucket insertion, allocation and gap search; CRT heap is an explicit validation boundary."""
import hashlib
import random
import struct
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from unicorn import (Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE,
    UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE, UC_HOOK_MEM_INVALID, UC_HOOK_INTR,
    UC_MEM_WRITE, UC_MEM_WRITE_UNMAPPED, UC_MEM_READ_UNMAPPED)
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
TEMPLATE, HEAD, TABLE = 0x51fc00, 0x51fe40, 0x51ff58
DLL = BUILD / 'sprite_packing_validation.dll'
ROUTINES = {
    'Gfx_AddSpritePackingBucket': (0x616c0,273,1,2,0x140b,'6da9083d32b7bd4186fb626b6fb5fe0fc099a6d769e332c98805b6d9686700ed'),
    'Gfx_FindSpritePackingGap': (0x61870,62,0,2,0x105,'9fec91b9a7a79937df056da6cea0f29c91ccd4aa4549a1eef45cf0716a71872d'),
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
        expected_relocs = ({0x617e4,0x617fe,0x6181c,0x61824,0x6182e} if r == 0x617e0 else
            {0x616d6,0x61704,0x61756,0x61760,0x617bc} if r == 0x616c0 else set())
        assert {e for e in reloc if r <= e < r+n} == expected_relocs
        for e in expected_relocs:
            assert struct.unpack('<I',pe.get_data(e,4))[0] == (TEMPLATE if e in (0x617e4,0x61704,0x61760) else TABLE if e == 0x617bc else HEAD)
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

    assert pe.get_data(0x617d1,15) == b'\xcc'*15
    assert [(r,i.address) for r,(n,*_) in fpo.items()
        for i in md.disasm(pe.get_data(r,n),r+0x400000)
        if i.mnemonic in ('call','jmp') and i.op_str == '0x4616c0'] == [(0x61530,0x4615f6)]
    assert [(i.mnemonic,i.op_str) for i in md.disasm(pe.get_data(0x615ec,24),0x4615ec)][:6] == [
        ('mov','eax, dword ptr [0x51fe40]'),('mov','ecx, dword ptr [ebx + 8]'),
        ('push','eax'),('push','ecx'),('call','0x4616c0'),('add','esp, 8')]
    assert pe.get_data(0x618ae,2) == b'\xcc'*2
    gap_instructions = [(i.mnemonic,i.op_str) for i in md.disasm(pe.get_data(0x61870,62),0x461870)]
    assert gap_instructions == [
        ('mov','eax, dword ptr [esp + 8]'),('push','esi'),('test','eax, eax'),
        ('jne','0x461880'),('mov','eax, 0xffffffff'),('pop','esi'),('ret',''),
        ('mov','ecx, dword ptr [esp + 8]'),('cmp','dword ptr [eax], ecx'),
        ('jl','0x46188f'),('mov','eax, 0xffffffff'),('pop','esi'),('ret',''),
        ('mov','edx, dword ptr [eax + 0x14]'),('mov','esi, 0x100'),
        ('test','edx, edx'),('je','0x46189d'),('mov','esi, dword ptr [edx]'),
        ('sub','esi, dword ptr [eax + 4]'),('cmp','esi, ecx'),('jge','0x4618ac'),
        ('mov','eax, edx'),('test','edx, edx'),('jne','0x46188f'),
        ('xor','eax, eax'),('pop','esi'),('ret','')]
    assert [(r,i.address) for r,(n,*_) in fpo.items()
        for i in md.disasm(pe.get_data(r,n),r+0x400000)
        if i.mnemonic in ('call','jmp') and i.op_str == '0x461870'] == [
            (0x61530,0x46158d),(0x616c0,0x4616e9)]
    assert hashlib.sha256(pe.get_data(0x61530,350)).hexdigest() == '028b7432fafcd6c13a8ea0d675bda862a30ca19e99fbd6a3ec3e9a4344f9c394'
    assert fpo[0x61530] == (350,18,1,0x140b)
    assert fpo[0x616c0] == (273,1,2,0x140b)
    for site,expected in (
        (0x61582,[('mov','ebp, dword ptr [esi + 4]'),('mov','ecx, dword ptr [ebx + 4]'),
            ('mov','eax, dword ptr [ebp + 0xc]'),('push','eax'),('push','ecx'),
            ('call','0x461870'),('mov','dword ptr [esp + 0x1c], eax'),('add','esp, 8'),
            ('test','eax, eax'),('jne','0x4615a1'),('mov','esi, dword ptr [esi]'),
            ('jmp','0x46157e'),('cmp','dword ptr [esp + 0x14], -1')]),
        (0x616e0,[('mov','eax, dword ptr [ebx + 0xc]'),('mov','ecx, dword ptr [esp + 0x18]'),
            ('push','eax'),('push','ecx'),('call','0x461870'),('add','esp, 8'),
            ('mov','ebp, eax'),('test','ebp, ebp'),('jne','0x4616fc'),
            ('mov','ebx, dword ptr [ebx + 0x14]'),('jmp','0x4616dc'),
            ('push','0x18'),('cmp','ebp, -1')])):
        decoded = list(md.disasm(pe.get_data(site,64),site+0x400000))
        assert [(i.mnemonic,i.op_str) for i in decoded[:len(expected)]] == expected


class GapStop(Exception):
    pass


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
        self.table = bytearray(rng.randbytes(1028))
        self.read_budget = None
        self.read_count = 0
        self.events = []
        self.fault = None

    def region(self, address, size):
        for base,data in ((ARENA,self.arena),(TEMPLATE,self.template),(HEAD,self.head),(TABLE,self.table)):
            if base <= address and address+size <= base+len(data):
                return data,address-base
        return None,None

    def snapshot(self):
        return (bytes(self.arena),bytes(self.template),bytes(self.head),bytes(self.table))

    def read(self, address):
        address &= MASK
        data,offset = self.region(address,4)
        if data is None:
            self.fault = ('read',address,4)
            raise GapStop
        value = struct.unpack_from('<I',data,offset)[0]
        self.events.append(('read',address,4,value))
        self.read_count += 1
        if self.read_budget is not None and self.read_count == self.read_budget:
            self.fault = ('budget',self.read_budget)
            raise GapStop
        return value

    def write(self, address, value):
        address &= MASK
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


    def gap(self, request, first, read_limit=None):
        def read(address):
            value = self.read(address)
            if read_limit is not None and len(self.events) == read_limit:
                self.fault = ('budget',read_limit)
                raise GapStop
            return value
        def signed(word):
            return word if word < 0x80000000 else word-0x100000000
        try:
            if first == 0 or signed(read(first)) >= signed(request):
                return MASK
            cursor = first
            while True:
                successor = read(cursor+20)
                upper = read(successor) if successor else 256
                available = (upper-read(cursor+4))&MASK
                if signed(available) >= signed(request):
                    return cursor
                cursor = successor
                if cursor == 0:
                    return 0
        except GapStop:
            return None

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


    def bucket(self, request, first, replies, read_budget=None):
        self.read_budget,self.read_count = read_budget,0
        def put(address,value):
            if not self.write(address,value&MASK):
                raise GapStop
        try:
            page = first
            while True:
                if page == 0:
                    self.boundary('Gfx_AddSpritePackingPage',())
                    self.page(replies,'real')
                    if self.fault:
                        return None
                    page = self.read(HEAD)
                    continue
                child = self.read(page+12)
                self.boundary('Gfx_FindSpritePackingGap',(request,child))
                previous = self.gap(request,child)
                if self.fault:
                    return None
                if previous:
                    break
                page = self.read(page+20)
            self.boundary('Gfx_AllocBytes',(24,))
            node = self.allocate(24,replies)
            for k in range(6):
                put(node+4*k,self.read(TEMPLATE+4*k))
            if previous == MASK:
                put(node+4,request)
                pixels = self.read(page+8)
                start = self.read(node)
                put(node+8,pixels+((start<<8)&MASK))
                following = self.read(page+12)
                put(node+20,following)
                if following:
                    put(following+16,node)
                put(page+12,node)
            else:
                put(node,self.read(previous+4))
                put(node+4,self.read(previous+4)+request)
                start = self.read(node)
                pixels = self.read(page+8)
                put(node+8,pixels+((start<<8)&MASK))
                following = self.read(previous+20)
                self.boundary('Gfx_LinkSpritePackingNode',(previous,node,following))
                put(node+20,following)
                put(node+16,previous)
                put(previous+20,node)
                if following:
                    put(following+16,node)
            self.boundary('Gfx_AllocBytes',(8,))
            bucket = self.allocate(8,replies)
            slot = (TABLE+((request<<2)&MASK))&MASK
            old = self.read(slot)
            put(bucket,old)
            put(bucket+4,node)
            put(slot,bucket)
            return bucket
        except GapStop:
            return None
        finally:
            self.read_budget = None


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
            {'malloc':0x469400,'Gfx_LinkSpritePackingNode':0x461690,
                'g_spritePackingTemplate':TEMPLATE,'g_spritePackingPages':HEAD,'g_spritePackingBuckets':TABLE}) if original else {
            e.name.decode():self.base+e.address for e in pe.DIRECTORY_ENTRY_EXPORT.symbols if e.name}
        self.fields = [(TEMPLATE,self.symbols['g_spritePackingTemplate'],24),
            (HEAD,self.symbols['g_spritePackingPages'],4),
            (TABLE,self.symbols['g_spritePackingBuckets'],1028)]
        for address,data in ((ARENA,state.arena),(self.fields[0][1],state.template),(self.fields[1][1],state.head),(self.fields[2][1],state.table)):
            self.uc.mem_write(address,bytes(data))
        self.uc.hook_add(UC_HOOK_CODE,self.code)
        self.uc.hook_add(UC_HOOK_MEM_READ | UC_HOOK_MEM_WRITE,self.memory)
        self.uc.hook_add(UC_HOOK_MEM_INVALID,self.invalid)
        self.uc.hook_add(UC_HOOK_INTR,self.interrupt)

    def normalize(self, address):
        if self.bucket_mode and address == self.slot[1]:
            return self.slot[0]
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
            bytes(self.uc.mem_read(self.fields[1][1],4)),
            bytes(self.uc.mem_read(self.fields[2][1],1028)))

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
            if not (self.base <= address and address+size <= self.base+self.size or
                    ARENA <= address and address+size <= ARENA+SIZE):
                return  # The invalid-access hook records the attempted read.
            value = int.from_bytes(uc.mem_read(address,size),'little')
        self.events.append((kind,self.normalize(address),size,value))
        self.read_count += int(kind == 'read')
        if self.read_limit is not None and (self.read_count if self.bucket_mode else len(self.events)) == self.read_limit:
            assert kind == 'read'
            self.fault = ('budget',self.read_limit)
            uc.emu_stop()

    def invalid(self, uc, access, address, size, value, _):
        assert access in (UC_MEM_WRITE_UNMAPPED,UC_MEM_READ_UNMAPPED), (access,hex(address))
        self.fault = ('write' if access == UC_MEM_WRITE_UNMAPPED else 'read',self.normalize(address),size)
        return False

    def interrupt(self, uc, number, _):
        assert number == 0, number
        self.fault = ('divide',)
        uc.emu_stop()

    def code(self, uc, address, size, _):
        if self.bucket_mode:
            for name,argc in (('Gfx_AddSpritePackingPage',0),('Gfx_FindSpritePackingGap',2),('Gfx_LinkSpritePackingNode',3)):
                if address == self.symbols[name]:
                    sp = uc.reg_read(UC_X86_REG_ESP)
                    args = struct.unpack('<'+'I'*argc,uc.mem_read(sp+4,4*argc)) if argc else ()
                    self.events.append(('entry',name,args,self.snapshot()))
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

    def run(self, name, args, replies, expected, eax, seed, page_mode=None, read_limit=None):
        self.bucket_mode = name == 'Gfx_AddSpritePackingBucket'
        if self.bucket_mode:
            offset = (args[0]<<2)&MASK
            self.slot = ((TABLE+offset)&MASK,(self.symbols['g_spritePackingBuckets']+offset)&MASK)
        self.read_count = 0
        self.page_mode = page_mode
        self.read_limit = read_limit
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


def verify_gap(original, rebuilt):
    name = 'Gfx_FindSpritePackingGap'
    coverage = {'comparisons':0,'persistent_followups':0,'faults':0,
        'cycle_prefixes':0,'sentinels':0,'exhausted':0,'predecessors':0}
    nodes = [ARENA+0x100+24*k for k in range(8)]
    boundaries = [0,1,2,255,256,257,0x7ffffffe,0x7fffffff,
        0x80000000,0x80000001,0xfffffffe,0xffffffff]
    def sequence(seed, records, requests, first=None, read_limit=None, expected_results=None):
        state = State(seed)
        # Random unconsumed pixels/children/previous words remain untouched.
        for index,(start,end,next_pointer) in enumerate(records):
            struct.pack_into('<II',state.arena,nodes[index]-ARENA,start&MASK,end&MASK)
            struct.pack_into('<I',state.arena,nodes[index]-ARENA+20,next_pointer)
        if first == ARENA+SIZE-4:
            struct.pack_into('<I',state.arena,SIZE-4,0)
        sessions = [Session(p,state,o) for p,o in ((original,True),(rebuilt,False))]
        for index,request in enumerate(requests):
            state.events,state.fault = [],None
            head = nodes[0] if first is None else first
            result = state.gap(request&MASK,head,read_limit)
            if expected_results is not None:
                assert result == expected_results[index], (records,request,result,expected_results[index])
            for session in sessions:
                session.run(name,(request&MASK,head),[],state,result,seed+index,read_limit=read_limit)
            coverage['comparisons'] += 1
            coverage['persistent_followups'] += int(index != 0)
            coverage['faults'] += int(state.fault is not None and state.fault[0] != 'budget')
            coverage['cycle_prefixes'] += int(state.fault is not None and state.fault[0] == 'budget')
            if state.fault is None:
                category = 'sentinels' if result == MASK else 'exhausted' if result == 0 else 'predecessors'
                coverage[category] += 1
            if state.fault is not None:
                break
    for request in boundaries:
        sequence(request,[],[request]*2,first=0,expected_results=[MASK]*2)
    # Early prefix exits must avoid reading an invalid successor. Other paths fault.
    for start in boundaries:
        for request in boundaries:
            sequence(start^request,[(start,0,0x80000000)],[request]*2)
    # Fixed 256 tail boundary with raw signed/wrapping endpoint words.
    for end in boundaries:
        for request in boundaries:
            sequence(end^request,[(0x80000000,end,0)],[request]*2)
    # Every neighboring boundary pair, requests exactly at and around its
    # wrapping difference; keep arbitrary unsorted and overlapping ranges.
    for end in boundaries:
        for successor in boundaries:
            gap = (successor-end)&MASK
            for request in ((gap-1)&MASK,gap,(gap+1)&MASK):
                sequence(end^successor^request,[(0x80000000,end,nodes[1]),
                    (successor,256,0)],[request]*2)
    rng = random.Random(0x61870)
    for seed in range(128):
        length = rng.randrange(1,9)
        records = [(rng.getrandbits(32),rng.getrandbits(32),nodes[k+1] if k+1<length else 0)
            for k in range(length)]
        requests = [rng.getrandbits(32) for _ in range(2)]
        sequence(10000+seed,records,requests)
    # Hand-derived return contracts, including late first-match and exhaustion.
    for seed in range(16):
        sequence(20000+seed,[(0,16,nodes[1]),(32,64,nodes[2]),(128,240,0)],
            [0,16,17,64,65,1],expected_results=[MASK,nodes[0],nodes[1],nodes[1],0,nodes[0]])
        sequence(21000+seed,[(0xffffffff,0xfffffffe,nodes[0])],[1,0],expected_results=[nodes[0],nodes[0]])
    for seed in range(16):
        records = [(32*k,32*(k+1),nodes[k+1] if k<7 else 0) for k in range(8)]
        records[-1] = (224,255,0)
        sequence(22000+seed,records,[1,2,1],expected_results=[nodes[7],0,nodes[7]])
    for bad in (0x80000000,ARENA+SIZE):
        sequence(30000+bad,[],[1],first=bad)
    # Mapped start, missing next field; no invented pointer validation.
    sequence(31000,[(0,0,0)],[1],first=ARENA+SIZE-4)
    # Nonproductive cycles preserve repeated reads, without returning. Stop
    # externally after 31 reads, rather than adding a production cycle guard.
    sequence(32000,[(0,256,nodes[0])],[1],read_limit=31)
    sequence(32001,[(0,256,nodes[1]),(0,256,nodes[0])],[1],read_limit=31)
    print('Gap coverage:',coverage,flush=True)
    return coverage


def verify_bucket(original, rebuilt):
    coverage = {'comparisons':0,'persistent_followups':0,'faults':0,
        'cycle_prefixes':0,'page_entries':0,'gap_entries':0,'link_entries':0,
        'prefix_insertions':0,'interior_insertions':0}
    page,other,child,following,node,bucket = [ARENA+x for x in (0x100,0x180,0x200,0x280,0x400,0x600)]
    def state_for(seed, records=()):
        state = State(seed)
        # Unconsumed template fields retain arbitrary bits. Its children can be
        # copied into newly allocated pages, so use a valid empty child list.
        struct.pack_into('<I',state.template,12,0)
        for address,words in records:
            struct.pack_into('<6I',state.arena,address-ARENA,*[x&MASK for x in words])
        return state
    def record(start=0,end=0,pixels=0xabcdef00,children=0,previous=0x98765432,next=0):
        return (start,end,pixels,children,previous,next)
    def sequence(state, calls, budget=None):
        sessions = [Session(p,state,o) for p,o in ((original,True),(rebuilt,False))]
        for index,(request,first,replies,mutations) in enumerate(calls):
            for dest,word in mutations:
                data,offset = state.region(dest,4)
                struct.pack_into('<I',data,offset,word&MASK)
                for session in sessions:
                    session.uc.mem_write(session.physical(dest),struct.pack('<I',word&MASK))
            state.events,state.fault = [],None
            pending = list(replies)
            result = state.bucket(request,first,pending,budget)
            consumed = replies[:len(replies)-len(pending)]
            for session in sessions:
                session.run('Gfx_AddSpritePackingBucket',(request,first),consumed,state,result,
                    coverage['comparisons']+index,page_mode='real',read_limit=budget)
            coverage['comparisons'] += 1
            coverage['persistent_followups'] += int(index != 0)
            coverage['faults'] += int(state.fault is not None and state.fault[0] != 'budget')
            coverage['cycle_prefixes'] += int(state.fault is not None and state.fault[0] == 'budget')
            entries = [e[1] for e in state.events if e[0] == 'entry']
            for name,key in (('Gfx_AddSpritePackingPage','page_entries'),('Gfx_FindSpritePackingGap','gap_entries'),('Gfx_LinkSpritePackingNode','link_entries')):
                coverage[key] += entries.count(name)
            if state.fault is None:
                coverage['interior_insertions' if 'Gfx_LinkSpritePackingNode' in entries else 'prefix_insertions'] += 1
                assert result == replies[-1][0]
            if state.fault is not None:
                break
    # Every ordinary table slot, including zero/256. Calls reuse the same CPU
    # and memory while consuming two separate empty page child lists.
    for request in range(257):
        for seed in range(4):
            state = state_for(seed,[(page,record()),(other,record(pixels=0x80000000))])
            sequence(state,[(request,page,[(node,[]),(bucket,[])],[]),
                (request,other,[(node+24,[]),(bucket+8,[])],[])])
    print('Bucket table-slot coverage:',coverage['comparisons'],flush=True)
    requests = (0,1,16,255,256,0x40000001,0x80000001,0xc0000001,0x800000ff)
    # Exact prefix equality and negative/high-bit requests. Shifted table
    # indexing wraps as a dword without a signed extent guard.
    for seed in range(8):
        for request in requests:
            state = state_for(seed,[(page,record(children=child)),
                (child,record(start=request,end=MASK,next=0x80000000))])
            sequence(state,[(request,page,[(node,[]),(bucket,[])],[])])
    # Interior fit; predecessor end is reread after the first node store.
    # Five allocation targets and five bucket targets cover exact aliases with
    # page, predecessor, successor and each other, beyond distinct allocations.
    for seed in range(8):
        for target in (node,page,child,following):
            for result in (bucket,page,child,following,target):
                state = state_for(seed,[(page,record(children=child)),
                    (child,record(end=16,next=following)),(following,record(start=64,end=256))])
                sequence(state,[(16,page,[(target,[]),(result,[])],[])])
    for seed in range(8):
        for target in (node,page,child):
            for result in (bucket,page,child,target):
                state = state_for(seed,[(page,record(children=child)),(child,record(start=64))])
                sequence(state,[(16,page,[(target,[]),(result,[])],[])])
    print('Bucket alias coverage:',coverage['comparisons'],flush=True)
    # Wrapped range sums and pixel shifts; size aliases slot 1 under dword
    # multiplication, with hand-picked gap inequalities rather than range guards.
    for request,end in ((1,0xffffffff),(256,0xffffffff),(0x40000001,0xc00000ff),
            (0x80000001,16),(0xc0000001,16),(0x800000ff,16)):
        for seed in range(8):
            state = state_for(seed,[(page,record(children=child)),
                (child,record(start=0x80000000,end=end))])
            sequence(state,[(request,page,[(node,[]),(bucket,[])],[])])
    # Existing page exhausted, next page fits; both gap calls execute real code.
    for seed in range(16):
        state = state_for(seed,[(page,record(children=child,next=other)),
            (child,record(end=256)),(other,record())])
        sequence(state,[(16,page,[(node,[]),(bucket,[])],[])])
    # Page creation reached through initial null and through list exhaustion.
    # Use the real page/aligned/wrapper bodies and modeled CRT only.
    for seed in range(16):
        for first in (0,page):
            state = state_for(seed,[(page,record(children=child)),(child,record(end=256))])
            struct.pack_into('<I',state.head,0,page if first else 0)
            replies = [(other,[]),(ARENA+0x40000+seed,[]),(node,[]),(bucket,[])]
            sequence(state,[(16,first,replies,[])])
    # Bounded allocator mutations distinguish saved page/predecessor addresses
    # from live range, child, pixel, template and table reads after allocation.
    for seed in range(16):
        for interior in (False,True):
            state = state_for(seed,[(page,record(children=child)),
                (child,record(start=0 if interior else 64,end=16,next=following)),
                (following,record(start=64,end=256)),(other,record())])
            first_mutations = [(HEAD,other),(TEMPLATE,0x80000001),
                (page+8,0xfffffff0),(page+12,following),(child+4,0xfffffffe),(child+20,0)]
            last_mutations = [(TABLE+64,0xaabbccdd),(node+4,0x11111111),(HEAD,0)]
            sequence(state,[(16,page,[(node,first_mutations),(bucket,last_mutations)],[])])
    # Persistent eight-bucket chain consumes actual newly linked nodes each time.
    for seed in range(16):
        state = state_for(seed,[(page,record())])
        struct.pack_into('<I',state.template,0,0)
        struct.pack_into('<I',state.table,64,0)
        calls = [(16,page,[(node+24*k,[]),(bucket+8*k,[])],[]) for k in range(8)]
        sequence(state,calls)
        # Hand-derived eight-node/bucket result independently constrains the
        # oracle, including first-prefix previous preservation and final head.
        template_previous = struct.unpack_from('<I',state.template,16)[0]
        for k in range(8):
            assert struct.unpack_from('<6I',state.arena,node+24*k-ARENA) == (
                16*k,16*(k+1),(0xabcdef00+(16*k<<8))&MASK,0,
                node+24*(k-1) if k else template_previous,
                node+24*(k+1) if k<7 else 0)
            assert struct.unpack_from('<2I',state.arena,bucket+8*k-ARENA) == (
                bucket+8*(k-1) if k else 0,node+24*k)
        assert struct.unpack_from('<I',state.table,64)[0] == bucket+56
    # Unchecked malloc failures at node/page/pixels/bucket; invalid initial page,
    # predecessor-successor, prefix repair, and cached page pixel read.
    for seed in range(8):
        for interior in (False,True):
            records = [(page,record(children=child)),(child,record(start=0 if interior else 64,end=16))]
            for replies in ([(0,[])],[(node,[]),(0,[])],
                    [(node,[(child+20,0x80000000)])] if interior else [(node,[(page+12,0x80000000)])]):
                state = state_for(seed,records)
                sequence(state,[(16,page,replies,[])])
        for replies in ([(0,[])],[(other,[]),(0,[])],
                [(other,[]),(ARENA+0x40000,[]),(0,[])],
                [(other,[]),(ARENA+0x40000,[]),(node,[]),(0,[])]):
            state = state_for(seed)
            sequence(state,[(16,0,replies,[])])
    sequence(state_for(500),[(16,0x80000000,[],[])])
    sequence(state_for(501,[(page,record(children=0x80000000))]),[(16,page,[],[])])
    # Truncated node/page mappings preserve read/write fault location and prefix.
    sequence(state_for(502),[(16,ARENA+SIZE-4,[],[])])
    sequence(state_for(503,[(page,record())]),[(16,page,[(ARENA+SIZE-8,[])],[])])
    # An unchecked out-of-table slot faults only after insertion and bucket
    # allocation. Normalize its relocated address without mapping it.
    for seed in range(8):
        state = state_for(seed,[(page,record())])
        sequence(state,[(0x01000000,page,[(node,[]),(bucket,[])],[])])
    # Nonproductive child and page cycles: externally stop after 31 reads.
    state = state_for(600,[(page,record(children=child)),(child,record(end=256,next=child))])
    sequence(state,[(16,page,[],[])],budget=31)
    state = state_for(601,[(page,record(children=child,next=page)),(child,record(end=256))])
    sequence(state,[(16,page,[],[])],budget=31)
    print('Bucket coverage:',coverage,flush=True)
    return coverage


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
    gap_counts = verify_gap(original,rebuilt)
    counts['Gfx_FindSpritePackingGap'] = gap_counts['comparisons']
    faults['Gfx_FindSpritePackingGap'] = gap_counts['faults']
    bucket_counts = verify_bucket(original,rebuilt)
    counts['Gfx_AddSpritePackingBucket'] = bucket_counts['comparisons']
    faults['Gfx_AddSpritePackingBucket'] = bucket_counts['faults']
    for name,data in ROUTINES.items():
        details = {'scope':'Real allocator bodies; modeled CRT malloc; ordered accesses and CRT-entry arena/template/head snapshots; full state/image preservation; return ABI on normal completion; fault kind/address and preceding effects on failure',
            'persistent_repeats':page_counts['persistent_followups'] if name == 'Gfx_AddSpritePackingPage' else (counts[name]-faults[name])//2,'fault_cases':faults[name],
            'limitations':'No native heap/runtime parity, instruction equality, arbitrary reentry or original compiler/link layout; emulated fault comparison is not defined portable C behavior',
            'page_coverage':page_counts if name == 'Gfx_AddSpritePackingPage' else None,
            'compiler':'Provisional Clang/LLD; strict C89 PE32 extracted production bodies'}
        if name == 'Gfx_FindSpritePackingGap':
            details.update(scope='Read-only packing gap search; raw-offset signed/wrapping oracle; exact ordered reads, full arena/template/head/image preservation, normal EAX/stack/nonvolatile/DF ABI; unmapped read effects and bounded nonreturning cycle prefixes',
                persistent_repeats=gap_counts['persistent_followups'],gap_coverage=gap_counts,
                bucket_integration=bucket_counts,
                limitations='Storage assignment remains statically analyzed only; bucket insertion executes real gap calls; no instruction equality, original compiler/link layout, native fault/runtime or game parity; cycle prefixes do not prove general liveness')
        if name == 'Gfx_AddSpritePackingBucket':
            details.update(scope='Real page/gap/link/allocation bodies with modeled CRT; independent raw-offset oracle; ordered accesses and boundary snapshots including the full bucket table; exact aliases, mutations, persistent insertion chains, normal return ABI, fault effects and bounded cycle prefixes',
                persistent_repeats=bucket_counts['persistent_followups'],bucket_coverage=bucket_counts,
                limitations='Storage/release callers and workspace renderers unreconstructed; no instruction equality, original compiler/link layout or native heap/graphics/game/fault parity; external cycle prefixes do not establish liveness')
        for kind in ('compilation','emulation'):
            record_run(data[0],kind,'pass',inputs=INPUTS,artifact=DLL.relative_to(ROOT).as_posix(),
                cases=counts[name] if kind == 'emulation' else 0,
                command='uv run python tools/verify_sprite_packing.py',details=details)
    print('PASS packing allocation:',counts,'fault cases:',faults,flush=True)
    return counts


if __name__ == '__main__':
    verify_sprite_packing()
