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

#include "ignition/platform.h"
#include <SDL.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static SDL_Window   *s_window = NULL;
static SDL_Renderer *s_renderer = NULL;
static SDL_Texture  *s_texture = NULL;
static uint32_t     *s_pixel_buffer_32 = NULL;
static int           s_virtual_width = 0;
static int           s_virtual_height = 0;

bool Platform_Init(const char *title, int virtual_width, int virtual_height, int window_scale) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER) < 0) {
        fprintf(stderr, "[SDL] Init failed: %s\n", SDL_GetError());
        return false;
    }

    s_virtual_width = virtual_width;
    s_virtual_height = virtual_height;

    int win_w = virtual_width * window_scale;
    int win_h = virtual_height * window_scale;

    s_window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        win_w,
        win_h,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!s_window) {
        fprintf(stderr, "[SDL] CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    s_renderer = SDL_CreateRenderer(
        s_window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!s_renderer) {
        // Fallback to software renderer if accelerated fails
        s_renderer = SDL_CreateRenderer(s_window, -1, SDL_RENDERER_SOFTWARE);
    }

    if (!s_renderer) {
        fprintf(stderr, "[SDL] CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(s_window);
        SDL_Quit();
        return false;
    }

    // Set logical presentation size with aspect ratio preservation
    SDL_RenderSetLogicalSize(s_renderer, virtual_width, virtual_height);

    s_texture = SDL_CreateTexture(
        s_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        virtual_width,
        virtual_height
    );

    if (!s_texture) {
        fprintf(stderr, "[SDL] CreateTexture failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(s_renderer);
        SDL_DestroyWindow(s_window);
        SDL_Quit();
        return false;
    }

    s_pixel_buffer_32 = (uint32_t *)malloc(virtual_width * virtual_height * sizeof(uint32_t));
    if (!s_pixel_buffer_32) {
        Platform_Shutdown();
        return false;
    }

    return true;
}

void Platform_Shutdown(void) {
    if (s_pixel_buffer_32) {
        free(s_pixel_buffer_32);
        s_pixel_buffer_32 = NULL;
    }
    if (s_texture) {
        SDL_DestroyTexture(s_texture);
        s_texture = NULL;
    }
    if (s_renderer) {
        SDL_DestroyRenderer(s_renderer);
        s_renderer = NULL;
    }
    if (s_window) {
        SDL_DestroyWindow(s_window);
        s_window = NULL;
    }
    SDL_Quit();
}

bool Platform_PollEvents(PlatformInput *out_input) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) {
            if (out_input) out_input->quit_requested = true;
            return false;
        }
    }

    if (out_input) {
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        out_input->key_up         = keys[SDL_SCANCODE_UP];
        out_input->key_down       = keys[SDL_SCANCODE_DOWN];
        out_input->key_left       = keys[SDL_SCANCODE_LEFT];
        out_input->key_right      = keys[SDL_SCANCODE_RIGHT];
        out_input->key_accelerate = keys[SDL_SCANCODE_Z] || keys[SDL_SCANCODE_UP];
        out_input->key_brake      = keys[SDL_SCANCODE_X] || keys[SDL_SCANCODE_DOWN];
        out_input->key_turbo      = keys[SDL_SCANCODE_SPACE];
        out_input->key_menu       = keys[SDL_SCANCODE_ESCAPE];
        out_input->key_enter      = keys[SDL_SCANCODE_RETURN];
    }

    return true;
}

void Platform_Present8bpp(const uint8_t *framebuffer_8bpp, const Palette256 *palette) {
    if (!framebuffer_8bpp || !palette || !s_texture || !s_pixel_buffer_32) return;

    size_t total_pixels = (size_t)s_virtual_width * (size_t)s_virtual_height;
    for (size_t i = 0; i < total_pixels; ++i) {
        uint8_t idx = framebuffer_8bpp[i];
        const ColorRGB *c = &palette->colors[idx];
        s_pixel_buffer_32[i] = ((uint32_t)0xFF << 24) |
                               ((uint32_t)c->r << 16) |
                               ((uint32_t)c->g << 8)  |
                               ((uint32_t)c->b);
    }

    SDL_UpdateTexture(s_texture, NULL, s_pixel_buffer_32, s_virtual_width * sizeof(uint32_t));
    SDL_RenderClear(s_renderer);
    SDL_RenderCopy(s_renderer, s_texture, NULL, NULL);
    SDL_RenderPresent(s_renderer);
}

uint32_t Platform_GetTicks(void) {
    return SDL_GetTicks();
}

void Platform_Delay(uint32_t ms) {
    SDL_Delay(ms);
}
