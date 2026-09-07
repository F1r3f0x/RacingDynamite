# Ignition (1997) Font Formats (.FNT, .LFT)

## 1. Overview
Ignition employs two bitmap font systems:
1. **`IGNITION.FNT` (`geputget.c` / `0x004133d0`)**: The primary proportional text font used for all in-game menus, HUD prompts, lap counters, race positions, and vehicle/circuit selection screens.
2. **`.LFT` Fonts (`lisa3d.c` / `0x00456270`)**: Specialized UI tile fonts, button border graphics, and alternate styling fonts (`MINI.LFT`, `SMALL.LFT`, `RED_DARK.LFT`, `RED_LITE.LFT`).

---

## 2. Authentic `IGNITION.FNT` Binary Structure (5,400 bytes)

| File Offset | Type | Size | Description |
| :--- | :--- | :--- | :--- |
| `0x00` | `uint16_t` | 2 bytes | Font format version / flags. |
| `0x02` | `int16_t` | 2 bytes | **Glyph Height** in pixels (10 px in `IGNITION.FNT`). |
| `0x04` | `int16_t` | 2 bytes | **Character Spacing** (-1 in header, defaults to 1 px). |
| `0x06` | `uint8_t[256]` | 256 bytes | **Glyph Widths Table**: Proportional width in pixels for each glyph index. |
| `0xC6` | `uint8_t[256]` | 256 bytes | **ASCII Mapping Table**: Maps input ASCII byte code `c` to glyph index (`glyph_idx = map[c]`). `255` = unmapped. |
| `0x1C6` | `int32_t[256]` | 1024 bytes | **Glyph Offset Table**: Relative byte offset from `0x546` to start of glyph raster data. `-1` = missing. |
| `0x546` | `uint8_t[]` | 4,050 bytes | **Raster Glyph Bitmaps**: Uncompressed 8bpp pixels (`width * height` bytes). |

### Pixel Value Encoding in `IGNITION.FNT`:
* `0`: Transparent background.
* `1`: Font outline and drop shadow (rendered as black / index 0).
* `2`: Font face / body (shaded dynamically using selection color, e.g. `215` for bright yellow or `251` for bright white).

---

## 3. Rendering Algorithm (`Font_DrawText` / `0x004133d0`)

```c
void Font_DrawText(uint8_t *fb, int stride, const LftFont *font, int x, int y, const char *text, uint8_t color_offset) {
    int cur_x = x;
    int height = font->header.height;
    int spacing = font->spacing > 0 ? font->spacing : 1;

    for (const char *p = text; *p != '\0'; ++p) {
        unsigned char c = (unsigned char)*p;
        if (c == ' ') {
            cur_x += 6;
            continue;
        }

        uint8_t g_idx = font->ascii_map[c];
        if (g_idx == 255) { cur_x += 4; continue; }

        int w = font->widths[g_idx];
        int32_t off = font->offsets[g_idx];
        if (off < 0) { cur_x += 4; continue; }

        const uint8_t *glyph_src = font->glyph_pixels + off;
        for (int r = 0; r < height; ++r) {
            int dest_y = y + r;
            for (int col = 0; col < w; ++col) {
                int dest_x = cur_x + col;
                uint8_t pix = glyph_src[r * w + col];
                if (pix != 0) {
                    uint8_t col_val = (pix == 2) ? color_offset : 0; // Body vs Shadow
                    fb[dest_y * stride + dest_x] = col_val;
                }
            }
        }
        cur_x += w + spacing;
    }
}
```

---

## 4. Input Navigation & Debouncing

In menus (`GAME_STATE_MAIN_MENU`, `GAME_STATE_CAR_SELECT`, `GAME_STATE_TRACK_SELECT`), input must be **edge-triggered**:
* Continuous key state (`keys[SDL_SCANCODE_UP]`) causes uncontrollable 60 FPS multi-triggering.
* Modern solution in `platform_sdl.c`: Filter `SDL_KEYDOWN` events with `event.key.repeat == 0` into single-frame impulses:
  * `nav_up`, `nav_down`: Cycles menu options exactly one position per tap.
  * `nav_left`, `nav_right`: Cycles vehicles or circuits exactly one item per tap.
  * `nav_confirm` (`ENTER` / `SPACE`): Confirms selection.
  * `nav_cancel` (`ESC`): Navigates back.
