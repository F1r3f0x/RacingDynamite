/* Windows handle-table initialization. Original source filename is unknown;
 * mem.c is the reconstruction module, inferred from adjacent handle consumers.
 */
#include "mem.h"

typedef char Mem_UIntMustBe32Bits[(sizeof(unsigned int) == 4) ? 1 : -1];
typedef char Mem_ShortMustBe16Bits[(sizeof(short) == 2) ? 1 : -1];
typedef char Mem_PointerMustBe32Bits[(sizeof(void *) == 4) ? 1 : -1];

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
