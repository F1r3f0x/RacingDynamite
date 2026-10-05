import sys
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
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

def main():
    if len(sys.argv) < 2:
        print("Usage: disasm_le.py <hex_address> [count_insns|hex_bytes]")
        sys.exit(1)

    addr = int(sys.argv[1], 16)
    count = int(sys.argv[2], 0) if len(sys.argv) > 2 else 50

    le = LEFile(r'Ignition\Ignition\MAINDOS.EXE')
    obj, offset = resolve_addr(le, addr)
    if not obj or offset is None:
        print(f"Address {hex(addr)} not found or is in BSS.")
        return

    # If count > 1000, treat as byte length, else treat as instruction count
    if count > 1000:
        max_bytes = count
        limit_insns = None
    else:
        max_bytes = count * 15
        limit_insns = count

    code = obj['data'][offset:offset + max_bytes]
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    
    insn_count = 0
    for i in md.disasm(code, addr):
        print(f"0x{i.address:08x}:  {i.mnemonic:<8} {i.op_str}")
        insn_count += 1
        if limit_insns and insn_count >= limit_insns:
            break

if __name__ == '__main__':
    main()
