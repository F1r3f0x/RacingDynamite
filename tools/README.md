# Racing Dynamite Tooling Documentation (`tools/`)

> Windows is the primary target from 2026-10-07. The DOS implementation is
> superseded. `build_decomp.py`, `verify_matching.py`, and `verify_fidelity.py`
> now support only the independently verified Windows `mem.c` routine. Other
> decomp modules, the DOS database CLI, old instruction diff/audits, runtime
> staging, and dashboards are **legacy**, not Windows acceptance gates.

## Active bounded Windows commands

Run from the repository root in PowerShell, with `uv`, Clang and LLD on PATH:

```powershell
uv run tools/windows_inspect.py
uv run python tools/build_decomp.py
uv run tools/verify_matching.py
uv run python tools/verify_fidelity.py
```

Analysis and CPU validation dependencies are pinned in each script's PEP 723
metadata; use `uv run <script>` rather than `uv run python <script>` for those two
commands. The build compiles only `decomp/src/mem.c` with strict C89 for x86 Windows,
then links a dependency-free validation DLL under `build/decomp/windows/`. It prints
actual compiler/linker commands, rejects the wrong target fingerprint, and never
uses stale DOS objects. Optional build arguments: `--compiler <path>` and
`--linker <path>`. The verifier always rebuilds, then runs 30 original-vs-C
emulator cases; it does not certify instruction equality or native game behavior.

The fidelity audit checks only the
[temporary Windows inventory](../docs/tracking/windows_inventory.json). See the
[startup map](../docs/ghidra/windows_startup.md) for addresses, data layout, compiler
limitations, runtime observations and the next dependency. The catalog below
also includes historical/asset tools; their results do not transfer DOS progress.

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
| **`verify_fidelity.py`** | Active Windows fingerprint/address/annotation audit for the temporary inventory and migrated `mem.c` only. Does not generate or certify legacy dashboards. |
| **`verify_tab_formula.py`** | Verifies the mathematical formula for `.TAB` shading matrix against original binary behavior. |
| **`verify_all_game_strings.py`** | Validates game strings in C code against font tables in `MAINDOS.EXE`. |
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

---

## 6. Decompilation Tracking Database (`tools/db.py`)

Current DOS/legacy tracking interface, scheduled for in-place Windows migration. Replace/reset active DOS records and rebuild progress from independently verified **`IGN_WIN.EXE`** evidence; no parallel DOS database is required:
* **`tools/db.py`**: SQLite database manager (`database/decomp.db` backed by `database/schema.sql` and `database/dump.sql`).
* **Commands**:
  * `uv run python tools/db.py init`: Initializes clean SQLite database from `database/schema.sql`.
  * `uv run python tools/db.py status`: Prints high-level progress report (functions, globals, modules, completion percentages).
  * `uv run python tools/db.py import-markdown`: Ingests legacy markdown documentation into the database.
  * `uv run python tools/db.py export-markdown`: Generates `docs/ghidra/functions.md` from the database.
  * `uv run python tools/db.py dump-sql`: Dumps database to version-controlled `database/dump.sql`.
  * `uv run python tools/db.py dashboard`: Generates the interactive HTML progress dashboard (`dashboard.html`).
  * `uv run python tools/db.py query "<SQL>"`: Runs ad-hoc SQL queries.
* **`tools/diff_func.py`**: Instruction-by-instruction bytecode comparison tool between authentic `MAINDOS.EXE` and Watcom-compiled COFF objects in `build/decomp/`.
  * Usage: `uv run python tools/diff_func.py <symbol_name> <dos_addr> <byte_size> [module.c]`

---

## 7. Interactive HTML Decompilation Dashboard (`tools/generate_dashboard.py`)

Legacy DOS visual analytics dashboard generator (not Windows progress):
* **`tools/generate_dashboard.py`**: Queries `database/decomp.db` and FCTS registries to generate a standalone, zero-dependency HTML dashboard with dark UI, live search, status filters, and structure inspection.
* **Outputs**:
  * `dashboard.html` (project root)
  * `docs/dashboard.html` (documentation hub mirror)
* **Usage**:
  ```bash
  uv run python tools/generate_dashboard.py
  # Or via db manager:
  uv run python tools/db.py dashboard
  ```


## 8. Windows Target Diagnostics and Project Skill

`tools/decomp_doctor.py` verifies the expected identity from `decomp/target.json`
and reports dependencies, compiler paths, legacy tool assumptions, and read-only
database metadata. Optional localhost GET probes check Ghidra reachability; they
do not verify loaded-program identity. Exit zero means inspection and local target
identity succeeded, not that the Windows pipeline is ready.

```powershell
uv run python tools/decomp_doctor.py
uv run python tools/decomp_doctor.py --probe-ghidra
uv run python -m unittest discover -s tests -p test_decomp_doctor.py
```

The repository skill `$ignition-windows-decomp` lives in
`.agents/skills/ignition-windows-decomp/`. Use it in a project chat for verified
binary analysis, bounded C89 reconstruction, and validation. It provides guidance;
it does not retarget the old build pipeline by itself.

See [the tooling audit](../docs/tooling_audit.md) for findings and migration order.
