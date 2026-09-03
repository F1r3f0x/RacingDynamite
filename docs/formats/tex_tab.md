# Ignition (1997) .TEX, .TAB, .SHD & .PAN Formats

## Overview
Ignition relies on fixed-size 64KB and 1MB binary tables to optimize 8-bit software rendering without floating-point math overhead on vintage CPUs.

---

## 1. `.TEX` (1024x1024 Texture Atlas)
* Size: exactly **1,048,576 bytes** (`1024 * 1024` bytes = 1 MB).
* Raw uncompressed 8-bit paletted pixels.
* Contains all road textures, terrain patches, roadside buildings, and sprite animations for a level mapped by `.TRI` uv coordinates.

---

## 2. `.TAB` (Color & Shading Lookup Table)
* Size: exactly **65,536 bytes** (`256 * 256` = 64 KB).
* Represents a $256 \times 256$ matrix indexed by:
  ```c
  uint8_t shaded_color = color_tab[(light_level << 8) | original_color];
  ```
* Provides instant Gouraud lighting, depth cueing / distance fog, and translucency without palette modification.

---

## 3. `.SHD` (Shadow Map)
* Size: exactly **65,536 bytes** (`256 * 256` = 64 KB).
* 256x256 8-bit shadow projection buffer used for vehicle shadows cast upon the track surface.

---

## 4. `.PAN` (Panorama Skybox / Background)
* Size: exactly **65,536 bytes** (`256 * 256` = 64 KB).
* Cylindrical 360-degree background horizon image drawn behind distant track geometry.
