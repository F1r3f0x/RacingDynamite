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

/**
 * @brief Load a RIFF/WAVE PCM audio file from disk into 16-bit PCM format.
 * @original FUN_00458e00 (MAINDOS_32BIT.EXE @ 0x00458e00, main.c)
 * @fidelity ADAPTED
 */
bool SoundPool_LoadWav(const char *path, SoundSample *out_sample) {
    if (!path || !out_sample) {
        return false;
    }

    memset(out_sample, 0, sizeof(SoundSample));

    SDL_AudioSpec wav_spec;
    Uint8 *wav_buf = NULL;
    Uint32 wav_len = 0;
    SDL_AudioSpec *ret_spec = SDL_LoadWAV(path, &wav_spec, &wav_buf, &wav_len);
    if (ret_spec == NULL) {
        // Fallback: Check if a matching Gravis UltraSound GF1 .PAT file exists (authentic MAINDOS_32BIT.EXE 0x00458e00)
        char pat_path[256];
        strncpy(pat_path, path, sizeof(pat_path) - 1);
        pat_path[sizeof(pat_path) - 1] = '\0';
        char *ext = strrchr(pat_path, '.');
        if (ext) {
            if (strcmp(ext, ".WAV") == 0 || strcmp(ext, ".wav") == 0) {
                strcpy(ext, ".PAT");
                if (SoundPool_LoadPat(pat_path, out_sample)) {
                    return true;
                }
                strcpy(ext, ".pat");
                if (SoundPool_LoadPat(pat_path, out_sample)) {
                    return true;
                }
            }
        }

        LOG_DEBUG("AUDIO", "Failed to load WAV/PAT file '%s': %s", path, SDL_GetError());
        return false;
    }

    // Convert loaded PCM samples to signed 16-bit PCM (S16SYS).
    // Original Ignition assets are 8-bit unsigned (AUDIO_U8) or 16-bit signed (AUDIO_S16).
    int bytes_per_sample = SDL_AUDIO_BITSIZE(wav_spec.format) / 8;
    if (bytes_per_sample == 0) bytes_per_sample = 1;
    uint32_t total_samples = wav_len / bytes_per_sample;
    uint32_t total_frames = (wav_spec.channels > 0) ? (total_samples / wav_spec.channels) : total_samples;

    int16_t *pcm_data = (int16_t *)malloc(total_samples * sizeof(int16_t));
    if (!pcm_data) {
        SDL_FreeWAV(wav_buf);
        return false;
    }

    if (wav_spec.format == AUDIO_U8) {
        for (uint32_t i = 0; i < total_samples; ++i) {
            pcm_data[i] = (int16_t)(((int)wav_buf[i] - 128) << 8);
        }
    } else if (wav_spec.format == AUDIO_S8) {
        int8_t *s8 = (int8_t *)wav_buf;
        for (uint32_t i = 0; i < total_samples; ++i) {
            pcm_data[i] = (int16_t)((int)s8[i] << 8);
        }
    } else if (wav_spec.format == AUDIO_S16SYS) {
        memcpy(pcm_data, wav_buf, total_samples * sizeof(int16_t));
    } else {
        int16_t *src16 = (int16_t *)wav_buf;
        for (uint32_t i = 0; i < total_samples; ++i) {
            pcm_data[i] = (int16_t)SDL_Swap16(src16[i]);
        }
    }

    SDL_FreeWAV(wav_buf);

    out_sample->data = pcm_data;
    out_sample->sample_count = total_frames;
    out_sample->sample_rate = (uint32_t)wav_spec.freq;
    out_sample->channels = wav_spec.channels;

    // Extract filename for identification
    const char *last_slash = strrchr(path, '/');
    const char *last_bslash = strrchr(path, '\\');
    const char *fname = path;
    if (last_slash && last_slash >= fname) fname = last_slash + 1;
    if (last_bslash && last_bslash >= fname) fname = last_bslash + 1;
    strncpy(out_sample->name, fname, sizeof(out_sample->name) - 1);

    LOG_DEBUG("AUDIO", "Loaded WAV '%s': %u frames, %u Hz, %d ch",
              out_sample->name, out_sample->sample_count,
              out_sample->sample_rate, out_sample->channels);
    return true;
}

/**
 * @brief Load a Gravis UltraSound GF1 .PAT audio file from disk into 16-bit PCM format.
 * @original FUN_004582b0 (MAINDOS_32BIT.EXE @ 0x004582b0, main.c)
 * @fidelity ADAPTED
 */
bool SoundPool_LoadPat(const char *path, SoundSample *out_sample) {
    if (!path || !out_sample) return false;
    memset(out_sample, 0, sizeof(SoundSample));

    FILE *fp = fopen(path, "rb");
    if (!fp) return false;

    uint8_t hdr[0x14F];
    if (fread(hdr, 1, sizeof(hdr), fp) != sizeof(hdr)) {
        fclose(fp);
        return false;
    }

    // Verify GF1PATCH110 magic
    if (memcmp(hdr, "GF1PATCH110", 11) != 0) {
        fclose(fp);
        return false;
    }

    uint32_t sample_size = *(const uint32_t *)(hdr + 247);
    uint16_t sample_rate = *(const uint16_t *)(hdr + 259);
    uint8_t  mode_flags  = hdr[294];

    if (sample_size == 0 || sample_size > 10000000) {
        fclose(fp);
        return false;
    }

    bool is_16bit = (mode_flags & 1) != 0;
    bool is_unsigned = (mode_flags & 2) != 0;

    uint8_t *raw_buf = (uint8_t *)malloc(sample_size);
    if (!raw_buf) {
        fclose(fp);
        return false;
    }

    if (fread(raw_buf, 1, sample_size, fp) != sample_size) {
        free(raw_buf);
        fclose(fp);
        return false;
    }
    fclose(fp);

    uint32_t total_samples = is_16bit ? (sample_size / 2) : sample_size;
    int16_t *pcm_data = (int16_t *)malloc(total_samples * sizeof(int16_t));
    if (!pcm_data) {
        free(raw_buf);
        return false;
    }

    if (!is_16bit) {
        if (is_unsigned) {
            for (uint32_t i = 0; i < total_samples; ++i) {
                pcm_data[i] = (int16_t)(((int)raw_buf[i] - 128) << 8);
            }
        } else {
            const int8_t *s8 = (const int8_t *)raw_buf;
            for (uint32_t i = 0; i < total_samples; ++i) {
                pcm_data[i] = (int16_t)((int)s8[i] << 8);
            }
        }
    } else {
        const int16_t *s16 = (const int16_t *)raw_buf;
        if (is_unsigned) {
            for (uint32_t i = 0; i < total_samples; ++i) {
                pcm_data[i] = (int16_t)((int)s16[i] - 32768);
            }
        } else {
            memcpy(pcm_data, raw_buf, total_samples * sizeof(int16_t));
        }
    }

    free(raw_buf);

    out_sample->data = pcm_data;
    out_sample->sample_count = total_samples;
    out_sample->sample_rate = (sample_rate > 0) ? (uint32_t)sample_rate : 22050;
    out_sample->channels = 1;

    // Extract filename for identification
    const char *last_slash = strrchr(path, '/');
    const char *last_bslash = strrchr(path, '\\');
    const char *fname = path;
    if (last_slash && last_slash >= fname) fname = last_slash + 1;
    if (last_bslash && last_bslash >= fname) fname = last_bslash + 1;
    strncpy(out_sample->name, fname, sizeof(out_sample->name) - 1);

    LOG_DEBUG("AUDIO", "Loaded PAT '%s': %u frames, %u Hz, %d ch",
              out_sample->name, out_sample->sample_count,
              out_sample->sample_rate, out_sample->channels);
    return true;
}


/**
 * @brief Release allocated PCM memory of a SoundSample.
 * @fidelity INFRASTRUCTURE
 */
void SoundPool_FreeSample(SoundSample *sample) {
    if (!sample) {
        return;
    }
    if (sample->data) {
        free(sample->data);
        sample->data = NULL;
    }
    sample->sample_count = 0;
    sample->channels = 0;
    sample->name[0] = '\0';
}

/**
 * @brief Load the standard general sound effect pool (ROLL, SKID, COLL, BOOST).
 * @original FUN_0041f9b0 (MAINDOS_32BIT.EXE @ 0x0041f9b0, main.c)
 * @fidelity ADAPTED
 */
bool SoundPool_LoadGeneralSFX(GeneralSFX *sfx, const char *assets_dir) {
    if (!sfx) return false;
    memset(sfx, 0, sizeof(GeneralSFX));

    char path[512];

    // Tire rolling sound (continuous loop)
    snprintf(path, sizeof(path), "%s/GENERAL/SOUND/ROLL/00_RULL.WAV", assets_dir);
    SoundPool_LoadWav(path, &sfx->roll);

    // Tire skid squeal sound (continuous loop)
    snprintf(path, sizeof(path), "%s/GENERAL/SOUND/SKID/00_SLADD.WAV", assets_dir);
    SoundPool_LoadWav(path, &sfx->skid);

    // Collision bump
    snprintf(path, sizeof(path), "%s/GENERAL/SOUND/COLL/06_MOS.WAV", assets_dir);
    SoundPool_LoadWav(path, &sfx->coll_bump);

    // Collision crash / explosion
    snprintf(path, sizeof(path), "%s/GENERAL/SOUND/COLL/04_EXPLO.WAV", assets_dir);
    SoundPool_LoadWav(path, &sfx->coll_crash);

    // Vehicle landing thud
    snprintf(path, sizeof(path), "%s/GENERAL/SOUND/COLL/09_LANDA.WAV", assets_dir);
    SoundPool_LoadWav(path, &sfx->coll_land);

    // Turbo boost roar/hiss
    snprintf(path, sizeof(path), "%s/GENERAL/SOUND/BOOST/00_BOOST.WAV", assets_dir);
    SoundPool_LoadWav(path, &sfx->boost);

    // Menu UI clicks
    snprintf(path, sizeof(path), "%s/BALTAZAR/DATA/SOUND/0_GRP0/0_SEL.WAV", assets_dir);
    SoundPool_LoadWav(path, &sfx->ui_select);

    snprintf(path, sizeof(path), "%s/BALTAZAR/DATA/SOUND/0_GRP0/1_FLIP.WAV", assets_dir);
    SoundPool_LoadWav(path, &sfx->ui_flip);

    sfx->loaded = true;
    LOG_INFO("AUDIO", "General SFX bank loaded from '%s'", assets_dir);
    return true;
}

/**
 * @brief Release all samples in the general SFX bank.
 * @fidelity INFRASTRUCTURE
 */
void SoundPool_FreeGeneralSFX(GeneralSFX *sfx) {
    if (!sfx || !sfx->loaded) return;

    if (sfx->roll_voice > 0) Audio_StopVoice(sfx->roll_voice);
    if (sfx->skid_voice > 0) Audio_StopVoice(sfx->skid_voice);

    SoundPool_FreeSample(&sfx->roll);
    SoundPool_FreeSample(&sfx->skid);
    SoundPool_FreeSample(&sfx->coll_bump);
    SoundPool_FreeSample(&sfx->coll_crash);
    SoundPool_FreeSample(&sfx->coll_land);
    SoundPool_FreeSample(&sfx->boost);
    SoundPool_FreeSample(&sfx->ui_select);
    SoundPool_FreeSample(&sfx->ui_flip);

    sfx->loaded = false;
}
