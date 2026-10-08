#!/usr/bin/env python3
"""
Batch classify identified MSVC CRT functions in database/decomp.db.
Extracts verified CRT function symbols from Ghidra in RVA range 0x69100..0x78A40
and updates functions table with classification='crt', ABI='cdecl', and evidence link.
"""
import argparse
import sqlite3
import sys
from pathlib import Path
import requests

ROOT_DIR = Path(__file__).resolve().parent.parent
DB_PATH = ROOT_DIR / "database" / "decomp.db"
EVIDENCE_PATH = "docs/ghidra/windows_crt.md"
GHIDRA_URL = "http://localhost:8080"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dry-run", action="store_true", help="Print updates without modifying database")
    args = parser.parse_args()

    if not (ROOT_DIR / EVIDENCE_PATH).is_file():
        print(f"[FAIL] Evidence document not found: {EVIDENCE_PATH}", file=sys.stderr)
        return 1

    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    c = conn.cursor()

    # Query all candidates in the contiguous CRT range
    rows = c.execute(
        "SELECT id, rva, symbol_name, classification, analysis_stage FROM functions WHERE rva BETWEEN 0x69100 AND 0x78A50 ORDER BY rva"
    ).fetchall()

    print(f"[*] Found {len(rows)} candidates in CRT range 0x69100..0x78A50")

    updates = []
    for row in rows:
        rva = row["rva"]
        va = 0x00400000 + rva
        addr_str = f"{va:08X}"

        try:
            resp = requests.get(f"{GHIDRA_URL}/get_function_by_address?address={addr_str}", timeout=2)
            if not resp.ok or not resp.text.startswith("Function: "):
                continue

            first_line = resp.text.splitlines()[0]
            gname = first_line.split(" ")[1]

            # Clean name
            if gname.startswith("FID_conflict:"):
                gname = gname.replace("FID_conflict:", "")

            # Exclude unanalyzed/generic function names
            if gname.startswith("FUN_") or gname == "NO_FUNC":
                continue

            updates.append((row["id"], rva, row["symbol_name"], gname))
        except requests.exceptions.RequestException as e:
            print(f"[WARN] Failed to query Ghidra for RVA 0x{rva:05X}: {e}", file=sys.stderr)

    print(f"[*] Identified {len(updates)} confirmed CRT functions to classify.")

    if args.dry_run:
        print("[*] Dry run mode; no changes committed.")
        for fn_id, rva, old_name, new_name in updates[:20]:
            print(f"  RVA 0x{rva:05X}: {old_name} -> {new_name}")
        return 0

    with conn:
        for fn_id, rva, old_name, new_name in updates:
            conn.execute(
                """
                UPDATE functions
                SET symbol_name = ?,
                    classification = 'crt',
                    abi = 'cdecl',
                    analysis_stage = CASE WHEN analysis_stage = 'unidentified' THEN 'named' ELSE analysis_stage END,
                    evidence_path = ?,
                    notes = 'Statically linked MSVC CRT'
                WHERE id = ?
                """,
                (new_name, EVIDENCE_PATH, fn_id),
            )

    print(f"[PASS] Successfully updated {len(updates)} CRT functions in {DB_PATH.name}.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
