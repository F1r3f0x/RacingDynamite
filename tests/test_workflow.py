"""Regression tests for staged isolation, omitted tracking, stale evidence and export guards."""
import json
import contextlib
import io
from pathlib import Path
import sqlite3
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import workflow
import windows_tracking as tracking
from test_windows_tracking import fixture


class WorkflowTests(unittest.TestCase):
    def setUp(self):
        temporary_root = workflow.ROOT / 'build/workflow/tests'
        temporary_root.mkdir(parents=True, exist_ok=True)
        self.temp = tempfile.TemporaryDirectory(dir=temporary_root)
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        data = fixture(self.root)
        # Workflow gates consume an active store. Test migration in its own suite,
        # avoiding Windows replacement races in every unrelated guard fixture.
        target = json.loads((self.root / 'decomp/target.json').read_text())
        with contextlib.closing(sqlite3.connect(self.root / 'database/decomp.db')) as conn:
            conn.executescript('DROP TABLE functions; DROP TABLE metadata;')
            conn.executescript((tracking.ROOT / 'database/schema.sql').read_text())
            conn.executemany('INSERT INTO metadata VALUES (?, ?)',
                             [('schema_version', tracking.SCHEMA_VERSION),
                              ('target_sha256', target['sha256']), ('inventory_complete', 'false')])
            candidates, _ = tracking.pe_inventory(data)
            for candidate in candidates:
                conn.execute('INSERT INTO functions(rva,byte_size,symbol_name,extent_origin,routine_sha256) VALUES(?,?,?,?,?)',
                             (candidate['rva'], candidate['byte_size'], 'sub_' + hex(candidate['rva']),
                              'fpo', candidate['routine_sha256']))
            conn.execute("UPDATE functions SET symbol_name='Foo',analysis_stage='reconstructed',abi='cdecl',fidelity='EXACT',classification='game',source_path='decomp/src/mem.c',evidence_path='docs/ghidra/evidence.md' WHERE rva=4224")
            conn.commit()
        tracking.export(self.root)
        self.source = 'decomp/src/mem.c'
        self.original = (self.root / self.source).read_bytes()
        with tracking.connect(self.root, writable=True) as conn:
            conn.execute("UPDATE functions SET classification='game' WHERE rva=4224")
        tracking.export(self.root)

    def stage_repo(self):
        subprocess.run(['git', 'init', '-q', str(self.root)], check=True)
        for key, value in [('user.name', 'Fixture'), ('user.email', 'fixture@example.invalid'),
                           ('core.autocrlf', 'false')]:
            subprocess.run(['git', '-C', str(self.root), 'config', key, value], check=True)
        # Fixture originals are deliberately outside the committed snapshot.
        (self.root / '.gitignore').write_text('/Ignition/\n/build/\n')
        subprocess.run(['git', '-C', str(self.root), 'add', '.'], check=True)
        subprocess.run(['git', '-C', str(self.root), 'commit', '-qm', 'test: fixture'], check=True)

    def test_new_untracked_function_is_rejected(self):
        path = self.root / self.source
        path.write_bytes(self.original + b'\nint Missing(void) { return 0; }\n')
        with self.assertRaisesRegex(ValueError, 'Missing requires exactly one'):
            workflow.policy(self.root, [self.source], {self.source: self.original})

    def test_modified_definition_requires_correct_provenance(self):
        path = self.root / self.source
        path.write_bytes(self.original.replace(b'00401080', b'00401081'))
        with self.assertRaisesRegex(ValueError, 'VA provenance'):
            workflow.policy(self.root, [self.source], {self.source: self.original})

    def test_changed_function_reports_rva(self):
        (self.root / self.source).write_bytes(self.original.replace(b'return 1', b'return 2'))
        self.assertEqual(workflow.policy(self.root, [self.source], {self.source: self.original}), {0x1080})

    def test_unchanged_legacy_function_is_allowed_but_edit_is_rejected(self):
        old = self.original + b'\n/* MAINDOS.EXE */\nint Legacy(void) { return 0; }\n'
        (self.root / self.source).write_bytes(old)
        workflow.policy(self.root, [self.source], {self.source: old})
        (self.root / self.source).write_bytes(old.replace(b'return 0', b'return 3'))
        with self.assertRaisesRegex(ValueError, 'Legacy requires exactly one'):
            workflow.policy(self.root, [self.source], {self.source: old})

    def test_deleted_reconstructed_definition_is_rejected(self):
        (self.root / self.source).write_text('')
        with self.assertRaisesRegex(ValueError, 'deleted definition'):
            workflow.policy(self.root, [self.source], {self.source: self.original})

    def test_originals_and_build_artifacts_rejected_even_if_deleted(self):
        for name in ['Ignition/Ignition/IGN_WIN.EXE', 'build/object.obj', 'decomp/raw/output.c']:
            with self.subTest(name=name), self.assertRaisesRegex(ValueError, 'protected original'):
                workflow.policy(self.root, [name], {})

    def test_inline_assembly_rejected_but_comments_ignored(self):
        (self.root / self.source).write_bytes(self.original + b'\n// __asm\n')
        workflow.policy(self.root, [self.source], {self.source: self.original})
        (self.root / self.source).write_bytes(self.original.replace(b'return 1', b'__asm("nop"); return 1'))
        with self.assertRaisesRegex(ValueError, 'assembly'):
            workflow.policy(self.root, [self.source], {self.source: self.original})

    def test_staged_snapshot_does_not_use_unstaged_fix(self):
        self.stage_repo()
        path = self.root / self.source
        broken = self.original.replace(b'00401080', b'00401081')
        path.write_bytes(broken)
        subprocess.run(['git', '-C', str(self.root), 'add', self.source], check=True)
        path.write_bytes(self.original)
        with workflow.staged_tree(self.root) as tree:
            self.assertEqual((tree / self.source).read_bytes(), broken)
            self.assertFalse((tree / 'Ignition/Ignition/IGN_WIN.EXE').exists())
            with self.assertRaisesRegex(ValueError, 'VA provenance'):
                workflow.policy(tree, [self.source], {self.source: self.original})
        self.assertEqual(path.read_bytes(), self.original)

    def test_unstaged_evidence_does_not_satisfy_policy(self):
        self.stage_repo()
        evidence = 'docs/ghidra/new.md'
        (self.root / evidence).write_text('unstaged')
        with tracking.connect(self.root, writable=True) as conn:
            conn.execute('UPDATE functions SET evidence_path=? WHERE rva=4224', (evidence,))
        (self.root / self.source).write_bytes(self.original.replace(b'return 1', b'return 2'))
        subprocess.run(['git', '-C', str(self.root), 'add', 'database/decomp.db', self.source], check=True)
        with workflow.staged_tree(self.root) as tree:
            with self.assertRaisesRegex(ValueError, 'evidence must be included'):
                workflow.policy(tree, [self.source], {self.source: self.original})

    def test_sql_inventory_and_dashboard_drift_rejected_read_only(self):
        workflow.portable_exports(self.root)
        for name in ['database/dump.sql', 'dashboard.html', 'docs/dashboard.html', 'docs/ghidra/functions.md']:
            path = self.root / name
            old = path.read_bytes()
            path.write_bytes(old + b'changed')
            with self.subTest(name=name), self.assertRaises(ValueError):
                workflow.portable_exports(self.root)
            self.assertEqual(path.read_bytes(), old + b'changed')
            path.write_bytes(old)
        inventory = self.root / 'docs/tracking/windows_inventory.json'
        data = json.loads(inventory.read_text())
        data['functions'][0]['symbol_name'] = 'Invented'
        inventory.write_text(json.dumps(data))
        with self.assertRaisesRegex(ValueError, 'SQLite disagree'):
            workflow.portable_exports(self.root)

    def test_fresh_compilation_and_emulation_required(self):
        artifact = self.root / 'build/fixture.dll'
        artifact.parent.mkdir()
        artifact.write_bytes(b'compiled')
        for kind in ('compilation', 'emulation'):
            tracking.record_run(0x1080, kind, 'pass', inputs=[self.source],
                                artifact='build/fixture.dll', command='fixture', root=self.root)
        workflow.require_results(tracking.snapshot(self.root), {0x1080})
        artifact.write_bytes(b'changed')
        with self.assertRaisesRegex(ValueError, 'fresh passing compilation'):
            workflow.require_results(tracking.snapshot(self.root), {0x1080})

    def test_missing_result_is_not_promoted_from_stage(self):
        with self.assertRaisesRegex(ValueError, 'fresh passing compilation'):
            workflow.require_results(tracking.snapshot(self.root), {0x1080})

    def test_database_only_reconstruction_promotion_requires_results(self):
        old = (self.root / 'database/decomp.db').read_bytes()
        with tracking.connect(self.root, writable=True) as conn:
            conn.execute("UPDATE functions SET analysis_stage='reconstructed' WHERE rva=4240")
        self.assertEqual(workflow.newly_reconstructed(self.root, old), {0x1090})

    def test_local_artifacts_cannot_supply_unstaged_verifier_inputs(self):
        artifact = self.root / 'build/fixture.dll'
        artifact.parent.mkdir()
        artifact.write_bytes(b'compiled')
        tracking.record_run(0x1080, 'compilation', 'pass', inputs=[self.source],
                            artifact='build/fixture.dll', command='fixture', root=self.root)
        self.stage_repo()
        with workflow.staged_tree(self.root) as tree:
            workflow.attach_local_evidence(self.root, tree)
            self.assertEqual((tree / 'build/fixture.dll').read_bytes(), b'compiled')
            (tree / self.source).unlink()
            with self.assertRaisesRegex(ValueError, 'input absent'):
                workflow.attach_local_evidence(self.root, tree)

    def test_ci_snapshot_ignores_index_and_working_tree(self):
        self.stage_repo()
        path = self.root / self.source
        path.write_bytes(self.original.replace(b'00401080', b'00401081'))
        subprocess.run(['git', '-C', str(self.root), 'add', self.source], check=True)
        with workflow.staged_tree(self.root, base='HEAD') as tree:
            self.assertEqual((tree / self.source).read_bytes(), self.original)

    def test_portable_gate_rejects_omitted_export(self):
        self.stage_repo()
        (self.root / 'dashboard.html').write_text('tampered')
        subprocess.run(['git', '-C', str(self.root), 'add', 'dashboard.html'], check=True)
        with self.assertRaisesRegex(ValueError, 'export drift'):
            workflow.check(self.root, portable=True)

    def test_complete_rejects_unknown_and_reconstructed_analysis_only(self):
        with self.assertRaisesRegex(ValueError, 'existing affected RVAs'):
            workflow.complete(self.root, [0x9999], False, [])
        with self.assertRaisesRegex(ValueError, 'implementation verification'):
            workflow.complete(self.root, [0x1080], True, ['analysis only'])
        self.assertFalse((self.root / 'build/workflow/handoff.json').exists())

    def test_unknown_rva_cannot_silently_pass_results_gate(self):
        with self.assertRaisesRegex(ValueError, 'Unknown affected'):
            workflow.require_results(tracking.snapshot(self.root), {0x9999})

    def test_analysis_completion_exports_report_without_verifier(self):
        self.stage_repo()
        with tracking.connect(self.root, writable=True) as conn:
            conn.execute("UPDATE functions SET analysis_stage='analyzed',abi='cdecl',classification='game',evidence_path='docs/ghidra/evidence.md' WHERE rva=4240")
        with patch.object(workflow, 'run') as run, contextlib.redirect_stdout(io.StringIO()):
            workflow.complete(self.root, [0x1090], True, ['No runtime evidence'])
        self.assertEqual(run.call_count, 1)
        self.assertIn('tools/verify_fidelity.py', run.call_args.args)
        report = json.loads((self.root / 'build/workflow/handoff.json').read_text())
        self.assertEqual(report['validation_scope'], 'analysis only')
        self.assertEqual(report['affected_rvas'], ['0x1090'])
        self.assertEqual(report['snapshot_id'], tracking.export(self.root, check=True)['snapshot_id'])

    def test_failed_verifier_leaves_no_handoff_or_new_exports(self):
        old = (self.root / 'docs/tracking/windows_inventory.json').read_bytes()
        report = self.root / 'build/workflow/handoff.json'
        report.parent.mkdir(parents=True, exist_ok=True)
        report.write_text('{"old": "success"}')
        with patch.object(workflow, 'run', side_effect=subprocess.CalledProcessError(1, 'verifier')):
            with self.assertRaises(subprocess.CalledProcessError):
                workflow.complete(self.root, [0x1080], False, [])
        self.assertFalse((self.root / 'build/workflow/handoff.json').exists())
        self.assertFalse((self.root / 'build/workflow/completion.lock').exists())
        self.assertEqual((self.root / 'docs/tracking/windows_inventory.json').read_bytes(), old)

    def test_completion_lock_preserves_other_owner(self):
        with workflow.completion_lock(self.root):
            path = self.root / 'build/workflow/completion.lock'
            old = path.read_bytes()
            with self.assertRaisesRegex(ValueError, 'another process'):
                with workflow.completion_lock(self.root):
                    self.fail('lock was acquired twice')
            self.assertEqual(path.read_bytes(), old)
        self.assertFalse(path.exists())

    def test_path_escape_rejected(self):
        with self.assertRaisesRegex(ValueError, 'escapes'):
            workflow.safe(self.root, '../outside')

    def test_commit_message_convention_and_rva(self):
        path = self.root / 'message.txt'
        with patch.object(workflow, 'ROOT', self.root), patch.object(workflow, 'required_commit_rvas', return_value={0x1080}):
            path.write_text('re: reconstruct helper\n\nRVA: 0x1080\n')
            workflow.commit_message(path)
            path.write_text('re: reconstruct helper\n')
            with self.assertRaisesRegex(ValueError, 'RVA'):
                workflow.commit_message(path)
            path.write_text('re: reconstruct helper\n\nRVA: 0x1090\n')
            with self.assertRaisesRegex(ValueError, '1080'):
                workflow.commit_message(path)
            path.write_text('random message')
            with self.assertRaisesRegex(ValueError, 'Conventional'):
                workflow.commit_message(path)

    @unittest.skipUnless(shutil.which('pwsh') and shutil.which('uv'), 'PowerShell 7 and uv required')
    def test_real_windows_hook_launchers_and_message_rejection(self):
        self.stage_repo()
        for name in ['workflow.py', 'git_hooks.ps1', 'windows_tracking.py', 'decomp_doctor.py',
                     'generate_dashboard.py', 'verify_fidelity.py', 'db.py']:
            destination = self.root / 'tools' / name
            destination.parent.mkdir(exist_ok=True)
            shutil.copyfile(workflow.ROOT / 'tools' / name, destination)
        shutil.copytree(workflow.ROOT / '.githooks', self.root / '.githooks')
        subprocess.run(['git', '-C', str(self.root), 'add', 'tools', '.githooks'], check=True)
        subprocess.run(['git', '-C', str(self.root), 'config', 'core.hooksPath', '.githooks'], check=True)
        result = subprocess.run(['git', '-C', str(self.root), 'hook', 'run', 'pre-commit'],
                                capture_output=True, text=True, timeout=60)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        message = self.root / 'build/message.txt'
        message.write_text('invalid subject')
        result = subprocess.run(['git', '-C', str(self.root), 'hook', 'run', 'commit-msg', '--', str(message)],
                                capture_output=True, text=True, timeout=60)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('Conventional Commit', result.stdout + result.stderr)


if __name__ == '__main__':
    unittest.main()
