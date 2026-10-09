#ifndef GEPUTGET_H
#define GEPUTGET_H

#include <stdint.h>
#include <stddef.h>
#include "mem.h"

/* Windows input callback ABI; semantic names, original symbols unknown. */
typedef void (*InputKeyEventCallback)(int pressed, int scan_code);
typedef void (*InputPollCallback)(void);
extern InputKeyEventCallback volatile g_inputKeyEventCallback;
extern InputPollCallback volatile g_inputPollCallback;
void Input_ResetCallbacks(void);

/* Windows selector: one caller-cleanup stack dword; zero installs both tables.
 * Both dispatch installers are reconstructed; downstream initialization is modeled. */
int Gfx_SelectBackend(int backend);
int Gfx_InstallSurfaceDispatch(void); /* VA 0x0045B690 */
int Gfx_InstallSpriteDispatch(void);  /* VA 0x00456E60 */

/* Windows surface dispatch. Records remain opaque; only consumer ABI is exposed.
 * Stub option words retain unknown semantics/signedness. All calls use the x86
 * ordinary C ABI; no-argument RET alone does not distinguish cdecl/stdcall. */
typedef struct GfxSurfaceRecord GfxSurfaceRecord;
typedef int (*GfxSurfaceConfigureProc)(unsigned int option0, unsigned int option1, unsigned int option2, unsigned int option3);
extern GfxSurfaceConfigureProc volatile g_surfaceConfigure; /* VA 0x0050EB68 */
int Gfx_SurfaceConfigureNative(unsigned int option0, unsigned int option1, unsigned int option2, unsigned int option3); /* VA 0x0045B730; body unreconstructed */
typedef int (*GfxSurfaceOpenProc)(void);
extern GfxSurfaceOpenProc volatile g_surfaceOpen; /* VA 0x0050EB6C */
int Gfx_SurfaceOpenNative(void); /* VA 0x0045B740; body unreconstructed */
typedef int (*GfxSurfaceCloseProc)(void);
extern GfxSurfaceCloseProc volatile g_surfaceClose; /* VA 0x0050EB70 */
int Gfx_SurfaceCloseNative(void); /* VA 0x0045BD70; body unreconstructed */
typedef int (*GfxSurfaceResetProc)(void);
extern GfxSurfaceResetProc volatile g_surfaceReset; /* VA 0x0050EB74 */
int Gfx_SurfaceResetNative(void); /* VA 0x0045C060; body unreconstructed */
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
int Gfx_SurfaceClearNative(GfxSurfaceRecord *surface); /* VA 0x0045C3D0; body unreconstructed */
typedef int (*GfxSurfacePresentProc)(GfxSurfaceRecord *surface);
extern GfxSurfacePresentProc volatile g_surfacePresent; /* VA 0x0050EB88 */
int Gfx_SurfacePresentNative(GfxSurfaceRecord *surface); /* VA 0x0045C4B0; body unreconstructed */
typedef int (*GfxSurfaceReservedProc)(void);
extern GfxSurfaceReservedProc volatile g_surfaceReserved; /* VA 0x0050EB8C */
int Gfx_SurfaceReservedNative(void); /* VA 0x0045C520; body unreconstructed */
typedef int (*GfxSurfaceLockProc)(GfxSurfaceRecord *surface, int mode);
extern GfxSurfaceLockProc volatile g_surfaceLock; /* VA 0x0050EB90 */
int Gfx_SurfaceLockNative(GfxSurfaceRecord *surface, int mode); /* VA 0x0045C530; body unreconstructed */
typedef int (*GfxSurfaceUnlockProc)(GfxSurfaceRecord *surface);
extern GfxSurfaceUnlockProc volatile g_surfaceUnlock; /* VA 0x0050EB94 */
int Gfx_SurfaceUnlockNative(GfxSurfaceRecord *surface); /* VA 0x0045C680; body unreconstructed */
typedef int (*GfxSurfaceSetPaletteProc)(const unsigned char *rgb);
extern GfxSurfaceSetPaletteProc volatile g_surfaceSetPalette; /* VA 0x0050EB98 */
int Gfx_SurfaceSetPaletteNative(const unsigned char *rgb); /* VA 0x0045C6D0; body unreconstructed */
typedef int (*GfxSurfaceRestoreProc)(void);
extern GfxSurfaceRestoreProc volatile g_surfaceRestore; /* VA 0x0050EB9C */
int Gfx_SurfaceRestoreNative(void); /* VA 0x0045C730; body unreconstructed */

/* Windows sprite dispatch. State remains opaque; descriptor copy footprint is verified.
 * Raw no-op option words retain unknown semantics/signedness. */
typedef struct {
    int32_t x;
    int32_t y;
} Point2D;

/* IGN_WIN.EXE native sprite request: only the consumed handle prefix is known.
 * Point/origin coordinates use eight fractional bits. No ownership transfer. */
typedef struct {
    int32_t image_id;
    int32_t origin_x;
    int32_t origin_y;
} GfxSpriteHandle;

typedef struct {
    float scale;
    float angle_radians;
} GfxSpriteTransform;

/* Verified 64-byte copy footprint, not a complete semantic field layout.
 * Words 6/7 are ownership-related slots cleared only for a live-record copy.
 * Words retain bit patterns, including pointer representations. */
typedef struct GfxSpriteDescriptor {
    uint32_t words[16];
} GfxSpriteDescriptor;

/* Independently verified pixel-copy prefix; the other descriptor words remain
 * opaque. Pointer and stride representations are unsigned 32-bit words. */
typedef struct GfxSpritePixelView {
    uint32_t opaque0;
    int32_t width;
    int32_t height;
    uint32_t opaque3;
    uint32_t pixels;
    uint32_t stride;
} GfxSpritePixelView;

/* Incidental EAX retains source; both arguments use caller cleanup. */
const GfxSpriteDescriptor *Gfx_CopySpriteDescriptorPixels(
    const GfxSpriteDescriptor *source, const GfxSpriteDescriptor *destination); /* VA 0x004612A0 */

#define GFX_SPRITE_HANDLE_COUNT 2000
extern GfxSpriteHandle *volatile g_nativeSpriteFreeList[GFX_SPRITE_HANDLE_COUNT]; /* VA 0x00512C58 */
extern volatile GfxSpriteHandle g_nativeSpriteHandles[GFX_SPRITE_HANDLE_COUNT]; /* VA 0x00514BD8 */
extern GfxSpriteHandle *volatile *volatile g_nativeSpriteFreeCursor; /* VA 0x0051A998 */
extern volatile GfxSpriteDescriptor g_nativeSpriteScratch; /* VA 0x00514B98 */
extern volatile GfxSpriteDescriptor g_nativeSpriteDefault; /* VA 0x0051FB88 */
extern const char g_nativeSpriteDefaultName[8]; /* VA 0x004BACF8; includes NUL */
extern volatile uint32_t g_nativeNextImageId; /* VA 0x00520380; raw search word */
int Gfx_InitDefaultSpriteDescriptor(void); /* VA 0x004611D0; EAX=0 */
/* Verified six-dword packing-node copy view. Range words retain raw bits;
 * consumers establish pixel/child/previous/next links, not allocator fidelity. */
typedef struct GfxSpritePackingNode {
    uint32_t range_start;
    uint32_t range_end;
    unsigned char *pixels;
    struct GfxSpritePackingNode *children;
    struct GfxSpritePackingNode *previous;
    struct GfxSpritePackingNode *next;
} GfxSpritePackingNode;
/* Eight-byte Windows bucket: next list entry, then its packing node. */
typedef struct GfxSpritePackingBucket {
    struct GfxSpritePackingBucket *next;
    GfxSpritePackingNode *node;
} GfxSpritePackingBucket;
#define GFX_SPRITE_PACKING_BUCKET_COUNT 257
extern volatile GfxSpritePackingNode g_spritePackingTemplate; /* VA 0x0051FC00 */
extern GfxSpritePackingBucket *volatile g_spritePackingBuckets[GFX_SPRITE_PACKING_BUCKET_COUNT]; /* VA 0x0051FF58 */
extern GfxSpritePackingNode *volatile g_spritePackingPages; /* VA 0x0051FE40 */
/* Native graphics allocation: zero bytes bypass CRT malloc. */
void *Gfx_AllocBytes(uint32_t size); /* VA 0x0045F4A0 */
/* Unsigned 32-bit size/alignment arithmetic; back-pointer precedes result.
 * No zero-alignment or allocation-failure guard exists in the original. */
unsigned char *Gfx_AllocAlignedBytes(uint32_t size, uint32_t alignment); /* VA 0x00461840 */
/* Incidental EAX: pixel allocation or live head; callers discard it. */
void *Gfx_AddSpritePackingPage(void); /* VA 0x004617E0 */
/* Signed request/comparisons with wrapping 32-bit gap subtraction.
 * Returns (node *)0xFFFFFFFF for empty/prefix space, predecessor for a gap,
 * or null when exhausted. Does not validate ranges, pointers or cycles. */
GfxSpritePackingNode *Gfx_FindSpritePackingGap(int32_t size,
    GfxSpritePackingNode *first); /* VA 0x00461870 */
/* Signed extent, initial page head; returns the published bucket (incidental
 * EAX, discarded by the sole direct caller). No validation or failure guards. */
GfxSpritePackingBucket *Gfx_AddSpritePackingBucket(int32_t size,
    GfxSpritePackingNode *first_page); /* VA 0x004616C0 */
int Gfx_InitSpritePackingState(void); /* VA 0x004614D0; EAX=0 */
/* Returns the unchanged previous argument (incidental original EAX).
 * Current must be writable; exact node aliases retain ordered stores. */
GfxSpritePackingNode *Gfx_LinkSpritePackingNode(GfxSpritePackingNode *previous,
    GfxSpritePackingNode *current, GfxSpritePackingNode *next); /* VA 0x00461690 */
extern volatile int32_t g_nativeImageCapacity; /* VA 0x00520388; signed */
extern GfxSpriteDescriptor *volatile *volatile g_nativeImageRecords; /* VA 0x0052038C */
int Gfx_CopySpriteDescriptor(GfxSpriteDescriptor *descriptor, int image_id); /* VA 0x004612E0 */
typedef struct GfxSpriteState GfxSpriteState;
typedef int (*GfxSpriteOpenProc)(void);
extern GfxSpriteOpenProc volatile g_spriteOpen; /* VA 0x0050EBA0 */
int Gfx_SpriteOpenNative(void); /* VA 0x00456F20; body unreconstructed */
typedef int (*GfxSpriteResetProc)(void);
extern GfxSpriteResetProc volatile g_spriteReset; /* VA 0x0050EBA4 */
int Gfx_SpriteResetNative(void); /* VA 0x00456F30; body unreconstructed */
typedef int (*GfxSpriteCloseProc)(void);
extern GfxSpriteCloseProc volatile g_spriteClose; /* VA 0x0050EBA8 */
int Gfx_SpriteCloseNative(void); /* VA 0x00456F40; body unreconstructed */
typedef int (*GfxSpriteOptionProc)(unsigned int option);
extern GfxSpriteOptionProc volatile g_spriteOption; /* VA 0x0050EBAC */
int Gfx_SpriteOptionNative(unsigned int option); /* VA 0x00456F50; body unreconstructed */
typedef int (*GfxSpriteConfigureProc)(unsigned char *pixels, int stride, int width, int height, unsigned int option);
extern GfxSpriteConfigureProc volatile g_spriteConfigure; /* VA 0x0050EBB0 */
int Gfx_SpriteConfigureNative(unsigned char *pixels, int stride, int width, int height, unsigned int option); /* VA 0x00456F60; body unreconstructed */
typedef int (*GfxSpriteSetClipProc)(int left, int top, int right, int bottom);
extern GfxSpriteSetClipProc volatile g_spriteSetClip; /* VA 0x0050EBB4 */
int Gfx_SpriteSetClipNative(int left, int top, int right, int bottom); /* VA 0x00457020; body unreconstructed */
typedef int (*GfxSpriteDrawListProc)(const unsigned int *list);
extern GfxSpriteDrawListProc volatile g_spriteDrawList; /* VA 0x0050EBB8 */
int Gfx_SpriteDrawListNative(const unsigned int *list); /* VA 0x00457250; body unreconstructed */
typedef int (*GfxSpriteDrawProc)(GfxSpriteHandle *handle, Point2D *position, const GfxSpriteTransform *transform);
extern GfxSpriteDrawProc volatile g_spriteDraw; /* VA 0x0050EBBC */
int Gfx_DrawSpriteNative(GfxSpriteHandle *handle, Point2D *position, const GfxSpriteTransform *transform); /* VA 0x004571B0; separately reconstructed */
typedef GfxSpriteHandle *(*GfxSpriteHandleOpProc)(const GfxSpriteDescriptor *descriptor, GfxSpriteHandle *handle);
extern GfxSpriteHandleOpProc volatile g_spriteHandleOp; /* VA 0x0050EBC0 */
GfxSpriteHandle *Gfx_SpriteHandleOpNative(const GfxSpriteDescriptor *descriptor, GfxSpriteHandle *handle); /* VA 0x0045C830; body unreconstructed */
typedef int (*GfxSpriteImageOpProc)(const GfxSpriteDescriptor *descriptor, int image_id);
extern GfxSpriteImageOpProc volatile g_spriteImageOp; /* VA 0x0050EBD8 */
int Gfx_SpriteImageOpNative(const GfxSpriteDescriptor *descriptor, int image_id); /* VA 0x00461360; body unreconstructed */
typedef int (*GfxSpriteCreateDescriptorProc)(GfxSpriteDescriptor *descriptor, int image_id);
extern GfxSpriteCreateDescriptorProc volatile g_spriteCreateDescriptor; /* VA 0x0050EBD4 */
int Gfx_SpriteCreateDescriptorNative(GfxSpriteDescriptor *descriptor, int image_id); /* VA 0x0045C920; body unreconstructed */
typedef int (*GfxSpriteFreeDescriptorProc)(GfxSpriteDescriptor *descriptor);
extern GfxSpriteFreeDescriptorProc volatile g_spriteFreeDescriptor; /* VA 0x0050EBDC */
int Gfx_SpriteFreeDescriptorNative(GfxSpriteDescriptor *descriptor); /* VA 0x0045C940; body unreconstructed */
typedef int (*GfxSpriteCopyDescriptorProc)(GfxSpriteDescriptor *descriptor, int image_id);
extern GfxSpriteCopyDescriptorProc volatile g_spriteCopyDescriptor; /* VA 0x0050EBE0 */
int Gfx_SpriteCopyDescriptorNative(GfxSpriteDescriptor *descriptor, int image_id); /* VA 0x0045C960; body unreconstructed */
typedef int (*GfxSpriteReservedProc)(unsigned int option);
extern GfxSpriteReservedProc volatile g_spriteReserved; /* VA 0x0050EBE4 */
int Gfx_SpriteReservedNative(unsigned int option); /* VA 0x00461A80; body unreconstructed */
typedef int (*GfxSpriteGetStateProc)(GfxSpriteState *state);
extern GfxSpriteGetStateProc volatile g_spriteGetState; /* VA 0x0050EBEC */
int Gfx_SpriteGetStateNative(GfxSpriteState *state); /* VA 0x00457110; body unreconstructed */
typedef int (*GfxSpriteSetStateProc)(const GfxSpriteState *state);
extern GfxSpriteSetStateProc volatile g_spriteSetState; /* VA 0x0050EBE8 */
int Gfx_SpriteSetStateNative(const GfxSpriteState *state); /* VA 0x00457170; body unreconstructed */

/* Separately owned lazy work buffers. Consumers use a two-dword header and
 * twelve-byte stepping, but a complete layout/capacity is not established. */
typedef struct GfxSpriteWorkspaceBuffer GfxSpriteWorkspaceBuffer;
extern volatile uint32_t g_spriteWorkspaceAAllocated; /* VA 0x004BACE8 */
extern volatile uint32_t g_spriteWorkspaceACount; /* VA 0x0051AA50; reset word */
extern GfxSpriteWorkspaceBuffer *volatile g_spriteWorkspaceABuffer0; /* VA 0x0051AA5C */
extern GfxSpriteWorkspaceBuffer *volatile g_spriteWorkspaceABuffer1; /* VA 0x0051AA60 */
extern GfxSpriteWorkspaceBuffer *volatile g_spriteWorkspaceABuffer2; /* VA 0x0051AA64 */
extern volatile uint32_t g_spriteWorkspaceBAllocated; /* VA 0x004BACE4 */
extern volatile uint32_t g_spriteWorkspaceBCount; /* VA 0x0051A9CC; reset word */
extern GfxSpriteWorkspaceBuffer *volatile g_spriteWorkspaceBBuffer0; /* VA 0x0051A9D8 */
extern GfxSpriteWorkspaceBuffer *volatile g_spriteWorkspaceBBuffer1; /* VA 0x0051A9DC */
extern GfxSpriteWorkspaceBuffer *volatile g_spriteWorkspaceBBuffer2; /* VA 0x0051A9E0 */
/* No stack arguments or consumed result. Null allocations remain installed. */
void Gfx_InitSpriteWorkspaceA(void); /* VA 0x0045D840 */
void Gfx_InitSpriteWorkspaceB(void); /* VA 0x0045C9F0 */

/* Reconstructed static handle pool initializer; the installer ignores EAX. */
int Gfx_InitSpriteHandles(void); /* VA 0x0045C7F0; EAX=1 */

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

/* Native coefficients are deliberately (cosine, sine, sine, cosine).
 * The duplicate positive sine is an original quirk, not a rotation correction. */
typedef struct {
    int32_t cosine_0;
    int32_t sine_1;
    int32_t sine_2;
    int32_t cosine_3;
} GfxSpriteCoefficients;

typedef struct {
    Point2D *position;
    GfxSpriteHandle *handle;
    volatile GfxSpriteCoefficients *coefficients;
} GfxSpriteRequest;

extern volatile GfxSpriteRequest g_nativeSpriteRequest;       /* VA 0x004BA708 */
extern volatile GfxSpriteCoefficients g_nativeSpriteCoefficients; /* VA 0x0050EC10 */
extern volatile GfxSpriteRequest *volatile g_activeSpriteRequest; /* VA 0x0050EBFC */
int Gfx_DrawSpriteNative(GfxSpriteHandle *handle, Point2D *position,
    const GfxSpriteTransform *transform);
/* VA 0x00457370: no stack arguments; consumes g_activeSpriteRequest.
 * Its native body and ESI rasterizer adapter remain unreconstructed. */
void Gfx_SubmitSpriteRequest(void);

/* Original _ftol returns the low dword of a truncated signed 64-bit result.
 * Indefinite conversion (NaN/outside signed 64-bit range) has low dword zero.
 * GCC x87 intrinsics retain 80-bit intermediates without handwritten assembly.
 * Evaluate only a side-effect-free long-double local through this macro. */
#define GFX_X87_TRUNCATE_LOW32(value) \
    (!(value >= -9223372036854775808.0L && value < 9223372036854775808.0L) \
        ? 0 : (int32_t)(value < 0.0L \
            ? 0u - (unsigned int)__builtin_fmodl(-value, 4294967296.0L) \
            : (unsigned int)__builtin_fmodl(value, 4294967296.0L)))

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
/* IGN_WIN.EXE 0x00456D20: null-only font client of native sprite dispatch.
 * Pass 0 for the pointer-sized third word; this client ignores renderer EAX.
 * General non-null transform handling is not reconstructed here. */
void Gfx_DrawSprite(void *handle, Point2D *position, int null_transform);
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
