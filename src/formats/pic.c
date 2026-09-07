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

#include "ignition/formats.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Image8bpp *Pic_LoadFromMemory(const uint8_t *data, size_t size) {
    if (!data || size < sizeof(PicHeader)) {
        return NULL;
    }

    const PicHeader *hdr = (const PicHeader *)data;
    uint32_t expected_pixels = (uint32_t)hdr->width * (uint32_t)hdr->height;
    if (size < sizeof(PicHeader) + expected_pixels) {
        return NULL;
    }

    Image8bpp *img = (Image8bpp *)malloc(sizeof(Image8bpp));
    if (!img) return NULL;

    img->width = hdr->width;
    img->height = hdr->height;
    memcpy(img->palette.colors, hdr->palette, sizeof(hdr->palette));

    img->pixels = (uint8_t *)malloc(expected_pixels);
    if (!img->pixels) {
        free(img);
        return NULL;
    }

    memcpy(img->pixels, data + sizeof(PicHeader), expected_pixels);
    return img;
}

Image8bpp *Pic_LoadFromFile(const char *filepath) {
    FILE *f = fopen(filepath, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (sz < (long)sizeof(PicHeader)) {
        fclose(f);
        return NULL;
    }

    uint8_t *buffer = (uint8_t *)malloc(sz);
    if (!buffer) {
        fclose(f);
        return NULL;
    }

    size_t read_bytes = fread(buffer, 1, sz, f);
    fclose(f);

    if (read_bytes != (size_t)sz) {
        free(buffer);
        return NULL;
    }

    Image8bpp *img = Pic_LoadFromMemory(buffer, sz);
    free(buffer);
    return img;
}

void Pic_Free(Image8bpp *image) {
    if (image) {
        if (image->pixels) {
            free(image->pixels);
        }
        free(image);
    }
}

void Image8bpp_ToRGBA32(const Image8bpp *src, uint32_t *dst_pixels) {
    if (!src || !src->pixels || !dst_pixels) return;

    size_t count = (size_t)src->width * (size_t)src->height;
    for (size_t i = 0; i < count; ++i) {
        uint8_t idx = src->pixels[i];
        const ColorRGB *c = &src->palette.colors[idx];
        // 0xAA BB GG RR (little-endian RGBA32)
        dst_pixels[i] = ((uint32_t)255 << 24) |
                        ((uint32_t)c->b << 16) |
                        ((uint32_t)c->g << 8)  |
                        ((uint32_t)c->r);
    }
}

void Image8bpp_ToBGRA32(const Image8bpp *src, uint32_t *dst_pixels) {
    if (!src || !src->pixels || !dst_pixels) return;

    size_t count = (size_t)src->width * (size_t)src->height;
    for (size_t i = 0; i < count; ++i) {
        uint8_t idx = src->pixels[i];
        const ColorRGB *c = &src->palette.colors[idx];
        // 0xAA RR GG BB (little-endian BGRA32)
        dst_pixels[i] = ((uint32_t)255 << 24) |
                        ((uint32_t)c->r << 16) |
                        ((uint32_t)c->g << 8)  |
                        ((uint32_t)c->b);
    }
}
