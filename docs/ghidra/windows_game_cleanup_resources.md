# Authentic Windows Game Cleanup Resources (Game_CleanupResources)

## Target Identification

- **Target Executable**: `Ignition/Ignition/IGN_WIN.EXE`
- **SHA-256**: `7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`
- **Symbol Name**: `Game_CleanupResources`
- **Virtual Address**: `0x00420B70`
- **RVA**: `0x00020B70`
- **Byte Size**: 130 bytes
- **Routine SHA-256**: `f2025078735e3667dfcde47cb6da7f80639335d9865dbdd8f894ef24566ac291`
- **FPO Record**: `(130, 0, 0, 257)` -> `cbProcSize = 130`, `cdwLocals = 0`, `cdwParams = 0`, `cbProlog = 0`, `cbRegs = 1` (`esi`), `fHasSEH = 0`, `fUseBP = 0`, `cbFrame = 1`.
- **Calling Convention**: `__cdecl` (`void -> int32_t`)
- **Module Ownership**: `platform` (`decomp/src/platform.c`, `decomp/include/platform.h`)

## Authentic Disassembly

```assembly
0x00420B70: push     esi
0x00420B71: call     0x457fc0              ; Audio_StopCD()
0x00420B76: mov      esi, 0x54f960         ; esi = &g_gameSysGfxHandles[0]
0x00420B7B: call     0x4580a0              ; Audio_CloseCD()
0x00420B80: call     0x459d30              ; Audio_ShutdownSound()
0x00420B85: call     0x401840              ; Menu_FreeBackground()
0x00420B8A: mov      eax, dword ptr [esi]  ; handle = *esi
0x00420B8C: add      esi, 4                ; esi++
0x00420B8F: push     eax                   ; arg1 = handle
0x00420B90: push     0                     ; arg0 = NULL descriptor
0x00420B92: call     0x456d40              ; Gfx_HandleOpDispatch(NULL, handle)
0x00420B97: add      esp, 8
0x00420B9A: cmp      esi, 0x54f970         ; loop over 4 handles (0x54f960..0x54f970)
0x00420BA0: jb       0x420b8a
0x00420BA2: mov      eax, dword ptr [0x552d84] ; handle = g_gameMouseCursorHandle
0x00420BA7: push     eax                   ; arg1 = handle
0x00420BA8: push     0                     ; arg0 = NULL descriptor
0x00420BAA: call     0x456d40              ; Gfx_HandleOpDispatch(NULL, g_gameMouseCursorHandle)
0x00420BAF: add      esp, 8
0x00420BB2: mov      ecx, dword ptr [0x49498c] ; ecx = g_gameTotalSeconds
0x00420BB8: test     ecx, ecx
0x00420BBA: jne      0x420bc1
0x00420BBC: mov      ecx, 1                ; if (seconds == 0) seconds = 1;
0x00420BC1: mov      eax, dword ptr [0x494988] ; eax = g_gameTotalFrames
0x00420BC6: mov      dword ptr [0x49498c], ecx ; g_gameTotalSeconds = ecx
0x00420BCC: cdq                            ; sign-extend eax into edx:eax
0x00420BCD: idiv     ecx                   ; eax = totalFrames / seconds
0x00420BCF: push     eax                   ; arg1 = avg_framerate
0x00420BD0: push     0x499174              ; arg0 = "Ignition has been shutdown normaly. Average framerate:%d\n"
0x00420BD5: call     0x45b5c0              ; Log_Printf(...)
0x00420BDA: add      esp, 8
0x00420BDD: call     0x45b590              ; Log_Shutdown()
0x00420BE2: mov      eax, dword ptr [0x5530e8] ; eax = g_gameMainPoolId
0x00420BE7: push     eax                   ; arg0 = pool_id
0x00420BE8: call     0x45b080              ; Mem_DestroyPool(g_gameMainPoolId)
0x00420BED: add      esp, 4
0x00420BF0: pop      esi
0x00420BF1: ret
```

## Functional Behavior

1. **Audio Teardown**:
   - Calls `Audio_StopCD()` (`0x00457FC0`).
   - Calls `Audio_CloseCD()` (`0x004580A0`).
   - Calls `Audio_ShutdownSound()` (`0x00459D30`).

2. **Menu Teardown**:
   - Calls `Menu_FreeBackground()` (`0x00401840`).

3. **Sprite / Cursor Handle Release**:
   - Iterates through the 4 entries in `g_gameSysGfxHandles` (`0x0054F960` to `0x0054F970`), calling `Gfx_HandleOpDispatch(NULL, handle)`.
   - Releases the extra handle `g_gameMouseCursorHandle` (`0x00552D84`) via `Gfx_HandleOpDispatch(NULL, handle)`.

4. **Shutdown Logging & Statistics**:
   - Reads `g_gameTotalSeconds` (`0x0049498C`). If zero, guards against divide-by-zero by clamping to 1 and updating `g_gameTotalSeconds = 1`.
   - Divides `g_gameTotalFrames` (`0x00494988`) by `g_gameTotalSeconds` to compute the average framerate.
   - Calls `Log_Printf("Ignition has been shutdown normaly. Average framerate:%d\n", avg_framerate)` (`0x0045B5C0`).
   - Calls `Log_Shutdown()` (`0x0045B590`).

5. **Pool Cleanup**:
   - Destroys the main game memory pool `g_gameMainPoolId` (`0x005530E8`) via `Mem_DestroyPool(g_gameMainPoolId)` (`0x0045B080`).
   - Returns the status from `Mem_DestroyPool` (or 0).
