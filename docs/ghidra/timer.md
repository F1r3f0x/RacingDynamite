# Authentic DOS timer reconstruction

Recovered from `Ignition/Ignition/MAINDOS.EXE` through `le_parser.py`, `disasm_le.py`, and `verify_capstone.py`. No Ghidra synchronization: localhost:8080 refused connections. No new structures or intentional game deviations were introduced.

| Function | DOS range | ABI | Behavior |
| --- | --- | --- | --- |
| Timer_Init | 0x10198..0x10236 | Watcom register, void(void) | Program PIT mode 2, latch BIOS epoch, calibrate rotated counter using 100 distinct samples and at least 15 parity changes |
| Timer_GetTime | 0x10238..0x10331 | Watcom register, double(void) | Sample BIOS/PIT/BIOS, reread PIT on BIOS boundary; return elapsed half-ticks; reset previous time on backward clock; cap returned interval at 10.0; update FPS, elapsed seconds and previous timestamp |
| Timer_GetPITCounter | 0x1034c..0x10387 | Watcom register, int(void) | PIT latch 0x04, low/high reads, complement, ROR32 by calibrated count, mask 0xffff |

`App_FrameTick` at 0x10060 calls 0x10238, multiplies its elapsed result by the stored 1000.0 and 1/36 constants, then accumulates into 0xe73a8 and truncates into 0xab5dc. The earlier implementation returned the absolute BIOS clock, causing the caller to accumulate uptime every frame.

Authentic doubles extracted from stored LE object data:

| Address | Value |
| --- | --- |
| 0xa0004 | 1000.0 |
| 0xa000c | 0.027777777777777776 |
| 0xa0014 / 0xa001c | 3.0516646830846227e-05 |
| 0xa0024 | 10.0 |
| 0xa002c | 36.418 |
| 0xa0034 | 0.0274589488714372 |

0xab5c0..0xab5d8 is stored as zero. The original helper at 0x55f9c sets truncation rounding before integer conversion. Preserve the original uncapped FPS divisor and its zero-interval FPU edge behavior, rather than introducing a guard. The portable C conversion at that edge is compiler-dependent; deterministic tests cover backward-clock state and returned delta but do not certify identical exception/indefinite-integer behavior. No deviation toggle is added because no intentional fix is introduced.

The BIOS helper uses C `int386` and CX:DX; no handwritten assembly. Timer code is C89. `tests/test_timer.py` compiles the actual recovered timer functions with w64devkit GCC in C89 mode against deterministic BIOS/PIT inputs, covering fractional intervals, BIOS boundary reread, forward clamp, backward reset, 32-bit rotate, startup epoch and calibration. This validates arithmetic/state behavior, not DOSBox timing fidelity.

Open Watcom V2 compatibility: the local pointer typedef shim now follows Watcom's guards and 32-bit long aliases. Uppercase decompiler ABS/SQRT pseudo-intrinsics were replaced by declared C `fabs`/`sqrt`, eliminating unresolved symbols. Authentic `Lisa_RenderSkyBackdrop` contains FSQRT at 0x58989. These are compiler compatibility corrections; game behavior of the surrounding renderer/FX routines remains unverified.

Post-change Open Watcom V2 build/symbol scan passes; post-change DOSBox playback was not observed because the user stopped Computer Use with physical Escape. The gray/black startup divergence is unresolved pending runtime replay. Do not infer that the timer repair resolves the observed failure.
