/*
 * geputget.c - 2D Graphics blitting, font rasterization, and palette management
 * Original file: geputget.c
 * Target: MAINDOS.EXE (Watcom C/C++ 10.6, 32-bit flat protected mode)
 */

#include "geputget.h"
#include <string.h>

/* Global font table matching MAINDOS @ 0x0024C214 */
FontSlot g_fonts[MAX_FONTS];

/* Global font system state */
int g_fontSystemInitialized = 0; /* MAINDOS @ 0x000D7C60 */
int g_fontSubsystemHandle = 0;   /* MAINDOS @ 0x0024C210 */

/* External subsystem helpers */
extern int Subsystem_Register(void);
extern void Subsystem_AddCallback(int handle);
extern void Subsystem_Unregister(int handle);
extern void *Gfx_SpriteOp(void *desc, int op);

/**
 * Font_InitSystem (MAINDOS @ 0x00061220)
 * Initializes the font subsystem and prepares all 30 font slots.
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
 * Font_Shutdown (MAINDOS @ 0x00061319)
 * Shuts down the font subsystem and unloads all active font slots.
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
 * Font_Load (MAINDOS @ 0x000615eb, IGN_WIN @ 0x00456270)
 * Loads a .LFT font from disk, creates glyph handles, and frees the source buffer.
 */
int Font_Load(const char *filename, int font_id) {
    void *buffer;
    int result;

    buffer = File_LoadToMemory(filename);
    if (buffer == NULL) {
        g_fileErrorLine = 1000;
        return -1;
    }

    result = Font_Parse(buffer, font_id);
    Mem_Free(0, buffer);
    return result;
}

/**
 * Font_Unload (MAINDOS @ 0x00061653)
 * Frees all sprite handles for a font slot and marks the slot available.
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
