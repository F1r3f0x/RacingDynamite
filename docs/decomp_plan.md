# Ignition Master Decompilation Plan (Windows Target)

Latest bounded feature (2026-10-08): [sprite dispatch installation](ghidra/windows_gfx_sprite_dispatch.md)
at RVA 0x56E60 is reconstructed with sixteen typed production globals.
306 standalone sprite installers, 532 selectors and 18 startups execute both
real installers; 48 sprite consumer ABI and 43 downstream original-only cases
verify contracts. Surface/lifecycle coverage is retained. Three downstream sprite
initializers remain modeled. 1,028 candidates, 30 reconstructed routines; current
compilation/differential evidence, instruction equality/native parity unverified.
Next bounded candidate: Gfx_InitSpriteHandles, RVA 0x5C7F0, and its default
descriptor helper RVA 0x612E0. Earlier entries are historical.

Latest bounded feature (2026-10-08): [surface dispatch installation](ghidra/windows_gfx_surface_dispatch.md)
at RVA 0x5B690 is reconstructed in native geputget.c with fourteen independently
typed production globals. 266 standalone installers, 532 selectors and 18
integrated startups execute its real production body; 39 original-only consumer
ABI sinks check forwarding and caller cleanup. Fourteen targets and thirteen
consumers remain analyzed only, including newly inventoried RVA 0x5C1D0 (497
bytes): 1,028 candidates, 29 reconstructed routines. Primitive and sprite
initialization, CRT printf/heap and callbacks remain modeled. Strict C89 focused
compilation passes; raw prefixes differ. Instruction equality, original linked
layout and native graphics/game parity remain unverified. Next bounded candidate:
Gfx_InstallSpriteDispatch, RVA 0x56E60 (181 bytes). Earlier entries are historical.

Latest bounded feature (2026-10-08): [graphics backend selector](ghidra/windows_gfx_backend.md)
at RVA 0x56AF0 is reconstructed in native geputget.c, with zero calling two
initializers in order and returning 1, every nonzero value returning 2 without
effects. 572 standalone comparisons and 18 integrated startups execute its
production body. Coverage retains 306 banner / 266 reset / 112 startup / 519
shutdown comparisons and ten persistent lifecycle calls. Dispatch initializers
at RVAs 0x5B690/0x56E60 are independently analyzed only; primitive initialization,
both dispatch initializers, CRT printf/heap and callbacks remain modeled. Strict
C89 focused DLL compilation passes; raw prefixes differ. Instruction equality,
original compiler/link layout and native game parity remain unverified. Next
bounded candidate: Gfx_InstallSurfaceDispatch, RVA 0x5B690 (146 bytes).

Latest bounded feature (2026-10-08): [Lisa version banner](ghidra/windows_lisa_version.md)
at RVA 0x5B4F0 is reconstructed in native lisa3d.c with exact Windows format and
version bytes, two cdecl printf calls and unconditional zero return. 306 standalone
differential comparisons and 18 integrated startups execute the production body.
Coverage retains 266 reset / 112 startup / 519 shutdown comparisons and ten
persistent lifecycle calls. Two graphics dependencies, CRT printf/heap and handle
callbacks remain modeled. Strict C89 focused DLL compilation passes; raw prefixes
differ. Instruction equality, original compiler/link layout, native console/game
parity and playable rebuilding remain unverified. Next bounded candidate:
Gfx_SelectBackend at RVA 0x56AF0 (30 bytes).

Latest bounded feature (2026-10-08): [input callback reset](ghidra/windows_resource_reset.md)
at RVA 0x55AB0 is reconstructed in native geputget.c as Input_ResetCallbacks.
Its former Gfx_ResetResourceFlags name was provisional: consumers establish two
keyboard callback pointers, with two-argument caller-cleanup and zero-argument
interfaces. 266 standalone differential invocations pass, and the real reset now
executes through 18 integrated startups in the retained 112 startup / 519 shutdown
lifecycle suite, including ten persistent calls. Three startup dependencies, CRT
heap and handle callbacks remain modeled. Raw prefixes differ; original compiler,
instruction equality, native input/timer/graphics/heap/game parity and playable
rebuilding remain unverified. See evidence for static consumer provenance.

Latest bounded feature (2026-10-08): [memory-system lifecycle wrappers](ghidra/windows_mem_lifecycle.md)
at RVAs 0x5B170/0x5B1A0 are reconstructed in production C89 mem.c/mem.h.
112 startup and 519 shutdown differential invocations pass, including 94/74
isolated dependency contracts, real memory bodies, all 200 callback slots in both
passes, cross-phase mutations and ten persistent allocation/registration/lifecycle
calls. Startup has six dependencies (including four analysis-only boundaries),
ignores all results and always returns 1; teardown invokes handle callbacks before
pool destruction. CRT heap and callbacks remain modeled; instruction equality,
original compiler/link layout and native startup/graphics/heap/game parity are
unverified. No playable build is produced. See linked evidence for boundaries
at RVAs 0x5B4F0/0x55AB0/0x5E610/0x56AF0 and remaining limitations.

Latest bounded feature (2026-10-08): [Windows pool destruction and shutdown](ghidra/windows_mem_destroy.md)
at RVAs 0x5B080/0x5B140 are reconstructed in production C89 mem.c/mem.h.
504 destruction and 265 shutdown differential invocations pass, including
37 persistent invocations through all six real pool lifecycle bodies, allocation
failures, stale zero-size records, cached/live free-boundary mutations and ABI.
CRT heap remains modeled; raw prefixes differ. Instruction equality, original
linked layout and native heap/game parity are unverified. No playable build
is produced; Font_Load retains its pool fixture.

Latest bounded feature (2026-10-08): [Windows pool initialization and creation](ghidra/windows_mem_pools.md)
at RVAs 0x5AD50/0x5AD80 are reconstructed in production C89 mem.c/mem.h.
75 initializer and 745 creator differential invocations pass with complete
root/heap/image state, exact ordered accesses, name truncation/tail preservation,
all first-hole slots and malloc failures, ignored default failure and ABI.
Twelve persistent invocations include six real Mem_Alloc/Mem_Free calls through
an actually created pool. CRT malloc/free remain modeled; raw prefixes differ,
instruction equality, original linked layout and native heap/game parity are
unverified. Earlier pool-analysis notes below are historical. The existing
Font_Load verifier retains its pool fixture; no original assets were changed.


Latest bounded validation follow-up: [Font_Load](ghidra/windows_font_load.md#real-file-loader-integration-follow-up)
at VA 0x00456420 / RVA 0x56420 links complete production file.c and mem.c.
536 real-chain differential invocations execute File_LoadToMemory, its helpers,
Mem_Alloc, Font_Parse, lazy initialization/handle bookkeeping and Mem_Free;
333 isolated wrapper comparisons are retained (869 total). Full pool hierarchy,
CRT-entry state, transferred payload, leakage/error precedence, ABI and three
persistent allocation/free/reuse sequences are checked against independent
oracles. CRT I/O/heap and sprites remain modeled; pool creation is a fixture.
No production game body changed or new routine reconstructed. Instruction
matching and native filesystem/heap/game parity remain unverified. Full
geputget.c remains blocked by legacy dependencies. Pool initialization/creation
at RVAs 0x5AD50/0x5AD80 remain suitable bounded reconstruction candidates.

Latest bounded follow-up (2026-10-07): [File_LoadToMemory](ghidra/windows_file_load.md)
at VA 0x004574A0 / RVA 0x574A0 (230 bytes) is reconstructed in production C89 file.c/file.h.
240 differential executions pass against authentic PE instructions with real File_CheckReadable,
File_GetSize, File_GetStreamSize and Mem_Alloc execution, verifying error precedence
(2030, 2040, 2050, 2000, 2010), unclosed stream on short read, retained allocation on open
failure, position words [0, 0], and full ABI/register preservation. CRT I/O and malloc
are modeled; raw bytes differ and native filesystem/game parity is unverified.
Font_Load now executes the real file-loader chain as recorded above.

Latest bounded follow-up (2026-10-07): the [native file helpers](ghidra/windows_file_load.md)
at RVAs 0x575F0/0x57630/0x576B0 are reconstructed in production C89 file.c/file.h.
714 differential executions pass with real GetStreamSize through GetSize, exact
ordered CRT calls, cached signed size/position words, ignored seek/close failures,
null-stream forwarding, complete state snapshots and ABI. CRT I/O is modeled;
raw bytes differ and native filesystem/game parity is unverified. File_LoadToMemory
at RVA 0x574A0 is reconstructed with these real helpers and real Mem_Alloc.

Latest bounded follow-up (2026-10-07): [Mem_Alloc](ghidra/windows_mem_alloc.md)
at VA 0x0045AE10 / RVA 0x5AE10 (406 bytes) is reconstructed in production C89.
Its focused suite covers 800 differential invocations plus three real Mem_Free
round trips, exact lookahead reads beyond exhausted arrays, first-size-zero
selection, all allocation failures and retained partial hierarchy, zero-size
reuse and cached stores across modeled CRT mutations. CRT heap remains modeled;
instruction equality and native heap/game parity are unverified. Pool initialization
and creation at RVAs 0x5AD50/0x5AD80 were independently rechecked and are analyzed
only. [File_LoadToMemory and size/readability helpers](ghidra/windows_file_load.md)
at RVAs 0x574A0/0x575F0/0x57630/0x576B0 have bounded static analysis only, with
no production C or execution results. Next reconstruct those native file wrappers
and execute real Mem_Alloc through the loader before replacing Font_Load's file
boundary. Font_Load still models file loading; full geputget.c remains blocked.

Latest bounded follow-up (2026-10-07): [Mem_Free](ghidra/windows_mem_free.md)
at VA `0x0045B000` / RVA `0x5B000` (127 bytes) is reconstructed in mem.c,
with independently recovered pool/page/record types and 519 differential
executions checking exact read/write order, first-pointer matches regardless
of size, repeated/null/stale pointers and post-CRT size clearing. The real
routine now executes through Font_Load in 333 differential cases, including
six unregistered-buffer cases. Only CRT freeing, file loading and sprite
creation remain modeled at those boundaries. Raw bytes differ; instruction
equality, native heap/game parity and invalid hierarchy behavior are unverified.
Full geputget.c remains blocked by legacy dependencies.

Latest bounded follow-up (2026-10-07): [Font_Load](ghidra/windows_font_load.md)
at VA `0x00456420` / RVA `0x56420` (71 bytes) now has fresh extracted-production-C
compilation and 327 differential comparisons: 35 null-load, 260 real-parser
and 32 modeled-parser-return cases. The wrapper forwards both parser arguments,
frees a nonnull buffer even on parse failure, and preserves the parser result
across freeing. Real lazy font initialization and handle bookkeeping execute;
file I/O, freeing and sprite creation are modeled boundaries. The existing
C89 wrapper required no source change. Live Ghidra inspection corroborates
local authenticated instructions and 17 callers under the user-provided active
program assumption. Instruction equality, native resource management and native
game parity remain unverified. Full geputget.c remains blocked by legacy dependencies.

Latest bounded follow-up (2026-10-07): [Font_Parse](ghidra/windows_font_parse.md)
at VA `0x00456270` / RVA `0x56270` (429 bytes) now has extracted-production-C
differential coverage. All 260 cases pass with complete font/handle state,
ordered calls, descriptor fields 4..31, callback-boundary snapshots and ABI
equality. Real lazy initialization, handle allocation and registration execute;
sprite creation is modeled. The existing C89 parser required no source edit.
Only offset -1 means absent; other negative offsets form raw pointer dwords.
Descriptor word zero is uninitialized and excluded. Instruction equality,
native allocation and native game parity remain unverified.

Latest bounded follow-up (2026-10-07): [Font_InitSystem](ghidra/windows_font_init.md)
at VA `0x00456180` / RVA `0x56180` (134 bytes) is independently recovered and
verified against its existing C89 reconstruction. The expanded focused font DLL
executes real memory allocation, registration and release bodies: 510 initializer
differential comparisons and five persistent lifecycle round trips (20 routine
invocations) pass, retaining 123 unload and 71 shutdown comparisons. Only sprite
release is modeled. The initializer resets six header dwords per slot and ignores
allocation/registration failure. No instruction equality or native game parity is
claimed; full geputget.c remains blocked by legacy dependencies. Live Ghidra
inspection was attempted after user confirmation but sandbox access was denied.

Previous bounded follow-up (2026-10-07): handle cleanup callback `Font_Shutdown`
at RVA `0x56210` (83 bytes) and downstream font unloader `Font_Unload` at RVA `0x56470`
(88 bytes) are reconstructed and verified. Focused validation DLL in
`tools/verify_font_cleanup.py` executes 123 `Font_Unload` and 71 `Font_Shutdown`
differential emulation test cases against authentic `IGN_WIN.EXE` instructions
in Unicorn with exact call-argument, state-mutation, and ABI equality.
Full geputget.c compilation remains blocked by unmigrated DOS legacy dependencies.

Previous bounded follow-up (2026-10-07): [triangle clipping analysis](ghidra/windows_triangle.md)

recovers the complete clipping and splitting contract at RVA `0x24F270` (537 bytes),
both two-segment wrappers at RVAs `0x24EFA0` and `0x24F0C0` (282 bytes each), the
right-endpoint clamp helper RVA `0x24F489` (63 bytes), and left-clipped span behavior
at RVAs `0x5C9D0`/`0x67F97`. An independent unified clipped rendering oracle verifies
62 direct splitter cases, 16 direct wrapper cases, 312 unclipped triangle cases,
60 clipped triangle cases, and all 24 transformed sprite cases through both triangle
calls and outer return with exact framebuffer byte and memory access equality.
No production C, instruction equality, original-versus-C differential or native
rendering parity is claimed. Complete C reconstruction remains blocked on resolving
the register calling conventions (ESI edge packet, EBX output buffer, and outer ESI
sprite packet) without forbidden assembly.

Previous bounded follow-up (2026-10-07): [native rasterizer analysis](ghidra/windows_sprite_rasterizer.md)
corrects RVA `0x65BB5` from a 437-byte prefix to its complete 474-byte extent,
including the narrow-span tail. 704 original-only cases execute real untransformed
framebuffer writes through return; 72 execute the transformed path and RVA
`0x68C70` to its first triangle call. Both entries remain analyzed, with no
production-C or differential-validation result. Complete C integration is blocked
on the one-packet cdecl triangle routine RVA `0x5CA50` (1,813 bytes), whose effects
and live-scratch mutations must be recovered next. No native rendering parity is
claimed. Missing non-FPO entry candidates are imported without guessed extents.

Previous bounded follow-up (2026-10-07): the native sprite backend at RVA `0x571B0`
is reconstructed, following both font routines. The [sprite preparation adapter
analysis](ghidra/windows_sprite_adapter.md) independently corroborates RVA
`0x57370` and its descriptor lookup dependency `0x612E0`; 168 original-only
executions stop before the rasterizer. Reconstruction is blocked on the ESI
packet interface at VA `0x00465BB5`; compiled conventional C ABI diagnostics
do not establish that register contract. Both candidates remain analyzed without
adapter differential validation. Next recover that rasterizer's native effects
and transitive contracts before choosing a C integration boundary. Older
milestone next-step notes below are historical.

Milestone update, 2026-10-07: the [verified startup map](ghidra/windows_startup.md),
C89 `Mem_InitHandles` (`0x0045B1F0`) and its
[handle-ID consumer](ghidra/windows_handles.md) (`0x0045B1B0`) are complete within
a focused validation scope. The build/verification entry points cover both:
30 initializer and 377 consumer original-instruction versus compiled-C emulator
comparisons passed, with eight original-only negative-cursor checks. Modern
Clang is provisional, instruction matching is not claimed, and visual/native
startup remains unverified. The active Windows SQLite store now generates
[the inventory](tracking/windows_inventory.json), registry and treemap from one snapshot.

Decision date: 2026-10-07 (America/Santiago). The primary target is the standard
Windows release, `Ignition/Ignition/IGN_WIN.EXE`. The DOS release remains a
historical binary reference. The Windows reconstruction replaces the DOS
implementation in `decomp/`; a parallel DOS reconstruction is out of scope.
The [earlier DOS plan](tracking/dos_decomp_plan.md) is historical documentation,
not an implementation-preservation requirement. This supersedes the earlier
same-day decision to maintain separate DOS and Windows source trees.

## 1. Decompile first, port second

Phase A reconstructs readable, functionally faithful C89 from the authentic
Windows executable. Preserve native architecture, software rendering, UI,
simulation, platform interactions, and original quirks. Phase B modernizes verified
code into the C11/SDL2 port. Byte matching is a useful diagnostic and a secondary
goal; compiling successfully does not prove behavioral fidelity.

Start with standard Windows software rendering. 3dfx and other renderer variants
are deferred and require their own binary fingerprints and evidence. This decision
does not rename the current branch or change implementation/build tooling.

## 2. Verified local target identity

The machine-readable expected identity is [decomp/target.json](../decomp/target.json).
Verify it with `uv run python tools/decomp_doctor.py`; the diagnostic does not
certify Windows pipeline readiness. See [the tooling audit](tooling_audit.md).

| Property | Observed value |
| --- | --- |
| Path | `Ignition/Ignition/IGN_WIN.EXE` |
| Size | 915,968 bytes |
| SHA-256 | `7665E4E736BFD6C90790CEDBB27E2DE7E98A167374EB77933533C54EF0DC8782` |
| Format | PE32, x86 (`Machine 0x014c`, optional header `0x010b`) |
| Preferred image base | `0x00400000` |
| Entry RVA | `0x00069950` |
| Entry VA at preferred base | `0x00469950` |

These values were read directly from the local file and PE headers. The entry
point is not automatically `WinMain`. Compiler version, compiler flags, imports,
function inventory, and runtime behavior have not been established by this
documentation change. Recheck the fingerprint before using another regional or
patched executable; do not borrow addresses from `FUN_WIN.EXE` or another release.

Load the authentic PE into a separate Ghidra program and verify its image base,
section map, imports, relocations, and entry point. Record both the binary identity
and address convention (VA/RVA) with every symbol and initializer.

## 3. Migration boundaries and tracking

- Use `decomp/` for the Windows reconstruction and replace the existing DOS
  implementation in place. Its code, stubs, architecture assumptions, and status
  labels are untrusted; carry over a routine only after independent Windows
  instruction, data, ABI, and behavior validation.
- Retarget existing build and verification entry points, database, symbol maps,
  and dashboards in place. Remove obsolete DOS scaffolding during migration;
  do not maintain a parallel DOS source tree or DOS build compatibility. Git
  history preserves the superseded implementation for recovery.
- Historical DOS binary observations and format research may remain as labeled
  references. They do not constrain the Windows implementation or block removal
  of flawed DOS reconstruction code. Original game files remain protected.
- Identify the Windows compiler and ABI from executable evidence before selecting
  a vintage compiler. MSVC 4.2 is a research candidate, not a verified local fact;
  DOS Watcom flags and register calling conventions must not be assumed valid.
- Active SQLite tracking is now Windows schema v2, seeded with authentic FPO
  extents and the independently verified handle initializer. No DOS statuses are
  imported. JSON, function Markdown, SQL dump and dashboards share a generated
  snapshot. See [database guidance](../database/README.md) for commands and evidence
  semantics. FPO coverage is incomplete and most candidates remain unclassified.
- Do not assume legacy `0x004...` addresses or port annotations belong to this
  executable. Validate them before using them as Windows provenance.
- Never treat DOS completion counts, matching scores, or runtime checks as Windows
  completion. No Windows reconstruction progress is certified by this migration.

## 4. Active milestone sequence

1. **Establish the Windows baseline.** Verify the local binary fingerprint;
   assume the user has authentic `IGN_WIN.EXE` loaded and active in Ghidra per
   `AGENTS.md`. A missing identity endpoint does not block inspection. If MCP
   is not responding or reports no active program, prompt the user to open
   `IGN_WIN.EXE` with the MCP plugin enabled, then retry. Inventory
   sections, imports, strings, functions, CRT code, initializers, and ABI evidence.
   Run a disposable original copy in a documented Windows-compatible environment
   and record startup, language selector, loading, intro, and first menu. Record
   any wrapper/compatibility settings and distinguish their behavior from the game.
2. **Replace the DOS workspace and infrastructure in place.** Select a compiler
   using Windows evidence; rebuild `decomp/` and retarget the existing build,
   verification, tracking, and dashboard entry points. Reset active DOS progress
   and remove obsolete scaffolding. Isolate generated outputs from originals and
   link a minimal Windows executable without claiming gameplay parity.
3. **Validate a small reconstruction.** Recover a bounded asset loader/decoder or
   memory routine with verified inputs, outputs, data layout, and calling convention.
   Compare against the original before scaling up.
4. **Recover native startup and presentation.** Trace entry/CRT to application
   initialization; recover window/message handling, graphics/palette presentation,
   timing, input, audio initialization, language UI, menu layouts, and CDP playback.
   Resolve the first observed Windows divergence before proceeding.
5. **Reconstruct engine dependencies.** Recover memory/file I/O and asset formats,
   surface queries, Lisa3D rendering, game state, vehicle physics, AI, race rules,
   camera, HUD, and audio using the Windows call graph. DOS naming/module hints
   are candidates that need independent Windows validation.
6. **Validate the full game, then modernize.** Compare representative races,
   controls, timing, visuals, audio, results, and menu return; extend coverage to
   tracks/cars and supported multiplayer modes. Port verified code to C11/SDL2
   with documented deviations and authentic defaults.

## 5. Evidence and acceptance criteria

Every reconstructed function needs a verified Windows address, size, signature,
ABI, source/module association where supported, and explicit evidence status.
Recover initialized data, relocations, BSS extents, and structure packing from PE
evidence. Never infer equivalence from shared asset filenames alone.

Track separately: identified/named, analyzed, C reconstructed, instruction match,
linked binary match, and runtime validation. Report compiler/relocation differences
when interpreting matching scores. A future treemap can use verified byte sizes
for area and these evidence stages for color; existing dashboards are not yet a
Windows inventory.

The focused Windows build and differential harness cover the handle initializer
and handle-ID consumer;
the active provenance auditor and tracking exports cover the Windows inventory.
LE helpers, the legacy instruction comparator and DOS runtime staging remain
historical tools, not active Windows acceptance gates. DOS compatibility is not
a requirement. Broader routine/runtime contracts still need independent recovery.

## 6. Preservation and provenance

Never overwrite original files under `Ignition/`. Stage disposable runtime copies
and isolate generated outputs. Keep the DOS inventory, timer recovery, and DOSBox
baseline as evidence about `MAINDOS.EXE`, including their unresolved failures.
Use `IGN_WIN.EXE` in verified Windows `@original` annotations when replacing
functions. Remove obsolete DOS implementation annotations with the code they
describe; any historical evidence kept remains attached to its original binary.
Do not relabel DOS addresses or inherited completion claims as Windows evidence.

The DOS implementation preservation policy is superseded. The Windows tracking
store and bounded handle-init tooling are migrated; other source modules remain
unmigrated and may be replaced in place. Their presence is not a fidelity claim.

Related guidance: [agent instructions](../AGENTS.md), [roadmap](roadmap.md),
[fidelity strategy](tracking/fidelity_strategy.md), and [tools](../tools/README.md).

## Current tracking migration (2026-10-07)

Completed: active Windows database, 1,012 authentic FPO candidates, preserved
76-byte `Mem_InitHandles` reconstruction, fresh persisted 30-case differential
emulation evidence, generic provenance audit, synchronized exports and interactive
treemap. Results carry target/routine/source/artifact hashes, compiler context and
commands; stale evidence does not count. Compilation, reconstruction, byte equality,
emulation and native validation remain separate. Native playable milestones are
unverified. The adjacent consumer is now reconstructed with its signed boundary
and negative-address contract preserved; next recover its caller's downstream
bookkeeping routine at `0x0045B360` and continue
classifying/corroborating Windows candidate extents, including non-FPO routines.

## Handle-bookkeeping milestone (2026-10-07)

The [independently recovered bookkeeping routine](ghidra/windows_handle_bookkeeping.md)
at `0x0045B360` is reconstructed as `Mem_RegisterHandle` in the existing C89
memory module. The focused DLL/emulator now covers three routines (254 original
bytes), with 30 initializer, 377 consumer and 590 bookkeeping differential
comparisons, plus eight original-only negative-cursor checks. New coverage checks
all 200 registration slots, first-zero selection, ordered effects, disabled/full
states and three-routine integration. SQLite and its synchronized exports retain
separate compilation, raw-code and emulation evidence; no instruction equality,
native layout or gameplay claim is added. The next verified dependency is
`0x0045B240`, which tests status/flags and invokes stored callbacks; recover its
full lifecycle and callback ABI before reconstruction.

## Handle-shutdown milestone (2026-10-07)

`Mem_ShutdownHandles` at `0x0045B240` is reconstructed in the existing memory
module. [Independent shutdown evidence](ghidra/windows_handle_shutdown.md) records
its initialized-flag clear, two ordered 200-slot scans, exact status/flag equality,
status clear before callback, and one-dword caller-cleanup callback ABI. The
verifier retains 30/377/590 differential comparisons and eight original-only
negative-cursor checks, adding 531 shutdown comparisons with explicit callback
models, mutation/reentry checks and four-routine integration. Four routines cover
409 original bytes. Callback body semantics, native runtime, original compiler
identity and instruction equality remain unverified. Next recover ID-based release
at `0x0045B410`, directly called by the registered callback at `0x00456210`.

## Handle-ID release milestone (2026-10-07)

`Mem_ReleaseHandleId` at `0x0045B410` is reconstructed in the memory module.
[Independent release evidence](ghidra/windows_handle_release.md) records the
single ascending 200-slot scan, clearing every status==1 entry with a matching
raw dword ID, and returning 1 whenever enabled even without a match. It invokes
no callback and ignores class flags. The verifier adds 418 comparisons while
retaining 30/377/590/531 differential comparisons and eight original-only negative
cursor checks. Five routines cover 471 original bytes. Native layout/runtime,
instruction equality, callback bodies and compiler identity remain unverified.
The registered cleanup callback's next verified dependency is `0x00456470`, called
at `0x00456244`; recover that contract before implementing the callback itself.

## Handle cleanup & font unload milestone (2026-10-07)

The registered handle cleanup callback `Font_Shutdown` at `0x00456210` (83 bytes)
and its downstream font slot unloader `Font_Unload` at `0x00456470` (88 bytes)
are reconstructed and verified in `decomp/src/geputget.c`.
[Independent font unload evidence](ghidra/windows_font_unload.md) and
[font shutdown evidence](ghidra/windows_font_shutdown.md) record the 224-glyph
presence scan, `Gfx_SpriteOp(NULL, handle)` invocation, `FontSlot` Windows packing,
handle release via `Mem_ReleaseHandleId(g_fontSubsystemHandle)`, ascending 30-slot
active scan, and uninitialized error return `1020`.
`tools/verify_font_cleanup.py` compiles a focused validation DLL and passes
123 `Font_Unload` and 71 `Font_Shutdown` differential emulation executions against
authentic PE instructions, covering uninitialized, empty, full, and sparse active
configurations with exact call-argument, state-mutation, and ABI equality.
Integrated into `tools/verify_matching.py`. Native layout, instruction equality,
and unmigrated DOS subsystems remain unverified.
