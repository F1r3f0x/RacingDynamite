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

#include "ignition/tri.h"
#include "ignition/formats.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define TRI_CHUNK_SIZE 500

typedef struct {
    float cx, cy, cz;
    uint32_t type;
    int32_t px, py, pz;
    int32_t submesh_offset;
} RoadChunkInfo;

/**
 * @brief Loads .TRI chunk indices, constructs left/right road boundary splines and AI waypoints.
 * @original FUN_00414e40 (MAINDOS.EXE @ 0x00414e40, main.c)
 * @fidelity ADAPTED
 */
TrackWaypoints *Track_BuildWaypoints(const char *track_dir, const char *track_name,
                                     const PlcData *plc, const MshData *msh) {
    if (!track_name || !plc || !msh || plc->count == 0) return NULL;

    char tri_path[256];
    if (track_dir && track_dir[0] != '\0') {
        snprintf(tri_path, sizeof(tri_path), "%s/%s.TRI", track_dir, track_name);
    } else {
        snprintf(tri_path, sizeof(tri_path), "assets/LEVELS/%s/%s.TRI", track_name, track_name);
    }

    FILE *fp = fopen(tri_path, "rb");
    if (!fp) {
        fprintf(stderr, "[TRI] Failed to open track spline file: %s\n", tri_path);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long file_len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (file_len < 4) {
        fclose(fp);
        return NULL;
    }

    uint8_t *tri_data = (uint8_t *)malloc(file_len);
    if (!tri_data) {
        fclose(fp);
        return NULL;
    }

    if (fread(tri_data, 1, file_len, fp) != (size_t)file_len) {
        free(tri_data);
        fclose(fp);
        return NULL;
    }
    fclose(fp);

    uint32_t tri_chunks = *(uint32_t *)tri_data;
    uint32_t max_chunks = (plc->count < tri_chunks) ? plc->count : tri_chunks;

    // Allocate array to analyze PLC road chunk positions and centroids
    RoadChunkInfo *chunks = (RoadChunkInfo *)calloc(plc->count, sizeof(RoadChunkInfo));
    if (!chunks) {
        free(tri_data);
        return NULL;
    }

    for (uint32_t i = 0; i < plc->count; ++i) {
        const PlcObject *obj = &plc->objects[i];
        chunks[i].type = (uint32_t)obj->model_type & 0xFFF;
        chunks[i].px = obj->pos_x;
        chunks[i].py = obj->pos_y;
        chunks[i].pz = obj->pos_z;
        chunks[i].submesh_offset = obj->submesh_offset;

        int32_t v_count = 0, p_count = 0;
        const int32_t *verts = NULL;
        const MshPolygon *polys = NULL;

        if (Msh_GetSubmeshAt(msh, obj->submesh_offset, &v_count, &p_count, &verts, &polys) && v_count > 0) {
            int64_t sx = 0, sy = 0, sz = 0;
            for (int32_t vi = 0; vi < v_count; ++vi) {
                sx += verts[vi * 3 + 0];
                sy += verts[vi * 3 + 1];
                sz += verts[vi * 3 + 2];
            }
            chunks[i].cx = (float)(sx / v_count + obj->pos_x);
            chunks[i].cy = (float)(sy / v_count + obj->pos_y);
            chunks[i].cz = (float)(sz / v_count + obj->pos_z);
        } else {
            chunks[i].cx = (float)obj->pos_x;
            chunks[i].cy = (float)obj->pos_y;
            chunks[i].cz = (float)obj->pos_z;
        }
    }

    // Identify start chunk (type 0) and first forward chunk (type 1)
    int start_idx = -1;
    int first_idx = -1;
    for (uint32_t i = 0; i < plc->count; ++i) {
        if (chunks[i].type == 0 && start_idx < 0) start_idx = (int)i;
        else if (chunks[i].type == 1 && first_idx < 0) first_idx = (int)i;
    }

    // Fallback if type 0 / 1 not explicitly present
    if (start_idx < 0) {
        for (uint32_t i = 0; i < plc->count; ++i) {
            if (chunks[i].type < 50) { start_idx = (int)i; break; }
        }
    }
    if (first_idx < 0) {
        first_idx = start_idx;
    }

    if (start_idx < 0) {
        free(chunks);
        free(tri_data);
        return NULL;
    }

    // Chain road chunks using nearest-neighbor centroid connectivity
    bool *visited = (bool *)calloc(plc->count, sizeof(bool));
    uint32_t *chain = (uint32_t *)calloc(plc->count, sizeof(uint32_t));
    uint32_t chain_len = 0;

    chain[chain_len++] = (uint32_t)start_idx;
    visited[start_idx] = true;

    if (first_idx != start_idx) {
        chain[chain_len++] = (uint32_t)first_idx;
        visited[first_idx] = true;
    }

    uint32_t curr = chain[chain_len - 1];

    while (chain_len < plc->count) {
        float best_dist = 1e12f;
        int best_cand = -1;
        float cx = chunks[curr].cx;
        float cy = chunks[curr].cy;
        float cz = chunks[curr].cz;

        for (uint32_t i = 0; i < plc->count; ++i) {
            if (visited[i]) continue;
            // Only connect road surface segments (type < 50)
            if (chunks[i].type < 50) {
                float dx = chunks[i].cx - cx;
                float dy = (chunks[i].cy - cy) * 2.0f; // Penalize elevation difference
                float dz = chunks[i].cz - cz;
                float dist = sqrtf(dx * dx + dy * dy + dz * dz);

                if (dist < best_dist) {
                    best_dist = dist;
                    best_cand = (int)i;
                }
            }
        }

        // Road chunk step limit (max span ~6000 units)
        if (best_cand < 0 || best_dist > 6000.0f) {
            break;
        }

        chain[chain_len++] = (uint32_t)best_cand;
        visited[best_cand] = true;
        curr = (uint32_t)best_cand;
    }

    free(visited);

    if (chain_len == 0) {
        free(chain);
        free(chunks);
        free(tri_data);
        return NULL;
    }

    // Build TrackWaypoint array from chained chunks and .TRI vertex records
    TrackWaypoints *result = (TrackWaypoints *)calloc(1, sizeof(TrackWaypoints));
    result->waypoints = (TrackWaypoint *)calloc(chain_len, sizeof(TrackWaypoint));
    result->count = chain_len;

    for (uint32_t i = 0; i < chain_len; ++i) {
        uint32_t c_idx = chain[i];
        TrackWaypoint *wp = &result->waypoints[i];
        wp->chunk_idx = c_idx;

        if (c_idx < max_chunks) {
            size_t tri_offset = 4 + (size_t)c_idx * TRI_CHUNK_SIZE;
            if (tri_offset + TRI_CHUNK_SIZE <= (size_t)file_len) {
                const uint8_t *rec = tri_data + tri_offset;
                int16_t s1 = *(const int16_t *)(rec + 1);
                int16_t s23 = *(const int16_t *)(rec + 23);
                wp->flag = rec[109];

                int32_t v_count = 0, p_count = 0;
                const int32_t *verts = NULL;
                const MshPolygon *polys = NULL;

                if (Msh_GetSubmeshAt(msh, chunks[c_idx].submesh_offset, &v_count, &p_count, &verts, &polys)) {
                    if (s1 >= 0 && s1 < v_count && s23 >= 0 && s23 < v_count) {
                        float px = (float)chunks[c_idx].px;
                        float py = (float)chunks[c_idx].py;
                        float pz = (float)chunks[c_idx].pz;

                        wp->left_x = (float)verts[s1 * 3 + 0] + px;
                        wp->left_y = (float)verts[s1 * 3 + 1] + py;
                        wp->left_z = (float)verts[s1 * 3 + 2] + pz;

                        wp->right_x = (float)verts[s23 * 3 + 0] + px;
                        wp->right_y = (float)verts[s23 * 3 + 1] + py;
                        wp->right_z = (float)verts[s23 * 3 + 2] + pz;

                        wp->center_x = (wp->left_x + wp->right_x) * 0.5f;
                        wp->center_y = (wp->left_y + wp->right_y) * 0.5f;
                        wp->center_z = (wp->left_z + wp->right_z) * 0.5f;
                        continue;
                    }
                }
            }
        }

        // Fallback if TRI vertex index lookup not available for this chunk
        wp->center_x = chunks[c_idx].cx;
        wp->center_y = chunks[c_idx].cy;
        wp->center_z = chunks[c_idx].cz;
        wp->left_x = wp->center_x - 150.0f;
        wp->left_y = wp->center_y;
        wp->left_z = wp->center_z;
        wp->right_x = wp->center_x + 150.0f;
        wp->right_y = wp->center_y;
        wp->right_z = wp->center_z;
    }

    // Compute heading tangents
    for (uint32_t i = 0; i < chain_len; ++i) {
        uint32_t next_i = (i + 1 < chain_len) ? i + 1 : 0;
        float dx = result->waypoints[next_i].center_x - result->waypoints[i].center_x;
        float dz = result->waypoints[next_i].center_z - result->waypoints[i].center_z;
        result->waypoints[i].heading = atan2f(dx, dz);
    }

    free(chain);
    free(chunks);
    free(tri_data);

    printf("[TRI] Successfully loaded track spline: %u waypoints chained for %s\n",
           result->count, track_name);
    return result;
}

void Track_FreeWaypoints(TrackWaypoints *wp) {
    if (!wp) return;
    if (wp->waypoints) {
        free(wp->waypoints);
        wp->waypoints = NULL;
    }
    free(wp);
}
