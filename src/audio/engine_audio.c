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

#include "ignition/audio.h"
#include "ignition/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/**
 * @brief Load ENGINE.INF curve tables and 00_A.WAV / 01_A.WAV for a car.
 * @original FUN_0041f9b0 (MAINDOS_32BIT.EXE @ 0x0041f9b0, main.c)
 * @fidelity ADAPTED
 */
bool EngineAudio_Init(EngineAudio *ea, const char *car_dir) {
    if (!ea || !car_dir) {
        return false;
    }

    memset(ea, 0, sizeof(EngineAudio));

    char path[512];
    snprintf(path, sizeof(path), "%s/ENGINE.INF", car_dir);

    FILE *f = fopen(path, "rb");
    if (!f) {
        // Fallback: search with lowercase
        snprintf(path, sizeof(path), "%s/engine.inf", car_dir);
        f = fopen(path, "rb");
    }

    if (!f) {
        LOG_WARN("AUDIO", "ENGINE.INF not found in '%s'", car_dir);
        return false;
    }

    uint8_t buffer[800];
    size_t read_bytes = fread(buffer, 1, 800, f);
    fclose(f);

    if (read_bytes != 800) {
        LOG_WARN("AUDIO", "ENGINE.INF '%s' invalid size (%zu != 800 bytes)", path, read_bytes);
        return false;
    }

    // Unpack 4 contiguous 200-byte curve lookup tables
    memcpy(ea->sample0_volume, buffer + 0,   200); // 0x000..0x0C7: Low-RPM volume
    memcpy(ea->sample0_pitch,  buffer + 200, 200); // 0x0C8..0x18F: Low-RPM pitch
    memcpy(ea->sample1_volume, buffer + 400, 200); // 0x190..0x257: High-RPM volume
    memcpy(ea->sample1_pitch,  buffer + 600, 200); // 0x258..0x31F: High-RPM pitch

    // Load low-RPM sample (00_A.WAV)
    snprintf(path, sizeof(path), "%s/00_A.WAV", car_dir);
    if (!SoundPool_LoadWav(path, &ea->sample0)) {
        snprintf(path, sizeof(path), "%s/00_a.wav", car_dir);
        SoundPool_LoadWav(path, &ea->sample0);
    }

    // Load high-RPM sample (01_A.WAV)
    snprintf(path, sizeof(path), "%s/01_A.WAV", car_dir);
    if (!SoundPool_LoadWav(path, &ea->sample1)) {
        snprintf(path, sizeof(path), "%s/01_a.wav", car_dir);
        SoundPool_LoadWav(path, &ea->sample1);
    }

    // Spawn continuous loop voices (initially muted)
    ea->voice0_handle = Audio_PlayVoice(&ea->sample0, 0.0f, 1.0f, 0.0f, true);
    ea->voice1_handle = Audio_PlayVoice(&ea->sample1, 0.0f, 1.0f, 0.0f, true);
    ea->initialized = true;

    LOG_INFO("AUDIO", "Engine audio initialized for '%s' (voices %d, %d)",
             car_dir, ea->voice0_handle, ea->voice1_handle);
    return true;
}

/**
 * @brief Update engine audio pitch and volume crossfade based on vehicle speed.
 * @original FUN_004452c0 (MAINDOS_32BIT.EXE @ 0x004452c0, main.c)
 * @fidelity ADAPTED
 * @deviation DEV-006 (High-RPM pitch curve safety clamp)
 * @fix_category FIX_CAT_AUDIO
 */
void EngineAudio_Update(EngineAudio *ea, double speed_abs, double top_speed,
                        bool is_airborne, const GameFixOptions *fixes) {
    if (!ea || !ea->initialized) {
        return;
    }

    if (top_speed < 1.0) {
        top_speed = 45.0; // Standard fallback m/s top speed
    }

    // Calculate normalized speed index [0..199]
    double normalized = speed_abs / top_speed;
    int curve_idx;

    if (fixes && fixes->fix_audio) {
        // DEV-006: Clamped speed index preventing pitch frequency overflow on turbo boost
        curve_idx = (int)(normalized * 199.0);
        if (curve_idx > 199) curve_idx = 199;
        if (curve_idx < 0)   curve_idx = 0;
    } else {
        // Authentic 1997 behavior: raw index wrap-around on extreme speeds
        curve_idx = (int)(normalized * 199.0);
        if (curve_idx < 1) curve_idx = 1;
        if (curve_idx > 199) {
            curve_idx = curve_idx % 256; // 8-bit wrap-around
            if (curve_idx > 199) curve_idx = 199;
        }
    }

    // Look up pitch and volume from ENGINE.INF
    uint8_t raw_p0 = ea->sample0_pitch[curve_idx];
    uint8_t raw_p1 = ea->sample1_pitch[curve_idx];
    uint8_t raw_v0 = ea->sample0_volume[curve_idx];
    uint8_t raw_v1 = ea->sample1_volume[curve_idx];

    // Pitch frequency formula matching FUN_004452c0:
    // frequency = pitch_raw * 600 + 20000 Hz
    // Base frequency is 22050 Hz (standard sample rate of 00_A.WAV and 01_A.WAV)
    float base_rate0 = (ea->sample0.sample_rate > 0) ? (float)ea->sample0.sample_rate : 22050.0f;
    float base_rate1 = (ea->sample1.sample_rate > 0) ? (float)ea->sample1.sample_rate : 22050.0f;

    float freq0 = (float)(raw_p0 * 600 + 20000);
    float freq1 = (float)(raw_p1 * 600 + 20000);

    float pitch0 = freq0 / base_rate0;
    float pitch1 = freq1 / base_rate1;

    // Normalizing volumes to [0.0..1.0]
    float vol0 = (float)raw_v0 / 100.0f;
    float vol1 = (float)raw_v1 / 100.0f;

    // When airborne without wheel traction, increase pitch slightly for revving effect
    if (is_airborne) {
        pitch0 *= 1.15f;
        pitch1 *= 1.15f;
    }

    Audio_SetVoiceParams(ea->voice0_handle, vol0, pitch0, 0.0f);
    Audio_SetVoiceParams(ea->voice1_handle, vol1, pitch1, 0.0f);
}

/**
 * @brief Stop engine voices and free audio memory.
 * @fidelity INFRASTRUCTURE
 */
void EngineAudio_Free(EngineAudio *ea) {
    if (!ea || !ea->initialized) {
        return;
    }

    if (ea->voice0_handle > 0) {
        Audio_StopVoice(ea->voice0_handle);
        ea->voice0_handle = 0;
    }
    if (ea->voice1_handle > 0) {
        Audio_StopVoice(ea->voice1_handle);
        ea->voice1_handle = 0;
    }

    SoundPool_FreeSample(&ea->sample0);
    SoundPool_FreeSample(&ea->sample1);
    ea->initialized = false;
}
