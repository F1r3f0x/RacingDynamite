# Windows generic pointer-table growth

Authentic standard IGN_WIN.EXE, 915,968 bytes, SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782.
PE32 i386, preferred image base VA 0x00400000. Routine VA 0x0045F560 /
RVA 0x5F560, 133 file-backed .text bytes, FPO (133,0,1,0x1408), SHA-256
c70cf873d3217422a169e38c5ba292d11bad39e619c1873c813d5830be610d82.
Ordinary RET at VA 0x0045F5E4 followed by eleven INT3 bytes. No internal
HIGHLOW operands or relocated entry pointer. Bounded Ghidra complete instructions/
decompilation agree with independent local decoding/FPO/hash. Active authentic
program selection is the user-provided operating assumption, not an automated
identity check. Current target doctor reports no errors and matching fingerprint.

One cdecl control-packet pointer; caller cleanup. EBX/ESI/EDI/EBP are saved;
incidental EAX after CRT free is unspecified and discarded by inspected callers.
Semantic signature is void. Natural Windows four-byte fields form a 16-byte
control view: +0/+4 preserved opaque words for this routine, signed capacity at
+8, pointer to four-byte record words at +12. Other callers use the first two
words; no system-wide unused/reserved meaning is inferred. This replaces the
previous image-only semantic name Gfx_GrowImagePointerTable with generic
Gfx_GrowPointerTable because local evidence identifies six different controls.

Read old records pointer first, then current capacity. Request direct CRT malloc
VA 0x00469400 with four times capacity plus 4,000, modulo 2^32, at call VA
0x0045F576. A zero-size request is still sent to malloc; the zero-skipping graphics
wrapper is not used. Immediately publish the allocation at packet +12. There
is no allocation-success, capacity, size or ownership guard.

Test live signed capacity against zero. For each admitted increasing copy index,
read the captured old pointer's current dword first, then reread live records
pointer and write that dword at its wrapping index offset. Increment the index
and reread capacity for every next signed comparison. Source pointer remains
captured through allocation and publication, but its contents are live. Records
pointer and capacity are not cached across iterations or snapshot-copied.

After copying, reread capacity into the clearing index. Compute index plus 1,000
modulo 2^32 and perform a signed comparison before clearing. For each admitted
slot, reread records pointer, store zero at its wrapping four-byte index offset,
increment index, then reread live capacity and add 1,000 modulo 2^32 for the next
signed comparison. Thus aliases can alter both pointer and bound during clearing.
The initial end comparison uses the captured clearing start, not an extra capacity
read. Negative capacities can clear before the returned allocation; positive
addition crossing the sign boundary can skip clearing. No repair guard is added.

Finally read live capacity, add 1,000 modulo 2^32 and store it. Call direct CRT
free VA 0x004693B0 at VA 0x0045F5D8 with the originally captured records pointer,
unconditionally even when null. No post-free writes exist. Callback mutations
at malloc are visible in publication/copy/clearing, and free mutations remain
visible on return. Native CRT callbacks/reentry/heap reuse are not certified.
Faults in packet/source/destination accesses preserve earlier publication/writes
without rollback or source free. Nontermination is possible under live aliases.

Local complete FPO instruction scans identify seven calls, each cleaning four
argument bytes, into six controls:

| Caller RVA / VA call | Control VA | Caller SHA-256 |
| --- | --- | --- |
| 0x5F4C0 / 0x0045F4E4 | 0x0051FC88 | eb1b0a07ea378bab1a4d2239fcf07a71a725fe445bcdaf762b63cc3635caa663 |
| 0x5F7B0 / 0x0045F7DA | 0x00520360 | e2bc725199e7c67bfa58483239fba8d777b8db55951726a1190e3f7933bfe4c2 |
| 0x60150 / 0x0046017B | 0x0051FE30 | 1818a9307d8d0f964619a2442a3c7b903673c8bb7ead3cb8775459e36f09e493 |
| 0x60860 / 0x0046089D | 0x0051FE98 | 1eb84b4cc5448f726e623b47e611860499a6f47b763d7148d91fc22f74035e4d |
| 0x60DE0 / 0x00460E0E | 0x005203B8 | fdff044c6897fa75554edde628d92840899d25f9d31c864a79936f31f1eb1260 |
| 0x60F90 / 0x0046102D | 0x005203B8 | 3b4153830d297745e0c123de5ef4249589bd58964d8be9d4993497a033167a75 |
| 0x61360 / 0x0046138A | 0x00520380 | 81b591def47aa754c8a41dd4255ba978ad5ff82514ca2533537dd607077a2020 |

Ghidra reports five of these xrefs; its absent functions at RVAs 0x5F4C0/0x60860
do not negate authenticated local calls. They are incomplete discovery, not
contradictory target instructions. Whole caller contracts remain unreconstructed.
All six controls are in the loader-zeroed .data virtual tail. Independently
inspected primitive initializer RVA 0x5E610 copies a separate zero packet from
VA 0x0051FD00 into these controls before downstream calls. The existing default
initializer sets image control word +0 to one. General startup body remains
unreconstructed; inferred field +4 meaning and whole control ownership are open.
Existing separate compiler globals for image fields cannot be assumed contiguous;
ImageOp integration must reconstruct its coherent control object independently.

Forty disposable original-only probes corroborate this ordered contract. Explicit
hand-derived expectations check old dword retention, 1,000 zero slots, final
capacity, unconditional old-pointer free and direct zero-size malloc for capacity
-1,000. Further probes cover capacity mutation at allocation, signed negative/
large words, original source-pointer capture, exact source/destination/control
aliases, forward overlap and free-boundary mutation, faults and one externally
bounded large-capacity prefix. These are analysis corroboration only, not new
compilation/differential/native results. CRT malloc/free are modeled boundaries.
Production needs a real C89 body, differential tests and all progress gates.


## Production reconstruction and differential validation (2026-10-09)

Production geputget.c now implements Gfx_GrowPointerTable in readable C89 with
an independently verified 16-byte GfxPointerTable view in geputget.h. Two opaque
prefix words are followed by signed capacity and a four-byte records pointer.
Unsigned pointer/index/size arithmetic and explicit signed comparisons preserve
wrapping x86 effects. Volatile control and dword accesses retain ordered live
rereads. Actual CRT malloc/free calls are production dependencies; only their
validation counterparts are nonreturning fixtures intercepted by the emulator.
No zero-size malloc skip, success/pointer/capacity/alias guard or rollback exists.
No source/global ImageOp control integration is implied by this standalone body.

Run `uv run python tools/verify_gfx_table.py`. The independent raw-offset oracle
was derived and corroborated against original instructions before the C body.
Original and rebuilt x86 images execute the complete growth body with explicit
CRT reply/mutation schedules and volatile-register clobbers. Compare all arena/
global/unrelated-image bytes, exact ordered dword reads/writes, malloc argument/
pre-malloc snapshot, unconditional captured-old free argument/pre-free snapshot,
persistent state, and normal cdecl caller-stack/nonvolatile/DF. Incidental EAX,
private stack layout and fault-time registers/stack frames are excluded.

**363 comparisons per binary** include **153 persistent follow-ups**, **45
faults**, **93 alias calls**, **15 mutation calls**, **two direct zero-size
malloc calls** and **five externally bounded large-copy prefixes**. Ordinary
capacities 0..64 and 255/256/257/999/1000/1001/2000/4000 execute three consecutive
real growth calls, retaining prior entries and appending zeros without resetting
state. Explicit hand-derived expectations independently assert retained words,
1,000 zero slots, capacity increment, allocation size and unconditional null/
nonnull free. Signed negative/high-bit capacities, before-allocation clearing,
wrapped offsets/sizes, exact/partial control/source/destination aliases, forward
copy propagation and unaligned mapped control views are covered. Allocation
mutations verify saved old-pointer behavior and live capacity; free mutations
verify no later repair/store. Fully unmapped and aligned partial packet/source/
destination faults retain earlier publication/writes. Cross-page dword stores
are excluded for the already documented Unicorn partial-store limitation.

The focused compiler is provisional Clang/LLD strict C89 PE32 with existing
-O2/no-builtin/no-inline/no-unroll/no-vectorization/no-SSE flags. Compilation and
differential emulation are recorded separately; instruction equality, original
compiler/link layout, native heap/graphics/game/fault parity, exhaustive aliases,
general reentry/concurrency, global/caller-stack aliases and portable unchecked-
fault guarantees remain unverified. External prefix budgets are observation
limits, not added termination guards. Scheduled heap replies do not certify
native ownership/reuse. No playable rebuilt executable is certified.

Full workflow completion renews all 49 reconstructed routines among 1,028
candidates, retaining every prior memory/font/file/backend/lifecycle/packing/
pixel/storage/release/string suite, then audits and synchronizes exports. Verify
LF working/index input bytes and pass staged gates/tracked hooks. Next recover
the coherent image control block at VA 0x00520380, then reconstruct complete
ImageOp RVA 0x61360 with all real string/storage/release/growth dependencies.
