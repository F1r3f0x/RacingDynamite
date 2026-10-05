# Ignition (1997) 3D Mesh & Triangle Formats (.MSH, .TRI)

## 1. Track Mesh Format (`.MSH`)

Track and car 3D geometry is stored in `.MSH` files (`LEVELS/<TRACK>/<TRACK>.MSH` and `CARS/CARS.MSH`).

Original source references: `Mesh_LoadTrackAndCars` (`0x00419bd0`) and `Mesh_InstantiatePlacedObjects` (`0x00416250`).

### Binary Layout
`.MSH` files consist of concatenated submesh blocks referenced by `submesh_offset` (measured in 4-byte dwords, multiply by 4 for byte offset) from `.PLC` placed object tables:

| Field | Type | Size | Description |
| :--- | :--- | :--- | :--- |
| `vertex_count` | `int32_t` | 4 bytes | Total number of 3D vertices in this submesh. |
| `polygon_count`| `int32_t` | 4 bytes | Number of polygon records in the stream. |
| `vertices[]`   | `int32_t[3]` | $12 \times \text{vertex\_count}$ | Array of 3D integer coordinates `(X, Y, Z)` in local object space. |
| `polygons[]`   | `MshPolygon` | $44 \times \text{polygon\_count}$| Stream of 44-byte polygon records. |

### Polygon Record Layout (`MshPolygon` - 44 bytes / 11 x `uint32_t`)
Each polygon starts with an opcode byte dispatched via opcode table `PTR_LAB_0049c8e0` in `MAINDOS.EXE` (`Lisa_RenderSubmeshes` `0x0044c1f0`):

| Offset | Type | Field | Description |
| :--- | :--- | :--- | :--- |
| `0x00` | `uint32_t` | `header` | Low byte = opcode (`0x11`, `0x12`, `0x13`, `0x15`, `0x17`); high 24 bits = flags / shading. |
| `0x04` | `uint32_t` | `vi0` | Index of first vertex in submesh vertex array. |
| `0x08` | `uint32_t` | `vi1` | Index of second vertex in submesh vertex array. |
| `0x0C` | `uint32_t` | `vi2` | Index of third vertex in submesh vertex array. |
| `0x10` | `int32_t`  | `tu0` | Vertex 0 U texture coordinate (8.8 fixed-point: divide by 65536.0 for $[0.0, 1.0]$, or by 256.0 for texel index $[0, 255]$). |
| `0x14` | `int32_t`  | `tv0` | Vertex 0 V texture coordinate (8.8 fixed-point). |
| `0x18` | `int32_t`  | `tu1` | Vertex 1 U texture coordinate (8.8 fixed-point). |
| `0x1C` | `int32_t`  | `tv1` | Vertex 1 V texture coordinate (8.8 fixed-point). |
| `0x20` | `int32_t`  | `tu2` | Vertex 2 U texture coordinate (8.8 fixed-point). |
| `0x24` | `int32_t`  | `tv2` | Vertex 2 V texture coordinate (8.8 fixed-point). |
| `0x28` | `uint32_t` | `extra`| Texture page byte offset within `.TEX` file (`page_index * 65536`). |


### Common Polygon Opcodes:
* `0x11`: Standard perspective-correct textured triangle (`0x0044c2e0`).
* `0x12`: Flat/shaded triangle with depth bias (`0x0044caa0` $\rightarrow$ `0x0044cb20`).
* `0x13`: Textured triangle with alpha/transparent test (`0x0044cac0` $\rightarrow$ `0x0044cb20`).
* `0x15`: Textured triangle with backface culling & mip/LOD selection (`0x0044d550`).
* `0x17`: Double-sided textured triangle (`0x0044cb00` $\rightarrow$ `0x0044cb20`).

When an object is placed on the circuit via `.PLC`:
$$\vec{V}_{\text{world}} = \vec{V}_{\text{local}} + (pos\_x, pos\_y, pos\_z)$$

---

## 2. Triangle Chunks Format (`.TRI`)

`.TRI` files define the track layout chunks, camera spline triggers, road segments, and collision bounds.

### Binary Layout
* **Header (4 bytes)**:
  * `uint32_t road_chunk_count`: Number of main circuit road chunks (e.g. 305 on Austria).
* **Chunk Table ($N \times 500$ bytes)**:
  * Each chunk is fixed at exactly **500 bytes**.
  * Total chunks matches `.PLC` placed objects 1-to-1 (e.g. 321 chunks on Austria $\times 500 = 160,500$ bytes).

| Chunk Offset | Type | Size | Description |
| :--- | :--- | :--- | :--- |
| `0x00` | `uint8_t` | 1 byte | Chunk segment flags (`0x04` = standard road segment). |
| `0x01` | `uint16_t` | 2 bytes | Primary vertex index into track `.MSH`. |
| `0x17` (`23`) | `uint16_t` | 2 bytes | Secondary vertex index. |
| `0x6D` (`109`)| `uint8_t` | 1 byte | Branching / shortcut indicator (`0` = single lane, `1` or `2` = junction/fork). |
| `0x6E`–`0x1F3`| `uint8_t[389]`| 389 bytes | Triangle indices, UV coordinates, and obstacle bounding spheres. |
