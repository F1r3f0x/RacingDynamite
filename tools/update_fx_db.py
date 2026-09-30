#!/usr/bin/env python3
"""
tools/update_fx_db.py
Synchronizes authentic DOS linear addresses and metadata for all 37 functions
in module fx.c in database/decomp.db.
"""

import sqlite3

def run():
    conn = sqlite3.connect("database/decomp.db")
    cur = conn.cursor()

    # Clear existing dos_address entries in fx.c to prevent collisions
    cur.execute("""
        UPDATE functions 
        SET dos_address = NULL 
        WHERE module_id = (SELECT id FROM modules WHERE name = 'fx.c')
    """)
    print(f"Cleared existing fx.c dos_addresses: {cur.rowcount} rows reset")

    fx_updates = [
        ("0x00049a8c", "Audio_PlaySampleVol", "decompiled", "EXACT", "Plays sound sample with channel, volume, panning and pitch parameters"),
        (None,         "Audio_StopSample", "decompiled", "EXACT", "Stops currently active sound voice on specified channel"),
        (None,         "Audio_LoadAssets", "decompiled", "EXACT", "Loads all sound bank pools and sample assets"),
        ("0x0006f2c4", "Video_SetGraphicsMode", "decompiled", "EXACT", "Configures VGA Mode 13h (320x200 8bpp) display mode"),
        ("0x00010060", "Video_FlipScreen", "decompiled", "EXACT", "Blits virtual double buffer to VGA 0xA0000 video memory"),
        ("0x00049724", "Lisa_Init", "decompiled", "EXACT", "Calculates camera viewports and FOV scaling tables for current screen resolution"),
        ("0x00049a54", "Lisa_FlushRasterizerCommands", "decompiled", "EXACT", "Command queue flush dispatcher calling Lisa_ExecuteRasterizerCommands"),
        (None,         "Math_LookupTrigAngle", "decompiled", "EXACT", "Computes arctangent angle lookup"),
        ("0x00038e6d", "Camera_UpdateOverview", "decompiled", "EXACT", "Simulates blimp overview camera following leading vehicles"),
        ("0x00047f94", "Camera_UpdateChase", "decompiled", "EXACT", "3rd-person chase camera dynamics, yaw smoothing, and road pitch tracking"),
        (None,         "Race_FindFocusedVehicle", "decompiled", "EXACT", "Identifies primary racer or human player to orient camera focus"),
        (None,         "Race_RenderViewport", "decompiled", "ADAPTED", "Renders single/splitscreen 3D scene viewport with HUD elements"),
        ("0x00048a14", "Car_UpdateDynamicObjects", "decompiled", "EXACT", "Synchronizes vehicle chassis, wheel meshes, shadows, and name tags with spatial grid"),
        (None,         "Pos_InitAnimatedObjects", "decompiled", "ADAPTED", "Initializes keyframed trackside animated obstacle nodes from POS asset"),
        ("0x000413fc", "Pos_UpdateAnimatedObjects", "decompiled", "EXACT", "Steps keyframed vertex animation for moving trackside obstacles"),
        (None,         "FX_SpawnWeather", "decompiled", "EXACT", "Spawns rain, snow, or fog particle effects according to track weather settings"),
        (None,         "FX_UpdateWeatherBounds", "decompiled", "ADAPTED", "Updates weather volume boundary box around active camera view"),
        (None,         "FX_UpdateWeatherGeometry", "decompiled", "ADAPTED", "Transforms rain streak / snow flake vertices relative to camera motion"),
        ("0x0003fee8", "FX_SpawnParticle", "decompiled", "EXACT", "Allocates active scenery particle slot with priority preemption"),
        ("0x0003d808", "FX_SpawnWaterSplashes", "decompiled", "EXACT", "Generates animated water splash spray sprites when car enters water surfaces"),
        ("0x0003ee70", "FX_SpawnTireDirtDebris", "decompiled", "EXACT", "Emits dirt and gravel kick-up particles behind spinning tires"),
        ("0x0003f37c", "FX_SpawnLandingDustPuffs", "decompiled", "EXACT", "Spawns impact dust clouds when car suspension compresses upon landing"),
        ("0x0003fac0", "FX_SpawnTireSkidSmoke", "decompiled", "EXACT", "Spawns tire friction smoke particles when vehicle drifts or brakes aggressively"),
        (None,         "FX_UpdateExplosionNode", "decompiled", "ADAPTED", "Animates fireball expansion and debris dispersion for destroyed vehicles"),
        ("0x0003c208", "FX_UpdateVehicleWreck", "decompiled", "EXACT", "Detaches wheels, triggers fire/smoke emitters, and disables physics on wrecked car"),
        (None,         "FX_UpdateDetachedWheel", "decompiled", "ADAPTED", "Simulates ballistic bouncing physics for tires sheared off during collisions"),
        ("0x0003db4c", "FX_UpdateVehicleCrashSequence", "decompiled", "EXACT", "Multi-frame rollover and flip crash trajectory integration"),
        (None,         "FX_UpdateCarDebris", "decompiled", "ADAPTED", "Simulates tumbling body panel fragments and glass shards"),
        ("0x0003fd04", "FX_FrameTick", "decompiled", "EXACT", "Master particle simulation tick updating all emitters and active effects"),
        (None,         "FX_UpdateAllParticles", "decompiled", "EXACT", "Wrapper dispatching particle simulation frame tick"),
        ("0x000534cc", "HUD_RenderPauseMenu", "decompiled", "EXACT", "Renders in-race pause menu overlay and handles Continue / Restart / Quit navigation"),
        ("0x000539f8", "HUD_RenderConfirmationPrompt", "decompiled", "EXACT", "Renders modal confirmation prompt for race restart and exit confirmation"),
        ("0x00043fe0", "HUD_RenderTrackResults", "decompiled", "EXACT", "Renders end-of-race leaderboard standings, split times, and championship points"),
        ("0x00047ae0", "HUD_RenderPlayAgainPrompt", "decompiled", "EXACT", "Displays play-again / next-track prompt following race completion"),
        ("0x00049d34", "HUD_RenderTelemetryOverlay", "decompiled", "EXACT", "Displays optional race debug telemetry, speedometer, and engine RPM indicators"),
        ("0x0004a150", "HUD_CheckWrongWayHeading", "decompiled", "EXACT", "Compares vehicle velocity vector against track spline tangent to detect wrong-way driving"),
        ("0x0004a320", "HUD_RenderPlayerElements", "decompiled", "EXACT", "Renders in-game dashboard HUD: tachometer, mini-map, position, lap timer, and turbo gauge"),
    ]

    for dos_addr, sym, status, fidelity, purpose in fx_updates:
        cur.execute("""
            UPDATE functions 
            SET dos_address = ?, status = ?, fidelity = ?, purpose = ?
            WHERE symbol_name = ?
        """, (dos_addr, status, fidelity, purpose, sym))
        print(f"Updated {sym:34} -> {str(dos_addr):12} ({status}, {fidelity}): rows={cur.rowcount}")

    conn.commit()
    conn.close()
    print("\nFX subsystem database synchronization completed successfully.")

if __name__ == '__main__':
    run()
