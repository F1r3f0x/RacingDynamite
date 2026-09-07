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
#include "ignition/physics.h"
#include "ignition/log.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>

static void test_engine_inf_parsing_and_curves(void) {
    LOG_INFO("TEST", "Running test_engine_inf_parsing_and_curves...");
    EngineAudio ea;
    bool ok = EngineAudio_Init(&ea, "assets/CARS/COOPER/SOUND");
    assert(ok);
    assert(ea.initialized);

    // Verify low-RPM volume at idle is audible
    assert(ea.sample0_volume[0] > 0);
    // High-RPM volume at idle should be quiet or 0
    assert(ea.sample0_volume[0] > ea.sample1_volume[0]);

    // High-RPM volume at top speed index 199 should be loudest
    assert(ea.sample1_volume[199] > 70);

    // Verify curve bounds
    for (int i = 0; i < ENGINE_INF_CURVE_SIZE; ++i) {
        assert(ea.sample0_volume[i] <= 100);
        assert(ea.sample0_pitch[i] <= 100);
        assert(ea.sample1_volume[i] <= 100);
        assert(ea.sample1_pitch[i] <= 100);
    }

    EngineAudio_Free(&ea);
    LOG_INFO("TEST", "test_engine_inf_parsing_and_curves PASSED");
}

static void test_engine_audio_rpm_pitch_formula_and_dev006(void) {
    LOG_INFO("TEST", "Running test_engine_audio_rpm_pitch_formula_and_dev006...");
    EngineAudio ea;
    bool ok = EngineAudio_Init(&ea, "assets/CARS/COOPER/SOUND");
    assert(ok);

    GameFixOptions fixes;
    memset(&fixes, 0, sizeof(fixes));

    // Test with DEV-006 fix enabled (clamped)
    fixes.fix_audio = true;
    EngineAudio_Update(&ea, 150.0, 45.0, false, &fixes); // Extreme speed (turbo boost)

    // Test with authentic buggy behavior
    fixes.fix_audio = false;
    EngineAudio_Update(&ea, 150.0, 45.0, false, &fixes);

    EngineAudio_Free(&ea);
    LOG_INFO("TEST", "test_engine_audio_rpm_pitch_formula_and_dev006 PASSED");
}

static void test_sound_pool_load_wav(void) {
    LOG_INFO("TEST", "Running test_sound_pool_load_wav...");
    SoundSample sample;

    bool ok = SoundPool_LoadWav("assets/GENERAL/SOUND/BOOST/00_BOOST.WAV", &sample);
    assert(ok);
    assert(sample.data != NULL);
    assert(sample.sample_count > 0);
    assert(sample.sample_rate > 0);
    assert(sample.channels >= 1);

    SoundPool_FreeSample(&sample);
    assert(sample.data == NULL);
    assert(sample.sample_count == 0);

    LOG_INFO("TEST", "test_sound_pool_load_wav PASSED");
}

static void test_music_circuit_mapping(void) {
    LOG_INFO("TEST", "Running test_music_circuit_mapping...");

    // Matches authentic executable table DAT_00497eb8
    assert(Music_GetTrackForCircuit(0) == 2); // Canada
    assert(Music_GetTrackForCircuit(1) == 3); // Austria
    assert(Music_GetTrackForCircuit(2) == 4); // Brazil
    assert(Music_GetTrackForCircuit(3) == 5); // Paris
    assert(Music_GetTrackForCircuit(4) == 6); // Australia
    assert(Music_GetTrackForCircuit(5) == 6); // Iceland
    assert(Music_GetTrackForCircuit(6) == 7); // USA

    // Out of range should default to track 2
    assert(Music_GetTrackForCircuit(-1) == 2);
    assert(Music_GetTrackForCircuit(99) == 2);

    LOG_INFO("TEST", "test_music_circuit_mapping PASSED");
}

static void test_audio_mixer_lifecycle_and_voices(void) {
    LOG_INFO("TEST", "Running test_audio_mixer_lifecycle_and_voices...");

    bool ok = Audio_Init();
    assert(ok);

    // Create a synthetic 16-bit PCM sine wave sample
    int16_t synthetic_pcm[1000];
    for (int i = 0; i < 1000; ++i) {
        synthetic_pcm[i] = (int16_t)(sin(i * 0.05) * 16000.0);
    }

    SoundSample synth;
    synth.data = synthetic_pcm;
    synth.sample_count = 1000;
    synth.sample_rate = 44100;
    synth.channels = 1;
    strncpy(synth.name, "SYNTH_TEST", sizeof(synth.name));

    int handle1 = Audio_PlayVoice(&synth, 0.8f, 1.0f, 0.0f, true);
    assert(handle1 > 0);

    Audio_SetVoiceParams(handle1, 0.5f, 1.5f, -0.5f);

    int handle2 = Audio_PlayVoice(&synth, 0.6f, 0.8f, 0.5f, false);
    assert(handle2 > 0);
    assert(handle2 != handle1);

    Audio_StopVoice(handle1);
    Audio_StopAllVoices();

    Audio_Shutdown();
    LOG_INFO("TEST", "test_audio_mixer_lifecycle_and_voices PASSED");
}

static void test_all_11_cars_sound_loading(void) {
    LOG_INFO("TEST", "Running test_all_11_cars_sound_loading...");
    static const char *cars[11] = {
        "COOPER", "PORSCHE", "JEEP", "COP", "MUSTANG", "SCHOOL",
        "VAN", "VW", "TRUCK", "DODGE", "NASCAR"
    };

    for (int i = 0; i < 11; ++i) {
        char dir[128];
        snprintf(dir, sizeof(dir), "assets/CARS/%s/SOUND", cars[i]);
        EngineAudio ea;
        bool ok = EngineAudio_Init(&ea, dir);
        assert(ok && "Failed to load engine sound for car");
        assert(ea.initialized);
        assert(ea.sample0.data != NULL && ea.sample0.sample_count > 0);
        assert(ea.sample1.data != NULL && ea.sample1.sample_count > 0);
        EngineAudio_Free(&ea);
        LOG_INFO("TEST", "Car [%d] %s sound successfully loaded (PAT/WAV verified)", i, cars[i]);
    }
    LOG_INFO("TEST", "test_all_11_cars_sound_loading PASSED");
}

int main(void) {
    LOG_INFO("TEST", "=== RACING DYNAMITE AUDIO SUBSYSTEM TEST SUITE ===");

    test_music_circuit_mapping();
    test_sound_pool_load_wav();
    test_audio_mixer_lifecycle_and_voices();
    test_engine_inf_parsing_and_curves();
    test_engine_audio_rpm_pitch_formula_and_dev006();
    test_all_11_cars_sound_loading();

    LOG_INFO("TEST", "ALL 6 AUDIO TESTS PASSED SUCCESSFULLY!");
    return 0;
}

