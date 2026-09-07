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
#include <math.h>
#include <string.h>
#include <stdlib.h>

// Forward declaration for digital music mixer mixdown
void Music_MixAudio(int16_t *stream, int frames, float master_music_vol);

typedef struct {
    SDL_AudioDeviceID   device_id;
    bool                initialized;
    AudioVoice          voices[AUDIO_MAX_VOICES];
    uint16_t            voice_generation[AUDIO_MAX_VOICES];
    float               master_sfx_vol;
    float               master_music_vol;
} AudioManager;

static AudioManager g_audio = {0};

/**
 * @brief Master audio mixing callback executing inside SDL2 audio thread.
 * @original FUN_00457890 (IGN_WIN.EXE @ 0x00457890, main.c)
 * @fidelity ADAPTED
 */
static void Audio_MixCallback(void *userdata, Uint8 *stream, int len) {
    (void)userdata;
    int16_t *out = (int16_t *)stream;
    int total_frames = len / (2 * (int)sizeof(int16_t)); // Stereo 16-bit

    // Temporary 32-bit mix accumulator to prevent integer overflow
    int32_t mix_l[AUDIO_BUFFER_SAMPLES];
    int32_t mix_r[AUDIO_BUFFER_SAMPLES];

    for (int i = 0; i < total_frames; ++i) {
        mix_l[i] = 0;
        mix_r[i] = 0;
    }

    // Mix all active voice channels
    for (int v = 0; v < AUDIO_MAX_VOICES; ++v) {
        AudioVoice *voice = &g_audio.voices[v];
        if (!voice->active || !voice->sample || !voice->sample->data || voice->sample->sample_count == 0) {
            continue;
        }

        const SoundSample *s = voice->sample;
        uint32_t sc = s->sample_count;
        uint8_t channels = s->channels;

        for (int i = 0; i < total_frames; ++i) {
            uint32_t idx = voice->playhead_int;
            if (idx >= sc) {
                if (voice->looping) {
                    voice->playhead_int %= sc;
                    idx = voice->playhead_int;
                } else {
                    voice->active = false;
                    break;
                }
            }

            // Resampling linear interpolation
            uint32_t next_idx = idx + 1;
            if (next_idx >= sc) {
                next_idx = voice->looping ? 0 : idx;
            }

            int32_t s0_l = s->data[idx * channels];
            int32_t s1_l = s->data[next_idx * channels];
            int32_t s0_r = (channels > 1) ? s->data[idx * channels + 1] : s0_l;
            int32_t s1_r = (channels > 1) ? s->data[next_idx * channels + 1] : s1_l;

            int32_t frac = (int32_t)(voice->playhead_frac & 0xFFFF);
            int32_t sample_l = s0_l + (((s1_l - s0_l) * frac) >> 16);
            int32_t sample_r = s0_r + (((s1_r - s0_r) * frac) >> 16);

            mix_l[i] += (sample_l * voice->volume_left) >> 15;
            mix_r[i] += (sample_r * voice->volume_right) >> 15;

            voice->playhead_frac += voice->rate_step;
            voice->playhead_int += (voice->playhead_frac >> 16);
            voice->playhead_frac &= 0xFFFF;
        }
    }

    // Apply master SFX volume and clamp to 16-bit signed PCM
    float sfx_vol = g_audio.master_sfx_vol;
    for (int i = 0; i < total_frames; ++i) {
        int32_t out_l = (int32_t)(mix_l[i] * sfx_vol);
        int32_t out_r = (int32_t)(mix_r[i] * sfx_vol);

        if (out_l > 32767) out_l = 32767;
        if (out_l < -32768) out_l = -32768;
        if (out_r > 32767) out_r = 32767;
        if (out_r < -32768) out_r = -32768;

        out[i * 2 + 0] = (int16_t)out_l;
        out[i * 2 + 1] = (int16_t)out_r;
    }

    // Mix digital background music stream
    Music_MixAudio(out, total_frames, g_audio.master_music_vol);
}

/**
 * @brief Initialize SDL2 audio device and allocate 32 software mixer voices.
 * @original FUN_0041f9b0 (IGN_WIN.EXE @ 0x0041f9b0, main.c)
 * @fidelity ADAPTED
 */
bool Audio_Init(void) {
    if (g_audio.initialized) {
        return true;
    }

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        LOG_WARN("AUDIO", "SDL_InitSubSystem(AUDIO) failed: %s", SDL_GetError());
        return false;
    }

    SDL_AudioSpec desired, obtained;
    memset(&desired, 0, sizeof(desired));
    desired.freq = AUDIO_SAMPLE_RATE;
    desired.format = AUDIO_S16SYS;
    desired.channels = 2;
    desired.samples = AUDIO_BUFFER_SAMPLES;
    desired.callback = Audio_MixCallback;
    desired.userdata = NULL;

    g_audio.device_id = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
    if (g_audio.device_id == 0) {
        LOG_WARN("AUDIO", "SDL_OpenAudioDevice failed: %s", SDL_GetError());
        return false;
    }

    g_audio.master_sfx_vol = 1.0f;
    g_audio.master_music_vol = 0.8f;
    memset(g_audio.voices, 0, sizeof(g_audio.voices));
    for (int i = 0; i < AUDIO_MAX_VOICES; ++i) {
        g_audio.voice_generation[i] = 1;
    }

    SDL_PauseAudioDevice(g_audio.device_id, 0); // Start playback
    g_audio.initialized = true;

    LOG_INFO("AUDIO", "SDL Audio initialized: %d Hz, %d channels, buffer %d frames",
             obtained.freq, obtained.channels, obtained.samples);
    return true;
}

/**
 * @brief Close audio hardware device and release all mixer voice buffers.
 * @original FUN_00412530 (IGN_WIN.EXE @ 0x00412530, main.c)
 * @fidelity ADAPTED
 */
void Audio_Shutdown(void) {
    if (!g_audio.initialized) {
        return;
    }

    Music_Stop();

    if (g_audio.device_id != 0) {
        SDL_LockAudioDevice(g_audio.device_id);
        for (int i = 0; i < AUDIO_MAX_VOICES; ++i) {
            g_audio.voices[i].active = false;
        }
        SDL_UnlockAudioDevice(g_audio.device_id);

        SDL_CloseAudioDevice(g_audio.device_id);
        g_audio.device_id = 0;
    }

    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    g_audio.initialized = false;
    LOG_INFO("AUDIO", "Audio subsystem terminated cleanly");
}

/**
 * @brief Allocate an active mixer voice and begin playback of a PCM sample.
 * @original FUN_004579b0 (IGN_WIN.EXE @ 0x004579b0, main.c)
 * @fidelity ADAPTED
 */
int Audio_PlayVoice(const SoundSample *sample, float volume, float pitch, float pan, bool loop) {
    if (!g_audio.initialized || !sample || !sample->data || sample->sample_count == 0) {
        return 0;
    }

    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    if (pitch < 0.05f) pitch = 0.05f;
    if (pitch > 8.0f)  pitch = 8.0f;
    if (pan < -1.0f) pan = -1.0f;
    if (pan > 1.0f)  pan = 1.0f;

    SDL_LockAudioDevice(g_audio.device_id);

    int channel = -1;
    for (int i = 0; i < AUDIO_MAX_VOICES; ++i) {
        if (!g_audio.voices[i].active) {
            channel = i;
            break;
        }
    }

    // Voice stealing fallback: steal oldest one-shot voice
    if (channel < 0) {
        for (int i = 0; i < AUDIO_MAX_VOICES; ++i) {
            if (!g_audio.voices[i].looping) {
                channel = i;
                break;
            }
        }
    }

    if (channel < 0) {
        SDL_UnlockAudioDevice(g_audio.device_id);
        return 0; // Exhausted voices
    }

    AudioVoice *v = &g_audio.voices[channel];
    v->sample = sample;
    v->playhead_int = 0;
    v->playhead_frac = 0;
    v->looping = loop;

    // Fixed-point 16.16 sampling rate step
    double src_rate = (double)sample->sample_rate * (double)pitch;
    v->rate_step = (uint32_t)((src_rate / (double)AUDIO_SAMPLE_RATE) * (double)AUDIO_FP_ONE);

    // Calculate left / right panning gains
    float pan_l = (1.0f - pan) * 0.5f;
    float pan_r = (1.0f + pan) * 0.5f;
    v->volume_left  = (int)(volume * pan_l * 32767.0f);
    v->volume_right = (int)(volume * pan_r * 32767.0f);

    uint16_t gen = ++g_audio.voice_generation[channel];
    if (gen == 0) gen = ++g_audio.voice_generation[channel];
    v->handle = (channel << 16) | (int)gen;
    v->active = true;

    SDL_UnlockAudioDevice(g_audio.device_id);
    return v->handle;
}

/**
 * @brief Update real-time playback parameters for an active voice.
 * @original FUN_00457aa0 (IGN_WIN.EXE @ 0x00457aa0, main.c)
 * @fidelity ADAPTED
 */
void Audio_SetVoiceParams(int voice_handle, float volume, float pitch, float pan) {
    if (!g_audio.initialized || voice_handle <= 0) {
        return;
    }

    int channel = (voice_handle >> 16) & 0xFFFF;
    uint16_t gen = (uint16_t)(voice_handle & 0xFFFF);

    if (channel < 0 || channel >= AUDIO_MAX_VOICES) {
        return;
    }

    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    if (pitch < 0.05f) pitch = 0.05f;
    if (pitch > 8.0f)  pitch = 8.0f;
    if (pan < -1.0f) pan = -1.0f;
    if (pan > 1.0f)  pan = 1.0f;

    SDL_LockAudioDevice(g_audio.device_id);

    if (g_audio.voice_generation[channel] == gen && g_audio.voices[channel].active) {
        AudioVoice *v = &g_audio.voices[channel];
        if (v->sample) {
            double src_rate = (double)v->sample->sample_rate * (double)pitch;
            v->rate_step = (uint32_t)((src_rate / (double)AUDIO_SAMPLE_RATE) * (double)AUDIO_FP_ONE);

            float pan_l = (1.0f - pan) * 0.5f;
            float pan_r = (1.0f + pan) * 0.5f;
            v->volume_left  = (int)(volume * pan_l * 32767.0f);
            v->volume_right = (int)(volume * pan_r * 32767.0f);
        }
    }

    SDL_UnlockAudioDevice(g_audio.device_id);
}

/**
 * @brief Stop and release a specific active voice.
 * @original FUN_00457980 (IGN_WIN.EXE @ 0x00457980, main.c)
 * @fidelity ADAPTED
 */
void Audio_StopVoice(int voice_handle) {
    if (!g_audio.initialized || voice_handle <= 0) {
        return;
    }

    int channel = (voice_handle >> 16) & 0xFFFF;
    uint16_t gen = (uint16_t)(voice_handle & 0xFFFF);

    if (channel < 0 || channel >= AUDIO_MAX_VOICES) {
        return;
    }

    SDL_LockAudioDevice(g_audio.device_id);
    if (g_audio.voice_generation[channel] == gen) {
        g_audio.voices[channel].active = false;
    }
    SDL_UnlockAudioDevice(g_audio.device_id);
}

/**
 * @brief Stop and release all currently playing voices.
 * @original FUN_00457980 (IGN_WIN.EXE @ 0x00457980, main.c)
 * @fidelity ADAPTED
 */
void Audio_StopAllVoices(void) {
    if (!g_audio.initialized) {
        return;
    }

    SDL_LockAudioDevice(g_audio.device_id);
    for (int i = 0; i < AUDIO_MAX_VOICES; ++i) {
        g_audio.voices[i].active = false;
    }
    SDL_UnlockAudioDevice(g_audio.device_id);
}

/**
 * @brief Set global master volume levels for sound effects and music.
 * @original FUN_00457ae0 (IGN_WIN.EXE @ 0x00457ae0, main.c)
 * @fidelity ADAPTED
 */
void Audio_SetMasterVolume(float sfx_volume, float music_volume) {
    if (sfx_volume < 0.0f) sfx_volume = 0.0f;
    if (sfx_volume > 1.0f) sfx_volume = 1.0f;
    if (music_volume < 0.0f) music_volume = 0.0f;
    if (music_volume > 1.0f) music_volume = 1.0f;

    g_audio.master_sfx_vol = sfx_volume;
    g_audio.master_music_vol = music_volume;
}

/**
 * @brief Query current master volume levels.
 * @fidelity INFRASTRUCTURE
 */
void Audio_GetMasterVolume(float *out_sfx, float *out_music) {
    if (out_sfx) *out_sfx = g_audio.master_sfx_vol;
    if (out_music) *out_music = g_audio.master_music_vol;
}

/**
 * @brief Helper for one-shot SFX playback.
 * @fidelity INFRASTRUCTURE
 */
int Audio_PlaySFX(const SoundSample *sample, float volume, float pan) {
    return Audio_PlayVoice(sample, volume, 1.0f, pan, false);
}

/**
 * @brief Play spatialized 3D audio relative to listener camera/car.
 * @original FUN_0043e710 (IGN_WIN.EXE @ 0x0043e710, main.c)
 * @fidelity ADAPTED
 */
int Audio_PlaySpatialSFX(const SoundSample *sample,
                         float emitter_x, float emitter_z,
                         float listener_x, float listener_z,
                         float base_volume, float max_distance) {
    if (!sample || max_distance <= 0.0f) {
        return 0;
    }

    float dx = emitter_x - listener_x;
    float dz = emitter_z - listener_z;
    float dist = sqrtf(dx * dx + dz * dz);

    if (dist >= max_distance) {
        return 0; // Out of hearing range
    }

    // Distance attenuation matching FUN_0043e710 formula
    float vol = (1.0f - (dist / max_distance)) * base_volume;
    if (vol <= 0.01f) {
        return 0;
    }

    // Stereo panning based on relative lateral offset
    float pan = dx / (max_distance * 0.5f);
    if (pan < -1.0f) pan = -1.0f;
    if (pan > 1.0f)  pan = 1.0f;

    return Audio_PlayVoice(sample, vol, 1.0f, pan, false);
}
