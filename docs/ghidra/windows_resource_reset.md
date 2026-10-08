# Windows input callback reset (formerly Gfx_ResetResourceFlags)

## Provenance and ownership

Authentic Ignition/Ignition/IGN_WIN.EXE: PE32 i386, preferred base VA
0x00400000, 915,968 bytes, SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782.
No DOS executable supplied evidence. Originals remain untouched.

Input_ResetCallbacks is a recovered semantic name, replacing the provisional
Gfx_ResetResourceFlags label. Original symbols/source names are unknown.
Production placement in geputget.c follows its existing input subsystem; neither
word belongs to graphics resources or Font_InitSystem's flag at VA 0x004BA6C4.

VA 0x00455AB0 / RVA 0x55AB0 has complete FPO extent 13 bytes, tuple
(13,0 local dwords,0 argument dwords,0 bits), ending at VA 0x00455ABD.
Three INT3 bytes precede the next function. Complete body hash:
c301c37754413d115bac22b922a9ed480b4080011c559a4485b7ea044270fd59.
Four instructions zero EAX, store that dword to VA 0x0050DE14, store it to
VA 0x0050E678, and RET. HIGHLOW operand relocations are RVAs 0x55AB3 and
0x55AB8, targeting those exact words in that order. There are no calls, tests,
registrations or ownership transfers. Both words lie beyond .data's file-backed
raw extent within its virtual extent and therefore start loader-zeroed; adjacent
keyboard state, timer identifiers, font state and handle IDs are untouched.

The only direct reset call in the bounded FPO scan is VA 0x0045B17F in
Mem_InitSystem. Its return is ignored and later calls overwrite EAX. No arguments,
plain RET, preserved nonvolatile registers and caller stack; ordinary x86 C ABI
is used. No-argument RET does not alone distinguish cdecl from stdcall. The
production prototype is void: the original incidental EAX=0 is not a consumed
return contract. Differential checks intentionally exclude EAX for this void
routine, while both int lifecycle wrappers still require EAX=1.

## Independently recovered globals and consumers

| Word VA / RVA | Semantic production type and name | Consumer evidence |
| --- | --- | --- |
| 0x0050DE14 / 0x10DE14 | volatile pointer to void(int pressed,int scan_code), g_inputKeyEventCallback | VA 0x004560C0 loads the pointer, skips null, forwards its two stack dwords and cleans eight bytes after indirect CALL |
| 0x0050E678 / 0x10E678 | volatile pointer to void(void), g_inputPollCallback | VA 0x00456165 loads it after keyboard polling; null skips, nonnull CALL passes no arguments |

These are 32-bit function pointers under the validation target, not integers,
booleans, handles or initialized flags. Callback results are discarded; void is
the conservative callable interface. The first callback uses caller cleanup
(cdecl); the second has no arguments so cleanup convention is indistinguishable.
Volatile pointer objects preserve the verified store order in provisional codegen.
No surrounding struct layout is asserted merely from adjacency.

VA 0x00455EF0 / RVA 0x55EF0 (376 bytes) processes buffered device records:
16-byte records carry a scan-code low byte and a data bit 0x80. The press path
calls VA 0x004560C0 at VA 0x00456003 with (1,scan_code); the release path at
VA 0x0045603D passes (0,scan_code). Both clean eight bytes. The press path also
calls the character-ring helper VA 0x004560E0. This establishes keyboard-event
ownership and argument meaning independently of DOS state/names.

VA 0x00455F48 compares a moving pointer with immediate 0x0050E678. It ends
iteration over 256 dword repeat timers at VA 0x0050E278; it neither reads nor
invokes the callback word. Treating this DATA xref as an access would be wrong.

VA 0x00456160 / RVA 0x56160 (19 bytes) calls keyboard polling VA 0x00455EF0,
then conditionally invokes g_inputPollCallback with no pushes, then RET 0x14.
FPO records five argument dwords for this outer timer callback, not for the
nested callback. VA 0x00455C60 / RVA 0x55C60 (436 bytes) pushes VA 0x00456160
at VA 0x00455DE2 and calls timeSetEvent through IAT VA 0x0064C474 with five
arguments. The authentic import is WINMM.dll!timeSetEvent. Its surrounding
initialization uses DirectInputCreateA and keyboard state/repeat tables.
The timer wrapper itself uses callee cleanup (stdcall); that must not be
transferred to the no-argument client callback.

Consumer hashes independently asserted by the verifier:

| RVA | Bytes | SHA-256 |
| --- | --- | --- |
| 0x560C0 | 26 | 3b23ff815d34e56903eb4477a3cb227730eae66e254731b7902b62fed7d10a8b |
| 0x56160 | 19 | 2d1dd1ad615325f87eae45141b3a1d7f53a09f9d2646d1adcd17f93e16c76f58 |
| 0x55EF0 | 376 | ac1bfc5738cf5a68eef9491b90049eb23453052f9350d545c82586248ec92f33 |
| 0x55C60 | 436 | 6ade6c95c919cb9ea6d550e552623c057dbefcdef7b325d552d883a7ad782efe |

Live localhost:8080 Ghidra disassembly and xrefs corroborate local bytes.
Authentic IGN_WIN.EXE being active is the user's operating assumption; no
loaded-program identity check is claimed. Ghidra has no function at 0x00456165
and its polling decompiler marks reachable event paths unreachable. Local
complete FPO extents/instructions recover those paths and the outer timer entry;
no decompiler output is committed and no Ghidra program was modified.
Ghidra xrefs list only reset writes and the above reads/end marker for the words.
A local HIGHLOW/FPO operand scan corroborates that bounded reference inventory;
no setter is established and arbitrary indirect writes remain possible.

## Build and differential execution

PowerShell, repository root, Python through uv:

~~~powershell
$env:UV_CACHE_DIR = Join-Path (Get-Location) 'build/uv-cache'
$env:UV_OFFLINE = 1
uv run python tools/verify_mem_lifecycle.py
uv run python tools/workflow.py complete --rva 0x55AB0 --rva 0x5B170 --rva 0x5B1A0 --limitation "Three startup dependencies, CRT heap and handle callbacks modeled; instruction equality and native input/graphics/heap/game parity unverified"
~~~

The focused builder compiles complete production mem.c and extracts the exact
callback typedefs, global definitions and reset body from geputget.h/geputget.c.
Shared font/file builders consume the same real body; no reset fixture remains.
Complete geputget.c compilation remains blocked by unrelated legacy dependencies.
Clang/LLD 19.1.1, i686-pc-windows-msvc, strict C89, -O2, freestanding/no-builtin,
no inlining/unrolling/vectorization/SSE remain a provisional behavior compiler.
Original compiler/link layout is unknown. Output is an import-free focused PE32
validation DLL, not a playable game or native-runtime test.

266 standalone differential invocations include representative/high-bit/-1 and
128 seeded random raw pointer words, followed by repeated clearing without
resetting the emulated state. Exact two writes in order, no callback invocation,
full tracked state, unchanged unrelated image bytes, caller stack and nonvolatile
registers are compared against an independent instruction-derived oracle.
112 startup / 519 shutdown comparisons and ten persistent lifecycle calls remain.
The real reset executes through 18 integrated startups (14 fresh and four
persistent); the 94 isolated startup dependency tests still intentionally model
all six helper returns to test wrapper behavior. Early banner-boundary mutations
to both callback words are cleared before the next startup boundary's full state
snapshot. Shutdown preserves both words. Graphics initialization/backend/banner,
CRT heap and handle callbacks remain modeled. Callback registration, consumers,
DirectInput and timer execution are static evidence, not runtime coverage.

Compilation, raw-prefix diagnostics and differential emulation have separate
records. Raw prefixes differ; no relocation-aware instruction equality is claimed.
Native input/timer/graphics/heap/game parity, original linked layout, concurrency,
arbitrary callback reentry and invalid/aliased storage remain unverified.
