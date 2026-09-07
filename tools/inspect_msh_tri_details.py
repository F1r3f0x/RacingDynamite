import struct

with open(r"C:\Stuff\Proyects\RacingDynamite\assets\LEVELS\AUSTRIA\AUSTRIA.TRI", "rb") as f:
    tri_data = f.read()

chunk_count = struct.unpack("<I", tri_data[:4])[0]
print(f"AUSTRIA.TRI: {chunk_count} chunks, total size = {len(tri_data)}")
print(f"Expected size = 4 + {chunk_count} * 500 = {4 + chunk_count * 500}")

with open(r"C:\Stuff\Proyects\RacingDynamite\assets\LEVELS\AUSTRIA\AUSTRIA.MSH", "rb") as f:
    msh_data = f.read()

print(f"AUSTRIA.MSH: total size = {len(msh_data)}")
# Look at first 10 integers of MSH
first_ints = struct.unpack("<16i", msh_data[:64])
print("AUSTRIA.MSH first 16 ints:", first_ints)

# Inspect chunk 0 of TRI
c0 = tri_data[4 : 4 + 500]
print("Chunk 0 first 32 bytes (hex):", c0[:32].hex())
print("Chunk 0 as shorts (first 16):", struct.unpack("<16h", c0[:32]))
