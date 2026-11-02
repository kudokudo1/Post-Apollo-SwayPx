#!/usr/bin/env python3
"""Real SwayFX screenshot pixels for the CI-only Chrome viewport experiment.

Measure screenshot relative to *actual compositor observed* native browser
geometry, and compare against screenshot BEFORE Chrome existed on same
1920x1080 headless output. Never accept scene hit-tests as pixel evidence.
"""
import json
import sys
from assert_news_render_pixels import parse_ppm, patch_average


def nodes(node):
    if isinstance(node, dict):
        yield node
        for key in ("nodes", "floating_nodes"):
            for child in node.get(key, []):
                yield from nodes(child)


def main():
    if len(sys.argv) != 5:
        raise SystemExit(
            "usage: assert_real_chrome_clip_pixels.py "
            "before.ppm with-chrome.ppm get-tree.json EXACT_NATIVE_PID")
    baseline = parse_ppm(sys.argv[1])
    with_chrome = parse_ppm(sys.argv[2])
    if baseline[:2] != with_chrome[:2]:
        raise SystemExit("FAIL: compositor output dimensions changed")
    width, height, base = baseline
    _, _, pixels = with_chrome
    if (width, height) != (1920, 1080):
        raise SystemExit(f"FAIL: unexpected CI output {width}x{height}")
    tree = json.load(open(sys.argv[3], encoding="utf-8"))
    pid = int(sys.argv[4])
    candidates = [n for n in nodes(tree)
                  if n.get("pid") == pid and n.get("type") == "con"
                  and n.get("shell") == "xdg_shell"]
    if len(candidates) != 1:
        raise SystemExit(f"FAIL: browser native candidate count={len(candidates)}")
    actor = candidates[0]
    rect = actor.get("rect") or {}
    x, y, w, h = (rect.get(k) for k in ("x", "y", "width", "height"))
    if not all(type(v) is int for v in (x, y, w, h)) or w < 600 or h < 440:
        raise SystemExit("FAIL: native browser geometry invalid")
    # Actual 320x240 clip is at local source coords 96,96. These are
    # independent left/top/right/bottom probes (no UI/title identity).
    inside = [(x + 160, y + 160), (x + 240, y + 220)]
    outside = [
        (x + 24, y + 160),
        (x + 160, y + 24),
        (x + 450, y + 150),
        (x + 160, y + 370),
        (x + 24, y + 24),
    ]
    all_points = inside + outside + [(10, 10)]
    if any(not (6 <= px < width - 6 and 6 <= py < height - 6)
           for px, py in all_points):
        raise SystemExit("FAIL: browser is not entirely measurable on output")
    before = [patch_average(base, width, height, *point)
              for point in all_points]
    after = [patch_average(pixels, width, height, *point)
             for point in all_points]
    print("SD_CI_REAL_CHROME_SCREENSHOT_MEASURE "
          f"actor={x},{y},{w},{h} "
          f"before_inside={[round(n, 1) for n in before[:2]]} "
          f"after_inside={[round(n, 1) for n in after[:2]]} "
          f"before_outside={[round(n, 1) for n in before[2:7]]} "
          f"after_outside={[round(n, 1) for n in after[2:7]]} "
          f"desktop_before={before[7]:.1f} "
          f"desktop_after={after[7]:.1f}", flush=True)
    if max(before) > 45:
        raise SystemExit("FAIL: baseline screenshot not dark at all probes")
    if min(after[:2]) < 165:
        raise SystemExit("FAIL: real browser was not visible inside viewport")
    if max(after[2:7]) > 50:
        raise SystemExit("FAIL: real browser pixels leak beyond viewport")
    if max(abs(a-b) for a,b in zip(after[2:7], before[2:7])) > 30:
        raise SystemExit("FAIL: non-background pixels outside viewport")
    if after[7] > 45:
        raise SystemExit("FAIL: desktop background changed outside actor")
    if min(after[:2]) - max(after[2:7]) < 120:
        raise SystemExit("FAIL: inside/outside contrast insufficient")
    print("SD_CI_REAL_CHROME_RENDER_PIXELS: real_chrome=1 "
          "actual_screenshot=1 visible_inside=1 "
          "no_left_top_right_bottom_leak=1 "
          "authority=NONE result=PASS")


if __name__ == "__main__":
    main()
