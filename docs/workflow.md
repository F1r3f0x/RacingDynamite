# Enforced agent workflow

`AGENTS.md` owns project policy. `tools/workflow.py` makes selected rules executable;
it does not certify whether an implementation preserves the original game architecture.

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

## Start and finish a Windows feature

```powershell
uv run python tools/workflow.py preflight --rva 0x5b410 --file decomp/src/mem.c
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
Completion takes an exclusive `build/workflow/completion.lock`, runs the current
real Windows verifier, audits fidelity, requires fresh passing compilation and
emulation for every requested RVA, exports and checks the exports. It writes
`build/workflow/handoff.json` with snapshot ID, affected records/results and limitations.
It never stages or commits. The verifier supports the five memory handle routines
and extracted-production-C `Font_GetTextWidth` and `Font_DrawText` contracts;
the latter executes the original sprite wrapper with a modeled renderer boundary.
Extend the verifier before
completing a different reconstructed routine.
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
```

For restricted execution environments with an unwritable default uv cache, set
`$env:UV_CACHE_DIR = Join-Path (Get-Location) 'build/uv-cache'` before invoking uv.

## Implementation validation (2026-10-07)

The guard suite has 24 tests, including staged versus unstaged content, the CI HEAD
snapshot, database-only promotions, missing inputs, stale artifacts, omitted exports,
completion failure/lock handling, the analysis-only handoff and actual Windows hook
launch/rejection (requires `uv`/`pwsh`). The existing Windows
tracking and doctor suites also passed (10 and 4 tests respectively).

The actual Windows verifier compiled the focused DLL and passed 1,946 differential
emulation comparisons for RVAs `0x5B1F0`, `0x5B1B0`, `0x5B360`, `0x5B240`,
`0x5B410`, plus eight original-only negative-cursor cases. Fresh records and exports
use snapshot `a3bf7f86a261`; the fidelity audit and staged export check passed.
Raw byte equality differed; instruction equality and native game parity are not
claimed. No game routines or analysis-stage labels changed in this workflow feature.
