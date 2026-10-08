# Windows surface dispatch installation

Authentic input: `Ignition/Ignition/IGN_WIN.EXE`, 915,968 bytes, SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782,
PE32 i386, preferred base VA 0x00400000. Original assets are untouched.
Names and geputget.c placement are semantic; original symbols remain unknown.
No DOS binary supplied evidence. The user assumes this executable is active in
Ghidra; that assumption is distinct from the local fingerprint verification.

Read-only localhost:8080 `disassemble_function?address=0x0045B690` and
`xrefs_to` for each slot corroborated local PE analysis. `decompile_function`
provided bounded target evidence at VAs 0x0045C4B0/530/680/730; several other
entries are not defined as functions in the active Ghidra program. Their ABI
and extents were recovered from authenticated local instructions, FPO, wrappers
and returns, rather than treating absent decompilation as evidence.

## Body, storage and ABI

Gfx_InstallSurfaceDispatch is VA 0x0045B690 / RVA 0x5B690, 146 bytes,
FPO (146,0,0,0), SHA-256
3cb7a08c993e9d2ce4db93a2ebe96e9165728ef6bae5b5014371ebaef3de41bf.
Fourteen ten-byte MOVs store function addresses in increasing slot order,
followed by MOV EAX,1 and ordinary RET at VA 0x0045B721. There are no calls,
reads of prior slot contents, guards, rollback or allocation. Both address
operands of each store have HIGHLOW relocations (28 total).
Every slot is a four-byte loader-zeroed word in the .data virtual tail.
Repeated installation overwrites every slot. The initializer has no stack
arguments, preserves caller stack/nonvolatile registers and returns int 1.
No-argument RET alone cannot distinguish cdecl from stdcall; ordinary C is used.

The consumers below forward their arguments in original order and clean the
stated bytes. All return a 32-bit EAX status. The Open/Reset wrappers test the
surface result, return zero on failure, otherwise tail-jump to the corresponding
sprite slot. Close/Restore are no-argument tail jumps. No common callable
prototype or contiguous production aggregate is inferred.

| Slot VA | Target VA | Production slot suffix | Consumer VA | Stack contract |
| --- | --- | --- | --- | --- |
| 0x0050EB68 | 0x0045B730 | Configure | 0x00456B10 | four raw option words; caller cleans 16 |
| 0x0050EB6C | 0x0045B740 | Open | 0x00456BC0 | no arguments; EAX status tested |
| 0x0050EB70 | 0x0045BD70 | Close | 0x00456BE0 | no arguments; tail jump |
| 0x0050EB74 | 0x0045C060 | Reset | 0x00456BF0 | no arguments; EAX status tested |
| 0x0050EB78 | 0x0045C150 | ConfigureSurface | 0x00456C60 | five raw option words; caller cleans 20 |
| 0x0050EB7C | 0x0045C160 | Blit | 0x00456B30 | source record, x,y,width,height,destination record,destination y,x; cleans 32 |
| 0x0050EB80 | 0x0045C1D0 | CopyPixels | 0x00456B70 | pixel pointer,stride,source x,y,width,height,destination record,destination x,y; cleans 36 |
| 0x0050EB84 | 0x0045C3D0 | Clear | 0x00456BB0 | record pointer; cleans 4 |
| 0x0050EB88 | 0x0045C4B0 | Present | 0x00456C90 | record pointer; cleans 4 |
| 0x0050EB8C | 0x0045C520 | Reserved | none found | target FPO has zero argument words; returns 2 |
| 0x0050EB90 | 0x0045C530 | Lock | 0x00456C10 | record pointer, mode; cleans 8 |
| 0x0050EB94 | 0x0045C680 | Unlock | 0x00456C30 | record pointer; cleans 4 |
| 0x0050EB98 | 0x0045C6D0 | SetPalette | 0x00456C40 | RGB byte pointer; cleans 4 |
| 0x0050EB9C | 0x0045C730 | Restore | 0x00456C50 | no arguments; tail jump |

Configure/ConfigureSurface/Reserved targets each consist of MOV EAX,2; RET.
Unused option semantics and signedness cannot be recovered from these stubs;
unsigned int preserves their raw 32-bit ABI without claiming semantics.
Reserved has no callable consumer in Ghidra xrefs or a complete bounded FPO
instruction scan. This is a stated gap, not an invented consumer or whole-program
absence proof. The production typedef uses its zero-argument target contract.

Pointer-versus-scalar recovery is independent of guessed decompiler types:
Blit dereferences arguments one/six as records, forms a source rectangle from
arguments two through five, and forwards destination x/y in reversed stack
order to the vtable call at VA 0x0045C1B4. CopyPixels computes the byte source
as pixels + stride * source_y + source_x, and destination as record pixel base
+ record stride * destination_y + destination_x. Signed IDIV and signed height
branches support int geometry; broader invalid geometry behavior is unverified.
Clear/Present/Lock/Unlock dereference their record argument. Lock compares its
mode to 1/2/3. SetPalette reads 256 RGB triplets from its argument. Production
records remain opaque: observed offsets do not certify a complete structure.

CopyPixels has no FPO entry. Its verified entry comes from the relocated
initializer literal and the nine-word consumer. Complete control-flow decoding
covers 497 bytes, ends with ordinary RET at VA 0x0045C3C0, and is followed by
15 INT3 bytes before the next authenticated FPO entry at VA 0x0045C3D0.
SHA-256 ab0e1a366b4876c8807819b388b91435c556b4546a92764e400e2eee3ecd3c39.
It is added as an analyzed candidate, increasing inventory from 1,027 to 1,028.
All fourteen targets and thirteen consumers are analyzed only; no production
target bodies or passing target compilation/emulation records are claimed.
Their complete hashes/extents and consumer FPO/cleanup contracts are independently
asserted by `tools/verify_mem_lifecycle.py`.

## Production integration and validation

Native geputget.c now owns fourteen separately typed volatile, zero-initialized
globals and the real initializer body. Public prototypes identify the unresolved
target entries. The build extracts these exact declarations/globals/body into
the same production translation unit as Gfx_SelectBackend. Nonreturning target
fixtures are defined in a separate validation translation unit; they cannot
be optimized into or mistaken for production implementations. They are never
entered by the initializer. The remaining 80 bytes of sprite slots and holes
are a validation-only array; production surface words no longer use that array.

Clang/LLD 19.1.1, i686-pc-windows-msvc, strict C89, -O2, freestanding/no-builtin,
disabled inlining/unrolling/vectorization/SSE remain provisional validation flags.
Fresh import-free focused DLL compilation passes. This is not a playable rebuild.

266 standalone original-versus-production installer calls cover representative
and random initial words, all-zero initialization and 133 repeated calls without
reset. The independent oracle checks exactly fourteen ordered stores, EAX=1,
all tracked state, stack/nonvolatile registers/DF and unrelated image bytes.
Only the fourteen verified target identities are normalized between original
and rebuilt pointer values; order and destination identities remain significant.
This normalization validates behavior, not instruction equality.

532 standalone selector calls include 266 nonzero calls without effects and
266 zero calls executing the real initializer before the modeled sprite boundary.
That boundary observes all installed pointers and can mutate a surface slot,
sprite/hole word and input callback; repeat selection reinstalls the surface
slot and retains the subsequent mutation. The real initializer also executes
through 18 integrated Mem_InitSystem startups, four persistent. Retained counts:
306 banner / 266 input reset / 112 startup / 519 shutdown comparisons, 94/74
isolated wrapper contracts and ten persistent lifecycle calls.

39 original-only consumer tests run the thirteen original forwarding wrappers
after original installation. Entry sinks compare all forwarded argument words
in order, return zero, clobber volatile registers, and verify caller cleanup,
nonvolatile registers and caller stack. Zero deliberately avoids the secondary
sprite tail call in Open/Reset. These tests establish forwarding ABI only;
neither the original graphics targets nor C target fixtures execute.

Raw-prefix diagnostics differ; relocation-aware instruction equality and original
compiler/link layout remain unverified. Primitive and sprite initialization,
CRT printf/heap and handle callbacks remain modeled. Native graphics/input/heap/
game parity, full record layout, invalid/aliased storage, concurrency and general
reentry remain unverified. EXACT describes the recovered installer behavior.
No behavioral deviation or bug fix is introduced.

PowerShell commands use `$env:UV_CACHE_DIR='build/uv-cache'` and
`$env:UV_OFFLINE='1'`. Run `uv run python tools/verify_mem_lifecycle.py` for the
focused contract; `uv run python tools/workflow.py complete --rva 0x5B690
--rva 0x56AF0 --rva 0x5B170 --rva 0x5B1A0 --rva 0x55AB0 --rva 0x5B4F0
--limitation "Primitive and sprite initializer, CRT printf/heap and callbacks
modeled; instruction equality and native graphics/game parity unverified"`
runs the real full verifier, fidelity audit and export/check sequence.
Current recorded input hashes are checked against actual LF working bytes and
Git index bytes before committing; generated exports come from SQLite.

Next bounded candidate: Gfx_InstallSpriteDispatch, VA 0x00456E60 / RVA 0x56E60,
181 bytes. Recover each consumer ABI and the three downstream initializer
contracts independently before replacing that model.

## Production sprite installer follow-up (2026-10-08)

[Sprite dispatch installation](windows_gfx_sprite_dispatch.md), RVA 0x56E60,
now owns sixteen independently typed production globals and executes through
both the real selector and 18 integrated startups, preserving the real surface
installer. Fresh coverage: 306 standalone sprite installers, 532 selectors,
266 standalone surface installers, 48/39 original-only sprite/surface consumer
ABI checks, 43 original-only downstream initializer contract cases. Existing
306 banner / 266 reset / 112 startup / 519 shutdown comparisons and ten persistent
lifecycle calls remain. Three downstream sprite initializers and primitive
initialization, CRT printf/heap and callbacks are explicitly modeled. Strict C89
focused compilation passes; raw prefixes differ. Instruction equality and native
graphics/game parity are unverified. Earlier model descriptions are historical.
