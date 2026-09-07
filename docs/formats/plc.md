# Ignition (1997) .PLC Track Placed Objects Format

## Overview
`.PLC` files (e.g. `LEVELS/<TRACK>/<TRACK>.PLC` and `CARS/CARS.PLC`) define placed track objects, roadside scenery, collision obstacles, animated barriers, and power-up triggers.

---

## 1. File Structure

| Offset | Type | Field | Description |
| :--- | :--- | :--- | :--- |
| `0x00` | `uint32_t` | `object_count` | Number of placed object records in the file. |
| `0x04` | `PlcObject[]` | `objects` | Array of `object_count` placed object records (20 bytes each). |

**Total File Size Formula**:
$$\text{File Size} = 4 + (\text{object\_count} \times 20)\text{ bytes}$$

### Verification Across Tracks:
| Track | Size (bytes) | Object Count |
| :--- | :--- | :--- |
| **Austria** | 6,424 | 321 |
| **Brazil** | 6,304 | 315 |
| **Canada** | 5,644 | 282 |
| **Carib** | 6,864 | 343 |
| **Iceland** | 4,664 | 233 |
| **Japan** | 9,024 | 451 |
| **USA** | 4,464 | 223 |

> [!NOTE]
> The object count in `.PLC` matches the chunk count in the corresponding `.TRI` file 1-to-1. Each track segment chunk corresponds to a placed track node or obstacle.

---

## 2. Object Record Layout (`PlcObject` - 20 bytes)

```c
typedef struct {
    int32_t submesh_offset; // 0x00: Offset index into .MSH geometry (measured in 4-byte dwords)
    int32_t model_type;     // 0x04: Model type / collision archetype ID
    int32_t pos_x;          // 0x08: World X position coordinate
    int32_t pos_y;          // 0x0C: World Y elevation coordinate
    int32_t pos_z;          // 0x10: World Z position coordinate
} PlcObject;
```

---

## 3. Object Archetypes (Categorized in `Race_InitSceneAndCars`)

* **`model_id` $\in [150, 200)$**: Dynamic and animated scenery objects (windmills, ski lifts, signs).
* **`model_id` $\in [300, 350)$**: Track interaction elements (jump ramps, bridges).
* **`model_id` $= 354$ (`0x162`)**: Turbo boost power-up pad triggers.
* **`model_id` $\in [400, 500)$**: Hard collision roadside obstacles (rock boulders, barrier posts, trees).

---

## 4. 12-Bit Archetype Bitmask (`FUN_0041b360`)

In raw `.PLC` files, `model_type` often contains higher-order packed flags (e.g. `0x4022002` for USA Water Tower, `0x6002004` for Austria Clouds). During track loading in `Track_PreprocessPlacements` (`FUN_0041b360`), the engine extracts the 12-bit archetype:

$$\text{raw\_type} = \text{model\_type} \& \text{0xFFF}$$

* If $\text{raw\_type} < 100 \lor \text{raw\_type} == 200 \lor (300 \le \text{raw\_type} \le 302)$, the object is treated as a **shadow/alpha blend model** (`is_shadow_type = true`). Opcodes `0x13`/`0x17` blend through `.SHD` without Z-writing.
* Otherwise, opcodes `0x13`/`0x17` fall back to 1-bit cutout transparency (opcode `0x12`).
