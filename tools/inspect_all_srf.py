import os
import struct

assets_dir = r"C:\Stuff\Proyects\RacingDynamite\assets\LEVELS"
for track in sorted(os.listdir(assets_dir)):
    track_dir = os.path.join(assets_dir, track)
    if os.path.isdir(track_dir):
        for f in os.listdir(track_dir):
            if f.upper().endswith('.SRF'):
                srf_path = os.path.join(track_dir, f)
                sz = os.path.getsize(srf_path)
                with open(srf_path, 'rb') as fp:
                    hdr_raw = fp.read(36)
                h = struct.unpack('<9i', hdr_raw)
                print(f"{track:<10}: size={sz:<7} cells=({h[0]}x{h[1]}) cell_sz=({h[3]}x{h[2]}) strides=({h[4]}x{h[5]}) tris={h[6]:<6} count1={h[7]:<6} count2={h[8]:<6}")
