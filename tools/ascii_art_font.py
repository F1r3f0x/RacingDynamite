import struct

with open(r"C:\Stuff\Proyects\RacingDynamite\assets\FONTS\SMALL.LFT", "rb") as f:
    data = f.read()

height = struct.unpack("<h", data[10:12])[0]
offsets = struct.unpack("<224i", data[0x0c : 0x0c + 224*4])
widths = struct.unpack("<224h", data[0x68c : 0x68c + 224*2])
pixels = data[0x84c:]

# Let's inspect indices 0 to 127
# For each index that has pixels, let's print a small ascii art of the glyph!
for idx in range(128):
    off = offsets[idx]
    w = widths[idx]
    if off < 0 or w <= 0:
        continue
    
    # Check if this glyph looks like a letter
    lines = []
    for r in range(height):
        line = ""
        for c in range(w):
            p = pixels[off + r * w + c] if (off + r * w + c) < len(pixels) else 0
            line += "#" if p != 0 else "."
        lines.append(line)
    
    # Print if it has content
    if any("#" in l for l in lines):
        print(f"--- Index {idx} (width={w}) ---")
        for l in lines:
            print("  " + l)
