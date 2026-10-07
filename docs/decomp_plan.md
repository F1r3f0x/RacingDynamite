# Ignition Master Decompilation Plan (Windows Target)

Decision date: 2026-10-07 (America/Santiago). The primary target is the standard
Windows release, `Ignition/Ignition/IGN_WIN.EXE`. The DOS release remains a
secondary reference. The earlier plan is retained in
[the DOS archive](tracking/dos_decomp_plan.md).

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

- Preserve the current DOS `decomp/` sources, LE tooling, Open Watcom build,
  database, symbol maps, and runtime observations. Pause new DOS reconstruction
  work unless needed to answer a Windows comparison question.
- Plan a separate Windows reconstruction source/build path; it does not exist yet.
  Choose and document the path when introducing the Windows build pipeline.
- Identify the Windows compiler and ABI from executable evidence before selecting
  a vintage compiler. MSVC 4.2 is a research candidate, not a verified local fact;
  DOS Watcom flags and register calling conventions must not be assumed valid.
- Existing `database/decomp.db` has DOS-primary records and legacy `win_address`
  cross-references. Existing CLI/status/dashboard/audit output remains legacy
  progress. Before recording Windows results, implement target-aware storage,
  address keys, evidence, and progress reporting without resetting DOS history.
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
2. **Create Windows build and tracking infrastructure.** Select a compiler using
   evidence, isolate output paths, add target-aware provenance and verification,
   and link a minimal Windows executable without claiming gameplay parity.
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

The existing `build_decomp.py`, `verify_matching.py`, LE helpers, DOS runtime
staging, and fidelity auditor have not been migrated by this docs-only change.
Use them for their retained scope, not as Windows acceptance gates.

## 6. Preservation and provenance

Never overwrite original files under `Ignition/`. Stage disposable runtime copies
and isolate generated outputs. Keep the DOS inventory, timer recovery, and DOSBox
baseline as evidence about `MAINDOS.EXE`, including their unresolved failures.
Use `IGN_WIN.EXE` in new verified Windows `@original` annotations; keep DOS
annotations and deviations attached to their original binary and addresses.

Related guidance: [agent instructions](../AGENTS.md), [roadmap](roadmap.md),
[fidelity strategy](tracking/fidelity_strategy.md), and [tools](../tools/README.md).
