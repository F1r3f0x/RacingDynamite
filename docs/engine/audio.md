# Audio Subsystem & Soundtrack Streaming Architecture

This document specifies the reverse-engineered sound architecture, 32-channel software voice mixer, vehicle acoustic synthesizer, sound effect pools, and CD-DA soundtrack streaming of **Ignition** (1997, Unique Development Studios / Virgin Interactive), reconstructed from `MAINDOS_32BIT.EXE` (`0x0041f9b0`, `0x004452c0`, `0x00458e00`, `0x00457890`, `0x004579b0`, `0x00457ed0`) and `MAINDOS.EXE` (`main.c`).

---

## 1. Subsystem Overview & Architectural Topology

Ignition utilizes a dual-engine sound pipeline:
1. **Digital Sound Effects & Engine Synthesizer**: Original Windows 95 builds communicated via DirectSound or fallback `waveOut` with a 32-channel software voice mixer. In Racing Dynamite, this is implemented as an authentic 32-channel software mixer outputting to an SDL2 audio callback stream at 44,100 Hz, 16-bit stereo.
2. **CD-DA Digital Audio Soundtrack**: Original retail CD-ROM copies streamed Red Book audio from CD tracks 2 through 8 based on the selected circuit. Racing Dynamite streams high-fidelity digital Ogg Vorbis recordings (`Track02.ogg` through `Track08.ogg`) via public-domain single-header `stb_vorbis`.

```
+------------------------------------------------------------------------------------+
|                               Game State / Physics                                 |
|               Vehicle Speed / RPM / Surface Impact / Boost / UI                    |
+-----------------------------------------+------------------------------------------+
                                          |
        +---------------------------------+---------------------------------+
        |                                                                   |
        v                                                                   v
+-------------------------------+                         +----------------------------------+
|   Vehicle Acoustic Synthesizer|                         |       General Sound Effects      |
|        (engine_audio.c)       |                         |          (sound_pool.c)          |
|  - Reads 800-byte ENGINE.INF  |                         |  - UI clicks (01_KLICK, 02_OK)   |
|  - Dual voices (00_A, 01_A)   |                         |  - Boost (00_BOOST.WAV)          |
|  - 16.16 pitch modulation     |                         |  - Impacts, skids, horn          |
+---------------+---------------+                         +-----------------+----------------+
                |                                                           |
                +-----------------------------+-----------------------------+
                                              |
                                              v
                              +-------------------------------+
                              |    32-Channel Software Mixer  |
                              |           (audio.c)           |
                              |  - 16.16 Fixed-Point Resample |
                              |  - Equal-power stereo panning |
                              |  - Master volume attenuation  |
                              |  - Saturation sample clamping |
                              +---------------+---------------+
                                              |
                                              v
                              +-------------------------------+
                              |       SDL2 Audio Stream       |
                              |   (44,100 Hz, S16SYS, Stereo) |
                              +-------------------------------+
```

---

## 2. 32-Channel Software Voice Mixer (`src/audio/audio.c`)

### 2.1 Mixer Architecture & State Representation
The mixer maintains an internal pool of 32 simultaneous hardware/software audio voices (`MAX_AUDIO_VOICES = 32`), matching the original DirectSound allocation in `Sound_InitAndLoadPools` (`0x0041f9b0`).

Each active voice tracks:
* Sample pointer (`SoundSample*` holding 16-bit signed PCM).
* 16.16 fixed-point playback playhead (`playhead_fp`): integer part is the sample index, fractional part is the sub-sample position.
* 16.16 fixed-point step increment (`step_fp`): determined by the voice pitch and native sample rate.
* Playback state: `active`, `looping`, `paused`.
* Volume attenuation ($0.0$ to $1.0$) and stereo panning ($-1.0$ full left, $0.0$ center, $+1.0$ full right).

### 2.2 16.16 Fixed-Point Linear Resampling
Original game sound effects are encoded as 8-bit unsigned mono PCM at varying rates (11,025 Hz, 14,385 Hz, 22,050 Hz). When mixed into the 44,100 Hz stereo output buffer, voices are dynamically resampled using 16.16 fixed-point linear interpolation:

$$\text{step\_fp} = \left\lfloor \frac{f_{\text{sample}} \cdot \text{pitch}}{44,100} \cdot 65,536 \right\rfloor$$

For each frame, the interpolated sample value is computed as:

$$i = \text{playhead} \gg 16$$

$$\text{frac} = \frac{\text{playhead} \ \& \ \text{0xFFFF}}{65,536}$$

$$s = (1.0 - \text{frac}) \cdot \text{sample}[i] + \text{frac} \cdot \text{sample}[i + 1]$$

### 2.3 Stereo Panning Law
Panning uses linear balance attenuation:
$$\text{gain}_L = \text{volume} \cdot \left(1.0 - \max(0, \text{pan})\right)$$
$$\text{gain}_R = \text{volume} \cdot \left(1.0 + \min(0, \text{pan})\right)$$

Mixed samples are accumulated into 32-bit integer accumulator buffers to prevent intermediate clipping and clamped to $[-32,768, 32,767]$ on output.

---

## 3. Vehicle Acoustic Synthesizer (`ENGINE.INF`)

### 3.1 Binary Curve Structure (`ENGINE.INF`)
Each vehicle directory (`assets/CARS/<CAR>/SOUND/`) contains an `ENGINE.INF` file defining the engine's acoustic profile. It consists of exactly **800 bytes** with **no header**, structured as **four contiguous 200-byte `uint8_t` lookup tables**:

| Byte Range | Size | Field | Type | Value Range | Description |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `0x000`–`0x0C7` | 200 bytes | `sample0_volume` | `uint8_t[200]` | 0–100 | Volume envelope for low-RPM sample (`00_A.WAV`). |
| `0x0C8`–`0x18F` | 200 bytes | `sample0_pitch`  | `uint8_t[200]` | 0–100 | Pitch modulation curve for `00_A.WAV`. |
| `0x190`–`0x257` | 200 bytes | `sample1_volume` | `uint8_t[200]` | 0–100 | Volume envelope for high-RPM sample (`01_A.WAV`). |
| `0x258`–`0x31F` | 200 bytes | `sample1_pitch`  | `uint8_t[200]` | 0–100 | Pitch modulation curve for `01_A.WAV`. |

Verified by `Sound_InitAndLoadPools` (`0x0041f9b0`) decompilation — the engine copies bytes (not dwords) at offsets 0, 200, 400, 600 into per-vehicle `uint8_t` arrays:
```c
for (int i = 0; i < 200; ++i) {
    car[idx].engine_s0_vol[i]   = ((uint8_t*)pvVar4)[i];         // table 0: 0x000
    car[idx].engine_s0_pitch[i] = ((uint8_t*)pvVar4)[i + 200];   // table 1: 0x0C8
    car[idx].engine_s1_vol[i]   = ((uint8_t*)pvVar4)[i + 400];   // table 2: 0x190
    car[idx].engine_s1_pitch[i] = ((uint8_t*)pvVar4)[i + 600];   // table 3: 0x258
}
```

All 11 vehicle `ENGINE.INF` files are exactly 800 bytes and use `uint8_t` values in the range `[0, 100]`. See [`docs/formats/inf.md`](file:///c:/Stuff/Proyects/RacingDynamite/docs/formats/inf.md) for the authoritative format spec and empirical curve samples.

### 3.2 Real-Time RPM Pitch Synthesis (`FUN_004452c0`)
During every physics tick, the vehicle powertrain speed is mapped to the `ENGINE.INF` tables:

1. **Normalized Speed Ratio**:
   $$\text{ratio} = \frac{|\text{speed}|}{\text{max\_speed}}$$
2. **Curve Sample Index**:
   $$k = \lfloor \text{ratio} \cdot 199.0 \rfloor$$
3. **Authentic Bug & Clamping (`DEV-006`)**:
   In `MAINDOS_32BIT.EXE`, when using turbo boost on fast vehicles (e.g. Vegas, Monster Truck), speed exceeds `max_speed`, causing $k \ge 200$. The original binary performed an unchecked read past the 800-byte buffer, corrupting pitch modulation and silencing the engine channel.
   Under `fixes->fix_audio` (`DEV-006`), $k$ is clamped to 199.
4. **Dual-Sample Cross-Fade**:
   Vehicles utilize two looping engine sound files:
   * `00_A.WAV`: Low-RPM rumble and intake drone.
   * `01_A.WAV`: High-RPM mechanical whine and exhaust roar.

   Volume and pitch for each sample at speed index $k$:
   $$\text{vol}_0 = \frac{\text{sample0\_volume}[k]}{100.0f}, \quad \text{pitch}_0 = 0.5f + \frac{\text{sample0\_pitch}[k]}{100.0f}$$
   $$\text{vol}_1 = \frac{\text{sample1\_volume}[k]}{100.0f}, \quad \text{pitch}_1 = 0.5f + \frac{\text{sample1\_pitch}[k]}{100.0f}$$

---

## 4. CD-DA Digital Audio Soundtrack Streaming

### 4.1 Circuit-to-CD Track Mapping (`DAT_00497eb8`)
In `MAINDOS_32BIT.EXE` at global address `0x00497eb8`, an 8-byte array maps each circuit index to its corresponding CD-DA audio track:

| Circuit Index | Circuit Name | CD-DA Track | File Name | Style / Theme |
| :---: | :---: | :---: | :---: | :---: |
| 0 | Moose Jaw Falls (Austria) | Track 2 | `Track02.ogg` | Rock / Accordion upbeat |
| 1 | Snake Island (Brazil) | Track 3 | `Track03.ogg` | Latin Samba / Percussion |
| 2 | Yoshi City (Japan) | Track 6 | `Track06.ogg` | Techno / Electronic synth |
| 3 | Goldstone Park (USA) | Track 7 | `Track07.ogg` | Southern Country / Slide guitar |
| 4 | Caldera Peak (Iceland) | Track 6 | `Track06.ogg` | Techno / Industrial |
| 5 | Henderson (USA 2) | Track 7 | `Track07.ogg` | Southern Country rock |
| 6 | Volcano (Iceland 2) | Track 8 | `Track08.ogg` | Dark electronic ambience |

### 4.2 Seamless Streaming via `stb_vorbis`
* Track playback initializes asynchronously without stalling the simulation loop.
* Decoded audio is loaded into a dedicated streaming voice channel and configured for continuous looping.
* Master music volume (`g_MasterMusicVolume`) modulates stream gain independently from SFX volume (`g_MasterSoundVolume`).
