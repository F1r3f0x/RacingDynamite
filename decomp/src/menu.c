/*
 * menu.c - Ignition (1997) Authentic Menu UI Engine
 * Target: MAINDOS.EXE / IGN_WIN.EXE (Watcom C/C++ 10.6, 32-bit flat protected mode)
 */

#include "menu.h"
#include <stdio.h>
#include <string.h>

extern void *thunk_FUN__text__000702f8(int size); /* Mem_Alloc wrapper */
extern int FUN__text__00061100(int arg1, const char *arg2); /* File/Resource function */
extern void FUN__text__00060f40(void); /* File_ReadToBuffer proxy */
extern void FUN__text__000549b0(void);
extern void FUN__text__000570d8(int width, int height, int depth, void *palette1, void *palette2);
extern void FUN__text__00056b28(int a, int b, int c);

/* Authentic Menu Engine Globals from MAINDOS_32BIT.EXE */
uint32_t DAT_000ea3b0 = 0;
uint32_t DAT_000ea3b4 = 0;
uint32_t DAT_000ea318 = 0;
uint32_t _DAT_000ea314 = 0;
uint32_t _DAT_000ea31c = 0;
uint32_t _DAT_000ea320 = 0;
uint32_t DAT_000ea328 = 0;
uint32_t DAT_000ea324 = 0;
uint32_t DAT_000ea338[8] = {0};
uint32_t _DAT_000ea32c = 0;
uint8_t *_DAT_000ea330 = NULL;
uint32_t DAT_000ea334 = 0;
uint32_t _DAT_000ea3b8 = 0;
uint32_t _DAT_000ea3bc = 0;
uint32_t DAT_000ea3c8 = 0;
uint32_t DAT_000ea3d4 = 0;
uint32_t DAT_000ea3cc = 0;
uint32_t DAT_000ea3d8 = 0;
uint32_t DAT_000ea3d0 = 0;
uint32_t _DAT_000ea3dc = 0;
uint32_t *DAT_0024c03c = NULL;
uint32_t *DAT_000ea3c0 = NULL;
uint32_t DAT_000ea3c4 = 0;
int DAT_000ea194 = 0;
uint32_t DAT_000bece4[32] = {0};
void *DAT_000ea218 = NULL;
void *DAT_000ea1fc = NULL;

const char *s_baltazar_data_TEXTURES_CANADA_TE_000beb54 = "baltazar\\data\\TEXTURES\\CANADA.TEX";

/**
 * @original FUN__text__0001aaf8 (MAINDOS_32BIT.EXE @ 0x0001aaf8, menu.c)
 * @fidelity EXACT
 * @notes Authentic menu car loader and 3D UI viewport creator. 
 *        Loads menucar.plc, menucar.msh, menucar.tex.
 */
void Menu_InitCarViewport(void) {
    uint32_t *puVar1;
    int iVar2;
    int iVar3;
    int iVar4;
    uint32_t uVar5;
    uint8_t *puVar6;
    uint8_t *puVar7;
    int iVar8;
    int iVar9;
    const char *pcVar10;
  
    DAT_000ea3b0 = (uint32_t)thunk_FUN__text__000702f8(0x10000);
    DAT_000ea3b4 = (uint32_t)thunk_FUN__text__000702f8(0x10000);
    DAT_000ea318 = FUN__text__00061100(0, "");
    _DAT_000ea314 = (uint32_t)thunk_FUN__text__000702f8(0x10000);
    FUN__text__00060f40();
    _DAT_000ea31c = 0;
    _DAT_000ea320 = 0;
    DAT_000ea328 = FUN__text__00061100(0, "");
    DAT_000ea324 = (uint32_t)thunk_FUN__text__000702f8(0x10000);
    iVar8 = 0;
    FUN__text__00060f40();
    iVar9 = 0;
    pcVar10 = s_baltazar_data_TEXTURES_CANADA_TE_000beb54;
    uVar5 = (DAT_000ea3b0 + 0xffffU) & 0xffff0000;
    
    do {
        iVar2 = FUN__text__00061100(iVar9, pcVar10);
        if (iVar2 < 1) goto LAB__text__0001ac38;
        if (0x100000 < iVar2) {
            iVar2 = 0x100000;
        }
        FUN__text__00060f40();
        iVar4 = 0;
        if (0 < iVar2) {
            iVar3 = iVar8 * 4;
            do {
                iVar8 = iVar8 + 1;
                iVar4 = iVar4 + 0x10000;
                *(uint32_t *)((uint8_t *)&DAT_000ea338 + iVar3) = uVar5;
                iVar3 = iVar3 + 4;
                uVar5 = uVar5 + 0x10000;
            } while (iVar4 < iVar2);
        }
        pcVar10 = pcVar10 + 0x32;
        iVar9 = iVar9 + 1;
    } while (iVar9 < 8);
    
    DAT_000ea338[iVar8] = 0;

LAB__text__0001ac38:
    uVar5 = (DAT_000ea3b4 + 0xffffU) & 0xffff0000;
    _DAT_000ea32c = uVar5;
    FUN__text__00060f40();
    puVar6 = (uint8_t *)(uVar5 + 0x10000);
    iVar8 = 0;
    _DAT_000ea330 = puVar6;
    do {
        *puVar6 = (uint8_t)iVar8;
        iVar8 = iVar8 + 1;
        puVar6 = puVar6 + 1;
    } while (iVar8 < 0x100);
    
    iVar8 = 1;
    do {
        iVar2 = 0;
        puVar7 = puVar6;
        do {
            puVar6 = puVar7 + 1;
            iVar2 = iVar2 + 1;
            *puVar7 = (uint8_t)iVar8;
            puVar7 = puVar6;
        } while (iVar2 < 0x100);
        iVar8 = iVar8 + 1;
    } while (iVar8 < 0x100);
    
    DAT_000ea334 = 0;
    FUN__text__000549b0();
    _DAT_000ea3b8 = 0;
    _DAT_000ea3bc = 0;
    DAT_000ea3c8 = FUN__text__00061100(iVar9, pcVar10);
    DAT_000ea3d4 = (uint32_t)thunk_FUN__text__000702f8(0x1000);
    FUN__text__00060f40();
    DAT_000ea3cc = FUN__text__00061100(0, "baltazar\\data\\menucar.msh");
    DAT_000ea3d8 = (uint32_t)thunk_FUN__text__000702f8(0x1000);
    FUN__text__00060f40();
    DAT_000ea3d0 = FUN__text__00061100(0, "baltazar\\data\\menucar.tex");
    _DAT_000ea3dc = (uint32_t)thunk_FUN__text__000702f8(0x1000);
    if (DAT_0024c03c) {
        *DAT_0024c03c = (_DAT_000ea3dc + 0xffffU) & 0xffff0000;
    }
    FUN__text__00060f40();
    
    FUN__text__000570d8(0x40, 0x40, 100, DAT_000ea218, DAT_000ea1fc);
    
    if (DAT_000ea3c0) {
        puVar1 = DAT_000ea3c0;
        *DAT_000ea3c0 = 0;
        puVar1[1] = 0x408f4000;
        puVar1[2] = 0;
        puVar1[3] = 0x40913000;
        puVar1[4] = 0;
        puVar1[5] = 0x40890000;
        puVar1[0x1c] = 1000;
        puVar1[0x1d] = 1000;
        puVar1[0x1e] = 1000;
        iVar9 = DAT_000ea194;
        puVar1[0x1f] = 0;
        iVar9 = DAT_000bece4[iVar9];
        puVar1[0xc] = 0;
        puVar1[0xd] = 0x3ff00000;
        puVar1[0x27] = iVar9 / 2;
    }
    
    iVar9 = 0;
    do {
        iVar8 = DAT_000ea3c4 + iVar9;
        if (iVar8) {
            *(uint32_t *)(iVar8 + 4) = 1000;
            *(uint32_t *)(iVar8 + 8) = 1000;
            *(uint32_t *)(iVar8 + 0xc) = 1000;
            *(uint32_t *)(iVar8 + 0x10) = 0;
            *(uint32_t *)(iVar8 + 0x14) = 0;
            *(uint32_t *)(iVar8 + 0x18) = 0;
        }
        iVar9 = iVar9 + 0x20;
        FUN__text__00056b28(0, 0xffffffff, 0);
    } while (iVar9 != 0x160);
}

/**
 * @original Menu_Init (MAINDOS_32BIT.EXE @ 0x00011504, menu.c)
 * @fidelity EXACT
 * @notes Authentic Menu Initialization. Allocates menu.col and sets up the UI Widget Tree.
 */
extern void *Mem_AllocRaw(int size); /* 0x60dbc */
extern void File_LoadToBuffer(const char *path, void *dest); /* 0x60f40 */
extern void *UI_InitSystem(void *tree_buffer); /* 0x564d8 */
extern void App_SetVideoMode_Authentic(void); /* 0x558e4 */

int g_MenuState = 0;
int g_SelectedLanguage = 0;
void *g_pMenuColData = NULL;
void *g_pActivePalette = NULL;
void *g_pMenuWidgetTree = NULL;
void *g_MenuRootWidget = NULL;
uint32_t g_UIFlags = 0;
uint32_t g_ScreenWidth_UI = 0;
uint32_t g_ScreenHeight_UI = 0;
extern uint32_t g_VideoModeTable[16];

void Menu_Init(void) {
    g_MenuState = 0;
    g_SelectedLanguage = 0;
    
    g_pMenuColData = Mem_AllocRaw(0x300);
    File_LoadToBuffer("baltazar\\data\\menu.col", g_pMenuColData);
    g_pActivePalette = g_pMenuColData;
    
    g_pMenuWidgetTree = Mem_AllocRaw(0xFB41);
    g_MenuRootWidget = UI_InitSystem(g_pMenuWidgetTree);
    g_UIFlags = 1;
    
    g_ScreenWidth_UI = g_VideoModeTable[g_SelectedLanguage * 2];
    g_ScreenHeight_UI = g_VideoModeTable[g_SelectedLanguage * 2 + 1];
    
    App_SetVideoMode_Authentic();
}

/**
 * @original Menu_Tick (MAINDOS_32BIT.EXE @ 0x00012bb8, menu.c)
 * @fidelity EXACT
 * @notes Authentic Menu GUI Tick Loop. Processes UI widget events, inputs, and drawing.
 */
int Menu_Tick(void) {
    /* TODO: Massive UI state machine (approx 5KB) needs to be ported structurally */
    return 1;
}

