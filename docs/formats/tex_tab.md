# Ignition (1997) .TEX, .TAB, .SHD & .PAN Formats

## Overview
Ignition relies on fixed-size 64KB and 1MB binary tables to optimize 8-bit software rendering without floating-point math overhead on vintage CPUs.

---

## 1. `.TEX` (Sequential 64KB Texture Pages)
* **File Architecture**: Rather than a single $1024 \times 1024$ image, `.TEX` is a flat sequence of independent **64KB ($256 \times 256$) texture pages** stored in file order.
* **Header**: None. The file begins immediately at byte 0 with page 0 pixels.
* **File Sizes Across Circuits**:
  * `BRAZIL.TEX`, `CANADA.TEX`, `USA.TEX`: Exactly **1,048,576 bytes** ($16 \times 65,536$ bytes = 16 pages).
  * `AUSTRIA.TEX`: **1,048,768 bytes** ($16 \times 65,536 + 192$ padding bytes at the end of the file; byte 0 is raw pixel data).
  * `CARIB.TEX`: **1,048,704 bytes** ($16 \times 65,536 + 128$ padding bytes at the end).
  * `ICELAND.TEX`: **1,065,088 bytes** ($16.25$ pages; uses 17 pages $0\dots 16$).
  * `JAPAN.TEX`: **1,032,192 bytes** ($15.75$ pages; page 15 has 192 active rows = 49,152 bytes, saving 16,384 unused bytes).
* **Engine Memory Allocation (`Texture_LoadAllPages` `0x00419d10`)**:
  `MAINDOS.EXE` rounds the allocation size up to the next 64KB multiple:
  $$\text{alloc\_size} = (\text{file\_length} + \text{0xFFFF}) \ \& \ \sim\text{0xFFFF}$$
  The file is read directly from byte offset 0 into the buffer.
* **Polygon Page Addressing (`poly->extra`)**:
  In `.MSH` polygon records (44 bytes), DWORD 10 (`poly->extra`) is the byte offset of the texture page from the start of the `.TEX` file:
  $$\text{page\_offset} = \text{poly}\to\text{extra} = \text{page\_index} \times 65536$$
  All track submesh polygons reference pages $0\dots 16$.
* **UV Coordinate System**:
  Polygon $U$ and $V$ coordinates (`u0, v0, u1, v1, u2, v2`) are stored in $8.8$ fixed-point format, normalized across the $256 \times 256$ page ($0\dots 65536 \rightarrow 0.0\dots 1.0$):
  $$U_{\text{texel}} = \frac{U_{\text{msh}}}{256.0} = \frac{U_{\text{msh}}}{65536.0} \times 256.0$$
  $$V_{\text{texel}} = \frac{V_{\text{msh}}}{256.0} = \frac{V_{\text{msh}}}{65536.0} \times 256.0$$
* **Texel Sampling Formula (`Lisa_DrawTexturedSpan` `FUN_004537dc`)**:
  $$X = \lfloor U_{\text{texel}} \rfloor \& 255$$
  $$Y = \lfloor V_{\text{texel}} \rfloor \& 255$$
  $$\text{texel} = \text{pixels}[\text{page\_offset} + Y \times 256 + X]$$
  Every page has a stride of **256 bytes**. Sub-tiles within a page (e.g. road strips, curbstones, building facades) are $64 \times 64$ pixels arranged in a $4 \times 4$ grid per page.
* **Alpha Transparency Testing**:
  * **Opcode `0x11`**: Opaque unshaded geometry (road surfaces, rock walls).
  * **Opcode `0x13, 0x16, 0x17`**: Transparent geometry (foliage overhangs, tree leaves, fences, warning signs). Color index `0x00` represents the transparent color key and is discarded without writing to framebuffer or Z-buffer.

---

## 2. `.TAB` (Color & Shading Lookup Table)
* Size: exactly **65,536 bytes** (`256 * 256` = 64 KB).
* Represents a $256 \times 256$ matrix indexed by:
  ```c
  uint8_t shaded_color = color_tab[(original_color << 8) | light_level];
  ```
  where:
  * `light_level`: 0 = full bright, ramping to 63 = maximum darkness / shadow.
  * Slices 0 and 64 are full bright identity (`tab[c * 256 + 64] == c` for all 256 palette entries). Slices 64..255 remain identity.
  * **Opcode 0x11** (Track Mesh Polygons): In Lisa3D (`MAINDOS.EXE` `0x004559a8`), track scenery and road triangles bypass `.TAB` shading completely and blit texels directly, guaranteeing crisp, vivid retro texturing without palette degradation.
  * **Opcode 0x15 & 0x17** (Dynamic Gouraud / Car Shading): Evaluates vertex lighting and applies `.TAB` color ramp lookups.

---

## 3. `.SHD` (Shadow & Alpha Blend Lookup Table)
* **File Size**: Exactly **65,536 bytes** ($256 \times 256 = 64\text{ KB}$).
* **Engine Loading (`Track_LoadAllAssets` `0x0041a0e0`)**:
  Loaded per track from `LEVELS/<TRACK>/<TRACK>.SHD` into global engine buffer `DAT_0063b5f0`.
* **Addressing & Blending Formula**:
  The table is a $256 \times 256$ matrix representing foreground-over-background composition:
  $$\text{blended\_color} = \text{shd\_table}[(\text{texel} \ll 8) \ | \ \text{bg\_pixel}]$$
  where:
  * `texel`: 8-bit foreground texel sampled from the texture page ($0\dots 255$).
  * `bg_pixel`: 8-bit destination pixel already residing in the framebuffer ($0\dots 255$).
* **Opcode Dispatch (`MAINDOS.EXE` `0x0044cac0` & `0x0044cb00`)**:
  * **Opcode `0x11`**: Standard opaque textured polygons.
  * **Opcode `0x12, 0x16`**: Standard 1-bit color key 0 discard (transparent cutout).
  * **Opcode `0x13, 0x17`**: Shadow / Alpha blend polygons. Pushes `DAT_0063b5f0` (`.SHD` table) into the triangle bucket rasterizer.
* **Special Properties Across Circuits**:
  * **Row 0 Transparent Identity**: When $\text{texel} = 0$, $\text{shd}[0 \times 256 + \text{bg}] = \text{bg}$ across virtually all entries.
  * **Foliage Color Remapping (Brazil)**:
    In `BRAZIL.MSH`, tree canopy and hill foliage use opcode `0x13` with raw texels in the range $160\dots 174$. In `BRAZIL.COL`, raw indices $160\dots 174$ are yellow/orange. Rows $160\dots 174$ of `BRAZIL.SHD` map these texels into dark green foliage indices ($102\dots 116$, RGB $(0, 26, 0)\dots (0, 78, 0)$), producing natural lush tree leaves rather than yellow flowers.
  * **Vehicle Ground Shadows**:
    Polygons with archetype/model type 250 cast dynamic translucent shadows under vehicles using `.SHD` darkening rows.

---

## 4. `.PAN` (Panorama Skybox / Background)
* Size: exactly **65,536 bytes** (`256 * 256` = 64 KB).
* Cylindrical 360-degree background horizon image drawn behind distant track geometry.
* See [`docs/formats/pan.md`](file:///c:/Stuff/Proyects/RacingDynamite/docs/formats/pan.md) for the full specification.

---

## 5. General Sprite Sub-TEX Pages (`GENERAL/` Particle Effects)

Discovered via decompilation of `Texture_LoadAllPages` (`0x00419d10`). In addition to the track `.TEX` atlas and `CARS.TEX`, the engine loads **5 additional 64KB particle/effect sprite pages** from a `GENERAL/` directory. Each is size-read, 64KB-aligned, and stored at a sequential offset from `g_pActiveTEX`.

| Global Pointer | TEX Sub-Page | Sprite Content |
| :--- | :--- | :--- |
| `DAT_005530f4` | `GENERAL/LIGHT.*` | Headlight and lens flare sprites. |
| `DAT_005530f8` | `GENERAL/DARKSMOK.*` | Dark exhaust smoke particle sprites. |
| `DAT_005530fc` | `GENERAL/EXSMOKE.*` | Light exhaust smoke/dust particle sprites. |
| `DAT_00553100` | `GENERAL/<4th>.*` | Additional particle sprite (strings at `DAT_00497260`). |
| `DAT_0055310c` | `GENERAL/<5th>.*` | Additional particle sprite (strings at `DAT_00497280`). |

All sub-pages use the same 64KB alignment formula as the main `.TEX`:
$$\text{alloc\_size} = (\text{file\_length} + \text{0xFFFF}) \ \& \ \sim\text{0xFFFF}$$

Sub-page pointers are offset **from** `g_pActiveTEX` (the 64KB-aligned base of the track `.TEX`), so they form a contiguous memory arena:
```c
DAT_005530f4 = (void*)((int)DAT_005530f4 + (int)g_pActiveTEX);
DAT_005530f8 = (void*)((int)DAT_005530f8 + (int)g_pActiveTEX);
// ... (all 5 sub-pages patched relative to g_pActiveTEX)
```

> [!NOTE]
> The exact filenames for the 4th and 5th sub-pages require reading the string data at `DAT_00497260` and `DAT_00497280` from `MAINDOS.EXE`. These are confirmed present in the binary but not yet decoded to their full path strings.
