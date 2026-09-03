/*
 * Racing Dynamite - Modern open-source source port of Ignition (1997)
 * Copyright (C) 2026 Racing Dynamite Contributors
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

#include "ignition/types.h"
#include "ignition/formats.h"
#include "ignition/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NATIVE_WIDTH  640
#define NATIVE_HEIGHT 480
#define WINDOW_SCALE  2

int main(int argc, char *argv[]) {
    int max_frames = 0;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            max_frames = atoi(argv[i + 1]);
        }
    }

    printf("========================================\n");
    printf("   Ignition (1997) Source Port v0.1     \n");
    printf("========================================\n");

    // Initialize SDL2 window and renderer
    if (!Platform_Init("Ignition (1997) - Source Port", NATIVE_WIDTH, NATIVE_HEIGHT, WINDOW_SCALE)) {
        fprintf(stderr, "Failed to initialize platform layer.\n");
        return 1;
    }
    printf("[Platform] SDL2 window initialized (640x480 @ %dx scale).\n", WINDOW_SCALE);

    // Load global palette
    Palette256 sys_palette;
    if (!Col_LoadFromFile("assets/SYS.COL", &sys_palette)) {
        fprintf(stderr, "[Warning] Could not load assets/SYS.COL. Fallback to embedded palette.\n");
    } else {
        printf("[Asset] Loaded assets/SYS.COL successfully.\n");
    }

    // Load test image (INSTALL.PIC - 640x480 title/installer screen)
    Image8bpp *splash = Pic_LoadFromFile("assets/INSTALL.PIC");
    if (!splash) {
        // Fallback to track preview if INSTALL.PIC isn't present
        splash = Pic_LoadFromFile("assets/LEVELS/AUSTRIA/AUSTRIA.PIC");
    }

    if (splash) {
        printf("[Asset] Loaded splash image: %ux%u.\n", splash->width, splash->height);
    } else {
        fprintf(stderr, "[Warning] No splash .PIC found in assets/\n");
    }

    // Allocate 640x480 8bpp framebuffer
    uint8_t *framebuffer = (uint8_t *)calloc(NATIVE_WIDTH * NATIVE_HEIGHT, sizeof(uint8_t));
    if (!framebuffer) {
        Platform_Shutdown();
        return 1;
    }

    // Copy splash image to framebuffer if available
    if (splash) {
        uint32_t copy_w = splash->width < NATIVE_WIDTH ? splash->width : NATIVE_WIDTH;
        uint32_t copy_h = splash->height < NATIVE_HEIGHT ? splash->height : NATIVE_HEIGHT;
        for (uint32_t y = 0; y < copy_h; ++y) {
            memcpy(framebuffer + y * NATIVE_WIDTH, splash->pixels + y * splash->width, copy_w);
        }
        // Use splash palette
        sys_palette = splash->palette;
    }

    printf("[Engine] Entering main game loop. Press ESC to exit.\n");

    PlatformInput input = {0};
    bool running = true;
    uint32_t frame_count = 0;
    uint32_t last_time = Platform_GetTicks();

    while (running) {
        // Poll input events
        if (!Platform_PollEvents(&input)) {
            running = false;
            break;
        }

        if (input.quit_requested || input.key_menu) {
            running = false;
            break;
        }

        // Present current frame
        Platform_Present8bpp(framebuffer, &sys_palette);

        // Frame timing (~60 FPS)
        uint32_t now = Platform_GetTicks();
        uint32_t elapsed = now - last_time;
        if (elapsed < 16) {
            Platform_Delay(16 - elapsed);
        }
        last_time = Platform_GetTicks();

        frame_count++;
        if (frame_count % 300 == 0) {
            printf("[Engine] Frame %u rendered.\n", frame_count);
        }
        if (max_frames > 0 && (int)frame_count >= max_frames) {
            printf("[Engine] Reached target frame limit (%d frames). Exiting test mode.\n", max_frames);
            running = false;
            break;
        }
    }

    printf("[Engine] Shutting down...\n");

    if (splash) {
        Pic_Free(splash);
    }
    free(framebuffer);
    Platform_Shutdown();

    printf("[Engine] Exited cleanly.\n");
    return 0;
}
