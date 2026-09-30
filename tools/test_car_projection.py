import struct
import math
from PIL import Image

with open('Ignition/Ignition/BALTAZAR/DATA/MENU.COL', 'rb') as f:
    col = list(f.read()[8:8+768])

with open('Ignition/Ignition/BALTAZAR/DATA/CAR_SEL.PIC', 'rb') as f:
    car_sel_raw = f.read()[846:]

with open('Ignition/Ignition/CARS/CARS.MSH', 'rb') as f:
    cars_msh = f.read()

with open('Ignition/Ignition/CARS/CARS.TEX', 'rb') as f:
    cars_tex = f.read()

# Let's verify Coop (car 0, sub_off 5464*4)
off = 5464 * 4
vc, pc = struct.unpack('<ii', cars_msh[off:off+8])
print(f"Coop: vc={vc}, pc={pc}")

# Let's check vertex coordinates range
v_data = []
for i in range(vc):
    x, y, z = struct.unpack('<3i', cars_msh[off+8+i*12 : off+8+(i+1)*12])
    v_data.append((x, y, z))

min_x = min(v[0] for v in v_data); max_x = max(v[0] for v in v_data)
min_y = min(v[1] for v in v_data); max_y = max(v[1] for v in v_data)
min_z = min(v[2] for v in v_data); max_z = max(v[2] for v in v_data)
print(f"X: {min_x}..{max_x}, Y: {min_y}..{max_y}, Z: {min_z}..{max_z}")
