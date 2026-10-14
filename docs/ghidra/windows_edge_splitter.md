# Windows top/bottom/right edge clipping and splitting

Authenticated standard IGN_WIN.EXE, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
Gfx_ClipAndSplitTriangleEdge is an analysis name, not a recovered linker symbol.
RVA 0x24F270 / VA 0x64F270, 537 bytes through RET at 0x64F488. The next
instruction belongs to the independently authenticated clamp at 0x64F489;
no padding or FPO boundary is invented. SHA-256
`6c18e8828be874b0f51f9f8e808820be102209247fa3746bbccfae3fed7361c3`.
Local fingerprinted bytes and bounded Ghidra disassembly corroborate provenance
under the user-provided active authentic-program assumption. Original source
filenames are unknown; ownership is the directly compiled renderer module.

Original ESI supplies the mutable eight-word edge packet; there are no stack
arguments or semantic return. It sequentially clips top, bottom and right.
Top interpolation stores V0/U0/X0/Y0, bottom stores V1/U1/X1/Y1. Ratios are
unsigned DIV(distance:0,denominator), then SHR1. Doubled deltas wrap to32 bits;
signed product high words provide interpolation adjustments. Bounds and packet
fields are reread in the instruction order; final Y stores use live bounds.
Divide zero/overflow faults retain prior stores. There is no general safe guard.

After Y clipping it clears the split flag. For both endpoints outside right it
uses captured X distance and live U/V gradients to clamp endpoint1, then endpoint0.
For outside-to-inside crossing it sets the flag and saves X1/Y1/U1/V1 sequentially
before computing the intersection. It updates U1/V1/Y1/X1, then clamps endpoint0.
The originally outside-right branch falls through to a live second X1/right check.
If that check crosses, another split/save/intersection executes. Inside-to-outside
crossing uses the same endpoint order and explicitly bypasses DIV with raw quotient
0xFFFFFFFF when distance==denominator (X0==right); only that branch has this bypass.
The final ratio is 0x7FFFFFFF. No conventional clipping replacement is introduced.

Verified viewport backing is 36 bytes at VA 0x63F2B0: fixed top, integer bottom/top/
right/left, fixed bottom, stride, fixed left/right. It lies in loader-zeroed virtual
data. The existing right scalar migrates into this named object; its C lvalue and
scoped validation-symbol member alias preserve consumers. Validation builders export
the whole object, and the clamp verifier compares the full backing. Split state is
20 file-zeroed bytes at 0x4BAEC8: active flag and saved X1/Y1/U1/V1. Real viewport/
gradient initialization remains external and pending; no invented initializer is added.

Before implementation, 512 original-only expression cases checked 171 splits and
3,692 ordered stores, plus two actual DIV fault cases. Arithmetic diagnostics
compared 1,150 unsigned divisions (582 faults) and 3,049 signed high products to
actual original DIV/IMUL instructions and independent expressions. An additional
executed diagnostic reduces right from30*256 to25*256 after the endpoint0 U store:
the authentic body takes 0x64F404/40F/44C, publishes a split and final packet
(6400,1280,6400,1281,952,1779,953,1780). This proves the live fallthrough path
before production code; it is a perturbation, not claimed original callback behavior.

Readable strict C89 contains the complete body and real viewport/split/gradient
backing. DEV-007 adapts ESI to a typed volatile packet. Unsigned restoring division
and signed halfword high products reproduce CPU arithmetic without wide types,
runtime callees or assembly; invalid ratios reach actual i386 DIV faults under
the verified compiler. Volatile captures keep the exact access order. No native
register/flags/fault-stack equivalence is claimed, and this is not portable C
exception behavior for arbitrary compilers.

`uv run tools/verify_edge_splitter.py` passes 858 whole original-versus-C calls:
340 persistent followups, 177 actual divide/pointer faults, 340 viewport-packet
aliases and the executed live recheck/special-quotient case. Both complete bodies
execute, with no modeled callees. Every ordered non-stack read/write, all arena/
viewport/gradient/split bytes, unchanged unrelated image and caller arguments,
normal return/ESP/preserved registers/DF and fault prefixes are compared. Exception
instructions must be DIV; no synthetic failure result is accepted. Null/truncated
packets, exact bounds, both crossing directions, combined clips and aliased live
bounds are included. Arbitrary split-state/image aliases and concurrency remain open.
Static audit finds only the three real span-to-core calls; the full renderer links
import-free and the splitter introduces no callee. The migrated clamp separately
passes its existing 1,028 comparisons and live mutations.

Full 109-routine renewal, fidelity audit and synchronized exports pass at snapshot
`721d387ecc1d04658a8b8da2c7c9cf3fc302dbf234edf8dbdf39c28729c3e7b3`, with 109 fresh compilation/emulation and 43 existing scoped native records.
All 48 tracking/ownership/workflow tests pass. LF/index gates and tracked hooks
are required before commit; no new native splitter record is manufactured.
RVA 0x24F489 clamp body is unchanged with member-backed right; RVA 0x67F1C setup
body is unchanged with conservative source-context renewal. No new native splitter/
parent result is claimed. Original compiler, native NX/flags/SEH/unwinding and
complete Windows startup/menu/render/input/audio/races remain unverified. Continue
real clipped/ordinary edge wrappers, gradients and complete triangle/sprite/native
frame integration after the separate feature commit. The playable-game goal stays active.
