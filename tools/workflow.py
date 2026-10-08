"""Repository workflow gates. Hooks inspect the index; completion writes explicit exports."""
import argparse
from contextlib import closing, contextmanager
import io
import json
import os
from pathlib import Path
import re
import shutil
import sqlite3
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
EXPORTS = {'database/decomp.db', 'database/dump.sql', 'docs/tracking/windows_inventory.json',
           'docs/ghidra/functions.md', 'dashboard.html', 'docs/dashboard.html'}
ARTIFACT_SUFFIXES = {'.exe', '.dll', '.obj', '.o', '.lib', '.a', '.zip', '.gpr'}


def git(root, *args):
    return subprocess.check_output(['git', '-C', str(root), *args])


def safe(root, name):
    path = (root / name).resolve()
    if Path(name).is_absolute() or not path.is_relative_to(root.resolve()):
        raise ValueError('Path escapes repository: ' + name)
    return path


def changed_paths(root, base=None):
    args = ['diff', '--name-only', '-z', '--no-renames']
    args += [base, 'HEAD'] if base else ['--cached']
    output = git(root, *args)
    return output.decode().rstrip('\0').split('\0') if output else []


def blob(root, revision, name):
    result = subprocess.run(['git', '-C', str(root), 'show', revision + ':' + name],
                            capture_output=True)
    return result.stdout if result.returncode == 0 else b''


def mask_c(text):
    """Retain offsets/newlines while hiding comments and string/character literals."""
    return re.sub(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
                  lambda m: re.sub(r'[^\n]', ' ', m.group()), text, flags=re.S)


def functions(text):
    masked = mask_c(text)
    pattern = r'(?m)^[ \t]*(?:[A-Za-z_]\w*[ \t*]+)+([A-Za-z_]\w*)\s*\([^;{}]*\)\s*\{'
    found = {}
    previous_end = 0
    for match in re.finditer(pattern, masked):
        depth, end = 1, match.end()
        while depth and end < len(masked):
            depth += (masked[end] == '{') - (masked[end] == '}')
            end += 1
        if depth:
            raise ValueError('Unbalanced function body: ' + match[1])
        annotations = text[previous_end:match.start()]
        found[match[1]] = (text[match.start():end], annotations)
        previous_end = end
    return found


def records(root):
    with closing(sqlite3.connect(safe(root, 'database/decomp.db').as_uri() + '?mode=ro', uri=True)) as conn:
        conn.row_factory = sqlite3.Row
        return [dict(row) for row in conn.execute('SELECT * FROM functions')]


def newly_reconstructed(root, old_database):
    """Catch stage promotions even when no source definition changed in this commit."""
    if not old_database:
        old = {}
    else:
        with closing(sqlite3.connect(':memory:')) as conn:
            try:
                conn.deserialize(old_database)
                old = dict(conn.execute('SELECT rva,analysis_stage FROM functions'))
            except sqlite3.Error as exc:
                raise ValueError('Cannot compare old tracking store: ' + str(exc)) from exc
    return {row['rva'] for row in records(root) if row['analysis_stage'] == 'reconstructed'
            and old.get(row['rva']) != 'reconstructed'}


def policy(root, paths, old_contents):
    errors, affected = [], set()
    rows = records(root)
    target = json.loads((root / 'decomp/target.json').read_text(encoding='utf-8-sig'))
    base = int(target['image_base'], 16)
    for name in paths:
        lower = name.lower()
        if lower.startswith(('ignition/', 'assets/', 'original/', 'build/', 'bin/', 'obj/',
                             'docs/extracted_bitmaps/', 'ghidra/', 'ghidra_project/',
                             'scratch/', 'reports/', 'decomp/raw/')) or Path(lower).suffix in ARTIFACT_SUFFIXES:
            errors.append(name + ': protected original or generated/binary artifact')
        path = safe(root, name)
        if not lower.startswith('decomp/') or path.suffix != '.c':
            continue
        new = path.read_text(encoding='utf-8-sig') if path.exists() else ''
        old = old_contents.get(name, b'').decode('utf-8-sig').replace('\r\n', '\n')
        old_functions, new_functions = functions(old), functions(new)
        old_asm = re.findall(r'\b(?:__asm(?:__)?|asm)\b', mask_c(old))
        if re.findall(r'\b(?:__asm(?:__)?|asm)\b', mask_c(new)) != old_asm:
            errors.append(name + ': new handwritten assembly')
        for symbol, (body, annotations) in new_functions.items():
            if old_functions.get(symbol) == (body, annotations):
                continue
            matches = [r for r in rows if r['symbol_name'] == symbol and r['source_path'] == name]
            if len(matches) != 1:
                errors.append(f'{name}: {symbol} requires exactly one active Windows record')
                continue
            row = matches[0]
            affected.add(row['rva'])
            provenance = re.search(r'@original\s+' + re.escape(symbol) +
                                   r'\s+\(IGN_WIN\.EXE\s+@\s+(0x[0-9a-fA-F]+)\b', annotations)
            fidelity = re.findall(r'@fidelity\s+(EXACT|ADAPTED|EXTENDED|INFRASTRUCTURE)\b', annotations)
            if not provenance or int(provenance[1], 16) != base + row['rva']:
                errors.append(f'{symbol}: missing or mismatched Windows VA provenance')
            if fidelity != [row['fidelity']]:
                errors.append(f'{symbol}: exactly one fidelity annotation must match tracking')
            if row['analysis_stage'] != 'reconstructed' or not row['abi'] or row['classification'] == 'unknown':
                errors.append(f'{symbol}: reconstruction needs stage, ABI and classification')
            if not row['evidence_path'] or not safe(root, row['evidence_path']).is_file():
                errors.append(f'{symbol}: evidence must be included in the snapshot')
            for deviation in re.findall(r'@deviation\s+(DEV-[A-Za-z0-9-]+)', annotations):
                if deviation not in (root / 'docs/tracking/deviations.md').read_text(encoding='utf-8-sig'):
                    errors.append(f'{symbol}: undocumented deviation {deviation}')
        for symbol in old_functions.keys() - new_functions.keys():
            if any(r['symbol_name'] == symbol and r['source_path'] == name and
                   r['analysis_stage'] == 'reconstructed' for r in rows):
                errors.append(f'{symbol}: deleted definition still marked reconstructed')
    if errors:
        raise ValueError('\n'.join(errors))
    return affected


def run(root, *args):
    subprocess.run(list(args), cwd=root, check=True)


@contextmanager
def staged_tree(root, base=None):
    """Materialize Git blobs directly: no checkout, stash, hooks, or working-tree edits."""
    temporary_root = root / 'build/workflow'
    temporary_root.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='ignition-index-', dir=temporary_root) as directory:
        tree = Path(directory)
        listing = git(root, 'ls-tree', '-rz', 'HEAD') if base else git(root, 'ls-files', '--stage', '-z')
        entries = []
        for entry in listing.split(b'\0'):
            if not entry:
                continue
            metadata, name = entry.split(b'\t', 1)
            mode, middle, last = metadata.decode().split()
            oid, stage_or_type = (last, middle) if base else (middle, last)
            if mode not in ('100644', '100755') or (not base and stage_or_type != '0'):
                raise ValueError('Unsupported symlink, submodule or unresolved index entry')
            path = safe(tree, name.decode())
            entries.append((oid, path))
        request = ''.join(oid + '\n' for oid, _ in entries).encode('ascii')
        batch = io.BytesIO(subprocess.check_output(['git', '-C', str(root), 'cat-file', '--batch'], input=request))
        for oid, path in entries:
            actual, kind, length = batch.readline().decode('ascii').split()
            if actual != oid or kind != 'blob':
                raise ValueError('Unexpected Git batch object')
            data = batch.read(int(length))
            if len(data) != int(length) or batch.read(1) != b'\n':
                raise ValueError('Truncated Git batch object')
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(data)
        yield tree


def attach_local_evidence(root, tree):
    """Only originals and ignored result artifacts may come from outside the index."""
    target = json.loads((tree / 'decomp/target.json').read_text(encoding='utf-8-sig'))
    names = {target['binary']}
    with closing(sqlite3.connect((tree / 'database/decomp.db').as_uri() + '?mode=ro', uri=True)) as conn:
        for artifact, inputs in conn.execute('SELECT artifact_path,input_paths_json FROM verification_runs'):
            if artifact:
                if not artifact.startswith('build/'):
                    raise ValueError('Verification artifacts must live under build/: ' + artifact)
                names.add(artifact)
            for name in json.loads(inputs):
                if not safe(tree, name).is_file():
                    raise ValueError('Verification input absent from committed snapshot: ' + name)
    for name in names:
        source, destination = safe(root, name), safe(tree, name)
        if source.is_file():
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(source, destination)


def require_results(data, affected):
    known = {fn['rva'] for fn in data['functions']}
    if not affected.issubset(known):
        raise ValueError('Unknown affected RVA')
    for fn in data['functions']:
        if fn['rva'] not in affected:
            continue
        for kind in ('compilation', 'emulation'):
            result = fn['latest_runs'].get(kind, {})
            if result.get('outcome') != 'pass' or result.get('stale', True):
                raise ValueError(f"{fn['symbol_name']}: fresh passing {kind} required; extend the real verifier")


def portable_exports(root):
    """Structural consistency without original binary/artifact availability."""
    payload = json.loads((root / 'docs/tracking/windows_inventory.json').read_text(encoding='utf-8'))
    indexed = {fn['rva']: fn for fn in payload['functions']}
    rows = records(root)
    if set(indexed) != {row['rva'] for row in rows}:
        raise ValueError('Inventory and SQLite candidate sets disagree')
    for row in rows:
        for key in ('symbol_name', 'analysis_stage', 'source_path', 'evidence_path', 'abi', 'fidelity',
                    'classification', 'byte_size', 'routine_sha256'):
            if indexed[row['rva']].get(key) != row[key]:
                raise ValueError('Inventory and SQLite disagree: ' + key)
        if row['analysis_stage'] == 'reconstructed':
            if not row['source_path'] or not row['evidence_path'] or not row['abi'] or row['classification'] == 'unknown':
                raise ValueError(row['symbol_name'] + ': incomplete reconstruction record')
            source = safe(root, row['source_path']).read_text(encoding='utf-8-sig')
            definition = functions(source).get(row['symbol_name'])
            va = int(payload['target']['image_base'], 16) + row['rva']
            provenance = (re.search(r'@original\s+' + re.escape(row['symbol_name']) +
                          r'\s+\(IGN_WIN\.EXE\s+@\s+(0x[0-9a-fA-F]+)\b', definition[1])
                          if definition else None)
            if not provenance or int(provenance[1], 16) != va:
                raise ValueError(row['symbol_name'] + ': missing or mismatched Windows definition/provenance')
            if re.findall(r'@fidelity\s+(EXACT|ADAPTED|EXTENDED|INFRASTRUCTURE)\b', definition[1]) != [row['fidelity']]:
                raise ValueError(row['symbol_name'] + ': source fidelity disagrees with tracking')
            if not safe(root, row['evidence_path']).is_file():
                raise ValueError(row['symbol_name'] + ': evidence absent from snapshot')
    with closing(sqlite3.connect((root / 'database/decomp.db').as_uri() + '?mode=ro', uri=True)) as conn:
        expected = '\n'.join(conn.iterdump()) + '\n'
    if (root / 'database/dump.sql').read_text(encoding='utf-8') != expected:
        raise ValueError('SQL export drift')
    from generate_dashboard import generate_html
    from windows_tracking import markdown
    expected_files = {'dashboard.html': generate_html(payload),
                      'docs/dashboard.html': generate_html(payload),
                      'docs/ghidra/functions.md': markdown(payload)}
    for name, expected in expected_files.items():
        if safe(root, name).read_text(encoding='utf-8') != expected:
            raise ValueError('Generated export drift: ' + name)


def uv_python(root, script, *args):
    # Auditors use stdlib; reuse this interpreter without installing a temp project.
    run(root, 'uv', 'run', '--no-project', '--python', sys.executable, 'python', script, *args)


def check(root, base=None, portable=False):
    paths = changed_paths(root, base)
    run(root, 'git', 'diff', '--check', *([base, 'HEAD'] if base else ['--cached']))
    with staged_tree(root, base) as tree:
        old = {name: blob(root, base or 'HEAD', name) for name in paths}
        affected = policy(tree, paths, old)
        if 'database/decomp.db' in paths:
            affected.update(newly_reconstructed(tree, old['database/decomp.db']))
        windows = any(p.startswith(('decomp/', 'database/', 'docs/ghidra/', 'docs/tracking/')) or
                      p in EXPORTS or p.startswith('tools/') for p in paths)
        if windows:
            portable_exports(tree)
        if windows and not portable:
            attach_local_evidence(root, tree)
            # Execute the repository auditors from the staged snapshot.
            uv_python(tree, 'tools/verify_fidelity.py')
            uv_python(tree, 'tools/db.py', 'update', '--check')
            uv_python(tree, 'tools/workflow.py', 'results',
                *[arg for rva in sorted(affected) for arg in ('--rva', hex(rva))])
    print('PASS: committed policy snapshot; ' +
          ('portable only, fidelity NOT certified' if portable else 'applicable fidelity/export checks passed'))
    return affected


@contextmanager
def completion_lock(root):
    path = root / 'build/workflow/completion.lock'
    path.parent.mkdir(parents=True, exist_ok=True)
    try:
        fd = os.open(path, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    except FileExistsError:
        raise ValueError('Completion owned by another process; inspect ' + str(path)) from None
    try:
        with os.fdopen(fd, 'w') as stream:
            stream.write(str(os.getpid()))
        yield
    finally:
        path.unlink()


def complete(root, rvas, analysis_only, limitations):
    from windows_tracking import audit, export
    with completion_lock(root):
        # A failed attempt must not leave a prior success report looking current.
        (root / 'build/workflow/handoff.json').unlink(missing_ok=True)
        known = {row['rva'] for row in records(root)}
        if not rvas or not set(rvas).issubset(known):
            raise ValueError('Declare existing affected RVAs with --rva')
        if analysis_only and not limitations:
            raise ValueError('Analysis-only completion requires --limitation explaining absent runtime evidence')
        if analysis_only and any(row['rva'] in rvas and row['analysis_stage'] == 'reconstructed'
                                 for row in records(root)):
            raise ValueError('Reconstructed routines require implementation verification')
        if analysis_only and any(row['rva'] in rvas and
                                 (row['analysis_stage'] != 'analyzed' or not row['abi'] or
                                  row['classification'] == 'unknown' or not row['evidence_path'] or
                                  not safe(root, row['evidence_path']).is_file())
                                 for row in records(root)):
            raise ValueError('Analysis completion requires analyzed records, ABI, classification and evidence')
        if not analysis_only:
            run(root, 'uv', 'run', 'tools/verify_matching.py')
        run(root, 'uv', 'run', 'python', 'tools/verify_fidelity.py')
        data = audit(root)
        if not analysis_only:
            require_results(data, set(rvas))
        export(root)
        export(root, check=True)
        report = {'snapshot_id': data['snapshot_id'], 'affected_rvas': [hex(r) for r in rvas],
                  'validation_scope': 'analysis only' if analysis_only else 'compilation and differential emulation',
                  'limitations': limitations + ['No native game parity certified'],
                  'git_status': git(root, 'status', '--short').decode()}
        report['routines'] = [{key: fn[key] for key in ('rva', 'symbol_name', 'analysis_stage', 'latest_runs')}
                              for fn in data['functions'] if fn['rva'] in rvas]
        path = root / 'build/workflow/handoff.json'
        path.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
        print(json.dumps(report, indent=2))


def commit_message(path):
    message = path.read_text(encoding='utf-8-sig')
    if not re.match(r'^(?:re|port|docs|test|build|ci|chore|fix|feat|refactor)(?:\([\w.-]+\))?!?: .+', message):
        raise ValueError('Use a Conventional Commit subject')
    expected = required_commit_rvas(ROOT)
    supplied = {int(value, 16) for value in re.findall(r'(?m)^RVA:\s*(0x[0-9a-fA-F]+)\s*$', message)}
    if expected - supplied:
        raise ValueError('Reconstruction commits require RVA: 0x... trailers for: ' +
                         ', '.join(hex(value) for value in sorted(expected - supplied)))


def required_commit_rvas(root):
    paths = changed_paths(root)
    if not any(p.startswith('decomp/') and p.endswith('.c') for p in paths):
        return set()
    target = json.loads(blob(root, '', 'decomp/target.json'))
    base = int(target['image_base'], 16)
    required = set()
    for name in paths:
        if not name.startswith('decomp/') or not name.endswith('.c'):
            continue
        old = functions(blob(root, 'HEAD', name).decode('utf-8-sig').replace('\r\n', '\n'))
        new = functions(blob(root, '', name).decode('utf-8-sig').replace('\r\n', '\n'))
        for symbol, definition in new.items():
            if old.get(symbol) != definition:
                matches = re.findall(r'@original\s+' + re.escape(symbol) +
                                     r'\s+\(IGN_WIN\.EXE\s+@\s+(0x[0-9a-fA-F]+)', definition[1])
                if matches:
                    required.add(int(matches[-1], 16) - base)
    return required


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest='command', required=True)
    check_parser = sub.add_parser('check')
    mode = check_parser.add_mutually_exclusive_group()
    mode.add_argument('--staged', action='store_true', help='Default; inspect the index')
    mode.add_argument('--base', help='CI: compare base commit to HEAD and inspect HEAD')
    check_parser.add_argument('--portable', action='store_true', help='Policy only; never certifies fidelity')
    sub.add_parser('install-hooks')
    preflight = sub.add_parser('preflight')
    preflight.add_argument('--rva', action='append', type=lambda x: int(x, 0), default=[])
    preflight.add_argument('--file', action='append', default=[])
    complete_parser = sub.add_parser('complete')
    complete_parser.add_argument('--rva', action='append', type=lambda x: int(x, 0), required=True)
    complete_parser.add_argument('--analysis-only', action='store_true')
    complete_parser.add_argument('--limitation', action='append', default=[])
    results = sub.add_parser('results')
    results.add_argument('--rva', action='append', type=lambda x: int(x, 0), default=[])
    msg = sub.add_parser('commit-message')
    msg.add_argument('path', type=Path)
    args = parser.parse_args()
    try:
        if args.command == 'check':
            check(ROOT, args.base, args.portable)
        elif args.command == 'install-hooks':
            if not shutil.which('pwsh'):
                raise ValueError('PowerShell 7 (pwsh) required for hooks')
            existing = subprocess.run(['git', 'config', '--get', 'core.hooksPath'], cwd=ROOT, capture_output=True, text=True)
            if existing.returncode == 0 and existing.stdout.strip() != '.githooks':
                raise ValueError('Existing hooksPath preserved: ' + existing.stdout.strip())
            run(ROOT, 'git', 'config', '--local', 'core.hooksPath', '.githooks')
            print('Installed repository hooks; requires uv and pwsh on PATH')
        elif args.command == 'preflight':
            from decomp_doctor import diagnose
            data = diagnose(ROOT)
            data.update(affected_rvas=[hex(r) for r in args.rva], feature_files=args.file,
                        initial_git_status=git(ROOT, 'status', '--short').decode())
            for name in args.file:
                safe(ROOT, name)
            path = ROOT / 'build/workflow/preflight.json'
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(json.dumps(data, indent=2) + '\n', encoding='utf-8')
            print(json.dumps(data, indent=2))
            if data['errors']:
                return 1
        elif args.command == 'complete':
            complete(ROOT, args.rva, args.analysis_only, args.limitation)
        elif args.command == 'results':
            from windows_tracking import audit
            require_results(audit(ROOT), set(args.rva))
        else:
            commit_message(args.path)
        return 0
    except (OSError, ValueError, sqlite3.Error, subprocess.CalledProcessError) as exc:
        print('[FAIL] ' + str(exc))
        return 1


if __name__ == '__main__':
    raise SystemExit(main())
