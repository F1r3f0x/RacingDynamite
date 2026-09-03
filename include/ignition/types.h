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

#ifndef IGNITION_TYPES_H
#define IGNITION_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Fixed-point 16.16 types commonly used in retro 3D games
typedef int32_t fixed16_t;
#define TO_FIXED(x)   ((fixed16_t)((x) * 65536.0f))
#define FROM_FIXED(x) (((float)(x)) / 65536.0f)

// 2D Vector
typedef struct {
    int32_t x;
    int32_t y;
} Vec2i;

typedef struct {
    float x;
    float y;
} Vec2f;

// 3D Vector
typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
} Vec3i;

typedef struct {
    float x;
    float y;
    float z;
} Vec3f;

// Color Triplet
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} ColorRGB;

typedef struct {
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;
} ColorBGRA;

// 256 Color Palette
typedef struct {
    ColorRGB colors[256];
} Palette256;

#ifdef __cplusplus
}
#endif

#endif // IGNITION_TYPES_H
