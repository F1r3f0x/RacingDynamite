import os
import struct

assets_dir = r"C:\Stuff\Proyects\RacingDynamite\assets"
for root, dirs, files in os.walk(assets_dir):
    for f in files:
        if f.upper().endswith('.PIC'):
            p = os.path.join(root, f)
            with open(p, 'rb') as fp:
                data = fp.read(64)
            val0, val1, val2, val3 = struct.unpack('<HHHH', data[:8])
            rel = os.path.relpath(p, assets_dir)
            print(f"{rel:<25} size={os.path.getsize(p):<7} w={val0:<5} h={val1:<5} 0x{val0:04x} 0x{val1:04x} 0x{val2:04x} 0x{val3:04x}")
