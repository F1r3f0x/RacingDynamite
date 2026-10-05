#!/usr/bin/env python3
"""
tools/diff_func.py
Extracts disassembly of a function from authentic MAINDOS.EXE and from build/decomp/<module>.obj,
and compares them instruction-by-instruction.
"""

import sys
from pathlib import Path
import capstone

ROOT = Path(__file__).resolve().parent.parent

def load_coff_symbols(obj_path: Path):
    with open(obj_path, "rb") as f:
        data = f.read()
    
    if len(data) < 20 or data[0:2] != b'\x4c\x01':
        return [], {}  # Not a COFF file (likely OMF from wasm), skip it.
        
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
    from le_parser import LEFile
    le = LEFile(str(ROOT / "Ignition" / "Ignition" / "MAINDOS.EXE"))
    obj = le.objects[0]
    offset = dos_addr - obj['reloc_base']
    return obj['data'][offset : offset + size]

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

def get_symbol_size(symbols, target_sym, sec_size):
    target_val = target_sym["value"]
    sec_idx = target_sym["section"]
    next_vals = [s["value"] for s in symbols.values() if s["section"] == sec_idx and s["value"] > target_val]
    if next_vals:
        return min(next_vals) - target_val
    return sec_size - target_val


def get_rebuilt_func(symbol_name: str, size: int):
    map_path = ROOT / "build" / "decomp" / "MAINDOS_REBUILT.MAP"
    exe_path = ROOT / "build" / "decomp" / "MAINDOS_REBUILT.EXE"
    if not map_path.exists() or not exe_path.exists():
        return None, None
    with open(map_path, "r", encoding="utf-8") as f:
        map_lines = f.readlines()
    clean_name = symbol_name.strip('_')
    target_names = list(set([
        symbol_name, f"{symbol_name}_", f"_{symbol_name}",
        clean_name, f"_{clean_name}", f"{clean_name}_", f"_{clean_name}_"
    ]))
    syms = []
    target_info = None
    for line in map_lines:
        parts = line.split()
        if len(parts) >= 2 and ":" in parts[0]:
            addr_str = parts[0].rstrip("+*")
            seg_str, off_str = addr_str.split(":")
            if not off_str: continue
            try:
                seg = int(seg_str, 16)
                off = int(off_str, 16)
            except ValueError:
                continue
            sym = parts[1]
            syms.append((seg, off, sym))
            if sym in target_names:
                target_info = (seg, off, sym)
    
    if not target_info:
        return None, None
        
    t_seg, t_off, _ = target_info
    if size <= 0:
        same_seg = [s for s in syms if s[0] == t_seg and s[1] > t_off]
        if same_seg:
            same_seg.sort(key=lambda x: x[1])
            size = same_seg[0][1] - t_off
        else:
            size = 0x100

    with open(exe_path, "rb") as f:
        data = f.read()

    hdr_off = int.from_bytes(data[0x3C:0x40], "little")
    if hdr_off + 2 <= len(data) and data[hdr_off:hdr_off+2] == b"LE":
        data_pages_off = int.from_bytes(data[hdr_off+0x80:hdr_off+0x84], "little")
        if t_seg == 1:
            raw_off = data_pages_off + t_off
            return data[raw_off : raw_off + size], 0x10000 + t_off

    # PE fallback
    pe_hdr_off = hdr_off
    if pe_hdr_off + 4 <= len(data) and data[pe_hdr_off:pe_hdr_off+2] == b"PE":
        num_sections = int.from_bytes(data[pe_hdr_off+6:pe_hdr_off+8], "little")
        opt_hdr_sz = int.from_bytes(data[pe_hdr_off+20:pe_hdr_off+22], "little")
        sections_off = pe_hdr_off + 24 + opt_hdr_sz
        image_base = int.from_bytes(data[pe_hdr_off+52:pe_hdr_off+56], "little")
        val = image_base + (0x1000 if t_seg == 1 else 0x10000) + t_off
        rva = val - image_base
        for i in range(num_sections):
            sec = data[sections_off + i*40 : sections_off + (i+1)*40]
            v_size = int.from_bytes(sec[8:12], "little")
            v_addr = int.from_bytes(sec[12:16], "little")
            raw_size = int.from_bytes(sec[16:20], "little")
            raw_ptr = int.from_bytes(sec[20:24], "little")
            if v_addr <= rva < v_addr + max(v_size, raw_size):
                raw_off = raw_ptr + (rva - v_addr)
                return data[raw_off : raw_off + size], val
    return None, None

def diff_func(symbol_name: str, dos_addr: int, size: int = 0, obj_file = None, verbose: bool = True):
    rebuilt_code, target_val = get_rebuilt_func(symbol_name, size)
    
    if rebuilt_code is None:
        return _diff_func_orig(symbol_name, dos_addr, size, obj_file, verbose)
        
    orig_code = get_orig_func(dos_addr, len(rebuilt_code))
    
    import capstone
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    orig_ins = list(md.disasm(orig_code, dos_addr))
    comp_ins = list(md.disasm(rebuilt_code, target_val))
    
    if verbose:
        print(f"=== Diffing {symbol_name} (DOS: {hex(dos_addr)}, Size: {size} bytes) ===")
        print(f"{'ORIGINAL (MAINDOS)':<45} | {'COMPILED (Watcom)':<45}")
        print("-" * 93)
    
    max_len = max(len(orig_ins), len(comp_ins))
    matches = 0
    mismatches = 0
    
    for i in range(max_len):
        o_str = ""
        c_str = ""
        
        if i < len(orig_ins):
            o_ins = orig_ins[i]
            o_str = normalize_asm(o_ins.mnemonic, o_ins.op_str, o_ins.address, dos_addr)
        if i < len(comp_ins):
            c_ins = comp_ins[i]
            c_str = normalize_asm(c_ins.mnemonic, c_ins.op_str, c_ins.address, target_val)
            
        match = o_str == c_str
        if match:
            matches += 1
        else:
            mismatches += 1
            
        if verbose:
            flag = " " if match else "!"
            print(f"{flag} {o_str:<43} | {c_str:<43}")
            
    pct = (matches / max_len * 100.0) if max_len > 0 else 0.0
    return (mismatches == 0 and max_len > 0), pct, matches, max_len

def _diff_func_orig(symbol_name: str, dos_addr: int, size: int = 0, obj_file: Path = None, verbose: bool = True):
    if obj_file is None:
        # Search all obj files in build/decomp
        for cand in (ROOT / "build" / "decomp").glob("*.obj"):
            sections, symbols = load_coff_symbols(cand)
            if any(name in symbols for name in [symbol_name, f"{symbol_name}_", f"_{symbol_name}"]):
                obj_file = cand
                break
        if not obj_file:
            if verbose:
                print(f"Error: Symbol {symbol_name} not found in any object file under build/decomp/")
            return False, 0.0, 0, 0
    else:
        sections, symbols = load_coff_symbols(obj_file)
    
    # Watcom symbol naming: foo_ or _foo
    target_sym = None
    for name in [symbol_name, f"{symbol_name}_", f"_{symbol_name}"]:
        if name in symbols:
            target_sym = symbols[name]
            break
            
    if not target_sym:
        if verbose:
            print(f"Error: Symbol {symbol_name} not found in {obj_file.name}")
            print("Available symbols:", list(symbols.keys()))
        return False, 0.0, 0, 0
        
    sec = sections[target_sym["section"]]
    if not size or size <= 0:
        size = get_symbol_size(symbols, target_sym, sec["size"])

    compiled_code = sec["data"][target_sym["value"] : target_sym["value"] + size]
    orig_code = get_orig_func(dos_addr, size)
    
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    orig_ins = list(md.disasm(orig_code, dos_addr))
    comp_ins = list(md.disasm(compiled_code, target_sym["value"]))
    
    if verbose:
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
            
        if verbose:
            print(f"{marker} {o_str:<43} | {c_str:<45}")
        
    total = matches + mismatches
    pct = (matches / total * 100.0) if total > 0 else 0.0
    if verbose:
        print("-" * 93)
        print(f"Result: {matches}/{total} instructions matched ({pct:.1f}%)\n")
    return pct == 100.0, pct, matches, total

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python diff_func.py <symbol> <dos_addr_hex> [size] [obj_filename]")
        sys.exit(1)
    sym = sys.argv[1]
    addr = int(sys.argv[2], 16)
    sz = int(sys.argv[3]) if len(sys.argv) >= 4 and sys.argv[3].isdigit() else 0
    
    obj = None
    if len(sys.argv) >= 5:
        obj = ROOT / "build" / "decomp" / sys.argv[4]
    elif len(sys.argv) == 4 and not sys.argv[3].isdigit():
        obj = ROOT / "build" / "decomp" / sys.argv[3]
        sz = 0
            
    matched, pct, m, t = diff_func(sym, addr, sz, obj, verbose=True)
    sys.exit(0 if matched else 1)

