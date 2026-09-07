import struct

with open(r"C:\Stuff\Proyects\RacingDynamite\assets\FONTS\SMALL.LFT", "rb") as f:
    data = f.read()

height = struct.unpack("<h", data[2:4])[0]
spacing = struct.unpack("<h", data[4:6])[0]
print(f"SMALL.LFT: size={len(data)} height={height} spacing={spacing}")

ascii_map = data[0xc6 : 0xc6 + 256]
print(f"ASCII 'A' ({ord('A')}) maps to glyph index: {ascii_map[ord('A')]}")
print(f"ASCII '0' ({ord('0')}) maps to glyph index: {ascii_map[ord('0')]}")

glyph_A = ascii_map[ord('A')]
width_A = data[6 + glyph_A]
offset_A = struct.unpack("<i", data[0x1c6 + glyph_A*4 : 0x1c6 + glyph_A*4 + 4])[0]
print(f"Glyph 'A': width={width_A} offset={offset_A}")
