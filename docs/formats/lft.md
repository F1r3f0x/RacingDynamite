# Ignition (1997) .LFT Font Format

## Overview
`.LFT` files (e.g. `FONTS/SMALL.LFT`, `BALTAZAR/DATA/RED_DARK.LFT`) contain 2D bitmap proportional glyph fonts used for menu headings, timing digits, and HUD text.

## Glyph Layout & Rendering
As decompiled in `FUN_004133d0`:
```c
void Font_DrawText(
    uint8_t *framebuffer, 
    int y, 
    const char *text, 
    int x, 
    int stride, 
    const uint8_t *font_data, 
    uint8_t color_offset
);
```

* Font data contains:
  * ASCII character mapping table.
  * Per-character glyph width and height tables.
  * Raw 1-bit or 8-bit glyph bitmap masks.
* Characters are blitted directly onto the 8bpp software frame buffer with optional color tint offsets (`color_offset + mask_pixel`).
