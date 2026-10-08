# Windows default sprite descriptor initialization

Authentic input: `Ignition/Ignition/IGN_WIN.EXE`, 915,968 bytes, SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782,
PE32 i386, preferred base VA 0x00400000. Original assets are untouched.
The user's assumption that this program is active in Ghidra is separate from
the locally verified fingerprint. Read-only localhost:8080 inspection of the
initializer, primitive caller, image-ID consumer, default-copy consumer and
shared-string consumer corroborates authenticated local instruction decoding.
No DOS evidence supplies addresses, ABI or storage layout.
Routine/global names and geputget.c placement are semantic; original symbol
and source-file names have not been recovered.

## Provenance and contract

`Gfx_InitDefaultSpriteDescriptor`: VA **0x004611D0**, RVA **0x611D0**,
53 bytes, FPO tuple (53,0,0,0), routine SHA-256
bb24fdcef5b8b3fdd31746c899ca6523d467479ebc00098f5be527e9e414d70a.
Ten instructions end at ordinary RET VA 0x00461204, followed by eleven INT3
bytes before the next entry. There are no calls, stack arguments or data reads.
XOR EAX,EAX precedes six zero stores; subsequent stores leave EAX zero through
RET. The C boundary returns int zero. A no-argument RET cannot distinguish
cdecl from stdcall. EBX/ESI/EDI/EBP, caller stack and clear DF are preserved.

The eight ordered dword writes are:

| Order | Storage VA | Effect |
| --- | --- | --- |
| 1 | 0x0051FB88 | Default descriptor word 0 = VA 0x004BACF8 |
| 2..7 | 0x0051FB8C..0x0051FBA0 | Default words 1..6 = zero, ascending |
| 8 | 0x00520380 | Next image-ID search word = one |

Words 7..15 of the existing 64-byte descriptor copy view are untouched, even
after repeated initialization. Capacity VA 0x00520388 and table pointer VA
0x0052038C are independent globals and are neither read nor written here.
No allocation, destruction, table initialization, error handling or guard exists.
Production uses the existing volatile descriptor and explicit stores; it adds a
separate raw uint32_t search word and an eight-byte NUL-terminated `default`
string. A pointer-to-dword cast preserves the original 32-bit representation;
this is a PE32 reconstruction, not a 64-bit portability claim.

Nine HIGHLOW operands at RVAs 0x611D2/0x611D6/0x611DD/0x611E2/0x611E7/
0x611EC/0x611F1/0x611F6/0x611FC bind the descriptor, string, six subsequent
words and search word. Default storage and search storage lie in .data's virtual
tail, beyond raw bytes, so fresh loader contents are zero. The string is
file-backed .data at RVA 0xBACF8 and contains `default` plus NUL. It is also
referenced by separate primitive finalization RVA 0x60B50; this does not make
that routine a dependency of this initializer. No complete semantic descriptor
layout or encompassing image-table structure is inferred.

Ghidra xrefs and a complete FPO direct-call scan find the call at VA 0x0045E738
inside primitive initialization RVA 0x5E610. There are no relocated code pointers
to this entry. These are bounded reference findings, not proof against computed
aliases. The caller first copies a separate zero packet to VAs 0x00520380..8C,
including capacity/table, then calls two other initializers before this one.
Its next instruction calls RVA 0x614D0 without consuming EAX. Image operation
RVA 0x61360 reads/advances the search word while scanning table entries and
lowers it on release; this independently supports the search-word name without
transferring that unreconstructed routine's behavior into production C.

## Differential validation and integration

`tools/build_decomp.py` extracts the exact production declaration, globals and
body into the existing focused validation DLL. Strict C89, provisional
Clang/LLD 19.1.1, i686-pc-windows-msvc, -O2, freestanding/no-builtin, disabled
inlining/unrolling/vectorization/SSE are retained. The DLL has no imports and
is not a playable rebuilt executable. Symbolic dependency checks include this
new leaf; the existing 31 generated direct calls are retained.

The lifecycle suite adds 280 standalone differential comparisons, including
132 persistent repeats without CPU/image reset. Inputs cover zero/all-one/
high-bit words, pointer-like tails and 128 seeded arbitrary descriptors, arbitrary
prior search/capacity/table bits, and sixteen additional tail/client cases.
An independent instruction-derived oracle checks exactly eight ordered stores,
no tracked reads, unchanged nine-word tails and other state, full arena bytes,
EAX=0, stack/nonvolatile registers/DF and unrelated image bytes. Only verified
word-zero name pointers are translated between linked address spaces; tail bits
are never rewritten. Both compiled and original string bytes are checked.
The prior sixteen original-only initializer cases remain separately recorded.

Sixteen additional chains execute real descriptor copying and real handle
initialization after the new initializer. Startup integration executes the real
new body in all 112 startups, including 94 isolated wrapper cases and 18
integrated startups (four persistent). Total new-body executions are 392 per
binary. The validation-only primitive driver models selected search/capacity/
table reset effects, redirects execution to the real initializer, verifies its
zero result, then supplies the independently varied modeled primitive return.
**The real primitive initializer RVA 0x5E610 is not executed.** Its other reset,
helper and tail-jump effects remain excluded. No production primitive substitute
or success-returning dependency is added; its nonreturning fixture remains.

Retained standalone comparisons: 486 descriptor helpers, 48 handle initializers,
167 per workspace (83 persistent repeats), 306 sprite installers, 266 surface
installers, 532 selectors, 306 banners, 266 input resets, 112 startups and 519
shutdowns, plus ten persistent lifecycle calls. Real helper/handle body totals
increase to 1,156/654 per binary; each workspace retains 757 real executions.
The 48/39 original-only sprite/surface consumer ABI checks, 43 downstream
initializer contracts and all memory/font/file/backend suites remain intact.

Commands in PowerShell, UV_CACHE_DIR=build/uv-cache and UV_OFFLINE=1:

```powershell
uv run python tools/verify_mem_lifecycle.py
uv run python tools/workflow.py complete --rva 0x611D0 --limitation "Primitive driver, CRT and callbacks modeled; no instruction equality or native game parity"
uv run python tools/db.py update --check
uv run python tools/workflow.py check --staged
```

Compilation and differential emulation establish bounded behavior evidence;
EXACT denotes the recovered behavior, not instruction equality. Raw compiled
prefixes differ. Original compiler/link layout, relocation-aware instruction
equality, native graphics/heap/game parity and a playable reconstruction remain
unverified. Primitive/CRT/callback boundaries remain modeled; workspace consumers
remain unreconstructed. Invalid/unmapped storage, general reentry and concurrency
are outside this contract. Inventory: 1,028 candidates, 35 reconstructed routines.
