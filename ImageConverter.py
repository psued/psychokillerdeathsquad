import sys
import struct
from math import ceil, log2
from PIL import Image

out = open(sys.argv[2], 'wb')
img = Image.open(sys.argv[1])

palette = [(r >> 4, g >> 4, b >> 4) for (_, (r,g,b)) in img.getcolors()]
n = ceil(log2(len(palette))) # bits per color, one for transparency

# write metadata
out.write(struct.pack('H', img.width))
out.write(struct.pack('H', img.height))
out.write(struct.pack('B', n))

# write palette
v = -1
for c in palette:
    if v == -1:
        out.write(struct.pack('B', (c[0] << 4) + c[1]))
        v = c[2]
    else:
        out.write(struct.pack('B', (v << 4) + c[0]))
        out.write(struct.pack('B', (c[1] << 4) + c[2]))
        v = -1

if v != -1:
    out.write(struct.pack('B', v << 4))

bytes_left = int(12*(2**n)/8 - ceil(len(palette) * 1.5))
for _ in range(bytes_left):
    out.write(b'\x00')

# write pixel values
s = ''
for y in range(img.height):
    for x in range(img.width):
        r,g,b = img.getpixel((x,y))
        cindex = palette.index((r >> 4, g >> 4, b >> 4))
        s += f"{{:0{n}b}}".format(cindex)

for _ in range(ceil(len(s) / 8)):
    substr = s[:8]
    if len(substr) < 8:
        substr += '0' * (8 - len(substr))
    out.write(struct.pack('B', int(substr, 2)))
    s = s[8:]

img.close()
out.close()
