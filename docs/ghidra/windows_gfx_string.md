# Windows allocated-string copy

Authentic standard IGN_WIN.EXE, 915,968 bytes, SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782.
PE32 i386, image base VA 0x00400000. Routine VA 0x00460480 / RVA 0x60480,
72 file-backed .text bytes, FPO (72,0,2,0x0202), SHA-256
c74c6d2d995e97623501ae2f7f259573637233d61412dbf2db008c793eaa2e5d.
RET at VAs 0x00460494 and 0x004604C7; eight INT3 bytes follow. No HIGHLOW
operands or relocated entry pointer. Independent local decoding/FPO/hash and
live Ghidra full instructions/decompilation/xrefs agree. Active authentic program
selection remains the user-provided assumption, not an identity endpoint check.

Two cdecl pointer arguments: source string first, pointer destination second.
ESI/EDI preserved, ordinary RET and caller cleanup. Semantic signature is void;
incidental EAX is unspecified for null source and count/zero for other paths.
ImageOp RVA 0x61360 calls at VA 0x0046148A and discards EAX before storage.
Other direct calls occur at VAs 0x0045FF29/0x00460275/0x00460CCD/0x00460D32/
0x00460F42/0x00461181/0x00461261. Whole caller contracts remain unreconstructed.
Names and geputget.c placement are semantic inference, not recovered symbols.

Null source writes one zero dword to destination and returns without scanning
or allocating. Destination itself has no null guard. Otherwise scan source bytes
in increasing address order through first zero. Add one modulo 2^32 to the byte
count and call real Gfx_AllocBytes RVA 0x5F4A0. Publish its pointer to destination
immediately, before copying any byte. Copy exactly that captured count when
positive as signed, reading one current source byte and writing one destination
byte in increasing order. Pointer/counter arithmetic wraps as x86 dwords. The
scan occurs once; post-allocation changes can remove/change the terminator but
do not change the captured copy count. Allocation failure is unchecked. No
source free, previous destination free, ownership registration or rollback exists.

Source and output pointers stay captured through allocation and publication.
Source content remains live. Pointer publication can overwrite source bytes for
aliases. Forward overlap propagates overwritten bytes; copying abcde plus NUL
into source+1 produces seven a bytes across source/output, with no terminating
NUL. Empty nonnull source allocates/copies one byte. Null source does not allocate.
Malformed strings and invalid destination/source/output pointers fault after
prior effects. Full 32-bit scan/counter wrapping is not validated by bounded tests.

The instruction-derived disposable oracle and ten original-only probes confirm
null/empty/ordinary strings, forward overlap, destination-word/source alias,
post-malloc source mutation, null allocation and source/destination faults with
ordered effects and normal cdecl ABI. They are analysis corroboration only,
not compiled-C or native heap/game validation. Production needs meaningful
differential coverage with real allocation wrapper and explicit modeled CRT.


## Production reconstruction and differential validation (2026-10-09)

Production geputget.c now implements Gfx_CopyAllocatedString in readable C89,
with unsigned wrapping pointer/count arithmetic, a signed captured copy bound,
volatile byte reads/writes and immediate volatile destination publication. The
real Gfx_AllocBytes body executes. No strlen/strdup substitute, destination/source
free, pointer/allocation guard, rollback or success stub is introduced. The
focused builder extracts the production body and exports it. Provisional strict
C89 Clang/LLD PE32 -O2/no-builtin/no-inline/no-unroll/no-vectorization/no-SSE
settings support behavioral validation only; original compiler/link layout is open.

Run `uv run python tools/verify_gfx_string.py`. Original instructions and
compiled C execute the actual wrapper; only CRT malloc is modeled with explicit
reply/mutation schedules and volatile-register clobbers. The independently
derived raw-byte oracle checks full arena/global/unrelated-image bytes, exact
ordered scan/copy/publication accesses, allocation arguments and entry/pre-malloc
snapshots, persistent effects, and normal cdecl caller-stack/nonvolatile/DF ABI.
Incidental EAX and compiler-dependent private stack layout are excluded.

**941 comparisons per binary** include **434 persistent follow-ups**, **seven
faults**, **226 alias calls**, **65 mutation calls** and **874 CRT malloc calls**.
Nonzero bytes cover 1..255 and lengths 0..256; null and empty sources remain
separate. Forward and backward/exact allocation-source overlaps, destination
word/source aliases, destination word/output aliases, live post-malloc byte
mutation and partial mapped byte-source/output boundaries are checked. Explicit
hand-derived expectations assert ordinary/empty copies, forward propagation,
pointer-publication source bytes and a removed terminator after allocation.
Persistent calls retain all prior state and must rescan current source bytes.
Scheduled allocator replies are boundary tests, not native heap reuse evidence.

One cross-page dword publication case is **excluded** from reconstruction
validation. A disposable original-only diagnostic with Unicorn 2.1.4 writes the
two mapped low bytes of the four-byte pointer store, then reports the two unmapped
bytes as separate faults. This differs from the attempted dword store abstraction
used by the oracle; native x86 fault atomicity was not observed. It is a concrete
emulator limitation, not a production source repair or a passing case. Fully
unmapped publication and partial byte-copy faults remain in differential coverage.
Fault-time register/stack frames and native exceptions remain unverified.

Whole caller integration, 32-bit scan/count overflow, arbitrary reentry/concurrency,
caller-stack/global pointer aliases, native heap/game/fault parity and portable
unchecked-access guarantees remain outside the bounded contract. The eight
callers have static evidence only. Compilation and differential emulation are
separate from instruction equality and native execution; no playable build is
certified. Full workflow completion renews all 48 reconstructed routines among
1,028 candidates, retaining previous memory/font/file/backend/lifecycle/packing/
pixel/storage/release coverage, then audits/exports/checks the synchronized store.
Check LF working/index input bytes and pass staged gates and tracked hooks.
Next recover pointer-table growth RVA 0x5F560 and coherent ImageOp control-block
layout before reconstructing the whole native image-operation caller.
