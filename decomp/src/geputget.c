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
#include <stdlib.h>

/* Windows input callback state. Both words are loader-zeroed .data tail. */
InputKeyEventCallback volatile g_inputKeyEventCallback = 0; /* VA 0x0050DE14 */
InputPollCallback volatile g_inputPollCallback = 0;         /* VA 0x0050E678 */

/*
 * @original Input_ResetCallbacks (IGN_WIN.EXE @ 0x00455AB0, geputget.c)
 * @fidelity EXACT
 */
void Input_ResetCallbacks(void) {
    g_inputKeyEventCallback = 0;
    g_inputPollCallback = 0;
}

/*
 * @original Gfx_SelectBackend (IGN_WIN.EXE @ 0x00456AF0, geputget.c)
 * @fidelity EXACT
 */
int Gfx_SelectBackend(int backend) {
    if (backend != 0) {
        return 2;
    }

    Gfx_InstallSurfaceDispatch();
    Gfx_InstallSpriteDispatch();
    return 1;
}

/* Independently recovered Windows dispatch slots; loader-zeroed .data tail. */
GfxSurfaceConfigureProc volatile g_surfaceConfigure = 0; /* VA 0x0050EB68 */
GfxSurfaceOpenProc volatile g_surfaceOpen = 0; /* VA 0x0050EB6C */
GfxSurfaceCloseProc volatile g_surfaceClose = 0; /* VA 0x0050EB70 */
GfxSurfaceResetProc volatile g_surfaceReset = 0; /* VA 0x0050EB74 */
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
    g_surfaceClose = Gfx_SurfaceCloseNative;
    g_surfaceReset = Gfx_SurfaceResetNative;
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

/* Windows sprite dispatch words; four intervening words are not installed. */
GfxSpriteOpenProc volatile g_spriteOpen = 0; /* VA 0x0050EBA0 */
GfxSpriteResetProc volatile g_spriteReset = 0; /* VA 0x0050EBA4 */
GfxSpriteCloseProc volatile g_spriteClose = 0; /* VA 0x0050EBA8 */
GfxSpriteOptionProc volatile g_spriteOption = 0; /* VA 0x0050EBAC */
GfxSpriteConfigureProc volatile g_spriteConfigure = 0; /* VA 0x0050EBB0 */
GfxSpriteSetClipProc volatile g_spriteSetClip = 0; /* VA 0x0050EBB4 */
GfxSpriteDrawListProc volatile g_spriteDrawList = 0; /* VA 0x0050EBB8 */
GfxSpriteDrawProc volatile g_spriteDraw = 0; /* VA 0x0050EBBC */
GfxSpriteHandleOpProc volatile g_spriteHandleOp = 0; /* VA 0x0050EBC0 */
GfxSpriteImageOpProc volatile g_spriteImageOp = 0; /* VA 0x0050EBD8 */
GfxSpriteCreateDescriptorProc volatile g_spriteCreateDescriptor = 0; /* VA 0x0050EBD4 */
GfxSpriteFreeDescriptorProc volatile g_spriteFreeDescriptor = 0; /* VA 0x0050EBDC */
GfxSpriteCopyDescriptorProc volatile g_spriteCopyDescriptor = 0; /* VA 0x0050EBE0 */
GfxSpriteReservedProc volatile g_spriteReserved = 0; /* VA 0x0050EBE4 */
GfxSpriteGetStateProc volatile g_spriteGetState = 0; /* VA 0x0050EBEC */
GfxSpriteSetStateProc volatile g_spriteSetState = 0; /* VA 0x0050EBE8 */

/*
 * @original Gfx_InstallSpriteDispatch (IGN_WIN.EXE @ 0x00456E60, geputget.c)
 * @fidelity EXACT
 */
int Gfx_InstallSpriteDispatch(void) {
    g_spriteOpen = Gfx_SpriteOpenNative;
    g_spriteReset = Gfx_SpriteResetNative;
    g_spriteClose = Gfx_SpriteCloseNative;
    g_spriteOption = Gfx_SpriteOptionNative;
    g_spriteConfigure = Gfx_SpriteConfigureNative;
    g_spriteSetClip = Gfx_SpriteSetClipNative;
    g_spriteDrawList = Gfx_SpriteDrawListNative;
    g_spriteDraw = Gfx_DrawSpriteNative;
    g_spriteHandleOp = Gfx_SpriteHandleOpNative;
    g_spriteImageOp = Gfx_SpriteImageOpNative;
    g_spriteCreateDescriptor = Gfx_SpriteCreateDescriptorNative;
    g_spriteFreeDescriptor = Gfx_SpriteFreeDescriptorNative;
    g_spriteCopyDescriptor = Gfx_SpriteCopyDescriptorNative;
    g_spriteReserved = Gfx_SpriteReservedNative;
    g_spriteGetState = Gfx_SpriteGetStateNative;
    g_spriteSetState = Gfx_SpriteSetStateNative;
    Gfx_InitSpriteWorkspaceA();
    Gfx_InitSpriteWorkspaceB();
    Gfx_InitSpriteHandles();
    return 1;
}

/* Flags are file-backed zero words; reset/pointer words are separate BSS
 * storage. Allocations remain owned by these workspaces across reinitialization.
 * A nonzero flag retains all pointers, even after an unchecked allocation failure. */
volatile uint32_t g_spriteWorkspaceAAllocated = 0;
volatile uint32_t g_spriteWorkspaceACount;
GfxSpriteWorkspaceBuffer *volatile g_spriteWorkspaceABuffer0;
GfxSpriteWorkspaceBuffer *volatile g_spriteWorkspaceABuffer1;
GfxSpriteWorkspaceBuffer *volatile g_spriteWorkspaceABuffer2;
volatile uint32_t g_spriteWorkspaceBAllocated = 0;
volatile uint32_t g_spriteWorkspaceBCount;
GfxSpriteWorkspaceBuffer *volatile g_spriteWorkspaceBBuffer0;
GfxSpriteWorkspaceBuffer *volatile g_spriteWorkspaceBBuffer1;
GfxSpriteWorkspaceBuffer *volatile g_spriteWorkspaceBBuffer2;

/*
 * @original Gfx_InitSpriteWorkspaceA (IGN_WIN.EXE @ 0x0045D840, geputget.c)
 * @fidelity EXACT
 */
void Gfx_InitSpriteWorkspaceA(void) {
    g_spriteWorkspaceACount = 0;
    if (g_spriteWorkspaceAAllocated == 0) {
        g_spriteWorkspaceABuffer0 = (GfxSpriteWorkspaceBuffer *)malloc(0x20D8);
        g_spriteWorkspaceABuffer1 = (GfxSpriteWorkspaceBuffer *)malloc(0x20D8);
        g_spriteWorkspaceABuffer2 = (GfxSpriteWorkspaceBuffer *)malloc(0x20D8);
        g_spriteWorkspaceAAllocated = 1;
    }
}

/*
 * @original Gfx_InitSpriteWorkspaceB (IGN_WIN.EXE @ 0x0045C9F0, geputget.c)
 * @fidelity EXACT
 */
void Gfx_InitSpriteWorkspaceB(void) {
    g_spriteWorkspaceBCount = 0;
    if (g_spriteWorkspaceBAllocated == 0) {
        g_spriteWorkspaceBBuffer0 = (GfxSpriteWorkspaceBuffer *)malloc(0x20D8);
        g_spriteWorkspaceBBuffer1 = (GfxSpriteWorkspaceBuffer *)malloc(0x20D8);
        g_spriteWorkspaceBBuffer2 = (GfxSpriteWorkspaceBuffer *)malloc(0x20D8);
        g_spriteWorkspaceBAllocated = 1;
    }
}

/* Statically owned handle pool and copied descriptor views. The default record
 * is loader-zeroed, then RVA 0x611D0 writes only words 0..6; remaining words
 * must survive subsequent initialization. Image records/table belong to the
 * unreconstructed image allocator, not this copy helper. */
GfxSpriteHandle *volatile g_nativeSpriteFreeList[GFX_SPRITE_HANDLE_COUNT];
volatile GfxSpriteHandle g_nativeSpriteHandles[GFX_SPRITE_HANDLE_COUNT];
GfxSpriteHandle *volatile *volatile g_nativeSpriteFreeCursor;
volatile GfxSpriteDescriptor g_nativeSpriteScratch;
volatile GfxSpriteDescriptor g_nativeSpriteDefault;
const char g_nativeSpriteDefaultName[] = "default"; /* VA 0x004BACF8 */
volatile uint32_t g_nativeNextImageId; /* VA 0x00520380 */
volatile GfxSpritePackingNode g_spritePackingTemplate;
GfxSpritePackingBucket *volatile g_spritePackingBuckets[GFX_SPRITE_PACKING_BUCKET_COUNT];
GfxSpritePackingNode *volatile g_spritePackingPages;
volatile int32_t g_nativeImageCapacity;
GfxSpriteDescriptor *volatile *volatile g_nativeImageRecords;

/*
 * @original Gfx_InitDefaultSpriteDescriptor (IGN_WIN.EXE @ 0x004611D0, geputget.c)
 * @fidelity EXACT
 */
int Gfx_InitDefaultSpriteDescriptor(void) {
    g_nativeSpriteDefault.words[0] = (uint32_t)g_nativeSpriteDefaultName;
    g_nativeSpriteDefault.words[1] = 0;
    g_nativeSpriteDefault.words[2] = 0;
    g_nativeSpriteDefault.words[3] = 0;
    g_nativeSpriteDefault.words[4] = 0;
    g_nativeSpriteDefault.words[5] = 0;
    g_nativeSpriteDefault.words[6] = 0;
    g_nativeNextImageId = 1;
    return 0;
}

/*
 * @original Gfx_InitSpritePackingState (IGN_WIN.EXE @ 0x004614D0, geputget.c)
 * @fidelity EXACT
 */
int Gfx_InitSpritePackingState(void) {
    int bucket;

    g_spritePackingTemplate.previous = 0;
    g_spritePackingTemplate.next = 0;
    g_spritePackingTemplate.children = 0;
    g_spritePackingTemplate.range_start = 0;
    g_spritePackingTemplate.range_end = 0;
    g_spritePackingTemplate.pixels = 0;
    for (bucket = 0; bucket < GFX_SPRITE_PACKING_BUCKET_COUNT; ++bucket) {
        g_spritePackingBuckets[bucket] = 0;
    }
    g_spritePackingPages = 0;
    return 0;
}

/*
 * @original Gfx_LinkSpritePackingNode (IGN_WIN.EXE @ 0x00461690, geputget.c)
 * @fidelity EXACT
 */
GfxSpritePackingNode *Gfx_LinkSpritePackingNode(GfxSpritePackingNode *previous,
    GfxSpritePackingNode *current, GfxSpritePackingNode *next) {
    ((volatile GfxSpritePackingNode *)current)->next = next;
    ((volatile GfxSpritePackingNode *)current)->previous = previous;
    if (previous) {
        ((volatile GfxSpritePackingNode *)previous)->next = current;
    }
    if (next) {
        ((volatile GfxSpritePackingNode *)next)->previous = current;
    }
    return previous;
}

/*
 * @original Gfx_AllocBytes (IGN_WIN.EXE @ 0x0045F4A0, geputget.c)
 * @fidelity EXACT
 */
void *Gfx_AllocBytes(uint32_t size) {
    if (size == 0) {
        return 0;
    }
    return malloc(size);
}

/*
 * @original Gfx_AllocAlignedBytes (IGN_WIN.EXE @ 0x00461840, geputget.c)
 * @fidelity EXACT
 */
unsigned char *Gfx_AllocAlignedBytes(uint32_t size, uint32_t alignment) {
    uint32_t allocation;
    uint32_t remainder;
    uint32_t header;

    allocation = (uint32_t)Gfx_AllocBytes(size + alignment + 4u);
    remainder = (allocation + 4u) % alignment;
    header = allocation - remainder + alignment;
    *(volatile uint32_t *)header = allocation;
    return (unsigned char *)(header + 4u);
}

/*
 * @original Gfx_FreeBytes (IGN_WIN.EXE @ 0x0045F8B0, geputget.c)
 * @fidelity EXACT
 */
void Gfx_FreeBytes(void *pointer) {
    if (pointer != 0) {
        free(pointer);
    }
}

/*
 * @original Gfx_FreeAlignedBytes (IGN_WIN.EXE @ 0x00461A60, geputget.c)
 * @fidelity EXACT
 */
void Gfx_FreeAlignedBytes(unsigned char *pointer) {
    uint32_t allocation;

    allocation = *(const volatile uint32_t *)((uint32_t)pointer - 4u);
    Gfx_FreeBytes((void *)allocation);
}

/*
 * @original Gfx_AddSpritePackingPage (IGN_WIN.EXE @ 0x004617E0, geputget.c)
 * @fidelity EXACT
 */
void *Gfx_AddSpritePackingPage(void) {
    volatile GfxSpritePackingNode *page;
    GfxSpritePackingNode *repair;
    unsigned char *result;

    page = (GfxSpritePackingNode *)Gfx_AllocBytes(24u);
    page->range_start = g_spritePackingTemplate.range_start;
    page->range_end = g_spritePackingTemplate.range_end;
    page->pixels = g_spritePackingTemplate.pixels;
    page->children = g_spritePackingTemplate.children;
    page->previous = g_spritePackingTemplate.previous;
    page->next = g_spritePackingTemplate.next;
    page->next = g_spritePackingPages;
    result = Gfx_AllocAlignedBytes(0x10000u, 0x10000u);
    page->pixels = result;
    if (g_spritePackingPages != 0) {
        repair = g_spritePackingPages;
        ((volatile GfxSpritePackingNode *)repair)->previous =
            (GfxSpritePackingNode *)page;
        result = (unsigned char *)repair;
    }
    g_spritePackingPages = (GfxSpritePackingNode *)page;
    return result;
}

/*
 * @original Gfx_FindSpritePackingGap (IGN_WIN.EXE @ 0x00461870, geputget.c)
 * @fidelity EXACT
 */
GfxSpritePackingNode *Gfx_FindSpritePackingGap(int32_t size,
    GfxSpritePackingNode *first) {
    const volatile GfxSpritePackingNode *current;
    GfxSpritePackingNode *next;
    uint32_t limit;
    uint32_t gap;

    if (first == 0) {
        return (GfxSpritePackingNode *)0xFFFFFFFFu;
    }
    current = first;
    if ((int32_t)current->range_start >= size) {
        return (GfxSpritePackingNode *)0xFFFFFFFFu;
    }
    for (;;) {
        next = current->next;
        limit = 256u;
        if (next != 0) {
            limit = ((const volatile GfxSpritePackingNode *)next)->range_start;
        }
        gap = limit - current->range_end;
        if ((int32_t)gap >= size) {
            return (GfxSpritePackingNode *)current;
        }
        if (next == 0) {
            return 0;
        }
        current = next;
    }
}

/*
 * @original Gfx_AddSpritePackingBucket (IGN_WIN.EXE @ 0x004616C0, geputget.c)
 * @fidelity EXACT
 */
GfxSpritePackingBucket *Gfx_AddSpritePackingBucket(int32_t size,
    GfxSpritePackingNode *first_page) {
    volatile GfxSpritePackingNode *page;
    volatile GfxSpritePackingNode *node;
    volatile GfxSpritePackingNode *previous;
    GfxSpritePackingNode *next;
    volatile GfxSpritePackingBucket *bucket;
    GfxSpritePackingBucket *volatile *slot;
    GfxSpritePackingBucket *old_bucket;
    unsigned char *pixels;
    uint32_t offset;

    page = first_page;
    for (;;) {
        if (page == 0) {
            Gfx_AddSpritePackingPage();
            page = g_spritePackingPages;
            continue;
        }
        previous = Gfx_FindSpritePackingGap(size, page->children);
        if (previous != 0) {
            break;
        }
        page = page->next;
    }
    node = (GfxSpritePackingNode *)Gfx_AllocBytes(24u);
    node->range_start = g_spritePackingTemplate.range_start;
    node->range_end = g_spritePackingTemplate.range_end;
    node->pixels = g_spritePackingTemplate.pixels;
    node->children = g_spritePackingTemplate.children;
    node->previous = g_spritePackingTemplate.previous;
    node->next = g_spritePackingTemplate.next;
    if (previous == (GfxSpritePackingNode *)0xFFFFFFFFu) {
        node->range_end = (uint32_t)size;
        pixels = page->pixels;
        offset = node->range_start << 8;
        node->pixels = (unsigned char *)((uint32_t)pixels + offset);
        next = page->children;
        node->next = next;
        if (next != 0) {
            ((volatile GfxSpritePackingNode *)next)->previous =
                (GfxSpritePackingNode *)node;
        }
        page->children = (GfxSpritePackingNode *)node;
    } else {
        node->range_start = previous->range_end;
        node->range_end = previous->range_end + (uint32_t)size;
        offset = node->range_start << 8;
        pixels = page->pixels;
        node->pixels = (unsigned char *)((uint32_t)pixels + offset);
        next = previous->next;
        Gfx_LinkSpritePackingNode((GfxSpritePackingNode *)previous,
            (GfxSpritePackingNode *)node, next);
    }
    bucket = (GfxSpritePackingBucket *)Gfx_AllocBytes(8u);
    slot = (GfxSpritePackingBucket *volatile *)
        ((uint32_t)g_spritePackingBuckets + ((uint32_t)size << 2));
    old_bucket = *slot;
    bucket->next = old_bucket;
    bucket->node = (GfxSpritePackingNode *)node;
    *slot = (GfxSpritePackingBucket *)bucket;
    return (GfxSpritePackingBucket *)bucket;
}

/*
 * @original Gfx_ReleaseSpritePackingStorage (IGN_WIN.EXE @ 0x004618B0, geputget.c)
 * @fidelity EXACT
 */
void Gfx_ReleaseSpritePackingStorage(GfxSpriteDescriptor *descriptor) {
    const volatile GfxSpritePixelView *view;
    volatile GfxSpritePackingNode *page;
    volatile GfxSpritePackingNode *container;
    volatile GfxSpritePackingNode *leaf;
    volatile GfxSpritePackingBucket *bucket;
    volatile GfxSpritePackingBucket *previous_bucket;
    GfxSpritePackingNode *previous;
    GfxSpritePackingNode *next;
    GfxSpritePackingBucket *next_bucket;
    GfxSpritePackingBucket *volatile *slot;
    int32_t height;
    int32_t width;
    uint32_t pixels;

    view = (const volatile GfxSpritePixelView *)descriptor;
    height = view->height;
    if (height <= 0 || height > 256) {
        return;
    }
    width = view->width;
    if (width <= 0 || width > 256) {
        return;
    }
    page = g_spritePackingPages;
    pixels = view->pixels;
    while (page != 0) {
        if ((uint32_t)page->pixels == (pixels & 0xFFFF0000u)) {
            break;
        }
        page = page->next;
    }
    if (page == 0) {
        return;
    }
    container = page->children;
    while ((uint32_t)container->pixels != (pixels & 0xFFFFFF00u)) {
        container = container->next;
        if (container == 0) {
            return;
        }
    }
    leaf = container->children;
    while ((uint32_t)leaf->pixels != pixels) {
        leaf = leaf->next;
        if (leaf == 0) {
            return;
        }
    }
    previous_bucket = 0;
    bucket = g_spritePackingBuckets[height];
    while (bucket->node != (GfxSpritePackingNode *)container) {
        previous_bucket = bucket;
        bucket = bucket->next;
        if (bucket == 0) {
            return;
        }
    }
    next = leaf->next;
    if (next == 0 && leaf->previous == 0) {
        next = container->next;
        if (next == 0 && container->previous == 0) {
            Gfx_FreeAlignedBytes(page->pixels);
            previous = page->previous;
            next = page->next;
            if (previous != 0) {
                ((volatile GfxSpritePackingNode *)previous)->next = next;
            } else {
                g_spritePackingPages = next;
            }
            next = page->next;
            if (next != 0) {
                previous = page->previous;
                ((volatile GfxSpritePackingNode *)next)->previous = previous;
            }
            Gfx_FreeBytes((GfxSpritePackingNode *)page);
        } else {
            previous = container->previous;
            if (previous != 0) {
                ((volatile GfxSpritePackingNode *)previous)->next = next;
            } else {
                page->children = next;
            }
            next = container->next;
            if (next != 0) {
                previous = container->previous;
                ((volatile GfxSpritePackingNode *)next)->previous = previous;
            }
        }
        next_bucket = bucket->next;
        if (previous_bucket != 0) {
            previous_bucket->next = next_bucket;
        } else {
            height = view->height;
            slot = (GfxSpritePackingBucket *volatile *)
                ((uint32_t)g_spritePackingBuckets + ((uint32_t)height << 2));
            *slot = next_bucket;
        }
        Gfx_FreeBytes((GfxSpritePackingBucket *)bucket);
        Gfx_FreeBytes((GfxSpritePackingNode *)container);
    } else {
        previous = leaf->previous;
        if (previous != 0) {
            ((volatile GfxSpritePackingNode *)previous)->next = next;
        } else {
            container->children = next;
        }
        next = leaf->next;
        if (next != 0) {
            previous = leaf->previous;
            ((volatile GfxSpritePackingNode *)next)->previous = previous;
        }
    }
    Gfx_FreeBytes((GfxSpritePackingNode *)leaf);
}


/*
 * @original Gfx_CopySpriteDescriptorPixels (IGN_WIN.EXE @ 0x004612A0, geputget.c)
 * @fidelity EXACT
 */
const GfxSpriteDescriptor *Gfx_CopySpriteDescriptorPixels(
    const GfxSpriteDescriptor *source, const GfxSpriteDescriptor *destination) {
    const volatile GfxSpritePixelView *source_view;
    const volatile GfxSpritePixelView *destination_view;
    uint32_t source_row;
    uint32_t destination_row;
    uint32_t row;
    uint32_t column;
    unsigned char pixel;

    source_view = (const volatile GfxSpritePixelView *)source;
    destination_view = (const volatile GfxSpritePixelView *)destination;
    source_row = source_view->pixels;
    destination_row = destination_view->pixels;
    row = 0;
    while (source_view->height > (int32_t)row) {
        column = 0;
        while (source_view->width > (int32_t)column) {
            pixel = *(const volatile unsigned char *)(source_row + column);
            *(volatile unsigned char *)(destination_row + column) = pixel;
            column++;
        }
        source_row += source_view->stride;
        destination_row += destination_view->stride;
        row++;
    }
    return source;
}

/*
 * @original Gfx_AssignSpritePackingStorage (IGN_WIN.EXE @ 0x00461530, geputget.c)
 * @fidelity EXACT
 */
void Gfx_AssignSpritePackingStorage(GfxSpriteDescriptor *descriptor) {
    volatile GfxSpritePixelView *view;
    volatile GfxSpritePackingBucket *bucket;
    volatile GfxSpritePackingNode *container;
    volatile GfxSpritePackingNode *previous;
    volatile GfxSpritePackingNode *node;
    GfxSpritePackingNode *next;
    GfxSpritePackingNode *page;
    volatile GfxSpriteDescriptor snapshot;
    int32_t height;
    /* Keep the live width load before children and call-argument evaluation. */
    volatile int32_t width;
    uint32_t pixels;
    uint32_t start;
    uint32_t end;
    int word;

    view = (volatile GfxSpritePixelView *)descriptor;
    if (view->height <= 0) {
        return;
    }
    for (;;) {
        height = view->height;
        if (height > 256) {
            return;
        }
        width = view->width;
        if (width <= 0 || width > 256) {
            return;
        }
        bucket = g_spritePackingBuckets[height];
        if (bucket == 0) {
            /* Initially empty slots consume the already loaded height. */
            page = g_spritePackingPages;
        } else {
            for (;;) {
                container = bucket->node;
                width = view->width;
                next = container->children;
                previous = Gfx_FindSpritePackingGap(width, next);
                if (previous != 0) {
                    break;
                }
                bucket = bucket->next;
                if (bucket == 0) {
                    break;
                }
            }
            if (bucket != 0) {
                break;
            }
            /* Exhaustion reads page head before a fresh descriptor height. */
            page = g_spritePackingPages;
            height = view->height;
        }
        Gfx_AddSpritePackingBucket(height, page);
        if (view->height <= 0) {
            return;
        }
    }
    node = (GfxSpritePackingNode *)Gfx_AllocBytes(24u);
    node->range_start = g_spritePackingTemplate.range_start;
    node->range_end = g_spritePackingTemplate.range_end;
    node->pixels = g_spritePackingTemplate.pixels;
    node->children = g_spritePackingTemplate.children;
    node->previous = g_spritePackingTemplate.previous;
    node->next = g_spritePackingTemplate.next;
    if (previous == (GfxSpritePackingNode *)0xFFFFFFFFu) {
        node->range_end = (uint32_t)view->width;
        pixels = (uint32_t)container->pixels;
        start = node->range_start;
        node->pixels = (unsigned char *)(pixels + start);
        next = container->children;
        node->next = next;
        if (next != 0) {
            ((volatile GfxSpritePackingNode *)next)->previous =
                (GfxSpritePackingNode *)node;
        }
        container->children = (GfxSpritePackingNode *)node;
    } else {
        node->range_start = previous->range_end;
        width = view->width;
        end = previous->range_end;
        node->range_end = (uint32_t)width + end;
        pixels = (uint32_t)container->pixels;
        start = node->range_start;
        node->pixels = (unsigned char *)(pixels + start);
        next = previous->next;
        Gfx_LinkSpritePackingNode((GfxSpritePackingNode *)previous,
            (GfxSpritePackingNode *)node, next);
    }
    /* Snapshot after linking, before either descriptor pixel-field rewrite. */
    for (word = 0; word < 16; ++word) {
        snapshot.words[word] =
            ((const volatile GfxSpriteDescriptor *)descriptor)->words[word];
    }
    pixels = (uint32_t)node->pixels;
    view->pixels = pixels;
    view->stride = 256u;
    Gfx_CopySpriteDescriptorPixels((const GfxSpriteDescriptor *)&snapshot,
        descriptor);
}

/*
 * @original Gfx_CopySpriteDescriptor (IGN_WIN.EXE @ 0x004612E0, geputget.c)
 * @fidelity EXACT
 */
int Gfx_CopySpriteDescriptor(GfxSpriteDescriptor *descriptor, int image_id) {
    const volatile GfxSpriteDescriptor *source;
    volatile GfxSpriteDescriptor *destination;
    int live_record;
    int word;

    source = &g_nativeSpriteDefault;
    live_record = 0;
    if (image_id > 0 && image_id < g_nativeImageCapacity) {
        source = g_nativeImageRecords[image_id];
        if (source != 0) {
            live_record = 1;
        } else {
            source = &g_nativeSpriteDefault;
        }
    }
    destination = descriptor;
    for (word = 0; word < 16; ++word) {
        destination->words[word] = source->words[word];
    }
    if (live_record) {
        destination->words[6] = 0;
        destination->words[7] = 0;
        return 0;
    }
    return image_id;
}

/*
 * @original Gfx_InitSpriteHandles (IGN_WIN.EXE @ 0x0045C7F0, geputget.c)
 * @fidelity EXACT
 */
int Gfx_InitSpriteHandles(void) {
    int handle;

    g_nativeSpriteFreeCursor = g_nativeSpriteFreeList + GFX_SPRITE_HANDLE_COUNT;
    for (handle = 0; handle < GFX_SPRITE_HANDLE_COUNT; ++handle) {
        g_nativeSpriteFreeList[handle] = (GfxSpriteHandle *)&g_nativeSpriteHandles[handle];
        g_nativeSpriteHandles[handle].image_id = 0;
    }
    Gfx_CopySpriteDescriptor((GfxSpriteDescriptor *)&g_nativeSpriteScratch, 0);
    return 1;
}

/* Global font table matching IGN_WIN.EXE @ 0x0063F2E0 */
FontSlot g_fonts[MAX_FONTS];

/* Global font system state */
int g_fontSystemInitialized = 0;                      /* IGN_WIN.EXE @ 0x004BA6C4 */
static const char s_fontExitContext[] = "fontExit()"; /* IGN_WIN.EXE @ 0x004BA6C8 */
int g_fontSubsystemHandle = 0;                        /* IGN_WIN.EXE @ 0x0050E680 */
int g_fileErrorLine = 0;                              /* IGN_WIN.EXE @ 0x004BAB34 */

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
extern void *Gfx_SpriteOp(void *desc, int op);
extern void Gfx_DrawSprite(void *handle, Point2D *pos, int flags);
extern void *File_LoadToMemory(const char *filename);

/**
 * @original Font_InitSystem (IGN_WIN.EXE @ 0x00456180, geputget.c)
 * @fidelity EXACT
 * @notes IGN_WIN.EXE @ 0x00456180. Initializes font subsystem: allocates handle ID via
 *        Mem_NextHandleId, registers Font_Shutdown callback via Mem_RegisterHandle,
 *        and resets all 30 FontSlot entries in g_fonts.
 */
int Font_InitSystem(void) {
    int i;

    if (g_fontSystemInitialized == 1) {
        return 1010;
    }

    g_fontSystemInitialized = 1;
    g_fontSubsystemHandle = Mem_NextHandleId();
    g_memPendingContext = (unsigned int)(size_t)s_fontExitContext;
    g_memPendingCallback = (unsigned int)(size_t)Font_Shutdown;
    g_memPendingParameter = 0;
    Mem_RegisterHandle((unsigned int)g_fontSubsystemHandle);

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
 * @original Font_Shutdown (IGN_WIN.EXE @ 0x00456210, geputget.c)
 * @fidelity EXACT
 * @notes IGN_WIN.EXE @ 0x00456210. Registered font subsystem cleanup callback; releases handle ID and unloads all active font slots.
 */
int Font_Shutdown(void) {
    int i;

    if (g_fontSystemInitialized == 0) {
        return 1020;
    }

    Mem_ReleaseHandleId(g_fontSubsystemHandle);
    g_fontSystemInitialized = 0;

    for (i = 0; i < MAX_FONTS; i++) {
        if (g_fonts[i].in_use == 1) {
            Font_Unload(i);
        }
    }

    return 1;
}

/**
 * @original Font_Parse (IGN_WIN.EXE @ 0x00456270, geputget.c)
 * @fidelity EXACT
 * @notes IGN_WIN.EXE @ 0x00456270. Validates LFT header, allocates a font slot, extracts 224 glyph
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
    if (memcmp(hdr, "LFT", 4) != 0) {
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
 * @original Font_Load (IGN_WIN.EXE @ 0x00456420, geputget.c)
 * @fidelity EXACT
 * @notes IGN_WIN.EXE @ 0x00456420. Loads a .LFT font from disk via File_LoadToMemory, parses glyphs via Font_Parse, frees temporary buffer via Mem_Free, and returns font slot ID.
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
 * @original Font_Unload (IGN_WIN.EXE @ 0x00456470, geputget.c)
 * @fidelity EXACT
 * @notes IGN_WIN.EXE @ 0x00456470. Frees all sprite handles for a font slot and marks the slot available.
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
 * @original Font_GetTextWidth (IGN_WIN.EXE @ 0x004564D0, geputget.c)
 * @fidelity EXACT
 * @notes IGN_WIN.EXE @ 0x004564D0. Computes string pixel width considering proportional flag,
 *        character glyph presence, widths table, extra spacing, and space character sizing.
 */
int Font_GetTextWidth(const char *text, int font_id) {
    unsigned int total_width = 0;
    volatile int discarded_space;
    int i;

    if (font_id >= MAX_FONTS || g_fonts[font_id].in_use == 0) {
        g_fileErrorLine = 1040;
    }

    if (g_fonts[font_id].field_08 != 0) {
        return 2;
    }

    /* The original converts this value, then discards it without clamping. */
    discarded_space = (int)((double)((short)g_fonts[font_id].height * 256) * 0.35);
    (void)discarded_space;

    if (g_fonts[font_id].is_proportional == 0) {
        for (i = 0; i < (int)strlen(text); i++) {
            signed char c = (signed char)text[i];
            if (c == '\0') {
                break;
            }

            /* Signed bytes may address header bytes or the preceding slot. */
            if (*((const unsigned char *)&g_fonts + font_id * sizeof(FontSlot)
                    + offsetof(FontSlot, glyph_present) + (int)c - 32) == 1) {
                total_width += (unsigned int)(short)g_fonts[font_id].height
                    + (unsigned int)g_fonts[font_id].extra_spacing;
            } else if (c == ' ') {
                total_width += (unsigned int)(short)g_fonts[font_id].height
                    + (unsigned int)g_fonts[font_id].extra_spacing;
            }
        }
    } else {
        for (i = 0; i < (int)strlen(text); i++) {
            signed char c = (signed char)text[i];
            if (c == '\0') {
                break;
            }

            if (*((const unsigned char *)&g_fonts + font_id * sizeof(FontSlot)
                    + offsetof(FontSlot, glyph_present) + (int)c - 32) == 1) {
                /* Use the containing table, rather than an out-of-range subarray. */
                const short *width = (const short *)((const unsigned char *)&g_fonts
                    + font_id * sizeof(FontSlot) + offsetof(FontSlot, widths)
                    + ((int)c - 32) * (int)sizeof(short));
                total_width += (unsigned int)*width
                    + (unsigned int)g_fonts[font_id].extra_spacing;
            } else if (c == ' ') {
                int space_w = (int)((double)((short)g_fonts[font_id].height * 256)
                    * 0.35 * (1.0 / 256.0));
                total_width += (unsigned int)space_w;
            }
        }
    }

    return (int)total_width;
}

/**
 * @original Font_DrawText (IGN_WIN.EXE @ 0x00456660, geputget.c)
 * @fidelity EXACT
 * @notes Native sprite dispatch boundary; see windows_font_draw_text.md.
 */
int Font_DrawText(const char *text, int font_id, int x, int y) {
    unsigned int cursor = (unsigned int)x;
    unsigned int origin = (unsigned int)x;
    int proportional;
    int measure;
    int pass;
    int i;
    volatile int discarded_space_width;

    if (font_id >= MAX_FONTS || g_fonts[font_id].in_use == 0) {
        return 1040;
    }
    if (g_fonts[font_id].field_08 != 0) {
        return 2;
    }

    discarded_space_width = (int)((double)((short)g_fonts[font_id].height * 256)
        * 0.35);
    (void)discarded_space_width;
    proportional = g_fonts[font_id].is_proportional != 0;
    measure = g_fonts[font_id].alignment != 0;

    /* A separate measurement pass precedes all drawing when alignment is set. */
    for (pass = measure ? 0 : 1; pass < 2; pass++) {
        for (i = 0; i < (int)strlen(text); i++) {
            signed char c = (signed char)text[i];
            const unsigned char *slot_bytes = (const unsigned char *)&g_fonts
                + font_id * sizeof(FontSlot);
            const short *width = (const short *)(slot_bytes
                + offsetof(FontSlot, widths) + ((int)c - 32) * (int)sizeof(short));

            if (c == '\0') {
                break;
            }
            /* Signed character indexing can read header/preceding-slot bytes. */
            if (*(slot_bytes + offsetof(FontSlot, glyph_present) + (int)c - 32) == 1) {
                if (pass == 1) {
                    Point2D position;
                    void *handle;
                    int inset = 0;

                    if (!proportional) {
                        inset = ((short)g_fonts[font_id].height - (int)*width) / 2;
                    }
                    position.x = (int)((cursor + (unsigned int)inset) << 8);
                    position.y = (int)((unsigned int)y << 8);
                    handle = *(void *const *)(slot_bytes
                        + offsetof(FontSlot, glyph_handles)
                        + ((int)c - 32) * (int)sizeof(void *));
                    /* The native third argument is a pointer; this client uses
                     * only its all-zero/null representation. EAX is ignored. */
                    Gfx_DrawSprite(handle, &position, 0);
                }
                /* The renderer may change metrics: advance from the live table. */
                cursor += (unsigned int)(proportional ? (int)*width
                    : (int)(short)g_fonts[font_id].height)
                    + (unsigned int)g_fonts[font_id].extra_spacing;
            } else if (c == ' ') {
                if (proportional) {
                    int space_width = (int)((double)((short)g_fonts[font_id].height * 256)
                        * 0.35 * (1.0 / 256.0));
                    cursor += (unsigned int)space_width;
                } else {
                    cursor += (unsigned int)(short)g_fonts[font_id].height
                        + (unsigned int)g_fonts[font_id].extra_spacing;
                }
            }
        }
        if (pass == 0) {
            if (g_fonts[font_id].alignment == 1) {
                /* Original divides the wrapped origin-minus-end displacement. */
                cursor = origin + (unsigned int)((int)(origin - cursor) / 2);
            } else if (g_fonts[font_id].alignment == 2) {
                cursor = origin + origin - cursor;
            } else {
                cursor = origin;
            }
        }
    }
    return 1;
}

/* The request is initialized file-backed zero data. Coefficients and the active
 * pointer are loader-zeroed storage in IGN_WIN.EXE. These globals persist;
 * position and handle pointers are borrowed for synchronous submission. */
volatile GfxSpriteRequest g_nativeSpriteRequest = {0, 0, 0};
volatile GfxSpriteCoefficients g_nativeSpriteCoefficients;
volatile GfxSpriteRequest *volatile g_activeSpriteRequest;

/**
 * @original Gfx_DrawSpriteNative (IGN_WIN.EXE @ 0x004571B0, geputget.c)
 * @fidelity EXACT
 * @notes Native request/coefficient publication; downstream body not rebuilt.
 */
int Gfx_DrawSpriteNative(GfxSpriteHandle *handle, Point2D *position,
    const GfxSpriteTransform *transform) {
    g_nativeSpriteRequest.position = position;
    g_nativeSpriteRequest.coefficients = 0;
    g_nativeSpriteRequest.handle = handle;
    if (transform != 0) {
        long double value = (long double)transform->scale * 65536.0L;
        int32_t scale = GFX_X87_TRUNCATE_LOW32(value);
        /* A volatile copy prevents contraction into FSINCOS: the original uses
         * separate FCOS/FSIN, including their out-of-range operand behavior. */
        volatile float angle = transform->angle_radians;
        int32_t cosine;
        int32_t sine;

        value = (long double)scale * __builtin_cosl((long double)angle);
        cosine = GFX_X87_TRUNCATE_LOW32(value);
        g_nativeSpriteCoefficients.cosine_0 = cosine;
        value = (long double)scale * __builtin_sinl((long double)angle);
        sine = GFX_X87_TRUNCATE_LOW32(value);
        g_nativeSpriteCoefficients.sine_1 = sine;
        g_nativeSpriteCoefficients.sine_2 = sine;
        g_nativeSpriteRequest.coefficients = &g_nativeSpriteCoefficients;
        g_nativeSpriteCoefficients.cosine_3 = cosine;
    }
    g_activeSpriteRequest = &g_nativeSpriteRequest;
    Gfx_SubmitSpriteRequest();
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
