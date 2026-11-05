import struct, zlib, os

def read_png(path):
    with open(path, 'rb') as f:
        data = f.read()
    assert data[:8] == b'\x89PNG\r\n\x1a\n'
    pos = 8
    width = height = bit_depth = color_type = 0
    idat = b''
    while pos < len(data):
        length = struct.unpack('>I', data[pos:pos+4])[0]
        ctype = data[pos+4:pos+8]
        chunk = data[pos+8:pos+8+length]
        if ctype == b'IHDR':
            width, height, bit_depth, color_type = struct.unpack('>IIBB', chunk[:10])
        elif ctype == b'IDAT':
            idat += chunk
        pos += 12 + length
    raw = zlib.decompress(idat)
    channels = {0:1, 2:3, 3:1, 4:2, 6:4}[color_type]
    bpp = channels * (bit_depth // 8)
    stride = width * bpp
    rows = []
    i = 0
    prev = bytearray(stride)
    for y in range(height):
        filter_type = raw[i]; i += 1
        line = bytearray(raw[i:i+stride]); i += stride
        if filter_type == 1:
            for x in range(bpp, stride): line[x] = (line[x] + line[x-bpp]) & 0xFF
        elif filter_type == 2:
            for x in range(stride): line[x] = (line[x] + prev[x]) & 0xFF
        elif filter_type == 3:
            for x in range(stride):
                a = line[x-bpp] if x >= bpp else 0
                line[x] = (line[x] + ((a + prev[x]) // 2)) & 0xFF
        elif filter_type == 4:
            for x in range(stride):
                a = line[x-bpp] if x >= bpp else 0
                b = prev[x]
                c = prev[x-bpp] if x >= bpp else 0
                p = a + b - c
                pa, pb, pc = abs(p-a), abs(p-b), abs(p-c)
                pr = a if pa <= pb and pa <= pc else (b if pb <= pc else c)
                line[x] = (line[x] + pr) & 0xFF
        rows.append(line)
        prev = line
    return width, height, channels, rows

base = 'J:/net_api_test/res/image'
for name in ['vscrollbar.png', 'hscrollbar.png', 'scrollbar_bkg.png', 'scrollbar_thumb.png', 'scrollbar_thumb2.png']:
    path = os.path.join(base, name)
    w, h, ch, rows = read_png(path)
    print(f'{name}: {w}x{h}, channels={ch}')
    samples = [0, h//6, h//4, h//3, h//2, 2*h//3, 3*h//4, 5*h//6, h-1]
    for y in samples:
        if y < 0 or y >= h: continue
        x = w // 2
        idx = x * ch
        if ch == 4:
            r, g, b, a = rows[y][idx], rows[y][idx+1], rows[y][idx+2], rows[y][idx+3]
            print(f'  y={y:3d}: RGBA=({r:3d},{g:3d},{b:3d},{a:3d})')
        elif ch == 3:
            r, g, b = rows[y][idx], rows[y][idx+1], rows[y][idx+2]
            print(f'  y={y:3d}: RGB=({r:3d},{g:3d},{b:3d})')
        elif ch == 2:
            v, a = rows[y][idx], rows[y][idx+1]
            print(f'  y={y:3d}: VA=({v:3d},{a:3d})')
        elif ch == 1:
            v = rows[y][idx]
            print(f'  y={y:3d}: V={v:3d}')
    print()