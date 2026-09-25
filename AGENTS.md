# Ignition (1997) Reverse Engineering & Porting Rules

**CURRENT PHASE:** 1:1 Assembly Byte-Matching of `MAINDOS_32BIT.EXE` using pure C (Open Watcom V2). The goal is to reach 100% `EXACT` fidelity.


## 1. Decompilation Constraints (MAINDOS_32BIT.EXE)
- **TARGET:** ONLY reverse-engineer `MAINDOS_32BIT.EXE`. Use `IGN_WIN.EXE` purely as a reference.
- **NO RAW GHIDRA DUMPS:** NEVER commit unrefined decompiler output. You MUST:
  - **Structs:** Replace raw pointer offsets with proper C struct definitions and field accesses (e.g., `camera->enable_sky`).
  - **Variables:** Rename ALL register artifacts (e.g., `iVar1`, `param_1`) to meaningful semantic names.
  - **Globals:** Replace raw `DAT_XXXXXXXX` addresses with authentic globals (e.g., `g_ViewportMinX`) AND document them in `docs/ghidra/globals.md`.
  - **Functions:** Rename `FUN_XXXXXXXX` to semantic names in code, `docs/ghidra/functions.md`, AND `database/decomp.db`.
- **ARCHITECTURE (DOOM-Style Fallbacks):** For complex or heavily optimized functions where Watcom C cannot mathematically generate a 100% byte match, we adopt id Software's *DOOM* source release architecture:
  1. **Assembly Target:** Extract the original, hand-written 1997 assembly into `decomp/src/asm/<module>.asm`. This guarantees the 1:1 byte match.
  2. **Pure C Fallback:** Write the clean, 100% functionally equivalent Pure C version in the `.c` file, but disable it via preprocessor directives (e.g., `#if 0`). This preserves portability for the modern source port while satisfying the immediate byte-matching goal.
- **STYLE:** Use pure C89 (4-space indent). NEVER use inline assembly (`__asm`) in `.c` files. You MAY use `#pragma aux` in `.h` files or within `.c` files to force Watcom's register allocation and instruction generation for 100% byte-matching, **HOWEVER, every use of assembly or weird hacks MUST BE reviewed by the USER first. You must explain the problem, offer options to solve it, and wait for the USER to make the decision.**
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
