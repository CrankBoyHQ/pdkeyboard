# convert png image to c code, playdate

import sys
from pathlib import Path
from PIL import Image

path = Path(sys.argv[1])
img = Image.open(path).convert("RGBA")
w, h = img.size
basename = path.stem

bits = []
for y in range(h):
    for x in range(w):
        r, g, b, a = img.getpixel((x, y))
        if a == 0 or (r > 127 and g > 127 and b > 127):
            bits.append(1)
        else:
            bits.append(0)

bytes_out = []
for i in range(0, len(bits), 8):
    val = 0
    for j in range(8):
        if i + j < len(bits):
            val |= bits[i + j] << (7 - j)
    bytes_out.append(val)

lines = []
lines.append(f"unsigned char {basename}_data[] = {{")
chunks = []
for i in range(0, len(bytes_out), 8):
    row = ", ".join(f"0x{b:02X}" for b in bytes_out[i:i + 8])
    chunks.append(f"    {row}")
lines.append(",\n".join(chunks))
lines.append("};")
lines.append(f"unsigned {basename}_width = {w};")
lines.append(f"unsigned {basename}_height = {h};")

print("\n".join(lines))
