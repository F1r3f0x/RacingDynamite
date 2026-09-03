# Ignition (1997) .SRF Surface & Physics Format

## Overview
`.SRF` files (located in `LEVELS/<TRACK>/<TRACK>.SRF`) contain the 3D road surface mesh, heightfield collision grid, and physics attributes. In the original source code, this subsystem was implemented in `d:\projects\ignition\getsurf\getsurf.c`.

## Header Structure (First 36 bytes = 9 x `int32_t`)

| Offset | Type | Field | Description |
| :--- | :--- | :--- | :--- |
| `0x00` | `int32_t` | `grid_cells_x` | Grid dimension along X axis (e.g. 200). |
| `0x04` | `int32_t` | `grid_cells_z` | Grid dimension along Z axis (e.g. 200). |
| `0x08` | `int32_t` | `cell_size_x` | World dimension of one grid cell in X (e.g. 512 units). |
| `0x0C` | `int32_t` | `cell_size_z` | World dimension of one grid cell in Z (e.g. 512 units). |
| `0x10` | `int32_t` | `grid_stride_x` | Grid stride multiplier for X. |
| `0x14` | `int32_t` | `grid_stride_z` | Grid stride multiplier for Z. |
| `0x18` | `int32_t` | `triangle_count` | Number of surface collision triangles. |
| `0x1C` | `int32_t` | `vertex_offset_1` | Offset/count for vertex buffer table 1. |
| `0x20` | `int32_t` | `vertex_offset_2` | Offset/count for vertex buffer table 2. |

---

## Data Sections

### 1. Spatial Grid Index Table (Offset `0x24` / Byte 36)
Immediately following the 36-byte header:
* Contains `grid_cells_x * grid_cells_z` entries.
* Each entry is **12 bytes** (`3 * int32_t`):
  * `int32_t triangle_list_offset`: Offset into triangle index buffer for triangles overlapping this cell.
  * `int32_t vertex_list_offset`: Offset into vertex buffer.
  * `uint16_t min_height`, `uint16_t max_height`: Cell bounding box heights.

### 2. Surface Triangle Table
Located at:
```c
triangle_table = header_ptr + (grid_cells_x * grid_cells_z * 3) + 9;
```
* Contains `triangle_count` records.
* Each record is **24 bytes** (`0x18` bytes = 6 x `int32_t`):
  * `int32_t v0_idx`: Vertex 0 index.
  * `int32_t v1_idx`: Vertex 1 index.
  * `int32_t v2_idx`: Vertex 2 index.
  * `int32_t surface_material`: Material flags (asphalt, grass, dirt, snow, ice, oil, bridge).
  * `int32_t normal_x`: Normal vector X component.
  * `int32_t normal_z`: Normal vector Z component.

---

## Surface Raycast Query Algorithm (`getsurf`)

In `FUN_00412fc0`:
```c
// Given vehicle world position (x, z):
int cell_x = world_x / cell_size_x;
int cell_z = world_z / cell_size_z;
int cell_idx = (cell_z * grid_cells_x) + cell_x;

CellRecord *cell = &grid[cell_idx];

// Interpolate surface height y at (x, z):
// Height is calculated across intersecting triangles using barycentric coordinates:
y = (v0.y + v1.y + v2.y) / 3.0f;
```
