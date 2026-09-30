#!/usr/bin/env python3
"""
tools/verify_matching.py
Automated Batch Decompilation Matching & Verification Tool for Racing Dynamite.
Verifies compiled Watcom C object files against MAINDOS_32BIT.EXE.
Used in CI and local workflows to prevent regressions.
"""

import argparse
import sys
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT_DIR / "tools"))

from build_decomp import compile_all
from db import get_connection
from diff_func import diff_func

def verify_all(module_filter: str = None, symbol_filter: str = None, verbose: bool = False, check_all: bool = True, strict_matching: bool = False):
    print("=======================================================================")
    print("            MAINDOS_32BIT.EXE MATCHING VERIFICATION                    ")
    print("=======================================================================")
    
    # 0. Lint check for inline assembly in .c files
    print("[0/2] Linting C sources for pure C compliance...")
    decomp_src_dir = ROOT_DIR / "decomp" / "src"
    for c_file in decomp_src_dir.glob("*.c"):
        with open(c_file, "r", encoding="utf-8") as f:
            content = f.read()
            if "__asm" in content:
                print(f"[ERROR] Inline assembly (__asm) found in {c_file.name}!", file=sys.stderr)
                print("[ERROR] Project rule violation: Pure C Only. Move handwritten asm to separate .asm files.", file=sys.stderr)
                return False

    # 1. Compile decompiled code
    print("[1/2] Compiling authentic C sources with Watcom (wcc386)...")
    if not compile_all():
        print("[WARNING] Compilation had errors. Proceeding anyway...", file=sys.stderr)

    import subprocess
    import os
    print("[1.5/2] Linking object files using Watcom wlink to MAINDOS_REBUILT.EXE...")
    env = os.environ.copy()
    watcom_dir = ROOT_DIR / "tools" / "WATCOM"
    env["WATCOM"] = str(watcom_dir)
    env["PATH"] = str(watcom_dir / "BINNT") + ";" + str(watcom_dir / "BINW") + ";" + env.get("PATH", "")
    wlink_exe = str(watcom_dir / "BINNT" / "wlink.exe")
    log_path = ROOT_DIR / "build" / "decomp" / "wlink.log"
    try:
        with open(log_path, "w", encoding="utf-8") as log_f:
            subprocess.run([wlink_exe, "@build/decomp/wlink.lnk"], env=env, stdin=subprocess.DEVNULL, stdout=log_f, stderr=subprocess.STDOUT, cwd=str(ROOT_DIR))
    except Exception:
        pass # Expected due to missing symbols but we have undefsok

    status_scope = "PROJECT-WIDE (ALL DECOMPILED)" if check_all else "MATCHING ONLY"
    print(f"\n[2/2] Running byte/instruction diff against MAINDOS_32BIT.EXE ({status_scope})...")
    conn = get_connection()
    cur = conn.cursor()
    
    status_clause = "f.status IN ('matching', 'decompiled')" if check_all else "f.status = 'matching'"
    query = f"""
        SELECT f.symbol_name, f.dos_address, f.win_address, f.byte_size, f.status as func_status, m.name as module_name
        FROM functions f
        LEFT JOIN modules m ON f.module_id = m.id
        WHERE {status_clause}
    """
    params = []
    if module_filter:
        query += " AND m.name LIKE ?"
        params.append(f"%{module_filter}%")
    if symbol_filter:
        query += " AND f.symbol_name LIKE ?"
        params.append(f"%{symbol_filter}%")
        
    query += " ORDER BY m.name, f.dos_address"
    cur.execute(query, params)
    rows = cur.fetchall()
    conn.close()

    if not rows:
        print("No matching functions found matching criteria.")
        return True

    results = []
    total_funcs = len(rows)
    diffed_funcs = 0
    passed_funcs = 0
    compiled_funcs = 0

    col_w_sym = 36
    col_w_addr = 14
    col_w_mod = 14
    col_w_stat = 10
    col_w_pct = 12

    header = f"{'Symbol Name':<{col_w_sym}} {'DOS Addr':<{col_w_addr}} {'Module':<{col_w_mod}} {'Status':<{col_w_stat}} {'Match %':<{col_w_pct}}"
    print("-" * len(header))
    print(header)
    print("-" * len(header))

    from diff_func import get_rebuilt_func

    for r in rows:
        sym = r["symbol_name"]
        dos_addr_str = r["dos_address"]
        sz = r["byte_size"] or 0
        mod = r["module_name"] or "unknown"

        rebuilt_code, _ = get_rebuilt_func(sym, sz)
        is_compiled = rebuilt_code is not None
        if is_compiled:
            compiled_funcs += 1

        if dos_addr_str:
            diffed_funcs += 1
            dos_addr = int(dos_addr_str, 16)
            matched, pct, m, t = diff_func(sym, dos_addr, sz, verbose=verbose)
            stat_str = "PASS" if matched else "DIFF"
            if matched:
                passed_funcs += 1
            match_str = f"{pct:>5.1f}% ({m}/{t})"
            addr_display = dos_addr_str
        else:
            stat_str = "COMPILED" if is_compiled else "MISSING"
            match_str = f"({len(rebuilt_code)}b)" if is_compiled else "--"
            addr_display = "-"

        print(f"{sym:<{col_w_sym}} {addr_display:<{col_w_addr}} {mod:<{col_w_mod}} {stat_str:<{col_w_stat}} {match_str:>{col_w_pct}}")
        results.append((sym, stat_str, match_str))

    print("=" * len(header))
    summary_pct = (passed_funcs / diffed_funcs * 100.0) if diffed_funcs > 0 else 0.0
    print(f"Summary: {passed_funcs}/{diffed_funcs} diffable functions bit-matched 100% ({summary_pct:.1f}%). Total compiled: {compiled_funcs}/{total_funcs}.")
    
    if strict_matching and passed_funcs < diffed_funcs:
        print(f"\n[FAIL] {diffed_funcs - passed_funcs} function(s) regressed or failed matching verification!", file=sys.stderr)
        return False

    print(f"\n[INFO] Project verification scan complete.")
    return True

def main():
    parser = argparse.ArgumentParser(description="MAINDOS_32BIT Decompilation Batch Verification")
    parser.add_argument("-m", "--module", type=str, default=None, help="Filter by module (e.g. getsurf.c)")
    parser.add_argument("-s", "--symbol", type=str, default=None, help="Filter by symbol name")
    parser.add_argument("-v", "--verbose", action="store_true", help="Show full side-by-side assembly diffs")
    parser.add_argument("--matching-only", action="store_true", help="Only scan functions marked as 100 percent matching")
    parser.add_argument("--strict", action="store_true", help="Fail with exit code 1 if any function does not bit-match 100 percent")
    args = parser.parse_args()

    check_all = not args.matching_only
    success = verify_all(module_filter=args.module, symbol_filter=args.symbol, verbose=args.verbose, check_all=check_all, strict_matching=args.strict)
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
