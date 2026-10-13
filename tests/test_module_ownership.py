"""Ownership guards catch misplaced and duplicated reconstructed routines."""
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import module_ownership as ownership
from workflow import functions


class OwnershipTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / 'decomp/src').mkdir(parents=True)
        self.module = dict(source='decomp/src/input.c', header='decomp/include/input.h',
                           responsibility='Keyboard input', state=['keyboard'],
                           dependencies=['platform'], placement_reason='Verified input callers',
                           build_mode='direct', original_source=None, symbol_prefixes=['Input_'])
        self.write_manifest()
        (self.root / self.module['source']).write_text('int Input_Foo(void) { return 1; }')
        self.rows = [dict(symbol_name='Input_Foo', source_path=self.module['source'],
                          analysis_stage='reconstructed', rva=0x1000)]

    def write_manifest(self):
        (self.root / ownership.MANIFEST).write_text(json.dumps(
            {'schema_version': 1, 'modules': {'input': self.module}}))

    def test_unique_owned_definition_passes(self):
        ownership.validate(self.root, self.rows, functions)

    def test_duplicate_even_without_annotation_fails(self):
        (self.root / 'decomp/src/other.c').write_text('int Input_Foo(void) { return 1; }')
        with self.assertRaisesRegex(ValueError, 'one definition'):
            ownership.validate(self.root, self.rows, functions)

    def test_source_and_tracking_must_agree_with_owner(self):
        self.rows[0]['source_path'] = 'decomp/src/other.c'
        with self.assertRaisesRegex(ValueError, 'module owns'):
            ownership.validate(self.root, self.rows, functions)

    def test_missing_rule_and_manifest_fail(self):
        self.rows[0]['symbol_name'] = 'Other_Foo'
        with self.assertRaisesRegex(ValueError, 'no module ownership rule'):
            ownership.validate(self.root, self.rows, functions)
        (self.root / ownership.MANIFEST).unlink()
        with self.assertRaisesRegex(ValueError, 'Missing module'):
            ownership.load(self.root)

    def test_paths_and_missing_rationale_rejected(self):
        self.module['source'] = '../outside.c'
        self.write_manifest()
        with self.assertRaisesRegex(ValueError, 'escapes'):
            ownership.load(self.root)
        self.module['source'] = 'decomp/src/input.c'
        self.module['placement_reason'] = ''
        self.write_manifest()
        with self.assertRaisesRegex(ValueError, 'missing placement_reason'):
            ownership.load(self.root)

    def test_preflight_requires_concrete_placement(self):
        args = (self.root, self.rows, [0x1000], [self.module['source']])
        with self.assertRaisesRegex(ValueError, '--module'):
            ownership.placement(*args, [], '')
        report = ownership.placement(*args, ['input'], 'Move verified input and its state')
        self.assertIsNone(report['modules']['input']['original_source'])
        with self.assertRaisesRegex(ValueError, 'Unknown selected'):
            ownership.placement(*args, ['other'], 'reason')
        with self.assertRaisesRegex(ValueError, '--file'):
            ownership.placement(self.root, self.rows, [0x1000], [], ['input'], 'reason')

    def test_unknown_candidate_allows_proposed_owner_but_unknown_rva_fails(self):
        self.rows[0]['symbol_name'] = 'sub_00401000'
        ownership.placement(self.root, self.rows, [0x1000], [self.module['source']], ['input'], 'hypothesis')
        with self.assertRaisesRegex(ValueError, 'Unknown affected'):
            ownership.placement(self.root, self.rows, [0x9999], [self.module['source']], ['input'], 'reason')

    def test_legacy_module_cannot_absorb_new_routine_silently(self):
        self.module['build_mode'] = 'extracted'
        self.module['approved_symbols'] = {'Input_Previous': 'Existing routine pending migration'}
        self.write_manifest()
        with self.assertRaisesRegex(ValueError, 'explicit ownership review'):
            ownership.validate(self.root, self.rows, functions)
        with self.assertRaisesRegex(ValueError, 'explicit ownership review'):
            ownership.placement(self.root, self.rows, [0x1000], [self.module['source']], ['input'], 'reason')

    def test_legacy_approval_needs_a_documented_reason(self):
        self.module['build_mode'] = 'extracted'
        self.module['approved_symbols'] = {'Input_Foo': ''}
        self.write_manifest()
        with self.assertRaisesRegex(ValueError, 'placement reason'):
            ownership.load(self.root)


if __name__ == '__main__':
    unittest.main()
