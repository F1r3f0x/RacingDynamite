#ifndef SURFACE_H
#define SURFACE_H

#include <stdint.h>

/* Windows surface dispatch. Record fields are recovered per verified consumer.
 * Stub option words retain unknown semantics/signedness. All calls use the x86
 * ordinary C ABI; no-argument RET alone does not distinguish cdecl/stdcall. */
/* Verified 48-byte native record banks and access fields; surface is a COM object. */
struct IDirectDrawSurface;
typedef struct GfxSurfaceRecord {
    int32_t active;
    uint32_t restore_marker;
    uint32_t locked;
    int32_t lock_mode;
    const unsigned char *read_pixels;
    unsigned char *write_pixels;
    int32_t pitch_bytes;
    uint32_t width;
    uint32_t height;
    uint32_t bit_depth;
    uint32_t storage_kind;
    struct IDirectDrawSurface *surface;
} GfxSurfaceRecord;
extern volatile GfxSurfaceRecord g_nativePrimarySurface; /* VA 0x0050E778 */
extern volatile GfxSurfaceRecord g_nativeType1Surfaces[5]; /* VA 0x0050E688 */
extern volatile GfxSurfaceRecord g_nativeType2Surfaces[20]; /* VA 0x0050E7A8 */
int Gfx_InitSurfaceRecords(void); /* VA 0x00456A40; EAX=1 */
typedef int (*GfxSurfaceConfigureProc)(unsigned int option0, unsigned int option1, unsigned int option2, unsigned int option3);
extern GfxSurfaceConfigureProc volatile g_surfaceConfigure; /* VA 0x0050EB68 */
int Gfx_SurfaceConfigureNative(unsigned int option0, unsigned int option1, unsigned int option2, unsigned int option3); /* VA 0x0045B730 */
typedef int (*GfxSurfaceOpenProc)(void);
extern GfxSurfaceOpenProc volatile g_surfaceOpen; /* VA 0x0050EB6C */
int Gfx_SurfaceOpenNative(void); /* VA 0x0045B740 */
typedef int (*GfxSurfaceRebuildProc)(void);
extern GfxSurfaceRebuildProc volatile g_surfaceRebuild; /* VA 0x0050EB70 */
int Gfx_SurfaceRebuildNative(void); /* VA 0x0045BD70; rebuilds native surfaces */
typedef int (*GfxSurfaceShutdownProc)(void);
extern GfxSurfaceShutdownProc volatile g_surfaceShutdown; /* VA 0x0050EB74 */
int Gfx_SurfaceShutdownNative(void); /* VA 0x0045C060; resource shutdown, clipper pointer retained */
typedef int (*GfxSurfaceConfigureSurfaceProc)(unsigned int option0, unsigned int option1, unsigned int option2, unsigned int option3, unsigned int option4);
extern GfxSurfaceConfigureSurfaceProc volatile g_surfaceConfigureSurface; /* VA 0x0050EB78 */
int Gfx_SurfaceConfigureSurfaceNative(unsigned int option0, unsigned int option1, unsigned int option2, unsigned int option3, unsigned int option4); /* VA 0x0045C150; body unreconstructed */
typedef int (*GfxSurfaceBlitProc)(GfxSurfaceRecord *source, int x, int y, int width, int height, GfxSurfaceRecord *destination, int destination_y, int destination_x);
extern GfxSurfaceBlitProc volatile g_surfaceBlit; /* VA 0x0050EB7C */
int Gfx_SurfaceBlitNative(GfxSurfaceRecord *source, int x, int y, int width, int height, GfxSurfaceRecord *destination, int destination_y, int destination_x); /* VA 0x0045C160; body unreconstructed */
typedef int (*GfxSurfaceCopyPixelsProc)(const unsigned char *pixels, int stride, int source_x, int source_y, int width, int height, GfxSurfaceRecord *destination, int destination_x, int destination_y);
extern GfxSurfaceCopyPixelsProc volatile g_surfaceCopyPixels; /* VA 0x0050EB80 */
int Gfx_SurfaceCopyPixelsNative(const unsigned char *pixels, int stride, int source_x, int source_y, int width, int height, GfxSurfaceRecord *destination, int destination_x, int destination_y); /* VA 0x0045C1D0; body unreconstructed */
typedef int (*GfxSurfaceClearProc)(GfxSurfaceRecord *surface);
extern GfxSurfaceClearProc volatile g_surfaceClear; /* VA 0x0050EB84 */
int Gfx_SurfaceClearNative(GfxSurfaceRecord *surface); /* VA 0x0045C3D0 */
int Gfx_ClearSurfaceNative(GfxSurfaceRecord *surface); /* VA 0x00456BB0, installed-table entry */
typedef int (*GfxSurfacePresentProc)(GfxSurfaceRecord *surface);
extern GfxSurfacePresentProc volatile g_surfacePresent; /* VA 0x0050EB88 */
int Gfx_SurfacePresentNative(GfxSurfaceRecord *surface); /* VA 0x0045C4B0 */
int Gfx_PresentSurfaceNative(GfxSurfaceRecord *surface); /* VA 0x00456C90, installed-table entry */
typedef int (*GfxSurfaceReservedProc)(void);
extern GfxSurfaceReservedProc volatile g_surfaceReserved; /* VA 0x0050EB8C */
int Gfx_SurfaceReservedNative(void); /* VA 0x0045C520; body unreconstructed */
typedef int (*GfxSurfaceLockProc)(GfxSurfaceRecord *surface, int mode);
extern GfxSurfaceLockProc volatile g_surfaceLock; /* VA 0x0050EB90 */
int Gfx_SurfaceLockNative(GfxSurfaceRecord *surface, int mode); /* VA 0x0045C530 */
typedef int (*GfxSurfaceUnlockProc)(GfxSurfaceRecord *surface);
extern GfxSurfaceUnlockProc volatile g_surfaceUnlock; /* VA 0x0050EB94 */
int Gfx_SurfaceUnlockNative(GfxSurfaceRecord *surface); /* VA 0x0045C680 */
typedef int (*GfxSurfaceSetPaletteProc)(const unsigned char *rgb);
extern GfxSurfaceSetPaletteProc volatile g_surfaceSetPalette; /* VA 0x0050EB98 */
int Gfx_SurfaceSetPaletteNative(const unsigned char *rgb); /* VA 0x0045C6D0 */
int Gfx_SetPaletteNative(const unsigned char *rgb); /* VA 0x00456C40, installed-table entry */
typedef int (*GfxSurfaceRestoreProc)(void);
extern GfxSurfaceRestoreProc volatile g_surfaceRestore; /* VA 0x0050EB9C */
int Gfx_SurfaceRestoreNative(void); /* VA 0x0045C730 */

int Gfx_InstallSurfaceDispatch(void); /* VA 0x0045B690 */

#endif
