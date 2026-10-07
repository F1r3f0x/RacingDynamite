# Ignition (1997): Agent Instructions

## Goal and scope

As of 2026-10-07, reconstruct the authentic standard Windows release at
`Ignition/Ignition/IGN_WIN.EXE` with functional fidelity and the original game
architecture. Byte-for-byte matching is secondary to correct behavior.
See `docs/decomp_plan.md` for the active plan and target fingerprint.

- Windows reconstruction: readable C89; establish the original compiler, ABI,
  data layout, and build configuration from this binary before selecting a toolchain.
- `decomp/` currently contains the preserved DOS/Open Watcom V2 reconstruction.
  Do not overwrite it with Windows code. A separate Windows source/build path is
  planned; it is not implemented yet. Use 4-space indentation and no handwritten
  assembly or inline `__asm`.
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
4. Keep code, documentation, and tracking synchronized. Existing
   `docs/ghidra/functions.md`, `globals.md`, `structs.md`, and `database/decomp.db`
   contain legacy evidence; their Windows-style addresses are not automatically
   verified against `IGN_WIN.EXE`. Preserve provenance until target-aware tracking
   is implemented. Never transfer DOS completion statuses to Windows.
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
Keep existing DOS annotations tied to `MAINDOS.EXE`; do not relabel their addresses.
Record behavioral changes in `docs/tracking/deviations.md` and preserve original
bugs behind toggles defaulting to authentic behavior. An annotation alone is not
proof of instruction equality or runtime fidelity.

## Build and validation

- A Windows reconstruction build and verification pipeline does not exist yet.
  Establish it before claiming Windows build, matching, or runtime success.
- For changes to the retained DOS implementation or its tooling, run:

```powershell
uv run python tools/build_decomp.py
uv run python tools/verify_matching.py
uv run python tools/verify_fidelity.py
```

- `build_decomp.py --link` validates DOS executable linking when needed. These
  checks are DOS-specific and do not certify `IGN_WIN.EXE` reconstruction.
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
