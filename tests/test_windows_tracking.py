"""Exercise migration, independent evidence freshness and export consistency in isolated fixtures."""
import json
import sqlite3
import struct
import sys
import tempfile
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import windows_tracking as tracking


def fixture(root):
    for directory in ['database','decomp/src','docs/tracking','docs/ghidra','Ignition/Ignition']:
        (root/directory).mkdir(parents=True,exist_ok=True)
    (root/'database/schema.sql').write_bytes((tracking.ROOT/'database/schema.sql').read_bytes())
    data=bytearray(1024)
    data[:2]=b'MZ';struct.pack_into('<I',data,60,64);data[64:68]=b'PE\0\0'
    struct.pack_into('<HH',data,68,0x14c,1);struct.pack_into('<H',data,84,224)
    opt=88;struct.pack_into('<H',data,opt,0x10b);struct.pack_into('<I',data,opt+16,0x1080)
    struct.pack_into('<I',data,opt+28,0x400000);struct.pack_into('<I',data,opt+92,16)
    struct.pack_into('<II',data,opt+144,0x1000,28)
    section=312;data[section:section+8]=b'.text\0\0\0'
    struct.pack_into('<IIII',data,section+8,512,0x1000,512,512)
    struct.pack_into('<I',data,section+36,0x60000020)
    struct.pack_into('<IIII',data,512+12,3,32,0x1040,576)
    struct.pack_into('<IIIHH',data,576,0x1080,8,0,0,0)
    struct.pack_into('<IIIHH',data,592,0x1090,8,0,0,0)
    data[640:648]=b'\xb8\x01\x00\x00\x00\xc3\xcc\xcc'
    data[656:664]=b'\x31\xc0\xc3\xcc\xcc\xcc\xcc\xcc'
    (root/'Ignition/Ignition/IGN_WIN.EXE').write_bytes(data)
    manifest={'schema_version':1,'binary':'Ignition/Ignition/IGN_WIN.EXE','sha256':tracking.sha(data),
              'size':len(data),'machine':'0x014c','optional_magic':'0x010b','image_base':'0x00400000','entry_rva':'0x00001080'}
    (root/'decomp/target.json').write_text(json.dumps(manifest))
    fn={'symbol':'Foo','rva':'0x00001080','va':'0x00401080','size':8,
        'routine_sha256':tracking.sha(data[640:648]),'module':'decomp/src/mem.c',
        'abi':'cdecl','extent_confidence':'corroborated','evidence':'docs/ghidra/evidence.md',
        'c_reconstructed':True,'validation':'old reported 30-case result'}
    (root/'docs/tracking/windows_inventory.json').write_text(json.dumps({'target':manifest,'functions':[fn]}))
    (root/'decomp/src/mem.c').write_text('// @original Foo (IGN_WIN.EXE @ 0x00401080, unknown)\n// @fidelity EXACT\nint Foo(void) { return 1; }\n')
    (root/'docs/ghidra/evidence.md').write_text('Independent bounded evidence')
    with sqlite3.connect(root/'database/decomp.db') as conn:
        conn.executescript("CREATE TABLE metadata(key TEXT,value TEXT); INSERT INTO metadata VALUES('target_exe','MAINDOS.EXE'); CREATE TABLE functions(dos_address TEXT,status TEXT); INSERT INTO functions VALUES('0x10000','matching');")
    conn.close()
    return data


class TrackingTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup)
        self.root=Path(self.temp.name);self.data=fixture(self.root)

    def test_migration_resets_dos_and_preserves_verified_windows_evidence(self):
        self.assertTrue(tracking.migrate(self.root))
        snap=tracking.snapshot(self.root)
        self.assertEqual(snap['summary']['functions'],2)
        self.assertEqual(snap['summary']['stages']['reconstructed'],1)
        self.assertEqual(snap['summary']['current_passes']['emulation'],0)
        self.assertEqual(snap['functions'][0]['imported_evidence']['validation'],'old reported 30-case result')
        self.assertIsNone(snap['functions'][1]['abi'])
        self.assertEqual(snap['functions'][1]['fidelity'],'unknown')
        self.assertFalse(tracking.migrate(self.root))

    def test_fingerprint_failure_leaves_existing_db_untouched(self):
        old=(self.root/'database/decomp.db').read_bytes()
        target=self.root/'Ignition/Ignition/IGN_WIN.EXE';target.write_bytes(self.data[:-1]+b'\x01')
        with self.assertRaises(ValueError):tracking.migrate(self.root)
        self.assertEqual(old,(self.root/'database/decomp.db').read_bytes())

    def test_seed_claim_mismatch_rejected_atomically(self):
        path=self.root/'docs/tracking/windows_inventory.json';seed=json.loads(path.read_text());seed['functions'][0]['size']=9;path.write_text(json.dumps(seed))
        with self.assertRaises(ValueError):tracking.migrate(self.root)
        with sqlite3.connect(self.root/'database/decomp.db') as conn:
            self.assertEqual(conn.execute('SELECT status FROM functions').fetchone()[0],'matching')
        conn.close()

    def test_duplicate_or_truncated_fpo_rejected(self):
        duplicate=bytearray(self.data);struct.pack_into('<I',duplicate,592,0x1080)
        with self.assertRaises(ValueError):tracking.pe_inventory(duplicate)
        with self.assertRaises(ValueError):tracking.pe_inventory(self.data[:600])

    def test_latest_result_and_source_artifact_changes_invalidate_evidence(self):
        tracking.migrate(self.root)
        artifact=self.root/'artifact.dll';artifact.write_bytes(b'fresh object')
        kwargs={'inputs':['decomp/src/mem.c'],'artifact':'artifact.dll','cases':30,'command':'fixture test','root':self.root}
        tracking.record_run(0x1080,'emulation','pass',**kwargs)
        self.assertEqual(tracking.snapshot(self.root)['summary']['current_passes']['emulation'],1)
        artifact.write_bytes(b'changed')
        self.assertEqual(tracking.snapshot(self.root)['summary']['current_passes']['emulation'],0)
        tracking.record_run(0x1080,'emulation','pass',**kwargs)
        source=self.root/'decomp/src/mem.c';source.write_text(source.read_text()+'// changed\n')
        self.assertEqual(tracking.snapshot(self.root)['summary']['current_passes']['emulation'],0)
        tracking.record_run(0x1080,'emulation','fail',**kwargs)
        self.assertEqual(tracking.snapshot(self.root)['summary']['current_passes']['emulation'],0)
        tracking.audit(self.root)  # Superseded stale runs remain history, not active failures.

    def test_audit_accepts_multiple_candidates_and_rejects_bad_provenance(self):
        tracking.migrate(self.root);self.assertEqual(tracking.audit(self.root)['summary']['functions'],2)
        source=self.root/'decomp/src/mem.c';source.write_text(source.read_text().replace('00401080','00401081'))
        with self.assertRaises(ValueError):tracking.audit(self.root)

    def test_second_reconstructed_function_is_supported_and_fidelity_checked(self):
        tracking.migrate(self.root)
        source=self.root/'decomp/src/mem.c'
        source.write_text(source.read_text()+'// @original Bar (IGN_WIN.EXE @ 0x00401090, unknown)\n// @fidelity EXACT\nint Bar(void) { return 0; }\n')
        with tracking.connect(self.root,writable=True) as conn:
            conn.execute("UPDATE functions SET symbol_name='Bar',analysis_stage='reconstructed',source_path='decomp/src/mem.c',fidelity='EXACT',evidence_path='docs/ghidra/evidence.md' WHERE rva=4240")
        self.assertEqual(tracking.audit(self.root)['summary']['stages']['reconstructed'],2)
        source.write_text(source.read_text().replace('@fidelity EXACT','@fidelity ADAPTED'))
        with self.assertRaises(ValueError):tracking.audit(self.root)

    def test_generated_exports_share_snapshot_and_drift_check_is_read_only(self):
        tracking.migrate(self.root);snap=tracking.export(self.root)
        self.assertEqual((self.root/'dashboard.html').read_bytes(),(self.root/'docs/dashboard.html').read_bytes())
        self.assertEqual(json.loads((self.root/'docs/tracking/windows_inventory.json').read_text())['snapshot_id'],snap['snapshot_id'])
        tracking.export(self.root,check=True)
        path=self.root/'dashboard.html';path.write_text(path.read_text()+'tampered')
        changed=path.read_bytes()
        with self.assertRaises(ValueError):tracking.export(self.root,check=True)
        self.assertEqual(path.read_bytes(),changed)

    def test_sql_dump_restores_same_tracking_data(self):
        tracking.migrate(self.root);tracking.export(self.root)
        with sqlite3.connect(':memory:') as conn:
            conn.executescript((self.root/'database/dump.sql').read_text())
            self.assertEqual(conn.execute('SELECT count(*) FROM functions').fetchone()[0],2)
            self.assertEqual(conn.execute("SELECT value FROM metadata WHERE key='schema_version'").fetchone()[0],'2')
        conn.close()

    def test_union_does_not_double_count_overlap(self):
        self.assertEqual(tracking.union_bytes([(10,10),(15,10),(40,5)]),20)


if __name__=='__main__':unittest.main()
