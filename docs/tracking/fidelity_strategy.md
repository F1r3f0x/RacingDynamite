# Fidelity & Change Tracking Strategy (FCTS) Developer Guide

This document defines the strict engineering guidelines, code provenance standards, divergence tracking rules, and verification procedures for **Racing Dynamite** (Ignition 1997 source port).

---

## 1. Core Philosophy: Golden Rules of Reverse Engineering & Fidelity

1. **Never Guess When You Can Verify**: Every ported function, constant, lookup table, and algorithm must be traced back to its decompiled origin in `IGN_WIN.EXE` (base address `0x00400000`) or reference source in `MAINDOS.EXE`.
2. **Document First, Implement Second**: Update `docs/ghidra/functions.md`, `globals.md`, `structs.md`, or format specifications *before* or *simultaneously with* the C implementation.
3. **Preserve Authentic Behavior**: The baseline engine must behave *identically* to the original 1997 game. Any fixes, adaptations, or enhancements must be categorized, documented, and made toggleable via `GameFixOptions` or `RendererOptions`.
4. **Machine-Verifiable Traceability**: All C source files must use standardized inline provenance annotations so automated tooling (`tools/verify_fidelity.py`) can audit fidelity coverage continuously.

---

## 2. Standardized Inline Code Provenance Headers

Every function defined in `src/` and declared in `include/ignition/` must be preceded by a Doxygen-style provenance block.

### Required Header Syntax:

```c
/**
 * @brief Brief description of the function.
 * @original <SymbolName> (IGN_WIN.EXE @ 0x<Address>, <SourceFileHint>)
 * @fidelity EXACT | ADAPTED | EXTENDED | INFRASTRUCTURE
 * @deviation DEV-XXX (Optional: Deviation ID if behavior diverges or fixes bug)
 * @fix_category FIX_CAT_XXX (Optional: Category key controlling this fix)
 * @notes Nuances, formula details, original variable names, register allocations.
 *
 * @param ...
 * @return ...
 */
```

### Fidelity Classification Levels:

* **`EXACT`**:
  * 100% 1:1 algorithmic and mathematical equivalence with decompiled assembly.
  * Same constants, formulas, bit shifts, coordinate transformations, and branch conditions.
  * Example: `Car_UpdateAxleSpeeds`, `Surface_TestTrianglePositiveDZ`.

* **`ADAPTED`**:
  * Faithful preservation of authentic logic, but modernized for C11/SDL2 or platform portability.
  * Examples: replacing Win32 DirectDraw surfaces with SDL surfaces, Win32 `ReadFile` with libc `fopen`/`fread`, endian-safe byte-swapping macros.
  * Example: `File_LoadToMemory`, `Font_DrawText`, `Track_LoadSplines`.

* **`EXTENDED`**:
  * Authentic logic preserved, plus enhancements (e.g., telemetry hooks, widescreen aspect ratio corrections, out-of-bounds safety guards).
  * Must reference a `@deviation` ID if the change prevents authentic bugs.
  * Example: `Surface_Raycast` (includes out-of-bounds grid clamping to prevent crashes).

* **`INFRASTRUCTURE`**:
  * Modern source port scaffolding not present in `IGN_WIN.EXE`.
  * Examples: logging engine (`src/core/log.c`), SDL platform backend (`src/platform/platform_sdl.c`), CMake build files, test suites.

---

## 3. Authentic Divergence & Deviation Registry (`docs/tracking/deviations.md`)

When a bug in the 1997 game is discovered (e.g. falling through road meshes in Austria, vehicle floating/sinking, division by zero, camera clipping into geometry):
1. **Do not silently fix it without logging**.
2. **Assign a unique ID**: `DEV-001`, `DEV-002`, `DEV-003`, etc.
3. **Document the entry in `docs/tracking/deviations.md`**:
   * **ID**: `DEV-XXX`
   * **Category**: e.g., `FIX_CAT_NOCLIP`, `FIX_CAT_ELEVATION`, `FIX_CAT_CAMERA`, `FIX_CAT_AI_PATHING`, `FIX_CAT_AUDIO`, `FIX_CAT_RENDERER`.
   * **Original Address & Assembly**: Disassembly snippet and decompilation from `IGN_WIN.EXE`.
   * **Original Bug / Quirk**: Explanation of why the original engine broke or behaved strangely.
   * **Source Port Solution**: How the C11 implementation solves it cleanly.
   * **Option Toggle**: The corresponding field in `GameFixOptions` (e.g., `fixes->fix_noclip`).
4. **Conditional Execution**:
   ```c
   if (fixes && fixes->fix_noclip) {
       // Modern fix: safe clamp
       cx = Clamp(cx, 0, srf->header.grid_stride_x - 1);
   } else {
       // Authentic 1997 behavior: raw unchecked access
   }
   ```

---

## 4. Reverse Engineering Workflow

Follow these steps for every new subsystem or function:

```
[1. Ghidra Discovery]
         │
         ▼
[2. Documentation]  ──> docs/ghidra/functions.md
         │          ──> docs/engine/ or docs/formats/
         ▼
[3. C Implementation] ──> Inline @original & @fidelity headers
         │
         ▼
[4. Divergence Check] ──> If diverging/fixing bug: log in docs/tracking/deviations.md
         │
         ▼
[5. Unit Testing]   ──> tests/test_*.c
         │
         ▼
[6. Automated Audit] ──> uv run python tools/verify_fidelity.py
         │
         ▼
[7. Changelog]      ──> CHANGELOG.md & Conventional Commit
```

---

## 5. Commit Standards

All commits must follow the **Conventional Commits** specification with subsystem scopes:
* `feat(physics)`: New authentic physics or vehicle dynamics feature.
* `port(renderer)`: Porting an authentic rasterizer function from Ghidra.
* `fix(collision)`: Fixing a collision bug (must cite `DEV-XXX` if applicable).
* `re(lisa3d)`: Reverse engineering analysis, Ghidra symbol renames, decompilation notes.
* `docs(...)`: Updating format specs, engine architecture, or tracking documentation.
* `test(...)`: Unit test additions or test harness improvements.

### Commit Description Requirements:
If a commit introduces or touches ported code:
1. Mention the original function address: `(matches FUN_00412fc0 @ 0x00412fc0)`.
2. Mention any deviation ID: `(implements DEV-001 under FIX_CAT_NOCLIP)`.
