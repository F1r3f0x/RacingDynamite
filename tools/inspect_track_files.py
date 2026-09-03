import os
import struct

track_dir = r"C:\Stuff\Proyects\RacingDynamite\assets\LEVELS\AUSTRIA"

for fname in ["AUSTRIA.SRF", "AUSTRIA.MSH", "AUSTRIA.TRI", "AUSTRIA.POS", "AUSTRIA.PLC", "AUSTRIA.TAB"]:
    p = os.path.join(track_dir, fname)
    if os.path.exists(p):
        sz = os.path.getsize(p)
        with open(p, "rb") as f:
            head = f.read(32)
        ints = struct.unpack("<IIIIIIII", head)
        hex_ints = [f"0x{x:08x}" for x in ints[:4]]
        dec_ints = ints[:4]
        print(f"{fname:<12} (size {sz:>7}): {dec_ints} | {hex_ints}")
