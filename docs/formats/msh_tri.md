# Ignition (1997) .MSH & .TRI 3D Mesh Formats

## Overview
Ignition stores 3D track scenery, obstacles, and vehicle bodies in two companion binary files:
* **`.MSH` (Mesh Vertices)**: 3D point cloud, vertex normals, and per-vertex coordinate data.
* **`.TRI` (Triangle Face Lists)**: Polygon connectivity indices, texture coordinate mappings (`u, v`), and face normal references.

---

## 1. `.MSH` File Structure
Contains arrays of 3D vertex records.

```c
typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
} Vertex3D;
```

* Coordinate System:
  * **X**: Lateral position (Left / Right).
  * **Y**: Elevation / Height (Up / Down, inverted in some coordinate calculations).
  * **Z**: Longitudinal position (Forward / Back along track).

---

## 2. `.TRI` File Structure
Specifies triangle faces connecting the vertices in `.MSH` and mapping to the 1024x1024 `.TEX` texture page.

```c
typedef struct {
    uint16_t v0;           // Index into .MSH vertex buffer
    uint16_t v1;
    uint16_t v2;
    uint8_t  uv0[2];       // Texture coordinates on 1024x1024 texture page
    uint8_t  uv1[2];
    uint8_t  uv2[2];
    uint16_t material_flags; // Transparency, double-sided, animated texture flags
} TriangleFace;
```
