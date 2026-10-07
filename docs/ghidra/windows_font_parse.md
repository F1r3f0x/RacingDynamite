# Font_Parse (VA 0x00456270)

## Contract
- **Routine**: `Font_Parse` (font resource parser)
- **Address**: `IGN_WIN.EXE` @ VA `0x00456270` / RVA `0x00056270`
- **Size**: 429 bytes (`0x1AD`), FPO extent `[0x00456270, 0x0045641D)`
- **Padding**: 3 bytes of `0xCC` (`INT3`) ending at `0x00456420`
- **Routine SHA-256**: `cb425727822dfce52643cd3c6d418c7bb2495aed08be4ae0667f54ad6a67f549`
- **Signature**: `int Font_Parse(void *buffer, int unused)`
- **ABI**: `cdecl` (caller cleanup; callee preserves `ebx`, `esi`, `edi`, `ebp`; allocates `0x2C` bytes of local stack frame)
- **Return Value**:
  - `-1` (`0xFFFFFFFF`) on error (sets error code in `g_fileErrorLine` / `0x004BAB34`):
    - `1050` (`0x41A`): invalid magic (first 4 bytes do not match `"LFT\0"`)
    - `1060` (`0x424`): invalid version (`*(int16_t*)(buffer + 4) != 100`)
    - `1030` (`0x406`): table full (all 30 font slots in `g_fonts` are in use)
  - Allocated font slot ID `slot` (`0..29`) upon success
- **Dependencies**:
  - `Font_InitSystem` @ `0x00456180` (RVA `0x00056180`)
  - `0x00456D40` (wrapper around `Gfx_SpriteOp` at `0x0050EBC0`)
  - `g_fontSystemInitialized` @ `0x004BA6C4` (RVA `0x000BA6C4`)
  - `s_lftMagic` (`"LFT\0"`) @ `0x004BA6D4` (RVA `0x000BA6D4`)
  - `g_fileErrorLine` @ `0x004BAB34` (RVA `0x000BAB34`)
  - `g_fonts` array @ `0x0063F2E0` (RVA `0x0023F2E0`, 30 slots of 1600 bytes, ends at `0x0064AE60`)

## Binary Evidence

Disassembly and operand relocations from authentic `IGN_WIN.EXE`:
```assembly
0x00456270: 83 ec 2c                 sub    esp, 0x2c                  ; allocate 44 bytes local frame
0x00456273: 83 3d c4 a6 4b 00 00     cmp    dword ptr [0x4ba6c4], 0   ; test g_fontSystemInitialized (reloc @ 0x456275)
0x0045627a: 53                       push   ebx                        ; preserve ebx
0x0045627b: 56                       push   esi                        ; preserve esi
0x0045627c: 57                       push   edi                        ; preserve edi
0x0045627d: 55                       push   ebp                        ; preserve ebp
0x0045627e: 75 05                    jne    0x456285                   ; if initialized, skip
0x00456280: e8 fb fe ff ff           call   0x456180                   ; call Font_InitSystem()
0x00456285: bf d4 a6 4b 00           mov    edi, 0x4ba6d4              ; edi = "LFT\0" (reloc @ 0x456286)
0x0045628a: b9 04 00 00 00           mov    ecx, 4                     ; 4 bytes
0x0045628f: 8b 74 24 40              mov    esi, dword ptr [esp + 0x40]; esi = buffer (arg 0)
0x00456293: f3 a6                    repe cmpsb byte ptr [esi], es:[edi]; check magic == "LFT\0"
0x00456295: 74 17                    je     0x4562ae                   ; match -> proceed
0x00456297: b8 ff ff ff ff           mov    eax, 0xffffffff            ; return -1
0x0045629c: 5d                       pop    ebp                        ; restore ebp
0x0045629d: c7 05 34 ab 4b 00 1a 04 00 00 mov dword ptr [0x4bab34], 0x41a ; g_fileErrorLine = 1050 (reloc @ 0x45629f)
0x004562a7: 5f                       pop    edi                        ; restore edi
0x004562a8: 5e                       pop    esi                        ; restore esi
0x004562a9: 5b                       pop    ebx                        ; restore ebx
0x004562aa: 83 c4 2c                 add    esp, 0x2c                  ; release local frame
0x004562ad: c3                       ret                               ; return -1
0x004562ae: 8b 44 24 40              mov    eax, dword ptr [esp + 0x40]; eax = buffer
0x004562b2: 66 83 78 04 64           cmp    word ptr [eax + 4], 0x64   ; check version == 100
0x004562b7: 74 17                    je     0x4562d0                   ; match -> proceed
0x004562b9: b8 ff ff ff ff           mov    eax, 0xffffffff            ; return -1
0x004562be: 5d                       pop    ebp                        ; restore ebp
0x004562bf: c7 05 34 ab 4b 00 24 04 00 00 mov dword ptr [0x4bab34], 0x424 ; g_fileErrorLine = 1060 (reloc @ 0x4562c1)
0x004562c9: 5f                       pop    edi                        ; restore edi
0x004562ca: 5e                       pop    esi                        ; restore esi
0x004562cb: 5b                       pop    ebx                        ; restore ebx
0x004562cc: 83 c4 2c                 add    esp, 0x2c                  ; release local frame
0x004562cf: c3                       ret                               ; return -1
0x004562d0: 33 c9                    xor    ecx, ecx                   ; slot index i = 0
0x004562d2: b8 e0 f2 63 00           mov    eax, 0x63f2e0              ; eax = &g_fonts[0] (reloc @ 0x4562d3)
0x004562d7: 83 38 00                 cmp    dword ptr [eax], 0         ; check g_fonts[i].in_use == 0
0x004562da: 74 0d                    je     0x4562e9                   ; free slot found -> break
0x004562dc: 05 40 06 00 00           add    eax, 0x640                 ; eax += sizeof(FontSlot) = 1600
0x004562e1: 41                       inc    ecx                        ; i++
0x004562e2: 3d 60 ae 64 00           cmp    eax, 0x64ae60              ; check if past slot 29 (reloc @ 0x4562e3)
0x004562e7: 72 ee                    jb     0x4562d7                   ; loop while eax < 0x64ae60
0x004562e9: 83 f9 1e                 cmp    ecx, 0x1e                  ; check if i == 30 (all slots full)
0x004562ec: 75 17                    jne    0x456305                   ; slot available -> proceed
0x004562ee: b8 ff ff ff ff           mov    eax, 0xffffffff            ; return -1
0x004562f3: 5d                       pop    ebp                        ; restore ebp
0x004562f4: c7 05 34 ab 4b 00 06 04 00 00 mov dword ptr [0x4bab34], 0x406 ; g_fileErrorLine = 1030 (reloc @ 0x4562f6)
0x004562fe: 5f                       pop    edi                        ; restore edi
0x004562ff: 5e                       pop    esi                        ; restore esi
0x00456300: 5b                       pop    ebx                        ; restore ebx
0x00456301: 83 c4 2c                 add    esp, 0x2c                  ; release local frame
0x00456304: c3                       ret                               ; return -1
0x00456305: 8d 04 89                 lea    eax, [ecx + ecx*4]         ; eax = slot * 5
0x00456308: 89 4c 24 18              mov    dword ptr [esp + 0x18], ecx; local_slot = slot
0x0045630c: 8b 4c 24 40              mov    ecx, dword ptr [esp + 0x40]; ecx = buffer
0x00456310: 8d 04 80                 lea    eax, [eax + eax*4]         ; eax = slot * 25
0x00456313: c1 e0 06                 shl    eax, 6                     ; eax = slot * 1600 (slot byte offset)
0x00456316: 66 8b 51 06              mov    dx, word ptr [ecx + 6]     ; dx = *(uint16_t*)(buffer + 6)
0x0045631a: 89 44 24 10              mov    dword ptr [esp + 0x10], eax; local_slot_offset = eax
0x0045631e: 66 89 90 f8 f2 63 00     mov    word ptr [eax + 0x63f2f8], dx ; g_fonts[slot].field_18 = dx (reloc @ 0x456321)
0x00456325: 8d b9 8c 06 00 00        lea    edi, [ecx + 0x68c]         ; edi = buffer + 0x68c (widths source)
0x0045632b: 66 8b 51 08              mov    dx, word ptr [ecx + 8]     ; dx = *(uint16_t*)(buffer + 8)
0x0045632f: 8b f7                    mov    esi, edi                   ; esi = widths source pointer
0x00456331: 66 89 90 fa f2 63 00     mov    word ptr [eax + 0x63f2fa], dx ; g_fonts[slot].height = dx (reloc @ 0x456334)
0x00456338: 66 8b 51 0a              mov    dx, word ptr [ecx + 0xa]   ; dx = *(uint16_t*)(buffer + 10)
0x0045633c: 8d 88 60 f7 63 00        lea    ecx, [eax + 0x63f760]      ; ecx = &g_fonts[slot].widths[0] (reloc @ 0x45633e)
0x00456342: 66 89 90 fc f2 63 00     mov    word ptr [eax + 0x63f2fc], dx ; g_fonts[slot].spacing = dx (reloc @ 0x456345)
0x00456349: b8 e0 00 00 00           mov    eax, 0xe0                  ; 224 glyphs
0x0045634e: 66 8b 16                 mov    dx, word ptr [esi]         ; load width
0x00456351: 83 c6 02                 add    esi, 2                     ; advance source width ptr
0x00456354: 66 89 11                 mov    word ptr [ecx], dx         ; store to g_fonts[slot].widths[i]
0x00456357: 83 c1 02                 add    ecx, 2                     ; advance dest width ptr
0x0045635a: 48                       dec    eax                        ; count--
0x0045635b: 75 f1                    jne    0x45634e                   ; copy all 224 glyph widths
0x0045635d: 8b 44 24 40              mov    eax, dword ptr [esp + 0x40]; eax = buffer
0x00456361: 33 f6                    xor    esi, esi                   ; glyph index i = 0
0x00456363: 05 4c 08 00 00           add    eax, 0x84c                 ; eax = buffer + 0x84c (pixel_base)
0x00456368: 8b 6c 24 10              mov    ebp, dword ptr [esp + 0x10]; ebp = slot_offset
0x0045636c: 81 c5 e0 f3 63 00        add    ebp, 0x63f3e0              ; ebp = &g_fonts[slot].glyph_handles[0] (reloc @ 0x45636e)
0x00456372: 8b 5c 24 40              mov    ebx, dword ptr [esp + 0x40]; ebx = buffer
0x00456376: 83 c3 0c                 add    ebx, 0xc                   ; ebx = buffer + 12 (offsets table)
0x00456379: 89 44 24 14              mov    dword ptr [esp + 0x14], eax; local_pixel_base = eax
0x0045637d: 8b 4c 24 10              mov    ecx, dword ptr [esp + 0x10]; ecx = slot_offset
0x00456381: 83 3b ff                 cmp    dword ptr [ebx], -1        ; offset == -1? (missing glyph)
0x00456384: 75 11                    jne    0x456397                   ; if present, register sprite
0x00456386: c6 84 31 fe f2 63 00 00  mov    byte ptr [ecx + esi + 0x63f2fe], 0 ; g_fonts[slot].glyph_present[i] = 0 (reloc @ 0x456389)
0x0045638e: c7 45 00 00 00 00 00     mov    dword ptr [ebp], 0         ; g_fonts[slot].glyph_handles[i] = NULL
0x00456395: eb 56                    jmp    0x4563ed                   ; next glyph
0x00456397: 8b 54 24 40              mov    edx, dword ptr [esp + 0x40]; edx = buffer
0x0045639b: 6a 00                    push   0                          ; push op = 0
0x0045639d: c6 84 31 fe f2 63 00 01  mov    byte ptr [ecx + esi + 0x63f2fe], 1 ; g_fonts[slot].glyph_present[i] = 1 (reloc @ 0x4563a0)
0x004563a5: 0f bf 07                 movsx  eax, word ptr [edi]        ; eax = width
0x004563a8: 89 44 24 24              mov    dword ptr [esp + 0x24], eax; desc.width = width
0x004563ac: 0f bf 4a 0a              movsx  ecx, word ptr [edx + 0xa]  ; ecx = spacing (buffer + 10)
0x004563b0: 8d 54 24 20              lea    edx, [esp + 0x20]          ; edx = &desc
0x004563b4: 89 4c 24 28              mov    dword ptr [esp + 0x28], ecx; desc.height = spacing
0x004563b8: c7 44 24 2c 00 00 00 00  mov    dword ptr [esp + 0x2c], 0  ; desc.field_0c = 0
0x004563c0: 8b 03                    mov    eax, dword ptr [ebx]       ; eax = offset
0x004563c2: 52                       push   edx                        ; push &desc
0x004563c3: 03 44 24 1c              add    eax, dword ptr [esp + 0x1c]; eax = pixel_base + offset
0x004563c7: 89 44 24 34              mov    dword ptr [esp + 0x34], eax; desc.pixels = eax
0x004563cb: 0f bf 0f                 movsx  ecx, word ptr [edi]        ; ecx = width
0x004563ce: c7 44 24 3c 00 00 00 00  mov    dword ptr [esp + 0x3c], 0  ; desc.field_18 = 0
0x004563d6: c7 44 24 40 00 00 00 00  mov    dword ptr [esp + 0x40], 0  ; desc.field_1c = 0
0x004563de: 89 4c 24 38              mov    dword ptr [esp + 0x38], ecx; desc.stride = width
0x004563e2: e8 59 09 00 00           call   0x456d40                   ; call Gfx_SpriteOp(&desc, 0)
0x004563e7: 83 c4 08                 add    esp, 8                     ; caller cleanup (cdecl)
0x004563ea: 89 45 00                 mov    dword ptr [ebp], eax       ; g_fonts[slot].glyph_handles[i] = handle
0x004563ed: 83 c7 02                 add    edi, 2                     ; edi += 2 (next width in buffer)
0x004563f0: 83 c5 04                 add    ebp, 4                     ; ebp += 4 (next glyph_handle ptr)
0x004563f3: 83 c3 04                 add    ebx, 4                     ; ebx += 4 (next offset in buffer)
0x004563f6: 46                       inc    esi                        ; i++
0x004563f7: 81 fe e0 00 00 00        cmp    esi, 0xe0                  ; cmp i, 224
0x004563fd: 0f 8c 7a ff ff ff        jl     0x45637d                   ; loop while i < 224
0x00456403: 8b 4c 24 10              mov    ecx, dword ptr [esp + 0x10]; ecx = slot_offset
0x00456407: 8b 44 24 18              mov    eax, dword ptr [esp + 0x18]; eax = slot (return value)
0x0045640b: 5d                       pop    ebp                        ; restore ebp
0x0045640c: 5f                       pop    edi                        ; restore edi
0x0045640d: c7 81 e0 f2 63 00 01 00 00 00 mov dword ptr [ecx + 0x63f2e0], 1 ; g_fonts[slot].in_use = 1 (reloc @ 0x45640f)
0x00456417: 5e                       pop    esi                        ; restore esi
0x00456418: 5b                       pop    ebx                        ; restore ebx
0x00456419: 83 c4 2c                 add    esp, 0x2c                  ; release local frame
0x0045641c: c3                       ret                               ; return slot
```

## Relocations & Operands
- `0x00456275` -> `0x004BA6C4`: `g_fontSystemInitialized`
- `0x00456286` -> `0x004BA6D4`: string literal `"LFT\0"`
- `0x0045629F` -> `0x004BAB34`: `g_fileErrorLine` (set to `1050` / `0x41A` on magic mismatch)
- `0x004562C1` -> `0x004BAB34`: `g_fileErrorLine` (set to `1060` / `0x424` on version mismatch)
- `0x004562D3` -> `0x0063F2E0`: `g_fonts` (start of font array)
- `0x004562E3` -> `0x0064AE60`: `&g_fonts[30]` (array bounds limit: 30 slots * 1600 bytes)
- `0x004562F6` -> `0x004BAB34`: `g_fileErrorLine` (set to `1030` / `0x406` on table full)
- `0x00456321` -> `0x0063F2F8`: `g_fonts[0].field_18` (`0x63F2E0 + 0x18`)
- `0x00456334` -> `0x0063F2FA`: `g_fonts[0].height` (`0x63F2E0 + 0x1A`)
- `0x0045633E` -> `0x0063F760`: `g_fonts[0].widths[0]` (`0x63F2E0 + 0x480`)
- `0x00456345` -> `0x0063F2FC`: `g_fonts[0].spacing` (`0x63F2E0 + 0x1C`)
- `0x0045636E` -> `0x0063F3E0`: `g_fonts[0].glyph_handles[0]` (`0x63F2E0 + 0x100`)
- `0x00456389` -> `0x0063F2FE`: `g_fonts[0].glyph_present[0]` (`0x63F2E0 + 0x1E`)
- `0x004563A0` -> `0x0063F2FE`: `g_fonts[0].glyph_present[0]` (`0x63F2E0 + 0x1E`)
- `0x0045640F` -> `0x0063F2E0`: `g_fonts[0].in_use` (`0x63F2E0 + 0x00`)

## Data Layout & Sprite Registration
1. **LFT File Format**:
   - `+0x00`: 4 bytes magic (`"LFT\0"`, matched via 4-byte `repe cmpsb`)
   - `+0x04`: 2 bytes version (must equal 100)
   - `+0x06`: 2 bytes `field_18`
   - `+0x08`: 2 bytes `height`
   - `+0x0A`: 2 bytes `spacing`
   - `+0x0C`: 224 x 4-byte int32 glyph offsets (relative to `buffer + 0x84c`). Value `-1` means absent glyph.
   - `+0x68C`: 224 x 2-byte uint16 glyph widths (448 bytes)
   - `+0x84C`: pixel data buffer base (`pixel_base`)
2. **`FontSlot` Windows Natural Layout (1600 bytes)**:
   - `+0x00`: `in_use` (int32)
   - `+0x04`: `alignment` (int32)
   - `+0x08`: `field_08` (int32)
   - `+0x0C`: `is_proportional` (int32)
   - `+0x10`: `extra_spacing` (int32)
   - `+0x14`: `field_14` (int32)
   - `+0x18`: `field_18` (uint16)
   - `+0x1A`: `height` (uint16)
   - `+0x1C`: `spacing` (uint16)
   - `+0x1E`: `glyph_present[224]` (uint8)
   - `+0x100`: `glyph_handles[224]` (void*)
   - `+0x480`: `widths[224]` (uint16)
3. **`SpriteDesc` Descriptor (32 bytes)**:
   - `+0x04`: `width` = `widths[i]` (sign-extended to int32)
   - `+0x08`: `height` = `spacing` (`*(uint16_t*)(buffer + 10)`, sign-extended to int32)
   - `+0x0C`: `field_0c` = 0
   - `+0x10`: `pixels` = `pixel_base + offset`
   - `+0x14`: `stride` = `widths[i]` (sign-extended to int32)
   - `+0x18`: `field_18` = 0
   - `+0x1C`: `field_1c` = 0
   - Dispatched to `Gfx_SpriteOp(&desc, 0)` via wrapper `0x00456d40`. Result stored in `glyph_handles[i]`.

## Differential validation (2026-10-07)

The authentic local PE fingerprint, FPO record (11 local dwords, two parameter
words, flags 0x140E), complete instruction coverage, both calls, all 15 HIGHLOW
operand relocations and initialized magic were independently rechecked by
`tools/verify_font_parse.py`. The user-provided assumption is that authentic
IGN_WIN.EXE is active in Ghidra. The doctor probe reported Windows socket access
denied (10013); no live Ghidra corroboration is claimed. No DOS binary was used.

The existing production C89 parser matches the recovered contract without a
source edit. "EXACT" describes reconstructed behavior, not instruction equality.
The prior unsupported 100% fidelity statement is superseded by this measured scope.

**260 original-instruction versus compiled-production-C cases pass:**

- 90 cases reach all 30 slots with empty, sparse and full glyph sets. Nonzero
  status dwords (1, 2, 0x80000000, 0xFFFFFFFF) remain occupied; first zero wins.
- 50 cases cover flags 0/1/2/0x80000000/0xFFFFFFFF, corruption of each of the
  four magic bytes, five invalid versions, full tables and error precedence.
  Lazy initialization precedes header validation and resets slots even on error.
- 60 lazy initialization cases cover memory flags 0/1/2/0xFFFFFFFF, cursors
  0/198/199/200/32767 and first/last/full registration tables. Signed handle IDs
  include 0, 1, -1, -32768 and 32767. Allocation/registration failure is ignored.
- 60 randomized glyph patterns cover arbitrary return handles (including null),
  signed metric boundaries 0/32767/-32768/-1 and offset dwords 0/1/-2/INT_MIN/
  INT_MAX. Exactly offset -1 is absent; all other dwords form pixel pointers with
  32-bit wrapping. The modeled operator does not dereference these pointers.

The focused DLL extracts Font_Parse, Font_InitSystem, Font_Shutdown and Font_Unload
from production geputget.c, compiles complete production mem.c and includes
production headers with compile-time layout checks. A C memcmp supplies ordinary
immutable byte-comparison semantics; the original uses inline REPE CMPSB.
Gfx_SpriteOp is linked in a separate stub object so Clang cannot eliminate calls;
emulation intercepts it, records arguments and supplies explicit fixture handles.
The original cdecl wrapper at VA 0x00456D40 executes through its indirect boundary.
Only this sprite creation boundary is modeled; real memory lifecycle bodies execute.

Every execution compares complete 48,000-byte font state, all handle/pending fields,
error and return values, ordered dependency calls, seven descriptor dwords at
bytes 4..31, and complete state observed at each sprite call. Presence becomes
one before the call, the old glyph handle remains until return, copied metrics
precede all sprite calls, and in_use becomes one only after the loop. Missing
glyphs clear presence and handle. Header fields 4..23, padding 254..255, unrelated
slots and image bytes remain unchanged except for actual lazy initializer effects.
Caller argument/stack bytes, ESP, EBX/ESI/EDI/EBP, clear DF and the immutable input
buffer are checked. The unused second argument is varied over arbitrary dwords.

Descriptor word zero is left uninitialized by the authentic code and production
C; it is excluded from comparison. This parser does not establish whether the
native downstream operator reads it. No buffer-length, offset-range, sprite
failure or rollback check exists in the recovered parser; null handles still
leave glyphs present and the slot in use. Successful parsing preserves the old
error code. These original behaviors are retained.

## Reproduction and limits

PowerShell at repository root:

```powershell
$env:UV_CACHE_DIR = Join-Path (Get-Location) 'build/uv-cache'
$env:UV_OFFLINE = 1
uv run python tools/verify_font_parse.py
uv run python tools/workflow.py complete --rva 0x56270 --limitation "Modeled sprite creation; descriptor word zero excluded; no instruction equality or native game parity; Ghidra transport denied"
uv run python tools/db.py update --check
```

The standalone direct script resolver lacks the cached Unicorn wheel in this
sandbox; the existing uv project environment contains the required dependencies.
Full workflow completion uses the established pinned matching environment.
The full matching script pins pefile 2024.8.26, Capstone 5.0.7 and Unicorn 2.1.4;
the standalone project environment uses Capstone 5.0.9 with the same other versions.
Clang/LLD 19.1.1 is provisional, with i686-pc-windows-msvc, strict C89, -O2,
-ffreestanding, -fno-builtin, -fno-inline, -mno-sse and -mno-sse2.
Original compiler selection remains unresolved. Artifacts reside in
build/decomp/windows/font_parse_validation.dll and are not committed.

Compilation and differential emulation are separately recorded for RVA 0x56270.
Raw compiled-prefix bytes differ; compiled extent and relocation-aware instruction
equality are not established. Full geputget.c and native game builds remain
blocked by legacy dependencies. Invalid/unmapped buffers, negative memory-handle
cursors, aliased input/font storage, dependency mutation/reentry, native sprite
creation/destruction and linked/native game parity remain outside this scope.
