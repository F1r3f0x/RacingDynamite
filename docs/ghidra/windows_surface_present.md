# Authentic native surface presentation

Standard IGN_WIN.EXE SHA256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`,
PE32 i386 base0x400000. Ghidra MCP is reachable and agrees with body/wrapper
extents; loaded target is the user-provided operating assumption. Local fingerprint,
FPO, instructions, installed targets and callers independently authenticate them.

| Routine | RVA / VA | Bytes | FPO (size,locals,args,flags) | SHA256 |
| --- | --- | --- | --- | --- |
| Gfx_SurfacePresentNative |0x5C4B0 / 0x45C4B0|98|98,1,1,260|0a2b21a1bf12a224948977d5410f15eee668a81d3245a9d644f8f0846af6412e|
| Gfx_PresentSurfaceNative |0x56C90 / 0x456C90|15|15,0,1,0|90586f74ec900ac8213237cc025768b3c50c8782ddbb0130505452bcea742f01|

Both have ordinary C convention, one48-byte record pointer, caller cleanup and
EAX boolean return. The real installer0x5B690 binds slotVA0x50EB88 to Present;
the wrapper calls that slot. Caller RVA0x3B10 calls Clear entry0x56BB0 followed
by this presentation entry on type1 first recordVA0x50E688.

The body reads storage_kind@40 first, requiring exactly1; active@0 must be nonzero.
It captures surface@44 and calls SDK IDirectDrawSurface.GetCaps at vtable0x38
with a four-byte DDSCAPS output. HRESULT is ignored; output DDSCAPS_FLIP0x10 is
required. AtVA0x45C4ED..0x45C504 it reloads target@44 and primary pointerVA0x50E7A4
on every iteration, calls actual SDK Flip at vtable0x2C with flags0 and retries
only DDERR_WASSTILLDRAWING0x8876021C. Any other HRESULT stops; only zero returns1.
Production C89 uses the real SDK and named independently verified fields. It does
not replace windowed/offscreen presentation with a blit or add a timeout/default.
Failed GetCaps without an output leaves uninitialized original stack bytes;
that undefined transient is explicit and is not hidden behind zero initialization.
No behavioral deviation or instruction equality is claimed.

Before implementation,3,860 original-only independent-oracle executions and actual
native installed-table rejection on nonflip offscreen surfaces authenticated the
contract. The read-only SDK driver enumeration advertises640x480x8 on this host;
that is not proof of actual mode switching or successful flip-chain creation.

`uv run tools/verify_surface_present.py` authenticates PE/FPO/hashes, compiles the
real production C89 SDK chain and checks3,860 differential comparisons:2,554 body
and1,306 entry,2,550 persistent follow-ups, eight observed faults, two executed
SDK record mutations and87 finite busy retries. It covers all26 records,
kinds0/1/2/-1, active0/1/2/-1, ignored failed GetCaps with outputs, capability bits,
zero/nonzero HRESULTs,1/2/8/32 busy retries, live target/primary/active/kind changes,
null/unmapped target and primary. Raw-offset oracle comparisons retain ordered
reads, API arguments/state at calls, complete records/arena, unrelated image bytes,
caller bytes and preserved registers/stack/returns. Only the authenticated dispatch
pointer relocation is normalized. SDK calls/results are modeled for emulation.

`SURFACE_TEST_PRESENT` native host uses actual paired windowed constructors,
installer, GetCaps and body/entry paths. Seventy-two calls (36 direct/36 entry)
check actual nonflip capability rejection, inactive/type2 rejection and primary
kind rejection in windowed0/1/4 scenarios. Test-owned record mutations are restored
before real shutdown. The original WndProc remains a reference boundary.

`tools/verify_native_presentation.py` additionally generates a host from the shared
loader and `tools/native_present_frames.c`. Before adding this check, an original-only
fullscreen experiment proved four actual flips and complete primary pixel equality.
The paired original/C89 test now uses actual fullscreen640x480x8 constructors,
installer and body/entry, checks DDSCAPS_FLIP, fills the actual SDK backbuffer and
compares every primary pixel after each of eight total flips (2,457,600 checks).
Each side alternates two direct and two installed-entry calls. Test-owned interface
disposal precedes checked RestoreDisplayMode and SetCooperativeLevel(NORMAL), real
shutdown and checked window destruction. This is actual frame presentation for one
mode/one backbuffer, not a complete production rendering pipeline. Original WndProc
remains a reference boundary; its local400-byte FPO extent/branches show that only
Alt-Enter enters input/game teardown, disabled by the existing host controls.

An initial completion run was stopped before native phases because this new native
scope required final verifier inputs and accurate workflow limitations. Its own
process tree and stale lock owner were verified before cleanup; no active external
owner or gate was bypassed. The final full renewal includes both rejection and frame
checks with all production source inputs fixed.
Infinite busy loops, absent GetCaps outputs, arbitrary aliases/reentry/concurrency,
native fault frames and original compiler/link configuration remain open. The
focused DLL/host are not the rebuilt playable game; startup, menus, rendering,
input, audio and representative races still require native behavioral validation.

Full renewal, fidelity audit and generated exports pass at snapshot `e607f844ec6e`
for all 88 reconstructions, with 27 scoped native records. LF working/index checks,
staged gate and tracked hooks follow. Continue with real palette,
rendering and startup/window-message dependencies after the separate commit.
