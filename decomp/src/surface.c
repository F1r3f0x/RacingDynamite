/* Verified Windows DirectDraw surface subsystem.
 * Reconstruction ownership is separate from unknown original source filenames.
 */
#include <ddraw.h>
#include <stddef.h>
#include <string.h>
#include "surface.h"

/* Platform backing remains owned by main.c, not duplicated here. */
extern HINSTANCE volatile g_nativeWindowInstance;
extern HWND volatile g_nativeWindow;
extern const char g_nativeWindowClassName[];
extern const char g_nativeWindowTitle[];

typedef char surface_record_size[(sizeof(GfxSurfaceRecord)==48)?1:-1];
typedef char surface_width_offset[(offsetof(GfxSurfaceRecord,width)==28)?1:-1];
typedef char surface_height_offset[(offsetof(GfxSurfaceRecord,height)==32)?1:-1];
typedef char surface_depth_offset[(offsetof(GfxSurfaceRecord,bit_depth)==36)?1:-1];
typedef char surface_locked_offset[(offsetof(GfxSurfaceRecord,locked)==8)?1:-1];
typedef char surface_mode_offset[(offsetof(GfxSurfaceRecord,lock_mode)==12)?1:-1];
typedef char surface_read_offset[(offsetof(GfxSurfaceRecord,read_pixels)==16)?1:-1];
typedef char surface_write_offset[(offsetof(GfxSurfaceRecord,write_pixels)==20)?1:-1];
typedef char surface_pitch_offset[(offsetof(GfxSurfaceRecord,pitch_bytes)==24)?1:-1];
typedef char surface_descriptor_size[(sizeof(DDSURFACEDESC)==108)?1:-1];
typedef char surface_descriptor_pixels[(offsetof(DDSURFACEDESC,lpSurface)==36)?1:-1];
typedef char surface_descriptor_pitch[(offsetof(DDSURFACEDESC,lPitch)==16)?1:-1];
typedef char surface_lock_slot[(offsetof(IDirectDrawSurfaceVtbl,Lock)==0x64)?1:-1];
typedef char surface_unlock_slot[(offsetof(IDirectDrawSurfaceVtbl,Unlock)==0x80)?1:-1];
typedef char surface_pointer_offset[(offsetof(GfxSurfaceRecord,surface)==44)?1:-1];
typedef char surface_is_lost_slot[(offsetof(IDirectDrawSurfaceVtbl,IsLost)==0x60)?1:-1];
typedef char surface_restore_slot[(offsetof(IDirectDrawSurfaceVtbl,Restore)==0x6c)?1:-1];
typedef char surface_blit_fast_slot[(offsetof(IDirectDrawSurfaceVtbl,BltFast)==0x1c)?1:-1];

volatile GfxSurfaceRecord g_nativePrimarySurface;
volatile GfxSurfaceRecord g_nativeType1Surfaces[5];
volatile GfxSurfaceRecord g_nativeType2Surfaces[20];

/*
 * @original Gfx_InitSurfaceRecords (IGN_WIN.EXE @ 0x00456A40, inferred geputget.c)
 * @fidelity EXACT
 */
int Gfx_InitSurfaceRecords(void) {
    volatile GfxSurfaceRecord *record;
    int index;

    record = &g_nativePrimarySurface;
    record->active = 0;
    record->restore_marker = 0;
    record->locked = 0;
    record->lock_mode = 0;
    record->read_pixels = 0;
    record->write_pixels = 0;
    record->pitch_bytes = 0;
    record->width = 0;
    record->height = 0;
    record->bit_depth = 0;
    record->storage_kind = 0;
    record->surface = 0;
    for (index = 0; index < 5; index++) {
        record = &g_nativeType1Surfaces[index];
        record->active = 0;
        record->restore_marker = 0;
        record->locked = 0;
        record->lock_mode = 0;
        record->read_pixels = 0;
        record->write_pixels = 0;
        record->pitch_bytes = 0;
        record->width = 0;
        record->height = 0;
        record->bit_depth = 0;
        record->storage_kind = 1;
        record->surface = 0;
    }
    for (index = 0; index < 20; index++) {
        record = &g_nativeType2Surfaces[index];
        record->active = 0;
        record->restore_marker = 0;
        record->locked = 0;
        record->lock_mode = 0;
        record->read_pixels = 0;
        record->write_pixels = 0;
        record->pitch_bytes = 0;
        record->width = 0;
        record->height = 0;
        record->bit_depth = 0;
        record->storage_kind = 2;
        record->surface = 0;
    }
    return 1;
}

/*
 * @original Gfx_SurfaceConfigureNative (IGN_WIN.EXE @ 0x0045B730, inferred geputget.c)
 * @fidelity EXACT
 */
int Gfx_SurfaceConfigureNative(unsigned int option0, unsigned int option1,
        unsigned int option2, unsigned int option3) {
    (void)option0;
    (void)option1;
    (void)option2;
    (void)option3;
    return 2;
}

/*
 * @original Gfx_SurfaceRestoreNative (IGN_WIN.EXE @ 0x0045C730, inferred geputget.c)
 * @fidelity EXACT
 */
int Gfx_SurfaceRestoreNative(void) {
    volatile GfxSurfaceRecord *record;
    LPDIRECTDRAWSURFACE surface;
    int restored;
    int index;

    restored = 0;
    record = &g_nativePrimarySurface;
    if (record->active == 1) {
        surface = record->surface;
        if (surface->lpVtbl->IsLost(surface) == DDERR_SURFACELOST) {
            restored = 1;
            surface = record->surface;
            surface->lpVtbl->Restore(surface);
            record->restore_marker = 1;
        }
    }
    for (index = 0; index < 5; index++) {
        record = &g_nativeType1Surfaces[index];
        if (record->active == 1) {
            surface = record->surface;
            if (surface->lpVtbl->IsLost(surface) == DDERR_SURFACELOST) {
                restored++;
                surface = record->surface;
                surface->lpVtbl->Restore(surface);
                record->restore_marker = 1;
            }
        }
    }
    for (index = 0; index < 20; index++) {
        record = &g_nativeType2Surfaces[index];
        if (record->active == 1) {
            surface = record->surface;
            if (surface->lpVtbl->IsLost(surface) == DDERR_SURFACELOST) {
                restored++;
                surface = record->surface;
                surface->lpVtbl->Restore(surface);
                record->restore_marker = 1;
            }
        }
    }
    return restored <= 0;
}

/* Independently recovered Windows dispatch slots; loader-zeroed .data tail. */
GfxSurfaceConfigureProc volatile g_surfaceConfigure = 0; /* VA 0x0050EB68 */
GfxSurfaceOpenProc volatile g_surfaceOpen = 0; /* VA 0x0050EB6C */
GfxSurfaceRebuildProc volatile g_surfaceRebuild = 0; /* VA 0x0050EB70 */
GfxSurfaceShutdownProc volatile g_surfaceShutdown = 0; /* VA 0x0050EB74 */
GfxSurfaceConfigureSurfaceProc volatile g_surfaceConfigureSurface = 0; /* VA 0x0050EB78 */
GfxSurfaceBlitProc volatile g_surfaceBlit = 0; /* VA 0x0050EB7C */
GfxSurfaceCopyPixelsProc volatile g_surfaceCopyPixels = 0; /* VA 0x0050EB80 */
GfxSurfaceClearProc volatile g_surfaceClear = 0; /* VA 0x0050EB84 */
GfxSurfacePresentProc volatile g_surfacePresent = 0; /* VA 0x0050EB88 */
GfxSurfaceReservedProc volatile g_surfaceReserved = 0; /* VA 0x0050EB8C */
GfxSurfaceLockProc volatile g_surfaceLock = 0; /* VA 0x0050EB90 */
GfxSurfaceUnlockProc volatile g_surfaceUnlock = 0; /* VA 0x0050EB94 */
GfxSurfaceSetPaletteProc volatile g_surfaceSetPalette = 0; /* VA 0x0050EB98 */
GfxSurfaceRestoreProc volatile g_surfaceRestore = 0; /* VA 0x0050EB9C */

/*
 * @original Gfx_InstallSurfaceDispatch (IGN_WIN.EXE @ 0x0045B690, geputget.c)
 * @fidelity EXACT
 */
int Gfx_InstallSurfaceDispatch(void) {
    g_surfaceConfigure = Gfx_SurfaceConfigureNative;
    g_surfaceOpen = Gfx_SurfaceOpenNative;
    g_surfaceRebuild = Gfx_SurfaceRebuildNative;
    g_surfaceShutdown = Gfx_SurfaceShutdownNative;
    g_surfaceConfigureSurface = Gfx_SurfaceConfigureSurfaceNative;
    g_surfaceBlit = Gfx_SurfaceBlitNative;
    g_surfaceCopyPixels = Gfx_SurfaceCopyPixelsNative;
    g_surfaceClear = Gfx_SurfaceClearNative;
    g_surfacePresent = Gfx_SurfacePresentNative;
    g_surfaceReserved = Gfx_SurfaceReservedNative;
    g_surfaceLock = Gfx_SurfaceLockNative;
    g_surfaceUnlock = Gfx_SurfaceUnlockNative;
    g_surfaceSetPalette = Gfx_SurfaceSetPaletteNative;
    g_surfaceRestore = Gfx_SurfaceRestoreNative;
    return 1;
}


/* Native surface constructor state; independently recovered Windows initializers. */

LPDIRECTDRAW volatile g_nativeDirectDraw = 0; /* VA 0x00512C50 */
LPDIRECTDRAWCLIPPER volatile g_nativeClipper = 0; /* VA 0x004BAC8C */
LPDIRECTDRAWPALETTE volatile g_nativePalette = 0; /* VA 0x00512448 */
volatile PALETTEENTRY g_nativePaletteEntries[256]; /* VA 0x00512048 */
HWND volatile g_nativeSurfaceWindow = 0; /* VA 0x00493738 */
volatile int32_t g_nativeFullscreen = 1; /* VA 0x00493728 */
volatile int32_t g_nativeSurfaceWidth = 640; /* VA 0x004BA6E0 */
volatile int32_t g_nativeSurfaceHeight = 480; /* VA 0x004BA6E4 */
volatile int32_t g_nativeSurfaceBitDepth = 8; /* VA 0x004BA6E8 */
volatile int32_t g_nativeBackbufferCount = 1; /* VA 0x004BA6EC */
volatile int32_t g_nativeDesktopBitDepth = 8; /* VA 0x004BAC90 */
const char g_nativeBackbufferMessage[] = "Backbuffer couldn't be obtained";
const char g_nativeBackbufferLimitMessage[] = "The maximum amount of backbuffers is exceeded";

/*
 * @original Gfx_SurfaceOpenNative (IGN_WIN.EXE @ 0x0045B740, inferred geputget.c)
 * @fidelity EXACT
 */
int Gfx_SurfaceOpenNative(void) {
    HINSTANCE instance;
    HWND window;
    LPDIRECTDRAW draw;
    LPDIRECTDRAWSURFACE chain;
    LPDIRECTDRAWCLIPPER clipper;
    LPDIRECTDRAWPALETTE palette;
    DDSURFACEDESC descriptor;
    DDSCAPS caps;
    RECT rectangle;
    RECT work_area;
    HDC dc;
    LONG style;
    LONG extended_style;
    BOOL has_menu;
    int screen_width;
    int screen_height;
    int width;
    int height;
    int depth;
    int index;
    int backbuffer_limit;
    DWORD flags;
    volatile GfxSurfaceRecord *record;

    Gfx_InitSurfaceRecords();
    instance = g_nativeWindowInstance;
    screen_height = GetSystemMetrics(SM_CYSCREEN);
    screen_width = GetSystemMetrics(SM_CXSCREEN);
    window = CreateWindowExA(WS_EX_APPWINDOW, g_nativeWindowClassName,
        g_nativeWindowTitle, WS_POPUP | WS_SYSMENU, 0, 0,
        screen_width, screen_height, 0, 0, instance, 0);
    g_nativeWindow = window;
    g_nativeSurfaceWindow = window;
    if (!window) {
        return 0;
    }
    UpdateWindow(g_nativeWindow);
    SetFocus(g_nativeWindow);
    if (DirectDrawCreate(0, (LPDIRECTDRAW *)&g_nativeDirectDraw, 0) != DD_OK) {
        return 0;
    }
    flags = g_nativeFullscreen != 0 ? 0x53 : DDSCL_NORMAL;
    window = g_nativeWindow;
    draw = g_nativeDirectDraw;
    if (draw->lpVtbl->SetCooperativeLevel(draw, window, flags) != DD_OK) {
        return 0;
    }
    if (g_nativeFullscreen != 0) {
        depth = g_nativeSurfaceBitDepth;
        height = g_nativeSurfaceHeight;
        width = g_nativeSurfaceWidth;
        draw = g_nativeDirectDraw;
        if (draw->lpVtbl->SetDisplayMode(draw, width, height, depth) != DD_OK) {
            return 0;
        }
    } else {
        dc = GetDC(0);
        depth = GetDeviceCaps(dc, BITSPIXEL);
        depth = (int32_t)((uint32_t)depth * (uint32_t)GetDeviceCaps(dc, PLANES));
        g_nativeDesktopBitDepth = depth;
        ReleaseDC(0, dc);
        style = GetWindowLongA(g_nativeWindow, GWL_STYLE);
        style = (style & 0x7fffffffL) | 0xc60000L;
        SetWindowLongA(g_nativeWindow, GWL_STYLE, style);
        SetRect(&rectangle, 0, 0, 640, 480);
        extended_style = GetWindowLongA(g_nativeWindow, GWL_EXSTYLE);
        has_menu = GetMenu(g_nativeWindow) != 0;
        style = GetWindowLongA(g_nativeWindow, GWL_STYLE);
        AdjustWindowRectEx(&rectangle, style, has_menu, extended_style);
        height = (int32_t)((uint32_t)rectangle.bottom - (uint32_t)rectangle.top);
        width = (int32_t)((uint32_t)rectangle.right - (uint32_t)rectangle.left);
        SetWindowPos(g_nativeWindow, 0, 0, 0, width, height, 0x16);
        SetWindowPos(g_nativeWindow, HWND_NOTOPMOST, 0, 0, 0, 0, 0x13);
        SystemParametersInfoA(SPI_GETWORKAREA, 0, &work_area, 0);
        GetWindowRect(g_nativeWindow, &rectangle);
        if (rectangle.left < work_area.left) {
            rectangle.left = work_area.left;
        }
        if (rectangle.top < work_area.top) {
            rectangle.top = work_area.top;
        }
        SetWindowPos(g_nativeWindow, 0, rectangle.left, rectangle.top, 0, 0, 0x15);
    }
    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.dwSize = sizeof(descriptor);
    if (g_nativeFullscreen != 0) {
        descriptor.dwBackBufferCount = g_nativeBackbufferCount;
        descriptor.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
        descriptor.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
        draw = g_nativeDirectDraw;
        if (draw->lpVtbl->CreateSurface(draw, &descriptor,
                (LPDIRECTDRAWSURFACE *)&g_nativePrimarySurface.surface, 0) != DD_OK) {
            return 0;
        }
        width = g_nativeSurfaceWidth;
        height = g_nativeSurfaceHeight;
        g_nativePrimarySurface.active = 1;
        g_nativePrimarySurface.restore_marker = 1;
        g_nativePrimarySurface.width = width;
        depth = g_nativeSurfaceBitDepth;
        g_nativePrimarySurface.height = height;
        index = g_nativeBackbufferCount;
        g_nativePrimarySurface.bit_depth = depth;
        if (index >= 5) {
            MessageBoxA(g_nativeWindow, g_nativeBackbufferLimitMessage, 0, 0);
            return 0;
        }
        if (g_nativeBackbufferCount > 0) {
            chain = g_nativePrimarySurface.surface;
            caps.dwCaps = DDSCAPS_BACKBUFFER;
            index = 0;
            do {
                record = &g_nativeType1Surfaces[index];
                if (index != 0) {
                    caps.dwCaps = DDSCAPS_FLIP;
                }
                if (chain->lpVtbl->GetAttachedSurface(chain, &caps, &chain) != DD_OK) {
                    MessageBoxA(g_nativeWindow, g_nativeBackbufferMessage, 0, 0);
                    return 0;
                }
                width = g_nativeSurfaceWidth;
                record->surface = chain;
                height = g_nativeSurfaceHeight;
                record->active = 1;
                depth = g_nativeSurfaceBitDepth;
                record->restore_marker = 1;
                record->width = width;
                record->height = height;
                index++;
                backbuffer_limit = g_nativeBackbufferCount;
                record->bit_depth = depth;
            } while (index < backbuffer_limit);
        }
    } else {
        descriptor.dwFlags = DDSD_CAPS;
        descriptor.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
        draw = g_nativeDirectDraw;
        if (draw->lpVtbl->CreateSurface(draw, &descriptor,
                (LPDIRECTDRAWSURFACE *)&g_nativePrimarySurface.surface, 0) != DD_OK) {
            return 0;
        }
        width = g_nativeSurfaceWidth;
        height = g_nativeSurfaceHeight;
        g_nativePrimarySurface.active = 1;
        g_nativePrimarySurface.restore_marker = 1;
        g_nativePrimarySurface.width = width;
        depth = g_nativeSurfaceBitDepth;
        g_nativePrimarySurface.height = height;
        descriptor.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
        descriptor.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
        descriptor.dwWidth = 640;
        descriptor.dwHeight = 480;
        index = g_nativeBackbufferCount;
        g_nativePrimarySurface.bit_depth = depth;
        if (index >= 5) {
            MessageBoxA(g_nativeWindow, g_nativeBackbufferLimitMessage, 0, 0);
            return 0;
        }
        if (g_nativeBackbufferCount > 0) {
            index = 0;
            do {
                record = &g_nativeType1Surfaces[index];
                draw = g_nativeDirectDraw;
                if (draw->lpVtbl->CreateSurface(draw, &descriptor,
                        (LPDIRECTDRAWSURFACE *)&record->surface, 0) != DD_OK) {
                    return 0;
                }
                record->active = 1;
                width = g_nativeSurfaceWidth;
                record->restore_marker = 1;
                height = g_nativeSurfaceHeight;
                record->width = width;
                depth = g_nativeSurfaceBitDepth;
                record->height = height;
                record->bit_depth = depth;
                index++;
            } while (index < g_nativeBackbufferCount);
        }
        draw = g_nativeDirectDraw;
        if (draw->lpVtbl->CreateClipper(draw, 0, (LPDIRECTDRAWCLIPPER *)&g_nativeClipper, 0) != DD_OK) {
            return 0;
        }
        window = g_nativeWindow;
        clipper = g_nativeClipper;
        if (clipper->lpVtbl->SetHWnd(clipper, 0, window) != DD_OK) {
            return 0;
        }
        clipper = g_nativeClipper;
        chain = g_nativePrimarySurface.surface;
        if (chain->lpVtbl->SetClipper(chain, clipper) != DD_OK) {
            /* VA 0x0045BD11 returns success before restoration/show here. */
            return 1;
        }
    }
    descriptor.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    descriptor.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    if (g_nativeFullscreen != 0) {
        if (g_nativePalette != 0) {
            palette = g_nativePalette;
            palette->lpVtbl->Release(palette);
            g_nativePalette = 0;
        }
        g_nativePaletteEntries[0].peRed = 0;
        g_nativePaletteEntries[0].peGreen = 0;
        g_nativePaletteEntries[0].peBlue = 0;
        for (index = 1; index < 256; index++) {
            g_nativePaletteEntries[index].peRed = 255;
            g_nativePaletteEntries[index].peGreen = 255;
            g_nativePaletteEntries[index].peBlue = 255;
        }
        draw = g_nativeDirectDraw;
        if (draw->lpVtbl->CreatePalette(draw, DDPCAPS_8BIT | DDPCAPS_ALLOW256,
                (LPPALETTEENTRY)g_nativePaletteEntries, (LPDIRECTDRAWPALETTE *)&g_nativePalette, 0) != DD_OK) {
            return 0;
        }
        palette = g_nativePalette;
        chain = g_nativePrimarySurface.surface;
        if (chain->lpVtbl->SetPalette(chain, palette) != DD_OK) {
            return 0;
        }
    }
    Gfx_SurfaceRestoreNative();
    ShowWindow(g_nativeWindow, SW_SHOW);
    return 1;
}

/* End native surface constructor. */
/* Native surface resource shutdown. */
/*
 * @original Gfx_SurfaceShutdownNative (IGN_WIN.EXE @ 0x0045C060, inferred geputget.c)
 * @fidelity EXACT
 */
int Gfx_SurfaceShutdownNative(void) {
    LPDIRECTDRAWCLIPPER clipper;
    LPDIRECTDRAWSURFACE surface;
    LPDIRECTDRAWPALETTE palette;
    LPDIRECTDRAW draw;
    volatile GfxSurfaceRecord *record;
    int index;

    if (g_nativeFullscreen == 0 && g_nativeClipper != 0) {
        clipper = g_nativeClipper;
        clipper->lpVtbl->Release(clipper);
    }
    record = &g_nativePrimarySurface;
    if (g_nativeFullscreen == 0) {
        surface = record->surface;
        if (surface != 0) {
            surface->lpVtbl->Release(surface);
            record->surface = 0;
            record->active = 0;
        }
    }
    for (index = 0; index < 5; index++) {
        record = &g_nativeType1Surfaces[index];
        if (g_nativeFullscreen == 0) {
            surface = record->surface;
            if (surface != 0) {
                surface->lpVtbl->Release(surface);
                record->surface = 0;
                record->active = 0;
            }
        }
    }
    for (index = 0; index < 20; index++) {
        record = &g_nativeType2Surfaces[index];
        surface = record->surface;
        if (surface != 0) {
            surface->lpVtbl->Release(surface);
            record->surface = 0;
            record->active = 0;
        }
    }
    if (g_nativeFullscreen != 0 && g_nativePalette != 0) {
        palette = g_nativePalette;
        palette->lpVtbl->Release(palette);
        g_nativePalette = 0;
    }
    if (g_nativeDirectDraw != 0) {
        draw = g_nativeDirectDraw;
        draw->lpVtbl->Release(draw);
        g_nativeDirectDraw = 0;
    }
    return 1;
}

/* End native surface resource shutdown. */
/* Native surface rebuild and its independently recovered palette backing. */
volatile PALETTEENTRY g_nativeRebuildPaletteEntries[256]; /* VA 0x00512850 */
/*
 * @original Gfx_SurfaceRebuildNative (IGN_WIN.EXE @ 0x0045BD70, inferred geputget.c)
 * @fidelity EXACT
 */
int Gfx_SurfaceRebuildNative(void) {
    LPDIRECTDRAWSURFACE surface;
    LPDIRECTDRAWPALETTE palette;
    LPDIRECTDRAW draw;
    volatile GfxSurfaceRecord *record;
    DDSURFACEDESC descriptor;
    DDSCAPS caps;
    int width;
    int height;
    int depth;
    int index;
    int limit;

    record = &g_nativePrimarySurface;
    if (g_nativeFullscreen != 0) {
        surface = record->surface;
        if (surface != 0) {
            surface->lpVtbl->Release(surface);
            record->surface = 0;
            record->active = 0;
        }
    }
    for (index = 0; index < 20; index++) {
        record = &g_nativeType2Surfaces[index];
        surface = record->surface;
        if (surface != 0) {
            surface->lpVtbl->Release(surface);
            record->surface = 0;
            record->active = 0;
        }
    }
    if (g_nativeFullscreen != 0 && g_nativePalette != 0) {
        palette = g_nativePalette;
        palette->lpVtbl->Release(palette);
        g_nativePalette = 0;
    }
    Gfx_InitSurfaceRecords();
    if (g_nativeFullscreen != 0) {
        depth = g_nativeSurfaceBitDepth;
        height = g_nativeSurfaceHeight;
        width = g_nativeSurfaceWidth;
        draw = g_nativeDirectDraw;
        if (draw->lpVtbl->SetDisplayMode(draw, width, height, depth) != DD_OK) {
            return 0;
        }
    }
    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.dwSize = sizeof(descriptor);
    if (g_nativeFullscreen != 0) {
        descriptor.dwBackBufferCount = g_nativeBackbufferCount;
        descriptor.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
        descriptor.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
        draw = g_nativeDirectDraw;
        if (draw->lpVtbl->CreateSurface(draw, &descriptor,
                (LPDIRECTDRAWSURFACE *)&g_nativePrimarySurface.surface, 0) != DD_OK) {
            return 0;
        }
        width = g_nativeSurfaceWidth;
        height = g_nativeSurfaceHeight;
        g_nativePrimarySurface.active = 1;
        g_nativePrimarySurface.restore_marker = 1;
        g_nativePrimarySurface.width = width;
        depth = g_nativeSurfaceBitDepth;
        g_nativePrimarySurface.height = height;
        limit = g_nativeBackbufferCount;
        g_nativePrimarySurface.bit_depth = depth;
        if (limit >= 5) {
            MessageBoxA(g_nativeWindow, g_nativeBackbufferLimitMessage, 0, 0);
            return 0;
        }
        if (g_nativeBackbufferCount > 0) {
            surface = g_nativePrimarySurface.surface;
            caps.dwCaps = DDSCAPS_BACKBUFFER;
            index = 0;
            do {
                record = &g_nativeType1Surfaces[index];
                if (index != 0) {
                    caps.dwCaps = DDSCAPS_FLIP;
                }
                if (surface->lpVtbl->GetAttachedSurface(surface, &caps, &surface) != DD_OK) {
                    MessageBoxA(g_nativeWindow, g_nativeBackbufferMessage, 0, 0);
                    return 0;
                }
                width = g_nativeSurfaceWidth;
                record->surface = surface;
                height = g_nativeSurfaceHeight;
                record->active = 1;
                depth = g_nativeSurfaceBitDepth;
                record->restore_marker = 1;
                record->width = width;
                record->height = height;
                index++;
                limit = g_nativeBackbufferCount;
                record->bit_depth = depth;
            } while (index < limit);
        }
    }
    descriptor.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    descriptor.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    if (g_nativeFullscreen != 0) {
        if (g_nativePalette != 0) {
            palette = g_nativePalette;
            palette->lpVtbl->Release(palette);
            g_nativePalette = 0;
        }
        g_nativeRebuildPaletteEntries[0].peRed = 0;
        g_nativeRebuildPaletteEntries[0].peGreen = 0;
        g_nativeRebuildPaletteEntries[0].peBlue = 0;
        for (index = 1; index < 256; index++) {
            g_nativeRebuildPaletteEntries[index].peRed = 255;
            g_nativeRebuildPaletteEntries[index].peGreen = 255;
            g_nativeRebuildPaletteEntries[index].peBlue = 255;
        }
        draw = g_nativeDirectDraw;
        if (draw->lpVtbl->CreatePalette(draw, DDPCAPS_8BIT | DDPCAPS_ALLOW256,
                (LPPALETTEENTRY)g_nativeRebuildPaletteEntries,
                (LPDIRECTDRAWPALETTE *)&g_nativePalette, 0) != DD_OK) {
            return 0;
        }
        palette = g_nativePalette;
        surface = g_nativePrimarySurface.surface;
        if (surface->lpVtbl->SetPalette(surface, palette) != DD_OK) {
            return 0;
        }
    }
    Gfx_SurfaceRestoreNative();
    ShowWindow(g_nativeWindow, SW_SHOW);
    return 1;
}

/* End native surface rebuild. */
/* Native surface access implementation. */
/* @original Gfx_SurfaceLockNative (IGN_WIN.EXE @ 0x0045C530, inferred surface backend)
 * @fidelity EXACT */
int Gfx_SurfaceLockNative(GfxSurfaceRecord *surface, int mode) {
    volatile GfxSurfaceRecord *record;
    LPDIRECTDRAWSURFACE object;
    DDSURFACEDESC descriptor;
    HRESULT result;
    unsigned char *pixels;

    record = surface;
    if (record->active == 0) {
        return 0;
    }
    if (record->locked == 1) {
        if (record->lock_mode == mode) {
            return 1;
        }
        object = record->surface;
        result = object->lpVtbl->Unlock(object, (void *)&record->surface);
        if (result != 0) {
            return 0;
        }
        record->lock_mode = 0;
    }
    object = record->surface;
    result = object->lpVtbl->IsLost(object);
    if ((uint32_t)result == 0x887601c2u) {
        object = record->surface;
        object->lpVtbl->Restore(object);
        record->restore_marker = 1;
    }
    descriptor.dwSize = 108;
    /* The original stores lpSurface before checking HRESULT. A failed API
     * call that leaves this output unwritten exposes uninitialized data. */
    if (mode == 1) {
        object = record->surface;
        result = object->lpVtbl->Lock(object, 0, &descriptor, DDLOCK_WAIT | DDLOCK_READONLY, 0);
        pixels = (unsigned char *)descriptor.lpSurface;
        record->read_pixels = pixels;
        record->write_pixels = 0;
    } else if (mode == 2) {
        object = record->surface;
        result = object->lpVtbl->Lock(object, 0, &descriptor, DDLOCK_WAIT, 0);
        pixels = (unsigned char *)descriptor.lpSurface;
        record->read_pixels = pixels;
        record->write_pixels = pixels;
    } else if (mode == 3) {
        object = record->surface;
        result = object->lpVtbl->Lock(object, 0, &descriptor, DDLOCK_WAIT | DDLOCK_WRITEONLY, 0);
        pixels = (unsigned char *)descriptor.lpSurface;
        record->read_pixels = 0;
        record->write_pixels = pixels;
    } else {
        return 0;
    }
    if (result != 0) {
        record->read_pixels = 0;
        record->write_pixels = 0;
        record->locked = 0;
        return 0;
    }
    record->lock_mode = mode;
    record->pitch_bytes = descriptor.lPitch;
    record->locked = 1;
    return 1;
}

/* @original Gfx_SurfaceUnlockNative (IGN_WIN.EXE @ 0x0045C680, inferred surface backend)
 * @fidelity EXACT */
int Gfx_SurfaceUnlockNative(GfxSurfaceRecord *surface) {
    volatile GfxSurfaceRecord *record;
    LPDIRECTDRAWSURFACE object;

    record = surface;
    if (record->active == 0) {
        return 0;
    }
    if (record->locked == 0) {
        return 1;
    }
    object = record->surface;
    if (object->lpVtbl->Unlock(object, (void *)&record->surface) != 0) {
        return 0;
    }
    record->locked = 0;
    record->lock_mode = 0;
    return 1;
}

/* @original Gfx_SurfaceClearNative (IGN_WIN.EXE @ 0x0045C3D0, inferred surface backend)
 * @fidelity EXACT */
int Gfx_SurfaceClearNative(GfxSurfaceRecord *surface) {
    volatile GfxSurfaceRecord *record;
    volatile uint32_t *pixels;
    int32_t old_mode, row, column;
    uint32_t offset;

    record = surface;
    if (record->active == 0) {
        return 0;
    }
    old_mode = record->lock_mode;
    if (old_mode == 0) {
        if (!Gfx_SurfaceLockNative(surface, 3)) {
            return 0;
        }
    } else if (old_mode == 1) {
        if (!Gfx_SurfaceUnlockNative(surface) || !Gfx_SurfaceLockNative(surface, 3)) {
            return 0;
        }
    }
    pixels = (volatile uint32_t *)record->write_pixels;
    for (row = 0; row < (int32_t)record->height; ++row) {
        for (column = 0; column < (int32_t)record->width; ++column) {
            /* Authentic code uses SDK byte pitch as a DWORD index. */
            offset = (uint32_t)record->pitch_bytes * (uint32_t)row + (uint32_t)column;
            pixels[offset] = 0;
        }
    }
    if (record->lock_mode != old_mode) {
        if (!Gfx_SurfaceUnlockNative(surface)) {
            return 0;
        }
        if (old_mode != 0 && !Gfx_SurfaceLockNative(surface, old_mode)) {
            return 0;
        }
    }
    return 1;
}

/* @original Gfx_SurfacePresentNative (IGN_WIN.EXE @ 0x0045C4B0, inferred surface backend)
 * @fidelity EXACT */
int Gfx_SurfacePresentNative(GfxSurfaceRecord *surface) {
    volatile GfxSurfaceRecord *record;
    LPDIRECTDRAWSURFACE target, primary;
    DDSCAPS caps;
    HRESULT result;

    record = surface;
    if (record->storage_kind != 1) {
        return 0;
    }
    if (record->active == 0) {
        return 0;
    }
    target = record->surface;
    target->lpVtbl->GetCaps(target, &caps);
    /* The original ignores GetCaps HRESULT and reads its output regardless. */
    if ((caps.dwCaps & DDSCAPS_FLIP) == 0) {
        return 0;
    }
    do {
        target = record->surface;
        primary = g_nativePrimarySurface.surface;
        result = primary->lpVtbl->Flip(primary, target, 0);
    } while ((uint32_t)result == 0x8876021cu);
    return result == 0;
}

volatile PALETTEENTRY g_nativeSetPaletteEntries[256]; /* VA 0x00512450, separate from constructor/rebuild */

/* @original Gfx_SurfaceSetPaletteNative (IGN_WIN.EXE @ 0x0045C6D0, inferred surface backend)
 * @fidelity EXACT */
int Gfx_SurfaceSetPaletteNative(const unsigned char *rgb) {
    const volatile unsigned char *bytes;
    LPDIRECTDRAWPALETTE palette;
    uint32_t index;

    if (g_nativePalette == 0) {
        return 0;
    }
    bytes = rgb;
    for (index = 0; index < 256; ++index) {
        g_nativeSetPaletteEntries[index].peRed = bytes[3 * index];
        g_nativeSetPaletteEntries[index].peGreen = bytes[3 * index + 1];
        g_nativeSetPaletteEntries[index].peBlue = bytes[3 * index + 2];
    }
    palette = g_nativePalette;
    return palette->lpVtbl->SetEntries(palette, 0, 0, 256,
            (LPPALETTEENTRY)g_nativeSetPaletteEntries) == DD_OK;
}

/* End native surface access implementation. */

/* @original Gfx_SurfaceBlitNative (IGN_WIN.EXE @ 0x0045C160, unknown original source)
 * @fidelity EXACT
 */
int Gfx_SurfaceBlitNative(GfxSurfaceRecord *source, int x, int y,
        int width, int height, GfxSurfaceRecord *destination,
        int destination_x, int destination_y) {
    volatile GfxSurfaceRecord *source_record;
    volatile GfxSurfaceRecord *destination_record;
    LPDIRECTDRAWSURFACE target;
    LPDIRECTDRAWSURFACE input;
    RECT rectangle;

    source_record = source;
    destination_record = destination;
    if (source_record->active == 0) {
        return 0;
    }
    if (destination_record->active == 0) {
        return 0;
    }
    rectangle.left = x;
    rectangle.top = y;
    rectangle.right = (LONG)((uint32_t)x + (uint32_t)width);
    target = destination_record->surface;
    rectangle.bottom = (LONG)((uint32_t)y + (uint32_t)height);
    input = source_record->surface;
    return target->lpVtbl->BltFast(target, (DWORD)destination_x,
            (DWORD)destination_y, input, &rectangle, DDBLTFAST_WAIT) == DD_OK;
}

/* @original Gfx_SurfaceBlitDispatch (IGN_WIN.EXE @ 0x00456B30, unknown original source)
 * @fidelity EXACT
 */
int Gfx_SurfaceBlitDispatch(GfxSurfaceRecord *source, int x, int y,
        int width, int height, GfxSurfaceRecord *destination,
        int destination_x, int destination_y) {
    return g_surfaceBlit(source, x, y, width, height, destination,
            destination_x, destination_y);
}

/* @original Gfx_SurfaceCopyPixelsNative (IGN_WIN.EXE @ 0x0045C1D0, unknown original source)
 * @fidelity EXACT
 */
int Gfx_SurfaceCopyPixelsNative(const unsigned char *pixels, int stride,
        int source_x, int source_y, int width, int height,
        GfxSurfaceRecord *destination, int destination_x, int destination_y) {
    volatile GfxSurfaceRecord *record;
    volatile const uint32_t *source_words;
    volatile uint32_t *destination_words;
    int32_t original_mode;
    uint32_t source_address, destination_address, pitch;
    uint32_t source_cursor, destination_cursor;
    uint32_t groups, groups_left, rows_left, count;
    uint32_t leading, trailing, middle, index;

    record = destination;
    if (record->active == 0) {
        return 0;
    }
    original_mode = record->lock_mode;
    if (original_mode == 0) {
        if (!Gfx_SurfaceLockNative(destination, 3)) {
            return 0;
        }
    } else if (original_mode == 1) {
        if (!Gfx_SurfaceUnlockNative(destination) ||
                !Gfx_SurfaceLockNative(destination, 3)) {
            return 0;
        }
    }
    source_address = (uint32_t)(uintptr_t)pixels +
            (uint32_t)stride * (uint32_t)source_y + (uint32_t)source_x;
    pitch = (uint32_t)record->pitch_bytes;
    destination_address = (uint32_t)(uintptr_t)record->write_pixels +
            pitch * (uint32_t)destination_y + (uint32_t)destination_x;
    if ((source_address & 3) == 0 && ((uint32_t)stride & 3) == 0 &&
            width % 160 == 0) {
        groups = (uint32_t)(width / 160);
        rows_left = (uint32_t)height;
        source_cursor = source_address;
        destination_cursor = destination_address;
        /* Authentic unchecked do/while counts include zero/negative inputs. */
        do {
            groups_left = groups;
            do {
                source_words = (volatile const uint32_t *)(uintptr_t)source_cursor;
                destination_words = (volatile uint32_t *)(uintptr_t)destination_cursor;
                /* Preserve the five original prefetch reads and their faults. */
                (void)source_words[0];
                (void)source_words[8];
                (void)source_words[16];
                (void)source_words[24];
                (void)source_words[32];
                for (index = 0; index < 40; index++) {
                    destination_words[index] = source_words[index];
                }
                source_cursor += 160;
                destination_cursor += 160;
            } while (--groups_left != 0);
            source_cursor += (uint32_t)stride - (uint32_t)width;
            destination_cursor += pitch - (uint32_t)width;
        } while (--rows_left != 0);
    } else {
        leading = (0u - destination_address) & 3;
        trailing = (destination_address + (uint32_t)width) & 3;
        middle = (uint32_t)((int32_t)((uint32_t)width - leading - trailing) / 4);
        if (height > 0) {
            rows_left = (uint32_t)height;
            do {
                source_cursor = source_address;
                destination_cursor = destination_address;
                count = leading;
                while (count != 0) {
                    *(volatile unsigned char *)(uintptr_t)destination_cursor =
                            *(volatile const unsigned char *)(uintptr_t)source_cursor;
                    source_cursor++;
                    destination_cursor++;
                    count--;
                }
                count = middle;
                while (count != 0) {
                    *(volatile uint32_t *)(uintptr_t)destination_cursor =
                            *(volatile const uint32_t *)(uintptr_t)source_cursor;
                    source_cursor += 4;
                    destination_cursor += 4;
                    count--;
                }
                count = trailing;
                while (count != 0) {
                    *(volatile unsigned char *)(uintptr_t)destination_cursor =
                            *(volatile const unsigned char *)(uintptr_t)source_cursor;
                    source_cursor++;
                    destination_cursor++;
                    count--;
                }
                source_address += (uint32_t)stride;
                destination_address += pitch;
            } while (--rows_left != 0);
        }
    }
    if (record->lock_mode != original_mode) {
        if (!Gfx_SurfaceUnlockNative(destination)) {
            return 0;
        }
        if (original_mode != 0 && !Gfx_SurfaceLockNative(destination, original_mode)) {
            return 0;
        }
    }
    return 1;
}

/* @original Gfx_SurfaceCopyPixelsDispatch (IGN_WIN.EXE @ 0x00456B70, unknown original source)
 * @fidelity EXACT
 */
int Gfx_SurfaceCopyPixelsDispatch(const unsigned char *pixels, int stride,
        int source_x, int source_y, int width, int height,
        GfxSurfaceRecord *destination, int destination_x, int destination_y) {
    return g_surfaceCopyPixels(pixels, stride, source_x, source_y, width, height,
            destination, destination_x, destination_y);
}
