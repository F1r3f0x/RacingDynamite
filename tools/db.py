#!/usr/bin/env python3
"""CLI for active Windows tracking. Status queries are read-only; exports are explicit."""
import argparse
import json
import sqlite3
import sys
from windows_tracking import ROOT, STAGES, connect, migrate, snapshot, export, audit, inside, load_target, pe_inventory, sha


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest='command',required=True)
    sub.add_parser('migrate-windows',help='Replace legacy DOS tracking with verified Windows/FPO inventory')
    sub.add_parser('status',help='Read active Windows progress and current evidence counts')
    update = sub.add_parser('update',help='Generate SQL, JSON, registry and both dashboards from one snapshot')
    update.add_argument('--check',action='store_true',help='Report generated-output drift without writing')
    sub.add_parser('dashboard',help='Alias of update; all generated outputs stay consistent')
    sub.add_parser('export-markdown',help='Alias of update')
    sub.add_parser('dump-sql',help='Alias of update')
    sub.add_parser('audit',help='Validate Windows provenance and result freshness')
    add = sub.add_parser('add-candidate', help='Import a fingerprinted executable extent absent from FPO; boundary evidence required')
    add.add_argument('rva', type=lambda x: int(x, 0))
    add.add_argument('--size', type=lambda x: int(x, 0), help='Omit for an entry with unknown extent; no routine hash is then claimed')
    add.add_argument('--evidence', required=True)
    add.add_argument('--confidence', required=True)
    add.add_argument('--allow-nonexecutable', action='store_true', help='Observed call target in a file-backed section lacking PE execute flag; requires explicit evidence')
    stage = sub.add_parser('set-status',help='Set analysis stage by numeric Windows RVA; never marks validation passed')
    stage.add_argument('rva',type=lambda x:int(x,0))
    stage.add_argument('stage',choices=STAGES)
    describe = sub.add_parser('describe',help='Document an existing Windows candidate')
    describe.add_argument('rva',type=lambda x:int(x,0))
    describe.add_argument('--name')
    describe.add_argument('--class',dest='classification',choices=['unknown','game','crt','thunk'])
    describe.add_argument('--source')
    describe.add_argument('--evidence')
    describe.add_argument('--abi')
    describe.add_argument('--notes')
    describe.add_argument('--fidelity',choices=['unknown','EXACT','ADAPTED','EXTENDED','INFRASTRUCTURE'])
    describe.add_argument('--confidence')
    args = parser.parse_args(argv)
    try:
        if args.command=='migrate-windows':
            print('Migrated authentic Windows inventory.' if migrate() else 'Windows store already active; left existing evidence unchanged.')
        elif args.command=='status':
            data = snapshot()
            print(json.dumps({'target':data['target']['binary'],'binary_sha256':data['target']['sha256'],
                              **data['summary'],'snapshot_id':data['snapshot_id']},indent=2))
            print('Inventory coverage incomplete; these are inventoried extent metrics, not game completion.')
        elif args.command in ('update','dashboard','export-markdown','dump-sql'):
            data = export(check=getattr(args,'check',False))
            print(f"Snapshot {data['snapshot_id'][:12]}: {data['summary']['functions']} Windows candidates; outputs {'current' if getattr(args,'check',False) else 'updated'}.")
        elif args.command=='audit':
            data = audit()
            print(f"PASS: {data['summary']['functions']} Windows extents and recorded source/run provenance.")
        elif args.command == 'add-candidate':
            target, data = load_target()
            _, sections = pe_inventory(data)
            evidence = inside(ROOT, args.evidence)
            if not evidence.is_file() or args.rva < 0 or (args.size is not None and args.size <= 0):
                raise ValueError('Positive extent and existing boundary evidence required')
            length = args.size if args.size is not None else 1
            section = next((s for s in sections if (s['executable'] or args.allow_nonexecutable) and
                s['rva'] <= args.rva and args.rva + length <= s['rva'] +
                min(s['virtual_size'], s['raw_size'])), None)
            if section is None:
                raise ValueError('Candidate must be fully file-backed; non-executable sections require explicit opt-in')
            offset = section['raw_offset'] + args.rva - section['rva']
            with connect(writable=True) as conn:
                if conn.execute('SELECT id FROM functions WHERE rva=?', (args.rva,)).fetchone():
                    raise ValueError('Candidate already exists; use describe')
                conn.execute('''INSERT INTO functions(rva,byte_size,symbol_name,extent_origin,
                    extent_confidence,routine_sha256,evidence_path) VALUES (?,?,?,?,?,?,?)''',
                    (args.rva, args.size, f"sub_{int(target['image_base'],16)+args.rva:08X}",
                     'authenticated-file-analysis' if args.size is not None else 'authenticated-entry-analysis', args.confidence,
                     sha(data[offset:offset+args.size]) if args.size is not None else None, args.evidence))
            print('Imported candidate only; use describe and set-status to record actual findings.')
        else:
            with connect(writable=True) as conn:
                fn = conn.execute('SELECT id FROM functions WHERE rva=?',(args.rva,)).fetchone()
                if not fn:
                    raise ValueError('Unknown Windows RVA; export/import new candidates from authentic analysis first')
                if args.command=='set-status':
                    conn.execute('UPDATE functions SET analysis_stage=? WHERE id=?',(args.stage,fn['id']))
                else:
                    mapping = {'name':'symbol_name','classification':'classification','source':'source_path','evidence':'evidence_path',
                               'abi':'abi','notes':'notes','fidelity':'fidelity','confidence':'extent_confidence'}
                    for argument,column in mapping.items():
                        value = getattr(args,argument)
                        if value is not None:
                            if argument in ('source','evidence'):
                                inside(ROOT,value)
                            conn.execute(f'UPDATE functions SET {column}=? WHERE id=?',(value,fn['id']))
            print('Updated record; run db.py audit and db.py update to validate/export.')
        return 0
    except (OSError,ValueError,KeyError,sqlite3.Error) as exc:
        print(f'[FAIL] {exc}',file=sys.stderr)
        return 1


if __name__=='__main__':
    sys.exit(main())
