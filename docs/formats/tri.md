# Ignition (1997) .TRI Track Spline & Road Index Format

## Overview
`.TRI` files (e.g. `LEVELS/<TRACK>/<TRACK>.TRI`) define road boundary vertex indices, centerline splines, track width limits, and fork branching directives for all road surface segments in Ignition.

Each `.TRI` file corresponds 1-to-1 with the placed objects table (`.PLC`), allocating a fixed 500-byte record for every object index in the track.

---

## File Header & Record Structure

```c
#pragma pack(push, 1)

// Ignition .TRI File Header (4 bytes)
typedef struct {
    int32_t chunk_count;      // Total chunk records in file (matches PLC object count)
} TriHeader;

// Ignition .TRI Chunk Record (500 bytes per PLC object)
typedef struct {
    uint8_t  road_code;       // 0x00: Road surface attribute / code
    int16_t  left_vertex;     // 0x01: Submesh vertex index for left road boundary
    uint8_t  padding1[20];    // 0x03..0x16: Secondary surface attributes
    int16_t  right_vertex;    // 0x17: Submesh vertex index for right road boundary
    uint8_t  padding2[84];    // 0x19..0x6C: Internal physics / friction parameters
    uint8_t  fork_flag;       // 0x6D: Fork directive (0 = normal, 1 = fork left, 2 = fork right)
    uint8_t  reserved[390];   // 0x6E..0x1F3: Reserved / terrain collision bounds
} TriChunk;

#pragma pack(pop)
```

### Binary Layout Table

| Byte Offset | Field Name | C Type | Size (Bytes) | Description |
| :--- | :--- | :--- | :--- | :--- |
| `0x0000` | `chunk_count` | `int32_t` | 4 | Number of 500-byte records in the file. |
| `0x0004 + N*500 + 0` | `road_code` | `uint8_t` | 1 | Road surface material / spline attribute code. |
| `0x0004 + N*500 + 1` | `left_vertex` | `int16_t` | 2 | Index into submesh vertex buffer for left road rail. |
| `0x0004 + N*500 + 3` | `padding1` | `uint8_t[20]` | 20 | Sub-attributes and boundary flags. |
| `0x0004 + N*500 + 23` | `right_vertex` | `int16_t` | 2 | Index into submesh vertex buffer for right road rail. |
| `0x0004 + N*500 + 25` | `padding2` | `uint8_t[84]` | 84 | Surface friction / normal modifiers. |
| `0x0004 + N*500 + 109` | `fork_flag` | `uint8_t` | 1 | Circuit branching: `0` = mainline, `1` = left fork, `2` = right fork. |
| `0x0004 + N*500 + 110` | `reserved` | `uint8_t[390]` | 390 | Road collision bounds and alignment padding. |

---

## Waypoint & Road Boundary Construction

In `Track_LoadSplines` (`0x00414e40`), the engine pairs each road chunk from `.PLC` with its `.TRI` record:
1. **Submesh Offset**: Looked up via `plc->objects[chunk_idx].submesh_offset`.
2. **Left Boundary World Coordinate**:
   $$\vec{P}_{left} = \text{vertex}[\text{left\_vertex}] + \text{plc\_pos}$$
3. **Right Boundary World Coordinate**:
   $$\vec{P}_{right} = \text{vertex}[\text{right\_vertex}] + \text{plc\_pos}$$
4. **Waypoint Centerline**:
   $$\vec{P}_{center} = \frac{\vec{P}_{left} + \vec{P}_{right}}{2}$$
5. **Road Width**:
   $$W = \|\vec{P}_{right} - \vec{P}_{left}\|$$

---

## Example Real Data Across Circuits

| Track | File Size | Chunks Count | Start Chunk Index | Start Left Vertex | Start Right Vertex | Road Width (Units) |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `AUSTRIA` | 160,504 B | 321 | 215 | 28 | 23 | ~389 units |
| `BRAZIL` | 157,504 B | 315 | 0 | 28 | 23 | ~420 units |
| `CANADA` | 141,004 B | 282 | 0 | 28 | 23 | ~410 units |
| `CARIB` | 171,504 B | 343 | 0 | 28 | 23 | ~450 units |
| `ICELAND` | 123,504 B | 247 | 0 | 28 | 23 | ~395 units |
| `JAPAN` | 225,504 B | 451 | 0 | 28 | 23 | ~400 units |
| `USA` | 111,504 B | 223 | 0 | 28 | 23 | ~430 units |
