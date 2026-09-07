#!/usr/bin/env python3
"""
tools/diff_func.py
Extracts disassembly of a function from MAINDOS_32BIT.EXE and from build/decomp/<module>.obj,
and compares them instruction-by-instruction.
"""

import sys
from pathlib import Path
import capstone

ROOT = Path(__file__).resolve().parent.parent

def load_coff_symbols(obj_path: Path):
    with open(obj_path, "rb") as f:
        data = f.read()
    
    num_sections = int.from_bytes(data[2:4], "little")
    sym_table_ptr = int.from_bytes(data[8:12], "little")
    num_symbols = int.from_bytes(data[12:16], "little")
    str_table_ptr = sym_table_ptr + num_symbols * 18
    str_table = data[str_table_ptr:]
    
    # Read sections
    sections = []
    for s in range(num_sections):
        sec_header = data[20 + s * 40 : 20 + (s + 1) * 40]
        name = sec_header[0:8].split(b'\x00')[0].decode('ascii', errors='replace')
        size = int.from_bytes(sec_header[16:20], "little")
        raw_ptr = int.from_bytes(sec_header[20:24], "little")
        reloc_ptr = int.from_bytes(sec_header[24:28], "little")
        num_relocs = int.from_bytes(sec_header[32:34], "little")
        sections.append({
            "name": name,
            "size": size,
            "raw_ptr": raw_ptr,
            "data": data[raw_ptr : raw_ptr + size],
            "relocs": []
        })
        # Read relocations
        for r in range(num_relocs):
            rel_entry = data[reloc_ptr + r * 10 : reloc_ptr + (r + 1) * 10]
            r_vaddr = int.from_bytes(rel_entry[0:4], "little")
            r_sym = int.from_bytes(rel_entry[4:8], "little")
            r_type = int.from_bytes(rel_entry[8:10], "little")
            sections[-1]["relocs"].append((r_vaddr, r_sym, r_type))
            
    symbols = {}
    i = 0
    while i < num_symbols:
        sym_entry = data[sym_table_ptr + i * 18 : sym_table_ptr + (i + 1) * 18]
        if sym_entry[0:4] == b'\x00\x00\x00\x00':
            offset = int.from_bytes(sym_entry[4:8], "little")
            end = str_table.find(b'\x00', offset)
            name = str_table[offset:end].decode("ascii", errors="replace")
        else:
            name = sym_entry[0:8].split(b'\x00')[0].decode("ascii", errors="replace")
        
        val = int.from_bytes(sym_entry[8:12], "little")
        sec_num = int.from_bytes(sym_entry[12:14], "little", signed=True)
        storage_class = sym_entry[16]
        aux_count = sym_entry[17]
        
        if sec_num > 0 and storage_class == 2: # External defined
            symbols[name] = {
                "section": sec_num - 1,
                "value": val,
            }
        i += 1 + aux_count
        
    return sections, symbols

def get_orig_func(dos_addr: int, size: int):
    exe_path = ROOT / "Ignition" / "Ignition" / "MAINDOS_32BIT.EXE"
    with open(exe_path, "rb") as f:
        data = f.read()
    # In MAINDOS_32BIT.EXE, image base is 0x10000, .text starts at file offset 0x400
    file_off = 0x400 + (dos_addr - 0x10000)
    return data[file_off : file_off + size]

def normalize_asm(mnemonic: str, op_str: str, ins_addr: int = 0, base_addr: int = 0):
    import re
    op = op_str
    
    # Calls to external or linked functions
    if mnemonic == "call":
        return f"{mnemonic:<8} <FUNC>"
        
    # Relative jumps (normalize target to relative offset from start of function)
    if mnemonic.startswith("j"):
        m = re.search(r'0x[0-9a-f]+|\d+', op)
        if m:
            try:
                target = int(m.group(0), 16 if '0x' in m.group(0) else 10)
                rel = target - base_addr
                return f"{mnemonic:<8} <TARGET_+{rel}>"
            except Exception:
                return f"{mnemonic:<8} <TARGET>"
        return f"{mnemonic:<8} <TARGET>"
        
    # Relocations in data references: 0x000... or 0xaae... or 0x0 in unlinked COFF
    # e.g. "edx, 0xaae64" vs "edx, 0"
    op = re.sub(r'(edx|eax|ecx|esi|edi|ebx),\s*(0x[0-9a-f]{5,8}|0)\b', r'\1, <ADDR>', op)
    op = re.sub(r'\[(0x[0-9a-f]{5,8}|0)\]', '[<ADDR>]', op)
    op = re.sub(r'0x[0-9a-f]{5,8}', '<ADDR>', op)
    return f"{mnemonic:<8} {op}"

def diff_func(symbol_name: str, dos_addr: int, size: int, obj_file: Path):
    sections, symbols = load_coff_symbols(obj_file)
    
    # Watcom symbol naming: foo_ or _foo
    target_sym = None
    for name in [symbol_name, f"{symbol_name}_", f"_{symbol_name}"]:
        if name in symbols:
            target_sym = symbols[name]
            break
            
    if not target_sym:
        print(f"Error: Symbol {symbol_name} not found in {obj_file.name}")
        print("Available symbols:", list(symbols.keys()))
        return False
        
    sec = sections[target_sym["section"]]
    compiled_code = sec["data"][target_sym["value"] : target_sym["value"] + size]
    orig_code = get_orig_func(dos_addr, size)
    
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    orig_ins = list(md.disasm(orig_code, dos_addr))
    comp_ins = list(md.disasm(compiled_code, target_sym["value"]))
    
    print(f"=== Diffing {symbol_name} (DOS: {hex(dos_addr)}, Size: {size} bytes) ===")
    print(f"{'ORIGINAL (MAINDOS)':<45} | {'COMPILED (Watcom)':<45}")
    print("-" * 93)
    
    max_len = max(len(orig_ins), len(comp_ins))
    matches = 0
    mismatches = 0
    
    for i in range(max_len):
        o_str = ""
        c_str = ""
        o_norm = ""
        c_norm = ""
        
        if i < len(orig_ins):
            o = orig_ins[i]
            o_str = f"{hex(o.address)}: {o.mnemonic} {o.op_str}"
            o_norm = normalize_asm(o.mnemonic, o.op_str, o.address, dos_addr)
            
        if i < len(comp_ins):
            c = comp_ins[i]
            c_str = f"+{hex(c.address)}: {c.mnemonic} {c.op_str}"
            c_norm = normalize_asm(c.mnemonic, c.op_str, c.address, target_sym["value"])
            
        matched = (o_norm == c_norm) and (o_norm != "")
        marker = " " if matched else "!"
        if matched:
            matches += 1
        else:
            mismatches += 1
            
        print(f"{marker} {o_str:<43} | {c_str:<45}")
        
    total = matches + mismatches
    pct = (matches / total * 100.0) if total > 0 else 0.0
    print("-" * 93)
    print(f"Result: {matches}/{total} instructions matched ({pct:.1f}%)\n")
    return pct == 100.0

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: python diff_func.py <symbol> <dos_addr_hex> <size> [obj_filename]")
        sys.exit(1)
    sym = sys.argv[1]
    addr = int(sys.argv[2], 16)
    sz = int(sys.argv[3])
    
    if len(sys.argv) >= 5:
        obj = ROOT / "build" / "decomp" / sys.argv[4]
    else:
        # Search all obj files in build/decomp
        obj = None
        for cand in (ROOT / "build" / "decomp").glob("*.obj"):
            sections, symbols = load_coff_symbols(cand)
            if any(name in symbols for name in [sym, f"{sym}_", f"_{sym}"]):
                obj = cand
                break
        if not obj:
            obj = ROOT / "build" / "decomp" / "getsurf.obj"
            
    diff_func(sym, addr, sz, obj)

