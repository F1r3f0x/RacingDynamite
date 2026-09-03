# Master Global Variable & Memory Map (`IGN_WIN.EXE`)

This table documents the global memory addresses in the data segments (`.data`, `.rdata`, `.bss`) of `IGN_WIN.EXE` (`0x00400000` base address), their types, purpose, and engine context.

| Address | Type | Name | Purpose |
| :--- | :--- | :--- | :--- |
| `0x004c5398` | `HINSTANCE` | `g_hInstance` | Application instance handle passed from `WinMain`. |
| `0x004c5360` | `HCURSOR` | `g_hCursor` | Default application mouse cursor. |
| `0x00493720` | `uint32_t` | `g_HasPerfCounter` | Flag: `1` if high-resolution timer (`QueryPerformanceCounter`) is available. |
| `0x004c5348` | `int64_t` | `g_PerfFrequency` | Performance counter frequency from `QueryPerformanceFrequency`. |
| `0x004c5368` | `int64_t` | `g_LastPerfCount` | Last measured performance counter timestamp. |
| `0x00493734` | `int32_t` | `g_GameStage` | Master state: `0` = Init, `1` = Active, `2` = Shutdown. |
| `0x00639394` | `int32_t` | `g_IntroState` | Intro sequence: `2` = Publisher logos, `1` = Init menus, `0` = Done. |
| `0x00563c3c` | `int32_t` | `g_MenuState` | Menu state: `1` when interactive menu is rendering. |
| `0x00563d9c` | `int32_t` | `g_MenuSelection`| Menu selection processed flag. |
| `0x00553290` | `int32_t` | `g_RaceLoadStage`| Track/race loading stage (`1`, `2`, `3`). |
| `0x00525e5c` | `int32_t` | `g_InRace` | In-race simulation flag: `1` during active driving. |
| `0x004ba6e0` | `int32_t` | `g_ScreenWidth` | Target screen resolution width (`640` or `320`). |
| `0x004ba6e4` | `int32_t` | `g_ScreenHeight`| Target screen resolution height (`480` or `200`). |
| `0x004ba6e8` | `int32_t` | `g_ColorDepth` | Screen bit depth (`8` bits per pixel). |
| `0x0054f998` | `uint8_t*` | `g_pSysGfxPic` | Pointer to loaded `N_SYSGFX.PIC` buffer. |
| `0x00553088` | `uint8_t*` | `g_pSysG2Pic` | Pointer to loaded `N_SYSG_2.PIC` buffer. |
| `0x00563bfc` | `uint8_t*` | `g_pSysCol` | Pointer to loaded `SYS.COL` buffer (`+ 8` is the 256-color palette). |
| `0x004937bc` | `uint8_t*` | `g_pActiveSRF` | Pointer to currently loaded `.SRF` surface buffer. |
| `0x004c53b4` | `int32_t` | `g_SRF_GridCellsX`| Active track surface grid dimension X. |
| `0x004c53d0` | `int32_t` | `g_SRF_GridCellsZ`| Active track surface grid dimension Z. |
| `0x004c53b8` | `int32_t` | `g_SRF_CellSizeX` | Active track surface cell size X (512 units). |
| `0x004c53c8` | `int32_t` | `g_SRF_CellSizeZ` | Active track surface cell size Z (512 units). |
| `0x004c53a8` | `uint8_t*` | `g_pSRF_Triangles`| Pointer to 24-byte surface collision triangle records in active `.SRF`. |
