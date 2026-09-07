import struct

def inspect_font(path):
    with open(path, "rb") as f:
        data = f.read()

    magic = data[:4]
    version, ascii_base, max_w, height = struct.unpack("<4H", data[4:12])
    print(f"\nFont: {path}")
    print(f"Header: version={version}, ascii_base={ascii_base}, max_w={max_w}, height={height}")

    # Offsets array at 0x0C
    offsets = struct.unpack("<416i", data[0x0c : 0x0c + 416*4])
    widths = struct.unpack("<224h", data[0x68c : 0x68c + 224*2])

    print("First 30 defined characters:")
    count = 0
    for i in range(len(offsets)):
        if offsets[i] != -1:
            ch = chr(i) if 32 <= i < 127 else '?'
            w = widths[i] if i < len(widths) else -1
            print(f"  Index {i:3d} (char '{ch}'): offset={offsets[i]:5d}, width={w}")
            count += 1
            if count >= 30:
                break

inspect_font(r"C:\Stuff\Proyects\RacingDynamite\assets\FONTS\SMALL.LFT")
inspect_font(r"C:\Stuff\Proyects\RacingDynamite\assets\BALTAZAR\DATA\RED_DARK.LFT")
