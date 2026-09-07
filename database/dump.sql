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
INSERT INTO "functions" VALUES(75,NULL,'0x004029a0','Menu_Init','FUN_004029a0',5,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Loads MENU.COL, MENU.TAB, .LFT fonts, and initializes menu options.',NULL,NULL);
INSERT INTO "functions" VALUES(76,NULL,'0x00402c00','Menu_Tick','FUN_00402c00',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Handles menu input navigation (arrows, Enter, Esc), item highlight, and transitions.',NULL,NULL);
INSERT INTO "functions" VALUES(77,NULL,'0x0040e6b0','Car_IntegratePosition','FUN_0040e6b0',8,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','World coordinate velocity integrator with 21.76 scale factor and 72 Hz timestep.',NULL,NULL);
INSERT INTO "functions" VALUES(78,NULL,'0x004120a0','WinMain','FUN_004120a0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Main entry point; registers window class, queries timer, runs message/tick loop.',NULL,NULL);
INSERT INTO "functions" VALUES(79,NULL,'0x00412230','App_FrameTick','FUN_00412230',5,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Main engine tick; dispatches Init (0), Main Loop (1), and Shutdown (2).',NULL,NULL);
INSERT INTO "functions" VALUES(80,NULL,'0x00412500','App_Init','FUN_00412500',5,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Creates game window, initializes DirectDraw and DirectInput subsystems.',NULL,NULL);
INSERT INTO "functions" VALUES(81,NULL,'0x00412530','App_Shutdown','FUN_00412530',5,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Releases DirectDraw surfaces, DirectSound, and window handles.',NULL,NULL);
INSERT INTO "functions" VALUES(82,NULL,'0x00412580','Cdp_OpenFile','FUN_00412580',2,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Validates "CDP\0" header, dimensions, frame count, and embedded palette.',NULL,NULL);
INSERT INTO "functions" VALUES(83,NULL,'0x00412610','Cdp_DecodeFrame','FUN_00412610',2,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Advances animation stream and triggers inter-frame delta decompression.',NULL,NULL);
INSERT INTO "functions" VALUES(84,'0x0001fee0','0x00412670','Surface_LoadSRF','FUN_00412670',1,'matching','watcom_reg','int','(const char *filename, void *scene_objects)',330,65,'ADAPTED','-','Loads .SRF track collision surface, converts relative offsets to pointers',NULL,NULL);
INSERT INTO "functions" VALUES(85,'0x0002002c','0x004127a0','Surface_FreeSRF','FUN_004127a0',1,'matching','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Frees active .SRF surface memory buffer.',NULL,NULL);
INSERT INTO "functions" VALUES(86,'0x00020814','0x00412fc0','Surface_Raycast','FUN_00412fc0',1,'matching','watcom_reg','SurfaceRaycastResult*','(int qx, int qy, int qz)',932,135,'EXTENDED','-','Spatial grid query, candidate selection, cross product normal, world vertex transform',NULL,NULL);
INSERT INTO "functions" VALUES(87,'0x00020bbc','0x00413380','Surface_GetTriangleHeight','FUN_00413380',1,'matching','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Computes average elevation (y0 + y1 + y2) / -3 using vertex buffer indices from triangle.',NULL,NULL);
INSERT INTO "functions" VALUES(88,'0x00061959','0x004133d0','Font_DrawText','FUN_004133d0',3,'decompiled','watcom_reg','void',NULL,1359,NULL,'ADAPTED','-','2D bitmap font rasterizer blitting characters to 8bpp buffer.',NULL,NULL);
INSERT INTO "functions" VALUES(89,NULL,'0x004134e0','AI_FollowTrackSplines','FUN_004134e0',5,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Steering simulation updating car heading, track chunk position, distance to centerline.',NULL,NULL);
INSERT INTO "functions" VALUES(90,NULL,'0x00414e40','Track_LoadSplines','FUN_00414e40',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Loads .TRI chunk indices, constructs left/right road boundary splines and AI waypoints.',NULL,NULL);
INSERT INTO "functions" VALUES(91,NULL,'0x00416250','Mesh_InstantiatePlacedObjects','FUN_00416250',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Applies .PLC world translation offsets to .MSH submesh vertices.',NULL,NULL);
INSERT INTO "functions" VALUES(92,NULL,'0x00417270','Game_Init','FUN_00417270',5,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Sets initial game state flags, resets timers, initiates intro sequence.',NULL,NULL);
INSERT INTO "functions" VALUES(93,NULL,'0x004172b0','Game_StateDispatcher','FUN_004172b0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Top-level game loop state machine dispatcher (Intro -> Menus -> Race).',NULL,NULL);
INSERT INTO "functions" VALUES(94,'0x00021860','0x00418130','Load_SystemGraphicsAndFonts','FUN_00418130',3,'analyzed','watcom_reg','void',NULL,371,NULL,'-','-','Loads SYS.COL, N_SYSGFX.PIC, N_SYSG_2.PIC, and .LFT fonts.',NULL,NULL);
INSERT INTO "functions" VALUES(95,NULL,'0x00418dd0','Track_LoadAllAssets','FUN_00418dd0',5,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Master track loader: loads .COL, .PAN, .PIC, .SHD, .TAB, .MSH, .TEX, .POS.',NULL,NULL);
INSERT INTO "functions" VALUES(96,NULL,'0x00419a90','Track_LoadPlacements','FUN_00419a90',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Loads .PLC scenery object placement tables for level and cars.',NULL,NULL);
INSERT INTO "functions" VALUES(97,NULL,'0x00419bd0','Mesh_LoadTrackAndCars','FUN_00419bd0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Loads LEVELS/<TRACK>/<TRACK>.MSH and CARS/CARS.MSH into geometry memory.',NULL,NULL);
INSERT INTO "functions" VALUES(98,NULL,'0x00419d10','Texture_LoadAllPages','FUN_00419d10',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Loads 1MB track .TEX, car .TEX, and 64KB aligned sprite pages.',NULL,NULL);
INSERT INTO "functions" VALUES(99,'0x000240f4','0x0041ac40','Font_LoadHUDFonts','FUN_0041ac40',3,'analyzed','watcom_reg','void',NULL,746,NULL,'-','-','Loads HUD lettering glyphs (IGNITION.FNT, yellow.lft, speed.lft, etc.).',NULL,NULL);
INSERT INTO "functions" VALUES(100,'0x000243e0','0x0041af70','Track_LoadOverlayGfx','FUN_0041af70',3,'analyzed','watcom_reg','void',NULL,973,NULL,'-','-','Loads track sign textures and winner trophy bitmap (POKAL.PIC).',NULL,NULL);
INSERT INTO "functions" VALUES(101,NULL,'0x0041b360','Track_PreprocessPlacements','FUN_0041b360',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Unpacks model_type bitfields (& 0xFFF) and extracts animation and flag channels.',NULL,NULL);
INSERT INTO "functions" VALUES(102,NULL,'0x0041b470','Race_InitSceneAndCars','FUN_0041b470',5,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Instantiates player/AI cars on starting grid and binds scenery collision.',NULL,NULL);
INSERT INTO "functions" VALUES(103,NULL,'0x0041d190','Car_UnpackMeshGeometry','FUN_0041d190',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXTENDED','-','Extracts CARS.MSH submesh vertices, finds bottom tire vertex $\max(v_y)$ for ground alignment, and scales by $21.76$.',NULL,NULL);
INSERT INTO "functions" VALUES(104,NULL,'0x0041f9b0','Sound_InitAndLoadPools','Sound_InitAndLoadPools',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Initializes 32 DirectSound-compatible audio channels; loads SFX pools (ROLL, SKID, COLL, BOOST, DIV, KLICK, OK); loads track sounds; reads per-vehicle ENGINE.INF 800-byte curves into uint8_t[200] vol/pitch arrays at +0x658, +0x720, +0x7e8, +0x8b0.',NULL,NULL);
INSERT INTO "functions" VALUES(105,NULL,'0x00422680','Race_ResolveVehicleCollisions','FUN_00422680',5,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Inter-vehicle and scenery obstacle collision detection and impulse response.',NULL,NULL);
INSERT INTO "functions" VALUES(106,NULL,'0x00423aa0','Car_VerticalDynamics','FUN_00423aa0',8,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Gravity acceleration (-0.2/tick), rebound bounce on impact, and ride height equilibrium (+5.0).',NULL,NULL);
INSERT INTO "functions" VALUES(107,NULL,'0x00424570','Car_PhysicsTick','Car_PhysicsTick',8,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXTENDED','-','Master 72 Hz vehicle dynamics: 4-wheel independent raycast suspension, pitch/roll tilt, bicycle lateral slip, turbo boost.',NULL,NULL);
INSERT INTO "functions" VALUES(108,NULL,'0x00427d70','Car_UpdateAxleSpeeds','FUN_00427d70',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Averages left and right wheel velocities for front and rear axles with factor 0.5.',NULL,NULL);
INSERT INTO "functions" VALUES(109,NULL,'0x00429a40','Race_CheckCheckpointTriggers','FUN_00429a40',5,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Tests vehicle collision against type 150..154 split-time checkpoint gates.',NULL,NULL);
INSERT INTO "functions" VALUES(110,NULL,'0x004356d0','Pos_InitAnimatedObjects','FUN_004356d0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Converts keyframe coordinates in .POS to relative displacement deltas.',NULL,NULL);
INSERT INTO "functions" VALUES(111,NULL,'0x004357a0','Pos_UpdateAnimatedObjects','FUN_004357a0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Advances keyframe playheads and translates moving scenery objects via Lisa_MoveObject.',NULL,NULL);
INSERT INTO "functions" VALUES(112,NULL,'0x00436990','Race_RenderViewport','FUN_00436990',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXTENDED','-','Calculates camera transform, invokes scene renderer, draws HUD.',NULL,NULL);
INSERT INTO "functions" VALUES(113,NULL,'0x00438210','Lisa_RenderPanorama','FUN_00438210',2,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Cylindrical horizon background blitter sampling 64KB .PAN texture using camera yaw and pitch angles.',NULL,NULL);
INSERT INTO "functions" VALUES(114,NULL,'0x0043c910','Camera_UpdateChase','FUN_0043c910',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXTENDED','-','Multi-mode chase camera with velocity lookahead, 0.125 azimuth lag, slope adaptation (DEV-004).',NULL,NULL);
INSERT INTO "functions" VALUES(115,NULL,'0x0043e2a0','Lisa_Init','FUN_0043e2a0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Initializes Lisa 2 rasterizer viewport, Z-buffer, and focal lengths.',NULL,NULL);
INSERT INTO "functions" VALUES(116,NULL,'0x00442030','Car_ApplySteering','FUN_00442030',8,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Speed-attenuated front wheel steering lock and smoothing filter.',NULL,NULL);
INSERT INTO "functions" VALUES(117,NULL,'0x00442670','Car_PowertrainUpdate','FUN_00442670',8,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Engine propulsion, rolling and aerodynamic drag, transmission forward/reverse gear shifting.',NULL,NULL);
INSERT INTO "functions" VALUES(118,NULL,'0x004452c0','Sound_SynthesizeEngineRPM','FUN_004452c0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXTENDED','-','Computes RPM pitch modulation from 800-byte ENGINE.INF curve with DEV-006 protection.',NULL,NULL);
INSERT INTO "functions" VALUES(119,'0x00020c18','0x00446578','Surface_TestTrianglePositiveDZ','FUN_00446578',1,'matching','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','2D trapezoidal slope span test for table2 triangles ($dz \ge 0$).',NULL,NULL);
INSERT INTO "functions" VALUES(120,'0x00020c81','0x004465e1','Surface_TestTriangleNegativeDZ','FUN_004465e1',1,'matching','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','2D trapezoidal slope span test for table1 triangles ($dz < 0$).',NULL,NULL);
INSERT INTO "functions" VALUES(121,NULL,'0x004466d0','Lisa_RenderScene','FUN_004466d0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXTENDED','-','Master 3D frame render: culls objects, transforms vertices, rasterizes spans.',NULL,NULL);
INSERT INTO "functions" VALUES(122,NULL,'0x004468d0','Lisa_InitEngineMemory','FUN_004468d0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Allocates internal rasterizer buffers, vertex streams, and matrices.',NULL,NULL);
INSERT INTO "functions" VALUES(123,NULL,'0x00448e70','Lisa_FrustumCullObjects','FUN_00448e70',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Spatial grid frustum culler populating visible object list.',NULL,NULL);
INSERT INTO "functions" VALUES(124,NULL,'0x00449e70','Lisa_TransformVertices','FUN_00449e70',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Camera matrix rotation, perspective projection, and backface culling.',NULL,NULL);
INSERT INTO "functions" VALUES(125,NULL,'0x0044b480','Lisa_InitOpcodeTable','FUN_0044b480',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Binds polygon opcode rasterization dispatch table (PTR_LAB_0049c8e0).',NULL,NULL);
INSERT INTO "functions" VALUES(126,NULL,'0x0044c1f0','Lisa_RenderSubmeshes','FUN_0044c1f0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Dispatches polygon opcodes across all visible transformed submeshes.',NULL,NULL);
INSERT INTO "functions" VALUES(127,NULL,'0x0044caa0','Lisa_DrawPolygon_Op12','LAB_0044caa0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Opcode 0x12: 1-bit transparent cutout triangle (pushes g_pLisaTransparencyLUT).',NULL,NULL);
INSERT INTO "functions" VALUES(128,NULL,'0x0044cac0','Lisa_DrawPolygon_Op13','LAB_0044cac0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Opcode 0x13: shadow / foliage alpha blend triangle (pushes g_pActiveSHD).',NULL,NULL);
INSERT INTO "functions" VALUES(129,NULL,'0x0044cae0','Lisa_DrawPolygon_Op16','LAB_0044cae0',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Opcode 0x16: transparent cutout variant.',NULL,NULL);
INSERT INTO "functions" VALUES(130,NULL,'0x0044cb00','Lisa_DrawPolygon_Op17','LAB_0044cb00',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Opcode 0x17: shadow / alpha blend Gouraud triangle (pushes g_pActiveSHD).',NULL,NULL);
INSERT INTO "functions" VALUES(131,NULL,'0x0044cb20','Lisa_DrawTriangle_OpcodeHelper','FUN_0044cb20',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Common backface test, attribute pack, and span bucketer for opcodes 0x12, 0x13, 0x16, 0x17.',NULL,NULL);
INSERT INTO "functions" VALUES(132,NULL,'0x0044d550','Lisa_DrawTexturedTriangle_Op15','FUN_0044d550',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Opcode 0x15: perspective-correct textured triangle with 16.16 UV interpolation.',NULL,NULL);
INSERT INTO "functions" VALUES(133,NULL,'0x0044f0e9','Lisa_ExecuteRasterizerCommands','FUN_0044f0e9',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Traverses depth-bucket sorted polygon command list and executes rasterizers.',NULL,NULL);
INSERT INTO "functions" VALUES(134,NULL,'0x00452800','Lisa_RenderTexturedTriangle_Op11','FUN_00452800',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Opcode 0x11 triangle edge walker and span setup for unshaded texture mapping.',NULL,NULL);
INSERT INTO "functions" VALUES(135,NULL,'0x004537dc','Lisa_DrawTexturedSpan_Op11','FUN_004537dc',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'EXACT','-','Low-level perspective/affine textured span blitter reading texels directly with stride 256 and alpha test.',NULL,NULL);
INSERT INTO "functions" VALUES(136,'0x000615eb','0x00456270','Font_Load','FUN_00456270',3,'decompiled','watcom_reg','void',NULL,104,NULL,'ADAPTED','-','Loads and parses .LFT font header, offset tables, widths, and glyph raster data.',NULL,NULL);
INSERT INTO "functions" VALUES(137,NULL,'0x00456c40','Video_SetPalette','FUN_00456c40',2,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Uploads 256-color RGB palette to DirectDraw / hardware DAC.',NULL,NULL);
INSERT INTO "functions" VALUES(138,'0x00060f9c','0x004574a0','File_LoadToMemory','FUN_004574a0',4,'matching','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Generic binary loader (fopen, fread into allocated buffer).',NULL,NULL);
INSERT INTO "functions" VALUES(139,NULL,'0x00457890','Audio_MixCallback','FUN_00457890',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','32-channel software voice mixer with 16.16 fixed-point linear pitch resampling and stereo panning.',NULL,NULL);
INSERT INTO "functions" VALUES(140,NULL,'0x00457980','Audio_StopVoice','FUN_00457980',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Immediately stops voice playback and releases mixer channel allocation.',NULL,NULL);
INSERT INTO "functions" VALUES(141,NULL,'0x004579b0','Audio_PlayVoice','FUN_004579b0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Allocates mixer channel voice, configures volume, pan, loop flag, and starts playback.',NULL,NULL);
INSERT INTO "functions" VALUES(142,NULL,'0x00457aa0','Audio_SetVoiceParams','FUN_00457aa0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Real-time modulation of voice pitch frequency and stereo pan position.',NULL,NULL);
INSERT INTO "functions" VALUES(143,NULL,'0x00457ed0','Music_PlayTrack','FUN_00457ed0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','CD-DA track streamer using stb_vorbis mapped to circuits via DAT_00497eb8.',NULL,NULL);
INSERT INTO "functions" VALUES(144,NULL,'0x004582b0','Sound_LoadPAT','FUN_004582b0',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Loads Gravis UltraSound GF1 .PAT patch audio files and converts 8-bit/16-bit linear PCM to S16SYS format.',NULL,NULL);
INSERT INTO "functions" VALUES(145,NULL,'0x00458e00','Sound_LoadWAV','FUN_00458e00',5,'decompiled','watcom_reg','void',NULL,NULL,NULL,'ADAPTED','-','Loads RIFF/WAVE PCM 8-bit/16-bit audio file and converts to S16SYS format.',NULL,NULL);
INSERT INTO "functions" VALUES(146,NULL,'0x0045b4f0','Lisa_PrintVersion','FUN_0045b4f0',2,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Prints "Lisa 2 Development System" banner and build timestamp.',NULL,NULL);
INSERT INTO "functions" VALUES(147,NULL,'0x00469950','CRT_Entry','entry',7,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','C Runtime startup entry point, parses command line, calls WinMain.',NULL,NULL);
INSERT INTO "functions" VALUES(148,NULL,'0x00499abc','Cdp_DecompressRLE','FUN_00499abc',2,'analyzed','watcom_reg','void',NULL,NULL,NULL,'-','-','Delta-skip RLE decompression modifying active frame buffer with opcode skip codes.',NULL,NULL);
INSERT INTO "functions" VALUES(149,'0x00060f40',NULL,'File_ReadToBuffer',NULL,4,'matching','watcom_reg','void',NULL,NULL,NULL,'EXACT',NULL,'Reads binary file directly into preallocated buffer',NULL,NULL);
INSERT INTO "functions" VALUES(150,'0x00061100',NULL,'File_GetSize',NULL,4,'matching','watcom_reg','void',NULL,NULL,NULL,'EXACT',NULL,'Seeks to end and returns binary file size',NULL,NULL);
INSERT INTO "functions" VALUES(151,'0x0006117c',NULL,'File_Exists',NULL,4,'matching','watcom_reg','void',NULL,NULL,NULL,'EXACT',NULL,'Tests if file exists by attempting fopen',NULL,NULL);
INSERT INTO "functions" VALUES(152,'0x00061220',NULL,'Font_InitSystem','-',3,'decompiled','watcom_reg','int','(void)',249,NULL,'EXACT',NULL,'Initializes font subsystem tables (30 slots)',NULL,NULL);
INSERT INTO "functions" VALUES(153,'0x00061319',NULL,'Font_Shutdown','-',3,'decompiled','watcom_reg','int','(void)',128,NULL,'EXACT',NULL,'Unloads active fonts and shuts down font subsystem',NULL,NULL);
INSERT INTO "functions" VALUES(154,'0x00061399',NULL,'Font_Parse','-',3,'decompiled','watcom_reg','int','(void *buffer, int font_id)',594,NULL,'EXACT',NULL,'Parses LFT font header, initializes glyph handles and metrics',NULL,NULL);
INSERT INTO "functions" VALUES(155,'0x00061653',NULL,'Font_Unload','-',3,'decompiled','watcom_reg','int','(int font_id)',136,NULL,'EXACT',NULL,'Frees sprite handles for font glyphs and marks slot free',NULL,NULL);
INSERT INTO "functions" VALUES(156,'0x000616db',NULL,'Font_GetTextWidth','-',3,'decompiled','watcom_reg','int','(const char *text, int font_id)',638,NULL,'EXACT',NULL,'Calculates string rendering width in pixels',NULL,NULL);
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
INSERT INTO "globals" VALUES(63,NULL,'0x004c5398','g_hInstance',NULL,'HINSTANCE',NULL,'Application instance handle passed from `WinMain`.',NULL);
INSERT INTO "globals" VALUES(64,NULL,'0x004c5360','g_hCursor',NULL,'HCURSOR',NULL,'Default application mouse cursor.',NULL);
INSERT INTO "globals" VALUES(65,NULL,'0x00493720','g_HasPerfCounter',NULL,'uint32_t',NULL,'Flag: `1` if high-resolution timer (`QueryPerformanceCounter`) is available.',NULL);
INSERT INTO "globals" VALUES(66,NULL,'0x004c5348','g_PerfFrequency',NULL,'int64_t',NULL,'Performance counter frequency from `QueryPerformanceFrequency`.',NULL);
INSERT INTO "globals" VALUES(67,NULL,'0x004c5368','g_LastPerfCount',NULL,'int64_t',NULL,'Last measured performance counter timestamp.',NULL);
INSERT INTO "globals" VALUES(68,NULL,'0x00493734','g_GameStage',NULL,'int32_t',NULL,'Master state: `0` = Init, `1` = Active, `2` = Shutdown.',NULL);
INSERT INTO "globals" VALUES(69,NULL,'0x00639394','g_IntroState',NULL,'int32_t',NULL,'Intro sequence: `2` = Publisher logos, `1` = Init menus, `0` = Done.',NULL);
INSERT INTO "globals" VALUES(70,NULL,'0x00563c3c','g_MenuState',NULL,'int32_t',NULL,'Menu state: `1` when interactive menu is rendering.',NULL);
INSERT INTO "globals" VALUES(71,NULL,'0x00563d9c','g_MenuSelection',NULL,'int32_t',NULL,'Menu selection processed flag.',NULL);
INSERT INTO "globals" VALUES(72,NULL,'0x00553290','g_RaceLoadStage',NULL,'int32_t',NULL,'Track/race loading stage (`1`, `2`, `3`).',NULL);
INSERT INTO "globals" VALUES(73,NULL,'0x00525e5c','g_InRace',NULL,'int32_t',NULL,'In-race simulation flag: `1` during active driving.',NULL);
INSERT INTO "globals" VALUES(74,NULL,'0x004ba6e0','g_ScreenWidth',NULL,'int32_t',NULL,'Target screen resolution width (`640` or `320`).',NULL);
INSERT INTO "globals" VALUES(75,NULL,'0x004ba6e4','g_ScreenHeight',NULL,'int32_t',NULL,'Target screen resolution height (`480` or `200`).',NULL);
INSERT INTO "globals" VALUES(76,NULL,'0x004ba6e8','g_ColorDepth',NULL,'int32_t',NULL,'Screen bit depth (`8` bits per pixel).',NULL);
INSERT INTO "globals" VALUES(77,NULL,'0x0054f998','g_pSysGfxPic',NULL,'uint8_t*',NULL,'Pointer to loaded `N_SYSGFX.PIC` buffer.',NULL);
INSERT INTO "globals" VALUES(78,NULL,'0x00553088','g_pSysG2Pic',NULL,'uint8_t*',NULL,'Pointer to loaded `N_SYSG_2.PIC` buffer.',NULL);
INSERT INTO "globals" VALUES(79,NULL,'0x00563bfc','g_pSysCol',NULL,'uint8_t*',NULL,'Pointer to loaded `SYS.COL` buffer (`+ 8` is the 256-color palette).',NULL);
INSERT INTO "globals" VALUES(80,NULL,'0x004937bc','g_pActiveSRF',NULL,'uint8_t*',NULL,'Pointer to currently loaded `.SRF` surface buffer.',NULL);
INSERT INTO "globals" VALUES(81,NULL,'0x004c53b4','g_SRF_GridCellsX',NULL,'int32_t',NULL,'Active track surface grid dimension X.',NULL);
INSERT INTO "globals" VALUES(82,NULL,'0x004c53d0','g_SRF_GridCellsZ',NULL,'int32_t',NULL,'Active track surface grid dimension Z.',NULL);
INSERT INTO "globals" VALUES(83,NULL,'0x004c53b8','g_SRF_CellSizeX',NULL,'int32_t',NULL,'Active track surface cell size X (512 units).',NULL);
INSERT INTO "globals" VALUES(84,NULL,'0x004c53c8','g_SRF_CellSizeZ',NULL,'int32_t',NULL,'Active track surface cell size Z (512 units).',NULL);
INSERT INTO "globals" VALUES(85,NULL,'0x004c53a8','g_pSRF_Triangles',NULL,'uint8_t*',NULL,'Pointer to 24-byte surface collision triangle records in active `.SRF`.',NULL);
INSERT INTO "globals" VALUES(86,NULL,'0x00525e60','g_pActiveMSH',NULL,'uint8_t*',NULL,'Loaded `<TRACK>.MSH` geometry memory buffer.',NULL);
INSERT INTO "globals" VALUES(87,NULL,'0x0054f9cc','g_pActivePLC',NULL,'uint32_t*',NULL,'Loaded `<TRACK>.PLC` placed object table buffer.',NULL);
INSERT INTO "globals" VALUES(88,NULL,'0x00552fc8','g_pCarsPLC',NULL,'uint32_t*',NULL,'Loaded `CARS.PLC` placed car object buffer.',NULL);
INSERT INTO "globals" VALUES(89,NULL,'0x00563be4','g_pActiveTAB',NULL,'uint8_t*',NULL,'Loaded `<TRACK>.TAB` 64KB shading lookup matrix.',NULL);
INSERT INTO "globals" VALUES(90,NULL,'0x0049c9f8','g_pLisaActiveShading',NULL,'uint8_t*',NULL,'Active shading table pointer bound in Lisa3D rasterizer.',NULL);
INSERT INTO "globals" VALUES(91,NULL,'0x00639c0c','g_pActiveTRI',NULL,'uint8_t*',NULL,'Loaded `<TRACK>.TRI` 500-byte road chunk index buffer.',NULL);
INSERT INTO "globals" VALUES(92,NULL,'0x0063c5f0','g_LisaCamera',NULL,'LisaCamera*',NULL,'Lisa 3D camera state, matrices, and viewport parameters.',NULL);
INSERT INTO "globals" VALUES(93,NULL,'0x0049c8e0','g_LisaOpcodeTable',NULL,'void**',NULL,'Opcode function jump table (0x00..0x17).',NULL);
INSERT INTO "globals" VALUES(94,NULL,'0x0063c5cc','g_LisaVisibleObjects',NULL,'int**',NULL,'Array of pointers to visible objects from frustum culling.',NULL);
INSERT INTO "globals" VALUES(95,NULL,'0x0063c5fc','g_LisaVisibleSubmeshes',NULL,'int**',NULL,'Array of visible submeshes and projected screen vertices.',NULL);
INSERT INTO "globals" VALUES(96,NULL,'0x0063c5bc','g_LisaDrawCommands',NULL,'void**',NULL,'Depth-bucket sorted linked list of rasterizer polygon draw commands.',NULL);
INSERT INTO "globals" VALUES(97,NULL,'0x005530f0','g_pActiveTEX',NULL,'uint8_t*',NULL,'Loaded `<TRACK>.TEX` raw texture buffer (64KB aligned).',NULL);
INSERT INTO "globals" VALUES(98,NULL,'0x00553064','g_pActivePOS',NULL,'uint8_t*',NULL,'Loaded `<TRACK>.POS` scenery object keyframe animation buffer.',NULL);
INSERT INTO "globals" VALUES(99,NULL,'0x00552f60','g_pTrackRoadSequence',NULL,'int32_t*',NULL,'Ordered sequence of road chunks (24 bytes per chunk).',NULL);
INSERT INTO "globals" VALUES(100,NULL,'0x0063a01c','g_pTrackSplineNodes',NULL,'uint8_t*',NULL,'Array of 75-byte road spline nodes (left/right rails, center, heading).',NULL);
INSERT INTO "globals" VALUES(101,NULL,'0x0063a024','g_pTrackSplinePointers',NULL,'void**',NULL,'Pointers to active spline nodes indexed by chunk sequence.',NULL);
INSERT INTO "globals" VALUES(102,NULL,'0x0054f954','g_CheckpointCount',NULL,'int32_t',NULL,'Count of active type 150..154 split-time checkpoint trigger gates.',NULL);
INSERT INTO "globals" VALUES(103,NULL,'0x0054f904','g_pCheckpoints',NULL,'uint8_t*',NULL,'Checkpoint collision and trigger gate definitions.',NULL);
INSERT INTO "globals" VALUES(104,NULL,'0x0063b5f0','g_pActiveSHD',NULL,'uint8_t*',NULL,'Loaded `<TRACK>.SHD` 64KB shadow / alpha lookup table buffer.',NULL);
INSERT INTO "globals" VALUES(105,NULL,'0x0063b5e8','g_pLisaDepthBuckets',NULL,'void**',NULL,'6,000-entry depth bucket pointer array for polygon ordering.',NULL);
INSERT INTO "globals" VALUES(106,NULL,'0x0063c5b4','g_pLisaTransparencyLUT',NULL,'uint8_t*',NULL,'64KB color-key transparency lookup table (generated by `FUN_00402940`).',NULL);
INSERT INTO "globals" VALUES(107,NULL,'0x0063c5c8','g_pLisaTextureSheets',NULL,'uint8_t**',NULL,'Table of loaded texture base pointers (Track, Cars, Lights, Smoke).',NULL);
INSERT INTO "globals" VALUES(108,NULL,'0x004cdc28','g_pLisaActiveMipTable',NULL,'uint8_t**',NULL,'Pointer to active texture mip table bound during submesh dispatch.',NULL);
INSERT INTO "globals" VALUES(109,NULL,'0x004b63c4','g_pLisaActivePage',NULL,'uint8_t*',NULL,'Base pointer of active 64KB ($256 \times 256$) texture page in span rasterizer.',NULL);
INSERT INTO "globals" VALUES(110,NULL,'0x004792f0','g_PhysicsTimestep',NULL,'double',NULL,'72 Hz physics integration timestep ($1/72\text{ s} \approx 0.013888889\text{ s}$).',NULL);
INSERT INTO "globals" VALUES(111,NULL,'0x00479af0','g_PhysicsScaleFactor',NULL,'double',NULL,'World velocity integration multiplier (`21.76`).',NULL);
INSERT INTO "globals" VALUES(112,NULL,'0x004792c8','g_PhysicsGravity',NULL,'double',NULL,'Gravitational acceleration constant (`9.81` $m/s^2$).',NULL);
INSERT INTO "globals" VALUES(113,NULL,'0x00479b50','g_PhysicsGravityTick',NULL,'double',NULL,'Vertical downward velocity delta per tick (`0.2` units).',NULL);
INSERT INTO "globals" VALUES(114,NULL,'0x00479b58','g_PhysicsRideHeight',NULL,'double',NULL,'Ground clearance equilibrium offset (`5.0` units above surface).',NULL);
INSERT INTO "globals" VALUES(115,NULL,'0x00479ca0','g_PhysicsMaxSlopeSin',NULL,'double',NULL,'Arcsine clamping limit for pitch and roll calculation (`0.95`).',NULL);
INSERT INTO "globals" VALUES(116,NULL,'0x00479cb0','g_PhysicsRateLimitRad',NULL,'double',NULL,'Maximum pitch and roll angular change rate (`0.1` rad/tick).',NULL);
INSERT INTO "globals" VALUES(117,NULL,'0x00479c48','g_PhysicsWorldOffset',NULL,'double',NULL,'Spatial grid origin centering offset (`25600.0` units, $50 \times 512$).',NULL);
INSERT INTO "globals" VALUES(118,NULL,'0x00552fe0','g_pPlayerVehicle',NULL,'VehicleState*',NULL,'Pointer to active player vehicle state struct.',NULL);
INSERT INTO "globals" VALUES(119,NULL,'0x00553000','g_pAIVehicles',NULL,'VehicleState*',NULL,'Array of 5 opponent AI vehicle states.',NULL);
INSERT INTO "globals" VALUES(120,NULL,'0x00497eb8','g_TrackCDAudioMapping',NULL,'uint8_t[8]',NULL,'CD-DA audio track mapping index per circuit (Tracks 2..8).',NULL);
INSERT INTO "globals" VALUES(121,NULL,'0x00552f40','g_pAudioContext',NULL,'AudioContext*',NULL,'Master audio mixer device and state context.',NULL);
INSERT INTO "globals" VALUES(122,NULL,'0x00552f44','g_AudioChannels',NULL,'AudioVoice[32]',NULL,'32-channel software voice mixer table.',NULL);
INSERT INTO "globals" VALUES(123,NULL,'0x00552f48','g_MasterSoundVolume',NULL,'int32_t',NULL,'Master SFX attenuation level (0..128).',NULL);
INSERT INTO "globals" VALUES(124,NULL,'0x00552f4c','g_MasterMusicVolume',NULL,'int32_t',NULL,'Master CD-DA music attenuation level (0..128).',NULL);
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
INSERT INTO "struct_fields" VALUES(70,10,0,'int32_t','grid_cells_x',NULL,'Grid cells along X (200)');
INSERT INTO "struct_fields" VALUES(71,10,4,'int32_t','grid_cells_z',NULL,'Grid cells along Z (200)');
INSERT INTO "struct_fields" VALUES(72,10,8,'int32_t','cell_size_z',NULL,'World dimension of cell Z (512)');
INSERT INTO "struct_fields" VALUES(73,10,12,'int32_t','cell_size_x',NULL,'World dimension of cell X (512)');
INSERT INTO "struct_fields" VALUES(74,10,16,'int32_t','grid_stride_x',NULL,'Spatial index stride X (101)');
INSERT INTO "struct_fields" VALUES(75,10,20,'int32_t','grid_stride_z',NULL,'Spatial index stride Z (101)');
INSERT INTO "struct_fields" VALUES(76,10,24,'int32_t','triangle_count',NULL,'Total surface collision triangles');
INSERT INTO "struct_fields" VALUES(77,10,28,'int32_t','table1_count',NULL,'Primary index buffer integer count');
INSERT INTO "struct_fields" VALUES(78,10,32,'int32_t','table2_count',NULL,'Secondary index buffer integer count');
INSERT INTO "struct_fields" VALUES(79,11,0,'int32_t','table2_offset',NULL,'Byte offset into table2 (divide by 4)');
INSERT INTO "struct_fields" VALUES(80,11,4,'int32_t','table1_offset',NULL,'Byte offset into table1 (divide by 4)');
INSERT INTO "struct_fields" VALUES(81,11,8,'uint16_t','table1_count',NULL,'Number of triangles intersecting cell');
INSERT INTO "struct_fields" VALUES(82,11,10,'uint16_t','table2_count',NULL,'Number of secondary entities');
INSERT INTO "struct_fields" VALUES(83,12,0,'int32_t','x_base',NULL,'Base apex X coordinate');
INSERT INTO "struct_fields" VALUES(84,12,4,'int32_t','z_base',NULL,'Base apex Z coordinate');
INSERT INTO "struct_fields" VALUES(85,12,8,'int32_t','slope1',NULL,'16.16 fixed-point slope dx1/dz');
INSERT INTO "struct_fields" VALUES(86,12,12,'int32_t','slope2',NULL,'16.16 fixed-point slope dx2/dz');
INSERT INTO "struct_fields" VALUES(87,12,16,'int32_t','flags_and_dz',NULL,'Low 16 bits = dz (int16_t), High 16 bits = submesh polygon dword offset');
INSERT INTO "struct_fields" VALUES(88,12,20,'int32_t','v_ptr',NULL,'Byte offset into .PLC placed object array (obj_idx = v_ptr / 42)');
INSERT INTO "struct_fields" VALUES(89,13,0,'int32_t','submesh_offset',NULL,'Offset in 4-byte dwords into .MSH geometry');
INSERT INTO "struct_fields" VALUES(90,13,4,'int32_t','model_type',NULL,'Scenery / collision model archetype (e.g. 300, 2, 3)');
INSERT INTO "struct_fields" VALUES(91,13,8,'int32_t','pos_x',NULL,'World X position');
INSERT INTO "struct_fields" VALUES(92,13,12,'int32_t','pos_y',NULL,'World Y elevation');
INSERT INTO "struct_fields" VALUES(93,13,16,'int32_t','pos_z',NULL,'World Z position');
INSERT INTO "struct_fields" VALUES(94,14,0,'uint32_t','header',NULL,'Opcode in low byte (0x11, 0x12, 0x13, 0x15, 0x16, 0x17), flags in high 24 bits');
INSERT INTO "struct_fields" VALUES(95,14,4,'uint32_t','vi0',NULL,'Index of vertex 0 in submesh vertex buffer');
INSERT INTO "struct_fields" VALUES(96,14,8,'uint32_t','vi1',NULL,'Index of vertex 1 in submesh vertex buffer');
INSERT INTO "struct_fields" VALUES(97,14,12,'uint32_t','vi2',NULL,'Index of vertex 2 in submesh vertex buffer');
INSERT INTO "struct_fields" VALUES(98,14,16,'int32_t','tu0',NULL,'Vertex 0 U texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)');
INSERT INTO "struct_fields" VALUES(99,14,20,'int32_t','tv0',NULL,'Vertex 0 V texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)');
INSERT INTO "struct_fields" VALUES(100,14,24,'int32_t','tu1',NULL,'Vertex 1 U texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)');
INSERT INTO "struct_fields" VALUES(101,14,28,'int32_t','tv1',NULL,'Vertex 1 V texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)');
INSERT INTO "struct_fields" VALUES(102,14,32,'int32_t','tu2',NULL,'Vertex 2 U texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)');
INSERT INTO "struct_fields" VALUES(103,14,36,'int32_t','tv2',NULL,'Vertex 2 V texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)');
INSERT INTO "struct_fields" VALUES(104,14,40,'uint32_t','extra',NULL,'Texture page byte offset within .TEX file (page_index * 65536)');
INSERT INTO "struct_fields" VALUES(105,15,0,'double','engine_power',NULL,'Mass / power scaled by difficulty mode');
INSERT INTO "struct_fields" VALUES(106,15,8,'double','acceleration',NULL,'Forward traction acceleration');
INSERT INTO "struct_fields" VALUES(107,15,16,'double','top_speed',NULL,'Terminal velocity clamp');
INSERT INTO "struct_fields" VALUES(108,15,24,'double','steering_rate',NULL,'Turning responsiveness');
INSERT INTO "struct_fields" VALUES(109,15,32,'double','brake_force',NULL,'Deceleration coefficient');
INSERT INTO "struct_fields" VALUES(110,15,40,'double','turbo_boost',NULL,'Turbo propulsion multiplier');
INSERT INTO "struct_fields" VALUES(111,15,48,'double','suspension_k',NULL,'Spring rate');
INSERT INTO "struct_fields" VALUES(112,15,56,'double','damping_c',NULL,'Shock absorber damping');
INSERT INTO "struct_fields" VALUES(113,15,64,'double','collision_radius',NULL,'Spherical bounding volume');
INSERT INTO "struct_fields" VALUES(114,15,72,'int32_t','mass_integer',NULL,'Fixed-point mass value');
INSERT INTO "struct_fields" VALUES(115,15,96,'int32_t','wheel_fl_y',NULL,'Front-left wheel elevation');
INSERT INTO "struct_fields" VALUES(116,15,104,'int32_t','wheel_fr_y',NULL,'Front-right wheel elevation');
INSERT INTO "struct_fields" VALUES(117,15,112,'int32_t','wheel_rl_y',NULL,'Rear-left wheel elevation');
INSERT INTO "struct_fields" VALUES(118,15,120,'int32_t','wheel_rr_y',NULL,'Rear-right wheel elevation');
INSERT INTO "struct_fields" VALUES(119,16,0,'uint8_t','type',NULL,'Node type identifier');
INSERT INTO "struct_fields" VALUES(120,16,1,'uint8_t','code',NULL,'Road code passed from .TRI');
INSERT INTO "struct_fields" VALUES(121,16,2,'int32_t','left_x',NULL,'Left rail coordinate X (+ 0x6400)');
INSERT INTO "struct_fields" VALUES(122,16,6,'int32_t','left_y',NULL,'Left rail coordinate Y');
INSERT INTO "struct_fields" VALUES(123,16,10,'int32_t','left_z',NULL,'Left rail coordinate Z (+ 0x6400)');
INSERT INTO "struct_fields" VALUES(124,16,14,'int32_t','right_x',NULL,'Right rail coordinate X (+ 0x6400)');
INSERT INTO "struct_fields" VALUES(125,16,18,'int32_t','right_y',NULL,'Right rail coordinate Y');
INSERT INTO "struct_fields" VALUES(126,16,22,'int32_t','right_z',NULL,'Right rail coordinate Z (+ 0x6400)');
INSERT INTO "struct_fields" VALUES(127,16,42,'boundary padding
    float','heading',NULL,'Tangent heading angle in radians');
INSERT INTO "struct_fields" VALUES(128,16,74,'Secondary attributes
    uint8_t','fork_flag',NULL,'Branching directive (0 = mainline, 1 = left, 2 = right)');
INSERT INTO "struct_fields" VALUES(129,17,0,'uint8_t','road_code',NULL,'Surface material code');
INSERT INTO "struct_fields" VALUES(130,17,1,'int16_t','left_vertex',NULL,'Submesh vertex index for left boundary');
INSERT INTO "struct_fields" VALUES(131,17,23,'Secondary surface attributes
    int16_t','right_vertex',NULL,'Submesh vertex index for right boundary');
INSERT INTO "struct_fields" VALUES(132,17,109,'Internal friction parameters
    uint8_t','fork_flag',NULL,'Branching directive (0 = normal, 1 = left, 2 = right)');
INSERT INTO "struct_fields" VALUES(133,18,0,'int32_t','pos_x',NULL,'X position delta');
INSERT INTO "struct_fields" VALUES(134,18,4,'int32_t','pos_y',NULL,'Y elevation delta');
INSERT INTO "struct_fields" VALUES(135,18,8,'int32_t','pos_z',NULL,'Z position delta');
INSERT INTO "struct_fields" VALUES(136,18,12,'int32_t','rot_x',NULL,'Pitch angle (tenths of degree)');
INSERT INTO "struct_fields" VALUES(137,18,16,'int32_t','rot_y',NULL,'Yaw angle (tenths of degree)');
INSERT INTO "struct_fields" VALUES(138,18,20,'int32_t','rot_z',NULL,'Roll angle (tenths of degree)');
CREATE TABLE structs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT UNIQUE NOT NULL,             -- Struct name (e.g. 'SrfHeader')
    size INTEGER,                          -- Total size in bytes
    module_id INTEGER REFERENCES modules(id) ON DELETE SET NULL,
    description TEXT,
    notes TEXT
);
INSERT INTO "structs" VALUES(10,'SrfHeader',36,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(11,'SrfCell',12,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(12,'SrfTriangle',24,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(13,'PlcObject',20,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(14,'MshPolygon',44,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(15,'CarPhysicsState',124,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(16,'TrackSplineNode',75,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(17,'TriChunk',500,NULL,NULL,NULL);
INSERT INTO "structs" VALUES(18,'PosKeyframe',24,NULL,NULL,NULL);
CREATE INDEX idx_functions_dos_addr ON functions(dos_address);
CREATE INDEX idx_functions_module ON functions(module_id);
CREATE INDEX idx_functions_status ON functions(status);
CREATE INDEX idx_globals_dos_addr ON globals(dos_address);
CREATE INDEX idx_globals_name ON globals(name);
CREATE INDEX idx_struct_fields_struct ON struct_fields(struct_id);
DELETE FROM "sqlite_sequence";
INSERT INTO "sqlite_sequence" VALUES('modules',8);
INSERT INTO "sqlite_sequence" VALUES('functions',156);
INSERT INTO "sqlite_sequence" VALUES('globals',124);
INSERT INTO "sqlite_sequence" VALUES('structs',18);
INSERT INTO "sqlite_sequence" VALUES('struct_fields',138);
COMMIT;
