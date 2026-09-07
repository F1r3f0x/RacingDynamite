# Ignition (1997) ENGINE.INF Audio Pitch & Volume Format

## Overview

Each playable and AI vehicle in Ignition possesses an `ENGINE.INF` curve definition file located at `assets/CARS/<CAR>/SOUND/ENGINE.INF`. 

This file defines the real-time acoustic signature of the vehicle's engine as it accelerates, shifts gears, and reaches terminal velocity. It enables smooth acoustic crossfading between two audio loop samples:
* `00_A.WAV`: Low-RPM engine rumble / idle loop.
* `01_A.WAV`: High-RPM engine whine / throttle loop.

---

## 1. Binary Layout & Structure

The file is exactly **800 bytes** in length with no header or magic number. It consists of four contiguous 200-byte lookup tables:

| Byte Range | Size | Field Name | Type | Value Range | Description |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `0x000` .. `0x0C7` | 200 bytes | `sample0_volume` | `uint8_t[200]` | $0 \dots 100$ | Volume curve for low-RPM sample (`00_A.WAV`). |
| `0x0C8` .. `0x18F` | 200 bytes | `sample0_pitch`  | `uint8_t[200]` | $0 \dots 100$ | Frequency / pitch modulation for `00_A.WAV`. |
| `0x190` .. `0x257` | 200 bytes | `sample1_volume` | `uint8_t[200]` | $0 \dots 100$ | Volume curve for high-RPM sample (`01_A.WAV`). |
| `0x258` .. `0x31F` | 200 bytes | `sample1_pitch`  | `uint8_t[200]` | $0 \dots 100$ | Frequency / pitch modulation for `01_A.WAV`. |

### C Structure Definition:

```c
typedef struct {
    uint8_t sample0_volume[200]; // 0x000: Low-RPM volume (0 = silence, 100 = 100% volume)
    uint8_t sample0_pitch[200];  // 0x0C8: Low-RPM pitch multiplier curve
    uint8_t sample1_volume[200]; // 0x190: High-RPM volume (0 = silence, 100 = 100% volume)
    uint8_t sample1_pitch[200];  // 0x258: High-RPM pitch multiplier curve
} EngineInf;
```

---

## 2. Ghidra Decompilation & Binary Verification

In `IGN_WIN.EXE`, `ENGINE.INF` is loaded during race sound initialization in `FUN_0041f9b0` (`Sound_Init`):

```c
// Decompiled snippet from FUN_0041f9b0 (0x0041f9b0)
_sprintf(path, "%s%s\\SOUND\\ENGINE.INF", "CARS\\", s_CarNames[car_idx]);
pvVar4 = File_LoadToMemory(path);
if (pvVar4 != NULL) {
    for (int i = 0; i < 200; ++i) {
        car[idx].engine_s0_vol[i]   = ((uint8_t*)pvVar4)[i];       // +0x658
        car[idx].engine_s0_pitch[i] = ((uint8_t*)pvVar4)[i + 200]; // +0x720
        car[idx].engine_s1_vol[i]   = ((uint8_t*)pvVar4)[i + 400]; // +0x7e8
        car[idx].engine_s1_pitch[i] = ((uint8_t*)pvVar4)[i + 600]; // +0x8b0
    }
    Mem_Free(pvVar4);
}
```

Notice that the four tables are copied directly into the vehicle runtime state struct (`CarEntity`) at member offsets `+0x658`, `+0x720`, `+0x7e8`, and `+0x8b0` respectively.

---

## 3. Real-World Vehicle Acoustic Curves

Below is an empirical sampling of curves extracted from `CARS/COOPER/SOUND/ENGINE.INF` across the 200 speed tiers:

| Speed Tier Index | Low-RPM Vol (`V0`) | Low-RPM Pitch (`P0`) | High-RPM Vol (`V1`) | High-RPM Pitch (`P1`) | Acoustic State |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **0** (Idle) | `32` | `100` | `0` | `0` | Idle throb on sample 0 only. |
| **20** | `45` | `100` | `3` | `0` | Low gear pull; sample 1 barely audible. |
| **50** | `65` | `100` | `20` | `0` | Mid-range acceleration. |
| **80** | `85` | `100` | `36` | `41` | Sample 1 begins pitching up. |
| **100** (Mid-Speed) | `98` | `67` | `47` | `70` | Active crossfade between both samples. |
| **120** | `100` | `35` | `59` | `100` | Sample 0 fading; sample 1 takes over. |
| **150** | `100` | `0` | `75` | `100` | High-speed screaming engine. |
| **199** (Top Speed) | `100` | `0` | `98` | `100` | Maximum velocity engine output. |

### Verification Across All 11 Vehicles:
All 11 vehicles in the game (`COOPER`, `COP`, `DODGE`, `JEEP`, `MUSTANG`, `NASCAR`, `PORSCHE`, `SCHOOL`, `TRUCK`, `VAN`, `VW`) have identical file sizes of exactly **800 bytes**.

---

## 4. Source Port Implementation Formula

During the audio update tick:
1. Calculate the vehicle's normalized RPM / speed index $I \in [0, 199]$:
   $$I = \text{clamp}\left(\left\lfloor \frac{|v|}{\text{top\_speed}} \times 199.0 \right\rfloor, 0, 199\right)$$
2. Look up the volume and pitch factors:
   $$\text{vol}_0 = \frac{\text{sample0\_volume}[I]}{100.0f}, \quad \text{pitch}_0 = 0.5f + \frac{\text{sample0\_pitch}[I]}{100.0f} \times 1.0f$$
   $$\text{vol}_1 = \frac{\text{sample1\_volume}[I]}{100.0f}, \quad \text{pitch}_1 = 0.5f + \frac{\text{sample1\_pitch}[I]}{100.0f} \times 1.0f$$
3. Update the audio mixer's voice playback parameters in real-time.
