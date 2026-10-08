# Windows decompilation tooling audit

Inspected on 2026-10-07 (America/Santiago). Scope: build/verification scripts,
database schema/CLI, dashboard generator, Ghidra bridge/synchronizer, runtime
staging, dependency declarations, and project skill discovery. Existing DOS
builds and symbol synchronization were not executed.

## Status after the first Windows milestone

The findings table below records the **pre-migration inspection**. Build,
verification and fidelity entry points now support only the verified Windows
`mem.c` routine; see [current evidence and commands](ghidra/windows_startup.md).
PE/CPU dependencies are pinned in PEP 723 script metadata and supplied by `uv run`
in isolated environments. Database, dashboard, old audits and old runtime staging
remain legacy, explicitly excluded from active Windows progress. The target
manifest now records the bounded validation scope. Original compiler identity and
native visual baseline are still unresolved.

## Historical findings before this milestone

| Area | Observed implementation | Improvement |
| --- | --- | --- |
| Target identity | Tools independently hardcode DOS names; DB metadata says `MAINDOS.EXE` and Open Watcom V2 | Use `decomp/target.json` as the shared expected identity; verify SHA-256 before analysis/build comparisons |
| Build | `build_decomp.py` selects `wcc386`, Watcom register flags, a DOS link script, and `MAINDOS_REBUILT.EXE` | Retarget in place to an evidence-supported x86 Windows toolchain; introduce a bounded module/harness build first |
| Comparison | `diff_func.py` reads LE data; `normalize_asm` discards call targets and broad numeric operands | Read authentic PE sections and relocations; preserve call/global identity and semantic constants; report raw and relocation-aware comparisons separately |
| Fidelity audit | Uses DOS address fields and invokes `audit_pe_residue.py`, which flags `0x004...`/`0x005...` addresses | Replace the DOS-specific rule with target-aware provenance and initialized-data checks; legitimate Windows addresses are not residue |
| Tracking/dashboard | DOS-primary records, hardcoded DOS target/Watcom UI, inherited `matching` labels | Reset active inventory from verified Windows analysis; associate evidence with binary SHA/VA/RVA and keep reconstruction, matching, and runtime metrics distinct |
| Ghidra synchronization | Infers target from section text, defaults to DOS on failure, and applies database addresses directly | Establish loaded-program identity before mutation; add preview/dry-run and per-operation results, reject unknown identity, and sync only verified records |
| Bridge errors | `safe_get`/`safe_post` return ordinary text containing errors | Expose structured success/error results so failures cannot be mistaken for decompiler evidence |
| Runtime baseline | Stages DOS binaries and DOSBox configs | Retarget original/rebuilt staging to Windows disposable copies; record compatibility wrappers/configuration and observed behavior |
| Dependencies | `capstone` is installed locally but absent from project dependencies; `pefile`/`lief` are absent | Declare libraries actually required by active tools in the lockfile. Choose one PE reader when needed; don't install both without a use case |

Useful behavior to retain from current tooling: fresh-output checks, compiler/linker
failure propagation, missing-symbol detection, focused regression tests, and the
explicit separation of instruction comparisons from runtime fidelity. The Windows
migration supersedes DOS support, not these reliability properties.

## Improvements added with this audit

- `decomp/target.json`: machine-readable expected local Windows identity. Compiler
  remains unknown and pipeline status is `migration_pending`.
- `tools/decomp_doctor.py`: standard-library, read-only JSON diagnostics. Verifies
  target SHA/header identity, checks dependency availability and compiler paths,
  reports legacy markers and DB metadata, and optionally probes documented Ghidra
  GET endpoints. It never builds, edits tracking, or synchronizes symbols.
- `.agents/skills/ignition-windows-decomp/SKILL.md`: repository skill for authentic
  Windows analysis, bounded C89 reconstruction, and evidence-based validation.
  It points to current project policy rather than duplicating it.
- `tests/test_decomp_doctor.py`: verifies entry-address calculation, rejects
  truncated/non-PE32 data and path escapes, and detects changed binary content
  even when headers and file length are unchanged.

Run from the repository root:

```powershell
uv run python tools/decomp_doctor.py
uv run python tools/decomp_doctor.py --probe-ghidra
uv run python -m unittest discover -s tests -p test_decomp_doctor.py
```

Exit code zero means the inspection succeeded and expected target identity
matched. It does not mean the Windows build pipeline is ready. Inspect
`legacy_markers`, dependency availability, and the pending pipeline state.
Ghidra HTTP responses establish reachability only, not loaded-program identity.
Marker scanning is a migration aid, not proof of full tool correctness.

## Observed environment and validation

- `IGN_WIN.EXE` matched the expected SHA-256, size, PE32/x86 headers, image base,
  and entry RVA; its section count is eight.
- Project environment: `requests`, `mcp`, and `capstone` import locations are
  available; `pefile` and `lief` are absent.
- GCC and Clang are on PATH; `cl` was not found on PATH. This is not an installed
  compiler census, an x86 compile test, or identification of the game's compiler.
- Local Ghidra `/methods` and `/segments` returned HTTP 200. Loaded-program
  fingerprint verification remains unresolved; the bridge exposes no such
  identity method in the inspected Python interface.
- Four focused doctor tests passed; the bundled skill frontmatter/scaffold
  validator passed with temporary PyYAML. No Windows game build, instruction
  match, or runtime parity is claimed by this audit.

## Next work in order

1. Use the doctor plus Ghidra to establish the correct loaded program, then export
   verified Windows functions/imports/sections and trace startup. Extend the bridge
   with program identity when necessary; do not bulk-sync inherited records.
2. Reconstruct one bounded non-CRT helper and retarget its build/verification path.
   Record compiler evidence and actual x86 output architecture. Test against
   original execution or independently derived instruction/data expectations.
3. Migrate the active DB/CLI/fidelity audit to Windows identity and evidence stages,
   and retarget the comparator using genuine PE relocation records.
4. Generate docs and a byte-weighted treemap from that verified inventory, then
   retarget runtime staging for original/rebuilt startup comparisons.

The current skill is deliberately one focused workflow. Add separate function
validation or runtime-baseline skills only when their repeatable commands and
contracts have been implemented and demonstrated. Skills guide tool use; they
do not add a debugger, expose missing Ghidra APIs, or certify a matching score.

Repository skills are discoverable under `.agents/skills` for agents launched in
this repository. Start a fresh project chat and use `$ignition-windows-decomp`.
This projectless chat does not automatically inherit skills from another checkout.
See [the official skill documentation](https://learn.chatgpt.com/docs/build-skills).

## Windows tracking migration follow-up

Active database/CLI, generic provenance audit, persisted bounded harness results,
synchronized exports and Windows treemap are now implemented. Historical findings
above describe the earlier audit state. Remaining work includes compiler identity,
broader routine-specific harnesses, CRT/non-FPO classification, relocation-aware
comparison, Ghidra identity/synchronization and native runtime staging/observation.
