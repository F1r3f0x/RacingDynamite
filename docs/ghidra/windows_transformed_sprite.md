# Windows Transformed Sprite Preparation

Authenticated standard `IGN_WIN.EXE` SHA256:
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.

## Target Fingerprint and Provenance

- Symbol name: `Gfx_PrepareTransformedSprite` (analysis name; not a recovered linker symbol).
- Verified RVA: `0x00068C70` (VA `0x00468C70`).
- Extent: 631 bytes (`[0x00468C70, 0x00468EE7)`).
- SHA256: `c239f5eb8a12e6b59eb775b316f45e4d8c37ad81f2c0b66910cda5033f8f96b2`.
- Relocations: 68 base relocations within `[0x00068C70, 0x00068EE7)`, all targeting fields of the 116-byte scratch structure at `0x004BAF1C` (`0x004BAF1C` through `0x004BAF54`).
- Module ownership: `renderer` (`decomp/src/renderer.c`, `decomp/include/renderer.h`, `decomp/modules.json`).

## Architecture and Behavior

`Gfx_PrepareTransformedSprite` prepares rotated and scaled sprite quads for software rasterization:
1. **Input**: In the authentic binary, `ESI` points to a 20-byte input descriptor (`GfxTransformedSpriteInput`):
   - `+0x00`: unused DWORD.
   - `+0x04`: pointer to prepared sprite metrics array (`prepared[8]`, containing origin offsets `ox, oy`, UV bounds `u0, v0, u1, v1`, texture pointer, and lookup table pointer).
   - `+0x08`: pointer to 16.16 fixed-point transformation matrix coefficients (`c0, c1, c2, c3`).
   - `+0x0C`: integer world/screen translation `position_x`.
   - `+0x10`: integer world/screen translation `position_y`.
2. **Scratch Buffer**: Stores UV and texture parameters into global scratch buffer at `0x004BAF1C` (`GfxTransformedSpriteScratch`, 116 bytes):
   - `0x004BAF3C`: `param_u0 = prepared[3]`
   - `0x004BAF40`: `param_v0 = prepared[5] - 0x10`
   - `0x004BAF44`: `param_u1 = prepared[2]`
   - `0x004BAF48`: `param_v1 = prepared[4] - 0x10`
   - `0x004BAF4C`: `lookup = (unsigned char *)prepared[7]`
   - `0x004BAF50`: `texture = (unsigned char *)prepared[6]`
3. **Corner Transformation**: Rotates the negative origin offset `(-ox, -oy)` and span extents `dx = prepared[4] - 0x10 - prepared[2]`, `dy = prepared[5] - 0x10 - prepared[3]` using 16.16 fixed-point multiplication (`imul` / `mov ax, dx` / `rol eax, 16`):
   - `corner0 = R * (-ox, -oy) + (pos_x, pos_y)`
   - `corner1 = corner0 + R * (dx, 0)`
   - `corner2 = corner0 + R * (0, dy)`
   - `corner3 = corner0 + R * (dx, dy)`
   Stored at `0x004BAF1C`..`0x004BAF38`.
4. **Triangle Assembly and Submission**:
   Emits two 60-byte textured triangle packets (`GfxTexturedTrianglePacket`) at `0x004BAF54` and invokes `Gfx_RasterizeTexturedTriangle` (`0x0045CA50`) twice:
   - Triangle 1: Corners 0, 1, 2 with UV coordinates `(u1, u0)`, `(v1, u0)`, `(u1, v0)`.
   - Triangle 2: Corners 1, 2, 3 with UV coordinates `(v1, u0)`, `(u1, v0)`, `(v1, v0)`.

## Deviations and Fidelity

- **DEV-007**: Calling convention adaptation from register `ESI` to typed C89 function argument `const GfxTransformedSpriteInput *input`. Original binary preserves only `EDI`, clobbering `EBX`, `ESI`, and `EBP`; compiled standard C89 preserves all four callee-saved registers (`EBX, ESI, EDI, EBP`).
- Fidelity classification: `ADAPTED` (DEV-007).

## Verification

- `tools/verify_transformed_sprite.py` differentially executes `Gfx_PrepareTransformedSprite` against authentic `IGN_WIN.EXE` in Unicorn engine across 65 test cases (identity transforms, rotations, scale transforms, and randomized matrices/coordinates).
- Downstream `Gfx_RasterizeTexturedTriangle` calls intercepted via hook, asserting byte-for-byte identity of all 60-byte triangle packets and 116-byte scratch state.
- Compilation and emulation evidence recorded to SQLite database (`database/decomp.db`).
