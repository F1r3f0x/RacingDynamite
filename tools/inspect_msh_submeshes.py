import struct

with open("assets/LEVELS/AUSTRIA/AUSTRIA.MSH", "rb") as f:
    msh = f.read()

total_size = len(msh)
print(f"AUSTRIA.MSH total size: {total_size}")

# Let's inspect submeshes
pos = 0
submeshes = []
while pos + 8 <= total_size:
    v_count = struct.unpack("<I", msh[pos:pos+4])[0]
    if v_count == 0 or v_count > 10000:
        pos += 4
        continue
    
    # Check if next bytes look like 3D vertices (int32 triplets)
    v_bytes = v_count * 12
    if pos + 4 + v_bytes <= total_size:
        submeshes.append((pos, v_count))
        pos += 4 + v_bytes
    else:
        pos += 4

print(f"Found {len(submeshes)} submeshes in AUSTRIA.MSH")
# Print stats of first 5 submeshes
for i in range(min(5, len(submeshes))):
    spos, vc = submeshes[i]
    v_data = msh[spos+4 : spos+4+vc*12]
    coords = [struct.unpack("<3i", v_data[j*12:(j+1)*12]) for j in range(vc)]
    xs = [c[0] for c in coords]
    ys = [c[1] for c in coords]
    zs = [c[2] for c in coords]
    print(f"Submesh {i} at offset 0x{spos:06x}: {vc} vertices, X=[{min(xs)}, {max(xs)}], Y=[{min(ys)}, {max(ys)}], Z=[{min(zs)}, {max(zs)}]")
