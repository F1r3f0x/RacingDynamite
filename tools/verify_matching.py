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

def verify_all(module_filter: str = None, symbol_filter: str = None, verbose: bool = False, check_all: bool = False):
    print("=======================================================================")
    print("            MAINDOS_32BIT.EXE MATCHING VERIFICATION                    ")
    print("=======================================================================")
    
    # 1. Compile decompiled code
    print("[1/2] Compiling authentic C sources with Watcom (wcc386)...")
    if not compile_all():
        print("[ERROR] Compilation failed. Aborting verification.", file=sys.stderr)
        return False

    status_scope = "ALL DECOMPILED" if check_all else "MATCHING"
    print(f"\n[2/2] Running byte/instruction diff against MAINDOS_32BIT.EXE ({status_scope})...")
    conn = get_connection()
    cur = conn.cursor()
    
    status_clause = "f.status IN ('matching', 'decompiled')" if check_all else "f.status = 'matching'"
    query = f"""
        SELECT f.symbol_name, f.dos_address, f.byte_size, f.status as func_status, m.name as module_name
        FROM functions f
        LEFT JOIN modules m ON f.module_id = m.id
        WHERE {status_clause} AND f.dos_address IS NOT NULL
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
    passed_funcs = 0

    col_w_sym = 32
    col_w_addr = 14
    col_w_mod = 14
    col_w_stat = 10
    col_w_pct = 10

    header = f"{'Symbol Name':<{col_w_sym}} {'DOS Addr':<{col_w_addr}} {'Module':<{col_w_mod}} {'Status':<{col_w_stat}} {'Match %':<{col_w_pct}}"
    print("-" * len(header))
    print(header)
    print("-" * len(header))

    for r in rows:
        sym = r["symbol_name"]
        dos_addr_str = r["dos_address"]
        dos_addr = int(dos_addr_str, 16)
        sz = r["byte_size"] or 0
        mod = r["module_name"] or "unknown"

        matched, pct, m, t = diff_func(sym, dos_addr, sz, verbose=verbose)
        stat_str = "PASS" if matched else "FAIL"
        if matched:
            passed_funcs += 1

        print(f"{sym:<{col_w_sym}} {dos_addr_str:<{col_w_addr}} {mod:<{col_w_mod}} {stat_str:<{col_w_stat}} {pct:>6.1f}% ({m}/{t})")
        results.append((sym, matched, pct))

    print("=" * len(header))
    summary_pct = (passed_funcs / total_funcs * 100.0) if total_funcs > 0 else 0.0
    print(f"Summary: {passed_funcs}/{total_funcs} functions matched 100% ({summary_pct:.1f}%)")
    
    if not check_all and passed_funcs < total_funcs:
        print(f"\n[FAIL] {total_funcs - passed_funcs} function(s) regressed or failed matching verification!", file=sys.stderr)
        return False

    if check_all:
        print(f"\n[INFO] Progress scan complete: {passed_funcs}/{total_funcs} functions at 100% bit-match.")
    else:
        print("\n[SUCCESS] All matching functions verified byte-for-byte!")
    return True

def main():
    parser = argparse.ArgumentParser(description="MAINDOS_32BIT Decompilation Batch Verification")
    parser.add_argument("-m", "--module", type=str, default=None, help="Filter by module (e.g. getsurf.c)")
    parser.add_argument("-s", "--symbol", type=str, default=None, help="Filter by symbol name")
    parser.add_argument("-v", "--verbose", action="store_true", help="Show full side-by-side assembly diffs")
    parser.add_argument("-a", "--all", action="store_true", help="Scan all decompiled functions to measure progress")
    args = parser.parse_args()

    success = verify_all(module_filter=args.module, symbol_filter=args.symbol, verbose=args.verbose, check_all=args.all)
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
