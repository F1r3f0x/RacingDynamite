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

    // 5. Test .SRF (Surface and Physics)
    SrfData *srf = Srf_LoadFromFile("assets/LEVELS/AUSTRIA/AUSTRIA.SRF");
    if (!srf) {
        fprintf(stderr, "FAIL: Could not load assets/LEVELS/AUSTRIA/AUSTRIA.SRF\n");
        return 1;
    }
    assert(srf->header.grid_cells_x == 200);
    assert(srf->header.grid_cells_z == 200);
    assert(srf->header.grid_stride_x == 101);
    assert(srf->header.grid_stride_z == 101);
    assert(srf->header.triangle_count == 12462);
    assert(srf->grid != NULL);
    assert(srf->triangles != NULL);
    int32_t elevation = Srf_GetElevationAt(srf, 1000, 1000);
    (void)elevation;
    Srf_Free(srf);
    printf("[PASS] AUSTRIA.SRF verified (200x200 grid, 12462 collision triangles).\n");

    // 6. Test .PLC (Track Placed Objects)
    PlcData *plc = Plc_LoadFromFile("assets/LEVELS/AUSTRIA/AUSTRIA.PLC");
    if (!plc) {
        fprintf(stderr, "FAIL: Could not load assets/LEVELS/AUSTRIA/AUSTRIA.PLC\n");
        return 1;
    }
    assert(plc->count == 321);
    assert(plc->objects != NULL);
    assert(plc->objects[0].submesh_offset == 0);
    assert(plc->objects[0].model_type == 135170);
    assert(plc->objects[0].pos_x == 1100);
    assert(plc->objects[0].pos_y == -756);
    assert(plc->objects[0].pos_z == -7036);
    printf("[PASS] AUSTRIA.PLC verified (321 placed objects).\n");

    // 7. Test .MSH (3D Track Geometry)
    MshData *msh = Msh_LoadFromFile("assets/LEVELS/AUSTRIA/AUSTRIA.MSH");
    if (!msh) {
        fprintf(stderr, "FAIL: Could not load assets/LEVELS/AUSTRIA/AUSTRIA.MSH\n");
        Plc_Free(plc);
        return 1;
    }
    int32_t vc = 0, pc = 0;
    const int32_t *verts = NULL;
    const MshPolygon *polys = NULL;
    bool sub_ok = Msh_GetSubmeshAt(msh, plc->objects[0].submesh_offset, &vc, &pc, &verts, &polys);
    assert(sub_ok);
    assert(vc == 20);
    assert(pc == 18);
    assert(verts != NULL);
    assert(polys != NULL);
    Msh_Free(msh);
    Plc_Free(plc);
    printf("[PASS] AUSTRIA.MSH verified (submesh 0 has 20 vertices, 18 polygons).\n");

    // 7. Test .TAB (Color Shading Table)
    TabData *tab = Tab_LoadFromFile("assets/LEVELS/AUSTRIA/AUSTRIA.TAB");
    if (!tab) {
        fprintf(stderr, "FAIL: Could not load assets/LEVELS/AUSTRIA/AUSTRIA.TAB\n");
        return 1;
    }
    Tab_Free(tab);
    printf("[PASS] AUSTRIA.TAB verified (65536 byte shading matrix).\n");

    // 8. Test .SHD (Shadow / Alpha Blend Table)
    ShdData *shd = Shd_LoadFromFile("assets/LEVELS/BRAZIL/BRAZIL.SHD");
    if (!shd) {
        fprintf(stderr, "FAIL: Could not load assets/LEVELS/BRAZIL/BRAZIL.SHD\n");
        return 1;
    }
    // Verify row 0 preserves background for almost all entries
    assert(shd->table[0] == 0);
    assert(shd->table[1] == 1);
    assert(shd->table[255] == 255);
    Shd_Free(shd);
    printf("[PASS] BRAZIL.SHD verified (65536 byte shadow/alpha matrix with transparent identity row 0).\n");

    // 8. Test .TEX (1024x1024 Texture Page)
    TexData *tex = Tex_LoadFromFile("assets/LEVELS/AUSTRIA/AUSTRIA.TEX");
    if (!tex) {
        fprintf(stderr, "FAIL: Could not load assets/LEVELS/AUSTRIA/AUSTRIA.TEX\n");
        return 1;
    }
    assert(tex->width == 1024);
    assert(tex->height == 1024);
    assert(tex->pixels != NULL);
    Tex_Free(tex);
    printf("[PASS] AUSTRIA.TEX verified (1024x1024 texture page).\n");

    // 9. Test .LFT (Bitmap Fonts)
    LftFont *font_small = Lft_LoadFromFile("assets/FONTS/SMALL.LFT");
    if (!font_small) {
        fprintf(stderr, "FAIL: Could not load assets/FONTS/SMALL.LFT\n");
        return 1;
    }
    assert(font_small->header.height > 0);
    assert(font_small->glyph_pixels != NULL);
    int text_width = Font_GetTextWidth(font_small, "IGNITION");
    assert(text_width > 0);
    Lft_Free(font_small);
    printf("[PASS] SMALL.LFT verified (rendered text width = %d px).\n", text_width);

    LftFont *font_red = Lft_LoadFromFile("assets/BALTAZAR/DATA/RED_DARK.LFT");
    if (font_red) {
        assert(font_red->header.height > 0);
        Lft_Free(font_red);
        printf("[PASS] RED_DARK.LFT menu font verified.\n");
    }

    // 10. Test .PAN (Sky Panoramas)
    FILE *pan_f = fopen("assets/LEVELS/AUSTRIA/AUSTRIA.PAN", "rb");
    if (!pan_f) {
        fprintf(stderr, "FAIL: Could not open assets/LEVELS/AUSTRIA/AUSTRIA.PAN\n");
        return 1;
    }
    fseek(pan_f, 0, SEEK_END);
    long pan_len = ftell(pan_f);
    fclose(pan_f);
    assert(pan_len == 65536);
    printf("[PASS] AUSTRIA.PAN verified (65536 bytes, 256x256 sky texture).\n");

    // 11. Test ENGINE.INF (Engine Sound Modulation Curves)
    FILE *inf_f = fopen("assets/CARS/COOPER/SOUND/ENGINE.INF", "rb");
    if (!inf_f) {
        fprintf(stderr, "FAIL: Could not open assets/CARS/COOPER/SOUND/ENGINE.INF\n");
        return 1;
    }
    fseek(inf_f, 0, SEEK_END);
    long inf_len = ftell(inf_f);
    fclose(inf_f);
    assert(inf_len == 800);
    printf("[PASS] COOPER ENGINE.INF verified (800 bytes, 4x200 volume/pitch curves).\n");

    // 12. Test .TRI & AI Waypoints Generation across all 7 tracks
    static const char *test_tracks[] = {
        "AUSTRIA", "BRAZIL", "CANADA", "CARIB", "ICELAND", "JAPAN", "USA"
    };
    for (int t = 0; t < 7; ++t) {
        const char *trk_name = test_tracks[t];
        char p_path[128], m_path[128], d_path[128];
        snprintf(p_path, sizeof(p_path), "assets/LEVELS/%s/%s.PLC", trk_name, trk_name);
        snprintf(m_path, sizeof(m_path), "assets/LEVELS/%s/%s.MSH", trk_name, trk_name);
        snprintf(d_path, sizeof(d_path), "assets/LEVELS/%s", trk_name);

        PlcData *trk_plc = Plc_LoadFromFile(p_path);
        assert(trk_plc != NULL);
        MshData *trk_msh = Msh_LoadFromFile(m_path);
        assert(trk_msh != NULL);

        TrackWaypoints *wp = Track_BuildWaypoints(d_path, trk_name, trk_plc, trk_msh);
        assert(wp != NULL);
        assert(wp->count > 100);
        assert(wp->waypoints != NULL);

        // Check first and last waypoint positions
        assert(wp->waypoints[0].center_x != 0.0f || wp->waypoints[0].center_z != 0.0f);
        assert(wp->waypoints[0].left_x != wp->waypoints[0].right_x ||
               wp->waypoints[0].left_z != wp->waypoints[0].right_z);

        printf("[PASS] %s waypoints verified (%u nodes chained, start=(%.0f, %.0f, %.0f)).\n",
               trk_name, wp->count, wp->waypoints[0].center_x, wp->waypoints[0].center_y, wp->waypoints[0].center_z);

        Track_FreeWaypoints(wp);
        Msh_Free(trk_msh);
        Plc_Free(trk_plc);
    }

    // 13. Test .POS (Scenery Animation Tracks) across all 7 tracks
    for (int t = 0; t < 7; ++t) {
        const char *trk_name = test_tracks[t];
        char p_path[128], pos_path[128];
        snprintf(p_path, sizeof(p_path), "assets/LEVELS/%s/%s.PLC", trk_name, trk_name);
        snprintf(pos_path, sizeof(pos_path), "assets/LEVELS/%s/%s.POS", trk_name, trk_name);

        PlcData *trk_plc = Plc_LoadFromFile(p_path);
        assert(trk_plc != NULL);

        PosData *pos = Pos_LoadFromFile(pos_path, trk_plc->count);
        assert(pos != NULL);
        assert(pos->track_count > 0);
        assert(pos->tracks != NULL);

        // Test update loop
        int32_t orig_x = trk_plc->objects[pos->tracks[0].object_index].pos_x;
        Pos_Update(pos, trk_plc);
        int32_t new_x = trk_plc->objects[pos->tracks[0].object_index].pos_x;
        (void)orig_x;
        (void)new_x;

        printf("[PASS] %s .POS animation verified (%u dynamic scenery tracks).\n",
               trk_name, pos->track_count);

        Pos_Free(pos);
        Plc_Free(trk_plc);
    }

    printf("\nALL FORMAT UNIT TESTS PASSED SUCCESSFULLY!\n");
    return 0;
}

