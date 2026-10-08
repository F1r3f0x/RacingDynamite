# Windows sprite packing state reset

Authentic input: `Ignition/Ignition/IGN_WIN.EXE`, 915,968 bytes, SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782,
PE32 i386, preferred base VA 0x00400000. Original assets are untouched.
Local fingerprint verification is separate from the user-provided assumption
that IGN_WIN.EXE is active in Ghidra. Read-only localhost:8080 disassembly and
xrefs corroborate the reset, its primitive caller and four storage consumers.
Names and geputget.c placement are semantic; original symbols are not recovered.
No DOS evidence supplies addresses, ABI or types.

## Reset provenance and ordered contract

Gfx_InitSpritePackingState, VA **0x004614D0**, RVA **0x614D0**, 61 bytes,
FPO tuple (61,0,0,0x103), SHA-256
c7c2d85ffd985343137e2d89bee0193091d6bc304ebb041e360202275906b7e5.
All fifteen instructions decode through RET at VA 0x0046150C; three INT3
bytes follow. It saves/restores EDI, uses REP STOSD with ECX=0x101, and makes
no calls or tracked data reads. EAX=0 through return, no stack arguments,
ordinary RET; no-argument cdecl/stdcall cannot be distinguished. Production
returns int zero, preserving nonvolatile registers, caller stack and clear DF.

Eight HIGHLOW operands at RVAs 0x614D5/DB/E0/E6/F3/F9/FF and 0x61508 bind
the six template fields, table base and page head. Ghidra xrefs and a complete
FPO direct-call scan find VA 0x0045E73D in primitive initialization RVA 0x5E610,
immediately after default initialization and before call VA 0x004607C0. The
caller consumes no result. No relocated code pointer to this reset is found;
computed aliases are not excluded.

The reset performs exactly 264 ordered dword stores:

1. Template VA 0x0051FC00: offsets +0x10, +0x14, +0x0C, +0, +4, +8 become
   zero in that nonmonotonic order.
2. Bucket table VA 0x0051FF58: all 257 pointers, including index 0 and 256,
   become null in ascending order; end address is VA 0x0052035C.
3. Independent page-list head VA 0x0051FE40 becomes null last.

There is no allocation, pointer dereference, cleanup, flag guard or failure
branch. Repeated initialization repeats every store. Existing pages/buckets
are orphaned without freeing their allocations. The production body preserves
this consequence. All three storage ranges lie beyond .data raw bytes inside
its virtual tail and are initially loader-zeroed. Adjacency does not establish
an enclosing aggregate; in particular the preceding primitive packet ends at
the separate page-head word, and image-ID/capacity/table words remain separate.

## Consumer-derived storage views (analysis only)

| RVA | Bytes / FPO tuple | Routine SHA-256 |
| --- | --- | --- |
| 0x61530 | 350 / (350,18,1,0x140B) | 028b7432fafcd6c13a8ea0d675bda862a30ca19e99fbd6a3ec3e9a4344f9c394 |
| 0x616C0 | 273 / (273,1,2,0x140B) | 6da9083d32b7bd4186fb626b6fb5fe0fc099a6d769e332c98805b6d9686700ed |
| 0x617E0 | 85 / (85,0,0,0x308) | 3e028a97ebd2430f4181d83bd55f460261c9506827cddcee83adce2ea65650b9 |
| 0x618B0 | 419 / (419,1,1,0x1410) | 7563059eb087b1961ce5aac8901e5ed29574c2f1a9354e62ed943d5842fb0072 |

These are static ownership/layout findings, not full recovered contracts or
new consumer execution evidence. The first accepts one caller-cleanup descriptor
pointer; the second has two caller-cleanup stack words; the page allocator has
none; release has one descriptor pointer. Their complete return semantics,
allocator failure behavior and runtime fidelity remain unrecovered.

Consumers allocate 0x18-byte nodes and copy six dwords from VA 0x0051FC00.
For example the page allocator at VAs 0x004617E3..FB selects this template,
allocates 0x18, and uses REP MOVSD with ECX=6. Node +0/+4 supply range endpoints;
production keeps their raw bits in uint32_t fields without asserting signedness.
At +8 consumers form/read pixel pointers, including row offsets shifted by eight
bits. Fields +0x0C/+0x10/+0x14 are child/previous/next links: the page allocator
links old head through +0x14 and fixes its +0x10 before publishing the new head;
release walks those links and updates child heads. This independently justifies
the six-field, 24-byte Windows node view and its four-byte packing. It does not
implement allocation or certify the packing algorithm.

RVA 0x61530 bounds descriptor words at +4/+8 to 1..256 and looks up table index
word +8. RVA 0x616C0 allocates eight-byte bucket records, storing next at +0 and
node at +4 before replacing the table slot. RVA 0x618B0 traverses/unlinks these
records, rewrites table slots, and can replace the page head. Production retains
an opaque bucket type and typed pointer array; no complete bucket body or heap
owner implementation is introduced. Direct references to template +0x0C/+0x10/
0x14 include only reset writes, while consumer copies borrow the entire template.

## Compilation and differential scope

The builder extracts the exact production C89 node declarations, globals and
reset body into the existing focused DLL. Clang/LLD 19.1.1 remains provisional,
i686-pc-windows-msvc, -O2, freestanding/no-builtin, inlining/unrolling/vectorization
and SSE disabled. No imports, playable reconstruction or original compiler/link
layout is certified. The leaf adds no generated direct call; existing 31 symbolic
direct-call ownership checks are retained.

Independent instruction-derived expectations cover 810 standalone differential
invocations including 405 persistent repeats without CPU/image reset. Every one
of the 257 slots gets a singleton nonzero case; zero/all-one/high-bit/sequential
and 128 seeded arbitrary tables complement arbitrary template and page-head bits.
Sixteen persistent client reinsertion pairs replace the head with mapped arena
pointers between resets. Exact 264-store order, no reads/calls, EAX=0, complete
tracked state and arena bytes, ABI/stack/DF and unchanged unrelated image bytes
are checked. Clearing pointers never modifies or frees pointed-to memory.

The modeled primitive driver executes real default initialization and then this
real reset in authenticated caller order, checking each leaf's zero result before
supplying the independently varied modeled primitive result. The real primitive
initializer is **not executed** and its remaining native effects are excluded.
This adds 112 real reset executions through startup, for 922 total per binary.
The nonreturning primitive fixture remains; no production stub fabricates success.

Retained standalone coverage: 280 default initializer comparisons (132 repeats),
486 helpers, 48 handle initializers, 167 per workspace, 306/266 sprite/surface
installers, 532 selectors, 306 banners, 266 input resets, 112 startups, 519
shutdowns and ten persistent lifecycle calls. The 18 integrated startups and
48/39 original-only consumer ABI, 43 downstream and 16 default contracts remain.
Real body totals remain 392 default initializers, 1,156 helpers, 654 handle
initializers and 757 per workspace per binary. All memory/font/file/backend
suites remain intact. Inventory: 1,028 candidates, 36 reconstructed routines.

PowerShell, UV_CACHE_DIR=build/uv-cache and UV_OFFLINE=1:
`uv run python tools/verify_mem_lifecycle.py`; full verification/audit/export:
`uv run python tools/workflow.py complete --rva 0x614D0 --limitation "Primitive driver, CRT and callbacks modeled; native game parity unverified"`.
Complete four consumer records separately using `--analysis-only` and their RVAs.
Then check LF working/index input hashes and `workflow.py check --staged`.

EXACT records recovered behavior, not instruction equality. Raw prefixes differ;
instruction equality, original compiler/link layout, native graphics/heap/game
parity and a playable rebuilt game remain unverified. Primitive/CRT/callbacks
remain modeled. Packing consumers and workspace renderers are unreconstructed.
Invalid/unmapped storage, general reentry and concurrency are outside this scope.
