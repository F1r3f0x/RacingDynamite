# /// script
# requires-python = ">=3.13"
# dependencies = ["pefile==2024.8.26", "capstone==5.0.7"]
# ///
"""Reproduce PE inventory and asserted startup edges from authentic file bytes."""
import hashlib
import struct
from collections import Counter
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_IMM, X86_OP_MEM
from windows_target import TARGET, SHA256, ROUTINE_SHA256, verify_target

if not __debug__:
    raise RuntimeError('Inspection checks require assertions; disable -O/PYTHONOPTIMIZE')

def inspect():
    data = verify_target()
    pe = pefile.PE(data=data)
    base = pe.OPTIONAL_HEADER.ImageBase
    print(f'IGN_WIN.EXE: {len(data)} bytes; SHA256 {SHA256}')
    print(f'PE32 x86; preferred base {base:#010x}; entry RVA {pe.OPTIONAL_HEADER.AddressOfEntryPoint:#010x}')
    for s in pe.sections:
        print(f'{s.Name.rstrip(bytes([0])).decode():8} RVA={s.VirtualAddress:#010x} virtual={s.Misc_VirtualSize:#x} raw_offset={s.PointerToRawData:#x} raw_size={s.SizeOfRawData:#x} flags={s.Characteristics:#010x}')
    for d in pe.DIRECTORY_ENTRY_IMPORT:
        print(f'{d.dll.decode()}: {len(d.imports)} imports')
        for i in d.imports:
            print(f'  IAT VA {i.address:#010x}: {i.name.decode() if i.name else "ordinal "+str(i.ordinal)}')
    print('Relocation types:',dict(Counter(e.type for block in pe.DIRECTORY_ENTRY_BASERELOC for e in block.entries)))
    fpo = next(d.struct for d in pe.DIRECTORY_ENTRY_DEBUG if d.struct.Type == 3)
    records = {base+a:n for a,n,local,params,bits in struct.iter_unpack(
        '<IIIHH',data[fpo.PointerToRawData:fpo.PointerToRawData+fpo.SizeOfData])}
    for va in [0x4120a0,0x412230,0x4122d0,0x412460,0x412500,0x45b170,
               0x45b1b0,0x456180,0x45b360,0x45b1f0,0x456af0,0x45b690,0x456e60,0x456bc0,0x455ac0,
               0x45b740,0x456f20,0x417270,0x417ea0,0x4184c0,0x458040]:
        print(f'FPO: VA={va:#010x} RVA={va-base:#010x} size={records[va]} end={va+records[va]:#010x}')
    assert records[0x45b1f0] == 76
    assert hashlib.sha256(pe.get_data(0x5b1f0,76)).hexdigest() == ROUTINE_SHA256
    md = Cs(CS_ARCH_X86,CS_MODE_32)
    md.detail = True
    consumer = pe.get_data(0x5b1b0,58)
    assert records[0x45b1b0] == 58
    assert hashlib.sha256(consumer).hexdigest() == '696ce4a075f495b91bd79ce9fe531b4c474b66540d3935dcc8e5a45c74d6d139'
    assert pe.get_data(0x5b1ea,6) == b'\xcc'*6
    for va in [0x45b1b0,0x45b1f0,0x456180,0x45b360]:
        for i in md.disasm(pe.get_data(va-base,records[va]),va):
            print(f'{i.address:#010x}: {i.mnemonic} {i.op_str}')
    # Independent scan of all file-backed FPO code, not bridge xref completeness.
    for va,n in sorted(records.items()):
        for i in md.disasm(pe.get_data(va-base,n),va):
            addresses = [o.imm if o.type == X86_OP_IMM else o.mem.disp
                         for o in i.operands if o.type in (X86_OP_IMM,X86_OP_MEM)]
            if any(a in (0x512040,0x45b1b0) or 0x51122e <= a < 0x5113c0 for a in addresses):
                print(f'Handle reference {i.address:#010x}: {i.mnemonic} {i.op_str}')
    # Absolute-address relocation scan also covers code outside FPO records.
    refs = []
    text_section = next(s for s in pe.sections if s.Name.rstrip(b'\0') == b'.text')
    for block in pe.DIRECTORY_ENTRY_BASERELOC:
        for e in block.entries:
            if e.type != 3 or not text_section.VirtualAddress <= e.rva < text_section.VirtualAddress+text_section.Misc_VirtualSize:
                continue
            value = struct.unpack('<I',pe.get_data(e.rva,4))[0]
            if value == 0x512040 or 0x51122e <= value < 0x5113c0:
                refs.append((base+e.rva,value))
    assert refs == [(0x45b1c1,0x512040),(0x45b1d7,0x512040),
                    (0x45b1df,0x511230),(0x45b221,0x51122e),(0x45b235,0x512040)], refs
    print('Absolute .text relocations referencing cursor/ID storage:',refs)
    for site,target in [(0x469a96,0x4120a0),(0x412170,0x45b170),
        (0x45b17a,0x45b1f0),(0x4561a0,0x45b1b0),(0x4561c5,0x45b360),
        (0x412175,0x412500),(0x41250a,0x456bc0),
        (0x412513,0x455ac0),(0x412257,0x417270),(0x41729a,0x417ea0),
        (0x41729f,0x4184c0),(0x45b7c6,0x47879c),(0x455b1a,0x478778),
        (0x4679e2,0x4787a2),(0x417f25,0x456be0),(0x4180ff,0x458040)]:
        instruction = next(md.disasm(pe.get_data(site-base,5),site))
        assert instruction.mnemonic == 'call' and int(instruction.op_str,16) == target
        print(f'Confirmed direct call {site:#010x} -> {target:#010x}')
    for site,global_va,value in [(0x45b69a,0x50eb6c,0x45b740),(0x456e60,0x50eba0,0x456f20)]:
        instruction = next(md.disasm(pe.get_data(site-base,10),site))
        assert instruction.mnemonic == 'mov'
        assert struct.unpack('<II',instruction.bytes[2:]) == (global_va,value)
        print(f'Confirmed dispatch initializer {global_va:#010x} = {value:#010x}')
    bookkeeping = pe.get_data(0x5b360,120)
    print('Bookkeeping SHA256:',hashlib.sha256(bookkeeping).hexdigest())
    assert hashlib.sha256(bookkeeping).hexdigest() == '2dc672ad67179fa73bca4d901a607b72986ba7138a1d00ecb954b3b9280e5e34'
    assert records[0x45b360] == 120 and pe.get_data(0x5b3d8,8) == b'\xcc'*8
    decoded = list(md.disasm(bookkeeping,0x45b360))
    assert sum(i.size for i in decoded) == 120 and not any(i.mnemonic=='call' for i in decoded)
    state_addresses = (0x510bf0,0x510f10,0x5113c0,0x5116e0,0x511a00,0x511d20,
                       0x63c690,0x63c694,0x63c698)
    for va,n in sorted(records.items()):
        for i in md.disasm(pe.get_data(va-base,n),va):
            addresses = [o.imm if o.type == X86_OP_IMM else o.mem.disp
                         for o in i.operands if o.type in (X86_OP_IMM,X86_OP_MEM)]
            if any(a in (*state_addresses,0x45b360) for a in addresses):
                print(f'Bookkeeping reference (routine {va:#010x}) {i.address:#010x}: {i.mnemonic} {i.op_str}')
    for name,va,n in [('flag',0x4bab38,4),('status',0x5116e0,800),
                      ('ids',0x511230,400),('cursor',0x512040,2),
                      ('contexts',0x510bf0,800),('parameters',0x510f10,800),
                      ('registered IDs',0x5113c0,800),('callbacks',0x511a00,800),
                      ('flags',0x511d20,800),('pending context',0x63c690,4),
                      ('pending callback',0x63c694,4),('pending parameter',0x63c698,4)]:
        section = next(s for s in pe.sections if s.VirtualAddress<=va-base<s.VirtualAddress+s.Misc_VirtualSize)
        relative = va-base-section.VirtualAddress
        backed = relative+n <= section.SizeOfRawData
        assert not backed or pe.get_data(va-base,n) == bytes(n)
        print(f'{name}: VA={va:#010x} RVA={va-base:#010x} bytes={n}; '+('file initialized zero' if backed else 'loader-zeroed virtual tail'))
    print('PASS: independent PE/startup evidence; no Ghidra names or DOS inventory used.')

if __name__ == '__main__':
    inspect()
