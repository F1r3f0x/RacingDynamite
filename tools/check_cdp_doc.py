import struct
from PIL import Image

with open('Ignition/Ignition/BALTAZAR/DATA/IGN3_0.CDP', 'rb') as f:
    data = f.read()

magic, frame_cnt, loop, w, h = struct.unpack('<4shhhh', data[:12])
palette = list(data[0x10:0x10+768])
frame_data_start = 0x310

# In CDP format:
# Each frame has a header or size
# Let's check format from docs/formats/cdp.md
with open('docs/formats/cdp.md', 'r') as f:
    print(f.read())
