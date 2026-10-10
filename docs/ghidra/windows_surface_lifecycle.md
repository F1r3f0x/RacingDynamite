# Next native surface lifecycle preparation

Authentic IGN_WIN.EXE fingerprint verified. SurfaceOpen RVA5B740 n1575
FPO(1575,37,0,0x140A), SHA4626e6dbe7c24d021c37141e72e616e63bc43ebdd89cec16ea45012cf98acdc9.
Primary dependencies: surface-record initializer RVA56A40 n172; real Win32
CreateWindowExA/GetSystemMetrics/UpdateWindow/SetFocus/GDI/window sizing; real
DirectDrawCreate through thunk VA47879C ->IAT64C29C, then COM vtables;
restore-lost surfaces RVA5C730 n179. Full instructions, imports, Ghidra responses
and other close/reset bodies retained in surface-open-evidence.json. Ghidra has
missing function definitions; local fingerprint/FPO/hash/calls and dispatch agree.

Surface record initializer clears all12 words per48-byte record, type tag word10:
one type0 record VA50E778; five type1 records VA50E688;20 type2 records VA50E7A8;
returns1. Primary active word0, restore marker word1, type word10, COM surface
pointer word11 are corroborated by restore consumer. Remaining words2..9 require
native consumer validation before semantic naming. Separate compiler arrays must
not be assumed adjacent; represent each verified owned bank independently.

Restore VA45C730 (actual Gfx_SurfaceRestoreNative dispatch) tests active==1.
For primary then five bank1 and20 bank2 records, call actual surface IsLost
(vtable+0x60), compare exactly HRESULT0x887601C2; if equal, capture lost count,
reread live surface pointer, call Restore(vtable+0x6C), ignore HRESULT, then
write restore marker1. Return0 if any lost record processed, else1. Preserve
live callback effects, unchecked/null faults and ignored Restore failure; no
invented success-returning production COM boundary.

Open first invokes real record initialization. It calls CreateWindowExA with
exstyle0x40000, class/title literal Ignition (VAs493740/493778), style0x80080000,
origin0, screen-metric dimensions, parent/menu0, live instance VA4C5398, parameter0.
Publishes HWND VA4C539C then duplicate VA493738; failure returns0 without rollback.
UpdateWindow and SetFocus use successive live HWND reads. DirectDrawCreate(NULL,
&global VA512C50,NULL) follows; nonzero HRESULT returns0 with earlier HWND retained.
Cooperative flags53 for nonzero mode VA493728 or8 for zero; subsequent full
1575-byte path requires hand recovery from authenticated instructions, not raw
Ghidra output or invented UI. Register actual class/WndProc and HWND lifecycle
from verified callers before native Open execution. Return/no-op surface configure
RVA5B730 n6 returns2 for four ignored cdecl words.

No production/tracking promotion yet. Goal requires complete native window/
DirectDraw lifecycle and eventually real menus/input/audio/races; isolated modeled
COM/Win32 comparisons will not certify native startup or a playable game.

## Reconstructed lifecycle dependencies

Production C89 now implements the record initializer, constant configure routine
and restoration consumer at RVAs 0x56A40, 0x5B730 and 0x5C730. Their verified
extents are 172, 6 and 179 bytes; SHA-256 values are respectively
`053404ce23bc0313a565a6807a6bff041db95d72b1231fda8e5fdbe5467f2d65`,
`7140f35dee6220b79b12aecc27acf5105bf3b77d1588e89fce345de7c16c72b7`,
`b5fbe03e6aab008cadf961db34a86c9d25525993c52b3a60dbe72a09591d6f71`.
FPO confirms no arguments for initialization/restoration and four caller-cleanup
words for configure. All names/source hints are inferred, not recovered symbols.
The installer at VA 0x45B690 publishes configure/restoration addresses through
relocated immediates at RVAs 0x5B696 and 0x5B718. Original bytes are authenticated
again by the permanent verifier. Ghidra xrefs agree with dispatch ownership;
missing function definitions do not supply decompiled behavior.

`uv run python tools/verify_surface_lifecycle.py` freshly builds actual production
bodies and checks them against authentic x86 execution and an independent raw
address structural oracle. Initialization has 48 comparisons, configure 48,
restoration 1,160: 570 persistent follow-ups, 108 object/vtable faults and 106
mutation calls. Every slot, exact active equality, four IsLost results, ignored
Restore failure, multiple records sharing an object, all-lost banks and pointer/
active/marker changes during callbacks are covered. Ordered record reads/writes,
COM this arguments and entry snapshots, complete banks/arena/unrelated image,
caller bytes, nonvolatile registers, DF and stdcall cleanup are compared.
Initializer/restore/reinitializer sequences use real bodies on persistent state.

`build_decomp.py` extracts the restoration body from geputget.c into a strict C89
GCC 16.2 i386 SDK translation unit (`-m32 -std=c89 -pedantic-errors -Wall -Wextra
-Werror -O2`). The existing Clang/LLD builder links its fresh object with the other
production objects; three custom font/file builders use the same real object.
Compile-time layout assertions verify the 48-byte record, surface pointer at 44,
and actual SDK IsLost/Restore vtable offsets 0x60/0x6C. No production COM stub is
used. Emulator callbacks explicitly model only COM results/clobbers/mutations.
The original compiler/version and complete executable link configuration remain
unresolved; this provisional build does not establish instruction equality.

Native DirectDraw/window startup, rendering, menus, input, audio, races and native
fault frames/atomicity remain unverified. Arbitrary reentry/concurrency and unknown
adjacent global aliases are outside this contract. Eight record words remain
opaque until their consumers are authenticated. The focused DLL remains a
validation artifact. No native game parity or playable build is claimed.

Next recover complete Open/Close/Reset with real Win32/DirectDraw dependencies.
Original WinMain RVA 0x120A0 (390 bytes, stdcall four words) registers the Ignition
class with actual window procedure VA 0x4122D0. Recover that procedure and startup
caller before certifying native class/window integration. The constructor's
remaining instructions and all failure branches still require hand recovery.

## Constructor integration (2026-10-10)

[Constructor](windows_surface_open.md) independently identifies record words
+28/+32/+36 as width/height/bit depth. Five words remain opaque; stride and
initializer order stay unchanged. Production Open executes real initialization/
restoration and actual SDK APIs. Native evidence is limited to the constructor's
windowed scenarios with an authenticated original WndProc reference boundary.
The prior 1,256 dependency comparisons are retained. Original shutdown/rebuild,
complete native startup and game behavior remain pending.
