#ifndef IGNITION_MEM_H
#define IGNITION_MEM_H

/* Independently recovered Windows state; no aggregate packing is assumed. */
#define MEM_HANDLE_COUNT 200
extern unsigned int g_memHandlesInitialized;
extern unsigned int g_memHandleStatus[MEM_HANDLE_COUNT];
extern short g_memHandleIds[MEM_HANDLE_COUNT];
extern short g_memHandleCursor;

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
