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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void SaveBMP24(const char *filename, const uint8_t *fb, const Palette256 *pal, int w, int h) {
    FILE *f = fopen(filename, "wb");
    if (!f) return;

    int row_bytes = ((w * 3 + 3) / 4) * 4;
    int image_size = row_bytes * h;
    int file_size = 54 + image_size;

    uint8_t header[54] = {
        'B', 'M',
        file_size & 0xFF, (file_size >> 8) & 0xFF, (file_size >> 16) & 0xFF, (file_size >> 24) & 0xFF,
        0, 0, 0, 0,
        54, 0, 0, 0,
        40, 0, 0, 0,
        w & 0xFF, (w >> 8) & 0xFF, (w >> 16) & 0xFF, (w >> 24) & 0xFF,
        h & 0xFF, (h >> 8) & 0xFF, (h >> 16) & 0xFF, (h >> 24) & 0xFF,
        1, 0,
        24, 0,
        0, 0, 0, 0,
        image_size & 0xFF, (image_size >> 8) & 0xFF, (image_size >> 16) & 0xFF, (image_size >> 24) & 0xFF,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    fwrite(header, 1, 54, f);

    uint8_t *row = (uint8_t *)calloc(row_bytes, 1);
    for (int y = h - 1; y >= 0; --y) {
        for (int x = 0; x < w; ++x) {
            uint8_t idx = fb[y * w + x];
            row[x * 3 + 0] = pal->colors[idx].b;
            row[x * 3 + 1] = pal->colors[idx].g;
            row[x * 3 + 2] = pal->colors[idx].r;
        }
        fwrite(row, 1, row_bytes, f);
    }
    free(row);
    fclose(f);
}

int main(void) {
    printf("=== Testing Game State Machine & Palette Restoration ===\n");

    GameContext ctx;
    bool ok = Game_Init(&ctx);
    assert(ok);
    assert(ctx.current_state == GAME_STATE_INTRO);
    printf("[PASS] Game_Init: Started in INTRO.\n");

    PlatformInput input;
    memset(&input, 0, sizeof(input));

    // 1. Advance INTRO -> MAIN_MENU
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_MAIN_MENU);
    Game_Render(&ctx);
    printf("[PASS] Transitioned to MAIN_MENU.\n");

    // Check menu palette is INSTALL.PIC palette (not MENU.COL)
    Image8bpp *install_ref = Pic_LoadFromFile("assets/INSTALL.PIC");
    assert(install_ref != NULL);
    assert(memcmp(&ctx.active_palette, &install_ref->palette, sizeof(Palette256)) == 0);
    Pic_Free(install_ref);
    printf("[PASS] Main menu has correct INSTALL.PIC palette.\n");

    // 2. MAIN_MENU -> OPTIONS
    for (int i = 0; i < 3; ++i) {
        input.nav_down = true;
        Game_Update(&ctx, &input, 16);
        input.nav_down = false;
    }
    assert(ctx.menu_selection == 3);
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_OPTIONS);
    assert(ctx.menu_selection == 0);
    Game_Render(&ctx);
    SaveBMP24("docs/extracted_bitmaps/TEST_MENU_OPTIONS.bmp", ctx.framebuffer, &ctx.active_palette, 640, 480);
    printf("[PASS] Transitioned to OPTIONS.\n");

    // 2b. OPTIONS -> GAMEPLAY OPTIONS -> GAME FIXES
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_GAMEPLAY_OPTIONS);
    assert(ctx.menu_selection == 0);
    Game_Render(&ctx);
    SaveBMP24("docs/extracted_bitmaps/TEST_MENU_GAMEPLAY.bmp", ctx.framebuffer, &ctx.active_palette, 640, 480);
    printf("[PASS] Transitioned to GAMEPLAY OPTIONS.\n");

    // GAMEPLAY OPTIONS -> GAME FIXES
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_GAME_FIXES);
    assert(ctx.menu_selection == 0);
    assert(ctx.game_fixes.fix_noclip == false); // Default: Authentic 1997 Buggy (disabled)
    assert(ctx.game_fixes.fix_elevation == false);
    assert(ctx.game_fixes.fix_camera == false);
    assert(ctx.game_fixes.fix_ai_pathing == false);
    assert(ctx.game_fixes.fix_audio == false);
    assert(ctx.game_fixes.fix_renderer == false);
    Game_Render(&ctx);
    SaveBMP24("docs/extracted_bitmaps/TEST_MENU_GAME_FIXES.bmp", ctx.framebuffer, &ctx.active_palette, 640, 480);
    printf("[PASS] Transitioned to GAME FIXES (All Fixes [OFF] by default).\n");

    // Toggle No-Clip Fix ON
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.game_fixes.fix_noclip == true);
    Game_Render(&ctx);
    SaveBMP24("docs/extracted_bitmaps/TEST_MENU_GAME_FIXES_ON.bmp", ctx.framebuffer, &ctx.active_palette, 640, 480);
    printf("[PASS] Toggled No-Clip Fix to [ON].\n");

    // Toggle No-Clip Fix back OFF
    input.nav_left = true;
    Game_Update(&ctx, &input, 16);
    input.nav_left = false;
    assert(ctx.game_fixes.fix_noclip == false);
    printf("[PASS] Toggled No-Clip Fix back to [OFF].\n");

    // Test PRESET: 1997 AUTHENTIC (item 6)
    ctx.menu_selection = 6;
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.game_fixes.fix_noclip == false);
    assert(ctx.game_fixes.fix_elevation == false);
    assert(ctx.game_fixes.fix_camera == false);
    printf("[PASS] Applied PRESET: 1997 AUTHENTIC (All fixes OFF).\n");

    // Test PRESET: APPLY ALL FIXES (item 7)
    ctx.menu_selection = 7;
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.game_fixes.fix_noclip == true);
    assert(ctx.game_fixes.fix_elevation == true);
    assert(ctx.game_fixes.fix_camera == true);
    printf("[PASS] Applied PRESET: APPLY ALL FIXES (All fixes ON).\n");

    // Return to GAMEPLAY OPTIONS via BACK item (item 8)
    ctx.menu_selection = 8;
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_GAMEPLAY_OPTIONS);
    assert(ctx.menu_selection == 0);
    printf("[PASS] Returned from GAME FIXES to GAMEPLAY OPTIONS.\n");

    // Return to OPTIONS via BACK item
    input.nav_down = true;
    Game_Update(&ctx, &input, 16);
    input.nav_down = false;
    assert(ctx.menu_selection == 1);
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_OPTIONS);
    assert(ctx.menu_selection == 0);
    printf("[PASS] Returned from GAMEPLAY OPTIONS to OPTIONS.\n");

    // 2c. OPTIONS -> GFX OPTIONS
    input.nav_down = true;
    Game_Update(&ctx, &input, 16);
    input.nav_down = false;
    assert(ctx.menu_selection == 1);
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_GFX_OPTIONS);
    assert(ctx.menu_selection == 0);
    assert(ctx.renderer.options.backend_type == RENDERER_BACKEND_LISA3D_SOFTWARE);
    Game_Render(&ctx);
    SaveBMP24("docs/extracted_bitmaps/TEST_MENU_GFX_OPTIONS.bmp", ctx.framebuffer, &ctx.active_palette, 640, 480);
    printf("[PASS] Transitioned to GFX OPTIONS.\n");

    // Cycle backend (item 0): LISA3D_SOFTWARE -> GLIDE_3DFX
    input.nav_right = true;
    Game_Update(&ctx, &input, 16);
    input.nav_right = false;
    assert(ctx.renderer.options.backend_type == RENDERER_BACKEND_GLIDE_3DFX);
    printf("[PASS] Cycled Backend to GLIDE_3DFX.\n");

    // Cycle backend again: GLIDE_3DFX -> DIRECT3D
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.renderer.options.backend_type == RENDERER_BACKEND_DIRECT3D);
    printf("[PASS] Cycled Backend to DIRECT3D.\n");

    // Cycle backend again: DIRECT3D -> LISA3D_SOFTWARE
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.renderer.options.backend_type == RENDERER_BACKEND_LISA3D_SOFTWARE);
    printf("[PASS] Cycled Backend back to LISA3D_SOFTWARE.\n");

    // Toggle Z-buffering (item 4): from true (authentic buckets default) to false (modern per-pixel 1/Z)
    ctx.menu_selection = 4;
    assert(ctx.renderer.options.authentic_depth_buckets == true);
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.renderer.options.authentic_depth_buckets == false);
    printf("[PASS] Toggled Z-Buffering to modern per-pixel 1/Z.\n");

    // Apply PRESET: 1997 AUTHENTIC SOFTWARE (item 6)
    ctx.menu_selection = 6;
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.renderer.options.authentic_fixed_point_uv == true);
    assert(ctx.renderer.options.authentic_depth_buckets == true);
    printf("[PASS] Applied PRESET: 1997 AUTHENTIC SOFTWARE.\n");

    // Apply PRESET: MODERN ENHANCED (item 7)
    ctx.menu_selection = 7;
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.renderer.options.authentic_fixed_point_uv == false);
    assert(ctx.renderer.options.authentic_depth_buckets == false);
    printf("[PASS] Applied PRESET: MODERN ENHANCED.\n");

    // Return to OPTIONS via BACK item (item 8)
    ctx.menu_selection = 8;
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_OPTIONS);
    assert(ctx.menu_selection == 1);
    printf("[PASS] Returned from GFX OPTIONS to OPTIONS.\n");

    // 3. OPTIONS -> EXTRAS (from selection 1, advance down 2 times to selection 3)
    for (int i = 0; i < 2; ++i) {
        input.nav_down = true;
        Game_Update(&ctx, &input, 16);
        input.nav_down = false;
    }
    assert(ctx.menu_selection == 3);
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_EXTRAS);
    assert(ctx.menu_selection == 0);
    Game_Render(&ctx);
    SaveBMP24("docs/extracted_bitmaps/TEST_MENU_EXTRAS.bmp", ctx.framebuffer, &ctx.active_palette, 640, 480);
    printf("[PASS] Transitioned to EXTRAS.\n");

    // 4. EXTRAS -> ABOUT RACING DYNAMITE -> EXTRAS
    input.nav_down = true;
    Game_Update(&ctx, &input, 16);
    input.nav_down = false;
    assert(ctx.menu_selection == 1);
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_ABOUT);
    Game_Render(&ctx);
    SaveBMP24("docs/extracted_bitmaps/TEST_MENU_ABOUT.bmp", ctx.framebuffer, &ctx.active_palette, 640, 480);
    printf("[PASS] Transitioned to ABOUT screen.\n");

    // Press ESC to return to EXTRAS
    input.nav_cancel = true;
    Game_Update(&ctx, &input, 16);
    input.nav_cancel = false;
    assert(ctx.current_state == GAME_STATE_EXTRAS);
    assert(ctx.menu_selection == 1);
    printf("[PASS] Returned from ABOUT to EXTRAS.\n");

    // 5. EXTRAS -> TRACK VISUALIZER (Track Select)
    input.nav_up = true;
    Game_Update(&ctx, &input, 16);
    input.nav_up = false;
    assert(ctx.menu_selection == 0);
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_TRACK_SELECT);
    assert(ctx.is_visualizer_mode == true);
    Game_Render(&ctx);
    SaveBMP24("docs/extracted_bitmaps/TEST_TRACK_SELECT_VISUALIZER.bmp", ctx.framebuffer, &ctx.active_palette, 640, 480);
    printf("[PASS] Transitioned to TRACK_SELECT in visualizer mode.\n");

    // Confirm Track -> RACE_LOADING -> TRACK_VISUALIZER
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_RACE_LOADING);
    printf("[PASS] Track Visualizer loading assets...\n");

    Game_Update(&ctx, &input, 700);
    assert(ctx.current_state == GAME_STATE_TRACK_VISUALIZER);
    printf("[PASS] Transitioned to TRACK_VISUALIZER 3D inspection mode.\n");

    // Render 3D Track Visualizer Frame
    Game_Render(&ctx);
    SaveBMP24("docs/extracted_bitmaps/TEST_3D_VISUALIZER.bmp", ctx.framebuffer, &ctx.active_palette, 640, 480);
    printf("[PASS] Rendered 3D visualizer frame saved to docs/extracted_bitmaps/TEST_3D_VISUALIZER.bmp.\n");

    // 6. TRACK_VISUALIZER -> EXTRAS (User presses ESC)
    input.nav_cancel = true;
    Game_Update(&ctx, &input, 16);
    input.nav_cancel = false;
    assert(ctx.current_state == GAME_STATE_EXTRAS);
    assert(ctx.menu_selection == 0);
    printf("[PASS] Escaped from TRACK_VISUALIZER back to EXTRAS.\n");

    // Check menu palette is RESTORED properly!
    install_ref = Pic_LoadFromFile("assets/INSTALL.PIC");
    assert(install_ref != NULL);
    assert(memcmp(&ctx.active_palette, &install_ref->palette, sizeof(Palette256)) == 0);
    Pic_Free(install_ref);
    printf("[PASS] Palette after returning from visualizer to EXTRAS is verified intact!\n");

    // 7. EXTRAS -> OPTIONS -> MAIN_MENU
    input.nav_down = true;
    Game_Update(&ctx, &input, 16);
    input.nav_down = false;
    input.nav_down = true;
    Game_Update(&ctx, &input, 16);
    input.nav_down = false;
    assert(ctx.menu_selection == 2); // BACK
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_OPTIONS);
    assert(ctx.menu_selection == 3); // Return to EXTRAS entry in Options
    printf("[PASS] Returned from EXTRAS to OPTIONS.\n");

    input.nav_down = true;
    Game_Update(&ctx, &input, 16);
    input.nav_down = false;
    assert(ctx.menu_selection == 4); // BACK in Options
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_MAIN_MENU);
    assert(ctx.menu_selection == 3); // Return to OPTIONS entry in Main Menu
    printf("[PASS] Returned from OPTIONS to MAIN_MENU.\n");

    // Return to top of MAIN_MENU (SINGLE RACE)
    while (ctx.menu_selection != 0) {
        input.nav_up = true;
        Game_Update(&ctx, &input, 16);
        input.nav_up = false;
    }
    assert(ctx.menu_selection == 0);

    // 8. Standard Race Flow: MAIN_MENU -> CAR_SELECT
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_CAR_SELECT);
    Game_Render(&ctx);
    printf("[PASS] Transitioned to CAR_SELECT.\n");

    // 9. CAR_SELECT -> TRACK_SELECT
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_TRACK_SELECT);
    assert(ctx.is_visualizer_mode == false);
    Game_Render(&ctx);
    printf("[PASS] Transitioned to TRACK_SELECT (Austria loaded).\n");

    // 4. TRACK_SELECT -> RACE_LOADING -> IN_RACE
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_RACE_LOADING);
    printf("[PASS] Transitioned to RACE_LOADING.\n");

    // Wait 700ms for load
    Game_Update(&ctx, &input, 700);
    assert(ctx.current_state == GAME_STATE_IN_RACE);
    printf("[PASS] Transitioned to IN_RACE.\n");

    // Render 3D Race Frame
    Game_Render(&ctx);
    SaveBMP24("docs/extracted_bitmaps/TEST_3D_RACE.bmp", ctx.framebuffer, &ctx.active_palette, 640, 480);
    printf("[PASS] Rendered 3D race frame saved to docs/extracted_bitmaps/TEST_3D_RACE.bmp.\n");

    ctx.camera.target.x = 2066.6f;
    ctx.camera.target.y = -3609.8f;
    ctx.camera.target.z = -1440.7f;
    ctx.camera.yaw = 300.0f;
    ctx.camera.pitch = 47.0f;
    ctx.camera.distance = 5480.0f;
    Camera_Update(&ctx.camera, 0.0f);
    Game_Render(&ctx);
    SaveBMP24("docs/extracted_bitmaps/TEST_USER_AUSTRIA.bmp", ctx.framebuffer, &ctx.active_palette, 640, 480);
    printf("[PASS] Rendered User Austria frame saved to docs/extracted_bitmaps/TEST_USER_AUSTRIA.bmp.\n");

    // 5. IN_RACE -> MAIN_MENU (User presses ESC / nav_cancel)
    input.nav_cancel = true;
    Game_Update(&ctx, &input, 16);
    input.nav_cancel = false;
    assert(ctx.current_state == GAME_STATE_MAIN_MENU);
    printf("[PASS] Escaped from IN_RACE to MAIN_MENU.\n");

    // Check menu palette is RESTORED properly!
    install_ref = Pic_LoadFromFile("assets/INSTALL.PIC");
    assert(install_ref != NULL);
    assert(memcmp(&ctx.active_palette, &install_ref->palette, sizeof(Palette256)) == 0);
    Pic_Free(install_ref);
    printf("[PASS] Palette after returning to MAIN_MENU is verified intact!\n");

    // Render Main Menu after race and save BMP
    Game_Render(&ctx);
    SaveBMP24("docs/extracted_bitmaps/TEST_MENU_AFTER_RACE.bmp", ctx.framebuffer, &ctx.active_palette, 640, 480);
    printf("[PASS] Rendered post-race menu saved to docs/extracted_bitmaps/TEST_MENU_AFTER_RACE.bmp.\n");

    // 6. Test Brazil 3D render: MAIN_MENU -> CAR_SELECT -> TRACK_SELECT (choose Brazil) -> IN_RACE
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_CAR_SELECT);

    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_TRACK_SELECT);

    // Navigate right to Track 1 (Brazil)
    input.nav_right = true;
    Game_Update(&ctx, &input, 16);
    input.nav_right = false;
    assert(ctx.selected_track == 1);

    // Confirm Brazil selection
    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_RACE_LOADING);

    Game_Update(&ctx, &input, 700);
    assert(ctx.current_state == GAME_STATE_IN_RACE);

    Game_Render(&ctx);
    SaveBMP24("docs/extracted_bitmaps/TEST_3D_BRAZIL.bmp", ctx.framebuffer, &ctx.active_palette, 640, 480);
    printf("[PASS] Rendered Brazil 3D race frame saved to docs/extracted_bitmaps/TEST_3D_BRAZIL.bmp.\n");

    ctx.camera.target.x = 1458.5f;
    ctx.camera.target.y = -4183.8f;
    ctx.camera.target.z = 5575.0f;
    ctx.camera.yaw = 162.0f;
    ctx.camera.pitch = 35.0f;
    ctx.camera.distance = 7500.0f;
    Camera_Update(&ctx.camera, 0.0f);
    Game_Render(&ctx);
    SaveBMP24("docs/extracted_bitmaps/TEST_USER_BRAZIL.bmp", ctx.framebuffer, &ctx.active_palette, 640, 480);
    printf("[PASS] Rendered User Brazil frame saved to docs/extracted_bitmaps/TEST_USER_BRAZIL.bmp.\n");

    // 7. Test Japan 3D render: IN_RACE -> MAIN_MENU -> CAR_SELECT -> TRACK_SELECT (choose Japan) -> IN_RACE
    input.nav_cancel = true;
    Game_Update(&ctx, &input, 16);
    input.nav_cancel = false;
    assert(ctx.current_state == GAME_STATE_MAIN_MENU);

    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_CAR_SELECT);

    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_TRACK_SELECT);

    // Navigate to Track 5 (Japan)
    while (ctx.selected_track != 5) {
        input.nav_right = true;
        Game_Update(&ctx, &input, 16);
        input.nav_right = false;
    }
    assert(ctx.selected_track == 5);

    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_RACE_LOADING);

    Game_Update(&ctx, &input, 700);
    assert(ctx.current_state == GAME_STATE_IN_RACE);

    Game_Render(&ctx);
    SaveBMP24("docs/extracted_bitmaps/TEST_3D_JAPAN.bmp", ctx.framebuffer, &ctx.active_palette, 640, 480);
    printf("[PASS] Rendered Japan 3D race frame saved to docs/extracted_bitmaps/TEST_3D_JAPAN.bmp.\n");

    ctx.camera.target.x = -1096.3f;
    ctx.camera.target.y = -3735.8f;
    ctx.camera.target.z = -3063.1f;
    ctx.camera.yaw = 54.0f;
    ctx.camera.pitch = 35.0f;
    ctx.camera.distance = 7500.0f;
    Camera_Update(&ctx.camera, 0.0f);
    Game_Render(&ctx);
    SaveBMP24("docs/extracted_bitmaps/TEST_USER_JAPAN.bmp", ctx.framebuffer, &ctx.active_palette, 640, 480);
    printf("[PASS] Rendered User Japan frame saved to docs/extracted_bitmaps/TEST_USER_JAPAN.bmp.\n");

    // 8. Test USA 3D render (Water Tank support beams): IN_RACE -> MAIN_MENU -> CAR_SELECT -> TRACK_SELECT (choose USA) -> IN_RACE
    input.nav_cancel = true;
    Game_Update(&ctx, &input, 16);
    input.nav_cancel = false;
    assert(ctx.current_state == GAME_STATE_MAIN_MENU);

    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_CAR_SELECT);

    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_TRACK_SELECT);

    // Navigate to Track 6 (USA)
    while (ctx.selected_track != 6) {
        input.nav_right = true;
        Game_Update(&ctx, &input, 16);
        input.nav_right = false;
    }
    assert(ctx.selected_track == 6);

    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_RACE_LOADING);

    Game_Update(&ctx, &input, 700);
    assert(ctx.current_state == GAME_STATE_IN_RACE);

    // Focus camera on USA Water Tank (Object #54 at pos = -923, 229, -3370)
    ctx.camera.target.x = -923.0f;
    ctx.camera.target.y = 229.0f;
    ctx.camera.target.z = -3370.0f;
    ctx.camera.yaw = 332.0f;
    ctx.camera.pitch = 30.0f;
    ctx.camera.distance = 1500.0f;
    Camera_Update(&ctx.camera, 0.0f);
    Game_Render(&ctx);
    SaveBMP24("docs/extracted_bitmaps/TEST_USER_USA_TANK.bmp", ctx.framebuffer, &ctx.active_palette, 640, 480);
    printf("[PASS] Rendered USA Water Tank frame saved to docs/extracted_bitmaps/TEST_USER_USA_TANK.bmp.\n");

    // 9. Test Iceland 3D render (Steam gorge puffs): IN_RACE -> MAIN_MENU -> CAR_SELECT -> TRACK_SELECT (choose Iceland) -> IN_RACE
    input.nav_cancel = true;
    Game_Update(&ctx, &input, 16);
    input.nav_cancel = false;
    assert(ctx.current_state == GAME_STATE_MAIN_MENU);

    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_CAR_SELECT);

    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_TRACK_SELECT);

    // Navigate to Track 4 (Iceland)
    while (ctx.selected_track != 4) {
        if (ctx.selected_track > 4) {
            input.nav_left = true;
            Game_Update(&ctx, &input, 16);
            input.nav_left = false;
        } else {
            input.nav_right = true;
            Game_Update(&ctx, &input, 16);
            input.nav_right = false;
        }
    }
    assert(ctx.selected_track == 4);

    input.nav_confirm = true;
    Game_Update(&ctx, &input, 16);
    input.nav_confirm = false;
    assert(ctx.current_state == GAME_STATE_RACE_LOADING);

    Game_Update(&ctx, &input, 700);
    assert(ctx.current_state == GAME_STATE_IN_RACE);

    // Focus camera on Iceland Steam Gorge (exact viewpoint from user screenshot media_1788546198179.jpg)
    ctx.camera.target.x = -8328.6f;
    ctx.camera.target.y = -3799.8f;
    ctx.camera.target.z = -4368.9f;
    ctx.camera.yaw = 48.0f;
    ctx.camera.pitch = 35.0f;
    ctx.camera.distance = 7500.0f;
    Camera_Update(&ctx.camera, 0.0f);
    Game_Render(&ctx);
    SaveBMP24("docs/extracted_bitmaps/TEST_USER_ICELAND.bmp", ctx.framebuffer, &ctx.active_palette, 640, 480);
    printf("[PASS] Rendered Iceland Steam Gorge frame saved to docs/extracted_bitmaps/TEST_USER_ICELAND.bmp.\n");

    // 8. Test Texture Toggle & Camera Translation & Reset in IN_RACE
    assert(ctx.show_textures == true);
    // Toggle texture off
    input.toggle_textures = true;
    Game_Update(&ctx, &input, 16);
    input.toggle_textures = false;
    assert(ctx.show_textures == false);
    Game_Render(&ctx);
    SaveBMP24("docs/extracted_bitmaps/TEST_3D_UNTEXTURED.bmp", ctx.framebuffer, &ctx.active_palette, 640, 480);
    printf("[PASS] Rendered Untextured 3D frame saved to docs/extracted_bitmaps/TEST_3D_UNTEXTURED.bmp.\n");

    // Toggle texture back on
    input.toggle_textures = true;
    Game_Update(&ctx, &input, 16);
    input.toggle_textures = false;
    assert(ctx.show_textures == true);
    printf("[PASS] Toggled textures back ON successfully.\n");

    // Test Camera Translation (WASD + QE)
    Vec3 orig_target = ctx.camera.target;
    input.move_forward = true;
    input.move_right = true;
    input.move_up = true;
    Game_Update(&ctx, &input, 50); // 50 ms of movement
    input.move_forward = false;
    input.move_right = false;
    input.move_up = false;
    assert(ctx.camera.target.x != orig_target.x);
    assert(ctx.camera.target.y != orig_target.y);
    assert(ctx.camera.target.z != orig_target.z);
    Game_Render(&ctx);
    SaveBMP24("docs/extracted_bitmaps/TEST_3D_TRANSLATED.bmp", ctx.framebuffer, &ctx.active_palette, 640, 480);
    printf("[PASS] Camera translation verified and saved to docs/extracted_bitmaps/TEST_3D_TRANSLATED.bmp.\n");

    // Test Camera Reset ('C' / reset_camera)
    input.reset_camera = true;
    Game_Update(&ctx, &input, 16);
    input.reset_camera = false;
    assert(ctx.camera.target.x == 0.0f);
    assert(ctx.camera.target.y == -600.0f);
    assert(ctx.camera.target.z == 0.0f);
    assert(ctx.camera.yaw == 45.0f);
    assert(ctx.camera.pitch == 35.0f);
    assert(ctx.camera.distance == 7500.0f);
    printf("[PASS] Camera reset verified successfully.\n");

    // 9. Test AI Waypoints Visualization & Toggle
    assert(ctx.show_waypoints == false);
    input.toggle_waypoints = true;
    Game_Update(&ctx, &input, 16);
    input.toggle_waypoints = false;
    assert(ctx.show_waypoints == true);
    assert(ctx.active_waypoints != NULL);
    assert(ctx.active_waypoints->count > 0);
    Game_Render(&ctx);
    SaveBMP24("docs/extracted_bitmaps/TEST_WAYPOINTS_ON.bmp", ctx.framebuffer, &ctx.active_palette, 640, 480);
    printf("[PASS] Rendered 3D frame with AI Waypoints ENABLED saved to docs/extracted_bitmaps/TEST_WAYPOINTS_ON.bmp.\n");

    // Toggle waypoints OFF
    input.toggle_waypoints = true;
    Game_Update(&ctx, &input, 16);
    input.toggle_waypoints = false;
    assert(ctx.show_waypoints == false);
    Game_Render(&ctx);
    SaveBMP24("docs/extracted_bitmaps/TEST_WAYPOINTS_OFF.bmp", ctx.framebuffer, &ctx.active_palette, 640, 480);
    printf("[PASS] Rendered 3D frame with AI Waypoints DISABLED saved to docs/extracted_bitmaps/TEST_WAYPOINTS_OFF.bmp.\n");

    // Toggle waypoints back ON
    input.toggle_waypoints = true;
    Game_Update(&ctx, &input, 16);
    input.toggle_waypoints = false;
    assert(ctx.show_waypoints == true);
    printf("[PASS] Toggled AI waypoints back ON successfully.\n");

    Game_Shutdown(&ctx);
    printf("=== All Game State & Palette Restoration Tests PASSED ===\n");
    return 0;
}
