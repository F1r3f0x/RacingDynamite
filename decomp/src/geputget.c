/*
 * geputget.c - 2D Graphics blitting, font rasterization, and palette management
 * Original file: geputget.c
 * Target: MAINDOS.EXE (Watcom C/C++ 10.6, 32-bit flat protected mode)
 *         MAINDOS.EXE (MSVC 4.x / 5.0, Win32)
 */

#include "geputget.h"
#include <io.h>
#include <fcntl.h>
extern int g_MemoryAllocated;
#include <string.h>

/* Global font table matching MAINDOS @ 0x0024C214, MAINDOS @ 0x0063F2E0 */
FontSlot g_fonts[MAX_FONTS];

/* Global font system state */
int g_fontSystemInitialized = 0; /* MAINDOS @ 0x000D7C60 */
int g_fontSubsystemHandle = 0;   /* MAINDOS @ 0x0024C210, MAINDOS @ 0x0050E680 */

uint8_t *g_pSysGfxPic;
uint8_t *g_pSysG2Pic;
uint8_t *g_pSysCol;
int g_SystemFonts[8];

uint8_t *g_pHUDFonts;
int g_hudFontYellowSmall;
int g_hudFontPosSmall;
int g_hudFontSpeedSmall;
int g_hudFontYellow;
int g_hudFontPos;
int g_hudFontSpeed;
int g_hudFontYellowHuge;
int g_hudFontPosHuge;
int g_hudFontSpeedHuge;

uint8_t *g_pSPangfxPic;
uint8_t *g_pNPangfxPic;
uint8_t *g_pHPan1Pic;
uint8_t *g_pHPan2Pic;
uint8_t *g_pSSignsPic;
uint8_t *g_pNSignsPic;
uint8_t *g_pHSignsPic;
uint8_t *g_pPokalPic;

/* External subsystem helpers */
extern int Subsystem_Register(void);
extern void Subsystem_AddCallback(int handle);
extern void Subsystem_Unregister(int handle);
extern void *Gfx_SpriteOp(void *desc, int op);
extern void Gfx_DrawSprite(void *handle, Point2D *pos, int flags);

/**
 * @original Font_InitSystem (MAINDOS.EXE @ 0x00061220, geputget.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x00061220. Initializes 30 font slots and registers subsystem callback.
 */
int Font_InitSystem(void) {
    int i;

    if (g_fontSystemInitialized == 1) {
        return 1010;
    }

    g_fontSystemInitialized = 1;
    g_fontSubsystemHandle = Subsystem_Register();
    Subsystem_AddCallback(g_fontSubsystemHandle);

    for (i = 0; i < MAX_FONTS; i++) {
        g_fonts[i].in_use = 0;
        g_fonts[i].alignment = 0;
        g_fonts[i].field_08 = 0;
        g_fonts[i].is_proportional = 0;
        g_fonts[i].extra_spacing = 1;
        g_fonts[i].field_14 = 1;
    }

    return 1;
}

/**
 * @original Font_Shutdown (MAINDOS.EXE @ 0x00061319, geputget.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x00061319. Shuts down font subsystem and unloads all active font slots.
 */
int Font_Shutdown(void) {
    int i;

    if (g_fontSystemInitialized == 0) {
        return 1020;
    }

    Subsystem_Unregister(g_fontSubsystemHandle);
    g_fontSystemInitialized = 0;

    for (i = 0; i < MAX_FONTS; i++) {
        if (g_fonts[i].in_use == 1) {
            Font_Unload(i);
        }
    }

    return 1;
}

/**
 * @original Font_Parse (MAINDOS.EXE @ 0x00061399, geputget.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x00061399. Validates LFT header, allocates a font slot, extracts 224 glyph
 *        metrics, registers sprite handles with Gfx_SpriteOp, and returns allocated font slot ID.
 */
int Font_Parse(void *buffer, int unused) {
    uint8_t *hdr;
    uint8_t *pixel_base;
    int slot;
    int i;

    (void)unused;

    if (g_fontSystemInitialized == 0) {
        Font_InitSystem();
    }

    hdr = (uint8_t *)buffer;
    if (strcmp((const char *)hdr, "LFT") != 0) {
        g_fileErrorLine = 1050;
        return -1;
    }

    if (*(int16_t *)(hdr + 4) != 100) {
        g_fileErrorLine = 1060;
        return -1;
    }

    for (i = 0; i < MAX_FONTS; i++) {
        if (g_fonts[i].in_use == 0) {
            break;
        }
    }

    if (i == MAX_FONTS) {
        g_fileErrorLine = 1030;
        return -1;
    }

    slot = i;
    g_fonts[slot].field_18 = *(uint16_t *)(hdr + 6);
    g_fonts[slot].height   = *(uint16_t *)(hdr + 8);
    g_fonts[slot].spacing  = *(uint16_t *)(hdr + 10);

    for (i = 0; i < FONT_GLYPH_COUNT; i++) {
        g_fonts[slot].widths[i] = *(uint16_t *)(hdr + 0x68c + i * 2);
    }

    pixel_base = hdr + 0x84c;

    for (i = 0; i < FONT_GLYPH_COUNT; i++) {
        int32_t offset = *(int32_t *)(hdr + 12 + i * 4);
        if (offset == -1) {
            g_fonts[slot].glyph_present[i] = 0;
            g_fonts[slot].glyph_handles[i] = NULL;
        } else {
            SpriteDesc desc;
            g_fonts[slot].glyph_present[i] = 1;
            desc.width    = (int16_t)*(uint16_t *)(hdr + 0x68c + i * 2);
            desc.height   = (int16_t)*(uint16_t *)(hdr + 10);
            desc.field_0c = 0;
            desc.pixels   = pixel_base + offset;
            desc.stride   = (int16_t)*(uint16_t *)(hdr + 0x68c + i * 2);
            desc.field_18 = 0;
            desc.field_1c = 0;
            g_fonts[slot].glyph_handles[i] = Gfx_SpriteOp(&desc, 0);
        }
    }

    g_fonts[slot].in_use = 1;
    return slot;
}

/**
 * @original Font_Load (MAINDOS.EXE @ 0x000615eb, geputget.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x000615eb. Loads a .LFT font from disk, creates glyph handles, and frees buffer.
 */
int Font_Load(const char *filename, int unused) {
    void *buffer;
    int result;

    buffer = File_LoadToMemory(filename);
    if (buffer == NULL) {
        g_fileErrorLine = 1000;
        return -1;
    }

    result = Font_Parse(buffer, unused);
    Mem_Free(0, buffer);
    return result;
}

/**
 * @original Font_Unload (MAINDOS.EXE @ 0x00061653, geputget.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x00061653. Frees all sprite handles for a font slot and marks the slot available.
 */
int Font_Unload(int font_id) {
    int i;

    g_fonts[font_id].in_use = 0;

    for (i = 0; i < FONT_GLYPH_COUNT; i++) {
        if (g_fonts[font_id].glyph_present[i] == 1) {
            Gfx_SpriteOp(NULL, (int)g_fonts[font_id].glyph_handles[i]);
        }
    }

    return 1;
}

/**
 * @original Font_GetTextWidth (MAINDOS.EXE @ 0x000616db, geputget.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x000616db. Computes string pixel width considering proportional flag, spacing,
 *        and space character sizing (height * 0.35).
 */
int Font_GetTextWidth(const char *text, int font_id) {
    int total_width = 0;
    int unk_c = 0;
    int dummy_space;
    int i;

    if (font_id >= MAX_FONTS || g_fonts[font_id].in_use == 0) {
        g_fileErrorLine = 1040;
    }

    if (g_fonts[font_id].field_08 != 0) {
        return 2;
    }

    dummy_space = (int)((double)(g_fonts[font_id].height << 8) * 0.35);
    if (dummy_space < 1) {
        dummy_space = 1;
    }

    if (g_fonts[font_id].is_proportional == 0) {
        for (i = 0; i < (int)strlen(text); i++) {
            uint8_t c = (uint8_t)text[i];
            if (c == '\0') {
                break;
            }

            if (g_fonts[font_id].glyph_present[c - 32] == 1) {
                int unk_1c = 0;
                if (g_fonts[font_id].widths[c - 32] != g_fonts[font_id].height) {
                    unk_1c = (g_fonts[font_id].height - g_fonts[font_id].widths[c - 32]) / 2;
                }
                (void)unk_1c;
                total_width += g_fonts[font_id].height + g_fonts[font_id].extra_spacing;
            } else if (c == ' ') {
                total_width += g_fonts[font_id].height + g_fonts[font_id].extra_spacing;
            }
        }
    } else {
        for (i = 0; i < (int)strlen(text); i++) {
            uint8_t c = (uint8_t)text[i];
            if (c == '\0') {
                break;
            }

            if (g_fonts[font_id].glyph_present[c - 32] == 1) {
                total_width += g_fonts[font_id].widths[c - 32] + g_fonts[font_id].extra_spacing;
            } else if (c == ' ') {
                int space_w = (int)((double)(g_fonts[font_id].height << 8) * 0.35 * (1.0 / 256.0));
                total_width += space_w;
            }
        }
    }

    return total_width - unk_c;
}

/**
 * @original Font_DrawText (MAINDOS.EXE @ 0x00061959, geputget.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x00061959. Renders string to screen via Gfx_DrawSprite, taking into account
 *        text alignment (0=left, 1=center, 2=right), proportionality, and fixed-point coordinates.
 */
int Font_DrawText(const char *text, int font_id, int x, int y) {
    int orig_x = x;
    int dummy_space;
    int i;

    if (font_id >= MAX_FONTS || g_fonts[font_id].in_use == 0) {
        return 1040;
    }

    if (g_fonts[font_id].field_08 != 0) {
        return 2;
    }

    dummy_space = (int)((double)(g_fonts[font_id].height << 8) * 0.35);
    if (dummy_space < 1) {
        dummy_space = 1;
    }

    if (g_fonts[font_id].is_proportional == 0) {
        if (g_fonts[font_id].alignment != 0) {
            for (i = 0; i < (int)strlen(text); i++) {
                uint8_t c = (uint8_t)text[i];
                if (c == '\0') {
                    break;
                }

                if (g_fonts[font_id].glyph_present[c - 32] == 1) {
                    int unk_8 = 0;
                    if (g_fonts[font_id].widths[c - 32] != g_fonts[font_id].height) {
                        unk_8 = (g_fonts[font_id].height - g_fonts[font_id].widths[c - 32]) / 2;
                    }
                    (void)unk_8;
                    x += g_fonts[font_id].height + g_fonts[font_id].extra_spacing;
                } else if (c == ' ') {
                    x += g_fonts[font_id].height + g_fonts[font_id].extra_spacing;
                }
            }

            if (g_fonts[font_id].alignment == 1) {
                int diff = x - orig_x;
                x = orig_x - (diff / 2);
            } else if (g_fonts[font_id].alignment == 2) {
                int diff = x - orig_x;
                x = orig_x - diff;
            } else {
                x = orig_x;
            }
        }

        for (i = 0; i < (int)strlen(text); i++) {
            uint8_t c = (uint8_t)text[i];
            if (c == '\0') {
                break;
            }

            if (g_fonts[font_id].glyph_present[c - 32] == 1) {
                int unk_8 = 0;
                Point2D pos;
                if (g_fonts[font_id].widths[c - 32] != g_fonts[font_id].height) {
                    unk_8 = (g_fonts[font_id].height - g_fonts[font_id].widths[c - 32]) / 2;
                }
                pos.x = (x + unk_8) << 8;
                pos.y = y << 8;
                Gfx_DrawSprite(g_fonts[font_id].glyph_handles[c - 32], &pos, 0);
                x += g_fonts[font_id].height + g_fonts[font_id].extra_spacing;
            } else if (c == ' ') {
                x += g_fonts[font_id].height + g_fonts[font_id].extra_spacing;
            }
        }
    } else {
        if (g_fonts[font_id].alignment != 0) {
            for (i = 0; i < (int)strlen(text); i++) {
                uint8_t c = (uint8_t)text[i];
                if (c == '\0') {
                    break;
                }

                if (g_fonts[font_id].glyph_present[c - 32] == 1) {
                    x += g_fonts[font_id].widths[c - 32] + g_fonts[font_id].extra_spacing;
                } else if (c == ' ') {
                    int space_w = (int)((double)(g_fonts[font_id].height << 8) * 0.35 * (1.0 / 256.0));
                    x += space_w;
                }
            }

            if (g_fonts[font_id].alignment == 1) {
                int diff = x - orig_x;
                x = orig_x - (diff / 2);
            } else if (g_fonts[font_id].alignment == 2) {
                int diff = x - orig_x;
                x = orig_x - diff;
            } else {
                x = orig_x;
            }
        }

        for (i = 0; i < (int)strlen(text); i++) {
            uint8_t c = (uint8_t)text[i];
            if (c == '\0') {
                break;
            }

            if (g_fonts[font_id].glyph_present[c - 32] == 1) {
                Point2D pos;
                pos.x = x << 8;
                pos.y = y << 8;
                Gfx_DrawSprite(g_fonts[font_id].glyph_handles[c - 32], &pos, 0);
                x += g_fonts[font_id].widths[c - 32] + g_fonts[font_id].extra_spacing;
            } else if (c == ' ') {
                int space_w = (int)((double)(g_fonts[font_id].height << 8) * 0.35 * (1.0 / 256.0));
                x += space_w;
            }
        }
    }

    return 1;
}

/**
 * @original Font_DrawHUDText (MAINDOS.EXE @ 0x00043d60, geputget.c)
 * @fidelity EXACT
 * @notes MAINDOS @ 0x00043d60. Direct 8bpp bitmap font rasterizer blitting characters from IGNITION.FNT
 *        directly into the target framebuffer with stride, height, spacing, and color offset.
 */
void Font_DrawHUDText(int x, int y, const char *text, uint8_t *framebuffer, int stride, const uint8_t *font_data, uint8_t color_offset) {
    int cur_y_stride;
    int max_y_stride;
    short height;
    short spacing;
    const char *p;
    char c;

    if (!text || !framebuffer || !font_data) {
        return;
    }

    cur_y_stride = y * stride;
    spacing = *(const int16_t *)(font_data + 4);
    height = *(const int16_t *)(font_data + 2);
    p = text;
    c = *p;

    if (c == '\0') {
        return;
    }

    max_y_stride = cur_y_stride + (int)height * stride;

    do {
        uint8_t glyph_idx = font_data[0xC6 + (uint8_t)c];
        uint8_t width = font_data[6 + glyph_idx];
        int glyph_off = *(const int32_t *)(font_data + 0x1C6 + (int)glyph_idx * 4);

        if (cur_y_stride < max_y_stride) {
            int row_stride = cur_y_stride;
            int pixel_row_off = 0;

            do {
                if (width > 0) {
                    int col;
                    for (col = 0; col < (int)width; col++) {
                        uint8_t pix = font_data[0x546 + glyph_off + pixel_row_off + col];
                        if (pix != 0) {
                            framebuffer[row_stride + x + col] = (uint8_t)(color_offset + pix);
                        }
                    }
                }
                row_stride += stride;
                pixel_row_off += (int)width;
            } while (row_stride < max_y_stride);
        }

        x += (int)width + (int)spacing;
        p++;
        c = *p;
    } while (c != '\0');
}

extern void BootLog(const char *msg);

/**
 * @original Load_SystemGraphicsAndFonts (MAINDOS.EXE @ 0x00021860, geputget.c)
 * @fidelity EXACT
 */
void Load_SystemGraphicsAndFonts(void) {
    int i;
    
    BootLog("[MAINDOS] Load_SystemGraphicsAndFonts: Loading system assets...");
    g_pSysGfxPic = (uint8_t *)File_LoadToMemory("n_sysgfx.pic");
    if (!g_pSysGfxPic) {
        BootLog("[MAINDOS] FATAL: Error while loading N_SYSGFX.PIC");
        printf("Error while loading N_SYSGFX.PIC\n");
        exit(1);
    }
    
    g_pSysG2Pic = (uint8_t *)File_LoadToMemory("n_sysg_2.pic");
    if (!g_pSysG2Pic) {
        BootLog("[MAINDOS] FATAL: Error while loading N_SYSG_2.PIC");
        printf("Error while loading N_SYSG_2.PIC\n");
        exit(1);
    }
    
    g_pSysCol = (uint8_t *)File_LoadToMemory("sys.col");
    if (!g_pSysCol) {
        BootLog("[MAINDOS] FATAL: Error while loading SYS.COL");
        printf("Error while loading SYS.COL\n");
        exit(1);
    }
    
    BootLog("[MAINDOS] Load_SystemGraphicsAndFonts: Loading system fonts...");
    g_SystemFonts[0] = Font_Load("baltazar\\data\\red_dark.lft", 0);
    g_SystemFonts[1] = Font_Load("baltazar\\data\\red_lite.lft", 0);
    g_SystemFonts[2] = Font_Load("baltazar\\data\\bluedark.lft", 0);
    g_SystemFonts[3] = Font_Load("baltazar\\data\\bluelite.lft", 0);
    g_SystemFonts[4] = Font_Load("baltazar\\data\\red_grey.lft", 0);
    g_SystemFonts[5] = Font_Load("baltazar\\data\\bluegrey.lft", 0);
    g_SystemFonts[6] = Font_Load("fonts\\mini.lft", 0);
    g_SystemFonts[7] = Font_Load("fonts\\small.lft", 0);
    
    for (i = 0; i < 8; i++) {
        int font_id = g_SystemFonts[i];
        if (font_id == -1) {
            char err[64];
            sprintf(err, "[MAINDOS] FATAL: Font %d failed to load!", i);
            BootLog(err);
            printf("Cannot use this graphics mode\n");
            exit(-1);
        }
        g_fonts[font_id].alignment = 1;
        g_fonts[font_id].field_08 = 1;
    }
    BootLog("[MAINDOS] Load_SystemGraphicsAndFonts: All system assets & fonts loaded successfully.");
}

/**
 * @original Font_LoadHUDFonts (MAINDOS.EXE @ 0x000240f4, geputget.c)
 * @fidelity EXACT
 */
void Font_LoadHUDFonts(void) {
    char path[64];
    int fd;
    int size;
    
    sprintf(path, "%sIGNITION.FNT", "FONTS\\");
    fd = open(path, O_RDONLY | O_BINARY);
    if (fd < 0) {
        printf("Error while trying to read %s\n", path);
        exit(1);
    }
    size = filelength(fd);
    g_pHUDFonts = (uint8_t *)Mem_Alloc(1, size);
    g_MemoryAllocated += size;
    if (g_pHUDFonts == NULL) {
        exit(1);
    }
    read(fd, g_pHUDFonts, size);
    close(fd);
    
    sprintf(path, "%syellow_s.lft", "FONTS\\");
    g_hudFontYellowSmall = Font_Load(path, 0);
    if (g_hudFontYellowSmall == -1) { exit(-1); }
    
    sprintf(path, "%spos_s.lft", "FONTS\\");
    g_hudFontPosSmall = Font_Load(path, 0);
    if (g_hudFontPosSmall == -1) { exit(-1); }
    
    sprintf(path, "%sspeed_s.lft", "FONTS\\");
    g_hudFontSpeedSmall = Font_Load(path, 0);
    if (g_hudFontSpeedSmall == -1) { exit(-1); }
    
    sprintf(path, "%syellow.lft", "FONTS\\");
    g_hudFontYellow = Font_Load(path, 0);
    if (g_hudFontYellow == -1) { exit(-1); }
    
    sprintf(path, "%spos.lft", "FONTS\\");
    g_hudFontPos = Font_Load(path, 0);
    if (g_hudFontPos == -1) { exit(-1); }
    
    sprintf(path, "%sspeed.lft", "FONTS\\");
    g_hudFontSpeed = Font_Load(path, 0);
    if (g_hudFontSpeed == -1) { exit(-1); }
    
    sprintf(path, "%syellow_h.lft", "FONTS\\");
    g_hudFontYellowHuge = Font_Load(path, 0);
    if (g_hudFontYellowHuge == -1) { exit(-1); }
    
    sprintf(path, "%spos_h.lft", "FONTS\\");
    g_hudFontPosHuge = Font_Load(path, 0);
    if (g_hudFontPosHuge == -1) { exit(-1); }
    
    sprintf(path, "%sspeed_h.lft", "FONTS\\");
    g_hudFontSpeedHuge = Font_Load(path, 0);
    if (g_hudFontSpeedHuge == -1) { exit(-1); }
}

/**
 * @original Track_LoadOverlayGfx (MAINDOS.EXE @ 0x000243e0, geputget.c)
 * @fidelity EXACT
 */
void Track_LoadOverlayGfx(void) {
    int fd;
    int size;

    fd = open("s_pangfx.pic", O_RDONLY | O_BINARY);
    if (fd < 0) { printf("Error\n"); exit(1); }
    size = filelength(fd);
    g_pSPangfxPic = (uint8_t *)Mem_Alloc(1, size);
    g_MemoryAllocated += size;
    if (g_pSPangfxPic == NULL) exit(1);
    read(fd, g_pSPangfxPic, size);
    close(fd);
    
    fd = open("n_pangfx.pic", O_RDONLY | O_BINARY);
    if (fd < 0) { printf("Error\n"); exit(1); }
    size = filelength(fd);
    g_pNPangfxPic = (uint8_t *)Mem_Alloc(1, size);
    g_MemoryAllocated += size;
    if (g_pNPangfxPic == NULL) exit(1);
    read(fd, g_pNPangfxPic, size);
    close(fd);
    
    fd = open("h_pan1.pic", O_RDONLY | O_BINARY);
    if (fd < 0) { printf("Error\n"); exit(1); }
    size = filelength(fd);
    g_pHPan1Pic = (uint8_t *)Mem_Alloc(1, size);
    g_MemoryAllocated += size;
    if (g_pHPan1Pic == NULL) exit(1);
    read(fd, g_pHPan1Pic, size);
    close(fd);
    
    fd = open("h_pan2.pic", O_RDONLY | O_BINARY);
    if (fd < 0) { printf("Error\n"); exit(1); }
    size = filelength(fd);
    g_pHPan2Pic = (uint8_t *)Mem_Alloc(1, size);
    g_MemoryAllocated += size;
    if (g_pHPan2Pic == NULL) exit(1);
    read(fd, g_pHPan2Pic, size);
    close(fd);
    
    fd = open("s_signs.pic", O_RDONLY | O_BINARY);
    if (fd < 0) { printf("Error\n"); exit(1); }
    size = filelength(fd);
    g_pSSignsPic = (uint8_t *)Mem_Alloc(1, size);
    g_MemoryAllocated += size;
    if (g_pSSignsPic == NULL) exit(1);
    read(fd, g_pSSignsPic, size);
    close(fd);
    
    fd = open("n_signs.pic", O_RDONLY | O_BINARY);
    if (fd < 0) { printf("Error\n"); exit(1); }
    size = filelength(fd);
    g_pNSignsPic = (uint8_t *)Mem_Alloc(1, size);
    g_MemoryAllocated += size;
    if (g_pNSignsPic == NULL) exit(1);
    read(fd, g_pNSignsPic, size);
    close(fd);
    
    fd = open("h_signs.pic", O_RDONLY | O_BINARY);
    if (fd < 0) { printf("Error\n"); exit(1); }
    size = filelength(fd);
    g_pHSignsPic = (uint8_t *)Mem_Alloc(1, size);
    g_MemoryAllocated += size;
    if (g_pHSignsPic == NULL) exit(1);
    read(fd, g_pHSignsPic, size);
    close(fd);
    
    fd = open("pokal.pic", O_RDONLY | O_BINARY);
    if (fd < 0) { printf("Error\n"); exit(1); }
    size = filelength(fd);
    g_pPokalPic = (uint8_t *)Mem_Alloc(1, size);
    g_MemoryAllocated += size;
    if (g_pPokalPic == NULL) exit(1);
    read(fd, g_pPokalPic, size);
    close(fd);
}

/* ========================================================================= */
/* Authentic DOS Keyboard Driver Subsystem (MAINDOS.EXE @ 0x559e4 - 0x55dfc) */
/* ========================================================================= */

extern int g_TickInt; /* ds:0xab5dc */

// @original Input_InitKeyTables (MAINDOS.EXE @ 0x55d04, geputget.c)
// @fidelity EXACT
int Input_InitKeyTables(void) {
    int i;
    if (!g_KeyIsrInstalled) {
        for (i = 1; i <= 256; i++) {
            g_KeyRawState[i - 1] = 0;
            g_KeyReleasedFlag[i - 1] = 1;
            g_KeyToggleMask[i - 1] = 0;
        }
        for (i = 1; i <= 512; i++) {
            g_KeyToggleState[i - 1] = 0;
        }
        for (i = 1; i <= 16; i++) {
            g_KeyScancodeRingBuf[i - 1] = 0;
        }
        g_KeyIsrScancodeHead = 0;
        g_KeyIsrInstalled = 1;
    }
    return 1;
}

// @original Input_InitKeyboard (MAINDOS.EXE @ 0x559e4, geputget.c)
// @fidelity EXACT
int Input_InitKeyboard(int initial_delay, int repeat_interval) {
    int i;
    if (g_KeyDriverInstalled == 1) {
        return 0;
    }
    Input_InitKeyTables();
    g_KeyDriverInstalled = 1;
    g_KeyAsciiRingWritePtr = g_KeyAsciiRingBuf;
    memset(g_KeyAsciiRingBuf, 0xFF, sizeof(g_KeyAsciiRingBuf));

    for (i = 0; i < 256; i++) {
        g_KeyJustPressed[i] = 0;
        g_KeyboardState[i] = 0;
        g_KeyPreviousDown[i] = 0;
        g_KeyRepeatTriggered[i] = 0;
        g_KeyRepeatActiveTimer[i] = 0;
    }

    g_KeyRepeatInitialDelay = initial_delay;
    g_KeyRepeatInterval = repeat_interval;
    g_KeyLastPollTick = g_TickInt;
    return 1;
}

// @original Input_ShutdownKeyboard (MAINDOS.EXE @ 0x55a94, geputget.c)
// @fidelity EXACT
int Input_ShutdownKeyboard(void) {
    if (g_KeyDriverInstalled != 0) {
        g_KeyIsrInstalled = 0;
        g_KeyDriverInstalled = 0;
    }
    return 1;
}

// @original Input_PollKeyboard (MAINDOS.EXE @ 0x55ae4, geputget.c)
// @fidelity EXACT
void Input_PollKeyboard(void) {
    int delta = g_TickInt - g_KeyLastPollTick;
    int threshold = g_KeyRepeatInitialDelay + g_KeyRepeatInterval;
    int i;

    /* 1. Advance repeat timers and trigger auto-repeat pulses */
    for (i = 0; i < 256; i++) {
        if (g_KeyboardState[i] == 1) {
            g_KeyRepeatActiveTimer[i] += delta;
            if (g_KeyRepeatActiveTimer[i] >= threshold) {
                g_KeyRepeatTriggered[i] = 1;
                g_KeyRepeatActiveTimer[i] -= g_KeyRepeatInterval;
            }
        }
    }

    /* 2. Update keydown and just-pressed edge states */
    for (i = 0; i < 256; i++) {
        uint8_t raw = g_KeyRawState[i];
        if (raw == 1) {
            uint8_t prev = g_KeyPreviousDown[i];
            g_KeyboardState[i] = 1;
            if (prev == 0) {
                g_KeyJustPressed[i] = 1;
                g_KeyPreviousDown[i] = 1;
            }
            if (g_KeyRepeatActiveTimer[i] == 0) {
                g_KeyRepeatTriggered[i] = 1;
            }
            if (g_KeyCallback != NULL) {
                g_KeyCallback(1, i);
            }
            if (i <= 0x53) {
                char ch = (char)g_ScancodeToAsciiTable[i];
                *g_KeyAsciiRingWritePtr = ch;
                g_KeyAsciiRingWritePtr[32] = ch;
                g_KeyAsciiRingWritePtr++;
                if (g_KeyAsciiRingWritePtr == g_KeyAsciiRingBuf + 32) {
                    g_KeyAsciiRingWritePtr = g_KeyAsciiRingBuf;
                }
            }
        } else {
            g_KeyJustPressed[i] = 0;
            g_KeyboardState[i] = 0;
            g_KeyPreviousDown[i] = 0;
            g_KeyRepeatTriggered[i] = 0;
            g_KeyRepeatActiveTimer[i] = 0;
            if (g_KeyCallback != NULL) {
                g_KeyCallback(0, i);
            }
        }
    }

    g_KeyLastPollTick = g_TickInt;
}

// @original Input_IsKeyDown (MAINDOS.EXE @ 0x55c28, geputget.c)
// @fidelity EXACT
int Input_IsKeyDown(int scancode) {
    return (int)g_KeyboardState[scancode & 0xFF];
}

// @original Input_WasKeyPressed (MAINDOS.EXE @ 0x55c3c, geputget.c)
// @fidelity EXACT
int Input_WasKeyPressed(int scancode) {
    scancode &= 0xFF;
    if (g_KeyJustPressed[scancode] != 0) {
        g_KeyJustPressed[scancode] = 0;
        return 1;
    }
    return 0;
}

// @original Input_WasKeyRepeated (MAINDOS.EXE @ 0x55c60, geputget.c)
// @fidelity EXACT
int Input_WasKeyRepeated(int scancode) {
    scancode &= 0xFF;
    if (g_KeyRepeatTriggered[scancode] != 0) {
        g_KeyRepeatTriggered[scancode] = 0;
        return 1;
    }
    return 0;
}

// @original Input_SetKeyCallback (MAINDOS.EXE @ 0x55c84, geputget.c)
// @fidelity EXACT
void Input_SetKeyCallback(void (*cb)(int, int)) {
    g_KeyCallback = cb;
    if (g_KeyCallback != NULL) {
        g_KeyCallback(0, 0);
    }
}

// @original Input_EnqueueAscii (MAINDOS.EXE @ 0x55ca0, geputget.c)
// @fidelity EXACT
void Input_EnqueueAscii(int scancode) {
    if (scancode <= 0x53) {
        char ch = (char)g_ScancodeToAsciiTable[scancode];
        *g_KeyAsciiRingWritePtr = ch;
        g_KeyAsciiRingWritePtr[32] = ch;
        g_KeyAsciiRingWritePtr++;
        if (g_KeyAsciiRingWritePtr == g_KeyAsciiRingBuf + 32) {
            g_KeyAsciiRingWritePtr = g_KeyAsciiRingBuf;
        }
    }
}

// @original Input_GetQueuedKey (MAINDOS.EXE @ 0x55cd0, geputget.c)
// @fidelity EXACT
char *Input_GetQueuedKey(char *query_str) {
    char *match;
    char *p = query_str;
    while (*p != '\0') {
        if (*p >= 'a' && *p <= 'z') {
            *p -= 0x20;
        }
        p++;
    }
    match = strstr(g_KeyAsciiRingBuf, query_str);
    if (match != NULL) {
        memset(g_KeyAsciiRingBuf, 0xFF, sizeof(g_KeyAsciiRingBuf));
    }
    return match;
}

