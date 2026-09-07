import struct

with open(r"C:\Stuff\Proyects\RacingDynamite\assets\FONTS\IGNITION.FNT", "rb") as f:
    data = f.read()

height = struct.unpack("<h", data[2:4])[0]
spacing = struct.unpack("<h", data[4:6])[0]
ascii_map = data[0xC6 : 0xC6 + 256]
widths = data[6 : 6 + 256]
offsets = struct.unpack("<256i", data[0x1C6 : 0x1C6 + 256*4])

test_strings = [
    "PRESS ENTER OR SPACE TO START",
    "SINGLE RACE",
    "CHAMPIONSHIP",
    "TIME ATTACK",
    "OPTIONS",
    "QUIT",
    "SELECT VEHICLE",
    "SELECT CIRCUIT",
    "AUSTRIA", "BRAZIL", "CANADA", "CARIB", "ICELAND", "JAPAN", "USA"
]

print("Verifying all game text in IGNITION.FNT:")
all_ok = True
for s in test_strings:
    missing = []
    for ch in s:
        if ch == " ": continue
        c = ord(ch)
        g_idx = ascii_map[c]
        if g_idx == 255 or offsets[g_idx] < 0:
            missing.append(ch)
    if missing:
        print(f"FAILED: '{s}' has missing chars: {missing}")
        all_ok = False
    else:
        print(f"[OK] '{s}'")

if all_ok:
    print("\nALL GAME STRINGS ARE 100% SUPPORTED BY IGNITION.FNT!")
