# Ignition (1997): Agent Instructions

## Goal and scope

As of 2026-10-07, reconstruct the authentic standard Windows release at
`Ignition/Ignition/IGN_WIN.EXE` with functional fidelity and the original game
architecture. Byte-for-byte matching is secondary to correct behavior.
See `docs/decomp_plan.md` for the active plan and target fingerprint.

- Windows reconstruction: readable C89; establish the original compiler, ABI,
  data layout, and build configuration from this binary before selecting a toolchain.
- `decomp/` is the Windows reconstruction workspace. Replace the superseded
  DOS/Open Watcom implementation in place; do not create a parallel Windows tree
  or maintain a DOS build. Existing DOS code is untrusted implementation material,
  not a foundation that must be preserved. Recover behavior from `IGN_WIN.EXE`.
  Use 4-space indentation and no handwritten assembly or inline `__asm`.
- Retarget build, verification, database, symbol maps, and dashboards in place.
  Remove obsolete DOS scaffolding as part of the affected migration feature.
  Git history provides recovery of the old implementation; no duplicate DOS source
  archive is required. This documentation change itself does not replace code.
- `src/`: C11/SDL2 source port, built with CMake; modernization follows verified reconstruction.
- Tools: Python through `uv`. Use Windows PowerShell; do not run Linux package managers.

## Non-negotiable constraints

- `IGN_WIN.EXE` is the primary source of truth. `MAINDOS.EXE` is a secondary
  reference, with its own addresses, ABI, and runtime evidence.
- Never use `MAINDOS_32BIT.EXE`, the flawed DOS-to-PE repackaging, as evidence.
  The authentic Windows PE executable is a distinct, valid analysis target.
- Never modify or overwrite original binaries or assets in `Ignition/`.
  Use disposable copies under `build/runtime/` for runtime experiments.
- Reconstruct native systems in the appropriate module. Do not replace original
  UI widgets, menus, or rendering pipelines with invented substitutes.
- Never commit raw decompiler output, binary assets, or generated build outputs.
- Preserve unrelated working-tree changes. Stage only explicit feature files or hunks.

## Reverse engineering and implementation

1. Inspect authentic `IGN_WIN.EXE` PE sections, imports, relocations, initializers,
   and calling conventions before implementing behavior. Record VA versus RVA
   explicitly; do not apply DOS LE parsing or Watcom register assumptions to Windows.
2. Use the Ghidra MCP server at `localhost:8080` when available. Select and verify
   the Windows program by fingerprint before inspecting or synchronizing symbols.
   Report unavailable tooling; do not invent evidence or addresses.
3. Replace raw offsets and register artifacts with verified named structs, fields,
   and semantic names. Recover Windows structure packing independently.
4. Keep code, documentation, and tracking synchronized. `database/decomp.db`
   is the active Windows store; `docs/ghidra/functions.md`, the JSON inventory,
   SQL dump and dashboards are generated via `uv run python tools/db.py update`.
   Do not hand-edit generated progress. Run `db.py update --check` to detect drift.
   Remaining legacy globals/structs/source modules require independent Windows
   validation. Record reconstruction and test evidence separately; never transfer
   DOS completion or infer runtime parity from status labels.
5. Reuse DOS findings as hypotheses only after validating the corresponding Windows
   instructions, initializers, callers, and behavior.

## Fidelity tracking

New Windows functions require verified binary provenance:

```c
// @original <SymbolName> (IGN_WIN.EXE @ 0x<VerifiedVA>, <SourceFileHint>)
// @fidelity EXACT | ADAPTED | EXTENDED | INFRASTRUCTURE
// @deviation DEV-XXX
// @fix_category FIX_CAT_XXX
```

Choose one fidelity value. Add deviation/fix-category lines only when applicable.
When replacing a DOS function, replace its annotation with verified Windows
provenance. Do not relabel a DOS address as a Windows address. Any retained
historical DOS evidence remains explicitly tied to `MAINDOS.EXE`.
Record behavioral changes in `docs/tracking/deviations.md` and preserve original
bugs behind toggles defaulting to authentic behavior. An annotation alone is not
proof of instruction equality or runtime fidelity.

## Build and validation

- The Windows build/emulator harness currently covers `Mem_InitHandles` only;
  it produces a focused validation DLL, not a playable rebuilt game.
- `uv run tools/verify_matching.py` records fresh compilation and differential
  emulation evidence. `uv run python tools/verify_fidelity.py` audits the active
  Windows store and source/result provenance. Extend function-specific build/test
  contracts for new routines; the auditor supports the whole active inventory.
- Run `uv run python tools/db.py update` after tracking/results change, and
  `uv run python tools/db.py update --check` before committing exports.
- Once retargeted, run the applicable Windows build, link, and provenance checks
  after each implementation feature. Document the actual supported commands and
  inputs with the tooling change. No DOS compatibility or DOS runtime regression
  requirement applies; preserve original assets and unrelated working-tree work.
- For `src/` changes, run the applicable CMake build and focused tests/runtime checks.
- Resolve new failures before committing; report existing failures separately.
  Report compilation, instruction matching, and runtime validation separately.
- Documentation-only changes require diff review and `git diff --check`; builds
  and code audits are required when implementation or build tooling changes.

## Commits and completion

- Commit each completed feature separately with its accompanying documentation.
- Use Conventional Commits such as `re: ...`, `port: ...`, or `docs: ...`; include
  verified original addresses and deviation IDs when relevant.
- Review the staged diff and include only the current feature's changes.
- Report changes, validation, limitations, and commit hash. Explain any commit blocker.
