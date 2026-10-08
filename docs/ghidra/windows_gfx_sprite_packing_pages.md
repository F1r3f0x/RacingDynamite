# Windows sprite packing page allocation

Authentic IGN_WIN.EXE, 915,968 bytes, SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782,
PE32 i386, preferred base VA 0x00400000. Local target fingerprint verification
is separate from the user-provided assumption that this program is active in
Ghidra. Read-only localhost:8080 disassembly, decompilation and xrefs corroborate
local instruction decoding. Original assets are untouched. Names and module
placement are semantic; no DOS evidence is used.

## Provenance and ABI

Gfx_AddSpritePackingPage: VA **0x004617E0**, RVA **0x617E0**, 85 bytes;
FPO (85,0,0,0x308); SHA-256
3e028a97ebd2430f4181d83bd55f460261c9506827cddcee83adce2ea65650b9.
Twenty-seven complete instructions end at RET VA 0x00461834, followed by eleven
INT3 bytes. Five HIGHLOW operands at RVAs 0x617E4/0x617FE/0x6181C/0x61824/
0x6182E select template VA 0x0051FC00 and head VA 0x0051FE40. No relocated
pointer to the entry is found. A complete FPO call scan and Ghidra xrefs find
two direct callers, VAs 0x004616CF/0x0046174F in the 273-byte bucket routine
RVA 0x616C0 (hash 6da9083d32b7bd4186fb626b6fb5fe0fc099a6d769e332c98805b6d9686700ed).
Both immediately load EBX from the head and loop; neither consumes EAX.
The prior [bucket ownership/layout evidence](windows_gfx_sprite_packing_reset.md)
remains applicable. The bucket routine stays analysis-only; no bucket caller
or startup execution integration is claimed.
Computed/indirect references are not excluded by the bounded scan.

There are no arguments; ordinary RET alone cannot distinguish cdecl/stdcall.
EBX/ESI/EDI/EBP, caller stack and clear DF are preserved. EAX is incidental:
it retains the aligned allocator result when the later head is null, otherwise
the head loaded for repair. Production returns that raw pointer solely to
preserve machine behavior; it is not a success flag or the new-page return.

## Ordered effects and allocation boundaries

1. Call Gfx_AllocBytes(24), with no null-result check.
2. Copy six dwords from the template, alternating ascending reads and writes.
3. Read the page head and store it at the new node's next field (+0x14).
4. Call Gfx_AllocAlignedBytes(65536,65536).
5. Store its result at new node +8 (pixels).
6. Reread head to test for null. If nonnull, read it again and store new node
   at that head's previous field (+0x10).
7. Publish new node to the independent page-head global last.

The [allocation dependencies](windows_gfx_alloc_aligned.md) are separately
reconstructed and tracked. Integrated tests execute their real bodies through
a modeled CRT malloc. The second CRT size is 131076 (0x20004), not 65536.
Only the first 24-byte node and the aligned allocation's back-pointer are
written; payload bytes remain uninitialized. There is no freeing, rollback,
list traversal, child clearing or invented previous=null assignment. Arbitrary
template fields, including previous/children, are copied as they stand.

The first allocation may change the template before its live copy. The head
saved into next before pixel allocation can differ from the head repaired
afterward. Replacing a reread with a cached old head would change behavior.
Volatile template/node/head accesses retain original order and repeated reads.
Exact new-node/old-head aliasing preserves the original self-link consequences.
Tests also allow bounded allocator mutations of template, node links and head;
these do not establish arbitrary reentry or a native malloc mutation contract.

Node allocation failure attempts the first copied store at null, before pixel
allocation and publication. Pixel allocation failure attempts the aligned
helper's unchecked low-address back-pointer store after the node copy/next
store; it leaves the new node allocated and the global head unpublished.
No guard or cleanup is added. Isolated validation may return arbitrary raw
words from the aligned boundary to prove consumption/order, including zero;
this is a validation model, not a claim that the real helper cleanly returns
zero after CRT failure. Native exception behavior remains unverified.

## Validation scope

The real verifier executes exact extracted production C89 with provisional
Clang/LLD, i686-pc-windows-msvc, -O2 and the existing strict/freestanding flags.
Both real dependencies are linked. The aggregate symbolic guard includes two
page calls in order and preserves all existing dependency checks. No production
success-returning dependency is added.

Independent raw-offset expectations check complete arena/template/head state,
ordered accesses/calls, dependency-entry and CRT-entry snapshots, consumed
allocation results, EAX, normal stack/nonvolatile-register/DF behavior, fault
kind/address and effects before failure, and unchanged unrelated image bytes.
Persistent sequences add six fresh pages without CPU/image reset. Exact
new-node/head aliases and arbitrary template words are included; template
storage aliases and partial overlaps are outside the validated contract.

Compilation and differential emulation are separate from instruction matching
and native execution. EXACT means recovered bounded behavior. Instruction
equality, original compiler/link layout, native graphics/heap/game parity and
a playable rebuilt executable remain unverified. Packing bucket/storage/release
algorithms and workspace renderers remain unreconstructed. Primitive startup,
CRT and callbacks retain their previously documented modeled boundaries.

Fresh coverage: **1,574 page comparisons per binary**, comprising 1,440 isolated
aligned-boundary cases and 134 real-dependency cases, including six fault cases.
There are 816 persistent follow-ups. Eight template patterns include all-zero,
all-one, high-bit, sequential and four seeded arbitrary states; head choices
include null, distinct and exact new-node alias. Six raw aligned-boundary
return patterns cross five live-head mutation choices. Sixteen two-call
mutation chains and sixteen six-page real-helper sequences complement these.
The separate dependency counts remain 48 wrapper and 512 aligned-helper cases,
including 24/252 persistent repeats and eight aligned-helper faults.

Retained lifecycle counts: link 864 (464 persistent follow-ups), reset 810
(405 repeats; 922 real bodies), default 280 (132 repeats; 392 real bodies),
descriptor 486 (1,156 real bodies), handles 48 (654 real bodies), each workspace
167 (83 repeats; 757 real bodies), sprite/surface installers 306/266, selector
532, banner/input 306/266, startup/shutdown 112/519, 18 integrated startups and
ten persistent lifecycle calls. Original-only sprite/surface consumer ABI
48/39, downstream initializer contracts 43 and default checks 16 remain.
Memory/font/file/backend suites remain. The primitive driver still executes
real default/reset leaves in caller order, not real RVA 0x5E610.

Use UV_CACHE_DIR=build/uv-cache, UV_OFFLINE=1 and Python through uv.
`uv run python tools/verify_sprite_packing.py` runs the focused suite;
`uv run python tools/workflow.py complete --rva 0x617E0 --rva 0x5F4A0 --rva 0x61840`
runs the real aggregate verifier, fidelity audit and SQLite export/check.
LF working/index input checks and staged gates precede commit.
Inventory: 1,028 candidates, 40 reconstructed routines.
