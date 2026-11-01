#!/usr/bin/env python3
"""CI-only actual Chrome screenshots before/after compositor move + resize.

Require a stable SAME native XDG ID/PID but changed window position and size.
Require new 400x300 crop with a visibly expanded region versus the original
320x240 source crop. A stale framebuffer, no-op IPC or hit-test-only success
is never a PASS.
"""
import json
import sys
from pathlib import Path
from assert_news_render_pixels import parse_ppm, patch_average


def nodes(root):
    if isinstance(root, dict):
        yield root
        for key in ("nodes", "floating_nodes"):
            for n in root.get(key, []):
                yield from nodes(n)


def actor(path, pid):
    tree = json.loads(Path(path).read_text(encoding="utf-8"))
    matches = [n for n in nodes(tree)
               if n.get("type") == "con" and n.get("pid") == pid and
               n.get("shell") == "xdg_shell" and n.get("border") == "none"]
    if len(matches) != 1:
        raise ValueError(f"FAIL_NO_UNIQUE_NATIVE_CHROME_ACTOR count={len(matches)}")
    a = matches[0]
    r = a.get("rect") or {}
    dims = [r.get(k) for k in ("x", "y", "width", "height")]
    if type(a.get("id")) is not int or not all(type(x) is int for x in dims):
        raise ValueError("FAIL_INCOMPLETE_NATIVE_CHROME_GEOMETRY")
    return a["id"], dims


def value(ppm, x, y):
    w, h, data = ppm
    if x < 9 or y < 9 or x >= w - 9 or y >= h - 9:
        raise ValueError(f"FAIL_UNMEASURABLE_VIEWPORT_PIXEL: {x},{y}")
    return patch_average(data, w, h, x, y)


def main():
    if len(sys.argv) != 7:
        raise SystemExit(
            "usage: assert_real_chrome_motion_pixels.py "
            "baseline.ppm original.ppm moved.ppm "
            "before-tree.json after-tree.json native_pid")
    base, first, moved = [parse_ppm(p) for p in sys.argv[1:4]]
    if any(p[:2] != (1920, 1080) for p in (base, first, moved)):
        raise ValueError("FAIL_OUTPUT_RESOLUTION_CHANGED")
    pid = int(sys.argv[6])
    old_id, (x0, y0, w0, h0) = actor(sys.argv[4], pid)
    new_id, (x1, y1, w1, h1) = actor(sys.argv[5], pid)
    if old_id != new_id or (x0, y0, w0, h0) != (0, 0, 1920, 1080):
        raise ValueError("FAIL_BROWSER_IDENTITY_OR_ORIGINAL_RECT_CHANGED")
    dx, dy = x1 - x0, y1 - y0
    if not (330 <= dx <= 600 and 150 <= dy <= 330 and
            950 <= w1 <= 1250 and 600 <= h1 <= 850 and
            x1+w1 < 1920 and y1+h1 < 1080):
        raise ValueError(
            f"FAIL_MOVE_RESIZE_NOT_OBSERVED dx={dx} dy={dy} {w1}x{h1}")
    # The old crop and the new crop should be disjoint. Probe original
    # white content now black, plus new 400x300 inside including extension
    # BEYOND original 320x240 size, and four new outer edges.
    old_inside = (x0 + 160, y0 + 160)
    new_inside = [
        (x1 + 160, y1 + 160),
        (x1 + 240, y1 + 220),
        (x1 + 450, y1 + 170),  # new width extension: 96+320..96+400
        (x1 + 170, y1 + 355),  # new height extension: 96+240..96+300
    ]
    outside = [
        (x1 + 35, y1 + 170),   # left
        (x1 + 180, y1 + 35),   # top
        (x1 + 550, y1 + 180),  # right
        (x1 + 180, y1 + 445),  # bottom
        (x1 + 35, y1 + 35),    # diagonal
        (10, 10),              # desktop
    ]
    if value(first, *old_inside) < 200 or value(base, *old_inside) > 45:
        raise ValueError("FAIL_ORIGINAL_CHROME_SCREENSHOT_NOT_VISIBLE")
    old_after = value(moved, *old_inside)
    inside_values = [value(moved, *p) for p in new_inside]
    outside_values = [value(moved, *p) for p in outside]
    outside_baseline = [value(base, *p) for p in outside]
    print("SD_CI_REAL_CHROME_DYNAMIC_MEASURE "
          f"actor_id={old_id} dx={dx} dy={dy} resized={w1}x{h1} "
          f"old_after={old_after:.1f} "
          f"new_inside={[round(v, 1) for v in inside_values]} "
          f"new_outside={[round(v, 1) for v in outside_values]}",
          flush=True)
    if old_after > 45:
        raise ValueError("FAIL_OLD_CHROME_VIEWPORT_RESIDUAL_AFTER_MOVE")
    if min(inside_values) < 170:
        raise ValueError("FAIL_NEW_CHROME_CLIP_NOT_VISIBLE_OR_NOT_RESIZED")
    if max(outside_values) > 48:
        raise ValueError("FAIL_MOVED_CHROME_PIXEL_LEAK")
    if max(abs(a-b) for a, b in zip(outside_values, outside_baseline)) > 30:
        raise ValueError("FAIL_MOVED_CHROME_BACKGROUND_NOT_PRESERVED")
    if min(inside_values) - max(outside_values) < 120:
        raise ValueError("FAIL_DYNAMIC_VIEWPORT_CONTRAST_INSUFFICIENT")
    print("SD_CI_REAL_CHROME_DYNAMIC_PIXELS: result=PASS "
          "same_native_actor=1 moved=1 resized_window=1 "
          "viewport_before=320x240 viewport_after=400x300 "
          "expanded_pixels_verified=1 stale_old_pixels=0 "
          "all_four_edges_clear=1 authority=NONE")


if __name__ == "__main__":
    main()
