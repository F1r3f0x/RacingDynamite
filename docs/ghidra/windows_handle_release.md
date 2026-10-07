# Windows handle-ID release (2026-10-07)

Bounded reconstruction: `Mem_ReleaseHandleId` in `decomp/src/mem.c`. The semantic
name and module association are inferred; original symbol/source filename remain
unknown. All consequential evidence comes from authentic
`Ignition/Ignition/IGN_WIN.EXE`, 915,968 bytes, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
PE32 x86 preferred base is `0x00400000`; CRT entry RVA/VA
`0x00069950` / `0x00469950` is distinct from application entry `0x004120A0`.

## Independently recovered contract

VA `0x0045B410`, RVA `0x0005B410`, extent `[0x0045B410,0x0045B44E)`:
62 bytes / 16 instructions, followed by two INT3 bytes. Embedded FPO metadata,
complete control flow and padding agree. Routine SHA-256:
`d0fee3704a333538872dacd400ee150f38a7a7dcfe33e1770f357d09ffb8d1cb`.
One raw dword stack argument at ESP+4, ordinary RET with caller cleanup, EAX result.
ECX is scratch; EBX/ESI/EDI/EBP are untouched. No calls, imports or dependency stub.

- `0x0045B410` reads initialization flag at `0x004BAB38`; zero returns **0**
  immediately, without argument/table reads or writes. Any nonzero flag enables
  the scan. It does not write or recheck the flag.
- `0x0045B41E` loads the raw argument from ESP+4. ECX starts at zero and is a byte
  offset; ADD ECX,4 / CMP ECX,0x320 / signed JL visits **all 200 slots**, offsets
  0..796. No index-200 read/write occurs.
- `0x0045B422` compares current status at `0x005116E0+offset` with **exactly 1**.
  Other statuses, including nonzero values, skip the ID read entirely.
- `0x0045B42B` compares the registered dword ID at `0x005113C0+offset` with the
  argument. `0x0045B433` stores zero to status only on equality. The loop continues
  after a match, so **every matching active duplicate** is cleared.
- `0x0045B448` returns **1 whenever enabled**, including empty tables, no match
  and repeated release. This is not a count or a found/not-found result.

Zero, high-bit and all-one IDs are ordinary raw equality values; no signed-ID
guard or interpretation exists. Class flags are ignored. There is no callback,
parameter/context access, ID-pool recycling, cursor update or metadata clearing.
All matching IDs and callback words remain stored after status is cleared.
No bug fix, behavior deviation or extra guard is introduced.

The independently verified flag is file-initialized zero; the status/registered-ID
tables are separate 200-dword arrays in the loader-zeroed virtual `.data` tail.
Their widths/extent come from original scaled dword operations and the bounded
loop; no aggregate packing is inferred. See
[the bookkeeping data map](windows_handle_bookkeeping.md). The C uses explicit
32-bit unsigned words and volatile global accesses to preserve the observed read/
store sequence. `EXACT` describes this bounded behavior, not instruction equality.

## Caller and lifecycle implications

The authenticated FPO operand scan finds the direct call at `0x00456230`, inside
registered callback `[0x00456210,0x00456263)`. It loads the stored ID from
`0x0050E680`, pushes it at `0x0045622A`, calls release, and restores four stack
bytes at `0x00456235`; the release result is not checked. The PE inspector asserts
this direct call independently of Ghidra names or pseudocode.

`Mem_ShutdownHandles` clears the initialization flag before invoking its callbacks.
Thus this ID-release call returns 0 without table effects during ordinary shutdown
callbacks while the flag remains zero. Standalone release while enabled clears all
matching statuses, so later shutdown skips those cleared entries. These behaviors
are preserved rather than repaired into a different lifecycle.

The callback's next verified dependency is **VA `0x00456470` / RVA `0x00056470`**,
directly called at `0x00456244` with a pushed dword. Its full contract and memory
ownership remain unverified. Recover it before reconstructing the callback body.
Related callback-bearing memory release at `0x0045B490` also remains unreconstructed;
its status/ID tests and indirect call are independently observed, not validated.
No neighboring candidate's stage is promoted by this feature.

Live Ghidra bounded disassembly corroborates the release routine. The repository
bridge still lacks a selected-program fingerprint endpoint, so loaded-program
identity remains unverified. Authentic PE hash/bytes, FPO, initial data, instruction
operands and direct call targets supply the authoritative evidence. No program
mutation, bulk synchronization, DOS assumptions or original-asset edits occurred.

## Validation scope and reproduction

The C89 implementation assumes ordinary Windows x86 caller-cleanup ABI and
readable/writable nonaliasing globals in single-threaded execution. Compile-time
type-width checks remain in the module. Concurrent mutation, faulting addresses,
native linked-global layout and compiler-managed stack-read timing are outside
the validated contract. EAX/ESP, caller argument/stack, saved registers, clear DF,
global read/store order and complete state effects are checked. Caller-scratch
register/instruction/temporary-stack equality is not claimed.

Run from the repository root in PowerShell:

```powershell
uv run python tools/decomp_doctor.py --probe-ghidra
uv run tools/windows_inspect.py
uv run python tools/build_decomp.py
uv run tools/verify_matching.py
uv run python tools/verify_fidelity.py
uv run python -m unittest discover -s tests -p test_decomp_doctor.py
uv run python -m unittest discover -s tests -p test_windows_tracking.py
uv run python tools/db.py update
uv run python tools/db.py update --check
git diff --check
```

Inspection/emulation use pinned PEP 723 environments; invoke those scripts without
inserting `python`. Strict C89 x86 Clang/LLD 19.1.1 compilation links the focused,
dependency-free PE32 DLL exporting five routines; `.text` is 785 bytes. The original
compiler identity is still unknown. Raw comparisons differ; the release diagnostic
compares a 62-byte prefix, not an established compiled extent. Relocation-aware
instruction equality, linked matching and native game behavior are unverified.

**418 release differential comparisons pass**: 200 single-match every-slot cases;
150 flag/ID/table-pattern combinations; 64 seeded random states; and four lifecycle
release checks. Flags include 0/1/2/0x80000000/0xFFFFFFFF; arguments include
0/1/0x7FFFFFFF/0x80000000/0xFFFFFFFF. Patterns cover inactive tables, active
nonmatches, 200 duplicate matches, mixed exact/nonexact statuses, matching inactive
IDs and simultaneous first/last-slot matches. Other fields contain seeded random
data, including callback words, to detect any unintended access or mutation.

Independent expectations derive from original CMP/JNZ/conditional-store/loop
instructions. The original is checked against these expectations, then compiled
C is independently executed in Unicorn 2.1.4. Checks include exact global reads,
ascending status writes, all twelve state regions, full image/prefix guards and
the ABI described above. Five-routine integration runs actual original/compiled
initializer and consumer, registers three duplicate ID=42 entries, checks enabled
no-match release, releases all duplicates, repeats release, runs shutdown with no
eligible entries, and checks disabled release. No callback is invoked or substituted
in this new integration sequence.

All existing 30 initializer, 377 consumer, 590 registration and 531 shutdown
comparisons remain, plus eight original-only negative-cursor checks. Shutdown's
callbacks remain explicit test models at their recovered ABI boundary; original
callback bodies are not validated. The four diagnostic and ten tracking unit tests,
provenance audit, synchronized exports and diff checks pass.

SQLite describes RVA `0x0005B410` as reconstructed with supported ABI/fidelity and
fresh compilation/emulation results. Existing RVAs `0x0005B1F0`, `0x0005B1B0`,
`0x0005B360` and `0x0005B240` receive fresh verifier provenance. Five routines cover
471 original bytes. Obtain the synchronized snapshot ID with `db.py status`;
JSON/registry/SQL/dashboards are generated from that store. Original binaries/assets
and unrelated candidate records remain unchanged.
