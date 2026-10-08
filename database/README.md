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

Import a missing non-FPO candidate with `db.py add-candidate <RVA> --size <bytes>
--evidence <document> --confidence <boundary explanation>`. The command verifies
the target fingerprint, file-backed executable extent and existing evidence,
hashes its authentic bytes, rejects duplicate RVAs, and leaves stage/fidelity
unidentified/unknown. Omit `--size` for an observed call target whose extent is
not recovered; byte size and routine hash then remain null. A directly observed
call target in a file-backed section lacking the PE execute flag requires explicit
`--allow-nonexecutable` and documented evidence; this does not certify native NX
behavior. Import does not
certify control-flow closure or promote analysis/test results. Independently
recover boundaries before describing/promoting a candidate. The non-FPO import
hash is authenticated at import time; the current auditor's automatic original
extent/hash comparison covers FPO records, so rerun the feature's pinned-byte
analysis for non-FPO freshness.

After recovering a previously unknown extent, use
`db.py describe <RVA> --size <bytes> --evidence <document> --confidence <explanation>`.
This authenticates and hashes the file-backed extent without promoting its stage.
Entries in a section without the execute flag also require `--allow-nonexecutable`.
The command rejects replacement of an already established size; correcting such
an extent requires a separately reviewed migration. Seven isolated candidate tests
cover import and extent description. See the [triangle evidence](../docs/ghidra/windows_triangle.md)
for newly bounded native edge/gradient helpers and original-only rendering scope.

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
`Mem_ShutdownHandles` adds 531 differential comparisons using explicit callback
models; [shutdown evidence](../docs/ghidra/windows_handle_shutdown.md) distinguishes
verified call-boundary behavior from unvalidated original callback bodies.
`Mem_ReleaseHandleId` adds 418 differential comparisons for duplicate-ID release,
all-slot boundaries and five-routine integration; see
[release evidence](../docs/ghidra/windows_handle_release.md).
Exact/relocated instruction comparison, linked equality, and native game parity
remain unclaimed. Historical imported evidence is preserved but does not count
as a fresh passing run until reproduced.

`Gfx_DrawSpriteNative` at RVA `0x571B0` adds 2,588 differential cases with native
compiled x87 instructions and a recovered, modeled downstream boundary. Its
28 original-only downstream adapter/lookup executions corroborate that contract,
without promoting those dependencies to reconstructed status. See
[sprite backend evidence](../docs/ghidra/windows_sprite_backend.md). The focused
sprite DLL requires GCC x86/x87 and LLD; all result kinds retain separate source,
toolchain and artifact provenance. Framebuffer/native rendering is unverified.

The treemap offers function, module and address-order views, search and stage/class
filters. Blocks use original extent sizes, with minimum-area placeholders for
unknown sizes. Aggregate byte metrics use interval unions to avoid overlap
double-counting. There is no overall game-completion percentage.
