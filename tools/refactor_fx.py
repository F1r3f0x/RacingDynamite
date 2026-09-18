#!/usr/bin/env python3
"""
Refactor fx.c: Replace raw FUN_ functions with authentic semantic routine names.
"""

import re
from pathlib import Path

FX_PATH = Path("decomp/src/fx.c")

FUN_MAPPING = {
    "FUN_004010f0": "Menu_ClearLayout",
    "FUN_00401140": "Menu_AddLayoutItem",
    "FUN_00401190": "Menu_LayoutItems",
    "FUN_00401f00": "Track_UpdateMovingPathNodes",
    "FUN_00420c00": "Timer_GetDeltaTime",
    "FUN_004222b0": "Ghost_SaveGhostData",
    "FUN_00427dc0": "Track_FindSurfaceHeight",
    "FUN_00429a40": "Track_UpdateDynamicObjects_Type1",
    "FUN_0042aa00": "Track_UpdateDynamicObjects_Type2",
    "FUN_0042acb0": "Track_UpdateDynamicObjects_Type3",
    "FUN_0042b5d0": "Track_UpdateDynamicObjects_Type4",
    "FUN_004407c0": "HUD_FormatLapTime",
    "FUN_00440840": "HUD_RenderSpeedometerGauge",
    "FUN_00441170": "HUD_AddFloatingMessage",
    "FUN_00441210": "HUD_UpdateFloatingMessages",
    "FUN_00441250": "HUD_RenderFloatingMessages",
    "FUN_00441360": "Lisa_ResetRasterizerContext",
    "FUN_00444890": "FX_SpawnAmbientTrackParticles",
    "FUN_00445f20": "Palette_AdjustRGB",
    "FUN_00446180": "Math_RandomFloat0To1",
    "FUN_00446240": "Gfx_BlitTransparentLUT",
    "FUN_00446390": "Math_WrapAngle",
    "FUN_004564d0": "Font_GetTextWidth",
    "FUN_00456660": "Font_DrawText",
    "FUN_00456b70": "Font_PrintDirect",
    "FUN_00456be0": "Gfx_RestoreSurface",
    "FUN_00456c40": "Gfx_FreeSurface",
    "FUN_00456cc0": "Gfx_SetRenderTarget",
    "FUN_00456cf0": "Gfx_SetClipRect",
    "FUN_00456d20": "Gfx_DrawSprite",
    "FUN_004579b0": "Audio_PlaySound",
    "FUN_00457aa0": "Audio_GetVoice",
    "FUN_00457ed0": "Audio_StopSound",
    "FUN_0045b5c0": "Log_DebugPrintf",
    "FUN_0046991a": "fmod",
}

def main():
    content = FX_PATH.read_text(encoding="latin-1")
    total_replaced = 0
    for old_name, new_name in FUN_MAPPING.items():
        count = len(re.findall(r'\b' + old_name + r'\b', content))
        if count > 0:
            content = re.sub(r'\b' + old_name + r'\b', new_name, content)
            total_replaced += count
            print(f"Replaced {count} instances of {old_name} -> {new_name}")
            
    # Include fx.h
    if '#include "fx.h"' not in content:
        content = content.replace('#include "main.h"', '#include "main.h"\n#include "fx.h"')
        print("Added #include \"fx.h\"")

    FX_PATH.write_text(content, encoding="latin-1")
    print(f"Total replacements: {total_replaced}")

if __name__ == "__main__":
    main()
