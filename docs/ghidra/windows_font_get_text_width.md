# Font_GetTextWidth (VA 0x004564D0)

## Contract
- **Routine**: `Font_GetTextWidth` (text width measurement routine)
- **Address**: `IGN_WIN.EXE` @ VA `0x004564D0` / RVA `0x000564D0`
- **Size**: 397 bytes (`0x18D`), FPO extent `[0x004564D0, 0x0045665D)`
- **Padding**: 3 bytes of `0xCC` (`INT3`) ending at `0x00456660` (`Font_DrawText`)
- **Routine SHA-256**: `f69343cc05e2d5113fe34847fd7ab817a81c430ebe0b59cd7320ca7193dbf066`
- **Signature**: `int Font_GetTextWidth(const char *text, int font_id)`
- **ABI**: `cdecl` (caller cleanup; callee preserves `ebx`, `esi`, `edi`, `ebp`; allocates 8 bytes local stack frame; FPO record: `(397, 2, 2, 5129)`)
- **Parameters**:
  - `text`: pointer to null-terminated string (`const char *`)
  - `font_id`: font slot index (`0..29`) (`int`)
- **Return Value**:
  - Total measured pixel width of string `text`
  - Returns `2` immediately if `g_fonts[font_id].field_08 != 0`
- **Side Effects**:
  - If signed `font_id >= 30` or `g_fonts[font_id].in_use == 0`, sets `g_fileErrorLine` (`0x004BAB34`) to `1040` (`0x410`), but continues execution. There is no lower-bound guard.
- **Dependencies**:
  - `g_fonts` table @ VA `0x0063F2E0` (RVA `0x0023F2E0`)
  - `g_fileErrorLine` @ VA `0x004BAB34` (RVA `0x000BAB34`)
  - CRT `_ftol` helper @ VA `0x0046950C` (RVA `0x0006950C`)
  - Double constant `0.35` @ VA `0x0047AEC8` (RVA `0x0007AEC8`, `66 66 66 66 66 66 d6 3f`)
  - Double constant `0.00390625` (`1.0 / 256.0`) @ VA `0x0047AED0` (RVA `0x0007AED0`, `00 00 00 00 00 00 70 3f`)

## Binary Evidence

Rechecked against the authenticated 915,968-byte PE (SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`).
`tools/verify_font_width.py` asserts the function hash, FPO tuple, padding,
all 17 HIGHLOW relocations, constants and both helper call targets directly
from the file. Its disposable listing is `build/decomp/windows/font_width_original.txt`.
The PE section/import/startup inventory is reproduced by `tools/windows_inspect.py`.
Ghidra at localhost:8080 responds and lists Windows-shaped sections, but its
bridge exposes no independently verifiable loaded-program fingerprint. No
Ghidra symbol synchronization or unverified decompiler evidence was used.

The preliminary handoff's minimum-space clamp and fixed-font width-table lookup
are absent from the instructions. The first `_ftol` result is discarded.
Fixed fonts use signed `height + extra_spacing` for present glyphs and spaces;
proportional fonts use signed 16-bit glyph widths plus spacing. An absent space
uses the x87 product `(signed_height * 256) * 0.35 * (1/256)` truncated toward
zero, without spacing or a minimum clamp. Presence must equal exactly 1 and
is tested before the space fallback. Both paths repeat `strlen` for every iteration.

`movsx ecx, al` makes the character signed. Its presence address is
`slot + offsetof(FontSlot, glyph_present) + signed_character - 32`.
High-bit bytes address earlier slot/header storage, not glyphs 128..255;
this original quirk is preserved. Width loads and height loads also use `movsx`.
The shared header retains the recovered unsigned storage fields; this routine
interprets their bits as signed shorts at the verified load sites. Accumulation
uses unsigned 32-bit arithmetic to retain x86 wrapping without signed C overflow.

Disassembly and operand relocations from authentic `IGN_WIN.EXE`:
```assembly
0x004564d0: 83 ec 08                 sub      esp, 8                     ; allocate 8 bytes local frame
0x004564d3: 53                       push     ebx                        ; preserve ebx
0x004564d4: 56                       push     esi                        ; preserve esi
0x004564d5: 57                       push     edi                        ; preserve edi
0x004564d6: 33 db                    xor      ebx, ebx                   ; ebx = total_width = 0
0x004564d8: 55                       push     ebp                        ; preserve ebp
0x004564d9: 8b 6c 24 20              mov      ebp, dword ptr [esp + 0x20]; ebp = font_id (arg 1)
0x004564dd: 83 fd 1e                 cmp      ebp, 0x1e                  ; cmp font_id, 30
0x004564e0: 7d 12                    jge      0x004564f4                 ; if >= 30, set error
0x004564e2: 8d 44 ad 00              lea      eax, [ebp + ebp*4]         ; eax = font_id * 5
0x004564e6: 8d 04 80                 lea      eax, [eax + eax*4]         ; eax = font_id * 25
0x004564e9: c1 e0 06                 shl      eax, 6                     ; eax = font_id * 1600 (slot offset)
0x004564ec: 39 98 e0 f2 63 00        cmp      dword ptr [eax + 0x63f2e0], ebx ; cmp g_fonts[font_id].in_use, 0 (reloc @ 0x4564ee)
0x004564f2: 75 0a                    jne      0x004564fe                 ; in use -> proceed
0x004564f4: c7 05 34 ab 4b 00 10 04 00 00 mov dword ptr [0x004bab34], 0x410 ; g_fileErrorLine = 1040 (reloc @ 0x4564f6)
0x004564fe: 8d 74 ad 00              lea      esi, [ebp + ebp*4]         ; esi = font_id * 5
0x00456502: 8d 34 b6                 lea      esi, [esi + esi*4]         ; esi = font_id * 25
0x00456505: c1 e6 06                 shl      esi, 6                     ; esi = font_id * 1600 (slot offset)
0x00456508: 83 be e8 f2 63 00 00     cmp      dword ptr [esi + 0x63f2e8], 0 ; cmp g_fonts[font_id].field_08, 0 (reloc @ 0x45650a)
0x0045650f: 74 0d                    je       0x0045651e                 ; if field_08 == 0, proceed
0x00456511: b8 02 00 00 00           mov      eax, 2                     ; return 2
0x00456516: 5d                       pop      ebp                        ; restore ebp
0x00456517: 5f                       pop      edi                        ; restore edi
0x00456518: 5e                       pop      esi                        ; restore esi
0x00456519: 5b                       pop      ebx                        ; restore ebx
0x0045651a: 83 c4 08                 add      esp, 8                     ; free local frame
0x0045651d: c3                       ret                                 ; return 2
0x0045651e: 0f bf 86 fa f2 63 00     movsx    eax, word ptr [esi + 0x63f2fa] ; eax = g_fonts[font_id].height (reloc @ 0x456521)
0x00456525: c1 e0 08                 shl      eax, 8                     ; eax = height << 8
0x00456528: 89 44 24 10              mov      dword ptr [esp + 0x10], eax; store to stack for FPU
0x0045652c: db 44 24 10              fild     dword ptr [esp + 0x10]     ; load (double)(height << 8)
0x00456530: dc 0d c8 ae 47 00        fmul     qword ptr [0x0047aec8]     ; * 0.35 (reloc @ 0x456532)
0x00456536: e8 d1 2f 01 00           call     0x0046950c                 ; _ftol() (result discarded by following path)
0x0045653b: 83 be ec f2 63 00 00     cmp      dword ptr [esi + 0x63f2ec], 0 ; cmp g_fonts[font_id].is_proportional, 0 (reloc @ 0x45653d)
0x00456542: 75 78                    jne      0x004565bc                 ; if proportional, branch to 0x4565bc
0x00456544: 33 d2                    xor      edx, edx                   ; edx = i = 0 (string index)
0x00456546: 8b 7c 24 1c              mov      edi, dword ptr [esp + 0x1c]; edi = text (arg 0)
0x0045654a: b9 ff ff ff ff           mov      ecx, 0xffffffff            ; ecx = -1
0x0045654f: 2b c0                    sub      eax, eax                   ; al = 0
0x00456551: f2 ae                    repne scasb al, byte ptr es:[edi]   ; strlen(text)
0x00456553: f7 d1                    not      ecx                        ; ecx = strlen + 1
0x00456555: 49                       dec      ecx                        ; ecx = strlen(text)
0x00456556: 85 c9                    test     ecx, ecx                   ; test strlen
0x00456558: 7e 58                    jle      0x004565b2                 ; if strlen <= 0, return ebx
0x0045655a: 8b 4c 24 1c              mov      ecx, dword ptr [esp + 0x1c]; ecx = text
0x0045655e: 8a 04 11                 mov      al, byte ptr [ecx + edx]   ; al = text[i]
0x00456561: 84 c0                    test     al, al                     ; test if al == '\0'
0x00456563: 74 4d                    je       0x004565b2                 ; null terminator -> break loop
0x00456565: 0f be c8                 movsx    ecx, al                    ; ecx = (int)al
0x00456568: 80 bc 0e de f2 63 00 01  cmp      byte ptr [esi + ecx + 0x63f2de], 1 ; cmp glyph_present[c - 32], 1 (reloc @ 0x45656b)
0x00456570: 75 16                    jne      0x00456588                 ; if not present, check space
0x00456572: 66 8b 86 fa f2 63 00     mov      ax, word ptr [esi + 0x63f2fa] ; ax = g_fonts[slot].height (reloc @ 0x456575)
0x00456579: 8b 8e f0 f2 63 00        mov      ecx, dword ptr [esi + 0x63f2f0]; ecx = g_fonts[slot].extra_spacing (reloc @ 0x45657b)
0x0045657f: 0f bf c0                 movsx    eax, ax                    ; eax = height
0x00456582: 03 c8                    add      ecx, eax                   ; ecx = height + extra_spacing
0x00456584: 03 d9                    add      ebx, ecx                   ; total_width += height + extra_spacing
0x00456586: eb 15                    jmp      0x0045659d                 ; next character
0x00456588: 3c 20                    cmp      al, 0x20                   ; cmp c, ' '
0x0045658a: 75 11                    jne      0x0045659d                 ; if not space, skip
0x0045658c: 0f bf 8e fa f2 63 00     movsx    ecx, word ptr [esi + 0x63f2fa] ; ecx = height (reloc @ 0x45658f)
0x00456593: 8b 86 f0 f2 63 00        mov      eax, dword ptr [esi + 0x63f2f0]; eax = extra_spacing (reloc @ 0x456595)
0x00456599: 03 c1                    add      eax, ecx                   ; eax = height + extra_spacing
0x0045659b: 03 d8                    add      ebx, eax                   ; total_width += height + extra_spacing
0x0045659d: 42                       inc      edx                        ; i++
0x0045659e: 8b 7c 24 1c              mov      edi, dword ptr [esp + 0x1c]; edi = text
0x004565a2: b9 ff ff ff ff           mov      ecx, 0xffffffff            ; ecx = -1
0x004565a7: 2b c0                    sub      eax, eax                   ; al = 0
0x004565a9: f2 ae                    repne scasb al, byte ptr es:[edi]   ; strlen(text)
0x004565ab: f7 d1                    not      ecx                        ; ecx = strlen + 1
0x004565ad: 49                       dec      ecx                        ; ecx = strlen(text)
0x004565ae: 3b ca                    cmp      ecx, edx                   ; cmp strlen, i
0x004565b0: 7f a8                    jg       0x0045655a                 ; loop while i < strlen
0x004565b2: 8b c3                    mov      eax, ebx                   ; eax = total_width
0x004565b4: 5d                       pop      ebp                        ; restore ebp
0x004565b5: 5f                       pop      edi                        ; restore edi
0x004565b6: 5e                       pop      esi                        ; restore esi
0x004565b7: 5b                       pop      ebx                        ; restore ebx
0x004565b8: 83 c4 08                 add      esp, 8                     ; free local frame
0x004565bb: c3                       ret                                 ; return total_width
0x004565bc: 8b 7c 24 1c              mov      edi, dword ptr [esp + 0x1c]; edi = text
0x004565c0: b9 ff ff ff ff           mov      ecx, 0xffffffff            ; ecx = -1
0x004565c5: c7 44 24 14 00 00 00 00  mov      dword ptr [esp + 0x14], 0  ; [esp + 0x14] = i = 0
0x004565cd: 2b c0                    sub      eax, eax                   ; al = 0
0x004565cf: f2 ae                    repne scasb al, byte ptr es:[edi]   ; strlen(text)
0x004565d1: f7 d1                    not      ecx                        ; ecx = strlen + 1
0x004565d3: 49                       dec      ecx                        ; ecx = strlen(text)
0x004565d4: 85 c9                    test     ecx, ecx                   ; test strlen
0x004565d6: 7e 7b                    jle      0x00456653                 ; if strlen <= 0, return ebx
0x004565d8: 8b 4c 24 1c              mov      ecx, dword ptr [esp + 0x1c]; ecx = text
0x004565dc: 8b 44 24 14              mov      eax, dword ptr [esp + 0x14]; eax = i
0x004565e0: 8a 04 01                 mov      al, byte ptr [ecx + eax]   ; al = text[i]
0x004565e3: 84 c0                    test     al, al                     ; test if al == '\0'
0x004565e5: 74 6c                    je       0x00456653                 ; null terminator -> break loop
0x004565e7: 0f be c8                 movsx    ecx, al                    ; ecx = (int)al
0x004565ea: 80 bc 0e de f2 63 00 01  cmp      byte ptr [esi + ecx + 0x63f2de], 1 ; cmp glyph_present[c - 32], 1 (reloc @ 0x4565ed)
0x004565f2: 75 1c                    jne      0x00456610                 ; if not present, check space
0x004565f4: 8d 44 ad 00              lea      eax, [ebp + ebp*4]         ; eax = font_id * 5
0x004565f8: 8d 04 80                 lea      eax, [eax + eax*4]         ; eax = font_id * 25
0x004565fb: c1 e0 05                 shl      eax, 5                     ; eax = font_id * 800
0x004565fe: 03 c1                    add      eax, ecx                   ; eax = font_id * 800 + c
0x00456600: 0f bf 04 45 20 f7 63 00  movsx    eax, word ptr [eax*2 + 0x63f720] ; eax = widths[c - 32] (reloc @ 0x456604)
0x00456608: 03 86 f0 f2 63 00        add      eax, dword ptr [esi + 0x63f2f0] ; eax += extra_spacing (reloc @ 0x45660a)
0x0045660e: eb 27                    jmp      0x00456637                 ; add to total_width
0x00456610: 3c 20                    cmp      al, 0x20                   ; cmp c, ' '
0x00456612: 75 25                    jne      0x00456639                 ; if not space, skip
0x00456614: 0f bf 86 fa f2 63 00     movsx    eax, word ptr [esi + 0x63f2fa] ; eax = height (reloc @ 0x456617)
0x0045661b: c1 e0 08                 shl      eax, 8                     ; eax = height << 8
0x0045661e: 89 44 24 10              mov      dword ptr [esp + 0x10], eax; store to stack for FPU
0x00456622: db 44 24 10              fild     dword ptr [esp + 0x10]     ; load (double)(height << 8)
0x00456626: dc 0d c8 ae 47 00        fmul     qword ptr [0x0047aec8]     ; * 0.35 (reloc @ 0x456628)
0x0045662c: dc 0d d0 ae 47 00        fmul     qword ptr [0x0047aed0]     ; * (1.0 / 256.0) (reloc @ 0x45662e)
0x00456632: e8 d5 2e 01 00           call     0x0046950c                 ; _ftol() -> eax = space_width
0x00456637: 03 d8                    add      ebx, eax                   ; total_width += width
0x00456639: 8b 7c 24 1c              mov      edi, dword ptr [esp + 0x1c]; edi = text
0x0045663d: b9 ff ff ff ff           mov      ecx, 0xffffffff            ; ecx = -1
0x00456642: ff 44 24 14              inc      dword ptr [esp + 0x14]     ; i++
0x00456646: 2b c0                    sub      eax, eax                   ; al = 0
0x00456648: f2 ae                    repne scasb al, byte ptr es:[edi]   ; strlen(text)
0x0045664a: f7 d1                    not      ecx                        ; ecx = strlen + 1
0x0045664c: 49                       dec      ecx                        ; ecx = strlen(text)
0x0045664d: 3b 4c 24 14              cmp      ecx, dword ptr [esp + 0x14]; cmp strlen, i
0x00456651: 7f 85                    jg       0x004565d8                 ; loop while i < strlen
0x00456653: 8b c3                    mov      eax, ebx                   ; eax = total_width
0x00456655: 5d                       pop      ebp                        ; restore ebp
0x00456656: 5f                       pop      edi                        ; restore edi
0x00456657: 5e                       pop      esi                        ; restore esi
0x00456658: 5b                       pop      ebx                        ; restore ebx
0x00456659: 83 c4 08                 add      esp, 8                     ; free local frame
0x0045665c: c3                       ret                                 ; return total_width
```

## Relocations & Operands
- Direct call targets:
  - `0x00456536`: call `0x0046950C` (`_ftol`)
  - `0x00456632`: call `0x0046950C` (`_ftol`)
- Exactly 17 base relocations exist in the routine:
  - `0x004564EE` -> `0x0063F2E0`: `g_fonts[0].in_use`
  - `0x004564F6` -> `0x004BAB34`: `g_fileErrorLine` (set to `1040` / `0x410`)
  - `0x0045650A` -> `0x0063F2E8`: `g_fonts[0].field_08`
  - `0x00456521` -> `0x0063F2FA`: `g_fonts[0].height`
  - `0x00456532` -> `0x0047AEC8`: double constant `0.35`
  - `0x0045653D` -> `0x0063F2EC`: `g_fonts[0].is_proportional`
  - `0x0045656B` -> `0x0063F2DE`: `&g_fonts[0].glyph_present[-32]` (`0x63F2FE - 0x20`)
  - `0x00456575` -> `0x0063F2FA`: `g_fonts[0].height`
  - `0x0045657B` -> `0x0063F2F0`: `g_fonts[0].extra_spacing`
  - `0x0045658F` -> `0x0063F2FA`: `g_fonts[0].height`
  - `0x00456595` -> `0x0063F2F0`: `g_fonts[0].extra_spacing`
  - `0x004565ED` -> `0x0063F2DE`: `&g_fonts[0].glyph_present[-32]` (`0x63F2FE - 0x20`)
  - `0x00456604` -> `0x0063F720`: `&g_fonts[0].widths[-32]` (`0x63F760 - 0x40`)
  - `0x0045660A` -> `0x0063F2F0`: `g_fonts[0].extra_spacing`
  - `0x00456617` -> `0x0063F2FA`: `g_fonts[0].height`
  - `0x00456628` -> `0x0047AEC8`: double constant `0.35`
  - `0x0045662E` -> `0x0047AED0`: double constant `0.00390625` (`1.0 / 256.0`)

## Stack Layout and FPO Data
- FPO record: `(397, 2, 2, 5129)`
  - Function length: 397 bytes
  - Local frame dwords: 2 (8 bytes allocated via `sub esp, 8`)
  - Parameters: 2 dwords (8 bytes: `text`, `font_id`)
  - Callee-saved registers: 4 (`ebx`, `esi`, `edi`, `ebp`)
  - Frame pointer omitted (ESP-based addressing)
- Stack layout after prolog:
  - `[esp + 0x00]`: Saved `ebp`
  - `[esp + 0x04]`: Saved `edi`
  - `[esp + 0x08]`: Saved `esi`
  - `[esp + 0x0C]`: Saved `ebx`
  - `[esp + 0x10]`: local stack space (dword 0, scratch for FPU integer conversion)
  - `[esp + 0x14]`: local stack space (dword 1, loop index `i` in proportional path)
  - `[esp + 0x18]`: Return address
  - `[esp + 0x1C]`: `text` (`const char *`, arg 0)
  - `[esp + 0x20]`: `font_id` (`int`, arg 1)

## Callers and Usage
Direct calls to `Font_GetTextWidth` (`0x004564D0`) in `IGN_WIN.EXE` occur across UI, menu, and HUD systems:
- Menu item centering and layout: `0x00402B3E`
- UI dialog text box layout: `0x00409AC1`, `0x0040A8CE`, `0x0040A9FF`, `0x0040B000`, `0x0040B027`, `0x0040B082`
- Menu option string positioning: `0x0040B464` .. `0x0040C151`
- System text measurement: `0x004187C3`, `0x004188DD`, `0x00418AB4`
- In-game HUD timer and speedometer alignment: `0x00438455` .. `0x0043C662`

## Reproducible compilation and differential emulation

Run `uv run tools/verify_matching.py` for the integrated verifier, or
`uv run tools/verify_font_width.py` for the focused font run. Dependencies are
pinned to pefile 2024.8.26, Capstone 5.0.7 and Unicorn 2.1.4. The harness extracts
the actual production function body into a disposable translation unit, includes
the production header, and asserts the 1600-byte slot and offsets 30/1152.
It supplies table storage, the error word, a C `strlen`, and the MSVC `_fltused`
linker data marker; no reconstruction body or success-returning stub is copied.
Clang/LLD 19.1.1 compile strict C89 x86 Windows with x87 (`-mno-sse -mno-sse2`),
`-O2`, `/noentry` and `/nodefaultlib`. Exact commands are printed and recorded
with input/artifact hashes in SQLite. The original executes its real `_ftol`
at `[0x0046950C, 0x00469533)`, with no Python conversion model.

708 differential cases cover all 30 slots, fixed/proportional modes (including
noncanonical flags), present/absent/non-1 presence, empty strings, signed heights,
signed widths, extreme spacing, 32-bit wrap, all 255 nonzero character bytes,
and seeded mixed strings. Every case checks EAX, ordered error writes, unchanged
table/image/text, caller stack, callee registers, clear DF and restored x87
control word. Two original-only cases use mapped surrounding storage for slots
-1 and 30, showing error 1040 followed by return 2. These do not certify C access
outside its declared table.

Space rounding depends on the caller's x87 precision. Height 20 and an absent
space yield 6 with control word `0x037F` (extended precision) and 7 with `0x027F`
(double precision): the stored binary64 0.35 is slightly below 7/20. Both are
checked against original and compiled execution; the harness does not assume
which control word applies to every game caller. No artificial minimum is added.

## Lifecycle and limitations

This leaf reads existing font-slot metadata populated by the parse/load lifecycle.
It allocates/frees nothing and does not initialize the system. It writes only the
diagnostic error line for invalid/unallocated slots, then continues. It returns
2 before accessing text for nonzero `field_08`. The ordinary C contract requires
a readable terminated immutable string, valid table storage and glyph accesses
inside the containing table. Original unchecked accesses outside that table
remain unsafe; no new rejection or fallback is introduced.

`@fidelity EXACT` records reconstruction intent, not instruction equality.
Full `geputget.c` compilation remains blocked by DOS includes/dependencies
(`<io.h>`, `<fcntl.h>`, `File_LoadToMemory`). The extracted routine compiles and
links as `build/decomp/windows/font_width_validation.dll`; it does not rebuild
the full module or a playable game. Raw/relocated instruction equality, original
compiler identity, native DLL/game execution and visual alignment are unverified.
The historical 11-case narrative was not a reproducible compiled-C comparison;
this harness replaces it and the earlier unsupported 100% fidelity claim.

Next bounded candidate: `Font_DrawText` at VA `0x00456660` / RVA `0x00056660`;
independently recover its rendering dependencies before implementation.
