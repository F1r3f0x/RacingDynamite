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

#define LFT_ENTRIES_COUNT 224
#define LFT_PIXELS_OFFSET 0x84C

/**
 * @brief Loads and parses .LFT/.FNT font headers, character widths, and raster glyphs.
 * @original FUN_00456270 (IGN_WIN.EXE @ 0x00456270, geputget.c)
 * @fidelity ADAPTED
 */
LftFont *Lft_LoadFromFile(const char *filepath) {
    if (!filepath) return NULL;

    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        fprintf(stderr, "[Font] Error opening file: %s\n", filepath);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (file_size < 32) {
        fprintf(stderr, "[Font] File too small: %s\n", filepath);
        fclose(fp);
        return NULL;
    }

    uint8_t *raw_data = (uint8_t *)malloc((size_t)file_size);
    if (!raw_data) {
        fclose(fp);
        return NULL;
    }

    if (fread(raw_data, 1, (size_t)file_size, fp) != (size_t)file_size) {
        free(raw_data);
        fclose(fp);
        return NULL;
    }
    fclose(fp);

    LftFont *font = (LftFont *)calloc(1, sizeof(LftFont));
    if (!font) {
        free(raw_data);
        return NULL;
    }

    // Check if this is an authentic Ignition .FNT file (e.g. IGNITION.FNT, 5400 bytes)
    // or has no "LFT" header magic.
    if (file_size == 5400 || memcmp(raw_data, "LFT", 3) != 0) {
        font->is_fnt = true;
        font->header.height = (uint16_t)(raw_data[2] | (raw_data[3] << 8));
        font->spacing = (int16_t)(raw_data[4] | (raw_data[5] << 8));
        if (font->spacing < 0) font->spacing = 1;

        // In IGNITION.FNT:
        // 0xC6: 256 bytes ASCII lookup table
        memcpy(font->ascii_map, raw_data + 0xC6, 256);

        // 0x06: 256 bytes character widths
        memcpy(font->widths, raw_data + 0x06, 256);

        // 0x1C6: 256 int32_t offsets
        for (int i = 0; i < 256; ++i) {
            int off = 0x1C6 + i * 4;
            font->offsets[i] = (int32_t)(raw_data[off] |
                                        (raw_data[off + 1] << 8) |
                                        (raw_data[off + 2] << 16) |
                                        (raw_data[off + 3] << 24));
        }

        // 0x546: Start of glyph pixel raster data
        if (file_size > 0x546) {
            font->glyph_data_size = (size_t)(file_size - 0x546);
            font->glyph_pixels = (uint8_t *)malloc(font->glyph_data_size);
            if (font->glyph_pixels) {
                memcpy(font->glyph_pixels, raw_data + 0x546, font->glyph_data_size);
            }
        }
    } else {
        // Standard .LFT font
        font->is_fnt = false;
        memcpy(&font->header, raw_data, sizeof(LftHeader));
        font->spacing = 1;

        // Default identity ASCII map
        for (int i = 0; i < 256; ++i) {
            font->ascii_map[i] = (uint8_t)i;
        }

        // 0x0C: 224 int32_t offsets
        for (int i = 0; i < LFT_ENTRIES_COUNT; ++i) {
            int off = 0x0C + i * 4;
            font->offsets[i] = (int32_t)(raw_data[off] |
                                        (raw_data[off + 1] << 8) |
                                        (raw_data[off + 2] << 16) |
                                        (raw_data[off + 3] << 24));
        }

        // 0x68C: 224 int16_t widths
        for (int i = 0; i < LFT_ENTRIES_COUNT; ++i) {
            int off = 0x68C + i * 2;
            int16_t w = (int16_t)(raw_data[off] | (raw_data[off + 1] << 8));
            font->widths[i] = (w > 0 && w < 255) ? (uint8_t)w : 0;
        }

        // 0x84C: Start of glyph pixels
        if (file_size > LFT_PIXELS_OFFSET) {
            font->glyph_data_size = (size_t)(file_size - LFT_PIXELS_OFFSET);
            font->glyph_pixels = (uint8_t *)malloc(font->glyph_data_size);
            if (font->glyph_pixels) {
                memcpy(font->glyph_pixels, raw_data + LFT_PIXELS_OFFSET, font->glyph_data_size);
            }
        }
    }

    free(raw_data);
    return font;
}

void Lft_Free(LftFont *font) {
    if (!font) return;
    if (font->glyph_pixels) free(font->glyph_pixels);
    free(font);
}

/**
 * @brief 2D bitmap font rasterizer blitting characters to 8bpp buffer.
 * @original FUN_004133d0 (IGN_WIN.EXE @ 0x004133d0, geputget.c)
 * @fidelity ADAPTED
 */
void Font_DrawText(uint8_t *framebuffer, int stride, const LftFont *font, int x, int y, const char *text, uint8_t color_offset) {
    if (!framebuffer || !font || !text) return;

    int cur_x = x;
    int height = font->header.height > 0 ? font->header.height : 10;
    int spacing = font->spacing > 0 ? font->spacing : 1;

    for (const char *p = text; *p != '\0'; ++p) {
        unsigned char c = (unsigned char)*p;

        if (c == ' ') {
            cur_x += 6;
            continue;
        }

        int w = 0;
        int32_t off = -1;

        if (font->is_fnt) {
            uint8_t g_idx = font->ascii_map[c];
            if (g_idx != 255) {
                w = font->widths[g_idx];
                off = font->offsets[g_idx];
            }
        } else {
            off = font->offsets[c];
            w = font->widths[c];
        }

        if (off < 0 || w <= 0 || (size_t)off >= font->glyph_data_size) {
            cur_x += 4;
            continue;
        }

        const uint8_t *glyph_src = font->glyph_pixels + off;

        for (int r = 0; r < height; ++r) {
            int dest_y = y + r;
            if (dest_y < 0) continue;

            for (int col = 0; col < w; ++col) {
                int dest_x = cur_x + col;
                if (dest_x < 0 || dest_x >= stride) continue;

                uint8_t pix = glyph_src[r * w + col];
                if (pix != 0) {
                    uint8_t final_color = pix;
                    if (color_offset != 0) {
                        // pix == 2 is body, pix == 1 is outline/shadow
                        if (pix == 2) {
                            final_color = color_offset;
                        } else if (pix == 1) {
                            final_color = 0; // Crisp black drop shadow
                        }
                    } else {
                        // Default white text
                        if (pix == 2) final_color = 251;
                        else if (pix == 1) final_color = 0;
                    }
                    framebuffer[dest_y * stride + dest_x] = final_color;
                }
            }
        }

        cur_x += w + spacing;
    }
}

int Font_GetTextWidth(const LftFont *font, const char *text) {
    if (!font || !text) return 0;

    int total_w = 0;
    int spacing = font->spacing > 0 ? font->spacing : 1;

    for (const char *p = text; *p != '\0'; ++p) {
        unsigned char c = (unsigned char)*p;
        if (c == ' ') {
            total_w += 6;
            continue;
        }

        int w = 0;
        int32_t off = -1;

        if (font->is_fnt) {
            uint8_t g_idx = font->ascii_map[c];
            if (g_idx != 255) {
                w = font->widths[g_idx];
                off = font->offsets[g_idx];
            }
        } else {
            off = font->offsets[c];
            w = font->widths[c];
        }

        if (off >= 0 && w > 0) {
            total_w += w + spacing;
        } else {
            total_w += 4;
        }
    }
    return total_w;
}
