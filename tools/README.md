# Racing Dynamite Tooling Documentation (`tools/`)

> **Tool cleanup (2026-10-07):** [MIGRATION.md](MIGRATION.md) and
> [migration_required.json](migration_required.json) supersede the legacy catalog
> below. Retired DOS tools are removed; retained migration scripts stop before
> execution. Objdiff stays, but its DOS object configuration needs retargeting.
> The database and dashboard entry points are active Windows tools.

> Windows is the primary target from 2026-10-07. The DOS implementation is
> superseded. The active database, provenance audit, generated inventory and
> treemap now track Windows evidence. The bounded build/emulator harness covers
> the recovered memory routines; other source modules and standalone DOS
> comparison/runtime tools remain unverified legacy work.

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
`--linker <path>`. The verifier always rebuilds, then runs 30 initializer and 377
consumer and 590 bookkeeping original-vs-C cases, plus eight original-only
negative-cursor cases. The [bookkeeping evidence](../docs/ghidra/windows_handle_bookkeeping.md)
records all-slot coverage, ordered effects and three-routine integration.
It also runs 531 shutdown comparisons with explicit callback models, live-table
mutation/reentry checks and four-routine integration; see
[shutdown evidence](../docs/ghidra/windows_handle_shutdown.md). Original callback
bodies are not validated by these models.
The ID-based release routine adds 418 comparisons covering every slot, duplicate
IDs, exact eligibility and five-routine integration; see
[release evidence](../docs/ghidra/windows_handle_release.md).
The integrated verifier also freshly extracts and compiles the production
`Font_GetTextWidth` body from `geputget.c` into a separate focused DLL, then runs
708 differential font cases and two original-only invalid-slot checks. Its
standalone command is `uv run tools/verify_font_width.py`. It uses the production
header, a C `strlen` harness dependency and the original `_ftol` instructions;
signed bytes/words, wrap and x87 double/extended precision are covered. Full
`geputget.c` compilation remains blocked by legacy dependencies. See
[font-width evidence](../docs/ghidra/windows_font_get_text_width.md).
It does not certify instruction equality or native game behavior. The
[consumer evidence](../docs/ghidra/windows_handles.md) states the negative-state
memory contract and the validation DLL's differing data layout.

The fidelity audit checks the active SQLite inventory and current source/result
provenance. [The Windows inventory](../docs/tracking/windows_inventory.json) is generated. See the
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

## 6. Windows SQLite Tracking and Dashboard

See [database/README.md](../database/README.md) for the authoritative schema,
explicit migration, supported editing commands and evidence semantics.
`uv run python tools/db.py update` generates the registry, SQL/JSON snapshots
and both Windows dashboards. `update --check` checks freshness without writing.
`tools/diff_func.py` remains a legacy DOS comparator and is not an active gate.

---

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

## Active Windows tracking commands (supersedes legacy database/dashboard sections)

The authoritative store is now Windows SQLite schema v2. Use `db.py status` for
read-only metrics; `db.py describe`/`set-status` for documented analysis updates;
`db.py audit` for source/provenance freshness; and `db.py update` to generate the
registry, inventory JSON, SQL dump and both dashboards. `db.py update --check`
reports output drift without mutation. `dashboard`, `export-markdown` and
`dump-sql` are aliases of the synchronized update, so outputs cannot diverge.
Destructive DOS initialization, markdown imports and bulk sync were removed from
the active CLI. Standalone DOS residue/comparison/runtime tools remain legacy.

`windows_tracking.py` reads genuine PE/FPO metadata without additional dependencies,
verifies the imported milestone and migrates atomically. The existing Windows
verifier now persists fresh per-function compilation, raw-byte and emulation
results. `verify_fidelity.py` iterates active records rather than requiring exactly
one function. `generate_dashboard.py` renders the Windows snapshot with canvas
treemap, class/stage/search filters and separate evidence/native milestone cards.
See [database/README.md](../database/README.md) for supported commands and scope.
