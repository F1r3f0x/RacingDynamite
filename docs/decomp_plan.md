# Ignition Master Decompilation Plan (Windows Target)

Latest bounded follow-up (2026-10-07): handle cleanup callback `Font_Shutdown`
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
