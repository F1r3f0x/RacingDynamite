import contextlib
import io
import sqlite3
import sys
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import build_decomp as build
import verify_matching as matching
import verify_fidelity as fidelity
from source_audit import definitions

class BuildTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.compiler = self.root/'wcc.exe'
        self.compiler.touch()
        self.linker = self.root/'wlink.exe'
        self.linker.touch()
        (self.root/'wlink.lnk').write_text('option undefsok\nname build/decomp/MAINDOS_REBUILT.EXE\n')
        for attr, val in [('BUILD_DIR', self.root), ('ROOT_DIR', self.root), ('WCC386', self.compiler), ('WLINK', self.linker)]:
            p = patch.object(build, attr, val)
            p.start()
            self.addCleanup(p.stop)

    def test_compile_failure_removes_stale_object(self):
        obj = self.root/'sample.obj'
        obj.write_bytes(b'stale')
        with patch.object(build.subprocess, 'run', return_value=SimpleNamespace(returncode=1, stdout='bad', stderr='')):
            self.assertFalse(build.compile_file(self.root/'sample.c'))
        self.assertFalse(obj.exists())

    def test_compile_zero_exit_without_output_fails(self):
        with patch.object(build.subprocess, 'run', return_value=SimpleNamespace(returncode=0, stdout='', stderr='')):
            self.assertFalse(build.compile_file(self.root/'sample.c'))

    def test_compile_warning_visible(self):
        def compile(*args, **kwargs):
            (self.root/'sample.obj').write_bytes(b'fresh')
            return SimpleNamespace(returncode=0, stdout='Warning! W113\n', stderr='')
        out = io.StringIO()
        with patch.object(build.subprocess, 'run', side_effect=compile), contextlib.redirect_stdout(out):
            self.assertTrue(build.compile_file(self.root/'sample.c'))
        self.assertIn('Warning! W113', out.getvalue())

    def test_compile_exception_fails(self):
        with patch.object(build.subprocess, 'run', side_effect=OSError('cannot start')):
            self.assertFalse(build.compile_file(self.root/'sample.c'))

    def test_link_stale_executable_and_map_not_success(self):
        for name in ('MAINDOS_REBUILT.EXE', 'MAINDOS_REBUILT.MAP'):
            (self.root/name).write_bytes(b'stale')
        with patch.object(build.subprocess, 'run', return_value=SimpleNamespace(returncode=0)):
            self.assertFalse(build.link_rebuilt_binary())
        self.assertNotIn('undefsok', (self.root/'wlink_strict.lnk').read_text())

    def test_link_nonzero_even_with_fresh_output_fails(self):
        def link(*args, **kwargs):
            (self.root/'MAINDOS_REBUILT.EXE').write_bytes(b'fresh')
            (self.root/'MAINDOS_REBUILT.MAP').write_bytes(b'fresh')
            return SimpleNamespace(returncode=1)
        with patch.object(build.subprocess, 'run', side_effect=link):
            self.assertFalse(build.link_rebuilt_binary())

    def test_link_undefined_even_zero_exit_fails(self):
        def link(*args, **kwargs):
            kwargs['stdout'].write('Warning! undefined symbol missing_function\n')
            (self.root/'MAINDOS_REBUILT.EXE').write_bytes(b'fresh')
            (self.root/'MAINDOS_REBUILT.MAP').write_bytes(b'fresh')
            return SimpleNamespace(returncode=0)
        with patch.object(build.subprocess, 'run', side_effect=link):
            self.assertFalse(build.link_rebuilt_binary())

    def test_link_exception_fails(self):
        with patch.object(build.subprocess, 'run', side_effect=OSError('cannot start')):
            self.assertFalse(build.link_rebuilt_binary())

    def test_successful_link_has_fresh_outputs_and_project_cwd(self):
        def link(*args, **kwargs):
            self.assertEqual(kwargs['cwd'], self.root)
            (self.root/'MAINDOS_REBUILT.EXE').write_bytes(b'fresh')
            (self.root/'MAINDOS_REBUILT.MAP').write_bytes(b'fresh')
            return SimpleNamespace(returncode=0)
        with patch.object(build.subprocess, 'run', side_effect=link):
            self.assertTrue(build.link_rebuilt_binary())

    def test_matching_stops_on_compile_failure(self):
        with patch.object(matching, 'compile_all', return_value=False), patch.object(matching, 'link_rebuilt_binary') as link:
            self.assertFalse(matching.verify_all())
            link.assert_not_called()

    def test_matching_stops_on_link_failure(self):
        with patch.object(matching, 'compile_all', return_value=True), patch.object(matching, 'link_rebuilt_binary', return_value=False), patch.object(matching, 'get_connection') as db:
            self.assertFalse(matching.verify_all())
            db.assert_not_called()

    def test_missing_addressed_symbol_fails_without_instruction_diff(self):
        from unittest.mock import MagicMock
        conn = MagicMock()
        conn.cursor.return_value.fetchall.return_value = [
            {'symbol_name': 'Missing', 'dos_address': '0x10000', 'byte_size': 8,
             'module_name': 'main.c', 'func_status': 'decompiled'}]
        with patch.object(matching, 'compile_all', return_value=True), \
             patch.object(matching, 'link_rebuilt_binary', return_value=True), \
             patch.object(matching, 'get_connection', return_value=conn), \
             patch('diff_func.get_rebuilt_func', return_value=(None, None)), \
             patch.object(matching, 'diff_func') as diff:
            self.assertFalse(matching.verify_all())
            diff.assert_not_called()

    def test_instruction_difference_is_optional_and_behavior_unverified(self):
        from unittest.mock import MagicMock
        conn = MagicMock()
        conn.cursor.return_value.fetchall.return_value = [
            {'symbol_name': 'Example', 'dos_address': '0x10000', 'byte_size': 8,
             'module_name': 'main.c', 'func_status': 'decompiled'}]
        output = io.StringIO()
        with patch.object(matching, 'compile_all', return_value=True), \
             patch.object(matching, 'link_rebuilt_binary', return_value=True), \
             patch.object(matching, 'get_connection', return_value=conn), \
             patch('diff_func.get_rebuilt_func', return_value=(b'fresh', 0x10000)), \
             patch.object(matching, 'diff_func', return_value=(False, 25.0, 1, 4)), \
             contextlib.redirect_stdout(output):
            self.assertTrue(matching.verify_all())
            self.assertFalse(matching.verify_all(strict_matching=True))
        self.assertIn('Behavioral fidelity: UNVERIFIED', output.getvalue())

class AuditTests(unittest.TestCase):
    def test_registry_includes_addressless_rows_and_actual_columns(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp)/'functions.md'
            p.write_text('| DOS Addr | Win Addr | Ghidra Label | Symbol Name | Module | Status | Fidelity | Purpose |\n| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |\n| - | - | - | Example | main.c | Decompiled | EXACT | test |\n')
            rows = fidelity.parse_registry(p)
            self.assertEqual(len(rows), 1)
            self.assertEqual(rows[0]['Symbol Name'], 'Example')

    def test_stub_detector_ignores_comments_strings_and_prototypes(self):
        funcs = definitions('int prototype(void);\n/* int Fake(void) {} */\nint Stub(void *p) { (void)p; return 0; }\nvoid Empty(void) {}\nint Real(void) { const char *s = "}"; return call(s); }\n')
        self.assertEqual([f['symbol'] for f in funcs], ['Stub', 'Empty', 'Real'])
        self.assertEqual([f['stub_candidate'] for f in funcs], [True, True, False])

    def test_dos_headers_and_sources_audited_without_success_claim(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            for d in ('docs/ghidra', 'docs/tracking', 'database', 'decomp/src', 'decomp/include'):
                (root/d).mkdir(parents=True, exist_ok=True)
            (root/'docs/ghidra/functions.md').write_text('| DOS Addr | Win Addr | Ghidra Label | Symbol Name | Module | Status | Fidelity | Purpose |\n| 0x10000 | - | - | Example | main.c | Decompiled | EXACT | test |\n')
            (root/'docs/tracking/deviations.md').write_text('')
            (root/'decomp/src/main.c').write_text('int Example(void) { return 0; }\n')
            (root/'decomp/include/example.h').write_text('static int Helper(void) { return 1; }\n')
            conn = sqlite3.connect(root/'database/decomp.db')
            conn.execute('CREATE TABLE functions(symbol_name, status, fidelity, dos_address)')
            conn.execute("INSERT INTO functions VALUES ('Example', 'decompiled', 'EXACT', '0x10000')")
            conn.commit()
            conn.close()
            metrics, errors, warnings = fidelity.audit(root)
            self.assertEqual(metrics['source_definitions'], 2)
            self.assertEqual(metrics['registry_functions'], 1)
            self.assertTrue(any('decomp/include/example.h' in e for e in errors))
            self.assertTrue(any('missing @original' in e for e in errors))
            self.assertTrue(any('UNVERIFIED' in w for w in warnings))

if __name__ == '__main__':
    unittest.main()
