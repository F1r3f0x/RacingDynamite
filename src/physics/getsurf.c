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

#include "ignition/physics.h"
#include "ignition/log.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Material friction lookup table matching DAT_004972d0 from MAINDOS_32BIT.EXE.
 * @original DAT_004972d0 (MAINDOS_32BIT.EXE @ 0x004972d0, getsurf.c)
 * @fidelity EXACT
 */
static double GetMaterialFriction(int32_t material_id) {
    if (material_id == 3) {
        return 1.20; // Turbo boost pad
    }
    if (material_id >= 10 && material_id <= 19) {
        return 0.75; // Gravel / Dirt road
    }
    if (material_id >= 20 && material_id <= 29) {
        return 0.70; // Rough off-road
    }
    if (material_id >= 30 && material_id <= 39) {
        return 0.40; // Grass / soft terrain
    }
    if (material_id >= 40 && material_id <= 49) {
        return 0.18; // Ice / slippery snow
    }
    if (material_id >= 100) {
        return 0.30; // Barrier / hazard
    }
    return 1.00; // Default tarmac / road asphalt
}

/**
 * @brief Filter out non-mesh gameplay markers to match authentic .SRF object indexing.
 * @original FUN_0041b360 (MAINDOS_32BIT.EXE @ 0x0041b360, main.c)
 * @fidelity ADAPTED
 */
static inline bool IsGeometryMeshObject(int32_t model_type) {
    uint8_t base = (uint8_t)(model_type & 0xff);
    if (base == 0xc8 || base == 0x9a || (base >= 0x90 && base <= 0x98) || base == 0x62 || base == 0x2c) {
        return false;
    }
    if (model_type >= 150 && model_type <= 500) {
        return false;
    }
    return true;
}

/**
 * @brief Raycasts world coordinate (X, Z) against track surface collision grid.
 * @original FUN_00412fc0 (MAINDOS_32BIT.EXE @ 0x00412fc0, getsurf.c)
 * @original FUN_00413380 (MAINDOS_32BIT.EXE @ 0x00413380, getsurf.c)
 * @original FUN_00446578 (MAINDOS_32BIT.EXE @ 0x00446578, getsurf.c)
 * @original FUN_004465e1 (MAINDOS_32BIT.EXE @ 0x004465e1, getsurf.c)
 * @fidelity EXTENDED
 * @deviation DEV-001 (Boundary safety clamp preventing off-track crash and fall-through)
 * @fix_category FIX_CAT_NOCLIP
 * @notes Uses 50 * 512 unit coordinate offset matching DAT_00479c48.
 */
bool Surface_Raycast(const SrfData *srf, const PlcData *plc, const MshData *msh,
                     double world_x, double world_z, double ref_y,
                     SurfaceRaycastResult *result, const GameFixOptions *fixes) {
    if (!result) return false;
    memset(result, 0, sizeof(SurfaceRaycastResult));
    result->normal_y = -1.0;
    result->friction = 1.0;

    if (!srf || !srf->grid || !srf->triangles) {
        result->elevation = ref_y;
        return false;
    }

    // Authentic coordinate transformation: grid origin is offset by 25,600 (50 * 512)
    double raw_x = world_x + PHYSICS_WORLD_OFFSET;
    double raw_z = world_z + PHYSICS_WORLD_OFFSET;

    int32_t cx = (int32_t)floor(raw_x / 512.0);
    int32_t cz = (int32_t)floor(raw_z / 512.0);

    bool fix_noclip = (!fixes || fixes->fix_noclip);

    if (fix_noclip) {
        if (cx < 0 || cx >= srf->header.grid_stride_x ||
            cz < 0 || cz >= srf->header.grid_stride_z) {
            LOG_DEBUG("SURFACE", "Raycast out of bounds: world=(%.1f, %.1f) cell=(%d, %d)", world_x, world_z, cx, cz);
            result->elevation = ref_y;
            result->is_kill_plane = true;
            return false;
        }
    } else {
        // Authentic 1997 Buggy: clamp cell only to buffer size to avoid host OS fault
        if (cx < 0) cx = 0;
        if (cz < 0) cz = 0;
        if (cx >= srf->header.grid_stride_x) cx = srf->header.grid_stride_x - 1;
        if (cz >= srf->header.grid_stride_z) cz = srf->header.grid_stride_z - 1;
    }

    size_t cell_idx = (size_t)cz * (size_t)srf->header.grid_stride_x + (size_t)cx;
    const SrfCell *cell = &srf->grid[cell_idx];

    // Confirmed by FUN_00412fc0 decompile: two separate uint16_t at +8 (table1_count) and +10 (table2_count)
    uint16_t count_t1 = cell->table1_count;
    uint16_t count_t2 = cell->table2_count;

    if (count_t1 == 0 && count_t2 == 0) {
        // Empty off-track cell
        LOG_DEBUG("SURFACE", "Raycast empty cell: world=(%.1f, %.1f) cell=(%d, %d)", world_x, world_z, cx, cz);
        result->elevation = ref_y;
        result->is_kill_plane = true;
        return false;
    }

    int32_t qx = (int32_t)raw_x;
    int32_t qz = (int32_t)raw_z;

    bool found_hit = false;
    double best_diff = 1e12;
    int32_t best_tri_idx = -1;
    double best_y = ref_y;
    double best_nx = 0.0, best_ny = -1.0, best_nz = 0.0;
    int32_t best_material = 0;
    double best_v0[3] = {0}, best_v1[3] = {0}, best_v2[3] = {0};

    // Iterate through both table2 (pass 0: dz >= 0) and table1 (pass 1: dz < 0) candidates
    for (int pass = 0; pass < 2; pass++) {
        uint16_t count = (pass == 0) ? count_t2 : count_t1;
        const int32_t *table = (pass == 0) ? srf->table2 : srf->table1;
        int32_t byte_offset = (pass == 0) ? cell->table2_offset : cell->table1_offset;
        int32_t table_max = (pass == 0) ? srf->header.table2_count : srf->header.table1_count;


        if (count == 0 || !table) continue;
        int32_t table_idx = byte_offset / 4;
        if (table_idx < 0 || table_idx + count > table_max) continue;

        for (uint16_t i = 0; i < count; i++) {
            int32_t tri_byte = table[table_idx + i];
            int32_t tri_idx = tri_byte / 24;
            if (tri_idx < 0 || tri_idx >= srf->header.triangle_count) continue;

            // SrfTriangle binary layout (24 bytes, trapezoid span format per structs.md):
            //   v0_y  = X base coordinate of trapezoid (base_x in fixed-point integer space)
            //   v1_y  = Z base coordinate of trapezoid (base_z)
            //   v2_y  = slope1: X edge delta per unit Z on one side (16.16 fixed-point scale)
            //   material_flags = slope2: X edge delta per unit Z on other side
            //   normal_x = packed dz (low 16 bits) and poly_dwords (high 16 bits)
            //   normal_z = PLC object vertex pointer (encoded as byte offset / 42)
            const SrfTriangle *tri = &srf->triangles[tri_idx];
            int32_t base_x = tri->v0_y;
            int32_t base_z = tri->v1_y;
            int32_t slope1 = tri->v2_y;
            int32_t slope2 = tri->material_flags;
            int32_t h_flags = tri->normal_x;
            int16_t dz = (int16_t)(h_flags & 0xffff);


            // 2D trapezoid / triangle span test matching FUN_00446578 / FUN_004465e1
            bool in_z = false;
            int32_t z_diff = qz - base_z;

            if (dz >= 0) {
                in_z = (qz >= base_z && qz <= base_z + dz);
            } else {
                in_z = (qz <= base_z && qz >= base_z + dz);
            }

            if (!in_z) continue;

            int32_t x_bound1 = base_x + (int32_t)(((int64_t)z_diff * slope1) >> 16);
            int32_t x_bound2 = base_x + (int32_t)(((int64_t)z_diff * slope2) >> 16);
            int32_t x_min = (x_bound1 < x_bound2) ? x_bound1 : x_bound2;
            int32_t x_max = (x_bound1 > x_bound2) ? x_bound1 : x_bound2;

            // Allow 2-unit tolerance at triangle seams
            if (qx < x_min - 2 || qx > x_max + 2) continue;

            // Inside candidate 2D footprint! Retrieve 3D vertices from MSH + PLC if available
            int32_t poly_dwords = (h_flags >> 16) & 0xffff;
            int32_t obj_index = tri->normal_z / 42;

            double V0[3], V1[3], V2[3];
            int32_t mat_id = 0;
            bool resolved_verts = false;

            if (plc && msh && msh->raw_data && plc->count > 0 && obj_index >= 0) {
                const PlcObject *obj = NULL;
                int32_t cur_obj = 0;

                for (uint32_t p = 0; p < plc->count; p++) {
                    if (IsGeometryMeshObject(plc->objects[p].model_type)) {
                        if (cur_obj == obj_index) {
                            obj = &plc->objects[p];
                            break;
                        }
                        cur_obj++;
                    }
                }

                if (obj) {
                    size_t sub_offset = (size_t)obj->submesh_offset * 4;
                    if (sub_offset + 8 <= msh->raw_size) {
                        const int32_t *submesh = (const int32_t *)(msh->raw_data + sub_offset);
                        int32_t v_count = submesh[0];
                        size_t poly_byte_offset = sub_offset + (size_t)poly_dwords * 4;

                        if (poly_byte_offset + 16 <= msh->raw_size) {
                            const int32_t *poly = (const int32_t *)(msh->raw_data + poly_byte_offset);
                            mat_id = (poly[0] >> 16) & 0xff;
                            int32_t v0_i = poly[1];
                            int32_t v1_i = poly[2];
                            int32_t v2_i = poly[3];

                            if (v0_i >= 0 && v0_i < v_count &&
                                v1_i >= 0 && v1_i < v_count &&
                                v2_i >= 0 && v2_i < v_count) {
                                const int32_t *v_base = submesh + 2;
                                double ox = (double)(obj->pos_x + 25600) - PHYSICS_WORLD_OFFSET;
                                double oy = (double)obj->pos_y;
                                double oz = (double)(obj->pos_z + 25600) - PHYSICS_WORLD_OFFSET;

                                V0[0] = ox + (double)v_base[v0_i * 3 + 0];
                                V0[1] = oy - (double)v_base[v0_i * 3 + 1];
                                V0[2] = oz + (double)v_base[v0_i * 3 + 2];

                                V1[0] = ox + (double)v_base[v1_i * 3 + 0];
                                V1[1] = oy - (double)v_base[v1_i * 3 + 1];
                                V1[2] = oz + (double)v_base[v1_i * 3 + 2];

                                V2[0] = ox + (double)v_base[v2_i * 3 + 0];
                                V2[1] = oy - (double)v_base[v2_i * 3 + 1];
                                V2[2] = oz + (double)v_base[v2_i * 3 + 2];

                                resolved_verts = true;
                            }
                        }
                    }
                }
            }

            if (!resolved_verts) {
                // Fallback geometry using base coordinates
                V0[0] = (double)base_x - PHYSICS_WORLD_OFFSET;
                V0[1] = ref_y;
                V0[2] = (double)base_z - PHYSICS_WORLD_OFFSET;

                V1[0] = (double)x_bound1 - PHYSICS_WORLD_OFFSET;
                V1[1] = ref_y;
                V1[2] = (double)(base_z + dz) - PHYSICS_WORLD_OFFSET;

                V2[0] = (double)x_bound2 - PHYSICS_WORLD_OFFSET;
                V2[1] = ref_y;
                V2[2] = (double)(base_z + dz) - PHYSICS_WORLD_OFFSET;
            }

            // Normal calculation from cross product: (V0 - V1) x (V2 - V1)
            double e0x = V0[0] - V1[0], e0y = V0[1] - V1[1], e0z = V0[2] - V1[2];
            double e1x = V2[0] - V1[0], e1y = V2[1] - V1[1], e1z = V2[2] - V1[2];

            double Nx = e0y * e1z - e0z * e1y;
            double Ny = e0z * e1x - e0x * e1z;
            double Nz = e0x * e1y - e0y * e1x;

            // Normalize vector
            double len = sqrt(Nx * Nx + Ny * Ny + Nz * Nz);
            if (len > 1e-6) {
                Nx /= len;
                Ny /= len;
                Nz /= len;
            } else {
                Nx = 0.0; Ny = 1.0; Nz = 0.0;
            }
            // Ensure surface normal points upward into the world
            if (Ny < 0.0) {
                Nx = -Nx;
                Ny = -Ny;
                Nz = -Nz;
            }

            // Authentic plane height formula:
            // Y = V0y + [(V1y - V0y)*Ny + Nx*((V1x - V0x) - (x - V0x)) + Nz*((V1z - V0z) - (z - V0z))] / Ny
            double y_surf = V0[1];
            if (Ny >= 0.20) {
                double num = (V1[1] - V0[1]) * Ny +
                             Nx * (V1[0] - world_x) +
                             Nz * (V1[2] - world_z);
                y_surf = V0[1] + num / Ny;
            } else {
                // For near-vertical walls (Ny < 0.20), avoid dividing by near-zero Ny
                y_surf = (V0[1] + V1[1] + V2[1]) / 3.0;
            }

            // Candidate selection: penalize steep walls (Ny < 0.25) and barrier materials (>= 80)
            // so drivable road surfaces are always prioritized over steep mountain walls
            double slope_penalty = 0.0;
            if (Ny < 0.25 || mat_id >= 80) {
                slope_penalty = 2000.0;
            }

            double diff = fabs(y_surf - ref_y) + slope_penalty;
            if (diff < best_diff) {
                best_diff = diff;
                best_tri_idx = tri_idx;
                best_y = y_surf;
                best_nx = Nx;
                best_ny = Ny;
                best_nz = Nz;
                best_material = mat_id;
                memcpy(best_v0, V0, sizeof(V0));
                memcpy(best_v1, V1, sizeof(V1));
                memcpy(best_v2, V2, sizeof(V2));
                found_hit = true;
            }
        }
    }

    if (found_hit) {
        result->hit = true;
        result->triangle_idx = best_tri_idx;
        result->elevation = best_y;
        result->normal_x = best_nx;
        result->normal_y = best_ny;
        result->normal_z = best_nz;
        result->material_id = best_material;
        result->friction = GetMaterialFriction(best_material);
        result->is_boost_pad = (best_material == 3);
        memcpy(result->v0, best_v0, sizeof(best_v0));
        memcpy(result->v1, best_v1, sizeof(best_v1));
        memcpy(result->v2, best_v2, sizeof(best_v2));
        return true;
    }

    // Default fallback if no candidate triangle hit
    LOG_DEBUG("SURFACE", "Raycast candidate miss: world=(%.1f, %.1f)", world_x, world_z);
    result->elevation = ref_y;
    result->normal_x = 0.0;
    result->normal_y = 1.0;
    result->normal_z = 0.0;
    result->friction = 0.80;
    result->hit = false;
    result->is_kill_plane = false;
    return false;
}
