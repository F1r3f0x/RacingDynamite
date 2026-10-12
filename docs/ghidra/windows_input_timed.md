# Timed native input and state consumers (2026-10-12)

The target remains the authentic standard Windows `IGN_WIN.EXE`, SHA256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`,
PE32 i386 with image base `0x00400000`. Local PE/FPO instructions, initializers,
callers and original-only execution were checked before production changes.
Ghidra MCP responds; the loaded target is a user-provided assumption. Its timed
setup/reinit extents agree, but guessed stdcall/no-argument signatures conflict
with actual stack arguments and caller cleanup. Two consumer definitions are
absent in Ghidra; FPO, local instructions and original execution authenticate them.

| Routine | RVA / VA | Bytes | FPO size / locals / arguments / flags | SHA256 |
| --- | --- | --- | --- | --- |
| Input_InitTimedNative | 0x55C60 / 0x00455C60 | 436 | 436 / 9 / 3 / 270 | 6ade6c95c919cb9ea6d550e552623c057dbefcdef7b325d552d883a7ad782efe |
| Input_ShutdownNativeEntry | 0x55EA0 / 0x00455EA0 | 11 | 11 / 0 / 0 / 0 | 9600756fc2217d7a1487cb0390abec17132d31aeafb82b2141817396a6727d2c |
| Input_ReinitializeNative | 0x55EB0 / 0x00455EB0 | 57 | 57 / 0 / 0 / 0 | f6c11630f036e58381a9b2998e2a447b4089fbaa272abc80f77b81bdc2acdc2d |
| Input_ReadHeldNative | 0x56070 / 0x00456070 | 15 | 15 / 0 / 1 / 0 | 4be4b0c8433812e5d7a3abe390a16531f5998e56ae6a2e26b95da51a0eba564a |
| Input_ConsumePressNative | 0x56080 / 0x00456080 | 28 | 28 / 0 / 1 / 0 | 3ee48544758501dafd75914c3f6fbc8822648eb2536d881b73754074af371684 |
| Input_ConsumeRepeatNative | 0x560A0 / 0x004560A0 | 28 | 28 / 0 / 1 / 0 | 5fec67455369bb53eb5d47621fac91218a140bc0df6dfe9827f458bc03745e42 |

All six use ordinary C call contracts. Timed initialization has three DWORD
arguments in order `(period, repeat_delay, repeat_interval)`. It independently
executes real DirectInput3.0 creation, complete keyboard format/property setup,
Acquire and state clears. Its prefix follows the same verified SDK behavior as
ordinary initialization, including failure retention and failed Acquire continuing.
Calling ordinary initialization instead would add timer clears absent in the
original, so production retains the authentic separate body.

After clears it writes delay, interval, saved period at VA0x0050E67C, epoch and
timer-active1, then calls real `timeSetEvent(period, period, callback, 0, 1)`.
The callback is reconstructed VA0x00456160 with five-word stdcall ABI and real
polling. It stores the returned timer ID and returns1 iff that ID is nonzero.
Timer-active remains1 on creation failure; old ID persists until the API returns.
The period initializer is loader-zero `.data` virtual tail, independently checked
against raw/virtual section bounds and the mapped image. Reading file bytes beyond
raw section data does not prove an initializer. Real SDK imports now comprise
exactly21 authenticated APIs, including original WINMM `timeSetEvent`.

Reinitialization captures interval then delay, reads timer-active, and selects
timed setup with saved period for any nonzero flag or ordinary setup for zero.
It ignores the child result and returns1, without performing shutdown itself.
Shutdown entry calls the real teardown and returns1. Caller RVA0x4610 has two
shutdown/open/reinitialize sequences at VAs0x00404633/0x0040465F and
0x00404683/0x004046AA. This caller cleanup does not make standalone reinit safe
from original interface/timer replacement behavior; no ownership fix is added.

All three state consumers truncate the argument to its low byte. Held reads
return the raw unsigned byte0..255, including2/255, rather than a Boolean.
Press consumption reads VA0x0050E068, clears any nonzero byte and returns1;
repeat consumption does the same at VA0x0050DF68. Their actual consumer behavior
identifies the first array as `g_nativeKeyboardPressEdges`, and polling identifies
VA0x0050E168 as `g_nativeKeyboardPressLatch` retained until release. The formerly
opaque A/B names are replaced in source/build/test bindings. Existing initialization
RVA0x55AC0 and polling RVA0x55EF0 are included in this feature's regression and
commit scope. Superseded DOS state-consumer bodies/declarations are removed in place.
Held reads have41 direct sites across RVAs0x3BC0/0x182D0/0x20EB0/0x21BC0;
there are no direct FPO sites for the two consuming leaves, without disproving
indirect callers. Queued-character lookup0x56120 still calls unauthenticated helpers
0x478A40/0x469CA0 and is not reconstructed by this feature.

`uv run tools/verify_input_timed.py` authenticates all extents/hashes, SDK data and
initializers, builds production strict C89 with real dependencies, and runs an
independent raw-address oracle against original/C executions. SDK/client replies
are differential boundaries; no production success stubs replace these routines.
Ordered global accesses, SDK argument graphs, actual callback target, persistent
full state, arena/image/caller bytes, cdecl/stdcall cleanup, nonvolatile registers
and DF are checked. The real ordinary/timed/setup/shutdown/reinit chain executes.

Focused coverage passes6,448 comparisons:60 timed setup,150 shutdown entry,94
reinitialization and2,048 per consumer. It includes3,268 follow-ups, four faults and
18 executed boundary mutations. Cases cover timer IDs0/1/FFFFFFFF, arbitrary timing
argument words, SDK nonzero failures, Acquire sign behavior, zero/nonzero timer mode,
partial state, null outputs, live timer/global mutations and all256 indices with
high argument bits and raw byte values0/1/2/255, followed by repeated consumption.

The native `SURFACE_TEST_TIMED` host uses disposable originals and actual paired
windowed0/1/4 constructors with original WndProc as a reference boundary. Twelve
cycles execute real timed setup or reinit and actual asynchronous multimedia
timer->production poll->notification dispatch. SendInput scan30 press/release
checks real key callbacks, held state, one-shot press/repeat consumption, mirrored
A bytes and callback ABI; production shutdown entry follows. Test-owned timer
cancellation plus50ms quiescence precedes teardown, explicitly excluding arbitrary
concurrent release-versus-poll behavior. Original shutdown releases interfaces
before timer cancellation, and that ordering is retained, with modeled mutation/
fault checks inherited from the real teardown contract. One scan code and bounded
asynchronous observations do not certify all timing/reentry/hardware scenarios.

Original compiler/linker configuration, instruction equality, no-push startup/
WndProc timing arguments, negative character aliases, excess SDK counts, native
full startup/menus/render/audio/races/fullscreen and a playable rebuilt game remain
open. Full workflow renewal, fidelity audit and exports pass at snapshot
`28f372557d7a` for all 82 reconstructions, with 21 scoped native records. The SDK
unit uses `-fno-inline -fno-optimize-sibling-calls` to retain independently audited
helper calls in the provisional compiler. LF working/index checks, staged gate and
tracked hooks remain required before commit. No native full game parity is certified.
