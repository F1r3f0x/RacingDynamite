import struct

with open("assets/LEVELS/AUSTRIA/AUSTRIA.PLC", "rb") as f:
    data = f.read()

count = struct.unpack("<I", data[:4])[0]
print(f"AUSTRIA.PLC count: {count}")

xs, ys, zs = [], [], []
for i in range(count):
    m_id, off, x, y, z = struct.unpack("<2h3i", data[4 + i*16 : 4 + (i+1)*16])
    xs.append(x)
    ys.append(y)
    zs.append(z)

print(f"X bounds: min={min(xs)}, max={max(xs)}, avg={sum(xs)//len(xs)}")
print(f"Y bounds: min={min(ys)}, max={max(ys)}, avg={sum(ys)//len(ys)}")
print(f"Z bounds: min={min(zs)}, max={max(zs)}, avg={sum(zs)//len(zs)}")

# Check first 5 objects
for i in range(5):
    m_id, off, x, y, z = struct.unpack("<2h3i", data[4 + i*16 : 4 + (i+1)*16])
    print(f"Object {i}: model={m_id}, off={off}, pos=({x}, {y}, {z})")
