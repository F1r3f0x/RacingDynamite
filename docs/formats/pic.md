# Ignition (1997) .PIC Image Format

## Overview
`.PIC` files are 8-bit uncompressed paletted images used for UI screens, track previews, logos, menu backgrounds, and 2D sprite sheets.

## File Structure

| Offset | Type | Size | Description |
| :--- | :--- | :--- | :--- |
| `0x00` | `uint32_t` | 4 bytes | **Total File Size**: Length of the file in bytes (`846 + width * height`). |
| `0x04` | `uint16_t` | 2 bytes | **Format ID**: `0x9500` (UDS format signature). |
| `0x06` | `uint16_t` | 2 bytes | **Width**: Image width in pixels. |
| `0x08` | `uint16_t` | 2 bytes | **Height**: Image height in pixels. |
| `0x0A` | `uint8_t[68]` | 68 bytes | Header metadata / alignment padding up to offset 78 (`0x4E`). |
| `0x4E` | `uint8_t[768]` | 768 bytes | **Embedded Palette**: 256 RGB entries (3 bytes per color: `[R, G, B]`, values 0–255). |
| `0x34E` (`846`) | `uint8_t[]` | `width * height` | **Pixel Data**: 8-bit color indices, scanned sequentially from top-to-bottom, left-to-right. |

## Notes
- Header size is exactly **846 bytes** (`0x34E`).
- Total file size = `846 + (width * height)`.
- Pixel values are direct indices `[0..255]` into the embedded 768-byte RGB palette.
