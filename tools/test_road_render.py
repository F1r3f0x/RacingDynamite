import struct, math

# Load assets
with open("assets/LEVELS/AUSTRIA/AUSTRIA.COL", "rb") as f:
    col = f.read()[8:8+768]

with open("assets/LEVELS/AUSTRIA/AUSTRIA.TEX", "rb") as f:
    tex = f.read()

with open("assets/LEVELS/AUSTRIA/AUSTRIA.TAB", "rb") as f:
    tab = f.read()

with open("assets/LEVELS/AUSTRIA/AUSTRIA.PLC", "rb") as f:
    plc_count = struct.unpack("<I", f.read(4))[0]
    plc_objs = [struct.unpack("<5i", f.read(20)) for _ in range(plc_count)]

WIDTH, HEIGHT = 640, 480
fb = bytearray(WIDTH * HEIGHT)
zb = [0.0] * (WIDTH * HEIGHT)

# Camera definition (same as game)
cam_dist = 7500.0
pitch = 35.0 * math.pi / 180.0
yaw = 45.0 * math.pi / 180.0
target = (0.0, -600.0, 0.0)

cam_x = target[0] + cam_dist * math.cos(pitch) * math.sin(yaw)
cam_y = target[1] - cam_dist * math.sin(pitch)
cam_z = target[2] + cam_dist * math.cos(pitch) * math.cos(yaw)

# Camera axes
forward = (target[0] - cam_x, target[1] - cam_y, target[2] - cam_z)
flen = math.sqrt(forward[0]**2 + forward[1]**2 + forward[2]**2)
forward = (forward[0]/flen, forward[1]/flen, forward[2]/flen)

right = (forward[2], 0.0, -forward[0])
rlen = math.sqrt(right[0]**2 + right[1]**2)
right = (right[0]/rlen, 0.0, right[2]/rlen)

up = (right[1]*forward[2] - right[2]*forward[1],
      right[2]*forward[0] - right[0]*forward[2],
      right[0]*forward[1] - right[1]*forward[0])

fov_scale = 1.0 / math.tan(55.0 * 0.5 * math.pi / 180.0)
aspect = WIDTH / HEIGHT

def project(wx, wy, wz):
    dx, dy, dz = wx - cam_x, wy - cam_y, wz - cam_z
    cx = dx * right[0] + dy * right[1] + dz * right[2]
    cy = dx * up[0]    + dy * up[1]    + dz * up[2]
    cz = dx * forward[0] + dy * forward[1] + dz * forward[2]
    if cz <= 1.0:
        return None
    inv_z = 1.0 / cz
    sx = (cx * inv_z * fov_scale / aspect + 1.0) * 0.5 * WIDTH
    sy = (1.0 - cy * inv_z * fov_scale) * 0.5 * HEIGHT
    return (sx, sy, inv_z)

def draw_tri(v0, v1, v2, uv0, uv1, uv2, light):
    p0 = project(*v0)
    p1 = project(*v1)
    p2 = project(*v2)
    if not p0 or not p1 or not p2:
        return
    # Backface culling
    cross = (p1[0] - p0[0]) * (p2[1] - p0[1]) - (p1[1] - p0[1]) * (p2[0] - p0[0])
    if cross <= 0:
        return
    inv_cross = 1.0 / cross
    min_x = max(0, int(math.floor(min(p0[0], p1[0], p2[0]))))
    max_x = min(WIDTH - 1, int(math.ceil(max(p0[0], p1[0], p2[0]))))
    min_y = max(0, int(math.floor(min(p0[1], p1[1], p2[1]))))
    max_y = min(HEIGHT - 1, int(math.ceil(max(p0[1], p1[1], p2[1]))))

    l_level = int((1.0 - max(0.0, min(1.0, light))) * 31.0)

    for y in range(min_y, max_y + 1):
        py = y + 0.5
        row_idx = y * WIDTH
        for x in range(min_x, max_x + 1):
            px = x + 0.5
            w0 = ((p1[0] - px) * (p2[1] - py) - (p1[1] - py) * (p2[0] - px)) * inv_cross
            w1 = ((p2[0] - px) * (p0[1] - py) - (p2[1] - py) * (p0[0] - px)) * inv_cross
            w2 = 1.0 - w0 - w1
            if w0 >= 0 and w1 >= 0 and w2 >= 0:
                inv_z = w0 * p0[2] + w1 * p1[2] + w2 * p2[2]
                pix_idx = row_idx + x
                if inv_z > zb[pix_idx]:
                    zb[pix_idx] = inv_z
                    z_real = 1.0 / inv_z
                    u = (w0 * uv0[0] * p0[2] + w1 * uv1[0] * p1[2] + w2 * uv2[0] * p2[2]) * z_real
                    v = (w0 * uv0[1] * p0[2] + w1 * uv1[1] * p1[2] + w2 * uv2[1] * p2[2]) * z_real
                    tu = int(u * 1024.0) & 1023
                    tv = int(v * 1024.0) & 1023
                    texel = tex[(tv & 127) * 1024 + (tu & 127)] if (uv0[1] < 0.2) else tex[tv * 1024 + tu]
                    col_idx = tab[(texel << 8) | l_level]
                    fb[pix_idx] = col_idx

# 1. Render ground terrain
radius = 20
step = 2
cell_size = 512
for gz in range(-radius, radius, step):
    for gx in range(-radius, radius, step):
        x0, z0 = gx * cell_size, gz * cell_size
        x1, z1 = (gx + step) * cell_size, (gz + step) * cell_size
        y0 = -100.0 * math.sin(x0*0.001) * math.cos(z0*0.001)
        u0 = (gx * 0.05)
        v0 = (gz * 0.05)
        u1 = ((gx + step) * 0.05)
        v1 = ((gz + step) * 0.05)
        draw_tri((x0, y0, z0), (x0, y0, z1), (x1, y0, z0), (u0, v0), (u0, v1), (u1, v0), 0.7)
        draw_tri((x1, y0, z0), (x0, y0, z1), (x1, y0, z1), (u1, v0), (u0, v1), (u1, v1), 0.7)

# 2. Render road ribbon from 305 road nodes
road_objs = plc_objs[:305]
half_width = 280.0
for i in range(len(road_objs)):
    p_curr = road_objs[i]
    p_next = road_objs[(i + 1) % len(road_objs)]
    dx = p_next[2] - p_curr[2]
    dz = p_next[4] - p_curr[4]
    dlen = math.sqrt(dx*dx + dz*dz)
    if dlen < 1e-4:
        continue
    # Right normal vector
    nx = -dz / dlen * half_width
    nz =  dx / dlen * half_width
    
    # Vertices
    v_curr_l = (p_curr[2] - nx, p_curr[3] - 20.0, p_curr[4] - nz)
    v_curr_r = (p_curr[2] + nx, p_curr[3] - 20.0, p_curr[4] + nz)
    v_next_l = (p_next[2] - nx, p_next[3] - 20.0, p_next[4] - nz)
    v_next_r = (p_next[2] + nx, p_next[3] - 20.0, p_next[4] + nz)

    # Road texture coordinates (asphalt/snow road in Austria)
    u_l, u_r = 0.0, 0.25
    v_c = 0.60 + (i % 2) * 0.05
    v_n = 0.60 + ((i + 1) % 2) * 0.05

    draw_tri(v_curr_l, v_curr_r, v_next_l, (u_l, v_c), (u_r, v_c), (u_l, v_n), 0.9)
    draw_tri(v_curr_r, v_next_r, v_next_l, (u_r, v_c), (u_r, v_n), (u_l, v_n), 0.9)

# 3. Render scenery props
for i in range(305, plc_count):
    o = plc_objs[i]
    px, py, pz = float(o[2]), float(o[3]), float(o[4])
    bs = 180.0
    # Render building / prop box
    draw_tri((px-bs, py-bs*2, pz-bs), (px+bs, py-bs*2, pz-bs), (px-bs, py, pz-bs), (0.2, 0.25), (0.4, 0.25), (0.2, 0.45), 0.8)
    draw_tri((px+bs, py-bs*2, pz-bs), (px+bs, py, pz-bs), (px-bs, py, pz-bs), (0.4, 0.25), (0.4, 0.45), (0.2, 0.45), 0.8)

# Save 24-bit BMP
bmp_hdr = struct.pack("<2sIHHI", b"BM", 54 + WIDTH * HEIGHT * 3, 0, 0, 54)
dib_hdr = struct.pack("<IIIHHIIIIII", 40, WIDTH, HEIGHT, 1, 24, 0, WIDTH * HEIGHT * 3, 2835, 2835, 0, 0)
row_bytes = ((WIDTH * 3 + 3) // 4) * 4

with open("docs/extracted_bitmaps/TEST_NEW_ROAD.bmp", "wb") as f:
    f.write(bmp_hdr)
    f.write(dib_hdr)
    row = bytearray(row_bytes)
    for y in range(HEIGHT - 1, -1, -1):
        for x in range(WIDTH):
            idx = fb[y * WIDTH + x]
            row[x * 3 + 0] = col[idx * 3 + 2]
            row[x * 3 + 1] = col[idx * 3 + 1]
            row[x * 3 + 2] = col[idx * 3 + 0]
        f.write(row)

print("Saved TEST_NEW_ROAD.bmp successfully.")
