# Ignition (1997) .SRF Surface & Physics Format

## Overview
`.SRF` files (located in `LEVELS/<TRACK>/<TRACK>.SRF`) contain the 3D road surface mesh, heightfield collision grid, and physics attributes. In the original source code, this subsystem was implemented in `d:\projects\ignition\getsurf\getsurf.c`.

## Header Structure (First 36 bytes = 9 x `int32_t`)

| Offset | Type | Field | Verified Value Across All Tracks | Description |
| :--- | :--- | :--- | :--- | :--- |
| `0x00` | `int32_t` | `grid_cells_x` | `200` | Grid cells along X axis. |
| `0x04` | `int32_t` | `grid_cells_z` | `200` | Grid cells along Z axis. |
| `0x08` | `int32_t` | `cell_size_z` | `512` | World dimension of one grid cell in Z. |
| `0x0C` | `int32_t` | `cell_size_x` | `512` | World dimension of one grid cell in X. |
| `0x10` | `int32_t` | `grid_stride_x`| `101` | Spatial index grid dimension X. |
| `0x14` | `int32_t` | `grid_stride_z`| `101` | Spatial index grid dimension Z. |
| `0x18` | `int32_t` | `triangle_count`| Level-dependent (e.g. 12,462) | Number of surface collision triangles. |
| `0x1C` | `int32_t` | `table1_count` | Level-dependent (e.g. 10,122) | Primary index buffer integer count. |
| `0x20` | `int32_t` | `table2_count` | Level-dependent (e.g. 10,336) | Secondary index buffer integer count. |

### File Size Exact Validation Formula:
$$\text{File Size} = 36 + (101 \times 101 \times 12) + (\text{triangle\_count} \times 24) + (\text{table1\_count} \times 4) + (\text{table2\_count} \times 4)\text{ bytes}$$

| Track | File Size | Triangles | Table 1 | Table 2 | Formula Check |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Austria** | 503,368 bytes | 12,462 | 10,122 | 10,336 | 100% Exact Match |
| **Brazil** | 466,996 bytes | 11,074 | 9,750 | 9,943 | 100% Exact Match |
| **Canada** | 388,152 bytes | 8,752 | 6,905 | 7,009 | 100% Exact Match |
| **Carib** | 584,988 bytes | 15,126 | 12,298 | 12,581 | 100% Exact Match |
| **Iceland** | 407,684 bytes | 9,266 | 7,803 | 7,910 | 100% Exact Match |
| **Japan** | 494,128 bytes | 12,427 | 9,163 | 9,195 | 100% Exact Match |
| **USA** | 409,552 bytes | 9,685 | 6,840 | 6,826 | 100% Exact Match |

---

## Data Sections

### 1. Spatial Grid Index Table (Offset `0x24` / Byte 36)
Immediately following the 36-byte header:
* Contains $101 \times 101 = 10,201$ entries (122,412 bytes).
* Each entry is **12 bytes** (`SrfCell`):
  * `int32_t table2_offset`: Byte offset into `table2` (secondary index buffer; divide by 4 for integer index).
  * `int32_t table1_offset`: Byte offset into `table1` (primary index buffer; divide by 4 for integer index).
  * `uint16_t table1_count`: Number of triangles intersecting this spatial cell.
  * `uint16_t table2_count`: Number of secondary collision entities.

### 2. Surface Triangle Table
Located immediately after the spatial grid index table:
* Contains `triangle_count` records.
* Each record is **24 bytes** (`SrfTriangle`):
  * `int32_t v0_y`: Elevation / coordinate parameter 0.
  * `int32_t v1_y`: Elevation / coordinate parameter 1.
  * `int32_t v2_y`: Elevation / coordinate parameter 2.
  * `int32_t material_flags`: Material friction and surface attributes.
  * `int32_t normal_x`: Normal vector X component.
  * `int32_t normal_z`: Normal vector Z component.

### 3. Primary & Secondary Index Tables (`table1`, `table2`)
* `table1` (size `table1_count * 4` bytes): Array of 32-bit integers representing byte offsets into the Surface Triangle Table (`/ 24` gives the triangle index).
* `table2` (size `table2_count * 4` bytes): Array of 32-bit integers for secondary collision and trigger boundaries.

---

## Surface Raycast Query Algorithm (`getsurf`)

In `FUN_00412fc0` (`IGN_WIN.EXE`) / `getsurf.c`:
1. World coordinates $(X, Z)$ are centered around the track origin by adding half the spatial grid stride ($\text{stride} / 2 = 50$, verified at address `0x00479c48` as `double 50.0`):
$$\text{cell}_x = \lfloor X / \text{cell\_size}_x \rfloor + 50$$
$$\text{cell}_z = \lfloor Z / \text{cell\_size}_z \rfloor + 50$$
2. Index into the spatial cell:
$$\text{cell\_idx} = \text{cell}_z \times \text{grid\_stride}_x + \text{cell}_x$$
3. Read `table1_offset` from the cell, index into `table1`, and retrieve the triangle byte offset (`/ 24` for triangle index).
4. If a cell contains no triangles (`table1_offset < 0` or count $= 0$), the point is off-track (`0xFFFFFFFF`).

```c
int32_t Srf_GetElevationAt(const SrfData *srf, int32_t world_x, int32_t world_z) {
    if (!srf || !srf->grid || !srf->triangles || !srf->table1) return 0;

    int32_t cell_sz_x = srf->header.cell_size_x ? srf->header.cell_size_x : 512;
    int32_t cell_sz_z = srf->header.cell_size_z ? srf->header.cell_size_z : 512;

    int32_t cx = (world_x / cell_sz_x) + (srf->header.grid_stride_x / 2);
    int32_t cz = (world_z / cell_sz_z) + (srf->header.grid_stride_z / 2);

    if (cx < 0 || cz < 0 || cx >= srf->header.grid_stride_x || cz >= srf->header.grid_stride_z) {
        return 0;
    }

    size_t cell_idx = (size_t)cz * (size_t)srf->header.grid_stride_x + (size_t)cx;
    const SrfCell *cell = &srf->grid[cell_idx];

    int32_t t1_idx = cell->table1_offset / 4;
    if (t1_idx >= 0 && t1_idx < srf->header.table1_count) {
        int32_t tri_idx = srf->table1[t1_idx] / 24;
        if (tri_idx >= 0 && tri_idx < srf->header.triangle_count) {
            const SrfTriangle *tri = &srf->triangles[tri_idx];
            return (tri->v0_y + tri->v1_y + tri->v2_y) / 3;
        }
    }
    return 0;
}
```
