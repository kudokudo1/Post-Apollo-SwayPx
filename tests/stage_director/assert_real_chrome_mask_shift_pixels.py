#!/usr/bin/env python3
"""CI only: real Chrome screenshots with stationary XDG and relocated mask.

The browser must remain 1920x1080 with exact same native Sway container ID,
PID, border and rectangle. Two actual 1920x1080 screenshots prove old
320x240 crop is cleared, new 400x300 crop appears at a different location
and all four outside edges are black. One scene hit-test is not enough.
"""
import json
import sys
from pathlib import Path
from assert_news_render_pixels import parse_ppm, patch_average


def match(path, pid):
    root = json.loads(Path(path).read_text(encoding="utf-8"))
    def walk(n):
        if isinstance(n, dict):
            yield n
            for k in ("nodes", "floating_nodes"):
                for child in n.get(k, []):
                    yield from walk(child)
    candidates = [x for x in walk(root)
                  if x.get("type") == "con" and x.get("shell") == "xdg_shell"
                  and x.get("pid") == pid and x.get("border") == "none"]
    if len(candidates) != 1:
        raise ValueError(f"FAIL_NATIVE_CHROME_IDENTITY_NOT_UNIQUE {len(candidates)}")
    x = candidates[0]
    r = x.get("rect") or {}
    rect = tuple(r.get(k) for k in ("x", "y", "width", "height"))
    if not all(type(v) is int for v in rect) or type(x.get("id")) is not int:
        raise ValueError("FAIL_NATIVE_WINDOW_RECT_NOT_VALID")
    return x["id"], rect


def samples(ppm, coords):
    w, h, image = ppm
    for x, y in coords:
        if not (8 < x < w - 8 and 8 < y < h - 8):
            raise ValueError(f"FAIL_PIXEL_OUTSIDE_OUTPUT {x},{y}")
    return [patch_average(image, w, h, x, y) for x, y in coords]


def main():
    if len(sys.argv) != 7:
        raise SystemExit("usage: mask_shift_pixels.py baseline.ppm "
                         "initial.ppm shifted.ppm initial-tree.json "
                         "shifted-tree.json native-pid")
    before, initial, shifted = [parse_ppm(p) for p in sys.argv[1:4]]
    if any(p[:2] != (1920, 1080) for p in (before, initial, shifted)):
        raise ValueError("FAIL_OUTPUT_RESOLUTION_CHANGED")
    pid = int(sys.argv[6])
    old_id, old_rect = match(sys.argv[4], pid)
    new_id, new_rect = match(sys.argv[5], pid)
    if old_id != new_id or old_rect != new_rect or old_rect != (0, 0, 1920, 1080):
        raise ValueError(f"FAIL_NATIVE_CHROME_WINDOW_MOVED_OR_REPLACED "
                         f"old={old_id, old_rect} new={new_id, new_rect}")
    old_inside = [(160, 160), (240, 220)]
    # New crop starts at (520,240), width 400, height 300. Include points
    # beyond the original 320x240 dimensions, proving expansion.
    new_inside = [(600, 320), (760, 400), (880, 360), (650, 510)]
    new_outside = [(460, 340), (600, 180), (980, 340), (600, 600), (10, 10)]
    old_before = samples(initial, old_inside)
    old_after = samples(shifted, old_inside)
    inside_after = samples(shifted, new_inside)
    outer_after = samples(shifted, new_outside)
    outer_baseline = samples(before, new_outside)
    initial_old_background = samples(before, old_inside)
    print("SD_CI_CHROME_MASK_PIXELS_MEASURE "
          f"same_native_id={old_id} rect={old_rect} "
          f"old_before={old_before} old_after={old_after} "
          f"new_inside={inside_after} new_outside={outer_after}",
          flush=True)
    if min(old_before) < 180 or max(initial_old_background) > 45:
        raise ValueError("FAIL_ORIGINAL_CHROME_MASK_NOT_VISIBLE")
    if max(old_after) > 45:
        raise ValueError("FAIL_STALE_CHROME_PIXELS_OLD_MASK")
    if min(inside_after) < 170:
        raise ValueError("FAIL_NEW_EXPANDED_MASK_NOT_VISIBLE")
    if max(outer_after) > 45:
        raise ValueError("FAIL_NEW_MASK_LEAKED_PIXELS_OUTSIDE")
    if max(abs(a-b) for a,b in zip(outer_after, outer_baseline)) > 30:
        raise ValueError("FAIL_OUTSIDE_BACKGROUND_CHANGED")
    if min(inside_after) - max(outer_after) < 120:
        raise ValueError("FAIL_MASK_CONTRAST_INSUFFICIENT")
    print("SD_CI_CHROME_MASK_SHIFT_PIXELS: result=PASS "
          "stationary_native_xdg=1 same_process=1 "
          "old_pixels_cleared=1 new_mask_visible=1 "
          "source_viewport=400x300 four_edges_clear=1 authority=NONE")


if __name__ == "__main__":
    main()
