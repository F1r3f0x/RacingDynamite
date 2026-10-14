# Windows Sprite Configuration and Viewport Setters

Authenticated standard `IGN_WIN.EXE` SHA256:
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.

This subsystem configures 2D sprite clipping, surface metrics, and dual-writes the 3D triangle rasterizer viewport state.

## Routines and Binary Provenance

All six routines reside consecutively in `IGN_WIN.EXE` within `.text` (VA `0x00456F40` - `0x004571AF`):

1. **`Gfx_SpriteCloseNative`**
   - RVA: `0x00056F40` (VA: `0x00456F40`)
   - Extent: 6 bytes (`mov eax, 1; ret`)
   - SHA256: `2db31f4e09597946e56e859813bfdad7046d457c32332c3a882654d1e3ffdf67`
   - FPO: `(6, 0, 0, 0)`
   - Padding: 10 bytes `0xCC` through `0x00456F4F`

2. **`Gfx_SpriteOptionNative`**
   - RVA: `0x00056F50` (VA: `0x00456F50`)
   - Extent: 3 bytes (`xor eax, eax; ret`)
   - SHA256: `4bc724f3b1d0caf4fe369c18cba3102e6c4ea057f63fe1587e3973134a7f755e`
   - FPO: `(3, 0, 1, 0)`
   - Padding: 13 bytes `0xCC` through `0x00456F5F`

3. **`Gfx_SpriteConfigureNative`**
   - RVA: `0x00056F60` (VA: `0x00456F60`)
   - Extent: 187 bytes through RET `0x0045701A`
   - SHA256: `b9bcf21802d579d076091d7908499681c374a5786d4b5c46ce5d981135950c93`
   - FPO: `(187, 0, 5, 5142)`
   - Relocations: 24 base relocations (HIGHLOW)
   - Padding: 5 bytes `0xCC` through `0x0045701F`

4. **`Gfx_SpriteSetClipNative`**
   - RVA: `0x00057020` (VA: `0x00457020`)
   - Extent: 225 bytes through RET `0x00457100`
   - SHA256: `223c2cd20c7f79c950d9fc173d170037f4bde1047b285142be51fe551173dbcb`
   - FPO: `(225, 0, 4, 5130)`
   - Relocations: 18 base relocations (HIGHLOW)
   - Padding: 15 bytes `0xCC` through `0x0045710F`

5. **`Gfx_SpriteGetStateNative`**
   - RVA: `0x00057110` (VA: `0x00457110`)
   - Extent: 92 bytes through RET `0x0045716B`
   - SHA256: `9be413481f65ac603e7873f46b10cff7a09f8de06c3a0904b100e8b4f77a18ae`
   - FPO: `(92, 0, 1, 0)`
   - Relocations: 9 base relocations (HIGHLOW)
   - Padding: 4 bytes `0xCC` through `0x0045716F`

6. **`Gfx_SpriteSetStateNative`**
   - RVA: `0x00057170` (VA: `0x00457170`)
   - Extent: 63 bytes through RET `0x004571AE`
   - SHA256: `635cc8612df0e75ae8e7760faa6cd17b0730605c52c19d15184a89158f2092c7`
   - FPO: `(63, 0, 1, 257)`
   - Callee calls: `Gfx_SpriteConfigureNative` (`0x00456F60`) and `Gfx_SpriteSetClipNative` (`0x00457020`)
   - Padding: 1 byte `0xCC` at `0x004571AF`

## State Coupling and Dual-Write Architecture

`Gfx_SpriteConfigureNative` and `Gfx_SpriteSetClipNative` synchronize two graphics subsystems:
1. **2D Sprite Clip State**:
   - `g_spritePixels` (VA `0x004BA6F8`): frame buffer pointer
   - `g_spriteStride` (VA `0x0050EBF4`): byte stride per scanline
   - `g_spriteWidth` (VA `0x0050ED2C`): surface width
   - `g_spriteHeight` (VA `0x0050ED38`): surface height
   - `g_spriteConfigOption` (VA `0x0050ED34`): configuration flag word
   - `g_spriteClipLeft` (VA `0x0050ED20`): integer clip left
   - `g_spriteClipLeftFixed` (VA `0x0050EBF0`): 24.8 fixed-point clip left (`left << 8`)
   - `g_spriteClipTop` (VA `0x0050ED24`): integer clip top
   - `g_spriteClipTopFixed` (VA `0x0050EC00`): 24.8 fixed-point clip top (`top << 8`)
   - `g_spriteClipRight` (VA `0x0050ED28`): integer clip right
   - `g_spriteClipRightFixed` (VA `0x0050EBF8`): 24.8 fixed-point clip right (`right << 8`)
   - `g_spriteClipBottom` (VA `0x0050ED30`): integer clip bottom
   - `g_spriteClipBottomFixed` (VA `0x0050EC04`): 24.8 fixed-point clip bottom (`bottom << 8`)

2. **3D Triangle Viewport State**:
   - `g_triangleViewportHeight` (VA `0x0063F2AC`): viewport height
   - `g_triangleViewportFramebuffer` (VA `0x0063F2D4`): render target pointer
   - `g_triangleViewportState` (VA `0x0063F2B0` - `0x0063F2D3`, 36 bytes):
     - `clip_top_fixed` (+0): 8.8 fixed clip top
     - `clip_bottom_integer` (+4): integer clip bottom
     - `clip_top_integer` (+8): integer clip top
     - `clip_right_integer` (+12): integer clip right
     - `clip_left_integer` (+16): integer clip left
     - `clip_bottom_fixed` (+20): 8.8 fixed clip bottom
     - `row_stride` (+24): scanline stride
     - `clip_left_fixed` (+28): 8.8 fixed clip left
     - `clip_right_fixed` (+32): 8.8 fixed clip right

## Bounds Enforced by Gfx_SpriteSetClipNative

The authentic Windows function validates:
- `left >= 0`
- `top >= 0`
- `right > left`
- `bottom > top`
- `right <= g_spriteWidth`
- `bottom <= g_spriteHeight`

If any check fails, the function returns 0 immediately without mutating any state. On success, it updates all clip fields in both 2D sprite and 3D triangle viewport state and returns 1.

## GfxSpriteState Layout (40 bytes)

Recovered from `Gfx_SpriteGetStateNative` and `Gfx_SpriteSetStateNative`:
```c
typedef struct GfxSpriteState {
    int width;             /* +0x00 */
    int height;            /* +0x04 */
    unsigned int option;   /* +0x08 */
    int clip_left;         /* +0x0C */
    int clip_top;          /* +0x10 */
    int clip_right;        /* +0x14 */
    int clip_bottom;       /* +0x18 */
    int stride;            /* +0x1C */
    int format;            /* +0x20 (constant 3) */
    unsigned char *pixels; /* +0x24 */
} GfxSpriteState;
```

## Direct production placement and validation

The six routines and their state are reconstructed in directly compiled C89
`decomp/src/renderer.c`, with declarations in `renderer.h`. The legacy graphics
header imports this boundary; no duplicate production bodies or harness viewport
backing remain. Both the focused renderer DLL and shared validation builders
compile the complete renderer module and link its real helper calls. Original
source filenames remain unknown. The old initial extraction placement was corrected
before completion. Existing per-word state names preserve separately verified
addresses; no enclosing original aggregate is inferred from adjacency.

Signed left shifts were replaced with explicit unsigned-word shifts and signed
bit-pattern stores, preserving actual x86 wrapping for negative/full-word inputs.
Getter reads width,height,option,stride; publishes format before stride, then
pixels before clip fields, in authenticated instruction order. Setter captures
option,height,width,stride,pixels before the real configure call; after it, reads
bottom,right,top,left before the real clip call. It returns1 even if clipping
rejects. No safer validation or partial-state rollback is invented.

The local getter hash/FPO extent and complete decoding authenticate its bytes;
Ghidra currently has no function definition at that entry. This does not block
inspection: the live setter disassembly corroborates its configure/clip calls.
No automatic loaded-program fingerprint is inferred from endpoint reachability.

`uv run tools/verify_viewport_setters.py` passes2,448 whole original/C calls:
128 close,128 option,256 configure,1,152 clip,392 getter and392 setter.
1,792 persistent followups and14 actual null/truncated-record faults are included.
Exact ordered non-stack reads/writes, all16 named state objects, complete arena,
unchanged unrelated image/caller arguments, normal return/ESP/nonvolatile regs/DF
and actual fault prefixes compare. Real configure/clip bodies run for setter;
no callee effects are modeled. The linked renderer has no imports and its21 direct
calls target only verified production routines. The shared matching guard passes
with the same direct module; unrelated established boundaries remain explicit.

Earlier independent original-only evidence includes128 configure and1,152 clip
calls, with exact24/16-store sequences, ordered guard reads and ABI checks.
Production cases cover full-word/negative dimensions, wrapped bounds, all guard
rejections, get/set persistent roundtrips, random state records and pointer faults.
Arbitrary image/global/stack aliases, concurrency, flags/fault stack/SEH and native
triangle/game validation remain open. No native game or original compiler
claim is made. Full120 compilation/differential renewal, fidelity audit and exports pass at
snapshot `d76b20a5975120220a0cca3142514924345529541e7d860b99eff731513abe80`, with49 scoped native records.
All48 tracking/ownership/workflow tests pass. LF/index and staged gates/hooks
are required before commit. RVA0x67F1C has unchanged body with source-context renewal.

Paired native Windows validation now passes448 top-level original/C89 calls
(896 executions), plus actual nested configure/clip helper execution. Close/
option/configure/clip/setter have64 paired cases each; getter128. All16 state
objects and40byte records compare after configuration, clipping and state
restoration, including invalid clips and negative configuration width. The host
reads a fingerprinted disposable original PE, maps/relocates it as executable
reference code without game startup/import calls, and loads the production DLL.
This mapping changes loader/NX conditions; it does not prove original game
startup, rendered frames, fault/SEH atomicity or menus/input/audio/races.
`tools/native_viewport_probe.c` and the checked existing PE mapper are hashed
inputs; runtime products remain ignored under build/runtime/viewport_setters_native.

The first full renewal exposed older font cleanup/parser/file-load harnesses
that did not link the newly migrated renderer. All three now compile/link the
same whole module; their focused existing coverage passes. Shared result recording
automatically includes renderer source/header and the export-contract module
for every build_decomp user. The meaningful transitive-provenance test verifies
that a renderer edit invalidates an otherwise unchanged shared-builder result.
The failed attempt is not reported as completion; final full renewal is required.
