import struct

with open('Ignition/Ignition/MAINDOS_32BIT.EXE', 'rb') as f:
    data = f.read()

off = 0x0964f0
print(f"=== Pointers at file offset 0x{off:06x} ===")
for i in range(16):
    ptr = struct.unpack('<I', data[off+i*4 : off+(i+1)*4])[0]
    str_off = 0x77800 + (ptr - 0x000a0000) if 0xa0000 <= ptr < 0x100000 else -1
    s = ''
    if 0 < str_off < len(data):
        end = data.find(b'\x00', str_off)
        s = data[str_off:end].decode('latin1', errors='replace')
    print(f'[{i}] 0x{ptr:08x} -> "{s}"')

targets = [
    ("ign1.cdp", 0x000a64b5),
    ("ign2.cdp", 0x000a64cc),
    ("ign3_0.cdp", 0x000a64e3),
    ("ign3_1.cdp", 0x000a64fc),
    ("ign3_2.cdp", 0x000a6515),
    ("ign3_3.cdp", 0x000a652e),
    ("menucar.plc", 0x000a6417),
    ("menucar.msh", 0x000a644b),
    ("menucar.tex", 0x000a647f),
]

for delta in range(-64, 65, 4):
    target = 0x000becf0 + delta
    target_bytes = struct.pack('<I', target)
    idx = 0
    while True:
        idx = data.find(target_bytes, idx)
        if idx == -1: break
        if idx < 0x76200:
            va = 0x10000 + (idx - 0x400)
            print(f'Reference to 0x{target:08x} (delta {delta}) at VA 0x{va:08x}')
        idx += 4


