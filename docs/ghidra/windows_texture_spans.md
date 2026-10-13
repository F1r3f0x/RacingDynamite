# Windows texture-span reconstruction

Authenticated standard IGN_WIN.EXE, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
VA = 0x400000 + RVA. Original source filenames remain unknown. Ownership was
declared before reconstruction in the directly compiled renderer module.

| Routine | RVA | Bytes / FPO | SHA-256 |
| --- | --- | --- | --- |
| Gfx_SetTriangleSpanState | 0x5C990 | 24 / (24,0,4,257) | `bc5e0b937897780cefd8d87d3b95fff67e75ef98d1a91d60e0d2ceb368f0b9b8` |
| Gfx_SetTriangleSpanCore | 0x67F1C | 38 / no FPO | `d7b77c5b6446f23ae6be7ab2cb3e5034585cfe71362cf792222dd3003f3ed05c` |
| Gfx_DrawTriangleSpan | 0x5C9B0 | 32 / (32,0,5,771) | `e3dbc55544ab3db0f77135a83fac72ff4ba1293984655e31b72723eb1be5bf0d` |
| Gfx_DrawTriangleSpanCore | 0x67F42 | 85 / no FPO | `2d40bc4ff2b77b85bdba551897c8fc3ad8cf7437670fa2bff0afaa59aa827309` |
| Gfx_DrawClippedTriangleSpan | 0x5C9D0 | 28 / (28,0,4,771) | `fa88078708ae1fc6bd627bf2fd66459e95f5595bcc828ec82e108d4d7b2bf31e` |
| Gfx_DrawClippedTriangleSpanCore | 0x67F97 | 53 / no FPO | `b5ca9a8fa2762fafec0f0b81c28c95a7136d797e931800496e82aa6d7607fbab` |

Ghidra disassembly corroborates bounded local PE decoding. The active authentic
program is the user-provided operating assumption; local hashes/FPO authenticate
routine provenance independently. Existing [triangle analysis](windows_triangle.md)
establishes actual callers and complete rendering effects without substituted
dependencies. An additional 64 original-only wrapper executions independently
check 768 ordered writes and texture/framebuffer/lookup access order before C work.

Setup takes four cdecl words (gradient V, gradient U, texture word, lookup word).
It stores lookup first, texture next, then the byte step and packed step. Negations
wrap at 32 bits. Byte step is low8((-U)>>16); packed step combines low16(-U)
as its high half with low16((-V)>>8). Only the low byte at VA 0x4BAD60 changes;
upper three bytes persist. Other state words are at 0x4BAD64/68/6C/70 and begin
as authenticated file-backed zeros. No texture/table/framebuffer initialization
is invented by this module.

Ordinary span takes destination, count, packed UV, integer U and left edge record.
The record comprises V/U/X dwords; this leaf reads only the low V and U bytes.
It samples the left texture byte before testing count and writes only the low
byte at 0x4BAD70. Then it captures texture addressing, byte/packed steps and lookup.
The clipped span takes the first four arguments and omits left sampling entirely.
Texture and lookup low16 address bits are replaced, rather than added to the base.

Both loops decrement count with 32-bit wrapping, scan right to left and carry
the packed-coordinate addition into the texture low byte. Ordinary access order
is destination, texture, lookup, store; clipped order is texture, destination,
lookup, store. For original nonnegative count, ordinary adds the cached left
pixel at destination-1; count zero therefore still composites that byte. Negative
ordinary counts still sample the left texture but omit framebuffer writes.
INT_MIN decrement wraps to a large positive count and can fault; no guard fixes it.
Cached left sample is reread after the loop, preserving live alias effects.

Complete production C89 leaves and wrappers compile directly from renderer.c.
No unresolved register callee or synthetic return is linked. DEV-007 explicitly
adapts the three internal register interfaces to typed C parameters. Original
cdecl wrapper arguments and preservation remain the externally verified boundary.
Incidental EAX/ECX/EDX and flags have no established semantic output contract and
are not claimed equal; no handwritten assembly is used. No bug fix or toggle is
introduced. The original compiler/link configuration remains unresolved.

`uv run tools/verify_texture_spans.py` passes 896 differential calls, 320 persistent
followups and 74 actual fault executions. Real original wrappers/leaves and real
compiled helpers execute. Ordered non-stack accesses/stores, all mapped arena
bytes, all five state words, unchanged unrelated image storage, return/ESP/caller
stack, ordinary cdecl preserved registers and cleared direction flag are checked.
Register leaves use authentic original register inputs and documented C inputs;
their adaptation is not disguised as identical ABI. Random steps/carries,
unaligned base words, upper scratch bytes, zero/negative/wrapped counts, invalid
left/destination/texture/table pointers and texture/table/destination overlaps
are covered. Static audit pins all six extents/hashes, FPO tuples and zero initial
scratch; the DLL has no imports and exactly three calls to the real compiled cores.

Actual Win32 native validation passes 64 paired cases and 12,582,912 compared buffer
bytes. It maps a checked disposable original PE with existing HIGHLOW relocation
logic, calls its real cdecl wrappers/register leaves and the complete C89 DLL,
uses aligned owned buffers with unaligned input words, and compares every byte
of texture/table/framebuffer plus scratch/retained upper bytes. Counts -3/-1/0/
1/2/5/17/64, four gradient pairs, both paths and overlapping buffers are exercised.
Original imports are not called by these bounded render routines. Native invalid
pointers, flags/register equality, arbitrary state/image aliases and concurrent
reentry remain unverified.

Full 103-routine renewal, fidelity audit and synchronized exports pass at snapshot
`b30e9897223a1e6d750525cef47ee3a8b420bb245eb8954cc870139c191e6ba0`, with 103 fresh compilation/emulation and 42 scoped native records. All 48 tracking,
ownership and workflow regression tests pass. LF/index checks and tracked gates
are required before commit.
These bounded pixel routines do not establish complete triangle or sprite rendering,
native startup, menus, input, audio or races. Next reconstruct real triangle/edge
dependencies with the same explicit internal C boundary, then integrate original
surface/viewport/texture state. The playable-game goal remains active.
