# Ignition (1997) Technical Documentation

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

## 2. Documentation Directory

### 📂 File Formats ([`docs/formats/`](file:///c:/Stuff/Proyects/RacingDynamite/docs/formats))
Detailed binary structure tables, byte offsets, and parser specifications for all game assets:

* **[`.COL` (Color Palettes)](file:///c:/Stuff/Proyects/RacingDynamite/docs/formats/col.md)**: 776-byte 256-color RGB palette tables.
* **[`.PIC` (Images & Textures)](file:///c:/Stuff/Proyects/RacingDynamite/docs/formats/pic.md)**: 846-byte header + embedded 256-color palette + uncompressed 8bpp pixel data.
* **[`.SRF` (Surface & Physics)](file:///c:/Stuff/Proyects/RacingDynamite/docs/formats/srf.md)**: Track road elevation, heightfields, and collision physics (`getsurf.c`).
* **[`.MSH` & `.TRI` (3D Models & Meshes)](file:///c:/Stuff/Proyects/RacingDynamite/docs/formats/msh_tri.md)**: Track & car vertex positions and triangle index lists.
* **[`.POS` (Path Nodes & AI Waypoints)](file:///c:/Stuff/Proyects/RacingDynamite/docs/formats/pos.md)**: Camera paths, checkpoint gates, and AI waypoint splines.
* **[`.TEX` & `.TAB` (Textures & Shading Tables)](file:///c:/Stuff/Proyects/RacingDynamite/docs/formats/tex_tab.md)**: 1024x1024 texture sheets, distance fog, lighting, and shadow tables.
* **[`.LFT` (Font Glyphs)](file:///c:/Stuff/Proyects/RacingDynamite/docs/formats/lft.md)**: 2D bitmap lettering fonts used across menus and HUD.

### ⚙️ Engine Subsystems ([`docs/engine/`](file:///c:/Stuff/Proyects/RacingDynamite/docs/engine))
Architectural breakdowns of the core engine modules:

* **[Engine Architecture](file:///c:/Stuff/Proyects/RacingDynamite/docs/engine/architecture.md)**: Overall memory model, coordinates, fixed-point math, and subsystem overview.
* **[Game Loop & State Machine](file:///c:/Stuff/Proyects/RacingDynamite/docs/engine/game_loop.md)**: State transitions (`Logos` $\rightarrow$ `Menus` $\rightarrow$ `Loading` $\rightarrow$ `Race`), frame tick dispatch.
* **[Surface & Physics Engine](file:///c:/Stuff/Proyects/RacingDynamite/docs/engine/surface_physics.md)**: Vehicle suspension, ray-casting surface collision, friction, turbo mechanics.
* **[Lisa3D Rendering Pipeline](file:///c:/Stuff/Proyects/RacingDynamite/docs/engine/renderer.md)**: 3D perspective projection, polygon clipping, and 8-bit span rasterization.
* **[Audio Subsystem](file:///c:/Stuff/Proyects/RacingDynamite/docs/engine/audio.md)**: Sound effect pooling and CD-DA / OGG music track playback.

### 🔍 Ghidra Reverse Engineering Maps ([`docs/ghidra/`](file:///c:/Stuff/Proyects/RacingDynamite/docs/ghidra))
Tracking reverse engineered symbols and memory addresses:

* **[Function Map](file:///c:/Stuff/Proyects/RacingDynamite/docs/ghidra/functions.md)**: Master table of original function addresses, signatures, reconstructed names, and port status.
* **[Global Variables](file:///c:/Stuff/Proyects/RacingDynamite/docs/ghidra/globals.md)**: Global state variables, pointers, and game flags.
* **[Data Structures](file:///c:/Stuff/Proyects/RacingDynamite/docs/ghidra/structs.md)**: Reconstructed C structs with exact member byte offsets.

---

## 3. Original Binary Signatures

| Binary | Platform | Size | SHA-256 Checksum | Notes |
| :--- | :--- | :--- | :--- | :--- |
| `IGN_WIN.EXE` | Win32 PE (i386) | 915,968 bytes | `7665E4E736BFD6C90790CEDBB27E2DE7E98A167374EB77933533C54EF0DC8782` | Primary reverse engineering target |
| `MAINDOS.EXE` | DOS/4GW 32-bit | 1,156,502 bytes | `179C654BA65281F08BBCDA2B5943AB9F35C309EC718600F81104644D22DB948C` | Contains intact assertion strings & source filenames |
