# Native Win32/DirectDraw constructor

Authentic IGN_WIN.EXE SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`,
PE32 i386, image base 0x400000. Constructor VA 0x45B740 / RVA 0x5B740,
extent 1,575 bytes, FPO (1575,37,0,0x140A), caller-cleanup no arguments.
Routine SHA-256 `4626e6dbe7c24d021c37141e72e616e63bc43ebdd89cec16ea45012cf98acdc9`.
Local fingerprint/FPO/instructions/imports/relocations and surface dispatch agree;
Ghidra is reachable under the user-provided active-program assumption, with a
missing constructor definition. The disassembler, not guessed decompiler output,
supplies behavior. Constructor calls real initialization RVA 0x56A40 and real
restoration RVA 0x5C730. Production C89 now builds through actual Win32/DirectDraw
SDK methods and freshly generated i386 import libraries; 242 differential comparisons and four paired native scenarios now pass.
The full workflow must renew existing coverage before completing this feature.

Record words at +28/+32/+36 receive configured width/height/bit depth in both
constructor paths. Those independently named fields replace three formerly
opaque words, preserving the 48-byte layout and initializer store order. Five
other words remain opaque. SDK assertions check offsets and 108-byte
DDSURFACEDESC. Configured defaults are 640,480,8,1 at VAs 0x4BA6E0..0x4BA6EC;
fullscreen VA 0x493728 starts at 1, desktop depth VA 0x4BAC90 at 8. HWND/instance
backing belongs in main.c/main.h (VAs 0x4C539C/0x4C5398), with independently
verified class/title strings "Ignition". Legacy main/race bodies remain untrusted
and are excluded from this build. No DOS entrypoint is built or maintained.

The constructor preserves failure without rollback, live mode/configuration
reads, windowed fixed 640x480 offscreen surfaces, attached fullscreen chain,
palette flags retained while RGB becomes black/white, and the unusual successful
return on failed SetClipper before restoration/ShowWindow. Error texts are copied
from authenticated null-terminated original data, including "The maximum amount
of backbuffers is exceeded". No production API success fixture supplies behavior.

Original WinMain RVA 0x120A0 registers an Ignition class using actual WndProc RVA
0x122D0. Its 400-byte FPO extent includes a five-entry jump table (last 20 bytes);
SHA-256 `461e5b4afef4dd9642c7ce031556749db188c41ea06480eab871723b337452e0`.
The stdcall four-word procedure depends on input init/shutdown RVAs 0x55AC0/
0x55E20, graphics wrappers 0x56BC0/0x56BF0 and shutdown bridge 0x12550, which
require their own real dependency reconstruction. Its complete production
reconstruction is deferred while useful constructor work proceeds. A native
constructor probe may use the authenticated original procedure as an explicit
reference boundary; that cannot certify a rebuilt window procedure or full game.

## Validation scope and supported commands

`uv run python tools/verify_surface_open.py` freshly builds production C89 and
passes 242 comparisons: 108 persistent follow-ups, four unchecked interface faults
and 37 actually executed callback mutation calls. Both fullscreen and windowed
paths, counts -1/0/1/2/4/5, exact HRESULT/ignored BOOL behavior, API/COM failures
with retained earlier publications, SetClipper's failure-success path, palette
RGB/flag preservation, live mode/window/configuration changes and wrapping device/
rectangle arithmetic are checked. An independent raw-address structural oracle
compares ordered reads/writes, API arguments, 108-byte descriptor snapshots, all
known banks/globals/palette/arena/unrelated image, caller bytes and cdecl/stdcall/
nonvolatile/DF behavior. The real initializer and restoration bodies execute.
Modeled Win32/COM boundaries belong only to the differential harness.

`tools/native_surface_probe.c` builds a strict C89 PE32 native host. A fingerprinted
copy under `build/runtime/surface_native/` supplies original executable instructions,
PE sections, HIGHLOW relocations and actual USER32/GDI32/DDRAW imports. Both the
original and rebuilt constructor execute actual APIs and COM methods. Four paired
scenarios cover unregistered-class failure and registered windowed 0/1/4 backbuffers.
The probe checks return, HWND publication/visibility, 640x480 client geometry,
window style, complete record state, real primary/offscreen surface descriptors,
clipper HWND and primary attachment. It registers the independently authenticated
original class/WndProc as an explicit reference boundary. Its switching guard
prevents entry into unrebuilt game/input/shutdown code. Test-owned SDK cleanup
cannot certify production shutdown. Original assets are never modified.

The SDK unit uses GCC 16.2 i386 strict C89, with Clang/LLD for existing units and
LLVM dlltool to generate fresh i386 USER32/GDI32/DDRAW import libraries using
verified stdcall argument counts. Three custom font/file builders link the same
actual SDK object/imports. `windows_tracking.py` persists the builder's main.c/
main.h transitive inputs, so platform backing changes invalidate results. Legacy
DOS main/race bodies are outside this extracted native build and remain untrusted;
no DOS build is maintained. A focused DLL/host is not the rebuilt game.

Fullscreen native mode switching, real lost-device hardware, a rebuilt window
procedure, native startup/menus/rendering/input/audio/races and playable game
remain unverified. Arbitrary reentry/concurrency, out-of-bank count growth,
unknown adjacent aliases and native fault frames/atomicity are excluded. Original
compiler/link configuration and instruction equality remain unresolved.
Next reconstruct native display resource shutdown/rebuild through actual SDK
Release/Create methods, then their input/graphics dependencies and WinMain/WndProc.
