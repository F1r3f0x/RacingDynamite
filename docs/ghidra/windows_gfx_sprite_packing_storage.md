# Windows sprite storage assignment and pixel-copy boundary

Analysis feature, 2026-10-09. Neither routine below has production C or new
compilation/differential records. Authentic input: Ignition/Ignition/IGN_WIN.EXE,
915,968 bytes, SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782,
PE32 i386, preferred image base VA 0x00400000. The target doctor authenticates
local bytes. Ghidra localhost:8080 disassembly, decompilation and xrefs agree
with independent local Capstone decoding under the user-provided assumption
that this executable is active. This is not an automated Ghidra identity check.
No DOS evidence supplies this contract; original binaries/assets are untouched.
Names and eventual geputget.c placement are semantic, not recovered symbols.

## Provenance and callers

| Routine | VA / RVA | Bytes; FPO tuple | Routine SHA-256 |
| --- | --- | --- | --- |
| Gfx_AssignSpritePackingStorage | 0x00461530 / 0x61530 | 350; (350,18,1,0x140B) | 028b7432fafcd6c13a8ea0d675bda862a30ca19e99fbd6a3ec3e9a4344f9c394 |
| Gfx_CopySpriteDescriptorPixels | 0x004612A0 / 0x612A0 | 61; (61,0,2,0x1411) | 32a8a30b19b528630595c3efaab77bc2ff20798c4a133ae569091264cc784e5f |

Complete decoding reaches ordinary RET at VAs 0x0046168D and 0x004612DC;
two/three INT3 padding bytes follow. Storage also returns at VA 0x0046160F.
Both extents are file-backed .text and independently match authentic FPO and
hashes. Storage reserves 72 local bytes and saves/restores EBX/ESI/EDI/EBP.
It accepts one caller-cleanup descriptor pointer. Five HIGHLOW operands at
RVAs 0x6156D/0x61576/0x615AB/0x615ED/0x61611 reference table VA 0x0051FF58,
page head VA 0x0051FE40 and template VA 0x0051FC00. The pixel leaf has no
calls, global operands or HIGHLOW operands. No relocated entry pointer is
found for either routine. Computed/untracked references remain possible.

A complete direct call/jump scan of FPO extents and Ghidra xrefs agree:

- Storage: sole direct caller VA 0x00461493 in Gfx_SpriteImageOpNative,
  RVA 0x61360, 329 bytes, FPO (329,0,2,0x140C), SHA-256
  81b591def47aa754c8a41dd4255ba978ad5ff82514ca2533537dd607077a2020.
  It passes the live image-record descriptor in EBP, cleans four bytes at
  VA 0x00461498, overwrites EAX with the image ID in EBX, and returns.
  Immediately beforehand it copies sixteen dwords into that record and
  calls VA 0x00460480 with record word +0 first, descriptor pointer second.
  This bounded call-site inspection does not reconstruct the whole caller.
- Pixel leaf: VAs 0x00461289 in RVA 0x61210 and 0x0046167E in storage.
  Storage passes its local snapshot first and live descriptor second,
  cleans eight bytes, and returns without assigning a status.

Storage has no meaningful uniform EAX result. On initial height rejection,
EAX is untouched; width rejection leaves the loaded width in EAX. A stop
immediately after bucket creation retains bucket EAX. Successful pixel copy
leaves EAX equal to the local snapshot address because the leaf retains its
first argument. The sole caller discards every form. A future void storage
prototype should exclude EAX from differential return comparisons, rather
than inventing a success code or preserving a compiler-dependent local address.
Normal returns preserve caller stack/nonvolatile registers and clear DF.

## Descriptor and packing views

The descriptor is the existing verified sixteen-dword, 64-byte copy footprint.
This feature independently establishes the following consumed prefix; it does
not claim a complete semantic layout or change production declarations:

| Byte offset | Observed use |
| --- | --- |
| +4 | Signed width / requested child extent |
| +8 | Signed height / bucket-table index and requested bucket extent |
| +0x10 | Pixel pointer, replaced with the assigned node's pixels |
| +0x14 | Row stride in bytes, replaced with 256 |

The remaining twelve words are copied but not explicitly changed by storage.
In particular +0x18/+0x1C are not assigned a new node/bucket owner pointer.
Destination pixel aliases can nevertheless change any mapped word during
the final copy. The [six-field node](windows_gfx_sprite_packing_reset.md) and
[eight-byte bucket](windows_gfx_sprite_packing_buckets.md) views remain:
node start/end/pixels/children/previous/next at +0/+4/+8/+0x0C/+0x10/+0x14;
bucket next/node at +0/+4. Nodes in a bucket's children describe horizontal
byte intervals; page children provide vertical extents to buckets. The shift
by eight belongs to bucket placement, not this child placement.

## Complete storage control flow and live reads

1. Read descriptor height and reject signed height <= 0. Reread height into
   EDX and reject signed height > 256. Read width into EAX and reject signed
   width <= 0 or > 256. The first height is not reused. No descriptor-null
   check exists: a null pointer first attempts its +8 read.
2. Read table[EDX], retaining that previously loaded height. An empty slot
   reads the page head, then calls the real bucket helper with the cached
   height first and head second. It skips the traversal below.
3. For each nonnull bucket, capture bucket->node, read the live descriptor
   width, then read that captured node's children. Call the real gap helper
   with width first and children second; clean eight bytes. Null advances
   through the current bucket's live next pointer; -1 selects prefix insertion;
   a node pointer selects interior insertion. These are three distinct forms.
4. Exhausting the list reads the page head first, then descriptor height,
   and calls the real bucket helper with that newly read height. It differs
   from the initially empty-slot path, which uses the cached height.
5. After bucket creation, discard its EAX as a status, reread descriptor height
   and return if signed height <= 0. Otherwise go back to the second height
   read/check in step 1, followed by the width checks and fresh table lookup.
   A new height > 256 therefore exits on retry. There is no rollback of the
   bucket/page created before such an exit, and no retry limit or cycle guard.
6. On either successful gap form, call Gfx_AllocBytes(24), capture the new
   node, and copy six ascending template dwords with alternating reads/stores.
   Template contents are live after allocation; copied start/children/previous
   are not assumed zero. The container/predecessor addresses stay captured.
7. Link the node using one of the ordered paths below, snapshot the descriptor,
   change its pixels/stride, and run the real pixel leaf.

The dependencies page, gap, bucket, link and allocation are already reconstructed;
their [separate validation](windows_gfx_sprite_packing_buckets.md) does not
establish storage reconstruction. No invented bounds, allocation-success,
pointer, liveness or rollback guards are justified here.

## Ordered insertion paths

Prefix insertion (VAs 0x004615AA..EA):

1. After template copy, reread descriptor width; store it at new node end.
2. Read captured container pixels, then live new node start; add modulo 2^32
   **without shifting**, and store new node pixels.
3. Reread captured container children; store that pointer at new node next.
4. If nonnull, store new node at that saved child's previous field.
5. Publish new node at captured container children last. Copied node previous
   remains untouched unless an exact alias makes a later store affect it.

Interior insertion (VAs 0x00461610..57):

1. After template copy, read captured predecessor end; store new node start.
2. Read live descriptor width first, then reread predecessor end. Add modulo
   2^32 and store new node end. This width-before-end order matters for aliases.
3. Read captured container pixels first, then live new node start; add modulo
   2^32, without shifting, and store new node pixels.
4. Read live predecessor next, then call the real link helper with captured
   predecessor, new node and saved successor; clean twelve bytes. Its ordered
   stores are current next, current previous, previous next and successor
   previous when nonnull. Its incidental EAX is discarded.

Neither path rereads dimensions for validity after allocation or frees anything.
Allocator-boundary width changes can therefore alter the end despite the prior
gap decision. Cached container/predecessor addresses are distinct from live
pixel, range, children and successor words. A production implementation must
retain these reads/order rather than cache an entire node or descriptor.

## Snapshot, copy and ownership effects

At VA 0x00461665, copy sixteen ascending descriptor dwords into the local
64-byte snapshot at post-prologue ESP+0x18. This happens **after** insertion,
so exact descriptor/node aliases can make the snapshot reflect inserted fields.
Then read new node pixels, store them at descriptor +0x10, store 256 at
descriptor +0x14, and call RVA 0x612A0 with the snapshot as source and live
descriptor as destination. Descriptor pixel/stride changes occur after all
snapshot reads. Ordinary distinct descriptors retain the old source pixels
and stride in the snapshot while targeting the assigned page bytes.

There is no source-pixel free, descriptor ownership registration, image-ID
update, node pointer store into +0x18/+0x1C, or cleanup on return. The new node
is owned through the captured container's list. Faults in the final copy leave
that insertion and the pixel/stride rewrite in place. Initial source pointers
are not validated by storage before it changes state.

## Complete pixel-leaf contract

Gfx_CopySpriteDescriptorPixels takes source descriptor first, destination second,
with caller cleanup of eight bytes. Capture both descriptor addresses. Read
source pixels first, destination pixels second, before testing any dimension.
Set row index zero; compare live source height to row index as signed dwords.
For each admitted row, set column zero and compare live source width to column
as signed dwords. Each admitted column reads one source byte then immediately
stores one destination byte at the same column. Increment column and reread
source width for the next comparison. A nonpositive width skips byte accesses.

After each row, including a row with no columns, read source stride and add it
to the saved source-row pointer modulo 2^32, then read destination stride and
advance the saved destination-row pointer modulo 2^32. Increment row, reread
source height and continue only when signed height > row. Initial nonpositive
height skips all row/stride/byte accesses, but still reads both pixel pointers.
Destination dimensions are not read. EAX retains the source descriptor pointer;
the leaf preserves EBX/ESI/EDI/EBP, ordinary RET and clear DF. It makes no calls.

This is increasing forward byte copying, not memmove: copying `abcde` into
the next byte produces `aaaaaa` across six bytes. Pixel pointers are captured
once; dimension/stride reads remain live, so descriptor/pixel aliases can change
later loop bounds or row steps. No checks for pointer validity, upper dimension
bounds, overlap, signed stride, size overflow or exception recovery are present.
Arbitrary mutation, large live dimensions and concurrency remain unvalidated.

## Failure, aliases and termination limits

Node allocation failure attempts the first template read and null node store,
before range/link changes or descriptor snapshot. Existing unchecked page,
aligned allocation and bucket failure effects remain as separately recovered.
Unmapped buckets, containers, children, predecessors and prefix repair targets
fault at the ordered attempted access. A bad source pixel pointer faults only
after insertion, snapshot, descriptor rewrites and entry to the pixel leaf.
There is no rollback, leak correction or clean null result.

Exact node/container/predecessor/descriptor aliases are consequences of the
ordered accesses above; they are statically recovered here, not exhaustively
executed. Template/global-storage/caller-stack aliases, partial overlaps,
fault-time register/stack frames, general reentry/concurrency and native heap
exceptions are not certified. Storage and dependency list cycles can fail to
terminate; this feature adds no execution evidence for cycle prefixes.

## Original-only probes and completion scope

Thirty-two independently hand-derived original-instruction probes ran in
Unicorn 2.1.4 after target authentication. These execute authentic storage,
pixel, page, bucket, gap, link, wrapper and aligned-helper bodies; only CRT
malloc is intercepted, with volatile-register clobbering and explicit reply/
mutation schedules. They are analysis corroboration, not compiled-C comparisons
and not new passing reconstruction records. Every normal return checks stack
cleanup, caller-stack bytes, nonvolatile preservation and clear DF.

Twenty-five storage probes cover ten nonpositive/oversized signed dimension
rejections; all four 1/256 width/height corners; prefix with arbitrary copied
template fields and existing-child repair; interior tail insertion; live width,
predecessor-end and container-pixel mutation at allocation; wrapping prefix
pixels; empty-table real page creation with normal retry and mutated height
0/257/-1 exits; exhausted-bucket creation/retry; null node allocation and
unmapped source-pixel faults. Hand-derived node/list/descriptor snapshots,
malloc sizes and row bytes are asserted for the applicable paths. In the
empty-table success case actual CRT sizes are 24,131076,24,8,24. The two faults
check the expected address and state effects; native exception parity is excluded.
Seven standalone pixel probes cover nonpositive width/height, 1x1 and 3x2
copying with distinct source/destination strides and forward overlapping copy.

Local disposable reports: build/workflow/storage-ghidra.json and
build/workflow/storage-original-probes.json; the disposable driver is
build/workflow/storage_analysis.py, run with
`uv run python build/workflow/storage_analysis.py`. These ignored outputs are
not committed decompiler output, binary assets, production tests or permanent
verifier contracts. The instruction-derived contract and hand-derived scenario
expectations above are the committed evidence. No new verification_runs are
created for either analyzed RVA. Future reconstruction must extend the real
verifier with reproducible differential contracts and independent expectations.

PowerShell uses UV_CACHE_DIR=build/uv-cache and UV_OFFLINE=1. Completion:
`uv run python tools/workflow.py complete --analysis-only --rva 0x61530 --rva 0x612A0 --limitation "Storage and pixel leaf analyzed only; original-only probes with modeled malloc; no new compilation/differential evidence or native graphics/heap/game parity"`.
Then review the explicit feature files, verify recorded LF working/index inputs,
run the staged gate and tracked hooks, and commit the synchronized exports.

Inventory remains 1,028 candidates and 42 reconstructed routines. All retained
reconstruction inputs and artifacts are audited for freshness; no shared source,
builder or verifier changes require rerunning their suites for this analysis.
Retained counts include bucket 2,734 (1,140 persistent, 127 faults, two cycle
prefixes), page 1,574 (816 persistent, 134 real-dependency, six faults), wrapper/
aligned 48/512, gap 1,835, link 864, reset 810, default 280, descriptor helper
486, handles 48, each workspace 167, installers 306/266, selector 532, banner/
input 306/266, startup/shutdown 112/519, 18 integrated startups and ten persistent
lifecycle calls; memory/font/file/backend coverage is unchanged. These counts
are retained evidence, not freshly rerun cases in this analysis feature.

Next production feature: reconstruct the 61-byte pixel leaf with live bounds,
ordered byte/stride accesses, wrapping pointers, overlap and fault cases; then
reconstruct storage using its real body and all real packing dependencies.
Instruction equality, original compiler/link layout, native graphics/heap/game
parity and playable rebuilding remain unverified. Release RVA 0x618B0, workspace
renderers and real primitive startup remain unreconstructed; CRT and callbacks
retain their modeled boundaries.
