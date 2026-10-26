#!/usr/bin/env python3
"""Validate *actual screenshot pixels*, not just scene hit-test geometry.

One white native Wayland terminal is held behind a CI-only wlroots clip.
The desktop outside the content is black. Sample independent inside/outside
and desktop points on the same real grim P6 screenshot. Reject leaks.
"""
import re
import statistics
import sys
from pathlib import Path


def parse_ppm(path):
    data = Path(path).read_bytes()
    if not data.startswith(b"P6"):
        raise ValueError("Expected uncompressed P6 pixel screenshot from grim")
    at = 2
    tokens = []
    while len(tokens) < 3:
        while at < len(data) and data[at] in b" \r\n\t":
            at += 1
        if at < len(data) and data[at] == 35:
            at = data.find(b"\n", at) + 1
            if at == 0:
                raise ValueError("Malformed PPM comment")
            continue
        start = at
        while at < len(data) and data[at] not in b" \r\n\t":
            at += 1
        if at == start:
            raise ValueError("Truncated PPM header")
        tokens.append(int(data[start:at]))
    if at >= len(data) or data[at] not in b" \r\n\t":
        raise ValueError("Malformed PPM pixel data delimiter")
    # All pixels begin after one whitespace delimiter; valid output uses LF.
    at += 1
    w, h, maximum = tokens
    if not (500 <= w <= 5000 and 400 <= h <= 5000 and maximum == 255):
        raise ValueError(f"Unexpected screenshot size/range: {tokens}")
    if len(data) - at != w * h * 3:
        raise ValueError("PPM pixel count does not match screenshot dimensions")
    return w, h, memoryview(data)[at:]


def patch_average(image, width, height, x, y):
    if not (4 <= x < width-4 and 4 <= y < height-4):
        raise ValueError(f"Sample outside real compositor screenshot: ({x},{y})")
    lumas = []
    for yy in range(y-2, y+3):
        for xx in range(x-2, x+3):
            i = (yy * width + xx) * 3
            r, g, b = image[i:i+3]
            lumas.append(0.2126 * r + 0.7152 * g + 0.0722 * b)
    return statistics.mean(lumas)


def main():
    if len(sys.argv) != 3:
        raise SystemExit("usage: assert_news_render_pixels.py sway.log clip.ppm")
    log = Path(sys.argv[1]).read_text(encoding="utf-8")
    lines = [line for line in log.splitlines()
             if "SD_CI_RENDER_PIXEL inside=" in line and "clip_held=1" in line]
    if len(lines) != 1:
        raise SystemExit(f"expected one verified held native clip, got {len(lines)}")
    m = re.search(r"inside=(\d+),(\d+) outside=(\d+),(\d+)", lines[0])
    if not m:
        raise SystemExit("Native clip log omitted actual global pixel coordinates")
    ix, iy, ox, oy = map(int, m.groups())
    if not (abs(ix-ox) > 50 and abs(iy-oy) > 50):
        raise SystemExit("Pixel controls are not independent scene locations")
    width, height, pixels = parse_ppm(sys.argv[2])
    inside = patch_average(pixels, width, height, ix, iy)
    outside = patch_average(pixels, width, height, ox, oy)
    desktop = patch_average(pixels, width, height, 10, 10)
    print(
        f"NEWS real compositor pixels: dimensions={width}x{height}, "
        f"inside_luma={inside:.1f}, outside_luma={outside:.1f}, "
        f"desktop_luma={desktop:.1f}"
    )
    # Foot has opaque white background; the isolated Sway desktop is black.
    # At most 45 luma outside accommodates tiny compositor AA/shadow artifacts.
    if inside < 180 or outside > 45 or desktop > 45 or inside - outside < 130:
        raise SystemExit("FAIL: real screenshot does not demonstrate white-inside / black-outside clip")
    if abs(outside - desktop) > 25:
        raise SystemExit("FAIL: outside clip differs from the known black background")
    print("Stage Director actual compositor render PIXELS confined by wlroots clip: PASS")
    print("Does not establish real pointer, keyboard, popup or WATCH session containment.")


if __name__ == "__main__":
    main()
