import math
import struct

with open("assets/LEVELS/AUSTRIA/AUSTRIA.COL", "rb") as f:
    col_data = f.read()[8:8+768]

with open("assets/LEVELS/AUSTRIA/AUSTRIA.TAB", "rb") as f:
    tab_data = f.read()

with open("assets/LEVELS/AUSTRIA/AUSTRIA.TEX", "rb") as f:
    tex_data = f.read()

with open("assets/LEVELS/AUSTRIA/AUSTRIA.SRF", "rb") as f:
    srf_data = f.read()

# Load SRF
grid_w, grid_d, cell_w, cell_d, num_tri, num_t1 = struct.unpack("<6i", srf_data[:24])
tri_offset = 24 + grid_w * grid_d * 8

def get_elevation(x, z):
    cx = x // cell_w
    cz = z // cell_d
    if cx < 0: cx = 0
    if cz < 0: cz = 0
    if cx >= grid_w: cx = grid_w - 1
    if cz >= grid_d: cz = grid_d - 1
    cell_idx = cz * grid_w + cx
    cell_off = 24 + cell_idx * 8
    t1_off = struct.unpack("<i", srf_data[cell_off : cell_off + 4])[0]
    tri_idx = t1_off // 24
    if 0 <= tri_idx < num_tri:
        to = tri_offset + tri_idx * 24
        y0, y1, y2 = struct.unpack("<3i", srf_data[to : to + 12])
        return (y0 + y1 + y2) // 3
    return 0

W, H = 640, 480
fb = bytearray([0] * (W * H))
zbuffer = [0.0] * (W * H) # Storing 1/Z

# Camera
cam_yaw = 45.0 * math.pi / 180.0
cam_pitch = 35.0 * math.pi / 180.0
cam_dist = 6000.0

target_x, target_y, target_z = 0.0, -500.0, 0.0
cam_x = target_x + cam_dist * math.cos(cam_pitch) * math.sin(cam_yaw)
cam_y = target_y + cam_dist * math.sin(cam_pitch)
cam_z = target_z + cam_dist * math.cos(cam_pitch) * math.cos(cam_yaw)

# Basis vectors
fx = target_x - cam_x
fy = target_y - cam_y
fz = target_z - cam_z
flen = math.sqrt(fx*fx + fy*fy + fz*fz)
fx /= flen; fy /= flen; fz /= flen

# World up = (0, 1, 0)
rx = fy * 0.0 - fz * 1.0; ry = 0.0; rz = fx * 1.0 - fx * 0.0 # R = F x Up
# Wait: R = F x (0, 1, 0) = (F_y*0 - F_z*1, F_z*0 - F_x*0, F_x*1 - F_y*0) = (-F_z, 0, F_x)
rx = -fz
ry = 0.0
rz = fx
rlen = math.sqrt(rx*rx + rz*rz)
rx /= rlen; rz /= rlen

# Up = R x F
ux = ry * fz - rz * fy
uy = rz * fx - rx * fz
uz = rx * fy - ry * fx

fov_scale = (H * 0.5) / math.tan((60.0 * 0.5) * math.pi / 180.0)

def project(wx, wy, wz, u, v, light):
    dx = wx - cam_x
    dy = wy - cam_y
    dz = wz - cam_z
    zc = dx * fx + dy * fy + dz * fz
    if zc < 50.0 or zc > 50000.0:
        return None
    xc = dx * rx + dy * ry + dz * rz
    yc = dx * ux + dy * uy + dz * uz
    inv_z = 1.0 / zc
    xs = (xc * fov_scale * inv_z) + (W * 0.5)
    ys = (-yc * fov_scale * inv_z) + (H * 0.5)
    return {
        "x": xs, "y": ys, "z": zc, "inv_z": inv_z,
        "u_over_z": u * inv_z, "v_over_z": v * inv_z,
        "light": light
    }

def draw_tri(v0, v1, v2):
    area = (v1["x"] - v0["x"]) * (v2["y"] - v0["y"]) - (v1["y"] - v0["y"]) * (v2["x"] - v0["x"])
    if area <= 0.1: return
    inv_area = 1.0 / area

    min_x = max(0, int(math.floor(min(v0["x"], v1["x"], v2["x"]))))
    max_x = min(W - 1, int(math.ceil(max(v0["x"], v1["x"], v2["x"]))))
    min_y = max(0, int(math.floor(min(v0["y"], v1["y"], v2["y"]))))
    max_y = min(H - 1, int(math.ceil(max(v0["y"], v1["y"], v2["y"]))))
    if min_x > max_x or min_y > max_y: return

    for y in range(min_y, max_y + 1):
        py = y + 0.5
        row_off = y * W
        for x in range(min_x, max_x + 1):
            px = x + 0.5
            w0 = ((v1["x"] - px) * (v2["y"] - py) - (v1["y"] - py) * (v2["x"] - px)) * inv_area
            w1 = ((v2["x"] - px) * (v0["y"] - py) - (v2["y"] - py) * (v0["x"] - px)) * inv_area
            w2 = 1.0 - w0 - w1
            if w0 >= 0.0 and w1 >= 0.0 and w2 >= 0.0:
                inv_z = w0 * v0["inv_z"] + w1 * v1["inv_z"] + w2 * v2["inv_z"]
                pix_idx = row_off + x
                if inv_z > zbuffer[pix_idx]:
                    zbuffer[pix_idx] = inv_z
                    z = 1.0 / inv_z
                    u = (w0 * v0["u_over_z"] + w1 * v1["u_over_z"] + w2 * v2["u_over_z"]) * z
                    v = (w0 * v0["v_over_z"] + w1 * v1["v_over_z"] + w2 * v2["v_over_z"]) * z
                    light = w0 * v0["light"] + w1 * v1["light"] + w2 * v2["light"]
                    light = max(0.0, min(1.0, light))

                    tu = int(math.floor(u * 1024.0)) & 1023
                    tv = int(math.floor(v * 1024.0)) & 1023
                    texel = tex_data[tv * 1024 + tu]

                    # Proper TAB lookup:
                    # light_level 0 = bright, 31 = dark
                    light_level = int((1.0 - light) * 31.0)
                    if light_level > 31: light_level = 31
                    col = tab_data[(texel << 8) | light_level]
                    fb[pix_idx] = col

# Draw surface
sun = (0.4 / 1.0, 0.8 / 1.0, 0.3 / 1.0)
step = 2
for z in range(-30, 30, step):
    for x in range(-30, 30, step):
        wx0 = x * 512; wz0 = z * 512
        wx1 = (x + step) * 512; wz1 = (z + step) * 512
        y00 = get_elevation(wx0, wz0)
        y10 = get_elevation(wx1, wz0)
        y01 = get_elevation(wx0, wz1)
        y11 = get_elevation(wx1, wz1)

        # Smooth continuous UV mapping across terrain
        u0 = (x + 30) / 60.0; v0 = (z + 30) / 60.0
        u1 = (x + step + 30) / 60.0; v1 = (z + step + 30) / 60.0

        p0 = project(wx0, y00, wz0, u0, v0, 0.8)
        p1 = project(wx1, y10, wz0, u1, v0, 0.8)
        p2 = project(wx0, y01, wz1, u0, v1, 0.8)
        p3 = project(wx1, y11, wz1, u1, v1, 0.8)

        if p0 and p1 and p2: draw_tri(p0, p1, p2)
        if p1 and p3 and p2: draw_tri(p1, p3, p2)

# Save to BMP
row_stride = (W + 3) & ~3
bmp_pixels = bytearray(row_stride * H)
for r in range(H):
    dst_r = H - 1 - r
    bmp_pixels[dst_r * row_stride : dst_r * row_stride + W] = fb[r * W : (r + 1) * W]

color_table = bytearray()
for i in range(256):
    color_table.extend([col_data[i*3+2], col_data[i*3+1], col_data[i*3+0], 0])

total_size = 54 + 1024 + len(bmp_pixels)
bmp_hdr = struct.pack("<2sIHHI", b"BM", total_size, 0, 0, 54 + 1024)
dib_hdr = struct.pack("<IIIHHIIIIII", 40, W, H, 1, 8, 0, len(bmp_pixels), 2835, 2835, 256, 256)

with open(r"C:\Stuff\Proyects\RacingDynamite\docs\extracted_bitmaps\TEST_3D_SURFACE.bmp", "wb") as f_out:
    f_out.write(bmp_hdr)
    f_out.write(dib_hdr)
    f_out.write(color_table)
    f_out.write(bmp_pixels)

print("Saved TEST_3D_SURFACE.bmp!")
