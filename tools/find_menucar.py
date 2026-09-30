import struct
import capstone

with open('Ignition/Ignition/MAINDOS_32BIT.EXE', 'rb') as f:
    data = f.read()

md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)

# Where is 'baltazar\data\menucar.msh' used?
str_off = data.find(b'menucar.msh')
print(f"menucar.msh at offset 0x{str_off:06x}")
str_va = 0x000a0000 + (str_off - 0x77800)
print(f"menucar.msh VA: 0x{str_va:08x}")

# Find any references to this string or sub-string
for delta in range(0, 16):
    b = struct.pack('<I', str_va - delta)
    pos = 0
    while True:
        pos = data.find(b, pos)
        if pos == -1: break
        va = 0x10000 + (pos - 0x400)
        print(f"Reference to 0x{str_va-delta:08x} at file 0x{pos:06x} (VA 0x{va:08x})")
        pos += 4

# Let's also check all strings starting with 'baltazar\data\menucar'
for name in [b'menucar.plc', b'menucar.msh', b'menucar.tex', b'test2.pfm']:
    p = data.find(name)
    va = 0x000a0000 + (p - 0x77800)
    print(f"{name.decode()} at 0x{p:06x} -> VA 0x{va:08x}")
    b = struct.pack('<I', va)
    pos = 0
    while True:
        pos = data.find(b, pos)
        if pos == -1: break
        fva = 0x10000 + (pos - 0x400)
        print(f"  referenced at 0x{pos:06x} -> VA 0x{fva:08x}")
        pos += 4
