import sys
import struct
import binascii

def bitrev(x, width):
    y = 0
    for _ in range(width):
        y = (y << 1) | (x & 1)
        x >>= 1
    return y

p = sys.argv[1]

with open(p, "rb") as f:
    d = f.read()

# boot2 前 252 bytes + 最後 4 bytes checksum
d = d[:252] + bytes(max(0, 252 - len(d)))

# RP2040 boot2 checksum: bit-reverse each byte, crc32 seed = 0xffffffff
crc = binascii.crc32(
    bytes(bitrev(b, 8) for b in d),
    0xffffffff ^ 0xffffffff
) ^ 0xffffffff

crc &= 0xffffffff

d += struct.pack("<I", bitrev(crc, 32))

with open(p, "wb") as f:
    f.write(d)

print("boot2 checksum:", hex(bitrev(crc, 32)))