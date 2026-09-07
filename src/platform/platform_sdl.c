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
    if (out_input) {
        out_input->key_pressed = 0;
        out_input->nav_up = false;
        out_input->nav_down = false;
        out_input->nav_left = false;
        out_input->nav_right = false;
        out_input->nav_confirm = false;
        out_input->nav_cancel = false;
        out_input->toggle_textures = false;
        out_input->toggle_waypoints = false;
        out_input->cycle_camera = false;
        out_input->reset_camera = false;
    }
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) {
            if (out_input) out_input->quit_requested = true;
            return false;
        }
        if (ev.type == SDL_KEYDOWN && out_input) {
            out_input->key_pressed = ev.key.keysym.sym;
            // Only trigger on initial press (not repeat)
            if (ev.key.repeat == 0) {
                switch (ev.key.keysym.sym) {
                    case SDLK_UP:
                    case SDLK_w:
                        out_input->nav_up = true;
                        break;
                    case SDLK_DOWN:
                    case SDLK_s:
                        out_input->nav_down = true;
                        break;
                    case SDLK_LEFT:
                    case SDLK_a:
                        out_input->nav_left = true;
                        break;
                    case SDLK_RIGHT:
                    case SDLK_d:
                        out_input->nav_right = true;
                        break;
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                    case SDLK_SPACE:
                        out_input->nav_confirm = true;
                        break;
                    case SDLK_ESCAPE:
                        out_input->nav_cancel = true;
                        break;
                    case SDLK_t:
                        out_input->toggle_textures = true;
                        break;
                    case SDLK_p:
                    case SDLK_v:
                        out_input->toggle_waypoints = true;
                        break;
                    case SDLK_c:
                        out_input->cycle_camera = true;
                        break;
                    case SDLK_r:
                    case SDLK_HOME:
                        out_input->reset_camera = true;
                        break;
                }
            }
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

        out_input->move_forward   = keys[SDL_SCANCODE_W];
        out_input->move_backward  = keys[SDL_SCANCODE_S];
        out_input->move_left      = keys[SDL_SCANCODE_A];
        out_input->move_right     = keys[SDL_SCANCODE_D];
        out_input->move_up        = keys[SDL_SCANCODE_Q] || keys[SDL_SCANCODE_R] || keys[SDL_SCANCODE_PAGEUP];
        out_input->move_down      = keys[SDL_SCANCODE_E] || keys[SDL_SCANCODE_F] || keys[SDL_SCANCODE_PAGEDOWN];
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
