#!/usr/bin/env python3
"""
tools/update_lisa_source_annotations.py
Updates @original and @fidelity header comments in decomp/src/lisa3d.c
to cite verified DOS linear addresses (0x0005xxxx / 0x0006xxxx).
"""

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LISA3D_C = ROOT / "decomp" / "src" / "lisa3d.c"

updates = {
    "Lisa_PrintVersion": ("0x0005571c", "EXACT"),
    "Cdp_OpenFile": ("0x000552fc", "EXACT"),
    "Cdp_DecompressRLE": ("0x00054e00", "ADAPTED"),
    "Cdp_DecodeFrame": ("0x00055384", "EXACT"),
    "Lisa_RenderPanorama": (None, "ADAPTED"),
    "Lisa_DrawTexturedTriangle_Op11_Unshaded": ("0x0005c144", "EXACT"),
    "Lisa_DrawTexturedTriangle_Op11_Shaded": ("0x0005cc58", "EXACT"),
    "Lisa_RenderScene": ("0x00056410", "EXACT"),
    "Lisa_InitEngineMemory": ("0x000564d8", "EXACT"),
    "Lisa_FreeEngineMemory": ("0x00056974", "EXACT"),
    "Lisa_InitSpatialGrid": ("0x000569ec", "EXACT"),
    "Lisa_CreateDynamicObject": ("0x00056b28", "EXACT"),
    "Lisa_MoveDynamicObject": ("0x00056c74", "EXACT"),
    "Lisa_UpdateObjectSpatialGrid": ("0x00056f8c", "EXACT"),
    "Lisa_SetDynamicObjectMesh": ("0x00056d24", "EXACT"),
    "Lisa_DeleteDynamicObject": ("0x00056ffc", "ADAPTED"),
    "Lisa_SetCameraViewport": ("0x00057014", "ADAPTED"),
    "Lisa_GenerateMipmaps": ("0x000570d8", "EXACT"),
    "Lisa_GenerateTextureSpanTable": ("0x00057548", "EXACT"),
    "Lisa_DownsampleTextureMipmap": ("0x00058050", "EXACT"),
    "Lisa_FilterTextureBlock": ("0x0005829c", "EXACT"),
    "Lisa_LoadOrCreateShadingTable": ("0x000586d0", "EXACT"),
    "Lisa_FindClosestPaletteColor": ("0x000596c4", "EXACT"),
    "Lisa_RenderSkyBackdrop": ("0x0005891c", "EXACT"),
    "Lisa_CullObjectsOrthographic": ("0x00058c1c", "EXACT"),
    "Lisa_FrustumCullObjects": ("0x00058ebc", "EXACT"),
    "Lisa_CullObjects": ("0x00058ebc", "EXACT"),
    "Lisa_TransformVertices": ("0x0005a2c8", "EXACT"),
    "Lisa_TransformVerticesPanorama": ("0x0005a8c8", "EXACT"),
    "Lisa_ComputeObjectMatrix": ("0x0005ae60", "EXACT"),
    "Lisa_TransformSubmeshVerticesPanorama": ("0x0005b308", "EXACT"),
    "Lisa_ComputeCameraRotationMatrix": ("0x0005b7a8", "EXACT"),
    "Lisa_InitOpcodeTable": ("0x0005b8e8", "EXACT"),
    "Lisa_SortDepthBuckets": ("0x0005b9c4", "EXACT"),
    "Lisa_DrawTriangle_Op0F": ("0x0005bc10", "EXACT"),
    "Lisa_DrawTriangle_Op10": ("0x0005bea4", "EXACT"),
    "Lisa_RenderSubmeshes": ("0x0005cb48", "EXACT"),
    "Lisa_DrawTriangle_OpcodeHelper": ("0x0005d718", "EXACT"),
    "Lisa_DrawTriangle_Op14": ("0x0005dc08", "EXACT"),
    "Lisa_DrawBillboard_Op07": ("0x0005dea0", "EXACT"),
    "Lisa_DrawBillboard_Op08": ("0x0005e004", "EXACT"),
    "Lisa_DrawTexturedTriangle_Op15": ("0x0005e358", "EXACT"),
    "Lisa_DrawTexturedTriangle_Op15_Sub": ("0x00060210", "EXACT"),
    "Lisa_InitRasterizerTables": ("0x00060dc4", "EXACT"),
    "Lisa_ExecuteRasterizerCommands": ("0x00060e3d", "EXACT"),
    "Car_UnpackMeshGeometry": ("0x00016616", "EXACT"),
    "Lisa_DrawPolygon_Op12": ("0x0005d6d0", "EXACT"),
    "Lisa_DrawPolygon_Op13": ("0x0005d6e0", "EXACT"),
    "Lisa_DrawPolygon_Op16": ("0x0005d6f0", "EXACT"),
    "Lisa_DrawPolygon_Op17": ("0x0005d704", "EXACT"),
    "Lisa_RenderTexturedTriangle_Op11": ("0x0005d084", "EXACT"),
    "Lisa_DrawTexturedSpan_Op11": ("0x0005ef14", "EXACT"),
    "Video_SetPalette": (None, "EXACT"),
}

def run():
    with open(LISA3D_C, "r", encoding="utf-8") as f:
        lines = f.readlines()

    modified = False
    new_lines = []
    i = 0
    while i < len(lines):
        line = lines[i]
        orig_match = re.search(r"(\s*\*\s*@original\s+)([A-Za-z0-9_]+)(\s*\(MAINDOS(?:_32BIT)?\.EXE)([^,]*)(,\s*lisa3d\.c\))", line)
        if orig_match:
            prefix, sym, mid, old_addr, suffix = orig_match.groups()
            if sym in updates:
                new_addr, new_fid = updates[sym]
                if new_addr:
                    new_line = f"{prefix}{sym} (MAINDOS.EXE @ {new_addr}{suffix}\n"
                else:
                    new_line = f"{prefix}{sym} (MAINDOS.EXE{suffix}\n"
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
                modified = True
                continue
        new_lines.append(line)
        i += 1

    with open(LISA3D_C, "w", encoding="utf-8") as f:
        f.writelines(new_lines)

    print(f"Updated lisa3d.c annotations successfully.")

if __name__ == '__main__':
    run()
