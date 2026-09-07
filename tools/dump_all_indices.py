import struct

def dump_offsets(path):
    with open(path, "rb") as f:
        data = f.read()

    offsets = struct.unpack("<416i", data[0x0c : 0x0c + 416*4])
    widths = struct.unpack("<224h", data[0x68c : 0x68c + 224*2])
    print(f"\n--- Defined indices in {path} ---")
    res = []
    for i, off in enumerate(offsets):
        if off != -1:
            res.append((i, chr(i) if 32 <= i < 127 else f"#{i}", widths[i], off))
    for r in res:
        print(f"Index {r[0]:3d} ({r[1]:4s}): width={r[2]:2d}, offset={r[3]:5d}")

dump_offsets(r"C:\Stuff\Proyects\RacingDynamite\assets\BALTAZAR\DATA\RED_DARK.LFT")
