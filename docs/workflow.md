# Enforced Windows reconstruction workflow

Current instructions live here; completed feature reports are preserved in
[workflow history](workflow_history.md). The active reconstruction state and
remaining game limitations are in [the plan](decomp_plan.md).

## Setup

Run in PowerShell from the repository root, with `uv`, Git and PowerShell 7 (`pwsh`)
on PATH:

```powershell
uv run python tools/workflow.py install-hooks
```

This sets local `core.hooksPath` to the tracked `.githooks` directory and refuses
to replace an existing different hooks directory. Each clone must install its hooks.
Git's tiny shell launchers invoke `tools/git_hooks.ps1` through PowerShell 7;
all workflow operations run in PowerShell/Python. The `.ps1` extension is required
by PowerShell on Windows. The pre-commit hook validates the index and
the commit-msg hook checks Conventional Commits. Reconstruction source commits
need an `RVA: 0x...` trailer. Multiple RVAs can use separate trailers.

## Choose module ownership before implementation

`decomp/modules.json` records our reconstruction boundaries, state ownership,
dependencies, build mode and placement rationale. `original_source: null` means
the original Windows translation-unit filename is unknown. An annotation or
semantic subsystem grouping does not recover that filename.

Preflight with RVAs requires `--module` (repeat for affected owning modules),
`--placement-reason` and explicit `--file` entries for those module sources plus
all other feature files. It saves that decision with the target diagnostics.
An unidentified candidate can have a proposed owner; its eventual semantic name
and source must satisfy the ownership rule before reconstruction is completed.

New subsystems require a manifest entry and an appropriate production module.
Migrated modules compile directly as complete C89 translation units. Input uses
`decomp/src/input.c` / `decomp/include/input.h`, with its private state types and
explicit Win32 platform bindings. See [migration evidence](ghidra/windows_input_module.md).
The shared builder and three font/file link harnesses compile and link the entire
input object; its bodies and header are no longer extracted from `geputget.c`.

Remaining extracted modules have an `approved_symbols` map with a documented
placement reason for each existing routine. A new routine cannot silently join
`geputget.c`; prefer migrating its subsystem, or document a reviewed temporary
exception in that map. The staged gate checks the staged manifest, unique
reconstructed definitions and source/tracking agreement. The manifest does not
certify unvalidated legacy bodies or prove fidelity by itself.

Move one verified subsystem at a time, carrying its state and private declarations
with it. Keep mechanical moves separate from behavior changes. Use
`db.py describe <RVA> --source <path>` to update both source and module membership,
retain the original binary/ABI evidence, and record the actual reconstruction
stage using `set-status`. A source move does not promote an analysis stage.
Rerun affected contracts and callers before completing a migration.

`windows_tracking.record_run` includes full input source/header dependencies for
shared-builder runs. Snapshot generation hashes each shared file once per call;
the next call rereads it. No persistent hash cache or freshness bypass is used.
Read/write text with explicit UTF-8. Tracked text uses LF via `.gitattributes`,
so working/index hashes are reproducible
on Windows. Finish source changes before validation and check those bytes before
committing; cached metrics and dashboard snapshots do not refresh themselves.

## Start and finish a Windows feature

```powershell
uv run python tools/workflow.py preflight --rva 0x5b410 --module memory --placement-reason "Verified handle registry ownership" --file decomp/src/mem.c
# Analyze, implement, describe/set-status by verified RVA, and extend the real verifier.
uv run python tools/workflow.py complete --rva 0x5b410 --limitation "No native game runtime validation"
# Review changes, stage explicit feature paths, then:
uv run python tools/workflow.py check --staged
git diff --cached --check
git diff --cached
git commit
```

Preflight saves target diagnostics, declared RVAs/files and initial Git status to
`build/workflow/preflight.json`. Availability is not verified Ghidra program identity.
For Ghidra inspection, assume the user has the authentic `IGN_WIN.EXE` loaded
and active. This user-provided assumption does not require an identity endpoint
and must not be reported as an automated fingerprint check. If MCP is not
responding or reports no active program, prompt the user to open `IGN_WIN.EXE`
in Ghidra with the MCP plugin enabled, then retry. Contradictory target responses
require asking the user to select `IGN_WIN.EXE` before continuing. Local binary
fingerprint and routine provenance checks still apply.

Completion takes an exclusive `build/workflow/completion.lock`, runs the current
real Windows verifier, audits fidelity, requires fresh passing compilation and
emulation for every requested RVA, exports and checks the exports. It writes
`build/workflow/handoff.json` with snapshot ID, affected records/results and limitations.
It never stages or commits. The verifier runs the registered function-specific Windows contracts, including
compilation, differential emulation and their applicable scoped native checks.
Extend a real contract and result recording before completing a new routine.
The focused DLLs are validation artifacts; they are not a playable rebuilt game.
Compilation, instruction comparison, emulation and native execution remain separate
evidence kinds. Differential emulation does not establish native game parity.

Analysis-only work uses `complete --analysis-only --rva ... --limitation "..."`.
This requires a stated limitation and analyzed records with ABI, classification
and evidence; it refuses reconstructed routine records. It
audits and exports without manufacturing a test result. Ordinary documentation
changes need diff review and whitespace checks; tracking/evidence documentation
also triggers the applicable fidelity/export checks at commit time.

If completion crashes, inspect the lock's PID and confirm its owner is no longer
running before removing the lock. The lock only coordinates this completion command;
direct DB commands and verifier invocations do not acquire it. Prefer isolated
worktrees and one integration owner for harness, SQLite, exports and final commit.

## What the staged gate checks

- Staged whitespace, protected originals/assets, known build artifacts and raw
  output directories (`decomp/raw`, `scratch`, `reports`, Ghidra project directories).
- Added/changed C definitions in `decomp/`: a unique Windows tracking record,
  matching preferred VA and fidelity, reconstructed stage, ABI, classification
  and included evidence. Deviation tags must exist in the deviations document.
  Deleting a definition still tracked as reconstructed fails.
- Newly introduced assembly tokens outside comments/string literals. Existing
  unmigrated routines are not certified; unchanged legacy definitions are allowed.
- Module ownership, unique definitions, approved legacy placements and database/source agreement.
- SQL, inventory, Markdown and both dashboard exports agree with the staged store.
- Full local mode also runs the staged fidelity/export auditors against a temporary
  index snapshot. Only the fingerprinted original and `build/` result artifacts
  are copied from the working directory; source, evidence and verifier inputs must
  already be included in the snapshot. Changed reconstructed routines require
  fresh passing compilation and differential emulation records.
  A DB-only promotion to reconstructed is also subject to the result requirement.

The gate materializes Git blobs under `build/workflow/` and removes its disposable
tree on exit. It does not stash, alter the index, write the authoritative SQLite
store or regenerate working-tree exports. A working-tree fix absent from the index
cannot make the commit pass. The C scanner supports ordinary C89 definitions and
uses conservative text comparison; macro-generated/K&R definitions and semantic
fidelity still require review. Raw decompiler output in arbitrary text files cannot
be identified reliably by path rules.

## CI and enforcement limits

`.github/workflows/policy.yml` runs guard regression tests and checks the committed
HEAD snapshot against the event's base on Windows. `--base <commit> --portable`
does not need proprietary originals or ignored build artifacts. It checks policy
and structural export consistency and explicitly does **not** certify freshness,
binary provenance or runtime fidelity. No proprietary data is uploaded.

Configure the `Repository policy / policy` check as required in the repository's
branch rules. This workflow alone does not configure branch protection. For full
fidelity CI, provision a trusted Windows runner with the authentic binary, compiler
and local verification artifacts, run the real verifier/export sequence, then run
`check --base <commit>` without `--portable`. Restrict that runner to trusted code;
checks execute scripts from the revision under review.

Local hooks can be bypassed; they are feedback, while required remote checks protect
merges. Changes to policy/checker files also require review. There is no claim that
scripts can prove an agent read every instruction or that a recorded pass is honest.

Focused regression tests:

```powershell
uv run python -m unittest discover -s tests -p test_workflow.py
uv run python -m unittest discover -s tests -p test_module_ownership.py
uv run python -m unittest discover -s tests -p test_windows_tracking.py
```

For restricted execution environments with an unwritable default uv cache, set
`$env:UV_CACHE_DIR = Join-Path (Get-Location) 'build/uv-cache'` before invoking uv.
