/*
 * Racing Dynamite - Modern open-source source port of Ignition (1997)
 * Copyright (C) 2026 Patricio Labin Correa (@F1r3f0x)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef IGNITION_GAME_H
#define IGNITION_GAME_H

#include "ignition/types.h"
#include "ignition/formats.h"
#include "ignition/platform.h"
#include "ignition/renderer.h"
#include "ignition/physics.h"
#include "ignition/audio.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    GAME_STATE_INTRO,            // Publisher & Developer logos
    GAME_STATE_MAIN_MENU,        // Single Race, Championship, Time Attack, Options, Quit
    GAME_STATE_CAR_SELECT,       // Select vehicle (Cooper, Monster, Beetle, etc.)
    GAME_STATE_TRACK_SELECT,     // Select circuit (Austria to USA)
    GAME_STATE_RACE_LOADING,     // Loading models, physics, splines
    GAME_STATE_IN_RACE,          // 3D race simulation
    GAME_STATE_OPTIONS,          // Options menu (Gameplay, Gfx, Sound, Extras, Back)
    GAME_STATE_GAMEPLAY_OPTIONS, // Options > Gameplay (Game Fixes, Back)
    GAME_STATE_GAME_FIXES,       // Options > Gameplay > Game Fixes (Original Bug Fixes, Back)
    GAME_STATE_GFX_OPTIONS,      // Options > GFX Options (Backend, Res, Color, UV, Z-Buf, Pan, Presets)
    GAME_STATE_EXTRAS,           // Extras menu (Track Viz, About, Back)
    GAME_STATE_ABOUT,            // Project info & credits
    GAME_STATE_TRACK_VISUALIZER, // 3D real-time free-camera track inspector
    GAME_STATE_QUIT              // Exit application
} GameState;

typedef struct {

    GameState   current_state;
    uint32_t    state_timer;     // Milliseconds in current state
    int         menu_selection;  // Active menu option index (0..4)
    int         selected_car;    // Active vehicle index (0..CAR_ARCHETYPE_COUNT - 1)
    int         selected_track;  // Active circuit index (0..6)
    bool        is_visualizer_mode; // true when entering track select from Extras -> Track Visualizer
    GameFixOptions game_fixes;      // Options > Gameplay > Game Fixes: granular bug workarounds
    
    // Loaded system / menu assets
    Palette256  active_palette;
    TabData    *menu_tab;
    LftFont    *font_selected;   // Highlighted font
    LftFont    *font_unselected; // Normal font
    LftFont    *font_small;      // Small status font
    
    Image8bpp  *logo_img;        // IGN_LOGO.PIC
    Image8bpp  *bg_img;          // Title or track preview image
    
    uint8_t    *framebuffer;     // 640x480 virtual framebuffer
    
    // 3D Renderer Subsystem
    Renderer3D  renderer;
    Camera3D    camera;
    bool        auto_orbit;
    bool        show_textures;
    bool        show_waypoints;
    SrfData    *active_srf;
    PlcData    *active_plc;
    MshData    *active_msh;
    TexData    *active_tex;
    TabData    *active_tab;
    ShdData    *active_shd;
    PosData    *active_pos;
    TrackWaypoints *active_waypoints;
    
    // Vehicle Physics & Simulation Subsystem
    VehicleState    player_car;
    double          physics_accumulator;
    bool            chase_cam_mode;

    MshData        *cars_msh;
    TexData        *cars_tex;
    uint32_t        in_race_tick_count;
    uint32_t        last_telemetry_ms;

    // Audio Subsystem
    EngineAudio     player_engine_audio;
    GeneralSFX      general_sfx;
    bool            audio_initialized;
} GameContext;

/**
 * @brief Top-level game state machine initialization.
 * @original FUN_00417270 (MAINDOS_32BIT.EXE @ 0x00417270, main.c)
 * @fidelity ADAPTED
 */
bool Game_Init(GameContext *ctx);

/**
 * @brief Releases allocated game state memory and active assets.
 * @original FUN_00412530 (MAINDOS_32BIT.EXE @ 0x00412530, main.c)
 * @fidelity ADAPTED
 */
void Game_Shutdown(GameContext *ctx);

/**
 * @brief Top-level per-frame game state dispatcher and input handler.
 * @original FUN_004172b0 (MAINDOS_32BIT.EXE @ 0x004172b0, main.c)
 * @fidelity ADAPTED
 */
void Game_Update(GameContext *ctx, const PlatformInput *input, uint32_t delta_ms);

/**
 * @brief Master render dispatcher based on current game state.
 * @original FUN_00436990 (MAINDOS_32BIT.EXE @ 0x00436990, main.c)
 * @fidelity ADAPTED
 */
void Game_Render(GameContext *ctx);

#ifdef __cplusplus
}
#endif

#endif // IGNITION_GAME_H
