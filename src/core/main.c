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

#if !defined(_WIN32)
#include <strings.h>
#endif

#define NATIVE_WIDTH  640
#define NATIVE_HEIGHT 480
#define WINDOW_SCALE  2

static bool StrCaseEqual(const char *a, const char *b) {
#if defined(_WIN32)
    return _stricmp(a, b) == 0;
#else
    return strcasecmp(a, b) == 0;
#endif
}

/**
 * @brief Main entry point; parses command-line flags, initializes engine, and executes game loop.
 * @original FUN_004120a0 (MAINDOS_32BIT.EXE @ 0x004120a0, main.c)
 * @fidelity ADAPTED
 */
int main(int argc, char *argv[]) {
    int max_frames = 0;
    const char *log_path = "racing_dynamite.log";
    LogLevel min_log_level = LOG_LEVEL_DEBUG;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            max_frames = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--log") == 0 && i + 1 < argc) {
            log_path = argv[++i];
        } else if (strcmp(argv[i], "--log-level") == 0 && i + 1 < argc) {
            i++;
            if (StrCaseEqual(argv[i], "debug")) min_log_level = LOG_LEVEL_DEBUG;
            else if (StrCaseEqual(argv[i], "info"))  min_log_level = LOG_LEVEL_INFO;
            else if (StrCaseEqual(argv[i], "warn"))  min_log_level = LOG_LEVEL_WARN;
            else if (StrCaseEqual(argv[i], "error")) min_log_level = LOG_LEVEL_ERROR;
        }
    }

    Log_Init(log_path, min_log_level);

    printf("====================================================\n");
    printf("     Racing Dynamite - Ignition (1997) Source Port  \n");
    printf("====================================================\n");
    printf("Navigation:\n");
    printf("  [ENTER / SPACE] : Start / Confirm Selection\n");
    printf("  [UP / DOWN]     : Menu Navigation\n");
    printf("  [LEFT / RIGHT]  : Car / Track Cycling\n");
    printf("  [ESC]           : Back / Exit\n");
    printf("====================================================\n\n");

    LOG_INFO("ENGINE", "Starting Racing Dynamite (Log target: %s)", log_path ? log_path : "none");

    // Initialize SDL2 window and renderer
    if (!Platform_Init("Racing Dynamite - Ignition (1997)", NATIVE_WIDTH, NATIVE_HEIGHT, WINDOW_SCALE)) {
        LOG_ERROR("PLATFORM", "Failed to initialize SDL2 platform layer.");
        Log_Shutdown();
        return 1;
    }
    LOG_INFO("PLATFORM", "SDL2 window initialized (%dx%d @ %dx scale).", NATIVE_WIDTH, NATIVE_HEIGHT, WINDOW_SCALE);

    GameContext game;
    if (!Game_Init(&game)) {
        LOG_ERROR("ENGINE", "Failed to initialize game context.");
        Platform_Shutdown();
        Log_Shutdown();
        return 1;
    }

    PlatformInput input = {0};
    bool running = true;
    uint32_t frame_count = 0;
    uint32_t last_time = Platform_GetTicks();

    while (running) {
        // Poll input events
        if (!Platform_PollEvents(&input) || input.quit_requested) {
            running = false;
            break;
        }

        uint32_t now = Platform_GetTicks();
        uint32_t delta_ms = (now > last_time) ? (now - last_time) : 16;
        if (delta_ms > 100) delta_ms = 100; // Clamp lag spike
        last_time = now;

        // Update game state
        Game_Update(&game, &input, delta_ms);
        if (game.current_state == GAME_STATE_QUIT) {
            running = false;
            break;
        }

        // Render current game state
        Game_Render(&game);

        // Present framebuffer
        Platform_Present8bpp(game.framebuffer, &game.active_palette);

        // Frame timing (~60 FPS)
        uint32_t frame_end = Platform_GetTicks();
        uint32_t frame_duration = frame_end - now;
        if (frame_duration < 16) {
            Platform_Delay(16 - frame_duration);
        }

        frame_count++;
        if (max_frames > 0 && (int)frame_count >= max_frames) {
            LOG_INFO("ENGINE", "Reached target frame limit (%d frames). Exiting test mode.", max_frames);
            running = false;
            break;
        }
    }

    LOG_INFO("ENGINE", "Shutting down application...");
    Game_Shutdown(&game);
    Platform_Shutdown();

    LOG_INFO("ENGINE", "Exited cleanly.");
    Log_Shutdown();
    return 0;
}
