---
name: ignition-windows-decomp
description: Reconstruct and validate routines from RacingDynamite's authentic Ignition Windows executable, or migrate its DOS analysis/build tools to Windows. Use for this project's binary analysis and C89 decompilation, not generic C development or other games.
---

# Ignition Windows reconstruction

Use this skill from the RacingDynamite repository. Read its current `AGENTS.md`
and `docs/decomp_plan.md`; those files own project policy. Resolve paths relative
to that repository, not the chat's projectless directory.

## Delegate bounded work

Apply the delegation policy in `AGENTS.md` when independent tasks justify workers.
Read [references/delegation.md](references/delegation.md) before assigning work for
the Luna/Gemini Flash paths, task/return packets and three-routine pilot. The lead
owns integration and required progress handoff. Workers perform only their assigned
slice and return; they do not run the feature-wide tracking/export/commit sequence.

## Establish evidence

Run `uv run python tools/decomp_doctor.py` before trusting addresses or inherited
progress. `decomp/target.json` records the expected binary identity; the doctor
reads only, reports dependencies/legacy assumptions, and does not certify a build.
Use `--probe-ghidra` for bounded, read-only localhost reachability checks.

Assume the user has the authentic `IGN_WIN.EXE` loaded and active in Ghidra,
as specified in `AGENTS.md`. Record this as a user-provided operating assumption,
not an automated fingerprint check. The missing program-identity endpoint does
not block inspection. If MCP is not responding or reports no active program,
prompt the user to open `IGN_WIN.EXE` with the MCP plugin enabled, then retry.
If responses contradict the target, ask the user to select `IGN_WIN.EXE`.
Continue verifying the local binary fingerprint and each routine's provenance.
The legacy synchronizer guesses the target with a DOS fallback; do not run bulk
`sync_ghidra.py` until its target handling and source records have been verified.
Prefer bounded inspection by address, callers/xrefs, and initialized data.

The PE entry is not automatically `WinMain`; distinguish CRT/import thunks from
game routines. Record VA/RVA, extent confidence, ABI, signedness, field packing,
initializers, and side effects. Decompiler parameter lists and guessed names are
hypotheses. Recover function boundaries using control-flow/call evidence, not just
the next symbol. For PE reads, distinguish raw section bytes from virtual/BSS tails.

## Reconstruct a bounded feature

Choose a routine with a useful contract and manageable dependencies, then recover
its behavior from Windows instructions and data. Replace affected DOS code inside
`decomp/`; don't preserve flawed code as a required foundation or add a second tree.
Existing DOS statuses and `win_address` values are not verified Windows evidence.
Use the active plan for migration decisions rather than hardcoding an old policy.

Do not invent success-returning dependencies to make a build pass. Keep prototypes
and globals explicit. Select compiler/ABI settings from local binary evidence;
separate a provisional behavior-validation compiler from any exact-codegen claim.

## Validate and record

Read the relevant tools before running them: the doctor reports current legacy
markers. Migrate only what the task needs, keeping original assets untouched.
Record exact supported commands, compiler flags/version, source inputs, and fresh
output scope. A compiled object or harness is not a rebuilt playable executable.

Use original execution where practical, or independently derived assembly/data
expectations for normal and boundary cases. Compare output, side effects, and ABI.
The old `diff_func.py` discards call targets and broad numeric operands during
normalization; its score cannot establish equality. Preserve symbolic call/global
identity and normalize only verified relocations in any replacement comparator.

Track reconstructed source, raw/relocated instruction comparison, linked match,
and observed runtime behavior separately. Reset active progress from independently
verified Windows evidence, keeping retained historical provenance labeled.
Follow the repository's documentation, validation, and commit requirements.

For tooling migration priorities and inspected evidence, read
`docs/tooling_audit.md` only when changing the infrastructure. Do not install
unrelated tools or bulk rewrite the project simply because this skill is loaded.
