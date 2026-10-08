"""Print a PNG preview of an SDL BMP as base64 (stdlib only)."""
import base64
import struct
import sys
import zlib
from pathlib import Path

blob = Path(sys.argv[1]).read_bytes()
offset = struct.unpack_from('<I', blob, 10)[0]
width, height = struct.unpack_from('<ii', blob, 18)
bits = struct.unpack_from('<H', blob, 28)[0]
if bits not in (24, 32):
    raise ValueError('Expected RGB BMP')
step = int(sys.argv[2]) if len(sys.argv) > 2 else 2
out_width, out_height = width // step, abs(height) // step
stride = ((width * bits + 31) // 32) * 4
pixels = bytearray()
for y in range(out_height):
    pixels.append(0)
    source_y = abs(height) - 1 - y * step if height > 0 else y * step
    for x in range(out_width):
        start = offset + source_y * stride + x * step * (bits // 8)
        b, g, r = blob[start:start + 3]
        pixels.extend((r, g, b))

def chunk(kind, data):
    return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data) & 0xffffffff)

png = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', out_width, out_height, 8, 2, 0, 0, 0))
png += chunk(b'IDAT', zlib.compress(pixels)) + chunk(b'IEND', b'')
print(base64.b64encode(png).decode())
