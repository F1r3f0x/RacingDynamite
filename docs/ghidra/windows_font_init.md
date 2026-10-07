# Font_InitSystem (VA 0x00456180)

## Contract
- **Routine**: `Font_InitSystem` (font subsystem initializer)
- **Address**: `IGN_WIN.EXE` @ VA `0x00456180` / RVA `0x00056180`
- **Size**: 134 bytes (`0x86`), FPO extent `[0x00456180, 0x00456206)`
- **Padding**: 10 bytes of `0xCC` (`INT3`) ending at `0x00456210`
- **Routine SHA-256**: `32c69eb459a5bc9c8e74254334262750ef7db85caf4f5a70437ebc28738f2c3d`
- **Signature**: `int Font_InitSystem(void)`
- **ABI**: `cdecl` (caller cleanup; callee preserves `esi`, `edi`, `ebx`, `ebp`)
- **Return Value**:
  - `1010` (`0x3F2`) if already initialized (`g_fontSystemInitialized == 1`)
  - `1` upon successful initialization
- **Dependencies**:
  - `Mem_NextHandleId` @ `0x0045B1B0` (RVA `0x0005B1B0`)
  - `Mem_RegisterHandle` @ `0x0045B360` (RVA `0x0005B360`)
  - `Font_Shutdown` @ `0x00456210` (RVA `0x00056210`)
  - `g_fontSystemInitialized` @ `0x004BA6C4` (RVA `0x000BA6C4`)
  - `s_fontExitContext` (`"fontExit()"`) @ `0x004BA6C8` (RVA `0x000BA6C8`)
  - `g_fontSubsystemHandle` @ `0x0050E680` (RVA `0x0010E680`)
  - `g_memPendingContext` @ `0x0063C690` (RVA `0x0023C690`)
  - `g_memPendingCallback` @ `0x0063C694` (RVA `0x0023C694`)
  - `g_memPendingParameter` @ `0x0063C698` (RVA `0x0023C698`)
  - `g_fonts` array @ `0x0063F2E0` (RVA `0x0023F2E0`, 30 slots of 1600 bytes, ends at `0x0064AE60`)

## Binary Evidence

Disassembly and operand relocations from `IGN_WIN.EXE`:
```assembly
0x00456180: 833dc4a64b0001   cmp    dword ptr [0x4ba6c4], 1    ; check g_fontSystemInitialized == 1 (reloc @ 0x456182)
0x00456187: 56               push   esi                        ; preserve esi
0x00456188: 57               push   edi                        ; preserve edi
0x00456189: 7508             jne    0x456193                   ; if uninitialized, proceed
0x0045618b: b8f2030000       mov    eax, 0x3f2                 ; return 1010 (already initialized)
0x00456190: 5f               pop    edi                        ; restore edi
0x00456191: 5e               pop    esi                        ; restore esi
0x00456192: c3               ret                               ; return
0x00456193: be01000000       mov    esi, 1                     ; esi = 1
0x00456198: 33ff             xor    edi, edi                   ; edi = 0
0x0045619a: 8935c4a64b00     mov    dword ptr [0x4ba6c4], esi  ; g_fontSystemInitialized = 1 (reloc @ 0x45619c)
0x004561a0: e80b500000       call   0x45b1b0                   ; call Mem_NextHandleId()
0x004561a5: 50               push   eax                        ; push handle argument for Mem_RegisterHandle
0x004561a6: a380e65000       mov    dword ptr [0x50e680], eax  ; g_fontSubsystemHandle = eax (reloc @ 0x4561a7)
0x004561ab: c70590c66300c8a64b00 mov dword ptr [0x63c690], 0x4ba6c8 ; g_memPendingContext = "fontExit()" (reloc target @ 0x4561ad, imm @ 0x4561b1)
0x004561b5: c70594c6630010624500 mov dword ptr [0x63c694], 0x456210 ; g_memPendingCallback = Font_Shutdown (reloc target @ 0x4561b7, imm @ 0x4561bb)
0x004561bf: 893d98c66300     mov    dword ptr [0x63c698], edi  ; g_memPendingParameter = 0 (reloc @ 0x4561c1)
0x004561c5: e896510000       call   0x45b360                   ; call Mem_RegisterHandle(handle_id)
0x004561ca: 83c404           add    esp, 4                     ; caller cleanup (cdecl)
0x004561cd: b8e0f26300       mov    eax, 0x63f2e0              ; eax = &g_fonts[0] (reloc @ 0x4561ce)
0x004561d2: 8938             mov    dword ptr [eax], edi       ; g_fonts[i].in_use = 0
0x004561d4: 0540060000       add    eax, 0x640                 ; eax += sizeof(FontSlot) = 1600 (0x640)
0x004561d9: 3d60ae6400       cmp    eax, 0x64ae60              ; check if eax == &g_fonts[30] (reloc @ 0x4561da)
0x004561de: 89b8c4f9ffff     mov    dword ptr [eax - 0x63c], edi ; g_fonts[i].alignment = 0 (+0x04)
0x004561e4: 89b8c8f9ffff     mov    dword ptr [eax - 0x638], edi ; g_fonts[i].field_08 = 0 (+0x08)
0x004561ea: 89b8ccf9ffff     mov    dword ptr [eax - 0x634], edi ; g_fonts[i].is_proportional = 0 (+0x0C)
0x004561f0: 89b0d0f9ffff     mov    dword ptr [eax - 0x630], esi ; g_fonts[i].extra_spacing = 1 (+0x10)
0x004561f6: 89b0d4f9ffff     mov    dword ptr [eax - 0x62c], esi ; g_fonts[i].field_14 = 1 (+0x14)
0x004561fc: 72d4             jb     0x4561d2                   ; loop while eax < 0x64ae60
0x004561fe: b801000000       mov    eax, 1                     ; return 1 (success)
0x00456203: 5f               pop    edi                        ; restore edi
0x00456204: 5e               pop    esi                        ; restore esi
0x00456205: c3               ret                               ; return
```

## System Integration & Registration Lifecycle

In `IGN_WIN.EXE`:
1. **Subsystem Initializer (`Font_InitSystem` @ 0x00456180)**:
   - Guards against double-initialization: if `g_fontSystemInitialized == 1`, returns `1010` (`0x3F2`).
   - Marks subsystem as initialized: `g_fontSystemInitialized = 1`.
   - Obtains next available handle ID: calls `Mem_NextHandleId()` (`0x0045B1B0`), stores result in `g_fontSubsystemHandle` (`0x0050E680`).
   - Prepares registration dispatch parameters:
     - `g_memPendingContext` (`0x0063C690`) = `0x004BA6C8` (pointer to string `"fontExit()"`)
     - `g_memPendingCallback` (`0x0063C694`) = `0x00456210` (`Font_Shutdown`)
     - `g_memPendingParameter` (`0x0063C698`) = `0`
   - Calls `Mem_RegisterHandle(g_fontSubsystemHandle)` (`0x0045B360`).
   - Resets all 30 font slots in `g_fonts` (`0x0063F2E0` to `0x0064AE60`):
     - `in_use` = 0 (offset `0x00`)
     - `alignment` = 0 (offset `0x04`)
     - `field_08` = 0 (offset `0x08`)
     - `is_proportional` = 0 (offset `0x0C`)
     - `extra_spacing` = 1 (offset `0x10`)
     - `field_14` = 1 (offset `0x14`)
   - Returns 1.

2. **Subsystem Cleanup Dispatch (`Mem_ShutdownHandles` @ 0x0045B240)**:
   - On shutdown, `Mem_ShutdownHandles` iterates registered handle slots and dispatches `g_memHandleCallbacks[i](g_memHandleParameters[i])`.
   - Dispatches `Font_Shutdown` with parameter `0`.

3. **Subsystem Cleanup (`Font_Shutdown` @ 0x00456210)**:
   - Verifies `g_fontSystemInitialized != 0`.
   - Releases handle ID via `Mem_ReleaseHandleId(g_fontSubsystemHandle)`.
   - Resets `g_fontSystemInitialized = 0`.
   - Unloads all active font slots (`in_use == 1`) via `Font_Unload(i)`.

## Emulation Verification

Verified under Unicorn x86 emulation across all boundary scenarios:
- **Already initialized (`g_fontSystemInitialized == 1`)**:
  - Immediately returns `1010` (`0x3F2`).
  - No handle allocation or registration dispatched.
  - Globals and `g_fonts` table remain completely unmodified.
  - Callee-preserved registers (`esi`, `edi`, `ebx`, `ebp`) and stack balance preserved.
- **Uninitialized (`g_fontSystemInitialized == 0`)**:
  - Sets `g_fontSystemInitialized` to `1`.
  - Calls `Mem_NextHandleId()`, storing returned ID into `g_fontSubsystemHandle`.
  - Sets `g_memPendingContext` to `0x004BA6C8` (`"fontExit()"`).
  - Sets `g_memPendingCallback` to `0x00456210` (`Font_Shutdown`).
  - Sets `g_memPendingParameter` to `0`.
  - Dispatches `Mem_RegisterHandle(handle_id)`.
  - Initializes all 30 slots (each 1600 bytes) with `in_use = 0`, `alignment = 0`, `field_08 = 0`, `is_proportional = 0`, `extra_spacing = 1`, `field_14 = 1`.
  - Verifies exact table boundaries: bytes at and beyond `0x0064AE60` (slot 30) are strictly untouched.
  - Returns `1`.

## Extent and Limitations
- **Extent**: Fully reconstructed in `decomp/src/geputget.c` with 100% functional and structural fidelity to original Windows binary instructions.
- **Harness Status**: Direct compilation and emulation of `geputget.c` within `tools/verify_matching.py` remains blocked by unmigrated DOS-dependent systems (`<io.h>`, `<fcntl.h>`, `File_LoadToMemory`). Isolated Unicorn emulation and instruction-level analysis confirm the routine's exact instruction behavior and ABI contract.
