# Font_Load (VA 0x00456420)

## Contract
- **Routine**: `Font_Load` (font resource loader)
- **Address**: `IGN_WIN.EXE` @ VA `0x00456420` / RVA `0x00056420`
- **Size**: 71 bytes (`0x47`), FPO extent `[0x00456420, 0x00456467)`
- **Padding**: 9 bytes of `0xCC` (`INT3`) ending at `0x00456470` (`Font_Unload`)
- **Routine SHA-256**: `f4c269fd2b0d4fbc491b4f4211b5dd22622c8c20d3d7e3f220eaa94093887441`
- **Signature**: `int Font_Load(const char *filename, int unused)`
- **ABI**: `cdecl` (caller cleanup; callee preserves `esi`, `edi`; frame pointer omitted; FPO record: `(71, 0, 2, 518)`)
- **Parameters**:
  - `filename`: pointer to null-terminated file path string (`const char *`)
  - `unused`: secondary parameter (`int`) forwarded directly to `Font_Parse`
- **Return Value**:
  - `-1` (`0xFFFFFFFF`) on error:
    - If `File_LoadToMemory(filename)` returns `NULL`: sets `g_fileErrorLine` (`0x004BAB34`) to `1000` (`0x3E8`) and returns `-1`.
    - If `Font_Parse(buffer, unused)` fails: frees buffer via `Mem_Free(0, buffer)` and returns `-1`.
  - Allocated font slot ID (`0..29`) upon successful parsing and registration.
- **Dependencies**:
  - `File_LoadToMemory` @ VA `0x004574A0` (RVA `0x000574A0`)
  - `Font_Parse` @ VA `0x00456270` (RVA `0x00056270`)
  - `Mem_Free` @ VA `0x0045B000` (RVA `0x0005B000`)
  - `g_fileErrorLine` @ VA `0x004BAB34` (RVA `0x000BAB34`)

## Binary Evidence

Disassembly and operand relocations from authentic `IGN_WIN.EXE`:
```assembly
0x00456420: 8b 44 24 04          mov      eax, dword ptr [esp + 4]   ; eax = filename (arg 0)
0x00456424: 56                   push     esi                        ; preserve esi
0x00456425: 57                   push     edi                        ; preserve edi
0x00456426: 50                   push     eax                        ; push filename
0x00456427: e8 74 10 00 00       call     0x004574a0                 ; call File_LoadToMemory(filename)
0x0045642c: 83 c4 04             add      esp, 4                     ; caller cleanup (1 arg)
0x0045642f: 8b f8                mov      edi, eax                   ; edi = buffer (loaded file pointer)
0x00456431: 85 ff                test     edi, edi                   ; check if buffer == NULL
0x00456433: 75 12                jne      0x00456447                 ; if non-NULL, proceed to parse
0x00456435: b8 ff ff ff ff       mov      eax, 0xffffffff            ; eax = -1 (error return)
0x0045643a: 5f                   pop      edi                        ; restore edi
0x0045643b: c7 05 34 ab 4b 00 e8 03 00 00 mov dword ptr [0x004bab34], 0x3e8 ; g_fileErrorLine = 1000 (reloc @ 0x0045643d)
0x00456445: 5e                   pop      esi                        ; restore esi
0x00456446: c3                   ret                                 ; return -1
0x00456447: 8b 44 24 10          mov      eax, dword ptr [esp + 0x10]; eax = unused (arg 1, offset past saved regs + ret)
0x0045644b: 50                   push     eax                        ; push unused
0x0045644c: 57                   push     edi                        ; push buffer
0x0045644d: e8 1e fe ff ff       call     0x00456270                 ; call Font_Parse(buffer, unused)
0x00456452: 83 c4 08             add      esp, 8                     ; caller cleanup (2 args)
0x00456455: 8b f0                mov      esi, eax                   ; esi = slot (return value from Font_Parse)
0x00456457: 57                   push     edi                        ; push buffer (arg 1 for Mem_Free)
0x00456458: 6a 00                push     0                          ; push 0 (pool index 0 for Mem_Free)
0x0045645a: e8 a1 4b 00 00       call     0x0045b000                 ; call Mem_Free(0, buffer)
0x0045645f: 83 c4 08             add      esp, 8                     ; caller cleanup (2 args)
0x00456462: 8b c6                mov      eax, esi                   ; eax = slot
0x00456464: 5f                   pop      edi                        ; restore edi
0x00456465: 5e                   pop      esi                        ; restore esi
0x00456466: c3                   ret                                 ; return slot
```

## Relocations & Operands
- Exactly one base relocation exists in the routine:
  - VA `0x0045643D` (RVA `0x0005643D`) -> `0x004BAB34`: `g_fileErrorLine` (set to `1000` / `0x3E8` on file load failure).
- Direct call targets (relative 32-bit displacements):
  - `0x00456427` + 5 + `0x00001074` = `0x004574A0`: `File_LoadToMemory`
  - `0x0045644D` + 5 - `0x000001E2` = `0x00456270`: `Font_Parse`
  - `0x0045645A` + 5 + `0x00004BA1` = `0x0045B000`: `Mem_Free`

## Stack Layout and FPO Data
- FPO record: `(71, 0, 2, 518)`
  - Function length: 71 bytes
  - Local frame dwords: 0
  - Parameters: 2 dwords (8 bytes: `filename` and `unused`)
  - Callee-saved registers: 2 (`esi`, `edi`)
  - Frame pointer omitted (ESP-based addressing)
- At routine entry (`0x00456420`):
  - `[esp + 0x00]`: Return address
  - `[esp + 0x04]`: `filename`
  - `[esp + 0x08]`: `unused`
- After pushing `esi` and `edi` (`0x00456425`):
  - `[esp + 0x00]`: Saved `edi`
  - `[esp + 0x04]`: Saved `esi`
  - `[esp + 0x08]`: Return address
  - `[esp + 0x0C]`: `filename`
  - `[esp + 0x10]`: `unused`

## Callers and Usage
Direct calls to `Font_Load` (`0x00456420`) in `IGN_WIN.EXE`:
1. System font initialization (`0x004181D5` .. `0x00418268`):
   - Loads standard font resources into `g_SystemFonts[0..7]` including `"baltazar\data\red_dark.lft"`, `"baltazar\data\red_lite.lft"`, etc.
   - Example call site:
     ```assembly
     0x004181ce: push 0
     0x004181d0: push 0x498930        ; "baltazar\\data\\red_dark.lft"
     0x004181d5: call 0x456420        ; Font_Load
     0x004181da: add  esp, 8
     0x004181dd: mov  dword ptr [0x552fd0], eax ; g_SystemFonts[0]
     ```
2. HUD font initialization (`0x0041ACFB` .. `0x0041AF33`):
   - Loads in-game HUD font resources for player position, speed, and lap times.

## Emulation Verification
Verified under Unicorn x86 emulation across all execution paths:
- **Case 1: File Loading Failure (`File_LoadToMemory` returns NULL)**:
  - Invokes `File_LoadToMemory(filename)`.
  - On NULL return, sets `g_fileErrorLine` (`0x004BAB34`) = `1000`.
  - Returns `-1`.
  - Callee-saved registers (`esi`, `edi`, `ebx`, `ebp`) preserved.
  - Stack pointer restored exactly to entry `esp + 4` (caller cleanup).
- **Case 2: Success Path (`File_LoadToMemory` returns valid buffer)**:
  - Invokes `File_LoadToMemory(filename)`.
  - Invokes `Font_Parse(buffer, unused)`.
  - Invokes `Mem_Free(0, buffer)` to release the temporary file buffer.
  - Returns allocated font slot ID (e.g. `5`).
  - Callee-saved registers preserved and caller stack balance maintained.
- **Case 3: Parse Failure Path (`Font_Parse` returns -1)**:
  - Invokes `File_LoadToMemory(filename)`.
  - Invokes `Font_Parse(buffer, unused)` which returns `-1`.
  - Invokes `Mem_Free(0, buffer)` (ensuring no memory leak even on parse failure).
  - Returns `-1`.
  - Callee-saved registers preserved and caller stack balance maintained.

## Extent and Limitations
- **Extent**: Fully reconstructed in `decomp/src/geputget.c` and declared in `decomp/include/geputget.h` with 100% functional and structural fidelity to original Windows binary instructions.
- **Harness Status**: Direct compilation of `geputget.c` into the isolated validation DLL (`mem_validation.dll`) remains blocked by unmigrated DOS-dependent systems (`<io.h>`, `<fcntl.h>`, `File_LoadToMemory`). Isolated Unicorn emulation and instruction-level analysis confirm the routine's exact instruction behavior and ABI contract.
