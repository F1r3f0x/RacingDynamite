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

#ifndef IGNITION_PLATFORM_H
#define IGNITION_PLATFORM_H

#include <stdint.h>
#include <stdbool.h>
#include "ignition/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool quit_requested;
    bool key_up;
    bool key_down;
    bool key_left;
    bool key_right;
    bool key_accelerate;
    bool key_brake;
    bool key_turbo;
    bool key_menu;
    bool key_enter;
} PlatformInput;

// Initialize platform window and rendering context
// virtual_width / virtual_height = native resolution (e.g. 640x480 or 320x200)
// window_scale = multiplier (e.g. 2 for 1280x960 window)
bool Platform_Init(const char *title, int virtual_width, int virtual_height, int window_scale);

// Shutdown platform and free resources
void Platform_Shutdown(void);

// Poll OS/SDL events and update input state
bool Platform_PollEvents(PlatformInput *out_input);

// Blit 8-bit paletted virtual framebuffer to window using current palette
void Platform_Present8bpp(const uint8_t *framebuffer_8bpp, const Palette256 *palette);

// Frame timing in milliseconds
uint32_t Platform_GetTicks(void);
void Platform_Delay(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif // IGNITION_PLATFORM_H
