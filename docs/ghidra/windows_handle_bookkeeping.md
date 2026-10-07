# Windows handle bookkeeping (2026-10-07)

This bounded feature reconstructs `Mem_RegisterHandle` in the verified memory
module. The name and `mem.c` association are semantic hypotheses; neither is a
recovered original symbol/source filename. Evidence comes directly from authentic
`Ignition/Ignition/IGN_WIN.EXE`: 915,968 bytes, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
PE32 x86 preferred base is `0x00400000`; CRT entry RVA/VA is
`0x00069950` / `0x00469950`, distinct from application entry `0x004120A0`.

## Extent, ABI and recovered contract

VA `0x0045B360`, RVA `0x0005B360`; extent `[0x0045B360,0x0045B3D8)`:
120 bytes, 27 decoded instructions, followed by eight INT3 bytes. Embedded FPO,
complete control flow and padding agree. Routine SHA-256:
`2dc672ad67179fa73bca4d901a607b72986ba7138a1d00ecb954b3b9280e5e34`.
There are no calls/import dependencies. One dword argument is read at ESP+4;
ordinary RET leaves argument cleanup to the caller. EAX is the signed result;
ECX/EDX are scratch. EBX/ESI/EDI/EBP and caller stack remain unchanged.

- CMP initialization flag,0 / JNZ: zero returns **0** immediately without reading
  status, dispatch words or argument. Every nonzero flag enables registration.
- EAX/ECX start at zero. The CMP EAX,200 / JE exhaustion check precedes each
  status-table read. Nonzero status advances EAX; the signed JLE backedge reaches
  that check at 200. The first zero status from index 0 through **199** is selected.
  A full table returns **-1**, without reading pending dispatch words or writing.
  No index-200 read/write occurs. This differs from the consumer's 199-ID limit.
- Success returns the selected zero-based index. The argument is copied as an
  uninterpreted 32-bit word: zero, negative signed representations and duplicate
  values are accepted. No ID lookup, validity check or duplicate detection exists.
  Disabled return 0 is therefore indistinguishable from successful slot 0 by EAX.

Successful operations occur in this order (all reads/stores are dwords):

| Instruction VA | Operation |
| --- | --- |
| 0x0045B38F | Read pending context at 0x0063C690 |
| 0x0045B395 | Store status[index] = 1 |
| 0x0045B39F | Store flags[index] = 0x00010000 |
| 0x0045B3A9 | Store contexts[index] = previously read context |
| 0x0045B3AF | Read argument from ESP+4 |
| 0x0045B3B3 | Store registered IDs[index] = argument |
| 0x0045B3B9 | Read pending parameter at 0x0063C698 |
| 0x0045B3BF | Store parameters[index] = pending parameter |
| 0x0045B3C5 | Read pending callback at 0x0063C694 |
| 0x0045B3CB | Store callbacks[index] = pending callback |

The cursor, ID pool, initialization flag and pending words are never written.
No callback is invoked and no pointed-to memory is accessed by this routine.
All other slots and bytes remain unchanged. No behavior repair or deviation is
introduced; `EXACT` describes the bounded behavior, not instruction equality.

## Independent data and caller evidence

These are separate globals, not a verified aggregate or packing scheme:

| Reconstruction name | Preferred VA | RVA | Bytes |
| --- | --- | --- | --- |
| g_memHandlesInitialized | 0x004BAB38 | 0x000BAB38 | 4 |
| g_memHandleContexts | 0x00510BF0 | 0x00110BF0 | 800 |
| g_memHandleParameters | 0x00510F10 | 0x00110F10 | 800 |
| g_memRegisteredHandleIds | 0x005113C0 | 0x001113C0 | 800 |
| g_memHandleStatus | 0x005116E0 | 0x001116E0 | 800 |
| g_memHandleCallbacks | 0x00511A00 | 0x00111A00 | 800 |
| g_memHandleFlags | 0x00511D20 | 0x00111D20 | 800 |
| g_memPendingContext | 0x0063C690 | 0x0023C690 | 4 |
| g_memPendingCallback | 0x0063C694 | 0x0023C694 | 4 |
| g_memPendingParameter | 0x0063C698 | 0x0023C698 | 4 |

The flag is file-initialized zero. All other listed storage lies in the
loader-zeroed virtual tail of `.data`, beyond its file-backed raw extent. Array
width/200-element extent comes from scaled dword accesses and the bounded scan,
not DOS layout. The verified initializer clears only status; the five other
arrays and pending words retain their existing contents on initializer calls.
The C uses raw `unsigned int` words rather than guessed pointer/callback types.
Names describe transfers; ownership and callback signatures remain unresolved.

The independent FPO operand scan in `windows_inspect.py` finds the direct call at
`0x004561C5` in `[0x00456180,0x00456206)`. At `0x004561A0` the caller consumes an
ID; `0x004561A5` pushes EAX and `0x004561A6` stores it at `0x0050E680`.
Stores at `0x004561AB/B5/BF` set pending context=`0x004BA6C8`,
callback=`0x00456210`, parameter=0. After registration, `0x004561CA` adds four
to ESP; the caller does not inspect the registration result. That call path is
evidence for ABI and dispatch transfers, not a complete caller reconstruction.

The same authenticated instruction scan finds related registration at
`0x0045B2E0` using flags `0x20000`, status release at `0x0045B3E0`, ID-related
release at `0x0045B410`, and indirect callback reads/calls in `0x0045B240`,
`0x0045B450` and `0x0045B490`. These references corroborate semantic names but
do not establish whole-program invariants or their full contracts. The next
verified dependency is **`0x0045B240`** (RVA `0x0005B240`): instructions at
`0x0045B260/269/271` test status/flags and clear status; `0x0045B27B/282` load
the parameter and call the stored callback. Recover its complete dispatch,
argument ABI and lifecycle contract before implementing it. No neighboring
candidate is promoted by this feature.

Live Ghidra corroborated the bounded disassembly using the repository bridge's
`disassemble_function` GET endpoint. The bridge has no selected-program
fingerprint endpoint; loaded-program identity remains unverified. All consequential
bytes, extents, addresses, initial data and caller operations were checked against
the independently authenticated PE. No program mutation or bulk sync occurred.

## C memory contract and validation limits

Strict C89 targets 32-bit Windows x86; compile-time checks enforce unsigned-int,
short and pointer widths. Bookkeeping globals must be readable/writable, mapped,
nonaliasing storage in single-threaded execution. Volatile accesses preserve the
observed global-read/store ordering, including loading context before status is
marked active. This is not a thread-safety mechanism. Asynchronous observation,
concurrent mutation, faults and access timing relative to compiler-managed stack
loads are outside the validated contract. The argument is an ordinary ABI dword;
the harness checks its value, caller cleanup and unchanged caller stack, not the
compiler's timing of loading that stack word.

The focused validation DLL exports all three routines and their explicit globals;
it does not reproduce original global addresses or relative layout. The existing
consumer's signed negative-address contract is preserved; its eight -1/-2 cursor
checks remain original-only. No aggregate, guessed padding or dependency stub is
introduced. Original compiler identity remains unknown; Clang/LLD 19.1.1 are
provisional validation tools. Only `decomp/src/mem.c` is compiled. Native linked
layout, native DLL execution, game startup, presentation and gameplay are unverified.

## Reproduction and fresh results

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

PE inspection and emulation use pinned PEP 723 dependencies; use `uv run <script>`
for those scripts. The build retains strict C89/x86 flags and dependency-free PE32
DLL linking. The verifier always rebuilds and persists separate compilation,
raw-byte diagnostics and emulation records with input/artifact/routine hashes.
The generated DLL `.text` is 481 bytes. Raw comparisons differ; the registration
diagnostic compares a 120-byte prefix, not an established compiled extent.
Relocation-aware instruction equality and linked matching are not evaluated.

Fresh Unicorn 2.1.4 execution passes **590** registration comparisons:
200 every-slot cases (later free slots test first-zero selection), 125 flag/boundary/
argument combinations, 64 seeded random multi-hole states, and 201 integrated
initializer/consumer/registration iterations. Flags include 0/1/2/0x80000000/
0xFFFFFFFF; boundary slots include 0/1/198/199 and exhausted tables; nonzero
statuses include 1/2/0x80000000/0xFFFFFFFF. Argument and dispatch tests include
zero, 1, 0x7FFFFFFF, 0x80000000 and 0xFFFFFFFF. The integration consumes 199 IDs,
then registers the consumer's -1 representation in slot 199; the next registration
fails. This preserves the two routines' different exhaustion limits.

Each original execution is checked against expectations independently derived
from its instructions, then against compiled-C execution. Checks include signed
EAX, ESP/RET balance, argument/caller-stack preservation, saved registers, clear
DF, all twelve state regions, full image/prefix guards, exact global read sequence,
values and six-store order. Disabled/full states perform no dispatch reads/stores.
All 30 initializer and 377 consumer differential cases plus eight original-only
negative-cursor cases are retained. The four diagnostic and ten tracking tests,
provenance audit, synchronized export check and whitespace check pass.

The authoritative SQLite candidate at RVA `0x0005B360` is described and marked
reconstructed, with current compilation/emulation results. Existing RVAs
`0x0005B1F0` and `0x0005B1B0` receive fresh verifier results. Generated inventory,
registry, SQL dump and dashboards share the snapshot ID available from
`uv run python tools/db.py status`; snapshot identity is not embedded here to avoid
a self-referential generated-state dependency. Three routines / 254 original bytes
are reconstructed; native and instruction-match passes remain zero. Original
binaries/assets and all other candidates' stage labels are preserved.
