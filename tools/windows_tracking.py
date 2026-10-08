"""Windows tracking, authentic FPO migration, and reproducible exports. Standard library only."""
import hashlib
import json
import os
import re
import sqlite3
import struct
from contextlib import contextmanager
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SCHEMA_VERSION = '2'
STAGES = ('unidentified', 'named', 'analyzed', 'reconstructed')
KINDS = ('compilation', 'raw_bytes', 'instructions', 'linked', 'emulation', 'native')


def canonical(value):
    return json.dumps(value, sort_keys=True, separators=(',', ':'), ensure_ascii=False)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def utc_now():
    return datetime.now(timezone.utc).isoformat(timespec='seconds')


def inside(root, relative):
    path = (root / relative).resolve()
    if Path(relative).is_absolute() or not path.is_relative_to(root.resolve()):
        raise ValueError('Path must be relative and remain inside the repository')
    return path


def load_target(root=ROOT):
    target = json.loads((root/'decomp/target.json').read_text(encoding='utf-8-sig'))
    data = inside(root, target['binary']).read_bytes()
    from decomp_doctor import inspect_target
    if not inspect_target(root, target)['verified']:
        raise ValueError('Authentic binary does not match target manifest')
    return target, data


def pe_inventory(data):
    """Read file-backed executable FPO extents, rejecting invalid/duplicate records."""
    from decomp_doctor import pe_summary
    header = pe_summary(data)
    nt = struct.unpack_from('<I', data, 60)[0]
    opt = nt+24
    optional_size = struct.unpack_from('<H', data, nt+20)[0]
    if optional_size < 152 or struct.unpack_from('<I', data, opt+92)[0] < 7:
        raise ValueError('PE has no debug data-directory slot')
    sections = []
    for i in range(header['sections']):
        offset = opt+optional_size+40*i
        vs, rva, raw_size, raw_offset = struct.unpack_from('<IIII', data, offset+8)
        flags = struct.unpack_from('<I', data, offset+36)[0]
        if raw_offset+raw_size > len(data):
            raise ValueError('Truncated raw section')
        sections.append({'name': data[offset:offset+8].rstrip(b'\0').decode('ascii'),
                         'rva': rva, 'virtual_size': vs, 'raw_size': raw_size,
                         'raw_offset': raw_offset, 'executable': bool(flags & 0x20000000)})
    def offset_of(rva, size):
        for section in sections:
            delta = rva-section['rva']
            if delta >= 0 and delta+size <= section['raw_size']:
                return section['raw_offset']+delta
        raise ValueError('RVA extent is not file-backed')
    debug_rva, debug_size = struct.unpack_from('<II', data, opt+144)
    if not debug_size or debug_size % 28:
        raise ValueError('Missing or invalid debug directory')
    debug_offset = offset_of(debug_rva, debug_size)
    records = {}
    for offset in range(debug_offset, debug_offset+debug_size, 28):
        kind, size, address, pointer = struct.unpack_from('<IIII', data, offset+12)
        if kind != 3:
            continue
        if size % 16 or pointer+size > len(data):
            raise ValueError('Truncated FPO data')
        for rva, length, locals_, parameters, flags in struct.iter_unpack('<IIIHH', data[pointer:pointer+size]):
            if length <= 0 or rva in records:
                raise ValueError('Invalid or duplicate FPO extent')
            if not any(s['executable'] and s['rva'] <= rva and rva+length <= s['rva']+s['virtual_size'] for s in sections):
                raise ValueError('FPO extent leaves an executable section')
            raw = offset_of(rva, length)
            records[rva] = {'rva': rva, 'byte_size': length, 'routine_sha256': sha(data[raw:raw+length])}
    if not records:
        raise ValueError('No FPO records')
    return [records[rva] for rva in sorted(records)], sections


@contextmanager
def connect(root=ROOT, writable=False):
    path = root/'database/decomp.db'
    if not path.is_file():
        raise ValueError('Tracking database is missing; run db.py migrate-windows')
    conn = sqlite3.connect(path.resolve().as_uri()+('?mode=rw' if writable else '?mode=ro'), uri=True)
    conn.row_factory = sqlite3.Row
    conn.execute('PRAGMA foreign_keys=ON')
    meta = dict(conn.execute('SELECT key,value FROM metadata'))
    target = json.loads((root/'decomp/target.json').read_text(encoding='utf-8-sig'))
    if meta.get('schema_version') != SCHEMA_VERSION or meta.get('target_sha256', '').lower() != target['sha256'].lower():
        conn.close()
        raise ValueError('Database is legacy or targets another binary; run migrate-windows')
    try:
        with conn:
            yield conn
    finally:
        conn.close()


def migrate(root=ROOT):
    target, data = load_target(root)
    path = root/'database/decomp.db'
    if path.exists():
        with sqlite3.connect(path.as_uri()+'?mode=ro', uri=True) as old:
            meta = dict(old.execute('SELECT key,value FROM metadata'))
        old.close()
        if meta.get('schema_version') == SCHEMA_VERSION:
            with connect(root):
                pass
            return False
        if meta.get('target_exe') != 'MAINDOS.EXE':
            raise ValueError('Refusing to replace an unknown tracking schema')
    candidates, sections = pe_inventory(data)
    inventory = json.loads((root/'docs/tracking/windows_inventory.json').read_text(encoding='utf-8'))
    if inventory['target']['sha256'].lower() != target['sha256'].lower():
        raise ValueError('Windows seed inventory target mismatch')
    seed = inventory['functions']
    # Validate every imported claim against bytes before constructing the replacement.
    by_rva = {f['rva']: f for f in candidates}
    for fn in seed:
        rva = int(fn['rva'], 16)
        if int(fn['va'], 16)-int(target['image_base'],16) != rva:
            raise ValueError('Seed VA/RVA mismatch')
        observed = by_rva.get(rva)
        if not observed or observed['byte_size'] != fn['size'] or observed['routine_sha256'] != fn['routine_sha256']:
            raise ValueError('Seed routine does not match authentic FPO/bytes')
        inside(root, fn['module'])
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix('.migrating')
    if temporary.exists():
        raise ValueError('Interrupted migration file exists; inspect it before retrying')
    try:
        with sqlite3.connect(temporary) as conn:
            conn.executescript((root/'database/schema.sql').read_text(encoding='utf-8'))
            metadata = {'schema_version': SCHEMA_VERSION, 'target_exe': Path(target['binary']).name,
                        'target_sha256': target['sha256'].lower(), 'image_base': str(int(target['image_base'],16)),
                        'inventory_complete': 'false', 'inventory_basis': 'Authentic FPO extents; includes unclassified runtime/library candidates',
                        'sections_json': canonical(sections), 'migrated_at': utc_now()}
            conn.executemany('INSERT INTO metadata VALUES (?,?)', metadata.items())
            for fn in candidates:
                conn.execute('INSERT INTO functions(rva,byte_size,symbol_name,extent_origin,routine_sha256) VALUES (?,?,?,?,?)',
                             (fn['rva'],fn['byte_size'],f"sub_{int(target['image_base'],16)+fn['rva']:08X}",'fpo',fn['routine_sha256']))
            for fn in seed:
                module = Path(fn['module']).name
                conn.execute('INSERT OR IGNORE INTO modules(name,source_path) VALUES (?,?)', (module,fn['module']))
                module_id = conn.execute('SELECT id FROM modules WHERE name=?',(module,)).fetchone()[0]
                stage = 'reconstructed' if fn.get('c_reconstructed') else 'analyzed' if fn.get('analyzed') else 'named'
                conn.execute('''UPDATE functions SET symbol_name=?,module_id=?,extent_confidence=?,classification='game',
                    analysis_stage=?,source_path=?,abi=?,fidelity='EXACT',evidence_path=?,notes=?,imported_evidence_json=? WHERE rva=?''',
                    (fn['symbol'],module_id,fn['extent_confidence'],stage,fn['module'],fn['abi'],fn['evidence'],
                     'Imported Windows milestone: historical reported validation preserved; fresh runs recorded separately.',canonical(fn),int(fn['rva'],16)))
            conn.executemany('INSERT INTO milestones(key,title) VALUES (?,?)',
                             [('language','Language selector'),('intro','Intro'),('menu','First menu'),('race','First playable race'),('results','Full race, results and menu return')])
            if conn.execute('PRAGMA integrity_check').fetchone()[0] != 'ok':
                raise ValueError('Replacement database integrity failed')
        conn.close()
        os.replace(temporary, path)
    finally:
        if 'conn' in locals():
            conn.close()
        if temporary.exists():
            temporary.unlink()
    return True


def input_hash(root, paths):
    return sha(canonical({p: sha(inside(root,p).read_bytes()) for p in sorted(paths)}).encode())


def record_run(rva, kind, outcome, *, inputs, artifact=None, cases=0, command, details=None, root=ROOT):
    if kind not in KINDS or outcome not in ('pass','fail','different','unverified'):
        raise ValueError('Invalid verification kind or outcome')
    target, _ = load_target(root)
    digest = input_hash(root, inputs)
    artifact_hash = sha(inside(root,artifact).read_bytes()) if artifact else None
    with connect(root, True) as conn:
        fn = conn.execute('SELECT id,routine_sha256 FROM functions WHERE rva=?',(rva,)).fetchone()
        if not fn:
            raise ValueError('Routine must be inventoried before recording results')
        conn.execute('''INSERT INTO verification_runs(function_id,kind,outcome,case_count,recorded_at,target_sha256,
          routine_sha256,input_paths_json,input_sha256,artifact_path,artifact_sha256,command,details_json)
          VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?)''',
          (fn['id'],kind,outcome,cases,utc_now(),target['sha256'].lower(),fn['routine_sha256'],canonical(inputs),digest,
           artifact,artifact_hash,command,canonical(details or {})))


def run_stale(run, root):
    try:
        if input_hash(root,json.loads(run['input_paths_json'])) != run['input_sha256']:
            return True
        if run['artifact_path'] and sha(inside(root,run['artifact_path']).read_bytes()) != run['artifact_sha256']:
            return True
    except (OSError,ValueError):
        return True
    return False


def union_bytes(intervals):
    total, end = 0, -1
    for start, size in sorted(intervals):
        new_end = start+size
        total += max(0,new_end-max(start,end))
        end = max(end,new_end)
    return total


def snapshot(root=ROOT):
    target, _ = load_target(root)
    with connect(root) as conn:
        meta = dict(conn.execute('SELECT key,value FROM metadata'))
        runs = [dict(r) for r in conn.execute('SELECT * FROM verification_runs ORDER BY id')]
        functions = [dict(r) for r in conn.execute('''SELECT f.*,m.name AS module FROM functions f
                      LEFT JOIN modules m ON m.id=f.module_id ORDER BY rva''')]
        milestones = [dict(r) for r in conn.execute('SELECT * FROM milestones ORDER BY key')]
    grouped = {}
    for run in runs:
        run['stale'] = run_stale(run,root)
        run['details'] = json.loads(run.pop('details_json'))
        run['input_paths'] = json.loads(run.pop('input_paths_json'))
        grouped.setdefault(run['function_id'],{})[run['kind']] = run
    for fn in functions:
        fn['va'] = int(target['image_base'],16)+fn['rva']
        fn['latest_runs'] = grouped.get(fn['id'],{})
        imported = fn.pop('imported_evidence_json')
        fn['imported_evidence'] = json.loads(imported) if imported else None
    def passed(fn, kind):
        run = fn['latest_runs'].get(kind,{})
        return run.get('outcome') == 'pass' and not run.get('stale',True)
    extents = [(f['rva'],f['byte_size']) for f in functions if f['byte_size']]
    recovered = [(f['rva'],f['byte_size']) for f in functions if f['byte_size'] and f['analysis_stage']=='reconstructed']
    summary = {'functions': len(functions), 'recorded_extent_bytes': union_bytes(extents),
               'summed_extent_bytes': sum(size for rva,size in extents), 'reconstructed_extent_bytes': union_bytes(recovered),
               'unknown_sizes': sum(f['byte_size'] is None for f in functions),
               'stages': {s:sum(f['analysis_stage']==s for f in functions) for s in STAGES},
               'current_passes': {k:sum(passed(f,k) for f in functions) for k in KINDS},
               'inventory_complete': meta['inventory_complete']=='true',
               'overlap_bytes': sum(size for rva,size in extents)-union_bytes(extents)}
    result = {'schema_version':2,'scope':'Active Windows inventory; FPO-seeded coverage remains incomplete',
              'target':target,'metadata':meta,'summary':summary,'functions':functions,'milestones':milestones,
              'verification_runs':runs}
    result['snapshot_id'] = sha(canonical(result).encode())
    return result


def audit(root=ROOT, require_fresh=True):
    data = snapshot(root)
    errors = []
    candidates, _ = pe_inventory(load_target(root)[1])
    originals = {f['rva']:f for f in candidates}
    for fn in data['functions']:
        original = originals.get(fn['rva'])
        if fn['extent_origin']=='fpo' and (not original or any(fn[k]!=original[k] for k in ('byte_size','routine_sha256'))):
            errors.append(f"{fn['symbol_name']}: FPO extent/hash mismatch")
        if fn['analysis_stage']=='reconstructed':
            try:
                source = inside(root,fn['source_path']).read_text(encoding='utf-8')
                symbol = re.escape(fn['symbol_name'])
                pattern = rf'@original\s+{symbol}\s+\(IGN_WIN\.EXE\s+@\s+0x0*{fn["va"]:x}\b'
                provenance = re.search(pattern,source,re.I)
                definition = re.search(r'(?m)^[ \t]*(?:[A-Za-z_]\w*[ \t*]+)+'+symbol+r'[ \t]*\([^;{}]*\)[ \t\r\n]*\{',source)
                if not provenance or not definition:
                    errors.append(f"{fn['symbol_name']}: missing Windows provenance/definition")
                elif not re.search(r'@fidelity\s+'+re.escape(fn['fidelity'])+r'\b',source[provenance.start():definition.start()]):
                    errors.append(f"{fn['symbol_name']}: source fidelity annotation disagrees with record")
                if not fn['evidence_path'] or not inside(root,fn['evidence_path']).is_file():
                    errors.append(f"{fn['symbol_name']}: missing evidence document")
            except (OSError,ValueError,TypeError) as exc:
                errors.append(f"{fn['symbol_name']}: {exc}")
    latest_ids = {run['id'] for fn in data['functions'] for run in fn['latest_runs'].values()}
    for run in data['verification_runs']:
        if run['target_sha256']!=data['target']['sha256'].lower():
            errors.append(f"Run {run['id']}: target mismatch")
        if run['id'] in latest_ids and run['stale'] and require_fresh:
            errors.append(f"Run {run['id']}: stale inputs/artifact; revalidate")
    if errors:
        raise ValueError('\n'.join(errors))
    return data


def markdown(data):
    lines = ['# Windows function inventory','',f"<!-- snapshot:{data['snapshot_id']} -->",'',
             f"Generated from active SQLite for `{data['target']['binary']}`.",
             'FPO-seeded inventory coverage is incomplete; status is not overall game completion.','',
             '| VA | RVA | Bytes | Symbol | Class | Reconstruction | Evidence |',
             '| --- | --- | ---: | --- | --- | --- | --- |']
    for fn in data['functions']:
        evidence = f"[{Path(fn['evidence_path']).name}]({Path(fn['evidence_path']).name})" if fn['evidence_path'] else '-'
        name = fn['symbol_name'].replace('|','\\|')
        lines.append(f"| `0x{fn['va']:08X}` | `0x{fn['rva']:08X}` | {fn['byte_size'] or '-'} | `{name}` | {fn['classification']} | {fn['analysis_stage']} | {evidence} |")
    return '\n'.join(lines)+'\n'


def export(root=ROOT, check=False):
    data = audit(root, require_fresh=False)
    from generate_dashboard import generate_html
    generated = utc_now()
    if check:
        try:
            generated = json.loads((root/'docs/tracking/windows_inventory.json').read_text(encoding='utf-8'))['generated_at']
        except (OSError,ValueError,KeyError):
            raise ValueError('Missing or invalid generated Windows inventory') from None
    payload = {**data,'generated_at':generated}
    files = {'docs/tracking/windows_inventory.json':json.dumps(payload,indent=2)+'\n',
             'docs/ghidra/functions.md':markdown(data), 'dashboard.html':generate_html(payload)}
    files['docs/dashboard.html'] = files['dashboard.html']
    stale = []
    for name, text in files.items():
        path = root/name
        if check:
            if not path.is_file() or text != path.read_text(encoding='utf-8'):
                stale.append(name)
        else:
            path.parent.mkdir(parents=True,exist_ok=True)
            path.write_text(text,encoding='utf-8',newline='\n')
    with connect(root) as conn:
        dump = '\n'.join(conn.iterdump())+'\n'
    if check:
        path = root/'database/dump.sql'
        if not path.is_file() or path.read_text(encoding='utf-8')!=dump:
            stale.append('database/dump.sql')
        # Equal snapshot markers alone must not hide drift between the HTML copies.
        a,b = root/'dashboard.html',root/'docs/dashboard.html'
        if a.exists() and b.exists() and a.read_bytes()!=b.read_bytes():
            stale.append('dashboard mirror')
        if stale:
            raise ValueError('Stale generated outputs: '+', '.join(stale))
    else:
        (root/'database/dump.sql').write_text(dump,encoding='utf-8',newline='\n')
    return data
