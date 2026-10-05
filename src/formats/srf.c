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

/**
 * @brief Loads .SRF track surface collision grid, cell index tables, and triangle headers.
 * @original FUN_00412670 (MAINDOS.EXE @ 0x00412670, getsurf.c)
 * @fidelity ADAPTED
 */
SrfData *Srf_LoadFromFile(const char *filepath) {
    if (!filepath) return NULL;

    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        fprintf(stderr, "[SRF] Error opening file: %s\n", filepath);
        return NULL;
    }

    SrfData *srf = (SrfData *)calloc(1, sizeof(SrfData));
    if (!srf) {
        fclose(fp);
        return NULL;
    }

    if (fread(&srf->header, sizeof(SrfHeader), 1, fp) != 1) {
        fprintf(stderr, "[SRF] Error reading header: %s\n", filepath);
        fclose(fp);
        free(srf);
        return NULL;
    }

    size_t grid_cell_count = (size_t)srf->header.grid_stride_x * (size_t)srf->header.grid_stride_z;
    srf->grid = (SrfCell *)malloc(grid_cell_count * sizeof(SrfCell));
    if (!srf->grid) {
        fclose(fp);
        Srf_Free(srf);
        return NULL;
    }

    if (fread(srf->grid, sizeof(SrfCell), grid_cell_count, fp) != grid_cell_count) {
        fprintf(stderr, "[SRF] Error reading spatial grid cells: %s\n", filepath);
        fclose(fp);
        Srf_Free(srf);
        return NULL;
    }

    if (srf->header.triangle_count > 0) {
        srf->triangles = (SrfTriangle *)malloc((size_t)srf->header.triangle_count * sizeof(SrfTriangle));
        if (!srf->triangles) {
            fclose(fp);
            Srf_Free(srf);
            return NULL;
        }

        if (fread(srf->triangles, sizeof(SrfTriangle), (size_t)srf->header.triangle_count, fp) != (size_t)srf->header.triangle_count) {
            fprintf(stderr, "[SRF] Error reading triangle table: %s\n", filepath);
            fclose(fp);
            Srf_Free(srf);
            return NULL;
        }
    }

    if (srf->header.table1_count > 0) {
        srf->table1 = (int32_t *)malloc((size_t)srf->header.table1_count * sizeof(int32_t));
        if (srf->table1) {
            fread(srf->table1, sizeof(int32_t), (size_t)srf->header.table1_count, fp);
        }
    }

    if (srf->header.table2_count > 0) {
        srf->table2 = (int32_t *)malloc((size_t)srf->header.table2_count * sizeof(int32_t));
        if (srf->table2) {
            fread(srf->table2, sizeof(int32_t), (size_t)srf->header.table2_count, fp);
        }
    }

    fclose(fp);
    return srf;
}

/**
 * @brief Releases loaded .SRF track surface memory.
 * @original FUN_004127a0 (MAINDOS.EXE @ 0x004127a0, getsurf.c)
 * @fidelity ADAPTED
 */
void Srf_Free(SrfData *srf) {
    if (!srf) return;
    if (srf->grid) free(srf->grid);
    if (srf->triangles) free(srf->triangles);
    if (srf->table1) free(srf->table1);
    if (srf->table2) free(srf->table2);
    free(srf);
}

int32_t Srf_GetElevationAt(const SrfData *srf, int32_t world_x, int32_t world_z) {
    if (!srf || !srf->grid || !srf->triangles) return 0;

    int32_t cell_sz_x = srf->header.cell_size_x ? srf->header.cell_size_x : 512;
    int32_t cell_sz_z = srf->header.cell_size_z ? srf->header.cell_size_z : 512;

    // Authentic engine offset: world origin (0,0) is centered at (stride/2, stride/2)
    int32_t cx = (world_x / cell_sz_x) + (srf->header.grid_stride_x / 2);
    int32_t cz = (world_z / cell_sz_z) + (srf->header.grid_stride_z / 2);

    if (cx < 0) cx = 0;
    if (cz < 0) cz = 0;
    if (cx >= srf->header.grid_stride_x) cx = srf->header.grid_stride_x - 1;
    if (cz >= srf->header.grid_stride_z) cz = srf->header.grid_stride_z - 1;

    size_t cell_idx = (size_t)cz * (size_t)srf->header.grid_stride_x + (size_t)cx;
    const SrfCell *cell = &srf->grid[cell_idx];

    // SrfTriangle stores 2D trapezoid bounds (x_base, z_base, slopes);
    // true 3D elevation is evaluated in getsurf via PLC + MSH geometry.
    (void)cell;
    return 0;
}
