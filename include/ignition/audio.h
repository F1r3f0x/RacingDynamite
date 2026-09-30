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

#ifndef IGNITION_AUDIO_H
#define IGNITION_AUDIO_H

#include "ignition/physics.h"
#include <stdint.h>
#include <stdbool.h>

#define AUDIO_MAX_VOICES        32
#define AUDIO_SAMPLE_RATE       44100
#define AUDIO_BUFFER_SAMPLES    1024
#define AUDIO_FP_SHIFT          16
#define AUDIO_FP_ONE            (1 << AUDIO_FP_SHIFT)
#define ENGINE_INF_CURVE_SIZE   200

/**
 * @brief Raw decoded 16-bit PCM audio sample.
 */
typedef struct {
    int16_t     *data;           // Interleaved 16-bit signed PCM samples
    uint32_t     sample_count;   // Total sample frames (per channel)
    uint32_t     sample_rate;    // Sample rate in Hz (e.g. 22050, 44100)
    uint8_t      channels;       // 1 = mono, 2 = stereo
    char         name[32];       // Identifier or filename
} SoundSample;

/**
 * @brief Audio pool types matching original DirectSound asset banks.
 * @original DAT_004990b4 (MAINDOS_32BIT.EXE @ 0x0041f9b0, main.c)
 */
typedef enum {
    SOUND_POOL_GENERAL = 0, // ROLL, SKID, COLL, BOOST, DIV
    SOUND_POOL_TRACK   = 1, // LEVELS/<TRACK>/SOUND/*.WAV
    SOUND_POOL_CAR     = 2, // CARS/<CAR>/SOUND/00_A.WAV, 01_A.WAV
    SOUND_POOL_UI      = 3  // BALTAZAR/DATA/SOUND/0_GRP0/*.WAV
} SoundPoolType;

/**
 * @brief Single mixer voice channel state.
 * @original DAT_0063caa4 (MAINDOS_32BIT.EXE @ 0x004579b0, main.c)
 */
typedef struct {
    const SoundSample *sample;
    uint32_t     playhead_int;   // Current sample index
    uint32_t     playhead_frac;  // 16.16 fixed-point fractional sample position
    uint32_t     rate_step;      // 16.16 increment per output mix sample
    int          volume_left;    // 0..32767
    int          volume_right;   // 0..32767
    bool         looping;        // True for continuous loops
    bool         active;         // True if voice is actively rendering
    int          handle;         // (channel_index << 16) | generation
} AudioVoice;

/**
 * @brief Vehicle engine acoustic state driven by ENGINE.INF curve lookup tables.
 * @original DAT_005daffc + 0x650 (MAINDOS_32BIT.EXE @ 0x0041f9b0, main.c)
 */
typedef struct {
    uint8_t     sample0_volume[ENGINE_INF_CURVE_SIZE]; // 0x000..0x0C7: Low-RPM volume curve
    uint8_t     sample0_pitch[ENGINE_INF_CURVE_SIZE];  // 0x0C8..0x18F: Low-RPM pitch curve
    uint8_t     sample1_volume[ENGINE_INF_CURVE_SIZE]; // 0x190..0x257: High-RPM volume curve
    uint8_t     sample1_pitch[ENGINE_INF_CURVE_SIZE];  // 0x258..0x31F: High-RPM pitch curve
    SoundSample sample0;                              // Low-RPM idle loop (00_A.WAV)
    SoundSample sample1;                              // High-RPM throttle loop (01_A.WAV)
    int         voice0_handle;                        // Active voice handle for sample 0
    int         voice1_handle;                        // Active voice handle for sample 1
    bool        initialized;
} EngineAudio;

/**
 * @brief Master general SFX bank loaded during race startup.
 * @original DAT_004990b4 (MAINDOS_32BIT.EXE @ 0x0041f9b0, main.c)
 */
typedef struct {
    SoundSample roll;       // GENERAL/SOUND/ROLL/00_RULL.WAV
    SoundSample skid;       // GENERAL/SOUND/SKID/00_SLADD.WAV
    SoundSample coll_bump;  // GENERAL/SOUND/COLL/06_MOS.WAV
    SoundSample coll_crash; // GENERAL/SOUND/COLL/04_EXPLO.WAV
    SoundSample coll_land;  // GENERAL/SOUND/COLL/09_LANDA.WAV
    SoundSample boost;      // GENERAL/SOUND/BOOST/00_BOOST.WAV
    SoundSample ui_select;  // BALTAZAR/DATA/SOUND/0_GRP0/0_SEL.WAV
    SoundSample ui_flip;    // BALTAZAR/DATA/SOUND/0_GRP0/1_FLIP.WAV
    int         roll_voice; // Continuous loop handle for tire rolling
    int         skid_voice; // Continuous loop handle for tire squeal
    bool        loaded;
} GeneralSFX;

/* ========================================================================= */
/* Mixer & Subsystem Lifecycle API                                           */
/* ========================================================================= */

/**
 * @brief Initialize SDL2 audio device and allocate 32 software mixer voices.
 * @original FUN_0041f9b0 (MAINDOS_32BIT.EXE @ 0x0041f9b0, main.c)
 * @fidelity ADAPTED
 * @return true on success, false on failure
 */
bool Audio_Init(void);

/**
 * @brief Close audio hardware device and release all mixer voice buffers.
 * @original FUN_00412530 (MAINDOS_32BIT.EXE @ 0x00412530, main.c)
 * @fidelity ADAPTED
 */
void Audio_Shutdown(void);

/**
 * @brief Allocate an active mixer voice and begin playback of a PCM sample.
 * @original FUN_004579b0 (MAINDOS_32BIT.EXE @ 0x004579b0, main.c)
 * @fidelity ADAPTED
 *
 * @param sample Pointer to PCM sound sample
 * @param volume Master volume (0.0f silence to 1.0f full volume)
 * @param pitch Frequency multiplier (1.0f = normal speed, 2.0f = double pitch)
 * @param pan Stereo panning (-1.0f full left, 0.0f center, +1.0f full right)
 * @param loop True for continuous looping
 * @return Positive voice handle on success, 0 on failure/voice exhaustion
 */
int Audio_PlayVoice(const SoundSample *sample, float volume, float pitch, float pan, bool loop);

/**
 * @brief Update real-time playback parameters for an active voice.
 * @original FUN_00457aa0 (MAINDOS_32BIT.EXE @ 0x00457aa0, main.c)
 * @fidelity ADAPTED
 *
 * @param voice_handle Handle returned by Audio_PlayVoice
 * @param volume Master volume (0.0f to 1.0f)
 * @param pitch Frequency multiplier
 * @param pan Stereo panning (-1.0f to +1.0f)
 */
void Audio_SetVoiceParams(int voice_handle, float volume, float pitch, float pan);

/**
 * @brief Stop and release a specific active voice.
 * @original FUN_00457980 (MAINDOS_32BIT.EXE @ 0x00457980, main.c)
 * @fidelity ADAPTED
 *
 * @param voice_handle Handle returned by Audio_PlayVoice
 */
void Audio_StopVoice(int voice_handle);

/**
 * @brief Stop and release all currently playing voices.
 * @original FUN_00457980 (MAINDOS_32BIT.EXE @ 0x00457980, main.c)
 * @fidelity ADAPTED
 */
void Audio_StopAllVoices(void);

/**
 * @brief Set global master volume levels for sound effects and music.
 * @original FUN_00457ae0 (MAINDOS_32BIT.EXE @ 0x00457ae0, main.c)
 * @fidelity ADAPTED
 *
 * @param sfx_volume Volume multiplier for sound effects (0.0f to 1.0f)
 * @param music_volume Volume multiplier for music streaming (0.0f to 1.0f)
 */
void Audio_SetMasterVolume(float sfx_volume, float music_volume);

/**
 * @brief Query current master volume levels.
 * @fidelity INFRASTRUCTURE
 */
void Audio_GetMasterVolume(float *out_sfx, float *out_music);

/**
 * @brief Helper for one-shot SFX playback.
 * @fidelity INFRASTRUCTURE
 */
int Audio_PlaySFX(const SoundSample *sample, float volume, float pan);

/**
 * @brief Play spatialized 3D audio relative to listener camera/car.
 * @original FUN_0043e710 (MAINDOS_32BIT.EXE @ 0x0043e710, main.c)
 * @fidelity ADAPTED
 *
 * @param sample Pointer to sound sample
 * @param emitter_x World X position of sound source
 * @param emitter_z World Z position of sound source
 * @param listener_x World X position of camera / player car
 * @param listener_z World Z position of camera / player car
 * @param base_volume Base sound volume
 * @param max_distance Distance beyond which sound is silent
 * @return Voice handle or 0
 */
int Audio_PlaySpatialSFX(const SoundSample *sample,
                         float emitter_x, float emitter_z,
                         float listener_x, float listener_z,
                         float base_volume, float max_distance);

/* ========================================================================= */
/* Engine Sound Synthesis API (ENGINE.INF)                                   */
/* ========================================================================= */

/**
 * @brief Load ENGINE.INF curve tables and 00_A.WAV / 01_A.WAV for a car.
 * @original FUN_0041f9b0 (MAINDOS_32BIT.EXE @ 0x0041f9b0, main.c)
 * @fidelity ADAPTED
 *
 * @param ea Engine audio container
 * @param car_dir Path to vehicle directory (e.g. "assets/CARS/COOPER")
 * @return true on success, false on failure
 */
bool EngineAudio_Init(EngineAudio *ea, const char *car_dir);

/**
 * @brief Update engine audio pitch and volume crossfade based on vehicle speed.
 * @original FUN_004452c0 (MAINDOS_32BIT.EXE @ 0x004452c0, main.c)
 * @fidelity ADAPTED
 * @deviation DEV-006 (High-RPM pitch curve safety clamp)
 * @fix_category FIX_CAT_AUDIO
 *
 * @param ea Engine audio container
 * @param speed_abs Absolute longitudinal speed
 * @param top_speed Vehicle maximum speed specification
 * @param is_airborne True if vehicle is currently airborne (engine revs free)
 * @param fixes Active game fix options
 */
void EngineAudio_Update(EngineAudio *ea, double speed_abs, double top_speed,
                        bool is_airborne, const GameFixOptions *fixes);

/**
 * @brief Stop engine voices and free audio memory.
 * @fidelity INFRASTRUCTURE
 */
void EngineAudio_Free(EngineAudio *ea);

/* ========================================================================= */
/* Sound Pool & WAV Loader API                                               */
/* ========================================================================= */

/**
 * @brief Load a RIFF/WAVE PCM audio file from disk into 16-bit PCM format.
 * @original FUN_00458e00 (MAINDOS_32BIT.EXE @ 0x00458e00, main.c)
 * @fidelity ADAPTED
 *
 * @param path Filesystem path to .WAV file
 * @param out_sample Destination SoundSample structure
 * @return true on success, false on failure
 */
bool SoundPool_LoadWav(const char *path, SoundSample *out_sample);

/**
 * @brief Load a Gravis UltraSound GF1 .PAT audio file from disk into 16-bit PCM format.
 * @original FUN_004582b0 (MAINDOS_32BIT.EXE @ 0x004582b0, main.c)
 * @fidelity ADAPTED
 *
 * @param path Filesystem path to .PAT file
 * @param out_sample Destination SoundSample structure
 * @return true on success, false on failure
 */
bool SoundPool_LoadPat(const char *path, SoundSample *out_sample);

/**
 * @brief Release allocated PCM memory of a SoundSample.
 * @fidelity INFRASTRUCTURE
 */
void SoundPool_FreeSample(SoundSample *sample);


/**
 * @brief Load the standard general sound effect pool (ROLL, SKID, COLL, BOOST).
 * @original FUN_0041f9b0 (MAINDOS_32BIT.EXE @ 0x0041f9b0, main.c)
 * @fidelity ADAPTED
 *
 * @param sfx Destination GeneralSFX bank
 * @param assets_dir Root assets path (e.g. "assets")
 * @return true on success, false on failure
 */
bool SoundPool_LoadGeneralSFX(GeneralSFX *sfx, const char *assets_dir);

/**
 * @brief Release all samples in the general SFX bank.
 * @fidelity INFRASTRUCTURE
 */
void SoundPool_FreeGeneralSFX(GeneralSFX *sfx);

/* ========================================================================= */
/* Digital CD-DA Soundtrack Streaming API                                    */
/* ========================================================================= */

/**
 * @brief Map circuit index to CD soundtrack track number.
 * @original DAT_00497eb8 (MAINDOS_32BIT.EXE @ 0x00419260, main.c)
 * @fidelity EXACT
 *
 * @param track_index Circuit index (0..6)
 * @return CD track index (e.g. 2 for Canada, 3 for Austria)
 */
int Music_GetTrackForCircuit(int track_index);

/**
 * @brief Begin streaming a music track file (e.g. Track02.ogg).
 * @original FUN_00457ed0 (MAINDOS_32BIT.EXE @ 0x00457ed0, main.c)
 * @fidelity ADAPTED
 *
 * @param track_number Track index (2..8)
 * @param loop True for seamless loop playback
 * @return true on success, false on failure
 */
bool Music_PlayTrack(int track_number, bool loop);

/**
 * @brief Stop digital music streaming.
 * @original FUN_00457ed0 (MAINDOS_32BIT.EXE @ 0x00457ed0, main.c)
 * @fidelity ADAPTED
 */
void Music_Stop(void);

/**
 * @brief Set music playback volume.
 * @original FUN_00457ae0 (MAINDOS_32BIT.EXE @ 0x00457ae0, main.c)
 * @fidelity ADAPTED
 *
 * @param volume Music volume (0.0f to 1.0f)
 */
void Music_SetVolume(float volume);

/**
 * @brief Check if music is actively streaming.
 * @original FUN_00457b10 (MAINDOS_32BIT.EXE @ 0x00457b10, main.c)
 * @fidelity ADAPTED
 */
bool Music_IsPlaying(void);

/**
 * @brief Update music streaming buffer (call periodically or from main loop).
 * @original FUN_0041775a (MAINDOS_32BIT.EXE @ 0x0041775a, main.c)
 * @fidelity ADAPTED
 */
void Music_Update(void);

#endif /* IGNITION_AUDIO_H */
