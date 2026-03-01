#!/usr/bin/env python3
"""
make_icon.py — Build AppIcon.icns from macos_app/icon.svg
Requires: rsvg-convert (brew install librsvg) + pillow
Output:   AppIcon.icns in the current working directory
"""

import os
import sys
import shutil
import subprocess

SCRIPT_DIR  = os.path.dirname(os.path.abspath(__file__))
SVG_PATH    = os.path.join(SCRIPT_DIR, "icon.svg")
ICONSET_DIR = "Basic++.iconset"
OUTPUT_ICNS = "AppIcon.icns"
MASTER_PNG  = "icon_master_1024.png"

SIZES = [
    ("icon_16x16.png",        16),
    ("icon_16x16@2x.png",     32),
    ("icon_32x32.png",        32),
    ("icon_32x32@2x.png",     64),
    ("icon_128x128.png",     128),
    ("icon_128x128@2x.png",  256),
    ("icon_256x256.png",     256),
    ("icon_256x256@2x.png",  512),
    ("icon_512x512.png",     512),
    ("icon_512x512@2x.png", 1024),
]

def find_rsvg():
    for p in ["/opt/homebrew/bin/rsvg-convert", "/usr/local/bin/rsvg-convert",
              shutil.which("rsvg-convert") or ""]:
        if p and os.path.isfile(p):
            return p
    return None

def main():
    try:
        from PIL import Image
    except ImportError:
        subprocess.check_call([sys.executable, "-m", "pip", "install", "--quiet", "pillow"])
        from PIL import Image

    rsvg = find_rsvg()
    if not rsvg:
        print("ERROR: rsvg-convert not found. Fix with: brew install librsvg", file=sys.stderr)
        sys.exit(1)

    if not os.path.exists(SVG_PATH):
        print(f"ERROR: SVG not found at {SVG_PATH}", file=sys.stderr)
        sys.exit(1)

    print("Rendering icon.svg at 1024x1024 ...")
    r = subprocess.run(
        [rsvg, "-w", "1024", "-h", "1024", SVG_PATH, "-o", MASTER_PNG],
        capture_output=True, text=True
    )
    if r.returncode != 0:
        print(f"ERROR: rsvg-convert failed:\n{r.stderr}", file=sys.stderr)
        sys.exit(1)

    master = Image.open(MASTER_PNG).convert("RGBA")

    if os.path.exists(ICONSET_DIR):
        shutil.rmtree(ICONSET_DIR)
    os.makedirs(ICONSET_DIR)

    for filename, size in SIZES:
        dst = os.path.join(ICONSET_DIR, filename)
        master.resize((size, size), Image.LANCZOS).save(dst, "PNG")
        print(f"  {filename}  ({size}x{size})")

    if os.path.exists(OUTPUT_ICNS):
        os.remove(OUTPUT_ICNS)

    r = subprocess.run(
        ["iconutil", "-c", "icns", ICONSET_DIR, "-o", OUTPUT_ICNS],
        capture_output=True, text=True
    )
    if r.returncode != 0:
        print(f"ERROR: iconutil failed:\n{r.stderr}", file=sys.stderr)
        sys.exit(1)

    shutil.rmtree(ICONSET_DIR)
    os.remove(MASTER_PNG)
    print(f"Created {OUTPUT_ICNS}")

if __name__ == "__main__":
    main()
