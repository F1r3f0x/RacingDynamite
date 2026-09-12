/*
 * geputget.c - 2D Graphics blitting, font rasterization, and palette management
 * Original file: geputget.c
 * Target: MAINDOS.EXE (Watcom C/C++ 10.6, 32-bit flat protected mode)
 *         IGN_WIN.EXE (MSVC 4.x / 5.0, Win32)
 */

#include "geputget.h"
#include <string.h>

/* Global font table matching MAINDOS @ 0x0024C214, IGN_WIN @ 0x0063F2E0 */
FontSlot g_fonts[MAX_FONTS];

/* Global font system state */
int g_fontSystemInitialized = 0; /* MAINDOS @ 0x000D7C60, IGN_WIN @ 0x004BA6C4 */
int g_fontSubsystemHandle = 0;   /* MAINDOS @ 0x0024C210, IGN_WIN @ 0x0050E680 */

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
 * @original Font_InitSystem (IGN_WIN.EXE @ 0x00455580, geputget.c)
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
 * @original Font_Shutdown (IGN_WIN.EXE @ 0x00455610, geputget.c)
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
 * @original Font_Parse (IGN_WIN.EXE @ 0x00455670, geputget.c)
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
 * @original Font_Load (IGN_WIN.EXE @ 0x00455820, geputget.c)
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
 * @original Font_Unload (IGN_WIN.EXE @ 0x00455870, geputget.c)
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
 * @original Font_GetTextWidth (IGN_WIN.EXE @ 0x004558d0, geputget.c)
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
 * @original Font_DrawText (IGN_WIN.EXE @ 0x00455a60, geputget.c)
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
 * @original Load_SystemGraphicsAndFonts (IGN_WIN.EXE @ 0x00418130)
 * @fidelity ADAPTED
 */
void Load_SystemGraphicsAndFonts(void) {
    int i;
    
    g_pSysGfxPic = (uint8_t *)File_LoadToMemory("n_sysgfx.pic");
    if (!g_pSysGfxPic) exit(1);
    
    g_pSysG2Pic = (uint8_t *)File_LoadToMemory("n_sysg_2.pic");
    if (!g_pSysG2Pic) exit(1);
    
    g_pSysCol = (uint8_t *)File_LoadToMemory("sys.col");
    if (!g_pSysCol) exit(1);
    
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
            exit(-1);
        }
        g_fonts[font_id].field_08 = 1;
        g_fonts[font_id].extra_spacing = 1;
    }
}

/**
 * @original Font_LoadHUDFonts (IGN_WIN.EXE @ 0x0041ac40)
 * @fidelity ADAPTED
 */
void Font_LoadHUDFonts(void) {
    char path[64];
    
    sprintf(path, "%sIGNITION.FNT", "FONTS\\");
    g_pHUDFonts = (uint8_t *)File_LoadToMemory(path);
    if (!g_pHUDFonts) exit(1);
    
    sprintf(path, "%syellow_s.lft", "FONTS\\");
    g_hudFontYellowSmall = Font_Load(path, 0);
    if (g_hudFontYellowSmall == -1) exit(-1);
    
    sprintf(path, "%spos_s.lft", "FONTS\\");
    g_hudFontPosSmall = Font_Load(path, 0);
    if (g_hudFontPosSmall == -1) exit(-1);
    
    sprintf(path, "%sspeed_s.lft", "FONTS\\");
    g_hudFontSpeedSmall = Font_Load(path, 0);
    if (g_hudFontSpeedSmall == -1) exit(-1);
    
    sprintf(path, "%syellow.lft", "FONTS\\");
    g_hudFontYellow = Font_Load(path, 0);
    if (g_hudFontYellow == -1) exit(-1);
    
    sprintf(path, "%spos.lft", "FONTS\\");
    g_hudFontPos = Font_Load(path, 0);
    if (g_hudFontPos == -1) exit(-1);
    
    sprintf(path, "%sspeed.lft", "FONTS\\");
    g_hudFontSpeed = Font_Load(path, 0);
    if (g_hudFontSpeed == -1) exit(-1);
    
    sprintf(path, "%syellow_h.lft", "FONTS\\");
    g_hudFontYellowHuge = Font_Load(path, 0);
    if (g_hudFontYellowHuge == -1) exit(-1);
    
    sprintf(path, "%spos_h.lft", "FONTS\\");
    g_hudFontPosHuge = Font_Load(path, 0);
    if (g_hudFontPosHuge == -1) exit(-1);
    
    sprintf(path, "%sspeed_h.lft", "FONTS\\");
    g_hudFontSpeedHuge = Font_Load(path, 0);
    if (g_hudFontSpeedHuge == -1) exit(-1);
}

/**
 * @original Track_LoadOverlayGfx (IGN_WIN.EXE @ 0x0041af70)
 * @fidelity ADAPTED
 */
void Track_LoadOverlayGfx(void) {
    g_pSPangfxPic = (uint8_t *)File_LoadToMemory("s_pangfx.pic");
    if (!g_pSPangfxPic) exit(1);
    
    g_pNPangfxPic = (uint8_t *)File_LoadToMemory("n_pangfx.pic");
    if (!g_pNPangfxPic) exit(1);
    
    g_pHPan1Pic = (uint8_t *)File_LoadToMemory("h_pan1.pic");
    if (!g_pHPan1Pic) exit(1);
    
    g_pHPan2Pic = (uint8_t *)File_LoadToMemory("h_pan2.pic");
    if (!g_pHPan2Pic) exit(1);
    
    g_pSSignsPic = (uint8_t *)File_LoadToMemory("s_signs.pic");
    if (!g_pSSignsPic) exit(1);
    
    g_pNSignsPic = (uint8_t *)File_LoadToMemory("n_signs.pic");
    if (!g_pNSignsPic) exit(1);
    
    g_pHSignsPic = (uint8_t *)File_LoadToMemory("h_signs.pic");
    if (!g_pHSignsPic) exit(1);
    
    g_pPokalPic = (uint8_t *)File_LoadToMemory("pokal.pic");
    if (!g_pPokalPic) exit(1);
}
