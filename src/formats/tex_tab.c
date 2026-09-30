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

TabData *Tab_LoadFromFile(const char *filepath) {
    if (!filepath) return NULL;

    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        fprintf(stderr, "[TAB] Error opening file: %s\n", filepath);
        return NULL;
    }

    TabData *tab = (TabData *)malloc(sizeof(TabData));
    if (!tab) {
        fclose(fp);
        return NULL;
    }

    if (fread(tab->table, 1, sizeof(tab->table), fp) != sizeof(tab->table)) {
        fprintf(stderr, "[TAB] Error reading 64KB table: %s\n", filepath);
        fclose(fp);
        free(tab);
        return NULL;
    }

    fclose(fp);
    return tab;
}

void Tab_Free(TabData *tab) {
    if (!tab) return;
    free(tab);
}

ShdData *Shd_LoadFromFile(const char *filepath) {
    if (!filepath) return NULL;

    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        fprintf(stderr, "[SHD] Error opening file: %s\n", filepath);
        return NULL;
    }

    ShdData *shd = (ShdData *)malloc(sizeof(ShdData));
    if (!shd) {
        fclose(fp);
        return NULL;
    }

    if (fread(shd->table, 1, sizeof(shd->table), fp) != sizeof(shd->table)) {
        fprintf(stderr, "[SHD] Error reading 64KB table: %s\n", filepath);
        fclose(fp);
        free(shd);
        return NULL;
    }

    fclose(fp);
    return shd;
}

void Shd_Free(ShdData *shd) {
    if (!shd) return;
    free(shd);
}

/**
 * @brief Loads 1MB track .TEX, car .TEX, and 64KB aligned texture pages into memory.
 * @original FUN_00419d10 (MAINDOS_32BIT.EXE @ 0x00419d10, main.c)
 * @fidelity ADAPTED
 */
TexData *Tex_LoadFromFile(const char *filepath) {
    if (!filepath) return NULL;

    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        fprintf(stderr, "[TEX] Error opening file: %s\n", filepath);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (file_size <= 0) {
        fclose(fp);
        return NULL;
    }

    // Authentic MAINDOS_32BIT.EXE (Texture_LoadAllPages 0x00419d10):
    // Buffer size is rounded up to 64KB multiples: (file_size + 0xFFFF) & ~0xFFFF.
    // Minimum buffer is 16 pages (1MB = 1,048,576 bytes).
    size_t alloc_size = (size_t)((file_size + 0xFFFF) & ~0xFFFF);
    if (alloc_size < 1048576) {
        alloc_size = 1048576;
    }

    TexData *tex = (TexData *)malloc(sizeof(TexData));
    if (!tex) {
        fclose(fp);
        return NULL;
    }

    tex->width = 1024;
    tex->height = 1024;
    tex->pixels = (uint8_t *)calloc(1, alloc_size);
    if (!tex->pixels) {
        fclose(fp);
        free(tex);
        return NULL;
    }

    size_t bytes_read = fread(tex->pixels, 1, (size_t)file_size, fp);
    if (bytes_read != (size_t)file_size) {
        fprintf(stderr, "[TEX] Warning: read %zu of %ld bytes from %s\n", bytes_read, file_size, filepath);
    }

    fclose(fp);
    return tex;
}

void Tex_Free(TexData *tex) {
    if (!tex) return;
    if (tex->pixels) free(tex->pixels);
    free(tex);
}
