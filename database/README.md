# Active Windows tracking

`decomp.db` is the authoritative tracking store for the fingerprinted standard
Windows `IGN_WIN.EXE`. `schema.sql` defines schema v2 and `dump.sql` is its
version-controlled, restorable SQL snapshot. No DOS completion labels carry over.

The initial inventory contains 1,012 authentic FPO extents. Most are unclassified
candidates: FPO does not establish complete game-function coverage or code-only
extents. Boundaries start with medium confidence; the recovered handle initializer
has separately corroborated boundaries and preserved milestone evidence.

Addresses are integer RVAs; VAs derive from the target's preferred image base.
ABI/fidelity default to unknown. Reconstruction stages, classification, source
provenance, and verification runs are separate records. Current passing evidence
excludes stale source/harness/artifact hashes and superseded results.

Run from the repository root:

```powershell
uv run python tools/db.py status
uv run python tools/db.py audit
uv run python tools/db.py update
uv run python tools/db.py update --check
```

`update` generates the JSON inventory, function registry, SQL dump, and both HTML
dashboards from one snapshot. `--check` reports drift without writing. JSON and
Markdown are outputs, not independently editable progress stores. Describe a
candidate through `db.py describe <RVA>`; set its analysis stage through
`db.py set-status <RVA> <stage>`. Neither command marks tests passed. Run audit
and update after edits. Source reconstruction requires verified provenance and
an evidence document; exact instructions/native behavior need separate results.

`migrate-windows` replaces the recognized legacy DOS database only after verifying
the original fingerprint and imported Windows claims. It atomically installs the
replacement and is a no-op on the already-active Windows store. It rejects unknown
schemas/targets instead of silently resetting them. The legacy database is
recoverable through Git history. The old destructive `init`, markdown importer,
arbitrary SQL mutation and bulk Ghidra sync are not active CLI commands.

Verification results are appended by the real harness, not status labels. The
current `uv run tools/verify_matching.py` freshly compiles and emulates
`Mem_InitHandles`, `Mem_NextHandleId` and `Mem_RegisterHandle`; it records
compilation, raw-byte differences and emulation separately (30, 377 and 590
differential cases).
Eight additional consumer cases execute only the original; the
[consumer evidence](../docs/ghidra/windows_handles.md) explains differing negative
address surroundings and the memory contract. Other routines need their own
verified contracts/harnesses.

The [bookkeeping evidence](../docs/ghidra/windows_handle_bookkeeping.md) records
the first-free-slot contract, ordered effects and three-routine integration.
Exact/relocated instruction comparison, linked equality, and native game parity
remain unclaimed. Historical imported evidence is preserved but does not count
as a fresh passing run until reproduced.

The treemap offers function, module and address-order views, search and stage/class
filters. Blocks use original extent sizes, with minimum-area placeholders for
unknown sizes. Aggregate byte metrics use interval unions to avoid overlap
double-counting. There is no overall game-completion percentage.
