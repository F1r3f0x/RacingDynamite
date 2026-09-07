import math
import struct

with open("assets/LEVELS/AUSTRIA/AUSTRIA.COL", "rb") as f:
    col_data = f.read()[8:8+768]

with open("assets/LEVELS/AUSTRIA/AUSTRIA.TAB", "rb") as f:
    tab_data = f.read()

with open("assets/LEVELS/AUSTRIA/AUSTRIA.TEX", "rb") as f:
    tex_data = f.read()

# Verify tab lookup:
# Row is col, Col is light_level (0 = full bright, 31 = dark)
for c in [50, 100, 150, 200, 251]:
    full_bright = tab_data[c * 256 + 0]
    mid_bright = tab_data[c * 256 + 15]
    dark = tab_data[c * 256 + 31]
    print(f"Color {c:3d}: bright={full_bright:3d}, mid={mid_bright:3d}, dark={dark:3d}")
