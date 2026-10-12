# Authentic native surface Clear and entry

Target: standard `Ignition/Ignition/IGN_WIN.EXE`, SHA256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`,
PE32 i386 image base0x400000. Ghidra MCP responds; loaded target identity is the
user-provided operating assumption. Local PE/FPO/instructions/callers independently
authenticate the following extents, rather than adopting DOS addresses.

| Routine | RVA / VA | Bytes | FPO (size,locals,args,flags) | SHA256 |
| --- | --- | --- | --- | --- |
| Gfx_SurfaceClearNative | 0x5C3D0 / 0x45C3D0 |210|210,0,1,775|dd3bd132e2b814763aab3ab5f58c6b162ddf69bb41a46f98e2cdd5a9b87df8d6|
| Gfx_ClearSurfaceNative | 0x56BB0 / 0x456BB0 |15|15,0,1,0|652dfcd5dbaf40c20c1d2f5745962ddee365aa37d367a97cba92f986ba2e2685|

Both use ordinary C calling convention, one48-byte record pointer, EAX boolean
return and caller stack cleanup. The wrapper calls slotVA0x50EB84; the authentic
installer RVA0x5B690 binds that slot to Clear. Caller RVA0x3B10 clears type1 first
recordVA0x50E688 through this wrapper and then calls presentation entry0x56C90.
Presentation is still an unresolved real dependency, rather than a success stub.

Clear checks active!=0 and captures mode@12. Old0 locks write-only mode3; old1
unlocks then locks3; other old modes proceed directly. Pixel base@20 is captured
once. Signed width@28 and height@32 are reread at each loop comparison; pitch@24
is reread for each write. The instructions atVA0x45C43C..0x45C445 multiply raw
pitch by row, add column and scale the result by4 for a DWORD zero store.
`DDSURFACEDESC.lPitch` is a byte count. The authentic code nevertheless uses it
as a DWORD index; production preserves this observed bug, with no pitch division.
The mode is reread after writes; if changed, unlock and restore the captured
nonzero mode, returning0 on any failed dependency. Stale pointers/pitch remain as
specified by real reconstructed Lock/Unlock. SDK calls use the real48-byte record
layout, not opaque DOS offsets. Volatile accesses preserve live geometry/pitch and
alias-visible order. No new deviation or invented original defaults are introduced.

`uv run tools/verify_surface_clear.py` authenticates hashes/FPO and builds strict
C89 with the current provisional GCC SDK + Clang/LLD memory boundary. It executes
2,996 differential comparisons:1,966 body and1,030 installed-table entry, with
1,964 persistent follow-ups, five observed faults, eight executed SDK mutations
and24 pixel/record alias comparisons. Cases include26 banks, old modes0/1/2/3/4/-1,
zero/negative dimensions, live SDK record changes, failing HRESULTs, null/unmapped
pixels, positive/zero/negative bounded pitch and field aliases. The independent
raw-record oracle compares ordered reads/writes, SDK calls and state at calls,
all records/arena, preserved registers, return/stack, caller bytes and unrelated
image bytes. Only verified pixel-pointer and installed-target relocations are
normalized; the initial alias test detected and corrected a harness relocation
error rather than suppressing state/write comparisons.

The native `SURFACE_TEST_CLEAR` host runs actual original and C constructors,
installer, Clear/body or entry, Lock and Unlock. Sixteen clears cover all four old
modes in paired windowed1/4 backbuffer scenarios. Test-owned8x8 record dimensions
stay within actual640x480 offscreen allocation; the first32 pitch rows are seeded
0xA5, then every byte is checked for the exact DWORD-scaled zero pattern and
unchanged remaining bytes. Original mode/locked flag are restored. Eight calls
use each body/entry path. Original WndProc is a reference boundary and teardown
uses the already reconstructed real surface shutdown.

This bounded offscreen test does not certify native full-frame rendering. The
original byte-pitch indexing can exceed allocation with full dimensions; native
fault behavior/atomicity is unresolved. Failed Lock without descriptor outputs,
arbitrary aliasing/reentry/concurrency, fullscreen presentation and original
compiler/ABI configuration remain explicit limitations. The validation DLL and
host are not a playable rebuilt game; startup, menus, rendering pipeline, input,
audio and representative races still require complete native behavioral validation.

Full renewal, fidelity audit and generated exports pass at snapshot `21647812d76d`
for all 86 reconstructions, with 25 scoped native records. LF working/index bytes,
staged gate and tracked hooks follow. Unchanged Lock RVA0x5C530 is included in
the commit trailer because the gate detects adjacent annotation changes; its
implementation and existing differential/native coverage are freshly renewed.
