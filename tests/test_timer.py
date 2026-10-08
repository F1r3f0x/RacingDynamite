"""Execute the recovered C timer against deterministic BIOS/PIT inputs."""
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

class TimerTests(unittest.TestCase):
    def test_recovered_timer_clock_rollover_clamp_rotate_and_calibration(self):
        compiler = shutil.which('gcc')
        self.assertIsNotNone(compiler, 'w64devkit gcc is required for this deterministic C harness')
        source = (ROOT/'decomp/src/main.c').read_text()
        timer = source[source.index('/* Authentic timer state:'):]
        harness = r'''
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

union REGS { struct { unsigned short ax, bx, cx, dx; } w; };
static unsigned long bios_ticks[2];
static int bios_index;
static unsigned long pit_counts[2];
static int pit_index, pit_byte, calibrating;
static unsigned long calibration_count;
static int mode_writes;
static void int386(int interrupt, union REGS *input, union REGS *output) {
    unsigned long ticks;
    assert(interrupt == 0x1a && input->w.ax == 0);
    ticks = bios_ticks[bios_index++ % 2];
    output->w.cx = (unsigned short)(ticks >> 16);
    output->w.dx = (unsigned short)ticks;
}
static int outp(int port, int value) {
    if (port == 0x43 && value == 4) pit_byte = 0;
    if (port == 0x43 && value == 0x34) mode_writes++;
    return value;
}
static int inp(int port) {
    unsigned long count, down;
    assert(port == 0x40);
    count = calibrating ? calibration_count : pit_counts[pit_index % 2];
    down = 0xffffUL - count;
    if (pit_byte++ == 0) return (int)(down & 255);
    if (calibrating) calibration_count++;
    else pit_index++;
    return (int)((down >> 8) & 255);
}
'''
        checks = r'''
static void clock_inputs(unsigned long first, unsigned long later, unsigned long p1, unsigned long p2) {
    bios_ticks[0] = first; bios_ticks[1] = later; bios_index = 0;
    pit_counts[0] = p1; pit_counts[1] = p2; pit_index = 0;
}
int main(void) {
    double expected, result;
    assert(sizeof(unsigned long) == 4);
    g_TimerShiftScale = 0;
    g_TimerPreviousTime = 200.0;
    g_TimerEpoch = 190;
    clock_inputs(100, 100, 32768, 0);
    result = Timer_GetTime();
    expected = 32768.0 * 3.0516646830846227e-05;
    assert(fabs(result - expected) < 1e-12);
    assert(g_TimerFrameRate == (int)(36.418 / expected));
    assert(fabs(g_TimerElapsedSeconds - (10.0+expected)*0.0274589488714372) < 1e-12);
    assert(g_TimerPreviousTime == g_TimerCurrentTime);
    g_TimerPreviousTime = 200.0;
    clock_inputs(100, 101, 65535, 3);
    assert(fabs(Timer_GetTime() - (2.0+3.0*g_TimerBaseScale)) < 1e-12);
    g_TimerPreviousTime = 0.0;
    clock_inputs(100, 100, 0, 0);
    assert(Timer_GetTime() == 10.0);
    g_TimerPreviousTime = 1000.0;
    clock_inputs(1, 1, 0, 0);
    assert(Timer_GetTime() == 0.0);
    assert(g_TimerPreviousTime == 2.0);
    g_TimerShiftScale = 17;
    clock_inputs(0, 0, 65535, 0);
    assert(Timer_GetPITCounter() == 0x8000);
    calibrating = 1; calibration_count = 1;
    g_TimerShiftScale = 0;
    clock_inputs(100, 100, 0, 0);
    Timer_Init();
    assert(mode_writes == 1);
    assert(g_TimerEpoch == 200);
    assert(g_TimerShiftScale == 0);
    assert(calibration_count == 103);
    puts("PASS: elapsed delta, fractional PIT, BIOS rollover, clamp, backward clock, ROR, epoch and calibration");
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp)/'timer_test.c'
            exe = Path(tmp)/'timer_test.exe'
            path.write_text(harness+timer+checks)
            compiled = subprocess.run([compiler, '-std=c89', '-pedantic', '-Wall', '-Wextra', '-Werror', '-Wno-unused-parameter', str(path), '-lm', '-o', str(exe)], capture_output=True, text=True)
            self.assertEqual(compiled.returncode, 0, compiled.stdout+compiled.stderr)
            result = subprocess.run([str(exe)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)

if __name__ == '__main__':
    unittest.main()
