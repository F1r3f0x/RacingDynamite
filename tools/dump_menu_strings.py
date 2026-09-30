with open('Ignition/Ignition/MAINDOS_32BIT.EXE', 'rb') as f:
    data = f.read()

# Let's inspect strings around 0x078000 to 0x07b500
chunk = data[0x078000:0x07b500]

entries = []
cur = bytearray()
for b in chunk:
    if 32 <= b <= 126 or b in [196, 214, 220, 223, 224, 225, 232, 233, 234, 238, 239, 244, 249, 251]:
        cur.append(b)
    else:
        if len(cur) >= 3:
            entries.append(cur.decode('latin1', errors='replace'))
        cur = bytearray()

print(f"Found {len(entries)} strings in range 0x078000..0x07b500")
for s in entries[:120]:
    print(s)
