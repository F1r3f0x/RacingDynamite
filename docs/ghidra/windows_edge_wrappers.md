# Windows clipped corrected and raw edge wrappers

Authenticated standard IGN_WIN.EXE SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
Analysis names Gfx_BuildClippedCorrectedTriangleEdge and
Gfx_BuildClippedRawTriangleEdge are not recovered linker symbols.
RVA 0x24EFA0 / VA 0x64EFA0 and RVA 0x24F0C0 / VA 0x64F0C0 each span
282 bytes through RET at 0x64F0B9 / 0x64F1D9. Hashes respectively:
`e10f563c0f702038ca423f24d915feeb909d74f16dad34b9eba4581dbe78209c`
and `481a1b8d63fbc07baff6bb008b9916fe07f4e5f7cc1c5a0cb7808e4f7c08fbb2`.
Local fingerprinted bytes and bounded Ghidra instructions corroborate provenance
under the user-provided active-program assumption. Original source files and
compiler remain unknown. Renderer owns these directly compiled C89 routines.

Original ESI is the mutable eight-word packet and EBX the two-word header,
followed by 12-byte V/U/X records. Only EBP is preserved. DEV-007 supplies typed
packet/header parameters; incidental register/flags outputs are not claimed.
Top rejection writes count zero before loading/storing integer top; bottom
rejection likewise uses integer bottom. Rejections leave split state untouched.
Accepted packets execute the real splitter, publish header+8, then read Y0/store
start and read Y1/store count. JE skips only zero: negative counts execute real
initializer and emitter and retain mandatory first records. Header count is
reloaded after initialization; initial V/X/U loads retain original order.

After first emission, advanced output is published. Only live split flag==1
continues. X1/Y1/U1/V1 copy sequentially to endpoint0, saved endpoint1 fields
restore sequentially, and the real right clamp executes. Y1 is read before Y0.
The second count is published to separate corrected/raw globals; nonzero counts
are added to the header before initialization, then reloaded from shared state.
The second emitter uses the live advanced pointer and does not republish it.
Verified 16-byte file-zeroed continuation backing at VA0x4BAEDC contains corrected
count, an intervening word, raw advanced pointer, and raw count. Existing 32-byte
step state owns the corrected advanced pointer. No invented capacity guard exists.

Preimplementation original-only execution covered 128 clipped calls and 2,954
records plus two negative clipped cases (header start20/count-10, one record).
`uv run tools/verify_edge_wrappers.py` now passes 1,284 whole original/C calls,
432 persistent followups, 568 packet/output alias calls and 420 actual pointer
faults. Both full wrappers execute actual splitter, initializer, clamp and emitter
bodies. All ordered non-stack reads/writes, complete arena and seven shared-state
objects, unrelated image bytes, caller arguments, normal ESP/DF and ABI-preserved
registers and actual fault prefixes compare. Cases include both split directions,
exact right, top/bottom rejection, zero/negative counts, null output and truncated
headers/records. Macro inputs are captured once to preserve packet access order.
The linked renderer has no imports; its 15 direct calls target only the actual
production helpers. Existing sibling coverage is retained by the full workflow.

Full 111-routine compilation/differential renewal, fidelity audit and synchronized
exports pass at snapshot `bdf9392387ee1f2fdfd0bb2ec3768dd1ce8dec45c1e42d073a07943041b266e7`.
All48 tracking/ownership/workflow guard tests pass. Existing43 scoped native
records are retained; no new native wrapper record is claimed. LF/index and
staged hooks are required before commit. RVA0x67F1C has unchanged body with
conservative source-context renewal.
No native wrapper, complete triangle/game, exception unwinding, arbitrary image
alias or concurrency proof is claimed. GCC/Clang are provisional validation
compilers; original build configuration is unresolved. Next close ordinary edge
wrappers and gradients, then triangle/sprite integration and native frames.
Playable startup, menus, rendering, input, audio and races remain the active goal.
