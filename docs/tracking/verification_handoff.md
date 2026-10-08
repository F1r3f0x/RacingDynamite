# RacingDynamite verification and handoff

> Retained DOS/legacy reference as of 2026-10-07. Windows `IGN_WIN.EXE` is now the primary target; see [the active plan](../decomp_plan.md). This document does not establish Windows progress, compiler selection, or runtime fidelity. Historical findings and unresolved checks are preserved below.

Work remains incomplete: the startup divergence is reproducible, but the repaired timer has not been validated in DOSBox. The user stopped Computer Use with physical Escape; all subsequent desktop interaction stopped. No commits were created.

## Changes

- Build verification now stops on compilation/linker failure or missing tracked symbols, requires fresh nonempty objects/executable/map, removes undefsok from the generated link script, exposes compiler/linker diagnostics and propagates launch failures.
- Build uses bundled Open Watcom V2 (2.0 beta, Sep 22 2026), register ABI. The local C89 pointer typedef shim follows Watcom's guards/aliases. Uppercase decompiler ABS/SQRT pseudo-intrinsics now use declared fabs/sqrt to eliminate unresolved symbols.
- Fidelity auditing reads registry columns by name, includes addressless rows and DOS sources/headers, checks implementation annotations, registry/database counts/status/address/fidelity, deviation references/categories/toggles, and PE residue. All summaries are dynamic. It explicitly reports behavioral fidelity as unverified.
- Timer_Init (0x10198), Timer_GetTime (0x10238), Timer_GetPITCounter (0x1034c) restored from authentic LE instructions and initializers. Elapsed-time return, rollover sampling/reset, clamp, calibration, rotation and side effects are recovered. The uncapped FPS divisor remains; its zero-interval FPU conversion edge is not claimed identical across compilers.
- Database function/global records, SQL dump/schema, function/global/structure docs and timer provenance updated. Historical status labels remain separate from the new implementation_audits evidence table.
- Inventory covers 270 DOS definitions, 52 constant-return/empty candidates (not all proven stubs), 475 uncertain initializer/type/extent declarations, callers/users, known/unknown addresses, and verification needs. The 198-row registry reconciles with the database; the earlier 180 count excluded 18 addressless functions. Obsolete PE-conversion guidance was removed.

## Verification

| Check | Result |
| --- | --- |
| Open Watcom V2 fresh compilation/link | PASS: 8 translation units; 195/195 tracked implementation symbols present |
| Compiler warnings | 174; codes {'W113': 25, 'W102': 135, 'W131': 3, 'W1180': 4, 'W1181': 1, 'W124': 3, 'W126': 1, 'W111': 1, 'W104': 1}; files {'fx.c': 106, 'lisa3d.c': 67, 'menu.c': 1} |
| Compiler errors / linker diagnostics | 0 compiler errors; final linker log empty |
| Optional normalized instruction comparison | 5/179 exact normalized instruction matches; not a byte-equality or functional-completion metric |
| Verification regression tests | 16 tests PASS |
| Actual timer C89 harness | 1 test PASS, covering elapsed/fractional time, BIOS boundary, clamp/backward reset, ROR, epoch/calibration |
| Fidelity audit | FAIL: 210 errors, 18 warnings; existing annotation/tracking gaps remain visible |
| git diff --check | PASS; only line-ending normalization notice |
| Original asset hash comparison | No original asset changes; two preexisting generated rebuilt executables disappeared externally during the session and are reported separately |
| Ghidra | localhost:8080 refused connections; no synchronization claimed |
| Post-fix DOSBox | UNVERIFIED: Computer Use stopped by user |

Compiler warnings primarily concern existing pointer conversions/type mismatches in FX and Lisa3D. Their visibility is now preserved; this is not a warning-free or fidelity-complete build. Full logs are build/status_matching.log, status_fidelity.log, status_regression.log and status_timer_tests.log; build/decomp/wlink.log is empty for the final successful link.

## Runtime evidence and next work

Authentic startup showed its language selector twice, accepted Enter, displayed LOADING and played recognizable car/person intro animation. The pre-fix rebuild showed gray/black output and no recognizable selector/intro; its boot.log reached successful Menu_Init. Car/track selection, driving, HUD, audio, results and return were not validated. See docs/tracking/runtime_baseline.md for reproduction and evidence limits.

Next: replay the freshly repaired timer build in the same DOSBox configuration. If startup still fails, recover the native menu initialization/layout/display/CDP chain from authentic MAINDOS.EXE. Menu_AddLayoutItem, Menu_ClearLayout, Menu_LayoutItems, Lisa_ResetRasterizerContext, Track_LoadPlacementsAndCars, height queries, HUD and audio remain candidates. Do not infer their priority from compilation alone or implement guessed replacements.

Useful project files: docs/tracking/implementation_inventory.md, docs/tracking/runtime_baseline.md, docs/ghidra/timer.md. Reproduce code checks with `uv run python -m unittest discover -s tests -p test_verification.py`, `uv run python -m unittest discover -s tests -p test_timer.py`, `uv run python tools/verify_matching.py` and `uv run python tools/verify_fidelity.py`.
