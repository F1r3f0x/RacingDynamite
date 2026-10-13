# Windows right-endpoint edge clamp

Authenticated standard IGN_WIN.EXE, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
Gfx_ClampTriangleEdgeRightEndpoint is an analysis name, not a recovered symbol.
RVA 0x24F489 / VA 0x64F489, 63 bytes through RET at 0x64F4C7; no FPO.
SHA-256 `65b2cb3d21d75ff2be58a259eb79e9430f6b01539cb992e6722b2c680aeb27f0`.
The original code section lacks an execute flag; native NX behavior is unverified.
Local bytes and bounded Ghidra disassembly corroborate provenance under the
user-provided active authentic-program assumption. Actual callers are
0x64F072/0x64F192 in corrected/raw clipped-edge wrappers.

ESI supplies a mutable 32-byte edge packet: signed x0/y0/x1/y1 and texture
u0/v0/u1/v1 words. The helper has no stack arguments and no semantic return.
It reads x1, then signed fixed-point right bound at VA 0x63F2D0, returning
without stores when right>=x1. Otherwise it reloads x1 and right to capture
wrapped distance. It reads U gradient at 0x4BADA4, multiplies signed32 distance
by signed32 gradient, selects product bits16..47, and subtracts that low32
correction from u1. After this store it reads V gradient at 0x4BADA8 and performs
the same operation on v1. Finally it rereads the right bound and stores x1.
Distance stays captured even if later state changes. All arithmetic wraps as observed.

Gradient words are verified contiguous file-zeroed storage; right bound lies in
loader-zeroed virtual data. Real gradient/viewport initialization is external and
pending reconstruction. No invented initializer, guard or synthetic callee is
introduced. Before implementation, 256 original-only cases checked ordered stores,
signed/wrapped products and ABI, including 109 clamps and 22 diagnostic post-U
mutations establishing the live V-gradient and final-right reloads.

Production C89 lives in the directly owned renderer module. DEV-007 adapts the
ESI packet input to a typed volatile C pointer. Named fields preserve the packet
layout and actual shared state. Unsigned halfword partial products plus sign
corrections compute the exact signed product bits without 64-bit C types or
runtime arithmetic callees. An initial wide-product form could not compile with
the project's intentionally 32-bit C89 integer header; the halfword form resolves
that constraint without changing shared headers or introducing a compiler extension.
Signed comparison and packet/state loads remain explicitly ordered. GCC initially
folded the guard into reversed reads; volatile local captures now retain original
order. The exact access assertion remains enabled.

`uv run tools/verify_edge_clamp.py` passes 1,028 actual original-versus-C calls,
512 persistent followups, 218 clamps, 40 executed diagnostic live-state mutations
and four faults. Complete original instructions and whole production renderer
execute with no modeled callees. Ordered non-stack reads/writes, all packet/arena
bytes, gradient/right state, unrelated image preservation, return/ESP/arguments,
ordinary preserved registers and direction flag are checked. Extreme signed bounds,
equal/outside/inside cases, random product bits, null packet and partial/unmapped
texture fields at arena boundaries are covered. The mutation hook is an explicit
diagnostic perturbation, not claimed original callback behavior or native proof.

The strict C89 renderer compiles and links import-free. No new native clamp/parent
result is claimed; existing span/slope native checks renew separately in the full
workflow. Arbitrary image/state aliases, scratch-register/flags equality, native
fault atomicity and complete triangle/game startup/menu/input/audio/races remain
unverified. Original compiler/link configuration is unresolved. The current wide
division path for initializer RVA 0x24F4D0 lacks verified installed 32-bit helpers;
see [slope diagnostics](windows_triangle_slope.md). That blocker does not prevent
this actual multiplication/clamp reconstruction or other useful renderer work.

Full 107-routine renewal, fidelity audit and synchronized exports pass at snapshot
`4d01660de828bf27a223ea48b82751830e815a642e05dfef20cd9d8ce399c118`, with 107 fresh compilation/emulation and 43 existing scoped native records.
All 48 tracking/ownership/workflow tests pass. LF/index gates and tracked hooks
are required before commit; no native clamp record is manufactured.
RVA 0x67F1C setup body is unchanged; preceding declarations/macros require
conservative source-context renewal. Continue real edge initialization, clipping
and gradient dependencies after the separate feature commit. The goal remains a
playable rebuilt Windows game with documented native behavioral validation.

Subsequent ignored arithmetic diagnostics establish a viable runtime-free next
route: 2,200 C89 paired-word comparisons against the authentic IDIV instruction
pass, including 102 divide faults, over the initializer numerator forms. An
out-of-domain signed64 MIN/-1 direct-instruction probe terminated its interpreter
process and is excluded explicitly; success requires actual instruction advancement.
Fresh engines avoid repeated-exception latch effects. The original complete
initializer separately passes 512 analytical cases, including 318 real faults.
Neither diagnostic is promoted to a production initializer result.
