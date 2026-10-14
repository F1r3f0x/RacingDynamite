# Windows Sprite Dispatch Table Wrappers

Authenticated standard `IGN_WIN.EXE` SHA256:
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.

This subsystem implements the authentic Windows sprite dispatch table entry wrappers in
`decomp/src/geputget.c` under `graphics_legacy` module ownership. These routines provide the public
dispatch surface for sprite operations, calling through a contiguous array of 20 function pointers
located at VA `0x0050EBA0`..`0x0050EBF0` (80 bytes) in the `.data` section:

1. `Gfx_CloseDispatch` (RVA `0x56CA0`, VA `0x00456CA0`):
   - Extent: 6 bytes through JMP `0x00456CA5` (`jmp [g_spriteClose]`, `0x50EBA8`)
   - SHA256: `f54576d17cf81a5b459c28177c5d638406200de58ced731d331e852d85c3b108`
   - FPO: `(6, 0, 0, 0)`, 10 trailing padding bytes (`0xCC * 10`)
   - Relocations: 1 (`0x56CA2` -> `0x50EBA8`)

2. `Gfx_OptionDispatch` (RVA `0x56CB0`, VA `0x00456CB0`):
   - Extent: 15 bytes through RET `0x00456CBE` (`call [g_spriteOption]`, `0x50EBAC`)
   - SHA256: `d5f47caf1c6baad1796a505d1ba148815dc490a6e2214fc8a00f3cc3233a4c64`
   - FPO: `(15, 0, 1, 0)`, 1 trailing padding byte (`0xCC * 1`)
   - Relocations: 1 (`0x56CB7` -> `0x50EBAC`)

3. `Gfx_ConfigureDispatch` (RVA `0x56CC0`, VA `0x00456CC0`):
   - Extent: 35 bytes through RET `0x00456CE2` (`call [g_spriteConfigure]`, `0x50EBB0`)
   - SHA256: `55920e775768dfba2721249cf60a49b924696a84bfe47a8ad80af5f2da278154`
   - FPO: `(35, 0, 5, 0)`, 13 trailing padding bytes (`0xCC * 13`)
   - Relocations: 1 (`0x56CDB` -> `0x50EBB0`)

4. `Gfx_SetClipDispatch` (RVA `0x56CF0`, VA `0x00456CF0`):
   - Extent: 30 bytes through RET `0x00456D0D` (`call [g_spriteSetClip]`, `0x50EBB4`)
   - SHA256: `9f7d76bab0660dc8db60d9fea1ab576779f890831a2109d76b88fd01dd461208`
   - FPO: `(30, 0, 4, 0)`, 2 trailing padding bytes (`0xCC * 2`)
   - Relocations: 1 (`0x56D06` -> `0x50EBB4`)

5. `Gfx_DrawListDispatch` (RVA `0x56D10`, VA `0x00456D10`):
   - Extent: 15 bytes through RET `0x00456D1E` (`call [g_spriteDrawList]`, `0x50EBB8`)
   - SHA256: `12a4a53eb2752ffee638c4a6ad965e4e19bf62d9f28382d673ed854136bc9e0b`
   - FPO: `(15, 0, 1, 0)`, 1 trailing padding byte (`0xCC * 1`)
   - Relocations: 1 (`0x56D17` -> `0x50EBB8`)

6. `Gfx_DrawDispatch` (RVA `0x56D20`, VA `0x00456D20`):
   - Extent: 25 bytes through RET `0x00456D38` (`call [g_spriteDraw]`, `0x50EBBC`)
   - SHA256: `e04def2288d23041976ef0a043cea2b296558f74cf82cce58476bf765e804623`
   - FPO: `(25, 0, 3, 0)`, 7 trailing padding bytes (`0xCC * 7`)
   - Relocations: 1 (`0x56D31` -> `0x50EBBC`)

7. `Gfx_HandleOpDispatch` (RVA `0x56D40`, VA `0x00456D40`):
   - Extent: 20 bytes through RET `0x00456D53` (`call [g_spriteHandleOp]`, `0x50EBC0`)
   - SHA256: `f76b5ff746f37d91b851c1a25161196baa69ffe94b4bfe1250a607da04aa97bc`
   - FPO: `(20, 0, 2, 0)`, 12 trailing padding bytes (`0xCC * 12`)
   - Relocations: 1 (`0x56D4C` -> `0x50EBC0`)

8. `Gfx_UninstalledDispatch_56D60` (RVA `0x56D60`, VA `0x00456D60`):
   - Extent: 20 bytes through RET `0x00456D73` (`call [0x50EBC4]`)
   - SHA256: `a4229314acee10caef57e12ca7628576757d95c5c13577d49df6b50ad6d007fe`
   - FPO: `(20, 0, 2, 0)`, 12 trailing padding bytes (`0xCC * 12`)
   - Relocations: 1 (`0x56D6C` -> `0x50EBC4`)

9. `Gfx_UninstalledDispatch_56D80` (RVA `0x56D80`, VA `0x00456D80`):
   - Extent: 15 bytes through RET `0x00456D8E` (`call [0x50EBC8]`)
   - SHA256: `5c64c5dbea01345484c79b6525b9279d4743a41e19e10ad7caf3c327dac204d2`
   - FPO: `(15, 0, 1, 0)`, 1 trailing padding byte (`0xCC * 1`)
   - Relocations: 1 (`0x56D87` -> `0x50EBC8`)

10. `Gfx_UninstalledDispatch_56D90` (RVA `0x56D90`, VA `0x00456D90`):
    - Extent: 20 bytes through RET `0x00456DA3` (`call [0x50EBCC]`)
    - SHA256: `47cac51c899a539e2fb9e21ea04f64c46ba0c964ca4d20b5f8f3101586364b1d`
    - FPO: `(20, 0, 2, 0)`, 12 trailing padding bytes (`0xCC * 12`)
    - Relocations: 1 (`0x56D9C` -> `0x50EBCC`)

11. `Gfx_UninstalledDispatch_56DB0` (RVA `0x56DB0`, VA `0x00456DB0`):
    - Extent: 15 bytes through RET `0x00456DBE` (`call [0x50EBD0]`)
    - SHA256: `8712df09524247c3b22eef18213a2c7d3289e0929ac9f3c54c221ead055087da`
    - FPO: `(15, 0, 1, 0)`, 1 trailing padding byte (`0xCC * 1`)
    - Relocations: 1 (`0x56DB7` -> `0x50EBD0`)

12. `Gfx_ImageOpDispatch` (RVA `0x56DC0`, VA `0x00456DC0`):
    - Extent: 20 bytes through RET `0x00456DD3` (`call [g_spriteImageOp]`, `0x50EBD8`)
    - SHA256: `a36dcdae6600a974858cc66f8c213e5b0e94e5aaa755c35fd54d6ea49daf9772`
    - FPO: `(20, 0, 2, 0)`, 12 trailing padding bytes (`0xCC * 12`)
    - Relocations: 1 (`0x56DCC` -> `0x50EBD8`)

13. `Gfx_CreateDescriptorDispatch` (RVA `0x56DE0`, VA `0x00456DE0`):
    - Extent: 20 bytes through RET `0x00456DF3` (`call [g_spriteCreateDescriptor]`, `0x50EBD4`)
    - SHA256: `4b892e967e307cfd490ce8c9e026a2c2f96f7ca77b1aceafccf2eb388847f4fc`
    - FPO: `(20, 0, 2, 0)`, 12 trailing padding bytes (`0xCC * 12`)
    - Relocations: 1 (`0x56DEC` -> `0x50EBD4`)

14. `Gfx_FreeDescriptorDispatch` (RVA `0x56E00`, VA `0x00456E00`):
    - Extent: 15 bytes through RET `0x00456E0E` (`call [g_spriteFreeDescriptor]`, `0x50EBDC`)
    - SHA256: `ac56b05bb0bb49106928975f6c55be891221b8a15887fc446520ac74fcf4b15c`
    - FPO: `(15, 0, 1, 0)`, 1 trailing padding byte (`0xCC * 1`)
    - Relocations: 1 (`0x56E07` -> `0x50EBDC`)

15. `Gfx_CopyDescriptorDispatch` (RVA `0x56E10`, VA `0x00456E10`):
    - Extent: 20 bytes through RET `0x00456E23` (`call [g_spriteCopyDescriptor]`, `0x50EBE0`)
    - SHA256: `4df461e4396b427c52bde31bb1c9b2c9173c009db8427aa2fd4cb41f0bdbc1ed`
    - FPO: `(20, 0, 2, 0)`, 12 trailing padding bytes (`0xCC * 12`)
    - Relocations: 1 (`0x56E1C` -> `0x50EBE0`)

16. `Gfx_ReservedDispatch` (RVA `0x56E30`, VA `0x00456E30`):
    - Extent: 15 bytes through RET `0x00456E3E` (`call [g_spriteReserved]`, `0x50EBE4`)
    - SHA256: `ed08e50628bdf08f53ad96b88e5d9dab86c6b337186a18f8d8e6951da1221f22`
    - FPO: `(15, 0, 1, 0)`, 1 trailing padding byte (`0xCC * 1`)
    - Relocations: 1 (`0x56E37` -> `0x50EBE4`)

17. `Gfx_GetStateDispatch` (RVA `0x56E40`, VA `0x00456E40`):
    - Extent: 15 bytes through RET `0x00456E4E` (`call [g_spriteGetState]`, `0x50EBEC`)
    - SHA256: `9d4fe8577c937219b06bcef289111aecbca1ca42481219197dc5b00852d4b5e2`
    - FPO: `(15, 0, 1, 0)`, 1 trailing padding byte (`0xCC * 1`)
    - Relocations: 1 (`0x56E47` -> `0x50EBEC`)

18. `Gfx_SetStateDispatch` (RVA `0x56E50`, VA `0x00456E50`):
    - Extent: 15 bytes through RET `0x00456E5E` (`call [g_spriteSetState]`, `0x50EBE8`)
    - SHA256: `d4a6aa806bb86c3e10bc4475cdc896c4ece70748edf944be89dccc226f876fda`
    - FPO: `(15, 0, 1, 0)`, 1 trailing padding byte (`0xCC * 1`)
    - Relocations: 1 (`0x56E57` -> `0x50EBE8`)

## Verification Evidence

- `tools/verify_sprite_dispatch.py`: Differential Unicorn emulator session comparing `IGN_WIN.EXE` and `sprite_dispatch_validation.dll`.
  - 576 comparisons executed across all 18 dispatch routines (32 seeds each).
  - Validated exact pointer dereferences, indirect call/jump semantics, and cdecl parameter forwarding.
  - Verified preservation of `EBX`, `ESI`, `EDI`, `EBP`, stack frame integrity (`ESP` restoration), and direction flag.
