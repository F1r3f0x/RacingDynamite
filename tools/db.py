#!/usr/bin/env python3
"""CLI for active Windows tracking. Status queries are read-only; exports are explicit."""
import argparse
import json
import sqlite3
import sys
from windows_tracking import ROOT, STAGES, connect, migrate, snapshot, export, audit, inside


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
