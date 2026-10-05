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
 * @brief Loads .PLC scenery placement table and unpacks object archetypes.
 * @original FUN_00419a90 (MAINDOS.EXE @ 0x00419a90, main.c)
 * @original FUN_0041b360 (MAINDOS.EXE @ 0x0041b360, main.c)
 * @fidelity ADAPTED
 */
PlcData *Plc_LoadFromFile(const char *filepath) {
    if (!filepath) return NULL;

    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        fprintf(stderr, "[PLC] Error opening file: %s\n", filepath);
        return NULL;
    }

    uint32_t count = 0;
    if (fread(&count, sizeof(uint32_t), 1, fp) != 1) {
        fprintf(stderr, "[PLC] Error reading count: %s\n", filepath);
        fclose(fp);
        return NULL;
    }

    PlcData *plc = (PlcData *)malloc(sizeof(PlcData));
    if (!plc) {
        fclose(fp);
        return NULL;
    }

    plc->count = count;
    plc->objects = (PlcObject *)malloc((size_t)count * sizeof(PlcObject));
    if (!plc->objects) {
        fclose(fp);
        free(plc);
        return NULL;
    }

    if (fread(plc->objects, sizeof(PlcObject), count, fp) != count) {
        fprintf(stderr, "[PLC] Incomplete object table: %s\n", filepath);
        fclose(fp);
        Plc_Free(plc);
        return NULL;
    }

    fclose(fp);
    return plc;
}

void Plc_Free(PlcData *plc) {
    if (!plc) return;
    if (plc->objects) free(plc->objects);
    free(plc);
}
