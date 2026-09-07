import struct

# Load MENU.COL palette
with open("assets/BALTAZAR/DATA/MENU.COL", "rb") as f:
    col_data = f.read()
palette = col_data[8:8+768]

# Load INSTALL.PIC
with open("assets/INSTALL.PIC", "rb") as f:
    pic_data = f.read()
pic_pixels = pic_data[846 : 846 + 640*480]

# Load IGNITION.FNT
with open("assets/FONTS/IGNITION.FNT", "rb") as f:
    fnt_data = f.read()

height = struct.unpack("<h", fnt_data[2:4])[0]
spacing = struct.unpack("<h", fnt_data[4:6])[0]
if spacing < 0: spacing = 1
ascii_map = fnt_data[0xC6 : 0xC6 + 256]
widths = fnt_data[6 : 6 + 256]
offsets = struct.unpack("<256i", fnt_data[0x1C6 : 0x1C6 + 256*4])
pixels = fnt_data[0x546:]

def draw_text(fb, text, x, y, color_offset):
    cur_x = x
    for ch in text:
        c = ord(ch)
        if c == 32:
            cur_x += 6
            continue
        g_idx = ascii_map[c]
        if g_idx != 255:
            w = widths[g_idx]
            off = offsets[g_idx]
            if off >= 0:
                for r in range(height):
                    dest_y = y + r
                    for col in range(w):
                        dest_x = cur_x + col
                        if 0 <= dest_y < 480 and 0 <= dest_x < 640:
                            p = pixels[off + r * w + col]
                            if p != 0:
                                final_col = color_offset if p == 2 else 0
                                fb[dest_y * 640 + dest_x] = final_col
                cur_x += w + spacing

def get_text_width(text):
    tw = 0
    for ch in text:
        if ord(ch) == 32:
            tw += 6
            continue
        g_idx = ascii_map[ord(ch)]
        if g_idx != 255 and offsets[g_idx] >= 0:
            tw += widths[g_idx] + spacing
        else:
            tw += 4
    return tw

def save_bmp(fb, filename):
    img_w, img_h = 640, 480
    row_stride = (img_w + 3) & ~3
    bmp_pixels = bytearray(row_stride * img_h)
    for r in range(img_h):
        dst_r = img_h - 1 - r
        bmp_pixels[dst_r * row_stride : dst_r * row_stride + img_w] = fb[r * img_w : (r+1) * img_w]

    color_table = bytearray()
    for i in range(256):
        r = palette[i*3+0]
        g = palette[i*3+1]
        b = palette[i*3+2]
        color_table.extend([b, g, r, 0])

    total_size = 54 + 1024 + len(bmp_pixels)
    bmp_hdr = struct.pack("<2sIHHI", b"BM", total_size, 0, 0, 54 + 1024)
    dib_hdr = struct.pack("<IIIHHIIIIII", 40, img_w, img_h, 1, 8, 0, len(bmp_pixels), 2835, 2835, 256, 256)

    with open(filename, "wb") as f_out:
        f_out.write(bmp_hdr)
        f_out.write(dib_hdr)
        f_out.write(color_table)
        f_out.write(bmp_pixels)
    print(f"Saved: {filename}")

# Render Intro screen
fb_intro = bytearray(pic_pixels)
prompt = "PRESS ENTER OR SPACE TO START"
pw = get_text_width(prompt)
draw_text(fb_intro, prompt, (640 - pw) // 2, 420, 215)
save_bmp(fb_intro, r"C:\Stuff\Proyects\RacingDynamite\docs\extracted_bitmaps\INTRO_VERIFIED.bmp")

# Render Main Menu screen
fb_menu = bytearray(pic_pixels)
items = ["SINGLE RACE", "CHAMPIONSHIP", "TIME ATTACK", "OPTIONS", "QUIT"]
start_y = 260
for i, item in enumerate(items):
    w = get_text_width(item)
    x = (640 - w) // 2
    y = start_y + i * 28
    color = 215 if i == 0 else 251
    if i == 0:
        draw_text(fb_menu, ">", x - 18, y, 215)
        draw_text(fb_menu, "<", x + w + 8, y, 215)
    draw_text(fb_menu, item, x, y, color)
save_bmp(fb_menu, r"C:\Stuff\Proyects\RacingDynamite\docs\extracted_bitmaps\MENU_VERIFIED.bmp")
