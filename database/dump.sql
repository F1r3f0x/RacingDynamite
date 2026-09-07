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
INSERT INTO "functions" VALUES(1,NULL,'0x00469950','CRT_Entry','entry',7,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','C Runtime startup entry point, parses command line, calls `WinMain`.',NULL,NULL);
INSERT INTO "functions" VALUES(2,NULL,'0x004120a0','WinMain','FUN_004120a0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/core/main.c','Main entry point; registers window class, queries timer, runs message/tick loop.',NULL,NULL);
INSERT INTO "functions" VALUES(3,NULL,'0x00412230','App_FrameTick','FUN_00412230',5,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Main engine tick; dispatches Init (0), Main Loop (1), and Shutdown (2).',NULL,NULL);
INSERT INTO "functions" VALUES(4,NULL,'0x00412500','App_Init','FUN_00412500',5,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Creates game window, initializes DirectDraw and DirectInput subsystems.',NULL,NULL);
INSERT INTO "functions" VALUES(5,NULL,'0x00412530','App_Shutdown','FUN_00412530',5,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Releases DirectDraw surfaces, DirectSound, and window handles.',NULL,NULL);
INSERT INTO "functions" VALUES(6,NULL,'0x00417270','Game_Init','FUN_00417270',5,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Sets initial game state flags, resets timers, initiates intro sequence.',NULL,NULL);
INSERT INTO "functions" VALUES(7,NULL,'0x004172b0','Game_StateDispatcher','FUN_004172b0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/core/game_state.c','Top-level game loop state machine dispatcher (Intro -> Menus -> Race).',NULL,NULL);
INSERT INTO "functions" VALUES(8,NULL,'0x00418130','Load_SystemGraphicsAndFonts','FUN_00418130',3,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Loads `SYS.COL`, `N_SYSGFX.PIC`, `N_SYSG_2.PIC`, and `.LFT` fonts.',NULL,NULL);
INSERT INTO "functions" VALUES(9,NULL,'0x004574a0','File_LoadToMemory','FUN_004574a0',4,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/formats/col.c','Generic binary loader (`fopen`, `fread` into allocated buffer).',NULL,NULL);
INSERT INTO "functions" VALUES(10,NULL,'0x00456c40','Video_SetPalette','FUN_00456c40',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/formats/col.c','Uploads 256-color RGB palette to DirectDraw / hardware DAC.',NULL,NULL);
INSERT INTO "functions" VALUES(11,'0x0001fee0','0x00412670','Surface_LoadSRF','FUN_00412670',1,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/formats/srf.c','Loads `.SRF` track surface heightfield and collision grid.',NULL,NULL);
INSERT INTO "functions" VALUES(12,'0x0002002c','0x004127a0','Surface_FreeSRF','FUN_004127a0',1,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/formats/srf.c','Frees active `.SRF` surface memory buffer.',NULL,NULL);
INSERT INTO "functions" VALUES(13,'0x00020814','0x00412fc0','Surface_GetCell','FUN_00412fc0',1,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXTENDED','src/physics/getsurf.c','Computes grid cell index `((z/512)+50)*stride + ((x/512)+50)` with origin offset 50.0.',NULL,NULL);
INSERT INTO "functions" VALUES(14,'0x00020bbc','0x00413380','Surface_GetTriangleHeight','FUN_00413380',1,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/physics/getsurf.c','Computes average elevation `(y0 + y1 + y2) / -3` using vertex buffer indices from triangle.',NULL,NULL);
INSERT INTO "functions" VALUES(15,NULL,'0x004133d0','Font_DrawText','FUN_004133d0',3,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/formats/lft.c','2D bitmap font rasterizer blitting characters to 8bpp buffer.',NULL,NULL);
INSERT INTO "functions" VALUES(16,NULL,'0x00456270','Font_Load','FUN_00456270',3,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/formats/lft.c','Loads and parses `.LFT` font header, offset tables, widths, and glyph raster data.',NULL,NULL);
INSERT INTO "functions" VALUES(17,NULL,'0x00412580','Cdp_OpenFile','FUN_00412580',2,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Validates `"CDP\0"` header, dimensions, frame count, and embedded palette.',NULL,NULL);
INSERT INTO "functions" VALUES(18,NULL,'0x00412610','Cdp_DecodeFrame','FUN_00412610',2,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Advances animation stream and triggers inter-frame delta decompression.',NULL,NULL);
INSERT INTO "functions" VALUES(19,NULL,'0x00499abc','Cdp_DecompressRLE','FUN_00499abc',2,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Delta-skip RLE decompression modifying active frame buffer with opcode skip codes.',NULL,NULL);
INSERT INTO "functions" VALUES(20,NULL,'0x004029a0','Menu_Init','FUN_004029a0',5,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Loads `MENU.COL`, `MENU.TAB`, `.LFT` fonts, and initializes menu options.',NULL,NULL);
INSERT INTO "functions" VALUES(21,NULL,'0x00402c00','Menu_Tick','FUN_00402c00',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/core/game_state.c','Handles menu input navigation (arrows, Enter, Esc), item highlight, and transitions.',NULL,NULL);
INSERT INTO "functions" VALUES(22,NULL,'0x00414e40','Track_LoadSplines','FUN_00414e40',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/formats/tri.c','Loads `.TRI` chunk indices, constructs left/right road boundary splines and AI waypoints.',NULL,NULL);
INSERT INTO "functions" VALUES(23,NULL,'0x0041b360','Track_PreprocessPlacements','FUN_0041b360',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/formats/plc.c','Unpacks `model_type` bitfields (`& 0xFFF`) and extracts animation and flag channels.',NULL,NULL);
INSERT INTO "functions" VALUES(24,NULL,'0x004134e0','AI_FollowTrackSplines','FUN_004134e0',5,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Steering simulation updating car heading, track chunk position, distance to centerline.',NULL,NULL);
INSERT INTO "functions" VALUES(25,NULL,'0x00429a40','Race_CheckCheckpointTriggers','FUN_00429a40',5,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Tests vehicle collision against type 150..154 split-time checkpoint gates.',NULL,NULL);
INSERT INTO "functions" VALUES(26,NULL,'0x004356d0','Pos_InitAnimatedObjects','FUN_004356d0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/formats/pos.c','Converts keyframe coordinates in `.POS` to relative displacement deltas.',NULL,NULL);
INSERT INTO "functions" VALUES(27,NULL,'0x004357a0','Pos_UpdateAnimatedObjects','FUN_004357a0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/formats/pos.c','Advances keyframe playheads and translates moving scenery objects via `Lisa_MoveObject`.',NULL,NULL);
INSERT INTO "functions" VALUES(28,NULL,'0x00418dd0','Track_LoadAllAssets','FUN_00418dd0',5,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Master track loader: loads .COL, .PAN, .PIC, .SHD, .TAB, .MSH, .TEX, .POS.',NULL,NULL);
INSERT INTO "functions" VALUES(29,NULL,'0x00419a90','Track_LoadPlacements','FUN_00419a90',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/formats/plc.c','Loads `.PLC` scenery object placement tables for level and cars.',NULL,NULL);
INSERT INTO "functions" VALUES(30,NULL,'0x00419bd0','Mesh_LoadTrackAndCars','FUN_00419bd0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/formats/msh.c','Loads `LEVELS/<TRACK>/<TRACK>.MSH` and `CARS/CARS.MSH` into geometry memory.',NULL,NULL);
INSERT INTO "functions" VALUES(31,NULL,'0x00419d10','Texture_LoadAllPages','FUN_00419d10',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/formats/tex_tab.c','Loads 1MB track `.TEX`, car `.TEX`, and 64KB aligned sprite pages.',NULL,NULL);
INSERT INTO "functions" VALUES(32,NULL,'0x0041ac40','Font_LoadHUDFonts','FUN_0041ac40',3,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Loads HUD lettering glyphs (`IGNITION.FNT`, `yellow.lft`, `speed.lft`, etc.).',NULL,NULL);
INSERT INTO "functions" VALUES(33,NULL,'0x0041af70','Track_LoadOverlayGfx','FUN_0041af70',3,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Loads track sign textures and winner trophy bitmap (`POKAL.PIC`).',NULL,NULL);
INSERT INTO "functions" VALUES(34,NULL,'0x0041b470','Race_InitSceneAndCars','FUN_0041b470',5,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Instantiates player/AI cars on starting grid and binds scenery collision.',NULL,NULL);
INSERT INTO "functions" VALUES(35,NULL,'0x00416250','Mesh_InstantiatePlacedObjects','FUN_00416250',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/renderer/rasterizer.c','Applies `.PLC` world translation offsets to `.MSH` submesh vertices.',NULL,NULL);
INSERT INTO "functions" VALUES(36,NULL,'0x00436990','Race_RenderViewport','FUN_00436990',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXTENDED','src/renderer/camera.c','Calculates camera transform, invokes scene renderer, draws HUD.',NULL,NULL);
INSERT INTO "functions" VALUES(37,NULL,'0x0043e2a0','Lisa_Init','FUN_0043e2a0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/renderer/rasterizer.c','Initializes Lisa 2 rasterizer viewport, Z-buffer, and focal lengths.',NULL,NULL);
INSERT INTO "functions" VALUES(38,NULL,'0x004466d0','Lisa_RenderScene','FUN_004466d0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXTENDED','src/renderer/rasterizer.c','Master 3D frame render: culls objects, transforms vertices, rasterizes spans.',NULL,NULL);
INSERT INTO "functions" VALUES(39,NULL,'0x004468d0','Lisa_InitEngineMemory','FUN_004468d0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/renderer/rasterizer.c','Allocates internal rasterizer buffers, vertex streams, and matrices.',NULL,NULL);
INSERT INTO "functions" VALUES(40,NULL,'0x00448e70','Lisa_FrustumCullObjects','FUN_00448e70',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/renderer/rasterizer.c','Spatial grid frustum culler populating visible object list.',NULL,NULL);
INSERT INTO "functions" VALUES(41,NULL,'0x00449e70','Lisa_TransformVertices','FUN_00449e70',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/renderer/rasterizer.c','Camera matrix rotation, perspective projection, and backface culling.',NULL,NULL);
INSERT INTO "functions" VALUES(42,NULL,'0x0044b480','Lisa_InitOpcodeTable','FUN_0044b480',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/renderer/rasterizer.c','Binds polygon opcode rasterization dispatch table (`PTR_LAB_0049c8e0`).',NULL,NULL);
INSERT INTO "functions" VALUES(43,NULL,'0x0044c1f0','Lisa_RenderSubmeshes','FUN_0044c1f0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/renderer/rasterizer.c','Dispatches polygon opcodes across all visible transformed submeshes.',NULL,NULL);
INSERT INTO "functions" VALUES(44,NULL,'0x0044caa0','Lisa_DrawPolygon_Op12','LAB_0044caa0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','src/renderer/rasterizer.c','Opcode 0x12: 1-bit transparent cutout triangle (pushes `g_pLisaTransparencyLUT`).',NULL,NULL);
INSERT INTO "functions" VALUES(45,NULL,'0x0044cac0','Lisa_DrawPolygon_Op13','LAB_0044cac0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','src/renderer/rasterizer.c','Opcode 0x13: shadow / foliage alpha blend triangle (pushes `g_pActiveSHD`).',NULL,NULL);
INSERT INTO "functions" VALUES(46,NULL,'0x0044cae0','Lisa_DrawPolygon_Op16','LAB_0044cae0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','src/renderer/rasterizer.c','Opcode 0x16: transparent cutout variant.',NULL,NULL);
INSERT INTO "functions" VALUES(47,NULL,'0x0044cb00','Lisa_DrawPolygon_Op17','LAB_0044cb00',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','src/renderer/rasterizer.c','Opcode 0x17: shadow / alpha blend Gouraud triangle (pushes `g_pActiveSHD`).',NULL,NULL);
INSERT INTO "functions" VALUES(48,NULL,'0x0044cb20','Lisa_DrawTriangle_OpcodeHelper','FUN_0044cb20',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/renderer/rasterizer.c','Common backface test, attribute pack, and span bucketer for opcodes 0x12, 0x13, 0x16, 0x17.',NULL,NULL);
INSERT INTO "functions" VALUES(49,NULL,'0x0044d550','Lisa_DrawTexturedTriangle_Op15','FUN_0044d550',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/renderer/rasterizer.c','Opcode 0x15: perspective-correct textured triangle with 16.16 UV interpolation.',NULL,NULL);
INSERT INTO "functions" VALUES(50,NULL,'0x0044f0e9','Lisa_ExecuteRasterizerCommands','FUN_0044f0e9',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/renderer/rasterizer.c','Traverses depth-bucket sorted polygon command list and executes rasterizers.',NULL,NULL);
INSERT INTO "functions" VALUES(51,NULL,'0x00452800','Lisa_RenderTexturedTriangle_Op11','FUN_00452800',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','src/renderer/rasterizer.c','Opcode 0x11 triangle edge walker and span setup for unshaded texture mapping.',NULL,NULL);
INSERT INTO "functions" VALUES(52,NULL,'0x004537dc','Lisa_DrawTexturedSpan_Op11','FUN_004537dc',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','src/renderer/rasterizer.c','Low-level perspective/affine textured span blitter reading texels directly with stride 256 and alpha test.',NULL,NULL);
INSERT INTO "functions" VALUES(53,NULL,'0x0045b4f0','Lisa_PrintVersion','FUN_0045b4f0',2,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Prints "Lisa 2 Development System" banner and build timestamp.',NULL,NULL);
INSERT INTO "functions" VALUES(54,NULL,'0x0041f9b0','Sound_InitAndLoadPools','Sound_InitAndLoadPools',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/audio/audio.c','Initializes 32 DirectSound-compatible audio channels; loads SFX pools (ROLL, SKID, COLL, BOOST, DIV, KLICK, OK); loads track sounds; reads per-vehicle `ENGINE.INF` 800-byte curves into `uint8_t[200]` vol/pitch arrays at `+0x658`, `+0x720`, `+0x7e8`, `+0x8b0`.',NULL,NULL);
INSERT INTO "functions" VALUES(55,NULL,'0x00438210','Lisa_RenderPanorama','FUN_00438210',2,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Cylindrical horizon background blitter sampling 64KB `.PAN` texture using camera yaw and pitch angles.',NULL,NULL);
INSERT INTO "functions" VALUES(56,NULL,'0x00427d70','Car_UpdateAxleSpeeds','FUN_00427d70',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','src/physics/vehicle.c','Averages left and right wheel velocities for front and rear axles with factor 0.5.',NULL,NULL);
INSERT INTO "functions" VALUES(57,NULL,'0x00424570','Car_PhysicsTick','Car_PhysicsTick',1,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXTENDED','src/physics/vehicle.c','Master 72 Hz vehicle dynamics: 4-wheel independent raycast suspension, pitch/roll tilt, bicycle lateral slip, turbo boost.',NULL,NULL);
INSERT INTO "functions" VALUES(58,'0x00020c18','0x00446578','Surface_TestTrianglePositiveDZ','FUN_00446578',1,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','src/physics/getsurf.c','2D trapezoidal slope span test for table2 triangles ($dz \ge 0$).',NULL,NULL);
INSERT INTO "functions" VALUES(59,'0x00020c81','0x004465e1','Surface_TestTriangleNegativeDZ','FUN_004465e1',1,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','src/physics/getsurf.c','2D trapezoidal slope span test for table1 triangles ($dz < 0$).',NULL,NULL);
INSERT INTO "functions" VALUES(60,NULL,'0x0040e6b0','Car_IntegratePosition','FUN_0040e6b0',8,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','src/physics/vehicle.c','World coordinate velocity integrator with 21.76 scale factor and 72 Hz timestep.',NULL,NULL);
INSERT INTO "functions" VALUES(61,NULL,'0x00423aa0','Car_VerticalDynamics','FUN_00423aa0',8,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','src/physics/vehicle.c','Gravity acceleration (-0.2/tick), rebound bounce on impact, and ride height equilibrium (+5.0).',NULL,NULL);
INSERT INTO "functions" VALUES(62,NULL,'0x00442030','Car_ApplySteering','FUN_00442030',8,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','src/physics/vehicle.c','Speed-attenuated front wheel steering lock and smoothing filter.',NULL,NULL);
INSERT INTO "functions" VALUES(63,NULL,'0x00442670','Car_PowertrainUpdate','FUN_00442670',8,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','src/physics/vehicle.c','Engine propulsion, rolling and aerodynamic drag, transmission forward/reverse gear shifting.',NULL,NULL);
INSERT INTO "functions" VALUES(64,NULL,'0x0041d190','Car_UnpackMeshGeometry','FUN_0041d190',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXTENDED','src/renderer/rasterizer.c','Extracts `CARS.MSH` submesh vertices, finds bottom tire vertex $\max(v_y)$ for ground alignment, and scales by $21.76$.',NULL,NULL);
INSERT INTO "functions" VALUES(65,NULL,'0x00422680','Race_ResolveVehicleCollisions','FUN_00422680',5,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Inter-vehicle and scenery obstacle collision detection and impulse response.',NULL,NULL);
INSERT INTO "functions" VALUES(66,NULL,'0x004452c0','Sound_SynthesizeEngineRPM','FUN_004452c0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXTENDED','src/audio/engine_audio.c','Computes RPM pitch modulation from 800-byte ENGINE.INF curve with DEV-006 protection.',NULL,NULL);
INSERT INTO "functions" VALUES(67,NULL,'0x00458e00','Sound_LoadWAV','FUN_00458e00',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/audio/sound_pool.c','Loads RIFF/WAVE PCM 8-bit/16-bit audio file and converts to S16SYS format.',NULL,NULL);
INSERT INTO "functions" VALUES(68,NULL,'0x004582b0','Sound_LoadPAT','FUN_004582b0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/audio/sound_pool.c','Loads Gravis UltraSound GF1 .PAT patch audio files and converts 8-bit/16-bit linear PCM to S16SYS format.',NULL,NULL);
INSERT INTO "functions" VALUES(69,NULL,'0x00457890','Audio_MixCallback','FUN_00457890',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/audio/audio.c','32-channel software voice mixer with 16.16 fixed-point linear pitch resampling and stereo panning.',NULL,NULL);
INSERT INTO "functions" VALUES(70,NULL,'0x004579b0','Audio_PlayVoice','FUN_004579b0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/audio/audio.c','Allocates mixer channel voice, configures volume, pan, loop flag, and starts playback.',NULL,NULL);
INSERT INTO "functions" VALUES(71,NULL,'0x00457980','Audio_StopVoice','FUN_00457980',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/audio/audio.c','Immediately stops voice playback and releases mixer channel allocation.',NULL,NULL);
INSERT INTO "functions" VALUES(72,NULL,'0x00457aa0','Audio_SetVoiceParams','FUN_00457aa0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/audio/audio.c','Real-time modulation of voice pitch frequency and stereo pan position.',NULL,NULL);
INSERT INTO "functions" VALUES(73,NULL,'0x00457ed0','Music_PlayTrack','FUN_00457ed0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','src/audio/music.c','CD-DA track streamer using stb_vorbis mapped to circuits via DAT_00497eb8.',NULL,NULL);
INSERT INTO "functions" VALUES(74,NULL,'0x0043c910','Camera_UpdateChase','FUN_0043c910',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXTENDED','src/renderer/camera.c','Multi-mode chase camera with velocity lookahead, 0.125 azimuth lag, slope adaptation (DEV-004).',NULL,NULL);
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
INSERT INTO "globals" VALUES(1,NULL,'0x004c5398','g_hInstance',NULL,'HINSTANCE',NULL,'Application instance handle passed from `WinMain`.',NULL);
INSERT INTO "globals" VALUES(2,NULL,'0x004c5360','g_hCursor',NULL,'HCURSOR',NULL,'Default application mouse cursor.',NULL);
INSERT INTO "globals" VALUES(3,NULL,'0x00493720','g_HasPerfCounter',NULL,'uint32_t',NULL,'Flag: `1` if high-resolution timer (`QueryPerformanceCounter`) is available.',NULL);
INSERT INTO "globals" VALUES(4,NULL,'0x004c5348','g_PerfFrequency',NULL,'int64_t',NULL,'Performance counter frequency from `QueryPerformanceFrequency`.',NULL);
INSERT INTO "globals" VALUES(5,NULL,'0x004c5368','g_LastPerfCount',NULL,'int64_t',NULL,'Last measured performance counter timestamp.',NULL);
INSERT INTO "globals" VALUES(6,NULL,'0x00493734','g_GameStage',NULL,'int32_t',NULL,'Master state: `0` = Init, `1` = Active, `2` = Shutdown.',NULL);
INSERT INTO "globals" VALUES(7,NULL,'0x00639394','g_IntroState',NULL,'int32_t',NULL,'Intro sequence: `2` = Publisher logos, `1` = Init menus, `0` = Done.',NULL);
INSERT INTO "globals" VALUES(8,NULL,'0x00563c3c','g_MenuState',NULL,'int32_t',NULL,'Menu state: `1` when interactive menu is rendering.',NULL);
INSERT INTO "globals" VALUES(9,NULL,'0x00563d9c','g_MenuSelection',NULL,'int32_t',NULL,'Menu selection processed flag.',NULL);
INSERT INTO "globals" VALUES(10,NULL,'0x00553290','g_RaceLoadStage',NULL,'int32_t',NULL,'Track/race loading stage (`1`, `2`, `3`).',NULL);
INSERT INTO "globals" VALUES(11,NULL,'0x00525e5c','g_InRace',NULL,'int32_t',NULL,'In-race simulation flag: `1` during active driving.',NULL);
INSERT INTO "globals" VALUES(12,NULL,'0x004ba6e0','g_ScreenWidth',NULL,'int32_t',NULL,'Target screen resolution width (`640` or `320`).',NULL);
INSERT INTO "globals" VALUES(13,NULL,'0x004ba6e4','g_ScreenHeight',NULL,'int32_t',NULL,'Target screen resolution height (`480` or `200`).',NULL);
INSERT INTO "globals" VALUES(14,NULL,'0x004ba6e8','g_ColorDepth',NULL,'int32_t',NULL,'Screen bit depth (`8` bits per pixel).',NULL);
INSERT INTO "globals" VALUES(15,NULL,'0x0054f998','g_pSysGfxPic',NULL,'uint8_t*',NULL,'Pointer to loaded `N_SYSGFX.PIC` buffer.',NULL);
INSERT INTO "globals" VALUES(16,NULL,'0x00553088','g_pSysG2Pic',NULL,'uint8_t*',NULL,'Pointer to loaded `N_SYSG_2.PIC` buffer.',NULL);
INSERT INTO "globals" VALUES(17,NULL,'0x00563bfc','g_pSysCol',NULL,'uint8_t*',NULL,'Pointer to loaded `SYS.COL` buffer (`+ 8` is the 256-color palette).',NULL);
INSERT INTO "globals" VALUES(18,'0x000bfe80','0x004937bc','g_pActiveSRF',NULL,'uint8_t*',NULL,'Pointer to currently loaded `.SRF` surface buffer.',NULL);
INSERT INTO "globals" VALUES(19,'0x000ebab4','0x004c53b4','g_SRF_GridCellsX',NULL,'int32_t',NULL,'Active track surface grid dimension X.',NULL);
INSERT INTO "globals" VALUES(20,'0x000ebabc','0x004c53d0','g_SRF_GridCellsZ',NULL,'int32_t',NULL,'Active track surface grid dimension Z.',NULL);
INSERT INTO "globals" VALUES(21,'0x000eba90','0x004c53b8','g_SRF_CellSizeX',NULL,'int32_t',NULL,'Active track surface cell size X (512 units).',NULL);
INSERT INTO "globals" VALUES(22,'0x000eba94','0x004c53c8','g_SRF_CellSizeZ',NULL,'int32_t',NULL,'Active track surface cell size Z (512 units).',NULL);
INSERT INTO "globals" VALUES(23,NULL,'0x004c53a8','g_pSRF_Triangles',NULL,'uint8_t*',NULL,'Pointer to 24-byte surface collision triangle records in active `.SRF`.',NULL);
INSERT INTO "globals" VALUES(24,NULL,'0x00525e60','g_pActiveMSH',NULL,'uint8_t*',NULL,'Loaded `<TRACK>.MSH` geometry memory buffer.',NULL);
INSERT INTO "globals" VALUES(25,NULL,'0x0054f9cc','g_pActivePLC',NULL,'uint32_t*',NULL,'Loaded `<TRACK>.PLC` placed object table buffer.',NULL);
INSERT INTO "globals" VALUES(26,NULL,'0x00552fc8','g_pCarsPLC',NULL,'uint32_t*',NULL,'Loaded `CARS.PLC` placed car object buffer.',NULL);
INSERT INTO "globals" VALUES(27,NULL,'0x00563be4','g_pActiveTAB',NULL,'uint8_t*',NULL,'Loaded `<TRACK>.TAB` 64KB shading lookup matrix.',NULL);
INSERT INTO "globals" VALUES(28,NULL,'0x0049c9f8','g_pLisaActiveShading',NULL,'uint8_t*',NULL,'Active shading table pointer bound in Lisa3D rasterizer.',NULL);
INSERT INTO "globals" VALUES(29,NULL,'0x00639c0c','g_pActiveTRI',NULL,'uint8_t*',NULL,'Loaded `<TRACK>.TRI` 500-byte road chunk index buffer.',NULL);
INSERT INTO "globals" VALUES(30,NULL,'0x0063c5f0','g_LisaCamera',NULL,'LisaCamera*',NULL,'Lisa 3D camera state, matrices, and viewport parameters.',NULL);
INSERT INTO "globals" VALUES(31,NULL,'0x0049c8e0','g_LisaOpcodeTable',NULL,'void**',NULL,'Opcode function jump table (0x00..0x17).',NULL);
INSERT INTO "globals" VALUES(32,NULL,'0x0063c5cc','g_LisaVisibleObjects',NULL,'int**',NULL,'Array of pointers to visible objects from frustum culling.',NULL);
INSERT INTO "globals" VALUES(33,NULL,'0x0063c5fc','g_LisaVisibleSubmeshes',NULL,'int**',NULL,'Array of visible submeshes and projected screen vertices.',NULL);
INSERT INTO "globals" VALUES(34,NULL,'0x0063c5bc','g_LisaDrawCommands',NULL,'void**',NULL,'Depth-bucket sorted linked list of rasterizer polygon draw commands.',NULL);
INSERT INTO "globals" VALUES(35,NULL,'0x005530f0','g_pActiveTEX',NULL,'uint8_t*',NULL,'Loaded `<TRACK>.TEX` raw texture buffer (64KB aligned).',NULL);
INSERT INTO "globals" VALUES(36,NULL,'0x00553064','g_pActivePOS',NULL,'uint8_t*',NULL,'Loaded `<TRACK>.POS` scenery object keyframe animation buffer.',NULL);
INSERT INTO "globals" VALUES(37,NULL,'0x00552f60','g_pTrackRoadSequence',NULL,'int32_t*',NULL,'Ordered sequence of road chunks (24 bytes per chunk).',NULL);
INSERT INTO "globals" VALUES(38,NULL,'0x0063a01c','g_pTrackSplineNodes',NULL,'uint8_t*',NULL,'Array of 75-byte road spline nodes (left/right rails, center, heading).',NULL);
INSERT INTO "globals" VALUES(39,NULL,'0x0063a024','g_pTrackSplinePointers',NULL,'void**',NULL,'Pointers to active spline nodes indexed by chunk sequence.',NULL);
INSERT INTO "globals" VALUES(40,NULL,'0x0054f954','g_CheckpointCount',NULL,'int32_t',NULL,'Count of active type 150..154 split-time checkpoint trigger gates.',NULL);
INSERT INTO "globals" VALUES(41,NULL,'0x0054f904','g_pCheckpoints',NULL,'uint8_t*',NULL,'Checkpoint collision and trigger gate definitions.',NULL);
INSERT INTO "globals" VALUES(42,NULL,'0x0063b5f0','g_pActiveSHD',NULL,'uint8_t*',NULL,'Loaded `<TRACK>.SHD` 64KB shadow / alpha lookup table buffer.',NULL);
INSERT INTO "globals" VALUES(43,NULL,'0x0063b5e8','g_pLisaDepthBuckets',NULL,'void**',NULL,'6,000-entry depth bucket pointer array for polygon ordering.',NULL);
INSERT INTO "globals" VALUES(44,NULL,'0x0063c5b4','g_pLisaTransparencyLUT',NULL,'uint8_t*',NULL,'64KB color-key transparency lookup table (generated by `FUN_00402940`).',NULL);
INSERT INTO "globals" VALUES(45,NULL,'0x0063c5c8','g_pLisaTextureSheets',NULL,'uint8_t**',NULL,'Table of loaded texture base pointers (Track, Cars, Lights, Smoke).',NULL);
INSERT INTO "globals" VALUES(46,NULL,'0x004cdc28','g_pLisaActiveMipTable',NULL,'uint8_t**',NULL,'Pointer to active texture mip table bound during submesh dispatch.',NULL);
INSERT INTO "globals" VALUES(47,NULL,'0x004b63c4','g_pLisaActivePage',NULL,'uint8_t*',NULL,'Base pointer of active 64KB ($256 \times 256$) texture page in span rasterizer.',NULL);
INSERT INTO "globals" VALUES(48,NULL,'0x004792f0','g_PhysicsTimestep',NULL,'double',NULL,'72 Hz physics integration timestep ($1/72\text{ s} \approx 0.013888889\text{ s}$).',NULL);
INSERT INTO "globals" VALUES(49,NULL,'0x00479af0','g_PhysicsScaleFactor',NULL,'double',NULL,'World velocity integration multiplier (`21.76`).',NULL);
INSERT INTO "globals" VALUES(50,NULL,'0x004792c8','g_PhysicsGravity',NULL,'double',NULL,'Gravitational acceleration constant (`9.81` $m/s^2$).',NULL);
INSERT INTO "globals" VALUES(51,NULL,'0x00479b50','g_PhysicsGravityTick',NULL,'double',NULL,'Vertical downward velocity delta per tick (`0.2` units).',NULL);
INSERT INTO "globals" VALUES(52,NULL,'0x00479b58','g_PhysicsRideHeight',NULL,'double',NULL,'Ground clearance equilibrium offset (`5.0` units above surface).',NULL);
INSERT INTO "globals" VALUES(53,NULL,'0x00479ca0','g_PhysicsMaxSlopeSin',NULL,'double',NULL,'Arcsine clamping limit for pitch and roll calculation (`0.95`).',NULL);
INSERT INTO "globals" VALUES(54,NULL,'0x00479cb0','g_PhysicsRateLimitRad',NULL,'double',NULL,'Maximum pitch and roll angular change rate (`0.1` rad/tick).',NULL);
INSERT INTO "globals" VALUES(55,NULL,'0x00479c48','g_PhysicsWorldOffset',NULL,'double',NULL,'Spatial grid origin centering offset (`25600.0` units, $50 \times 512$).',NULL);
INSERT INTO "globals" VALUES(56,NULL,'0x00552fe0','g_pPlayerVehicle',NULL,'VehicleState*',NULL,'Pointer to active player vehicle state struct.',NULL);
INSERT INTO "globals" VALUES(57,NULL,'0x00553000','g_pAIVehicles',NULL,'VehicleState*',NULL,'Array of 5 opponent AI vehicle states.',NULL);
INSERT INTO "globals" VALUES(58,NULL,'0x00497eb8','g_TrackCDAudioMapping',NULL,'uint8_t[8]',NULL,'CD-DA audio track mapping index per circuit (Tracks 2..8).',NULL);
INSERT INTO "globals" VALUES(59,NULL,'0x00552f40','g_pAudioContext',NULL,'AudioContext*',NULL,'Master audio mixer device and state context.',NULL);
INSERT INTO "globals" VALUES(60,NULL,'0x00552f44','g_AudioChannels',NULL,'AudioVoice[32]',NULL,'32-channel software voice mixer table.',NULL);
INSERT INTO "globals" VALUES(61,NULL,'0x00552f48','g_MasterSoundVolume',NULL,'int32_t',NULL,'Master SFX attenuation level (0..128).',NULL);
INSERT INTO "globals" VALUES(62,NULL,'0x00552f4c','g_MasterMusicVolume',NULL,'int32_t',NULL,'Master CD-DA music attenuation level (0..128).',NULL);
CREATE TABLE metadata (
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL
);
INSERT INTO "metadata" VALUES('target_exe','MAINDOS.EXE');
INSERT INTO "metadata" VALUES('reference_exe','IGN_WIN.EXE');
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
INSERT INTO "struct_fields" VALUES(1,1,0,'int32_t','grid_cells_x',NULL,'Grid cells along X (200)');
INSERT INTO "struct_fields" VALUES(2,1,4,'int32_t','grid_cells_z',NULL,'Grid cells along Z (200)');
INSERT INTO "struct_fields" VALUES(3,1,8,'int32_t','cell_size_z',NULL,'World dimension of cell Z (512)');
INSERT INTO "struct_fields" VALUES(4,1,12,'int32_t','cell_size_x',NULL,'World dimension of cell X (512)');
INSERT INTO "struct_fields" VALUES(5,1,16,'int32_t','grid_stride_x',NULL,'Spatial index stride X (101)');
INSERT INTO "struct_fields" VALUES(6,1,20,'int32_t','grid_stride_z',NULL,'Spatial index stride Z (101)');
INSERT INTO "struct_fields" VALUES(7,1,24,'int32_t','triangle_count',NULL,'Total surface collision triangles');
INSERT INTO "struct_fields" VALUES(8,1,28,'int32_t','table1_count',NULL,'Primary index buffer integer count');
INSERT INTO "struct_fields" VALUES(9,1,32,'int32_t','table2_count',NULL,'Secondary index buffer integer count');
INSERT INTO "struct_fields" VALUES(10,2,0,'int32_t','table2_offset',NULL,'Byte offset into table2 (divide by 4)');
INSERT INTO "struct_fields" VALUES(11,2,4,'int32_t','table1_offset',NULL,'Byte offset into table1 (divide by 4)');
INSERT INTO "struct_fields" VALUES(12,2,8,'uint16_t','table1_count',NULL,'Number of triangles intersecting cell');
INSERT INTO "struct_fields" VALUES(13,2,10,'uint16_t','table2_count',NULL,'Number of secondary entities');
INSERT INTO "struct_fields" VALUES(14,3,0,'int32_t','x_base',NULL,'Base apex X coordinate');
INSERT INTO "struct_fields" VALUES(15,3,4,'int32_t','z_base',NULL,'Base apex Z coordinate');
INSERT INTO "struct_fields" VALUES(16,3,8,'int32_t','slope1',NULL,'16.16 fixed-point slope dx1/dz');
INSERT INTO "struct_fields" VALUES(17,3,12,'int32_t','slope2',NULL,'16.16 fixed-point slope dx2/dz');
INSERT INTO "struct_fields" VALUES(18,3,16,'int32_t','flags_and_dz',NULL,'Low 16 bits = dz (int16_t), High 16 bits = submesh polygon dword offset');
INSERT INTO "struct_fields" VALUES(19,3,20,'int32_t','v_ptr',NULL,'Byte offset into .PLC placed object array (obj_idx = v_ptr / 42)');
INSERT INTO "struct_fields" VALUES(20,4,0,'int32_t','submesh_offset',NULL,'Offset in 4-byte dwords into .MSH geometry');
INSERT INTO "struct_fields" VALUES(21,4,4,'int32_t','model_type',NULL,'Scenery / collision model archetype (e.g. 300, 2, 3)');
INSERT INTO "struct_fields" VALUES(22,4,8,'int32_t','pos_x',NULL,'World X position');
INSERT INTO "struct_fields" VALUES(23,4,12,'int32_t','pos_y',NULL,'World Y elevation');
INSERT INTO "struct_fields" VALUES(24,4,16,'int32_t','pos_z',NULL,'World Z position');
INSERT INTO "struct_fields" VALUES(25,5,0,'uint32_t','header',NULL,'Opcode in low byte (0x11, 0x12, 0x13, 0x15, 0x16, 0x17), flags in high 24 bits');
INSERT INTO "struct_fields" VALUES(26,5,4,'uint32_t','vi0',NULL,'Index of vertex 0 in submesh vertex buffer');
INSERT INTO "struct_fields" VALUES(27,5,8,'uint32_t','vi1',NULL,'Index of vertex 1 in submesh vertex buffer');
INSERT INTO "struct_fields" VALUES(28,5,12,'uint32_t','vi2',NULL,'Index of vertex 2 in submesh vertex buffer');
INSERT INTO "struct_fields" VALUES(29,5,16,'int32_t','tu0',NULL,'Vertex 0 U texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)');
INSERT INTO "struct_fields" VALUES(30,5,20,'int32_t','tv0',NULL,'Vertex 0 V texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)');
INSERT INTO "struct_fields" VALUES(31,5,24,'int32_t','tu1',NULL,'Vertex 1 U texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)');
INSERT INTO "struct_fields" VALUES(32,5,28,'int32_t','tv1',NULL,'Vertex 1 V texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)');
INSERT INTO "struct_fields" VALUES(33,5,32,'int32_t','tu2',NULL,'Vertex 2 U texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)');
INSERT INTO "struct_fields" VALUES(34,5,36,'int32_t','tv2',NULL,'Vertex 2 V texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)');
INSERT INTO "struct_fields" VALUES(35,5,40,'uint32_t','extra',NULL,'Texture page byte offset within .TEX file (page_index * 65536)');
INSERT INTO "struct_fields" VALUES(36,6,0,'double','engine_power',NULL,'Mass / power scaled by difficulty mode');
INSERT INTO "struct_fields" VALUES(37,6,8,'double','acceleration',NULL,'Forward traction acceleration');
INSERT INTO "struct_fields" VALUES(38,6,16,'double','top_speed',NULL,'Terminal velocity clamp');
INSERT INTO "struct_fields" VALUES(39,6,24,'double','steering_rate',NULL,'Turning responsiveness');
INSERT INTO "struct_fields" VALUES(40,6,32,'double','brake_force',NULL,'Deceleration coefficient');
INSERT INTO "struct_fields" VALUES(41,6,40,'double','turbo_boost',NULL,'Turbo propulsion multiplier');
INSERT INTO "struct_fields" VALUES(42,6,48,'double','suspension_k',NULL,'Spring rate');
INSERT INTO "struct_fields" VALUES(43,6,56,'double','damping_c',NULL,'Shock absorber damping');
INSERT INTO "struct_fields" VALUES(44,6,64,'double','collision_radius',NULL,'Spherical bounding volume');
INSERT INTO "struct_fields" VALUES(45,6,72,'int32_t','mass_integer',NULL,'Fixed-point mass value');
INSERT INTO "struct_fields" VALUES(46,6,96,'int32_t','wheel_fl_y',NULL,'Front-left wheel elevation');
INSERT INTO "struct_fields" VALUES(47,6,104,'int32_t','wheel_fr_y',NULL,'Front-right wheel elevation');
INSERT INTO "struct_fields" VALUES(48,6,112,'int32_t','wheel_rl_y',NULL,'Rear-left wheel elevation');
INSERT INTO "struct_fields" VALUES(49,6,120,'int32_t','wheel_rr_y',NULL,'Rear-right wheel elevation');
INSERT INTO "struct_fields" VALUES(50,7,0,'uint8_t','type',NULL,'Node type identifier');
INSERT INTO "struct_fields" VALUES(51,7,1,'uint8_t','code',NULL,'Road code passed from .TRI');
INSERT INTO "struct_fields" VALUES(52,7,2,'int32_t','left_x',NULL,'Left rail coordinate X (+ 0x6400)');
INSERT INTO "struct_fields" VALUES(53,7,6,'int32_t','left_y',NULL,'Left rail coordinate Y');
INSERT INTO "struct_fields" VALUES(54,7,10,'int32_t','left_z',NULL,'Left rail coordinate Z (+ 0x6400)');
INSERT INTO "struct_fields" VALUES(55,7,14,'int32_t','right_x',NULL,'Right rail coordinate X (+ 0x6400)');
INSERT INTO "struct_fields" VALUES(56,7,18,'int32_t','right_y',NULL,'Right rail coordinate Y');
INSERT INTO "struct_fields" VALUES(57,7,22,'int32_t','right_z',NULL,'Right rail coordinate Z (+ 0x6400)');
INSERT INTO "struct_fields" VALUES(58,7,42,'boundary padding
    float','heading',NULL,'Tangent heading angle in radians');
INSERT INTO "struct_fields" VALUES(59,7,74,'Secondary attributes
    uint8_t','fork_flag',NULL,'Branching directive (0 = mainline, 1 = left, 2 = right)');
INSERT INTO "struct_fields" VALUES(60,8,0,'uint8_t','road_code',NULL,'Surface material code');
INSERT INTO "struct_fields" VALUES(61,8,1,'int16_t','left_vertex',NULL,'Submesh vertex index for left boundary');
INSERT INTO "struct_fields" VALUES(62,8,23,'Secondary surface attributes
    int16_t','right_vertex',NULL,'Submesh vertex index for right boundary');
INSERT INTO "struct_fields" VALUES(63,8,109,'Internal friction parameters
    uint8_t','fork_flag',NULL,'Branching directive (0 = normal, 1 = left, 2 = right)');
INSERT INTO "struct_fields" VALUES(64,9,0,'int32_t','pos_x',NULL,'X position delta');
INSERT INTO "struct_fields" VALUES(65,9,4,'int32_t','pos_y',NULL,'Y elevation delta');
INSERT INTO "struct_fields" VALUES(66,9,8,'int32_t','pos_z',NULL,'Z position delta');
INSERT INTO "struct_fields" VALUES(67,9,12,'int32_t','rot_x',NULL,'Pitch angle (tenths of degree)');
INSERT INTO "struct_fields" VALUES(68,9,16,'int32_t','rot_y',NULL,'Yaw angle (tenths of degree)');
INSERT INTO "struct_fields" VALUES(69,9,20,'int32_t','rot_z',NULL,'Roll angle (tenths of degree)');
CREATE TABLE structs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT UNIQUE NOT NULL,             -- Struct name (e.g. 'SrfHeader')
    size INTEGER,                          -- Total size in bytes
    module_id INTEGER REFERENCES modules(id) ON DELETE SET NULL,
    description TEXT,
    notes TEXT
);
INSERT INTO "structs" VALUES(1,'SrfHeader',36,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(2,'SrfCell',12,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(3,'SrfTriangle',24,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(4,'PlcObject',20,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(5,'MshPolygon',44,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(6,'CarPhysicsState',124,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(7,'TrackSplineNode',75,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(8,'TriChunk',500,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(9,'PosKeyframe',24,NULL,NULL,NULL);
CREATE INDEX idx_functions_dos_addr ON functions(dos_address);
CREATE INDEX idx_functions_module ON functions(module_id);
CREATE INDEX idx_functions_status ON functions(status);
CREATE INDEX idx_globals_dos_addr ON globals(dos_address);
CREATE INDEX idx_globals_name ON globals(name);
CREATE INDEX idx_struct_fields_struct ON struct_fields(struct_id);
DELETE FROM "sqlite_sequence";
INSERT INTO "sqlite_sequence" VALUES('modules',8);
INSERT INTO "sqlite_sequence" VALUES('functions',74);
INSERT INTO "sqlite_sequence" VALUES('globals',62);
INSERT INTO "sqlite_sequence" VALUES('structs',9);
INSERT INTO "sqlite_sequence" VALUES('struct_fields',69);
COMMIT;
