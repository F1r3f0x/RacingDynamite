# Windows sprite packing gap search

Authentic input: Ignition/Ignition/IGN_WIN.EXE, 915,968 bytes, SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782,
PE32 i386, preferred image base VA 0x00400000. The local target doctor and
routine/FPO checks authenticate local bytes independently of the user-provided
assumption that IGN_WIN.EXE is active in Ghidra. Read-only localhost:8080
function disassembly, decompilation and xrefs corroborate the bounded helper
and both callers. No automated Ghidra identity verification is claimed.
Original binaries/assets are untouched. The name and geputget.c placement are
semantic, not recovered symbols. No DOS findings supply this contract.

## Provenance and ABI

Gfx_FindSpritePackingGap: VA **0x00461870**, RVA **0x61870**, **62 bytes**,
FPO (62,0,2,0x105), SHA-256
9fec91b9a7a79937df056da6cea0f29c91ccd4aa4549a1eef45cf0716a71872d.
All 27 instructions decode through RET at VA 0x004618AD. Two INT3 bytes
follow before the separate release routine. The body has no calls, global
operands or HIGHLOW relocations; no relocated entry pointer is found.
A complete FPO direct call/jump scan and Ghidra xrefs both find exactly
VAs 0x0046158D and 0x004616E9. Computed/untracked references remain possible.

Two caller-cleanup stack arguments: signed requested extent first, node-list
head second. The entry loads the second word from ESP+8 before PUSH ESI;
it loads the first word from ESP+8 afterward. Both callers ADD ESP,8.
The body preserves EBX/ESI/EDI/EBP, caller stack and clear DF. EAX carries
one of three meaningful raw pointer representations. Volatile registers and
arithmetic flags are not extra return contracts.

## Exact bounded contract

1. A null first node returns pointer bits 0xFFFFFFFF without any node read.
2. Read first->range_start (+0). If its signed value is greater than or
   equal to the signed request, return 0xFFFFFFFF. Equality admits prefix space.
3. Otherwise, repeatedly read current->next (+0x14). Set the upper bound to
   256 when null, or read next->range_start (+0) when nonnull.
4. Read current->range_end (+4) after the successor start. Subtract this end
   from the upper bound modulo 2^32, then compare the resulting signed dword
   with the signed request. If greater than or equal, return current.
5. On a failed gap comparison, advance to the saved successor. If null,
   return null; otherwise repeat step 3 without retesting that node's start
   against the request.

No writes, allocations, frees, callbacks, pixel/child/previous reads or global
accesses occur. Prefix space returns the special predecessor marker, a gap
after a node returns that node, and exhaustion returns null. The sentinel
is not an allocation error or a node to dereference. The fixed trailing
boundary is 256. The complete 24-byte node view comes from the independent
[reset/consumer evidence](windows_gfx_sprite_packing_reset.md); this routine
uses only +0/+4/+0x14. Other range bits remain raw in the shared type.

JL and JGE establish signed comparisons. The subtraction is machine dword
wrapping: production performs unsigned subtraction then a target-specific
int32_t interpretation, avoiding signed C overflow. There is no request
validation or range sorting/clamping. Zero, negative and high-bit request
or endpoint words retain their original consequences. Ordered volatile reads
retain the initial short circuit and successor-start-before-current-end order.

There are no pointer validity checks or cycle guards. Invalid pointers fault
at the attempted read; a prefix exit can avoid an invalid successor. A cycle
with no qualifying gap can run forever, while a qualifying gap can return
from a cyclic list. Validation stops two such nonproductive traversals
externally after 31 reads. This is a bounded execution-prefix comparison,
not proof of general termination or a production behavior change. Native
exception behavior and arbitrary concurrent mutation are unverified.

## Both callers (static analysis only)

- Gfx_AssignSpritePackingStorage, RVA 0x61530: 350 bytes, FPO
  (350,18,1,0x140B), SHA-256
  028b7432fafcd6c13a8ea0d675bda862a30ca19e99fbd6a3ec3e9a4344f9c394.
  At VAs 0x00461582..8D it reads a bucket node from ESI+4, passes that
  node's children (+0x0C) second and descriptor word +4 first. It saves EAX,
  removes eight argument bytes, walks to the next bucket on null, and tests
  the saved result against -1 before selecting prefix versus interior insertion.
- Gfx_AddSpritePackingBucket, RVA 0x616C0: 273 bytes, FPO
  (273,1,2,0x140B), SHA-256
  6da9083d32b7bd4186fb626b6fb5fe0fc099a6d769e332c98805b6d9686700ed.
  At VAs 0x004616E0..E9 it passes the current page's children second and
  its first caller argument first. After cleanup it saves EAX to EBP,
  walks the page's next link on null, and compares EBP with -1 before
  selecting prefix versus interior insertion. The storage caller passes
  descriptor word +8 as that requested bucket extent.

Authenticated full caller hashes, FPO tuples and local argument/cleanup/result
instruction sequences are asserted by the real verifier. Existing page EAX
consumer checks remain. Neither complete packing caller is reconstructed or
executed by this feature. Helper validation does not promote either caller.

## Compilation and differential verification

The builder exports the exact extracted production C89 declaration/body and
existing node type in the focused PE32 DLL. Provisional Clang/LLD 19.1.1,
i686-pc-windows-msvc, -O2 and existing strict/freestanding/no-inline/no-SSE
flags remain. The aggregate symbolic call-owner guard knows the new leaf;
existing ordered dependency checks remain and the leaf adds no calls.

The independent raw-offset oracle checks exact ordered reads, full one-MiB
arena/template/head state, unchanged unrelated image bytes, meaningful EAX,
normal caller stack/nonvolatile-register/DF ABI, and fault address/read effects.
Boundary words include zero, one, 255/256/257, signed extrema and neighbors.
Cases cross prefix comparisons, fixed tail gaps and wrapped subtraction
boundaries with requests exactly at/around the available extent. Unsorted,
overlapping and arbitrary seeded chains retain their raw behavior. Explicit
hand-derived results check prefix, first and later fits, exhaustion, successful
self-cycles and full eight-node traversal. Repeated searches and varied request
sequences run without resetting image/CPU state. Unconsumed node fields remain
arbitrary. Invalid heads, invalid successors and a truncated mapped node are
included, plus two externally bounded nonproductive cycles.

Fresh coverage: **1,835 comparisons per binary**, **922 persistent follow-ups**,
**69 read-fault cases**, and **two nonreturning cycle prefixes**. Native pointer
fault behavior is not a portable C guarantee. Instruction equality, original
compiler/link layout and native graphics/heap/game parity remain unverified.
There is no playable rebuilt executable.

Retained coverage: page 1,574 (816 persistent, 134 real-dependency, six faults),
allocation wrapper/aligned 48/512 (24/252 repeats, eight aligned faults),
link 864 (464 persistent), packing reset 810 (405 repeats; 922 real bodies),
default initializer 280 (132 repeats; 392 real bodies), descriptor 486
(1,156 real bodies), handles 48 (654 real bodies), each workspace 167
(83 repeats; 757 real bodies), sprite/surface installers 306/266, selector 532,
banner/input 306/266, startup/shutdown 112/519, 18 integrated startups and
ten persistent lifecycle calls. Original-only consumer ABI 48/39, downstream
contracts 43 and default checks 16 remain, as do memory/font/file/backend suites.
The primitive driver executes real default initialization then real packing
reset in all 112 startups; real RVA 0x5E610 is not executed. Remaining primitive
effects, CRT and callbacks stay modeled. Packing callers and workspace renderers
remain unreconstructed.

Use PowerShell, UV_CACHE_DIR=build/uv-cache, UV_OFFLINE=1 and Python through uv.
Focused: `uv run python tools/verify_sprite_packing.py`.
Full: `uv run python tools/workflow.py complete --rva 0x61870 --limitation "Packing callers unreconstructed; primitive/CRT/callbacks modeled; instruction equality and native graphics/heap/game parity unverified"`.
Then verify current input hashes against LF working bytes and Git index blobs,
review/stage explicit feature files, and pass `workflow.py check --staged` and
tracked commit hooks. Inventory: 1,028 candidates, 41 reconstructed routines.
