# Windows raw and corrected triangle edge records

Authenticated standard IGN_WIN.EXE, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
VA = 0x400000 + RVA. Original filenames are unknown. Reconstruction belongs
to the directly compiled renderer module, declared before this feature.

| Routine | RVA / VA | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| Gfx_EmitRawTriangleEdgeRecords | 0x24F1E0 / 0x64F1E0 | 51 | `a236e05fa139724a52b65926d5407dead47076e422411d8a02528937d2c48811` |
| Gfx_EmitCorrectedTriangleEdgeRecords | 0x24F220 / 0x64F220 | 70 | `39c7e37061dbb0ba74b5e5735eee6081a8d152da7a6391807d978cc1e9c0bbe6` |

Neither entry has FPO. Both complete bodies end in RET. Raw alignment padding
is six XCHG EBX,EBX instructions plus NOP; corrected padding is five XCHG
EBX,EBX instructions. The initial verifier assumption of INT3 padding was rejected
against real bytes and replaced with these exact assertions before recording
any result. These entries lie in the PE code section lacking an execute flag;
native NX implications remain unresolved. Bounded Ghidra disassembly corroborates
the fingerprinted local bytes; no program-identity endpoint is claimed.

Actual callers are authenticated in [triangle analysis](windows_triangle.md):
raw wrappers at 0x64EC40/0x64F0C0 and corrected at 0x64EC00/0x64EFA0.
Before implementation, 17 direct original-only executions checked 32,797 ordered
records and all updated register outputs. The renewed original triangle oracle
also passed 312 unclipped triangles, 60 clipped triangles, 24 transformed sprites,
62 splitter cases and 16 clipped-wrapper cases; this is original-only evidence.

Inputs and outputs are V in EAX, U in ESI, X in EDX, output pointer in EBX and
count in ECX. Each iteration writes U at output+4, V at output+0, X at output+8.
Raw writes SAR(U,16), SAR(V,16), SAR(X,16). Corrected uses bucket=(X>>13)&7,
adds that bucket's U/V corrections with 32-bit wrapping, and shifts U/V by eight.
Accumulator additions wrap. Output advances 12 bytes after every record.

U increment at VA 0x4BAEAC is read after writing U; V increment at 0x4BAEB0
after writing V. X increment at 0x4BAEC0 is read before writing X. Corrected U
correction at 0x4BADB4+bucket*4 is read before U write, while V correction at
0x4BADD4+bucket*4 is read after updating U. Aliases can therefore change later
reads in the same iteration. Named volatile state and record fields preserve
that order. The three increments and two eight-word arrays are authenticated
file-zeroed backing. Intervening increment-storage words retain their verified
distances without assigning unsupported meanings. Real gradient/step initializer
reconstruction remains pending; no invented initializer or callee is introduced.

Raw decrements only CX and preserves ECX's upper 16 bits. Corrected decrements
all ECX. Both then use signed JG, including overflow, and always execute the first
record. Raw count0/negative low16 and 0x10000 each emit one record; 0x7FFF emits
32,767. Raw 0x8000 emits one record because decrement sets OF; corrected INT_MIN
also emits one. The initial independent raw oracle missed this OF case and was
corrected from the actual execution before production reconstruction. This differs
from the span's JNS branch, which ignores overflow. No positive-count guard fixes
these original behaviors.

DEV-007 adapts the complete native register interface into a typed internal C
cursor containing every input/output register value. Inputs are captured before
record writes; all updated values are published on normal return. The cursor must
be disjoint from output storage, replacing register passing rather than defining
arbitrary new cursor/output aliases. This adaptation claims no equivalence for
other clobbered registers or flags. Production is readable strict C89 with explicit
unsigned wrapping and arithmetic shifts, no assembly and no modeled dependencies.

`uv run tools/verify_edge_emitters.py` passes 408 original-versus-C calls,
192 persistent followups, 64 state/table aliases and ten fault executions, with
298,822 complete records. Original instructions and whole compiled renderer execute.
Every ordered non-stack read/write, all updated accumulators/count/pointer advance,
all output bytes, all increment/correction backing, unrelated image preservation,
return/ESP/arguments and compiled cdecl preserved registers are checked. Raw high
count halves, signed-overflow boundaries, 16/32-bit loop differences, correction
buckets, extreme wraps, live state aliases, null output, wrapped output and partial
record prefixes at buffer ends are exercised. On faults, memory/state/access prefixes
are checked; unavailable register-to-cursor publication is not invented.

The builder compiles the full renderer C89 translation unit and links an import-free
DLL with real emitter exports. Existing 896 span differential calls and 64 paired
native span cases still pass after this addition. Those native results validate
spans, not the new register-input emitters. No fabricated native register shim,
source-level model or fake callee return is used. Native emitter/parent integration,
flags, arbitrary overlapping cursor/output, complete triangle rendering and original
compiler/link configuration remain unresolved.

Full 105-routine renewal, fidelity audit and synchronized exports pass at snapshot
`cb5157ea80c4ea5c7715bbee28338985365ff40f85b8393cadf7c5b66a308103`, with 105 fresh compilation/emulation and 42 existing scoped native records.
All 48 tracking/ownership/workflow tests pass. LF/index gates and tracked hooks
are required before commit; no new emitter native record is manufactured.
RVA 0x67F1C receives conservative renewal because preceding globals/annotation
context changed; its setup body is unchanged. Continue real edge setup, clipping,
gradients and triangle/sprite integration after the separate commit. The playable
Windows startup/menu/render/input/audio/race objective remains active.
