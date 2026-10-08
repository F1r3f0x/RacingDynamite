# Windows memory-system lifecycle wrappers

## Authenticated provenance

Input: Ignition/Ignition/IGN_WIN.EXE, 915,968 bytes, SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782,
PE32 i386, preferred image base VA 0x00400000. Original assets are untouched.
No DOS executable was used as evidence. Names below are semantic names;
original source names are unknown and the mem.c association is inferred.

| Name | VA / RVA | Complete extent | SHA-256 |
| --- | --- | --- | --- |
| Mem_InitSystem | 0x0045B170 / 0x5B170 | 41 bytes; end 0x0045B199 | e4026b0791ef97b56f8fa7013dd568dfd4772d64e2638d850a217c9d338ce2b8 |
| Mem_ShutdownSystem | 0x0045B1A0 / 0x5B1A0 | 16 bytes; end 0x0045B1B0 | 75f6e3f589d50b5cb564113e1270c4f2a5df39c0614d41516b520d0d3ee064a3 |

Local fingerprint, FPO, complete decoding (10/4 instructions), relative call
targets, returns and caller instructions independently corroborate live Ghidra
MCP inspection. Both FPO tuples are (bytes,0 local dwords,0 argument dwords,0
bits); neither body has a HIGHLOW relocation. Startup has seven INT3 padding
bytes. Shutdown ends immediately before Mem_NextHandleId, without padding.
Both have no arguments, ordinary RET, EAX=1, preserved EBX/ESI/EDI/EBP and
balanced caller stack. No-argument RET alone does not distinguish cdecl/stdcall;
production uses the ordinary x86 C ABI. Clear DF is required.

Authentic IGN_WIN.EXE being loaded and active in Ghidra is the user's operating
assumption, separate from local fingerprint verification. No automated
loaded-program identity is claimed. Ghidra disassembly/decompilation/xrefs were
read without modifying its program. Its old labels/comments are hypotheses.

## Dependency order, callers and registration

Startup unconditionally makes these calls, then overwrites EAX with 1:

| Call VA | Target VA | Recovered boundary |
| --- | --- | --- |
| 0x0045B170 | 0x0045B4F0 | Lisa_PrintVersion, no arguments |
| 0x0045B175 | 0x0045AD50 | Mem_InitPools, no arguments |
| 0x0045B17A | 0x0045B1F0 | Mem_InitHandles, no arguments |
| 0x0045B17F | 0x00455AB0 | Input_ResetCallbacks, no arguments |
| 0x0045B184 | 0x0045E610 | Gfx_InitPrimitiveState, no arguments |
| 0x0045B18B | 0x00456AF0 | Gfx_SelectBackend, one stack dword zero |

The final argument is PUSH 0, followed by caller ADD ESP,4. No result is tested;
there is no initialized flag, retry, rollback, early failure or extra dependency.
Mem_InitPools always resets roots and ignores default pool creation failure.
Mem_InitHandles skips only when its flag equals 1; other raw values reset the
handle table/IDs/cursor. Repeated startup can orphan previous pools and preserve
registered handles when that exact-one guard skips reset. These authentic quirks
remain intact. This wrapper initializes more than the memory subsystem despite
its reconstruction name and placement in mem.c.

Shutdown calls Mem_ShutdownHandles at VA 0x0045B1A0, then Mem_ShutdownPools at
VA 0x0045B1A5, ignores both results and returns 1. Registered handle callbacks
therefore execute before remaining pools are destroyed. Callback pool mutations
are visible to the later live root scan; pool frees cannot cause an already
finished handle scan to restart. Repeated shutdown still calls both helpers.

The only direct callers found in Ghidra and a local FPO instruction scan are
WinMain's CALL at VA 0x00412170 and App_FrameTick's CALL at VA 0x004122A7.
Their local displacements are asserted. WinMain calls startup after window-class
and timer setup, before App_Init, without testing its result. The stage-two
App_FrameTick path calls game shutdown VA 0x00417E90, then this memory-system
shutdown, then conditionally timeEndPeriod(1); it marks its shutdown guard and
returns zero. These callers are inspected statically, not executed by this suite.
A second direct backend-selector caller at VA 0x00412502 belongs to App_Init.

Neither wrapper registers an atexit/CRT hook, stores a cleanup function pointer,
or registers a handle. No HIGHLOW relocation in the authentic image points to
either wrapper entry; the FPO scan finds only the above direct calls. These are
bounded scans, not proof that arbitrary indirect references cannot exist. Handle
registration is client-driven through Mem_RegisterHandle, as independently
recovered in the handle contracts; teardown dispatches its live registered slots.
The backend dependencies install graphics dispatch pointers, not wrapper teardown
registration. No invented lifecycle registration was added.

## Independently inspected startup boundaries

Follow-up: [input callback reset](windows_resource_reset.md) reconstructs RVA
0x55AB0 and executes its real production body through integrated startup.
The former Gfx_ResetResourceFlags name was provisional; the two words are
keyboard-event and post-poll callbacks. The dependency details below reflect this follow-up; the other three startup
bodies remain analysis-only.

| Semantic name | RVA | Bytes | SHA-256 |
| --- | --- | --- | --- |
| Lisa_PrintVersion | 0x5B4F0 | 34 | 719ae03224b3f4a4ab08fa52985cd65454730bd8de870f8afe632d9216951802 |
| Input_ResetCallbacks | 0x55AB0 | 13 | c301c37754413d115bac22b922a9ed480b4080011c559a4485b7ea044270fd59 |
| Gfx_InitPrimitiveState | 0x5E610 | 326 | fd2abfef167ef5b92074ef41c17478dd955591025afb3aad714da050a29704a3 |
| Gfx_SelectBackend | 0x56AF0 | 30 | d1fcb40ea924f27f8afef2a4b1e804ee70ce3fed2ff4f9e74a376877e7c3c4cd |

All four hashes, FPO extents and complete instruction decoding are asserted.
Input_ResetCallbacks is now reconstructed and verified; the other three remain
analyzed without production Windows reconstruction or compilation/emulation
results. Existing Lisa_PrintVersion DOS code is not trusted or linked
as a Windows implementation. Production mem.c has explicit extern declarations,
not substitute bodies. The two remaining graphics names reflect inspected effects,
not recovered original symbols or a complete subsystem contract.

- Banner: calls CRT printf VA 0x0046A500 twice, with version string VA
  0x004BAB20, format VA 0x004BAB5C and copyright VA 0x004BAB3C; caller cleans
  two/one dwords. Returns EAX=0. Three HIGHLOW operands refer to those strings.
- Input callback reset: XOR EAX,EAX, stores zero to the two loader-zeroed .data words
  VA 0x0050DE14 and 0x0050E678, then RET. Two HIGHLOW operands select them.
  Independent consumer recovery identifies keyboard-event and post-poll callback
  pointers; VA 0x00455F48 only references the repeat-array end address. See
  windows_resource_reset.md; neither word is the font flag VA 0x004BA6C4.
  EAX happens to be zero;
  callers consume no result, so the reconstruction boundary prototype is void.
- Primitive initialization: clears four dwords at VA 0x0051FD00 then copies
  that zero packet to seven 16-byte destinations (0x0051FE98, 0x005203B8,
  0x00520360, 0x0051FE30, 0x0051FC88, 0x005203A0, 0x00520380). It calls
  0x0045F490, 0x00460740, 0x004611D0, 0x004614D0, 0x004607C0, 0x0045F640
  and 0x0045FA50, then tail-JMPs to 0x00460B50. Its extent does not end in
  RET; downstream tables/packing and the final body's return need recovery.
  The wrapper ignores its return; the extern prototype is void.
- Backend selector: one caller-cleanup integer argument; nonzero returns 2
  without initialization. Zero calls VA 0x0045B690 then VA 0x00456E60 and
  returns 1. Neither result is checked. No absolute relocations in this wrapper.
  First dependency installs fourteen function words at VA 0x0050EB68..9C;
  second installs sixteen dispatch words in VA 0x0050EBA0..EC and calls
  0x0045D840, 0x0045C9F0, 0x0045C7F0. The exact store order and downstream
  interfaces are further reconstruction work, not modeled native equivalence.

## Compilation, differential validation and limits

Run from repository root in PowerShell with Python through uv:

~~~powershell
$env:UV_CACHE_DIR = Join-Path (Get-Location) 'build/uv-cache'
$env:UV_OFFLINE = 1
uv run python tools/verify_mem_lifecycle.py
uv run python tools/decomp_doctor.py --probe-ghidra
uv run python tools/workflow.py complete --rva 0x5B170 --rva 0x5B1A0 --limitation "Three startup dependencies, CRT heap and handle callbacks modeled; instruction equality, original link layout, native startup/graphics/heap/game parity, invalid/aliased storage, concurrency and general reentry unverified"
uv run python tools/db.py update
uv run python tools/db.py update --check
~~~

The builder compiles complete production mem.c under strict C89 with provisional
Clang/LLD 19.1.1, i686-pc-windows-msvc, -O2, freestanding/no-builtin, disabled
inlining/unrolling/vectorization/SSE. Original compiler and linked layout remain
unknown. The separate fresh artifact is build/decomp/windows/mem_lifecycle_validation.dll,
PE32 with no imports, not a playable game. The input reset body is extracted
from production C. Three startup and CRT malloc/free fixtures are nonreturning
C loops, intercepted explicitly in emulation. They
cannot silently manufacture success if interception is missing. Existing font/
file validation builders also link those startup fixtures because they compile
complete mem.c; their unrelated dependency models are unchanged.

112 startup and 519 shutdown original-versus-production-C differential
invocations pass against an independent instruction-derived oracle:

- 94/74 isolated comparisons intercept all six/two helpers, check exact call
  order and backend-zero argument, and supply 0/1/2/high-bit/-1 plus random EAX
  values and explicit state mutations. Every result is ignored; the wrapper
  always returns 1 and adds no data accesses.
- Real pool/handle startup with flag 0/1/2/0x80000000/0xFFFFFFFF, successful/
  failed default malloc, occupied roots and early/late boundary mutations.
- 400 real shutdown comparisons cover every registered handle slot in each
  pass, a size-nonzero stale payload in the final page/block/record, and prove
  callbacks precede actual payload/block/page/root frees.
- Forty seeded shutdown states cover disabled/noncanonical enabled flags,
  exact status/class eligibility, both callback passes and ignored return words.
  A cross-phase mutation case removes/inserts roots, schedules a later second-
  pass callback, then changes handle state during CRT free without rescanning.
- Ten persistent calls include eight wrappers and real Mem_Alloc and
  Mem_RegisterHandle, with no state reset between execution calls. Explicit
  identical pending callback/parameter client setup occurs once, after allocation.
  Successful allocation, registered callback teardown, repeated shutdown,
  failed default creation and repeated startup orphaning are checked. All real
  pool and handle bodies are retained; three remaining startup dependencies,
  heap and callbacks are modeled; input reset executes.

The suite checks EAX, ESP, nonvolatile registers, caller stack, clear DF, exact
ordered tracked accesses and call arguments, complete handle arrays/pending
words/root table/one-MiB heap, full snapshots at every modeled dependency entry,
and unchanged unrelated image bytes. Boundary models clobber EAX/ECX/EDX and
condition flags. Wrapper call ownership/targets are guarded in the aggregate
build: exactly 21 generated direct calls, including six/two lifecycle calls
in exact symbolic order and the existing thirteen memory dependency calls.
Counts are compiler-specific, not an instruction equality claim.

Raw compiled prefixes differ. Relocation-aware instruction equality, original
compiler/link layout, native startup/graphics/CRT heap/callback/game parity and
playable rebuilding remain unverified. Invalid/unmapped/aliased storage,
concurrency and general reentry are excluded. Bounded dependency mutations do
not validate arbitrary reentry. Font_Load retains its pool fixture and complete
geputget.c/native rebuilding retain legacy blockers. The three remaining startup bodies
are explicitly analysis-only; their native effects are not included in passing
wrapper emulation claims. Compilation, raw-prefix diagnostics and differential
emulation are separately recorded for the two reconstructed RVAs.

The current verifier also records 266 standalone Input_ResetCallbacks comparisons;
its real body clears both callback words through 18 integrated startups.
See windows_resource_reset.md for ABI recovery, extraction and limitations.

## Production Lisa banner integration follow-up (2026-10-08)

[Lisa_PrintVersion](windows_lisa_version.md) at RVA 0x5B4F0 now executes the
extracted production lisa3d.c body through all 18 integrated startups, alongside
the real reset, pools and handles. Its exact Windows formats and string vararg
are checked at two modeled CRT printf boundaries. Both printf returns are
ignored; boundary mutations occur before initialization/reset, and startup
still returns 1. The 94 isolated startup dependency cases retain modeled banner
returns to test the wrapper independently. Shutdown coverage is unchanged.

Fresh coverage: 306 standalone banner, 266 standalone reset, 112 startup and
519 shutdown comparisons; ten persistent lifecycle calls. Two graphics startup
dependencies, CRT printf/heap and handle callbacks remain modeled. Separate
banner compilation prevents optimization based on the nonreturning CRT fixture.
See linked evidence for ABI, provenance, commands and exact validation limits.
Earlier banner-as-boundary descriptions above record the original scope.

## Production backend selector integration follow-up (2026-10-08)

[Gfx_SelectBackend](windows_gfx_backend.md), RVA 0x56AF0, now executes its
extracted production geputget.c body through all 18 integrated startups,
alongside real banner/reset/pools/handles. 572 standalone selector comparisons
verify all nonzero selectors return 2 without effects; zero invokes the surface
then sprite dispatch initializers, ignores both modeled results and returns 1.
Both initializer boundaries capture complete tracked entry state and can mutate
it; the later startup mutation case now runs at Gfx_InstallSpriteDispatch.
The 94 isolated startup cases retain a modeled selector return for independent
wrapper validation. Retained counts: 306 banner / 266 reset / 112 startup / 519
shutdown comparisons and ten persistent lifecycle calls. Raw prefixes differ.
Primitive initialization, both dispatch initializers, CRT printf/heap and handle
callbacks remain modeled. No native graphics/game parity or instruction equality
is claimed. Earlier dependency descriptions above record historical scope.

## Production surface initializer follow-up (2026-10-08)

[Surface dispatch installation](windows_gfx_surface_dispatch.md), RVA 0x5B690,
now has fourteen independently typed production globals and a real initializer
executing through the production selector and 18 integrated startups. Fresh
coverage: 266 standalone installers, 532 selectors, 306 banner / 266 input reset /
112 startup / 519 shutdown comparisons, plus 39 original-only forwarding ABI
sink cases and ten persistent lifecycle calls. Original target bodies remain
unreconstructed; the reserved slot has no observed consumer. Primitive and
sprite initialization, CRT printf/heap and callbacks remain modeled. Strict C89
focused compilation passes; raw prefixes differ. Instruction equality and native
graphics/game parity are unverified. Earlier model descriptions are historical.
