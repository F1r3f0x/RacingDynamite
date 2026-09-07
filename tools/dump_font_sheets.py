import os
import struct

def dump_font_sheet(lft_path, out_bmp):
    with open(lft_path, "rb") as f:
        data = f.read()

    height = struct.unpack("<h", data[10:12])[0]
    offsets = struct.unpack("<224i", data[0x0c : 0x0c + 224*4])
    widths = struct.unpack("<224h", data[0x68c : 0x68c + 224*2])
    pixels = data[0x84c:]

    # Grid: 16 columns x 14 rows = 224 cells
    cell_w = 20
    cell_h = height + 4
    img_w = 16 * cell_w
    img_h = 14 * cell_h

    # Create 24-bit RGB bitmap
    fb = bytearray([40, 40, 40] * (img_w * img_h))

    for idx in range(224):
        off = offsets[idx]
        w = widths[idx]
        if off < 0 or w <= 0 or off >= len(pixels):
            continue

        grid_x = (idx % 16) * cell_w + 2
        grid_y = (idx // 16) * cell_h + 2

        for r in range(height):
            for c in range(w):
                p_idx = off + r * w + c
                if p_idx < len(pixels):
                    p = pixels[p_idx]
                    if p != 0:
                        # Draw white/yellow pixel
                        dest_y = grid_y + r
                        dest_x = grid_x + c
                        if dest_y < img_h and dest_x < img_w:
                            pix_offset = (dest_y * img_w + dest_x) * 3
                            fb[pix_offset + 0] = 255
                            fb[pix_offset + 1] = 255
                            fb[pix_offset + 2] = 255

    # Write BMP
    row_stride = (img_w * 3 + 3) & ~3
    image_size = row_stride * img_h
    offset_to_bits = 54
    total_bmp_size = offset_to_bits + image_size

    bmp_header = struct.pack("<2sIHHI", b"BM", total_bmp_size, 0, 0, offset_to_bits)
    dib_header = struct.pack("<IIIHHIIIIII", 40, img_w, img_h, 1, 24, 0, image_size, 2835, 2835, 0, 0)

    bmp_pixels = bytearray(image_size)
    for y in range(img_h):
        dst_y = img_h - 1 - y
        for x in range(img_w):
            b = fb[(y * img_w + x) * 3 + 0]
            g = fb[(y * img_w + x) * 3 + 1]
            r = fb[(y * img_w + x) * 3 + 2]
            bmp_pixels[dst_y * row_stride + x * 3 + 0] = b
            bmp_pixels[dst_y * row_stride + x * 3 + 1] = g
            bmp_pixels[dst_y * row_stride + x * 3 + 2] = r

    with open(out_bmp, "wb") as f_out:
        f_out.write(bmp_header)
        f_out.write(dib_header)
        f_out.write(bmp_pixels)
    print(f"Wrote font sheet to: {out_bmp}")

dump_font_sheet(r"C:\Stuff\Proyects\RacingDynamite\assets\FONTS\SMALL.LFT",
                r"C:\Stuff\Proyects\RacingDynamite\docs\extracted_bitmaps\SMALL_SHEET.bmp")
dump_font_sheet(r"C:\Stuff\Proyects\RacingDynamite\assets\BALTAZAR\DATA\RED_DARK.LFT",
                r"C:\Stuff\Proyects\RacingDynamite\docs\extracted_bitmaps\RED_DARK_SHEET.bmp")
