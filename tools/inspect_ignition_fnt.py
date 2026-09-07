import os
import struct

path = r"C:\Stuff\Proyects\RacingDynamite\assets\FONTS\IGNITION.FNT"
print("Exists:", os.path.exists(path))
if os.path.exists(path):
    with open(path, "rb") as f:
        data = f.read()
    print("Size:", len(data))
    height = struct.unpack("<h", data[2:4])[0]
    spacing = struct.unpack("<h", data[4:6])[0]
    print(f"Header: height={height}, spacing={spacing}")
    
    # Check geputget format:
    # 0xC6 + char -> glyph index
    # 0x06 + glyph_idx -> width
    # 0x1C6 + glyph_idx*4 -> offset
    # 0x546 + offset -> pixels
    test_str = "PRESS ENTER"
    print("geputget mapping test:")
    for ch in test_str:
        c = ord(ch)
        g_idx = data[0xC6 + c]
        w = data[6 + g_idx]
        off = struct.unpack("<i", data[0x1C6 + g_idx*4 : 0x1C6 + g_idx*4 + 4])[0]
        print(f"'{ch}' ({c}) -> glyph_idx={g_idx:2d}, width={w:2d}, off={off}")
