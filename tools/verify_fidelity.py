#!/usr/bin/env python3
"""Audit tracking/annotation consistency. Never certifies runtime fidelity."""
import re
import sqlite3
import subprocess
import sys
from collections import Counter
from pathlib import Path
from source_audit import source_inventory, annotations

ROOT_DIR = Path(__file__).resolve().parent.parent
VALID_FIDELITY = {'EXACT', 'ADAPTED', 'EXTENDED', 'INFRASTRUCTURE'}

def parse_registry(path):
    """Read current generated and legacy registries by column name."""
    headers, rows = [], []
    for line in path.read_text(encoding='utf-8').splitlines():
        if not line.startswith('|'):
            continue
        parts = [p.strip().strip('`') for p in line.split('|')[1:-1]]
        if 'DOS Addr' in parts or 'Address' in parts:
            headers = parts
        elif headers and len(parts) == len(headers) and not all(p.startswith(':') or p.startswith('-') for p in parts):
            row = dict(zip(headers, parts))
            if row.get('Symbol Name', row.get('Symbol', '')):
                rows.append(row)
    return rows

def audit(root):
    errors, warnings = [], []
    registry = parse_registry(root / 'docs/ghidra/functions.md')
    files = source_inventory(root)
    definitions, tags = {}, {}
    for path, (text, funcs) in files.items():
        file_tags = annotations(text)
        tags[path] = file_tags
        for fn in funcs:
            definitions.setdefault(fn['symbol'], []).append((path, fn))
            tag = file_tags.get(fn['symbol'])
            if not tag:
                errors.append(f"{path}:{fn['line']} {fn['symbol']}: missing @original and @fidelity")
            elif tag[1] not in VALID_FIDELITY:
                errors.append(f"{path}:{fn['line']} {fn['symbol']}: invalid/missing @fidelity {tag[1]}")
            elif not tag[0] and tag[1] != 'INFRASTRUCTURE':
                errors.append(f"{path}:{fn['line']} {fn['symbol']}: missing authentic DOS address")
    db_path = root / 'database/decomp.db'
    conn = sqlite3.connect(f'{db_path.as_uri()}?mode=ro', uri=True)
    conn.row_factory = sqlite3.Row
    database = list(conn.execute('SELECT * FROM functions'))
    conn.close()
    docs = {r.get('Symbol Name', r.get('Symbol')): r for r in registry}
    db_names = {r['symbol_name'] for r in database}
    if len(docs) != len(registry):
        errors.append('Duplicate function symbols in registry')
    if len(db_names) != len(database):
        errors.append('Duplicate function symbols in database')
    for name in sorted(db_names - docs.keys()):
        errors.append(f'Database function {name} missing from function registry')
    for name in sorted(docs.keys() - db_names):
        errors.append(f'Registry function {name} missing from database')
    for row in database:
        name = row['symbol_name']
        doc = docs.get(name)
        if doc:
            for column, key in [('Status', 'status'), ('Fidelity', 'fidelity'), ('DOS Addr', 'dos_address')]:
                left, right = doc.get(column, '-'), row[key] or '-'
                if column == 'DOS Addr' and left != '-' and right != '-':
                    same = int(left, 16) == int(right, 16)
                else:
                    same = left.lower() == right.lower()
                if not same:
                    errors.append(f'{name}: registry/database disagree on {column}: {left} / {right}')
        if row['status'] in ('decompiled', 'matching', 'ported'):
            locations = [(p, f) for p, f in definitions.get(name, []) if p.startswith('decomp/')]
            if not locations:
                if name == '__cstart':
                    warnings.append('__cstart: Watcom library implementation; source annotations outside audit scope')
                else:
                    errors.append(f'{name}: tracked implementation missing from DOS source')
            for path, fn in locations:
                if fn['stub_candidate']:
                    warnings.append(f'{path}:{fn["line"]} {name}: constant-return/empty candidate; behavior UNVERIFIED')
                tag = tags[path].get(name)
                if tag and tag[0] and row['dos_address'] and int(tag[0],16) != int(row['dos_address'],16):
                    errors.append(f'{name}: annotation/database DOS address mismatch')
            if not row['dos_address']:
                warnings.append(f'{name}: tracked implementation has no authentic DOS address')
    deviations = (root / 'docs/tracking/deviations.md').read_text(encoding='utf-8')
    known = set(re.findall(r'\bDEV-\d+\b', deviations))
    used = set()
    for path, (text, _) in files.items():
        for dev in re.findall(r'@deviation\s+(DEV-\d+)', text):
            used.add(dev)
            if dev not in known:
                errors.append(f'{path}: unknown deviation {dev}')
    valid_categories = {'FIX_CAT_NOCLIP', 'FIX_CAT_ELEVATION', 'FIX_CAT_CAMERA',
                        'FIX_CAT_AI_PATHING', 'FIX_CAT_AUDIO', 'FIX_CAT_RENDERER'}
    for line in deviations.splitlines():
        if not line.startswith('|') or not re.search(r'DEV-\d+', line):
            continue
        cells = [cell.strip().strip('*`') for cell in line.split('|')[1:-1]]
        if len(cells) >= 5:
            if cells[1] not in valid_categories:
                errors.append(f'{cells[0]}: invalid deviation category {cells[1]}')
            if cells[4] in ('', '-'):
                errors.append(f'{cells[0]}: missing preservation toggle')
    for dev in sorted(known - used):
        warnings.append(f'{dev}: registry entry without source @deviation')
    metrics = {'registry_functions': len(registry), 'database_functions': len(database),
               'source_files': len(files), 'source_definitions': sum(len(f) for _, f in files.values()),
               'database_statuses': dict(Counter(r['status'] for r in database)),
               'registry_statuses': dict(Counter(r.get('Status', '?') for r in registry)),
               'stub_candidates': sum(f['stub_candidate'] for _, fs in files.values() for f in fs),
               'registered_deviations': len(known)}
    return metrics, errors, warnings

def main():
    try:
        metrics, errors, warnings = audit(ROOT_DIR)
        residue = subprocess.run([sys.executable, str(ROOT_DIR/'tools/audit_pe_residue.py')], capture_output=True, text=True)
        if residue.returncode:
            errors.append('PE residue audit failed:\n' + residue.stdout + residue.stderr)
    except (OSError, ValueError, sqlite3.Error) as exc:
        print(f'[FATAL] Audit could not complete: {exc}', file=sys.stderr)
        return 1
    for name, value in metrics.items():
        print(f'{name}: {value}')
    for message in warnings:
        print(f'[WARN] {message}')
    for message in errors:
        print(f'[FAIL] {message}')
    print(f'[RESULT] Tracking audit: {len(errors)} errors, {len(warnings)} warnings; ' + ('FAIL' if errors else 'PASS'))
    print('Behavioral fidelity: UNVERIFIED. Compilation, status labels, and annotations do not prove game behavior.')
    return int(bool(errors))

if __name__ == '__main__':
    sys.exit(main())
