#!/usr/bin/env python3
"""
Refactor fx.c globals: Replace raw DAT_ addresses with authentic named symbols.
"""

import re
from pathlib import Path

FX_PATH = Path("decomp/src/fx.c")

GLOBAL_MAP = {
    # Vehicles and Racers
    "DAT_005daffc": "g_Vehicles",
    "DAT_005285c0": "g_ActiveVehicleIndex",
    "DAT_005daff4": "g_VehicleConfigs",
    "DAT_006192f0": "g_NumRacers",
    "DAT_00552ffc": "g_PlayerHUDState",
    "DAT_00527f24": "g_PlayerCarModel",
    "DAT_005530a8": "g_PlayerCarChoice",
    "DAT_00527f6c": "g_GameMode",
    "DAT_00639493": "g_DifficultyLevel",
    "DAT_00552fc4": "g_CurrentTrackIndex",
    "DAT_00552fbc": "g_RaceFinished",
    "DAT_00552f60": "g_pTrackRoadSequence",

    # Screen and Video
    "DAT_00553000": "g_ScreenWidth",
    "DAT_00563c10": "g_ScreenHeight",
    "DAT_00525e58": "g_ScreenHeightAlt",
    "DAT_00563db0": "g_VirtualFramebuffer",
    "DAT_00563d40": "g_RenderTargetSurface",
    "DAT_0055306c": "g_IsSplitScreen",
    "DAT_00527f2c": "g_SplitScreenPlayer",
    "DAT_00639933": "g_LanguageId",

    # Fonts & UI
    "DAT_00552fd0": "g_FontId_Large",
    "DAT_00552fd4": "g_FontId_Medium",
    "DAT_00552fe8": "g_FontId_Small",
    "DAT_00552fec": "g_FontId_Menu",
    "DAT_0054f9d4": "g_ActiveFontColor",
    "DAT_0063f2e4": "g_FontAlignMode",
    "DAT_004949a4": "g_MenuCursorPos",
    "DAT_00494990": "g_IsDemoMode",
    "DAT_0049373c": "g_MasterTickCount",

    # HUD Elements
    "DAT_00553780": "g_SpeedoConfig",
    "DAT_00553770": "g_SpeedoPosition",
    "DAT_00553774": "g_HudEnabled",
    "DAT_005285ec": "g_pSpeedoGaugeSprite",
    "DAT_005285f0": "g_pSpeedoNeedleSprite",
    "DAT_00552f24": "g_HudFloatingMessages",

    # Camera & Lisa 3D Rasterizer
    "DAT_0063c5f0": "g_LisaCamera",
    "DAT_0063c600": "g_LisaDrawCommandBuffer",
    "DAT_0063c5bc": "g_pLisaDrawCommandQueue",
    "DAT_005dfe64": "g_pLisaDrawCommandWritePtr",
    "DAT_00498730": "g_LisaDrawQueueHead",
    "DAT_00498734": "g_LisaFramebufferPtr1",
    "DAT_00498738": "g_LisaFramebufferPtr2",
    "DAT_0049873c": "g_LisaShadingTablePtr",
    "DAT_00498740": "g_ViewportMinX",
    "DAT_00498744": "g_ViewportMinY",
    "DAT_00498748": "g_ViewportMaxX",
    "DAT_0049874c": "g_ViewportMaxY",
    "DAT_00563be4": "g_pActiveTAB",
    "DAT_0054f900": "g_ActiveTrackPalette",
    "DAT_0054f954": "g_CheckpointCount",
    "DAT_0054f904": "g_pCheckpoints",

    # Particle Struct Fields (g_ActiveParticle)
    "_DAT_00639330": "g_ActiveParticle.type",
    "DAT_00639330": "g_ActiveParticle.type",
    "_DAT_00639334": "g_ActiveParticle.pos_x",
    "_DAT_00639338": "g_ActiveParticle.pos_y",
    "_DAT_0063933c": "g_ActiveParticle.pos_z",
    "_DAT_00639340": "g_ActiveParticle.vel_x",
    "_DAT_00639344": "g_ActiveParticle.vel_y",
    "_DAT_00639348": "g_ActiveParticle.vel_z",
    "_DAT_0063934c": "g_ActiveParticle.drag",
    "_DAT_00639350": "g_ActiveParticle.gravity",
    "_DAT_00639354": "g_ActiveParticle.field_24",
    "_DAT_00639358": "g_ActiveParticle.field_28",
    "_DAT_0063935c": "g_ActiveParticle.field_2c",
    "_DAT_00639360": "g_ActiveParticle.rot_x",
    "_DAT_00639364": "g_ActiveParticle.rot_y",
    "_DAT_00639368": "g_ActiveParticle.rot_z",
    "_DAT_0063936c": "g_ActiveParticle.field_3c",
    "_DAT_00639370": "g_ActiveParticle.field_40",
    "_DAT_00639374": "g_ActiveParticle.field_44",
    "_DAT_00639378": "g_ActiveParticle.field_48",
    "_DAT_0063937c": "g_ActiveParticle.field_4c",
    "_DAT_00639380": "g_ActiveParticle.life",
    "_DAT_00639384": "g_ActiveParticle.field_54",
    "_DAT_00639388": "g_ActiveParticle.field_58",

    # Math Constants
    "_DAT_0047a4d8": "g_Const_DegToRad",
    "_DAT_0047a4e0": "g_Const_0_05",
    "_DAT_0047a4e8": "g_Const_0_1",
    "_DAT_0047a4f0": "g_Const_TwoPi",
    "_DAT_0047a4f8": "g_Const_Pi",
    "_DAT_0047a500": "g_Const_NegPi",
    "_DAT_0047a968": "g_Const_0_5",
    "_DAT_0047a980": "g_Const_1_0",
    "_DAT_0047a988": "g_Const_0_1_B",
    "_DAT_0047a990": "g_Const_160_0",
    "_DAT_0047a998": "g_Const_100_0",
    "_DAT_0047a9a0": "g_Const_DegToRad_B",
    "_DAT_0047a9a8": "g_Const_Neg100_0",
    "_DAT_0047a530": "g_Const_200_0",
    "_DAT_0047a538": "g_Const_0_005",
    "_DAT_0047ad50": "g_Const_Inv16384",
    "_DAT_0047ad58": "g_Const_0_0",
    "_DAT_0047ada8": "g_Const_0_0_B",
}

def main():
    content = FX_PATH.read_text(encoding="latin-1")
    total_replaced = 0
    for old_name, new_name in GLOBAL_MAP.items():
        # Avoid replacing inside words
        pattern = r'\b' + old_name + r'\b'
        count = len(re.findall(pattern, content))
        if count > 0:
            content = re.sub(pattern, new_name, content)
            total_replaced += count
            print(f"Replaced {count} instances of {old_name} -> {new_name}")

    FX_PATH.write_text(content, encoding="latin-1")
    print(f"Total replacements: {total_replaced}")

if __name__ == "__main__":
    main()
