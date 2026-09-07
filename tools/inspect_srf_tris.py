import struct

with open(r"C:\Stuff\Proyects\RacingDynamite\assets\LEVELS\AUSTRIA\AUSTRIA.SRF", "rb") as fp:
    hdr = struct.unpack('<9i', fp.read(36))
    grid_w, grid_h = hdr[4], hdr[5]
    grid_bytes = grid_w * grid_h * 12
    grid_raw = fp.read(grid_bytes)
    
    # Read first 10 triangles
    tri_count = hdr[6]
    print(f"Total triangles: {tri_count}")
    for i in range(10):
        tri_raw = fp.read(24)
        v = struct.unpack('<6i', tri_raw)
        print(f"Tri {i:>2}: v0={v[0]:<5} v1={v[1]:<5} v2={v[2]:<5} mat=0x{v[3]:08x} nX={v[4]:<6} nZ={v[5]:<6}")
