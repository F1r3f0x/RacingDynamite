BEGIN TRANSACTION;
CREATE TABLE deviations (
    id TEXT PRIMARY KEY,                   -- e.g. 'DEV-001'
    category TEXT NOT NULL,                -- e.g. 'FIX_CAT_NOCLIP'
    title TEXT NOT NULL,
    description TEXT,
    dos_address TEXT,
    win_address TEXT,
    toggle_key TEXT                        -- Corresponding flag in GameFixOptions
);
INSERT INTO "deviations" VALUES('DEV-001','FIX_CAT_NOCLIP','Surface raycast out-of-bounds cell crash & fall-through','Surface raycast out-of-bounds cell crash & fall-through',NULL,'0x00412fc0','fixes->fix_noclip');
INSERT INTO "deviations" VALUES('DEV-002','FIX_CAT_ELEVATION','Vehicle tire ground alignment offset (prevents floating/sinking)','Vehicle tire ground alignment offset (prevents floating/sinking)',NULL,'0x0041d190','fixes->fix_elevation');
INSERT INTO "deviations" VALUES('DEV-003','FIX_CAT_NOCLIP','Mountain wall climbing & steep gradient adhesion clamp','Mountain wall climbing & steep gradient adhesion clamp',NULL,'0x00424570','fixes->fix_noclip');
INSERT INTO "deviations" VALUES('DEV-004','FIX_CAT_CAMERA','Right-handed camera basis vector normalization','Right-handed camera basis vector normalization',NULL,'0x00436990','fixes->fix_camera');
INSERT INTO "deviations" VALUES('DEV-005','FIX_CAT_RENDERER','1/Z depth buffering precision vs 6,000 depth buckets','1/Z depth buffering precision vs 6,000 depth buckets',NULL,'0x004466d0','renderer->options.authentic_depth_buckets');
INSERT INTO "deviations" VALUES('DEV-006','FIX_CAT_AUDIO','High-RPM engine pitch modulation overflow protection','High-RPM engine pitch modulation overflow protection',NULL,'0x0041f9b0','fixes->fix_audio');
CREATE TABLE functions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    dos_address TEXT UNIQUE,               -- Address in MAINDOS.EXE (e.g. '0x00012340')
    win_address TEXT UNIQUE,               -- Cross-ref address in IGN_WIN.EXE (e.g. '0x00412fc0')
    symbol_name TEXT NOT NULL,             -- Reconstructed / authentic C function name
    original_ghidra_name TEXT,             -- Ghidra default label (e.g. 'FUN_00412fc0')
    module_id INTEGER REFERENCES modules(id) ON DELETE SET NULL,
    status TEXT NOT NULL DEFAULT 'unidentified' 
        CHECK (status IN ('unidentified', 'analyzed', 'decompiled', 'matching')),
    calling_convention TEXT DEFAULT 'watcom_reg'
        CHECK (calling_convention IN ('watcom_reg', 'cdecl', 'stdcall', 'fastcall')),
    return_type TEXT DEFAULT 'void',
    parameters TEXT,                       -- Formatted parameter list e.g. '(int x, int z)'
    byte_size INTEGER,                     -- Byte size in binary
    line_count INTEGER,                    -- Decompiled C line count
    fidelity TEXT DEFAULT 'EXACT'
        CHECK (fidelity IN ('EXACT', 'ADAPTED', 'EXTENDED', 'INFRASTRUCTURE', '-')),
    port_location TEXT,                    -- Location in src/ if ported
    purpose TEXT,                          -- High-level description of functionality
    notes TEXT,                            -- Technical notes, registers, formulas
    assembly_hash TEXT                     -- Hash of original disassembly for matching
);
INSERT INTO "functions" VALUES(259,'0x00060f40',NULL,'File_ReadToBuffer','-',4,'matching','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Reads binary file directly into preallocated buffer',NULL,NULL);
INSERT INTO "functions" VALUES(260,'0x00061100',NULL,'File_GetSize','-',4,'matching','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Seeks to end and returns binary file size',NULL,NULL);
INSERT INTO "functions" VALUES(261,'0x0006117c',NULL,'File_Exists','-',4,'matching','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Tests if file exists by attempting fopen',NULL,NULL);
INSERT INTO "functions" VALUES(262,'0x00011504','0x004029a0','Menu_Init','FUN_004029a0',10,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Loads MENU.COL, MENU.TAB, .LFT fonts, and initializes menu options.',NULL,NULL);
INSERT INTO "functions" VALUES(263,'0x00012bb8','0x00402c00','Menu_Tick','FUN_00402c00',10,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Handles menu input navigation (arrows, Enter, Esc), item highlight, and transitions.',NULL,NULL);
INSERT INTO "functions" VALUES(264,NULL,'0x0040e6b0','Car_IntegratePosition','FUN_0040e6b0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','World coordinate velocity integrator with 21.76 scale factor and 72 Hz timestep.',NULL,NULL);
INSERT INTO "functions" VALUES(265,NULL,'0x004120a0','WinMain','FUN_004120a0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Main entry point; registers window class, queries timer, runs message/tick loop.',NULL,NULL);
INSERT INTO "functions" VALUES(266,'0x10060','0x00412230','App_FrameTick','FUN_00412230',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Main engine tick; dispatches Init (0), Main Loop (1), and Shutdown (2).',NULL,NULL);
INSERT INTO "functions" VALUES(267,NULL,'0x00412500','App_Init','FUN_00412500',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Creates game window, initializes DirectDraw and DirectInput subsystems.',NULL,NULL);
INSERT INTO "functions" VALUES(268,'0x215c8','0x00412530','App_Shutdown','FUN_00412530',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Releases DirectDraw surfaces, DirectSound, and window handles.',NULL,NULL);
INSERT INTO "functions" VALUES(269,NULL,'0x00412580','Cdp_OpenFile','FUN_00412580',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Validates "CDP\0" header, dimensions, frame count, and embedded palette.',NULL,NULL);
INSERT INTO "functions" VALUES(270,NULL,'0x00412610','Cdp_DecodeFrame','FUN_00412610',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Advances animation stream and triggers inter-frame delta decompression.',NULL,NULL);
INSERT INTO "functions" VALUES(271,'0x0001fee0','0x00412670','Surface_LoadSRF','FUN_00412670',1,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Loads .SRF track collision surface, converts relative offsets to pointers',NULL,NULL);
INSERT INTO "functions" VALUES(272,'0x0002002c','0x004127a0','Surface_FreeSRF','FUN_004127a0',1,'decompiled','watcom_reg','void',NULL,30,NULL,'ADAPTED','-','Frees active .SRF surface memory buffer.',NULL,NULL);
INSERT INTO "functions" VALUES(273,'0x00020814','0x00412fc0','Surface_Raycast','FUN_00412fc0',1,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXTENDED','-','Spatial grid query, candidate selection, cross product normal, world vertex transform',NULL,NULL);
INSERT INTO "functions" VALUES(274,'0x00020bbc','0x00413380','Surface_GetTriangleHeight','FUN_00413380',1,'decompiled','watcom_reg','void',NULL,91,NULL,'ADAPTED','-','Computes average elevation (y0 + y1 + y2) / -3 using vertex buffer indices from triangle.',NULL,NULL);
INSERT INTO "functions" VALUES(275,'0x00043d60','0x004133d0','Font_DrawHUDText','FUN_004133d0',3,'decompiled','watcom_reg','void',NULL,231,NULL,'EXACT','-','2D bitmap font rasterizer blitting characters from IGNITION.FNT directly to 8bpp framebuffer.',NULL,NULL);
INSERT INTO "functions" VALUES(276,'0x1aea0','0x004134e0','AI_FollowTrackSplines','FUN_004134e0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Steering simulation updating AI vehicle heading, track spline waypoint progression, speed moderation, and obstacle evasion.',NULL,NULL);
INSERT INTO "functions" VALUES(277,'0x1ce1c','0x00414e40','Track_LoadSplines','FUN_00414e40',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Loads .TRI chunk indices, constructs left/right road boundary splines and AI waypoints.',NULL,NULL);
INSERT INTO "functions" VALUES(278,NULL,'0x00416250','Mesh_InstantiatePlacedObjects','FUN_00416250',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Applies .PLC world translation offsets to .MSH submesh vertices.',NULL,NULL);
INSERT INTO "functions" VALUES(279,'0x20d18','0x00417270','Game_Init','FUN_00417270',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Sets initial game state flags, resets timers, initiates intro sequence.',NULL,NULL);
INSERT INTO "functions" VALUES(280,'0x20d60','0x004172b0','Game_StateDispatcher','FUN_004172b0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Top-level game loop state machine dispatcher (Intro -> Menus -> Race).',NULL,NULL);
INSERT INTO "functions" VALUES(281,'0x00021860','0x00418130','Load_SystemGraphicsAndFonts','FUN_00418130',3,'decompiled','watcom_reg','void',NULL,371,NULL,'EXACT','-','Loads SYS.COL, N_SYSGFX.PIC, N_SYSG_2.PIC, and .LFT fonts.',NULL,NULL);
INSERT INTO "functions" VALUES(282,NULL,'0x00418dd0','Track_LoadAllAssets','FUN_00418dd0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Master track loader: loads .COL, .PAN, .PIC, .SHD, .TAB, .MSH, .TEX, .POS.',NULL,NULL);
INSERT INTO "functions" VALUES(283,'0x230f4','0x00419a90','Track_LoadPlacements','FUN_00419a90',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Loads .PLC scenery object placement tables for level and cars.',NULL,NULL);
INSERT INTO "functions" VALUES(284,'0x2322c','0x00419bd0','Mesh_LoadTrackAndCars','FUN_00419bd0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Loads LEVELS/<TRACK>/<TRACK>.MSH and CARS/CARS.MSH into geometry memory.',NULL,NULL);
INSERT INTO "functions" VALUES(285,'0x23368','0x00419d10','Texture_LoadAllPages','FUN_00419d10',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Loads 1MB track .TEX, car .TEX, and 64KB aligned sprite pages.',NULL,NULL);
INSERT INTO "functions" VALUES(286,'0x000240f4','0x0041ac40','Font_LoadHUDFonts','FUN_0041ac40',3,'decompiled','watcom_reg','void',NULL,746,NULL,'EXACT','-','Loads HUD lettering glyphs (IGNITION.FNT, yellow.lft, speed.lft, etc.).',NULL,NULL);
INSERT INTO "functions" VALUES(287,'0x000243e0','0x0041af70','Track_LoadOverlayGfx','FUN_0041af70',3,'decompiled','watcom_reg','void',NULL,973,NULL,'EXACT','-','Loads track sign textures and winner trophy bitmap (POKAL.PIC).',NULL,NULL);
INSERT INTO "functions" VALUES(288,'0x247b0','0x0041b360','Track_PreprocessPlacements','FUN_0041b360',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Unpacks model_type bitfields (& 0xFFF) and extracts animation and flag channels.',NULL,NULL);
INSERT INTO "functions" VALUES(289,'0x2489c','0x0041b470','Race_InitSceneAndCars','FUN_0041b470',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Instantiates player and AI cars on starting grid, loads track surface, and binds collision and physics states.',NULL,NULL);
INSERT INTO "functions" VALUES(290,NULL,'0x0041d190','Car_UnpackMeshGeometry','FUN_0041d190',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXTENDED','-','Extracts CARS.MSH submesh vertices, finds bottom tire vertex $\max(v_y)$ for ground alignment, and scales by $21.76$.',NULL,NULL);
INSERT INTO "functions" VALUES(291,'0x2a4f1','0x0041f9b0','Sound_InitAndLoadPools','Sound_InitAndLoadPools',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Initializes 32 DirectSound-compatible audio channels; loads SFX pools (ROLL, SKID, COLL, BOOST, DIV, KLICK, OK); loads track sounds; reads per-vehicle ENGINE.INF 800-byte curves into uint8_t[200] vol/pitch arrays at +0x658, +0x720, +0x7e8, +0x8b0.',NULL,NULL);
INSERT INTO "functions" VALUES(292,'0x2a40e','0x00420090','Obstacle_TriggerAction','FUN_00420090',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Evaluates type 0xFA (250) trigger obstacles and triggers associated actions.',NULL,NULL);
INSERT INTO "functions" VALUES(293,'0x2a4ec','0x004200e0','AI_InitSteeringConeLookup','FUN_004200e0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Precomputes 800x405 lookahead steering cone and obstacle threat table.',NULL,NULL);
INSERT INTO "functions" VALUES(294,'0x2a97a','0x00420240','Ghost_LoadCarAndPath','FUN_00420240',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Loads recorded Time Attack ghost car trajectory from GHOSTS\%s.GST.',NULL,NULL);
INSERT INTO "functions" VALUES(295,'0x2adf8','0x00420870','Track_LoadBinaryCache','FUN_00420870',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Loads preprocessed level data cache (ign_win.btz / ign_dos.btz).',NULL,NULL);
INSERT INTO "functions" VALUES(296,'0x2b42d','0x00420990','Sound_FreeAllSounds','FUN_00420990',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Frees active level audio buffers and sample tables upon track unload.',NULL,NULL);
INSERT INTO "functions" VALUES(297,'0x2b51a','0x00420b70','Game_Shutdown','FUN_00420b70',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Releases all allocated game memory and shuts down engine subsystems cleanly.',NULL,NULL);
INSERT INTO "functions" VALUES(298,'0x3b996','0x00420c00','Timer_GetDeltaTime','FUN_00420c00',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Computes elapsed frame delta time using tick counter scaled by 0.036. Updates accumulator, frame counter, and clamps delta to max 10.8 ticks.',NULL,NULL);
INSERT INTO "functions" VALUES(299,'0x2b9d3','0x00420d10','Race_UpdateCountdownAndFinish','FUN_00420d10',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Updates start-line traffic light timer and checks race winner victory condition.',NULL,NULL);
INSERT INTO "functions" VALUES(300,'0x2b9e4','0x00420e60','HUD_UpdateRaceTimes','FUN_00420e60',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Updates player split times and current lap timer display on in-game HUD.',NULL,NULL);
INSERT INTO "functions" VALUES(301,'0x2c5b8','0x00420eb0','Input_ProcessRaceHotkeys','FUN_00420eb0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Polls keyboard hotkeys during race: Pause (P), Escape menu, camera toggle, and volume.',NULL,NULL);
INSERT INTO "functions" VALUES(302,'0x2ccc5','0x00421bc0','Input_PollPlayerVehicleControls','FUN_00421bc0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Reads keyboard / joystick axes and maps to vehicle steering, throttle, brake, and turbo.',NULL,NULL);
INSERT INTO "functions" VALUES(303,'0x2cf5d','0x004222b0','Ghost_SaveCarAndPath','FUN_004222b0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Serializes recorded lap waypoint trajectory into GHOSTS\%s.GST.',NULL,NULL);
INSERT INTO "functions" VALUES(304,'0x2cfbc','0x004225d0','Track_SaveBinaryCache','FUN_004225d0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Saves preprocessed level collision cache file.',NULL,NULL);
INSERT INTO "functions" VALUES(305,'0x2d008','0x00422680','Race_ResolveVehicleCollisions','FUN_00422680',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Inter-vehicle and scenery obstacle collision detection and impulse response. Fixed 72 Hz physics integration coordinator.',NULL,NULL);
INSERT INTO "functions" VALUES(306,'0x2de2e','0x00423620','Car_HandleElimination','FUN_00423620',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Checks trailing vehicle elimination condition in knock-out races. Marks vehicle blown (+0x354 = 1), sets race status, and logs elimination string.',NULL,NULL);
INSERT INTO "functions" VALUES(307,'0x2e234','0x00423aa0','Car_VerticalDynamics','FUN_00423aa0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Gravity acceleration (-0.2/tick), rebound bounce on impact, and ride height equilibrium (+5.0).',NULL,NULL);
INSERT INTO "functions" VALUES(308,'0x2e65e','0x00423ea0','Car_CheckLandingStatus','FUN_00423ea0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Checks if airborne car has touched ground (pos_y < ground_y + 5.0), clears airborne flag (+0x270) and asserts landing impact trigger (+0x278).',NULL,NULL);
INSERT INTO "functions" VALUES(309,'0x2e6a2','0x00423f00','Car_UpdateShadowTracking','FUN_00423f00',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Advances secondary position/shadow tracking for car. Integrates velocity at 72 Hz timestep (factor 1.0 / 72.0 = 0.013888889).',NULL,NULL);
INSERT INTO "functions" VALUES(310,'0x2e6f6','0x00423f70','Car_UpdateBodyVelocity','FUN_00423f70',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Computes local lateral and longitudinal acceleration from target waypoint error, rotates by vehicle heading, applies tire drag and clamps to max speed.',NULL,NULL);
INSERT INTO "functions" VALUES(311,'0x2ea42','0x004242b0','Car_SpawnExplosionEffects','FUN_004242b0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Plays vehicle explosion audio sample (vol capped at 0x10000, sample 2, freq 22000) and emits 6 explosion/debris particle sprites.',NULL,NULL);
INSERT INTO "functions" VALUES(312,'0x2ed00','0x00424570','Car_PhysicsTick','Car_PhysicsTick',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Master fixed-timestep 72 Hz vehicle dynamics simulation: 4-wheel independent raycast suspension, lateral slip, steering, and traction.',NULL,NULL);
INSERT INTO "functions" VALUES(313,'0x31856','0x004269a0','Car_ChangeMesh','FUN_004269a0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Swaps current vehicle 3D mesh representation to damaged or alternative model geometry in Lisa3D rasterizer instance table.',NULL,NULL);
INSERT INTO "functions" VALUES(314,'0x319d6','0x00426b40','Car_ApplyMeshDamage','FUN_00426b40',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Evaluates high-velocity collision impact against vehicle chassis, morphs vertex positions inward toward impact point, and emits impact sparks.',NULL,NULL);
INSERT INTO "functions" VALUES(315,'0x32d8c','0x00427d70','Car_UpdateAxleSpeeds','FUN_00427d70',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Averages left and right wheel velocities for front and rear axles with factor 0.5.',NULL,NULL);
INSERT INTO "functions" VALUES(316,'0x32dc8','0x00427dc0','Collision_TestTrackTriangles','FUN_00427dc0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Iterates over track collision triangles from octree query, calculates closest point on triangle, plane distance, and penetration restitution.',NULL,NULL);
INSERT INTO "functions" VALUES(317,'0x3383c','0x00428730','Collision_RaycastVehicleSphere','FUN_00428730',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Sets up vertical collision query ray from vehicle center (pos_y + 500.0 downward 850 units) and invokes spatial partition octree query.',NULL,NULL);
INSERT INTO "functions" VALUES(318,'0x33ee8','0x00428b90','Collision_FilterTrackClearance','FUN_00428b90',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Filters candidate collision triangles according to track-specific ground clearance thresholds (Snake: 350, Moose: 200, Mountain: 450, Ski: 125, Default: 2500).',NULL,NULL);
INSERT INTO "functions" VALUES(319,'0x346cc','0x004292c0','Collision_TestLineIntersection','FUN_004292c0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Tests 2D collision impulse transfer between two vehicles. Calculates contact lever arm distances, applies impulse restitution (-1.5), and updates both linear and angular velocities.',NULL,NULL);
INSERT INTO "functions" VALUES(320,'0x349e5','0x004295e0','Collision_TestPolygonOverlap','FUN_004295e0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Tests pairwise edge crossings between two 2D convex vehicle polygons. Averages contact points to calculate centroid contact position and normal angle.',NULL,NULL);
INSERT INTO "functions" VALUES(321,'0x350c9','0x00429a10','Math_Signum','FUN_00429a10',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Standard 32-bit integer signum returning -1 for negative, 1 for positive, 0 for zero.',NULL,NULL);
INSERT INTO "functions" VALUES(322,'0x350f0','0x00429a40','Race_CheckCheckpointTriggers','FUN_00429a40',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Tests vehicle collision against type 150..154 split-time checkpoint gates and dynamic track scenery obstacles with 3D ballistic trajectory and ground bounce.',NULL,NULL);
INSERT INTO "functions" VALUES(323,'0x35f38','0x0042a8d0','Physics_ReflectVelocityOffNormal','FUN_0042a8d0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Rotates 3D velocity into plane-aligned space via yaw and pitch of contact normal, reflects penetrating velocity, and transforms back into world space.',NULL,NULL);
INSERT INTO "functions" VALUES(324,'0x360ed','0x0042aa00','Audio_UpdateDynamicDoppler','FUN_0042aa00',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Updates sound pitch and volume for dynamic track ambient sources.',NULL,NULL);
INSERT INTO "functions" VALUES(325,'0x3636c','0x0042acb0','Track_SpawnEnvironmentalParticles','FUN_0042acb0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Emits environmental smoke/dust from track waypoint emitters.',NULL,NULL);
INSERT INTO "functions" VALUES(326,'0x36cdd','0x0042b5d0','Track_SpawnWeatherParticles','FUN_0042b5d0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Spawns rain and snow weather particles in viewport frustum.',NULL,NULL);
INSERT INTO "functions" VALUES(327,'0x372d8','0x0042bc20','Car_UpdateEffects','FUN_0042bc20',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Coordinates real-time vehicle visual effects (tire skid marks, turbo exhaust flames, engine damage smoke, and surface scraping sparks).',NULL,NULL);
INSERT INTO "functions" VALUES(328,'0x3731c','0x0042bc80','FX_UpdateSkidMarks','FUN_0042bc80',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Generates ground skidmarks behind slipping vehicle tires.',NULL,NULL);
INSERT INTO "functions" VALUES(329,'0x398f8','0x0042e2e0','FX_UpdateTransparentSpriteObject','FUN_0042e2e0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Updates 3D world position and animation of transparent billboard sprites.',NULL,NULL);
INSERT INTO "functions" VALUES(330,'0x39cf2','0x0042e5a0','FX_UpdateTransparentSpriteObject2','FUN_0042e5a0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Updates transparent billboard sprite instance variation.',NULL,NULL);
INSERT INTO "functions" VALUES(331,'0x39d1a','0x0042e860','FX_UpdateHandlePlotObject','FUN_0042e860',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Updates particle plot marker object positions in scene.',NULL,NULL);
INSERT INTO "functions" VALUES(332,'0x39040','0x0042ea60','FX_UpdateSuperPlotObject','FUN_0042ea60',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Updates high-intensity spark / super plot particle positions.',NULL,NULL);
INSERT INTO "functions" VALUES(333,'0x39350','0x0042ed60','Obstacle_SimulateDynamics','FUN_0042ed60',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Performs dynamic physics integration (ballistic velocity, restitution, and world model transforms) for track scenery obstacles.',NULL,NULL);
INSERT INTO "functions" VALUES(334,'0x3a280','0x0042fc80','FX_UpdateFlyingParticles','FUN_0042fc80',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Updates ballistic trajectory and ground bounce for flying vehicle debris.',NULL,NULL);
INSERT INTO "functions" VALUES(335,NULL,'0x00430300','FX_UpdateExplosionNode','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','',NULL,NULL);
INSERT INTO "functions" VALUES(336,NULL,'0x004307b0','FX_UpdateVehicleWreck','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Particle type 7 vehicle catastrophic wreck simulator (unlinks 4 wheels, sets airborne flag, spawns flying wheel debris).',NULL,NULL);
INSERT INTO "functions" VALUES(337,NULL,'0x00431d00','FX_UpdateDetachedWheel','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Particle type 9 flying detached car wheel simulation with track bounce.',NULL,NULL);
INSERT INTO "functions" VALUES(338,NULL,'0x00432040','FX_UpdateVehicleCrashSequence','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Particle type 10 vehicle spin/flip crash sequence, tumbling airborne dynamics, and shrapnel explosion trigger.',NULL,NULL);
INSERT INTO "functions" VALUES(339,NULL,'0x00432bb0','FX_UpdateCarDebris','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Particle type 11 vehicle shrapnel/debris particle bouncing on collision surface.',NULL,NULL);
INSERT INTO "functions" VALUES(340,NULL,'0x00432f20','FX_SpawnWaterSplashes','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Water surface (type 0x5a) wheel contact detector emitting type 2 water splash particles.',NULL,NULL);
INSERT INTO "functions" VALUES(341,NULL,'0x004333e0','FX_SpawnTireDirtDebris','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Dirt, gravel, and grass wheel contact detector emitting type 8 rooster tail particles.',NULL,NULL);
INSERT INTO "functions" VALUES(342,NULL,'0x004338d0','FX_SpawnLandingDustPuffs','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Hard landing impact detector (+0x278) emitting left and right type 1 dust puffs.',NULL,NULL);
INSERT INTO "functions" VALUES(343,NULL,'0x00433f90','FX_SpawnTireSkidSmoke','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','High-speed asphalt drifting/braking detector emitting type 1 smoke puffs.',NULL,NULL);
INSERT INTO "functions" VALUES(344,NULL,'0x00434190','FX_UpdateAllParticles','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','',NULL,NULL);
INSERT INTO "functions" VALUES(345,NULL,'0x00434380','FX_SpawnParticle','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','',NULL,NULL);
INSERT INTO "functions" VALUES(346,NULL,'0x00434580','FX_UpdateWeatherBounds','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','',NULL,NULL);
INSERT INTO "functions" VALUES(347,NULL,'0x00434840','FX_UpdateWeatherGeometry','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Dynamic 3D rain streak / snowflake geometry builder and lightning palette flasher.',NULL,NULL);
INSERT INTO "functions" VALUES(348,NULL,'0x00435350','FX_FrameTick','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Master visual effects coordinator updating body lean/pitch visuals, weather meshes, and skid timers.',NULL,NULL);
INSERT INTO "functions" VALUES(349,NULL,'0x004356d0','Pos_InitAnimatedObjects','FUN_004356d0',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Converts keyframe coordinates in .POS to relative displacement deltas.',NULL,NULL);
INSERT INTO "functions" VALUES(350,NULL,'0x004357a0','Pos_UpdateAnimatedObjects','FUN_004357a0',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Advances keyframe playheads and translates moving scenery objects via Lisa_MoveObject.',NULL,NULL);
INSERT INTO "functions" VALUES(351,NULL,'0x004358e0','Math_LookupTrigAngle','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Fixed-point trigonometric sine/cosine lookup in 1024-entry LUT (DAT_004944e0).',NULL,NULL);
INSERT INTO "functions" VALUES(352,NULL,'0x00435910','Camera_UpdateOverview','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Track aerial overview / flyby camera positioned relative to road sequence splines.',NULL,NULL);
INSERT INTO "functions" VALUES(353,NULL,'0x00436990','Race_RenderViewport','FUN_00436990',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXTENDED','-','Calculates camera transform, invokes scene renderer, draws HUD.',NULL,NULL);
INSERT INTO "functions" VALUES(354,NULL,'0x00438030','Lisa_FlushRasterizerCommands','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Direct dispatch wrapper invoking Lisa_ExecuteRasterizerCommands.',NULL,NULL);
INSERT INTO "functions" VALUES(355,NULL,'0x00438050','Race_FindFocusedVehicle','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Trailing/leading vehicle target selector for broadcast and chase cameras.',NULL,NULL);
INSERT INTO "functions" VALUES(356,NULL,'0x00438210','Lisa_RenderPanorama','FUN_00438210',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Cylindrical horizon background blitter sampling 64KB .PAN texture using camera yaw and pitch angles.',NULL,NULL);
INSERT INTO "functions" VALUES(357,NULL,'0x004383b0','HUD_RenderPauseMenu','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','In-game pause menu renderer (Continue, Restart, Retire, CD Track selection).',NULL,NULL);
INSERT INTO "functions" VALUES(358,NULL,'0x00438880','HUD_RenderConfirmationPrompt','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','In-game confirmation modal renderer (Restart Y/N, Quit Y/N).',NULL,NULL);
INSERT INTO "functions" VALUES(359,'0x00037ee0','0x00438a60','HUD_RenderTrackResults','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Post-race leaderboard, standings, split times, and multiplayer wait screen renderer.',NULL,NULL);
INSERT INTO "functions" VALUES(360,NULL,'0x0043c3c0','HUD_RenderPlayAgainPrompt','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Post-race Play Track Again? (Y/N) prompt dialog renderer.',NULL,NULL);
INSERT INTO "functions" VALUES(361,NULL,'0x0043c910','Camera_UpdateChase','FUN_0043c910',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXTENDED','-','Multi-mode chase camera with velocity lookahead, 0.125 azimuth lag, slope adaptation (DEV-004).',NULL,NULL);
INSERT INTO "functions" VALUES(362,NULL,'0x0043d540','Car_UpdateDynamicObjects','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Transforms car body and wheel dynamic objects in Lisa 3D spatial grid.',NULL,NULL);
INSERT INTO "functions" VALUES(363,NULL,'0x0043dea0','Video_SetGraphicsMode','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Mode switch handler for 320x200, 640x480, and 800x600 display modes and viewport setup.',NULL,NULL);
INSERT INTO "functions" VALUES(364,NULL,'0x0043e2a0','Lisa_Init','FUN_0043e2a0',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Initializes Lisa 2 rasterizer viewport, Z-buffer, and focal lengths.',NULL,NULL);
INSERT INTO "functions" VALUES(365,NULL,'0x0043e6a0','Audio_LoadAssets','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','',NULL,NULL);
INSERT INTO "functions" VALUES(366,NULL,'0x0043e6c0','Video_FlipScreen','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Virtual framebuffer blitter presenting backbuffer to DirectDraw surface.',NULL,NULL);
INSERT INTO "functions" VALUES(367,NULL,'0x0043e6d0','Audio_StopSample','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','',NULL,NULL);
INSERT INTO "functions" VALUES(368,NULL,'0x0043e710','Audio_PlaySampleVol','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','',NULL,NULL);
INSERT INTO "functions" VALUES(369,NULL,'0x0043e9c0','HUD_RenderTelemetryOverlay','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','On-screen debug telemetry overlay (FPS, speed, pitch/roll, road index, diff).',NULL,NULL);
INSERT INTO "functions" VALUES(370,NULL,'0x0043ec70','FX_SpawnWeather','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','',NULL,NULL);
INSERT INTO "functions" VALUES(371,NULL,'0x0043edf0','HUD_CheckWrongWayHeading','-',9,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Evaluates car heading vs road chunk vector and manages the Wrong Way warning timer.',NULL,NULL);
INSERT INTO "functions" VALUES(372,'0x0003a725','0x0043ef30','HUD_RenderPlayerElements','-',9,'decompiled','watcom_reg','void',NULL,6859,310,'ADAPTED','-','In-race player HUD renderer (tachometer, turbo bar, position indicator, lap splits).',NULL,NULL);
INSERT INTO "functions" VALUES(373,NULL,'0x00442030','Car_ApplySteering','FUN_00442030',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Speed-attenuated front wheel steering lock and smoothing filter.',NULL,NULL);
INSERT INTO "functions" VALUES(374,NULL,'0x00442670','Car_PowertrainUpdate','FUN_00442670',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Engine propulsion, rolling and aerodynamic drag, transmission forward/reverse gear shifting.',NULL,NULL);
INSERT INTO "functions" VALUES(375,NULL,'0x004452c0','Sound_SynthesizeEngineRPM','FUN_004452c0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXTENDED','-','Computes RPM pitch modulation from 800-byte ENGINE.INF curve with DEV-006 protection.',NULL,NULL);
INSERT INTO "functions" VALUES(376,'0x00020c18','0x00446578','Surface_TestTrianglePositiveDZ','FUN_00446578',1,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','2D trapezoidal slope span test for table2 triangles ($dz \ge 0$).',NULL,NULL);
INSERT INTO "functions" VALUES(377,'0x00020c81','0x004465e1','Surface_TestTriangleNegativeDZ','FUN_004465e1',1,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','2D trapezoidal slope span test for table1 triangles ($dz < 0$).',NULL,NULL);
INSERT INTO "functions" VALUES(378,NULL,'0x004466d0','Lisa_RenderScene','FUN_004466d0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Master 3D frame render: culls objects, transforms vertices, rasterizes spans.',NULL,NULL);
INSERT INTO "functions" VALUES(379,NULL,'0x004468d0','Lisa_InitEngineMemory','FUN_004468d0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Allocates internal rasterizer buffers, vertex streams, and matrices.',NULL,NULL);
INSERT INTO "functions" VALUES(380,NULL,'0x00446c30','Lisa_FreeEngineMemory','FUN_00446c30',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','',NULL,NULL);
INSERT INTO "functions" VALUES(381,NULL,'0x00446ca0','Lisa_InitSpatialGrid','FUN_00446ca0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','',NULL,NULL);
INSERT INTO "functions" VALUES(382,NULL,'0x00446d90','Lisa_CreateDynamicObject','FUN_00446d90',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','',NULL,NULL);
INSERT INTO "functions" VALUES(383,NULL,'0x00446eb0','Lisa_MoveDynamicObject','FUN_00446eb0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','',NULL,NULL);
INSERT INTO "functions" VALUES(384,NULL,'0x00446f30','Lisa_UpdateObjectSpatialGrid','FUN_00446f30',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','',NULL,NULL);
INSERT INTO "functions" VALUES(385,NULL,'0x00447070','Lisa_SetDynamicObjectMesh','FUN_00447070',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','',NULL,NULL);
INSERT INTO "functions" VALUES(386,NULL,'0x00447150','Lisa_DeleteDynamicObject','FUN_00447150',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','',NULL,NULL);
INSERT INTO "functions" VALUES(387,NULL,'0x004471e0','Lisa_SetCameraViewport','FUN_004471e0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','',NULL,NULL);
INSERT INTO "functions" VALUES(388,NULL,'0x00447280','Lisa_GenerateMipmaps','FUN_00447280',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Multi-level texture downsampler and mipmap chain generator (invokes Lisa_DownsampleTextureMipmap, Lisa_FilterTextureBlock, Lisa_GenerateTextureSpanTable).',NULL,NULL);
INSERT INTO "functions" VALUES(389,NULL,'0x004475c0','Lisa_GenerateTextureSpanTable','FUN_004475c0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','',NULL,NULL);
INSERT INTO "functions" VALUES(390,NULL,'0x00447fb0','Lisa_DownsampleTextureMipmap','FUN_00447fb0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','',NULL,NULL);
INSERT INTO "functions" VALUES(391,NULL,'0x004481f0','Lisa_FilterTextureBlock','FUN_004481f0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','',NULL,NULL);
INSERT INTO "functions" VALUES(392,NULL,'0x00448620','Lisa_LoadOrCreateShadingTable','FUN_00448620',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','',NULL,NULL);
INSERT INTO "functions" VALUES(393,NULL,'0x00448860','Lisa_FindClosestPaletteColor','FUN_00448860',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','',NULL,NULL);
INSERT INTO "functions" VALUES(394,NULL,'0x00448990','Lisa_RenderSkyBackdrop','FUN_00448990',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','',NULL,NULL);
INSERT INTO "functions" VALUES(395,NULL,'0x00448c30','Lisa_CullObjectsOrthographic','FUN_00448c30',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','',NULL,NULL);
INSERT INTO "functions" VALUES(396,NULL,'0x00448e70','Lisa_FrustumCullObjects','FUN_00448e70',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Spatial grid frustum culler populating visible object list.',NULL,NULL);
INSERT INTO "functions" VALUES(397,NULL,'0x00449470','Lisa_CullObjects','FUN_00449470',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','',NULL,NULL);
INSERT INTO "functions" VALUES(398,NULL,'0x00449e70','Lisa_TransformVertices','FUN_00449e70',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Camera matrix rotation, perspective projection, and backface culling.',NULL,NULL);
INSERT INTO "functions" VALUES(399,NULL,'0x0044a3d0','Lisa_TransformVerticesPanorama','FUN_0044a3d0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','',NULL,NULL);
INSERT INTO "functions" VALUES(400,NULL,'0x0044a900','Lisa_ComputeObjectMatrix','FUN_0044a900',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','',NULL,NULL);
INSERT INTO "functions" VALUES(401,NULL,'0x0044ae20','Lisa_TransformSubmeshVerticesPanorama','FUN_0044ae20',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Cylindrical/panoramic submesh vertex transformer with 3D Euler rotation and perspective depth scaling.',NULL,NULL);
INSERT INTO "functions" VALUES(402,NULL,'0x0044b340','Lisa_ComputeCameraRotationMatrix','FUN_0044b340',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','',NULL,NULL);
INSERT INTO "functions" VALUES(403,NULL,'0x0044b480','Lisa_InitOpcodeTable','FUN_0044b480',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Binds polygon opcode rasterization dispatch table (PTR_LAB_0049c8e0).',NULL,NULL);
INSERT INTO "functions" VALUES(404,NULL,'0x0044b570','Lisa_SortDepthBuckets','FUN_0044b570',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Back-to-front 6,000 depth bucket flattener populating g_LisaDrawCommands.',NULL,NULL);
INSERT INTO "functions" VALUES(405,NULL,'0x0044b770','Lisa_DrawTriangle_Op0F','FUN_0044b770',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Opcode 0x0F flat-shaded triangle span preprocessor, backface cull, subpixel clipping, and depth bucket dispatcher.',NULL,NULL);
INSERT INTO "functions" VALUES(406,NULL,'0x0044b980','Lisa_DrawTriangle_Op10','FUN_0044b980',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Opcode 0x10 Gouraud-shaded triangle span preprocessor with vertex lighting unpack.',NULL,NULL);
INSERT INTO "functions" VALUES(407,NULL,'0x0044c1f0','Lisa_RenderSubmeshes','FUN_0044c1f0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Dispatches polygon opcodes across all visible transformed submeshes.',NULL,NULL);
INSERT INTO "functions" VALUES(408,NULL,'0x0044caa0','Lisa_DrawPolygon_Op12','LAB_0044caa0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Opcode 0x12: 1-bit transparent cutout triangle (pushes g_pLisaTransparencyLUT).',NULL,NULL);
INSERT INTO "functions" VALUES(409,NULL,'0x0044cac0','Lisa_DrawPolygon_Op13','LAB_0044cac0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Opcode 0x13: shadow / foliage alpha blend triangle (pushes g_pActiveSHD).',NULL,NULL);
INSERT INTO "functions" VALUES(410,NULL,'0x0044cae0','Lisa_DrawPolygon_Op16','LAB_0044cae0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Opcode 0x16: transparent cutout variant.',NULL,NULL);
INSERT INTO "functions" VALUES(411,NULL,'0x0044cb00','Lisa_DrawPolygon_Op17','LAB_0044cb00',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Opcode 0x17: shadow / alpha blend Gouraud triangle (pushes g_pActiveSHD).',NULL,NULL);
INSERT INTO "functions" VALUES(412,NULL,'0x0044cb20','Lisa_DrawTriangle_OpcodeHelper','FUN_0044cb20',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Common backface test, attribute pack, and span bucketer for opcodes 0x12, 0x13, 0x16, 0x17.',NULL,NULL);
INSERT INTO "functions" VALUES(413,NULL,'0x0044cf00','Lisa_DrawTriangle_Op14','FUN_0044cf00',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Opcode 0x14 shadow/alpha blend triangle preprocessor binding g_pActiveSHD.',NULL,NULL);
INSERT INTO "functions" VALUES(414,NULL,'0x0044d0f0','Lisa_DrawBillboard_Op07','FUN_0044d0f0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Opcode 0x07 shadow-blended camera-facing billboard/sprite dispatcher.',NULL,NULL);
INSERT INTO "functions" VALUES(415,NULL,'0x0044d230','Lisa_DrawBillboard_Op08','FUN_0044d230',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Opcode 0x08 color-key cutout transparent billboard/sprite dispatcher.',NULL,NULL);
INSERT INTO "functions" VALUES(416,NULL,'0x0044d550','Lisa_DrawTexturedTriangle_Op15','FUN_0044d550',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Opcode 0x15: perspective-correct textured triangle with 16.16 UV interpolation.',NULL,NULL);
INSERT INTO "functions" VALUES(417,'0x0004d718','0x0044dc60','Lisa_DrawTexturedTriangle_Op11_Unshaded','Lisa_DrawTexturedTriangle_Op11_Unshaded',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Lisa 3D textured triangle span preprocessor, backface cull, and depth bucket dispatcher (Opcode 0x11, unshaded).',NULL,NULL);
INSERT INTO "functions" VALUES(418,'0x0004cc58','0x0044e1b0','Lisa_DrawTexturedTriangle_Op11_Shaded','Lisa_DrawTexturedTriangle_Op11_Shaded',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Lisa 3D textured triangle Gouraud-shaded preprocessor and depth bucket dispatcher (Opcode 0x11, shaded).',NULL,NULL);
INSERT INTO "functions" VALUES(419,NULL,'0x0044e900','Lisa_DrawTexturedTriangle_Op15_Sub','FUN_0044e900',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','',NULL,NULL);
INSERT INTO "functions" VALUES(420,NULL,'0x0044f070','Lisa_InitRasterizerTables','FUN_0044f070',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','',NULL,NULL);
INSERT INTO "functions" VALUES(421,NULL,'0x0044f0e9','Lisa_ExecuteRasterizerCommands','FUN_0044f0e9',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Traverses depth-bucket sorted polygon command list and executes rasterizers.',NULL,NULL);
INSERT INTO "functions" VALUES(422,NULL,'0x00452800','Lisa_RenderTexturedTriangle_Op11','FUN_00452800',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Opcode 0x11 triangle edge walker and span setup for unshaded texture mapping.',NULL,NULL);
INSERT INTO "functions" VALUES(423,NULL,'0x004537dc','Lisa_DrawTexturedSpan_Op11','FUN_004537dc',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Low-level perspective/affine textured span blitter reading texels directly with stride 256 and alpha test.',NULL,NULL);
INSERT INTO "functions" VALUES(424,'0x00061220','0x00455580','Font_InitSystem','-',3,'decompiled','watcom_reg','void',NULL,249,NULL,'EXACT','-','Initializes font subsystem tables (30 slots)',NULL,NULL);
INSERT INTO "functions" VALUES(425,'0x00061319','0x00455610','Font_Shutdown','-',3,'decompiled','watcom_reg','void',NULL,128,NULL,'EXACT','-','Unloads active fonts and shuts down font subsystem',NULL,NULL);
INSERT INTO "functions" VALUES(426,'0x00061399','0x00455670','Font_Parse','-',3,'decompiled','watcom_reg','void',NULL,594,NULL,'EXACT','-','Parses LFT font header, initializes glyph handles and metrics',NULL,NULL);
INSERT INTO "functions" VALUES(427,'0x000615eb','0x00455820','Font_Load','FUN_00456270',3,'decompiled','watcom_reg','void',NULL,104,NULL,'ADAPTED','-','Loads and parses .LFT font header, offset tables, widths, and glyph raster data.',NULL,NULL);
INSERT INTO "functions" VALUES(428,'0x00061653','0x00455870','Font_Unload','-',3,'decompiled','watcom_reg','void',NULL,136,NULL,'EXACT','-','Frees sprite handles for font glyphs and marks slot free',NULL,NULL);
INSERT INTO "functions" VALUES(429,'0x000616db','0x004558d0','Font_GetTextWidth','-',3,'decompiled','watcom_reg','void',NULL,638,NULL,'EXACT','-','Calculates string rendering width in pixels',NULL,NULL);
INSERT INTO "functions" VALUES(430,'0x00061959','0x00456660','Font_DrawText','FUN_00456660',3,'decompiled','watcom_reg','void',NULL,1359,NULL,'EXACT','-','2D bitmap font rasterizer blitting characters to 8bpp buffer.',NULL,NULL);
INSERT INTO "functions" VALUES(431,NULL,'0x00456c40','Video_SetPalette','FUN_00456c40',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Uploads 256-color RGB palette to DirectDraw / hardware DAC.',NULL,NULL);
INSERT INTO "functions" VALUES(432,'0x00060f9c','0x004574a0','File_LoadToMemory','FUN_004574a0',4,'decompiled','watcom_reg','void',NULL,219,NULL,'ADAPTED','-','Generic binary loader (fopen, fread into allocated buffer).',NULL,NULL);
INSERT INTO "functions" VALUES(433,NULL,'0x00457890','Audio_MixCallback','FUN_00457890',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','32-channel software voice mixer with 16.16 fixed-point linear pitch resampling and stereo panning.',NULL,NULL);
INSERT INTO "functions" VALUES(434,NULL,'0x00457980','Audio_StopVoice','FUN_00457980',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Immediately stops voice playback and releases mixer channel allocation.',NULL,NULL);
INSERT INTO "functions" VALUES(435,NULL,'0x004579b0','Audio_PlayVoice','FUN_004579b0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Allocates mixer channel voice, configures volume, pan, loop flag, and starts playback.',NULL,NULL);
INSERT INTO "functions" VALUES(436,NULL,'0x00457aa0','Audio_SetVoiceParams','FUN_00457aa0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Real-time modulation of voice pitch frequency and stereo pan position.',NULL,NULL);
INSERT INTO "functions" VALUES(437,NULL,'0x00457ed0','Music_PlayTrack','FUN_00457ed0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','CD-DA track streamer using stb_vorbis mapped to circuits via DAT_00497eb8.',NULL,NULL);
INSERT INTO "functions" VALUES(438,NULL,'0x004582b0','Sound_LoadPAT','FUN_004582b0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Loads Gravis UltraSound GF1 .PAT patch audio files and converts 8-bit/16-bit linear PCM to S16SYS format.',NULL,NULL);
INSERT INTO "functions" VALUES(439,NULL,'0x00458e00','Sound_LoadWAV','FUN_00458e00',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Loads RIFF/WAVE PCM 8-bit/16-bit audio file and converts to S16SYS format.',NULL,NULL);
INSERT INTO "functions" VALUES(440,NULL,'0x0045b4f0','Lisa_PrintVersion','FUN_0045b4f0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Prints "Lisa 2 Development System" banner and build timestamp.',NULL,NULL);
INSERT INTO "functions" VALUES(441,'0x56034','0x00469950','CRT_Entry','entry',7,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','C Runtime startup entry point, parses command line, calls WinMain.',NULL,NULL);
INSERT INTO "functions" VALUES(442,NULL,'0x00499abc','Cdp_DecompressRLE','FUN_00499abc',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Delta-skip RLE decompression modifying active frame buffer with opcode skip codes.',NULL,NULL);
INSERT INTO "functions" VALUES(443,'0x10010',NULL,'main',NULL,5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT',NULL,NULL,NULL,NULL);
INSERT INTO "functions" VALUES(444,'0x6f549',NULL,'__cstart',NULL,7,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT',NULL,NULL,NULL,NULL);
INSERT INTO "functions" VALUES(445,'0x10198',NULL,'Timer_Init',NULL,5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT',NULL,NULL,NULL,NULL);
INSERT INTO "functions" VALUES(446,'0x1034c',NULL,'Timer_GetPITCounter',NULL,5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT',NULL,NULL,NULL,NULL);
INSERT INTO "functions" VALUES(447,'0x10238',NULL,'Timer_GetTime',NULL,5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT',NULL,NULL,NULL,NULL);
INSERT INTO "functions" VALUES(448,'0x63310',NULL,'Sound_LoadAsset',NULL,5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT',NULL,NULL,NULL,NULL);
INSERT INTO "functions" VALUES(449,'0x63f3e',NULL,'Audio_Init',NULL,5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT',NULL,NULL,NULL,NULL);
INSERT INTO "functions" VALUES(450,'0x20d14',NULL,'FatalError',NULL,5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT',NULL,NULL,NULL,NULL);
INSERT INTO "functions" VALUES(451,'0x0001aaf8',NULL,'Menu_InitCarViewport',NULL,10,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT',NULL,'Loads menucar.plc, menucar.msh, menucar.tex for 3D car viewport',NULL,NULL);
INSERT INTO "functions" VALUES(452,'0x00015344',NULL,'Menu_RenderCarViewport',NULL,10,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT',NULL,'Renders 3D rotating car model onto pedestal in car select screen',NULL,NULL);
CREATE TABLE globals (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    dos_address TEXT,                      -- Address in MAINDOS.EXE
    win_address TEXT UNIQUE,               -- Address in IGN_WIN.EXE
    name TEXT NOT NULL,                    -- Variable name (e.g. 'g_pActiveSRF')
    module_id INTEGER REFERENCES modules(id) ON DELETE SET NULL,
    type TEXT NOT NULL,                    -- Data type (e.g. 'uint8_t*', 'int32_t')
    size INTEGER,                          -- Size in bytes
    description TEXT,                      -- Description and context
    initial_value TEXT
);
INSERT INTO "globals" VALUES(125,NULL,'0x004c5398','g_hInstance',NULL,'HINSTANCE',NULL,'Application instance handle passed from `WinMain`.',NULL);
INSERT INTO "globals" VALUES(126,NULL,'0x004c5360','g_hCursor',NULL,'HCURSOR',NULL,'Default application mouse cursor.',NULL);
INSERT INTO "globals" VALUES(127,NULL,'0x00493720','g_HasPerfCounter',NULL,'uint32_t',NULL,'Flag: `1` if high-resolution timer (`QueryPerformanceCounter`) is available.',NULL);
INSERT INTO "globals" VALUES(128,NULL,'0x004c5348','g_PerfFrequency',NULL,'int64_t',NULL,'Performance counter frequency from `QueryPerformanceFrequency`.',NULL);
INSERT INTO "globals" VALUES(129,NULL,'0x004c5368','g_LastPerfCount',NULL,'int64_t',NULL,'Last measured performance counter timestamp.',NULL);
INSERT INTO "globals" VALUES(130,NULL,'0x00493734','g_GameStage',NULL,'int32_t',NULL,'Master state: `0` = Init, `1` = Active, `2` = Shutdown.',NULL);
INSERT INTO "globals" VALUES(131,NULL,'0x00639394','g_IntroState',NULL,'int32_t',NULL,'Intro sequence: `2` = Publisher logos, `1` = Init menus, `0` = Done.',NULL);
INSERT INTO "globals" VALUES(132,NULL,'0x00563c3c','g_MenuState',NULL,'int32_t',NULL,'Menu state: `1` when interactive menu is rendering.',NULL);
INSERT INTO "globals" VALUES(133,NULL,'0x00563d9c','g_MenuSelection',NULL,'int32_t',NULL,'Menu selection processed flag.',NULL);
INSERT INTO "globals" VALUES(134,NULL,'0x00553290','g_RaceLoadStage',NULL,'int32_t',NULL,'Track/race loading stage (`1`, `2`, `3`).',NULL);
INSERT INTO "globals" VALUES(135,NULL,'0x00525e5c','g_InRace',NULL,'int32_t',NULL,'In-race simulation flag: `1` during active driving.',NULL);
INSERT INTO "globals" VALUES(136,NULL,'0x004ba6e0','g_ScreenWidth',NULL,'int32_t',NULL,'Target screen resolution width (`640` or `320`).',NULL);
INSERT INTO "globals" VALUES(137,NULL,'0x004ba6e4','g_ScreenHeight',NULL,'int32_t',NULL,'Target screen resolution height (`480` or `200`).',NULL);
INSERT INTO "globals" VALUES(138,NULL,'0x004ba6e8','g_ColorDepth',NULL,'int32_t',NULL,'Screen bit depth (`8` bits per pixel).',NULL);
INSERT INTO "globals" VALUES(139,NULL,'0x0054f998','g_pSysGfxPic',NULL,'uint8_t*',NULL,'Pointer to loaded `N_SYSGFX.PIC` buffer.',NULL);
INSERT INTO "globals" VALUES(140,NULL,'0x00553088','g_pSysG2Pic',NULL,'uint8_t*',NULL,'Pointer to loaded `N_SYSG_2.PIC` buffer.',NULL);
INSERT INTO "globals" VALUES(141,NULL,'0x00563bfc','g_pSysCol',NULL,'uint8_t*',NULL,'Pointer to loaded `SYS.COL` buffer (`+ 8` is the 256-color palette).',NULL);
INSERT INTO "globals" VALUES(142,NULL,'0x004937bc','g_pActiveSRF',NULL,'uint8_t*',NULL,'Pointer to currently loaded `.SRF` surface buffer.',NULL);
INSERT INTO "globals" VALUES(143,NULL,'0x004c53b4','g_SRF_GridCellsX',NULL,'int32_t',NULL,'Active track surface grid dimension X.',NULL);
INSERT INTO "globals" VALUES(144,NULL,'0x004c53d0','g_SRF_GridCellsZ',NULL,'int32_t',NULL,'Active track surface grid dimension Z.',NULL);
INSERT INTO "globals" VALUES(145,NULL,'0x004c53b8','g_SRF_CellSizeX',NULL,'int32_t',NULL,'Active track surface cell size X (512 units).',NULL);
INSERT INTO "globals" VALUES(146,NULL,'0x004c53c8','g_SRF_CellSizeZ',NULL,'int32_t',NULL,'Active track surface cell size Z (512 units).',NULL);
INSERT INTO "globals" VALUES(147,NULL,'0x004c53a8','g_pSRF_Triangles',NULL,'uint8_t*',NULL,'Pointer to 24-byte surface collision triangle records in active `.SRF`.',NULL);
INSERT INTO "globals" VALUES(148,NULL,'0x00525e60','g_pActiveMSH',NULL,'uint8_t*',NULL,'Loaded `<TRACK>.MSH` geometry memory buffer.',NULL);
INSERT INTO "globals" VALUES(149,NULL,'0x0054f9cc','g_pActivePLC',NULL,'uint32_t*',NULL,'Loaded `<TRACK>.PLC` placed object table buffer.',NULL);
INSERT INTO "globals" VALUES(150,NULL,'0x00552fc8','g_pCarsPLC',NULL,'uint32_t*',NULL,'Loaded `CARS.PLC` placed car object buffer.',NULL);
INSERT INTO "globals" VALUES(151,NULL,'0x00563be4','g_pActiveTAB',NULL,'uint8_t*',NULL,'Loaded `<TRACK>.TAB` 64KB shading lookup matrix.',NULL);
INSERT INTO "globals" VALUES(152,NULL,'0x0049c9f8','g_pLisaActiveShading',NULL,'uint8_t*',NULL,'Active shading table pointer bound in Lisa3D rasterizer.',NULL);
INSERT INTO "globals" VALUES(153,NULL,'0x00639c0c','g_pActiveTRI',NULL,'uint8_t*',NULL,'Loaded `<TRACK>.TRI` 500-byte road chunk index buffer.',NULL);
INSERT INTO "globals" VALUES(154,NULL,'0x0063c5f0','g_LisaCamera',NULL,'LisaCamera*',NULL,'Lisa 3D camera state, matrices, and viewport parameters.',NULL);
INSERT INTO "globals" VALUES(155,NULL,'0x0049c8e0','g_LisaOpcodeTable',NULL,'void**',NULL,'Opcode function jump table (0x00..0x17).',NULL);
INSERT INTO "globals" VALUES(156,NULL,'0x0063c5cc','g_LisaVisibleObjects',NULL,'int**',NULL,'Array of pointers to visible objects from frustum culling.',NULL);
INSERT INTO "globals" VALUES(157,NULL,'0x0063c5fc','g_LisaVisibleSubmeshes',NULL,'int**',NULL,'Array of visible submeshes and projected screen vertices.',NULL);
INSERT INTO "globals" VALUES(158,NULL,'0x0063c5bc','g_LisaDrawCommands',NULL,'void**',NULL,'Depth-bucket sorted linked list of rasterizer polygon draw commands.',NULL);
INSERT INTO "globals" VALUES(159,NULL,'0x005530f0','g_pActiveTEX',NULL,'uint8_t*',NULL,'Loaded `<TRACK>.TEX` raw texture buffer (64KB aligned).',NULL);
INSERT INTO "globals" VALUES(160,NULL,'0x00553064','g_pActivePOS',NULL,'uint8_t*',NULL,'Loaded `<TRACK>.POS` scenery object keyframe animation buffer.',NULL);
INSERT INTO "globals" VALUES(161,NULL,'0x00552f60','g_pTrackRoadSequence',NULL,'int32_t*',NULL,'Ordered sequence of road chunks (24 bytes per chunk).',NULL);
INSERT INTO "globals" VALUES(162,NULL,'0x0063a01c','g_pTrackSplineNodes',NULL,'uint8_t*',NULL,'Array of 75-byte road spline nodes (left/right rails, center, heading).',NULL);
INSERT INTO "globals" VALUES(163,NULL,'0x0063a024','g_pTrackSplinePointers',NULL,'void**',NULL,'Pointers to active spline nodes indexed by chunk sequence.',NULL);
INSERT INTO "globals" VALUES(164,NULL,'0x0054f954','g_CheckpointCount',NULL,'int32_t',NULL,'Count of active type 150..154 split-time checkpoint trigger gates.',NULL);
INSERT INTO "globals" VALUES(165,NULL,'0x0054f904','g_pCheckpoints',NULL,'uint8_t*',NULL,'Checkpoint collision and trigger gate definitions.',NULL);
INSERT INTO "globals" VALUES(166,NULL,'0x0063b5f0','g_pActiveSHD',NULL,'uint8_t*',NULL,'Loaded `<TRACK>.SHD` 64KB shadow / alpha lookup table buffer.',NULL);
INSERT INTO "globals" VALUES(167,NULL,'0x0063b5e8','g_pLisaDepthBuckets',NULL,'void**',NULL,'6,000-entry depth bucket pointer array for polygon ordering.',NULL);
INSERT INTO "globals" VALUES(168,NULL,'0x0063c5b4','g_pLisaTransparencyLUT',NULL,'uint8_t*',NULL,'64KB color-key transparency lookup table (generated by `FUN_00402940`).',NULL);
INSERT INTO "globals" VALUES(169,NULL,'0x0063c5c8','g_pLisaTextureSheets',NULL,'uint8_t**',NULL,'Table of loaded texture base pointers (Track, Cars, Lights, Smoke).',NULL);
INSERT INTO "globals" VALUES(170,NULL,'0x004cdc28','g_pLisaActiveMipTable',NULL,'uint8_t**',NULL,'Pointer to active texture mip table bound during submesh dispatch.',NULL);
INSERT INTO "globals" VALUES(171,NULL,'0x004b63c4','g_pLisaActivePage',NULL,'uint8_t*',NULL,'Base pointer of active 64KB ($256 \times 256$) texture page in span rasterizer.',NULL);
INSERT INTO "globals" VALUES(172,NULL,'0x00498740','g_ViewportMinX',NULL,'int32_t',NULL,'Left screen pixel boundary for viewport clipping.',NULL);
INSERT INTO "globals" VALUES(173,NULL,'0x00498744','g_ViewportMinY',NULL,'int32_t',NULL,'Top screen pixel boundary for viewport clipping.',NULL);
INSERT INTO "globals" VALUES(174,NULL,'0x00498748','g_ViewportMaxX',NULL,'int32_t',NULL,'Right screen pixel boundary for viewport clipping.',NULL);
INSERT INTO "globals" VALUES(175,NULL,'0x0049874c','g_ViewportMaxY',NULL,'int32_t',NULL,'Bottom screen pixel boundary for viewport clipping.',NULL);
INSERT INTO "globals" VALUES(176,NULL,'0x0049ca2c','g_SubpixelMinX',NULL,'int32_t',NULL,'Fixed-point 8.8 subpixel left clipping bound (`g_ViewportMinX << 8`).',NULL);
INSERT INTO "globals" VALUES(177,NULL,'0x0049ca30','g_SubpixelMinY',NULL,'int32_t',NULL,'Fixed-point 8.8 subpixel top clipping bound (`g_ViewportMinY << 8`).',NULL);
INSERT INTO "globals" VALUES(178,NULL,'0x0049ca34','g_SubpixelMaxX',NULL,'int32_t',NULL,'Fixed-point 8.8 subpixel right clipping bound (`(g_ViewportMaxX + 1) * 256`).',NULL);
INSERT INTO "globals" VALUES(179,NULL,'0x0049ca38','g_SubpixelMaxY',NULL,'int32_t',NULL,'Fixed-point 8.8 subpixel bottom clipping bound (`(g_ViewportMaxY + 1) * 256`).',NULL);
INSERT INTO "globals" VALUES(180,NULL,'0x005db040','g_SceneryParticles',NULL,'SceneryParticle[]',NULL,'Global active particle and dynamic scenery simulation pool.',NULL);
INSERT INTO "globals" VALUES(181,NULL,'0x004792f0','g_PhysicsTimestep',NULL,'double',NULL,'72 Hz physics integration timestep ($1/72\text{ s} \approx 0.013888889\text{ s}$).',NULL);
INSERT INTO "globals" VALUES(182,NULL,'0x00479af0','g_PhysicsScaleFactor',NULL,'double',NULL,'World velocity integration multiplier (`21.76`).',NULL);
INSERT INTO "globals" VALUES(183,NULL,'0x004792c8','g_PhysicsGravity',NULL,'double',NULL,'Gravitational acceleration constant (`9.81` $m/s^2$).',NULL);
INSERT INTO "globals" VALUES(184,NULL,'0x00479b50','g_PhysicsGravityTick',NULL,'double',NULL,'Vertical downward velocity delta per tick (`0.2` units).',NULL);
INSERT INTO "globals" VALUES(185,NULL,'0x00479b58','g_PhysicsRideHeight',NULL,'double',NULL,'Ground clearance equilibrium offset (`5.0` units above surface).',NULL);
INSERT INTO "globals" VALUES(186,NULL,'0x00479ca0','g_PhysicsMaxSlopeSin',NULL,'double',NULL,'Arcsine clamping limit for pitch and roll calculation (`0.95`).',NULL);
INSERT INTO "globals" VALUES(187,NULL,'0x00479cb0','g_PhysicsRateLimitRad',NULL,'double',NULL,'Maximum pitch and roll angular change rate (`0.1` rad/tick).',NULL);
INSERT INTO "globals" VALUES(188,NULL,'0x00479c48','g_PhysicsWorldOffset',NULL,'double',NULL,'Spatial grid origin centering offset (`25600.0` units, $50 \times 512$).',NULL);
INSERT INTO "globals" VALUES(189,NULL,'0x00552fe0','g_pPlayerVehicle',NULL,'VehicleState*',NULL,'Pointer to active player vehicle state struct.',NULL);
INSERT INTO "globals" VALUES(190,NULL,'0x00553000','g_pAIVehicles',NULL,'VehicleState*',NULL,'Array of 5 opponent AI vehicle states.',NULL);
INSERT INTO "globals" VALUES(191,NULL,'0x00497eb8','g_TrackCDAudioMapping',NULL,'uint8_t[8]',NULL,'CD-DA audio track mapping index per circuit (Tracks 2..8).',NULL);
INSERT INTO "globals" VALUES(192,NULL,'0x00552f40','g_pAudioContext',NULL,'AudioContext*',NULL,'Master audio mixer device and state context.',NULL);
INSERT INTO "globals" VALUES(193,NULL,'0x00552f44','g_AudioChannels',NULL,'AudioVoice[32]',NULL,'32-channel software voice mixer table.',NULL);
INSERT INTO "globals" VALUES(194,NULL,'0x00552f48','g_MasterSoundVolume',NULL,'int32_t',NULL,'Master SFX attenuation level (0..128).',NULL);
INSERT INTO "globals" VALUES(195,NULL,'0x00552f4c','g_MasterMusicVolume',NULL,'int32_t',NULL,'Master CD-DA music attenuation level (0..128).',NULL);
CREATE TABLE metadata (
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL
);
INSERT INTO "metadata" VALUES('target_exe','MAINDOS.EXE');
INSERT INTO "metadata" VALUES('reference_exe','MAINDOS_32BIT.EXE');
INSERT INTO "metadata" VALUES('compiler','Watcom C/C++ 10.6');
INSERT INTO "metadata" VALUES('project_name','Racing Dynamite Decompilation');
CREATE TABLE modules (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT UNIQUE NOT NULL,             -- e.g. 'getsurf.c', 'lisa3d.c'
    original_path TEXT,                    -- e.g. 'd:\projects\ignition\getsurf\getsurf.c'
    decomp_path TEXT,                      -- e.g. 'decomp/getsurf.c'
    description TEXT,                      -- Module purpose / subsystem
    notes TEXT
);
INSERT INTO "modules" VALUES(1,'getsurf.c','d:\projects\ignition\getsurf\getsurf.c','decomp/getsurf.c','Track surface raycasting and collision grid',NULL);
INSERT INTO "modules" VALUES(2,'lisa3d.c','lisa3d.c','decomp/lisa3d.c','Lisa 2 3D rasterizer, polygon opcodes, and scene transformation',NULL);
INSERT INTO "modules" VALUES(3,'geputget.c','geputget.c','decomp/geputget.c','2D graphics blitting, font loading, and palette management',NULL);
INSERT INTO "modules" VALUES(4,'mem.c','mem.c','decomp/mem.c','Memory management, buffer allocation, and file I/O',NULL);
INSERT INTO "modules" VALUES(5,'main.c','main.c','decomp/main.c','Main game loop, vehicle state, physics integration, AI navigation',NULL);
INSERT INTO "modules" VALUES(6,'sound.c','sound.c','decomp/sound.c','Sound effects pools, engine RPM audio synthesis, and mixer',NULL);
INSERT INTO "modules" VALUES(7,'MSVC CRT',NULL,NULL,NULL,NULL);
INSERT INTO "modules" VALUES(8,'vehicle.c',NULL,NULL,NULL,NULL);
INSERT INTO "modules" VALUES(9,'fx.c',NULL,NULL,NULL,NULL);
INSERT INTO "modules" VALUES(10,'menu.c','menu.c','decomp/src/menu.c','Menu user interface, CDP video background streams, and car select viewport',NULL);
CREATE TABLE struct_fields (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    struct_id INTEGER NOT NULL REFERENCES structs(id) ON DELETE CASCADE,
    offset INTEGER NOT NULL,               -- Byte offset within struct
    type TEXT NOT NULL,                    -- Field C type
    name TEXT NOT NULL,                    -- Field name
    size INTEGER,                          -- Field size in bytes
    description TEXT,
    UNIQUE(struct_id, offset)
);
INSERT INTO "struct_fields" VALUES(139,19,0,'int32_t','grid_cells_x',NULL,'Grid cells along X (200)');
INSERT INTO "struct_fields" VALUES(140,19,4,'int32_t','grid_cells_z',NULL,'Grid cells along Z (200)');
INSERT INTO "struct_fields" VALUES(141,19,8,'int32_t','cell_size_z',NULL,'World dimension of cell Z (512)');
INSERT INTO "struct_fields" VALUES(142,19,12,'int32_t','cell_size_x',NULL,'World dimension of cell X (512)');
INSERT INTO "struct_fields" VALUES(143,19,16,'int32_t','grid_stride_x',NULL,'Spatial index stride X (101)');
INSERT INTO "struct_fields" VALUES(144,19,20,'int32_t','grid_stride_z',NULL,'Spatial index stride Z (101)');
INSERT INTO "struct_fields" VALUES(145,19,24,'int32_t','triangle_count',NULL,'Total surface collision triangles');
INSERT INTO "struct_fields" VALUES(146,19,28,'int32_t','table1_count',NULL,'Primary index buffer integer count');
INSERT INTO "struct_fields" VALUES(147,19,32,'int32_t','table2_count',NULL,'Secondary index buffer integer count');
INSERT INTO "struct_fields" VALUES(148,20,0,'int32_t','table2_offset',NULL,'Byte offset into table2 (divide by 4)');
INSERT INTO "struct_fields" VALUES(149,20,4,'int32_t','table1_offset',NULL,'Byte offset into table1 (divide by 4)');
INSERT INTO "struct_fields" VALUES(150,20,8,'uint16_t','table1_count',NULL,'Number of triangles intersecting cell');
INSERT INTO "struct_fields" VALUES(151,20,10,'uint16_t','table2_count',NULL,'Number of secondary entities');
INSERT INTO "struct_fields" VALUES(152,21,0,'int32_t','x_base',NULL,'Base apex X coordinate');
INSERT INTO "struct_fields" VALUES(153,21,4,'int32_t','z_base',NULL,'Base apex Z coordinate');
INSERT INTO "struct_fields" VALUES(154,21,8,'int32_t','slope1',NULL,'16.16 fixed-point slope dx1/dz');
INSERT INTO "struct_fields" VALUES(155,21,12,'int32_t','slope2',NULL,'16.16 fixed-point slope dx2/dz');
INSERT INTO "struct_fields" VALUES(156,21,16,'int32_t','flags_and_dz',NULL,'Low 16 bits = dz (int16_t), High 16 bits = submesh polygon dword offset');
INSERT INTO "struct_fields" VALUES(157,21,20,'int32_t','v_ptr',NULL,'Byte offset into .PLC placed object array (obj_idx = v_ptr / 42)');
INSERT INTO "struct_fields" VALUES(158,22,0,'int32_t','submesh_offset',NULL,'Offset in 4-byte dwords into .MSH geometry');
INSERT INTO "struct_fields" VALUES(159,22,4,'int32_t','model_type',NULL,'Scenery / collision model archetype (e.g. 300, 2, 3)');
INSERT INTO "struct_fields" VALUES(160,22,8,'int32_t','pos_x',NULL,'World X position');
INSERT INTO "struct_fields" VALUES(161,22,12,'int32_t','pos_y',NULL,'World Y elevation');
INSERT INTO "struct_fields" VALUES(162,22,16,'int32_t','pos_z',NULL,'World Z position');
INSERT INTO "struct_fields" VALUES(163,23,0,'uint32_t','header',NULL,'Opcode in low byte (0x11, 0x12, 0x13, 0x15, 0x16, 0x17), flags in high 24 bits');
INSERT INTO "struct_fields" VALUES(164,23,4,'uint32_t','vi0',NULL,'Index of vertex 0 in submesh vertex buffer');
INSERT INTO "struct_fields" VALUES(165,23,8,'uint32_t','vi1',NULL,'Index of vertex 1 in submesh vertex buffer');
INSERT INTO "struct_fields" VALUES(166,23,12,'uint32_t','vi2',NULL,'Index of vertex 2 in submesh vertex buffer');
INSERT INTO "struct_fields" VALUES(167,23,16,'int32_t','tu0',NULL,'Vertex 0 U texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)');
INSERT INTO "struct_fields" VALUES(168,23,20,'int32_t','tv0',NULL,'Vertex 0 V texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)');
INSERT INTO "struct_fields" VALUES(169,23,24,'int32_t','tu1',NULL,'Vertex 1 U texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)');
INSERT INTO "struct_fields" VALUES(170,23,28,'int32_t','tv1',NULL,'Vertex 1 V texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)');
INSERT INTO "struct_fields" VALUES(171,23,32,'int32_t','tu2',NULL,'Vertex 2 U texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)');
INSERT INTO "struct_fields" VALUES(172,23,36,'int32_t','tv2',NULL,'Vertex 2 V texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)');
INSERT INTO "struct_fields" VALUES(173,23,40,'uint32_t','extra',NULL,'Texture page byte offset within .TEX file (page_index * 65536)');
INSERT INTO "struct_fields" VALUES(174,24,24,'24 bytes internal transformation state
    double','rot_x',NULL,'Camera rotation pitch');
INSERT INTO "struct_fields" VALUES(175,24,32,'double','rot_y',NULL,'Camera rotation yaw');
INSERT INTO "struct_fields" VALUES(176,24,40,'double','rot_z',NULL,'Camera rotation roll');
INSERT INTO "struct_fields" VALUES(177,24,48,'double','zoom',NULL,'Focal length / zoom factor');
INSERT INTO "struct_fields" VALUES(178,24,56,'int32_t','enable_sky',NULL,'Sky backdrop enable flag');
INSERT INTO "struct_fields" VALUES(179,24,60,'int32_t','enable_frustum_cull',NULL,'Spatial grid frustum cull flag');
INSERT INTO "struct_fields" VALUES(180,24,64,'int32_t','enable_transform',NULL,'Vertex transformation flag');
INSERT INTO "struct_fields" VALUES(181,24,80,'12 bytes
    int32_t','enable_submeshes',NULL,'Submesh processing flag');
INSERT INTO "struct_fields" VALUES(182,24,84,'int32_t','enable_depth_sort',NULL,'Depth bucket sorting flag');
INSERT INTO "struct_fields" VALUES(183,24,88,'int32_t','shading_mode',NULL,'0 = unshaded, 1 = shaded/alpha');
INSERT INTO "struct_fields" VALUES(184,24,92,'int32_t','vertex_counter',NULL,'Transformed vertex counter');
INSERT INTO "struct_fields" VALUES(185,24,96,'int32_t','visible_obj_count',NULL,'Count of visible objects passed culling');
INSERT INTO "struct_fields" VALUES(186,24,100,'int32_t','submesh_count',NULL,'Count of visible submeshes');
INSERT INTO "struct_fields" VALUES(187,24,104,'int32_t','active_draw_cmd',NULL,'Active draw command index');
INSERT INTO "struct_fields" VALUES(188,24,128,'20 bytes
    int32_t','viewport_x',NULL,'Viewport center X');
INSERT INTO "struct_fields" VALUES(189,24,132,'int32_t','viewport_y',NULL,'Viewport center Y');
INSERT INTO "struct_fields" VALUES(190,24,136,'int32_t','viewport_width',NULL,'Viewport screen width');
INSERT INTO "struct_fields" VALUES(191,24,156,'16 bytes
    int32_t','fov_x',NULL,'Horizontal FOV scale (8.8 fixed-point)');
INSERT INTO "struct_fields" VALUES(192,24,160,'int32_t','fov_y',NULL,'Vertical FOV scale (8.8 fixed-point)');
INSERT INTO "struct_fields" VALUES(193,24,164,'int32_t','projection_type',NULL,'0 = perspective 3D, 1 = panorama/ortho');
INSERT INTO "struct_fields" VALUES(194,25,0,'double','engine_power',NULL,'Mass / power scaled by difficulty mode');
INSERT INTO "struct_fields" VALUES(195,25,8,'double','acceleration',NULL,'Forward traction acceleration');
INSERT INTO "struct_fields" VALUES(196,25,16,'double','top_speed',NULL,'Terminal velocity clamp');
INSERT INTO "struct_fields" VALUES(197,25,24,'double','steering_rate',NULL,'Turning responsiveness');
INSERT INTO "struct_fields" VALUES(198,25,32,'double','brake_force',NULL,'Deceleration coefficient');
INSERT INTO "struct_fields" VALUES(199,25,40,'double','turbo_boost',NULL,'Turbo propulsion multiplier');
INSERT INTO "struct_fields" VALUES(200,25,48,'double','suspension_k',NULL,'Spring rate');
INSERT INTO "struct_fields" VALUES(201,25,56,'double','damping_c',NULL,'Shock absorber damping');
INSERT INTO "struct_fields" VALUES(202,25,64,'double','collision_radius',NULL,'Spherical bounding volume');
INSERT INTO "struct_fields" VALUES(203,25,72,'int32_t','mass_integer',NULL,'Fixed-point mass value');
INSERT INTO "struct_fields" VALUES(204,25,96,'int32_t','wheel_fl_y',NULL,'Front-left wheel elevation');
INSERT INTO "struct_fields" VALUES(205,25,104,'int32_t','wheel_fr_y',NULL,'Front-right wheel elevation');
INSERT INTO "struct_fields" VALUES(206,25,112,'int32_t','wheel_rl_y',NULL,'Rear-left wheel elevation');
INSERT INTO "struct_fields" VALUES(207,25,120,'int32_t','wheel_rr_y',NULL,'Rear-right wheel elevation');
INSERT INTO "struct_fields" VALUES(208,26,0,'uint8_t','type',NULL,'Node type identifier');
INSERT INTO "struct_fields" VALUES(209,26,1,'uint8_t','code',NULL,'Road code passed from .TRI');
INSERT INTO "struct_fields" VALUES(210,26,2,'int32_t','left_x',NULL,'Left rail coordinate X (+ 0x6400)');
INSERT INTO "struct_fields" VALUES(211,26,6,'int32_t','left_y',NULL,'Left rail coordinate Y');
INSERT INTO "struct_fields" VALUES(212,26,10,'int32_t','left_z',NULL,'Left rail coordinate Z (+ 0x6400)');
INSERT INTO "struct_fields" VALUES(213,26,14,'int32_t','right_x',NULL,'Right rail coordinate X (+ 0x6400)');
INSERT INTO "struct_fields" VALUES(214,26,18,'int32_t','right_y',NULL,'Right rail coordinate Y');
INSERT INTO "struct_fields" VALUES(215,26,22,'int32_t','right_z',NULL,'Right rail coordinate Z (+ 0x6400)');
INSERT INTO "struct_fields" VALUES(216,26,42,'boundary padding
    float','heading',NULL,'Tangent heading angle in radians');
INSERT INTO "struct_fields" VALUES(217,26,74,'Secondary attributes
    uint8_t','fork_flag',NULL,'Branching directive (0 = mainline, 1 = left, 2 = right)');
INSERT INTO "struct_fields" VALUES(218,27,0,'uint8_t','road_code',NULL,'Surface material code');
INSERT INTO "struct_fields" VALUES(219,27,1,'int16_t','left_vertex',NULL,'Submesh vertex index for left boundary');
INSERT INTO "struct_fields" VALUES(220,27,23,'Secondary surface attributes
    int16_t','right_vertex',NULL,'Submesh vertex index for right boundary');
INSERT INTO "struct_fields" VALUES(221,27,109,'Internal friction parameters
    uint8_t','fork_flag',NULL,'Branching directive (0 = normal, 1 = left, 2 = right)');
INSERT INTO "struct_fields" VALUES(222,28,0,'int32_t','pos_x',NULL,'X position delta');
INSERT INTO "struct_fields" VALUES(223,28,4,'int32_t','pos_y',NULL,'Y elevation delta');
INSERT INTO "struct_fields" VALUES(224,28,8,'int32_t','pos_z',NULL,'Z position delta');
INSERT INTO "struct_fields" VALUES(225,28,12,'int32_t','rot_x',NULL,'Pitch angle (tenths of degree)');
INSERT INTO "struct_fields" VALUES(226,28,16,'int32_t','rot_y',NULL,'Yaw angle (tenths of degree)');
INSERT INTO "struct_fields" VALUES(227,28,20,'int32_t','rot_z',NULL,'Roll angle (tenths of degree)');
CREATE TABLE structs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT UNIQUE NOT NULL,             -- Struct name (e.g. 'SrfHeader')
    size INTEGER,                          -- Total size in bytes
    module_id INTEGER REFERENCES modules(id) ON DELETE SET NULL,
    description TEXT,
    notes TEXT
);
INSERT INTO "structs" VALUES(19,'SrfHeader',36,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(20,'SrfCell',12,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(21,'SrfTriangle',24,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(22,'PlcObject',20,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(23,'MshPolygon',44,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(24,'LisaCamera',168,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(25,'CarPhysicsState',124,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(26,'TrackSplineNode',75,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(27,'TriChunk',500,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(28,'PosKeyframe',24,NULL,NULL,NULL);
CREATE INDEX idx_functions_dos_addr ON functions(dos_address);
CREATE INDEX idx_functions_module ON functions(module_id);
CREATE INDEX idx_functions_status ON functions(status);
CREATE INDEX idx_globals_dos_addr ON globals(dos_address);
CREATE INDEX idx_globals_name ON globals(name);
CREATE INDEX idx_struct_fields_struct ON struct_fields(struct_id);
DELETE FROM "sqlite_sequence";
INSERT INTO "sqlite_sequence" VALUES('modules',10);
INSERT INTO "sqlite_sequence" VALUES('functions',452);
INSERT INTO "sqlite_sequence" VALUES('globals',195);
INSERT INTO "sqlite_sequence" VALUES('structs',28);
INSERT INTO "sqlite_sequence" VALUES('struct_fields',227);
COMMIT;
