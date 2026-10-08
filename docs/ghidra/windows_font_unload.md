# Font_Unload (VA 0x00456470)

## Contract
- **Routine**: `Font_Unload`
- **Address**: `IGN_WIN.EXE` @ VA `0x00456470` / RVA `0x00056470`
- **Size**: 88 bytes (`0x58`) from FPO extent `[0x00456470, 0x004564C8)`
- **Padding**: 8 bytes of `0xCC` (`INT3`) ending at `0x004564D0`
- **Routine SHA-256**: `d398726038e727ae65ae4041981d9970b581fa2078b75d4e946c1dc0147a4009`
- **Signature**: `int Font_Unload(int font_id)`
- **ABI**: `cdecl` (caller cleanup; callee preserves `ebx`, `esi`, `edi`, `ebp`; returns 1 in EAX)
- **Dependencies**:
  - `0x00456d40` (cdecl thunk around `Gfx_SpriteOp` at `0x0050ebc0`)
  - `g_fonts` array @ `0x0063F2E0` (30 slots of 1600 bytes)

## Binary Evidence
Disassembly and operand relocations from `IGN_WIN.EXE`:
```assembly
0x00456470: 53               push   ebx
0x00456471: 56               push   esi
0x00456472: 8b74240c         mov    esi, dword ptr [esp+0xc]   ; font_id argument
0x00456476: 57               push   edi
0x00456477: 55               push   ebp
0x00456478: 33ff             xor    edi, edi                   ; glyph index i = 0
0x0045647a: 8d2c76           lea    ebp, [esi+esi*4]           ; font_id * 5
0x0045647d: bb01000000       mov    ebx, 1
0x00456482: 8d6c2d00         lea    ebp, [ebp+ebp*4]           ; font_id * 25
0x00456486: c1e506           shl    ebp, 6                     ; font_id * 1600 (0x640)
0x00456489: 89bde0f26300     mov    dword ptr [ebp+0x63f2e0], edi  ; g_fonts[font_id].in_use = 0 (reloc @ 0x45648b)
0x0045648f: 389c2dfef26300   cmp    byte ptr [edi+ebp+0x63f2fe], bl ; check glyph_present[i] == 1 (reloc @ 0x456492)
0x00456496: 751d             jne    0x4564b5                   ; skip if not present
0x00456498: 8d0476           lea    eax, [esi+esi*4]           ; font_id * 5
0x0045649b: 8d0440           lea    eax, [eax+eax*4]           ; font_id * 25
0x0045649e: c1e004           shl    eax, 4                     ; font_id * 400
0x004564a1: 03c7             add    eax, edi                   ; font_id * 400 + i
0x004564a3: 8b8ce0e0f36300   mov    ecx, dword ptr [eax*4+0x63f3e0] ; ecx = glyph_handles[i] (reloc @ 0x4564a6)
0x004564aa: 51               push   ecx                        ; push glyph handle
0x004564ab: 6a00             push   0                          ; push NULL / op 0
0x004564ad: e88e080000       call   0x456d40                   ; call Gfx_SpriteOp(NULL, handle)
0x004564b2: 83c408           add    esp, 8                     ; caller cleanup
0x004564b5: 47               inc    edi                        ; i++
0x004564b6: 81ffe0000000     cmp    edi, 0xe0                  ; check i < 224 (0xE0)
0x004564bc: 7ce1             jl     0x45648f                   ; loop 224 glyphs
0x004564be: b801000000       mov    eax, 1                     ; return 1
0x004564c3: 5d               pop    ebp
0x004564c4: 5f               pop    edi
0x004564c5: 5e               pop    esi
0x004564c6: 5b               pop    ebx
0x004564c7: c3               ret
```

- Clears the first dword (`in_use`) of the `FontSlot` structure to 0.
- `FontSlot` size is computed as `font_id * 25 * 64` = `1600` bytes (`0x640`).
- Array `g_fonts` begins at `0x63F2E0`.
- The loop executes 224 times (`0xE0`), checking a byte array at offset `30` (`0x1E` + `0x63F2E0`). If the byte is `1`, it reads a dword handle at offset `256` (`0x100` + `0x63F2E0`).
- This confirms `FontSlot` layout is naturally packed in Windows (1600 bytes). Implicit padding aligns the `void *glyph_handles` array to offset 256.
- Calls `0x00456d40`, which is a cdecl wrapper taking `(NULL, glyph_handle)`.

## Verification Results
Validated under differential x86 emulation in `tools/verify_font_cleanup.py` and `tools/verify_matching.py`:
- **123 differential emulation test cases passed** with exact equality against authentic PE instructions:
  - All 30 font slots (0..29) tested with sparse glyphs.
  - All 30 font slots tested with empty fonts (0 glyphs present).
  - Slots tested with full fonts (all 224 glyphs present).
  - 60 randomized glyph patterns with arbitrary 32-bit handle values and non-1 presence tags.
  - Strict check verification: presence flags not equal to 1 (e.g. 0, 2, 0xFF) do not trigger sprite release calls.
  - Returns 1 in EAX across all cases.
  - Caller stack balance, preserved registers (`ebx`, `esi`, `edi`, `ebp`), and clear DF maintained.
  - Unrelated font slots and memory regions remain untouched.

## Extent and Limitations
- **Extent**: 88 bytes, complete FPO extent `[0x00456470, 0x004564C8)`.
- **Validation Scope**: Focused validation DLL (`build/font_cleanup_validation.dll`). Full `geputget.c` compilation remains blocked by unmigrated DOS I/O legacy dependencies. Instruction equality is unclaimed.
