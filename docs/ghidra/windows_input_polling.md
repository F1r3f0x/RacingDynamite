# Native buffered keyboard polling (2026-10-12)

The primary target is the unmodified `Ignition/Ignition/IGN_WIN.EXE`, SHA256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`,
PE32 i386, image base `0x00400000`. RVA and VA are distinguished below.
The local fingerprint, FPO boundaries, instructions, static callers and initialized
data were independently checked before implementing these bodies in
`decomp/src/geputget.c`. No DOS behavior or addresses establish Windows evidence.

| Routine | RVA / VA | Bytes | FPO size / locals / arguments / flags | SHA256 |
| --- | --- | --- | --- | --- |
| Input_PollNative | 0x55EF0 / 0x00455EF0 | 376 | 376 / 129 / 0 / 5150 | ac1bfc5738cf5a68eef9491b90049eb23453052f9350d545c82586248ec92f33 |
| Input_DispatchKeyNative | 0x560C0 / 0x004560C0 | 26 | 26 / 0 / 2 / 0 | 3b23ff815d34e56903eb4477a3cb227730eae66e254731b7902b62fed7d10a8b |
| Input_QueueCharacterNative | 0x560E0 / 0x004560E0 | 59 | 59 / 0 / 2 / 0 | 12a599cf6a94f6b5f0e50c31159d04c6dab5785edbc0eadd208b3573c2139093 |
| Input_TimerCallbackNative | 0x56160 / 0x00456160 | 19 | 19 / 0 / 5 / 0 | 2d1dd1ad615325f87eae45141b3a1d7f53a09f9d2646d1adcd17f93e16c76f58 |

Ghidra MCP at localhost:8080 now responds to methods and PE segments. The loaded
authentic executable is a user-provided operating assumption; there is no automated
program identity endpoint. Bounded inspection agrees with local function extents.
Its guessed initialization signature and COM prototypes are contradicted by local
instructions. In particular the decompiler removes the polling event loop as
unreachable; original execution below confirms it. The timer callback lacks a
Ghidra function definition, while its authentic FPO extent and direct call remain
verified. These limitations do not supply missing evidence or guessed addresses.

Polling has no stack arguments. Original callers at RVAs0x29A0/0x2C00/0x3BC0/
0x172B0/0x182D0 and timer0x56160 do not consume its incidental EAX as a result.
The successor at RVA0x3B00 is a bare RET; RVA0x3E80 overwrites EAX through its first
malloc before consuming it. Key dispatch and character insertion likewise have
semantic void contracts. Timer callback retains five 32-bit stdcall arguments and
RET20. Its downstream notification has the separately verified no-argument C ABI.

Polling first examines all 256 held bytes and only advances counters for value1.
Counter subtraction/addition wraps modulo32; the delay-plus-interval threshold
and comparison are signed. It subtracts one live interval before setting the event
byte and storing the counter. Each volatile access preserves original order.
It uses actual SDK DirectInput3.0 `DIDEVICEOBJECTDATA`:16-byte records,32 entries,
vtable GetDeviceData offset0x28, five stdcall words and flags0. Compile assertions
check layout independently. The acquired flag does not guard this call.

Only HRESULT0 and1 process a signed-positive returned count. Exact HRESULT
0x8007001E clears acquired, rereads the device and calls real Acquire; nonnegative
Acquire sets acquired1. Either reacquisition outcome returns without updating
the epoch or processing records. Other failures skip records and update the live
clock epoch. Press/release state, latch bytes and counters retain original order.
Key dispatch executes the real callback wrapper. After a press callback the record
offset is reread and masked again before character insertion. Count is also reread
after callbacks; differential mutations cover both local record and count aliases.

Character insertion ignores its first argument and uses a signed scan>83 guard.
The authentic 84-byte map at VA0x004BA660 includes uppercase US characters and
literal0xFF entries; sentinels are inserted rather than filtered. Cursor VA0x0050DE10
addresses a mirrored32-byte ring beginning VA0x0050DE18. It writes at cursor and
cursor+32, rereading the cursor, increments one byte and wraps only on equality
with VA0x0050DE38. The64-byte union exposes byte access and preserves initialization's
sixteen DWORD0xFFFFFFFF stores. RVA0x55AC0 is included in this feature's regression
and commit scope for that backing-type correction. Superseded DOS polling and
character insertion bodies/declarations are removed in place.

`uv run tools/verify_input_polling.py` builds strict production C89 with actual SDK
COM calls and the real four-routine dependency chain. The independent raw-address
oracle executes original and C code with ordered global reads/writes, persistent
state, callback snapshots, SDK argument checks, entire arena/image/caller bytes,
nonvolatile registers, DF and ordinary/stdcall stack cleanup. The DLL uses existing
real input initialization/shutdown and platform imports. SDK replies and client
callbacks are explicit differential boundaries, never production success stubs.

Focused coverage is1,166 comparisons:188 polling,16 key dispatch,832 character
insertion and130 timer callback. It includes630 follow-ups, two invalid-device
faults and24 executed mutation cases. All32 valid ring cursors cover13 scan values;
DWORD wraps, held bytes0/1/2/255, seeded signed repeat arithmetic, SDK success/
overflow/failure/input-lost and Acquire sign behavior, 0/1/32/negative counts,
mixed press/releases and callback mutations are exercised. The four-routine
integration is executed through polling and timer calls, not replaced with models.

The native `SURFACE_TEST_POLL` host uses disposable originals under `build/runtime/`,
actual paired windowed constructors with0/1/4 backbuffers and original WndProc as
a reference boundary. Across twelve input cycles, SendInput scan30 press/release
is observed through actual original/C GetDeviceData and real callbacks. It checks
held/event clearing and mirrored A bytes, then calls the five-word timer callback
synchronously and checks its client notification. Actual input shutdown and timer
cancellation follow. This is native evidence for one representative key, not
asynchronous multimedia timer dispatch or rebuilt WndProc/full-game validation.

Negative standalone character indices can alias preceding original globals and
are outside this table contract; polling supplies0..255. SDK counts above32,
arbitrary cursor/global aliases, reentry, concurrent mutation and native fault
atomicity remain unverified. The two latch-table consumer semantics remain pending.
The no-push initialization callers still have their authentic timing-argument
anomaly. Original compiler, linker configuration, instruction equality, native
startup/menus/render/audio/races/fullscreen and a playable rebuilt game remain open.

Full workflow renewal, fidelity audit and generated exports pass at snapshot
`ca2851cbbf7e` with 76 current compilation/emulation records and fifteen scoped
native records. The SDK unit uses GCC `-fno-inline` so symbolic helper dependencies
remain independently audited. LF working/index checks, staged gate and tracked
hooks remain required before commit. No full native game parity is certified.
