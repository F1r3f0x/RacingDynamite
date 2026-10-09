# Windows lazy sprite workspaces

Authentic input is `Ignition/Ignition/IGN_WIN.EXE`, 915,968 bytes, SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782,
PE32 i386, preferred base VA 0x00400000. Original assets are untouched.
The user-provided assumption that this binary is active in Ghidra is separate
from the locally verified fingerprint. Read-only localhost:8080 disassembly
of both initializers, all four direct buffer consumers and four code-section
adapters, and xrefs to flags/pointer slots, corroborate local complete decoding,
FPO extents and HIGHLOW operands. No DOS evidence or legacy type supplies a layout.

| Routine | VA / RVA | Bytes; FPO | SHA-256 |
| --- | --- | --- | --- |
| Gfx_InitSpriteWorkspaceA | 0x0045D840 / 0x5D840 | 84; (84,0,0,0) | 445faf9112110b216014f95c057ab6d572a81fec3fa64a867b9d5f03739c1d43 |
| Gfx_InitSpriteWorkspaceB | 0x0045C9F0 / 0x5C9F0 | 84; (84,0,0,0) | 922d88eb43e5d8b7ede3d71df0d81a543dc389bfad921e3faa13815d0ac5468e |

Both bodies decode to seventeen instructions and end in ordinary RET at VAs
0x0045D893/0x0045CA43, followed by twelve INT3 bytes. Each has six HIGHLOW
operands at relative offsets +2/+12/+33/+51/+69/+75. Three direct calls target
CRT malloc VA 0x00469400, each followed by ADD ESP,4. No stack arguments exist;
installer RVA 0x56E60 consumes neither result. Production prototypes are void;
incidental EAX is excluded. No-argument RET cannot distinguish cdecl/stdcall.

## Contract and separately owned storage

| Side | Flag VA | Reset word VA | Three pointer VAs |
| --- | --- | --- | --- |
| A | 0x004BACE8 | 0x0051AA50 | 0x0051AA5C / 0x0051AA60 / 0x0051AA64 |
| B | 0x004BACE4 | 0x0051A9CC | 0x0051A9D8 / 0x0051A9DC / 0x0051A9E0 |

The two flags are separate file-backed zero dwords. Reset/pointer words are
inside .data's virtual tail, beyond raw bytes. Loader zeroing does not justify
runtime contents. Ten independent production globals avoid inferring an enclosing
aggregate from adjacency. `Count` names the reset word provisionally; relocation
inspection finds only its initializer store, so no signedness/count semantics
or additional layout is asserted. uint32_t preserves the verified dword bits.

Each invocation first clears its reset word, then compares its flag with exactly
zero. Every nonzero value, including high-bit/noncanonical values, skips allocation
and preserves all three pointer words. Zero makes three ordered malloc calls,
each requesting 0x20D8 (8,408) bytes. Each result is immediately stored before
the next call, including null. All three calls execute despite independent failures;
the flag is then set to one. There is no memset, buffer read, allocation validation,
free, registration, retry or rollback in these bodies. A flag changed by malloc
does not cause the already-taken branch to be retested.

Repeated initialization with a nonzero flag resets only the reset word. Partial
allocation failure remains installed and is not retried. Explicitly clearing a
flag causes fresh allocation and overwrites old pointers without releasing them.
These original consequences are preserved; no corrective behavior is introduced.

## Consumers and ownership evidence

The four existing FPO candidates remain analyzed only. This feature performs
static storage/ownership checks; existing B triangle execution evidence in
[windows_triangle.md](windows_triangle.md) is retained separately and is not
promoted into production-C validation:

| Side / consumer RVA | Bytes / FPO | SHA-256 |
| --- | --- | --- |
| A / 0x5D8A0 | 1805 / (1805,0,1,0x140B) | 78d4b4b04194828bc6b6683a6890568822a230369f947edda82cb6222c4640c4 |
| A / 0x5DFB0 | 1561 / (1561,0,1,0x140F) | 739bf60339f6915a8460f79fdaeb474af0426aab014e2b386bd74c2e37ac433e |
| B / 0x5CA50 | 1813 / (1813,0,1,0x1408) | 4458bb7690590a3289e6ce7d99b3a80338382fe2afc3b67a25891b2af0e5f8bf |
| B / 0x5D170 | 1569 / (1569,0,1,0x140F) | d1d8d33f6f947f949ae7160d07da0fcc3e66d4a67168ad30c5eb80fa62f175f7 |

For each side, HIGHLOW scans find 20/17/18 references to pointer slots 0/1/2:
one initializer write per slot, followed by only loads in the two consumers.
Flags have exactly two relocated references each (initializer compare/store).
These are bounded direct-reference findings, not proof against all computed aliases.

For example, VA 0x0045DBDB loads A buffer 0 into EBX, then calls VA 0x0064EC40
with ESI pointing at a separate eight-dword input packet. VA 0x0045DC45 similarly
loads A buffer 1 for adapter 0x0064EC00; VA 0x0045DCAF loads buffer 2. Those adapters
store header dwords at EBX/+4, then pass EBX+8 to downstream writers. Clipped
adapters VAs 0x0064EFA0/0x0064F0C0 likewise write headers and retain derived
buffer cursors in VAs 0x004BAEB8/0x004BAEE4. They borrow workspace memory; they
do not free it or replace the workspace slot. The consumers inspect header word
+4 and step by twelve bytes from +8, reading fields at +8/+0C/+10. Examples are
VAs 0x0045DCD5..0x0045DCFE and 0x0045E293..0x0045E338; B has corresponding
accesses at VAs 0x0045CE8D..0x0045CEB6 and 0x0045D45B onward.

This establishes heap work-buffer use and persistent slot ownership sufficiently
for opaque `GfxSpriteWorkspaceBuffer *` storage. It does not establish complete
record meaning, buffer capacity, writer bounds, resource shutdown, native heap
ownership enforcement or safe consumer behavior after null allocation. No fixed
array or complete struct is inferred from allocation size. The reset word is
separate from each allocated buffer header. Adapters and downstream rasterizers
remain unreconstructed and are not added as reconstructed production substitutes.

## Production validation

`tools/build_decomp.py` extracts the exact production C89 workspace declarations,
globals and bodies from geputget.h/geputget.c into the existing focused DLL.
Both former nonreturning workspace fixtures are removed. CRT malloc remains an
explicit nonreturning fixture intercepted by the emulator; primitive startup,
printf, free and callbacks remain modeled. No native heap/graphics parity is implied.
`tools/verify_matching.py` extends symbolic dependency ownership by six malloc
calls; the compiler-specific guard now expects 31 direct calls.

Each workspace passes 167 standalone invocations, including 83 persistent repeats,
and executes 757 real bodies per binary across standalone and integrated coverage.
The lifecycle verifier uses independent instruction-derived effects and validates
full tracked state, complete arena bytes, exact read/write/call order, allocator
entry snapshots, caller stack, nonvolatile registers, DF and unrelated image bytes.
It exercises every independent allocation-failure combination, exact-zero and
noncanonical/high-bit flags, stale pointers, persistent skip, forced-zero pointer
replacement without free, arbitrary allocator-return bits and allocator mutations
between immediate stores. Pointer contents are never initialized by these routines.
Both real bodies execute through real sprite installation, backend selection and
lifecycle startup, preserving the real descriptor helper, handles and surface
installation. Original-only 43 initializer contracts, 16 default-initializer
checks and 48/39 sprite/surface consumer ABI checks remain separate evidence.
Retained comparisons: 486 descriptor helpers, 48 handle initializers, 306 sprite
installers, 266 surface installers, 532 selectors, 306 banners, 266 input resets,
112 startups (94 isolated), 519 shutdowns (74 isolated) and ten persistent lifecycle
calls. Integrated coverage executes 1,124 real helpers and 638 real handle bodies
per binary; 18 startups include four persistent invocations.

Commands: PowerShell with UV_CACHE_DIR=build/uv-cache and UV_OFFLINE=1;
`uv run python tools/verify_mem_lifecycle.py` is the focused verifier.
`uv run python tools/workflow.py complete --rva 0x5D840 --rva 0x5C9F0`
runs the full verifier, fidelity audit, synchronized exports and export check.
Complete analyzed consumer records separately with `--analysis-only` and their
four RVAs. Existing memory/font/file/sprite-backend validation is retained.

Strict C89 compilation and differential emulation are behavior evidence for this
bounded feature. EXACT records its recovered behavior, not instruction equality.
Raw prefixes differ. Original compiler/link layout, relocation-aware instruction
equality, native graphics/heap/game parity and a playable reconstruction remain
unverified. Invalid/unmapped pointers, general reentry and concurrency are outside
the validated contract. Inventory remains 1,028 candidates; 34 are reconstructed.
