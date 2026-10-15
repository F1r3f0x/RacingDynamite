# Authentic Windows CD Audio Interface Routines

## Target Identification

- **Target Executable**: `Ignition/Ignition/IGN_WIN.EXE`
- **SHA-256**: `7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`
- **Module Ownership**: `audio` (`decomp/src/audio.c`, `decomp/include/audio.h`)
- **Shared State**: `char g_cdAudioReturnString[256]; /* VA 0x0050EF70 */`

### Routines

| Symbol Name | RVA | VA | Byte Size | SHA-256 | FPO Record | Calling Convention |
|-------------|-----|----|-----------|---------|------------|--------------------|
| `Audio_StopCD` | `0x00057FC0` | `0x00457FC0` | 31 | `46ebcedec71524bae04d1209584d1d5095159a057c8b44e3521809efe80d5bc2` | `(31, 0, 0, 0)` | `__cdecl (void) -> int32_t` |
| `Audio_OpenCD` | `0x00058040` | `0x00458040` | 89 | `b827c0b4fbab3f4a78aaa85396e801e9500f94ba82313a6090a058627d6788a4` | `(89, 0, 0, 257)` | `__cdecl (void) -> int32_t` |
| `Audio_CloseCD` | `0x000580A0` | `0x004580A0` | 31 | `526162ad95278c952596a9ca0b48853d19d392abe5058e4f8548877b048ee81a` | `(31, 0, 0, 0)` | `__cdecl (void) -> int32_t` |
| `Audio_EjectCD` | `0x000580C0` | `0x004580C0` | 31 | `4b75cc2471d34550330b8b52aeab8e325185db3f0f50632fb66a110769e847de` | `(31, 0, 0, 0)` | `__cdecl (void) -> int32_t` |
| `Audio_ShutdownSound` | `0x00059D30` | `0x00459D30` | 3 | `4bc724f3b1d0caf4fe369c18cba3102e6c4ea057f63fe1587e3973134a7f755e` | `(3, 0, 0, 0)` | `__cdecl (void) -> int32_t` |

## Authentic Disassembly

### 1. `Audio_StopCD` (0x00457FC0)

```assembly
0x00457FC0: push     0                     ; hwndCallback = NULL
0x00457FC2: push     0x100                 ; cchReturn = 256
0x00457FC7: push     0x50ef70              ; lpszReturnString = g_cdAudioReturnString
0x00457FCC: push     0x4ba934              ; lpszCommand = "stop cdaudio"
0x00457FD1: call     dword ptr [0x64c468]  ; mciSendStringA
0x00457FD7: cmp      eax, 1                ; carry flag set if eax == 0
0x00457FDA: sbb      eax, eax              ; eax = -1 if carry, else 0
0x00457FDC: neg      eax                   ; eax = 1 if success (0), else 0
0x00457FDE: ret
```

### 2. `Audio_OpenCD` (0x00458040)

```assembly
0x00458040: push     esi
0x00458041: push     0
0x00458043: push     0x100
0x00458048: mov      esi, dword ptr [0x64c468] ; mciSendStringA
0x0045804E: push     0x50ef70              ; g_cdAudioReturnString
0x00458053: push     0x4ba97c              ; "open cdaudio"
0x00458058: call     esi
0x0045805A: test     eax, eax
0x0045805C: je       0x458062
0x0045805E: xor      eax, eax
0x00458060: pop      esi
0x00458061: ret
0x00458062: push     0
0x00458064: push     0x100
0x00458069: push     0x50ef70
0x0045806E: push     0x4ba8dc              ; "set cdaudio audio all on"
0x00458073: call     esi
0x00458075: test     eax, eax
0x00458077: je       0x45807d
0x00458079: xor      eax, eax
0x0045807B: pop      esi
0x0045807C: ret
0x0045807D: push     0
0x0045807F: push     0x100
0x00458084: push     0x50ef70
0x00458089: push     0x4ba960              ; "set cdaudio time format msf"
0x0045808E: call     esi
0x00458090: pop      esi
0x00458091: cmp      eax, 1
0x00458094: sbb      eax, eax
0x00458096: neg      eax
0x00458098: ret
```

### 3. `Audio_CloseCD` (0x004580A0)

```assembly
0x004580A0: push     0
0x004580A2: push     0x100
0x004580A7: push     0x50ef70
0x004580AC: push     0x4ba98c              ; "close cdaudio"
0x004580B1: call     dword ptr [0x64c468]  ; mciSendStringA
0x004580B7: cmp      eax, 1
0x004580BA: sbb      eax, eax
0x004580BC: neg      eax
0x004580BE: ret
```

### 4. `Audio_EjectCD` (0x004580C0)

```assembly
0x004580C0: push     0
0x004580C2: push     0x100
0x004580C7: push     0x50ef70
0x004580CC: push     0x4ba99c              ; "set cdaudio door open"
0x004580D1: call     dword ptr [0x64c468]  ; mciSendStringA
0x004580D7: cmp      eax, 1
0x004580DA: sbb      eax, eax
0x004580DC: neg      eax
0x004580DE: ret
```

### 5. `Audio_ShutdownSound` (0x00459D30)

```assembly
0x00459D30: xor      eax, eax
0x00459D32: ret
```

## Functional Behavior

1. **Audio_StopCD**:
   - Sends the MCI command `"stop cdaudio"` using `mciSendStringA`.
   - Returns 1 if `mciSendStringA` succeeded (returned 0), otherwise 0.

2. **Audio_OpenCD**:
   - Opens the CD device via `"open cdaudio"`. Returns 0 on error.
   - Enables audio channels via `"set cdaudio audio all on"`. Returns 0 on error.
   - Sets the time format to minutes-seconds-frames (MSF) via `"set cdaudio time format msf"`.
   - Returns 1 if all steps succeeded, otherwise 0.

3. **Audio_CloseCD**:
   - Closes the CD device via `"close cdaudio"`.
   - Returns 1 on success (return code 0), otherwise 0.

4. **Audio_EjectCD**:
   - Ejects the CD drive tray via `"set cdaudio door open"`.
   - Returns 1 on success, otherwise 0.

5. **Audio_ShutdownSound**:
   - Sound system shutdown leaf stub; returns 0.
