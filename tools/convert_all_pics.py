import os
from pic_to_bmp import pic_to_bmp

out_dir = r"C:\Stuff\Proyects\RacingDynamite\docs\extracted_bitmaps"
tracks = ["AUSTRIA", "BRAZIL", "CANADA", "CARIB", "ICELAND", "JAPAN", "USA"]

for t in tracks:
    pic_path = os.path.join(r"C:\Stuff\Proyects\RacingDynamite\assets\LEVELS", t, f"{t}.PIC")
    if os.path.exists(pic_path):
        pic_to_bmp(pic_path, os.path.join(out_dir, f"{t}.bmp"))

# Also convert BALTAZAR and other PICs
for root, dirs, files in os.walk(r"C:\Stuff\Proyects\RacingDynamite\assets"):
    for f in files:
        if f.upper().endswith(".PIC") and not any(t in f.upper() for t in tracks) and f.upper() != "INSTALL.PIC":
            p = os.path.join(root, f)
            name = os.path.splitext(f)[0] + ".bmp"
            pic_to_bmp(p, os.path.join(out_dir, name))
