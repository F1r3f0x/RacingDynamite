#ifndef IGNITION_MEM_H
#define IGNITION_MEM_H

/* Independently recovered Windows state; no aggregate packing is assumed. */
#define MEM_HANDLE_COUNT 200
extern unsigned int g_memHandlesInitialized;
extern unsigned int g_memHandleStatus[MEM_HANDLE_COUNT];
extern short g_memHandleIds[MEM_HANDLE_COUNT];
extern short g_memHandleCursor;
/* Raw 32-bit bookkeeping words; pointer/callback signatures remain unknown.
 * Separate arrays, each 200 dwords; no aggregate or original link layout assumed.
 * Volatile accesses retain the observed dispatch-read/store ordering.
 */
extern volatile unsigned int g_memHandleContexts[MEM_HANDLE_COUNT];
extern volatile unsigned int g_memHandleParameters[MEM_HANDLE_COUNT];
extern volatile unsigned int g_memRegisteredHandleIds[MEM_HANDLE_COUNT];
extern volatile unsigned int g_memHandleCallbacks[MEM_HANDLE_COUNT];
extern volatile unsigned int g_memHandleFlags[MEM_HANDLE_COUNT];
extern volatile unsigned int g_memPendingContext;
extern volatile unsigned int g_memPendingCallback;
extern volatile unsigned int g_memPendingParameter;

/* Caller-cleanup x86 ABI, one 32-bit argument (copied without interpretation).
 * Disabled returns 0; enabled returns first free slot 0..199, or -1 if full.
 * Single-threaded, readable/writable nonaliasing globals; no callback is invoked.
 */
int Mem_RegisterHandle(unsigned int handle_id);

int Mem_InitHandles(void);
/* x86 flat-address contract: if an enabled cursor is negative, the signed
 * word at byte address (unsigned int)g_memHandleIds + 2*cursor must be
 * readable. Differential negative-state validation uses nonaliasing storage;
 * original linked-global layout is not reconstructed by the validation DLL.
 * No lower bound guard exists in the original. Normal initialized use
 * stays 0..199.
 */
int Mem_NextHandleId(void);

#endif
