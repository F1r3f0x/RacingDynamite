import os
import struct
import shutil

cue_bin_path = r"C:\Stuff\Proyects\RacingDynamite\Ignition\Ignition\game.gog"
assets_out = r"C:\Stuff\Proyects\RacingDynamite\assets"
music_src = r"C:\Stuff\Proyects\RacingDynamite\Ignition\Ignition\MUSIC"

def read_sector(f, lba):
    f.seek(lba * 2352 + 16)
    return f.read(2048)

def extract_all():
    os.makedirs(assets_out, exist_ok=True)
    with open(cue_bin_path, "rb") as f:
        pvd = read_sector(f, 16)
        root_record = pvd[156:156+34]
        
        def parse_and_extract(lba, length, current_path=""):
            sectors_to_read = (length + 2047) // 2048
            dir_data = b""
            for i in range(sectors_to_read):
                dir_data += read_sector(f, lba + i)
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
                        # Skip DIRECTX folder
                        if clean_name.upper() != "DIRECTX":
                            os.makedirs(os.path.join(assets_out, rel_path), exist_ok=True)
                            parse_and_extract(ext_lba, data_len, rel_path)
                    else:
                        # Skip windows installer/directx binaries
                        upper = clean_name.upper()
                        if not (upper.endswith('.EXE') or upper.endswith('.DLL') or upper.endswith('.INF')):
                            # Extract file
                            file_dest = os.path.join(assets_out, rel_path)
                            os.makedirs(os.path.dirname(file_dest), exist_ok=True)
                            
                            file_sectors = (data_len + 2047) // 2048
                            file_data = b""
                            for s in range(file_sectors):
                                file_data += read_sector(f, ext_lba + s)
                            file_data = file_data[:data_len]
                            
                            with open(file_dest, "wb") as out_f:
                                out_f.write(file_data)
                
                offset += record_len

        root_lba = struct.unpack("<I", root_record[2:6])[0]
        root_len = struct.unpack("<I", root_record[10:14])[0]
        print("Extracting game assets from CD-ROM image...")
        parse_and_extract(root_lba, root_len)
        print("CD-ROM asset extraction complete.")

    # Copy music if present
    if os.path.exists(music_src):
        music_dest = os.path.join(assets_out, "MUSIC")
        if not os.path.exists(music_dest):
            shutil.copytree(music_src, music_dest)
            print("Copied OGG soundtrack files to assets/MUSIC/")

if __name__ == "__main__":
    extract_all()
