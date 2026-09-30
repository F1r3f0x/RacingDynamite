with open('Ignition/Ignition/MAINDOS_32BIT.EXE', 'rb') as f:
    data = f.read()

import re

# Look in 0x098000 to 0x09a000
chunk = data[0x098000:0x09a000]

# Split by null bytes and filter strings
entries = []
cur = bytearray()
for b in chunk:
    if 32 <= b <= 126 or b in [196, 214, 220, 223, 224, 225, 232, 233, 234, 238, 239, 244, 249, 251]:
        cur.append(b)
    else:
        if len(cur) >= 3:
            entries.append(cur.decode('latin1', errors='replace'))
        cur = bytearray()

print(f"Found {len(entries)} strings in range 0x098000..0x09a000")
for s in entries[:100]:
    print(s)
