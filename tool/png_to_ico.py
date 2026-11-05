from PIL import Image
import os

src = r'J:\net_api_test\res\image\logo.png'
dst = r'J:\net_api_test\res\app.ico'

img = Image.open(src)
print(f"logo.png size: {img.size}, mode: {img.mode}")

img.save(dst, format='ICO', sizes=[(16,16),(32,32),(48,48),(64,64),(128,128),(256,256)])
print(f"Saved: {dst} ({os.path.getsize(dst)} bytes)")