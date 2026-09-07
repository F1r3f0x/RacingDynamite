# Changelog

All notable changes to **Racing Dynamite** (Ignition 1997 source port) will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]

### Added
- **Phase 5: Dynamic Chase Camera & Authentic Audio Subsystem**:
  - Multi-mode dynamic chase camera (`src/renderer/camera.c`) with 5 views (Classic Isometric, Close Chase, Far Chase, Bumper View, Free Orbit) supporting dynamic lookahead along velocity, 0.125 spring-damper azimuth lag, shortest-path angle wrapping, and terrain slope pitch adaptation (`0x0043c910`, `DEV-004`).
  - 32-channel software voice mixer (`src/audio/audio.c`) with 16.16 fixed-point linear resampling, equal-power stereo panning, master volume attenuation, and saturation clamping (`0x00459010`, `0x0041fad0`).
  - Vehicle acoustic synthesizer (`src/audio/engine_audio.c`) parsing 800-byte `ENGINE.INF` curves and modulating dual looping engine audio samples (`00_A.WAV`, `01_A.WAV`) in real-time (`0x004452c0`, `DEV-006`).
  - General SFX pool (`src/audio/sound_pool.c`) loading RIFF/WAVE PCM assets (turbo boost `00_BOOST.WAV`, UI clicks, tire skids, collision impacts, landing thuds).
  - CD-DA digital music streaming (`src/audio/music.c`) via `stb_vorbis` mapped to circuits via authentic track table `DAT_00497eb8` (`0x0041fc80`).
  - Added camera and audio test suites (`tests/test_camera.c`, `tests/test_audio.c`).
  - Detailed architectural documentation in `docs/engine/audio.md`.
- **Phase 4: Tooling Hygiene, Authentic Multi-Backend Renderer & Fidelity Tracking (FCTS)**:
  - Created `tools/README.md` cataloguing all 33 active reverse engineering, inspection, and asset pipeline scripts.
  - Implemented `docs/tracking/fidelity_strategy.md` defining standardized inline code provenance headers (`@original`, `@fidelity`, `@deviation`, `@fix_category`).
  - Implemented `docs/tracking/deviations.md` cataloguing authentic divergences and bug fixes (`DEV-001` through `DEV-006`).
  - Implemented granular per-category game fix options (`GameFixCategory`, `GameFixOptions`) allowing independent toggling of No-Clip, Elevation, Camera, AI Navigation, and Audio fixes.
  - Added multi-backend renderer architecture (`RendererBackendType`) supporting Lisa3D software rasterization, 3dfx Glide, and Direct3D modes.
  - Automated parity verification tool `tools/verify_fidelity.py` integrated into CMake/CTest.

### Changed
- Pruned 17 obsolete, redundant, or one-off exploratory scratch scripts from `tools/`.
- Updated `docs/roadmap.md` and `docs/README.md` to introduce Phase 4 milestone.

### Fixed
- **Vehicle Roster, Audio & Scenery Animation Parity**:
  - Expanded vehicle roster from 8 to all 11 authentic Ignition archetypes (COOP, EVOR, BUGGY, ENFORCER, RED-DEVIL, SCHOOL BUS, SMOKE, BUG, MONSTER, VEGAS, IGNITION) mapped to exact `CARS.MSH` submesh offsets and vehicle physics specs (`0x00494a70`, `0x00495800`).
  - Added Gravis UltraSound GF1 `.PAT` patch loader (`SoundPool_LoadPat`, `FUN_004582b0`) with transparent `.PAT` fallback in `SoundPool_LoadWav`, enabling engine and horn audio for COP, JEEP, SCHOOL, TRUCK, and VAN.
  - Resolved vehicle sinking/ground clipping in `src/renderer/rasterizer.c` by subtracting `max_vy` (lowest tire vertex) in chassis ground alignment formula per `FUN_0041d190`.
  - Tuned steering dynamics in `src/physics/vehicle.c` with speed-dependent lock attenuation, angular yaw acceleration clamping, and smoothing filter (`FUN_00442030`).
  - Implemented `.POS` scenery keyframe animation track loader and per-frame update loop (`src/formats/pos.c`, `0x004356d0`, `0x004357a0`), bringing moving scenery objects (windmills, ski lifts, boats, airplanes, blimps) to life across all 7 tracks.
- **Binary Format & Surface Physics Parity**:
  - Fixed `SrfCell` spatial grid layout in `include/ignition/formats.h` and `src/physics/getsurf.c`: replaced packed `flags` field with discrete `table1_count` (offset `+0x08`) and `table2_count` (offset `+0x0A`), verified via `Surface_LoadSRF` (`0x00412670`) and `Surface_GetCell` (`0x00412fc0`).
  - Renamed `MshPolygon` texture coordinates to non-colliding `tu0/tv0..` across `include/ignition/formats.h`, `docs/ghidra/structs.md`, `docs/formats/msh_tri.md`, and `src/renderer/rasterizer.c`.
  - Corrected `ENGINE.INF` curve specification in `docs/engine/audio.md` to authentic 4 x 200 `uint8_t` lookup tables (vol/pitch envelopes) verified against `Sound_InitAndLoadPools` (`0x0041f9b0`).
  - Documented general sprite sub-TEX pages (`GENERAL/LIGHT`, `DARKSMOK`, `EXSMOKE`) in `docs/formats/tex_tab.md` from `Texture_LoadAllPages` (`0x00419d10`).
  - Clarified chase camera lag factor sign convention (`|DAT_0047a720| = |-0.125| = 0.125`) in `docs/engine/camera.md` and `src/renderer/camera.c`.
  - Merged duplicate `Sound_InitAndLoadPools` (`0x0041f9b0`) entry in `docs/ghidra/functions.md`.

---

## [0.3.0] - 2026-09-04

### Added
- **Phase 3: In-Race 3D Car & Dynamic Scenery Rendering**:
  - 3D vehicle instancing loading `CARS.MSH` chassis geometry and `CARS.TEX` textures.
  - 4-wheel independent suspension visualization with dynamic wheel spin and steering angle articulation.
  - Dual console/disk file logging engine with log rotation and in-race telemetry capture (`src/core/log.c`, `include/ignition/log.h`).
  - Dedicated `Options > Gameplay > Game Fixes` menu screen with original game bug workaround toggle.

### Fixed
- **Vehicle Orientation & Scaling (`DEV-002`, `DEV-004`)**:
  - Scaled 3D car models by authentic `PHYSICS_SCALE_FACTOR` (`21.76`).
  - Corrected right-handed camera basis vector normalization to eliminate gimbal flip.
  - Fixed vehicle floating above ground on Austria circuit and sinking on Brazil circuit.
- **Mountain Wall Adhesion (`DEV-003`)**:
  - Clamped maximum slope climbing angle using `PHYSICS_MAX_SLOPE_SIN` to prevent vertical wall driving.

---

## [0.2.0] - 2026-09-02

### Added
- **Phase 2: Authentic Vehicle Physics & Surface Raycasting (`getsurf.c`)**:
  - Reverse-engineered `Surface_GetCell` (`0x00412fc0`) and `Surface_GetTriangleHeight` (`0x00413380`).
  - 4-wheel independent raycast suspension model matching authentic 72 Hz fixed timestep (`PHYSICS_DT_SEC`).
  - Longitudinal traction, engine torque curve, lateral tire slip friction, and chassis pitch/roll tilt.
  - Automated physics test suite `tests/test_physics.c`.

### Fixed
- **Surface Collision Grid Out-of-Bounds (`DEV-001`)**:
  - Added boundary safety clamp preventing memory access violations on off-track jumps.

---

## [0.1.0] - 2026-08-30

### Added
- **Phase 1: Stabilization & Asset Pipeline**:
  - Reverse-engineered file formats: `.COL`, `.PIC`, `.SRF`, `.MSH`, `.TRI`, `.PLC`, `.TEX`, `.TAB`, `.LFT`, `ENGINE.INF`.
  - Core game loop state machine dispatcher (`FUN_004172b0`).
  - Lisa3D software rasterizer foundations and 2D bitmap font blitter (`FUN_004133d0`).
  - Asset extraction pipeline `tools/extract_assets.py` for GOG / CD media.
  - Unit tests: `tests/test_formats.c` and `tests/test_game_states.c`.
