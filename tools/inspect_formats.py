import os
import struct

col_path = r"C:\Stuff\Proyects\RacingDynamite\assets\SYS.COL"
with open(col_path, "rb") as f:
    col_data = f.read()

print(f"SYS.COL size: {len(col_data)} bytes")
# Check if 768 bytes (256 * 3) or 776 bytes (8-byte header + 768)
if len(col_data) == 776:
    hdr = col_data[:8]
    print(f"Header: {hdr.hex()} ({hdr})")
    palette = col_data[8:]
    print(f"Palette length: {len(palette)} bytes (256 * 3 = {256*3})")
    print("First 4 RGB entries:")
    for i in range(4):
        r, g, b = palette[i*3 : (i+1)*3]
        print(f"  Color {i}: R={r} G={g} B={b}")

# Check .PIC files
pic_path = r"C:\Stuff\Proyects\RacingDynamite\assets\H_SIGNS.PIC"
with open(pic_path, "rb") as f:
    pic_data = f.read()

print(f"\nH_SIGNS.PIC size: {len(pic_data)} bytes")
print("First 32 bytes:")
print(pic_data[:32].hex())
w, h = struct.unpack("<HH", pic_data[:4])
print(f"Possible width: {w}, height: {h} (w*h = {w*h})")
