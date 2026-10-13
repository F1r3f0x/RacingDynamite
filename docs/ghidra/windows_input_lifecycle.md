# Native Windows input lifecycle (in progress)

Current reconstruction placement: `decomp/src/input.c` with `decomp/include/input.h`.
See [the module migration](windows_input_module.md). Earlier `geputget.c` references
below describe the historical feature layout; original source filenames remain unknown.

Primary source is the authentic standard Windows `IGN_WIN.EXE`, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
PE32 i386 preferred VA base is `0x00400000`. Local PE/FPO and SDK inspection
provide the primary evidence. After the connection was restored, bounded Ghidra
inspection returned matching section ranges and Init/Shutdown routine extents and
instructions. The active target remains the user-provided operating assumption,
with local fingerprints independently verified. Ghidra's guessed no-argument
Init signature is rejected by FPO, stack loads and explicit reinitializer callers.
Its incorrect COM prototypes remove the polling event loop as unreachable;
instruction and execution evidence retain that loop. Ghidra has not defined the
timer callback at VA0x00456160; its local FPO and instructions remain authoritative.
Raw decompiler output stays in disposable preparation, not committed source.

| Routine | RVA / VA | FPO | Complete-body SHA-256 |
| --- | --- | --- | --- |
| Input_InitNative | 0x55AC0 / 0x00455AC0 | 406 bytes, 9 local dwords, 2 arguments, flags 270 | cf2e4a6c7b8596487d881bf18519ca764fcdfc408e3b4411252c92e6a61aa174 |
| Input_ShutdownNative | 0x55E20 / 0x00455E20 | 117 bytes, no locals/arguments/flags | 1f060a91fa8308cad4a1f8cfe55b587770a24172b6d4801993012f8c9c74e39c |

Both return a full EAX integer with ordinary caller cleanup. Init's two words
are signed repeat delay/interval, demonstrated by polling RVA0x55EF0 arithmetic
and explicit forwarding by reinitializer RVA0x55EB0. They are not callbacks.
WndProc's Alt-Enter call VA0x00412420 pushes neither: four original-only probes
prove that Init sees saved EDI/ESI as those timing words. Startup RVA0x12500 also
omits argument pushes. Whole-startup reconstruction must preserve or explicitly
track this authentic missing-argument behavior; no default timing is invented.

Init calls DirectInputCreateA's authentic veneer VA0x00478778 with version0x300,
instance VA0x004C5398 and output VA0x0050DF60. It creates GUID_SysKeyboard,
sets the verified keyboard data format, cooperative flags6 using HWND VA0x004C539C,
buffer property32, then acquires. Every setup HRESULT other than zero returns
zero; Acquire's negative HRESULT only clears the acquired flag, and Init still
returns one after clearing input state. The acquired flag is first written one.
The character queue gets sixteen FFFFFFFF dwords; two 256-byte state controls
are cleared as sixty-four DWORD stores each. Separate 256-byte tables and
256 DWORD repeat counters are then cleared in the original interleaved order.
Timing arguments and current clock VA0x0049373C are retained, timer words zeroed.

The SDK format at VA0x00478760 has words24/16/2/256/256 plus object-table pointer
VA0x00477760. All 256 objects use GUID_Key at VA0x00479518, offset equal to index,
type `0x8000000C | (index << 8)` and flags zero. GUID_SysKeyboard is copied from
VA0x00479548. The readable C89 table is generated from these verified semantics,
with SDK structures; original binaries/assets are unchanged.

Shutdown conditionally Unacquires the keyboard when VA0x0050E270 is nonzero,
then clears that flag. It independently rereads and releases the keyboard at
VA0x0050E26C and DirectInput at VA0x0050DF60, zeroing each after release. If
VA0x004BA65C is nonzero it calls `WINMM.dll!timeKillEvent` through IATVA0x0064C46C
with timer IDVA0x004BA658. It retains both timer words and ignores all replies.
Input/device/acquired start loader-zeroed in the virtual .data tail; timer words
are file-backed zeros. SDK 32-bit checks independently confirm Release offset8,
Unacquire offset32 and four-byte pointers/UINT.

Preparation has 28 original-only setup cases and 49 shutdown cases, including
callback pointer replacement, skipped parent release, late timer activation,
retained late acquired-flag changes, shared-interface duplicate releases and
invalid object/vtable faults with captured partial state. Boundaries are explicitly
modeled, so these preparation probes alone do not establish native input behavior.

The production SDK build and `uv run tools/verify_input_lifecycle.py` now pass
272 original-versus-C comparisons: 94 Init and 178 Shutdown, including 177
persistent follow-ups, twelve faults and ten executed callback mutations. The
independent raw-address oracle compares ordered reads/writes, full globals/arena,
unrelated image bytes, caller bytes, cdecl/stdcall cleanup, preserved registers
and direction flag. SDK entry snapshots check the full 256-object format graph,
GUID/property bytes and every call argument. Setup failures, Acquire signedness,
null/invalid returned interfaces, live pointer/window/clock changes, duplicate
releases and timer retention are covered. A read-order mismatch was corrected:
HWND is read before the keyboard pointer for SetCooperativeLevel.

The native host input variant also passes: three paired windowed 0/1/4 scenarios
execute twelve actual original/C89 Init/Shutdown cycles plus two empty shutdowns.
Six cycles cancel real test-owned multimedia timers through production timeKillEvent;
a second actual cancellation rejects the already-disposed ID. Interfaces are zeroed
and timer words retained. Constructors and surface shutdown remain real dependencies.
The original WndProc is a reference boundary. Real foreground state may make Acquire
fail; the native contract checks authentic successful initialization with Boolean
acquired state, not a fabricated acquisition guarantee. Timed input initialization
and its event callback are outside this native scope.

The builder extracts the production SDK block and uses real i386 DINPUT/WINMM
import libraries. Exact imported APIs are checked against the authentic PE.
The core helper audit includes the authenticated DirectInputCreateA veneer and
orchestrates this contract. Full workflow renewal passes for all 72 reconstructed
routines, with 72 fresh compilation/emulation records and eleven scoped native
records. Audit and synchronized exports pass at snapshot `924c053b83b0`.
All 46 verifier inputs have identical LF working/index bytes. Raw/instruction/
linked equality is not claimed.

No playable rebuilt game, native input event/polling/race behavior, original
compiler/link configuration or instruction equality is certified. Two state-table
meanings and general asynchronous timer/reentry behavior remain unresolved.
