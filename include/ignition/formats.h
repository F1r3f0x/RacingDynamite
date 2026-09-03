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

#ifndef IGNITION_FORMATS_H
#define IGNITION_FORMATS_H

#include "ignition/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

// Ignition .PIC Header (846 bytes)
typedef struct {
    uint32_t file_size;       // Total file size in bytes
    uint16_t format_id;       // 0x9500
    uint16_t width;           // Image width in pixels
    uint16_t height;          // Image height in pixels
    uint8_t  reserved[68];    // Header padding up to byte 78
    ColorRGB palette[256];    // 256 RGB entries (768 bytes)
} PicHeader;

// Ignition .COL Header (776 bytes)
typedef struct {
    uint32_t file_size;       // 776 bytes
    uint32_t signature;       // 0x0000B123
    ColorRGB palette[256];    // 256 RGB entries (768 bytes)
} ColHeader;

#pragma pack(pop)

// In-memory decoded image representation
typedef struct {
    uint32_t   width;
    uint32_t   height;
    Palette256 palette;
    uint8_t   *pixels;        // 8-bit paletted pixel buffer (width * height)
} Image8bpp;

// PIC functions
Image8bpp *Pic_LoadFromFile(const char *filepath);
Image8bpp *Pic_LoadFromMemory(const uint8_t *data, size_t size);
void       Pic_Free(Image8bpp *image);

// Convert 8-bit paletted buffer to 32-bit RGBA/BGRA for SDL texture upload
void       Image8bpp_ToRGBA32(const Image8bpp *src, uint32_t *dst_pixels);
void       Image8bpp_ToBGRA32(const Image8bpp *src, uint32_t *dst_pixels);

// COL functions
bool       Col_LoadFromFile(const char *filepath, Palette256 *out_palette);
bool       Col_LoadFromMemory(const uint8_t *data, size_t size, Palette256 *out_palette);

#ifdef __cplusplus
}
#endif

#endif // IGNITION_FORMATS_H
