with open("assets/LEVELS/AUSTRIA/AUSTRIA.TEX", "rb") as f:
    h = f.read(192)

print("First 32 bytes of AUSTRIA.TEX (hex):", h[:32].hex())
print("First 16 shorts:", [int.from_bytes(h[i:i+2], 'little') for i in range(0, 32, 2)])
print("First 8 ints:", [int.from_bytes(h[i:i+4], 'little') for i in range(0, 32, 4)])
print("Total file size:", f.seek(0, 2))
