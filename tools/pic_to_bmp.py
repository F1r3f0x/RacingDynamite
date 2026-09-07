import os
import struct

def pic_to_bmp(pic_path, bmp_path):
    with open(pic_path, "rb") as f:
        data = f.read()
    
    if len(data) < 846:
        print(f"File too small: {pic_path}")
        return False
    
    file_size = struct.unpack("<I", data[0:4])[0]
    magic = struct.unpack("<H", data[4:6])[0]
    width = struct.unpack("<H", data[6:8])[0]
    height = struct.unpack("<H", data[8:10])[0]
    
    # Palette is present at offset 72..840 (768 bytes = 256 * 3)
    palette_data = data[72:840]
    pixel_data = data[846:]
    
    expected_pixels = width * height
    if len(pixel_data) < expected_pixels:
        print(f"Warning: {pic_path} pixel data ({len(pixel_data)}) < width*height ({expected_pixels})")
        return False
    
    # Create standard 8-bit paletted BMP
    # BMP File Header (14 bytes)
    # DIB Header (BITMAPINFOHEADER = 40 bytes)
    # Color Table (256 * 4 bytes = 1024 bytes, BGRA)
    # Pixel rows (bottom-up, padded to 4-byte boundary)
    
    row_stride = (width + 3) & ~3
    image_size = row_stride * height
    offset_to_bits = 14 + 40 + 1024
    total_bmp_size = offset_to_bits + image_size
    
    # 14-byte BMP header
    bmp_header = struct.pack("<2sIHHI", b"BM", total_bmp_size, 0, 0, offset_to_bits)
    # 40-byte DIB header
    dib_header = struct.pack("<IIIHHIIIIII", 40, width, height, 1, 8, 0, image_size, 2835, 2835, 256, 256)
    
    # Color table: Ignition palette is RGB, BMP needs BGR0
    color_table = bytearray()
    for i in range(256):
        r = palette_data[i*3]
        g = palette_data[i*3 + 1]
        b = palette_data[i*3 + 2]
        color_table.extend([b, g, r, 0])
        
    # Pixels: BMP is bottom-up
    bmp_pixels = bytearray(image_size)
    padding = b"\x00" * (row_stride - width)
    for y in range(height):
        # source row (top-down)
        src_row = y
        # dest row (bottom-up)
        dst_row = height - 1 - y
        row_slice = pixel_data[src_row * width : (src_row + 1) * width]
        bmp_pixels[dst_row * row_stride : dst_row * row_stride + width] = row_slice
        
    with open(bmp_path, "wb") as f_out:
        f_out.write(bmp_header)
        f_out.write(dib_header)
        f_out.write(color_table)
        f_out.write(bmp_pixels)
        
    print(f"Successfully converted {os.path.basename(pic_path)} ({width}x{height}) -> {bmp_path}")
    return True

if __name__ == "__main__":
    out_dir = r"C:\Stuff\Proyects\RacingDynamite\docs\extracted_bitmaps"
    os.makedirs(out_dir, exist_ok=True)
    test_files = [
        r"C:\Stuff\Proyects\RacingDynamite\assets\INSTALL.PIC",
        r"C:\Stuff\Proyects\RacingDynamite\assets\LEVELS\AUSTRIA\AUSTRIA.PIC",
        r"C:\Stuff\Proyects\RacingDynamite\assets\LEVELS\USA\USA.PIC",
        r"C:\Stuff\Proyects\RacingDynamite\assets\POKAL.PIC",
    ]
    for p in test_files:
        if os.path.exists(p):
            bmp_name = os.path.splitext(os.path.basename(p))[0] + ".bmp"
            pic_to_bmp(p, os.path.join(out_dir, bmp_name))
