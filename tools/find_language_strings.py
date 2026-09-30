with open('Ignition/Ignition/MAINDOS_32BIT.EXE', 'rb') as f:
    data = f.read()

import re

# Find occurrences of English menu strings and see how they are structured
phrases = [
    b'SINGLE RACE',
    b'CHAMPIONSHIP',
    b'TIME ATTACK',
    b'OPTIONS',
    b'QUIT',
    b'SELECT YOUR CAR',
    b'SELECT TRACK',
]

for p in phrases:
    pos = 0
    while True:
        pos = data.find(p, pos)
        if pos == -1: break
        va = 0x000a0000 + (pos - 0x77800)
        print(f"Phrase '{p.decode()}' at offset 0x{pos:06x} -> VA 0x{va:08x}")
        # print surrounding 100 bytes
        chunk = data[max(0, pos-20):min(len(data), pos+80)]
        print("  surrounding:", chunk)
        pos += len(p)
