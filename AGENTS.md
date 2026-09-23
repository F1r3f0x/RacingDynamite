# Ignition (1997) Reverse Engineering & Porting Rules

**CURRENT PHASE:** Functional decompilation of `MAINDOS_32BIT.EXE` to pure C (Open Watcom V2). Byte-matching is NOT required in this phase.


## 1. Decompilation Constraints (MAINDOS_32BIT.EXE)
- **TARGET:** ONLY reverse-engineer `MAINDOS_32BIT.EXE`. Use `IGN_WIN.EXE` purely as a reference.
- **NO RAW GHIDRA DUMPS:** NEVER commit unrefined decompiler output. You MUST:
  - **Structs:** Replace raw pointer offsets with proper C struct definitions and field accesses (e.g., `camera->enable_sky`).
  - **Variables:** Rename ALL register artifacts (e.g., `iVar1`, `param_1`) to meaningful semantic names.
  - **Globals:** Replace raw `DAT_XXXXXXXX` addresses with authentic globals (e.g., `g_ViewportMinX`) AND document them in `docs/ghidra/globals.md`.
  - **Functions:** Rename `FUN_XXXXXXXX` to semantic names in code, `docs/ghidra/functions.md`, AND `database/decomp.db`.
- **STYLE:** Use pure C89 (4-space indent). NEVER use inline assembly (`__asm`) in `.c` files.
- **WORKFLOW:** `tools/unpack_dos_le.py` creates `wasm` stubs, linked via `wlink`. Run `uv run python tools/verify_matching.py` to verify.

## 2. Source Port & Fidelity Tracking
- **STACK:** C11/SDL2 engine, Python tools (via `uv`), CMake build.
- **ANNOTATIONS:** EVERY function in `src/` MUST include this header comment:
  ```c
  // @original <SymbolName> (IGN_WIN.EXE @ 0x<Address>, <SourceFileHint>)
  // @fidelity EXACT | ADAPTED | EXTENDED | INFRASTRUCTURE
  // @deviation DEV-XXX (if logic diverges)
  // @fix_category FIX_CAT_XXX (if toggleable)
  ```
- **DEVIATIONS:** ALWAYS register bug fixes in `docs/tracking/deviations.md`. Original bugs MUST remain toggleable (defaulting to 1997 behavior).
- **ENFORCEMENT:** Run `uv run python tools/verify_fidelity.py` before committing.

## 3. Tooling & Documentation
- **GHIDRA MCP:** Use the active MCP server (`localhost:8080`) to inspect and sync the symbol database.
- **DOCS FIRST:** Code and docs (`docs/ghidra/functions.md`, `globals.md`, `structs.md`) MUST evolve in lockstep.
- **COMMITS:** Use Conventional Commits (`re: ...`, `port: ...`) citing addresses and deviation IDs.
- **ENVIRONMENT:** Windows PowerShell. `w64devkit`, `git`, and `uv` are on PATH. NEVER run Linux package managers. NEVER commit binary assets/build outputs.
