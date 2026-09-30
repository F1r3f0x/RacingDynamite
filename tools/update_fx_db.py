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
        (None,         "Lisa_Init", "decompiled", "ADAPTED", "Calculates camera viewports and FOV scaling tables for current screen resolution"),
        (None,         "Lisa_FlushRasterizerCommands", "decompiled", "EXACT", "Command queue flush dispatcher calling Lisa_ExecuteRasterizerCommands"),
        (None,         "Math_LookupTrigAngle", "decompiled", "EXACT", "Computes arctangent angle lookup"),
        ("0x00038e6d", "Camera_UpdateOverview", "decompiled", "EXACT", "Simulates blimp overview camera following leading vehicles"),
        ("0x00037883", "Camera_UpdateChase", "decompiled", "EXACT", "3rd-person chase camera dynamics, yaw smoothing, and road pitch tracking"),
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
        (None,         "FX_SpawnTireDirtDebris", "decompiled", "ADAPTED", "Emits dirt and gravel kick-up particles behind spinning tires"),
        (None,         "FX_SpawnLandingDustPuffs", "decompiled", "ADAPTED", "Spawns impact dust clouds when car suspension compresses upon landing"),
        ("0x0003e6ec", "FX_SpawnTireSkidSmoke", "decompiled", "EXACT", "Spawns tire friction smoke particles when vehicle drifts or brakes aggressively"),
        (None,         "FX_UpdateExplosionNode", "decompiled", "ADAPTED", "Animates fireball expansion and debris dispersion for destroyed vehicles"),
        ("0x0003c208", "FX_UpdateVehicleWreck", "decompiled", "EXACT", "Detaches wheels, triggers fire/smoke emitters, and disables physics on wrecked car"),
        (None,         "FX_UpdateDetachedWheel", "decompiled", "ADAPTED", "Simulates ballistic bouncing physics for tires sheared off during collisions"),
        ("0x0003db4c", "FX_UpdateVehicleCrashSequence", "decompiled", "EXACT", "Multi-frame rollover and flip crash trajectory integration"),
        (None,         "FX_UpdateCarDebris", "decompiled", "ADAPTED", "Simulates tumbling body panel fragments and glass shards"),
        (None,         "FX_FrameTick", "decompiled", "ADAPTED", "Master particle simulation tick updating all emitters and active effects"),
        (None,         "FX_UpdateAllParticles", "decompiled", "EXACT", "Wrapper dispatching particle simulation frame tick"),
        ("0x000534cc", "HUD_RenderPauseMenu", "decompiled", "EXACT", "Renders in-race pause menu overlay and handles Continue / Restart / Quit navigation"),
        ("0x000539f8", "HUD_RenderConfirmationPrompt", "decompiled", "EXACT", "Renders modal confirmation prompt for race restart and exit confirmation"),
        (None,         "HUD_RenderTrackResults", "decompiled", "ADAPTED", "Renders end-of-race leaderboard standings, split times, and championship points"),
        (None,         "HUD_RenderPlayAgainPrompt", "decompiled", "ADAPTED", "Displays play-again / next-track prompt following race completion"),
        (None,         "HUD_RenderTelemetryOverlay", "decompiled", "ADAPTED", "Displays optional race debug telemetry, speedometer, and engine RPM indicators"),
        (None,         "HUD_CheckWrongWayHeading", "decompiled", "ADAPTED", "Compares vehicle velocity vector against track spline tangent to detect wrong-way driving"),
        ("0x0003a788", "HUD_RenderPlayerElements", "decompiled", "EXACT", "Renders in-game dashboard HUD: tachometer, mini-map, position, lap timer, and turbo gauge"),
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
