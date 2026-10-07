# Ignition (1997) Technical Documentation

> Active target (2026-10-07): standard Windows `IGN_WIN.EXE`. Windows work replaces the DOS implementation in `decomp/` and retargets build/tracking in place; no parallel DOS implementation is required. See [the active plan](decomp_plan.md). The active Windows database, generated function map and treemap now share one snapshot. Other engine/format/global/struct notes contain legacy evidence and require independent Windows validation.

Welcome to the central technical documentation and reverse engineering knowledge base for **Ignition** (1997, Unique Development Studios / Virgin Interactive).

---

## 1. Game Overview & Specifications

| Property | Value |
| :--- | :--- |
| **Title** | Ignition (also known as *Bleifuss Fun* in Germany / *Fun Tracks* in France) |
| **Developer** | Unique Development Studios (UDS) |
| **Publisher** | Virgin Interactive Entertainment |
| **Release Date** | Late 1997 (Win95 release: August 28, 1997) |
| **Original Target** | MS-DOS (DOS/4GW) & Windows 95 (DirectX 3/5) |
| **Rendering Engine** | UDS "Lisa3D" (Software polygon rasterizer + 2D sprite blitter) |
| **Resolution** | 640x480 (Menus / Hi-Res) & 320x200 (Gameplay / Lo-Res options) @ 8-bit paletted color |
| **Audio** | DirectSound 8-bit/16-bit PCM SFX (.WAV, .PAT) + Red Book CD-DA audio tracks |

---

## 2. Master Roadmap

* **[Windows Tooling Audit](tooling_audit.md)**: Current migration gaps, read-only diagnostics, and the project decompilation skill.

* 🎯 **[Master Implementation Roadmap](roadmap.md)**: Comprehensive 8-phase reverse engineering and implementation plan to complete the source port.

---

## 3. Documentation Directory

### 📂 File Formats ([`docs/formats/`](file:///c:/Stuff/Proyects/RacingDynamite/docs/formats))
Detailed binary structure tables, byte offsets, and parser specifications for all game assets:

* **[`.COL` (Color Palettes)](file:///c:/Stuff/Proyects/RacingDynamite/docs/formats/col.md)**: 776-byte 256-color RGB palette tables.
* **[`.PIC` (Images & Textures)](file:///c:/Stuff/Proyects/RacingDynamite/docs/formats/pic.md)**: 846-byte header + embedded 256-color palette + uncompressed 8bpp pixel data.
* **[`.SRF` (Surface & Physics)](file:///c:/Stuff/Proyects/RacingDynamite/docs/formats/srf.md)**: Track road elevation, heightfields, and collision physics (`getsurf.c`).
* **[`.MSH` & `.TRI` (3D Models & Meshes)](file:///c:/Stuff/Proyects/RacingDynamite/docs/formats/msh_tri.md)**: Track & car vertex positions and triangle index lists.
* **[`.TRI` (Track Splines & Road Ribbons)](file:///c:/Stuff/Proyects/RacingDynamite/docs/formats/tri.md)**: Road boundary vertex indices, road width, and circuit branch directives.
* **[`.POS` (Scenery Object Keyframe Animations)](file:///c:/Stuff/Proyects/RacingDynamite/docs/formats/pos.md)**: Delta keyframe streams for dynamic track scenery objects.
* **[`.TEX` & `.TAB` (Textures & Shading Tables)](file:///c:/Stuff/Proyects/RacingDynamite/docs/formats/tex_tab.md)**: 1024x1024 texture sheets, distance fog, lighting, and shadow tables.
* **[`.LFT` (Font Glyphs)](file:///c:/Stuff/Proyects/RacingDynamite/docs/formats/lft.md)**: 2D bitmap lettering fonts used across menus and HUD.
* **[`.PAN` (Sky Panoramas)](file:///c:/Stuff/Proyects/RacingDynamite/docs/formats/pan.md)**: 64KB raw 256x256 cylindrical horizon background textures.
* **[`ENGINE.INF` (Engine Acoustic Curves)](file:///c:/Stuff/Proyects/RacingDynamite/docs/formats/inf.md)**: 800-byte speed/RPM volume and pitch modulation tables.
* **[`.CDP` (Cinematics & FMVs)](file:///c:/Stuff/Proyects/RacingDynamite/docs/formats/cdp.md)**: RLE delta-frame animation format for intro sequences.

### ⚙️ Engine Subsystems ([`docs/engine/`](file:///c:/Stuff/Proyects/RacingDynamite/docs/engine))
Architectural breakdowns of the core engine modules:

* **[Engine Architecture](file:///c:/Stuff/Proyects/RacingDynamite/docs/engine/architecture.md)**: Overall memory model, coordinates, fixed-point math, and subsystem overview.
* **[Game Loop & State Machine](file:///c:/Stuff/Proyects/RacingDynamite/docs/engine/game_loop.md)**: State transitions (`Logos` $\rightarrow$ `Menus` $\rightarrow$ `Loading` $\rightarrow$ `Race`), frame tick dispatch.
* **[Surface & Physics Engine](file:///c:/Stuff/Proyects/RacingDynamite/docs/engine/surface_physics.md)**: Vehicle suspension, ray-casting surface collision, friction, turbo mechanics.
* **[Track Splines & AI Waypoints](file:///c:/Stuff/Proyects/RacingDynamite/docs/engine/ai_waypoints.md)**: Road chunk sequencing, spline extraction, and AI steering simulation.
* **[Lisa3D Rendering Pipeline](file:///c:/Stuff/Proyects/RacingDynamite/docs/engine/renderer.md)**: 3D perspective projection, polygon clipping, and 8-bit span rasterization.
* **[Dynamic Chase Camera](file:///c:/Stuff/Proyects/RacingDynamite/docs/engine/camera.md)**: Multi-mode follow chase camera, velocity lookahead, 0.125 lag damping, and hill pitch adaptation.
* **[Audio Subsystem](file:///c:/Stuff/Proyects/RacingDynamite/docs/engine/audio.md)**: Sound effect pooling, 32-channel software voice mixer, and CD-DA / OGG music track playback.
* **[Logging & Telemetry Subsystem](file:///c:/Stuff/Proyects/RacingDynamite/docs/engine/logging.md)**: Dual console/disk file logging, in-race telemetry capture, and runtime diagnostics.

### 🔍 Ghidra Reverse Engineering Maps ([`docs/ghidra/`](file:///c:/Stuff/Proyects/RacingDynamite/docs/ghidra))
Tracking reverse engineered symbols and memory addresses:

* **[Function Map](file:///c:/Stuff/Proyects/RacingDynamite/docs/ghidra/functions.md)**: Master table of original function addresses, signatures, reconstructed names, and port status.
* **[Global Variables](file:///c:/Stuff/Proyects/RacingDynamite/docs/ghidra/globals.md)**: Global state variables, pointers, and game flags.
* **[Data Structures](file:///c:/Stuff/Proyects/RacingDynamite/docs/ghidra/structs.md)**: Reconstructed C structs with exact member byte offsets.

### 📋 Fidelity & Change Tracking ([`docs/tracking/`](file:///c:/Stuff/Proyects/RacingDynamite/docs/tracking))
Methodology, provenance standards, and divergence tracking:

* **[FCTS Master Plan](file:///c:/Stuff/Proyects/RacingDynamite/docs/tracking/fidelity_system_plan.md)**: Master technical specification for Phase 4, covering tooling hygiene, Lisa3D software authenticity vs multi-backend, granular per-category game fixes, and automated parity audits.

---

## 4. Original Binary Signatures

| Binary | Platform | Size | SHA-256 Checksum | Notes |
| :--- | :--- | :--- | :--- | :--- |
| `MAINDOS.EXE` | DOS/4GW 32-bit LE | 1,156,502 bytes | `179C654BA65281F08BBCDA2B5943AB9F35C309EC718600F81104644D22DB948C` | Secondary DOS reference; retained evidence |
| `IGN_WIN.EXE` | Windows x86 PE32 | 915,968 bytes | `7665E4E736BFD6C90790CEDBB27E2DE7E98A167374EB77933533C54EF0DC8782` | Primary target; headers/fingerprint verified, reconstruction and runtime baseline pending |
