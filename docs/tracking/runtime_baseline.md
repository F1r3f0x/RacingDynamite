# DOSBox runtime baseline

Tested on 2026-10-06 (America/Santiago), before the timer restoration. Two startup comparisons were attempted using DOSBox 0.74-2.1, normal core, fixed 50000 cycles, svga_s3, 32 MB RAM, surface output, identical CD mounting and disposable copies of the original game directory under build/runtime. No writes to Ignition/ were made by the build or staging tools.

## Observed behavior

| Scenario | Authentic MAINDOS.EXE | Pre-fix rebuilt executable |
| --- | --- | --- |
| Startup, launch 1 | Four visible language flags and SELECT YOUR LANGUAGE | Uniform light-gray frame; boot.log reaches Menu_Init completed |
| Startup, launch 2 | Same language selector | Uniform gray frame again; later black/dark-gray frames with no readable UI |
| Confirm language | Enter accepted; LOADING displayed | No selector available; one accepted Enter action produced a dark-gray frame with no menu text |
| Intro | Car/person animation visible after language confirmation | No recognizable animation observed |
| Menu/car/track selection | Not evaluated beyond intro | Blocked by startup rendering divergence |
| Track/race/driving/HUD/audio/results/return | Not evaluated | Not reachable during these observations |

The first reproducible divergence is failure to show the native language selector at startup. Later gray/black output is an additional observation; the exact cause is not proven. Screenshots were displayed directly by the Computer Use tool in this chat. Rebuilt boot.log records engine/video/timer startup and successful Menu_Init, but that message is not proof of rendered UI.

Some input attempts were interrupted by detected physical user input. Both initial DOSBox windows disappeared before further tests. The user then stopped Computer Use with physical Escape; no subsequent desktop input or observation was performed. Audio was not evaluated.

## Binary evidence and change

The authentic App_FrameTick at 0x10060 expects elapsed time from 0x10238. The pre-fix implementation returned absolute BIOS time and used a zero PIT multiplier; Timer_Init omitted the authentic calibration. This is a confirmed source/binary defect, but its causal relationship to the visible startup failure remains unverified. The restored timer is described in docs/ghidra/timer.md and passes deterministic C89 tests. The post-fix Open Watcom V2 executable has not been replayed in DOSBox.

## Reproduction and next dependency

1. `uv run python tools/verify_matching.py` generates a fresh executable/map with a strict link.
2. `uv run python tools/runtime_baseline.py stage` creates equivalent copies/configs and snapshots original file hashes.
3. `uv run python tools/runtime_baseline.py original` launches the original; record language selector, Enter, loading and intro.
4. `uv run python tools/runtime_baseline.py rebuilt` launches the rebuild; record startup and equivalent key sequence.
5. `uv run python tools/runtime_baseline.py check` detects original asset changes, separately reporting preexisting generated executables.

Repeat startup with the repaired timer first. If the selector remains absent, trace authentic Game_Init (0x20d18), Game_StateDispatcher (0x20d60), Menu_Init (0x11504), Menu_Tick (0x12bb8), palette/video-buffer state and CDP decoding (0x552fc/0x55384). The source currently dispatches custom menu states and draws pill buttons, while the native layout helpers remain stub candidates; recover native layout behavior rather than adding another replacement UI. Candidate caller dependencies and unknown addresses are in implementation_inventory.md.

Ghidra localhost:8080 actively refused connections during this session. No symbol synchronization or post-fix runtime completion is claimed.
