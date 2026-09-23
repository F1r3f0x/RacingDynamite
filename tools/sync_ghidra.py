#!/usr/bin/env python3
"""
Racing Dynamite (Ignition 1997) - Ghidra Decompilation Progress Synchronizer
Syncs symbols, comments, and decompilation status from database/decomp.db
directly into the active Ghidra session via the Ghidra MCP HTTP server.
"""

import argparse
import os
import sqlite3
import sys
from pathlib import Path
from typing import Dict, List, Optional, Tuple

import requests

ROOT_DIR = Path(__file__).resolve().parent.parent
DB_PATH = ROOT_DIR / "database" / "decomp.db"
DEFAULT_GHIDRA_URL = "http://127.0.0.1:8080"


def normalize_address(addr: Optional[str]) -> Optional[str]:
    if not addr or addr.strip() == "-" or addr.strip() == "":
        return None
    addr = addr.strip().lower()
    if addr.startswith("0x"):
        addr = addr[2:]
    return addr.zfill(8)


def detect_target_binary(server_url: str) -> str:
    """
    Detect whether the active binary in Ghidra is IGN_WIN.EXE or MAINDOS.EXE
    based on loaded memory segments.
    """
    try:
        resp = requests.get(f"{server_url}/segments", timeout=5)
        if resp.ok:
            text = resp.text
            if "00401000" in text or ".text" in text:
                return "win"
            if "00010000" in text:
                return "dos"
    except Exception:
        pass
    return "win"


def sync_ghidra(
    server_url: str = DEFAULT_GHIDRA_URL,
    target: str = "auto",
    sync_functions: bool = True,
    sync_globals: bool = True,
    sync_comments: bool = True,
    verbose: bool = False,
) -> bool:
    if not DB_PATH.exists():
        print(f"[ERROR] Database not found at {DB_PATH}", file=sys.stderr)
        return False

    print(f"[*] Checking Ghidra connection at {server_url}...")
    try:
        test_resp = requests.get(f"{server_url}/methods?limit=1", timeout=5)
        if not test_resp.ok:
            print(f"[ERROR] Ghidra server returned status {test_resp.status_code}", file=sys.stderr)
            return False
    except requests.exceptions.RequestException as e:
        print(f"[ERROR] Could not connect to Ghidra server at {server_url}: {e}", file=sys.stderr)
        print("    Ensure Ghidra is running with the GhidraMCP plugin active on port 8080.")
        return False

    detected = detect_target_binary(server_url)
    active_target = detected if target == "auto" else target
    target_desc = "IGN_WIN.EXE (Windows PE)" if active_target == "win" else "MAINDOS.EXE (Watcom DOS/4GW)"
    print(f"[*] Target executable: {target_desc} (selection: {target})")

    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    cur = conn.cursor()

    # 1. Sync Functions
    funcs_renamed = 0
    funcs_failed = 0
    funcs_skipped = 0
    comments_set = 0

    if sync_functions:
        print("\n[*] Synchronizing functions from database/decomp.db...")
        cur.execute("""
            SELECT f.id, f.dos_address, f.win_address, f.symbol_name, f.original_ghidra_name,
                   f.status, f.fidelity, f.purpose, COALESCE(m.name, 'unknown') as module_name
            FROM functions f
            LEFT JOIN modules m ON f.module_id = m.id
            ORDER BY f.win_address ASC, f.dos_address ASC
        """)
        functions = cur.fetchall()

        for fn in functions:
            raw_addr = fn["win_address"] if active_target == "win" else fn["dos_address"]
            addr = normalize_address(raw_addr)
            if not addr:
                funcs_skipped += 1
                if verbose:
                    print(f"  [-] Skipped {fn['symbol_name']} (no address for target {active_target})")
                continue

            sym_name = fn["symbol_name"]
            status = fn["status"] or "analyzed"
            fidelity = fn["fidelity"] or "ADAPTED"
            module = fn["module_name"]
            purpose = fn["purpose"] or ""

            # Attempt rename
            try:
                rename_resp = requests.post(
                    f"{server_url}/rename_function_by_address",
                    data={"function_address": addr, "new_name": sym_name},
                    timeout=5,
                )
                if rename_resp.ok and "success" in rename_resp.text.lower():
                    funcs_renamed += 1
                    if verbose:
                        print(f"  [+] Renamed 0x{addr} -> {sym_name}")
                else:
                    funcs_failed += 1
                    if verbose:
                        print(f"  [!] Failed rename at 0x{addr} ({sym_name}): {rename_resp.text.strip()}")
            except Exception as e:
                funcs_failed += 1
                if verbose:
                    print(f"  [!] Error renaming 0x{addr} ({sym_name}): {e}")

            # Set comments if requested
            if sync_comments:
                comment_text = f"[@{status.lower()} {fidelity} | module: {module}]"
                if purpose:
                    comment_text += f" {purpose}"

                try:
                    # Disassembly comment
                    requests.post(
                        f"{server_url}/set_disassembly_comment",
                        data={"address": addr, "comment": comment_text},
                        timeout=5,
                    )
                    # Decompiler pseudocode comment
                    requests.post(
                        f"{server_url}/set_decompiler_comment",
                        data={"address": addr, "comment": comment_text},
                        timeout=5,
                    )
                    comments_set += 1
                except Exception:
                    pass

        print(f"    - Functions Renamed: {funcs_renamed}")
        if funcs_failed > 0:
            print(f"    - Functions Not Found / Failed: {funcs_failed}")
        if funcs_skipped > 0:
            print(f"    - Functions Skipped (no {active_target} address): {funcs_skipped}")
        print(f"    - Header Comments Added: {comments_set}")

    # 2. Sync Globals
    globs_renamed = 0
    globs_failed = 0
    globs_skipped = 0

    if sync_globals:
        print("\n[*] Synchronizing globals from database/decomp.db...")
        cur.execute("""
            SELECT win_address, name, type, description
            FROM globals
            ORDER BY win_address ASC
        """)
        globals_list = cur.fetchall()

        for g in globals_list:
            raw_addr = g["win_address"]
            addr = normalize_address(raw_addr)
            if not addr:
                globs_skipped += 1
                continue

            name = g["name"]
            try:
                resp = requests.post(
                    f"{server_url}/renameData",
                    data={"address": addr, "newName": name},
                    timeout=5,
                )
                if resp.ok and ("attempted" in resp.text.lower() or "success" in resp.text.lower()):
                    globs_renamed += 1
                    if verbose:
                        print(f"  [+] Renamed global 0x{addr} -> {name}")
                else:
                    globs_failed += 1
                    if verbose:
                        print(f"  [!] Failed rename global 0x{addr} ({name}): {resp.text.strip()}")
            except Exception as e:
                globs_failed += 1
                if verbose:
                    print(f"  [!] Error renaming global 0x{addr} ({name}): {e}")

        print(f"    - Globals Renamed: {globs_renamed}")
        if globs_failed > 0:
            print(f"    - Globals Failed: {globs_failed}")
        if globs_skipped > 0:
            print(f"    - Globals Skipped: {globs_skipped}")

    conn.close()

    print("\n" + "=" * 60)
    print("      GHIDRA SYNCHRONIZATION COMPLETE")
    print("=" * 60)
    print(f"Functions synced : {funcs_renamed}")
    print(f"Globals synced   : {globs_renamed}")
    print(f"Comments set     : {comments_set}")
    print("Ghidra Symbol Tree, Listing, and Decompiler views are now updated.")
    print("=" * 60)
    return True


def main():
    parser = argparse.ArgumentParser(description="Synchronize decomp.db symbols into active Ghidra session")
    parser.add_argument("--url", default=DEFAULT_GHIDRA_URL, help=f"Ghidra MCP HTTP URL (default: {DEFAULT_GHIDRA_URL})")
    parser.add_argument("--target", choices=["auto", "win", "dos"], default="auto", help="Target binary (default: auto)")
    parser.add_argument("--no-functions", action="store_true", help="Skip function renames")
    parser.add_argument("--no-globals", action="store_true", help="Skip global variable renames")
    parser.add_argument("--no-comments", action="store_true", help="Skip decompiler/disassembly comments")
    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose per-symbol logging")

    args = parser.parse_args()
    success = sync_ghidra(
        server_url=args.url,
        target=args.target,
        sync_functions=not args.no_functions,
        sync_globals=not args.no_globals,
        sync_comments=not args.no_comments,
        verbose=args.verbose,
    )
    if not success:
        sys.exit(1)


if __name__ == "__main__":
    main()
