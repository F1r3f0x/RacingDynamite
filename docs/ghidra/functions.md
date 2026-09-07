# Master Function Registry (MAINDOS.EXE / IGN_WIN.EXE)

> Auto-generated from `database/decomp.db`. Edit via `tools/db.py`.

| DOS Addr | Win Addr | Ghidra Label | Symbol Name | Module | Status | Fidelity | Purpose |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| - | `0x004029a0` | `FUN_004029a0` | `Menu_Init` | `main.c` | Analyzed | - | Loads `MENU.COL`, `MENU.TAB`, `.LFT` fonts, and initializes menu options. |
| - | `0x00402c00` | `FUN_00402c00` | `Menu_Tick` | `main.c` | Decompiled | ADAPTED | Handles menu input navigation (arrows, Enter, Esc), item highlight, and transitions. |
| - | `0x0040e6b0` | `FUN_0040e6b0` | `Car_IntegratePosition` | `vehicle.c` | Decompiled | EXACT | World coordinate velocity integrator with 21.76 scale factor and 72 Hz timestep. |
| - | `0x004120a0` | `FUN_004120a0` | `WinMain` | `main.c` | Decompiled | ADAPTED | Main entry point; registers window class, queries timer, runs message/tick loop. |
| - | `0x00412230` | `FUN_00412230` | `App_FrameTick` | `main.c` | Analyzed | - | Main engine tick; dispatches Init (0), Main Loop (1), and Shutdown (2). |
| - | `0x00412500` | `FUN_00412500` | `App_Init` | `main.c` | Analyzed | - | Creates game window, initializes DirectDraw and DirectInput subsystems. |
| - | `0x00412530` | `FUN_00412530` | `App_Shutdown` | `main.c` | Analyzed | - | Releases DirectDraw surfaces, DirectSound, and window handles. |
| - | `0x00412580` | `FUN_00412580` | `Cdp_OpenFile` | `lisa3d.c` | Analyzed | - | Validates `"CDP\0"` header, dimensions, frame count, and embedded palette. |
| - | `0x00412610` | `FUN_00412610` | `Cdp_DecodeFrame` | `lisa3d.c` | Analyzed | - | Advances animation stream and triggers inter-frame delta decompression. |
| `0x0001fee0` | `0x00412670` | `FUN_00412670` | `Surface_LoadSRF` | `getsurf.c` | Decompiled | ADAPTED | Loads `.SRF` track surface heightfield and collision grid. |
| `0x0002002c` | `0x004127a0` | `FUN_004127a0` | `Surface_FreeSRF` | `getsurf.c` | Decompiled | ADAPTED | Frees active `.SRF` surface memory buffer. |
| `0x00020814` | `0x00412fc0` | `FUN_00412fc0` | `Surface_GetCell` | `getsurf.c` | Decompiled | EXTENDED | Computes grid cell index `((z/512)+50)*stride + ((x/512)+50)` with origin offset 50.0. |
| `0x00020bbc` | `0x00413380` | `FUN_00413380` | `Surface_GetTriangleHeight` | `getsurf.c` | Decompiled | ADAPTED | Computes average elevation `(y0 + y1 + y2) / -3` using vertex buffer indices from triangle. |
| - | `0x004133d0` | `FUN_004133d0` | `Font_DrawText` | `geputget.c` | Decompiled | ADAPTED | 2D bitmap font rasterizer blitting characters to 8bpp buffer. |
| - | `0x004134e0` | `FUN_004134e0` | `AI_FollowTrackSplines` | `main.c` | Analyzed | - | Steering simulation updating car heading, track chunk position, distance to centerline. |
| - | `0x00414e40` | `FUN_00414e40` | `Track_LoadSplines` | `main.c` | Decompiled | ADAPTED | Loads `.TRI` chunk indices, constructs left/right road boundary splines and AI waypoints. |
| - | `0x00416250` | `FUN_00416250` | `Mesh_InstantiatePlacedObjects` | `main.c` | Decompiled | ADAPTED | Applies `.PLC` world translation offsets to `.MSH` submesh vertices. |
| - | `0x00417270` | `FUN_00417270` | `Game_Init` | `main.c` | Analyzed | - | Sets initial game state flags, resets timers, initiates intro sequence. |
| - | `0x004172b0` | `FUN_004172b0` | `Game_StateDispatcher` | `main.c` | Decompiled | ADAPTED | Top-level game loop state machine dispatcher (Intro -> Menus -> Race). |
| - | `0x00418130` | `FUN_00418130` | `Load_SystemGraphicsAndFonts` | `geputget.c` | Analyzed | - | Loads `SYS.COL`, `N_SYSGFX.PIC`, `N_SYSG_2.PIC`, and `.LFT` fonts. |
| - | `0x00418dd0` | `FUN_00418dd0` | `Track_LoadAllAssets` | `main.c` | Analyzed | - | Master track loader: loads .COL, .PAN, .PIC, .SHD, .TAB, .MSH, .TEX, .POS. |
| - | `0x00419a90` | `FUN_00419a90` | `Track_LoadPlacements` | `main.c` | Decompiled | ADAPTED | Loads `.PLC` scenery object placement tables for level and cars. |
| - | `0x00419bd0` | `FUN_00419bd0` | `Mesh_LoadTrackAndCars` | `main.c` | Decompiled | ADAPTED | Loads `LEVELS/<TRACK>/<TRACK>.MSH` and `CARS/CARS.MSH` into geometry memory. |
| - | `0x00419d10` | `FUN_00419d10` | `Texture_LoadAllPages` | `main.c` | Decompiled | ADAPTED | Loads 1MB track `.TEX`, car `.TEX`, and 64KB aligned sprite pages. |
| - | `0x0041ac40` | `FUN_0041ac40` | `Font_LoadHUDFonts` | `geputget.c` | Analyzed | - | Loads HUD lettering glyphs (`IGNITION.FNT`, `yellow.lft`, `speed.lft`, etc.). |
| - | `0x0041af70` | `FUN_0041af70` | `Track_LoadOverlayGfx` | `geputget.c` | Analyzed | - | Loads track sign textures and winner trophy bitmap (`POKAL.PIC`). |
| - | `0x0041b360` | `FUN_0041b360` | `Track_PreprocessPlacements` | `main.c` | Decompiled | ADAPTED | Unpacks `model_type` bitfields (`& 0xFFF`) and extracts animation and flag channels. |
| - | `0x0041b470` | `FUN_0041b470` | `Race_InitSceneAndCars` | `main.c` | Analyzed | - | Instantiates player/AI cars on starting grid and binds scenery collision. |
| - | `0x0041d190` | `FUN_0041d190` | `Car_UnpackMeshGeometry` | `lisa3d.c` | Decompiled | EXTENDED | Extracts `CARS.MSH` submesh vertices, finds bottom tire vertex $\max(v_y)$ for ground alignment, and scales by $21.76$. |
| - | `0x0041f9b0` | `Sound_InitAndLoadPools` | `Sound_InitAndLoadPools` | `main.c` | Decompiled | ADAPTED | Initializes 32 DirectSound-compatible audio channels; loads SFX pools (ROLL, SKID, COLL, BOOST, DIV, KLICK, OK); loads track sounds; reads per-vehicle `ENGINE.INF` 800-byte curves into `uint8_t[200]` vol/pitch arrays at `+0x658`, `+0x720`, `+0x7e8`, `+0x8b0`. |
| - | `0x00422680` | `FUN_00422680` | `Race_ResolveVehicleCollisions` | `main.c` | Analyzed | - | Inter-vehicle and scenery obstacle collision detection and impulse response. |
| - | `0x00423aa0` | `FUN_00423aa0` | `Car_VerticalDynamics` | `vehicle.c` | Decompiled | EXACT | Gravity acceleration (-0.2/tick), rebound bounce on impact, and ride height equilibrium (+5.0). |
| - | `0x00424570` | `Car_PhysicsTick` | `Car_PhysicsTick` | `getsurf.c` | Decompiled | EXTENDED | Master 72 Hz vehicle dynamics: 4-wheel independent raycast suspension, pitch/roll tilt, bicycle lateral slip, turbo boost. |
| - | `0x00427d70` | `FUN_00427d70` | `Car_UpdateAxleSpeeds` | `main.c` | Decompiled | EXACT | Averages left and right wheel velocities for front and rear axles with factor 0.5. |
| - | `0x00429a40` | `FUN_00429a40` | `Race_CheckCheckpointTriggers` | `main.c` | Analyzed | - | Tests vehicle collision against type 150..154 split-time checkpoint gates. |
| - | `0x004356d0` | `FUN_004356d0` | `Pos_InitAnimatedObjects` | `lisa3d.c` | Decompiled | ADAPTED | Converts keyframe coordinates in `.POS` to relative displacement deltas. |
| - | `0x004357a0` | `FUN_004357a0` | `Pos_UpdateAnimatedObjects` | `lisa3d.c` | Decompiled | ADAPTED | Advances keyframe playheads and translates moving scenery objects via `Lisa_MoveObject`. |
| - | `0x00436990` | `FUN_00436990` | `Race_RenderViewport` | `main.c` | Decompiled | EXTENDED | Calculates camera transform, invokes scene renderer, draws HUD. |
| - | `0x00438210` | `FUN_00438210` | `Lisa_RenderPanorama` | `lisa3d.c` | Analyzed | - | Cylindrical horizon background blitter sampling 64KB `.PAN` texture using camera yaw and pitch angles. |
| - | `0x0043c910` | `FUN_0043c910` | `Camera_UpdateChase` | `main.c` | Decompiled | EXTENDED | Multi-mode chase camera with velocity lookahead, 0.125 azimuth lag, slope adaptation (DEV-004). |
| - | `0x0043e2a0` | `FUN_0043e2a0` | `Lisa_Init` | `lisa3d.c` | Decompiled | ADAPTED | Initializes Lisa 2 rasterizer viewport, Z-buffer, and focal lengths. |
| - | `0x00442030` | `FUN_00442030` | `Car_ApplySteering` | `vehicle.c` | Decompiled | EXACT | Speed-attenuated front wheel steering lock and smoothing filter. |
| - | `0x00442670` | `FUN_00442670` | `Car_PowertrainUpdate` | `vehicle.c` | Decompiled | EXACT | Engine propulsion, rolling and aerodynamic drag, transmission forward/reverse gear shifting. |
| - | `0x004452c0` | `FUN_004452c0` | `Sound_SynthesizeEngineRPM` | `main.c` | Decompiled | EXTENDED | Computes RPM pitch modulation from 800-byte ENGINE.INF curve with DEV-006 protection. |
| `0x00020c18` | `0x00446578` | `FUN_00446578` | `Surface_TestTrianglePositiveDZ` | `getsurf.c` | Decompiled | EXACT | 2D trapezoidal slope span test for table2 triangles ($dz \ge 0$). |
| `0x00020c81` | `0x004465e1` | `FUN_004465e1` | `Surface_TestTriangleNegativeDZ` | `getsurf.c` | Decompiled | EXACT | 2D trapezoidal slope span test for table1 triangles ($dz < 0$). |
| - | `0x004466d0` | `FUN_004466d0` | `Lisa_RenderScene` | `lisa3d.c` | Decompiled | EXTENDED | Master 3D frame render: culls objects, transforms vertices, rasterizes spans. |
| - | `0x004468d0` | `FUN_004468d0` | `Lisa_InitEngineMemory` | `lisa3d.c` | Decompiled | ADAPTED | Allocates internal rasterizer buffers, vertex streams, and matrices. |
| - | `0x00448e70` | `FUN_00448e70` | `Lisa_FrustumCullObjects` | `lisa3d.c` | Decompiled | ADAPTED | Spatial grid frustum culler populating visible object list. |
| - | `0x00449e70` | `FUN_00449e70` | `Lisa_TransformVertices` | `lisa3d.c` | Decompiled | ADAPTED | Camera matrix rotation, perspective projection, and backface culling. |
| - | `0x0044b480` | `FUN_0044b480` | `Lisa_InitOpcodeTable` | `lisa3d.c` | Decompiled | ADAPTED | Binds polygon opcode rasterization dispatch table (`PTR_LAB_0049c8e0`). |
| - | `0x0044c1f0` | `FUN_0044c1f0` | `Lisa_RenderSubmeshes` | `lisa3d.c` | Decompiled | ADAPTED | Dispatches polygon opcodes across all visible transformed submeshes. |
| - | `0x0044caa0` | `LAB_0044caa0` | `Lisa_DrawPolygon_Op12` | `lisa3d.c` | Decompiled | EXACT | Opcode 0x12: 1-bit transparent cutout triangle (pushes `g_pLisaTransparencyLUT`). |
| - | `0x0044cac0` | `LAB_0044cac0` | `Lisa_DrawPolygon_Op13` | `lisa3d.c` | Decompiled | EXACT | Opcode 0x13: shadow / foliage alpha blend triangle (pushes `g_pActiveSHD`). |
| - | `0x0044cae0` | `LAB_0044cae0` | `Lisa_DrawPolygon_Op16` | `lisa3d.c` | Decompiled | EXACT | Opcode 0x16: transparent cutout variant. |
| - | `0x0044cb00` | `LAB_0044cb00` | `Lisa_DrawPolygon_Op17` | `lisa3d.c` | Decompiled | EXACT | Opcode 0x17: shadow / alpha blend Gouraud triangle (pushes `g_pActiveSHD`). |
| - | `0x0044cb20` | `FUN_0044cb20` | `Lisa_DrawTriangle_OpcodeHelper` | `lisa3d.c` | Decompiled | ADAPTED | Common backface test, attribute pack, and span bucketer for opcodes 0x12, 0x13, 0x16, 0x17. |
| - | `0x0044d550` | `FUN_0044d550` | `Lisa_DrawTexturedTriangle_Op15` | `lisa3d.c` | Decompiled | ADAPTED | Opcode 0x15: perspective-correct textured triangle with 16.16 UV interpolation. |
| - | `0x0044f0e9` | `FUN_0044f0e9` | `Lisa_ExecuteRasterizerCommands` | `lisa3d.c` | Decompiled | ADAPTED | Traverses depth-bucket sorted polygon command list and executes rasterizers. |
| - | `0x00452800` | `FUN_00452800` | `Lisa_RenderTexturedTriangle_Op11` | `lisa3d.c` | Decompiled | EXACT | Opcode 0x11 triangle edge walker and span setup for unshaded texture mapping. |
| - | `0x004537dc` | `FUN_004537dc` | `Lisa_DrawTexturedSpan_Op11` | `lisa3d.c` | Decompiled | EXACT | Low-level perspective/affine textured span blitter reading texels directly with stride 256 and alpha test. |
| - | `0x00456270` | `FUN_00456270` | `Font_Load` | `geputget.c` | Decompiled | ADAPTED | Loads and parses `.LFT` font header, offset tables, widths, and glyph raster data. |
| - | `0x00456c40` | `FUN_00456c40` | `Video_SetPalette` | `lisa3d.c` | Decompiled | ADAPTED | Uploads 256-color RGB palette to DirectDraw / hardware DAC. |
| - | `0x004574a0` | `FUN_004574a0` | `File_LoadToMemory` | `mem.c` | Decompiled | ADAPTED | Generic binary loader (`fopen`, `fread` into allocated buffer). |
| - | `0x00457890` | `FUN_00457890` | `Audio_MixCallback` | `main.c` | Decompiled | ADAPTED | 32-channel software voice mixer with 16.16 fixed-point linear pitch resampling and stereo panning. |
| - | `0x00457980` | `FUN_00457980` | `Audio_StopVoice` | `main.c` | Decompiled | ADAPTED | Immediately stops voice playback and releases mixer channel allocation. |
| - | `0x004579b0` | `FUN_004579b0` | `Audio_PlayVoice` | `main.c` | Decompiled | ADAPTED | Allocates mixer channel voice, configures volume, pan, loop flag, and starts playback. |
| - | `0x00457aa0` | `FUN_00457aa0` | `Audio_SetVoiceParams` | `main.c` | Decompiled | ADAPTED | Real-time modulation of voice pitch frequency and stereo pan position. |
| - | `0x00457ed0` | `FUN_00457ed0` | `Music_PlayTrack` | `main.c` | Decompiled | ADAPTED | CD-DA track streamer using stb_vorbis mapped to circuits via DAT_00497eb8. |
| - | `0x004582b0` | `FUN_004582b0` | `Sound_LoadPAT` | `main.c` | Decompiled | ADAPTED | Loads Gravis UltraSound GF1 .PAT patch audio files and converts 8-bit/16-bit linear PCM to S16SYS format. |
| - | `0x00458e00` | `FUN_00458e00` | `Sound_LoadWAV` | `main.c` | Decompiled | ADAPTED | Loads RIFF/WAVE PCM 8-bit/16-bit audio file and converts to S16SYS format. |
| - | `0x0045b4f0` | `FUN_0045b4f0` | `Lisa_PrintVersion` | `lisa3d.c` | Analyzed | - | Prints "Lisa 2 Development System" banner and build timestamp. |
| - | `0x00469950` | `entry` | `CRT_Entry` | `MSVC CRT` | Analyzed | - | C Runtime startup entry point, parses command line, calls `WinMain`. |
| - | `0x00499abc` | `FUN_00499abc` | `Cdp_DecompressRLE` | `lisa3d.c` | Analyzed | - | Delta-skip RLE decompression modifying active frame buffer with opcode skip codes. |
