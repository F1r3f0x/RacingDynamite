import sqlite3
import sys

con = sqlite3.connect('database/decomp.db')
cur = con.cursor()
pattern = sys.argv[1] if len(sys.argv) > 1 else 'Menu'

rows = cur.execute(
    "SELECT dos_address, symbol_name, port_location, purpose FROM functions WHERE symbol_name LIKE ? OR purpose LIKE ?",
    (f'%{pattern}%', f'%{pattern}%')
).fetchall()

for r in rows:
    print(f"{r[0]}: {r[1]} -> {r[2]} ({r[3]})")
