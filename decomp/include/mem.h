#ifndef IGNITION_MEM_H
#define IGNITION_MEM_H

/* Independently recovered Windows state; no aggregate packing is assumed. */
#define MEM_HANDLE_COUNT 200
extern unsigned int g_memHandlesInitialized;
extern unsigned int g_memHandleStatus[MEM_HANDLE_COUNT];
extern short g_memHandleIds[MEM_HANDLE_COUNT];
extern short g_memHandleCursor;

int Mem_InitHandles(void);

#endif
