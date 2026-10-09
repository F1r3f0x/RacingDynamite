# Windows sprite packing bucket insertion

Authentic IGN_WIN.EXE: 915,968 bytes, SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782,
PE32 i386, preferred base VA 0x00400000. The target doctor authenticates local
bytes. Read-only localhost:8080 disassembly, decompilation and xrefs corroborate
local decoding under the user-provided assumption that this executable is active
in Ghidra; no automated Ghidra identity check is claimed. Original binaries and
assets are untouched. Names/module placement are semantic, not recovered symbols.
No DOS evidence supplies this contract.

## Provenance and ABI

Gfx_AddSpritePackingBucket: VA **0x004616C0**, RVA **0x616C0**, **273 bytes**,
FPO (273,1,2,0x140B), SHA-256
6da9083d32b7bd4186fb626b6fb5fe0fc099a6d769e332c98805b6d9686700ed.
Complete instruction decoding ends at RET VA 0x004617D0, followed by fifteen
INT3 bytes. Five HIGHLOW operands at RVAs 0x616D6/0x61704/0x61756/0x61760/
0x617BC select page head VA 0x0051FE40, template VA 0x0051FC00 and bucket table
VA 0x0051FF58. No relocated pointer to its entry is found. FPO call/jump scanning
and Ghidra xrefs find the sole direct call VA 0x004615F6 in RVA 0x61530.
Computed/untracked references remain possible.

Two cdecl stack arguments: signed requested extent, initial page-list head.
At VAs 0x004615EC..F6 the storage caller reads the global head, then descriptor
word +8, and pushes head then extent. ADD ESP,8 follows; EAX is discarded before
rereading descriptor +8 to decide whether to retry. The full storage routine
remains analysis-only; this feature does not certify its complete contract.
The bucket routine returns the allocated bucket pointer in incidental EAX,
uses ordinary RET, and preserves EBX/ESI/EDI/EBP, caller stack and clear DF.
Volatile registers/flags are not extra return contracts.

## Complete bounded contract

The initial page argument is used even when different from the global head.
A null page invokes the real page allocator, discards its EAX, reloads the global
head and retries. Otherwise read page children (+0x0C), then call the real gap
helper with extent first and children second. Null gap advances through the
page next link (+0x14). Exhausting pages allocates another page and reloads the
head; the 0xFFFFFFFF gap marker selects prefix insertion, while a node pointer
selects insertion after that predecessor. There are no extent, pointer, cycle,
allocation-success or newly-created-page fitness guards.

Both insertion paths call Gfx_AllocBytes(24), then alternate six ascending
volatile template dword reads and node stores. The node view remains 24 bytes:
range start/end at +0/+4, pixel pointer +8, children +0x0C, previous +0x10,
next +0x14. Arbitrary template fields are copied; no invented clearing occurs.

Prefix insertion performs these ordered accesses:

1. Store requested extent at node +4, retaining the copied start.
2. Read cached page +8, then read live node start; shift start left eight bits
   modulo 2^32, add to the pixel pointer modulo 2^32, and store node +8.
3. Reread cached page children; save this pointer and store it at node +0x14.
4. If nonnull, store node at saved child's previous field (+0x10).
5. Store node at cached page children last. Copied node previous remains untouched.

Interior insertion performs these ordered accesses:

1. Read cached predecessor end and store node start.
2. Reread predecessor end, add extent modulo 2^32, and store node end.
3. Read live node start, shift left eight bits, then read cached page pixels;
   wrapping addition supplies node pixels.
4. Read live predecessor next, then call the real link helper with saved
   predecessor, node and successor. Its ordered stores are node next, node
   previous, predecessor next, then nonnull successor previous.

Finally call Gfx_AllocBytes(8). Only afterward form the table slot by wrapping
32-bit extent multiplication by four and addition to table base. Read its live
old pointer; store it at bucket +0, then store the cached new node at bucket +4,
then publish the new bucket to the slot. Return that bucket pointer. There is
no node/bucket cleanup or rollback. The eight-byte bucket view is independently
established by these stores and the storage/release consumers, not by a guessed
host pointer size. The 257-slot table has no index bounds check here.

## Aliases, allocator mutation and failure

Page/predecessor addresses and the new-node pointer remain captured across
allocation. Template fields, predecessor end/next, page pixels/children and the
table slot contents are read live where listed. The page head is consulted
only after creating a page. Replacing a live read with a cached word changes
alias and allocation-boundary mutation consequences. Exact arena aliases among
page, predecessor, successor, new node and bucket are compared. For node equal
to predecessor, the first store and second end read are distinct; for node equal
to page, copying and pixel/child stores can alter later page reads. Bucket aliases
can overwrite previously linked nodes before table publication. The original
ordered effects are retained without invented unlinking or ownership repair.
Bounded malloc mutations test the timing; they do not establish arbitrary reentry,
concurrency or a native malloc mutation guarantee. Template/global-storage aliases,
caller-stack aliases, partial overlaps and arbitrary mapped out-of-table addresses remain unvalidated.

Node allocation failure attempts the first copied store at null before field
initialization or bucket allocation. Bucket allocation failure occurs after node
insertion; it reads the live table pointer before its attempted null store, leaving
the inserted node and existing table publication intact. Invalid initial pages,
child pointers, prefix repair targets and truncated writable nodes retain their
unchecked faults and preceding effects. Real page node/pixel failures retain the
separate page/helper failure effects. No exception-to-clean-null behavior is added.
Native exceptions and portable C definedness for invalid pointer accesses are
not asserted; memory/call effects are compared for the provisional x86 build.
Fault-time register/stack frames are compiler-dependent and are not compared;
register preservation and stack cleanup are asserted on normal completion only.

Nonproductive child/page cycles can continue indefinitely. External read budgets
compare finite prefixes without adding production guards or proving liveness.

## Compilation and execution evidence

Exact production C89 declarations/bodies are extracted into the focused PE32
validation DLL using provisional Clang/LLD 19.1.1, i686-pc-windows-msvc, -O2,
strict/freestanding/no-builtin/no-inline/no-unroll/no-vectorize/no-SSE flags.
Page, gap, link, wrapper and aligned helpers execute real reconstructed bodies;
only CRT malloc is modeled. No success-returning production stub is introduced.
The aggregate symbolic dependency guard checks the compiled bucket's five ordered
calls and all existing dependency ownership checks. Different optimized control
flow merges the two original node-allocation sites and page-allocation sites;
this is no claim of original instruction equality.

An independent raw-address oracle checks ordered reads/writes/dependency entries,
complete one-MiB arena, template, page head and full bucket-table state, allocation
entry snapshots, unrelated image preservation, normal EAX/stack/nonvolatile/DF
ABI, and fault kind/address/preceding effects. Coverage includes all 257 slots,
zero/high-bit/wrapping extent words, arbitrary template bits and pixel shifts,
all three gap return forms, multiple pages, both page-creation entry routes,
exact arena aliases, live allocation mutations and persistent insertion chains.
Fresh coverage: **2,734 comparisons per binary**, **1,140 persistent follow-ups**,
**127 fault cases**, and **two externally bounded cycle prefixes** stopped after
31 reads. Bucket calls enter the real page helper 64 times, gap helper 2,753
times and link helper 352 times, including failure/nonreturning paths. There are
2,304 completed prefix insertions and 301 completed interior insertions. These
helper-entry totals are additional integration coverage, not extra standalone
comparison counts. Hand-derived eight-node/bucket final states independently
constrain the oracle's persistent chains. An unmapped out-of-table slot is also
compared after node insertion and bucket allocation.

Retained suites: page 1,574 (816 persistent, 134 real-dependency, six faults),
wrapper/aligned 48/512 (24/252 repeats, eight aligned faults), gap 1,835 (922
persistent, 69 read faults, two cycle prefixes), link 864 (464 persistent), reset
810 (405 repeats; 922 real bodies), default 280 (132 repeats; 392 real bodies),
descriptor 486 (1,156 real bodies), handles 48 (654 real bodies), each workspace
167 (83 repeats; 757 real bodies), sprite/surface installers 306/266, selector
532, banner/input 306/266, startup/shutdown 112/519, 18 integrated startups and
ten persistent lifecycle calls. Original-only consumer ABI 48/39, downstream
contracts 43 and default checks 16, memory/font/file/backend suites remain.
The primitive driver executes real default initialization then real reset in all
112 startups; it does not execute real RVA 0x5E610. Remaining primitive effects,
CRT and callbacks remain modeled. No new startup integration is claimed.

PowerShell with UV_CACHE_DIR=build/uv-cache, UV_OFFLINE=1 and Python through uv:
`uv run python tools/verify_sprite_packing.py`; full completion uses
`uv run python tools/workflow.py complete --rva 0x616C0 --limitation "Storage/release callers and workspace renderers unreconstructed; primitive/CRT/callbacks modeled; instruction equality and native graphics/heap/game parity unverified"`.
Verify LF working/index input hashes, explicit staged files and tracked hooks.
EXACT denotes recovered bounded behavior. Instruction equality, original
compiler/link layout, native graphics/heap/game parity and playable rebuilding
remain unverified. Inventory: 1,028 candidates, 42 reconstructed routines.
