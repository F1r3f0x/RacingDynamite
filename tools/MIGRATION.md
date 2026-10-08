# Tools retirement and Windows migration

Cleanup date: 2026-10-07. `IGN_WIN.EXE` is the active reconstruction target.

## Active tools

Keep the Windows build, verification, inspection, tracking and dashboard tools:
`build_decomp.py`, `verify_matching.py`, `verify_fidelity.py`, `windows_target.py`,
`windows_inspect.py`, `windows_tracking.py`, `db.py`, `generate_dashboard.py` and
`decomp_doctor.py`. Asset inspectors, converters, source helpers and renderer
prototypes remain available; their output alone does not certify Windows fidelity.
The Ghidra MCP bridge remains available for bounded inspection.

## Retained, migration required

The authoritative list and individual migration requirements are in
[migration_required.json](migration_required.json). Display it with:

```powershell
uv run python tools/tool_migration.py
```

Every listed Python script has a `MIGRATION REQUIRED` header and an unconditional
guard before its old imports and code. Execution and import stop with a nonzero
exit before legacy dependencies, asset/binary reads, database writes, downloads or
Ghidra requests. The guard itself reads only the migration manifest.
There is no bypass flag. Retained LE imports are historical implementation material;
the deleted parser must be replaced, not restored as a Windows dependency.

To reactivate a script, implement its documented Windows contract, verify the
target fingerprint and relevant behavior, remove its guard and manifest entry,
and update this documentation in the same feature. Do not merely remove the notice.

Objdiff binaries, `tools/objdiff/` and `objdiff.json` are preserved. The configuration
still names obsolete DOS objects and **must be retargeted before use**; its manifest
entry is advisory because the external objdiff application does not read our guard.
`setup_decomp_tools.py` now installs objdiff only. Objconv and its source are retained
as potentially reusable object-format tooling, without an active matching claim.

## Retired

- `tools/WATCOM/` and `tools/openwatcomv2/`: local DOS compiler installations.
- `le_parser.py`: DOS LE reader; Windows inspection uses PE readers.
- `audit_pe_residue.py`: obsolete audit that rejects legitimate Windows addresses.
- `update_fx_db.py`, `update_lisa_db.py`, `update_fx_source_annotations.py`,
  `update_lisa_source_annotations.py`: one-off hardcoded DOS metadata rewrites.
- Watcom download/install code in `setup_decomp_tools.py`.
- Root DOSBox launchers `run_dosbox.bat` and `run_original.bat`.

Git history retains retired source. Historical DOS research documents remain
labeled evidence, including references to retired tools. Original game files,
assets, scratch experiments, build/runtime outputs and Ghidra projects are outside
this cleanup. Existing DOS reconstruction tests need migration with their modules;
they are not Windows acceptance gates.

## Validation

```powershell
uv run python -m unittest discover -s tests -p test_tool_retirement.py
uv run python -m unittest discover -s tests -p test_decomp_doctor.py
uv run python tools/decomp_doctor.py
uv run python tools/build_decomp.py
uv run python tools/verify_fidelity.py
git diff --check
```

The retirement test executes every blocked script from an empty working directory,
with network/database/process mutations trapped, and checks that no output files
are created. Windows compilation/linking and provenance audits remain separate
from instruction matching and native runtime validation. This cleanup claims no
new instruction or native runtime fidelity.

Cleanup verification: all 33 guarded execution/import attempts passed, as did the
four target-doctor tests. The retained objdiff installer reused existing downloads
and verified CLI 3.8.1. The focused DLL compiled/linked; the Windows verifier passed
30 initializer and 377 consumer differential executions, plus eight original-only
cursor probes. Raw code bytes differed; instruction equality and native runtime
parity were not claimed. The provenance audit passed immediately after refreshing
results, but subsequently reported stale inputs/artifacts as the separate Windows
working-tree feature continued changing. Generated-output drift was also reported.
Those source/tracking/export changes are excluded from the tool-cleanup commit;
refresh validation and exports when that feature stabilizes.
