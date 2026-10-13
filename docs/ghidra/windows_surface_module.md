# Direct Windows surface module (2026-10-12)

This mechanical migration moves twelve existing reconstructed routines from
`geputget.c` into the full production `surface.c` / `surface.h`. Every body
compares unchanged with commit `2b3df29`. The authentic local PE fingerprint and
each extent/hash below were independently rechecked before moving code. Ghidra
MCP responds; its loaded target is the user-provided operating assumption.
Original translation-unit filenames remain unknown (`original_source: null`).

| RVA | Routine | Bytes | Authentic routine SHA-256 | Prior behavioral evidence |
| --- | --- | ---: | --- | --- |
| 0x56A40 | Gfx_InitSurfaceRecords | 172 | `053404ce23bc0313a565a6807a6bff041db95d72b1231fda8e5fdbe5467f2d65` | [windows_surface_lifecycle](windows_surface_lifecycle.md) |
| 0x5B730 | Gfx_SurfaceConfigureNative | 6 | `7140f35dee6220b79b12aecc27acf5105bf3b77d1588e89fce345de7c16c72b7` | [windows_surface_lifecycle](windows_surface_lifecycle.md) |
| 0x5C730 | Gfx_SurfaceRestoreNative | 179 | `b5fbe03e6aab008cadf961db34a86c9d25525993c52b3a60dbe72a09591d6f71` | [windows_surface_lifecycle](windows_surface_lifecycle.md) |
| 0x5B690 | Gfx_InstallSurfaceDispatch | 146 | `3cb7a08c993e9d2ce4db93a2ebe96e9165728ef6bae5b5014371ebaef3de41bf` | [windows_gfx_surface_dispatch](windows_gfx_surface_dispatch.md) |
| 0x5B740 | Gfx_SurfaceOpenNative | 1575 | `4626e6dbe7c24d021c37141e72e616e63bc43ebdd89cec16ea45012cf98acdc9` | [windows_surface_open](windows_surface_open.md) |
| 0x5C060 | Gfx_SurfaceShutdownNative | 229 | `60b2e80bfcaf2f7199f8b91e331b6ee717b18925e975d0845d2196fbcfc90090` | [windows_surface_shutdown](windows_surface_shutdown.md) |
| 0x5BD70 | Gfx_SurfaceRebuildNative | 745 | `9f9272e9f5f76e0318c38fc925e2bc4d03b304c169f0daf740b995b6ec055745` | [windows_surface_rebuild](windows_surface_rebuild.md) |
| 0x5C530 | Gfx_SurfaceLockNative | 326 | `35592a3ed0bfaa7af2ea2a5d0ca5116f8f7f3f225a78705c31cc9ddfa8b3b512` | [windows_surface_access](windows_surface_access.md) |
| 0x5C680 | Gfx_SurfaceUnlockNative | 71 | `0e6fdf8dbb9b1279859342662207e6693a69b37cf823335fd050d2c6aae1c7af` | [windows_surface_access](windows_surface_access.md) |
| 0x5C3D0 | Gfx_SurfaceClearNative | 210 | `dd3bd132e2b814763aab3ab5f58c6b162ddf69bb41a46f98e2cdd5a9b87df8d6` | [windows_surface_clear](windows_surface_clear.md) |
| 0x5C4B0 | Gfx_SurfacePresentNative | 98 | `0a2b21a1bf12a224948977d5410f15eee668a81d3245a9d644f8f0846af6412e` | [windows_surface_present](windows_surface_present.md) |
| 0x5C6D0 | Gfx_SurfaceSetPaletteNative | 83 | `cce59efd58c21d910a12512b15c5641b586e6beac7a4fd5daad80a56c991f5f0` | [windows_surface_palette](windows_surface_palette.md) |

The manifest declares surface record banks, fourteen dispatch slots, DirectDraw
interfaces, three distinct palette banks and surface configuration as this module's
state. Platform instance/window/class/title backing remains owned by `main.c`.
Typed SDK extern declarations connect that backing; obsolete DOS `main.h` pragmas
do not enter the production surface translation unit. Graphics entry wrappers
and sprite/font resources retain their existing ownership.

The shared builder compiles `decomp/src/surface.c` directly under GCC i386 strict
C89, with `-O2 -fno-inline -fno-optimize-sibling-calls`. Existing record/SDK size,
field-offset and vtable-slot assertions now live in that production module.
No production surface body/header is extracted into a generated file. A separate
generated harness object exposes only authenticated platform backing from `main.c`;
all four link builders consume it and the existing explicit SDK import libraries.
Four unreconstructed dispatch targets remain separate nonreturning test boundaries.

`windows_tracking.record_run` records complete surface source/header inputs for
shared-builder runs. Thirteen tracking tests pass, including source/header changes
invalidating current results. Whole-module compilation/link passed. Full differential
and scoped native renewal, fidelity audit and exports pass at snapshot
`d17f20ac4617cc15e3c16fcead286d100b6e411810fb58bb25d7a646dd3c2bf5`: 90 fresh
compilation/emulation records and29 scoped native records. LF/index checks, staged
gates and tracked hooks are required before commit; no source move promotes
behavioral evidence or instruction equality.

Validation uses provisional GCC/Clang/LLD toolchains. Original compiler/linker
configuration, complete production rendering and native startup, menus, audio,
input integration and representative races remain unresolved. Validation DLLs
are not a playable rebuilt standard Windows game. After this migration, continue
authentic surface blitting in the declared owner, then pixel copying and its callers.


## Native foreground prerequisite correction

The first full renewal stopped at native polling: Windows did not give the probe
window foreground focus (exit74). A focused rerun confirmed the failure. Preparing
focus after initialization was also too late: authentic Acquire had already failed,
and the original polling routine does not repair a NOTACQUIRED result. Production
input bodies remain unchanged. The harness now prepares foreground before each
real initialization/reinitialization, briefly sharing thread input queues for
activation and checking detachment before initialization, then retains the observed
foreground, actual buffered scan30, mirrored character and timer assertions.
See Microsoft's [AttachThreadInput documentation](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-attachthreadinput).
No global foreground policy or original binary is changed. Native polling and
asynchronous timed probes both pass in focused reruns. A complete renewal is
required because shared native-harness inputs changed; the failed run is retained
under `build/workflow/surface-module-complete.log` and never presented as complete.
All thirteen unchanged input contracts receive fresh coverage. RVA0x56F20 has an
adjacent annotation-boundary change, with its sprite body unchanged; it receives
fresh coverage and a commit trailer as required by the conservative scanner.


A second full attempt (`build/workflow/surface-module-renewal.log`) passed native
buffered polling but stopped at the timed probe's pre-injection foreground
prerequisite. The launcher now permits at most three fresh processes only for
exit74 plus the explicit foreground setup/retention marker. It retains every
exit code/stdout/stderr in recorded native details and per-attempt runtime logs.
Any behavioral failure, failed detach, missing success marker or timeout fails
without retry. Four focused policy tests verify those distinctions; actual
polling/timed probes both passed in renewed focused runs (one attempt each).
This does not waive foreground or buffered-input assertions. The full workflow
must pass after these shared harness changes before the feature is complete.


## Final renewal and supported commands

`uv run python tools/build_decomp.py` compiles/links the focused DLL with the whole
surface module and explicit platform data boundary. `uv run tools/verify_matching.py`
renews all registered differential/scoped native contracts. `workflow.py complete`
runs that verifier, fidelity audit and export/update-check sequence; the final
successful run is `build/workflow/surface-module-final-renewal.log` with the snapshot
above and all26 declared RVAs. Native polling and timed input each used one passing
process in this final renewal; prior foreground failures are not counted as passes.
The four policy,13 tracking,9 ownership and26 workflow regression tests pass.
Native presentation retains eight fullscreen flips and2,457,600 primary-pixel checks;
palette retains24 SDK updates,48 null guards and corrected1,424 differential cases.
Full native gameplay and original toolchain remain open. Finish LF working/index,
staged gate and tracked hooks, then proceed to independently authenticated blitting.
