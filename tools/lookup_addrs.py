
# MIGRATION REQUIRED: retained for Windows adaptation; do not use yet.
if __package__:
    from .tool_migration import require_migration
else:
    from tool_migration import require_migration
require_migration(__file__)

import sqlite3
import sys

con = sqlite3.connect('database/decomp.db')
cur = con.cursor()

if len(sys.argv) > 1:
    addrs = sys.argv[1:]
    for a in addrs:
        val = int(a, 16)
        # Try both formats: '0x00012345' and '0x12345'
        fmt1 = f"0x{val:08x}"
        fmt2 = f"0x{val:x}"
        rows = cur.execute(
            "SELECT dos_address, symbol_name, purpose FROM functions WHERE dos_address IN (?, ?)",
            (fmt1, fmt2)
        ).fetchall()
        if rows:
            for r in rows:
                print(f"{r[0]}: {r[1]} -> {r[2]}")
        else:
            print(f"{a}: NOT IN DB")
