import struct

with open("assets/LEVELS/AUSTRIA/AUSTRIA.POS", "rb") as f:
    pos_data = f.read()

print(f"AUSTRIA.POS total size: {len(pos_data)}")
# Check first few records
# In 3D racing games, waypoints are often (x, y, z) triplets of ints or shorts
for i in range(10):
    vals_i = struct.unpack("<3i", pos_data[i*12 : (i+1)*12])
    print(f"POS {i} as ints: {vals_i}")
