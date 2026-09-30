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
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Disable MSVC warnings for third-party stb_vorbis
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4244 4245 4456 4457 4701)
#endif

#define STB_VORBIS_NO_PUSHDATA_API
#include "../../third_party/stb/stb_vorbis.c"

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

// CD-DA soundtrack mapping from original executable table DAT_00497eb8
// Index 0: Canada, 1: Austria, 2: Brazil, 3: Paris, 4: Australia, 5: Iceland, 6: USA
static const int s_circuit_tracks[7] = {
    2, // Track 0 (Canada)    -> Track02.ogg
    3, // Track 1 (Austria)   -> Track03.ogg
    4, // Track 2 (Brazil)    -> Track04.ogg
    5, // Track 3 (Paris)     -> Track05.ogg
    6, // Track 4 (Australia) -> Track06.ogg
    6, // Track 5 (Iceland)   -> Track06.ogg
    7  // Track 6 (USA)       -> Track07.ogg
};

typedef struct {
    stb_vorbis  *stream;
    bool         playing;
    bool         looping;
    float        volume;
    int          track_number;
    int16_t      temp_buf[AUDIO_BUFFER_SAMPLES * 2];
    SDL_mutex   *mutex;
    bool         initialized;
} MusicState;

static MusicState g_music = {0};

/**
 * @brief Map circuit index to CD soundtrack track number.
 * @original DAT_00497eb8 (MAINDOS_32BIT.EXE @ 0x00419260, main.c)
 * @fidelity EXACT
 */
int Music_GetTrackForCircuit(int track_index) {
    if (track_index < 0 || track_index >= 7) {
        return 2; // Default to Track 02 (Menu / Canada)
    }
    return s_circuit_tracks[track_index];
}

/**
 * @brief Mix decoded music PCM samples directly into the audio output stream.
 * @original FUN_00457890 (MAINDOS_32BIT.EXE @ 0x00457890, main.c)
 * @fidelity ADAPTED
 */
void Music_MixAudio(int16_t *stream, int frames, float master_music_vol) {
    if (!g_music.playing || !g_music.stream || frames <= 0) {
        return;
    }

    if (g_music.mutex) SDL_LockMutex(g_music.mutex);

    float effective_vol = g_music.volume * master_music_vol;
    if (effective_vol <= 0.001f) {
        if (g_music.mutex) SDL_UnlockMutex(g_music.mutex);
        return;
    }

    int samples_needed = frames * 2; // Stereo samples
    int samples_read = stb_vorbis_get_samples_short_interleaved(
        g_music.stream, 2, g_music.temp_buf, samples_needed);

    // Loop handling if end of stream reached
    if (samples_read < samples_needed && g_music.looping) {
        stb_vorbis_seek_start(g_music.stream);
        int remaining = samples_needed - samples_read;
        int second_read = stb_vorbis_get_samples_short_interleaved(
            g_music.stream, 2, g_music.temp_buf + samples_read, remaining);
        samples_read += second_read;
    }

    // Mixdown with master volume and saturation clamp
    for (int i = 0; i < samples_read; ++i) {
        int32_t current = stream[i];
        int32_t music_samp = (int32_t)(g_music.temp_buf[i] * effective_vol);
        int32_t mixed = current + music_samp;

        if (mixed > 32767) mixed = 32767;
        if (mixed < -32768) mixed = -32768;
        stream[i] = (int16_t)mixed;
    }

    if (samples_read < samples_needed && !g_music.looping) {
        g_music.playing = false;
    }

    if (g_music.mutex) SDL_UnlockMutex(g_music.mutex);
}

/**
 * @brief Begin streaming a music track file (e.g. Track02.ogg).
 * @original FUN_00457ed0 (MAINDOS_32BIT.EXE @ 0x00457ed0, main.c)
 * @fidelity ADAPTED
 */
bool Music_PlayTrack(int track_number, bool loop) {
    if (!g_music.initialized) {
        g_music.mutex = SDL_CreateMutex();
        g_music.volume = 1.0f;
        g_music.initialized = true;
    }

    if (g_music.mutex) SDL_LockMutex(g_music.mutex);

    // Close any currently playing track
    if (g_music.stream) {
        stb_vorbis_close(g_music.stream);
        g_music.stream = NULL;
        g_music.playing = false;
    }

    char path[512];
    snprintf(path, sizeof(path), "assets/MUSIC/Track%02d.ogg", track_number);

    int error = 0;
    stb_vorbis *v = stb_vorbis_open_filename(path, &error, NULL);
    if (!v) {
        // Fallback: try lowercase
        snprintf(path, sizeof(path), "assets/MUSIC/track%02d.ogg", track_number);
        v = stb_vorbis_open_filename(path, &error, NULL);
    }
    if (!v) {
        // Fallback: search in Ignition/Ignition/MUSIC/
        snprintf(path, sizeof(path), "Ignition/Ignition/MUSIC/Track%02d.ogg", track_number);
        v = stb_vorbis_open_filename(path, &error, NULL);
    }

    if (!v) {
        LOG_WARN("AUDIO", "Failed to open music track %d (file '%s', err %d)", track_number, path, error);
        if (g_music.mutex) SDL_UnlockMutex(g_music.mutex);
        return false;
    }

    g_music.stream = v;
    g_music.track_number = track_number;
    g_music.looping = loop;
    g_music.playing = true;

    if (g_music.mutex) SDL_UnlockMutex(g_music.mutex);

    LOG_INFO("AUDIO", "Playing soundtrack Track%02d.ogg (loop=%d)", track_number, loop);
    return true;
}

/**
 * @brief Stop digital music streaming.
 * @original FUN_00457ed0 (MAINDOS_32BIT.EXE @ 0x00457ed0, main.c)
 * @fidelity ADAPTED
 */
void Music_Stop(void) {
    if (!g_music.initialized) return;

    if (g_music.mutex) SDL_LockMutex(g_music.mutex);

    if (g_music.stream) {
        stb_vorbis_close(g_music.stream);
        g_music.stream = NULL;
    }
    g_music.playing = false;
    g_music.track_number = 0;

    if (g_music.mutex) SDL_UnlockMutex(g_music.mutex);
    LOG_DEBUG("AUDIO", "Music stopped");
}

/**
 * @brief Set music playback volume.
 * @original FUN_00457ae0 (MAINDOS_32BIT.EXE @ 0x00457ae0, main.c)
 * @fidelity ADAPTED
 */
void Music_SetVolume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    g_music.volume = volume;
}

/**
 * @brief Check if music is actively streaming.
 * @original FUN_00457b10 (MAINDOS_32BIT.EXE @ 0x00457b10, main.c)
 * @fidelity ADAPTED
 */
bool Music_IsPlaying(void) {
    return g_music.playing && (g_music.stream != NULL);
}

/**
 * @brief Update music streaming buffer (call periodically or from main loop).
 * @original FUN_0041775a (MAINDOS_32BIT.EXE @ 0x0041775a, main.c)
 * @fidelity ADAPTED
 */
void Music_Update(void) {
    // Background mixing occurs directly inside Audio_MixCallback via Music_MixAudio.
    // This hook allows future cross-fading or playlist logic.
}
