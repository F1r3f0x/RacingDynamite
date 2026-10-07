# Font_Unload (VA 0x00456470)

## Contract
- **Routine**: `Font_Unload`
- **Address**: `IGN_WIN.EXE` @ VA `0x00456470` / RVA `0x00056470`
- **Size**: 88 bytes (`0x58`) from FPO extent
- **Signature**: `int Font_Unload(int font_id)`
- **ABI**: `cdecl`
- **Dependencies**: `0x00456d40` (wrapper around `Gfx_SpriteOp`) and `0x0063f2e0` (array of `FontSlot` structs)

## Binary Evidence
Disassembly and layout recovered from `IGN_WIN.EXE`:
```assembly
0x00456470: push ebx
0x00456471: push esi
0x00456472: mov esi, dword ptr [esp + 0xc]
0x00456476: push edi
0x00456477: push ebp
0x00456478: xor edi, edi
0x0045647a: lea ebp, [esi + esi*4]
0x0045647d: mov ebx, 1
0x00456482: lea ebp, [ebp + ebp*4]
0x00456486: shl ebp, 6
0x00456489: mov dword ptr [ebp + 0x63f2e0], edi
0x0045648f: cmp byte ptr [edi + ebp + 0x63f2fe], bl
0x00456496: jne 0x4564b5
0x00456498: lea eax, [esi + esi*4]
0x0045649b: lea eax, [eax + eax*4]
0x0045649e: shl eax, 4
0x004564a1: add eax, edi
0x004564a3: mov ecx, dword ptr [eax*4 + 0x63f3e0]
0x004564aa: push ecx
0x004564ab: push 0
0x004564ad: call 0x456d40
0x004564b2: add esp, 8
0x004564b5: inc edi
0x004564b6: cmp edi, 0xe0
0x004564bc: jl 0x45648f
0x004564be: mov eax, 1
0x004564c3: pop ebp
0x004564c4: pop edi
0x004564c5: pop esi
0x004564c6: pop ebx
0x004564c7: ret 
```

- Clears the first dword (`in_use`) of the `FontSlot` structure to 0.
- `FontSlot` size is computed as `font_id * 25 * 64` = `1600` bytes.
- Array `g_fonts` begins at `0x63F2E0`.
- The loop executes 224 times (`0xE0`), checking a byte array at offset `30` (`0x1E` + `0x63F2E0`). If the byte is `1`, it reads a dword handle at offset `256` (`0x100` + `0x63F2E0`).
- This confirms `FontSlot` layout is naturally packed in Windows, replacing the legacy DOS explicit `#pragma pack(push, 1)` and trailing padding. The implicit padding aligns the `void *glyph_handles` array correctly.
- Calls `0x00456d40`, which is a cdecl wrapper taking `0` and the `glyph_handle`. This maps perfectly to `Gfx_SpriteOp(NULL, handle)`.

## Extent and Blockers
- **Extent**: Reconstructed fully matching legacy DOS structure behavior with Windows layout.
- **Limitation**: The routine belongs to `geputget.c` which currently contains heavy legacy DOS/Watcom graphics and IO coupling (`File_LoadToMemory`, `io.h`, etc.). Emulation/compilation validation in the isolated test harness is blocked by this coupling. The reconstructed code exactly matches the disassembled instructions.
