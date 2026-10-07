# Ignition Master Decompilation Plan (Windows Target)

Milestone update, 2026-10-07: the [verified startup map](ghidra/windows_startup.md)
and C89 `Mem_InitHandles` (`0x0045B1F0`) are complete within a focused validation
scope. The three build/verification entry points now target that Windows routine;
30 original-instruction versus compiled-C emulator comparisons passed. Modern
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

1. **Establish the Windows baseline.** Verify the fingerprint in Ghidra; inventory
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

The focused Windows build and differential harness cover the handle initializer;
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
unverified. Next: reconstruct/validate the adjacent handle consumer and continue
classifying/corroborating Windows candidate extents, including non-FPO routines.
