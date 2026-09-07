import os
import struct

def inspect_msh_tri(msh_path, tri_path, label):
    if not os.path.exists(msh_path) or not os.path.exists(tri_path):
        return
    msh_sz = os.path.getsize(msh_path)
    tri_sz = os.path.getsize(tri_path)
    
    with open(msh_path, "rb") as f:
        msh_head = f.read(32)
    with open(tri_path, "rb") as f:
        tri_head = f.read(32)
        
    msh_ints = struct.unpack("<8i", msh_head)
    tri_ints = struct.unpack("<8i", tri_head)
    print(f"=== {label} ===")
    print(f"MSH size={msh_sz}: {msh_ints[:4]}")
    print(f"TRI size={tri_sz}: {tri_ints[:4]}")

inspect_msh_tri(r"assets\LEVELS\AUSTRIA\AUSTRIA.MSH", r"assets\LEVELS\AUSTRIA\AUSTRIA.TRI", "Track Austria")
inspect_msh_tri(r"assets\LEVELS\USA\USA.MSH", r"assets\LEVELS\USA\USA.TRI", "Track USA")
inspect_msh_tri(r"assets\CARS\COOPER\COOPER.MSH", r"assets\CARS\COOPER\COOPER.TRI", "Car Cooper")
inspect_msh_tri(r"assets\CARS\PORSCHE\PORSCHE.MSH", r"assets\CARS\PORSCHE\PORSCHE.TRI", "Car Porsche")
