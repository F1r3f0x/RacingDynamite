import struct

with open("assets/LEVELS/AUSTRIA/AUSTRIA.MSH", "rb") as f:
    msh = f.read()

vc = struct.unpack("<i", msh[:4])[0]
vertices = []
for i in range(vc):
    v = struct.unpack("<3i", msh[4 + i*12 : 4 + (i+1)*12])
    vertices.append(v)

print(f"Decoded {len(vertices)} vertices for Submesh 0:")
for i, v in enumerate(vertices[:5]):
    print(f"  V{i}: {v}")

poly_start = 4 + vc * 12
poly_count = struct.unpack("<i", msh[poly_start : poly_start + 4])[0]
print(f"Polygon count: {poly_count}")

# Inspect polygon record structure
# Record 1:
# 11 00 04 00 -> flags?
# 00 00 00 00 -> idx 0
# 01 00 00 00 -> idx 1
# 02 00 00 00 -> idx 2
# then UV coords?
p_ptr = poly_start + 4
for p in range(min(5, poly_count)):
    header_val = struct.unpack("<I", msh[p_ptr : p_ptr + 4])[0]
    i0, i1, i2 = struct.unpack("<3I", msh[p_ptr + 4 : p_ptr + 16])
    # Let's inspect next 24 bytes (6 ints or 6 floats or 6 fixed-point 16.16)
    extra = struct.unpack("<6I", msh[p_ptr + 16 : p_ptr + 40])
    print(f"\nPoly {p}: hdr=0x{header_val:08x}, indices=({i0}, {i1}, {i2})")
    print(f"  Extra 6 ints: {[hex(x) for x in extra]}")
    # Check if extra are fixed 16.16 UVs:
    uvs = [x / 65536.0 for x in extra]
    print(f"  Extra as 16.16: {[round(x, 2) for x in uvs]}")
    p_ptr += 40
