# Ignition (1997) .CDP Video / Cutscene Format

## Overview
`.CDP` ("Cinematic / CD Presentation") files store delta-compressed paletted animation sequences used for the 3D rotating track flybys, menu transitions, and game intro screens.

Original source references: `Cdp_OpenFile` (`0x00412580`), `Cdp_DecodeFrame` (`0x00412610`), and `Cdp_DecompressRLE` (`0x00499abc`).

---

## File Structure

| Offset | Type | Size | Description |
| :--- | :--- | :--- | :--- |
| `0x00` | `char[4]` | 4 bytes | **Magic**: `"CDP\0"` signature. |
| `0x04` | `uint16_t` | 2 bytes | **Version**: `100` (`0x0064`). |
| `0x06` | `uint16_t` | 2 bytes | **Width**: Video frame width in pixels (e.g. 320). |
| `0x08` | `uint16_t` | 2 bytes | **Height**: Video frame height in pixels (e.g. 200). |
| `0x0A` | `uint16_t` | 2 bytes | **Frame Count**: Total number of animation frames. |
| `0x0C` | `uint16_t` | 2 bytes | **Frame Delay**: Timing in milliseconds / frame rate divisor. |
| `0x0E` | `uint16_t` | 2 bytes | Padding / alignment. |
| `0x10` | `uint8_t[768]` | 768 bytes | **Embedded Palette**: 256 RGB color triplets. |
| `0x310` | `uint8_t[]` | Variable | **Frame Stream**: Inter-frame delta compressed stream. |

---

## Decompression Algorithm (`Cdp_DecompressRLE`)

The frame stream is an opcode-based byte stream that modifies the active frame buffer in-place:

| Opcode Byte | Operation |
| :--- | :--- |
| `0x00` – `0xF5` | **Raw Pixel**: Write byte to framebuffer, advance pointer by 1. |
| `0xF6` | Copy next byte from stream to framebuffer, advance pointer by 1. |
| `0xF7` | **Skip 2 Pixels**: Advance destination pointer by 2 (pixels unchanged). |
| `0xF8` | **Skip 3 Pixels**: Advance destination pointer by 3. |
| `0xF9` | **Skip 4 Pixels**: Advance destination pointer by 4. |
| `0xFA` | **Skip N Pixels**: Read next byte $N$, advance destination pointer by $N$. |
| `0xFF` | **End of Frame**: Advance to next frame in sequence. |

This delta-skip encoding achieves high compression for static cameras and slow pans where only moving vehicles or water change between consecutive frames.
