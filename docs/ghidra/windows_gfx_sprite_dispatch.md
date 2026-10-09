# Windows sprite dispatch installation

Authentic input: `Ignition/Ignition/IGN_WIN.EXE`, 915,968 bytes, SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782,
PE32 i386, preferred base VA 0x00400000. No original assets were changed;
no DOS executable supplied evidence. Names are semantic, not original symbols.
The user assumes this binary is active in Ghidra; this is distinct from local
fingerprint verification. Read-only localhost:8080 disassembly of the installer
and all three initializers corroborated authenticated local decoding. Slot xrefs
and bounded decompilation of Configure, DrawList and ImageOp corroborate recovery;
several consumers/targets are undefined in Ghidra. Local FPO/instruction scans
recover those contracts independently rather than treating absent xrefs as proof.

## Installer and independently recovered slots

Gfx_InstallSpriteDispatch is VA 0x00456E60 / RVA 0x56E60, 181 bytes,
FPO (181,0,0,0), SHA-256
215bc3324a371904ec72c0e780eec9b178b47f01e195eaaac6124781c612a8b6.
Sixteen ten-byte MOV stores precede three CALLs at VAs 0x00456F00/05/0A;
MOV EAX,1 and ordinary RET end the body at VA 0x00456F14. Both operands of
each store have HIGHLOW relocations, 32 total. All destination words are
loader-zeroed .data virtual tail. The order below is the original instruction
order, including D8 before D4 and EC before E8. Four words at VAs
0x0050EBC4/C8/CC/D0 are untouched; their wrappers at RVAs 0x56D60/80/90/B0
have two/one/two/one caller-cleanup words. This installer does not initialize
them or certify their target identities. There is no guard, rollback or allocation
in the installer. Every invocation repeats all stores and downstream calls,
ignores downstream results, preserves caller stack/nonvolatile registers and
returns int 1. No-argument RET alone cannot distinguish cdecl/stdcall.

| Slot VA | Target RVA / bytes | Production suffix | Consumer RVA / words | Typed contract |
| --- | --- | --- | --- | --- |
| 0x0050EBA0 | 0x56F20 / 6 | Open | 0x56BC0 / 0 | void; int result |
| 0x0050EBA4 | 0x56F30 / 6 | Reset | 0x56BF0 / 0 | void; int result |
| 0x0050EBA8 | 0x56F40 / 6 | Close | 0x56CA0 / 0 | void; int result |
| 0x0050EBAC | 0x56F50 / 3 | Option | 0x56CB0 / 1 | unsigned int option; int result |
| 0x0050EBB0 | 0x56F60 / 187 | Configure | 0x56CC0 / 5 | unsigned char *pixels, int stride, int width, int height, unsigned int option; int result |
| 0x0050EBB4 | 0x57020 / 225 | SetClip | 0x56CF0 / 4 | int left, int top, int right, int bottom; int result |
| 0x0050EBB8 | 0x57250 / 96 | DrawList | 0x56D10 / 1 | const unsigned int *list; int result |
| 0x0050EBBC | 0x571B0 / 145 | Draw | 0x56D20 / 3 | GfxSpriteHandle *handle, Point2D *position, const GfxSpriteTransform *transform; int result |
| 0x0050EBC0 | 0x5C830 / 207 | HandleOp | 0x56D40 / 2 | const GfxSpriteDescriptor *descriptor, GfxSpriteHandle *handle; handle pointer result |
| 0x0050EBD8 | 0x61360 / 329 | ImageOp | 0x56DC0 / 2 | const GfxSpriteDescriptor *descriptor, int image_id; int result |
| 0x0050EBD4 | 0x5C920 / 24 | CreateDescriptor | 0x56DE0 / 2 | GfxSpriteDescriptor *descriptor, int image_id; int result |
| 0x0050EBDC | 0x5C940 / 19 | FreeDescriptor | 0x56E00 / 1 | GfxSpriteDescriptor *descriptor; int result |
| 0x0050EBE0 | 0x5C960 / 24 | CopyDescriptor | 0x56E10 / 2 | GfxSpriteDescriptor *descriptor, int image_id; int result |
| 0x0050EBE4 | 0x61A80 / 3 | Reserved | 0x56E30 / 1 | unsigned int option; int result |
| 0x0050EBEC | 0x57110 / 92 | GetState | 0x56E40 / 1 | GfxSpriteState *state; int result |
| 0x0050EBE8 | 0x57170 / 63 | SetState | 0x56E50 / 1 | const GfxSpriteState *state; int result |

All consumers forward argument words in order. Nonzero stack counts are cleaned
by the caller (4 bytes per word). Close tail-jumps; Open/Reset first call the
surface slot, return zero if it fails, otherwise tail-jump to the sprite slot.
Targets Open/Reset/Close return 1; Option/Reserved return 0 without inspecting
their arguments. Raw unsigned option words preserve unknown signedness/meaning.
Reserved has an observed one-word consumer; it is unrelated to the unconsumed
surface reserved slot. Full target and consumer hashes/extents and forwarding
contracts are asserted in `tools/verify_mem_lifecycle.py`.

Configure publishes pixels and stride with width/height and an uninterpreted
fifth word. Representative caller RVA 0x18610 passes the byte buffer at VA
0x00563DB0, stride=width=320, height=200, fifth word=8; its complete 51-byte
instruction body is asserted. SetClip uses signed comparisons for left/top/right/
bottom. DrawList dereferences its argument as encoded 32-bit list words and then
nested request pointers. Draw retains the separately reconstructed three-pointer
Gfx_DrawSpriteNative contract. HandleOp dereferences descriptor and handle and
returns a handle pointer or zero; ImageOp reads a descriptor and uses signed
image-ID bounds, returning an integer ID/status. Create/CopyDescriptor forward a
record pointer and signed ID to VAs 0x00461210/0x004612E0; FreeDescriptor forwards
a record to VA 0x004614B0, which frees fields at +0x18/+0x1C. These helper bodies,
FPO argument counts and hashes are independently asserted. GetState writes a
40-byte observed prefix; SetState reads it and forwards fields to Configure and
SetClip. Descriptor/state remain opaque; observed offsets do not certify complete
layouts. Target bodies are analyzed only except the preexisting DrawSpriteNative;
no additional target reconstruction or target differential evidence is claimed.

## Downstream initializer contracts

All three have no stack arguments, ordinary RET and balanced caller stack. They
run in the stated order after every slot has been installed.

| Semantic boundary | RVA / bytes | SHA-256 | Contract |
| --- | --- | --- | --- |
| Gfx_InitSpriteWorkspaceA | 0x5D840 / 84 | 445faf9112110b216014f95c057ab6d572a81fec3fa64a867b9d5f03739c1d43 | void, EAX not stable/consumed |
| Gfx_InitSpriteWorkspaceB | 0x5C9F0 / 84 | 922d88eb43e5d8b7ede3d71df0d81a543dc389bfad921e3faa13815d0ac5468e | void, EAX not stable/consumed |
| Gfx_InitSpriteHandles | 0x5C7F0 / 62 | 41862be4869238f9f9261e170ab377306cd2aa18afbbd6761a59bf2f32a5b132 | int, EAX=1 |

A unconditionally zeros VA 0x0051AA50, tests lazy flag VA 0x004BACE8 for zero,
and only then makes three cdecl CRT malloc calls at VA 0x00469400, each requesting
0x20D8 bytes. Each result, including null, is stored immediately at VAs
0x0051AA5C/60/64, then the flag becomes 1. B has the same control contract with
count VA 0x0051A9CC, flag VA 0x004BACE4 and pointers VAs 0x0051A9D8/DC/E0.
Any nonzero flag skips allocations and preserves pointers; count still resets.
Both flags are file-zero .data. Failure is unchecked; a failed allocation still
leaves the lazy flag set. No stable status return is inferred from residual EAX.

Handles sets VA 0x0051A998 to 0x00514B98, writes 2,000 pointers starting at
VA 0x00512C58 to successive 12-byte entries starting at VA 0x00514BD8, and clears
only each entry's first dword. It leaves each entry's other eight bytes intact.
It then calls VA 0x004612E0 with (descriptor VA 0x00514B98, image ID 0), cleans
eight stack bytes, ignores that helper's EAX and returns 1. The helper's zero-ID
path copies sixteen dwords from VA 0x0051FB88; descriptor allocation/layout and
native effects beyond this boundary are not production reconstructed.

43 original-only initializer cases verify exact ordered accesses/calls and
complete image/ABI state: 40 lazy cases cover zero/noncanonical/sign-bit flags,
independent null allocations and pointer preservation; three handle cases cover
seeded entry tails and arbitrary descriptor-helper returns. CRT malloc and the
descriptor helper are explicit sinks. These are original contract checks, not
original-versus-production downstream validation. All three boundaries remain
analyzed only and are nonreturning fixtures in a separate validation translation
unit; no success-returning production substitutes exist.

## Production execution and validation

Production geputget.c owns sixteen separately typed volatile zero-initialized
sprite globals and the installer. Exact production declarations/globals/body are
extracted alongside the existing selector and real surface installer. Nonreturning
target fixtures are separate from production C and are never entered. Even the
preexisting DrawSpriteNative is only an address fixture in this installer suite;
its existing independent GCC x87 differential suite still executes its real body.
Only the four untouched intervening words use a validation-only array. Only
verified surface/sprite target identities are normalized between original and
rebuilt addresses; store destinations/order remain significant.

Strict C89 focused import-free PE32 DLL compilation passes with provisional
Clang/LLD 19.1.1, i686-pc-windows-msvc, -O2, freestanding/no-builtin, disabled
inlining/unrolling/vectorization/SSE. No playable rebuild is produced.
306 standalone sprite comparisons cover representative and random initial words,
independent downstream return pairs, mutation at each downstream entry and 153
persistent repeated calls. Boundary snapshots prove all slots precede the first
call and later calls observe previous mutations. Repetition reinstalls pointers
before calls. Full tracked state, exact access/call order, EAX=1, caller stack,
nonvolatile registers, clear DF and unchanged unrelated image bytes are checked.

532 selectors retain 266 nonzero calls without effects and 266 zero calls with
both real installers; 18 integrated startups (four persistent) execute both.
Retained coverage: 266 standalone surface installers, 39 original-only surface
consumer ABI cases, 306 banner, 266 input reset, 112 startup (94 isolated), 519
shutdown (74 isolated), ten persistent lifecycle calls. 48 original-only sprite
consumer sink cases verify forwarding/cleanup, including surface-success tails,
with zero/high-bit/-1 results and volatile-register clobbering. Renderer bodies
do not execute in those cases. Existing full workflow validation is retained. The aggregate build now asserts
24 generated direct calls with symbolic ownership/ordered targets, including
the three sprite initializer dependencies; this is a compiler-specific guard,
not instruction equality.

Raw-prefix diagnostics differ. Relocation-aware instruction equality, original
compiler/link layout and native graphics/input/heap/game parity remain unverified.
Primitive initialization, the three downstream sprite initializers, CRT printf/
heap and handle callbacks remain modeled. Invalid/aliased storage, concurrency,
general reentry, native target rendering and complete descriptor layouts remain
unverified. EXACT denotes recovered installer behavior; no deviation or bug fix
is introduced. Inventory remains 1,028 candidates; 30 routines are reconstructed.

Run PowerShell with UV_CACHE_DIR=build/uv-cache and UV_OFFLINE=1. Use
`uv run python tools/verify_mem_lifecycle.py`, then real `tools/workflow.py complete`
for reconstructed RVAs 0x56E60/0x5B690/0x56AF0/0x5B170/0x5B1A0/0x55AB0/0x5B4F0.
Analysis-only completion covers the affected analyzed targets/consumers/initializers
and descriptor helpers. Generated exports come from SQLite. Current recorded
input hashes must match LF working bytes and Git index blobs before staged gates
and commit. Compilation, instruction matching and native runtime remain separate.

Next bounded candidate: Gfx_InitSpriteHandles at RVA 0x5C7F0 (62 bytes), after
recovering the complete default descriptor state and executing its real helper
at RVA 0x612E0; the two lazy workspaces remain separately modeled.

## Real sprite handle/helper follow-up (2026-10-08)

[Handle initialization and descriptor copying](windows_gfx_sprite_handles.md)
reconstruct RVAs 0x5C7F0/0x612E0 and execute the real helper through real handles,
sprite installation, selector and 18 lifecycle startups. Fresh coverage adds
486 helper and 48 handle comparisons, plus 16 original-only default initializer
checks; existing 306/266 sprite/surface installers, 532 selectors, 48/39 consumer
ABI and 43 downstream contracts, banner/input/memory validation remain.
Default initialization writes seven words and preserves nine; only live copies
clear +0x18/+0x1C. Two workspace initializers and primitive/CRT/callback boundaries
remain modeled. Strict C89 compilation and differential emulation pass;
instruction equality, original compiler/link layout and native graphics/game
parity remain unverified. Earlier model/opaque-descriptor descriptions are
historical. Inventory: 1,028 candidates, 32 reconstructed routines.
