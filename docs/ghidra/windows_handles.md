# Windows handle-ID consumer (2026-10-07)

Scope: the second bounded memory reconstruction, integrated with the verified
initializer. All consequential observations below come from the authenticated
`Ignition/Ignition/IGN_WIN.EXE`: 915,968 bytes, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
Preferred base is `0x00400000`; CRT entry RVA `0x00069950` / VA `0x00469950`
is distinct from the independently verified application entry `0x004120A0`.

## Routine and state evidence

`Mem_NextHandleId` is a semantic reconstruction name, not an original symbol.
Original source filename remains unknown; association with `mem.c` is inferred.
VA `0x0045B1B0`, RVA `0x0005B1B0`, extent `[0x0045B1B0,0x0045B1EA)`:
58 bytes / 15 instructions, followed by six INT3 bytes. Embedded FPO extent,
control flow and padding agree. Routine SHA-256:
`696ce4a075f495b91bd79ce9fe531b4c474b66540d3935dcc8e5a45c74d6d139`.
No arguments or calls; ordinary x86 RET; signed int result in EAX. ECX/EDX
are scratch; EBX/ESI/EDI/EBP and caller stack balance are preserved.

The flag is a 32-bit word at `0x004BAB38`, file-initialized zero. The signed
16-bit cursor at `0x00512040` and 200 signed 16-bit IDs at `0x00511230`
are in the loader-zeroed virtual tail of `.data`. These are independent globals,
not evidence for a packed aggregate. The original operations are:

- CMP flag,0 / JNZ: zero returns -1 without reading the cursor or ID table;
  **any nonzero flag** enables access. The initializer instead skips only flag=1.
- MOV AX,cursor / MOVSX ECX,AX; LEA EDX,[ECX+1]; CMP EDX,200 / signed JGE:
  cursor 199 and larger return -1 without a store or ID read.
- INC AX / 16-bit store cursor, then MOVSX EAX,[ECX*2+0x00511230].
  Index 198 is consumed and advances to 199; index 199 remains unused during
  normal initialized use. A returned negative ID still consumes the slot.

`tools/windows_inspect.py` independently scans all file-backed FPO code operands
and absolute HIGHLOW relocations in `.text`. Cursor references occur at
`0x0045B1BF` (read), `0x0045B1D5` (write) and `0x0045B232` (initializer reset).
ID read is `0x0045B1DB`; initializer store is `0x0045B21D`, with displacement
`0x0051122E` and loop index starting at 1. The five relocation **operand** VAs
are `0x0045B1C1`, `0x0045B1D7`, `0x0045B1DF`, `0x0045B221`, `0x0045B235`.
No other direct cursor/ID references are found by these scans. They do not prove
absence of indirect aliases, externally corrupted state, or non-FPO code calls.

The FPO scan and bounded Ghidra xrefs find one direct caller: `0x004561A0`
in `[0x00456180,0x00456206)` (134 bytes). It pushes EAX at `0x004561A5`,
stores it at `0x0050E680` at `0x004561A6`, initializes bookkeeping dispatch data,
and calls `0x0045B360` at `0x004561C5`, then restores ESP by four bytes.
Font naming remains a hypothesis. The downstream 120-byte routine at
`0x0045B360` scans the 200-dword status table and records its argument with
related bookkeeping arrays. It is inspected, not reconstructed or validated.
It is the next verified dependency for this caller.

The live localhost bridge is reachable. Its current repository wrapper exposes
bounded disassembly and xrefs but no selected-program fingerprint endpoint.
Ghidra corroborates the consumer, caller and cursor/ID xrefs; it is not treated
as authenticated identity evidence. No project mutation or symbol sync was used.

## Signed cursor and memory contract

There is no lower-bound check. Initializer reset followed by this consumer alone
maintains cursor 0..199, but the scans do not establish a whole-program invariant.
Negative cursors read a signed word at `ID_base + 2 * signed_cursor`, after the
cursor is incremented. At -32768, the original address is `0x00501230`;
at -1/-2 it is `0x0051122E` / `0x0051122C`. All are inside the original mapped
`.data`, but their semantic ownership is not reconstructed here.

The C89 implementation uses a verified 32-bit pointer/unsigned-int width and
modular integer address arithmetic, then a signed-short dereference. This avoids
negative array indexing and adds no guard. Integer/pointer conversion relies on
the x86 Windows flat-address implementation; it is not portable ISO C. Enabled
negative state requires readable signed-word storage at that effective address.
Unmapped/faulting addresses, concurrent mutation and asynchronous observation are
outside the validated contract. `@fidelity EXACT` describes this bounded behavior,
not exact code generation or original linked-global layout.

The validation DLL relocates globals and does **not** reproduce their original
relative positions. In its current layout, negative index -1 aliases its cursor
and -2 aliases part of its flag. The harness must not plant independent sentinels
there and pretend layouts agree. It therefore compares -4 and -32768 against C
with nonaliasing mapped sentinel words; -1/-2 are original-only checks. Native
negative-state layout parity remains unverified. No surrounding structure or
padding has been invented to hide that limitation.

## Reproduction and results

From repository root in PowerShell:

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

PE analysis/emulation dependencies remain pinned PEP 723 environments; invoke
those two scripts as `uv run <script>`. Strict C89 Clang/LLD 19.1.1 compilation
and PE32 x86 DLL linking pass with the existing provisional flags and the new
`Mem_NextHandleId` export. Only `decomp/src/mem.c` is compiled; no new dependency
stub, DOS build, playable executable or native game integration is introduced.
Generated `.text` is 289 bytes. The original compiler/version remains unknown.

The fresh harness executes original and compiled instructions in separate
Unicorn 2.1.4 x86 CPUs. All 30 existing initializer comparisons remain intact.
377 consumer comparisons pass: flags 0/1/2/0xFFFFFFFF/0x80000000; cursors
0/198/199/200/32767/-4/-32768; IDs 0/1/32767/-1/-32768; 175 state combinations,
then initializer output followed by 202 successive consumption/exhaustion checks.
Eight additional original-only cases cover cursors -1/-2 with signed words
0/32767/-1/-32768. Full state equality, unchanged flag/tables, exact cursor writes,
signed EAX, actual effective word-read addresses, image/prefix write bounds,
nonvolatile registers, ESP balance and clear DF are checked. Original expectations
derive from its decoded MOVSX/JGE/INC instructions, not the new C.

The four diagnostic tests and ten tracking tests pass, as do modified-tool Python
syntax checks, the active provenance/freshness audit and generated-output drift
check. Tracking reports two reconstructed routines (134 original bytes), two
current compilation passes and two current emulation passes. The generated
snapshot is `4049f74485c61cdcb1bed0c3246862d685c2ee74b6c7545d2bd20d36e6c1506d`.
No other candidate's reconstruction status is promoted by this feature.

Raw code comparisons differ. The consumer diagnostic compares a 58-byte compiled
prefix against the original, not a verified compiled extent. Relocation-aware
instruction equality and linked matching are not evaluated or claimed. Emulator
execution is independent behavior evidence; native routine/game execution,
language selector, menu, sound and gameplay remain unverified. Original binaries
and assets are untouched; this feature adds no native runtime claim to the earlier
disposable launch probe.
