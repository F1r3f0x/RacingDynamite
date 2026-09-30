#ifndef GEPUTGET_H
#define GEPUTGET_H

#include <stdint.h>
#include <stddef.h>
#include "mem.h"

#define MAX_FONTS 30
#define FONT_GLYPH_COUNT 224

/**
 * Authentic 1997 Ignition Font Slot Structure (1598 bytes, 0x63E)
 * Matches layout in MAINDOS.EXE @ 0x0024C214
 */
#pragma pack(push, 1)
typedef struct {
    int32_t in_use;                     /* 0x00: 1 if allocated, 0 if free */
    int32_t alignment;                  /* 0x04: 0 = left, 1 = center, 2 = right */
    int32_t field_08;                   /* 0x08: flags / mode */
    int32_t is_proportional;            /* 0x0C: 0 = fixed width, 1 = proportional */
    int32_t extra_spacing;              /* 0x10: inter-character spacing */
    int32_t field_14;                   /* 0x14: secondary spacing / flags */
    uint16_t field_18;                  /* 0x18: font header field (+6) */
    uint16_t height;                    /* 0x1A: font glyph height in pixels */
    uint16_t spacing;                   /* 0x1C: default character spacing */
    uint8_t glyph_present[FONT_GLYPH_COUNT];  /* 0x1E: 1 if glyph exists, 0 if missing */
    void *glyph_handles[FONT_GLYPH_COUNT];    /* 0xFE: pointer / sprite handle */
    uint16_t widths[FONT_GLYPH_COUNT];
    uint16_t padding;
} FontSlot;
#pragma pack(pop)

typedef struct {
    int32_t x;
    int32_t y;
} Point2D;

typedef struct {
    int32_t unk00;
    int32_t width;
    int32_t height;
    int32_t field_0c;
    void *pixels;
    int32_t stride;
    int32_t field_18;
    int32_t field_1c;
} SpriteDesc;

/* Global font table matching MAINDOS @ 0x0024C214, MAINDOS @ 0x0063F2E0 */
extern FontSlot g_fonts[MAX_FONTS];

/* Global font system initialized flag (MAINDOS @ 0x000D7C60, MAINDOS @ 0x004BA6C4) */
extern int g_fontSystemInitialized;

/* Global Graphics and Font Pointers */
extern uint8_t *g_pSysGfxPic;
extern uint8_t *g_pSysG2Pic;
extern uint8_t *g_pSysCol;
extern int g_SystemFonts[8];

extern uint8_t *g_pHUDFonts;
extern int g_hudFontYellowSmall;
extern int g_hudFontPosSmall;
extern int g_hudFontSpeedSmall;
extern int g_hudFontYellow;
extern int g_hudFontPos;
extern int g_hudFontSpeed;
extern int g_hudFontYellowHuge;
extern int g_hudFontPosHuge;
extern int g_hudFontSpeedHuge;

extern uint8_t *g_pSPangfxPic;
extern uint8_t *g_pNPangfxPic;
extern uint8_t *g_pHPan1Pic;
extern uint8_t *g_pHPan2Pic;
extern uint8_t *g_pSSignsPic;
extern uint8_t *g_pNSignsPic;
extern uint8_t *g_pHSignsPic;
extern uint8_t *g_pPokalPic;

/* Function prototypes */
int Font_InitSystem(void);
int Font_Shutdown(void);
int Font_Parse(void *buffer, int unused);
int Font_Load(const char *filename, int unused);
int Font_Unload(int font_id);
int Font_GetTextWidth(const char *text, int font_id);
int Font_DrawText(const char *text, int font_id, int x, int y);
void Font_DrawHUDText(int x, int y, const char *text, uint8_t *framebuffer, int stride, const uint8_t *font_data, uint8_t color_offset);
void Load_SystemGraphicsAndFonts(void);
void Font_LoadHUDFonts(void);
void Track_LoadOverlayGfx(void);

#endif /* GEPUTGET_H */
