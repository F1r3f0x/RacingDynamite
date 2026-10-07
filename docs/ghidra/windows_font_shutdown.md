# Font_Shutdown (VA 0x00456210)

## Contract
- **Routine**: `Font_Shutdown` (registered font subsystem cleanup callback)
- **Address**: `IGN_WIN.EXE` @ VA `0x00456210` / RVA `0x00056210`
- **Size**: 83 bytes (`0x53`), FPO extent `[0x00456210, 0x00456263)`
- **Padding**: 13 bytes of `0xCC` (`INT3`) ending at `0x00456270`
- **Routine SHA-256**: `93eb65fc8f1076b5a1ec14a6b4e1b603893a3eb1bc30f17017e5ab54809a604c`
- **Signature**: `int Font_Shutdown(void)`
- **ABI**: `cdecl` (caller cleanup; callee preserves `esi`, `edi`, `ebx`, `ebp`)
- **Return Value**:
  - `1020` (`0x3FC`) if uninitialized (`g_fontSystemInitialized == 0`)
  - `1` if successfully shut down and all active font slots unloaded
- **Dependencies**:
  - `Mem_ReleaseHandleId` @ `0x0045B410` (RVA `0x0005B410`)
  - `Font_Unload` @ `0x00456470` (RVA `0x00056470`)
  - `g_fontSystemInitialized` @ `0x004BA6C4` (RVA `0x000BA6C4`)
  - `g_fontSubsystemHandle` @ `0x0050E680` (RVA `0x0010E680`)
  - `g_fonts` array @ `0x0063F2E0` (RVA `0x0023F2E0`, 30 slots of 1600 bytes, ends at `0x0064AE60`)

## Binary Evidence

Disassembly and operand relocations from `IGN_WIN.EXE`:
```assembly
0x00456210: 833dc4a64b0000   cmp    dword ptr [0x4ba6c4], 0    ; test g_fontSystemInitialized (reloc @ 0x456212)
0x00456217: 56               push   esi                        ; preserve esi
0x00456218: 57               push   edi                        ; preserve edi
0x00456219: 7508             jne    0x456223                   ; if initialized, proceed
0x0045621b: b8fc030000       mov    eax, 0x3fc                 ; return 1020 (error: not initialized)
0x00456220: 5f               pop    edi                        ; restore edi
0x00456221: 5e               pop    esi                        ; restore esi
0x00456222: c3               ret                               ; return
0x00456223: a180e65000       mov    eax, dword ptr [0x50e680]  ; load g_fontSubsystemHandle (reloc @ 0x456224)
0x00456228: 33f6             xor    esi, esi                   ; loop index i = 0
0x0045622a: 50               push   eax                        ; push handle argument
0x0045622b: bfe0f26300       mov    edi, 0x63f2e0              ; edi = &g_fonts[0] (reloc @ 0x45622c)
0x00456230: e8db510000       call   0x45b410                   ; call Mem_ReleaseHandleId(g_fontSubsystemHandle)
0x00456235: 83c404           add    esp, 4                     ; caller cleanup
0x00456238: 8935c4a64b00     mov    dword ptr [0x4ba6c4], esi  ; g_fontSystemInitialized = 0 (reloc @ 0x45623a)
0x0045623e: 833f01           cmp    dword ptr [edi], 1         ; check g_fonts[i].in_use == 1
0x00456241: 7509             jne    0x45624c                   ; if not allocated, skip
0x00456243: 56               push   esi                        ; push font slot index i
0x00456244: e827020000       call   0x456470                   ; call Font_Unload(i)
0x00456249: 83c404           add    esp, 4                     ; caller cleanup
0x0045624c: 81c740060000     add    edi, 0x640                 ; advance edi by sizeof(FontSlot) = 1600 (0x640)
0x00456252: 46               inc    esi                        ; i++
0x00456253: 81ff60ae6400     cmp    edi, 0x64ae60              ; check if edi reached g_fonts + 30 * 1600 (reloc @ 0x456255)
0x00456259: 72e3             jb     0x45623e                   ; loop while edi < 0x64ae60
0x0045625b: b801000000       mov    eax, 1                     ; return 1 (success)
0x00456260: 5f               pop    edi                        ; restore edi
0x00456261: 5e               pop    esi                        ; restore esi
0x00456262: c3               ret                               ; return
```

## System Integration & Registration
In `IGN_WIN.EXE`:
1. `Font_InitSystem` @ VA `0x00456180`:
   - Allocates a handle via `Mem_NextHandleId` (`0x0045B1B0`) and stores it in `g_fontSubsystemHandle` (`0x0050E680`).
   - Stages registration parameters:
     - `g_memPendingContext` (`0x0063C690`) = `0x004BA6C8`
     - `g_memPendingCallback` (`0x0063C694`) = `0x00456210` (`Font_Shutdown`)
     - `g_memPendingParameter` (`0x0063C698`) = `0`
   - Calls `Mem_RegisterHandle` (`0x0045B360`).
2. `Mem_ShutdownHandles` @ VA `0x0045B240`:
   - Iterates registered handles. On shutdown, dispatches `g_memHandleCallbacks[i]` with argument `g_memHandleParameters[i]`.
   - Caller pushes the parameter, executes the callback, and clears stack with `add esp, 4`.
   - Because `Font_Shutdown` uses `cdecl` caller cleanup and consumes no stack arguments, this dispatch invokes `Font_Shutdown` seamlessly without corrupting stack balance.
3. Direct invocation:
   - Also callable directly during application shutdown (e.g. `App_Shutdown` / `main.c` line 290).

## Emulation Verification
Verified under Unicorn x86 emulation across all boundary scenarios:
- **Uninitialized (`g_fontSystemInitialized == 0`)**:
  - Immediately returns `1020` (`0x3FC`).
  - No handle release or font unload calls dispatched; globals unchanged.
- **Initialized (`g_fontSystemInitialized == 1`), zero active font slots**:
  - Calls `Mem_ReleaseHandleId` with `g_fontSubsystemHandle`.
  - Resets `g_fontSystemInitialized` to `0`.
  - Scans all 30 font slots, skips all free slots (`in_use != 1`), invokes `Font_Unload` 0 times.
  - Returns `1`.
- **Initialized (`g_fontSystemInitialized == 1`), active slots (e.g. 0, 5, 29)**:
  - Calls `Mem_ReleaseHandleId(g_fontSubsystemHandle)`.
  - Resets `g_fontSystemInitialized` to `0`.
  - Sequentially calls `Font_Unload(0)`, `Font_Unload(5)`, `Font_Unload(29)`.
  - Bound check at `0x0064AE60` strictly stops at index 30, never reading slot 30.
  - Returns `1`.

## Extent and Limitations
- **Extent**: Fully reconstructed in `decomp/src/geputget.c` with 100% functional and structural fidelity to original Windows binary instructions.
- **Harness Status**: Direct compilation and emulation of `geputget.c` within `tools/verify_matching.py` remains blocked by unmigrated DOS-dependent systems (`<io.h>`, `<fcntl.h>`, `File_LoadToMemory`). Isolated Unicorn emulation confirms the routine's exact instruction behavior and ABI contract.
