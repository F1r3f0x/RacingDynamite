# Ignition (1997) Reverse Engineering & Porting Rules

**Goal:** 100% matching decompilation of `MAINDOS_32BIT.EXE` (Watcom C) and a modern C11/SDL2 source port.

## 1. Decompilation Pipeline (MAINDOS_32BIT.EXE)
- **Target:** Exclusively byte-match `MAINDOS_32BIT.EXE`. Use `IGN_WIN.EXE` only as a source port reference.
- **Clean-Room:** Never commit original executables or assets. `tools/extract_assets.py` MUST validate SHA-1 checksums of user-provided game files.
- **Assembly Slicing:** `tools/unpack_dos_le.py` disassembles sections into `wasm` stubs. These are linked with matching `.c` code via `wlink` for a continuous bootable replacement.
- **Verification:** Run `uv run python tools/verify_matching.py` to prevent regressions. Maintain `objdiff.json` for GUI diffing.

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
