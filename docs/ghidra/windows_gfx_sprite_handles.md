# Windows sprite handle initialization and descriptor copying

Input is authentic `Ignition/Ignition/IGN_WIN.EXE`, 915,968 bytes, SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782,
PE32 i386 at preferred VA 0x00400000. Originals are untouched. No DOS binary
or guessed decompiler type supplies evidence. Names and geputget.c placement
are semantic. The user assumes this executable is active in Ghidra; this is
separate from local fingerprint verification and is not an automated identity
check. Read-only localhost:8080 disassembly of both routines and the default
initializer, with helper/storage/capacity/table/string xrefs, corroborated local
complete instruction decoding, FPO, relative calls, relocated operands and data.

| Routine | VA / RVA | Bytes / FPO tuple | SHA-256 |
| --- | --- | --- | --- |
| Gfx_InitSpriteHandles | 0x0045C7F0 / 0x5C7F0 | 62 / (62,0,0,0) | 41862be4869238f9f9261e170ab377306cd2aa18afbbd6761a59bf2f32a5b132 |
| Gfx_CopySpriteDescriptor | 0x004612E0 / 0x612E0 | 114 / (114,0,2,0x208) | 75f04bc300b71fce757262cbb88c22e365cd785b8fc1a87578179dba1c442858 |
| Default descriptor initializer, analyzed only | 0x004611D0 / 0x611D0 | 53 / (53,0,0,0) | bb24fdcef5b8b3fdd31746c899ca6523d467479ebc00098f5be527e9e414d70a |
| Pointer-table growth, analyzed only | 0x0045F560 / 0x5F560 | 133 / (133,0,1,0x1408) | c70cf873d3217422a169e38c5ba292d11bad39e619c1873c813d5830be610d82 |

Both reconstructed bodies end in ordinary RET at VAs 0x0045C82D/0x00461351;
no next-symbol extent assumption is needed. Initializer has no arguments and
EAX=1; no-argument RET alone cannot distinguish cdecl/stdcall. Helper has two
caller-cleanup stack words: writable 64-byte descriptor view and signed image ID.
Its callers add ESP,8. EBX/ESI/EDI/EBP, caller stack and clear DF are retained.
Initializer/helper have six/five HIGHLOW operands, independently asserted.

## Contract and storage ownership

| Storage VA | Observed ownership and extent |
| --- | --- |
| 0x00512C58 | Static freelist: 2,000 four-byte pointers, ends at 0x00514B98 |
| 0x00514B98 | Separate static 64-byte scratch descriptor; adjacent to freelist end |
| 0x00514BD8 | Static pool: 2,000 entries of exactly 12 bytes, ends at 0x0051A998 |
| 0x0051A998 | Four-byte freelist cursor; separate from preceding pool |
| 0x0051FB88 | Static default record, sixteen copied dwords |
| 0x00520388 | Signed pointer-table capacity, distinct from image ID search cursor |
| 0x0052038C | Dynamically allocated pointer-table base |

All these static storage ranges lie beyond .data raw bytes and inside its virtual
extent; the loader initially zeros them. Runtime values are recovered separately.
Production owns seven separately named globals; no aggregate layout is inferred
from their adjacency. GfxSpriteDescriptor is only the verified 16-word copy view,
with uninterpreted word bits, not a complete semantic descriptor field layout.
The preexisting handle image/origin fields agree with stride/consumer accesses.
No allocation, registration, destruction, guard or rollback occurs in either
reconstructed routine.

Initializer sets cursor to freelist + 2,000 (the original address is also scratch
start), then interleaves each freelist pointer store with a zero store to that
entry's first dword. It preserves every remaining eight-byte tail, including
stale origins. It calls the real helper with (scratch,0), ignores its EAX, and
returns 1. Every invocation repeats the entire operation. The original HandleOp
at RVA 0x5C830 decrements this cursor to pop a handle, and increments it when
returning a handle; those instructions corroborate stack ownership independently
of the old names. The pool and scratch are static, not heap-owned descriptors.

Helper branches, in original order:

1. ID zero: copy default without reading capacity or table; return zero.
2. Negative signed ID: copy default without reading capacity or table; return
   the original ID bit pattern.
3. Positive ID >= signed capacity: copy default, never read table; return ID.
   Negative/zero capacities therefore reject every positive ID.
4. Positive ID < signed capacity: read table base and exactly table[ID]. A null
   record selects default and returns ID; slot zero is never selected.
5. A nonnull live record: copy all sixteen dwords in ascending read/write order,
   then clear only destination words 6/7 (+0x18/+0x1C), return zero.

Fallback does not clear either ownership slot after copying. REP MOVSD establishes
forward copying, not an atomic snapshot or memmove contract. Disjoint sources are
unchanged; an exact live source/destination alias clears its own words 6/7 after
the copy. No pointer/ID validation is added: a valid ID with an invalid table or
record, or invalid destination, can fault. No native invalid-pointer behavior or
partial-overlap C contract is claimed. All pointer words retain 32-bit bits.

The image table is owned by unreconstructed image management. Primitive startup
RVA 0x5E610 copies its zero packet to VAs 0x00520380..8C, clearing capacity and
table before calling default initializer RVA 0x611D0. The latter writes default
word 0 = VA 0x004BACF8 (file-backed `default` plus NUL), words 1..6 = zero, and
next image-ID search word VA 0x00520380 = 1. It leaves words 7..15 unchanged.
Thus fresh-loader words 7..15 happen to be zero, but repeated initialization
must preserve their prior bits. Neither a whole zero descriptor nor a whole
semantic layout is inferred. Bounded FPO scans and Ghidra xrefs corroborate these
writes/calls; absence of further direct xrefs is not a whole-program proof.

ImageOp RVA 0x61360 passes VA 0x00520380 to growth RVA 0x5F560. Growth reads
capacity/base at packet +8/+0xC, mallocs four times capacity plus 4,000 bytes,
retains existing pointer words, zeros 1,000 appended slots, adds 1,000 to capacity,
and frees the old table. ImageOp allocates a 64-byte record for a null slot and
copies sixteen words to it, with other unreconstructed resource preparation;
its deletion path releases nested resource/record storage and zeros the slot.
Create helper RVA 0x61210 copies sixteen words then populates ownership slots;
release helper RVA 0x614B0 forwards +0x18/+0x1C to the resource free routine.
These establish why the copy helper clears ownership-related words only for a
live copy. It neither takes table ownership nor allocates/frees resources.
The broader allocator and descriptor resource semantics remain unreconstructed.

## Production integration and validation

Exact production C89 declarations/globals/bodies are extracted from geputget.c
into the focused validation build. The old nonreturning handle initializer
fixture is removed. Surface and sprite installers, selector, banner/reset and
memory lifecycle bodies continue to execute. Workspace initializers at VAs
0x0045D840/0x0045C9F0, primitive startup, CRT printf/heap and callbacks remain
explicit boundary models; no success-returning production substitute is added.
The primitive model includes only independently inspected descriptor words 0..6
and capacity/table effects. Other primitive effects are excluded. Default init
itself is original-only validated, not reconstructed or differential-tested.

Fresh strict C89 compilation passes with provisional Clang/LLD 19.1.1,
i686-pc-windows-msvc, -O2, freestanding/no-builtin, disabled
inlining/unrolling/vectorization/SSE. The import-free focused DLL is not playable.
The generated dependency guard now checks 25 direct calls with symbolic ownership,
including the initializer's real helper call; this is compiler-specific evidence,
not instruction matching.

The real lifecycle verifier adds 486 standalone helper comparisons (243 persistent
repeats) and 48 standalone handle initializers (24 persistent repeats). It covers
zero/negative/high-bit IDs, signed capacities, equality/upper bounds, null/live
slots, randomized sixteen-word records, exact live self-copy, poisoned table
pointers on short-circuit branches, entry tails and every freelist position.
An independent instruction-derived oracle checks complete state, exact ordered
reads/writes/calls, EAX, caller stack/nonvolatile registers/DF, and unchanged
unrelated image bytes. Sixteen original-only default initializer cases prove
seven-word writes and nine-word preservation with arbitrary prior bits.

Retained coverage: 306 sprite installers, 266 surface installers, 532 selectors
(266 zero,266 nonzero), 18 integrated startups (four persistent), 306 banner,
266 input reset, 112 startup (94 isolated), 519 shutdown (74 isolated), ten
persistent lifecycle calls, 48/39 original-only sprite/surface consumer ABI checks
and all 43 original-only downstream initializer checks. The last three retain
an intercepted helper to independently prove ignored-return behavior. Other
runs execute the actual helper through the real initializer, sprite installer,
selector and startup: 638 real initializer executions and 1,124 real helper
executions per binary. Arbitrary downstream mutations moved to remaining
workspace boundaries; real handle initialization replaces the former model.
Only verified dispatch identities and freelist/pool pointer relationships are
relocation-normalized. Four intervening sprite words stay untouched by installers.

Existing banner/input/memory/font/file/sprite-backend suites remain in the full
workflow verifier. No additional renderer body, native graphics parity or
complete descriptor semantics is certified. EXACT describes bounded recovered
behavior. Raw prefixes differ; relocation-aware instruction equality, original
compiler/link layout, native graphics/input/heap/game parity and playable
rebuilding remain unverified. Invalid/unmapped storage, partial overlap,
concurrency and general reentry remain outside validation.

Use PowerShell, Python through uv, UV_CACHE_DIR=build/uv-cache and UV_OFFLINE=1.
Run `uv run python tools/verify_mem_lifecycle.py`, then `tools/workflow.py complete`
for all nine affected reconstructed RVAs; separately complete analysis-only
updates for 0x611D0/0x5F560/0x5E610/0x5D840/0x5C9F0. The real completion performs
full verification, fidelity audit, export and export check. Recorded input hashes
must agree with LF working bytes and Git index blobs before staged gates/commit.
Inventory stays at 1,028 candidates, with 32 reconstructed routines.
