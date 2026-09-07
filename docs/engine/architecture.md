# Ignition (1997) Engine Architecture & Subsystems

## 1. Subsystem Architecture Overview

**Racing Dynamite** is structured as a modular C11/SDL2 engine designed for 100% behavioral parity with the 1997 Windows 95 release (`IGN_WIN.EXE`) and DOS release (`MAINDOS.EXE`).

```
+-----------------------------------------------------------------------------------+
|                                  Racing Dynamite                                  |
+-----------------------------------------------------------------------------------+
|  [Platform Layer (platform_sdl.c)]                                                |
|  - SDL2 Windowing & Virtual 8bpp Framebuffer Blitter                              |
|  - High-resolution timing (Platform_GetTicks)                                     |
|  - Input abstraction (Keyboard, Mouse, SDL_GameController)                        |
|  - Audio streaming & device callback                                             |
+-----------------------------------------------------------------------------------+
|  [Core State Machine & Main Dispatcher (main.c, game_state.c, race_session.c)]    |
|  - App_FrameTick (0x00412230) / Game_StateDispatcher (0x004172b0)                 |
|  - Intro (Logos / CDP) -> Menus -> Loading -> Active Race -> Results             |
|  - Checkpoint tracking, lap counting, and leaderboard ranking                     |
+-----------------------------------------------------------------------------------+
|  [Vehicle Physics Engine (physics/car_physics.c, getsurf.c)]                      |
|  - Surface heightfield raycasting via getsurf (0x00412fc0)                        |
|  - 4-wheel independent spring-damper suspension                                   |
|  - Longitudinal traction, torque curves (ENGINE.INF), braking, reverse            |
|  - Lateral slip, drifting mechanics, angular dynamics (pitch/roll/yaw)            |
|  - Obstacle & car-to-car collision resolution (0x00422680)                        |
+-----------------------------------------------------------------------------------+
|  [Lisa3D Software Rasterizer (renderer/rasterizer.c, camera.c)]                   |
|  - Perspective-correct barycentric polygon rasterization (8bpp indexed color)     |
|  - Floating-point / 16.16 depth buffering (Z-buffer)                              |
|  - 1024x1024 texture sampling (.TEX), 64KB lighting (.TAB), 64KB shadow (.SHD)   |
|  - Opcode dispatch (0x11 textured, 0x12 transparent, 0x13 shadow, 0x15 lit tex)   |
|  - Dynamic scenery animation (.POS) and 360-degree cylindrical panorama (.PAN)   |
+-----------------------------------------------------------------------------------+
|  [AI Opponent Driving Subsystem (ai/ai_driver.c)]                                 |
|  - Spline node pursuit along road ribbon (.TRI, TrackSplineNode)                  |
|  - Curvature-based speed & corner braking (kappa = dHeading / dDistance)         |
|  - Overtaking logic, lane selection, collision avoidance                          |
|  - Difficulty tiers (Novice, Amateur, Pro) & catch-up rubber banding              |
+-----------------------------------------------------------------------------------+
|  [Audio & Sound Subsystem (audio/audio_sdl.c)]                                    |
|  - DirectSound emulation via 16-channel software voice mixer                      |
|  - Dual-sample engine pitch & volume crossfade modulation via ENGINE.INF          |
|  - SFX pools (General, Level, Car) & spatial 3D attenuation                       |
|  - Streaming Redbook CD-DA soundtrack playback (OGG Vorbis)                       |
+-----------------------------------------------------------------------------------+
|  [Binary Format Loaders (formats/)]                                               |
|  - .COL (Palettes), .PIC (Bitmaps), .SRF (Surface), .MSH (Meshes), .PLC (Objects) |
|  - .TEX (Textures), .TAB (Shading), .SHD (Shadows), .TRI (Splines), .LFT (Fonts) |
|  - .POS (Keyframe Scenery Animations), .CDP (FMV Video), .INF (Engine Curves)     |
+-----------------------------------------------------------------------------------+
```

---

## 2. Coordinate System Conventions

Ignition utilizes a right-handed Cartesian world coordinate system:

* **$X$-Axis (East / West)**:
  * Horizontal axis across the track terrain.
  * Typical coordinates range from $-10,000$ to $+10,000$ world units.
* **$Y$-Axis (Elevation)**:
  * World coordinate space: positive values denote elevation **above** sea level; negative values denote elevation **below** into canyons, valleys, or underwater tunnels.
  * In certain internal vertex arrays of `IGN_WIN.EXE` and `getsurf.c`, the elevation coordinate is inverted (`-Y`) when loading from `.MSH` or calculating triangle heights `(y0 + y1 + y2) / -3`.
* **$Z$-Axis (North / South)**:
  * Horizontal depth axis along the track terrain.
  * Typical coordinates range from $-10,000$ to $+10,000$ world units.
* **Angular Conventions (Euler Angles)**:
  * **Yaw ($\theta_{\text{yaw}}$)**: Azimuth heading in the horizontal $XZ$ plane ($0^\circ = \text{North}$, $+90^\circ = \text{East}$).
  * **Pitch ($\theta_{\text{pitch}}$)**: Elevation angle tilting above or below the horizontal plane.
  * **Roll ($\theta_{\text{roll}}$)**: Lateral chassis bank angle along the vehicle's forward axis.
  * Angles in `.POS` keyframe data are stored as signed 32-bit integers in **tenths of a degree** ($10 = 1.0^\circ$, $3600 = 360.0^\circ$).

---

## 3. Original Memory Model & Asset Allocation

In the original DOS/Win95 binaries (`mem.c` at `0x004574a0`), memory was organized into static arenas managed by custom block allocators:

1. **System Memory Pool**:
   * Allocated on engine startup (`App_Init` at `0x00412500`).
   * Hosts UI fonts (`.LFT`), system color palettes (`SYS.COL`), and base GUI sprites (`N_SYSGFX.PIC`).
2. **Level Asset Arena (`g_LevelMemoryPool`)**:
   * Cleared and re-allocated upon each circuit load in `Track_LoadAllAssets` (`0x00418dd0`).
   * Contains track geometry (`.MSH`), surface collision (`.SRF`), placement tables (`.PLC`), texture pages (`.TEX`), shading tables (`.TAB`), shadow lookup (`.SHD`), dynamic keyframes (`.POS`), and sky panorama (`.PAN`).
3. **Car Geometry & Audio Pool**:
   * Allocates `CARS/CARS.MSH`, `CARS.PLC`, `CARS.TEX`, and individual vehicle sound pools.
4. **Framebuffer & Rasterizer Buffers**:
   * Fixed $640 \times 480 \times 1\text{ byte}$ virtual color buffer (307,200 bytes).
   * Matching $640 \times 480 \times 4\text{ bytes}$ Z-buffer (1,228,800 bytes).
   * 6,000 depth-bucket linked list pointers (`g_pLisaDepthBuckets` at `0x0063b5e8`) for ordered polygon rasterization.

In the source port, dynamic allocations are encapsulated cleanly in standard C structures with explicit constructors (`*_LoadFromFile`) and destructors (`*_Free`), preventing memory leaks while retaining authentic data alignments (e.g. 64KB texture page boundaries).

---

## 4. Frame Timing & Simulation Loop

* **Target Refresh Rate**: 60 Hz (~16.6 ms per frame).
* **High-Resolution Clock**:
  * Original Win95 code queried `QueryPerformanceCounter` / `QueryPerformanceFrequency` (`0x004120a0`).
  * Source port uses `Platform_GetTicks` (`SDL_GetTicks`) with delta time clamping:
    ```c
    uint32_t now = Platform_GetTicks();
    uint32_t delta_ms = (now > last_time) ? (now - last_time) : 16;
    if (delta_ms > 100) delta_ms = 100; // Clamp lag spikes
    last_time = now;
    ```
* **Decoupled Simulation**:
  * Game logic and state dispatch (`Game_Update`) step with delta time.
  * Vehicle dynamics integrate with fixed sub-steps (e.g. 120 Hz) for numerical stability during high-speed collisions and steep slope transitions.
  * Rasterization (`Game_Render`) produces an 8-bit paletted frame presenting to the screen through SDL2 hardware texture streaming with linear filtering.
