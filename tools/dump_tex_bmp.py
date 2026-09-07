import struct

with open("assets/LEVELS/AUSTRIA/AUSTRIA.COL", "rb") as f:
    col = f.read()[8:8+768]

with open("assets/LEVELS/AUSTRIA/AUSTRIA.TEX", "rb") as f:
    tex = f.read()

print("TEX size:", len(tex))
img_w, img_h = 1024, 1024
row_stride = 1024
bmp_pixels = bytearray(1024 * 1024)
for r in range(1024):
    dst_r = 1023 - r
    bmp_pixels[dst_r * 1024 : (dst_r + 1) * 1024] = tex[r * 1024 : (r + 1) * 1024]

color_table = bytearray()
for i in range(256):
    color_table.extend([col[i*3+2], col[i*3+1], col[i*3+0], 0])

total_size = 54 + 1024 + len(bmp_pixels)
bmp_hdr = struct.pack("<2sIHHI", b"BM", total_size, 0, 0, 54 + 1024)
dib_hdr = struct.pack("<IIIHHIIIIII", 40, 1024, 1024, 1, 8, 0, len(bmp_pixels), 2835, 2835, 256, 256)

with open(r"C:\Stuff\Proyects\RacingDynamite\docs\extracted_bitmaps\AUSTRIA_TEX.bmp", "wb") as f:
    f.write(bmp_hdr)
    f.write(dib_hdr)
    f.write(color_table)
    f.write(bmp_pixels)

print("Saved AUSTRIA_TEX.bmp")
