# Windows Surface Dispatch Entry Wrappers

Authentic standard `IGN_WIN.EXE` SHA256:
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.

This subsystem implements authentic Windows surface dispatch entry wrappers in
`decomp/src/surface.c` under `surface` module ownership. These routines provide the
public dispatch interface for surface operations, calling through function pointers
installed into `.data`:

1. `Gfx_SurfaceConfigureDispatch` (RVA `0x56B10`, VA `0x00456B10`):
   - Extent: 30 bytes through RET `0x00456B2D` (`call [g_surfaceConfigure]`, `0x50EB68`)
   - SHA256: `0aefb3b8a35847643dbeb6d4c15ca363c8fb17a65cf8c0bf1a1e9df12262f907`
   - FPO: `(30, 0, 4, 0)`, 2 trailing padding bytes (`0xCC * 2`)
   - Relocations: 1 (`0x56B26` -> `0x50EB68`)
   - Contract: 4 stack arguments, cdecl caller cleanup 16 bytes, EAX status return.

2. `Gfx_SurfaceLockDispatch` (RVA `0x56C10`, VA `0x00456C10`):
   - Extent: 20 bytes through RET `0x00456C23` (`call [g_surfaceLock]`, `0x50EB90`)
   - SHA256: `46a5ae6e26d29bf872659b827ff00209cb3cae73e67617b72aa1e49bf4a528d3`
   - FPO: `(20, 0, 2, 0)`, 12 trailing padding bytes (`0xCC * 12`)
   - Relocations: 1 (`0x56C1C` -> `0x50EB90`)
   - Contract: 2 stack arguments `(GfxSurfaceRecord *surface, int mode)`, cdecl caller cleanup 8 bytes, EAX status return.

3. `Gfx_SurfaceUnlockDispatch` (RVA `0x56C30`, VA `0x00456C30`):
   - Extent: 15 bytes through RET `0x00456C3E` (`call [g_surfaceUnlock]`, `0x50EB94`)
   - SHA256: `23960952e78fdc4df291be1b843aa2b238ceee65bdc1baa79770bb2e88857aa5`
   - FPO: `(15, 0, 1, 0)`, 1 trailing padding byte (`0xCC * 1`)
   - Relocations: 1 (`0x56C37` -> `0x50EB94`)
   - Contract: 1 stack argument `(GfxSurfaceRecord *surface)`, cdecl caller cleanup 4 bytes, EAX status return.

4. `Gfx_SurfaceRestoreDispatch` (RVA `0x56C50`, VA `0x00456C50`):
   - Extent: 6 bytes through JMP `0x00456C55` (`jmp [g_surfaceRestore]`, `0x50EB9C`)
   - SHA256: `9bac4d3e6b644d4ea3199af28c48924cd6366565dfb7bf1f29fe153d81102e31`
   - FPO: `(6, 0, 0, 0)`, 10 trailing padding bytes (`0xCC * 10`)
   - Relocations: 1 (`0x56C52` -> `0x50EB9C`)
   - Contract: 0 stack arguments, tail jump forwarding to installed target.

## Verification

The verifier `tools/verify_surface_dispatch.py` executes:
1. Static byte inspection against `IGN_WIN.EXE`:
   - Exact routine SHA256 digest checks.
   - Authenticated FPO records from PE debug directory.
   - Trailing padding byte values (`0xCC`).
   - Base relocations targeting `g_surfaceConfigure`, `g_surfaceLock`, `g_surfaceUnlock`, `g_surfaceRestore`.
2. Differential Unicorn emulation:
   - 32 randomized seeds executing all 4 routines.
   - Dynamic code hooks simulating target functions, capturing argument words, returning randomized status values.
   - Full register preservation checks (`EBX`, `ESI`, `EDI`, `EBP`).
   - Direction flag verification (DF=0).
   - Stack frame integrity and cdecl caller cleanup verification.
