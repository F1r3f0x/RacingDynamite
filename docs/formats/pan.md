# Ignition (1997) .PAN Horizon Sky Panorama Format

## Overview

`.PAN` files (located at `LEVELS/<TRACK>/<TRACK>.PAN`) store the 360-degree cylindrical horizon backdrop (distant mountains, hills, clouds, urban skylines, and atmospheric gradients) for each circuit in Ignition.

During scene rendering, the Lisa3D software rasterizer blits this background before drawing the 3D track geometry and scenery meshes, providing the illusion of immense distance and atmospheric depth.

---

## 1. Binary Layout & Specifications

Across all 7 circuits in the game, `.PAN` files have a uniform, uncompressed binary layout:

| Property | Value | Notes |
| :--- | :--- | :--- |
| **File Size** | **65,536 bytes** | $256 \times 256$ bytes exact across all tracks. |
| **Header** | None | Raw uncompressed raster byte stream. |
| **Color Depth** | **8 bits per pixel** | Indexed color referencing the level's `.COL` palette. |
| **Texture Dimensions** | **$256 \times 256$ pixels** | Width = 256, Height = 256, Stride = 256 bytes per row. |
| **Coordinate Space** | Row-major | Horizontal axis wraps seamlessly ($X \pmod{256}$) for $360^\circ$ rotation. |

### Track File Sizes Across All Circuits:

| Track | File Path | File Size | Dimensions | Palette Dependency |
| :--- | :--- | :--- | :--- | :--- |
| **Austria** | `LEVELS/AUSTRIA/AUSTRIA.PAN` | 65,536 bytes | $256 \times 256$ | `AUSTRIA.COL` (Snowy Alpine peaks) |
| **Brazil** | `LEVELS/BRAZIL/BRAZIL.PAN`   | 65,536 bytes | $256 \times 256$ | `BRAZIL.COL` (Tropical canopy & cloudy skies) |
| **Canada** | `LEVELS/CANADA/CANADA.PAN`   | 65,536 bytes | $256 \times 256$ | `CANADA.COL` (Pine forests & rolling hills) |
| **Carib**  | `LEVELS/CARIB/CARIB.PAN`     | 65,536 bytes | $256 \times 256$ | `CARIB.COL` (Ocean horizon & tropical islands) |
| **Iceland**| `LEVELS/ICELAND/ICELAND.PAN` | 65,536 bytes | $256 \times 256$ | `ICELAND.COL` (Glacial peaks & volcanic ash) |
| **Japan**  | `LEVELS/JAPAN/JAPAN.PAN`     | 65,536 bytes | $256 \times 256$ | `JAPAN.COL` (Metropolitan skyscrapers & neon dusk) |
| **USA**    | `LEVELS/USA/USA.PAN`         | 65,536 bytes | $256 \times 256$ | `USA.COL` (Red rock canyon cliffs) |

---

## 2. Ghidra Decompilation & Memory Management

### 2.1 Asset Loading (`Track_LoadAllAssets` at `0x00418e80`)
The engine allocates a 64KB-aligned buffer (`DAT_005dfe64`) and reads the entire 65,536 bytes into memory:

```c
_sprintf(pan_path, "LEVELS\\%s\\%s.PAN", track_dir, track_name);
file_handle = _open(pan_path, 0x8000); // O_BINARY
if (file_handle >= 0) {
    uint32_t len = _filelength(file_handle); // 65536
    // Allocates with 64KB page alignment (0x10000)
    void *buf = Mem_Alloc(g_LevelMemoryPool, len + 0x10000);
    g_pActivePAN = (uint8_t*)(((uintptr_t)buf + 0xFFFF) & ~0xFFFF);
    _read(file_handle, g_pActivePAN, len);
    _close(file_handle);
}
```

### 2.2 Background Rasterization (`FUN_00438210` / `FUN_00438a60`)
Before rendering 3D geometry, Lisa3D fills the top portion of the 8bpp virtual framebuffer ($640 \times 480$ or $320 \times 200$) with the panorama backdrop:

* **Horizontal Cylindrical Mapping**:
  The horizontal sampling coordinate $u$ wraps cyclically around the camera's azimuth yaw angle $\theta_{\text{yaw}}$:
  $$u = \left\lfloor \frac{\theta_{\text{yaw}}}{2\pi} \times 256.0 \right\rfloor \pmod{256}$$
* **Vertical Pitch Elevation**:
  The vertical sampling line $v$ adjusts proportionally to the camera's vertical pitch angle $\theta_{\text{pitch}}$:
  $$v = \text{clamp}\left(128 - \left\lfloor \frac{\theta_{\text{pitch}}}{90.0^\circ} \times 64.0 \right\rfloor, 0, 255\right)$$
* **Texel Lookup**:
  $$\text{color} = \text{g\_pActivePAN}[v \times 256 + u]$$

---

## 3. Visual Interpretation & Rendering Behavior

* When the camera rotates horizontally (yaw), the horizon scrolls smoothly across the screen without perspective distortion.
* When the camera pitches up or down, the horizon moves vertically to maintain realistic horizon anchoring.
* Because the bottom half of the panorama often contains deep ground colors or gradient fades, the 3D track mesh seamlessly covers the lower section of the screen without visible seams.
