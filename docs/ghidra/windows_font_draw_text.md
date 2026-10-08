# Font_DrawText — authentic Windows recovery

Recovered independently on 2026-10-07 from `Ignition/Ignition/IGN_WIN.EXE`:
915,968 bytes, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
PE32 x86, preferred base `0x00400000`, CRT entry RVA `0x69950`;
application entry VA `0x004120A0` is independently asserted by
`tools/windows_inspect.py`. Addresses below are preferred VAs unless labeled RVA.
No DOS binary was used. The PE sections, imports, relocation directory and
startup/initializer edges were rechecked with the existing Windows inspector;
compiler/ABI clues remain those documented in `windows_startup.md`.

The localhost:8080 bridge answered read-only methods/segments probes. Inspection
of its local MCP interface found no loaded-program fingerprint endpoint, so its
program identity remains unverified. No Ghidra symbols, pseudocode, synchronization
or mutations were used as evidence. Authoritative disassembly uses pinned pefile
and Capstone against the independently fingerprinted file.

## Extent, signature and return contract

`Font_DrawText`: VA `0x00456660`, RVA `0x00056660`, 978 bytes (`0x3D2`),
extent `[0x00456660,0x00456A32)`. FPO tuple `(978,5,4,5135)` agrees with the
20-byte local frame, four argument dwords and preserved EBX/ESI/EDI/EBP.
Every instruction decodes across the extent; all paths end in a plain `RET`.
Fourteen INT3 padding bytes precede `0x00456A40`. Routine SHA-256:
`f726ad45a7e5cf42d9fa835cdfcdaf008d57a7540556d9e8ff6c044d5e27d147`.

```c
int Font_DrawText(const char *text, int font_id, int x, int y);
```

This is x86 cdecl, with arguments at entry ESP+4/+8/+12/+16. The prologue
loads X into EBX, font index into ESI and retains original X locally. The
representative caller at `0x00403C49` pushes Y=90, X=160, font and text, calls
at `0x00403C52`, then cleans 16 bytes at `0x00403C5B`. Direct FPO-code scans
also found calls at `0x00402B6F`, `0x00403C80`, `0x00418348`, `0x004385FD`
and `0x00441314` (among many UI/HUD callers); the verifier checks these sites.
The scan is not a complete non-FPO call inventory.

The signed `font_id >= 30` check branches immediately to return 1040 at
`0x00456A25`; otherwise `in_use == 0` does likewise. There is no lower guard.
Any nonzero in-use value is accepted. A nonzero `field_08` returns 2. Both
guards precede text access. Neither error path writes `g_fileErrorLine`.
Valid drawing, including an empty string, returns 1 regardless of renderer return.
No font/global/text writes originate in this routine.

## Recovered behavior

Windows slot stride is 1600 bytes, independently computed by LEA/SHL in this
routine. It reads alignment at +4, mode at +8, proportional flag at +12,
extra spacing at +16, signed height at +26, presence at +30, handle dwords at
+256 and signed widths at +1152. Natural x86 layout agrees with the production
header. Height and widths are stored as unsigned words but interpreted as signed
shorts at their load sites. Characters use `MOVSX` from a byte; presence must
equal exactly 1. The signed character minus 32 index is retained for presence,
width and handle reads. High-bit characters can therefore read earlier header
or slot storage. The C reconstruction addresses the containing table rather than
an out-of-range glyph subarray. First-slot accesses before the table and negative
font indices remain outside the defined C object contract.

At `0x004566A9–0x004566C1`, signed height times 256 is converted through x87,
multiplied by binary64 0.35 and passed to original `_ftol` (`0x0046950C`).
The result is discarded. There is no minimum-space clamp. A volatile scratch
in C retains this otherwise dead calculation. Original `_ftol` saves/restores
the control word and truncates toward zero. Proportional absent spaces calculate
`(height * 256) * 0.35 * (1/256)` without extra spacing. Extended x87 precision
(`0x037F`) gives 6 for height 20; double precision (`0x027F`) gives 7. The
validation compiler uses x87, and both caller precision settings are covered.
FP exception flags/traps, other rounding/precision settings and native FP state
are not certified.

The proportional branch is selected once, before either scan. If alignment is
nonzero, a measurement pass runs before drawing. Fixed fonts accumulate signed
height plus extra spacing for present glyphs and spaces; other missing glyphs
contribute zero. Proportional fonts accumulate signed width plus extra spacing
for present glyphs, with the absent-space rule above. Presence takes precedence
over the space fallback. Both scans reevaluate `strlen` each iteration and test
the current byte for zero.

Alignment is reread after measuring: 1 computes original X plus the signed,
truncated half of **wrapped original-X minus measured-end-X**; 2 computes
twice original X minus measured end X; all other values restore original X.
The subtraction order matters at signed-overflow boundaries. Unsigned C
arithmetic preserves 32-bit wrap, and conversion back to signed x86 integers
retains the original displacement interpretation.

Fixed glyphs add `(signed_height - signed_width) / 2`, truncated toward zero,
only to the drawing position. Proportional glyphs use the cursor directly.
X and Y positions are shifted left 8 with 32-bit wrap into a local `Point2D`.
Calls are ordered in text order. Metrics are reread **after** each rendering
call to advance the cursor; presence is not rechecked for that character.
Renderer mutation may affect subsequent glyphs and the repeated length scan.
The reconstructed shared two-pass loop preserves these observations; EXACT
denotes recovered behavior in the stated scope, not instruction equality.

## Relocation and dependency evidence

There are exactly 33 HIGHLOW operands in the routine. Operand sites below are
**RVAs**, while targets are preferred **VAs**; `verify_font_draw.py` asserts the
complete ordered list, preserving identities rather than stripping operands.

| Operand RVA(s) | Target VA | Meaning |
| --- | --- | --- |
| 0x56688 | 0x0063F2E0 | slot in-use |
| 0x56695 | 0x0063F2E8 | slot mode |
| 0x566AC, 0x56711, 0x5672B, 0x567C8, 0x56830, 0x568D0, 0x569D8 | 0x0063F2FA | height |
| 0x566BD, 0x568E1, 0x569E9 | 0x0047AEC8 | binary64 0.35 |
| 0x566C8 | 0x0063F2EC | proportional flag |
| 0x566D5, 0x56750, 0x5686B, 0x5690E | 0x0063F2E4 | alignment |
| 0x56707, 0x567BB, 0x568A7, 0x56979 | 0x0063F2DE | presence base minus 32 |
| 0x56717, 0x56731, 0x56836, 0x568C3, 0x569CB | 0x0063F2F0 | extra spacing |
| 0x567DA, 0x568BD, 0x569C5 | 0x0063F720 | width base minus 64 |
| 0x5681A, 0x569A9 | 0x0063F360 | handle base minus 128 |
| 0x568E7, 0x569EF | 0x0047AED0 | binary64 1/256 |

Calls at `0x004566C1`, `0x004568EB`, `0x004569F3` target `_ftol`.
Calls at `0x0045681F` and `0x004569AE` target `0x00456D20`.
This 25-byte native dispatch wrapper reads three stack arguments, pushes them
unchanged, calls through `0x0050EBBC`, cleans 12 bytes and returns its EAX.
Its code hash is
`e04def2288d23041976ef0a043cea2b296558f74cf82cce58476bf765e804623`.
The verified font call contract is `(glyph_handle, Point2D *, NULL)`; the third
argument is a transform pointer, not an integer flags word.
The production font client retains the existing void/int declaration solely for
its all-zero third word and ignored EAX; the header documents this null-only ABI
use. No non-null transform call or general renderer reconstruction is certified.

The initializer instruction at `0x00456EA6` sets `0x0050EBBC = 0x004571B0`.
This backend's FPO extent is 145 bytes; hash
`f2fe1b07e3a37fe080d3cfcaeadad8e3bae3d2ca326e16c4a99e6ac563e01dac`.
Its three-argument cdecl prologue stores the point pointer at `0x004BA708`,
handle at `0x004BA70C`, and clears transform state at `0x004BA710`.
A non-null third argument is dereferenced as floating-point data; fonts pass null
and skip that path. The backend then stores `0x004BA708` at `0x0050EBFC`,
calls `0x00457370`, and returns 1. These are static boundary observations,
not a recovered complete rasterizer contract. No dependency body is added or
promoted into reconstruction tracking in this feature.

## Validation and limits

Run `uv run tools/verify_font_draw.py` alone, or `uv run tools/verify_matching.py`
for the integrated run. Both use pinned pefile 2024.8.26, Capstone 5.0.7 and
Unicorn 2.1.4. The harness extracts the actual production `Font_DrawText`,
includes the production header and asserts slot/field layout. Clang/LLD 19.1.1
compile/link a fresh strict-C89 x86 DLL, with x87, O2, freestanding/no builtins
and no vectorization. Tool versions/commands and raw input/artifact hashes are
persisted separately from the emulation result. The original compiler version
and optimization settings remain unresolved.

The original `_ftol` and native dispatch wrapper execute. The renderer is
intercepted at `0x004571B0`, and the compiled external symbol is intercepted
at the equivalent cdecl boundary. A separately compiled infinite-loop trap
provides its address; it never executes or returns success, and is not production
code. The harness supplies storage, a real C string-length implementation and
the MSVC `_fltused` linker marker. The existing legacy `globals.c` sprite stub
is not linked or used. Full-module compilation remains blocked by legacy `io.h`
and other dependencies; this feature does not migrate them.

1,543 original-versus-compiled-C comparisons passed: all 30 valid font slots,
nonzero flag variants, all alignment branches, empty/early-error paths, exact
presence variants, signed metrics/characters, odd fixed insets, extreme/wrapping
coordinates, both x87 precision modes and randomized cases. Forty-two explicit
renderer mutation cases change metrics, presence, handle, mode or text after a
call and check later calls and final state. Each comparison checks EAX, ordered
handle/point/transform arguments, boundary table/text snapshots, modeled changes,
image immutability outside those changes, caller stack/arguments, saved registers,
DF and restored x87 control word. Three additional original-only negative-slot
cases confirm the missing lower guard, separately from the valid C object contract.

Compilation and modeled-boundary differential emulation passed. Raw code equality,
relocation-aware instruction equality, renderer-body execution, framebuffer output,
native visual/runtime behavior and a playable rebuilt game are not established.
The next evidence-supported candidate is the native sprite backend at VA
`0x004571B0` / RVA `0x571B0`; recover its downstream `0x00457370` contract
before implementation. SQLite and generated exports are updated by the real
workflow completion command; their snapshot ID is reported in its handoff.
