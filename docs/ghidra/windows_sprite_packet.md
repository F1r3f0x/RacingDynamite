# Windows Sprite Packet Rasterizer

Authenticated standard `IGN_WIN.EXE` SHA256:
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.

## Target Fingerprint and Provenance

- Symbol name: `Gfx_RasterizeSpritePacket` (analysis name; not a recovered linker symbol).
- Verified RVA: `0x00065BB5` (VA `0x00465BB5`).
- Extent: 474 bytes (`[0x00465BB5, 0x00465D8F)`).
- SHA256: `2cdb543878a2ce01c034f50d8f679a881f1491d088a64746b18ce6273d464fbb`.
- Relocations: 31 base relocations within RVA `[0x00065BB5, 0x00065D8F)`, targeting:
  - `0x004BAD28`: `g_spriteBlitSource` (texture source pixel pointer)
  - `0x004BAD2C`: `g_spriteBlitWidth` (clipped blit width)
  - `0x004BAD30`: `g_spriteBlitHeight` (clipped blit height)
  - `0x004BAD34`..`0x004BAD44`: `g_transformedSpriteInput` (input descriptor for transformed sprite prep)
  - `0x0063F2B4`: `g_triangleViewportState.clip_bottom_integer`
  - `0x0063F2B8`: `g_triangleViewportState.clip_top_integer`
  - `0x0063F2BC`: `g_triangleViewportState.clip_right_integer`
  - `0x0063F2C0`: `g_triangleViewportState.clip_left_integer`
  - `0x0063F2C8`: `g_triangleViewportState.row_stride`
  - `0x0063F2D4`: `g_triangleViewportFramebuffer`
- Module ownership: `renderer` (`decomp/src/renderer.c`, `decomp/include/renderer.h`, `decomp/modules.json`).

## Architecture and Behavior

`Gfx_RasterizeSpritePacket` consumes a 16-byte sprite rasterization packet (`GfxSpriteRasterPacket`):
- `+0x00`: unused DWORD.
- `+0x04`: pointer to `GfxPoint2D position` (`pos->x`, `pos->y` in 8.8 fixed-point).
- `+0x08`: pointer to `prepared[8]` sprite descriptor array.
- `+0x0C`: pointer to 16.16 transformation matrix coefficients, or `NULL` if untransformed.

The routine executes one of two paths:
1. **Transformed Sprite Path (`coefficients != NULL`)**:
   Populates `g_transformedSpriteInput` at `0x004BAD34` with:
   - `prepared = packet->prepared`
   - `coefficients = packet->coefficients`
   - `position_x = packet->position->x`
   - `position_y = packet->position->y`
   Then invokes `Gfx_PrepareTransformedSprite(&g_transformedSpriteInput)` (`0x00468C70`).
2. **Untransformed Translucent Blit Path (`coefficients == NULL`)**:
   - Computes initial source texture offset:
     `v0 = prepared[3]`, `u0 = prepared[2]`.
     `source = texture + ((v0 & 0xFFFFFF00) | ((u0 >> 8) & 255))` (wrapped uint32 addition)
   - Computes sprite dimensions:
     `width = SAR32(prepared[4], 8) - SAR32(prepared[2], 8)`
     `height = SAR32(prepared[5], 8) - SAR32(prepared[3], 8)`
   - Computes screen coordinates:
     `screen_x = (packet->position->x - prepared[0]) >> 8`
     `screen_y = (packet->position->y - prepared[1]) >> 8`
   - Performs viewport clipping:
     - Top clip (`screen_y < clip_top`): adjusts `height`, advances `source` by `clip_dy * 256`, clamps `screen_y`.
     - Bottom clip (`screen_y + height > clip_bottom`): reduces `height`.
     - Left clip (`screen_x < clip_left`): adjusts `width`, advances `source` by `clip_dx`, clamps `screen_x`.
     - Right clip (`screen_x + width > clip_right`): reduces `width`.
     Only an executed clipping subtraction can take its signed nonpositive exit. There is no unconditional width/height guard. Zero/negative width skips pixels; the row counter always decrements and continues until zero, including initial zero/negative height.
   - Computes destination framebuffer pointer:
     `dest = framebuffer + screen_y * row_stride + screen_x`
   - Composites translucent pixels row by row using the color lookup table (`prepared[7]`):
     `dest[x] = *(byte *)((lookup & 0xFFFF0000) | (src[x] << 8) | dest[x])`
     Advanced by 256 bytes per source row and `row_stride` per destination row.

## Deviations and Fidelity

- **DEV-007**: Calling convention adaptation from register `ESI` to typed C89 function argument `const GfxSpriteRasterPacket *packet`. Original binary wraps body in `pushal` / `popal`, preserving all registers; compiled standard C89 preserves all callee-saved registers (`EBX, ESI, EDI, EBP`).
- Fidelity classification: `ADAPTED` (DEV-007).

## Independent review and corrected implementation (2026-10-14)

Local Capstone disassembly and live Ghidra `disassemble_function` at VA
`0x00465BB5` agree. Ghidra target identity is the user-provided operating
assumption; the local target/routine hashes are independently authenticated.
The routine's data at VA `0x004BAD28..0x004BAD47` are zero initialized in
file-backed `.data`; viewport/framebuffer storage belongs to the verified renderer.

The previously staged scalar draft passed 64 ordinary cases but failed two new
original/C comparisons: `prepared[3]=0x120000` lost its high bits in the source
pointer, and `prepared[2]=0xFFFFFF00` produced width `0xFF000002` instead of 2.
Both are fixed by authenticated byte replacement and signed shifts. Full-word
position/dimension arithmetic uses unsigned wrapping with signed comparisons.

The blitter prefetches two lookup addresses and accumulates four colors before
one dword store, then emits descending byte tails. Volatile byte/dword accesses
preserve the observed reads, store widths and order under source/frame overlap.
The table's low WORD is replaced, not added. The stride and width are reloaded
at each row boundary; DEC/JNE row continuation and clipping-only exits remain.
Transformed inputs are written in original order: position X/Y, prepared pointer,
coefficients pointer; input word zero remains unchanged. C uses a typed packet
argument in place of ESI as documented by DEV-007. Unsigned-to-signed conversion,
arithmetic signed right shift and unaligned x86 dword stores are explicit target
assumptions of the provisional compiler, not portable-C or original-codegen claims.

## Verification and limitations

`uv run tools/verify_sprite_packet.py` compiles the full production renderer in
C89 mode using the existing GCC/LLD provisional validation configuration. It
checks the original 474-byte hash, all 31 relocations, the single call target,
compiled imports and the shared renderer's symbolic call identities.

352 paired original/C executions retain all 64 original fixtures and add:

- Full high-word V offsets and signed U dimensions on rejected sprites.
- Widths 1..33 with source/frame overlaps -5, -1, 0, +1, +5 bytes.
- The same widths with lookup low-WORD offsets 1, 255 and 65535.
- Zero/negative widths and zero/negative heights, with eight actual unmapped
  access faults compared by access kind, address and width.

334 untransformed cases compare the complete 512 KiB arena, scratch globals,
ordered arena reads (including values), ordered stores (including widths/values)
and fault prefixes. 18 transformed cases compare ordered input reads and the
20-byte delegation packet. Globals begin with deterministic nonzero poison;
untouched bytes, image state, caller stack and preserved registers are checked
on returning calls. Original preserves all PUSHAD registers; C guarantees its
cdecl callee-saved set through DEV-007.

The transformed callee is intercepted in this focused contract, despite being
linked as a real production dependency. Whole packet-to-triangle/native game
integration, all possible image aliases, native fault handling and nonterminating
zero-width/zero-height DEC/JNE paths remain unvalidated. No instruction equality,
original compiler identity, native packet rendering or playable-game claim is made.
Full workflow audit/export and staged gates must pass before feature completion.
