# Ignition (1997) Reverse Engineering & Porting Rules

**Goal:** Phase 1: Functional decompilation of `MAINDOS_32BIT.EXE` to pure C. Phase 2: 100% byte-matching decompilation. Phase 3: Modern C11/SDL2 source port.

## 1. Decompilation Pipeline (MAINDOS_32BIT.EXE)
- **Target:** Exclusively reverse-engineer `MAINDOS_32BIT.EXE`. Use `IGN_WIN.EXE` only as a source port reference.
- **Phase 1 (Functional & Semantic Reverse Engineering):** Decompile functions into pure, functionally identical, human-readable C code using Open Watcom V2. A 100% byte-match is NOT required in this phase due to compiler differences. ASM Fallbacks should ONLY be used for genuinely handwritten assembly, not for C compiler mismatches.
- **Full Reverse Engineering Standard (No Raw Ghidra Dumps):** Raw decompiler pseudocode must NEVER be committed. Every decompiled function must undergo a full reverse engineering pass:
  - **Struct Recovery:** Replace raw pointer offsets (`*(int *)(ptr + 0x38)`) with proper C struct definitions and field accesses (`camera->enable_sky`).
  - **Semantic Variables:** Rename all decompiler register artifacts (`iVar1`, `uVar2`, `puVar3`, `param_1`, `local_400`) to meaningful, human-readable variable names reflecting game logic.
  - **Global Identification:** Identify and replace raw `DAT_XXXXXXXX` memory addresses with authentic, descriptive global symbols (`g_ViewportMinX`, `g_LisaCamera`) and register them in `docs/ghidra/globals.md`.
  - **Symbol Identification:** Determine the authentic role of unnamed `FUN_XXXXXXXX` routines, assigning meaningful names in code, `docs/ghidra/functions.md`, and `database/decomp.db`.
  - **Clean Code Style:** Use clean, idiomatic C89 formatting (4-space indentation, no redundant blank lines, standard control flow).
- **Phase 2 (Byte-Matching):** (Future) Procure Watcom C/C++ 10.6 to resolve compiler idiosyncrasies and shift functional C code to 100% bit-for-bit matching.
- **Clean-Room:** Never commit original executables or assets. `tools/extract_assets.py` MUST validate SHA-1 checksums of user-provided game files.
- **Pure C Only:** Never use inline assembly (`__asm`) in `.c` files.
- **Assembly Slicing:** `tools/unpack_dos_le.py` disassembles sections into `wasm` stubs. These are linked with matching `.c` code via `wlink` for a continuous bootable replacement.
- **Verification:** Run `uv run python tools/verify_matching.py` to track progress. Maintain `objdiff.json` for GUI diffing.

## 2. Source Port & Fidelity Tracking (FCTS)
- **Tech Stack:** Engine in C11/SDL2. Tools in Python (managed via `uv`). Build via CMake.
- **Provenance Annotations:** Every ported function in `src/` MUST have a header containing:
  - `@original <SymbolName> (IGN_WIN.EXE @ 0x<Address>, <SourceFileHint>)`
  - `@fidelity EXACT | ADAPTED | EXTENDED | INFRASTRUCTURE`
  - `@deviation DEV-XXX` (required if logic diverges or fixes an authentic bug)
  - `@fix_category FIX_CAT_XXX` (required if toggleable in options)
- **Deviation Logging:** Register all bug fixes in `docs/tracking/deviations.md`. Original bugs must remain toggleable via options (defaulting to 1997 authentic behavior).
- **Enforcement:** Run `uv run python tools/verify_fidelity.py` before committing to ensure 100% parity between docs and code.

## 3. Documentation & Tooling
- **Docs First:** Code and documentation must evolve in lockstep. Always update `docs/ghidra/functions.md`, `globals.md`, `structs.md`, and `docs/formats/*.md`.
- **Ghidra MCP:** Use the active MCP server (`localhost:8080`) to inspect disassembly, decompile, and sync the symbol database directly.
- **Commits:** Use Conventional Commits (`feat`, `port`, `fix`, `re`, `docs`). Cite function addresses and deviation IDs.
- **No Git Bloat:** Keep binary assets (`assets/`) and build outputs out of version control.

## 4. Agent Environment Constraints
- **OS & Shell:** Windows 11 with PowerShell.
- **Tooling:** `w64devkit` is on PATH (provides standard UNIX/GNU tools like `make`, `gcc`, `grep`, `sed`, `awk`). Git and `uv` (Python) are installed.
- **Restrictions:** Do NOT attempt to run Linux package managers (`apt`, `brew`) or assume a native Linux environment.
