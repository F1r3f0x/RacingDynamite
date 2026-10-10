# Native surface rebuild

Authentic IGN_WIN.EXE VA0x45BD70 / RVA0x5BD70, FPO(745,29,0,0x140E),
SHA-256 `9f9272e9f5f76e0318c38fc925e2bc4d03b304c169f0daf740b995b6ec055745`.
Previously authenticated instructions/imports/Ghidra responses and caller/dispatch
bindings establish this body. MCP remains offline pending the user's reopening;
no fresh Ghidra response is fabricated. The historical Close slot at VA0x50EB70
actually rebuilds surfaces, so production names now use Rebuild.

Nonzero live mode releases primary if its captured pointer is nonnull, then zeros
pointer/active. Twenty type2 surfaces always use that release/clear path. Type1
surfaces are not released. A live nonzero mode releases/zeros palette; the real
initializer then clears/retypes every record. Nonzero mode sets configured display
mode, creates primary flipchain, publishes metadata and retrieves attached buffers
with first BACKBUFFER and later FLIP caps. Count>=5 or failed attached retrieval
shows the original message and returns0, preserving earlier state. Zero mode
skips primary/type1 creation after clearing, retaining old interfaces outside
records without releasing them. This unusual ownership behavior is authentic.

The palette tail uses a separate verified buffer VA0x512850, not Open's0x512048.
Live nonzero mode may release a callback-installed palette again, sets RGB black
for entry0/white for1..255 preserving flags, creates/attaches palette, then calls
real restore (result ignored), shows the live HWND and returns1. HRESULT failures
retain prior state; no rollback, window creation or destruction is invented.

Actual SDK C89 executes with real initializer/restoration and resource dependencies.
The production dispatch installer now names the previously mislabeled slot Rebuild;
its verified address is unchanged. No production success fixture supplies behavior.

`uv run python tools/verify_surface_rebuild.py` passes 360 comparisons, including
164 persistent follow-ups, 43 unchecked faults and22 executed mutation calls.
An independent raw-address oracle checks release/clear/init/display/create/metadata/
palette/restore/show ordering, actual arguments and108-byte descriptors, every
record slot and both palette buffers, full known globals/arena/unrelated image,
caller bytes, nonvolatile/DF and cdecl/stdcall cleanup. Counts -1/0/1/2/4/5, exact
HRESULT failures, shared/sparse ownership, live mode/count/configuration/palette
changes, retained flags and persistent calls are covered. SDK replies/clobbers
are modeled only in differential emulation. The generic execution harness now
accepts per-session field specifications so the separate buffer is tracked without
assuming linker adjacency or altering constructor/shutdown contracts.

The `SURFACE_TEST_REBUILD` native host runs actual original/C constructors followed
by two real rebuilds and actual production shutdown for each windowed count0/1/4:
three paired scenarios, six constructor paths and12 rebuild invocations. Type2
boundary interfaces are created through actual SDK setup. Rebuild clears/retypes
all records, retains HWND/DirectDraw/clipper, and does not release windowed primary/
type1 interfaces: test-held references still answer real surface descriptor queries.
Test cleanup releases only interfaces intentionally lost from records; production
shutdown handles the remaining global resources, then test code disposes HWND/class.
The authenticated original WndProc remains a reference boundary. Constructor and
shutdown native variants remain separately verified; missing-class failure remains
constructor/shutdown coverage, not a rebuild scenario with no valid driver.

Native fullscreen/display switching/palette/lost-device behavior, a rebuilt WndProc,
complete game/startup/menu/render/input/audio/races, original compiler/link identity
and instruction equality remain unverified. Unknown adjacent aliases, out-of-bank
count growth, arbitrary reentry/concurrency and native fault frames/atomicity are
excluded. Original assets remain untouched; runtime copies are fingerprinted.
The focused DLL/host are not a playable rebuilt game. Next recover the real sprite
shutdown and graphics wrappers required by WinMain/WndProc, preserving the original
architecture and the standard Windows target.
