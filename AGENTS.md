# Ignition (1997) Reverse Engineering & Source Port Guidelines

This repository hosts the reverse engineering and modern C11/SDL source port of **Ignition** (1997, Unique Development Studios / Virgin Interactive), targeting the original Windows 95 executable (`IGN_WIN.EXE`) with reference insights from the DOS executable (`MAINDOS.EXE`).

---

## 1. Golden Rule: Mandatory Documentation

> [!IMPORTANT]
> **Every step of decompilation, reverse engineering, and source porting MUST be thoroughly documented.**  
> Code implementation and documentation must evolve in lockstep. Never write or port engine code without documenting the underlying binary structures, memory layouts, and algorithms in `docs/`.

### Documentation Requirements for Every Feature:
1. **File Formats (`docs/formats/`)**:
   - Exact binary layout table (byte offsets, field names, C data types, sizes, descriptions).
   - Endianness, padding, header structures, and coordinate conventions.
   - Example values from real game files across tracks and cars.
2. **Engine Subsystems (`docs/engine/`)**:
   - Algorithmic explanations of game mechanics (e.g. physics raycasting, state transitions, span rasterization, AI navigation).
   - Cross-references to original function addresses in `IGN_WIN.EXE` and source files from `MAINDOS.EXE` (`getsurf.c`, `lisa3d.c`, `geputget.c`, `mem.c`).
3. **Ghidra Progress & Symbol Tracking (`docs/ghidra/`)**:
   - Maintain `docs/ghidra/functions.md` with every identified function, address, signature, and decompilation status.
   - Maintain `docs/ghidra/globals.md` with global variable addresses, types, and state meanings.
   - Maintain `docs/ghidra/structs.md` with reconstructed structs and member offsets.

---

## 2. Directory Layout & Organization

All technical documentation resides in `docs/`:

```text
docs/
├── README.md               # Master index and technical specification hub
├── tracking/               # Fidelity & change tracking system (FCTS)
│   ├── fidelity_strategy.md# Master guideline, header specs & workflow
│   ├── deviations.md       # Registry of authentic divergences and bug fixes
│   └── fidelity_system_plan.md # Master Phase 4 implementation specification
├── formats/                # Binary file format specifications
│   ├── col.md              # 256-color palette format (.COL)
│   ├── pic.md              # 8-bit paletted image format (.PIC)
│   ├── srf.md              # Road surface, heightmap & collision physics (.SRF)
│   ├── msh_tri.md          # 3D meshes & triangle indices (.MSH, .TRI)
│   ├── pos.md              # AI waypoints, camera splines & checkpoints (.POS)
│   ├── plc.md              # Track object placement (.PLC)
│   ├── tex_tab.md          # Textures (.TEX), color lookup tables (.TAB), shadows (.SHD), panoramas (.PAN)
│   └── lft.md              # Lettering / bitmap font format (.LFT)
├── engine/                 # Engine subsystems and architecture
│   ├── architecture.md     # Engine overview, memory model, coordinate system
│   ├── game_loop.md        # State machine (Logos -> Menus -> Loading -> Race)
│   ├── surface_physics.md  # Vehicle dynamics, surface collision (getsurf)
│   ├── renderer.md         # Lisa3D software rasterizer & 2D blitter
│   └── audio.md            # Sound effects (.WAV, .PAT) & CD-DA soundtrack
└── ghidra/                 # Reverse engineering tracking
    ├── functions.md        # Master table of identified functions
    ├── globals.md          # Master table of global variables & memory addresses
    └── structs.md          # Reconstructed data structures
```

---

## 3. Tooling & Workflow Conventions

* **Language**: C11 for engine code, Python for format verification and tooling scripts.
* **Platform Layer**: SDL2 for windowing, 2D framebuffer presentation, input, and audio streaming.
* **Ghidra MCP Server**: Use the active Ghidra MCP server (`localhost:8080`) to inspect disassembly, decompile, rename functions, add comments, and update symbol databases directly from the agent.
* **Build System**: CMake with Ninja / GCC or MSVC.
* **Dependencies**: Managed strictly via `uv` for Python (`pyproject.toml`, `uv.lock`) and CMake for C.
* **Tooling Documentation**: All active tools in `tools/` must be documented in `tools/README.md`. Scratch scripts must be cleaned after exploration.
* **No Git Bloat**: Binary game assets (`assets/`) and build artifacts (`build/`, `.venv/`) must remain excluded from Git.

---

## 4. Fidelity & Change Tracking Rules (FCTS)

> [!IMPORTANT]
> **Every change, port, bug fix, and deviation MUST follow the Fidelity & Change Tracking System.**

1. **Standardized Inline Provenance Headers**:
   Every function declared in `include/ignition/` and implemented in `src/` must be documented with:
   - `@original <SymbolName> (IGN_WIN.EXE @ 0x<Address>, <SourceFileHint>)`
   - `@fidelity EXACT | ADAPTED | EXTENDED | INFRASTRUCTURE`
   - `@deviation DEV-XXX` (required if logic diverges from original 1997 binary or fixes a bug)
   - `@fix_category FIX_CAT_XXX` (required if toggleable in options)
   - `@notes <Details on registers, formulas, nuances>`

2. **Mandatory Deviation Logging (`docs/tracking/deviations.md`)**:
   Never fix an authentic 1997 bug (e.g. falling through meshes, car floating, camera clipping, audio cutoff) silently. Register it in `docs/tracking/deviations.md` with:
   - Unique ID (`DEV-XXX`)
   - Category (`FIX_CAT_NOCLIP`, `FIX_CAT_ELEVATION`, `FIX_CAT_CAMERA`, `FIX_CAT_AI_PATHING`, `FIX_CAT_AUDIO`, `FIX_CAT_RENDERER`)
   - Original address & assembly breakdown
   - Port solution
   - Option toggle key (`GameFixOptions` or `RendererOptions`)

3. **Granular Game Fix Controls**:
   All original bug workarounds must be individually toggleable under `Options > Gameplay > Game Fixes` and `Options > GFX Options`. Always support `1997 AUTHENTIC` (all fixes off) to preserve original behavior.

4. **Change Logging (`CHANGELOG.md`)**:
   Log all milestone progress, features, fixes, and reverse-engineering discoveries under `CHANGELOG.md` adhering to Keep a Changelog.

5. **Conventional Commits**:
   Commits must follow Conventional Commits (`feat`, `port`, `fix`, `re`, `docs`, `test`) and cite function addresses (`FUN_00412fc0 @ 0x00412fc0`) and deviation IDs (`DEV-001`).

6. **Automated Fidelity Verification**:
   Before submitting changes, run `uv run python tools/verify_fidelity.py` and ensure 100% pass without unannotated or mismatched functions.
