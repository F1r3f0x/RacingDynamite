import struct
from PIL import Image

with open('Ignition/Ignition/BALTAZAR/DATA/MENU.COL', 'rb') as f:
    col = list(f.read()[8:8+768])

with open('Ignition/Ignition/BALTAZAR/DATA/IGN3_0.CDP', 'rb') as f:
    data = f.read()

magic, frame_cnt, loop, w, h = struct.unpack('<4shhhh', data[:12])
print(f"IGN3_0.CDP: magic={magic} frames={frame_cnt} w={w} h={h} size={len(data)}")

# Let's inspect frame 0
# Palette is at 0x10 (768 bytes) if present, or from MENU.COL
# In CDP format, frame offsets or RLE chunks follow
