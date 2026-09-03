# Master Function Map (`IGN_WIN.EXE`)

This document tracks all identified functions in the original Windows 95 binary (`IGN_WIN.EXE`), their signatures, reconstructed names, corresponding source files from the DOS build (`MAINDOS.EXE`), and their source port implementation status.

| Address | Original Ghidra Name | Reconstructed Symbol | Source File Hint | Status | Purpose |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `0x00469950` | `entry` | `CRT_Entry` | MSVC CRT | Analyzed | C Runtime startup entry point, parses command line, calls `WinMain`. |
| `0x004120a0` | `FUN_004120a0` | `WinMain` | `main.c` | Ported | Main entry point; registers window class, queries timer, runs message/tick loop. |
| `0x00412230` | `FUN_00412230` | `App_FrameTick` | `main.c` | Analyzed | Main engine tick; dispatches Init (0), Main Loop (1), and Shutdown (2). |
| `0x00412500` | `FUN_00412500` | `App_Init` | `main.c` | Analyzed | Creates game window, initializes DirectDraw and DirectInput subsystems. |
| `0x00412530` | `FUN_00412530` | `App_Shutdown` | `main.c` | Analyzed | Releases DirectDraw surfaces, DirectSound, and window handles. |
| `0x00417270` | `FUN_00417270` | `Game_Init` | `main.c` | Analyzed | Sets initial game state flags, resets timers, initiates intro sequence. |
| `0x004172b0` | `FUN_004172b0` | `Game_StateDispatcher` | `main.c` | Analyzed | Heart of the state machine (Logos $\rightarrow$ Menus $\rightarrow$ Track Loading $\rightarrow$ In-Race). |
| `0x00418130` | `FUN_00418130` | `Load_SystemGraphicsAndFonts`| `geputget.c` | Analyzed | Loads `SYS.COL`, `N_SYSGFX.PIC`, `N_SYSG_2.PIC`, and `.LFT` fonts. |
| `0x004574a0` | `FUN_004574a0` | `File_LoadToMemory` | `mem.c` | Ported | Generic binary loader (`fopen`, `fread` into allocated buffer). |
| `0x00456c40` | `FUN_00456c40` | `Video_SetPalette` | `lisa3d.c` | Ported | Uploads 256-color RGB palette to DirectDraw / hardware DAC. |
| `0x00412670` | `FUN_00412670` | `Surface_LoadSRF` | `getsurf.c` | Analyzed | Loads `.SRF` track surface heightfield and collision grid. |
| `0x004127a0` | `FUN_004127a0` | `Surface_FreeSRF` | `getsurf.c` | Analyzed | Frees active `.SRF` surface memory buffer. |
| `0x00412fc0` | `FUN_00412fc0` | `Surface_GetCell` | `getsurf.c` | Analyzed | Computes grid cell index `(z/512)*w + (x/512)` from world coordinates. |
| `0x00413380` | `FUN_00413380` | `Surface_GetTriangleHeight` | `getsurf.c` | Analyzed | Computes average elevation `(y0 + y1 + y2) / 3` for a surface triangle. |
| `0x004133d0` | `FUN_004133d0` | `Font_DrawText` | `geputget.c` | Analyzed | 2D bitmap font rasterizer blitting characters to 8bpp buffer. |
| `0x00418b70` | `FUN_00418b70` | `Race_LoadVehicles` | `main.c` | Identified | Race loading stage 1: loads selected player and AI car models. |
| `0x00418be0` | `FUN_00418be0` | `Race_LoadTrackGeometry`| `main.c` | Identified | Race loading stage 2: loads track `.MSH`, `.TRI`, `.TEX`, `.TAB`. |
| `0x00418c40` | `FUN_00418c40` | `Race_LoadTrackPhysics` | `main.c` | Identified | Race loading stage 3: loads `.SRF` surface and `.POS` AI spline nodes. |

### Status Key:
* **Identified**: Function purpose recognized from cross-references and strings.
* **Analyzed**: Decompiled, parameters, local variables, and algorithms documented.
* **Ported**: Re-implemented and verified in the C11/SDL source port codebase.
