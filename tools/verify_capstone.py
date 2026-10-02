import sys
import struct
from capstone import *
from le_parser import LEFile

def resolve_addr(le, vaddr):
    for obj in le.objects:
        base = obj['reloc_base']
        vsize = obj['vsize']
        if base <= vaddr < base + vsize:
            offset = vaddr - base
            if offset < len(obj['data']):
                return obj, offset
            else:
                return obj, None
    return None, None

def disassemble_func(le, vaddr, max_bytes=5000):
    obj, offset = resolve_addr(le, vaddr)
    if not obj or offset is None:
        print(f"Address {hex(vaddr)} not found or is in BSS.")
        return
        
    code = obj['data'][offset:offset+max_bytes]
    
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    print(f"Disassembly of {hex(vaddr)}:")
    
    for i in md.disasm(code, vaddr):
        print("0x%x:\t%s\t%s" % (i.address, i.mnemonic, i.op_str))
        if i.mnemonic in ('ret', 'retn') and i.address > vaddr + 10:
            break


def dump_data(le, vaddr, size=16):
    obj, offset = resolve_addr(le, vaddr)
    if not obj or offset is None:
        print(f"Address {hex(vaddr)} not found or is in BSS.")
        return
        
    data = obj['data'][offset:offset+size]
    print(f"Data at {hex(vaddr)}:")
    
    for i in range(0, len(data), 4):
        chunk = data[i:i+4]
        if len(chunk) == 4:
            val = struct.unpack('<I', chunk)[0]
            print(f"0x{vaddr+i:X}:\t{hex(val)}")
        else:
            print(f"0x{vaddr+i:X}:\t{chunk.hex()}")

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage: verify_capstone.py [disasm|data] <hex_address>")
        sys.exit(1)
        
    mode = sys.argv[1]
    addr = int(sys.argv[2], 16)
    
    le = LEFile(r'Ignition\Ignition\MAINDOS.EXE')
    
    if mode == 'disasm':
        disassemble_func(le, addr)
    elif mode == 'data':
        dump_data(le, addr, 64)
    else:
        print("Unknown mode.")
