"""Foreground retries cannot turn behavioral failures into passing evidence."""
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from verify_surface_open import native_probe_process


class NativeProbeProcessTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.runtime = Path(self.temp.name)
        self.expected = 'PASS native timed:'

    def result(self, code, text):
        return subprocess.CompletedProcess(['probe.exe'], code, text, '')

    def test_prerequisite_retry_retains_failure_and_requires_actual_success_marker(self):
        failure = self.result(74, 'FAIL native keyboard foreground retention prerequisite\n')
        success = self.result(0, self.expected + ' all scenarios\n')
        with patch('subprocess.run', side_effect=[failure, success]) as run:
            result, attempts = native_probe_process('probe.exe', self.runtime, self.expected, True)
        self.assertIs(result, success)
        self.assertEqual(run.call_count, 2)
        self.assertEqual([a['exit_code'] for a in attempts], [74, 0])
        self.assertEqual((self.runtime / 'timed_attempt1.log').read_text(encoding='utf-8'), failure.stdout)

    def test_behavior_failure_is_not_retried(self):
        for code in (67, 74):
            with patch('subprocess.run', return_value=self.result(code, 'FAIL buffered key data\n')) as run:
                with self.assertRaises(RuntimeError):
                    native_probe_process('probe.exe', self.runtime, self.expected, True)
            self.assertEqual(run.call_count, 1)

    def test_prerequisite_retries_are_bounded_and_disabled_for_other_probes(self):
        failure = self.result(74, 'FAIL native keyboard foreground setup prerequisite\n')
        for enabled, limit in ((True, 3), (False, 1)):
            with patch('subprocess.run', return_value=failure) as run:
                with self.assertRaises(RuntimeError):
                    native_probe_process('probe.exe', self.runtime, self.expected, enabled)
            self.assertEqual(run.call_count, limit)

    def test_zero_exit_without_success_marker_and_timeout_both_fail(self):
        with patch('subprocess.run', return_value=self.result(0, 'incomplete\n')) as run:
            with self.assertRaises(RuntimeError):
                native_probe_process('probe.exe', self.runtime, self.expected, True)
        self.assertEqual(run.call_count, 1)
        with patch('subprocess.run', side_effect=subprocess.TimeoutExpired('probe.exe', 45)) as run:
            with self.assertRaises(subprocess.TimeoutExpired):
                native_probe_process('probe.exe', self.runtime, self.expected, True)
        self.assertEqual(run.call_count, 1)


if __name__ == '__main__':
    unittest.main()
