#!/usr/bin/env python3
"""Read-only Windows target and tooling diagnostics; never builds or syncs symbols."""
import argparse
import hashlib
import importlib.util
import json
import shutil
import sqlite3
import struct
import sys
import urllib.error
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LEGACY_MARKERS = {
    'tools/build_decomp.py': ('MAINDOS_REBUILT.EXE', 'wcc386.exe'),
    'tools/verify_matching.py': ('MAINDOS.EXE', 'dos_address'),
    'tools/diff_func.py': ('LEFile', 'MAINDOS.EXE'),
    'tools/verify_fidelity.py': ('authentic DOS address', 'audit_pe_residue.py'),
    'tools/db.py': ('"target_exe", "MAINDOS.EXE"',),
    'tools/generate_dashboard.py': ('Target: <strong>MAINDOS.EXE',),
    'tools/runtime_baseline.py': ('DOSBOX/DOSBox.exe',),
}


def pe_summary(data):
    """Bounded PE32 header inspection; entry point is not assumed to be WinMain."""
    if len(data) < 64 or data[:2] != b'MZ':
        raise ValueError('Missing/truncated MZ header')
    pe = struct.unpack_from('<I', data, 60)[0]
    if pe > len(data) - 24 or data[pe:pe + 4] != b'PE\0\0':
        raise ValueError('Missing/truncated PE header')
    machine, sections = struct.unpack_from('<HH', data, pe + 4)
    optional_size = struct.unpack_from('<H', data, pe + 20)[0]
    optional = pe + 24
    if optional_size < 96 or optional + optional_size + sections * 40 > len(data):
        raise ValueError('Truncated PE32 optional header or section table')
    magic = struct.unpack_from('<H', data, optional)[0]
    if magic != 0x10b:
        raise ValueError(f'Expected PE32, found optional magic 0x{magic:04x}')
    base = struct.unpack_from('<I', data, optional + 28)[0]
    entry = struct.unpack_from('<I', data, optional + 16)[0]
    return {'machine': f'0x{machine:04x}', 'optional_magic': f'0x{magic:04x}',
            'image_base': f'0x{base:08x}', 'entry_rva': f'0x{entry:08x}',
            'entry_va': f'0x{base + entry:08x}', 'sections': sections}


def inspect_target(root, manifest):
    relative = Path(manifest['binary'])
    path = (root / relative).resolve()
    if relative.is_absolute() or not path.is_relative_to(root.resolve()):
        raise ValueError('Manifest binary must stay inside the project root')
    data = path.read_bytes()
    actual = {'sha256': hashlib.sha256(data).hexdigest().upper(), 'size': len(data),
              **pe_summary(data)}
    mismatches = [key for key in ('sha256', 'size', 'machine', 'optional_magic',
                                  'image_base', 'entry_rva')
                  if str(actual[key]).lower() != str(manifest[key]).lower()]
    return {'path': str(path), 'actual': actual, 'mismatches': mismatches,
            'verified': not mismatches}


def probe_ghidra():
    result = {'url': 'http://127.0.0.1:8080', 'identity_verified': False, 'endpoints': {}}
    # These GET endpoints exist in the repository bridge; do not guess mutation APIs.
    for endpoint in ('methods?offset=0&limit=1', 'segments?offset=0&limit=20'):
        try:
            with urllib.request.urlopen(result['url'] + '/' + endpoint, timeout=3) as response:
                content = response.read(4096)
                result['endpoints'][endpoint] = {'http_status': response.status,
                                                'response_bytes_sampled': len(content)}
        except (OSError, urllib.error.URLError) as exc:
            result['endpoints'][endpoint] = {'error': str(exc)}
    result['note'] = 'Reachability does not verify the loaded program fingerprint.'
    return result


def diagnose(root, probe=False):
    report = {'schema_version': 1, 'root': str(root), 'read_only': True,
              'python': sys.executable, 'errors': []}
    try:
        manifest = json.loads((root / 'decomp/target.json').read_text(encoding='utf-8-sig'))
        if not isinstance(manifest, dict) or manifest.get('schema_version') != 1:
            raise ValueError('Unsupported target manifest schema')
        report['manifest'] = manifest
        report['target'] = inspect_target(root, manifest)
        if not report['target']['verified']:
            report['errors'].append('Binary does not match the target manifest')
    except (OSError, ValueError, KeyError, TypeError, struct.error) as exc:
        report['errors'].append(f'Target verification failed: {exc}')
    report['dependencies'] = {name: importlib.util.find_spec(name) is not None
                              for name in ('requests', 'mcp', 'capstone', 'pefile', 'lief')}
    report['compiler_candidates_on_path'] = {name: shutil.which(name)
                                             for name in ('cl', 'clang', 'gcc')}
    report['compiler_note'] = 'Availability does not establish original compiler or x86 support.'
    report['legacy_markers'] = {}
    for name, markers in LEGACY_MARKERS.items():
        try:
            content = (root / name).read_text(encoding='utf-8-sig')
            found = [marker for marker in markers if marker in content]
            if found:
                report['legacy_markers'][name] = found
        except OSError as exc:
            report['errors'].append(f'Cannot inspect {name}: {exc}')
    db = root / 'database/decomp.db'
    if db.exists():
        try:
            with sqlite3.connect(db.resolve().as_uri() + '?mode=ro', uri=True) as conn:
                report['database_metadata'] = dict(conn.execute('SELECT key, value FROM metadata'))
        except sqlite3.Error as exc:
            report['errors'].append(f'Database metadata inspection failed: {exc}')
    else:
        report['errors'].append('Tracking database is missing')
    report['ghidra'] = probe_ghidra() if probe else {'probe': 'not requested', 'identity_verified': False}
    report['windows_pipeline_certified'] = False
    report['note'] = ('Static diagnostics only: no Windows build, matching, or runtime certification. '
                      'Legacy markers are migration clues, not a full readiness test.')
    return report


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=ROOT)
    parser.add_argument('--probe-ghidra', action='store_true', help='Read-only localhost GET probes')
    args = parser.parse_args(argv)
    report = diagnose(args.root.resolve(), args.probe_ghidra)
    print(json.dumps(report, indent=2))
    # Exit success certifies only that the inspection/identity checks succeeded.
    return int(bool(report['errors']))


if __name__ == '__main__':
    sys.exit(main())
