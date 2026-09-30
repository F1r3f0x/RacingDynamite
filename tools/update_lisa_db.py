#!/usr/bin/env python3
"""
tools/update_lisa_db.py
Synchronizes authentic DOS linear addresses (0x0005xxxx / 0x0006xxxx)
for all functions in module lisa3d.c in database/decomp.db.
"""

import sqlite3

def run():
    conn = sqlite3.connect("database/decomp.db")
    cur = conn.cursor()

    # Clear existing dos_address entries in lisa3d to prevent collisions during re-addressing
    cur.execute("""
        UPDATE functions 
        SET dos_address = NULL 
        WHERE module_id = (SELECT id FROM modules WHERE name = 'lisa3d.c')
          AND symbol_name != 'Car_UnpackMeshGeometry'
    """)
    print(f"Cleared existing lisa3d dos_addresses: {cur.rowcount} rows reset")

    lisa_updates = [
        ("0x000552fc", "Cdp_OpenFile", "decompiled", "EXACT", "Decodes CDP movie file header and initializes playback stream"),
        ("0x00055384", "Cdp_DecodeFrame", "decompiled", "EXACT", "Advances CDP video stream and decompresses next frame"),
        ("0x00054e00", "Cdp_DecompressRLE", "decompiled", "ADAPTED", "Delta RLE decompressor for CDP video frames"),
        ("0x0005571c", "Lisa_PrintVersion", "decompiled", "EXACT", "Outputs Lisa 3D graphics system version and copyright banner"),
        ("0x00056410", "Lisa_RenderScene", "decompiled", "EXACT", "Master scene rendering loop: frustum culling, vertex transform, depth sorting, and rasterization"),
        ("0x000564d8", "Lisa_InitEngineMemory", "decompiled", "EXACT", "Allocates vertex, triangle, object, and depth bucket memory pools"),
        ("0x00056974", "Lisa_FreeEngineMemory", "decompiled", "EXACT", "Deallocates all dynamic render memory pools"),
        ("0x000569ec", "Lisa_InitSpatialGrid", "decompiled", "EXACT", "Initializes uniform spatial partitioning grid for object culling"),
        ("0x00056b28", "Lisa_CreateDynamicObject", "decompiled", "EXACT", "Allocates and initializes a dynamic 3D entity instance"),
        ("0x00056c74", "Lisa_MoveDynamicObject", "decompiled", "EXACT", "Updates dynamic object world position and re-links in spatial grid"),
        ("0x00056d24", "Lisa_SetDynamicObjectMesh", "decompiled", "EXACT", "Assigns mesh geometry and bounding hierarchy to dynamic object"),
        ("0x00056f8c", "Lisa_UpdateObjectSpatialGrid", "decompiled", "EXACT", "Recomputes cell occupancy in spatial grid following entity translation"),
        ("0x00056ffc", "Lisa_DeleteDynamicObject", "decompiled", "ADAPTED", "Removes dynamic object from spatial grid and returns instance to free pool"),
        ("0x00057014", "Lisa_SetCameraViewport", "decompiled", "ADAPTED", "Sets camera viewport extents and projection aspect ratio"),
        ("0x000570d8", "Lisa_GenerateMipmaps", "decompiled", "EXACT", "Downsamples texture source through box filter pyramid"),
        ("0x00057548", "Lisa_GenerateTextureSpanTable", "decompiled", "EXACT", "Generates pre-scaled scanline UV stepping tables for perspective texture mapper"),
        ("0x00058050", "Lisa_DownsampleTextureMipmap", "decompiled", "EXACT", "2x2 box filter mipmap reduction kernel"),
        ("0x0005829c", "Lisa_FilterTextureBlock", "decompiled", "EXACT", "Bilinear / average filter block kernel for texture page generation"),
        ("0x000586d0", "Lisa_LoadOrCreateShadingTable", "decompiled", "EXACT", "Builds or validates 32-level light ramp shading lookup table from 256-color palette"),
        ("0x000596c4", "Lisa_FindClosestPaletteColor", "decompiled", "EXACT", "Euclidean RGB distance nearest-match palette color search"),
        ("0x0005891c", "Lisa_RenderSkyBackdrop", "decompiled", "EXACT", "Draws gradient horizon or textured panoramic sky background"),
        ("0x00058c1c", "Lisa_CullObjectsOrthographic", "decompiled", "EXACT", "Orthographic AABB view-frustum culling pass for mini-map/mirrors"),
        ("0x00058ebc", "Lisa_FrustumCullObjects", "decompiled", "EXACT", "Perspective 6-plane frustum culling with sphere/AABB hierarchical rejection"),
        ("0x0005a2c8", "Lisa_TransformVertices", "decompiled", "EXACT", "Transforms local mesh coordinates to camera space with homogeneous clipping flags"),
        ("0x0005a8c8", "Lisa_TransformVerticesPanorama", "decompiled", "EXACT", "Specialized vertex transform for panoramic background sky domes"),
        ("0x0005ae60", "Lisa_ComputeObjectMatrix", "decompiled", "EXACT", "Composes pitch-yaw-roll orientation into 3x3 transformation matrix"),
        ("0x0005b308", "Lisa_TransformSubmeshVerticesPanorama", "decompiled", "EXACT", "Transforms hierarchical submesh vertices with compound parent matrices"),
        ("0x0005b7a8", "Lisa_ComputeCameraRotationMatrix", "decompiled", "EXACT", "Computes inverted camera view orientation matrix from Euler angles"),
        ("0x0005b8e8", "Lisa_InitOpcodeTable", "decompiled", "EXACT", "Populates rasterizer polygon drawer jump table based on current shading/filtering modes"),
        ("0x0005b9c4", "Lisa_SortDepthBuckets", "decompiled", "EXACT", "Radix sort depth bucketer ordering polygons back-to-front"),
        ("0x0005bc10", "Lisa_DrawTriangle_Op0F", "decompiled", "EXACT", "Flat-shaded untextured triangle rasterizer (Opcode 0x0F)"),
        ("0x0005bea4", "Lisa_DrawTriangle_Op10", "decompiled", "EXACT", "Gouraud-shaded untextured triangle rasterizer (Opcode 0x10)"),
        ("0x0005cb48", "Lisa_RenderSubmeshes", "decompiled", "EXACT", "Iterates visible entity submeshes and dispatches polygons into depth buckets"),
        ("0x0005c144", "Lisa_DrawTexturedTriangle_Op11_Unshaded", "decompiled", "EXACT", "Textured triangle rasterizer without lighting ramp (Opcode 0x11, Mode 1)"),
        ("0x0005cc58", "Lisa_DrawTexturedTriangle_Op11_Shaded", "decompiled", "EXACT", "Textured triangle rasterizer with 32-level light shading (Opcode 0x11, Mode 0)"),
        ("0x0005d084", "Lisa_RenderTexturedTriangle_Op11", "decompiled", "EXACT", "Textured triangle dispatch helper (Opcode 0x11, Mode 0 Shaded NoFilter)"),
        ("0x0005d6d0", "Lisa_DrawPolygon_Op12", "decompiled", "EXACT", "Polygon drawer Opcode 0x12 dispatch (textured polygon variant)"),
        ("0x0005d6e0", "Lisa_DrawPolygon_Op13", "decompiled", "EXACT", "Polygon drawer Opcode 0x13 dispatch (shaded textured polygon variant)"),
        ("0x0005d6f0", "Lisa_DrawPolygon_Op16", "decompiled", "EXACT", "Polygon drawer Opcode 0x16 dispatch (transparent/translucent polygon variant)"),
        ("0x0005d704", "Lisa_DrawPolygon_Op17", "decompiled", "EXACT", "Polygon drawer Opcode 0x17 dispatch (shaded transparent polygon variant)"),
        ("0x0005d718", "Lisa_DrawTriangle_OpcodeHelper", "decompiled", "EXACT", "Unified rasterizer command bucketer dispatching polygon commands into scanline pipeline"),
        ("0x0005dc08", "Lisa_DrawTriangle_Op14", "decompiled", "EXACT", "Textured triangle rasterizer with transparency chroma keying (Opcode 0x14)"),
        ("0x0005dea0", "Lisa_DrawBillboard_Op07", "decompiled", "EXACT", "Camera-facing billboard sprite rasterizer (Opcode 0x07)"),
        ("0x0005e004", "Lisa_DrawBillboard_Op08", "decompiled", "EXACT", "Scaled camera-facing billboard sprite rasterizer (Opcode 0x08)"),
        ("0x0005e358", "Lisa_DrawTexturedTriangle_Op15", "decompiled", "EXACT", "Perspective-correct filtered textured triangle rasterizer (Opcode 0x15)"),
        ("0x0005ef14", "Lisa_DrawTexturedSpan_Op11", "decompiled", "EXACT", "Textured span rasterizer kernel with bilinear filtering (Opcode 0x11, Mode 0 Filtered)"),
        ("0x00060210", "Lisa_DrawTexturedTriangle_Op15_Sub", "decompiled", "EXACT", "Sub-pixel trapezoid scanline rasterizer kernel for Opcode 0x15"),
        ("0x00060dc4", "Lisa_InitRasterizerTables", "decompiled", "EXACT", "Computes scanline edge stepping tables and reciprocal lookup tables"),
        ("0x00060e3d", "Lisa_ExecuteRasterizerCommands", "decompiled", "EXACT", "Flushes queued draw commands from depth buckets through active opcode drawer functions"),
    ]

    for dos_addr, sym, status, fidelity, purpose in lisa_updates:
        cur.execute("""
            UPDATE functions 
            SET dos_address = ?, status = ?, fidelity = ?, purpose = ?
            WHERE symbol_name = ?
        """, (dos_addr, status, fidelity, purpose, sym))
        print(f"Updated {sym:38} -> {dos_addr} ({status}, {fidelity}): rows={cur.rowcount}")

    # Inlined and special helpers
    cur.execute("""
        UPDATE functions 
        SET dos_address = NULL, status = 'decompiled', fidelity = 'EXACT', 
            notes = 'Inlined into Lisa_FrustumCullObjects @ 0x00058ebc',
            purpose = 'Hierarchical frustum culling dispatcher'
        WHERE symbol_name = 'Lisa_CullObjects'
    """)
    print(f"Updated Lisa_CullObjects (inlined): rows={cur.rowcount}")

    cur.execute("""
        UPDATE functions 
        SET dos_address = NULL, status = 'decompiled', fidelity = 'ADAPTED',
            purpose = 'Panoramic backdrop sky blitter sampling active .PAN texture'
        WHERE symbol_name = 'Lisa_RenderPanorama'
    """)
    print(f"Updated Lisa_RenderPanorama: rows={cur.rowcount}")

    cur.execute("""
        UPDATE functions 
        SET dos_address = NULL, status = 'decompiled', fidelity = 'EXACT',
            purpose = 'VGA Mode 13h DAC palette update wrapper'
        WHERE symbol_name = 'Video_SetPalette'
    """)
    print(f"Updated Video_SetPalette: rows={cur.rowcount}")

    conn.commit()
    conn.close()
    print("\nLisa3D database synchronization completed successfully.")

if __name__ == '__main__':
    run()
