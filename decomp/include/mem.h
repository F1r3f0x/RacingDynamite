#ifndef IGNITION_MEM_H
#define IGNITION_MEM_H

/* Independently recovered Windows state; no aggregate packing is assumed. */
#define MEM_POOL_COUNT 256
#define MEM_POOL_PAGE_COUNT 64
#define MEM_POOL_BLOCK_COUNT 64
#define MEM_POOL_RECORD_COUNT 16

/* Windows allocation hierarchy, independently corroborated by pool creation
 * and allocation. Names are semantic; sizes are raw 32-bit requested lengths.
 */
typedef struct {
    void * volatile pointer;
    volatile unsigned int size;
} MemAllocationRecord;

typedef struct {
    MemAllocationRecord * volatile blocks[MEM_POOL_BLOCK_COUNT];
} MemAllocationPage;

typedef struct {
    char name[64];
    MemAllocationPage * volatile pages[MEM_POOL_PAGE_COUNT];
} MemPool;

extern MemPool * volatile g_memPools[MEM_POOL_COUNT];
/* Caller-cleanup, one name pointer; first null slot or -1 on full/heap failure.
 * Null names are empty; copies <=63 bytes, reads byte 63 when reached, preserves tail.
 * Attaches the 320-byte allocation before name/page initialization.
 */
int Mem_CreatePool(const char *name);
/* No arguments; resets roots without freeing, creates DEFAULT, always EAX=1. */
int Mem_InitPools(void);
/* One cdecl pool-ID dword; requires a valid root/readable hierarchy.
 * Frees only nonzero-size payloads, then blocks/pages/root; clears root last.
 * Leaves record words and hierarchy slots untouched; always returns 1.
 */
int Mem_DestroyPool(int pool_id);
/* Live ascending scan of 256 roots; skips null, destroys nonnull; returns 1. */
int Mem_ShutdownPools(void);
/* Two cdecl stack dwords, EAX=1 on first pointer match, 0 on no match.
 * A valid pool and readable hierarchy are required; no bounds/null guards.
 * Ignores size when selecting. Calls CRT free, then clears size only; retains
 * the pointer, including null/stale pointers. Repeated frees are authentic.
 */
int Mem_Free(int pool_id, void *pointer);

/* Two cdecl dwords. First free size selects a record; zero-size requests
 * remain reusable. Failures retain any newly attached page/block. No guards.
 * Requires readable lookahead words beyond full pool/page/record arrays.
 */
void *Mem_Alloc(int pool_id, unsigned int size);

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

/* Shutdown callbacks use x86 caller cleanup with one raw dword argument.
 * Only this invocation ABI is recovered; callback results are ignored.
 * Raw callback words must name valid functions when their slots are selected.
 */
typedef void (*Mem_ShutdownCallback)(unsigned int parameter);
int Mem_ShutdownHandles(void);

/* Clears every status==1 slot whose registered dword ID matches; no callback.
 * One stack dword, caller cleanup; returns 0 disabled, 1 enabled (even no match).
 */
int Mem_ReleaseHandleId(unsigned int handle_id);

int Mem_InitHandles(void);
/* x86 flat-address contract: if an enabled cursor is negative, the signed
 * word at byte address (unsigned int)g_memHandleIds + 2*cursor must be
 * readable. Differential negative-state validation uses nonaliasing storage;
 * original linked-global layout is not reconstructed by the validation DLL.
 * No lower bound guard exists in the original. Normal initialized use
 * stays 0..199.
 */
int Mem_NextHandleId(void);

/* Engine lifecycle wrappers, no arguments, ordinary RET, always EAX=1.
 * Startup: banner, pools, handles, graphics-resource flags, primitive state,
 * backend zero, unconditionally. Teardown: handle callbacks before pool frees.
 * Startup has no guard; repeated calls can orphan pools/reset resource flags.
 */
int Mem_InitSystem(void);
int Mem_ShutdownSystem(void);

#endif
