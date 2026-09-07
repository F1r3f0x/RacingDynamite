#!/usr/bin/env python3
# Racing Dynamite - Modern open-source source port of Ignition (1997)
# Copyright (C) 2026 Patricio Labin Correa (@F1r3f0x)
# Licensed under GNU General Public License v3.0

import os
import sys
import struct
import shutil

def detect_image_type(f):
    """Detect whether file is raw 2352-byte MODE1 (with 16-byte sync) or standard 2048-byte ISO."""
    f.seek(0)
    head = f.read(16)
    sync = b"\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00"
    if head.startswith(sync):
        # Raw 2352 sector image (MODE1/2352)
        return 2352, 16
    
    # Check for PVD at 2048 * 16 = 0x8000
    f.seek(16 * 2048)
    pvd_head = f.read(6)
    if pvd_head == b"\x01CD001":
        return 2048, 0
        
    # Check for PVD at 2352 * 16 + 16
    f.seek(16 * 2352 + 16)
    pvd_head = f.read(6)
    if pvd_head == b"\x01CD001":
        return 2352, 16

    # Default to 2048
    return 2048, 0

def extract_from_iso_or_gog(image_path, assets_out):
    print(f"[Asset Extractor] Opening disc image: {image_path}")
    with open(image_path, "rb") as f:
        sector_size, header_offset = detect_image_type(f)
        print(f"[Asset Extractor] Format detected: Sector Size={sector_size}, Sync Offset={header_offset}")
        
        def read_sector(lba):
            f.seek(lba * sector_size + header_offset)
            return f.read(2048)

        pvd = read_sector(16)
        if pvd[1:6] != b"CD001":
            print("[Error] Primary Volume Descriptor not found. Invalid ISO/GOG image.")
            return False

        root_record = pvd[156:156+34]
        root_lba = struct.unpack("<I", root_record[2:6])[0]
        root_len = struct.unpack("<I", root_record[10:14])[0]

        def parse_and_extract(lba, length, current_path=""):
            sectors_to_read = (length + 2047) // 2048
            dir_data = b""
            for i in range(sectors_to_read):
                dir_data += read_sector(lba + i)
            dir_data = dir_data[:length]
            
            offset = 0
            while offset < len(dir_data):
                record_len = dir_data[offset]
                if record_len == 0:
                    next_sector = ((offset // 2048) + 1) * 2048
                    if next_sector >= len(dir_data):
                        break
                    offset = next_sector
                    continue
                
                record = dir_data[offset:offset+record_len]
                ext_lba = struct.unpack("<I", record[2:6])[0]
                data_len = struct.unpack("<I", record[10:14])[0]
                flags = record[25]
                name_len = record[32]
                name = record[33:33+name_len].decode('latin1', errors='ignore')
                
                is_dir = bool(flags & 2)
                if name not in ("\x00", "\x01"):
                    clean_name = name.split(';')[0]
                    rel_path = os.path.join(current_path, clean_name) if current_path else clean_name
                    
                    if is_dir:
                        if clean_name.upper() != "DIRECTX":
                            os.makedirs(os.path.join(assets_out, rel_path), exist_ok=True)
                            parse_and_extract(ext_lba, data_len, rel_path)
                    else:
                        upper = clean_name.upper()
                        # Extract non-installer files (allow ENGINE.INF in car sound dirs)
                        if not (upper.endswith('.EXE') or upper.endswith('.DLL') or (upper.endswith('.INF') and upper != 'ENGINE.INF')):
                            file_dest = os.path.join(assets_out, rel_path)
                            os.makedirs(os.path.dirname(file_dest), exist_ok=True)
                            
                            file_sectors = (data_len + 2047) // 2048
                            file_data = b""
                            for s in range(file_sectors):
                                file_data += read_sector(ext_lba + s)
                            file_data = file_data[:data_len]
                            
                            with open(file_dest, "wb") as out_f:
                                out_f.write(file_data)
                
                offset += record_len

        parse_and_extract(root_lba, root_len)
        print("[Asset Extractor] Disc asset extraction completed successfully.")
        return True

def copy_directory_assets(src_dir, assets_out):
    print(f"[Asset Extractor] Copying game assets from directory: {src_dir}")
    target_dirs = ["LEVELS", "CARS", "FONTS", "BALTAZAR", "GENERAL", "MUSIC", "SOUND"]
    target_files = ["SYS.COL", "INSTALL.PIC"]
    
    count = 0
    for root, dirs, files in os.walk(src_dir):
        rel = os.path.relpath(root, src_dir)
        top = rel.split(os.sep)[0].upper()
        if top in target_dirs:
            dest_dir = os.path.join(assets_out, rel)
            os.makedirs(dest_dir, exist_ok=True)
            for f in files:
                shutil.copy2(os.path.join(root, f), os.path.join(dest_dir, f))
                count += 1
        else:
            for f in files:
                if f.upper() in target_files:
                    dest_file = os.path.join(assets_out, f)
                    shutil.copy2(os.path.join(root, f), dest_file)
                    count += 1
    print(f"[Asset Extractor] Copied {count} game asset files.")

def main():
    assets_dir = os.path.abspath(sys.argv[2] if len(sys.argv) > 2 else "assets")
    os.makedirs(assets_dir, exist_ok=True)
    
    # Check source argument or find defaults
    source = sys.argv[1] if len(sys.argv) > 1 else None
    if not source:
        candidates = [
            "Ignition/Ignition/game.gog",
            "Ignition/game.gog",
            "game.gog",
            "ignition.iso",
            "Ignition",
        ]
        for c in candidates:
            if os.path.exists(c):
                source = c
                break

    if not source or not os.path.exists(source):
        print("Usage: python tools/extract_assets.py <path_to_game.gog_or_iso_or_folder> [output_assets_dir]")
        print("\nExamples:")
        print("  python tools/extract_assets.py 'C:\\GOG Games\\Ignition\\game.gog'")
        print("  python tools/extract_assets.py 'D:\\' assets/")
        print("  python tools/extract_assets.py 'ignition.iso'")
        sys.exit(1)

    if os.path.isdir(source):
        # Check if game.gog is inside the directory
        gog_candidate = os.path.join(source, "game.gog")
        sub_gog = os.path.join(source, "Ignition", "game.gog")
        if os.path.exists(gog_candidate):
            extract_from_iso_or_gog(gog_candidate, assets_dir)
        elif os.path.exists(sub_gog):
            extract_from_iso_or_gog(sub_gog, assets_dir)
        else:
            copy_directory_assets(source, assets_dir)
            
        # Copy MUSIC folder if present alongside
        music_candidates = [
            os.path.join(source, "MUSIC"),
            os.path.join(source, "Ignition", "MUSIC"),
            os.path.join(os.path.dirname(source), "MUSIC")
        ]
        for m in music_candidates:
            if os.path.exists(m):
                dest = os.path.join(assets_dir, "MUSIC")
                if not os.path.exists(dest):
                    shutil.copytree(m, dest)
                    print(f"[Asset Extractor] Copied soundtrack from {m} to assets/MUSIC/")
                break
    else:
        # It's a file (.gog, .iso, .bin)
        extract_from_iso_or_gog(source, assets_dir)
        
        # Check for MUSIC folder adjacent to image
        parent_dir = os.path.dirname(os.path.abspath(source))
        music_cand = os.path.join(parent_dir, "MUSIC")
        if os.path.exists(music_cand):
            dest = os.path.join(assets_dir, "MUSIC")
            if not os.path.exists(dest):
                shutil.copytree(music_cand, dest)
                print(f"[Asset Extractor] Copied soundtrack from {music_cand} to assets/MUSIC/")

if __name__ == "__main__":
    main()
