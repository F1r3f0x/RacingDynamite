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

// Discrete categories of authentic original bug workarounds
typedef enum {
    FIX_CAT_NOCLIP        = (1 << 0), // Mesh fall-through, wall clipping, mountain climbing (DEV-001, DEV-003)
    FIX_CAT_ELEVATION     = (1 << 1), // Vehicle ground alignment & terrain floating/sinking (DEV-002)
    FIX_CAT_CAMERA        = (1 << 2), // Right-handed camera basis & boundary clipping (DEV-004)
    FIX_CAT_AI_PATHING    = (1 << 3), // AI spline transition traps
    FIX_CAT_AUDIO         = (1 << 4), // High-RPM pitch curve cutoff (DEV-006)
    FIX_CAT_RENDERER      = (1 << 5), // Polygon sorting & depth buffering (DEV-005)
} GameFixCategory;

// Granular game fix configuration toggles
typedef struct {
    bool fix_noclip;       // [FIXED] vs [AUTHENTIC BUGGY]
    bool fix_elevation;    // [FIXED] vs [AUTHENTIC BUGGY]
    bool fix_camera;       // [FIXED] vs [AUTHENTIC BUGGY]
    bool fix_ai_pathing;   // [FIXED] vs [AUTHENTIC BUGGY]
    bool fix_audio;        // [FIXED] vs [AUTHENTIC BUGGY]
    bool fix_renderer;     // [FIXED] vs [AUTHENTIC BUGGY]
} GameFixOptions;

#ifdef __cplusplus
}
#endif

#endif // IGNITION_TYPES_H
