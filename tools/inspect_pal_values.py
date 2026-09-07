import struct

with open(r"C:\Stuff\Proyects\RacingDynamite\assets\INSTALL.PIC", "rb") as f:
    hdr = f.read(846)
    pal_pic = hdr[78:78+768]

with open(r"C:\Stuff\Proyects\RacingDynamite\assets\SYS.COL", "rb") as f:
    hdr_col = f.read(776)
    pal_col = hdr_col[8:8+768]

print("INSTALL.PIC max byte in palette:", max(pal_pic))
print("SYS.COL max byte in palette:", max(pal_col))

print("\nFirst 10 colors in INSTALL.PIC (RGB):")
for i in range(10):
    r, g, b = pal_pic[i*3 : i*3+3]
    print(f"[{i:02d}] R={r:3d}, G={g:3d}, B={b:3d}")

print("\nFirst 10 colors in SYS.COL (RGB):")
for i in range(10):
    r, g, b = pal_col[i*3 : i*3+3]
    print(f"[{i:02d}] R={r:3d}, G={g:3d}, B={b:3d}")

print("\nColors 16 to 32 in INSTALL.PIC (RGB):")
for i in range(16, 32):
    r, g, b = pal_pic[i*3 : i*3+3]
    print(f"[{i:02d}] R={r:3d}, G={g:3d}, B={b:3d}")

print("\nColors 16 to 32 in SYS.COL (RGB):")
for i in range(16, 32):
    r, g, b = pal_col[i*3 : i*3+3]
    print(f"[{i:02d}] R={r:3d}, G={g:3d}, B={b:3d}")
