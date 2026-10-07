#!/usr/bin/env python3
"""Refresh a conservative implementation evidence inventory and its DB snapshot."""
import json
import re
import sqlite3
from collections import Counter
from pathlib import Path
from source_audit import source_inventory, masked
from le_parser import LEFile

ROOT = Path(__file__).resolve().parents[1]

def main():
    files = source_inventory(ROOT)
    conn = sqlite3.connect(ROOT/'database/decomp.db')
    conn.row_factory = sqlite3.Row
    functions = {r['symbol_name']: dict(r) for r in conn.execute('SELECT * FROM functions')}
    globals_db = {r['name']: dict(r) for r in conn.execute('SELECT * FROM globals')}
    funcs = [(p, f) for p, (_, fs) in files.items() if p.startswith('decomp/') for f in fs]
    names = {f['symbol'] for _, f in funcs}
    le = LEFile(str(ROOT/'Ignition/Ignition/MAINDOS.EXE'))
    lines = ['# DOS implementation evidence inventory', '',
             'Generated with `uv run python tools/inventory_gaps.py`. Candidate detection is conservative; inspect authentic instructions before classifying short routines as genuine stubs. Source callers are lexical references inside function bodies, not a binary call graph.', '',
             'Historical database `decompiled`/`matching` statuses describe reconstruction progress. The `implementation_audits` table separately records source evidence; neither status nor EXACT annotations prove behavior. Runtime verification is recorded in `docs/tracking/runtime_baseline.md`.', '',
             f'Database: {len(functions)} functions, {len(globals_db)} globals, {conn.execute("SELECT COUNT(*) FROM structs").fetchone()[0]} structures. DOS source definitions: {len(funcs)} ({len(names)} unique symbols).', '',
             '## Constant-return and empty candidates', '',
             '| Symbol | Source | DOS address | Source callers | Verification needed |',
             '| --- | --- | --- | --- | --- |']
    conn.execute('''CREATE TABLE IF NOT EXISTS implementation_audits (
        symbol_name TEXT NOT NULL, source_path TEXT NOT NULL, source_line INTEGER NOT NULL,
        implementation_state TEXT NOT NULL, runtime_verification TEXT NOT NULL,
        evidence TEXT NOT NULL, PRIMARY KEY(symbol_name, source_path))''')
    conn.execute('DELETE FROM implementation_audits')
    for path, f in funcs:
        state = 'stub_candidate' if f['stub_candidate'] else 'implemented_unverified'
        conn.execute('INSERT INTO implementation_audits VALUES (?, ?, ?, ?, ?, ?)',
                     (f['symbol'], path, f['line'], state, 'unverified', 'Lexical body inventory; authentic semantics not certified'))
        if f['stub_candidate']:
            db = functions.get(f['symbol'], {})
            callers = [f'{other["symbol"]} ({p}:{other["line"]})' for p, other in funcs if re.search(r'\b'+re.escape(f['symbol'])+r'\s*\(', other['body'])]
            lines.append(f'| {f["symbol"]} | {path}:{f["line"]} | {db.get("dos_address") or "unknown"} | {"; ".join(callers) or "No direct DOS source caller found"} | Recover ABI, side effects, return semantics, and runtime coverage |')
    lines += ['', '## Initializer/type/size candidates', '',
              'Zero initialization may be authentic BSS. The table flags uncertainty, not confirmed defects. Binary samples use LE relocation-aware object data; sizes and pointer meanings still require authentic users and layout recovery.', '',
              '| Symbol | Source declaration | Tracked DOS address/type/size | Authentic bytes (up to 16) | DOS source users | Verification needed |',
              '| --- | --- | --- | --- | --- | --- |']
    suspected = []
    for path, (text, _) in files.items():
        if path != 'decomp/src/globals.c':
            continue
        # Globals.c holds placeholder types/sizes; include all initialized declarations there.
        for m in re.finditer(r'(?m)^([^\n;{}]+?\b(\w+)\s*(?:\[[^\n]*?\])?\s*=\s*[^;]*);', text):
            name, decl = m[2], ' '.join(m[1].split())
            if name not in globals_db and not name.startswith(('g_', 's_', 'DAT_', 'str_', 'a_', 'd_')):
                continue
            row = globals_db.get(name, {})
            addr = row.get('dos_address')
            sample = 'unknown address'
            if addr:
                vaddr = int(addr, 16)
                for obj in le.objects:
                    off = vaddr - obj['reloc_base']
                    if 0 <= off < obj['vsize']:
                        sample = bytes(obj['data'][off:off+min(row.get('size') or 16, 16)]).hex(' ') if off < len(obj['data']) else 'BSS / no stored page'
                        break
            line = text.count('\n', 0, m.start())+1
            suspected.append(name)
            users = [f'{f["symbol"]} ({p}:{f["line"]})' for p, f in funcs if re.search(r'\b'+re.escape(name)+r'\b', f['body'])]
            user_text = '; '.join(users[:10]) or 'No direct DOS source user found'
            if len(users) > 10:
                user_text += f'; plus {len(users)-10} additional source users'
            lines.append(f'| {name} | {path}:{line}: `{decl}` | {addr or "unknown"} / {row.get("type") or "untracked"} / {row.get("size") or "unknown"} | `{sample}` | {user_text} | Recover full initializer, references, actual C type and extent |')
    lines += ['', '## Address/name gaps', '', '| Symbol | DOS address | Source locations | Verification needed |', '| --- | --- | --- | --- |']
    for name, row in functions.items():
        if not row['dos_address'] or name.startswith(('FUN_', 'Unknown_', 'LAB_')):
            locations = [f'{p}:{f["line"]}' for p, f in funcs if f['symbol'] == name]
            lines.append(f'| {name} | {row["dos_address"] or "unknown"} | {"; ".join(locations) or "no DOS definition"} | Recover DOS address and semantic name from authentic callers/strings; legacy Windows labels are only cross-references |')
    lines += ['', '## Scope reconciliation', '', f'Database status counts: {dict(Counter(r["status"] for r in functions.values()))}.', '',
              f'Database symbols absent from DOS definitions: {sorted(functions.keys() - names)}.', '',
              f'DOS source symbols absent from function database: {sorted(names - functions.keys())}.', '',
              f'The generated function registry has {len(functions)} rows, including {sum(not r["dos_address"] for r in functions.values())} rows without DOS addresses. The previous 180 count came from discarding addressless rows. Headers, static helpers, infrastructure, and stubs explain why source definitions and the database have different scopes. `__cstart` is supplied by the Watcom library, not a decomp/src definition.', '',
              'Ghidra localhost:8080 refused connections during this assessment. No synchronization claimed. Authentic LE analysis remains available.', '',
              f'Initializer/type/extent candidates in globals.c: {len(suspected)}. All require review; zero-filled declarations alone do not establish a bug.', '']
    (ROOT/'docs/tracking/implementation_inventory.md').write_text('\n'.join(lines), encoding='utf-8')
    conn.commit()
    conn.close()
    print('Inventory refreshed:', len(funcs), 'DOS definitions;', sum(f['stub_candidate'] for _, f in funcs), 'short/empty candidates;', len(suspected), 'initializer/type candidates')

if __name__ == '__main__':
    main()
