#!/usr/bin/env python3
"""Validate *actual screenshot pixels*, not just scene hit-test geometry.

One white native Wayland terminal is held behind a CI-only wlroots clip.
The desktop outside the content is black. Sample independent inside/outside
and desktop points on the same real grim P6 screenshot. Reject leaks.
"""
import json
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
    if len(sys.argv) != 4:
        raise SystemExit("usage: assert_news_render_pixels.py sway.log clip.ppm sway_get_tree.json")
    log = Path(sys.argv[1]).read_text(encoding="utf-8")
    lines = [line for line in log.splitlines()
             if "SD_CI_RENDER_PIXEL inside=" in line and "clip_held=1" in line]
    if len(lines) != 1:
        raise SystemExit(f"expected one verified held native clip, got {len(lines)}")
    m = re.search(r"inside=(\d+),(\d+) outside=(\d+),(\d+)", lines[0])
    if not m:
        raise SystemExit("Native clip log omitted original scene hit-test coordinates")
    # The initial native clip probe can run before Sway's transaction has
    # placed the content tree at its final on-screen location. The settled
    # Sway IPC tree is the authoritative compositor-observed actor position.
    tree = json.loads(Path(sys.argv[3]).read_text(encoding="utf-8"))

    def nodes(node):
        yield node
        for key in ("nodes", "floating_nodes"):
            for child in node.get(key, []):
                yield from nodes(child)

    actors = [n for n in nodes(tree)
              if n.get("app_id") == "post-apollo-weather-news"
              and n.get("name") == "SD1-CI-DISPOSABLE-NOT-BROWSER"
              and n.get("shell") == "xdg_shell"]
    if len(actors) != 1:
        raise SystemExit("Missing or ambiguous native actor in actual Sway IPC tree")
    rect = actors[0].get("rect") or {}
    x, y, w, h = (rect.get(k) for k in ("x", "y", "width", "height"))
    if not all(isinstance(v, int) for v in (x, y, w, h)) or w < 600 or h < 440:
        raise SystemExit("Actual Sway IPC actor rect invalid or too small")
    width, height, pixels = parse_ppm(sys.argv[2])
    inside_points = [(x + 160, y + 160), (x + 240, y + 220)]
    # Sample ALL FOUR sides independently. Border/title decorations are
    # outside these points; each is in the expected black content margin.
    outside_points = [
        (x + 24, y + 160),   # left of the intentional 320x240 viewport
        (x + 160, y + 24),   # above
        (x + 450, y + 150),  # right
        (x + 160, y + 370),  # below
        (x + 24, y + 24),    # top-left corner regression
    ]
    inside_values = [patch_average(pixels, width, height, px, py)
                     for px, py in inside_points]
    outside_values = [patch_average(pixels, width, height, px, py)
                      for px, py in outside_points]
    inside = min(inside_values)
    outside = max(outside_values)
    desktop = patch_average(pixels, width, height, 10, 10)
    print(
        f"NEWS real compositor pixels: image={width}x{height} "
        f"actual_actor={x},{y} {w}x{h} "
        f"inside_luma={[round(v, 1) for v in inside_values]} "
        f"outside_luma={[round(v, 1) for v in outside_values]} "
        f"desktop_luma={desktop:.1f}"
    )
    # Foot's native buffer is opaque white; Sway's headless wallpaper black.
    if inside < 180 or outside > 45 or desktop > 45 or inside - outside < 130:
        raise SystemExit("FAIL: screenshot lacks white-inside / black-outside crop")
    if max(abs(v - desktop) for v in outside_values) > 25:
        raise SystemExit("FAIL: outside crop contains visible content or decorations")
    print("Stage Director actual compositor render PIXELS confined by wlroots clip: PASS")
    print("Does not establish real pointer, keyboard, popup or WATCH session containment.")


if __name__ == "__main__":
    main()
