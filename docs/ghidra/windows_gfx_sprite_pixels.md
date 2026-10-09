# Windows sprite descriptor pixel copying

Production reconstruction, 2026-10-09. This supersedes only the pixel-leaf
analysis-only limitation in [storage analysis](windows_gfx_sprite_packing_storage.md).
Storage RVA 0x61530 is now reconstructed and differentially validated; see
[the storage feature](windows_gfx_sprite_packing_storage.md). Earlier leaf-only
coverage and limitations below describe this preceding feature.

Authentic input: Ignition/Ignition/IGN_WIN.EXE, 915,968 bytes, SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782,
PE32 i386, preferred image base VA 0x00400000. The target doctor independently
authenticates local bytes. Read-only localhost:8080 disassembly of the leaf and
both callers, plus xrefs, agrees with independent local Capstone/FPO inspection.
The active Ghidra program is the user-provided IGN_WIN.EXE assumption, not an
automated identity check. Original binaries/assets remain untouched. Names and
geputget.c placement are semantic, not recovered original symbols. No DOS
evidence supplies this implementation.

## Boundaries and ABI

Gfx_CopySpriteDescriptorPixels: VA **0x004612A0**, RVA **0x612A0**, **61 bytes**;
FPO (61,0,2,0x1411), SHA-256
32a8a30b19b528630595c3efaab77bc2ff20798c4a133ae569091264cc784e5f.
Complete decoding ends at ordinary RET VA 0x004612DC, followed by three INT3
bytes. No calls, global operands, HIGHLOW operands or relocated entry pointers
are present. The complete FPO direct call/jump scan finds exactly calls at
VAs 0x00461289 and 0x0046167E; Ghidra xrefs agree. Computed references remain
possible. Caller RVA 0x61210 has a 143-byte FPO extent. It pushes EBX destination,
then EBP source; storage pushes its live descriptor, then local snapshot.
Both clean eight bytes. These are bounded caller inspections, not reconstructed
caller integration.

Two cdecl arguments: source descriptor first, destination descriptor second.
EAX retains source on normal return. Production returns that pointer solely to
retain the incidental machine value; it is not a success result. EBX/ESI/EDI/EBP,
caller stack and clear DF are preserved. Arithmetic flags/volatile registers
and fault-time compiler stack/register frames are not return contracts.

## Independently recovered ordered contract

Read source pixels (+0x10), then destination pixels (+0x10), before any bound.
Capture those row pointers once. Read source signed height (+8); nonpositive
height returns without reading widths, strides or pixel bytes. For each admitted
row, read signed source width (+4). Each admitted column reads one byte, stores
it immediately to destination, increments column and rereads width. Nonpositive
width skips bytes, but the admitted row still reads strides. Read source stride
(+0x14) and advance its captured row pointer, then read destination stride and
advance its pointer. Increment row and reread height. Destination dimensions
are never read. Row/column additions and addresses retain unsigned 32-bit
representations; dimensions compare signed dwords. There is no 256 upper bound.

The existing descriptor retains its independently established 64-byte footprint.
GfxSpritePixelView names the verified 24-byte prefix: opaque dword, signed width,
signed height, opaque dword, raw pixel address and raw stride. It does not rename
or guess the remaining words. Volatile view/byte accesses retain ordered live
reads even when pixel destinations overlap descriptor fields. Separate statements
preserve source-pointer-before-destination-pointer and source-stride-before-
destination-stride reads. Unsigned arithmetic retains wrapping without signed
overflow. The focused build checks observable x86 behavior of this view.

Forward copying is authentic: abcde copied into the following byte yields
aaaaaa across six bytes. No memcpy/memmove substitution, pointer validation,
overlap guard, dimension ceiling, allocation, cleanup or rollback is added.
Aliases can change later bounds/strides, while changed pixel-address words do
not replace the captured row pointers.

## Fresh compilation and differential evidence

Production C89 lives in decomp/src/geputget.c with declarations in geputget.h.
tools/build_decomp.py extracts the exact production body and exports it in the
focused PE32 validation DLL. Provisional Clang/LLD 19.1.1 targets
i686-pc-windows-msvc with -O2, -std=c89, -pedantic-errors, -Wall/-Wextra/-Werror,
freestanding/no-builtin, disabled inlining/unrolling/vectorization/SSE. No
assembly or success-returning production stubs are introduced. This is not
the established original compiler or linked game configuration.

`uv run python tools/verify_sprite_pixels.py` performs fresh compilation and
**659 comparisons per binary**, including **376 persistent follow-ups**,
**14 faults**, **62 live-alias calls** and **10 wrapping calls**. The real
aggregate `uv run tools/verify_matching.py` invokes this suite as well. Every
comparison independently derives expected raw byte state and ordered read/write
events from the instructions; emulator output never generates expectations.
Both authentic instructions and extracted compiled C must match the oracle.
Persistent sessions retain mapped data/image/CPU state between calls; the caller
reseeds volatile/nonvolatile inputs and supplies a fresh invocation frame.

Coverage includes signed zero/negative/high-bit bounds; positive width and height
257; distinct/zero/negative strides; forward/backward/exact pixel overlap;
identical/partially overlapping descriptors; byte writes to live width, height,
both strides and captured pixel fields; width/height expansion and shrinkage;
source pixels within descriptor fields; unsigned address wrap both within rows
and between rows; unmapped pointer/bound/stride reads and first/later pixel
read/write faults. An explicit independent check requires the aaaaaa result;
expansion fixtures require three/four stores respectively. Random small images
add 96 three-call sequences. No CRT, callback or dependency is modeled or called
inside this leaf.

All mapped data bytes, unrelated image bytes, caller-stack bytes and ordered
accesses are checked. Normal completion also checks EAX, ESP, nonvolatile
registers and clear DF. Faults check kind/address/size and preceding events/state;
earlier destination stores persist. They do not receive normal-return ABI claims.
The oracle bounds its fixtures externally, without production guards.

The full completion workflow renews compilation/differential evidence for all
43 reconstructed routines among 1,028 candidates. Prior coverage is retained:
bucket 2,734 (1,140 persistent, 127 faults, two cycle prefixes), page 1,574
(816 persistent, 134 real-dependency, six faults), wrapper/aligned 48/512,
gap 1,835 (922 persistent, 69 faults, two cycle prefixes), link 864 (464
persistent), reset 810 (405 repeats, 922 real bodies), default 280 (132
repeats, 392 real bodies), descriptor helper 486 (1,156 real bodies), handles
48 (654 real bodies), each workspace 167 (83 repeats, 757 real bodies),
installers 306/266, selector 532, banner/input 306/266, startup/shutdown
112/519, 18 integrated startups and ten persistent lifecycle calls. Existing
memory/font/file/backend suites remain. The modeled primitive driver runs real
default/reset bodies in 112 startups; real primitive RVA 0x5E610 is not executed.

## Completion and limits

PowerShell sets UV_CACHE_DIR=build/uv-cache and UV_OFFLINE=1. Record the candidate
with db.py describe/set-status. Before verification, check that recorded text
inputs use LF bytes matching the intended Git blobs: Windows core.autocrlf can
leave a clean checkout with CRLF working bytes. A later normalization requires
fresh applicable validation. Then run:

`uv run python tools/workflow.py complete --rva 0x612A0 --limitation "Pixel leaf differential validation only; storage callers unreconstructed; no instruction equality, original compiler/link layout or native graphics/game/fault parity"`.

Completion runs the real verifier, fidelity audit, synchronized exports and
export check. Review LF working/index input hashes, stage explicit feature files,
pass workflow.py check --staged and tracked hooks, then commit with RVA trailer.

Instruction equality, original compiler/link layout, native graphics/game/fault
parity and playable rebuilding remain unverified. Invalid pointer accesses and
cross-object views are observed in the stated provisional x86 build, not certified
portable C guarantees. Caller-stack/image pixel aliases, arbitrary concurrent
mutation, huge live bounds and exhaustive alias combinations are not certified.
Storage RVA 0x61530, release RVA 0x618B0 and workspace renderers remain
unreconstructed. Storage is the next separate production feature and must execute
this real leaf together with the real packing dependencies.
