#ifndef GEPUTGET_H
#define GEPUTGET_H

#include <stdint.h>
#include <stddef.h>
#include "mem.h"

#define MAX_FONTS 30
#define FONT_GLYPH_COUNT 224

/**
 * Authentic 1997 Ignition Font Slot Structure
 * Windows native layout (1600 bytes, 0x640)
 */
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
    /* 2 bytes implicit padding */
    void *glyph_handles[FONT_GLYPH_COUNT];    /* 0x100: pointer / sprite handle */
    uint16_t widths[FONT_GLYPH_COUNT];        /* 0x480: glyph widths */
} FontSlot;

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

/* Global font table matching IGN_WIN.EXE @ 0x0063F2E0 */
extern FontSlot g_fonts[MAX_FONTS];

/* Global font system state (IGN_WIN.EXE @ 0x004BA6C4, 0x0050E680, 0x004BAB34) */
extern int g_fontSystemInitialized;
extern int g_fontSubsystemHandle;
extern int g_fileErrorLine;

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

/* Authentic keyboard subsystem globals & functions (MAINDOS.EXE @ 0x559e4 - 0x55dfc) */
extern int32_t  g_KeyRepeatActiveTimer[256];
extern uint8_t  g_KeyToggleState[256];
extern uint8_t  g_KeyRepeatTriggered[256];
extern uint8_t  g_KeyToggleMask[256];
extern uint8_t  g_KeyReleasedFlag[256];
extern uint8_t  g_KeyJustPressed[256];
extern uint8_t  g_KeyPreviousDown[256];
extern uint8_t  g_KeyboardState[256];
extern uint8_t  g_KeyScancodeRingBuf[16];
extern uint8_t  g_KeyRawState[256];
extern char    *g_KeyAsciiRingWritePtr;
extern int32_t  g_KeyLastPollTick;
extern int32_t  g_KeyRepeatInterval;
extern int32_t  g_KeyRepeatInitialDelay;
extern int32_t  g_KeyDriverInstalled;
extern void   (*g_KeyCallback)(int, int);
extern char     g_KeyAsciiRingBuf[64];
extern uint8_t  g_KeyIsrScancodeHead;
extern uint8_t  g_KeyIsrInstalled;
extern const uint8_t g_ScancodeToAsciiTable[84];

int  Input_InitKeyboard(int initial_delay, int repeat_interval);
int  Input_ShutdownKeyboard(void);
void Input_PollKeyboard(void);
int  Input_IsKeyDown(int scancode);
int  Input_WasKeyPressed(int scancode);
int  Input_WasKeyRepeated(int scancode);
void Input_SetKeyCallback(void (*cb)(int, int));
void Input_EnqueueAscii(int scancode);
char *Input_GetQueuedKey(char *query_str);
int  Input_InitKeyTables(void);

#endif /* GEPUTGET_H */
