# Windows graphics backend selector

Input: authentic Ignition/Ignition/IGN_WIN.EXE, 915,968 bytes, SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782,
PE32 i386, preferred base VA 0x00400000. Originals are untouched; no DOS
executable supplied evidence. Names and geputget.c placement are semantic,
not recovered original symbols. The user-provided operating assumption is that
this executable is active in Ghidra; no automated loaded-program identity is claimed.
Read-only localhost:8080 disassembly, decompilation and xrefs corroborate the
independently authenticated local PE instructions and data.

## Selector and ABI

Gfx_SelectBackend is VA 0x00456AF0 / RVA 0x56AF0, FPO tuple (30,0,1,0),
complete extent ending VA 0x00456B0E, followed by two INT3 bytes. Body SHA-256:
d1fcb40ea924f27f8afef2a4b1e804ee70ce3fed2ff4f9e74a376877e7c3c4cd.
Nine instructions load [ESP+4] into ECX, test it, and branch on any nonzero
bit pattern. Zero calls VA 0x0045B690 then VA 0x00456E60, ignores both results,
and returns EAX=1. Nonzero returns EAX=2 without a call or global access.
There is no selector global, selected-backend cache, initialized flag, retry,
failure propagation or rollback in this routine. Repeated zero repeats both calls.
No HIGHLOW relocation lies inside its extent.

The public interface is int Gfx_SelectBackend(int backend), one 32-bit stack
argument with ordinary RET. Mem_InitSystem pushes zero at VA 0x0045B189,
calls at 0x0045B18B and adds ESP,4. App_Init pushes zero at 0x00412500,
calls at 0x00412502 and adds ESP,4. Both ignore the selector result.
These are the only direct calls found in a complete bounded FPO scan and
Ghidra xrefs; this does not exclude arbitrary indirect callers. EBX/ESI/EDI/EBP
and caller stack are preserved. Both dependencies take zero stack arguments,
return EAX=1 and use ordinary RET; no-argument RET alone does not establish
cdecl versus stdcall. Ordinary C ABI is used. Clear DF is the harness precondition.

## Dispatch ownership and initialization

Gfx_InstallSurfaceDispatch, VA 0x0045B690 / RVA 0x5B690, FPO (146,0,0,0),
SHA-256 3cb7a08c993e9d2ce4db93a2ebe96e9165728ef6bae5b5014371ebaef3de41bf,
performs fourteen ordered dword stores then returns 1, with no calls:

| Dispatch VA | Function VA |
| --- | --- |
| 0x0050EB68 | 0x0045B730 |
| 0x0050EB6C | 0x0045B740 |
| 0x0050EB70 | 0x0045BD70 |
| 0x0050EB74 | 0x0045C060 |
| 0x0050EB78 | 0x0045C150 |
| 0x0050EB7C | 0x0045C160 |
| 0x0050EB80 | 0x0045C1D0 |
| 0x0050EB84 | 0x0045C3D0 |
| 0x0050EB88 | 0x0045C4B0 |
| 0x0050EB8C | 0x0045C520 |
| 0x0050EB90 | 0x0045C530 |
| 0x0050EB94 | 0x0045C680 |
| 0x0050EB98 | 0x0045C6D0 |
| 0x0050EB9C | 0x0045C730 |

Gfx_InstallSpriteDispatch, VA 0x00456E60 / RVA 0x56E60, FPO (181,0,0,0),
SHA-256 215bc3324a371904ec72c0e780eec9b178b47f01e195eaaac6124781c612a8b6,
performs sixteen stores in the order below, calls VA 0x0045D840,
0x0045C9F0 and 0x0045C7F0 without stack arguments, then returns 1.

| Dispatch VA | Function VA |
| --- | --- |
| 0x0050EBA0 | 0x00456F20 |
| 0x0050EBA4 | 0x00456F30 |
| 0x0050EBA8 | 0x00456F40 |
| 0x0050EBAC | 0x00456F50 |
| 0x0050EBB0 | 0x00456F60 |
| 0x0050EBB4 | 0x00457020 |
| 0x0050EBB8 | 0x00457250 |
| 0x0050EBBC | 0x004571B0 (reconstructed Gfx_DrawSpriteNative) |
| 0x0050EBC0 | 0x0045C830 |
| 0x0050EBD8 | 0x00461360 |
| 0x0050EBD4 | 0x0045C920 |
| 0x0050EBDC | 0x0045C940 |
| 0x0050EBE0 | 0x0045C960 |
| 0x0050EBE4 | 0x00461A80 |
| 0x0050EBEC | 0x00457110 |
| 0x0050EBE8 | 0x00457170 |

The four words VA 0x0050EBC4..D0 are not written by either initializer.
Every destination and function literal in these stores has its own HIGHLOW
relocation. All thirty dispatch words lie in the loader-zeroed .data virtual
tail, beyond its file-backed bytes. They are four-byte function pointer slots;
no aggregate struct layout or common callable prototype is inferred. For example,
VA 0x00456BC0 calls [0x0050EB6C], returns zero on its zero result, otherwise
tail-jumps through [0x0050EBA0], without arguments. VA 0x00456A40 uses
0x0050EB68 as the preceding record array's end marker, not a selector variable.
Other dispatched signatures require independent recovery before reconstruction.

The three downstream bodies were inspected locally and in Ghidra:
0x0045D840 (84 bytes, SHA-256
445faf9112110b216014f95c057ab6d572a81fec3fa64a867b9d5f03739c1d43) clears
VA 0x0051AA50, checks file-zero flag 0x004BACE8 and, only when zero,
makes three cdecl malloc(0x20D8) calls at VA 0x00469400, storing results at
0x0051AA5C/60/64, then sets the flag to 1 without checking allocation failure.
0x0045C9F0 (84 bytes, SHA-256
922d88eb43e5d8b7ede3d71df0d81a543dc389bfad921e3faa13815d0ac5468e) does
the same for counter 0x0051A9CC, flag 0x004BACE4 and pointers 0x0051A9D8/DC/E0.
Their incidental EAX values are ignored. 0x0045C7F0 (62 bytes, SHA-256
41862be4869238f9f9261e170ab377306cd2aa18afbbd6761a59bf2f32a5b132) publishes
0x00514B98 at 0x0051A998, fills 2,000 pointer slots beginning 0x00512C58
with consecutive 12-byte records beginning 0x00514BD8, clears each record's
first word, calls VA 0x004612E0 with (0x00514B98,0), cleans eight bytes and
returns 1. These effects are static evidence, not executed graphics parity.

## Production integration, validation and limits

The checkout had no native production selector definition to preserve. Its
untrusted nonreturning validation body is replaced with the exact production
geputget.c body and public declarations, extracted into the shared banner
translation unit. Initializer fixtures are separate nonreturning loops, avoiding
compiler elimination of the second call. Existing consumers of compile_banner
also link the selector; their other dependency models retain their prior scope.
Full geputget.c remains unvalidated legacy material. No new production dispatch
globals or substitute initializer implementation is invented.

Clang/LLD 19.1.1, i686-pc-windows-msvc, strict C89, -O2, freestanding/no-builtin,
disabled inlining/unrolling/vectorization/SSE are provisional validation flags.
The import-free focused validation DLL is not a playable rebuilt game.
572 original-versus-production differential calls pass: 266 nonzero selectors
(representative/sign-bit/random words, repeated without reset), and 306 zero
selectors (25 representative and 128 random independent return pairs, each
repeated without reset). Models mutate callback and dispatch words at both
initializer boundaries, clobber volatile registers/flags and return arbitrary
failure words. The second boundary observes the first mutation; the selector
preserves both and returns 1. Nonzero makes no calls or data accesses. Dispatch
storage is a validation-only array covering 136 bytes including untouched holes;
it is not a reconstructed production struct or execution of initializer effects.

The production selector executes through 18 integrated startups, including four
persistent startups. Retained coverage: 306 banner, 266 reset, 112 startup,
519 shutdown comparisons, 94/74 isolated wrapper contracts, ten persistent
lifecycle calls. Isolated wrapper tests still model the selector to independently
exercise ignored helper results. Ordered calls/arguments/accesses, all tracked
state, entry snapshots, EAX, caller stack/nonvolatile registers/DF and unrelated
image bytes are checked. Static assertions authenticate exact selector instructions,
initializer stores/relocations/order, downstream hashes/FPO, flags and callers.

Compilation and raw-prefix diagnostics are recorded separately from emulation.
Raw prefixes differ; relocation-aware instruction equality remains unverified.
Primitive initialization, both dispatch initializers, CRT printf/heap and handle
callbacks remain modeled. Native graphics/console/input/heap/game parity,
original compiler/link layout, concurrency, general reentry and invalid/aliased
storage remain unverified. EXACT denotes recovered selector behavior only.

Run in PowerShell with UV_CACHE_DIR=build/uv-cache and UV_OFFLINE=1:
`uv run python tools/verify_mem_lifecycle.py`, then
`uv run python tools/workflow.py complete --rva 0x56AF0 --rva 0x5B170 --rva 0x5B1A0 --rva 0x55AB0 --rva 0x5B4F0 --limitation "Primitive and dispatch initializers, CRT printf/heap and callbacks modeled; instruction equality and native game parity unverified"`.
Tracking records both initializers as analyzed only, without compilation or
emulation evidence. Next bounded candidate: Gfx_InstallSurfaceDispatch,
VA 0x0045B690 / RVA 0x5B690, 146 bytes; independently recover each slot's
consumer ABI before adding typed production dispatch globals and executing it.
