# Bounded delegation

`AGENTS.md` owns policy. This reference supplies model paths, assignments and
handoffs; it is not an executable lock or a substitute for workflow gates.

## Assign work

Keep ambiguous ABI/layout recovery and architecture with the lead. Suitable worker
tasks include bounded caller/xref surveys, initialized-data inspection, implementing
a settled routine contract and independent evidence review. Avoid duplicate broad
analysis or delegation whose briefing/review costs exceed doing the task directly.

Identify the integration owner and record each task and file ownership before edits.
Coordinate with other active work; staged changes do not authorize including them.
Use an isolated worktree if clean write ownership cannot be maintained. Worktrees
do not isolate Ghidra. Do not merge SQLite files or worker-generated exports: the
owner applies verified records to the active store after source changes settle.

## Codex Luna path

Use `collaboration.spawn_agent` with `model: "gpt-6-luna"`,
`reasoning_effort: "high"` (or `"low"` for mechanical tasks), and
`fork_turns: "none"` or a supported limited-history count. Send the task packet
below. Full-history forks in this interface inherit the parent model. Other clients
may use different fields; use supported model selection and check the actual model.
Keep global defaults unchanged. Missing Luna access is a limitation to report.

## Antigravity / agy Flash path

Use the user's Antigravity environment or `agy` CLI, not Gemini CLI agent files.
`GEMINI.md` routes Antigravity to the shared project policy and skill. In the IDE,
open this repository/worktree, select an available Gemini Flash model in the model
picker and send a filled task packet. Record the actual model/effort. Model choice
is a session setting, not something this Markdown file can enforce.

For `agy`, run from the assigned repository/worktree in PowerShell:

```powershell
agy --help
agy models
```

Choose an exact Flash slug from that listing. Do not hardcode a version from a
documentation example or use Pro/Auto fallback. For an interactive worker:

```powershell
$flashModel = Read-Host 'Exact Flash slug from agy models'
if ($flashModel -notmatch 'flash') { throw 'Select a listed Gemini Flash model' }
agy --model $flashModel --effort high
```

The string check is not availability validation; match the slug to `agy models`.
For a headless evidence/review task, the lead first creates `build/delegation/`
and saves a filled task packet as UTF-8 `task.txt` there. Then run:

```powershell
$taskPacket = Get-Content -Raw -LiteralPath 'build/delegation/task.txt'
agy --model $flashModel --effort high --mode plan --print-timeout 120s --output-format json -p $taskPacket
if ($LASTEXITCODE -ne 0) { throw 'agy worker failed; inspect diagnostics and JSON error' }
```

Plan mode supports read/review; still include explicit read-only boundaries in the
packet. Keep normal permission controls. For implementation, use a separate Flash
session with a settled contract and exclusive files; hand ownership back before
integration. File ownership is an instruction, not a filesystem sandbox: use an
isolated worktree when necessary. Do not pass `--dangerously-skip-permissions` or
broaden global tool permissions to make headless work run.

Keep raw logs under disposable `build/delegation/` or return results in conversation.
Inspect JSON status/error, actual model if exposed, duration and usage. Report
unavailable metrics honestly. For user-selected custom Antigravity agents, inspect
`agy agents` and confirm their effective model; do not assume the main-session
selection changes nested agents. This path uses one separate worker session, with
no further delegation, counted in the same worker budget as Luna.

Local setup check on 2026-10-07: `agy.exe` was found and `--help` confirmed these
flags. `agy models` failed in this execution environment with authentication and
profile-write errors. Available slugs and a live Flash run remain unverified; the
user's IDE session may have different access. Report that limitation rather than
claiming absent account access or a passing worker run. The lead can continue with
Luna while Antigravity access is resolved. Codex collaboration does not accept
Gemini model IDs; launch `agy` as a separate process only within allowed permissions.

Sources (accessed 2026-10-07):
[Antigravity headless/model flags](https://www.antigravity.google/docs/cli/headless/),
[Antigravity project rules](https://www.antigravity.google/docs/rules/),
[OpenAI subagents](https://learn.chatgpt.com/docs/agent-configuration/subagents).

## Evidence-first sequence

1. Lead selects an authorized routine and performs the normal doctor/preflight.
   Share target diagnostics and starting revision/input hashes. Reuse current
   doctor evidence only while inputs remain unchanged. Ghidra's active target is
   the user-provided assumption, separate from the local fingerprint check.
2. Assign independent evidence questions to up to two workers, such as callers
   versus initialized data. Facts need instruction/data provenance; label hypotheses.
3. Lead resolves contradictions and establishes ABI, inputs/outputs, side effects,
   verified fields, branches/boundaries, dependencies and limitations. Missing
   behavior blocks dependent implementation; independent evidence work may continue.
4. Assign implementation with exclusive source/evidence files. Shared headers and
   harness edits are proposals for the integration owner. Only explicitly allowed
   isolated checks may run in a worker without recording results or altering shared
   artifacts. Flash evidence/review sessions return proposals, not implementation edits.
5. Release implementation ownership and assign another worker to review settled
   source against original instructions/data, C89, provenance and coverage. Supply
   evidence locations and the contract, not the author's confidence as proof.
6. Lead reviews and resolves findings. Once edits settle, the owner updates each
   affected RVA, extends verifier contracts and runs the existing completion,
   staged-gate and commit sequence in `docs/workflow.md`. Revalidate changed inputs.

Stop the affected worker task and return evidence if it needs an unverified ABI,
layout or boundary, target evidence contradicts expectations, necessary Ghidra is
unavailable, a write is outside ownership or a shared operation is required. The
lead handles required user questions/reassignment or continues the difficult part.
Do not invent facts/stubs, broaden ownership, change models or retry identical
failures repeatedly. Independent work may continue.

## Task packet

Fill relevant fields; retain policy and ownership boundaries.

```text
Role and objective:
Repository/worktree absolute path:
Read: AGENTS.md; .agents/skills/ignition-windows-decomp/SKILL.md;
      .agents/skills/ignition-windows-decomp/references/delegation.md
Applicable user constraints:
Integration owner and other assigned work:
Target identity/diagnostic evidence; starting revision or input hashes:
Routine VA and RVA; evidence supporting those addresses:
Evidence paths and bounded callers/data to inspect:
Established facts / implementation contract:
Open questions (label hypotheses, including historical DOS findings):
Allowed writable files: none unless explicitly listed
Permitted checks and isolated output locations:
Acceptance criteria and stop/escalation conditions:
Return findings and changes using the return packet below.
Do not spawn agents or mutate shared Ghidra state. Do not run preflight,
completion, shared verifiers, DB writes, exports, staging or commits.
```

## Return packet

```text
Outcome: complete assigned slice / partial / blocked
Verified VA/RVA and provenance (instruction/data locations and evidence paths):
Findings: ABI, extent confidence, fields/packing, branches, side effects,
          dependencies and observations relevant to the assignment
Hypotheses and contradictory/missing evidence:
Files changed (or none), proposed shared changes, input revision/hashes:
Checks actually run: command, result, artifact location, limitations
Checks not run:
Suggested tracking fields for the integration owner (not applied by worker):
Remaining questions and next action:
Ownership released / remaining work:
```

Keep returns concise and link evidence instead of dumping raw decompiler output.
Assigned-slice completion is not feature completion. Separate compilation,
instruction comparison, emulation and native runtime observations. Never infer a
result from status labels or review approval.

## Three-routine pilot

Status: not yet measured. Apply to the next three suitable routines within
authorized reconstruction work. This setup is not a routine trial and does not
authorize extra reconstruction. Avoid selecting only trivial work for broad claims.

Maintain a pilot note with feature evidence or under `docs/`; keep operational
metrics out of generated inventories. Record every trial, including failures:

| Field | Record |
| --- | --- |
| Scope | Verified RVA, feature/commit, complexity and dependencies |
| Assignment | Actual provider/model/effort, roles, concurrency and owned files |
| Time | UTC start/end, feature wall time, briefing/review/integration time and worker elapsed times; overlapping worker times are not wall time |
| Usage | Observed per-run tokens/cost if exposed; otherwise unavailable; account limits are not per-task usage |
| Rework | Lead correction time and substantive corrections, retries/escalations |
| Verification | First integrated attempt pass/fail/blocked/not run; command/result evidence, scope and later attempts |
| Completion | Real snapshot ID, affected RVAs, commit and limitations, or blocker |
| Comparison | Comparable lead-only baseline if available; otherwise no savings claim |

Analysis-only trials mark implementation verification not run with the limitation.
After three trials compare time, observable usage and rework with comparable
lead-only work, accounting for complexity and provider. Keep up to two workers by
default; a third needs independent work and evidence that coordination pays off.
Narrow assignments or reduce concurrency when corrections erase the benefit.
Do not claim measured savings without data.
