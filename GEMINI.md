# Antigravity / agy project context

Read `AGENTS.md` from this repository; it owns project policy. Use
`.agents/skills/ignition-windows-decomp/SKILL.md` for Ignition Windows analysis and
reconstruction, and its `references/delegation.md` before assigning worker tasks.

For a bounded worker session in Antigravity, explicitly select an available Gemini
Flash model in the model picker. For `agy`, list models with `agy models` and pin
the exact available Flash slug with `--model`. Record actual model and effort.
Do not silently switch to Pro or Auto if Flash is unavailable.

Use the delegation task/return packets and default to read-only evidence/review.
Implementation needs an established contract and exclusive file ownership. The
lead retains uncertain binary interpretation; one integration owner controls
SQLite, shared harnesses, verification/results, exports, staging and commits.
Workers do not spawn further agents or mutate shared Ghidra state.

This context file routes to shared project instructions. It does not configure
authentication, enforce filesystem isolation or change global model defaults.
