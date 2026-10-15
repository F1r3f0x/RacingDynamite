# Windows Workspace A ordinary and clipped textured triangle parents

Authenticated standard IGN_WIN.EXE SHA256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
Analysis names `Gfx_RasterizeWorkspaceATriangle` and `Gfx_RasterizeWorkspaceAClippedTriangle`
are not recovered linker symbols. RVA 0x5D8A0 / VA 0x45D8A0, 1805 bytes through
RET 0x45DFAC, hash `78d4b4b04194828bc6b6683a6890568822a230369f947edda82cb6222c4640c4`.
RVA 0x5DFB0 / VA 0x45DFB0, 1561 bytes through RET 0x45E5C8, hash
`739bf60339f6915a8460f79fdaeb474af0426aab014e2b386bd74c2e37ac433e`.
Local pinned bytes, FPO extents, and Ghidra disassembly authenticate both complete entries.
Original source filenames and original compiler toolchain remain unknown.

Both routines are cdecl with one 60-byte packet pointer, preserving EBX/ESI/EDI/EBP.
Original EAX is incidental. The outer parent copies vertex coordinates and UV pairs into
verified 48-byte geometry scratch (`g_triangleVertices` at 0x004BAD74), stores texture pointer
into `g_triangleWorkspaceARasterTexture` (0x0051AA48), initializes frame cursor
`g_triangleWorkspaceARasterCursor` (0x0051AA54) from `g_triangleFramebuffer` (0x0063F2D4),
computes signed Xmin/max before sorting (`g_triangleWorkspaceARasterMinimumX` at 0x0051AA58,
`g_triangleWorkspaceARasterMaximumX` at 0x0051AA78), and executes the authentic Y0/Y2, Y0/Y1,
else Y2/Y1 compare/swap order. Equal-Y ties preserve original vertex order.
Four signed viewport tests route to the authentic clipped parent; GCC merges those equivalent
call sites in the compiled output.

The clipped parent consumes prepared sorted scratch and presort X-range; it is not an
independent packet decoder. Strict outside rejections preserve boundary equality. It compares
BC/AB slopes using `Gfx_ComputeWorkspaceATriangleSlope` (0x5E5D0); the outer parent compares
AC/AB slopes. Both execute authentic gradient computation (`Gfx_ComputeTriangleGradients`,
0x64EA80), span state setup (`Gfx_SetWorkspaceATriangleSpanState`, 0x5D7E0), slope helper,
edge-wrapper, and span bodies. Three buffer slots are borrowed from the verified Workspace A
initializer RVA 0x5D840 (`g_spriteWorkspaceABuffer0/1/2` at 0x0051AA5C, 0x0051AA60, 0x0051AA64).

New parent-owned mutable edge packets at 0x0051AA28 (`g_triangleWorkspaceAOrdinaryPacket`, 32 bytes)
and 0x0051AA80 (`g_triangleWorkspaceAClippedPacket`, 32 bytes) are loader-zeroed.
Shared vertex, gradient, and viewport backing are reused. Source placement is `decomp/src/renderer.c`,
with explicit module ownership preflight under `renderer`. DEV-007 retains public cdecl
while adapting internal original register-only helpers to their verified typed C interfaces.

Unlike Workspace B spans, which composite through a 256x256 lookup table, Workspace A spans
rasterize unshaded texture pixels directly to the destination framebuffer using
`Gfx_DrawWorkspaceATriangleSpan` (0x5D800) and `Gfx_DrawWorkspaceAClippedTriangleSpan` (0x5D820).
Accordingly, `Gfx_SetWorkspaceATriangleSpanState` (0x5D7E0) accepts three arguments
(`gradient_v`, `gradient_u`, `texture_word`), omitting the 4th lookup argument and reducing
both parent functions by 8 bytes compared to their Workspace B counterparts.
Row processing and edge traversal retain the exact two-edge assignment and upper/lower halves.
Frame starts from raw long/short header startY times live stride. Empty or reversed spans
retain original helper semantics and cursor advancement.
