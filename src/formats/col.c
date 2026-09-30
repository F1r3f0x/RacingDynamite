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

/**
 * @brief Parses and unpacks 256-color palette from memory buffer.
 * @original FUN_00456c40 (MAINDOS_32BIT.EXE @ 0x00456c40, lisa3d.c)
 * @fidelity ADAPTED
 */
bool Col_LoadFromMemory(const uint8_t *data, size_t size, Palette256 *out_palette) {
    if (!data || !out_palette || size < sizeof(ColHeader)) {
        return false;
    }

    const ColHeader *hdr = (const ColHeader *)data;
    memcpy(out_palette->colors, hdr->palette, sizeof(out_palette->colors));
    return true;
}

/**
 * @brief Loads 256-color palette from .COL file.
 * @original FUN_004574a0 (MAINDOS_32BIT.EXE @ 0x004574a0, mem.c)
 * @fidelity ADAPTED
 */
bool Col_LoadFromFile(const char *filepath, Palette256 *out_palette) {
    if (!filepath || !out_palette) return false;

    FILE *f = fopen(filepath, "rb");
    if (!f) return false;

    ColHeader hdr;
    size_t read_bytes = fread(&hdr, 1, sizeof(ColHeader), f);
    fclose(f);

    if (read_bytes != sizeof(ColHeader)) {
        return false;
    }

    memcpy(out_palette->colors, hdr.palette, sizeof(out_palette->colors));
    return true;
}
