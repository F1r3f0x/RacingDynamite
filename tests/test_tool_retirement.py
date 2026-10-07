"""Retained tools must stop before legacy mutations or optional dependencies."""
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SCRIPTS = json.loads((ROOT / 'tools/migration_required.json').read_text())['scripts']
HARNESS = '''
import runpy, socket, sqlite3, subprocess, sys, urllib.request
from pathlib import Path
script = Path(sys.argv[1])
sys.path.insert(0, str(script.parent))
import tool_migration
def forbidden(*args, **kwargs):
    raise AssertionError('Legacy side effect reached before migration guard')
socket.socket = forbidden
sqlite3.connect = forbidden
subprocess.Popen = forbidden
urllib.request.urlopen = forbidden
if sys.argv[2] == 'namespace':
    sys.path.insert(0, str(script.parent.parent))
    runpy.run_module('tools.' + script.stem, run_name='retained_module')
else:
    runpy.run_path(str(script), run_name=sys.argv[2])
'''


class RetirementTests(unittest.TestCase):
    def test_cli_and_import_stop_before_legacy_side_effects(self):
        with tempfile.TemporaryDirectory() as directory:
            for name in SCRIPTS:
                for mode in ('__main__', 'retained_module', 'namespace'):
                    with self.subTest(script=name, mode=mode):
                        result = subprocess.run(
                            [sys.executable, '-c', HARNESS, str(ROOT / 'tools' / name), mode],
                            cwd=directory, capture_output=True, text=True, timeout=10)
                        self.assertNotEqual(result.returncode, 0)
                        self.assertIn(f'MIGRATION REQUIRED: {name}:', result.stderr)
                        self.assertNotIn('Traceback', result.stderr)
                        self.assertEqual(result.stdout, '')
                        self.assertEqual(list(Path(directory).iterdir()), [])


if __name__ == '__main__':
    unittest.main()
