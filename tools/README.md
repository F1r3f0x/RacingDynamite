# Racing Dynamite Tooling Documentation (`tools/`)

This directory contains utility scripts, reverse engineering inspectors, asset pipeline tools, and verification scripts for **Racing Dynamite** (Ignition 1997 source port).

All Python tools are managed using **`uv`**. Run any script using:
```bash
uv run python tools/<script_name>.py [args]
```

---

## 1. Core Asset Pipeline Tools

| Script | Purpose | Usage / Command |
| :--- | :--- | :--- |
| **`extract_assets.py`** | Master asset extractor. Extracts game files from original CD images, ISOs, cue/bin, or GOG `game.gog`. Decodes audio tracks and organizes assets into `assets/`. | `uv run python tools/extract_assets.py` |
| **`download_sdl2.py`** | Fetches and unpacks SDL2 development headers and binaries for Windows MSVC / MinGW. | `uv run python tools/download_sdl2.py` |
| **`pic_to_bmp.py`** | Converts a single 8bpp `.PIC` file into a standard Windows 8bpp BMP file with embedded or external `.COL` palette. | `uv run python tools/pic_to_bmp.py <input.pic> [output.bmp] [palette.col]` |
| **`convert_all_pics.py`** | Batch converts all `.PIC` files across all track, vehicle, and UI directories into BMP format. | `uv run python tools/convert_all_pics.py` |
| **`dump_font_sheets.py`** | Extracts all `.LFT` fonts in `assets/` and exports each font's complete glyph set into a consolidated BMP sprite sheet. | `uv run python tools/dump_font_sheets.py` |
| **`dump_tex_bmp.py`** | Dumps 1024x1024 master `.TEX` sheets and individual 64KB texture pages as BMP images. | `uv run python tools/dump_tex_bmp.py [track_name]` |
| **`decode_submesh_polys.py`** | Decodes `.MSH` 3D submeshes, extracting polygon opcodes (0x11, 0x12, 0x13, 0x15, 0x16, 0x17), vertex indices, and material IDs. | `uv run python tools/decode_submesh_polys.py <msh_path>` |

---

## 2. Reverse Engineering & Format Inspectors

| Script | Format / Target | Description |
| :--- | :--- | :--- |
| **`inspect_all_srf.py`** | `.SRF` (Terrain) | Dumps header dimensions, cell sizes, triangle counts, and table offsets for all 7 track `.SRF` files. |
| **`inspect_srf_triangles.py`** | `.SRF` (Physics) | Inspects the 24-byte surface collision triangle records in active `.SRF` files. |
| **`inspect_srf_tris.py`** | `.SRF` (Grid) | Dumps cell-to-triangle candidate pointers and spatial grid lookup offsets. |
| **`inspect_msh_submeshes.py`** | `.MSH` (Geometry) | Inspects vertex counts, submesh counts, and bounding geometry for level and car meshes. |
| **`inspect_msh_tri_all.py`** | `.MSH` / `.TRI` | Cross-track comparison of mesh submeshes and `.TRI` road spline sequences. |
| **`inspect_msh_tri_details.py`** | `.MSH` / `.TRI` | Detailed polygon topology analysis and road boundary vertex verification. |
| **`inspect_plc_positions.py`** | `.PLC` (Objects) | Dumps object placements, coordinates, archetype IDs, and submesh references. |
| **`inspect_pos.py`** | `.POS` (Animations) | Dumps keyframe transformation deltas for animated track scenery objects. |
| **`inspect_tab_matrix.py`** | `.TAB` (Shading) | Analyzes 64KB distance fog, lighting, and shadow lookup tables. |
| **`inspect_tex_header.py`** | `.TEX` (Textures) | Inspects texture file headers, page alignments, and texture sheet layouts. |
| **`inspect_pal_values.py`** | `.COL` (Palettes) | Analyzes RGB palette entries, color ramps, and brightness ranges across `.COL` files. |
| **`inspect_pics.py`** | `.PIC` (Bitmaps) | Scans all `.PIC` files to verify header size, width, height, and palette presence. |
| **`inspect_track_files.py`** | All Level Assets | Validates file presence, naming, and byte sizes across all circuits. |
| **`inspect_lft.py`** | `.LFT` (Fonts) | Dumps `.LFT` font header, glyph height, spacing, and character count. |
| **`inspect_lft_chars.py`** | `.LFT` (Glyphs) | Dumps character code offsets, glyph widths, and direct ASCII mapping tables. |
| **`inspect_small_lft.py`** | `SMALL.LFT` | Specific inspector for `SMALL.LFT` HUD and status font. |
| **`inspect_ignition_fnt.py`** | `IGNITION.FNT` | Specific inspector for the master menu title font. |
| **`ascii_art_font.py`** | `.LFT` (Visual) | Renders font glyphs as ASCII art directly in the command-line console. |
| **`dump_all_indices.py`** | `.TRI` (Indices) | Dumps track chunk indices across all circuits. |
| **`inspect_formats.py`** | Multi-format | Sanity checker for headers across all binary formats. |

---

## 3. Mathematical Verification & Parity Scripts

| Script | Purpose |
| :--- | :--- |
| **`verify_fidelity.py`** | Automated Fidelity & Change Tracking System (FCTS) auditor. Validates bidirectional consistency between `docs/ghidra/functions.md`, `docs/tracking/deviations.md`, and C source code annotations (`@original`, `@fidelity`, `@deviation`, `@fix_category`). Generates parity dashboard and returns non-zero on broken references. |
| **`verify_tab_formula.py`** | Verifies the mathematical formula for `.TAB` shading matrix against original binary behavior. |
| **`verify_all_game_strings.py`** | Validates game strings in C code against the string table extracted from `IGN_WIN.EXE`. |
| **`verify_screens.py`** | Verifies UI screen coordinates, button positions, and layout dimensions. |

---

## 4. Software Rasterizer Prototypes

| Script | Purpose |
| :--- | :--- |
| **`test_3d_fix.py`** | Python reference implementation of Lisa3D perspective projection and span rasterization. |
| **`test_road_render.py`** | Python road spline triangulation and span rasterization prototype. |

---

## 5. Ghidra MCP Server Bridge (`tools/ghidra_mcp/`)

Provides live bidirectional synchronization between the Antigravity agent and the Ghidra reverse-engineering environment:
* **`bridge_mcp_ghidra.py`**: MCP JSON-RPC server connecting the agent to Ghidra's API.
* Allows inspecting disassembly, renaming functions, adding decompiler comments, and syncing memory maps directly with `RacingDynamite.gpr`.
