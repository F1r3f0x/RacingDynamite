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

#include "ignition/types.h"
#include "ignition/formats.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(void) {
    printf("--- Running Ignition Format Tests ---\n");

    // 1. Test .COL loading
    Palette256 pal;
    bool ok = Col_LoadFromFile("assets/SYS.COL", &pal);
    if (!ok) {
        fprintf(stderr, "FAIL: Could not load assets/SYS.COL\n");
        return 1;
    }
    printf("[PASS] SYS.COL loaded successfully.\n");

    // 2. Test INSTALL.PIC (640x480)
    Image8bpp *install_pic = Pic_LoadFromFile("assets/INSTALL.PIC");
    if (!install_pic) {
        fprintf(stderr, "FAIL: Could not load assets/INSTALL.PIC\n");
        return 1;
    }
    assert(install_pic->width == 640);
    assert(install_pic->height == 480);
    assert(install_pic->pixels != NULL);
    printf("[PASS] INSTALL.PIC loaded: %ux%u.\n", install_pic->width, install_pic->height);

    // 3. Test conversion to RGBA32
    uint32_t *rgba = (uint32_t *)malloc(install_pic->width * install_pic->height * sizeof(uint32_t));
    assert(rgba != NULL);
    Image8bpp_ToRGBA32(install_pic, rgba);
    // Alpha must be 0xFF
    assert((rgba[0] >> 24) == 0xFF);
    free(rgba);
    Pic_Free(install_pic);
    printf("[PASS] Image8bpp_ToRGBA32 conversion verified.\n");

    // 4. Test all 7 Track preview .PIC files (320x200)
    const char *tracks[] = {
        "assets/LEVELS/AUSTRIA/AUSTRIA.PIC",
        "assets/LEVELS/BRAZIL/BRAZIL.PIC",
        "assets/LEVELS/CANADA/CANADA.PIC",
        "assets/LEVELS/CARIB/CARIB.PIC",
        "assets/LEVELS/ICELAND/ICELAND.PIC",
        "assets/LEVELS/JAPAN/JAPAN.PIC",
        "assets/LEVELS/USA/USA.PIC"
    };

    for (size_t i = 0; i < 7; ++i) {
        Image8bpp *trk = Pic_LoadFromFile(tracks[i]);
        if (!trk) {
            fprintf(stderr, "FAIL: Could not load track PIC: %s\n", tracks[i]);
            return 1;
        }
        assert(trk->width == 320);
        assert(trk->height == 200);
        assert(trk->pixels != NULL);
        Pic_Free(trk);
        printf("[PASS] Track PIC %s verified (320x200).\n", tracks[i]);
    }

    printf("\nALL FORMAT UNIT TESTS PASSED SUCCESSFULLY!\n");
    return 0;
}
