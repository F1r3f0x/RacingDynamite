# Authentic native palette copy and installed entry

Target: standard IGN_WIN.EXE SHA256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`,
PE32 i386 image base0x400000. Local PE/FPO/instructions/callers/loader data
independently authenticate this feature. Ghidra MCP responds; its loaded target is
the user-provided assumption. Ghidra identifies the entry as Video_SetPalette;
the body is not defined there, so its local FPO/instruction extent supplies evidence.

| Routine | RVA / VA | Bytes | FPO (size,locals,args,flags) | SHA256 |
| --- | --- | --- | --- | --- |
| Gfx_SurfaceSetPaletteNative |0x5C6D0 / 0x45C6D0|83|83,0,1,0|cce59efd58c21d910a12512b15c5641b586e6beac7a4fd5daad80a56c991f5f0|
| Gfx_SetPaletteNative |0x56C40 / 0x456C40|15|15,0,1,0|2f73ffb25305361147dd76513ce9b4125182485565ad60410532f9cc6f60f308|

Ordinary C convention, one const RGB-triplet byte pointer, caller cleanup, full EAX
boolean return. The installer0x5B690 binds slotVA0x50EB98 to the body; the entry
calls that slot. Caller0x3B10 passes RGB bufferVA0x4BEB38 to this entry before
Clear/presentation. The body first checks palette pointerVA0x512448 for zero and
returns0 without reading RGB on that path. It reads/stores three bytes in order
for each of256 entries, advancing source3/destination4. Bytes0/1/2 are red/green/
blue, byte3 is retained. It reloads the palette pointer after copying, then calls
SDK SetEntries at vtable0x18 with flags0,start0,count256,entry-bank pointer.
Only HRESULT0 returns1. No color scaling, gamma adjustment, copied flag byte or
second null check is introduced.

The SetPalette bankVA0x512450 is separate from constructor bank0x512048 and rebuild
bank0x512850; each is1024 bytes. These banks, palette pointer and dispatch slot
are loader-zero .data virtual-tail storage, beyond raw data endRVA0xBCE00 and
within virtual end0x24BF80. SDK PALETTEENTRY has verified four-byte RGB/flags layout.
The production storage uses its actual SDK type in the SDK implementation unit;
freestanding units do not need that declaration in their header. The initial
header exposure failed strict compilation and was removed before validation.

Before implementation,1,376 original-only independent-oracle comparisons and actual SDK
palette creation/update/readback authenticated the contract. Production C89 uses
volatile byte reads/stores to retain interleaved alias-visible order and reloads
the live interface. The nonreturning build fixture is replaced by the real body.
No DOS implementation or modified original asset supplies production behavior.

`uv run tools/verify_surface_palette.py` compiles the production body/entry and
checks1,424 differential comparisons:1,036 body/388 entry,768 persistent follow-ups,
268 faults and504 input/output alias comparisons. It checks constructor/rebuild
input banks while preserving them, retained randomized flags, null guard paths,
HRESULTs, unmapped inputs/interfaces and eight post-copy interface-change cases.
Assertions require72 normal mapped copies with768 source-byte reads and a real
SetEntries boundary per copy. Each of eight post-copy changes must trigger in the
oracle and both executing images; valid replacements must reach the new interface,
while null/unmapped replacements must fault after the copy.
An independent raw-offset oracle compares ordered byte/DWORD accesses, API arguments
and snapshots, every record/arena and all three palette banks, preserved registers,
caller stack/return and unrelated image bytes. Only verified dispatch/data-bank
relocations are normalized. SDK outputs and injected read changes are modeled.

The shared native-loader host now accepts a helper and insertion site rather than
duplicating loader/platform import logic. Native palette helper executes actual
paired windowed0/1/4 constructors, CreatePalette(8BIT|ALLOW256), original/C89 body
and real installer/entry, then GetEntries checks all256 RGB entries after each of
24 updates. Forty-eight null guards and retained shadow flags0 are checked.
Test-owned palettes are released and the global restored before actual shutdown.
Original WndProc remains a reference boundary. The helper-builder refactor also
renews presentation RVAs0x5C4B0/0x56C90: its unchanged C bodies still pass eight
actual paired fullscreen flips/2,457,600 primary-pixel checks and display restoration.

SDK palette-object updates do not prove complete attached-palette rendering or
production drawing. Arbitrary aliases/reentry, native fault atomicity, full startup,
menus/input/audio/races and original compiler/link configuration remain unresolved.
The validation DLL/host are not a playable rebuilt game. No instruction equality
or native game parity is claimed.

Full renewal, fidelity audit and exports pass at snapshot `166b80e3a1ac`
for all 90 reconstructions, with 29 scoped native records. LF working/index bytes,
staged gates and tracked hooks follow. Continue real pixel blitting/copying/rendering and
startup/window-message dependencies after this separate feature commit.


## Mapped-source oracle correction (2026-10-12)

The original verifier allocated a0x4000-byte arena but used ordinary RGB source
at offset0x5000. Those comparisons really tested unmapped-input faults; the eight
intended post-copy pointer changes never reached their trigger. Previously recorded
runs were actual execution results, but their stated normal-source/mutation scope
was too broad. Bank-alias comparisons and24 actual native SDK updates were valid.
This repair places768 source bytes at mapped offset0x3000, asserts the allocation
bound and successful normal-copy paths, and counts each executed pointer-change
hook in both images. The corrected scope above supersedes the old claim. Production
C89 and native helpers are unchanged. No validation result or fidelity label alone
is treated as evidence that an intended path ran.

Full workflow renewal, fidelity audit and exports pass at snapshot
`c7c1048073f99c8342805ea00271840cd6970eb241c60f9c9ec1a33e2e53d43e`
for 90 reconstructions and 29 scoped native records. LF working/index checks,
staged gate and tracked hooks follow for this validation repair. The retained graphics_legacy module is an
existing approved exception; this changes its oracle rather than adding a new
subsystem or claiming the original source filename.
