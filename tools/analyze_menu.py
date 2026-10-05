import sys
import os
sys.path.insert(0, os.path.dirname(__file__))

from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from le_parser import LEFile
import struct

le = LEFile(r'Ignition\Ignition\MAINDOS.EXE')

def resolve_addr(vaddr):
    for obj in le.objects:
        base = obj['reloc_base']
        vsize = obj['vsize']
        if base <= vaddr < base + vsize:
            offset = vaddr - base
            if offset < len(obj['data']):
                return obj, offset
    return None, None

def disasm_range(start_va, end_va):
    obj, offset = resolve_addr(start_va)
    if not obj or offset is None:
        print(f"Address {hex(start_va)} not found")
        return
    length = end_va - start_va
    code = obj['data'][offset:offset+length]
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    for i in md.disasm(code, start_va):
        print(f"0x{i.address:08x}:  {i.mnemonic:<8} {i.op_str}")

if __name__ == '__main__':
    start = int(sys.argv[1], 16) if len(sys.argv) > 1 else 0x11504
    end = int(sys.argv[2], 16) if len(sys.argv) > 2 else start + 0x200
    disasm_range(start, end)
