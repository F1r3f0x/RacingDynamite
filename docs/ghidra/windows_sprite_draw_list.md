# Windows Sprite Draw List Traversal and Triangle Submission

Authentic input: `Ignition/Ignition/IGN_WIN.EXE`, 915,968 bytes, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`,
PE32 i386, preferred base VA `0x00400000`. No original assets were changed;
no DOS executable supplied evidence. Names are semantic, not original symbols.
The user assumes this binary is active in Ghidra; this is distinct from local
fingerprint verification. Local FPO extents, Capstone disassembly, and Unicorn
differential emulation authenticate all three routines.

## Inventory and binary extents

| Routine | VA / RVA | Bytes; FPO | SHA-256 |
| --- | --- | --- | --- |
| `Gfx_SpriteDrawListNative` | 0x00457250 / 0x57250 | 96; (96,0,0,0) | `12d36bd728eefff1bcccf08ecd1e7a52c39ef5d1ce833f2baeee78a7ae52b50d` |
| `Gfx_SpriteDrawListCallback` | 0x004572B0 / 0x572B0 | 1; (1,0,0,0) | `ae3f4619b0413d70d3004b9131c3752153074e45725be13b9a148978895e359e` |
| `Gfx_SubmitDrawListTriangleRequest` | 0x004572C0 / 0x572C0 | 173; (173,0,0,0) | `a116b31c386c034e36c50180e8d4bf76dcde6d08064b825d0c5880373880e8de` |

The three routines occupy a contiguous extent in `.text` from `0x00457250` to `0x0045736D`,
directly preceding `Gfx_SubmitSpriteRequest` at `0x00457370`.
- `0x00457250` ends in `RET` at `0x004572AF`, immediately followed by `0x004572B0`.
- `0x004572B0` consists of a single `RET` instruction (`0xC3`), followed by 15 `INT3` (`0xCC`) padding bytes.
- `0x004572C0` ends in `RET` at `0x0045736C`, followed by 3 `INT3` (`0xCC`) padding bytes aligning to `0x00457370`.

## Relocations and callees

`Gfx_SpriteDrawListNative` has four HIGHLOW relocations:
- `0x00457275`: `0x0050EBFC` (`g_activeSpriteRequest`)
- `0x00457284`: `0x004BA704` (`g_triangleDrawListStride`)
- `0x0045728D`: `0x0045EBFC` (`g_activeSpriteRequest`)
- `0x00457293`: `0x0045EBFC` (`g_activeSpriteRequest`)
Direct calls inside `Gfx_SpriteDrawListNative`:
- `0x00457262`: `call 0x004572B0` (`Gfx_SpriteDrawListCallback`)
- `0x0045727E`: `call 0x004572C0` (`Gfx_SubmitDrawListTriangleRequest`)

`Gfx_SpriteDrawListCallback` has no relocations or downstream calls.

`Gfx_SubmitDrawListTriangleRequest` has 17 HIGHLOW relocations:
- Three references to `0x0050EBFC` (`g_activeSpriteRequest`) at `0x572C1`, `0x572F3`, `0x57311`.
- Direct pointer push of `0x004BA718` (`g_triangleWorkspacePacket`) at `0x5734C`.
- Thirteen direct stores into `g_triangleWorkspacePacket` offsets `+0x04` through `+0x34` (`0x004BA71C..0x004BA74C`).
Direct call inside `Gfx_SubmitDrawListTriangleRequest`:
- `0x00457364`: `call 0x0045D8A0` (`Gfx_RasterizeWorkspaceATriangle`).

## Execution semantics and data layout

### Draw list structure
The draw list is passed as `const unsigned int *list` (`[esp + 4]` in cdecl).
1. The list is composed of groups, terminated by a null dword (`0`).
2. Each group begins with a header dword, forwarded to `Gfx_SpriteDrawListCallback(header)`.
3. Following the header is a list of pointers to triangle request batches, terminated by a null dword (`0`).
4. For each batch pointer `batch`:
   - `g_activeSpriteRequest = batch;`
   - If `*batch != 0`:
     - In a `do-while` loop:
       - Calls `Gfx_SubmitDrawListTriangleRequest()`.
       - Reads stride `g_triangleDrawListStride` from `0x004BA704` (initial value 4).
       - Advances `g_activeSpriteRequest = (const char *)g_activeSpriteRequest + (stride << 2)` (16 bytes).
       - Continues while `*(const unsigned int *)g_activeSpriteRequest != 0`.
5. The loop skips the null delimiter and continues to the next group until the outer null terminator is encountered.
6. The function returns int `1`.

### Triangle request layout and packet unmarshaling
Each triangle request in a batch contains 4 dwords (pointers):
- `request[0]`: pointer to vertex 0 (`x`, `y` integers)
- `request[1]`: pointer to vertex 1 (`x`, `y` integers)
- `request[2]`: pointer to vertex 2 (`x`, `y` integers)
- `request[3]`: pointer to attribute block containing:
  - `attrs[0]`: texture base address
  - `attrs[2]`: texture coordinate `u0`
  - `attrs[3]`: texture coordinate `v0`
  - `attrs[4]`: texture coordinate `u1`
  - `attrs[5]`: texture coordinate `v1`
  - `attrs[6]`: texture coordinate `u2`
  - `attrs[7]`: texture coordinate `v2`

`Gfx_SubmitDrawListTriangleRequest` unpacks these values into the shared workspace packet at `0x004BA718`:
- `g_triangleWorkspacePacket[0]` (offset 0): initial type 18 (`0x12`)
- `g_triangleWorkspacePacket[1..2]` (offset 4, 8): vertex 0 `x`, `y`
- `g_triangleWorkspacePacket[3..4]` (offset 12, 16): vertex 1 `x`, `y`
- `g_triangleWorkspacePacket[5..6]` (offset 20, 24): vertex 2 `x`, `y`
- `g_triangleWorkspacePacket[7..8]` (offset 28, 32): `u0`, `v0`
- `g_triangleWorkspacePacket[9..10]` (offset 36, 40): `u1`, `v1`
- `g_triangleWorkspacePacket[11..12]` (offset 44, 48): `u2`, `v2`
- `g_triangleWorkspacePacket[13]` (offset 52): texture base pointer
It then pushes `0x004BA718` (`(const GfxTexturedTrianglePacket *)g_triangleWorkspacePacket`) and calls `Gfx_RasterizeWorkspaceATriangle`.

## Verification

The verification suite `tools/verify_sprite_draw_list.py` executes:
1. Exact hash, FPO, and relocation validation on `IGN_WIN.EXE`.
2. Compilation of production `decomp/src/renderer.c` and linking of `sprite_draw_list_validation.dll`.
3. Differential emulation comparing original executable instructions against compiled C:
   - 64 callback trials verifying return and register preservation.
   - 64 triangle submission trials verifying 14-dword packet layout and downstream call arguments.
   - 64 full draw list traversal trials across diverse nested trees verifying complete call sequences, return values, and state mutations.
