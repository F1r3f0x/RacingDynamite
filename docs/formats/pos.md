# Ignition (1997) .POS Scenery Object Animation Format

## Overview
`.POS` files (e.g. `LEVELS/<TRACK>/<TRACK>.POS`) define position and orientation keyframe animation tracks for dynamic scenery objects (e.g. ski lifts, boats, airplanes, rotating windmills, flags, and stadium blimps).

Historically assumed to be AI waypoints, decompilation of `Track_LoadAllAssets` (`0x00418dd0`), `Pos_InitAnimatedObjects` (`0x004356d0`), and `Pos_UpdateAnimatedObjects` (`0x004357a0`) reveals that `.POS` contains delta keyframes bound to specific `.PLC` scenery object instances.

---

## File Structure & Memory Layout

A `.POS` file consists of two sections:
1. **Object Offset Table**: An array of `int32_t` entries, one per object in `.PLC` (`plc->count`).
   - If an object is static, its entry is `-1` (`0xFFFFFFFF`).
   - If an object has animation keyframes, its entry is a relative dword offset $D$ such that the keyframe track begins at:
     $$\text{byte\_offset} = (\text{object\_index} + D) \times 4$$
2. **Keyframe Track Streams**: Sequences of 24-byte keyframe records preceded by a header.

```c
#pragma pack(push, 1)

// Ignition .POS Keyframe Record (24 bytes)
typedef struct {
    int32_t pos_x;            // 0x00: X position delta / coordinate
    int32_t pos_y;            // 0x04: Y elevation delta / coordinate
    int32_t pos_z;            // 0x08: Z position delta / coordinate
    int32_t rot_x;            // 0x0C: Pitch angle (0..3599 tenths of degree)
    int32_t rot_y;            // 0x10: Yaw angle (0..3599 tenths of degree)
    int32_t rot_z;            // 0x14: Roll angle (0..3599 tenths of degree)
} PosKeyframe;

// Ignition .POS Track Header (8 bytes)
typedef struct {
    int32_t frame_count;      // Total animation keyframes in track
    int32_t current_frame;    // Playhead index (0 during loading)
    // Followed immediately by PosKeyframe[frame_count]
} PosTrackHeader;

#pragma pack(pop)
```

---

## Animation Processing Engine Loop

1. **Initialization (`Pos_InitAnimatedObjects`, `0x004356d0`)**:
   - Loops through all `.PLC` objects.
   - For objects with `pos_table[i] != -1`, converts keyframe coordinate deltas to relative displacements between consecutive frames.
2. **Per-Frame Update (`Pos_UpdateAnimatedObjects`, `0x004357a0`)**:
   - Increments `current_frame`. When reaching `frame_count`, loops back to `0`.
   - Displaces object position by current delta:
     $$\text{obj.pos} = \text{obj.pos} - \text{keyframe.delta}$$
   - Sets object orientation matrix from `(rot_x, rot_y, rot_z)`.
   - Calls `Lisa_MoveObject` (`0x00446f30`). If it fails, emits engine panic:
     `"FEL VID LI_MOVEOBJECT ANIM OBJ"`

---

## Real File Inspection Across Circuits

| Track | File Size | Animated Objects Count | Example Object Indices | Typical Keyframe Counts |
| :--- | :--- | :--- | :--- | :--- |
| `AUSTRIA` | 112,356 B | 20 objects | 256, 257, 258, 302..320 | 401 frames (ski lift cables & cabins) |
| `BRAZIL` | 12,428 B | 5 objects | 282, 299, 300, 301, 315 | 33 to 111 frames (boats, signs) |
| `CANADA` | 120,216 B | 10 objects | 208, 209, 210, 211, 212 | 74 to 551 frames (trains, timber cranes) |
| `CARIB` | 78,236 B | 11 objects | 332, 333, 334, 335, 339 | 11 to 400 frames (yachts, windmills) |
| `ICELAND` | 8,060 B | 3 objects | 192, 193, 247 | 55 to 143 frames (geyser plumes, radar) |
| `JAPAN` | 124,204 B | 10 objects | 250, 251, 252, 253, 254 | 201 frames (bullet train, blimp) |
| `USA` | 92,540 B | 10 objects | 173, 174, 181, 182, 198 | 50 to 288 frames (airplanes, neon billboards) |
