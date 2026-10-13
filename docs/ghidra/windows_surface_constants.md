# Windows surface constant bodies and configuration entry

Authenticated against standard IGN_WIN.EXE; VA = 0x400000 + RVA. Original
translation-unit filenames and the meaning of return value 2 remain unknown.
Ownership is the directly compiled production surface module.

| Routine | RVA / VA | Verified extent and FPO |
| --- | --- | --- |
| Gfx_SurfaceConfigureSurfaceNative | 0x5C150 / 0x45C150 | 6 bytes; (6,0,5,0) |
| Gfx_SurfaceReservedNative | 0x5C520 / 0x45C520 | 6 bytes; (6,0,0,0) |
| Gfx_SurfaceConfigureSurfaceDispatch | 0x56C60 / 0x456C60 | 35 bytes; (35,0,5,0) |

Both bodies execute `mov eax,2; ret`, with SHA-256
`7140f35dee6220b79b12aecc27acf5105bf3b77d1588e89fce345de7c16c72b7`.
The configuration entry forwards five caller-owned stack words through live
slot VA 0x50EB78, then caller-cleans 20 bytes. Its SHA-256 is
`a6101b85671afd849e7b08e410e5db71224d69dfdaddd303e64e7e1f572ea8e3`.
The actual installer binds both slots; no replacement dispatch table is invented.

Before implementation, 2,049 original-instruction calls established constant
returns, unchanged image and caller stack, and ordinary i386 cdecl preservation.
Thirty-six original native calls used the actual installer and configuration
entry through three real windowed DirectDraw lifecycle scenarios.

Readable C89 bodies ignore their authenticated arguments and return 2. The entry
calls its volatile live target with all five words. The builder removes the last
two nonreturning surface-only fixtures; all surface bodies now compile directly
from surface.c. Remaining graphics/startup boundaries are explicit and unresolved.

`uv run tools/verify_surface_constants.py` compares 1,024 argument combinations
for each configuration body/entry and one no-argument reserved call, executing
both original and compiled instructions. An observational target-entry hook
asserts every forwarded word without replacing execution. Return, caller stack,
ESP/EIP, preserved registers, clear direction flag and all image bytes are checked.
Thus a constant-return replacement for the forwarding entry cannot pass.

Actual paired native validation executes 72 calls across three windowed lifecycle
scenarios, using real installer targets and four varied five-word patterns.
Focused strict C89 compilation, 2,049 differential cases and paired native checks
pass. Full 97-routine renewal, fidelity audit and synchronized exports pass at
snapshot `8dbcc3c631d34a5f792a13847fded214ee9b6b7766682ddf8a2ed91793f74701`, with 97 fresh compilation/emulation and 36 scoped native records. All 48 tracking,
ownership and workflow regression tests pass. LF/index checks and tracked gates
are required before this feature is committed.

Return 2 is preserved without assigning an unsupported success/failure meaning.
Instruction equality, original compiler/link configuration, rebuilt WndProc and
complete startup, menus, rendering, input, audio and races remain unresolved.
These validation DLLs do not constitute a playable rebuilt game. Continue the
verified rendering/startup dependency graph after the separate feature commit.
