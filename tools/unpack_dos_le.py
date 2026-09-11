#!/usr/bin/env python3
"""
Unpacker and PE Converter for Ignition (1997) MAINDOS.EXE (DOS/4GW 32-bit Linear Executable)
Converts raw Watcom LE protected mode binary into a standard 32-bit PE executable so that
Ghidra, IDA, and modern disassemblers can analyze the full 32-bit game without needing custom LE plugins.
"""

import struct
import sys
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parent.parent
DOS_EXE_PATH = ROOT_DIR / "Ignition" / "Ignition" / "MAINDOS.EXE"
OUTPUT_PE_PATH = ROOT_DIR / "Ignition" / "Ignition" / "MAINDOS_32BIT.EXE"

def unpack():
    if not DOS_EXE_PATH.exists():
        print(f"Error: {DOS_EXE_PATH} not found", file=sys.stderr)
        return False

    print(f"Reading {DOS_EXE_PATH}...")
    with open(DOS_EXE_PATH, "rb") as f:
        data = f.read()

    le_off = 0x2c80
    if data[le_off:le_off+2] != b"LE":
        print("Error: LE signature not found at 0x2c80", file=sys.stderr)
        return False

    # Header parameters
    num_pages = struct.unpack_from("<I", data, le_off + 0x14)[0]
    cs_obj = struct.unpack_from("<I", data, le_off + 0x18)[0]
    eip = struct.unpack_from("<I", data, le_off + 0x1c)[0]
    page_size = struct.unpack_from("<I", data, le_off + 0x28)[0]
    obj_tbl_off = struct.unpack_from("<I", data, le_off + 0x40)[0] + le_off
    num_objs = struct.unpack_from("<I", data, le_off + 0x44)[0]
    fixup_page_tbl_off = struct.unpack_from("<I", data, le_off + 0x68)[0] + le_off
    fixup_rec_tbl_off = struct.unpack_from("<I", data, le_off + 0x6c)[0] + le_off
    data_pages_off = struct.unpack_from("<I", data, le_off + 0x80)[0]

    print(f"LE Header: {num_objs} objects, {num_pages} pages, Entry CS:EIP = Obj {cs_obj}:{hex(eip)}")
    print(f"Data pages start at file offset: {hex(data_pages_off)}")

    # Parse Objects
    objects = []
    for i in range(num_objs):
        off = obj_tbl_off + i * 24
        vsize, rbase, flags, pidx, pcount, _ = struct.unpack_from("<IIIIII", data, off)
        objects.append({
            "id": i + 1,
            "vsize": vsize,
            "rbase": rbase,
            "flags": flags,
            "pidx": pidx - 1, # 1-based in LE
            "pcount": pcount
        })
        print(f"  Obj {i+1}: Base={hex(rbase)}, VSize={hex(vsize)}, Pages={pcount} (start page {pidx-1})")

    # Reconstruct flat virtual memory buffer (0x0 to 0x270000)
    memory = bytearray(0x270000)
    page_vaddrs = []
    for p in range(num_pages):
        if p < 118:
            vaddr = 0x10000 + p * 4096
        elif p < 120:
            vaddr = 0x90000 + (p - 118) * 4096
        else:
            vaddr = 0xa0000 + (p - 120) * 4096
        page_vaddrs.append(vaddr)
        file_off = data_pages_off + p * 4096
        memory[vaddr : vaddr + 4096] = data[file_off : file_off + 4096]

    # Apply LE fixups
    print("Applying LE fixup relocations...")
    fixup_count = 0
    obj_bases = {1: 0x10000, 2: 0x90000, 3: 0xa0000}

    for p in range(num_pages):
        rec_start = fixup_rec_tbl_off + struct.unpack_from("<I", data, fixup_page_tbl_off + p * 4)[0]
        rec_end = fixup_rec_tbl_off + struct.unpack_from("<I", data, fixup_page_tbl_off + (p + 1) * 4)[0]

        pos = rec_start
        while pos < rec_end:
            stype = data[pos]
            tflags = data[pos + 1]
            soff = struct.unpack_from("<h", data, pos + 2)[0] # signed short
            pos += 4

            if tflags & 0x40:
                tobj = struct.unpack_from("<H", data, pos)[0]
                pos += 2
            else:
                tobj = data[pos]
                pos += 1

            if tflags & 0x10:
                toff = struct.unpack_from("<I", data, pos)[0]
                pos += 4
            else:
                toff = struct.unpack_from("<H", data, pos)[0]
                pos += 2

            target_linear = obj_bases[tobj] + toff
            source_linear = page_vaddrs[p] + soff

            if stype == 0x7: # 32-bit linear address
                curr_val = struct.unpack_from("<I", memory, source_linear)[0]
                new_val = (obj_bases[tobj] + curr_val) & 0xFFFFFFFF
                struct.pack_into("<I", memory, source_linear, new_val)
                fixup_count += 1
            elif stype == 0x8: # 32-bit relative offset (relative to instruction end: source_linear + 4)
                rel_val = (target_linear - (source_linear + 4)) & 0xFFFFFFFF
                struct.pack_into("<I", memory, source_linear, rel_val)
                fixup_count += 1
            else:
                print(f"Warning: Unsupported fixup type {hex(stype)} at page {p}, offset {hex(soff)}")

    print(f"Successfully applied {fixup_count} relocations.")

    # Build Standard 32-bit PE Executable
    print(f"Constructing standard 32-bit PE at {OUTPUT_PE_PATH}...")
    
    pe_hdr_offset = 0x80
    file_align = 0x200
    sec_align = 0x1000

    def align(val, a):
        return (val + a - 1) & ~(a - 1)

    raw_text = memory[0x10000 : 0x10000 + objects[0]["vsize"]]
    raw_rdata = memory[0x90000 : 0x90000 + objects[1]["vsize"]]
    raw_data = memory[0xa0000 : 0xa0000 + 72 * 4096] # 72 initialized data pages

    raw_text_pad = align(len(raw_text), file_align)
    raw_rdata_pad = align(len(raw_rdata), file_align)
    raw_data_pad = align(len(raw_data), file_align)

    header_size = 0x400 # 1024 bytes aligned to 0x200
    ptr_text = header_size
    ptr_rdata = ptr_text + raw_text_pad
    ptr_data = ptr_rdata + raw_rdata_pad

    total_image_size = 0x90000 + align(objects[2]["vsize"], sec_align)

    # DOS Header (64 bytes)
    dos_header = bytearray(64)
    dos_header[0:2] = b"MZ"
    struct.pack_into("<H", dos_header, 0x3C, pe_hdr_offset)

    # DOS stub (64 bytes)
    dos_stub = b"\x0e\x1f\xba\x0e\x00\xb4\x09\xcd\x21\xb8\x01\x4c\xcd\x21" + b"This program cannot be run in DOS mode.\r\r\n$\x00\x00\x00\x00\x00\x00\x00"
    dos_stub = dos_stub.ljust(pe_hdr_offset - 64, b"\x00")

    # PE Signature
    pe_sig = b"PE\x00\x00"

    # COFF File Header (20 bytes)
    coff_header = struct.pack("<HHIIIHH", 0x014c, 3, 0, 0, 0, 224, 0x0102)

    # Optional Header (224 bytes)
    entry_rva = eip # 0x45fbc (relative to ImageBase 0x10000 -> 0x55fbc)
    opt_header = bytearray(224)
    struct.pack_into("<H", opt_header, 0, 0x010b) # PE32 Magic
    opt_header[2] = 6 # MajorLinker
    opt_header[3] = 0 # MinorLinker
    struct.pack_into("<I", opt_header, 4, raw_text_pad) # SizeOfCode
    struct.pack_into("<I", opt_header, 8, raw_rdata_pad + raw_data_pad) # SizeOfInitializedData
    struct.pack_into("<I", opt_header, 12, objects[2]["vsize"] - len(raw_data)) # SizeOfUninitializedData
    struct.pack_into("<I", opt_header, 16, entry_rva) # AddressOfEntryPoint
    struct.pack_into("<I", opt_header, 20, 0x0000) # BaseOfCode
    struct.pack_into("<I", opt_header, 24, 0x80000) # BaseOfData
    struct.pack_into("<I", opt_header, 28, 0x00010000) # ImageBase
    struct.pack_into("<I", opt_header, 32, sec_align) # SectionAlignment
    struct.pack_into("<I", opt_header, 36, file_align) # FileAlignment
    struct.pack_into("<H", opt_header, 40, 4) # MajorOS
    struct.pack_into("<H", opt_header, 42, 0) # MinorOS
    struct.pack_into("<H", opt_header, 44, 0) # MajorImage
    struct.pack_into("<H", opt_header, 46, 0) # MinorImage
    struct.pack_into("<H", opt_header, 48, 4) # MajorSubsystem
    struct.pack_into("<H", opt_header, 50, 0) # MinorSubsystem
    struct.pack_into("<I", opt_header, 56, total_image_size) # SizeOfImage
    struct.pack_into("<I", opt_header, 60, header_size) # SizeOfHeaders
    struct.pack_into("<H", opt_header, 68, 3) # Subsystem = CUI
    struct.pack_into("<I", opt_header, 72, 0x100000) # SizeOfStackReserve
    struct.pack_into("<I", opt_header, 76, 0x1000) # SizeOfStackCommit
    struct.pack_into("<I", opt_header, 80, 0x100000) # SizeOfHeapReserve
    struct.pack_into("<I", opt_header, 84, 0x1000) # SizeOfHeapCommit
    struct.pack_into("<I", opt_header, 92, 16) # NumberOfRvaAndSizes

    # Section Headers (3 * 40 = 120 bytes)
    sec1 = bytearray(40)
    sec1[0:5] = b".text"
    struct.pack_into("<IIIIIIHHI", sec1, 8, objects[0]["vsize"], 0x00000, raw_text_pad, ptr_text, 0, 0, 0, 0, 0x60000020)

    sec2 = bytearray(40)
    sec2[0:6] = b".rdata"
    struct.pack_into("<IIIIIIHHI", sec2, 8, objects[1]["vsize"], 0x80000, raw_rdata_pad, ptr_rdata, 0, 0, 0, 0, 0x40000040)

    sec3 = bytearray(40)
    sec3[0:5] = b".data"
    struct.pack_into("<IIIIIIHHI", sec3, 8, objects[2]["vsize"], 0x90000, raw_data_pad, ptr_data, 0, 0, 0, 0, 0xc0000040)

    # Assemble file
    out = bytearray(header_size)
    out[0:64] = dos_header
    out[64:pe_hdr_offset] = dos_stub
    out[pe_hdr_offset:pe_hdr_offset+4] = pe_sig
    out[pe_hdr_offset+4:pe_hdr_offset+24] = coff_header
    out[pe_hdr_offset+24:pe_hdr_offset+248] = opt_header
    out[pe_hdr_offset+248:pe_hdr_offset+288] = sec1
    out[pe_hdr_offset+288:pe_hdr_offset+328] = sec2
    out[pe_hdr_offset+328:pe_hdr_offset+368] = sec3

    # Append sections padded to file alignment
    out += raw_text.ljust(raw_text_pad, b"\x00")
    out += raw_rdata.ljust(raw_rdata_pad, b"\x00")
    out += raw_data.ljust(raw_data_pad, b"\x00")

    with open(OUTPUT_PE_PATH, "wb") as f:
        f.write(out)

    print(f"Success! Output written to {OUTPUT_PE_PATH} ({len(out)} bytes).")
    print(f"Memory Map in PE:")
    print(f"  .text : 0x00010000 - {hex(0x00010000 + objects[0]['vsize'])} (Entry: {hex(0x00010000 + entry_rva)})")
    print(f"  .rdata: 0x00090000 - {hex(0x00090000 + objects[1]['vsize'])}")
    print(f"  .data : 0x000a0000 - {hex(0x000a0000 + objects[2]['vsize'])}")
    return True

def create_target_coff(output_path: Path, funcs: dict):
    """Generate standard 32-bit COFF object file (.obj) for objdiff containing exact machine code and symbols."""
    code_bytes = bytearray()
    symbols = []
    
    for name, b in funcs.items():
        pad = (4 - (len(code_bytes) % 4)) % 4
        code_bytes.extend(b"\x90" * pad)
        offset = len(code_bytes)
        code_bytes.extend(b)
        symbols.append((name, offset, len(b)))
        
    sec_data_size = len(code_bytes)
    header_size = 20 + 40 # 1 section
    raw_data_ptr = header_size
    sym_tbl_ptr = raw_data_ptr + sec_data_size
    num_symbols = len(symbols)
    
    coff_hdr = struct.pack('<HHIIIHH', 0x014c, 1, 0, sym_tbl_ptr, num_symbols, 0, 0x0104)
    sec_hdr = struct.pack('<8sIIIIIIHHI', b'.text\x00\x00\x00', 0, 0, sec_data_size, raw_data_ptr, 0, 0, 0, 0, 0x60000020)
    
    sym_entries = bytearray()
    str_table = bytearray(b'\x04\x00\x00\x00')
    
    for name, val, sz in symbols:
        name_bytes = name.encode('ascii')
        if len(name_bytes) <= 8:
            short_name = name_bytes.ljust(8, b'\x00')
        else:
            str_off = len(str_table)
            short_name = struct.pack('<II', 0, str_off)
            str_table.extend(name_bytes + b'\x00')
            
        sym_entry = struct.pack('<8sIHhBB', short_name, val, 1, 0x20, 2, 0)
        sym_entries.extend(sym_entry)
        
    struct.pack_into('<I', str_table, 0, len(str_table))
    full_obj = coff_hdr + sec_hdr + code_bytes + sym_entries + str_table
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_bytes(full_obj)

def create_wasm_stub(output_path: Path, funcs: dict):
    """Generate .asm assembly file compatible with Watcom Assembler (wasm)."""
    lines = [
        "; Generated by tools/unpack_dos_le.py - Assembly Slicing for Watcom",
        ".386",
        ".model flat",
        ".code",
        ""
    ]
    for name, b in funcs.items():
        lines.append(f"public {name}")
        lines.append(f"{name}:")
        for i in range(0, len(b), 16):
            chunk = b[i:i+16]
            lines.append("    db " + ", ".join(f"0{x:02x}h" for x in chunk))
        lines.append("")
    lines.append("end")
    lines.append("")
    
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text("\n".join(lines))

def generate_objdiff_json(modules: list):
    import json
    objdiff_path = ROOT_DIR / "objdiff.json"
    units = []
    for m in sorted(modules):
        units.append({
            "name": m,
            "target_path": f"build/decomp/asm/{m}.obj",
            "base_path": f"build/decomp/{m}.obj",
            "metadata": {
                "complete": False
            }
        })
        
    config = {
        "$schema": "https://raw.githubusercontent.com/encounter/objdiff/main/config.schema.json",
        "min_version": "2.0.0",
        "custom_make": "uv run python tools/build_decomp.py",
        "build_base": True,
        "units": units
    }
    
    with open(objdiff_path, "w", encoding="utf-8") as f:
        json.dump(config, f, indent=2)
    print(f"[objdiff] Generated {objdiff_path.name} with {len(units)} units.")

def generate_wlink_script(modules: list):
    wlink_path = ROOT_DIR / "build" / "decomp" / "wlink.lnk"
    wlink_path.parent.mkdir(parents=True, exist_ok=True)
    lines = [
        "# Watcom Linker Directive Script for Racing Dynamite (MAINDOS)",
        "system dos4g",
        "name build/decomp/MAINDOS_REBUILT.EXE",
        "option quiet",
        "option map=build/decomp/MAINDOS_REBUILT.MAP",
    ]
    for m in sorted(modules):
        lines.append(f"file build/decomp/{m}.obj")
        
    with open(wlink_path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")
    print(f"[wlink] Generated linker script: {wlink_path}")

def slice_all_modules():
    """Slices original functions from MAINDOS_32BIT.EXE into per-module asm and COFF objects."""
    if not OUTPUT_PE_PATH.exists():
        print("Unpacking MAINDOS.EXE first...")
        if not unpack():
            return False

    with open(OUTPUT_PE_PATH, "rb") as f:
        pe_data = f.read()

    import sqlite3
    db_path = ROOT_DIR / "database" / "decomp.db"
    if not db_path.exists():
        print(f"Warning: Database {db_path} not found.")
        return False

    conn = sqlite3.connect(db_path)
    conn.row_factory = sqlite3.Row
    cur = conn.cursor()
    cur.execute("""
        SELECT f.symbol_name, f.dos_address, f.byte_size, m.name as module_name
        FROM functions f
        JOIN modules m ON f.module_id = m.id
        WHERE f.dos_address IS NOT NULL AND f.byte_size IS NOT NULL
        ORDER BY m.name, f.dos_address
    """)
    rows = cur.fetchall()
    conn.close()

    modules = {}
    for r in rows:
        mod = r["module_name"]
        stem = Path(mod).stem
        sym = r["symbol_name"]
        dos_addr = int(r["dos_address"], 16)
        sz = r["byte_size"]
        
        file_off = 0x400 + (dos_addr - 0x10000)
        code = pe_data[file_off : file_off + sz]
        
        if stem not in modules:
            modules[stem] = {}
        modules[stem][f"{sym}_"] = code

    asm_dir = ROOT_DIR / "build" / "decomp" / "asm"
    asm_dir.mkdir(parents=True, exist_ok=True)
    
    print(f"\n[Assembly Slicing] Slicing {len(modules)} modules into {asm_dir}...")
    for stem, funcs in modules.items():
        obj_path = asm_dir / f"{stem}.obj"
        asm_path = asm_dir / f"{stem}.asm"
        
        create_target_coff(obj_path, funcs)
        create_wasm_stub(asm_path, funcs)
        print(f"  Sliced {stem}: {len(funcs)} functions -> {obj_path.name}, {asm_path.name}")
        
    generate_objdiff_json(list(modules.keys()))
    generate_wlink_script(list(modules.keys()))
    return True

if __name__ == "__main__":
    unpack()
    slice_all_modules()
