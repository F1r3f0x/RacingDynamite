"""Meaningful identity/header rejection tests for the read-only doctor."""
import hashlib
import importlib.util
import struct
import tempfile
import unittest
from pathlib import Path

spec = importlib.util.spec_from_file_location('decomp_doctor', Path(__file__).resolve().parents[1] / 'tools/decomp_doctor.py')
doctor = importlib.util.module_from_spec(spec)
spec.loader.exec_module(doctor)


def fixture():
    data = bytearray(512)
    data[:2] = b'MZ'
    struct.pack_into('<I', data, 60, 64)
    data[64:68] = b'PE\0\0'
    struct.pack_into('<HH', data, 68, 0x14c, 1)
    struct.pack_into('<H', data, 84, 224)
    struct.pack_into('<H', data, 88, 0x10b)
    struct.pack_into('<I', data, 104, 0x1234)
    struct.pack_into('<I', data, 116, 0x400000)
    return data


class DoctorTests(unittest.TestCase):
    def test_entry_va_and_format(self):
        self.assertEqual(doctor.pe_summary(fixture())['entry_va'], '0x00401234')

    def test_truncated_and_wrong_format_rejected(self):
        for data in (b'', fixture()[:120], b'XX' + fixture()[2:]):
            with self.subTest(length=len(data)), self.assertRaises(ValueError):
                doctor.pe_summary(data)
        data = fixture()
        struct.pack_into('<H', data, 88, 0x20b)
        with self.assertRaises(ValueError):
            doctor.pe_summary(data)

    def test_same_headers_different_content_fails_identity(self):
        data = fixture()
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            binary = root / 'game.exe'
            binary.write_bytes(data)
            manifest = {**doctor.pe_summary(data), 'binary': 'game.exe', 'size': len(data),
                        'sha256': hashlib.sha256(data).hexdigest()}
            self.assertTrue(doctor.inspect_target(root, manifest)['verified'])
            data[-1] ^= 1
            binary.write_bytes(data)
            self.assertEqual(doctor.inspect_target(root, manifest)['mismatches'], ['sha256'])

    def test_path_escape_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaises(ValueError):
                doctor.inspect_target(Path(directory), {'binary': '../outside.exe'})


if __name__ == '__main__':
    unittest.main()
