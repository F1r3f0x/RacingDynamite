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
    cur.execute("INSERT OR REPLACE INTO metadata (key, value) VALUES (?, ?)", ("reference_exe", "MAINDOS_32BIT.EXE"))
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
        headers = {}
        for line in lines:
            line = line.strip()
            if not line.startswith("|"):
                continue
            parts = [clean_val(p) for p in line.split("|")[1:-1]]
            if len(parts) < 6:
                continue
            if not headers:
                # Detect header row
                norm_parts = [p.lower() for p in parts]
                if any("addr" in p or "symbol" in p for p in norm_parts):
                    for idx, p in enumerate(norm_parts):
                        headers[p] = idx
                    continue
            if parts[0] == ":---" or parts[0].startswith("---"):
                continue

            # Resolve column values using header map
            def get_col(candidates, default=""):
                for c in candidates:
                    for h, idx in headers.items():
                        if c in h and idx < len(parts):
                            return parts[idx]
                return default

            dos_addr = get_col(["dos addr", "dos"], "")
            win_addr = get_col(["win addr", "win", "address"], "")
            orig_ghidra = get_col(["ghidra label", "ghidra", "label"], "")
            sym_name = get_col(["symbol name", "symbol", "name"], "")
            raw_module = get_col(["module"], "")
            status = get_col(["status"], "unidentified").lower()
            fidelity = get_col(["fidelity"], "-")
            port_loc = get_col(["port location", "location", "port"], "-")
            purpose = get_col(["purpose", "notes", "description"], "")

            if not win_addr or win_addr == "-":
                if not dos_addr or dos_addr == "-":
                    continue

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
            
            if fidelity not in ("EXACT", "ADAPTED", "EXTENDED", "INFRASTRUCTURE"):
                fidelity = "-"

            cur.execute("""
                INSERT OR REPLACE INTO functions 
                (dos_address, win_address, symbol_name, original_ghidra_name, module_id, status, fidelity, port_location, purpose)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
            """, (dos_addr if dos_addr != "-" else None, 
                  win_addr if win_addr != "-" else None, 
                  sym_name, orig_ghidra, mod_id, status, fidelity, port_loc, purpose))
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
        
        table_matches = re.findall(
            r"\|\s*\*\*`?(DEV-\d+)`?\*\*\s*\|\s*`?([A-Z0-9_]+)`?\s*\|\s*`?(0x[0-9a-fA-F]+)`?\s*\|\s*([^|]+)\|\s*([^|]+)\s*\|",
            content
        )
        imported_devs = 0
        for dev_id, category, addr, desc, toggle in table_matches:
            cur.execute("""
                INSERT OR REPLACE INTO deviations (id, category, title, description, win_address, toggle_key)
                VALUES (?, ?, ?, ?, ?, ?)
            """, (dev_id.strip(), category.strip(), desc.strip(), desc.strip(), addr.strip(), toggle.strip().strip("`")))
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
    print("--------------------------------------------------")
    cur.execute("""
        SELECT symbol_name, dos_address, notes
        FROM functions
        WHERE notes LIKE '%ASM fallback%'
        ORDER BY symbol_name
    """)
    asm_rows = cur.fetchall()
    if asm_rows:
        print("Active ASM Fallbacks (Pending C matching):")
        for r in asm_rows:
            print(f"  * {r['symbol_name']:<30} {r['dos_address']:<12} - {r['notes']}")
    else:
        print("Active ASM Fallbacks: None")
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
        f.write("# Master Function Registry (MAINDOS_32BIT.EXE)\n\n")
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

def link_func(win_addr: str, dos_addr: str):
    conn = get_connection()
    cur = conn.cursor()
    cur.execute("UPDATE functions SET dos_address = ? WHERE win_address = ?", (dos_addr, win_addr))
    if cur.rowcount > 0:
        print(f"Linked Win {win_addr} -> DOS {dos_addr} (updated {cur.rowcount} row)")
        conn.commit()
    else:
        print(f"Warning: No function found with win_address '{win_addr}'", file=sys.stderr)
    conn.close()

def set_status(addr: str, status: str):
    conn = get_connection()
    cur = conn.cursor()
    cur.execute("""
        UPDATE functions 
        SET status = ? 
        WHERE dos_address = ? OR win_address = ?
    """, (status, addr, addr))
    if cur.rowcount > 0:
        print(f"Updated status of {addr} -> {status}")
        conn.commit()
    else:
        print(f"Warning: No function found matching address '{addr}'", file=sys.stderr)
    conn.close()

def add_func(dos_addr: str, name: str, module_name: str, purpose: str = ""):
    conn = get_connection()
    cur = conn.cursor()
    cur.execute("SELECT id FROM modules WHERE name = ?", (module_name,))
    row = cur.fetchone()
    mod_id = row[0] if row else None
    if not mod_id and module_name:
        cur.execute("INSERT INTO modules (name) VALUES (?)", (module_name,))
        mod_id = cur.lastrowid
    
    cur.execute("""
        INSERT OR REPLACE INTO functions (dos_address, symbol_name, module_id, status, purpose)
        VALUES (?, ?, ?, 'analyzed', ?)
    """, (dos_addr, name, mod_id, purpose))
    conn.commit()
    print(f"Added function {name} at DOS {dos_addr} in {module_name}")
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

    link_p = subparsers.add_parser("link", help="Link Windows address to DOS address")
    link_p.add_argument("win_addr", type=str, help="Address in MAINDOS_32BIT.EXE (e.g. 0x00412fc0)")
    link_p.add_argument("dos_addr", type=str, help="Address in MAINDOS.EXE (e.g. 0x00012340)")

    stat_p = subparsers.add_parser("set-status", help="Update function status")
    stat_p.add_argument("addr", type=str, help="Function address (DOS or Win)")
    stat_p.add_argument("status", choices=["unidentified", "analyzed", "decompiled", "matching"])

    add_p = subparsers.add_parser("add-func", help="Add newly identified DOS function")
    add_p.add_argument("dos_addr", type=str, help="DOS address (e.g. 0x00010a20)")
    add_p.add_argument("name", type=str, help="Function symbol name")
    add_p.add_argument("module", type=str, help="Module name (e.g. getsurf.c)")
    add_p.add_argument("--purpose", type=str, default="", help="Function purpose")
    
    subparsers.add_parser("dashboard", help="Generate interactive HTML decompilation progress dashboard")

    ghidra_p = subparsers.add_parser("sync-ghidra", help="Sync decomp.db symbols and comments into active Ghidra session")
    ghidra_p.add_argument("--url", default="http://127.0.0.1:8080", help="Ghidra MCP HTTP URL")
    ghidra_p.add_argument("--target", choices=["auto", "win", "dos"], default="auto", help="Target binary (default: auto)")
    ghidra_p.add_argument("--no-functions", action="store_true", help="Skip function renames")
    ghidra_p.add_argument("--no-globals", action="store_true", help="Skip global variable renames")
    ghidra_p.add_argument("--no-comments", action="store_true", help="Skip comments")
    ghidra_p.add_argument("-v", "--verbose", action="store_true", help="Verbose output")

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
    elif args.command == "dashboard":
        from generate_dashboard import main as gen_main
        gen_main(argv=[])
    elif args.command == "sync-ghidra":
        from sync_ghidra import sync_ghidra
        sync_ghidra(
            server_url=args.url,
            target=args.target,
            sync_functions=not args.no_functions,
            sync_globals=not args.no_globals,
            sync_comments=not args.no_comments,
            verbose=args.verbose,
        )
    elif args.command == "query":
        query(args.sql)
    elif args.command == "link":
        link_func(args.win_addr, args.dos_addr)
    elif args.command == "set-status":
        set_status(args.addr, args.status)
    elif args.command == "add-func":
        add_func(args.dos_addr, args.name, args.module, args.purpose)

if __name__ == "__main__":
    main()

