# Windows sprite packing node link helper

Authentic input: `Ignition/Ignition/IGN_WIN.EXE`, 915,968 bytes, SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782,
PE32 i386, preferred image base VA 0x00400000. Local target doctor verifies
the file independently of the user-provided assumption that IGN_WIN.EXE is
active in Ghidra. Read-only localhost:8080 disassembly, decompilation and xrefs
corroborate the helper and its two callers; consumer disassembly corroborates
node link ownership. Original assets are untouched. Names and geputget.c
placement are semantic, not recovered symbols. No DOS evidence is used.

## Complete provenance and ABI

Gfx_LinkSpritePackingNode: VA **0x00461690**, RVA **0x61690**, 33 bytes,
FPO tuple (33,0,3,0), SHA-256
698f357becf28ec039300d9e01f27d48f7a04c675486b3506e7ca6107dded174.
Twelve complete instructions end with RET at VA 0x004616B0; fifteen INT3
padding bytes follow. There are no calls, absolute operands or HIGHLOW
relocations in the body. No HIGHLOW pointer to its entry is found. The full
FPO direct-call scan and Ghidra xrefs both identify exactly two direct callers;
computed or untracked indirect references are not excluded.

Three stack arguments are previous, current and next node, in that order.
They load from ESP+4/+8/+0x0C into EAX/ECX/EDX before any node store.
Both callers ADD ESP,0x0C after return, independently supporting cdecl.
There are no locals or nonvolatile-register writes. The clear-DF contract,
caller stack and EBX/ESI/EDI/EBP preservation are checked in emulation.
The body leaves EAX equal to the first argument. Both callers discard that
result; the production pointer return preserves the incidental machine value
without assigning a success or allocation meaning. ECX/EDX/condition flags
are volatile and are not an additional production return contract.

## Ordered effects and aliases

The complete instruction-derived contract is:

1. Store next at current+0x14.
2. Store previous at current+0x10.
3. If previous is nonnull, store current at previous+0x14.
4. If next is nonnull, store current at next+0x10.

There are no node reads, traversal, allocation, freeing or global accesses.
Current must be writable; no null-current guard is invented. Nonnull neighbors
must be writable. Existing node link bits are never followed. Range, pixel and
children words at +0/+4/+8/+0x0C remain untouched. Null neighbors cause no
neighbor dereference. Distinct mapped nodes and exact aliases are supported.
Arguments remain captured even when earlier writes overwrite aliased links.
For example, previous=current=next makes both current links point to itself;
previous=next distinct from current causes both neighbor links to become current.
No old-list unlink, head publication or cleanup is added. Volatile node lvalues
preserve all original stores and their order, including repeated same-address
writes. Partial overlaps, unmapped pointers and concurrent access are unvalidated.

The independently recovered [24-byte node view](windows_gfx_sprite_packing_reset.md)
retains raw range words and pointers at verified offsets. Allocation/template
copy instructions in callers and the page allocator establish the footprint;
page allocation/release instructions independently establish previous at +0x10
and next at +0x14. Bucket bodies remain opaque. The helper adds no dependency
to reset and does not reconstruct a packing algorithm.

## Callers (static analysis only)

- Gfx_AssignSpritePackingStorage, RVA 0x61530, call VA 0x00461652:
  EDX is the existing node and ECX the new node; pushes existing->next,
  new node, existing node. ADD ESP,0x0C follows, then descriptor copying;
  MOV EAX,[ECX+8] at VA 0x00461670 overwrites the unused result.
- Gfx_AddSpritePackingBucket, RVA 0x616C0, call VA 0x0046179F:
  EBP is the existing node and EDX the new node; pushes existing->next,
  new node, existing node. ADD ESP,0x0C follows, then PUSH 8 and an allocator
  call overwrites EAX. Bucket publication is outside this helper.

Authenticated caller hashes/extents remain guarded by the existing reset
inspection. Argument preparation, cleanup and subsequent instructions are
asserted by the link contract. Neither caller is reconstructed or executed
here; their existing analysis stage is unchanged. No modeled packing caller
or invented helper call is introduced.

## Compilation and differential validation

The existing focused builder extracts the exact production declaration and
body with the verified node type into its PE32 validation DLL. Strict C89
uses provisional Clang/LLD 19.1.1, i686-pc-windows-msvc, -O2,
freestanding/no-builtin, disabled inlining/unrolling/vectorization/SSE.
The leaf adds no direct dependency; all 31 existing symbolic direct-call
ownership checks remain. The DLL is not a playable reconstructed game.

864 standalone original-instruction versus compiled-C invocations per binary
check an independent raw-offset ordered-store oracle. Three adjacent mapped
24-byte nodes yield all 48 argument combinations (three writable current nodes
and four choices for each neighbor). Eight patterns cover zero, all-one,
high-bit, sequential and four seeded arbitrary node states. Those 384 initial
cases each repeat without image/CPU reset, for 768 calls. Sixteen explicit
six-call client relinking sequences add 96 calls; 464 total calls are persistent
follow-ups. All exact pair/triple aliases and null neighbor combinations occur.
Arbitrary stale links are never read. Exact two-to-four ordered stores, no reads
or calls, previous-pointer EAX, full tracked state and one-MiB arena equality,
unrelated image preservation, caller stack, nonvolatile registers and DF pass.
There is no packing-caller or startup integration for this helper.

Retained standalone comparisons: packing reset 810 (405 persistent; 922 real
bodies per binary), default initializer 280 (132 persistent; 392 real bodies),
descriptor helper 486 (1,156 real bodies), handles 48 (654 real bodies), each
workspace 167 (83 persistent; 757 real bodies), sprite/surface installers
306/266, selector 532, banner/input reset 306/266, startup/shutdown 112/519,
18 integrated startups and ten persistent lifecycle calls. Original-only
sprite/surface consumer ABI 48/39, downstream initializer contracts 43 and
default initializer checks 16 remain. Memory/font/file/backend suites remain.

PowerShell with UV_CACHE_DIR=build/uv-cache and UV_OFFLINE=1:
`uv run python tools/verify_mem_lifecycle.py` runs the focused contract;
`uv run python tools/workflow.py complete --rva 0x61690 --limitation "Packing callers unreconstructed; primitive/CRT/callbacks modeled; instruction equality and native graphics/heap/game parity unverified"`
runs the real aggregate verifier, audit and synchronized export/check sequence.
LF working/index input hashes and `workflow.py check --staged` precede commit.
Inventory: 1,028 candidates, 37 reconstructed routines.

EXACT denotes recovered behavior, not instruction equality. Raw prefixes differ;
instruction matching, original compiler/link layout and native runtime parity
remain unverified. Primitive initialization is a modeled driver executing real
default and reset leaves in authenticated order, not the real RVA 0x5E610 body.
CRT malloc/free/printf and callbacks remain modeled. Packing callers, workspace
renderers, native graphics/heap/game behavior and playable rebuilding are excluded.
