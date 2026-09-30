#!/usr/bin/env python3
"""
tools/update_fx_source_annotations.py
Updates @original and @fidelity header comments in decomp/src/fx.c.
"""

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
FX_C = ROOT / "decomp" / "src" / "fx.c"

updates = {
    "Audio_PlaySampleVol": ("0x00049a8c", "EXACT"),
    "Audio_StopSample": (None, "EXACT"),
    "Audio_LoadAssets": (None, "EXACT"),
    "Video_SetGraphicsMode": ("0x0006f2c4", "EXACT"),
    "Video_FlipScreen": ("0x00010060", "EXACT"),
    "Lisa_Init": (None, "ADAPTED"),
    "Lisa_FlushRasterizerCommands": (None, "EXACT"),
    "Math_LookupTrigAngle": (None, "EXACT"),
    "Camera_UpdateOverview": ("0x00038e6d", "EXACT"),
    "Camera_UpdateChase": ("0x00037883", "EXACT"),
    "Race_FindFocusedVehicle": (None, "EXACT"),
    "Race_RenderViewport": (None, "ADAPTED"),
    "Car_UpdateDynamicObjects": ("0x00048a14", "EXACT"),
    "Pos_InitAnimatedObjects": (None, "ADAPTED"),
    "Pos_UpdateAnimatedObjects": ("0x000413fc", "EXACT"),
    "FX_SpawnWeather": (None, "EXACT"),
    "FX_UpdateWeatherBounds": (None, "ADAPTED"),
    "FX_UpdateWeatherGeometry": (None, "ADAPTED"),
    "FX_SpawnParticle": ("0x0003fee8", "EXACT"),
    "FX_SpawnWaterSplashes": ("0x0003d808", "EXACT"),
    "FX_SpawnTireDirtDebris": (None, "ADAPTED"),
    "FX_SpawnLandingDustPuffs": (None, "ADAPTED"),
    "FX_SpawnTireSkidSmoke": ("0x0003e6ec", "EXACT"),
    "FX_UpdateExplosionNode": (None, "ADAPTED"),
    "FX_UpdateVehicleWreck": ("0x0003c208", "EXACT"),
    "FX_UpdateDetachedWheel": (None, "ADAPTED"),
    "FX_UpdateVehicleCrashSequence": ("0x0003db4c", "EXACT"),
    "FX_UpdateCarDebris": (None, "ADAPTED"),
    "FX_FrameTick": (None, "ADAPTED"),
    "FX_UpdateAllParticles": (None, "EXACT"),
    "HUD_RenderPauseMenu": ("0x000534cc", "EXACT"),
    "HUD_RenderConfirmationPrompt": ("0x000539f8", "EXACT"),
    "HUD_RenderTrackResults": (None, "ADAPTED"),
    "HUD_RenderPlayAgainPrompt": (None, "ADAPTED"),
    "HUD_RenderTelemetryOverlay": (None, "ADAPTED"),
    "HUD_CheckWrongWayHeading": (None, "ADAPTED"),
    "HUD_RenderPlayerElements": ("0x0003a788", "EXACT"),
}

def run():
    with open(FX_C, "r", encoding="utf-8") as f:
        lines = f.readlines()

    new_lines = []
    i = 0
    while i < len(lines):
        line = lines[i]
        orig_match = re.search(r"(\s*\*\s*@original\s+)([A-Za-z0-9_]+)(\s*\(MAINDOS_32BIT\.EXE)([^,]*)(,\s*fx\.c\))", line)
        if orig_match:
            prefix, sym, mid, old_addr, suffix = orig_match.groups()
            if sym in updates:
                new_addr, new_fid = updates[sym]
                if new_addr:
                    new_line = f"{prefix}{sym} (MAINDOS_32BIT.EXE @ {new_addr}{suffix}\n"
                else:
                    new_line = f"{prefix}{sym} (MAINDOS_32BIT.EXE{suffix}\n"
                new_lines.append(new_line)
                i += 1
                # Check next line for @fidelity
                if i < len(lines):
                    fid_line = lines[i]
                    fid_match = re.search(r"(\s*\*\s*@fidelity\s+)[A-Z_]+", fid_line)
                    if fid_match:
                        new_lines.append(f"{fid_match.group(1)}{new_fid}\n")
                        i += 1
                    else:
                        new_lines.append(fid_line)
                        i += 1
                continue
        new_lines.append(line)
        i += 1

    with open(FX_C, "w", encoding="utf-8") as f:
        f.writelines(new_lines)

    print("Updated fx.c annotations successfully.")

if __name__ == '__main__':
    run()
