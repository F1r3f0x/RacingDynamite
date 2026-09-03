import os
import urllib.request
import zipfile
import io
import shutil

dest_dir = r"C:\Stuff\Proyects\RacingDynamite\third_party\sdl2"
os.makedirs(dest_dir, exist_ok=True)

url = "https://github.com/libsdl-org/SDL/releases/download/release-2.30.10/SDL2-devel-2.30.10-VC.zip"
print(f"Downloading SDL2 from {url}...")
req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})

with urllib.request.urlopen(req) as resp:
    data = resp.read()
    print(f"Downloaded {len(data)} bytes. Extracting...")
    with zipfile.ZipFile(io.BytesIO(data)) as z:
        for member in z.namelist():
            # Strip top-level folder 'SDL2-2.30.10/'
            parts = member.split('/', 1)
            if len(parts) > 1 and parts[1]:
                rel_path = parts[1]
                target_path = os.path.join(dest_dir, rel_path)
                if member.endswith('/'):
                    os.makedirs(target_path, exist_ok=True)
                else:
                    os.makedirs(os.path.dirname(target_path), exist_ok=True)
                    with open(target_path, "wb") as f:
                        f.write(z.read(member))

print(f"Extracted SDL2 to {dest_dir}")
