#!/usr/bin/env python3
"""Pack res/ directory into a zip file for embedding into the exe."""
import os
import sys
import zipfile

EXCLUDE_FILES = {'bkimage.png'}

def pack_resources(res_dir, output_zip):
    if not os.path.isdir(res_dir):
        print(f"Error: {res_dir} does not exist")
        sys.exit(1)

    with zipfile.ZipFile(output_zip, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as zf:
        for root, dirs, files in os.walk(res_dir):
            for fname in files:
                if fname in EXCLUDE_FILES:
                    print(f"  - {fname} (excluded)")
                    continue
                fpath = os.path.join(root, fname)
                arcname = os.path.relpath(fpath, res_dir).replace(os.sep, '/')
                zf.write(fpath, arcname)
                print(f"  + {arcname}")

    size = os.path.getsize(output_zip)
    print(f"\nDone! {output_zip} ({size} bytes, {size/1024:.1f} KB)")

if __name__ == '__main__':
    res_dir = sys.argv[1] if len(sys.argv) > 1 else 'res'
    output_zip = sys.argv[2] if len(sys.argv) > 2 else 'resources.zip'
    print(f"Packing {res_dir} -> {output_zip}")
    pack_resources(res_dir, output_zip)
