"""Non-FPO import guards using isolated fingerprinted PE/SQLite fixtures."""
import contextlib
import io
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
import db
import windows_tracking as tracking
from test_windows_tracking import fixture


class CandidateTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        fixture(self.root)
        tracking.migrate(self.root)
        for name, value in (
                ('ROOT', self.root),
                ('load_target', lambda: tracking.load_target(self.root)),
                ('connect', lambda writable=False: tracking.connect(self.root, writable=writable))):
            patcher = patch.object(db, name, value)
            patcher.start()
            self.addCleanup(patcher.stop)

    def invoke(self, *args):
        with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
            return db.main(['add-candidate', *args, '--confidence', 'test evidence',
                '--evidence', 'docs/ghidra/evidence.md'])

    def test_import_hash_stage_and_duplicate_preservation(self):
        self.assertEqual(self.invoke('0x10a0', '--size', '8'), 0)
        with tracking.connect(self.root) as conn:
            row = conn.execute('SELECT * FROM functions WHERE rva=4256').fetchone()
            self.assertEqual(row['routine_sha256'], tracking.sha(bytes(8)))
            self.assertEqual(row['analysis_stage'], 'unidentified')
            self.assertEqual(row['fidelity'], 'unknown')
            self.assertEqual(row['extent_origin'], 'authenticated-file-analysis')
        before = (self.root/'database/decomp.db').read_bytes()
        self.assertEqual(self.invoke('0x10a0', '--size', '12'), 1)
        self.assertEqual((self.root/'database/decomp.db').read_bytes(), before)

    def test_unknown_extent_does_not_claim_routine_hash(self):
        self.assertEqual(self.invoke('0x10a0'), 0)
        with tracking.connect(self.root) as conn:
            row = conn.execute('SELECT * FROM functions WHERE rva=4256').fetchone()
            self.assertIsNone(row['byte_size'])
            self.assertIsNone(row['routine_sha256'])

    def test_invalid_extent_and_fingerprint_do_not_mutate_store(self):
        before = (self.root/'database/decomp.db').read_bytes()
        for args in (('0x11ff','--size','2'), ('-1','--size','4'), ('0x10a0','--size','0'), ('0x3000',)):
            self.assertEqual(self.invoke(*args), 1)
        binary = self.root/'Ignition/Ignition/IGN_WIN.EXE'
        binary.write_bytes(binary.read_bytes()[:-1]+b'\xff')
        self.assertEqual(self.invoke('0x10a0','--size','8'), 1)
        self.assertEqual((self.root/'database/decomp.db').read_bytes(), before)

    def test_nonexecutable_section_requires_explicit_opt_in(self):
        candidates, sections = tracking.pe_inventory((self.root/'Ignition/Ignition/IGN_WIN.EXE').read_bytes())
        sections[0]['executable'] = False
        with patch.object(db, 'pe_inventory', return_value=(candidates, sections)):
            self.assertEqual(self.invoke('0x10a0'), 1)
            self.assertEqual(self.invoke('0x10a0', '--allow-nonexecutable'), 0)

    def describe(self, *args):
        with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
            return db.main(['describe', '0x10a0', *args])

    def test_describe_authenticates_unknown_extent_without_promoting_stage(self):
        self.assertEqual(self.invoke('0x10a0'), 0)
        self.assertEqual(self.describe('--size','8','--evidence','docs/ghidra/evidence.md',
            '--confidence','closed control flow'), 0)
        with tracking.connect(self.root) as conn:
            row = conn.execute('SELECT * FROM functions WHERE rva=4256').fetchone()
            self.assertEqual(row['byte_size'],8)
            self.assertEqual(row['routine_sha256'],tracking.sha(bytes(8)))
            self.assertEqual(row['analysis_stage'],'unidentified')
            self.assertEqual(row['extent_origin'],'authenticated-file-analysis')
        before = (self.root/'database/decomp.db').read_bytes()
        self.assertEqual(self.describe('--size','12','--evidence','docs/ghidra/evidence.md',
            '--confidence','replacement'),1)
        self.assertEqual((self.root/'database/decomp.db').read_bytes(),before)

    def test_describe_extent_guards_are_atomic(self):
        self.assertEqual(self.invoke('0x10a0'),0)
        before = (self.root/'database/decomp.db').read_bytes()
        required = ['--evidence','docs/ghidra/evidence.md','--confidence','test']
        for args in (['--size','0',*required], ['--size','8'],
                ['--size','1024',*required], ['--size','8','--evidence','missing.md','--confidence','test']):
            self.assertEqual(self.describe(*args),1)
            self.assertEqual((self.root/'database/decomp.db').read_bytes(),before)
        binary = self.root/'Ignition/Ignition/IGN_WIN.EXE'
        binary.write_bytes(binary.read_bytes()[:-1]+b'\xff')
        self.assertEqual(self.describe('--size','8',*required),1)
        self.assertEqual((self.root/'database/decomp.db').read_bytes(),before)

    def test_describe_nonexecutable_extent_requires_opt_in(self):
        self.assertEqual(self.invoke('0x10a0'),0)
        candidates, sections = tracking.pe_inventory((self.root/'Ignition/Ignition/IGN_WIN.EXE').read_bytes())
        sections[0]['executable'] = False
        args = ['--size','8','--evidence','docs/ghidra/evidence.md','--confidence','test']
        with patch.object(db,'pe_inventory',return_value=(candidates,sections)):
            self.assertEqual(self.describe(*args),1)
            self.assertEqual(self.describe(*args,'--allow-nonexecutable'),0)


if __name__ == '__main__':
    unittest.main()
