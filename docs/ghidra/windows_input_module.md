# Windows input module migration

This feature reorganizes already reconstructed authentic `IGN_WIN.EXE` routines.
It does not identify new binary extents or claim the original source filename.
The target fingerprint and VA/RVA base remain those in `decomp/target.json`.

## Ownership and provenance

Reconstruction module `decomp/src/input.c` owns keyboard interfaces, callback
slots, buffered polling, character insertion, repeat state and timer consumers.
Its public header provides functions and callback interfaces. State layout types
stay inside the implementation; the validation DLL explicitly exports observable
state for existing independent oracles. Shared clock backing remains provisionally
here with its original ownership question retained. Win32 instance/window words
remain owned by the existing platform backing in `main.c`; input declares those
two external bindings and neither duplicates nor initializes them.

Original Windows translation-unit names are unknown. `decomp/modules.json`
records reconstruction placement separately with `original_source: null`.
Historical DOS input remains explicitly unvalidated in `geputget.c`; this migration
does not transfer its evidence or certify it.

| Routine | Verified RVA | Existing evidence |
| --- | --- | --- |
| Input_ResetCallbacks | 0x55AB0 | [Callback reset](windows_resource_reset.md) |
| Input_InitNative | 0x55AC0 | [Lifecycle](windows_input_lifecycle.md) |
| Input_InitTimedNative | 0x55C60 | [Timed input](windows_input_timed.md) |
| Input_ShutdownNative | 0x55E20 | [Lifecycle](windows_input_lifecycle.md) |
| Input_ShutdownNativeEntry | 0x55EA0 | [Timed input](windows_input_timed.md) |
| Input_ReinitializeNative | 0x55EB0 | [Timed input](windows_input_timed.md) |
| Input_PollNative | 0x55EF0 | [Polling](windows_input_polling.md) |
| Input_ReadHeldNative | 0x56070 | [Timed input](windows_input_timed.md) |
| Input_ConsumePressNative | 0x56080 | [Timed input](windows_input_timed.md) |
| Input_ConsumeRepeatNative | 0x560A0 | [Timed input](windows_input_timed.md) |
| Input_DispatchKeyNative | 0x560C0 | [Polling](windows_input_polling.md) |
| Input_QueueCharacterNative | 0x560E0 | [Polling](windows_input_polling.md) |
| Input_TimerCallbackNative | 0x56160 | [Polling](windows_input_polling.md) |

Preferred VA is RVA plus `0x00400000`. Retain each routine's existing ABI,
fidelity, stage and independent binary evidence; relocation is not a promotion.

## Build contract

`tools/build_decomp.py` compiles the entire production `input.c` directly with
the existing provisional GCC Win32 SDK settings: i386, strict C89, warnings as
errors, O2, no inlining and no sibling-call optimization. Its object is an explicit
fresh link input to each focused DLL. No input body/header text is sliced out of
`geputget.c`. DirectInput and WinMM remain actual SDK imports; no successful
dependency substitute is introduced. Remaining graphics/platform extraction and
CRT validation boundaries retain their stated limitations.

Verification records include the complete input source and header whenever the
shared builder is used, so later module edits invalidate dependent evidence.

## Validation and limitations

Run the existing lifecycle, polling and timed input contracts through the migrated
module, followed by `workflow.py complete` for the affected RVAs. This renews the
shared build's compilation, emulation and applicable scoped native evidence.
Ownership guard tests additionally exercise duplicates, wrong paths, unknown
rules, preflight declarations and catch-all module additions. Actual completion
results and snapshot are recorded after that sequence succeeds.

Instruction equality and the original compiler/link configuration remain
unestablished. Native checks retain their existing reference WndProc, bounded
input scenarios and test-owned timer quiescence; arbitrary concurrent teardown
and a playable reconstructed game remain unverified.

## Completed migration results (2026-10-12)

Snapshot `6b811d6c4e9a` records 15 affected RVAs: the thirteen input entries
above plus unchanged selector `0x56AF0` and memory-free `0x5B000` definitions
whose surrounding header/annotation context changed. Mechanical comparison with
the pre-migration source confirms all thirteen input bodies are unchanged.

The full completion sequence passed: 90 fresh compilation records, 90 differential
emulation records and 29 scoped native records across the active reconstruction.
The moved routines have 8,152 recorded differential comparisons (266 reset,
272 lifecycle, 1,166 polling and 6,448 timed/consumer comparisons). Twelve retain
bounded native input evidence; callback reset has emulation/startup evidence.
Raw/instruction/linked equality remains unclaimed. All 52 ownership, workflow,
tracking and target diagnostic tests pass. The font parser boundary needed an
explicit include path after replacing embedded declarations with a real header;
its repaired 260-case contract and subsequent full renewal passed.

During coordination, the rebuild chat identified a pre-existing palette oracle
gap: its normal RGB source at `ARENA+0x5000` exceeds the 0x4000-byte arena, and
eight scripted post-copy interface changes did not reach their trigger. Existing
native 24 SDK palette updates and palette-bank alias cases are real. This migration
preserves palette files and does not broaden that oracle's coverage; the rebuild
chat owns a separate mapped-source validation repair after this handoff.

Next migrate fonts, DirectDraw surfaces and sprite resources in bounded features,
with genuine whole-module compilation and existing contract renewal. This feature
establishes module ownership and directly compiles verified input; it does not
complete all input consumers or certify the remaining legacy source modules.
