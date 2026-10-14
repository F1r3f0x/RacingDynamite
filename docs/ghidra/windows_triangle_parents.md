# Windows ordinary and clipped textured triangle parents

Authenticated standard IGN_WIN.EXE SHA256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
Analysis names Gfx_RasterizeTexturedTriangle and Gfx_RasterizeClippedTriangle
are not recovered linker symbols. RVA0x5CA50/VA0x45CA50,1813bytes through
RET0x45D164, hash`4458bb7690590a3289e6ce7d99b3a80338382fe2afc3b67a25891b2af0e5f8bf`.
RVA0x5D170/VA0x45D170,1569bytes through RET0x45D790, hash
`d1d8d33f6f947f949ae7160d07da0fcc3e66d4a67168ad30c5eb80fa62f175f7`.
Local pinned bytes/FPO extents and bounded live Ghidra instructions authenticate
both complete entries. Loaded target is the user-provided operating assumption,
not an endpoint fingerprint. Original source filenames/compiler remain unknown.

Both are cdecl with one60byte packet pointer, preserving EBX/ESI/EDI/EBP.
Original EAX is incidental. Packet word0 is unconsumed; signed X/Y pairs are
at+4..24, U/V pairs+28..48, texture+52, lookup+56. The outer parent copies
fields into verified48byte geometry/UV scratch, captures framebuffer, texture
and cursor, computes signed Xmin/max before sorting and performs original
Y0/Y2, Y0/Y1, else Y2/Y1 compare/swap order. Equal-Y ties retain that order.
Packet remains read-only. Four signed viewport tests route to the real clipped
parent; GCC merges their equivalent call site, which is verified as one compiled
call versus four original sites. Runtime dispatch effects/accesses still compare.

The clipped parent consumes already prepared sorted scratch and presort Xrange;
it is not an independent packet decoder. Strict outside rejections preserve
boundary equality. It compares BC/AB slopes; the outer compares AC/AB. Both
execute actual gradient, setup, slope, edge-wrapper and span bodies. Three buffer
slots remain owned by the existing verified graphics_legacy workspaceB initializer
RVA0x5C9F0. No duplicated allocation result or invented dependency is added.
The focused builder compiles exact production-owned storage declarations from
that retained owner, while the whole renderer module compiles directly. Actual
workspace initializer/native allocator lifecycle remains a separate integration
boundary; tests supply caller-owned arrays of the authentic8408byte capacity.

New parent-owned ordinary/clipped32byte mutable packets at51A9A0/51AA00 are
loader-zeroed, as are texture51A9C4, frame cursor51A9D0 and Xrange51A9D4/F8.
Shared vertex/gradient/viewport backing is reused. Source placement is renderer,
with explicit module ownership/dependency preflight. DEV-007 retains public cdecl
while adapting internal original register-only helpers to their verified typed
C interfaces; incidental scratch registers/flags are not claimed.

Complete row processing retains two edge assignments and upper/lower halves.
Frame starts from the appropriate raw long/short header startY times live stride.
Header rowcount tests signed positive, while edge wrappers skip only zero counts.
Counts and slot pointers reload after every row. Ordinary spans consume rawleft
and correctedright with count rightX-leftX-1 and destination cursor+leftX+1.
Clipped rows compare rawleftX to live integer clipleft; outside-left uses the
actual four-argument clipped span, otherwise the actual five-argument span and
left-record sampling. Empty/reversed spans retain original helper semantics and
cursor advancement. Longedge offsets continue across halves; cursor/header-read
order differs between branches and is preserved. Wrapped addresses/arithmetic
and divide/pointer fault prefixes remain authentic under the provisional compiler.

Before production, fresh original-only execution renewed312 ordinary,60 clipped
and24 transformed-sprite fixtures with independent frame oracles. Production
`uv run tools/verify_triangles.py` preserves all312/60 triangle fixtures and adds
permutations, fractional/degenerate/reversed cases,128 random triangles, direct
prepared clipped entries, persistent calls and malformed pointers/divide overflow.
1,139 top-level comparisons pass:530followups,9actual faults and52,092 recorded
initial/direct frame stores. Actual entry counters agree:1,069outer and422clipped
visits, including nested routing. Every ordered non-stack read/store, entire
frame/texture/table/edge arena and24 shared state objects, unrelated image/caller
bytes and normal cdecl registers/ESP/DF compare. No modeled rendering callees.
The whole renderer links import-free with54verified production direct calls;
shared verifier also enforces exact ordered symbolic targets.

Paired native Windows validation passes48 top-level parent-call pairs
(96executions),24each outer/direct clipped, plus real nested helper execution.
Complete65536byte pixel planes, all three8408byte edge arrays and shared state
match. Real configure/clip routines initialize the viewport. Synthetic texture/
lookup patterns exercise actual compositing; no original game palette interpretation
or asset-level rendering is claimed. Fingerprinted original PE is mapped/relocated
with executable pages; original loader/NX conditions are adapted. Direct clipped
calls reuse real sorted state from outer execution and reset cursor/buffers explicitly.
Caller-owned workspace fixtures, native faults/SEH/flags/concurrency/arbitrary
image/stack aliases and original toolchain remain limitations.

Full122 compilation/differential renewal, fidelity audit and synchronized exports
pass at snapshot `c0ae71a8e923ea587214ce795d95879ecabf69b79ed5f098721319d598c8ade9`, with51 scoped native
records. All48 tracking/ownership/workflow tests pass. Final LF/index and staged
gates/hooks are required before commit. RVA0x67F1C body remains unchanged with
conservative source-context renewal.
No playable-game completion is claimed. Next reconstruct/migrate real sprite
consumers and workspace/startup integration, then validate menus/input/audio and
representative races natively. Original assets remain unchanged; goal stays active.
