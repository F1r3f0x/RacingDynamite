#!/usr/bin/env python3
"""
Decompilation Project SQLite Database Manager
Manages tracking database for Ignition (1997) DOS decompilation.
"""

import argparse
import os
import re
import sqlite3
import sys
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parent.parent
DB_PATH = ROOT_DIR / "database" / "decomp.db"
SCHEMA_PATH = ROOT_DIR / "database" / "schema.sql"
DUMP_PATH = ROOT_DIR / "database" / "dump.sql"

def get_connection():
    os.makedirs(DB_PATH.parent, exist_ok=True)
    conn = sqlite3.connect(DB_PATH)
    conn.execute("PRAGMA foreign_keys = ON;")
    conn.row_factory = sqlite3.Row
    return conn

def init_db():
    print(f"Initializing database at {DB_PATH}...")
    if DB_PATH.exists():
        DB_PATH.unlink()
    if not SCHEMA_PATH.exists():
        print(f"Error: Schema not found at {SCHEMA_PATH}", file=sys.stderr)
        return False
    with open(SCHEMA_PATH, "r", encoding="utf-8") as f:
        schema_sql = f.read()
    
    conn = get_connection()
    conn.executescript(schema_sql)
    
    # Set default metadata
    cur = conn.cursor()
    cur.execute("INSERT OR REPLACE INTO metadata (key, value) VALUES (?, ?)", ("target_exe", "MAINDOS.EXE"))
    cur.execute("INSERT OR REPLACE INTO metadata (key, value) VALUES (?, ?)", ("reference_exe", "IGN_WIN.EXE"))
    cur.execute("INSERT OR REPLACE INTO metadata (key, value) VALUES (?, ?)", ("compiler", "Watcom C/C++ 10.6"))
    cur.execute("INSERT OR REPLACE INTO metadata (key, value) VALUES (?, ?)", ("project_name", "Racing Dynamite Decompilation"))
    
    # Pre-populate known modules
    modules = [
        ("getsurf.c", r"d:\projects\ignition\getsurf\getsurf.c", "decomp/getsurf.c", "Track surface raycasting and collision grid"),
        ("lisa3d.c", "lisa3d.c", "decomp/lisa3d.c", "Lisa 2 3D rasterizer, polygon opcodes, and scene transformation"),
        ("geputget.c", "geputget.c", "decomp/geputget.c", "2D graphics blitting, font loading, and palette management"),
        ("mem.c", "mem.c", "decomp/mem.c", "Memory management, buffer allocation, and file I/O"),
        ("main.c", "main.c", "decomp/main.c", "Main game loop, vehicle state, physics integration, AI navigation"),
        ("sound.c", "sound.c", "decomp/sound.c", "Sound effects pools, engine RPM audio synthesis, and mixer"),
    ]
    for name, orig_path, decomp_path, desc in modules:
        cur.execute("""
            INSERT OR IGNORE INTO modules (name, original_path, decomp_path, description)
            VALUES (?, ?, ?, ?)
        """, (name, orig_path, decomp_path, desc))
    
    conn.commit()
    conn.close()
    print("Database initialized successfully.")
    return True

def clean_val(v: str) -> str:
    if not v:
        return ""
    return v.replace("`", "").strip()

def import_markdown():
    conn = get_connection()
    cur = conn.cursor()
    
    # 1. Import Functions from docs/ghidra/functions.md
    fn_md = ROOT_DIR / "docs" / "ghidra" / "functions.md"
    if fn_md.exists():
        print(f"Importing functions from {fn_md}...")
        with open(fn_md, "r", encoding="utf-8") as f:
            lines = f.readlines()
        
        imported_fns = 0
        for line in lines:
            line = line.strip()
            if not line.startswith("| `0x") and not line.startswith("| 0x"):
                continue
            parts = [p.strip() for p in line.split("|")[1:-1]]
            if len(parts) < 8:
                continue
            
            win_addr = clean_val(parts[0])
            orig_ghidra = clean_val(parts[1])
            sym_name = clean_val(parts[2])
            raw_module = clean_val(parts[3])
            status = clean_val(parts[4]).lower()
            fidelity = clean_val(parts[5])
            port_loc = clean_val(parts[6])
            purpose = parts[7].strip()
            
            # Normalize module
            module_name = raw_module.split("/")[0].strip() if "/" in raw_module else raw_module
            module_name = module_name.replace("`", "").strip()
            
            mod_id = None
            if module_name and module_name != "-":
                cur.execute("SELECT id FROM modules WHERE name = ? OR name LIKE ?", (module_name, f"%{module_name}"))
                row = cur.fetchone()
                if row:
                    mod_id = row[0]
                else:
                    cur.execute("INSERT INTO modules (name) VALUES (?)", (module_name,))
                    mod_id = cur.lastrowid
            
            if status not in ("unidentified", "analyzed", "decompiled", "matching"):
                status = "analyzed" if status == "analyzed" else "decompiled" if status == "ported" else "unidentified"
            
            cur.execute("""
                INSERT OR REPLACE INTO functions 
                (win_address, symbol_name, original_ghidra_name, module_id, status, fidelity, port_location, purpose)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?)
            """, (win_addr, sym_name, orig_ghidra, mod_id, status, fidelity, port_loc, purpose))
            imported_fns += 1
        print(f"Imported {imported_fns} functions.")

    # 2. Import Globals from docs/ghidra/globals.md
    glob_md = ROOT_DIR / "docs" / "ghidra" / "globals.md"
    if glob_md.exists():
        print(f"Importing globals from {glob_md}...")
        with open(glob_md, "r", encoding="utf-8") as f:
            lines = f.readlines()
        
        imported_globs = 0
        for line in lines:
            line = line.strip()
            if not line.startswith("| `0x") and not line.startswith("| 0x"):
                continue
            parts = [p.strip() for p in line.split("|")[1:-1]]
            if len(parts) < 4:
                continue
            
            win_addr = clean_val(parts[0])
            gtype = clean_val(parts[1])
            name = clean_val(parts[2])
            purpose = parts[3].strip()
            
            cur.execute("""
                INSERT OR REPLACE INTO globals (win_address, name, type, description)
                VALUES (?, ?, ?, ?)
            """, (win_addr, name, gtype, purpose))
            imported_globs += 1
        print(f"Imported {imported_globs} globals.")

    # 3. Import Structs from docs/ghidra/structs.md
    structs_md = ROOT_DIR / "docs" / "ghidra" / "structs.md"
    if structs_md.exists():
        print(f"Importing structs from {structs_md}...")
        with open(structs_md, "r", encoding="utf-8") as f:
            content = f.read()
        
        struct_blocks = re.findall(r"###\s+`(\w+)`\s+\((\d+)\s+bytes[^\)]*\)\s*```c(.*?)```", content, re.DOTALL)
        imported_structs = 0
        for sname, ssize, scode in struct_blocks:
            cur.execute("INSERT OR REPLACE INTO structs (name, size) VALUES (?, ?)", (sname, int(ssize)))
            struct_id = cur.execute("SELECT id FROM structs WHERE name = ?", (sname,)).fetchone()[0]
            
            field_lines = re.findall(r"(\w+[\*\s\w]+)\s+(\w+);\s*//\s*(0x[0-9a-fA-F]+):\s*(.*)", scode)
            for ftype, fname, foff_str, fdesc in field_lines:
                foff = int(foff_str, 16)
                cur.execute("""
                    INSERT OR REPLACE INTO struct_fields (struct_id, offset, type, name, description)
                    VALUES (?, ?, ?, ?, ?)
                """, (struct_id, foff, ftype.strip(), fname.strip(), fdesc.strip()))
            imported_structs += 1
        print(f"Imported {imported_structs} structs.")

    # 4. Import Deviations from docs/tracking/deviations.md
    dev_md = ROOT_DIR / "docs" / "tracking" / "deviations.md"
    if dev_md.exists():
        print(f"Importing deviations from {dev_md}...")
        with open(dev_md, "r", encoding="utf-8") as f:
            content = f.read()
        
        dev_blocks = re.findall(r"###\s+(DEV-\d+):\s+([^\n]+)", content)
        imported_devs = 0
        for dev_id, title in dev_blocks:
            cur.execute("INSERT OR REPLACE INTO deviations (id, title, category) VALUES (?, ?, ?)",
                        (dev_id, title.strip(), "GENERAL"))
            imported_devs += 1
        print(f"Imported {imported_devs} deviations.")

    conn.commit()
    conn.close()
    print("Import complete.")

def dump_sql():
    conn = get_connection()
    print(f"Dumping database to {DUMP_PATH}...")
    with open(DUMP_PATH, "w", encoding="utf-8") as f:
        for line in conn.iterdump():
            f.write(f"{line}\n")
    conn.close()
    print("SQL dump complete.")

def show_status():
    conn = get_connection()
    cur = conn.cursor()
    
    cur.execute("SELECT COUNT(*) FROM functions")
    total_fns = cur.fetchone()[0]
    
    cur.execute("SELECT status, COUNT(*) FROM functions GROUP BY status")
    status_counts = dict(cur.fetchall())
    
    cur.execute("SELECT COUNT(*) FROM globals")
    total_globs = cur.fetchone()[0]
    
    cur.execute("SELECT COUNT(*) FROM structs")
    total_structs = cur.fetchone()[0]
    
    cur.execute("SELECT COUNT(*) FROM modules")
    total_mods = cur.fetchone()[0]
    
    print("==================================================")
    print("   IGNITION (1997) DECOMPILATION STATUS (DOS)     ")
    print("==================================================")
    print(f"Target Executable : MAINDOS.EXE (Watcom DOS/4GW 32-bit)")
    print(f"Modules Tracked   : {total_mods}")
    print(f"Structures Defined: {total_structs}")
    print(f"Globals Documented: {total_globs}")
    print(f"Total Functions   : {total_fns}")
    print("--------------------------------------------------")
    for s in ("unidentified", "analyzed", "decompiled", "matching"):
        cnt = status_counts.get(s, 0)
        pct = (cnt / total_fns * 100) if total_fns > 0 else 0
        print(f"  {s.capitalize():<14}: {cnt:>4} ({pct:>5.1f}%)")
    print("--------------------------------------------------")
    
    print(f"{'Module':<16} {'Total':<6} {'Analyzed':<10} {'Decompiled':<12} {'Matching':<8}")
    cur.execute("""
        SELECT m.name, 
               COUNT(f.id) as total,
               SUM(CASE WHEN f.status = 'analyzed' THEN 1 ELSE 0 END) as analyzed,
               SUM(CASE WHEN f.status = 'decompiled' THEN 1 ELSE 0 END) as decompiled,
               SUM(CASE WHEN f.status = 'matching' THEN 1 ELSE 0 END) as matching
        FROM modules m
        LEFT JOIN functions f ON f.module_id = m.id
        GROUP BY m.id
        ORDER BY total DESC
    """)
    for row in cur.fetchall():
        print(f"{row['name']:<16} {row['total']:<6} {row['analyzed']:<10} {row['decompiled']:<12} {row['matching']:<8}")
    print("==================================================")
    conn.close()

def export_markdown():
    conn = get_connection()
    cur = conn.cursor()
    
    out_path = ROOT_DIR / "docs" / "ghidra" / "functions.md"
    print(f"Exporting function registry to {out_path}...")
    
    cur.execute("""
        SELECT f.dos_address, f.win_address, f.original_ghidra_name, f.symbol_name, 
               COALESCE(m.name, '-') as module_name, f.status, f.fidelity, f.purpose
        FROM functions f
        LEFT JOIN modules m ON f.module_id = m.id
        ORDER BY f.win_address ASC
    """)
    rows = cur.fetchall()
    
    with open(out_path, "w", encoding="utf-8") as f:
        f.write("# Master Function Registry (MAINDOS.EXE / IGN_WIN.EXE)\n\n")
        f.write("> Auto-generated from `database/decomp.db`. Edit via `tools/db.py`.\n\n")
        f.write("| DOS Addr | Win Addr | Ghidra Label | Symbol Name | Module | Status | Fidelity | Purpose |\n")
        f.write("| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |\n")
        for r in rows:
            dos_addr = f"`{r['dos_address']}`" if r['dos_address'] else "-"
            win_addr = f"`{r['win_address']}`" if r['win_address'] else "-"
            ghidra_lbl = f"`{r['original_ghidra_name']}`" if r['original_ghidra_name'] else "-"
            sym_name = f"`{r['symbol_name']}`"
            mod_name = f"`{r['module_name']}`" if r['module_name'] != "-" else "-"
            stat = r['status'].capitalize()
            fid = r['fidelity'] or "-"
            purp = r['purpose'] or ""
            f.write(f"| {dos_addr} | {win_addr} | {ghidra_lbl} | {sym_name} | {mod_name} | {stat} | {fid} | {purp} |\n")
            
    print("Export complete.")
    conn.close()

def query(sql: str):
    conn = get_connection()
    cur = conn.cursor()
    try:
        cur.execute(sql)
        if sql.strip().upper().startswith("SELECT"):
            rows = cur.fetchall()
            if not rows:
                print("0 rows returned.")
                return
            headers = rows[0].keys()
            print(" | ".join(headers))
            print("-" * 50)
            for r in rows:
                print(" | ".join(str(r[h]) for h in headers))
        else:
            conn.commit()
            print(f"Executed. Rows affected: {cur.rowcount}")
    except Exception as e:
        print(f"SQL Error: {e}", file=sys.stderr)
    finally:
        conn.close()

def main():
    parser = argparse.ArgumentParser(description="Decompilation Database CLI")
    subparsers = parser.add_subparsers(dest="command", required=True)
    
    subparsers.add_parser("init", help="Initialize the database")
    subparsers.add_parser("import-markdown", help="Import data from docs/ghidra markdown files")
    subparsers.add_parser("export-markdown", help="Export database to docs/ghidra/functions.md")
    subparsers.add_parser("dump-sql", help="Dump entire database to database/dump.sql")
    subparsers.add_parser("status", help="Show decompilation progress metrics")
    
    q_parser = subparsers.add_parser("query", help="Execute an arbitrary SQL query")
    q_parser.add_argument("sql", type=str, help="SQL string to execute")
    
    args = parser.parse_args()
    if args.command == "init":
        init_db()
    elif args.command == "import-markdown":
        import_markdown()
    elif args.command == "export-markdown":
        export_markdown()
    elif args.command == "dump-sql":
        dump_sql()
    elif args.command == "status":
        show_status()
    elif args.command == "query":
        query(args.sql)

if __name__ == "__main__":
    main()
