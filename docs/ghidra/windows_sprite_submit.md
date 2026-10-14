# Native sprite submission adapter — reconstruction and verification

Recovered 2026-10-14 from the authenticated standard Windows release,
`Ignition/Ignition/IGN_WIN.EXE`, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`,
915,968 bytes, PE32 x86, preferred image base `0x00400000`.

## Subsystem placement and boundaries

- Module: `renderer` (`decomp/src/renderer.c`, `decomp/include/renderer.h`, `decomp/modules.json`)
- Routine name: `Gfx_SubmitSpriteRequest`
- VA: `0x00457370` / RVA `0x57370`, extent `[0x00457370, 0x00457417)`, 167 bytes
- Routine SHA-256: `203cc8648be172994ad229e423e9f05c83ff3a5d541277de7e9864ac47ae79fe`
- FPO: `(167, 16, 0, 267)`
- Padding: 9 trailing `0xCC` INT3 bytes (`0x00457417..0x00457420`)
- ABI: `x86 no stack arguments; 64 local bytes; preserves ESI; consumes active request; calls rasterizer with packet in ESI; no semantic return`
- Fidelity: `ADAPTED` (`@deviation DEV-007`)
- Evidence: `docs/ghidra/windows_sprite_submit.md`
- Verifier: `tools/verify_sprite_submit.py`

## Binary and relocation analysis

`Gfx_SubmitSpriteRequest` contains fifteen authenticated HIGHLOW base relocations:
- `0x57374 → 0x50EBFC` (`g_activeSpriteRequest`)
- `0x5737D → 0x4BA77C` (`g_spriteRasterPacket.position`)
- `0x57386 → 0x4BA784` (`g_spriteRasterPacket.coefficients`)
- `0x573A2 → 0x4BA758` (`g_spritePreparedStorage[0]`, origin_x)
- `0x573B6 → 0x4BA75C` (`g_spritePreparedStorage[1]`, origin_y)
- `0x573BE → 0x4BA760` (`g_spritePreparedStorage[2]`, (word4 & 0xFF) << 8)
- `0x573CA → 0x4BA764` (`g_spritePreparedStorage[3]`, word4 & 0xFF00)
- `0x573DD → 0x4BA760` (`g_spritePreparedStorage[2]`)
- `0x573E3 → 0x4BA770` (`g_spritePreparedStorage[6]`, word4 & 0xFFFF0000)
- `0x573E8 → 0x4BA768` (`g_spritePreparedStorage[4]`, (word1 << 8) + prepared[2])
- `0x573F5 → 0x4BA764` (`g_spritePreparedStorage[3]`)
- `0x573FA → 0x4BA76C` (`g_spritePreparedStorage[5]`, (word2 << 8) + prepared[3])
- `0x573FF → 0x63F2D8` (`g_spriteRenderLookup`)
- `0x57404 → 0x4BA774` (`g_spritePreparedStorage[7]`, render lookup table)
- `0x57409 → 0x4BA778` (`g_spriteRasterPacket`)

Two internal function calls:
1. `call 0x004612E0`: `Gfx_CopySpriteDescriptor` (`decomp/src/geputget.c`) with arguments `(local_descriptor, handle->image_id)`.
2. `call 0x00465BB5`: `Gfx_RasterizeSpritePacket` (`decomp/src/renderer.c`) with packet in register ESI.

## Reconstructed implementation

The function is reconstructed in C89 in `decomp/src/renderer.c`:

```c
volatile unsigned int g_spritePreparedStorage[8] = {0, 0, 0, 0, 0, 0, 0, 0}; /* VA 0x004BA758 */
volatile GfxSpriteRasterPacket g_spriteRasterPacket = {
    4u,
    (const GfxPoint2D *)0,
    (const unsigned int *)g_spritePreparedStorage,
    (const int *)0
}; /* VA 0x004BA778 */
volatile unsigned int g_spriteRenderLookup = 0; /* VA 0x0063F2D8 */

/*
 * @original Gfx_SubmitSpriteRequest (IGN_WIN.EXE @ 0x00457370, unknown original source)
 * @fidelity ADAPTED
 * @deviation DEV-007
 */
void Gfx_SubmitSpriteRequest(void)
{
    GfxSpriteDescriptor local_descriptor;
    volatile GfxSpriteRequest *request = g_activeSpriteRequest;
    GfxSpriteHandle *handle;
    unsigned int word4;

    g_spriteRasterPacket.position = (const GfxPoint2D *)request->position;
    g_spriteRasterPacket.coefficients = (const int *)request->coefficients;
    handle = request->handle;

    Gfx_CopySpriteDescriptor(&local_descriptor, handle->image_id);

    g_spritePreparedStorage[0] = (unsigned int)handle->origin_x;
    word4 = local_descriptor.words[4];
    g_spritePreparedStorage[1] = (unsigned int)handle->origin_y;
    g_spritePreparedStorage[2] = (word4 & 0xFFu) << 8;
    g_spritePreparedStorage[3] = word4 & 0xFF00u;
    g_spritePreparedStorage[6] = word4 & 0xFFFF0000u;
    g_spritePreparedStorage[4] = (local_descriptor.words[1] << 8) + g_spritePreparedStorage[2];
    g_spritePreparedStorage[5] = (local_descriptor.words[2] << 8) + g_spritePreparedStorage[3];
    g_spritePreparedStorage[7] = g_spriteRenderLookup;

    Gfx_RasterizeSpritePacket((const GfxSpriteRasterPacket *)&g_spriteRasterPacket);
}
```

## Differential validation

`tools/verify_sprite_submit.py` tests 264 paired executions against original `IGN_WIN.EXE`:
1. **168 exhaustive adapter lookup trials**:
   - Covers all count, image ID, empty table slot, and active request mutation permutations.
   - Verifies packet dispatch, packet fields (`type=4`, `position`, `coefficients`, `prepared`), and all 8 words of `prepared` storage byte-for-byte.
2. **32 transformed sprite delegation trials**:
   - Verifies coefficient delegation from `Gfx_SubmitSpriteRequest` to `Gfx_RasterizeSpritePacket` and onto `Gfx_PrepareTransformedSprite`.
3. **64 full-pipeline untransformed blit trials**:
   - Verifies end-to-end execution through `Gfx_RasterizeSpritePacket` to `RET`.
   - Compares complete framebuffer pixel output (SHA-256 match), callee-saved registers (EBX, ESI, EDI, EBP), and stack pointer return state.

## Limitations

- DEV-007 adapts the register ESI packet argument to a typed C parameter inside the renderer module.
- Full 3D game scene compositing and multi-sprite display lists are certified separately.
