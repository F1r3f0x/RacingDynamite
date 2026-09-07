import os
import struct

def check_font(path):
    with open(path, "rb") as f:
        data = f.read()
    offsets = struct.unpack("<224i", data[0x0c : 0x0c + 224*4])
    defined = [i for i, off in enumerate(offsets) if off != -1]
    defined_ascii = "".join(chr(i) for i in defined if 32 <= i < 127)
    name = os.path.basename(path)
    print(f"{name:15s}: count={len(defined):3d}, chars: {defined_ascii}")

for root, dirs, files in os.walk("assets"):
    for f in files:
        if f.upper().endswith(".LFT"):
            check_font(os.path.join(root, f))
