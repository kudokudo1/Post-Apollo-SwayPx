#!/usr/bin/env python3
"""Read actual grim pixels after a second Chrome executable XDG toplevel.

This is DIAGNOSTIC ONLY, not a 'popup containment passed' checker.
It reports escaped browser-like pixels and whether the original viewport
changed. Only a *separate mitigation test* can claim containment.
"""
import json
import sys
from pathlib import Path

from assert_news_render_pixels import parse_ppm


def high_outside(ppm):
    width, height, rgb = parse_ppm(ppm)
    if (width, height) != (1920, 1080):
        raise ValueError("FAIL_SIBLING_SCREENSHOT_DIMENSIONS")
    count = 0
    total = 0
    # Sample evenly across the entire output, excluding the first fixed
    # 320x240 clip. Do not infer from status text or popup rectangles.
    for y in range(12, height, 24):
        for x in range(12, width, 24):
            if 96 <= x < 416 and 96 <= y < 336:
                continue
            idx = (y * width + x) * 3
            pixel = rgb[idx:idx + 3]
            total += 1
            if min(pixel) > 175:
                count += 1
    return count, total


def main():
    if len(sys.argv) != 6:
        raise SystemExit("usage: sibling_discovery.py crop.ppm after.ppm "
                         "before-tree.json after-tree.json original-id")
    before, total = high_outside(sys.argv[1])
    after, _ = high_outside(sys.argv[2])
    original_id = int(sys.argv[5])
    old_tree = json.loads(Path(sys.argv[3]).read_text())
    new_tree = json.loads(Path(sys.argv[4]).read_text())

    def native(tree):
        result = []
        def walk(node):
            if isinstance(node, dict):
                if node.get("type") == "con" and node.get("shell") == "xdg_shell":
                    result.append(node)
                for key in ("nodes", "floating_nodes"):
                    for sub in node.get(key, []):
                        walk(sub)
        walk(tree)
        return result

    prior = {n["id"]: n for n in native(old_tree)}
    latest = {n["id"]: n for n in native(new_tree)}
    if original_id not in prior:
        raise ValueError("FAIL_SIBLING_ORIGINAL_NOT_PREVIOUSLY_MAPPED")
    if original_id not in latest:
        raise ValueError("FAIL_SIBLING_ORIGINAL_REMOVED")
    extras = [n for k, n in latest.items() if k != original_id]
    if not extras:
        raise ValueError("FAIL_SIBLING_NO_NEW_NATIVE_XDG_WINDOW")
    shifted = prior[original_id].get("rect") != latest[original_id].get("rect")
    changed = max(0, after - before)
    observation = "VISIBLE_OUTSIDE_MASK" if changed > 0 else "NO_OUTSIDE_BRIGHT_PIXELS"
    print("SD_CI_CHROME_SECOND_TOPLEVEL_PIXELS "
          f"before_bright_outside={before} after_bright_outside={after} "
          f"new_bright_outside={changed} sampled_outside={total} "
          f"original_geometry_changed={int(shifted)} result={observation}",
          flush=True)
    print("SD_CI_CHROME_SECOND_TOPLEVEL_DIAGNOSTIC: "
          f"result=OBSERVED second_native_xdg={len(extras)} "
          f"pixel_classification={observation} "
          "containment=UNVERIFIED actor=CHROME_EXECUTABLE "
          "authority=NONE", flush=True)


if __name__ == "__main__":
    main()
