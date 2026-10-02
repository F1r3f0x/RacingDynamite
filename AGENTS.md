# Ignition (1997) Reverse Engineering & Porting Rules

**CURRENT PHASE:** Functional Implementation of `MAINDOS.EXE` using pure C (Open Watcom V2). The goal is to reach 100% `FUNCTIONAL` fidelity. Structural equivalence is what matters.


## 1. Decompilation Constraints
- **TARGET:** Our primary source of truth is the authentic `Ignition/Ignition/MAINDOS.EXE` DOS4GW binary. We have scrapped absolute reliance on the flawed `MAINDOS_32BIT.EXE` Ghidra database, which was repackaged and lost critical `.data` segment initialization (e.g. jump tables) and calling convention fidelity.
- **VERIFICATION PIPELINE:** You MUST use the `tools/verify_capstone.py` and `tools/le_parser.py` pipeline to disassemble and extract initializers directly from the authentic `MAINDOS.EXE` LE file when porting complex systems (like the 3D rasterizer).
- **NO RAW GHIDRA DUMPS:** NEVER commit unrefined decompiler output. You MUST:
  - **Structs:** Replace raw pointer offsets with proper C struct definitions and field accesses (e.g., `camera->enable_sky`).
  - **Variables:** Rename ALL register artifacts (e.g., `iVar1`, `tmp_esi`) to meaningful semantic names. Beware of Watcom C `ESI`-based `.data` segment addressing being incorrectly decompiled as parameters.
  - **Globals:** Replace raw `DAT_XXXXXXXX` addresses with authentic globals (e.g., `g_ViewportMinX`) AND document them in `docs/ghidra/globals.md`.
  - **Functions:** Rename `FUN_XXXXXXXX` to semantic names in code, `docs/ghidra/functions.md`, AND `database/decomp.db`.
- **ARCHITECTURE (Pure C):** The project is strictly a Pure C implementation. Hand-written assembly fallbacks (`decomp/src/asm/*.asm`) are banned. If Watcom C fails to produce a 1:1 byte match due to optimization differences (e.g. instruction selection, floating point ops), it is fully acceptable as long as the logic is functionally exact and respects the original game architecture.
- **NO AD-HOC IMPLEMENTATIONS:** You MUST reverse-engineer the authentic game systems and architectures (UI widgets, menus, render pipelines). Writing custom state machines, bypassing native systems, or placing code in completely unrelated files (e.g. putting UI rendering code inside the 3D renderer) just to "make something work quickly" is **STRICTLY FORBIDDEN**.
- **PRESERVE ORIGINAL ASSETS & BINARIES:** NEVER modify or overwrite `MAINDOS.EXE` or any other original game assets in `Ignition/`. Doing so will invalidate our tests and corrupt test environments.
- **STYLE:** Use pure C89 (4-space indent). NEVER use inline assembly (`__asm`) in `.c` files.
- **WORKFLOW:** Run `uv run python tools/verify_matching.py` to verify. The tool still outputs matching percentages, but aim for a clean, warning-free build and logically equivalent implementation rather than agonizing over 100% instruction diffs.

## 2. Source Port & Fidelity Tracking
- **STACK:** C11/SDL2 engine, Python tools (via `uv`), CMake build.
- **ANNOTATIONS:** EVERY function in `src/` MUST include this header comment:
  ```c
  // @original <SymbolName> (MAINDOS_32BIT.EXE @ 0x<Address>, <SourceFileHint>)
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
