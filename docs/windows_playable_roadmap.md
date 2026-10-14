# Authentic Windows playable reconstruction roadmap

Updated 2026-10-14. This is an acceptance and dependency ledger; quantitative
routine progress remains generated from `database/decomp.db`. Historical DOS
completion and isolated DLL tests do not satisfy any whole-game acceptance item.

## Acceptance status

A playable rebuilt standard Windows executable has **not** been validated.
All whole-game items below remain open. Existing native subsystem observations
are bounded evidence and must retain their documented loader, fixture and API
boundaries when reused.

| Required outcome | Existing starting evidence | Evidence still required |
| --- | --- | --- |
| Startup | [Authentic startup map](ghidra/windows_startup.md), reconstructed memory/platform dependencies | Rebuilt executable launches with real initialization and assets; normal and failure exits; original/rebuilt host and compatibility settings documented |
| Menus | Font loading/drawing and sprite pipeline reconstruction | Original widgets/layout/navigation, language flow, loading and menu transitions in the rebuilt process |
| Rendering | [Whole triangle native checks](ghidra/windows_triangle_parents.md), surface/palette/presentation helpers | Real sprite/triangle/workspace integration and representative original/rebuilt menu and race frames; no intercepted renderer dependencies |
| Input | [Input module evidence](ghidra/windows_input_module.md) and polling/lifecycle contracts | Native keyboard/control behavior in menus and races, timing/repeat, focus loss/recovery and teardown |
| Audio | Imported DirectSound/WinMM API and startup-map clues | Authenticated mixer/resource/device lifecycle and native game music/effects observations; no silence or success stubs |
| Races | Unmigrated source is only hypothesis material | Windows-authenticated simulation, vehicles, AI, camera, collision/rules, HUD, finish/results/menu return; representative playable original/rebuilt runs |

## Next separately validated features

1. Finish sprite packet RVA `0x65BB5`: authenticate pipelined reads/stores,
   signed clipping and texture address construction; renew shared contracts,
   fidelity/export checks and commit. The focused contract retains an explicit
   transformed-callee boundary. See [packet evidence](ghidra/windows_sprite_packet.md).
2. Close the real transformed packet-to-triangle chain at RVAs `0x68C70` and
   `0x65BB5`. Review live access order and wrapped arithmetic in the existing
   transform implementation. Use the complete original routines, not just
   triangle-call interception; compare persistent state, frame bytes and faults.
3. Recover the cdecl sprite submission adapter `0x57370` and descriptor lookup
   `0x612E0` with their verified packet/table state. This authentic adapter is a
   prospective native entry to the original ESI rasterizer without handwritten
   assembly. Recover and test it before treating that boundary as solved.
4. Integrate production workspace allocation/storage and sprite resource lifetime.
   Remove affected extracted legacy harness boundaries as direct modules replace
   them; preserve subsystem ownership and actual dependency contracts.
5. Return to the verified startup graph and close window/event-loop, presentation,
   asset and font/menu dependencies in native execution. Build a real executable
   only when its required behaviors have implementations; no fabricated stubs.
6. Reconstruct audio and game/race dependencies from the Windows call graph.
   Record each subsystem's state and source ownership before implementation.
7. Execute and document the acceptance matrix above, including repeatable inputs,
   host environment, binary/build identity, deviations, captures/traces and actual
   observed outcomes. Broaden representative track/car scenarios and exit/reentry.

Reorder unblocked dependencies when evidence warrants it. A blocked candidate
must not prevent independent useful work, and a stage label cannot replace a test.

## Fidelity and build limits

- Primary source: authentic `Ignition/Ignition/IGN_WIN.EXE`, SHA-256
  `7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
  Preferred base `0x00400000`; keep VA and RVA distinct.
- Linker 4.20, PDB/FPO and Microsoft-compatible ABI evidence do not establish
  the original compiler version or flags. Modern GCC/Clang/LLD configurations
  are provisional behavioral-validation compilers. Record actual commands for
  each artifact; no byte/instruction equality is implied by compilation.
- DEV-007 adapts internal register interfaces to typed C calls. Preserve the
  original subsystem design and side effects, with explicit ABI boundaries.
- C89 source targets 32-bit x86. Document arithmetic shifts, wrapped words,
  layout, pointer width and unaligned accesses where relevant. No handwritten
  assembly, substituted native UI, or parallel DOS reconstruction.
- Original loader/section execution adaptations in native probes are probe
  boundaries, not evidence that the rebuilt game has equivalent startup.
- Original assets remain untouched. Runtime copies and generated probes belong
  under `build/runtime/`; generated artifacts and raw decompiler output stay
  outside commits.

## Integration and handoff discipline

Astra owns this checkout's shared harness, SQLite, exports and feature commits
following explicit user authorization and Sol's release of mutation ownership.
Reconfirm ownership if another writer appears. Preserve all unrelated changes.

For each feature: preflight explicit files/module/RVAs; describe and set status
by authenticated RVA; run the real contract and `workflow.py complete`; review
fresh snapshot and affected records; stage explicit files; check LF working/index
bytes and `workflow.py check --staged`; commit with Conventional Commit and RVA
trailers. Record compilation, emulation, instruction matching and native scope
separately. Continue with the next useful feature after each commit.
