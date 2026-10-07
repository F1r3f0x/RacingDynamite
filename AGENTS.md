# Ignition (1997): Agent Instructions

## Goal and scope

Reimplement the authentic `Ignition/Ignition/MAINDOS.EXE` DOS4GW binary with functional fidelity and the original game architecture. Byte-for-byte matching is secondary to correct behavior.

- `decomp/`: pure C89, Open Watcom V2, 4-space indentation. No handwritten assembly or inline `__asm`.
- `src/`: C11/SDL2 source port, built with CMake.
- Tools: Python through `uv`. Use Windows PowerShell; `git`, `uv`, and `w64devkit` are on PATH. Do not run Linux package managers.

## Non-negotiable constraints

- Treat `MAINDOS.EXE` as the source of truth. Do not use the flawed `MAINDOS_32BIT.EXE` PE repackaging; its data initialization and calling conventions are unreliable.
- Never modify or overwrite original binaries or assets in `Ignition/`. Use disposable copies for runtime experiments.
- Reconstruct native systems and place code in the appropriate module. Do not bypass original UI widgets, menus, or rendering pipelines with invented replacements.
- Never commit raw decompiler output, binary assets, or generated build outputs.
- Preserve unrelated working-tree changes. Stage explicit files or hunks belonging to the current feature.

## Reverse engineering and implementation

1. Inspect the authentic binary before implementing behavior. For complex systems, use `tools/verify_capstone.py`, `tools/disasm_le.py`, and `tools/le_parser.py` to verify disassembly and extract LE initializers.
2. Use the Ghidra MCP server at `localhost:8080` to inspect and synchronize symbols when available. Report unavailable tooling; do not invent evidence or addresses.
3. Replace raw offsets with named structs and fields. Rename register artifacts (`iVar1`, `tmp_esi`) semantically. Watch for Watcom ESI-based data references misidentified as function parameters.
4. Replace `DAT_XXXXXXXX` and `FUN_XXXXXXXX` with meaningful names. Update global and struct definitions in `docs/ghidra/globals.md` and `docs/ghidra/structs.md`; keep function names synchronized across code, `docs/ghidra/functions.md`, and `database/decomp.db`.
5. Update code and its documentation together. Accept compiler instruction differences only when behavior and architecture remain faithful.

## Fidelity tracking

Every function in `src/` and `decomp/` requires these annotations. Add deviation and fix-category lines only when applicable:

```c
// @original <SymbolName> (MAINDOS.EXE @ 0x<Address>, <SourceFileHint>)
// @fidelity EXACT | ADAPTED | EXTENDED | INFRASTRUCTURE
// @deviation DEV-XXX
// @fix_category FIX_CAT_XXX
```

Choose one fidelity value and use verified addresses. Register behavioral changes and bug fixes in `docs/tracking/deviations.md`. Preserve original bugs behind toggles defaulting to 1997 behavior.

## Build and validation

Run commands from the repository root after each implemented feature and before its commit:

```powershell
uv run python tools/build_decomp.py
uv run python tools/verify_matching.py
uv run python tools/verify_fidelity.py
```

- `build_decomp.py` is the required decompilation build entry point. Its default mode compiles; use `uv run python tools/build_decomp.py --link` to validate executable linking when the feature affects the rebuilt game.
- For `src/` changes, also run the applicable CMake build. Run focused tests or runtime checks appropriate to the changed behavior.
- Resolve build failures and new verification failures before committing. Aim for a warning-free build. Report existing failures separately with evidence; do not describe failing checks as passing or expand the feature into unrelated repairs.
- Compilation, annotations, and matching percentages do not prove behavioral fidelity. Report runtime validation separately.
- For documentation-only changes, review the diff and run `git diff --check`; builds and code audits are required when implementation or build tooling changes.

## Commits and completion

- Commit each completed feature separately after validation, with its accompanying documentation. Do not accumulate multiple completed features in one commit.
- Use Conventional Commits such as `re: ...`, `port: ...`, or `docs: ...`. Include relevant original addresses and deviation IDs when applicable.
- Review the staged diff before committing; include only the current feature's changes.
- In the completion report, state what changed, validation results and limitations, and the commit hash. If validation prevents a commit, explain the blocker explicitly.
