/* Windows handle-table initialization. Original source filename is unknown;
 * mem.c is the reconstruction module, inferred from adjacent handle consumers.
 */
#include "mem.h"
#include <stddef.h>

/* CRT caller-cleanup free boundary; native heap behavior is not reconstructed. */
extern void free(void *pointer);
extern void *malloc(size_t size);

typedef char Mem_UIntMustBe32Bits[(sizeof(unsigned int) == 4) ? 1 : -1];
typedef char Mem_ShortMustBe16Bits[(sizeof(short) == 2) ? 1 : -1];
typedef char Mem_PointerMustBe32Bits[(sizeof(void *) == 4) ? 1 : -1];
typedef char Mem_CallbackMustBe32Bits[(sizeof(Mem_ShutdownCallback) == 4) ? 1 : -1];

/* IGN_WIN.EXE preferred VAs: flag 0x004BAB38 (file initialized zero),
 * status 0x005116E0 (800 bytes), IDs 0x00511230 (400 bytes),
 * cursor 0x00512040 (2 bytes). The latter three are loader-zeroed .data.
 */
unsigned int g_memHandlesInitialized = 0;
unsigned int g_memHandleStatus[MEM_HANDLE_COUNT];
short g_memHandleIds[MEM_HANDLE_COUNT];
short g_memHandleCursor;

/* IGN_WIN.EXE loader-zeroed .data: contexts 0x00510BF0,
 * parameters 0x00510F10, registered IDs 0x005113C0,
 * callbacks 0x00511A00, flags 0x00511D20 (800 bytes each).
 * Pending context/callback/parameter: 0x0063C690/694/698 (4 bytes each).
 * Names describe observed transfers, not recovered original symbols/types.
 */
volatile unsigned int g_memHandleContexts[MEM_HANDLE_COUNT];
volatile unsigned int g_memHandleParameters[MEM_HANDLE_COUNT];
volatile unsigned int g_memRegisteredHandleIds[MEM_HANDLE_COUNT];
volatile unsigned int g_memHandleCallbacks[MEM_HANDLE_COUNT];
volatile unsigned int g_memHandleFlags[MEM_HANDLE_COUNT];
volatile unsigned int g_memPendingContext;
volatile unsigned int g_memPendingCallback;
volatile unsigned int g_memPendingParameter;


/* IGN_WIN.EXE VA 0x0063C6A0: 256 loader-zeroed pool pointers.
 * Pool name occupies 64 bytes, then 64 page pointers. Pages contain 64
 * record-block pointers; each block contains 16 pointer/size dword pairs.
 */
MemPool * volatile g_memPools[MEM_POOL_COUNT];
typedef char Mem_RecordSize[(sizeof(MemAllocationRecord) == 8) ? 1 : -1];
typedef char Mem_RecordSizeOffset[(offsetof(MemAllocationRecord, size) == 4) ? 1 : -1];
typedef char Mem_PageSize[(sizeof(MemAllocationPage) == 256) ? 1 : -1];
typedef char Mem_PoolSize[(sizeof(MemPool) == 320) ? 1 : -1];
typedef char Mem_PoolPagesOffset[(offsetof(MemPool, pages) == 64) ? 1 : -1];

/* @original Mem_Free (IGN_WIN.EXE @ 0x0045B000, inferred mem.c)
 * @fidelity EXACT
 * First pointer match in ascending page/block/record order, regardless of size.
 * Retains the record pointer; clears size after CRT free, including on repeat.
 */
int Mem_Free(int pool_id, void *pointer)
{
    int page_index;
    int block_index;
    int record_index;
    unsigned int pool_address;
    MemPool *pool;
    MemAllocationPage *page;
    MemAllocationRecord *records;

    /* Preserve raw x86 indexed address arithmetic without signed C overflow.
     * Normal pool IDs are 0..255; surrounding-table invalid IDs are unverified.
     */
    pool_address = (unsigned int)g_memPools + (unsigned int)pool_id * 4U;
    pool = *(MemPool * volatile *)pool_address;
    for (page_index = 0; page_index < MEM_POOL_PAGE_COUNT; ++page_index) {
        page = pool->pages[page_index];
        if (page != NULL) {
            for (block_index = 0; block_index < MEM_POOL_BLOCK_COUNT; ++block_index) {
                records = page->blocks[block_index];
                if (records != NULL) {
                    for (record_index = 0; record_index < MEM_POOL_RECORD_COUNT; ++record_index) {
                        if (records[record_index].pointer == pointer) {
                            free(pointer);
                            records[record_index].size = 0U;
                            return 1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

/* @original Mem_Alloc (IGN_WIN.EXE @ 0x0045AE10, inferred mem.c)
 * @fidelity EXACT
 * Ascending first-zero-size allocation; retains partial hierarchy on failure.
 * Lookahead reads at each exhausted array are authentic, even beyond its end.
 */
void *Mem_Alloc(int pool_id, unsigned int size)
{
    unsigned int pool_address;
    MemPool *pool;
    MemAllocationPage *page;
    MemAllocationRecord *records;
    void *pointer;
    int page_index;
    int block_index;
    int record_index;

    pool_address = (unsigned int)g_memPools + (unsigned int)pool_id * 4U;
    pool = *(MemPool * volatile *)pool_address;
    page_index = 0;
    /* Integer address calculation keeps the exhausted-array lookahead read
     * without C array indexing beyond the declared aggregate. Caller storage
     * must include readable trailing words, as required by native execution.
     */
#define MEM_PAGE_AT(i) (*(MemAllocationPage * volatile *)((unsigned int)pool + \
    offsetof(MemPool, pages) + (unsigned int)(i) * sizeof(page)))
#define MEM_BLOCK_AT(i) (*(MemAllocationRecord * volatile *)((unsigned int)page + \
    offsetof(MemAllocationPage, blocks) + (unsigned int)(i) * sizeof(records)))
#define MEM_SIZE_AT(i) (*(volatile unsigned int *)((unsigned int)records + \
    offsetof(MemAllocationRecord, size) + (unsigned int)(i) * sizeof(*records)))
    while (MEM_PAGE_AT(page_index) != NULL && page_index < MEM_POOL_PAGE_COUNT) {
        page = MEM_PAGE_AT(page_index);
        block_index = 0;
        while (MEM_BLOCK_AT(block_index) != NULL && block_index < MEM_POOL_BLOCK_COUNT) {
            records = MEM_BLOCK_AT(block_index);
            record_index = 0;
            while (MEM_SIZE_AT(record_index) != 0U && record_index < MEM_POOL_RECORD_COUNT) {
                ++record_index;
            }
            if (record_index != MEM_POOL_RECORD_COUNT) {
                pointer = malloc(size);
                if (pointer == NULL) {
                    return NULL;
                }
                records[record_index].pointer = pointer;
                records[record_index].size = size;
                return pointer;
            }
            ++block_index;
        }
        if (block_index != MEM_POOL_BLOCK_COUNT) {
            records = (MemAllocationRecord *)malloc(MEM_POOL_RECORD_COUNT * sizeof(*records));
            if (records == NULL) {
                return NULL;
            }
            page->blocks[block_index] = records;
            /* Existing-page branch leaves record zero's size untouched until
             * payload allocation succeeds. Pointer words remain indeterminate.
             */
            for (record_index = 1; record_index < MEM_POOL_RECORD_COUNT; ++record_index) {
                records[record_index].size = 0U;
            }
            pointer = malloc(size);
            if (pointer == NULL) {
                return NULL;
            }
            records[0].pointer = pointer;
            records[0].size = size;
            return pointer;
        }
        ++page_index;
    }
#undef MEM_PAGE_AT
#undef MEM_BLOCK_AT
#undef MEM_SIZE_AT
    if (page_index == MEM_POOL_PAGE_COUNT) {
        return NULL;
    }
    page = (MemAllocationPage *)malloc(sizeof(*page));
    if (page == NULL) {
        return NULL;
    }
    pool->pages[page_index] = page;
    for (block_index = 0; block_index < MEM_POOL_BLOCK_COUNT; ++block_index) {
        page->blocks[block_index] = NULL;
    }
    records = (MemAllocationRecord *)malloc(MEM_POOL_RECORD_COUNT * sizeof(*records));
    if (records == NULL) {
        return NULL;
    }
    page->blocks[0] = records;
    for (record_index = 0; record_index < MEM_POOL_RECORD_COUNT; ++record_index) {
        records[record_index].size = 0U;
    }
    pointer = malloc(size);
    if (pointer == NULL) {
        return NULL;
    }
    records[0].pointer = pointer;
    records[0].size = size;
    return pointer;
}

/* @original Mem_ReleaseHandleId (IGN_WIN.EXE @ 0x0045B410, inferred mem.c)
 * @fidelity EXACT
 * Semantic name; one stack dword, ordinary RET, EAX=0 disabled or 1 enabled.
 * Full ascending scan, including duplicate IDs; no callback or other cleanup.
 */
int Mem_ReleaseHandleId(unsigned int handle_id)
{
    int index;
    volatile unsigned int *status;

    if (*(volatile unsigned int *)&g_memHandlesInitialized == 0U) {
        return 0;
    }
    status = g_memHandleStatus;
    for (index = 0; index < MEM_HANDLE_COUNT; ++index) {
        if (status[index] == 1U && g_memRegisteredHandleIds[index] == handle_id) {
            status[index] = 0U;
        }
    }
    return 1;
}

/* @original Mem_ShutdownHandles (IGN_WIN.EXE @ 0x0045B240, inferred mem.c)
 * @fidelity EXACT
 * No arguments; x86 caller cleanup, EAX=1. Semantic name, unknown source name.
 * Two live scans; callbacks may mutate later slots or reenter after flag clear.
 */
int Mem_ShutdownHandles(void)
{
    int pass;
    int index;
    unsigned int flag;
    /* Keep the parameter load before the callback-word load, even when the
     * compiler folds an argument into a PUSH memory operand.
     */
    volatile unsigned int parameter;
    Mem_ShutdownCallback callback;
    volatile unsigned int *status;

    if (*(volatile unsigned int *)&g_memHandlesInitialized == 0U) {
        return 1;
    }
    *(volatile unsigned int *)&g_memHandlesInitialized = 0U;
    status = g_memHandleStatus;
    for (pass = 0; pass < 2; ++pass) {
        flag = (pass == 0) ? 0x10000U : 0x20000U;
        for (index = 0; index < MEM_HANDLE_COUNT; ++index) {
            if (status[index] == 1U && g_memHandleFlags[index] == flag) {
                status[index] = 0U;
                parameter = g_memHandleParameters[index];
                callback = (Mem_ShutdownCallback)g_memHandleCallbacks[index];
                callback(parameter);
            }
        }
    }
    return 1;
}

/* @original Mem_RegisterHandle (IGN_WIN.EXE @ 0x0045B360, inferred mem.c)
 * @fidelity EXACT
 * Semantic name; bounded behavioral reconstruction, no instruction match claim.
 * One stack dword; ordinary RET, caller cleanup; signed EAX result.
 */
int Mem_RegisterHandle(unsigned int handle_id)
{
    int index;
    unsigned int context;
    volatile unsigned int *status;

    if (*(volatile unsigned int *)&g_memHandlesInitialized == 0U) {
        return 0;
    }
    status = g_memHandleStatus;
    for (index = 0; index < MEM_HANDLE_COUNT; ++index) {
        if (status[index] == 0U) {
            context = g_memPendingContext;
            status[index] = 1U;
            g_memHandleFlags[index] = 0x10000U;
            g_memHandleContexts[index] = context;
            g_memRegisteredHandleIds[index] = handle_id;
            g_memHandleParameters[index] = g_memPendingParameter;
            g_memHandleCallbacks[index] = g_memPendingCallback;
            return index;
        }
    }
    return -1;
}

/* @original Mem_NextHandleId (IGN_WIN.EXE @ 0x0045B1B0, inferred mem.c)
 * @fidelity EXACT
 * Semantic name, not a recovered symbol. No arguments; signed EAX result.
 * Behavioral reconstruction under the x86 flat-address contract in mem.h.
 * Integer/pointer conversion is target-specific, not portable ISO C.
 */
int Mem_NextHandleId(void)
{
    int cursor;
    unsigned int address;

    if (g_memHandlesInitialized == 0U) {
        return -1;
    }
    cursor = g_memHandleCursor;
    if (cursor + 1 >= MEM_HANDLE_COUNT) {
        return -1;
    }
    g_memHandleCursor = (short)(cursor + 1);
    /* Avoid undefined negative array indexing. Preserve the original signed
     * displacement using 32-bit modular address arithmetic, including reads
     * before the ID table. The caller supplies readable signed-word storage.
     */
    address = (unsigned int)g_memHandleIds;
    address += (unsigned int)(cursor * 2);
    return *(short *)address;
}

/* @original Mem_InitHandles (IGN_WIN.EXE @ 0x0045B1F0, inferred mem.c)
 * @fidelity EXACT
 * Behavioral reconstruction; compiler instruction equality is not claimed.
 * No arguments; EAX result, ordinary x86 caller-cleanup ABI, DF clear.
 */
int Mem_InitHandles(void)
{
    int i;

    if (g_memHandlesInitialized == 1U) {
        return 1;
    }
    g_memHandlesInitialized = 1U;
    for (i = 0; i < MEM_HANDLE_COUNT; ++i) {
        g_memHandleStatus[i] = 0U;
    }
    for (i = 0; i < MEM_HANDLE_COUNT; ++i) {
        g_memHandleIds[i] = (short)(i + 1);
    }
    g_memHandleCursor = 0;
    return 1;
}
