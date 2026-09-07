with open("assets/LEVELS/AUSTRIA/AUSTRIA.TAB", "rb") as f:
    tab = f.read()

print("AUSTRIA.TAB total size:", len(tab))

# Let's inspect rows: tab[row * 256 + col]
# What is along row 0? row 1? row 16? row 32? row 128? row 255?
for row in [0, 1, 15, 16, 31, 32, 63, 64, 127, 128, 192, 255]:
    first_16 = list(tab[row*256 : row*256 + 16])
    print(f"Row {row:3d} first 16 bytes: {first_16}")

# Check identity row: is there a row where tab[row * 256 + c] == c?
identity_rows = []
for r in range(256):
    matches = sum(1 for c in range(256) if tab[r*256 + c] == c)
    if matches > 200:
        identity_rows.append((r, matches))

print("\nIdentity / 100% brightness rows:", identity_rows)

# What if row is the color and col is the light level?
# Let's check if tab[c * 256 + light] or tab[light * 256 + c]:
col_identity = []
for c in range(256):
    matches = sum(1 for r in range(256) if tab[r*256 + c] == r)
    if matches > 200:
        col_identity.append((c, matches))
print("Column identity:", col_identity)
