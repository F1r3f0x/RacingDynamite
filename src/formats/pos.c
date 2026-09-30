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
 * @brief Loads .POS scenery object animation tracks matching PLC objects.
 * @original Pos_InitAnimatedObjects (MAINDOS_32BIT.EXE @ 0x004356d0, lisa3d.c)
 * @fidelity ADAPTED
 */
PosData *Pos_LoadFromFile(const char *filepath, uint32_t plc_count) {
    if (!filepath || plc_count == 0) return NULL;

    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
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

    if ((size_t)file_size < (size_t)plc_count * sizeof(int32_t)) {
        free(raw_data);
        return NULL;
    }

    const int32_t *offset_table = (const int32_t *)raw_data;

    // Count animated objects
    uint32_t anim_count = 0;
    for (uint32_t i = 0; i < plc_count; ++i) {
        if (offset_table[i] != -1) {
            anim_count++;
        }
    }

    if (anim_count == 0) {
        free(raw_data);
        return NULL;
    }

    PosData *pos = (PosData *)calloc(1, sizeof(PosData));
    if (!pos) {
        free(raw_data);
        return NULL;
    }

    pos->tracks = (PosTrack *)calloc(anim_count, sizeof(PosTrack));
    if (!pos->tracks) {
        free(pos);
        free(raw_data);
        return NULL;
    }
    pos->track_count = anim_count;

    uint32_t track_idx = 0;
    for (uint32_t i = 0; i < plc_count; ++i) {
        int32_t rel_dword = offset_table[i];
        if (rel_dword == -1) continue;

        // Byte offset formula from authentic binary: (i + rel_dword) * 4
        size_t byte_off = (size_t)(i + rel_dword) * 4;
        if (byte_off + sizeof(PosTrackHeader) > (size_t)file_size) {
            continue;
        }

        const PosTrackHeader *hdr = (const PosTrackHeader *)(raw_data + byte_off);
        int32_t frame_count = hdr->frame_count;
        if (frame_count <= 0 || frame_count > 10000) {
            continue;
        }

        size_t keyframes_bytes = (size_t)frame_count * sizeof(PosKeyframe);
        if (byte_off + sizeof(PosTrackHeader) + keyframes_bytes > (size_t)file_size) {
            continue;
        }

        PosTrack *track = &pos->tracks[track_idx];
        track->object_index = i;
        track->frame_count = frame_count;
        track->current_frame = 0;
        track->keyframes = (PosKeyframe *)malloc(keyframes_bytes);
        if (track->keyframes) {
            memcpy(track->keyframes, raw_data + byte_off + sizeof(PosTrackHeader), keyframes_bytes);
            track_idx++;
        }
    }

    pos->track_count = track_idx;
    free(raw_data);
    return pos;
}

/**
 * @brief Releases loaded .POS animation tracks.
 * @fidelity INFRASTRUCTURE
 */
void Pos_Free(PosData *pos) {
    if (!pos) return;
    if (pos->tracks) {
        for (uint32_t i = 0; i < pos->track_count; ++i) {
            if (pos->tracks[i].keyframes) {
                free(pos->tracks[i].keyframes);
            }
        }
        free(pos->tracks);
    }
    free(pos);
}

/**
 * @brief Advances animated scenery keyframe playheads and updates PLC object coordinates.
 * @original Pos_UpdateAnimatedObjects (MAINDOS_32BIT.EXE @ 0x004357a0, lisa3d.c)
 * @fidelity ADAPTED
 */
void Pos_Update(PosData *pos, PlcData *plc) {
    if (!pos || !plc || !pos->tracks || !plc->objects) return;

    for (uint32_t t = 0; t < pos->track_count; ++t) {
        PosTrack *track = &pos->tracks[t];
        if (track->frame_count <= 0 || !track->keyframes) continue;
        if (track->object_index >= plc->count) continue;

        int32_t frame = track->current_frame;
        const PosKeyframe *kf = &track->keyframes[frame];

        PlcObject *obj = &plc->objects[track->object_index];
        obj->pos_x = kf->pos_x;
        obj->pos_y = kf->pos_y;
        obj->pos_z = kf->pos_z;

        // Advance playhead with seamless cyclic loop
        track->current_frame = (frame + 1) % track->frame_count;
    }
}
