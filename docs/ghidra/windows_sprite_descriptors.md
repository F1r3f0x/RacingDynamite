# Windows Sprite Descriptor Lifecycle Routines

Authenticated standard `IGN_WIN.EXE` SHA256:
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.

This subsystem encompasses the authentic sprite descriptor lifecycle, allocation, copy,
and cleanup routines in `decomp/src/geputget.c` under `graphics_legacy` module ownership:

1. `Gfx_SpriteCreateDescriptorNative` (RVA `0x5C920`, VA `0x0045C920`):
   - Extent: 24 bytes through RET `0x0045C937`
   - SHA256: `fd5cdd32124a607730e33cc3717641031757e7524d4bc8252ea6d7a0fe506b0e`
   - FPO: `(24, 0, 2, 0)`, 8 trailing padding bytes (`0xCC * 8`)
   - Relocations: None
   - Calls: `0x00461210` (`Gfx_CreateSpriteDescriptor`)
   - Semantics: cdecl `(GfxSpriteDescriptor *descriptor, int image_id) -> int`, calls `Gfx_CreateSpriteDescriptor(descriptor, image_id)` and returns 1 in EAX.

2. `Gfx_SpriteFreeDescriptorNative` (RVA `0x5C940`, VA `0x0045C940`):
   - Extent: 19 bytes through RET `0x0045C952`
   - SHA256: `6fbd0f1c63dd28502523b19906d18ca89eee90c5df9045d4c2471ca7147d7795`
   - FPO: `(19, 0, 1, 0)`, 13 trailing padding bytes (`0xCC * 13`)
   - Relocations: None
   - Calls: `0x004614B0` (`Gfx_FreeSpriteDescriptor`)
   - Semantics: cdecl `(GfxSpriteDescriptor *descriptor) -> int`, calls `Gfx_FreeSpriteDescriptor(descriptor)` and returns 1 in EAX.

3. `Gfx_SpriteCopyDescriptorNative` (RVA `0x5C960`, VA `0x0045C960`):
   - Extent: 24 bytes through RET `0x0045C977`
   - SHA256: `7f21b8803e5134922b22aefa7896ec5d43d3aa997dddb3ad20939b8639a7cfc4`
   - FPO: `(24, 0, 2, 0)`, 8 trailing padding bytes (`0xCC * 8`)
   - Relocations: None
   - Calls: `0x004612E0` (`Gfx_CopySpriteDescriptor`)
   - Semantics: cdecl `(GfxSpriteDescriptor *descriptor, int image_id) -> int`, calls `Gfx_CopySpriteDescriptor(descriptor, image_id)` and returns 1 in EAX.

4. `Gfx_CreateSpriteDescriptor` (RVA `0x61210`, VA `0x00461210`):
   - Extent: 143 bytes through RET `0x0046129E`
   - SHA256: `d1ca418b53f65abb20e0ec43adeb04905cbd9b247a7c5c188818a512b3620dc9`
   - FPO: `(143, 1, 2, 5133)`, 1 trailing padding byte (`0xCC * 1`)
   - Relocations: 4 (`0x61221` -> `0x520388`, `0x61228` -> `0x52038C`, `0x61235` -> `0x52038C`, `0x61249` -> `0x51FB88`)
   - Calls: `0x00460480` (`Gfx_CopyAllocatedString`), `0x0045F4A0` (`Gfx_AllocBytes`), `0x004612A0` (`Gfx_CopySpriteDescriptorPixels`)
   - Semantics: cdecl `(GfxSpriteDescriptor *descriptor, int image_id) -> void`. Looks up `image_id` in `g_nativeImageControl` (`0x520380`); if `image_id <= 0`, `image_id >= capacity`, or `records[image_id] == 0`, falls back to `g_nativeSpriteDefault` (`0x51FB88`). Copies 16 dwords from source, duplicates string name via `Gfx_CopyAllocatedString`, sets `words[6] = words[0]`, allocates `width * height` pixel bytes via `Gfx_AllocBytes`, sets `words[4] = pixels`, sets `words[5] = stride`, copies pixel grid via `Gfx_CopySpriteDescriptorPixels`, and sets `words[7] = words[4]`. Leaves EAX incidental.

5. `Gfx_FreeSpriteDescriptor` (RVA `0x614B0`, VA `0x004614B0`):
   - Extent: 31 bytes through RET `0x004614CE`
   - SHA256: `4b278fe6701f014a83c9ddb65ba7d69c2ca3c12814bedc44e9766dcd98408493`
   - FPO: `(31, 0, 1, 257)`, 1 trailing padding byte (`0xCC * 1`)
   - Relocations: None
   - Calls: `0x0045F8B0` (`Gfx_FreeBytes`) twice
   - Semantics: cdecl `(GfxSpriteDescriptor *descriptor) -> void`, frees `descriptor->words[6]` and `descriptor->words[7]` via `Gfx_FreeBytes`.

6. `Gfx_SpriteReservedNative` (RVA `0x61A80`, VA `0x00461A80`):
   - Extent: 3 bytes through RET `0x00461A82`
   - SHA256: `4bc724f3b1d0caf4fe369c18cba3102e6c4ea057f63fe1587e3973134a7f755e`
   - FPO: `(3, 0, 1, 0)`, 13 trailing padding bytes (`0xCC * 13`)
   - Relocations: None
   - Calls: None
   - Semantics: cdecl `(unsigned int option) -> int`, sets EAX = 0 and returns.

## Verification Evidence

- `tools/verify_matching.py`: Enforces call-target count assertions and byte-level disassemblies for all 6 routines.
- `tools/verify_sprite_descriptors.py`: Differential Unicorn emulator session comparing `IGN_WIN.EXE` and `sprite_descriptors_validation.dll`.
  - 288 total comparisons executed across all 6 routines.
  - Verified exact cdecl conventions, register preservation (`EBX`, `ESI`, `EDI`, `EBP`), direction flag zeroing, and stack restoration.
  - Validated memory allocation sequence (`malloc`) and pointer management (`free`) under multiple record geometries and dimensions.
  - Validated edge cases: `image_id = 0`, negative `image_id`, `image_id >= capacity`, null record entries, custom dimensions (1x1, 3x2, 6x4, 8x8), zero-length pixel buffers, and null pointers.
