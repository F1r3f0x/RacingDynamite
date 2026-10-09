# Windows sprite packing storage release

Authentic standard IGN_WIN.EXE, PE32 i386, preferred image base VA 0x00400000,
915,968 bytes, SHA-256 7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782.
Routine VA 0x004618B0 / RVA 0x618B0, 419 file-backed .text bytes, authentic
FPO (419,1,1,0x1410), SHA-256
7563059eb087b1961ce5aac8901e5ed29574c2f1a9354e62ed943d5842fb0072.
RET paths at VAs 0x00461920, 0x00461937, 0x0046194E, 0x00461975 and
0x00461A52; thirteen INT3 bytes
follow. Four HIGHLOW operands at RVAs 0x618EF/0x6195A/0x619B8/0x61A0E
reference page head VA 0x0051FE40 and bucket table VA 0x0051FF58. Both globals
are in the loader-zeroed .data virtual tail. No relocated entry pointer exists.
Local decoding/FPO/hash/relocations and live localhost:8080 Ghidra full
instructions/decompilation/xrefs agree. Active authentic program selection is
a user-provided assumption, not an automated identity check. No DOS evidence.

One descriptor pointer, caller cleanup, four saved nonvolatile registers and
four local bytes. Incidental EAX is discarded by both calls at VAs 0x004613C0
and 0x0046146C in ImageOp RVA 0x61360, 329 bytes, SHA-256
81b591def47aa754c8a41dd4255ba978ad5ff82514ca2533537dd607077a2020.
Semantic C signature is void. It does not clear or free the descriptor, source
name, image ID or record itself; the caller owns those actions.

Read descriptor height once as signed; require 1..256. Then width once with the
same signed bounds. Read global page head, then descriptor pixels, even for an
empty page list. Compare page pixels to captured pixels & 0xFFFF0000; traverse
next until first match or null. First container comes from page children and is
dereferenced without a null check; compare its pixels to captured pixels &
0xFFFFFF00 and walk next. First leaf from container children is similarly
unguarded, comparing exact pixels and traversing next. Next load the bucket
head using captured original height. Dereference initial bucket without a null
check; find first node pointer equal to captured container, capturing bucket
predecessor. Exhausted searches return without writes/frees. Cycles are unguarded.

Read leaf next. Only if zero read leaf previous for singleton determination.
If not singleton, reread previous; write previous next or container children
using captured successor, then reread leaf next and, if nonnull, reread previous
and repair successor previous. Free leaf through the real byte wrapper last.

For singleton leaf, read container next and, only when zero, previous. If
container is not singleton, reread previous, write its next or page children
using captured container successor. Reread next and repair its previous with
a fresh container previous read. If container is singleton, read page pixels,
call real aligned free (unguarded back-pointer read, then real byte wrapper),
then reread page previous and next, repairing previous next or publishing head.
Reread page next and, if nonnull, previous, repairing next previous. Free page
through the byte wrapper. Heap-boundary mutations are visible in these rereads.

Then read bucket next. If captured bucket predecessor exists, write predecessor
next. Otherwise reread descriptor height *after* page/pixel frees and use raw
wrapping table-address arithmetic to publish bucket next. No repeated height
validation is present. Free bucket, container and leaf, in that order, through
the real byte wrapper. All frees are CRT modeled boundaries only in validation.
Original CRT/free reuse/exception behavior is not claimed. Null/invalid headers,
node reads/repair writes fault after all prior effects without rollback. Exact
and partial aliases follow ordered accesses; no liveness/ownership/cycle guards.

Original-only disposable analysis corroborates 256 structural list choices,
35 CRT mutation schedules, 18 fault/cycle probes and 38 link/descriptor aliases.
These are analysis evidence, not compiled-C or native execution records.
Production must add reproducible differential tests and all progress gates.


## Production reconstruction and differential validation (2026-10-09)

Production geputget.c now implements the full release contract in readable C89,
using the verified pixel-prefix/node/bucket types, volatile live reads/stores,
raw unsigned wrapping table-address arithmetic and the real byte/aligned free
bodies. It introduces no pointer, allocation, cycle or rollback guard. The
original descriptor is retained after release. The builder extracts this actual
production body and exports it in the focused validation DLL. Both inspected
ImageOp calls remain statically authenticated; the whole caller is unreconstructed.

Run `uv run python tools/verify_sprite_release.py`. The raw-offset oracle derives
its expectations independently from authentic instructions. Original and rebuilt
x86 images execute real release, aligned-free and byte-free bodies; CRT free
is intercepted with explicit mutations, arbitrary return register and volatile
clobbers. Compare complete arena/global/unrelated-image bytes, exact ordered
external reads/writes, every CRT argument and pre-free snapshot, and cdecl
caller-stack/nonvolatile/DF on normal return. EAX and private local-stack layout
are excluded. The provisional Clang/LLD strict C89 PE32 settings are the existing
-O2/no-builtin/no-inline/no-unroll/no-vectorization/no-SSE focused build settings;
no original compiler or instruction-equality claim is made.

**1,267 comparisons per binary** include **603 persistent follow-ups**, **24
faults**, **74 alias calls**, **35 CRT-mutation calls**, **four externally bounded
cycle prefixes**, and **1,963 CRT free calls**. The structural grid covers head,
interior, tail and singleton page/container/leaf/bucket positions. Independently
specified free sequences distinguish retained container, retained page and
whole-page removal. All 256 admitted bucket-height indices, four dimension
corners, signed nonpositive/oversized rejection, all search misses and null
aligned back-pointer are checked. Persistent calls run on already mutated state;
they do not reconstruct lists between calls.

Mutations at all five full-release CRT calls test captured/live height, page
previous/next/pixels, bucket next, container children and leaf links. Header-zero
skips only the initial CRT free. Descriptor/node, self, cross-node, bucket and
neighbor aliases retain ordered effects; exhaustive partial aliases are not
certified. Unmapped and partial descriptor/page/container/leaf/bucket accesses,
null initial children/bucket, and invalid previous/next repair stores are checked.
Prior frees/writes survive faults. Fault-time registers/stack frames and native
exceptions are excluded. Cycle prefixes stop externally after 31 reads and make
no liveness claim. Negative/out-of-table post-free height mutation, arbitrary
reentry/concurrency, global/template/caller-stack aliases and native heap reuse
remain unvalidated without adding production guards.

PE headers, all eight section extents, imports (KERNEL32/USER32/GDI32/WINMM,
DirectInput/DirectDraw/DirectSound/DirectPlay), 44,089 HIGHLOW relocations and
loader-zeroed page-head/table virtual-tail storage were independently inspected
in this session. These observations do not identify the original compiler/version.

Full completion uses `uv run python tools/workflow.py complete --rva 0x618b0`
with explicit limitations. It renews all 47 reconstructed routines among 1,028
candidates, retaining every earlier memory/font/file/backend/lifecycle/packing/
pixel/storage suite, then audits provenance and synchronizes exports. Verify
LF working/index input bytes and pass the staged gate and tracked hooks.
Compilation and differential emulation remain separate from instruction equality
and native execution. No playable rebuilt game, native heap/graphics/game/fault
parity or whole ImageOp integration is certified. Next authenticate the ImageOp
name-copy and image-table-growth dependencies before reconstructing its caller.
