# Enforced agent workflow

## Graphics aligned allocation dependencies (2026-10-08)

[Allocation dependencies](ghidra/windows_gfx_alloc_aligned.md), RVAs
0x5F4A0/0x61840, now have readable production C89. The zero-size wrapper and
unsigned aligned allocator retain unchecked failure behavior and back-pointer
placement. 48/512 differential comparisons include 24/252 persistent repeats
and eight aligned-helper fault cases; CRT malloc is modeled. Prior suites
remain intact. Inventory: 1,028 candidates, 39 reconstructed routines.
Page allocation RVA 0x617E0 remains the next separate feature. Instruction
equality, original compiler/link layout and native heap/game parity are unverified.


## Sprite packing node links (2026-10-08)

[The link helper](ghidra/windows_gfx_sprite_packing_links.md), VA 0x00461690 /
RVA 0x61690, now has production C89 preserving its ordered two-to-four node
stores and incidental previous-pointer EAX. 864 standalone differential calls
per binary include all null/distinct/exact-alias configurations and 464
persistent follow-ups, including sixteen six-call relinking sequences. Node
reads, traversal and calls are absent. Both callers are inspected statically;
no packing-caller/startup integration is claimed. All prior coverage remains,
including 810 packing resets / 922 real bodies and 280 default initializers /
392 real bodies through the explicit modeled primitive driver. Full completion
renews compilation/emulation evidence for all 37 reconstructed routines among
1,028 candidates. Instruction equality, original compiler/link layout and native
graphics/heap/game parity remain unverified. Packing callers are unreconstructed;
primitive/CRT/callback boundaries remain modeled.

## Sprite packing state reset (2026-10-08)

[The packing reset](ghidra/windows_gfx_sprite_packing_reset.md), RVA 0x614D0,
now has production C89 with an independently recovered 24-byte node template,
257 opaque bucket pointers and separate page head. It preserves exact store
order and repeated orphaning without free. 810 standalone comparisons include
405 persistent repeats; 922 real executions per binary include 112 startups
through the explicitly modeled primitive driver. Default initialization executes
first. Four consumers have static ownership/layout analysis only; no consumer
production or native packing parity is claimed. All prior suites remain intact.
Inventory: 1,028 candidates, 36 reconstructed routines; instruction equality,
original compiler/link layout and native graphics/heap/game parity unverified.
Earlier milestone entries below are historical.


## Default sprite descriptor initialization (2026-10-08)

[The default initializer](ghidra/windows_gfx_default_descriptor.md), VA
0x004611D0 / RVA 0x611D0, now has production C89 and independent differential
coverage: 280 standalone comparisons (132 persistent repeats), exact eight
ordered stores/no reads, arbitrary nine-word tail preservation and EAX=0.
It executes 392 real bodies per binary, including 112 startups through an
explicitly modeled primitive driver. The real primitive initializer remains
unreconstructed and is not executed. Sixteen real helper/handle consumption
chains add coverage; existing standalone comparison counts and all prior suites
remain. Full completion renews compilation/emulation evidence for all 35
reconstructed routines; inventory remains 1,028 candidates. Instruction equality,
original compiler/link layout and native graphics/heap/game parity remain
unverified. CRT/callbacks remain modeled and workspace consumers unreconstructed.
Use `uv run python tools/workflow.py complete --rva 0x611D0` with an explicit
limitation. Earlier milestone entries below are historical.


## Real lazy sprite workspaces (2026-10-08)

[Workspaces A/B](ghidra/windows_gfx_sprite_workspaces.md) at RVAs 0x5D840/0x5C9F0
execute production C89 bodies through sprite installation, backend selection and
18 lifecycle startups. Each adds 167 standalone comparisons, 83 persistent repeats
and 757 total real executions per binary. All allocation-failure combinations,
exact-zero/noncanonical flags, immediate pointer stores, skipped preservation and
forced-zero orphaning are checked with CRT malloc modeled. Existing descriptor,
handle, surface, banner, input, memory/font/file/backend coverage is retained.
Four direct consumers are analyzed only; heap buffers remain opaque. Primitive
startup, CRT and callbacks remain modeled. Instruction equality, original linked
layout and native graphics/heap/game parity remain unverified. Full completion
uses `uv run python tools/workflow.py complete --rva 0x5D840 --rva 0x5C9F0`;
the verifier regenerates current compilation/emulation evidence for all 34 routines.

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
It never stages or commits. The verifier supports the five memory handle routines
and extracted-production-C `Font_GetTextWidth` and `Font_DrawText` contracts;
the latter executes the original sprite wrapper with a modeled renderer boundary.
The native sprite backend `Gfx_DrawSpriteNative` at RVA `0x571B0` additionally
uses extracted production C with GCC x87; its zero-argument downstream boundary
is modeled after executing the original adapter and descriptor lookup separately.
Rasterizer bodies and framebuffer output are unverified. Extend the verifier before
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

## Sprite installer validation follow-up (2026-10-08)

The real verifier now supports Gfx_InstallSpriteDispatch at RVA 0x56E60,
with sixteen typed globals, three explicit downstream models and preserved
real surface/selector/lifecycle integration. See
[the contract and validation scope](ghidra/windows_gfx_sprite_dispatch.md).
Current coverage adds 306 sprite comparisons, 48 original-only consumer ABI
checks and 43 original-only initializer contracts. Instruction equality and
native graphics/game parity remain unverified.

## Real sprite handle/helper follow-up (2026-10-08)

[Handle initialization and descriptor copying](ghidra/windows_gfx_sprite_handles.md)
reconstruct RVAs 0x5C7F0/0x612E0 and execute the real helper through real handles,
sprite installation, selector and 18 lifecycle startups. Fresh coverage adds
486 helper and 48 handle comparisons, plus 16 original-only default initializer
checks; existing 306/266 sprite/surface installers, 532 selectors, 48/39 consumer
ABI and 43 downstream contracts, banner/input/memory validation remain.
Default initialization writes seven words and preserves nine; only live copies
clear +0x18/+0x1C. Two workspace initializers and primitive/CRT/callback boundaries
remain modeled. Strict C89 compilation and differential emulation pass;
instruction equality, original compiler/link layout and native graphics/game
parity remain unverified. Earlier model/opaque-descriptor descriptions are
historical. Inventory: 1,028 candidates, 32 reconstructed routines.
