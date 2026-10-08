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

# Base framebuffer 320x200
fb = [0] * (320 * 200)

# Draw pedestal at (32, 105)
for r in range(74):
    for c in range(255):
        px = 32 + c
        py = 105 + r
        color = car_sel_raw[r * 255 + c]
        if color != 0:
            fb[py * 320 + px] = color

# Now let's project the car vertices for Coop
# camera: cam_x=0, cam_y=-60, cam_z=140
# target: 0, 0, 0
cam_x, cam_y, cam_z = 0.0, -50.0, 120.0
tx, ty, tz = 0.0, 0.0, 0.0

fx = tx - cam_x; fy = ty - cam_y; fz = tz - cam_z
flen = math.sqrt(fx*fx + fy*fy + fz*fz)
fx /= flen; fy /= flen; fz /= flen

rx = -fz; ry = 0.0; rz = fx
rlen = math.sqrt(rx*rx + rz*rz)
rx /= rlen; rz /= rlen

ux = ry * fz - rz * fy
uy = rz * fx - rx * fz
uz = rx * fy - ry * fx

fov = 220.0
yaw = 0.6 # nice 3/4 angle
cos_y = math.cos(yaw)
sin_y = math.sin(yaw)

off = 5464 * 4
vc, pc = struct.unpack('<ii', cars_msh[off:off+8])
v_raw = cars_msh[off+8 : off+8+vc*12]
p_raw = cars_msh[off+8+vc*12 : off+8+vc*12+pc*44]

# Wireframe / simple triangle plot to verify positioning
for p in range(pc):
    poly = p_raw[p*44 : (p+1)*44]
    i0, i1, i2 = struct.unpack('<3I', poly[4:16])
    pts = []
    for vi in (i0, i1, i2):
        sx, sy, sz = struct.unpack('<3i', v_raw[vi*12:(vi+1)*12])
        wx = sx * cos_y + sz * sin_y
        wy = -sy # invert Y
        wz = -sx * sin_y + sz * cos_y
        dx = wx - cam_x; dy = wy - cam_y; dz = wz - cam_z
        zc = dx * fx + dy * fy + dz * fz
        inv_z = 1.0 / zc
        px = int(((dx * rx + dy * ry + dz * rz) * fov * inv_z) + 160.0)
        py = int((-(dx * ux + dy * uy + dz * uz) * fov * inv_z) + 138.0)
        pts.append((px, py))
    # Draw simple lines between pts
    for a, b in [(pts[0], pts[1]), (pts[1], pts[2]), (pts[2], pts[0])]:
        x0, y0 = a; x1, y1 = b
        steps = max(abs(x1-x0), abs(y1-y0), 1)
        for s in range(steps+1):
            lx = int(x0 + (x1-x0)*s/steps)
            ly = int(y0 + (y1-y0)*s/steps)
            if 0 <= lx < 320 and 0 <= ly < 200:
                fb[ly*320 + lx] = 213 # yellow/gold

img = Image.frombytes('P', (320, 200), bytes(bytearray(fb)))
img.putpalette(col)
img.save('docs/extracted_bitmaps/test_car_pedestal.png')
print("Saved test_car_pedestal.png")
