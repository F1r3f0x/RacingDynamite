# Native surface access (2026-10-12)

The primary binary is authentic standard Windows `IGN_WIN.EXE`, SHA256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`,
PE32 i386, image base `0x00400000`. Local section/FPO boundaries, routine hashes,
instructions and callers were checked before production implementation. Ghidra
MCP responds and agrees with both extents; loaded identity remains the user's
operating assumption, separate from the independently verified local fingerprint.
Decompiler signatures are hypotheses, not ABI evidence.

| Routine | RVA / VA | Bytes | FPO size / locals / arguments / flags | SHA256 |
| --- | --- | --- | --- | --- |
| Gfx_SurfaceLockNative | 0x5C530 / 0x0045C530 | 326 | 326 / 27 / 2 / 778 | 35592a3ed0bfaa7af2ea2a5d0ca5116f8f7f3f225a78705c31cc9ddfa8b3b512 |
| Gfx_SurfaceUnlockNative | 0x5C680 / 0x0045C680 | 71 | 71 / 0 / 1 / 257 | 0e6fdf8dbb9b1279859342662207e6693a69b37cf823335fd050d2c6aae1c7af |

Both use ordinary C stack contracts, return int0/1 and preserve nonvolatile
registers. Lock takes a record pointer and a 32-bit mode; Unlock takes a record.
Verified forwarding wrappers at RVAs0x56C10/0x56C30 call installed entries with
two/one arguments and caller cleanup. Clear0x5C3D0 directly calls both, but is
not reconstructed here; its DWORD indexing of SDK byte pitch remains explicit.

Five formerly opaque words in the48-byte record are independently identified:

| Byte offset | Field | Evidence |
| --- | --- | --- |
| 8 | uint32 locked | Lock stores1 on success/0 on failure; Unlock tests nonzero and clears |
| 12 | int32 lock_mode | Captured mode equality, mode restoration and clear on unlock |
| 16 | const byte pointer read_pixels | SDK lpSurface for modes1/2, zero for mode3 |
| 20 | byte pointer write_pixels | SDK lpSurface for modes2/3, zero for mode1 |
| 24 | int32 pitch_bytes | SDK DDSURFACEDESC.lPitch, raw byte pitch |

Compile assertions preserve size48, all field offsets, descriptor size108 and
existing geometry/interface offsets. SDK layout checks also verify Lock0x64,
Unlock0x80, descriptor lpSurface36/lPitch16 and mode flags READONLY|WAIT0x11,
WAIT0x01 and WRITEONLY|WAIT0x21. Independent original native execution corroborates
pointer roles and SDK acceptance. Initializer RVA0x56A40 now clears these named
fields in the same DWORD order; its existing fresh regression is part of this
feature's completion/trailer scope. No DOS packing or field names establish proof.

Lock rejects active0. If locked is exactly1 and mode matches, it returns1 without
SDK calls. For another mode it calls actual SDK Unlock with the **address of the
record's surface field**, not its pixel pointer. Nonzero HRESULT stops0; success
clears only the mode at this step. It then calls IsLost, and only exact
0x887601C2 calls Restore; Restore's result is ignored and marker becomes1.
Invalid modes still execute that prefix, then return0. Modes1/2/3 invoke real
Lock with a null rectangle/event and the verified flags.

The descriptor has only dwSize initialized. The original stores lpSurface into
the appropriate pointer fields before checking HRESULT. Failure then clears both
pointers and locked, leaving pitch and mode unchanged after the earlier prefix.
Success stores mode, descriptor byte pitch and locked1. This deliberately retains
the original uninitialized-data exposure when a failing API leaves lpSurface
unwritten; those transient stack bytes are not certified by this contract.

Unlock rejects active0, returns1 immediately for locked0, otherwise calls the SDK
with the same surface-field address. Failure preserves fields; success clears
locked/mode only. Pointer and pitch values remain stale. Production uses readable
C89 with typed SDK calls, volatile record accesses and live interface rereads,
not invented success-returning dependencies. Builder nonreturning boundaries for
these two routines are removed; the authentic installer now references real bodies.

`uv run tools/verify_surface_access.py` authenticates hashes/FPO, builds the real
SDK unit and compares original/C executions against an independent raw-record
oracle. Modeled SDK replies explicitly include deterministic descriptor outputs
on errors. Ordered record accesses, SDK arguments/snapshots, complete arena/image/
caller bytes, result, stdcall/cdecl cleanup, nonvolatile registers and DF are checked.

Focused coverage passes1,920 comparisons:962 Lock and958 Unlock, with1,116
follow-ups, four object faults and eight executed mutation cases. All26 records
cover repeated modes0/1/2/3/4/FFFFFFFF; the mode matrix includes active and locked
words0/1/2/FFFFFFFF, signed mode bit patterns, SDK errors, lost/restoration branches,
negative pitch bits, live metadata mutation and persistent unlock/relock. Failed
Lock without descriptor writes, arbitrary aliases/reentry and native fault frames/
atomicity remain outside scope, rather than being hidden by initialized substitutes.

Native `SURFACE_TEST_ACCESS` runs actual original/C calls after paired windowed
0/1/4 constructors:90 Lock calls and60 Unlock calls across primary/backbuffer
interfaces. It verifies mode pointer roles/pitch, unchanged same-mode access,
real automatic unlock/relock on mode changes, the unusual SDK argument, retained
pointers/pitch and repeated unlock. Actual production resource shutdown follows.
The original WndProc remains a reference boundary; native successful access does
not certify failed/lost-device cases or a reconstructed rendering pipeline.

Full renewal and fidelity/export checks pass at snapshot `af3bc931c777` for all
84 reconstructions, with 23 scoped native records. LF working/index checks,
staged gate and tracked hooks remain required before commit. Native Clear/presentation/palette,
full startup/menus/render/input/audio/races and playable game remain incomplete.
Original compiler/link configuration and instruction equality remain unresolved.

An initial full renewal stopped at the existing native input check67, without
diagnostics identifying the exact cause. Standalone polling/timed probes with the
same compiled behavior passed. The test had assumed the A byte was always at ring
offset0 despite unrelated buffered events. The shared native host now captures the
actual insertion slot in the real key callback, verifies A and its exact mirror,
checks foreground ownership explicitly and seeds valid offsets13/31 on the two
cycles (including wrap). Failure diagnostics distinguish held/event/callback/ring/
foreground values. This strengthens validation without changing production input
behavior or relaxing key/mirror assertions. Full renewal passed after this validation fix.
