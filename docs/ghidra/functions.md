# Master Function Registry (MAINDOS.EXE)

> Auto-generated from `database/decomp.db`. Edit via `tools/db.py`.

| DOS Addr | Win Addr | Ghidra Label | Symbol Name | Module | Status | Fidelity | Purpose |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `0x00060f40` | - | `-` | `File_ReadToBuffer` | `mem.c` | Matching | EXACT | Reads binary file directly into preallocated buffer |
| `0x00061100` | - | `-` | `File_GetSize` | `mem.c` | Matching | EXACT | Seeks to end and returns binary file size |
| `0x0006117c` | - | `-` | `File_Exists` | `mem.c` | Matching | EXACT | Tests if file exists by attempting fopen |
| `0x00011504` | - | `FUN_004029a0` | `Menu_Init` | `menu.c` | Decompiled | EXACT | Loads MENU.COL, MENU.TAB, .LFT fonts, and initializes menu options. |
| `0x00012bb8` | - | `FUN_00402c00` | `Menu_Tick` | `menu.c` | Decompiled | EXACT | Handles menu input navigation (arrows, Enter, Esc), item highlight, and transitions. |
| `0x10060` | - | `FUN_00412230` | `App_FrameTick` | `main.c` | Decompiled | EXACT | Main engine tick; dispatches Init (0), Main Loop (1), and Shutdown (2). |
| - | - | `FUN_00412500` | `App_Init` | `main.c` | Unidentified | - | Creates game window, initializes DirectDraw and DirectInput subsystems. |
| `0x215c8` | - | `FUN_00412530` | `App_Shutdown` | `main.c` | Decompiled | EXACT | Releases DirectDraw surfaces, DirectSound, and window handles. |
| `0x000552fc` | - | `FUN_00412580` | `Cdp_OpenFile` | `lisa3d.c` | Decompiled | EXACT | Decodes CDP movie file header and initializes playback stream |
| `0x00055384` | - | `FUN_00412610` | `Cdp_DecodeFrame` | `lisa3d.c` | Decompiled | EXACT | Advances CDP video stream and decompresses next frame |
| `0x0001fee0` | - | `FUN_00412670` | `Surface_LoadSRF` | `getsurf.c` | Decompiled | ADAPTED | Loads .SRF track collision surface, converts relative offsets to pointers |
| `0x0002002c` | - | `FUN_004127a0` | `Surface_FreeSRF` | `getsurf.c` | Decompiled | ADAPTED | Frees active .SRF surface memory buffer. |
| `0x00020814` | - | `FUN_00412fc0` | `Surface_Raycast` | `getsurf.c` | Decompiled | EXTENDED | Spatial grid query, candidate selection, cross product normal, world vertex transform |
| `0x00020bbc` | - | `FUN_00413380` | `Surface_GetTriangleHeight` | `getsurf.c` | Decompiled | ADAPTED | Computes average elevation (y0 + y1 + y2) / -3 using vertex buffer indices from triangle. |
| `0x00043d60` | - | `FUN_004133d0` | `Font_DrawHUDText` | `geputget.c` | Decompiled | EXACT | 2D bitmap font rasterizer blitting characters from IGNITION.FNT directly to 8bpp framebuffer. |
| `0x1aea0` | - | `FUN_004134e0` | `AI_FollowTrackSplines` | `main.c` | Decompiled | EXACT | Steering simulation updating AI vehicle heading, track spline waypoint progression, speed moderation, and obstacle evasion. |
| `0x1ce1c` | - | `FUN_00414e40` | `Track_LoadSplines` | `main.c` | Decompiled | EXACT | Loads .TRI chunk indices, constructs left/right road boundary splines and AI waypoints. |
| `0x20d18` | - | `FUN_00417270` | `Game_Init` | `main.c` | Decompiled | EXACT | Sets initial game state flags, resets timers, initiates intro sequence. |
| `0x20d60` | - | `FUN_004172b0` | `Game_StateDispatcher` | `main.c` | Decompiled | EXACT | Top-level game loop state machine dispatcher (Intro -> Menus -> Race). |
| `0x00021860` | - | `FUN_00418130` | `Load_SystemGraphicsAndFonts` | `geputget.c` | Decompiled | EXACT | Loads SYS.COL, N_SYSGFX.PIC, N_SYSG_2.PIC, and .LFT fonts. |
| - | - | `FUN_00418dd0` | `Track_LoadAllAssets` | `main.c` | Unidentified | - | Master track loader: loads .COL, .PAN, .PIC, .SHD, .TAB, .MSH, .TEX, .POS. |
| `0x230f4` | - | `FUN_00419a90` | `Track_LoadPlacements` | `main.c` | Decompiled | ADAPTED | Loads .PLC scenery object placement tables for level and cars. |
| `0x2322c` | - | `FUN_00419bd0` | `Mesh_LoadTrackAndCars` | `main.c` | Decompiled | ADAPTED | Loads LEVELS/<TRACK>/<TRACK>.MSH and CARS/CARS.MSH into geometry memory. |
| `0x23368` | - | `FUN_00419d10` | `Texture_LoadAllPages` | `main.c` | Decompiled | ADAPTED | Loads 1MB track .TEX, car .TEX, and 64KB aligned sprite pages. |
| `0x000240f4` | - | `FUN_0041ac40` | `Font_LoadHUDFonts` | `geputget.c` | Decompiled | EXACT | Loads HUD lettering glyphs (IGNITION.FNT, yellow.lft, speed.lft, etc.). |
| `0x000243e0` | - | `FUN_0041af70` | `Track_LoadOverlayGfx` | `geputget.c` | Decompiled | EXACT | Loads track sign textures and winner trophy bitmap (POKAL.PIC). |
| `0x247b0` | - | `FUN_0041b360` | `Track_PreprocessPlacements` | `main.c` | Decompiled | ADAPTED | Unpacks model_type bitfields (& 0xFFF) and extracts animation and flag channels. |
| `0x2489c` | - | `FUN_0041b470` | `Race_InitSceneAndCars` | `main.c` | Decompiled | EXACT | Instantiates player and AI cars on starting grid, loads track surface, and binds collision and physics states. |
| `0x00016616` | - | `FUN_0041d190` | `Car_UnpackMeshGeometry` | `lisa3d.c` | Decompiled | EXACT | Extracts CARS.MSH submesh vertices, finds bottom tire vertex $\max(v_y)$ for ground alignment, and scales by $21.76$. |
| `0x2a4f1` | - | `Sound_InitAndLoadPools` | `Sound_InitAndLoadPools` | `main.c` | Decompiled | EXACT | Initializes 32 DirectSound-compatible audio channels; loads SFX pools (ROLL, SKID, COLL, BOOST, DIV, KLICK, OK); loads track sounds; reads per-vehicle ENGINE.INF 800-byte curves into uint8_t[200] vol/pitch arrays at +0x658, +0x720, +0x7e8, +0x8b0. |
| `0x2a40e` | - | `FUN_00420090` | `Obstacle_TriggerAction` | `main.c` | Decompiled | EXACT | Evaluates type 0xFA (250) trigger obstacles and triggers associated actions. |
| `0x2a4ec` | - | `FUN_004200e0` | `AI_InitSteeringConeLookup` | `main.c` | Decompiled | EXACT | Precomputes 800x405 lookahead steering cone and obstacle threat table. |
| `0x2a97a` | - | `FUN_00420240` | `Ghost_LoadCarAndPath` | `main.c` | Decompiled | EXACT | Loads recorded Time Attack ghost car trajectory from GHOSTS\%s.GST. |
| `0x2adf8` | - | `FUN_00420870` | `Track_LoadBinaryCache` | `main.c` | Decompiled | EXACT | Loads preprocessed level data cache (ign_win.btz / ign_dos.btz). |
| `0x2b42d` | - | `FUN_00420990` | `Sound_FreeAllSounds` | `main.c` | Decompiled | EXACT | Frees active level audio buffers and sample tables upon track unload. |
| `0x2b51a` | - | `FUN_00420b70` | `Game_Shutdown` | `main.c` | Decompiled | EXACT | Releases all allocated game memory and shuts down engine subsystems cleanly. |
| `0x3b996` | - | `FUN_00420c00` | `Timer_GetDeltaTime` | `main.c` | Decompiled | EXACT | Computes elapsed frame delta time using tick counter scaled by 0.036. Updates accumulator, frame counter, and clamps delta to max 10.8 ticks. |
| `0x2b9d3` | - | `FUN_00420d10` | `Race_UpdateCountdownAndFinish` | `main.c` | Decompiled | EXACT | Updates start-line traffic light timer and checks race winner victory condition. |
| `0x2b9e4` | - | `FUN_00420e60` | `HUD_UpdateRaceTimes` | `main.c` | Decompiled | EXACT | Updates player split times and current lap timer display on in-game HUD. |
| `0x2c5b8` | - | `FUN_00420eb0` | `Input_ProcessRaceHotkeys` | `main.c` | Decompiled | EXACT | Polls keyboard hotkeys during race: Pause (P), Escape menu, camera toggle, and volume. |
| `0x2ccc5` | - | `FUN_00421bc0` | `Input_PollPlayerVehicleControls` | `main.c` | Decompiled | EXACT | Reads keyboard / joystick axes and maps to vehicle steering, throttle, brake, and turbo. |
| `0x2cf5d` | - | `FUN_004222b0` | `Ghost_SaveCarAndPath` | `main.c` | Decompiled | EXACT | Serializes recorded lap waypoint trajectory into GHOSTS\%s.GST. |
| `0x2cfbc` | - | `FUN_004225d0` | `Track_SaveBinaryCache` | `main.c` | Decompiled | EXACT | Saves preprocessed level collision cache file. |
| `0x2d008` | - | `FUN_00422680` | `Race_ResolveVehicleCollisions` | `main.c` | Decompiled | EXACT | Inter-vehicle and scenery obstacle collision detection and impulse response. Fixed 72 Hz physics integration coordinator. |
| `0x2de2e` | - | `FUN_00423620` | `Car_HandleElimination` | `main.c` | Decompiled | EXACT | Checks trailing vehicle elimination condition in knock-out races. Marks vehicle blown (+0x354 = 1), sets race status, and logs elimination string. |
| `0x2e234` | - | `FUN_00423aa0` | `Car_VerticalDynamics` | `main.c` | Decompiled | EXACT | Gravity acceleration (-0.2/tick), rebound bounce on impact, and ride height equilibrium (+5.0). |
| `0x2e65e` | - | `FUN_00423ea0` | `Car_CheckLandingStatus` | `main.c` | Decompiled | EXACT | Checks if airborne car has touched ground (pos_y < ground_y + 5.0), clears airborne flag (+0x270) and asserts landing impact trigger (+0x278). |
| `0x2e6a2` | - | `FUN_00423f00` | `Car_UpdateShadowTracking` | `main.c` | Decompiled | EXACT | Advances secondary position/shadow tracking for car. Integrates velocity at 72 Hz timestep (factor 1.0 / 72.0 = 0.013888889). |
| `0x2e6f6` | - | `FUN_00423f70` | `Car_UpdateBodyVelocity` | `main.c` | Decompiled | EXACT | Computes local lateral and longitudinal acceleration from target waypoint error, rotates by vehicle heading, applies tire drag and clamps to max speed. |
| `0x2ea42` | - | `FUN_004242b0` | `Car_SpawnExplosionEffects` | `main.c` | Decompiled | EXACT | Plays vehicle explosion audio sample (vol capped at 0x10000, sample 2, freq 22000) and emits 6 explosion/debris particle sprites. |
| `0x2ed00` | - | `Car_PhysicsTick` | `Car_PhysicsTick` | `main.c` | Decompiled | EXACT | Master fixed-timestep 72 Hz vehicle dynamics simulation: 4-wheel independent raycast suspension, lateral slip, steering, and traction. |
| `0x31856` | - | `FUN_004269a0` | `Car_ChangeMesh` | `main.c` | Decompiled | EXACT | Swaps current vehicle 3D mesh representation to damaged or alternative model geometry in Lisa3D rasterizer instance table. |
| `0x319d6` | - | `FUN_00426b40` | `Car_ApplyMeshDamage` | `main.c` | Decompiled | EXACT | Evaluates high-velocity collision impact against vehicle chassis, morphs vertex positions inward toward impact point, and emits impact sparks. |
| `0x32d8c` | - | `FUN_00427d70` | `Car_UpdateAxleSpeeds` | `main.c` | Decompiled | EXACT | Averages left and right wheel velocities for front and rear axles with factor 0.5. |
| `0x32dc8` | - | `FUN_00427dc0` | `Collision_TestTrackTriangles` | `main.c` | Decompiled | EXACT | Iterates over track collision triangles from octree query, calculates closest point on triangle, plane distance, and penetration restitution. |
| `0x3383c` | - | `FUN_00428730` | `Collision_RaycastVehicleSphere` | `main.c` | Decompiled | EXACT | Sets up vertical collision query ray from vehicle center (pos_y + 500.0 downward 850 units) and invokes spatial partition octree query. |
| `0x33ee8` | - | `FUN_00428b90` | `Collision_FilterTrackClearance` | `main.c` | Decompiled | EXACT | Filters candidate collision triangles according to track-specific ground clearance thresholds (Snake: 350, Moose: 200, Mountain: 450, Ski: 125, Default: 2500). |
| `0x346cc` | - | `FUN_004292c0` | `Collision_TestLineIntersection` | `main.c` | Decompiled | EXACT | Tests 2D collision impulse transfer between two vehicles. Calculates contact lever arm distances, applies impulse restitution (-1.5), and updates both linear and angular velocities. |
| `0x349e5` | - | `FUN_004295e0` | `Collision_TestPolygonOverlap` | `main.c` | Decompiled | EXACT | Tests pairwise edge crossings between two 2D convex vehicle polygons. Averages contact points to calculate centroid contact position and normal angle. |
| `0x350c9` | - | `FUN_00429a10` | `Math_Signum` | `main.c` | Decompiled | EXACT | Standard 32-bit integer signum returning -1 for negative, 1 for positive, 0 for zero. |
| `0x350f0` | - | `FUN_00429a40` | `Race_CheckCheckpointTriggers` | `main.c` | Decompiled | EXACT | Tests vehicle collision against type 150..154 split-time checkpoint gates and dynamic track scenery obstacles with 3D ballistic trajectory and ground bounce. |
| `0x35f38` | - | `FUN_0042a8d0` | `Physics_ReflectVelocityOffNormal` | `main.c` | Decompiled | EXACT | Rotates 3D velocity into plane-aligned space via yaw and pitch of contact normal, reflects penetrating velocity, and transforms back into world space. |
| `0x360ed` | - | `FUN_0042aa00` | `Audio_UpdateDynamicDoppler` | `main.c` | Decompiled | EXACT | Updates sound pitch and volume for dynamic track ambient sources. |
| `0x3636c` | - | `FUN_0042acb0` | `Track_SpawnEnvironmentalParticles` | `main.c` | Decompiled | EXACT | Emits environmental smoke/dust from track waypoint emitters. |
| `0x36cdd` | - | `FUN_0042b5d0` | `Track_SpawnWeatherParticles` | `main.c` | Decompiled | EXACT | Spawns rain and snow weather particles in viewport frustum. |
| `0x372d8` | - | `FUN_0042bc20` | `Car_UpdateEffects` | `main.c` | Decompiled | EXACT | Coordinates real-time vehicle visual effects (tire skid marks, turbo exhaust flames, engine damage smoke, and surface scraping sparks). |
| `0x3731c` | - | `FUN_0042bc80` | `FX_UpdateSkidMarks` | `main.c` | Decompiled | EXACT | Generates ground skidmarks behind slipping vehicle tires. |
| `0x398f8` | - | `FUN_0042e2e0` | `FX_UpdateTransparentSpriteObject` | `main.c` | Decompiled | EXACT | Updates 3D world position and animation of transparent billboard sprites. |
| `0x39cf2` | - | `FUN_0042e5a0` | `FX_UpdateTransparentSpriteObject2` | `main.c` | Decompiled | EXACT | Updates transparent billboard sprite instance variation. |
| `0x39d1a` | - | `FUN_0042e860` | `FX_UpdateHandlePlotObject` | `main.c` | Decompiled | EXACT | Updates particle plot marker object positions in scene. |
| `0x39040` | - | `FUN_0042ea60` | `FX_UpdateSuperPlotObject` | `main.c` | Decompiled | EXACT | Updates high-intensity spark / super plot particle positions. |
| `0x39350` | - | `FUN_0042ed60` | `Obstacle_SimulateDynamics` | `main.c` | Decompiled | EXACT | Performs dynamic physics integration (ballistic velocity, restitution, and world model transforms) for track scenery obstacles. |
| `0x3a280` | - | `FUN_0042fc80` | `FX_UpdateFlyingParticles` | `main.c` | Decompiled | EXACT | Updates ballistic trajectory and ground bounce for flying vehicle debris. |
| - | - | `-` | `FX_UpdateExplosionNode` | `fx.c` | Decompiled | ADAPTED | Animates fireball expansion and debris dispersion for destroyed vehicles |
| `0x0003c208` | - | `-` | `FX_UpdateVehicleWreck` | `fx.c` | Decompiled | EXACT | Detaches wheels, triggers fire/smoke emitters, and disables physics on wrecked car |
| - | - | `-` | `FX_UpdateDetachedWheel` | `fx.c` | Decompiled | ADAPTED | Simulates ballistic bouncing physics for tires sheared off during collisions |
| `0x0003db4c` | - | `-` | `FX_UpdateVehicleCrashSequence` | `fx.c` | Decompiled | EXACT | Multi-frame rollover and flip crash trajectory integration |
| - | - | `-` | `FX_UpdateCarDebris` | `fx.c` | Decompiled | ADAPTED | Simulates tumbling body panel fragments and glass shards |
| `0x0003d808` | - | `-` | `FX_SpawnWaterSplashes` | `fx.c` | Decompiled | EXACT | Generates animated water splash spray sprites when car enters water surfaces |
| `0x0003ee70` | - | `-` | `FX_SpawnTireDirtDebris` | `fx.c` | Decompiled | EXACT | Emits dirt and gravel kick-up particles behind spinning tires |
| `0x0003f37c` | - | `-` | `FX_SpawnLandingDustPuffs` | `fx.c` | Decompiled | EXACT | Spawns impact dust clouds when car suspension compresses upon landing |
| `0x0003fac0` | - | `-` | `FX_SpawnTireSkidSmoke` | `fx.c` | Decompiled | EXACT | Spawns tire friction smoke particles when vehicle drifts or brakes aggressively |
| - | - | `-` | `FX_UpdateAllParticles` | `fx.c` | Decompiled | EXACT | Wrapper dispatching particle simulation frame tick |
| `0x0003fee8` | - | `-` | `FX_SpawnParticle` | `fx.c` | Decompiled | EXACT | Allocates active scenery particle slot with priority preemption |
| - | - | `-` | `FX_UpdateWeatherBounds` | `fx.c` | Decompiled | ADAPTED | Updates weather volume boundary box around active camera view |
| - | - | `-` | `FX_UpdateWeatherGeometry` | `fx.c` | Decompiled | ADAPTED | Transforms rain streak / snow flake vertices relative to camera motion |
| `0x0003fd04` | - | `-` | `FX_FrameTick` | `fx.c` | Decompiled | EXACT | Master particle simulation tick updating all emitters and active effects |
| - | - | `FUN_004356d0` | `Pos_InitAnimatedObjects` | `fx.c` | Decompiled | ADAPTED | Initializes keyframed trackside animated obstacle nodes from POS asset |
| `0x000413fc` | - | `FUN_004357a0` | `Pos_UpdateAnimatedObjects` | `fx.c` | Decompiled | EXACT | Steps keyframed vertex animation for moving trackside obstacles |
| - | - | `-` | `Math_LookupTrigAngle` | `fx.c` | Decompiled | EXACT | Computes arctangent angle lookup |
| `0x00038e6d` | - | `-` | `Camera_UpdateOverview` | `fx.c` | Decompiled | EXACT | Simulates blimp overview camera following leading vehicles |
| - | - | `FUN_00436990` | `Race_RenderViewport` | `fx.c` | Decompiled | ADAPTED | Renders single/splitscreen 3D scene viewport with HUD elements |
| `0x00049a54` | - | `-` | `Lisa_FlushRasterizerCommands` | `fx.c` | Decompiled | EXACT | Command queue flush dispatcher calling Lisa_ExecuteRasterizerCommands |
| - | - | `-` | `Race_FindFocusedVehicle` | `fx.c` | Decompiled | EXACT | Identifies primary racer or human player to orient camera focus |
| - | - | `FUN_00438210` | `Lisa_RenderPanorama` | `lisa3d.c` | Decompiled | ADAPTED | Panoramic backdrop sky blitter sampling active .PAN texture |
| `0x000534cc` | - | `-` | `HUD_RenderPauseMenu` | `fx.c` | Decompiled | EXACT | Renders in-race pause menu overlay and handles Continue / Restart / Quit navigation |
| `0x000539f8` | - | `-` | `HUD_RenderConfirmationPrompt` | `fx.c` | Decompiled | EXACT | Renders modal confirmation prompt for race restart and exit confirmation |
| `0x00043fe0` | - | `-` | `HUD_RenderTrackResults` | `fx.c` | Decompiled | EXACT | Renders end-of-race leaderboard standings, split times, and championship points |
| `0x00047ae0` | - | `-` | `HUD_RenderPlayAgainPrompt` | `fx.c` | Decompiled | EXACT | Displays play-again / next-track prompt following race completion |
| `0x00047f94` | - | `FUN_0043c910` | `Camera_UpdateChase` | `fx.c` | Decompiled | EXACT | 3rd-person chase camera dynamics, yaw smoothing, and road pitch tracking |
| `0x00048a14` | - | `-` | `Car_UpdateDynamicObjects` | `fx.c` | Decompiled | EXACT | Synchronizes vehicle chassis, wheel meshes, shadows, and name tags with spatial grid |
| `0x0006f2c4` | - | `-` | `Video_SetGraphicsMode` | `fx.c` | Decompiled | EXACT | Configures VGA Mode 13h (320x200 8bpp) display mode |
| `0x00049724` | - | `FUN_0043e2a0` | `Lisa_Init` | `fx.c` | Decompiled | EXACT | Calculates camera viewports and FOV scaling tables for current screen resolution |
| - | - | `-` | `Audio_LoadAssets` | `fx.c` | Decompiled | EXACT | Loads all sound bank pools and sample assets |
| `0x00010060` | - | `-` | `Video_FlipScreen` | `fx.c` | Decompiled | EXACT | Blits virtual double buffer to VGA 0xA0000 video memory |
| - | - | `-` | `Audio_StopSample` | `fx.c` | Decompiled | EXACT | Stops currently active sound voice on specified channel |
| `0x00049a8c` | - | `-` | `Audio_PlaySampleVol` | `fx.c` | Decompiled | EXACT | Plays sound sample with channel, volume, panning and pitch parameters |
| `0x00049d34` | - | `-` | `HUD_RenderTelemetryOverlay` | `fx.c` | Decompiled | EXACT | Displays optional race debug telemetry, speedometer, and engine RPM indicators |
| - | - | `-` | `FX_SpawnWeather` | `fx.c` | Decompiled | EXACT | Spawns rain, snow, or fog particle effects according to track weather settings |
| `0x0004a150` | - | `-` | `HUD_CheckWrongWayHeading` | `fx.c` | Decompiled | EXACT | Compares vehicle velocity vector against track spline tangent to detect wrong-way driving |
| `0x0004a320` | - | `-` | `HUD_RenderPlayerElements` | `fx.c` | Decompiled | EXACT | Renders in-game dashboard HUD: tachometer, mini-map, position, lap timer, and turbo gauge |
| `0x00020c18` | - | `FUN_00446578` | `Surface_TestTrianglePositiveDZ` | `getsurf.c` | Decompiled | EXACT | 2D trapezoidal slope span test for table2 triangles ($dz \ge 0$). |
| `0x00020c81` | - | `FUN_004465e1` | `Surface_TestTriangleNegativeDZ` | `getsurf.c` | Decompiled | EXACT | 2D trapezoidal slope span test for table1 triangles ($dz < 0$). |
| `0x00056410` | - | `FUN_004466d0` | `Lisa_RenderScene` | `lisa3d.c` | Decompiled | EXACT | Master scene rendering loop: frustum culling, vertex transform, depth sorting, and rasterization |
| `0x000564d8` | - | `FUN_004468d0` | `Lisa_InitEngineMemory` | `lisa3d.c` | Decompiled | EXACT | Allocates vertex, triangle, object, and depth bucket memory pools |
| `0x00056974` | - | `FUN_00446c30` | `Lisa_FreeEngineMemory` | `lisa3d.c` | Decompiled | EXACT | Deallocates all dynamic render memory pools |
| `0x000569ec` | - | `FUN_00446ca0` | `Lisa_InitSpatialGrid` | `lisa3d.c` | Decompiled | EXACT | Initializes uniform spatial partitioning grid for object culling |
| `0x00056b28` | - | `FUN_00446d90` | `Lisa_CreateDynamicObject` | `lisa3d.c` | Decompiled | EXACT | Allocates and initializes a dynamic 3D entity instance |
| `0x00056c74` | - | `FUN_00446eb0` | `Lisa_MoveDynamicObject` | `lisa3d.c` | Decompiled | EXACT | Updates dynamic object world position and re-links in spatial grid |
| `0x00056f8c` | - | `FUN_00446f30` | `Lisa_UpdateObjectSpatialGrid` | `lisa3d.c` | Decompiled | EXACT | Recomputes cell occupancy in spatial grid following entity translation |
| `0x00056d24` | - | `FUN_00447070` | `Lisa_SetDynamicObjectMesh` | `lisa3d.c` | Decompiled | EXACT | Assigns mesh geometry and bounding hierarchy to dynamic object |
| `0x00056ffc` | - | `FUN_00447150` | `Lisa_DeleteDynamicObject` | `lisa3d.c` | Decompiled | ADAPTED | Removes dynamic object from spatial grid and returns instance to free pool |
| `0x00057014` | - | `FUN_004471e0` | `Lisa_SetCameraViewport` | `lisa3d.c` | Decompiled | ADAPTED | Sets camera viewport extents and projection aspect ratio |
| `0x000570d8` | - | `FUN_00447280` | `Lisa_GenerateMipmaps` | `lisa3d.c` | Decompiled | EXACT | Downsamples texture source through box filter pyramid |
| `0x00057548` | - | `FUN_004475c0` | `Lisa_GenerateTextureSpanTable` | `lisa3d.c` | Decompiled | EXACT | Generates pre-scaled scanline UV stepping tables for perspective texture mapper |
| `0x00058050` | - | `FUN_00447fb0` | `Lisa_DownsampleTextureMipmap` | `lisa3d.c` | Decompiled | EXACT | 2x2 box filter mipmap reduction kernel |
| `0x0005829c` | - | `FUN_004481f0` | `Lisa_FilterTextureBlock` | `lisa3d.c` | Decompiled | EXACT | Bilinear / average filter block kernel for texture page generation |
| `0x000586d0` | - | `FUN_00448620` | `Lisa_LoadOrCreateShadingTable` | `lisa3d.c` | Decompiled | EXACT | Builds or validates 32-level light ramp shading lookup table from 256-color palette |
| `0x000596c4` | - | `FUN_00448860` | `Lisa_FindClosestPaletteColor` | `lisa3d.c` | Decompiled | EXACT | Euclidean RGB distance nearest-match palette color search |
| `0x0005891c` | - | `FUN_00448990` | `Lisa_RenderSkyBackdrop` | `lisa3d.c` | Decompiled | EXACT | Draws gradient horizon or textured panoramic sky background |
| `0x00058c1c` | - | `FUN_00448c30` | `Lisa_CullObjectsOrthographic` | `lisa3d.c` | Decompiled | EXACT | Orthographic AABB view-frustum culling pass for mini-map/mirrors |
| `0x00058ebc` | - | `FUN_00448e70` | `Lisa_FrustumCullObjects` | `lisa3d.c` | Decompiled | EXACT | Perspective 6-plane frustum culling with sphere/AABB hierarchical rejection |
| - | - | `FUN_00449470` | `Lisa_CullObjects` | `lisa3d.c` | Decompiled | EXACT | Hierarchical frustum culling dispatcher |
| `0x0005a2c8` | - | `FUN_00449e70` | `Lisa_TransformVertices` | `lisa3d.c` | Decompiled | EXACT | Transforms local mesh coordinates to camera space with homogeneous clipping flags |
| `0x0005a8c8` | - | `FUN_0044a3d0` | `Lisa_TransformVerticesPanorama` | `lisa3d.c` | Decompiled | EXACT | Specialized vertex transform for panoramic background sky domes |
| `0x0005ae60` | - | `FUN_0044a900` | `Lisa_ComputeObjectMatrix` | `lisa3d.c` | Decompiled | EXACT | Composes pitch-yaw-roll orientation into 3x3 transformation matrix |
| `0x0005b308` | - | `FUN_0044ae20` | `Lisa_TransformSubmeshVerticesPanorama` | `lisa3d.c` | Decompiled | EXACT | Transforms hierarchical submesh vertices with compound parent matrices |
| `0x0005b7a8` | - | `FUN_0044b340` | `Lisa_ComputeCameraRotationMatrix` | `lisa3d.c` | Decompiled | EXACT | Computes inverted camera view orientation matrix from Euler angles |
| `0x0005b8e8` | - | `FUN_0044b480` | `Lisa_InitOpcodeTable` | `lisa3d.c` | Decompiled | EXACT | Populates rasterizer polygon drawer jump table based on current shading/filtering modes |
| `0x0005b9c4` | - | `FUN_0044b570` | `Lisa_SortDepthBuckets` | `lisa3d.c` | Decompiled | EXACT | Radix sort depth bucketer ordering polygons back-to-front |
| `0x0005bc10` | - | `FUN_0044b770` | `Lisa_DrawTriangle_Op0F` | `lisa3d.c` | Decompiled | EXACT | Flat-shaded untextured triangle rasterizer (Opcode 0x0F) |
| `0x0005bea4` | - | `FUN_0044b980` | `Lisa_DrawTriangle_Op10` | `lisa3d.c` | Decompiled | EXACT | Gouraud-shaded untextured triangle rasterizer (Opcode 0x10) |
| `0x0005cb48` | - | `FUN_0044c1f0` | `Lisa_RenderSubmeshes` | `lisa3d.c` | Decompiled | EXACT | Iterates visible entity submeshes and dispatches polygons into depth buckets |
| `0x0005d6d0` | - | `LAB_0044caa0` | `Lisa_DrawPolygon_Op12` | `lisa3d.c` | Decompiled | EXACT | Polygon drawer Opcode 0x12 dispatch (textured polygon variant) |
| `0x0005d6e0` | - | `LAB_0044cac0` | `Lisa_DrawPolygon_Op13` | `lisa3d.c` | Decompiled | EXACT | Polygon drawer Opcode 0x13 dispatch (shaded textured polygon variant) |
| `0x0005d6f0` | - | `LAB_0044cae0` | `Lisa_DrawPolygon_Op16` | `lisa3d.c` | Decompiled | EXACT | Polygon drawer Opcode 0x16 dispatch (transparent/translucent polygon variant) |
| `0x0005d704` | - | `LAB_0044cb00` | `Lisa_DrawPolygon_Op17` | `lisa3d.c` | Decompiled | EXACT | Polygon drawer Opcode 0x17 dispatch (shaded transparent polygon variant) |
| `0x0005d718` | - | `FUN_0044cb20` | `Lisa_DrawTriangle_OpcodeHelper` | `lisa3d.c` | Decompiled | EXACT | Unified rasterizer command bucketer dispatching polygon commands into scanline pipeline |
| `0x0005dc08` | - | `FUN_0044cf00` | `Lisa_DrawTriangle_Op14` | `lisa3d.c` | Decompiled | EXACT | Textured triangle rasterizer with transparency chroma keying (Opcode 0x14) |
| `0x0005dea0` | - | `FUN_0044d0f0` | `Lisa_DrawBillboard_Op07` | `lisa3d.c` | Decompiled | EXACT | Camera-facing billboard sprite rasterizer (Opcode 0x07) |
| `0x0005e004` | - | `FUN_0044d230` | `Lisa_DrawBillboard_Op08` | `lisa3d.c` | Decompiled | EXACT | Scaled camera-facing billboard sprite rasterizer (Opcode 0x08) |
| `0x0005e358` | - | `FUN_0044d550` | `Lisa_DrawTexturedTriangle_Op15` | `lisa3d.c` | Decompiled | EXACT | Perspective-correct filtered textured triangle rasterizer (Opcode 0x15) |
| `0x0005c144` | - | `Lisa_DrawTexturedTriangle_Op11_Unshaded` | `Lisa_DrawTexturedTriangle_Op11_Unshaded` | `lisa3d.c` | Decompiled | EXACT | Textured triangle rasterizer without lighting ramp (Opcode 0x11, Mode 1) |
| `0x0005cc58` | - | `Lisa_DrawTexturedTriangle_Op11_Shaded` | `Lisa_DrawTexturedTriangle_Op11_Shaded` | `lisa3d.c` | Decompiled | EXACT | Textured triangle rasterizer with 32-level light shading (Opcode 0x11, Mode 0) |
| `0x00060210` | - | `FUN_0044e900` | `Lisa_DrawTexturedTriangle_Op15_Sub` | `lisa3d.c` | Decompiled | EXACT | Sub-pixel trapezoid scanline rasterizer kernel for Opcode 0x15 |
| `0x00060dc4` | - | `FUN_0044f070` | `Lisa_InitRasterizerTables` | `lisa3d.c` | Decompiled | EXACT | Computes scanline edge stepping tables and reciprocal lookup tables |
| `0x00060e3d` | - | `FUN_0044f0e9` | `Lisa_ExecuteRasterizerCommands` | `lisa3d.c` | Decompiled | EXACT | Flushes queued draw commands from depth buckets through active opcode drawer functions |
| `0x0005d084` | - | `FUN_00452800` | `Lisa_RenderTexturedTriangle_Op11` | `lisa3d.c` | Decompiled | EXACT | Textured triangle dispatch helper (Opcode 0x11, Mode 0 Shaded NoFilter) |
| `0x0005ef14` | - | `FUN_004537dc` | `Lisa_DrawTexturedSpan_Op11` | `lisa3d.c` | Decompiled | EXACT | Textured span rasterizer kernel with bilinear filtering (Opcode 0x11, Mode 0 Filtered) |
| `0x00061220` | - | `-` | `Font_InitSystem` | `geputget.c` | Decompiled | EXACT | Initializes font subsystem tables (30 slots) |
| `0x00061319` | - | `-` | `Font_Shutdown` | `geputget.c` | Decompiled | EXACT | Unloads active fonts and shuts down font subsystem |
| `0x00061399` | - | `-` | `Font_Parse` | `geputget.c` | Decompiled | EXACT | Parses LFT font header, initializes glyph handles and metrics |
| `0x000615eb` | - | `FUN_00456270` | `Font_Load` | `geputget.c` | Decompiled | ADAPTED | Loads and parses .LFT font header, offset tables, widths, and glyph raster data. |
| `0x00061653` | - | `-` | `Font_Unload` | `geputget.c` | Decompiled | EXACT | Frees sprite handles for font glyphs and marks slot free |
| `0x000616db` | - | `-` | `Font_GetTextWidth` | `geputget.c` | Decompiled | EXACT | Calculates string rendering width in pixels |
| `0x00061959` | - | `FUN_00456660` | `Font_DrawText` | `geputget.c` | Decompiled | EXACT | 2D bitmap font rasterizer blitting characters to 8bpp buffer. |
| - | - | `FUN_00456c40` | `Video_SetPalette` | `lisa3d.c` | Decompiled | EXACT | VGA Mode 13h DAC palette update wrapper |
| `0x00060f9c` | - | `FUN_004574a0` | `File_LoadToMemory` | `mem.c` | Decompiled | ADAPTED | Generic binary loader (fopen, fread into allocated buffer). |
| `0x6280c` | - | `FUN_004582b0` | `Sound_LoadPAT` | `main.c` | Decompiled | EXACT | Loads Gravis UltraSound GF1 .PAT patch audio files and converts 8-bit/16-bit linear PCM to S16SYS format. |
| `0x62cf4` | - | `FUN_00458e00` | `Sound_LoadWAV` | `main.c` | Decompiled | EXACT | Loads RIFF/WAVE PCM 8-bit/16-bit audio file and converts to S16SYS format. |
| `0x0005571c` | - | `FUN_0045b4f0` | `Lisa_PrintVersion` | `lisa3d.c` | Decompiled | EXACT | Outputs Lisa 3D graphics system version and copyright banner |
| `0x56034` | - | `entry` | `CRT_Entry` | `MSVC CRT` | Analyzed | - | C Runtime startup entry point, parses command line, calls WinMain. |
| `0x00054e00` | - | `FUN_00499abc` | `Cdp_DecompressRLE` | `lisa3d.c` | Decompiled | ADAPTED | Delta RLE decompressor for CDP video frames |
| `0x10010` | - | - | `main` | `main.c` | Decompiled | EXACT |  |
| `0x6f549` | - | - | `__cstart` | `MSVC CRT` | Decompiled | EXACT |  |
| `0x10198` | - | - | `Timer_Init` | `main.c` | Decompiled | EXACT |  |
| `0x1034c` | - | - | `Timer_GetPITCounter` | `main.c` | Decompiled | EXACT |  |
| `0x10238` | - | - | `Timer_GetTime` | `main.c` | Decompiled | EXACT |  |
| `0x63310` | - | - | `Sound_LoadAsset` | `main.c` | Decompiled | EXACT |  |
| `0x63d56` | - | - | `Audio_Init` | `main.c` | Decompiled | EXACT |  |
| `0x20d14` | - | - | `FatalError` | `main.c` | Decompiled | EXACT |  |
| `0x0001aaf8` | - | - | `Menu_InitCarViewport` | `menu.c` | Decompiled | EXACT | Loads menucar.plc, menucar.msh, menucar.tex for 3D car viewport |
| `0x00015344` | - | - | `Menu_RenderCarViewport` | `menu.c` | Decompiled | EXACT | Renders 3D rotating car model onto pedestal in car select screen |
| `0x000559e4` | - | - | `Input_InitKeyboard` | `geputget.c` | Decompiled | EXACT | Initializes keyboard driver, clears key state arrays, sets repeat rate/delay. |
| `0x00055a94` | - | - | `Input_ShutdownKeyboard` | `geputget.c` | Decompiled | EXACT | Shuts down keyboard driver and resets driver active flag. |
| `0x00055ae4` | - | - | `Input_PollKeyboard` | `geputget.c` | Decompiled | EXACT | Polls hardware state, updates down/edge flags, advances repeat timers, enqueues ASCII. |
| `0x00055c28` | - | - | `Input_IsKeyDown` | `geputget.c` | Decompiled | EXACT | Returns 1 if scancode is currently held down, 0 otherwise. |
| `0x00055c3c` | - | - | `Input_WasKeyPressed` | `geputget.c` | Decompiled | EXACT | Returns 1 on edge-triggered keypress and clears flag. |
| `0x00055c60` | - | - | `Input_WasKeyRepeated` | `geputget.c` | Decompiled | EXACT | Returns 1 on auto-repeat pulse and clears flag. |
| `0x00055c84` | - | - | `Input_SetKeyCallback` | `geputget.c` | Decompiled | EXACT | Registers user callback invoked on key transition. |
| `0x00055ca0` | - | - | `Input_EnqueueAscii` | `geputget.c` | Decompiled | EXACT | Translates scancode to ASCII and pushes into circular buffer. |
| `0x00055cd0` | - | - | `Input_GetQueuedKey` | `geputget.c` | Decompiled | EXACT | Searches ASCII circular buffer for substring match (used for cheat codes). |
| `0x00055d04` | - | - | `Input_InitKeyTables` | `geputget.c` | Decompiled | EXACT | Initializes ISR buffers, toggle masks, and release flags. |
| `0x00051a9c` | - | `FUN_00051a9c` | `Palette_Fade` | `menu.c` | Decompiled | EXACT | Blends two 768-byte palettes using 8.8 fixed-point factor |
| `0x00051818` | - | `FUN_00051818` | `Palette_BlitTrans` | `menu.c` | Decompiled | EXACT | Blits rectangular region with 64KB transparency lookup table blending |
| `0x0001a864` | - | `FUN_0001a864` | `Menu_InitSettings` | `menu.c` | Decompiled | EXACT | Initializes default game settings, keybindings, and player names |
| `0x0001680d` | - | `FUN_0001680d` | `Menu_LoadIntroCDPs` | `menu.c` | Decompiled | EXACT | Loads 6 intro CDP animation files and starts sequence playback |
| `0x00013a5c` | - | `FUN_00013a5c` | `Menu_Shutdown` | `menu.c` | Decompiled | EXACT | Frees menu buffers, palettes, CDP streams, and Lisa memory |
