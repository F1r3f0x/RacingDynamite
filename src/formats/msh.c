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
 * @brief Loads .MSH 3D submesh geometry pools for tracks and car models.
 * @original FUN_00419bd0 (MAINDOS_32BIT.EXE @ 0x00419bd0, main.c)
 * @fidelity ADAPTED
 */
MshData *Msh_LoadFromFile(const char *filepath) {
    if (!filepath) return NULL;

    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        fprintf(stderr, "[MSH] Error opening file: %s\n", filepath);
        return NULL;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return NULL;
    }

    long file_size = ftell(fp);
    if (file_size <= 0) {
        fclose(fp);
        return NULL;
    }
    rewind(fp);

    MshData *msh = (MshData *)malloc(sizeof(MshData));
    if (!msh) {
        fclose(fp);
        return NULL;
    }

    msh->raw_size = (size_t)file_size;
    msh->raw_data = (uint8_t *)malloc(msh->raw_size);
    if (!msh->raw_data) {
        fclose(fp);
        free(msh);
        return NULL;
    }

    if (fread(msh->raw_data, 1, msh->raw_size, fp) != msh->raw_size) {
        fprintf(stderr, "[MSH] Incomplete read of %s\n", filepath);
        fclose(fp);
        Msh_Free(msh);
        return NULL;
    }

    fclose(fp);
    return msh;
}

void Msh_Free(MshData *msh) {
    if (!msh) return;
    if (msh->raw_data) {
        free(msh->raw_data);
        msh->raw_data = NULL;
    }
    free(msh);
}

bool Msh_GetSubmeshAt(const MshData *msh, int32_t dword_offset,
                      int32_t *out_v_count, int32_t *out_p_count,
                      const int32_t **out_vertices, const MshPolygon **out_polygons) {
    if (!msh || !msh->raw_data || dword_offset < 0) return false;

    size_t byte_offset = (size_t)dword_offset * 4;
    if (byte_offset + 8 > msh->raw_size) return false;

    const int32_t *header = (const int32_t *)(msh->raw_data + byte_offset);
    int32_t v_count = header[0];
    int32_t p_count = header[1];

    if (v_count < 0 || v_count > 10000 || p_count < 0 || p_count > 10000) {
        return false;
    }

    size_t verts_size = (size_t)v_count * 12;
    size_t polys_offset = byte_offset + 8 + verts_size;
    size_t polys_size = (size_t)p_count * sizeof(MshPolygon); // 44 bytes each

    if (polys_offset + polys_size > msh->raw_size) {
        return false;
    }

    if (out_v_count) *out_v_count = v_count;
    if (out_p_count) *out_p_count = p_count;
    if (out_vertices) *out_vertices = (const int32_t *)(msh->raw_data + byte_offset + 8);
    if (out_polygons) *out_polygons = (const MshPolygon *)(msh->raw_data + polys_offset);

    return true;
}
