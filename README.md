# Racing Dynamite

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)
[![Standard: C11](https://img.shields.io/badge/Standard-C11-green.svg)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))
[![Platform: SDL2](https://img.shields.io/badge/Platform-SDL2-red.svg)](https://www.libsdl.org/)

**Racing Dynamite** is an open-source, portable C implementation and reverse-engineering effort of **Ignition** (1997, Unique Development Studios / Virgin Interactive), targeting the authentic standard Windows release (`IGN_WIN.EXE`). The DOS release (`MAINDOS.EXE`) is retained as a secondary reference.

---

## Legal & Asset Notice

This repository contains **strictly clean-room reverse-engineered source code, tools, and technical documentation**.  
**No copyrighted game assets (graphics, 3D meshes, levels, sound effects, or music) are distributed with this repository.**

To run the game, you must supply game assets from a legally purchased copy of *Ignition* (available on GOG.com or original CD-ROM).

---

## Current Reconstruction Focus

As of 2026-10-07, the active goal is faithful reconstruction of
`Ignition/Ignition/IGN_WIN.EXE`, followed by modernization into C11/SDL2.
See [the Windows decompilation plan](docs/decomp_plan.md) for the verified target
fingerprint, migration boundaries, and next milestones. Windows reconstruction
replaces the superseded DOS work in `decomp/`; build tools, tracking, and symbol
maps will be retargeted in place. The old implementation is recoverable in Git
history and does not need a parallel source tree or maintained DOS build.
Historical DOS evidence remains a reference, not Windows completion. The first
[Windows milestone](docs/ghidra/windows_startup.md) now verifies startup edges and
reconstructs `Mem_InitHandles` at `0x0045B1F0`, compiled as a focused PE32 validation
DLL and checked against original instructions in x86 emulation. Run
`uv run tools/verify_matching.py` and `uv run python tools/verify_fidelity.py`.
The original compiler and visual/native startup baseline remain unresolved; the
[temporary Windows inventory](docs/tracking/windows_inventory.json) has one
routine. Remaining modules, DOS database progress, and dashboards are legacy.

## Features & Goals

- **Native Cross-Platform**: Replaces legacy DirectDraw 3/5, DirectSound, and DirectInput with modern SDL2 (Windows, Linux, macOS).
- **Accurate 1997 Simulation**: Complete reverse-engineering of vehicle physics, surface interaction (`getsurf`), AI waypoints (`.POS`), and track elevation (`.SRF`).
- **Software Rendering Parity**: Authentic recreation of UDS's "Lisa3D" software rasterizer and 2D blitter.
- **Thoroughly Documented**: Every binary structure and engine subsystem is documented under [`docs/`](docs/).
- **Implementation Roadmap**: Track completion milestones in [`docs/roadmap.md`](docs/roadmap.md).

---

## Building from Source

### Prerequisites
- **C11 Compiler**: GCC 14+, Clang 18+, or Visual Studio 2022.
- **CMake**: Version 3.20 or newer.
- **Python & UV**: Python 3.10+ managed via [`uv`](https://astral.sh/uv).
- **SDL2**: Development libraries (an automated fetch tool is provided).

### 1. Environment & Dependencies Setup
Install python tools and dependencies:
```bash
uv sync
```

Download SDL2 development files (if not using system package):
```bash
python tools/download_sdl2.py
```

### 2. Asset Extraction
Extract original game assets from your GOG install or CD image (`game.gog`):
```bash
python tools/extract_assets.py
```
*(Assets are extracted locally to `assets/` and remain strictly excluded from version control).*

### 3. Compile & Run
Configure and compile using CMake:
```bash
cmake -B build -G "Ninja"
cmake --build build
```

Run format unit tests:
```bash
./build/test_formats
```

Launch the engine:
```bash
./build/racing_dynamite
```

### 4. Windows Reconstruction Migration

`decomp/` is the designated Windows reconstruction workspace. Its existing DOS
code and Open Watcom build are superseded and will be replaced in place. There
is no supported Windows reconstruction command yet: the existing
`tools/build_decomp.py`, `tools/verify_matching.py`, and `tools/verify_fidelity.py`
still contain DOS assumptions and must be retargeted before use as Windows gates.

Start with the fingerprinted `IGN_WIN.EXE` and the
[active migration milestones](docs/decomp_plan.md). Use disposable runtime copies
under `build/runtime/`; never overwrite original binaries or assets in `Ignition/`.


---

## Project Structure

```text
RacingDynamite/
├── include/ignition/       # Reconstructed engine headers, types, and format definitions
├── src/
│   ├── core/               # Engine entry point, timing, and main game loop
│   ├── formats/            # Binary asset decoders (.PIC, .COL, .SRF, etc.)
│   └── platform/           # SDL2 windowing, event loop, and framebuffer blitter
├── tests/                  # Automated verification and format unit tests
├── tools/                  # Python utilities (asset extraction, format inspectors)
├── docs/                   # Full technical reverse engineering documentation
│   ├── formats/            # Binary file layout specifications
│   ├── engine/             # Engine architecture, game loop, physics, Lisa3D renderer
│   └── ghidra/             # Symbol tracking, function maps, and global memory tables
├── AGENTS.md               # Repository rules and reverse engineering guidelines
├── CMakeLists.txt          # CMake build configuration
└── LICENSE                 # GNU General Public License v3.0
```

---

## License

This project is licensed under the **GNU General Public License v3.0 (GPL-3.0-or-later)**.  
Copyright (C) 2026 Patricio Labin Correa (@F1r3f0x).  
See the [`LICENSE`](LICENSE) file for the full license text.
