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

#include "ignition/game.h"
#include "ignition/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define NATIVE_WIDTH  640
#define NATIVE_HEIGHT 480

static const char *s_car_names[CAR_ARCHETYPE_COUNT] = {
    "COOP",
    "EVOR",
    "BUGGY",
    "ENFORCER",
    "RED-DEVIL",
    "SCHOOL BUS",
    "SMOKE",
    "BUG",
    "MONSTER",
    "VEGAS",
    "IGNITION"
};

static const char *s_car_folders[CAR_ARCHETYPE_COUNT] = {
    "COOPER",
    "PORSCHE",
    "JEEP",
    "COP",
    "MUSTANG",
    "SCHOOL",
    "VAN",
    "VW",
    "TRUCK",
    "DODGE",
    "NASCAR"
};


static const char *s_track_names[] = {
    "AUSTRIA",
    "BRAZIL",
    "CANADA",
    "CARIB",
    "ICELAND",
    "JAPAN",
    "USA"
};

static const char *s_main_menu_items[] = {
    "SINGLE RACE",
    "CHAMPIONSHIP",
    "TIME ATTACK",
    "OPTIONS",
    "QUIT"
};
#define MAIN_MENU_COUNT 5

static const char *s_options_menu_items[] = {
    "GAMEPLAY",
    "GFX OPTIONS",
    "SOUND OPTIONS",
    "EXTRAS",
    "BACK"
};
#define OPTIONS_MENU_COUNT 5

static const char *s_gameplay_menu_items[] = {
    "GAME FIXES",
    "BACK"
};
#define GAMEPLAY_MENU_COUNT 2

static const char *s_game_fixes_menu_items[] = {
    "COLLISION & NO-CLIP",
    "SURFACE ELEVATION",
    "CAMERA & VIEWPORT",
    "AI NAVIGATION",
    "AUDIO & SOUND",
    "RENDERER GLITCHES",
    "PRESET: 1997 AUTHENTIC",
    "PRESET: APPLY ALL FIXES",
    "BACK"
};
#define GAME_FIXES_MENU_COUNT 9

static const char *s_gfx_menu_items[] = {
    "RENDERER BACKEND",
    "RESOLUTION",
    "COLOR DEPTH",
    "UV PRECISION",
    "Z-BUFFERING",
    "HORIZON PANORAMAS",
    "PRESET: 1997 AUTHENTIC SOFTWARE",
    "PRESET: MODERN ENHANCED",
    "BACK"
};
#define GFX_MENU_COUNT 9

static const char *s_extras_menu_items[] = {
    "TRACK VISUALIZER",
    "ABOUT RACING DYNAMITE",
    "BACK"
};
#define EXTRAS_MENU_COUNT 3

static void BlitImageToFramebuffer(uint8_t *fb, const Image8bpp *img) {
    if (!fb || !img) return;

    memset(fb, 0, NATIVE_WIDTH * NATIVE_HEIGHT);
    uint32_t start_x = (img->width < NATIVE_WIDTH) ? (NATIVE_WIDTH - img->width) / 2 : 0;
    uint32_t start_y = (img->height < NATIVE_HEIGHT) ? (NATIVE_HEIGHT - img->height) / 2 : 0;
    uint32_t copy_w = (img->width < NATIVE_WIDTH) ? img->width : NATIVE_WIDTH;
    uint32_t copy_h = (img->height < NATIVE_HEIGHT) ? img->height : NATIVE_HEIGHT;

    for (uint32_t y = 0; y < copy_h; ++y) {
        memcpy(fb + (start_y + y) * NATIVE_WIDTH + start_x,
               img->pixels + y * img->width, copy_w);
    }
}

static void LoadTrackPreview(GameContext *ctx, int track_idx) {
    if (track_idx < 0 || track_idx >= 7) return;
    const char *name = s_track_names[track_idx];

    char pic_path[128], col_path[128];
    snprintf(pic_path, sizeof(pic_path), "assets/LEVELS/%s/%s.PIC", name, name);
    snprintf(col_path, sizeof(col_path), "assets/LEVELS/%s/%s.COL", name, name);

    if (ctx->bg_img) {
        Pic_Free(ctx->bg_img);
        ctx->bg_img = NULL;
    }

    ctx->bg_img = Pic_LoadFromFile(pic_path);
    if (ctx->bg_img) {
        BlitImageToFramebuffer(ctx->framebuffer, ctx->bg_img);
    }

    // Load track palette
    if (!Col_LoadFromFile(col_path, &ctx->active_palette)) {
        if (ctx->bg_img) {
            ctx->active_palette = ctx->bg_img->palette;
        }
    }
}

static void RestoreMenuBackground(GameContext *ctx) {
    if (ctx->bg_img) {
        Pic_Free(ctx->bg_img);
        ctx->bg_img = NULL;
    }

    ctx->bg_img = Pic_LoadFromFile("assets/INSTALL.PIC");
    if (ctx->bg_img) {
        ctx->active_palette = ctx->bg_img->palette;
        BlitImageToFramebuffer(ctx->framebuffer, ctx->bg_img);
    } else {
        Col_LoadFromFile("assets/BALTAZAR/DATA/MENU.COL", &ctx->active_palette);
    }
}

/**
 * @brief Top-level game state machine initialization.
 * @original FUN_00417270 (MAINDOS_32BIT.EXE @ 0x00417270, main.c)
 * @fidelity ADAPTED
 */
bool Game_Init(GameContext *ctx) {
    if (!ctx) return false;
    memset(ctx, 0, sizeof(GameContext));

    ctx->current_state = GAME_STATE_INTRO;
    ctx->framebuffer = (uint8_t *)calloc(NATIVE_WIDTH * NATIVE_HEIGHT, sizeof(uint8_t));
    if (!ctx->framebuffer) return false;

    // Load menu palette
    if (!Col_LoadFromFile("assets/BALTAZAR/DATA/MENU.COL", &ctx->active_palette)) {
        Col_LoadFromFile("assets/SYS.COL", &ctx->active_palette);
    }

    // Load authentic Ignition text font
    ctx->font_selected = Lft_LoadFromFile("assets/FONTS/IGNITION.FNT");
    if (!ctx->font_selected) {
        ctx->font_selected = Lft_LoadFromFile("assets/FONTS/SMALL.LFT");
    }
    ctx->font_unselected = ctx->font_selected;
    ctx->font_small = ctx->font_selected;

    // Load splash background
    ctx->bg_img = Pic_LoadFromFile("assets/INSTALL.PIC");
    if (ctx->bg_img) {
        BlitImageToFramebuffer(ctx->framebuffer, ctx->bg_img);
        ctx->active_palette = ctx->bg_img->palette;
    }

    // Initialize 3D renderer and camera
    if (!Renderer_Init(&ctx->renderer, NATIVE_WIDTH, NATIVE_HEIGHT)) {
        return false;
    }
    ctx->renderer.framebuffer = ctx->framebuffer;
    Camera_Init(&ctx->camera, (float)NATIVE_WIDTH / (float)NATIVE_HEIGHT);
    ctx->auto_orbit = true;
    ctx->show_textures = true;
    ctx->show_waypoints = false;
    // Bug workaround options: all disabled by default for 100% authentic 1997 recreation
    ctx->game_fixes.fix_noclip = false;
    ctx->game_fixes.fix_elevation = false;
    ctx->game_fixes.fix_camera = false;
    ctx->game_fixes.fix_ai_pathing = false;
    ctx->game_fixes.fix_audio = false;
    ctx->game_fixes.fix_renderer = false;
    ctx->chase_cam_mode = true;
    ctx->physics_accumulator = 0.0;

    // Load car 3D models and textures
    ctx->cars_msh = Msh_LoadFromFile("assets/CARS/CARS.MSH");
    ctx->cars_tex = Tex_LoadFromFile("assets/CARS/CARS.TEX");

    // Initialize audio subsystem and load general SFX pool
    ctx->audio_initialized = Audio_Init();
    if (ctx->audio_initialized) {
        SoundPool_LoadGeneralSFX(&ctx->general_sfx, "assets");
        Music_PlayTrack(2, true); // Play authentic title & menu music (Track02.ogg)
    }

    LOG_INFO("STATE", "State machine initialized. Starting in INTRO state.");
    return true;
}

/**
 * @brief Releases allocated game state memory and active assets.
 * @original FUN_00412530 (MAINDOS_32BIT.EXE @ 0x00412530, main.c)
 * @fidelity ADAPTED
 */
void Game_Shutdown(GameContext *ctx) {
    if (!ctx) return;
    if (ctx->audio_initialized) {
        EngineAudio_Free(&ctx->player_engine_audio);
        SoundPool_FreeGeneralSFX(&ctx->general_sfx);
        Audio_Shutdown();
        ctx->audio_initialized = false;
    }
    Renderer_Shutdown(&ctx->renderer);
    if (ctx->active_srf) Srf_Free(ctx->active_srf);
    if (ctx->active_plc) Plc_Free(ctx->active_plc);
    if (ctx->active_msh) Msh_Free(ctx->active_msh);
    if (ctx->active_tex) Tex_Free(ctx->active_tex);
    if (ctx->active_tab) Tab_Free(ctx->active_tab);
    if (ctx->active_shd) Shd_Free(ctx->active_shd);
    if (ctx->active_pos) Pos_Free(ctx->active_pos);
    if (ctx->cars_msh) Msh_Free(ctx->cars_msh);

    if (ctx->cars_tex) Tex_Free(ctx->cars_tex);
    if (ctx->active_waypoints) Track_FreeWaypoints(ctx->active_waypoints);
    if (ctx->font_selected) {
        LftFont *f = ctx->font_selected;
        if (ctx->font_unselected == f) ctx->font_unselected = NULL;
        if (ctx->font_small == f) ctx->font_small = NULL;
        Lft_Free(f);
        ctx->font_selected = NULL;
    }
    if (ctx->font_unselected) Lft_Free(ctx->font_unselected);
    if (ctx->font_small) Lft_Free(ctx->font_small);
    if (ctx->bg_img) Pic_Free(ctx->bg_img);
    if (ctx->logo_img) Pic_Free(ctx->logo_img);
    if (ctx->menu_tab) Tab_Free(ctx->menu_tab);
    if (ctx->framebuffer) free(ctx->framebuffer);
}

/**
 * @brief Top-level per-frame game state dispatcher and input handler.
 * @original FUN_004172b0 (MAINDOS_32BIT.EXE @ 0x004172b0, main.c)
 * @original FUN_00402c00 (MAINDOS_32BIT.EXE @ 0x00402c00, main.c)
 * @fidelity ADAPTED
 */
void Game_Update(GameContext *ctx, const PlatformInput *input, uint32_t delta_ms) {
    if (!ctx || !input) return;
    ctx->state_timer += delta_ms;

    // Play authentic UI sounds during menu navigation
    if (ctx->audio_initialized && ctx->current_state != GAME_STATE_IN_RACE && ctx->current_state != GAME_STATE_TRACK_VISUALIZER) {
        if (input->nav_up || input->nav_down || input->nav_left || input->nav_right) {
            Audio_PlaySFX(&ctx->general_sfx.ui_flip, 0.45f, 0.0f);
        } else if (input->nav_confirm) {
            Audio_PlaySFX(&ctx->general_sfx.ui_select, 0.65f, 0.0f);
        }
    }

    switch (ctx->current_state) {
        case GAME_STATE_INTRO:
            if (input->nav_confirm || input->key_enter) {
                ctx->current_state = GAME_STATE_MAIN_MENU;
                ctx->state_timer = 0;
                ctx->menu_selection = 0;
                LOG_INFO("STATE", "Intro -> Main Menu");
            }
            break;

        case GAME_STATE_MAIN_MENU:
            if (input->nav_up) {
                if (ctx->menu_selection > 0) ctx->menu_selection--;
                else ctx->menu_selection = MAIN_MENU_COUNT - 1;
            } else if (input->nav_down) {
                if (ctx->menu_selection < MAIN_MENU_COUNT - 1) ctx->menu_selection++;
                else ctx->menu_selection = 0;
            } else if (input->nav_confirm) {
                if (ctx->menu_selection == 0 || ctx->menu_selection == 1 || ctx->menu_selection == 2) {
                    ctx->is_visualizer_mode = false;
                    ctx->current_state = GAME_STATE_CAR_SELECT;
                    ctx->state_timer = 0;
                    LOG_INFO("STATE", "Main Menu -> Car Select");
                } else if (ctx->menu_selection == 3) {
                    ctx->current_state = GAME_STATE_OPTIONS;
                    ctx->menu_selection = 0;
                    ctx->state_timer = 0;
                    LOG_INFO("STATE", "Main Menu -> Options");
                } else if (ctx->menu_selection == 4) {
                    ctx->current_state = GAME_STATE_QUIT;
                    LOG_INFO("STATE", "Main Menu -> Quit");
                }
            } else if (input->nav_cancel) {
                ctx->current_state = GAME_STATE_QUIT;
            }
            break;

        case GAME_STATE_OPTIONS:
            if (input->nav_up) {
                if (ctx->menu_selection > 0) ctx->menu_selection--;
                else ctx->menu_selection = OPTIONS_MENU_COUNT - 1;
            } else if (input->nav_down) {
                if (ctx->menu_selection < OPTIONS_MENU_COUNT - 1) ctx->menu_selection++;
                else ctx->menu_selection = 0;
            } else if (input->nav_confirm) {
                if (ctx->menu_selection == 0) { // GAMEPLAY
                    ctx->current_state = GAME_STATE_GAMEPLAY_OPTIONS;
                    ctx->menu_selection = 0;
                    ctx->state_timer = 0;
                    LOG_INFO("STATE", "Options -> Gameplay");
                } else if (ctx->menu_selection == 1) { // GFX OPTIONS
                    ctx->current_state = GAME_STATE_GFX_OPTIONS;
                    ctx->menu_selection = 0;
                    ctx->state_timer = 0;
                    LOG_INFO("STATE", "Options -> GFX Options");
                } else if (ctx->menu_selection == 3) { // EXTRAS
                    ctx->current_state = GAME_STATE_EXTRAS;
                    ctx->menu_selection = 0;
                    ctx->state_timer = 0;
                    LOG_INFO("STATE", "Options -> Extras");
                } else if (ctx->menu_selection == 4) { // BACK
                    ctx->current_state = GAME_STATE_MAIN_MENU;
                    ctx->menu_selection = 3;
                    ctx->state_timer = 0;
                    LOG_INFO("STATE", "Options -> Main Menu");
                } else {
                    LOG_INFO("MENU", "%s not yet implemented.", s_options_menu_items[ctx->menu_selection]);
                }
            } else if (input->nav_cancel) {
                ctx->current_state = GAME_STATE_MAIN_MENU;
                ctx->menu_selection = 3;
                ctx->state_timer = 0;
                LOG_INFO("STATE", "Options -> Main Menu");
            }
            break;

        case GAME_STATE_GAMEPLAY_OPTIONS:
            if (input->nav_up) {
                if (ctx->menu_selection > 0) ctx->menu_selection--;
                else ctx->menu_selection = GAMEPLAY_MENU_COUNT - 1;
            } else if (input->nav_down) {
                if (ctx->menu_selection < GAMEPLAY_MENU_COUNT - 1) ctx->menu_selection++;
                else ctx->menu_selection = 0;
            } else if (input->nav_confirm) {
                if (ctx->menu_selection == 0) { // GAME FIXES -> Submenu
                    ctx->current_state = GAME_STATE_GAME_FIXES;
                    ctx->menu_selection = 0;
                    ctx->state_timer = 0;
                    LOG_INFO("STATE", "Gameplay -> Game Fixes");
                } else if (ctx->menu_selection == 1) { // BACK
                    ctx->current_state = GAME_STATE_OPTIONS;
                    ctx->menu_selection = 0;
                    ctx->state_timer = 0;
                    LOG_INFO("STATE", "Gameplay -> Options");
                }
            } else if (input->nav_cancel) {
                ctx->current_state = GAME_STATE_OPTIONS;
                ctx->menu_selection = 0;
                ctx->state_timer = 0;
                LOG_INFO("STATE", "Gameplay -> Options");
            }
            break;

        case GAME_STATE_GAME_FIXES:
            if (input->nav_up) {
                if (ctx->menu_selection > 0) ctx->menu_selection--;
                else ctx->menu_selection = GAME_FIXES_MENU_COUNT - 1;
            } else if (input->nav_down) {
                if (ctx->menu_selection < GAME_FIXES_MENU_COUNT - 1) ctx->menu_selection++;
                else ctx->menu_selection = 0;
            } else if (input->nav_left || input->nav_right || input->nav_confirm) {
                switch (ctx->menu_selection) {
                    case 0: // COLLISION & NO-CLIP
                        ctx->game_fixes.fix_noclip = !ctx->game_fixes.fix_noclip;
                        LOG_INFO("SETTINGS", "Fix No-Clip / Collision: %s", ctx->game_fixes.fix_noclip ? "FIXED" : "AUTHENTIC BUGGY");
                        break;
                    case 1: // SURFACE ELEVATION
                        ctx->game_fixes.fix_elevation = !ctx->game_fixes.fix_elevation;
                        LOG_INFO("SETTINGS", "Fix Surface Elevation: %s", ctx->game_fixes.fix_elevation ? "FIXED" : "AUTHENTIC BUGGY");
                        break;
                    case 2: // CAMERA & VIEWPORT
                        ctx->game_fixes.fix_camera = !ctx->game_fixes.fix_camera;
                        LOG_INFO("SETTINGS", "Fix Camera Basis: %s", ctx->game_fixes.fix_camera ? "FIXED" : "AUTHENTIC BUGGY");
                        break;
                    case 3: // AI NAVIGATION
                        ctx->game_fixes.fix_ai_pathing = !ctx->game_fixes.fix_ai_pathing;
                        LOG_INFO("SETTINGS", "Fix AI Navigation: %s", ctx->game_fixes.fix_ai_pathing ? "FIXED" : "AUTHENTIC BUGGY");
                        break;
                    case 4: // AUDIO & SOUND
                        ctx->game_fixes.fix_audio = !ctx->game_fixes.fix_audio;
                        LOG_INFO("SETTINGS", "Fix Audio Pitch: %s", ctx->game_fixes.fix_audio ? "FIXED" : "AUTHENTIC BUGGY");
                        break;
                    case 5: // RENDERER GLITCHES
                        ctx->game_fixes.fix_renderer = !ctx->game_fixes.fix_renderer;
                        LOG_INFO("SETTINGS", "Fix Renderer Glitches: %s", ctx->game_fixes.fix_renderer ? "FIXED" : "AUTHENTIC BUGGY");
                        break;
                    case 6: // PRESET: 1997 AUTHENTIC
                        ctx->game_fixes.fix_noclip = false;
                        ctx->game_fixes.fix_elevation = false;
                        ctx->game_fixes.fix_camera = false;
                        ctx->game_fixes.fix_ai_pathing = false;
                        ctx->game_fixes.fix_audio = false;
                        ctx->game_fixes.fix_renderer = false;
                        LOG_INFO("SETTINGS", "Preset Applied: 1997 AUTHENTIC (All fixes disabled)");
                        break;
                    case 7: // PRESET: APPLY ALL FIXES
                        ctx->game_fixes.fix_noclip = true;
                        ctx->game_fixes.fix_elevation = true;
                        ctx->game_fixes.fix_camera = true;
                        ctx->game_fixes.fix_ai_pathing = true;
                        ctx->game_fixes.fix_audio = true;
                        ctx->game_fixes.fix_renderer = true;
                        LOG_INFO("SETTINGS", "Preset Applied: APPLY ALL FIXES (All recommended fixes enabled)");
                        break;
                    case 8: // BACK
                        ctx->current_state = GAME_STATE_GAMEPLAY_OPTIONS;
                        ctx->menu_selection = 0;
                        ctx->state_timer = 0;
                        LOG_INFO("STATE", "Game Fixes -> Gameplay");
                        break;
                    default:
                        break;
                }
            } else if (input->nav_cancel) {
                ctx->current_state = GAME_STATE_GAMEPLAY_OPTIONS;
                ctx->menu_selection = 0;
                ctx->state_timer = 0;
                LOG_INFO("STATE", "Game Fixes -> Gameplay");
            }
            break;

        case GAME_STATE_GFX_OPTIONS:
            if (input->nav_up) {
                if (ctx->menu_selection > 0) ctx->menu_selection--;
                else ctx->menu_selection = GFX_MENU_COUNT - 1;
            } else if (input->nav_down) {
                if (ctx->menu_selection < GFX_MENU_COUNT - 1) ctx->menu_selection++;
                else ctx->menu_selection = 0;
            } else if (input->nav_left || input->nav_right || input->nav_confirm) {
                switch (ctx->menu_selection) {
                    case 0: { // RENDERER BACKEND
                        int next_backend = ((int)ctx->renderer.options.backend_type + 1) % 3;
                        if (input->nav_left) {
                            next_backend = ((int)ctx->renderer.options.backend_type + 2) % 3;
                        }
                        ctx->renderer.options.backend_type = (RendererBackendType)next_backend;
                        const char *b_str = (ctx->renderer.options.backend_type == RENDERER_BACKEND_LISA3D_SOFTWARE) ? "LISA3D SOFTWARE" :
                                            (ctx->renderer.options.backend_type == RENDERER_BACKEND_GLIDE_3DFX) ? "3DFX GLIDE" : "DIRECT3D";
                        LOG_INFO("GFX", "Renderer Backend: %s", b_str);
                        break;
                    }
                    case 1: { // RESOLUTION
                        if (ctx->renderer.options.internal_width == 640) {
                            ctx->renderer.options.internal_width = 320;
                            ctx->renderer.options.internal_height = 200;
                        } else {
                            ctx->renderer.options.internal_width = 640;
                            ctx->renderer.options.internal_height = 480;
                        }
                        LOG_INFO("GFX", "Internal Resolution: %dx%d", ctx->renderer.options.internal_width, ctx->renderer.options.internal_height);
                        break;
                    }
                    case 2: // COLOR DEPTH
                        ctx->renderer.options.force_8bit_paletted = !ctx->renderer.options.force_8bit_paletted;
                        LOG_INFO("GFX", "Color Depth: %s", ctx->renderer.options.force_8bit_paletted ? "8-BIT PALETTED" : "32-BIT TRUECOLOR");
                        break;
                    case 3: // UV PRECISION
                        ctx->renderer.options.authentic_fixed_point_uv = !ctx->renderer.options.authentic_fixed_point_uv;
                        LOG_INFO("GFX", "UV Stepping: %s", ctx->renderer.options.authentic_fixed_point_uv ? "1997 FIXED-POINT" : "ACCURATE 1/W");
                        break;
                    case 4: // Z-BUFFERING
                        ctx->renderer.options.authentic_depth_buckets = !ctx->renderer.options.authentic_depth_buckets;
                        LOG_INFO("GFX", "Depth Buffering: %s", ctx->renderer.options.authentic_depth_buckets ? "AUTHENTIC 6000 BUCKETS" : "PER-PIXEL 1/Z");
                        break;
                    case 5: // HORIZON PANORAMAS
                        ctx->renderer.options.render_panoramas = !ctx->renderer.options.render_panoramas;
                        LOG_INFO("GFX", "Horizon Panoramas: %s", ctx->renderer.options.render_panoramas ? "ENABLED" : "DISABLED");
                        break;
                    case 6: // PRESET: 1997 AUTHENTIC SOFTWARE
                        ctx->renderer.options.backend_type = RENDERER_BACKEND_LISA3D_SOFTWARE;
                        ctx->renderer.options.authentic_fixed_point_uv = true;
                        ctx->renderer.options.authentic_depth_buckets = true;
                        ctx->renderer.options.authentic_dithering = true;
                        ctx->renderer.options.force_8bit_paletted = true;
                        ctx->renderer.options.render_panoramas = true;
                        ctx->renderer.options.internal_width = 640;
                        ctx->renderer.options.internal_height = 480;
                        LOG_INFO("GFX", "Preset Applied: 1997 AUTHENTIC SOFTWARE");
                        break;
                    case 7: // PRESET: MODERN ENHANCED
                        ctx->renderer.options.backend_type = RENDERER_BACKEND_LISA3D_SOFTWARE;
                        ctx->renderer.options.authentic_fixed_point_uv = false;
                        ctx->renderer.options.authentic_depth_buckets = false;
                        ctx->renderer.options.authentic_dithering = false;
                        ctx->renderer.options.force_8bit_paletted = true;
                        ctx->renderer.options.render_panoramas = true;
                        ctx->renderer.options.internal_width = 640;
                        ctx->renderer.options.internal_height = 480;
                        LOG_INFO("GFX", "Preset Applied: MODERN ENHANCED");
                        break;
                    case 8: // BACK
                        ctx->current_state = GAME_STATE_OPTIONS;
                        ctx->menu_selection = 1;
                        ctx->state_timer = 0;
                        LOG_INFO("STATE", "GFX Options -> Options");
                        break;
                    default:
                        break;
                }
            } else if (input->nav_cancel) {
                ctx->current_state = GAME_STATE_OPTIONS;
                ctx->menu_selection = 1;
                ctx->state_timer = 0;
                LOG_INFO("STATE", "GFX Options -> Options");
            }
            break;

        case GAME_STATE_EXTRAS:
            if (input->nav_up) {
                if (ctx->menu_selection > 0) ctx->menu_selection--;
                else ctx->menu_selection = EXTRAS_MENU_COUNT - 1;
            } else if (input->nav_down) {
                if (ctx->menu_selection < EXTRAS_MENU_COUNT - 1) ctx->menu_selection++;
                else ctx->menu_selection = 0;
            } else if (input->nav_confirm) {
                if (ctx->menu_selection == 0) { // TRACK VISUALIZER
                    ctx->is_visualizer_mode = true;
                    ctx->current_state = GAME_STATE_TRACK_SELECT;
                    ctx->state_timer = 0;
                    LoadTrackPreview(ctx, ctx->selected_track);
                    LOG_INFO("STATE", "Extras -> Track Visualizer (Track Select)");
                } else if (ctx->menu_selection == 1) { // ABOUT RACING DYNAMITE
                    ctx->current_state = GAME_STATE_ABOUT;
                    ctx->state_timer = 0;
                    LOG_INFO("STATE", "Extras -> About");
                } else if (ctx->menu_selection == 2) { // BACK
                    ctx->current_state = GAME_STATE_OPTIONS;
                    ctx->menu_selection = 3;
                    ctx->state_timer = 0;
                    LOG_INFO("STATE", "Extras -> Options");
                }
            } else if (input->nav_cancel) {
                ctx->current_state = GAME_STATE_OPTIONS;
                ctx->menu_selection = 3;
                ctx->state_timer = 0;
                LOG_INFO("STATE", "Extras -> Options");
            }
            break;

        case GAME_STATE_ABOUT:
            if (input->nav_confirm || input->nav_cancel || input->key_enter) {
                ctx->current_state = GAME_STATE_EXTRAS;
                ctx->menu_selection = 1;
                ctx->state_timer = 0;
                LOG_INFO("STATE", "About -> Extras");
            }
            break;

        case GAME_STATE_CAR_SELECT:
            if (input->nav_left) {
                if (ctx->selected_car > 0) ctx->selected_car--;
                else ctx->selected_car = CAR_ARCHETYPE_COUNT - 1;
            } else if (input->nav_right) {
                if (ctx->selected_car < CAR_ARCHETYPE_COUNT - 1) ctx->selected_car++;
                else ctx->selected_car = 0;
            } else if (input->nav_confirm) {
                ctx->is_visualizer_mode = false;
                ctx->current_state = GAME_STATE_TRACK_SELECT;
                ctx->state_timer = 0;
                LoadTrackPreview(ctx, ctx->selected_track);
                LOG_INFO("STATE", "Car Selected: %s -> Track Select", s_car_names[ctx->selected_car]);
            } else if (input->nav_cancel) {
                ctx->current_state = GAME_STATE_MAIN_MENU;
                ctx->state_timer = 0;
                RestoreMenuBackground(ctx);
                LOG_INFO("STATE", "Car Select -> Main Menu");
            }
            break;

        case GAME_STATE_TRACK_SELECT:
            if (input->nav_left) {
                if (ctx->selected_track > 0) ctx->selected_track--;
                else ctx->selected_track = 6;
                LoadTrackPreview(ctx, ctx->selected_track);
            } else if (input->nav_right) {
                if (ctx->selected_track < 6) ctx->selected_track++;
                else ctx->selected_track = 0;
                LoadTrackPreview(ctx, ctx->selected_track);
            } else if (input->nav_confirm) {
                const char *name = s_track_names[ctx->selected_track];
                char srf_path[128], plc_path[128], msh_path[128], tex_path[128], tab_path[128], shd_path[128], col_path[128], pos_path[128];
                snprintf(srf_path, sizeof(srf_path), "assets/LEVELS/%s/%s.SRF", name, name);
                snprintf(plc_path, sizeof(plc_path), "assets/LEVELS/%s/%s.PLC", name, name);
                snprintf(msh_path, sizeof(msh_path), "assets/LEVELS/%s/%s.MSH", name, name);
                snprintf(tex_path, sizeof(tex_path), "assets/LEVELS/%s/%s.TEX", name, name);
                snprintf(tab_path, sizeof(tab_path), "assets/LEVELS/%s/%s.TAB", name, name);
                snprintf(shd_path, sizeof(shd_path), "assets/LEVELS/%s/%s.SHD", name, name);
                snprintf(col_path, sizeof(col_path), "assets/LEVELS/%s/%s.COL", name, name);
                snprintf(pos_path, sizeof(pos_path), "assets/LEVELS/%s/%s.POS", name, name);

                if (ctx->active_srf) Srf_Free(ctx->active_srf);
                if (ctx->active_plc) Plc_Free(ctx->active_plc);
                if (ctx->active_msh) Msh_Free(ctx->active_msh);
                if (ctx->active_tex) Tex_Free(ctx->active_tex);
                if (ctx->active_tab) Tab_Free(ctx->active_tab);
                if (ctx->active_shd) Shd_Free(ctx->active_shd);
                if (ctx->active_pos) Pos_Free(ctx->active_pos);
                if (ctx->active_waypoints) Track_FreeWaypoints(ctx->active_waypoints);

                ctx->active_srf = Srf_LoadFromFile(srf_path);
                ctx->active_plc = Plc_LoadFromFile(plc_path);
                ctx->active_msh = Msh_LoadFromFile(msh_path);
                ctx->active_tex = Tex_LoadFromFile(tex_path);
                ctx->active_tab = Tab_LoadFromFile(tab_path);
                ctx->active_shd = Shd_LoadFromFile(shd_path);
                ctx->active_pos = ctx->active_plc ? Pos_LoadFromFile(pos_path, ctx->active_plc->count) : NULL;
                Col_LoadFromFile(col_path, &ctx->active_palette);

                char trk_dir[128];
                snprintf(trk_dir, sizeof(trk_dir), "assets/LEVELS/%s", name);
                ctx->active_waypoints = Track_BuildWaypoints(trk_dir, name, ctx->active_plc, ctx->active_msh);


                Camera_Init(&ctx->camera, (float)NATIVE_WIDTH / (float)NATIVE_HEIGHT);
                ctx->camera.target.x = 0.0f;
                ctx->camera.target.y = -600.0f;
                ctx->camera.target.z = 0.0f;
                ctx->camera.distance = 7500.0f;
                ctx->camera.pitch = 35.0f;
                ctx->camera.yaw = 45.0f;
                ctx->auto_orbit = true;

                ctx->current_state = GAME_STATE_RACE_LOADING;
                ctx->state_timer = 0;
                LOG_INFO("STATE", "Track Selected: %s -> Loading 3D assets", s_track_names[ctx->selected_track]);
            } else if (input->nav_cancel) {
                if (ctx->is_visualizer_mode) {
                    ctx->current_state = GAME_STATE_EXTRAS;
                    ctx->menu_selection = 0;
                    ctx->state_timer = 0;
                    RestoreMenuBackground(ctx);
                    LOG_INFO("STATE", "Track Select -> Extras");
                } else {
                    ctx->current_state = GAME_STATE_CAR_SELECT;
                    ctx->state_timer = 0;
                    RestoreMenuBackground(ctx);
                    LOG_INFO("STATE", "Track Select -> Car Select");
                }
            }
            break;

        case GAME_STATE_RACE_LOADING:
            if (ctx->state_timer > 600) {
                if (ctx->is_visualizer_mode) {
                    ctx->current_state = GAME_STATE_TRACK_VISUALIZER;
                    ctx->state_timer = 0;
                    LOG_INFO("STATE", "3D Assets Loaded -> Entering Track Visualizer");
                } else {
                    ctx->current_state = GAME_STATE_IN_RACE;
                    ctx->state_timer = 0;
                    ctx->in_race_tick_count = 0;
                    ctx->last_telemetry_ms = 0;

                    // Initialize vehicle at starting waypoint
                    double start_x = 0.0, start_y = 0.0, start_z = 0.0, start_yaw = 0.0;
                    if (ctx->active_waypoints && ctx->active_waypoints->count > 0) {
                        const TrackWaypoint *wp0 = &ctx->active_waypoints->waypoints[0];
                        start_x = wp0->center_x;
                        start_y = wp0->center_y;
                        start_z = wp0->center_z;
                        if (ctx->active_waypoints->count > 1) {
                            const TrackWaypoint *wp1 = &ctx->active_waypoints->waypoints[1];
                            double dx = (double)(wp1->center_x - wp0->center_x);
                            double dz = (double)(wp1->center_z - wp0->center_z);
                            start_yaw = atan2(dx, dz);
                        }
                    }

                    // Query surface elevation directly below starting point
                    if (ctx->active_srf && ctx->active_plc && ctx->active_msh) {
                        SurfaceRaycastResult ground;
                        if (Surface_Raycast(ctx->active_srf, ctx->active_plc, ctx->active_msh,
                                            start_x, start_z, start_y, &ground, &ctx->game_fixes)) {
                            start_y = ground.elevation + 5.0;
                        }
                    }

                    Vehicle_Init(&ctx->player_car, ctx->selected_car, start_x, start_y, start_z, start_yaw);
                    ctx->chase_cam_mode = true;
                    ctx->physics_accumulator = 0.0;

                    // Configure authentic dynamic chase camera (Classic Isometric default)
                    Camera_SetMode(&ctx->camera, CAMERA_MODE_CLASSIC);
                    ctx->camera.yaw = (float)(start_yaw * (180.0 / M_PI)) + 180.0f;
                    Camera_UpdateFollowChase(&ctx->camera, &ctx->player_car, 0.0f, &ctx->game_fixes);

                    // Load vehicle engine acoustic curves and audio loops
                    char car_sound_dir[256];
                    snprintf(car_sound_dir, sizeof(car_sound_dir), "assets/CARS/%s/SOUND", s_car_folders[ctx->selected_car]);
                    EngineAudio_Init(&ctx->player_engine_audio, car_sound_dir);


                    // Stream authentic circuit CD-DA soundtrack track
                    int cd_track = Music_GetTrackForCircuit(ctx->selected_track);
                    Music_PlayTrack(cd_track, true);

                    LOG_INFO("RACE", "3D Assets Loaded -> Entering In-Race (Car: %s, Pos: (%.1f, %.1f, %.1f), Yaw: %.2f rad)",
                             s_car_names[ctx->selected_car], start_x, start_y, start_z, start_yaw);
                }
            }
            break;

        case GAME_STATE_TRACK_VISUALIZER:
            if (input->nav_cancel || input->key_menu) {
                ctx->current_state = GAME_STATE_EXTRAS;
                ctx->menu_selection = 0;
                ctx->state_timer = 0;
                RestoreMenuBackground(ctx);
                LOG_INFO("STATE", "Track Visualizer -> Extras");
            } else {
                if (input->nav_confirm) {
                    ctx->auto_orbit = !ctx->auto_orbit;
                }
                if (input->toggle_textures) {
                    ctx->show_textures = !ctx->show_textures;
                    LOG_INFO("RENDER", "Textures: %s", ctx->show_textures ? "ENABLED" : "DISABLED");
                }
                if (input->toggle_waypoints) {
                    ctx->show_waypoints = !ctx->show_waypoints;
                    LOG_INFO("RENDER", "AI Waypoints: %s", ctx->show_waypoints ? "ENABLED" : "DISABLED");
                }
                if (input->reset_camera) {
                    ctx->camera.target.x = 0.0f;
                    ctx->camera.target.y = -600.0f;
                    ctx->camera.target.z = 0.0f;
                    ctx->camera.yaw = 45.0f;
                    ctx->camera.pitch = 35.0f;
                    ctx->camera.distance = 7500.0f;
                    ctx->auto_orbit = false;
                    Camera_Update(&ctx->camera, 0.0f);
                    LOG_DEBUG("CAMERA", "Reset to track origin (auto-orbit paused)");
                }

                // Camera translation (WASD + Q/E or R/F)
                float move_speed = (float)delta_ms * 4.0f;
                float fwd = 0.0f, right = 0.0f, up = 0.0f;
                if (input->move_forward)  fwd += move_speed;
                if (input->move_backward) fwd -= move_speed;
                if (input->move_right)    right += move_speed;
                if (input->move_left)     right -= move_speed;
                if (input->move_up)       up += move_speed;
                if (input->move_down)     up -= move_speed;
                if (fwd != 0.0f || right != 0.0f || up != 0.0f) {
                    Camera_Pan(&ctx->camera, fwd, right, up);
                }

                // Camera orbit (Arrows) and zoom (Z/X)
                if (input->key_left)  Camera_Orbit(&ctx->camera, -1.8f, 0.0f, 0.0f);
                if (input->key_right) Camera_Orbit(&ctx->camera,  1.8f, 0.0f, 0.0f);
                if (input->key_up)    Camera_Orbit(&ctx->camera, 0.0f,  1.2f, 0.0f);
                if (input->key_down)  Camera_Orbit(&ctx->camera, 0.0f, -1.2f, 0.0f);
                if (input->key_accelerate) Camera_Orbit(&ctx->camera, 0.0f, 0.0f, -140.0f);
                if (input->key_brake)      Camera_Orbit(&ctx->camera, 0.0f, 0.0f,  140.0f);

                if (ctx->auto_orbit) {
                    Camera_Orbit(&ctx->camera, 0.35f, 0.0f, 0.0f);
                }

                // Animate dynamic scenery objects
                if (ctx->active_pos && ctx->active_plc) {
                    Pos_Update(ctx->active_pos, ctx->active_plc);
                }
            }
            break;


        case GAME_STATE_IN_RACE:
            if (input->nav_cancel || input->key_menu) {
                if (ctx->audio_initialized) {
                    EngineAudio_Free(&ctx->player_engine_audio);
                    if (ctx->general_sfx.roll_voice > 0) {
                        Audio_StopVoice(ctx->general_sfx.roll_voice);
                        ctx->general_sfx.roll_voice = 0;
                    }
                    if (ctx->general_sfx.skid_voice > 0) {
                        Audio_StopVoice(ctx->general_sfx.skid_voice);
                        ctx->general_sfx.skid_voice = 0;
                    }
                    Music_PlayTrack(2, true); // Revert to menu music Track02.ogg
                }
                ctx->current_state = GAME_STATE_MAIN_MENU;
                ctx->state_timer = 0;
                RestoreMenuBackground(ctx);
                LOG_INFO("STATE", "Race -> Main Menu");
            } else {
                if (input->toggle_textures) {
                    ctx->show_textures = !ctx->show_textures;
                    LOG_INFO("RENDER", "Textures: %s", ctx->show_textures ? "ENABLED" : "DISABLED");
                }
                if (input->toggle_waypoints) {
                    ctx->show_waypoints = !ctx->show_waypoints;
                    LOG_INFO("RENDER", "AI Waypoints: %s", ctx->show_waypoints ? "ENABLED" : "DISABLED");
                }
                if (input->cycle_camera) {
                    Camera_CycleMode(&ctx->camera);
                    ctx->chase_cam_mode = true;
                    LOG_INFO("CAMERA", "Cycled camera view mode to %d", ctx->camera.mode);
                }
                if (input->reset_camera) {
                    Camera_SetMode(&ctx->camera, CAMERA_MODE_CLASSIC);
                    ctx->camera.target.x = 0.0f;
                    ctx->camera.target.y = -600.0f;
                    ctx->camera.target.z = 0.0f;
                    ctx->camera.yaw = 45.0f;
                    ctx->camera.pitch = 35.0f;
                    ctx->camera.distance = 7500.0f;
                    ctx->auto_orbit = false;
                    ctx->chase_cam_mode = false;
                    Camera_Update(&ctx->camera, 0.0f);
                    LOG_DEBUG("CAMERA", "Reset to track origin (chase cam paused)");
                }

                // Camera translation (WASD + Q/E or R/F)
                float move_speed = (float)delta_ms * 4.0f;
                float fwd = 0.0f, right = 0.0f, up = 0.0f;
                if (input->move_forward)  fwd += move_speed;
                if (input->move_backward) fwd -= move_speed;
                if (input->move_right)    right += move_speed;
                if (input->move_left)     right -= move_speed;
                if (input->move_up)       up += move_speed;
                if (input->move_down)     up -= move_speed;
                if (fwd != 0.0f || right != 0.0f || up != 0.0f) {
                    ctx->chase_cam_mode = false;
                    Camera_Pan(&ctx->camera, fwd, right, up);
                }

                // Driving controls
                // In Lisa3D camera space, world +X projects to Screen Left, so steering left increases yaw (+1.0)
                double steer = 0.0;
                if (input->key_left)  steer += 1.0;
                if (input->key_right) steer -= 1.0;

                double throttle = 0.0;
                if (input->key_accelerate) throttle = 1.0;

                double brake = 0.0;
                if (input->key_brake) brake = 1.0;

                bool boost = input->key_turbo;

                if (throttle > 0.0 || brake > 0.0 || steer != 0.0 || boost || input->nav_confirm) {
                    ctx->chase_cam_mode = true;
                }

                Vehicle_ApplyInput(&ctx->player_car, throttle, brake, steer, boost);

                // 72 Hz fixed-timestep physics simulation
                ctx->physics_accumulator += (double)delta_ms / 1000.0;
                if (ctx->physics_accumulator > 0.2) {
                    ctx->physics_accumulator = 0.2;
                }

                while (ctx->physics_accumulator >= PHYSICS_DT_SEC) {
                    ctx->in_race_tick_count++;
                    Vehicle_Update(&ctx->player_car, ctx->active_srf, ctx->active_plc, ctx->active_msh,
                                   PHYSICS_DT_SEC, &ctx->game_fixes);
                    ctx->physics_accumulator -= PHYSICS_DT_SEC;
                }

                // Animate dynamic scenery objects
                if (ctx->active_pos && ctx->active_plc) {
                    Pos_Update(ctx->active_pos, ctx->active_plc);
                }


                // In-race periodic telemetry logging (every ~500 ms)
                if (ctx->state_timer - ctx->last_telemetry_ms >= 500) {
                    ctx->last_telemetry_ms = ctx->state_timer;
                    Vehicle_LogTelemetry(&ctx->player_car, ctx->in_race_tick_count);
                }

                // Update authentic dynamic follow chase camera
                if (ctx->chase_cam_mode) {
                    Camera_UpdateFollowChase(&ctx->camera, &ctx->player_car, (float)delta_ms / 1000.0f, &ctx->game_fixes);
                } else {
                    if (input->key_left && !throttle && !brake)  Camera_Orbit(&ctx->camera, -1.8f, 0.0f, 0.0f);
                    if (input->key_right && !throttle && !brake) Camera_Orbit(&ctx->camera,  1.8f, 0.0f, 0.0f);
                    if (input->key_up && !throttle && !brake)    Camera_Orbit(&ctx->camera, 0.0f,  1.2f, 0.0f);
                    if (input->key_down && !throttle && !brake)  Camera_Orbit(&ctx->camera, 0.0f, -1.2f, 0.0f);
                }

                // Real-time in-race audio updates
                if (ctx->audio_initialized) {
                    double speed_abs = fabs(ctx->player_car.speed_long);
                    EngineAudio_Update(&ctx->player_engine_audio, speed_abs, 45.0, ctx->player_car.airborne, &ctx->game_fixes);

                    // Turbo boost roar SFX
                    if (ctx->player_car.turbo_active && !ctx->player_car.prev_turbo_active) {
                        Audio_PlaySFX(&ctx->general_sfx.boost, 0.95f, 0.0f);
                    }
                    // Landing impact thud
                    if (!ctx->player_car.airborne && ctx->player_car.prev_airborne) {
                        Audio_PlaySFX(&ctx->general_sfx.coll_land, 0.85f, 0.0f);
                    }

                    // Tire skid squeal
                    double slip = fabs(ctx->player_car.speed_lat);
                    if (slip > 2.0 && !ctx->player_car.airborne) {
                        if (ctx->general_sfx.skid_voice <= 0) {
                            ctx->general_sfx.skid_voice = Audio_PlayVoice(&ctx->general_sfx.skid, 0.0f, 1.0f, 0.0f, true);
                        }
                        float skid_vol = (float)((slip - 2.0) / 6.0);
                        if (skid_vol > 1.0f) skid_vol = 1.0f;
                        Audio_SetVoiceParams(ctx->general_sfx.skid_voice, skid_vol * 0.85f, 1.0f, 0.0f);
                    } else if (ctx->general_sfx.skid_voice > 0) {
                        Audio_SetVoiceParams(ctx->general_sfx.skid_voice, 0.0f, 1.0f, 0.0f);
                    }

                    // Tire rolling rumble
                    if (!ctx->player_car.airborne && speed_abs > 1.0) {
                        if (ctx->general_sfx.roll_voice <= 0) {
                            ctx->general_sfx.roll_voice = Audio_PlayVoice(&ctx->general_sfx.roll, 0.0f, 1.0f, 0.0f, true);
                        }
                        float roll_vol = (float)(speed_abs / 45.0) * 0.35f;
                        if (roll_vol > 0.35f) roll_vol = 0.35f;
                        Audio_SetVoiceParams(ctx->general_sfx.roll_voice, roll_vol, 1.0f, 0.0f);
                    } else if (ctx->general_sfx.roll_voice > 0) {
                        Audio_SetVoiceParams(ctx->general_sfx.roll_voice, 0.0f, 1.0f, 0.0f);
                    }
                }
            }
            break;

        case GAME_STATE_QUIT:
            break;
    }
}

/**
 * @brief Master render dispatcher based on current game state.
 * @original FUN_00436990 (MAINDOS_32BIT.EXE @ 0x00436990, main.c)
 * @fidelity ADAPTED
 */
void Game_Render(GameContext *ctx) {
    if (!ctx || !ctx->framebuffer) return;

    if (ctx->current_state != GAME_STATE_IN_RACE && 
        ctx->current_state != GAME_STATE_TRACK_VISUALIZER && 
        ctx->bg_img) {
        BlitImageToFramebuffer(ctx->framebuffer, ctx->bg_img);
    }

    switch (ctx->current_state) {
        case GAME_STATE_INTRO: {
            // Flash prompt every 500ms
            if ((ctx->state_timer / 500) % 2 == 0) {
                const char *prompt = "PRESS ENTER OR SPACE TO START";
                int w = Font_GetTextWidth(ctx->font_selected, prompt);
                Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_selected,
                              (NATIVE_WIDTH - w) / 2, 420, prompt, 215); // Yellow text
            }
            break;
        }

        case GAME_STATE_MAIN_MENU: {
            int start_y = 260;
            for (int i = 0; i < MAIN_MENU_COUNT; ++i) {
                const char *item = s_main_menu_items[i];
                bool selected = (i == ctx->menu_selection);
                LftFont *f = ctx->font_selected;
                int w = Font_GetTextWidth(f, item);
                int x = (NATIVE_WIDTH - w) / 2;
                int y = start_y + i * 28;
                uint8_t color = selected ? 215 : 251; // Yellow if selected, White if normal

                if (selected) {
                    Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, f, x - 18, y, ">", 215);
                    Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, f, x + w + 8, y, "<", 215);
                }
                Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, f, x, y, item, color);
            }
            break;
        }

        case GAME_STATE_OPTIONS: {
            const char *hdr = "OPTIONS";
            int hw = Font_GetTextWidth(ctx->font_selected, hdr);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_selected, (NATIVE_WIDTH - hw) / 2, 195, hdr, 251);

            int start_y = 235;
            for (int i = 0; i < OPTIONS_MENU_COUNT; ++i) {
                const char *item = s_options_menu_items[i];
                bool selected = (i == ctx->menu_selection);
                LftFont *f = ctx->font_selected;
                int w = Font_GetTextWidth(f, item);
                int x = (NATIVE_WIDTH - w) / 2;
                int y = start_y + i * 32;
                uint8_t color = selected ? 215 : 251;

                if (selected) {
                    Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, f, x - 18, y, ">", 215);
                    Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, f, x + w + 8, y, "<", 215);
                }
                Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, f, x, y, item, color);
            }

            const char *hint = "ENTER: SELECT    ESC: BACK";
            int hint_w = Font_GetTextWidth(ctx->font_small, hint);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_small, (NATIVE_WIDTH - hint_w) / 2, 440, hint, 251);
            break;
        }

        case GAME_STATE_GAMEPLAY_OPTIONS: {
            const char *hdr = "GAMEPLAY OPTIONS";
            int hw = Font_GetTextWidth(ctx->font_selected, hdr);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_selected, (NATIVE_WIDTH - hw) / 2, 200, hdr, 251);

            int start_y = 250;
            for (int i = 0; i < GAMEPLAY_MENU_COUNT; ++i) {
                const char *item = s_gameplay_menu_items[i];
                bool selected = (i == ctx->menu_selection);
                LftFont *f = ctx->font_selected;
                int w = Font_GetTextWidth(f, item);
                int x = (NATIVE_WIDTH - w) / 2;
                int y = start_y + i * 36;
                uint8_t color = selected ? 215 : 251;

                if (selected) {
                    Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, f, x - 18, y, ">", 215);
                    Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, f, x + w + 8, y, "<", 215);
                }
                Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, f, x, y, item, color);
            }

            const char *hint = "ENTER: SELECT    ESC: BACK";
            int hint_w = Font_GetTextWidth(ctx->font_small, hint);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_small, (NATIVE_WIDTH - hint_w) / 2, 440, hint, 251);
            break;
        }

        case GAME_STATE_GAME_FIXES: {
            const char *hdr = "GAME FIXES";
            int hw = Font_GetTextWidth(ctx->font_selected, hdr);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_selected, (NATIVE_WIDTH - hw) / 2, 130, hdr, 251);

            int start_y = 165;
            for (int i = 0; i < GAME_FIXES_MENU_COUNT; ++i) {
                char buf[64];
                switch (i) {
                    case 0:
                        snprintf(buf, sizeof(buf), "NO-CLIP / COLLISION: %s", ctx->game_fixes.fix_noclip ? "[FIXED]" : "[AUTHENTIC BUGGY]");
                        break;
                    case 1:
                        snprintf(buf, sizeof(buf), "SURFACE ELEVATION:  %s", ctx->game_fixes.fix_elevation ? "[FIXED]" : "[AUTHENTIC BUGGY]");
                        break;
                    case 2:
                        snprintf(buf, sizeof(buf), "CAMERA & VIEWPORT:   %s", ctx->game_fixes.fix_camera ? "[FIXED]" : "[AUTHENTIC BUGGY]");
                        break;
                    case 3:
                        snprintf(buf, sizeof(buf), "AI NAVIGATION:       %s", ctx->game_fixes.fix_ai_pathing ? "[FIXED]" : "[AUTHENTIC BUGGY]");
                        break;
                    case 4:
                        snprintf(buf, sizeof(buf), "AUDIO & SOUND:       %s", ctx->game_fixes.fix_audio ? "[FIXED]" : "[AUTHENTIC BUGGY]");
                        break;
                    case 5:
                        snprintf(buf, sizeof(buf), "RENDERER GLITCHES:   %s", ctx->game_fixes.fix_renderer ? "[FIXED]" : "[AUTHENTIC BUGGY]");
                        break;
                    default:
                        snprintf(buf, sizeof(buf), "%s", s_game_fixes_menu_items[i]);
                        break;
                }
                const char *item = buf;
                bool selected = (i == ctx->menu_selection);
                LftFont *f = ctx->font_selected;
                int w = Font_GetTextWidth(f, item);
                int x = (NATIVE_WIDTH - w) / 2;
                int y = start_y + i * 26;
                uint8_t color = selected ? 215 : 251;

                if (selected) {
                    Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, f, x - 18, y, ">", 215);
                    Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, f, x + w + 8, y, "<", 215);
                }
                Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, f, x, y, item, color);
            }

            const char *desc = "";
            switch (ctx->menu_selection) {
                case 0: desc = "PREVENTS MESH FALL-THROUGH & BARRIER CLIPPING (DEV-001, DEV-003)"; break;
                case 1: desc = "PREVENTS CARS FLOATING OR SINKING ON TERRAIN (DEV-002)"; break;
                case 2: desc = "NORMALIZES RIGHT-HANDED CAMERA BASIS VECTOR (DEV-004)"; break;
                case 3: desc = "PREVENTS AI DRIVERS FROM GETTING TRAPPED ON SPLINE BOUNDARIES"; break;
                case 4: desc = "PREVENTS HIGH-RPM AUDIO PITCH OVERFLOW (DEV-006)"; break;
                case 5: desc = "REDUCES POLYGON SORTING FLICKER AND Z-FIGHTING (DEV-005)"; break;
                case 6: desc = "PRESET: RESTORES 100% UNMODIFIED 1997 RETRO GLITCHES"; break;
                case 7: desc = "PRESET: ENABLES ALL RECOMMENDED STABILITY AND VISUAL FIXES"; break;
                case 8: desc = "RETURN TO GAMEPLAY OPTIONS"; break;
                default: break;
            }
            int desc_w = Font_GetTextWidth(ctx->font_small, desc);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_small, (NATIVE_WIDTH - desc_w) / 2, 410, desc, 215);

            const char *hint = "UP/DOWN: SELECT    LEFT/RIGHT/ENTER: TOGGLE    ESC: BACK";
            int hint_w = Font_GetTextWidth(ctx->font_small, hint);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_small, (NATIVE_WIDTH - hint_w) / 2, 445, hint, 251);
            break;
        }

        case GAME_STATE_GFX_OPTIONS: {
            const char *hdr = "GFX OPTIONS";
            int hw = Font_GetTextWidth(ctx->font_selected, hdr);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_selected, (NATIVE_WIDTH - hw) / 2, 130, hdr, 251);

            int start_y = 165;
            for (int i = 0; i < GFX_MENU_COUNT; ++i) {
                char buf[64];
                switch (i) {
                    case 0: {
                        const char *b_str = (ctx->renderer.options.backend_type == RENDERER_BACKEND_LISA3D_SOFTWARE) ? "[LISA3D SOFTWARE]" :
                                            (ctx->renderer.options.backend_type == RENDERER_BACKEND_GLIDE_3DFX) ? "[3DFX GLIDE]" : "[DIRECT3D]";
                        snprintf(buf, sizeof(buf), "RENDERER BACKEND: %s", b_str);
                        break;
                    }
                    case 1:
                        snprintf(buf, sizeof(buf), "RESOLUTION:       [%dx%d %s]",
                                 ctx->renderer.options.internal_width, ctx->renderer.options.internal_height,
                                 ctx->renderer.options.internal_width == 640 ? "HI-RES" : "LO-RES");
                        break;
                    case 2:
                        snprintf(buf, sizeof(buf), "COLOR DEPTH:      [%s]",
                                 ctx->renderer.options.force_8bit_paletted ? "8-BIT PALETTED" : "32-BIT TRUECOLOR");
                        break;
                    case 3:
                        snprintf(buf, sizeof(buf), "UV PRECISION:     [%s]",
                                 ctx->renderer.options.authentic_fixed_point_uv ? "1997 FIXED-POINT" : "ACCURATE 1/W");
                        break;
                    case 4:
                        snprintf(buf, sizeof(buf), "Z-BUFFERING:      [%s]",
                                 ctx->renderer.options.authentic_depth_buckets ? "AUTHENTIC BUCKETS" : "PER-PIXEL Z");
                        break;
                    case 5:
                        snprintf(buf, sizeof(buf), "HORIZON PAN:      [%s]",
                                 ctx->renderer.options.render_panoramas ? "ENABLED" : "DISABLED");
                        break;
                    default:
                        snprintf(buf, sizeof(buf), "%s", s_gfx_menu_items[i]);
                        break;
                }
                const char *item = buf;
                bool selected = (i == ctx->menu_selection);
                LftFont *f = ctx->font_selected;
                int w = Font_GetTextWidth(f, item);
                int x = (NATIVE_WIDTH - w) / 2;
                int y = start_y + i * 26;
                uint8_t color = selected ? 215 : 251;

                if (selected) {
                    Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, f, x - 18, y, ">", 215);
                    Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, f, x + w + 8, y, "<", 215);
                }
                Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, f, x, y, item, color);
            }

            const char *desc = "";
            switch (ctx->menu_selection) {
                case 0: desc = "SELECT ACTIVE RASTERIZATION BACKEND (LISA3D / 3DFX / D3D)"; break;
                case 1: desc = "INTERNAL RESOLUTION: 640x480 (HIGH) OR 320x200 (DOS LOW)"; break;
                case 2: desc = "AUTHENTIC 256-COLOR PALETTE OR 32-BIT TRUECOLOR COLORWAYS"; break;
                case 3: desc = "16.16 FIXED-POINT UV STEPPING OR ACCURATE 1/W FLOATING POINT"; break;
                case 4: desc = "AUTHENTIC 6000-BUCKET ORDERING TABLE OR FLOATING POINT 1/Z (DEV-005)"; break;
                case 5: desc = "CYLINDRICAL PANORAMA HORIZON RENDERING"; break;
                case 6: desc = "PRESET: RESTORES 100% UNMODIFIED 1997 LISA3D ENGINE BEHAVIOR"; break;
                case 7: desc = "PRESET: ENABLES MODERN HIGH-PRECISION RENDER PIPELINE"; break;
                case 8: desc = "RETURN TO OPTIONS MENU"; break;
                default: break;
            }
            int desc_w = Font_GetTextWidth(ctx->font_small, desc);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_small, (NATIVE_WIDTH - desc_w) / 2, 410, desc, 215);

            const char *hint = "UP/DOWN: SELECT    LEFT/RIGHT/ENTER: TOGGLE    ESC: BACK";
            int hint_w = Font_GetTextWidth(ctx->font_small, hint);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_small, (NATIVE_WIDTH - hint_w) / 2, 445, hint, 251);
            break;
        }

        case GAME_STATE_EXTRAS: {
            const char *hdr = "EXTRAS";
            int hw = Font_GetTextWidth(ctx->font_selected, hdr);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_selected, (NATIVE_WIDTH - hw) / 2, 200, hdr, 251);

            int start_y = 250;
            for (int i = 0; i < EXTRAS_MENU_COUNT; ++i) {
                const char *item = s_extras_menu_items[i];
                bool selected = (i == ctx->menu_selection);
                LftFont *f = ctx->font_selected;
                int w = Font_GetTextWidth(f, item);
                int x = (NATIVE_WIDTH - w) / 2;
                int y = start_y + i * 36;
                uint8_t color = selected ? 215 : 251;

                if (selected) {
                    Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, f, x - 18, y, ">", 215);
                    Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, f, x + w + 8, y, "<", 215);
                }
                Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, f, x, y, item, color);
            }

            const char *hint = "ENTER: SELECT    ESC: BACK";
            int hint_w = Font_GetTextWidth(ctx->font_small, hint);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_small, (NATIVE_WIDTH - hint_w) / 2, 440, hint, 251);
            break;
        }

        case GAME_STATE_ABOUT: {
            const char *hdr = "ABOUT RACING DYNAMITE";
            int hw = Font_GetTextWidth(ctx->font_selected, hdr);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_selected, (NATIVE_WIDTH - hw) / 2, 195, hdr, 215);

            const char *lines[] = {
                "MODERN C11/SDL2 SOURCE PORT OF IGNITION (1997)",
                "ORIGINAL GAME DEVELOPED BY UNIQUE DEVELOPMENT STUDIOS",
                "PUBLISHED BY VIRGIN INTERACTIVE ENTERTAINMENT",
                "REVERSE ENGINEERED BY PATRICIO LABIN CORREA (@F1R3F0X)",
                "REPRESENTING ACCURATE 8BPP LISA3D SOFTWARE RASTERIZATION"
            };
            int line_count = 5;
            for (int i = 0; i < line_count; ++i) {
                int lw = Font_GetTextWidth(ctx->font_small, lines[i]);
                Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_small,
                              (NATIVE_WIDTH - lw) / 2, 240 + i * 28, lines[i], 251);
            }

            const char *hint = "PRESS ENTER OR ESC TO RETURN";
            int hint_w = Font_GetTextWidth(ctx->font_small, hint);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_small, (NATIVE_WIDTH - hint_w) / 2, 440, hint, 215);
            break;
        }

        case GAME_STATE_CAR_SELECT: {
            const char *hdr = "SELECT VEHICLE";
            int hw = Font_GetTextWidth(ctx->font_selected, hdr);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_selected, (NATIVE_WIDTH - hw) / 2, 40, hdr, 251);

            const char *car_name = s_car_names[ctx->selected_car];
            char buf[64];
            snprintf(buf, sizeof(buf), "<  %s  >", car_name);
            int cw = Font_GetTextWidth(ctx->font_selected, buf);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_selected, (NATIVE_WIDTH - cw) / 2, 380, buf, 215);

            const char *hint = "ENTER: CONFIRM    ESC: BACK";
            int hint_w = Font_GetTextWidth(ctx->font_small, hint);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_small, (NATIVE_WIDTH - hint_w) / 2, 440, hint, 251);
            break;
        }

        case GAME_STATE_TRACK_SELECT: {
            const char *hdr = ctx->is_visualizer_mode ? "TRACK VISUALIZER - SELECT CIRCUIT" : "SELECT CIRCUIT";
            int hw = Font_GetTextWidth(ctx->font_selected, hdr);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_selected, (NATIVE_WIDTH - hw) / 2, 30, hdr, 251);

            const char *trk_name = s_track_names[ctx->selected_track];
            char buf[64];
            snprintf(buf, sizeof(buf), "<  %s  >", trk_name);
            int tw = Font_GetTextWidth(ctx->font_selected, buf);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_selected, (NATIVE_WIDTH - tw) / 2, 400, buf, 215);

            const char *hint = ctx->is_visualizer_mode ? "ENTER: INSPECT TRACK    ESC: BACK" : "ENTER: RACE!    ESC: BACK";
            int hint_w = Font_GetTextWidth(ctx->font_small, hint);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_small, (NATIVE_WIDTH - hint_w) / 2, 440, hint, 251);
            break;
        }

        case GAME_STATE_RACE_LOADING: {
            const char *loading = ctx->is_visualizer_mode ? "LOADING 3D TRACK MESH & TEXTURES..." : "LOADING 3D TRACK & CAR MESHES...";
            int lw = Font_GetTextWidth(ctx->font_selected, loading);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_selected, (NATIVE_WIDTH - lw) / 2, 240, loading, 0);
            break;
        }

        case GAME_STATE_TRACK_VISUALIZER: {
            // Clear 3D viewport (color 0, depth 1e9f)
            Renderer_Clear(&ctx->renderer, 0, 1e9f);
            Renderer_SetTexture(&ctx->renderer, ctx->show_textures ? ctx->active_tex : NULL);
            Renderer_SetShading(&ctx->renderer, ctx->active_tab);
            Renderer_SetShadow(&ctx->renderer, ctx->active_shd);

            if (ctx->active_msh && ctx->active_plc) {
                Renderer_DrawTrackMesh(&ctx->renderer, &ctx->camera, ctx->active_msh, ctx->active_plc);
            }

            if (ctx->show_waypoints && ctx->active_waypoints) {
                Renderer_DrawWaypoints(&ctx->renderer, &ctx->camera, ctx->active_waypoints);
            }

            // HUD Overlays
            char hud[128];
            snprintf(hud, sizeof(hud), "CIRCUIT: %s | 3D TRACK VISUALIZER (LISA3D)",
                     s_track_names[ctx->selected_track]);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_small, 20, 20, hud, 251);

            char cam_str[192];
            snprintf(cam_str, sizeof(cam_str), "CAM: (%.0f, %.0f, %.0f) | YAW %.0f PITCH %.0f DIST %.0f | %s | %s | %s",
                     ctx->camera.position.x, ctx->camera.position.y, ctx->camera.position.z,
                     ctx->camera.yaw, ctx->camera.pitch, ctx->camera.distance,
                     ctx->show_textures ? "[TEX ON]" : "[TEX OFF]",
                     ctx->show_waypoints ? "[WP ON]" : "[WP OFF]",
                     ctx->auto_orbit ? "[AUTO ON]" : "[AUTO OFF]");
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_small, 20, 40, cam_str, 251);

            const char *controls = "WASD: MOVE  ARROWS: ORBIT  Z/X: ZOOM  T: TEX  P: WP  C: RESET  ESC: BACK";
            int cw = Font_GetTextWidth(ctx->font_small, controls);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_small, (NATIVE_WIDTH - cw) / 2, 450, controls, 251);
            break;
        }

        case GAME_STATE_IN_RACE: {
            // Clear 3D viewport (color 0, depth 1e9f)
            Renderer_Clear(&ctx->renderer, 0, 1e9f);
            Renderer_SetTexture(&ctx->renderer, ctx->show_textures ? ctx->active_tex : NULL);
            Renderer_SetShading(&ctx->renderer, ctx->active_tab);
            Renderer_SetShadow(&ctx->renderer, ctx->active_shd);

            if (ctx->active_msh && ctx->active_plc) {
                Renderer_DrawTrackMesh(&ctx->renderer, &ctx->camera, ctx->active_msh, ctx->active_plc);
            }

            if (ctx->show_waypoints && ctx->active_waypoints) {
                Renderer_DrawWaypoints(&ctx->renderer, &ctx->camera, ctx->active_waypoints);
            }

            // Draw player vehicle with CARS.MSH and CARS.TEX
            if (ctx->cars_msh) {
                Renderer_DrawCar(&ctx->renderer, &ctx->camera, ctx->cars_msh, ctx->cars_tex, &ctx->player_car);
            }

            // In-race HUD Overlays
            char hud[128];
            snprintf(hud, sizeof(hud), "CIRCUIT: %s | CAR: %s | 3D RASTERIZER (LISA3D)",
                     s_track_names[ctx->selected_track], s_car_names[ctx->selected_car]);
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_small, 20, 20, hud, 251);

            double speed_kmh = fabs(ctx->player_car.speed_long) * 21.76 * 0.36;
            const char *gear_str = "1";
            if (ctx->player_car.gear == 0) gear_str = "R";
            else if (ctx->player_car.gear == 2) gear_str = "2";

            char telemetry[160];
            snprintf(telemetry, sizeof(telemetry), "SPEED: %3.0f KM/H | GEAR: %s | TURBO: %s | AIR: %s",
                     speed_kmh,
                     gear_str,
                     ctx->player_car.turbo_active ? "[ACTIVE!]" : "[READY]",
                     ctx->player_car.airborne ? "YES" : "NO");
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_small, 20, 40, telemetry, 215);

            char cam_info[160];
            snprintf(cam_info, sizeof(cam_info), "POS: (%.0f, %.0f, %.0f) | YAW: %.1f deg | CAM: %s",
                     ctx->player_car.x, ctx->player_car.y, ctx->player_car.z,
                     ctx->player_car.yaw * (180.0 / 3.14159265358979323846),
                     ctx->chase_cam_mode ? "[CHASE]" : "[FREE]");
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_small, 20, 60, cam_info, 251);

            const char *controls = "ARROWS/Z: DRIVE | SPACE: TURBO | C: CAM MODE | T: TEX | P: WAYPOINTS | ESC: MENU";
            Font_DrawText(ctx->framebuffer, NATIVE_WIDTH, ctx->font_small, 20, 455, controls, 251);
            break;
        }

        case GAME_STATE_QUIT:
            break;
    }
}
