import struct

with open(r"C:\Stuff\Proyects\RacingDynamite\assets\FONTS\SMALL.LFT", "rb") as f:
    data = f.read()

offsets = struct.unpack("<224i", data[0x0c : 0x0c + 224*4])
widths = struct.unpack("<224h", data[0x68c : 0x68c + 224*2])

print("Total defined in SMALL.LFT:")
for i in range(224):
    if offsets[i] != -1:
        ch = chr(i) if 32 <= i < 127 else f"#{i}"
        print(f"Index {i:3d} ({ch:4s}): width={widths[i]:2d}, offset={offsets[i]:5d}")
