# Windows Workspace A triangle span rasterization and slope

Authenticated standard `Ignition/Ignition/IGN_WIN.EXE`, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
Base VA = 0x00400000. All original assets remain untouched.
Disassembly and instruction provenance verified against authentic executable.

## Candidate Summary

| Symbol | RVA | VA | Bytes / FPO | SHA-256 |
| --- | --- | --- | --- | --- |
| `Gfx_SetWorkspaceATriangleSpanState` | 0x5D7E0 | 0x0045D7E0 | 20 / (20,0,0,0) | `b44a68c97f162d2529a4c675b0b063d311c240ea0f7fee2b016b3b3edc667cd2` |
| `Gfx_SetWorkspaceATriangleSpanCore` | 0x68030 | 0x00468030 | 32 / no FPO | `7e0f2a9e97ff168747a5b4303cb0903be73a4b4658b9cdb95b40b0ec17dfd2d0` |
| `Gfx_DrawWorkspaceATriangleSpan` | 0x5D800 | 0x0045D800 | 32 / (32,0,0,0) | `9417fc7bfab5bfc310affbac2cb9e0f47ec3e6ebd52cb9a218d3873467d746a6` |
| `Gfx_DrawWorkspaceATriangleSpanCore` | 0x68050 | 0x00468050 | 118 / no FPO | `83ce90f853837fb3ba06d863c4a0411ffd61618d479fea50d8b4b788376cc632` |
| `Gfx_DrawWorkspaceAClippedTriangleSpan` | 0x5D820 | 0x0045D820 | 28 / (28,0,0,0) | `2b8f4fe6957fecf632ed99781d89d42316878a56a1fe65fed6c8cf20b81c1613` |
| `Gfx_DrawWorkspaceAClippedTriangleSpanCore` | 0x680C6 | 0x004680C6 | 92 / no FPO | `4a669ae06b40d9562996cbc52da881c4d85877b5194eee5836628ad447b75826` |
| `Gfx_ComputeWorkspaceATriangleSlope` | 0x5E5D0 | 0x0045E5D0 | 53 / (53,0,0,0) | `3a5ff8a33d3695a02677653fd016253eea225ac790ae027f3de712cf1f890890` |

## Architecture and Subsystem Role

These routines implement the unshaded / direct software triangle span rasterization
pipeline for Workspace A (`g_spriteWorkspaceABuffer0..2`). In contrast to Side B
(`0x5CA50` / `0x5D170` and `0x67F1C`..`0x67F97`), which composites textured spans
through a 256x256 shading/lighting lookup table (`g_triangleSpanLookupWord`), Side A
rasterizes texture samples directly to the destination framebuffer without table lookup.

### Span Setup
- `0x5D7E0` (`Gfx_SetWorkspaceATriangleSpanState`):
  cdecl wrapper taking 3 stack words: `gradient_v`, `gradient_u`, `texture_word`.
  Loads registers `EAX=gradient_v`, `EBX=gradient_u`, `ECX=texture_word` and calls `0x468030`.
- `0x68030` (`Gfx_SetWorkspaceATriangleSpanCore`):
  Computes negative gradients `-gradient_v` (shifted right by 8) and `-gradient_u`.
  Stores byte step into `0x004BAEFC` (low byte of rotated `-gradient_u`).
  Stores packed coordinate step into `0x004BAF00` (high word: low16 of `-gradient_u`, low word: low16 of `(-gradient_v) >> 8`).
  Stores texture base word into `0x004BAF04`.

### Ordinary Span Rasterization
- `0x5D800` (`Gfx_DrawWorkspaceATriangleSpan`):
  cdecl wrapper taking 5 stack words: `destination`, `count`, `packed_uv`, `integer_u`, `left`.
  Passes registers `EDI=destination`, `ESI=count`, `ECX=packed_uv`, `EBX=integer_u`, `EAX=left` and calls `0x468050`.
- `0x68050` (`Gfx_DrawWorkspaceATriangleSpanCore`):
  Samples left edge initial pixel from texture and caches it in `0x004BAF0C`.
  Processes 4 pixels per loop iteration into EAX using Pentium-pipelined AGI-free stepping:
  samples are unpacked into EAX, swapped via `bswap`, and written as a 32-bit dword to `[EDI + ESI]`.
  Residual pixels (1..3) are rasterized in a descending byte tail loop.
  If count was nonnegative, the cached left sample at `0x004BAF0C` is stored at `[EDI - 1]`.

### Clipped Span Rasterization
- `0x5D820` (`Gfx_DrawWorkspaceAClippedTriangleSpan`):
  cdecl wrapper taking 4 stack words: `destination`, `count`, `packed_uv`, `integer_u`.
  Passes registers `EDI=destination`, `ESI=count`, `ECX=packed_uv`, `EBX=integer_u` and calls `0x4680C6`.
- `0x680C6` (`Gfx_DrawWorkspaceAClippedTriangleSpanCore`):
  Identical 4-pixel unrolled direct texture rasterization to `0x468050`, omitting the initial left sample and final left sample store.

### Edge Slope Helper
- `0x5E5D0` (`Gfx_ComputeWorkspaceATriangleSlope`):
  cdecl helper computing `(x0 - x1) << 8 / (y0 - y1)`.
  Preserves authentic IDIV signed divide and `#DE` exception behavior.
  When `y0 == y1`, returns `0x80010000` if `x1 < x0` (`x0 > x1`), otherwise `0x7FFF0000`.
  Functionally identical to `0x5D7A0` (`Gfx_ComputeTriangleEdgeSlope`).

## Fidelity and ABI Adaptation (DEV-007)
Internal leaf core routines (`0x68030`, `0x68050`, `0x680C6`) use register parameters in the original executable. Following `DEV-007`, they are adapted to clean C89 functions with explicit typed arguments, called directly by their authentic cdecl wrappers.
