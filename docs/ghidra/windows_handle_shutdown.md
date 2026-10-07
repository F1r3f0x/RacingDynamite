# Windows handle shutdown (2026-10-07)

Bounded reconstruction: `Mem_ShutdownHandles` in `decomp/src/mem.c`.
The name and module association are semantic hypotheses; original symbols/source
filename remain unknown. All consequential binary evidence comes from authentic
`Ignition/Ignition/IGN_WIN.EXE`, 915,968 bytes, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
Preferred base is `0x00400000`; PE32 x86 CRT entry RVA/VA is
`0x00069950` / `0x00469950`, distinct from application entry `0x004120A0`.

## Independently recovered extent and contract

VA `0x0045B240`, RVA `0x0005B240`, extent `[0x0045B240,0x0045B2DB)`:
155 bytes / 41 instructions, followed by five INT3 bytes. FPO metadata, complete
control flow and padding agree. Routine SHA-256:
`edf69cfb1fd2d860913575999d39e05e37e615c11fb1e4280e4c166836e3b59f`.
No arguments; ordinary RET, EAX=1 on every returning path. ESI/EDI are saved;
EBX/EBP are untouched. Callback callees must honor ordinary x86 nonvolatile
register, stack and direction-flag requirements.

The flag at `0x004BAB38` is read once on entry. Zero returns 1 without reading
tables or writing globals. Every nonzero value enables shutdown. At `0x0045B25A`,
the routine stores zero to the flag before any table read or callback invocation.
It does not reset the flag again or recheck it during either pass.

There are two complete ascending scans of the 200-dword status table at
`0x005116E0`. ESI is a byte offset, reset to zero for each pass, incremented by 4,
and compared with `0x320` using signed JL. Only offsets 0..796 are accessed.
The first pass accepts flags **exactly `0x10000`**, the second accepts flags
**exactly `0x20000`**, from the array at `0x00511D20`. These are equality tests,
not bit masks. Each slot is eligible only if its current status is **exactly 1**;
other status words, including nonzero values, are skipped without a flag read.

| Operation | First-pass instruction VA | Second-pass instruction VA |
| --- | --- | --- |
| Read/test current status==1 | 0x0045B260 | 0x0045B29D |
| Read/test current flags==required class | 0x0045B269 | 0x0045B2A6 |
| Store selected status=0 | 0x0045B271 | 0x0045B2AE |
| Read parameter[index] at 0x00510F10+offset | 0x0045B27B | 0x0045B2B8 |
| Push the raw dword parameter | 0x0045B281 | 0x0045B2BE |
| Read/call callback[index] at 0x00511A00+offset | 0x0045B282 | 0x0045B2BF |
| Caller removes four argument bytes | 0x0045B288 | 0x0045B2C5 |

The initialized flag and selected status are already zero when a callback starts.
There is no null-pointer check and no result/error handling after the call; the
eventual routine result is always 1. The invocation ABI is one unsigned/raw dword
stack argument with caller cleanup. Callback EAX is ignored; the C callback type
uses `void` for the discarded result, without claiming every original callback
had that source return type. Contexts and registered IDs are not consulted.

Flags, parameters, callback words, ID pool, cursor, contexts and pending dispatch
words are not directly written by this routine. They may be changed by callbacks.
Tables are read live, without snapshots: callbacks can suppress or create later
eligible entries, change parameters, create entries for the second pass, or leave
entries active behind a completed scan. Restoring the initialized flag in a
callback survives shutdown because there is no final zero store. A callback
reentering shutdown while the flag remains zero receives 1 and performs no scans.

The relevant globals and their loader-zeroed `.data` extents are independently
verified by `windows_inspect.py`; see the
[bookkeeping data map](windows_handle_bookkeeping.md). No new aggregate layout or
global is inferred. The initialization flag is file-initialized zero; the arrays
are separate 200-dword stores in the virtual tail of `.data`.

## Caller, callback and next dependency

The authentic FPO instruction scan finds a direct call at `0x0045B1A0` to this
routine. It also confirms that caller `0x00456180` supplies callback address
`0x00456210` through the pending callback word before `Mem_RegisterHandle`.
The callback's FPO extent is `[0x00456210,0x00456263)`, 83 bytes. Its directly
decoded instructions check state at `0x004BA6C4`; the active path pushes the
stored ID at `0x0050E680` and calls **`0x0045B410` at `0x00456230`**, then performs
further cleanup through `0x00456470`. This supports the shutdown/lifecycle name.
It does not establish the complete callback's behavior or dependencies.

The next bounded dependency is ID-based release at **VA `0x0045B410` /
RVA `0x0005B410`**, FPO extent 62 bytes. Bookkeeping references show status==1
and registered-ID comparisons followed by clearing status. Recover its whole
contract, index boundary and return value independently before reconstruction.
No caller/callback/neighboring candidate is promoted by this feature.

Live Ghidra bounded disassembly corroborates the shutdown listing. The current
repository bridge still lacks a selected-program fingerprint endpoint; loaded
program identity is unverified. Direct authenticated PE reads establish all
consequential addresses, bytes, ABI observations, initializers and references.
No Ghidra program mutation, bulk symbol synchronization or DOS evidence was used.

## C implementation and memory assumptions

Readable C89 extends the verified memory module with explicit callback invocation.
Compile-time checks require 32-bit unsigned-int, data-pointer and function-pointer
widths, and 16-bit short. Raw callback-word/function-pointer conversion depends on
the Windows x86 flat-address ABI and is not a portable ISO C guarantee. Selected
callback words must identify valid functions accepting the one-dword invocation
ABI; there is no invented dependency, callback stub, null guard or behavior fix.

Volatile accesses keep the scans and dispatch words live across callback effects.
A volatile local parameter retains the original parameter-before-callback-word
read order. The first verifier run caught Clang folding that parameter load into
PUSH after the callback load when the local was ordinary storage. That real failed
result remains in tracking history; the corrected source passes fresh runs.
Extra compiler-managed local stack accesses are allowed, while caller stack,
saved registers, global effects and the observed global read/store order are
checked. `EXACT` means bounded behavioral fidelity, not original instruction or
stack-temporary equality. Callback fault paths, callbacks violating their ABI,
concurrent mutation/asynchronous observation and native layout parity are outside
the tested contract.

## Validation and reproducible commands

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

Inspection/emulation dependencies remain pinned PEP 723 environments; invoke those
scripts without inserting `python`. The strict C89, x86 Clang/LLD 19.1.1 build
links a dependency-free focused DLL exporting four routines; `.text` is 673 bytes.
Original compiler identity remains unknown. Raw byte comparisons differ; the
shutdown diagnostic compares a 155-byte prefix, not a verified compiled extent.
Relocation-aware instruction comparison and linked matching are not evaluated.

The verifier passes **531 shutdown differential executions**: 30 initialization/
table-pattern combinations, 400 every-slot/every-pass cases, 60 boundary eligibility
cases, 32 seeded random cases, eight callback-mutation/reentry plans, and one actual
initializer/consumer/registration/shutdown integration sequence. Flags include
0/1/2/0x80000000/0xFFFFFFFF. Statuses include 0/1/2/0x80000000/0xFFFFFFFF; class
tests include 0/0x10000/0x20000/0x30000/0x10001/0xFFFFFFFF. Integration callbacks
receive raw parameters 0, 0x80000000 and 0xFFFFFFFF.

The original and compiled dispatcher instructions execute in separate Unicorn
2.1.4 CPUs. At callback entry, explicit Python test models check the stack argument
and all twelve state regions, apply specified mutations, return with caller cleanup,
and clobber EAX/ECX/EDX. Reentry actually executes the original or compiled shutdown
routine on the same emulated CPU with the flag cleared. Hook-entry bytes are copied
from authenticated callback `0x00456210` only to provide mapped/decodable addresses;
the model redirects execution before any fixture instruction executes. There is
no handwritten assembly. **Original callback bodies are not executed or validated
by this model.** It is a test double at the recovered ABI boundary, never linked
into the reconstruction or substituted for game callbacks.

Independent expectations derive from original comparisons, loop bounds, writes
and indirect calls. Checks cover exact global data events, pass/callback order,
pre-callback lifecycle state, mutation visibility, status/flag equality, no index
200 access, full image/prefix guards, EAX=1, ESP/caller-stack balance, saved registers
and clear DF. Existing 30 initializer, 377 consumer and 590 registration comparisons,
plus eight original-only negative-cursor cases, are retained. Four diagnostic and
ten tracking unit tests pass, as do provenance, synchronized exports and diff checks.

SQLite describes RVA `0x0005B240` as reconstructed with supported ABI/fidelity and
fresh compilation/emulation provenance. Existing RVAs `0x0005B1F0`, `0x0005B1B0`
and `0x0005B360` receive fresh verifier results. Four routines cover 409 original
bytes; no native/instruction/linked match pass is asserted. Obtain the synchronized
snapshot ID with `uv run python tools/db.py status`. Original assets/binaries and
all unrelated candidate records remain unchanged.
