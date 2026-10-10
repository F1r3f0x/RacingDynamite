# Native surface resource shutdown

Authentic IGN_WIN.EXE VA 0x45C060 / RVA 0x5C060, FPO (229,0,0,0x209),
SHA-256 `60b2e80bfcaf2f7199f8b91e331b6ee717b18925e975d0845d2196fbcfc90090`.
Previously authenticated local fingerprint/instructions/imports and Ghidra
responses are retained in disposable surface lifecycle evidence. MCP is now
unavailable; the user has been asked to reopen the target/plugin. No new Ghidra
response is invented or required to reinterpret this already inspected body.

The historical Reset slot VA 0x50EB74 actually shuts down native graphics
resources; readable names now use Gfx_SurfaceShutdownNative/g_surfaceShutdown.
A live zero fullscreen flag releases the clipper if nonnull, retaining its global
pointer. Primary then five type1 records are released only in zero mode, reading
mode per record; each captured nonnull surface is released then its pointer and
active word are zeroed. Twenty type2 records always use that release/clear path.
Other record fields remain unchanged. Live nonzero mode releases a nonnull palette
and zeros its global; any nonnull DirectDraw interface is released and zeroed.
Return is always 1. Release results are ignored. No window destruction occurs.
The stale clipper pointer and retained fullscreen primary/backbuffers are original
behavior, not added guards or cleanup policies.

Production C89 uses actual SDK Release methods in the shared i386 SDK unit.
The dispatch installer now binds the verified address under its correct shutdown
name. No production success fixture or replacement UI supplies behavior.

`uv run python tools/verify_surface_shutdown.py` freshly builds and checks 384
comparisons: 173 persistent follow-ups, 51 unchecked faults, 18 executed callback
mutation calls and 12 shared-surface comparisons. The independent raw-address
oracle checks exact live mode/pointer reads, captured Release this arguments,
pre-callback snapshots, pointer then active clear order, ignored Release values,
all record/global/palette/arena/unrelated image bytes, caller/nonvolatile/DF and
stdcall cleanup. Every bank slot, sparse/all/shared ownership, mode values
0/1/2/-1, late callback changes and stale-clipper repeated-shutdown faults are
covered. SDK Release results/clobbers/mutations are modeled only in emulation.

The native probe's `SURFACE_TEST_SHUTDOWN` variant runs both original and actual
C89 shutdown with real interfaces created by their constructors. Four paired
scenarios cover missing class and windowed 0/1/4 backbuffers. Real offscreen
interfaces are also supplied at type2 bank boundaries 0 and19, with active0 and
nonzero marker/geometry, validating pointer-based release and retained metadata.
Production shutdown clears active/pointer fields, preserves markers/geometry,
keeps the stale clipper word and HWND, and releases/zeros DirectDraw. Test code
only disposes the remaining HWND/class afterward. The authenticated original
WndProc remains a reference boundary; game/input/shutdown-message paths are
prevented. No rebuilt procedure or full game startup is certified.

Original compiler/link layout and instruction equality remain open. Native
fullscreen/palette branches, lost-device behavior, arbitrary reentry/concurrency,
unknown adjacent aliases and native fault frames/atomicity are unverified.
Original assets are immutable; native experiments use fingerprinted copies under
build/runtime. The focused DLL/native host are validation artifacts, not a
playable rebuilt game. Next recover the surface-rebuild routine RVA0x5BD70,
whose historical Close label also requires semantic correction, then the real
input/graphics/WinMain dependencies. MCP access remains pending reconnection.
