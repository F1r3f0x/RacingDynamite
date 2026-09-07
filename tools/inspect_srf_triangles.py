import struct

with open("assets/LEVELS/AUSTRIA/AUSTRIA.SRF", "rb") as f:
    data = f.read()

# Header is 24 bytes
header = struct.unpack("<6i", data[:24])
grid_w, grid_d, cell_w, cell_d, num_tri, num_t1 = header
print(f"SRF: {num_tri} triangles, grid {grid_w}x{grid_d}, cell {cell_w}x{cell_d}")

# Triangles start at offset 24 + grid_w * grid_d * 8
tri_offset = 24 + grid_w * grid_d * 8
print(f"Triangle table offset: 0x{tri_offset:06x} ({tri_offset})")

# Each triangle is 24 bytes:
# int32 v0_y, v1_y, v2_y
# int16 nx, nz
# uint16 flags, material, unk1, unk2
triangles = []
for i in range(min(10, num_tri)):
    off = tri_offset + i * 24
    v0_y, v1_y, v2_y = struct.unpack("<3i", data[off : off + 12])
    nx, nz, flags, mat, u1, u2 = struct.unpack("<2h4H", data[off + 12 : off + 24])
    print(f"Tri {i:3d}: Y=({v0_y}, {v1_y}, {v2_y}), normal=({nx}, {nz}), flags=0x{flags:04x}, mat={mat}")
