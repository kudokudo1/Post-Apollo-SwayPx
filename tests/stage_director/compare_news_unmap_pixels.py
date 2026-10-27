#!/usr/bin/env python3
"""Classify actual CI rendered pixels before/after native actor destruction.

This is supplementary evidence only. The original mapped-actor viewport
pixel test remains mandatory and fails independently if content leaks.
"""
import json
import sys
from pathlib import Path

from assert_news_render_pixels import parse_ppm, patch_average
from compare_news_reallocated_pixels import actor_rect, screenshot


def active_actors(path):
    tree = json.loads(Path(path).read_text(encoding="utf-8"))

    def walk(node):
        yield node
        for key in ("nodes", "floating_nodes"):
            for child in node.get(key, []):
                yield from walk(child)

    return [
        node for node in walk(tree)
        if node.get("app_id") == "post-apollo-weather-news"
        and node.get("name") == "SD1-CI-DISPOSABLE-NOT-BROWSER"
        and node.get("shell") == "xdg_shell"
    ]


def main():
    if len(sys.argv) == 3 and sys.argv[1] == "--check-absent":
        if active_actors(sys.argv[2]):
            raise SystemExit("NEWS actor still mapped")
        return
    if len(sys.argv) != 5:
        raise SystemExit(
            "usage: compare_news_unmap_pixels.py mapped.ppm unmapped.ppm "
            "mapped-tree.json unmapped-tree.json"
        )

    before_path, after_path, before_tree, after_tree = sys.argv[1:]
    if len(active_actors(before_tree)) != 1 or active_actors(after_tree):
        raise SystemExit("FAIL: missing initial actor or actor still mapped")

    before = screenshot(before_path, before_tree)
    width, height, pixels = parse_ppm(after_path)
    x, y, _, _ = actor_rect(before_tree)
    if (width, height) != before["size"]:
        raise SystemExit("FAIL: output size changed during native unmap diagnostic")
    after = {
        "inside": patch_average(pixels, width, height, x + 160, y + 160),
        "outside_right": patch_average(pixels, width, height, x + 450, y + 150),
        "outside_bottom": patch_average(pixels, width, height, x + 160, y + 370),
        "desktop": patch_average(pixels, width, height, 10, 10),
    }
    if before["inside"] < 180 or before["desktop"] > 45 or after["desktop"] > 45:
        raise SystemExit("FAIL: missing pre-unmap actor or nonblack desktop reference")

    leak_before = max(before["outside_right"], before["outside_bottom"]) > 45
    residual = max(after.values()) > 45
    result = "WHITE_PERSISTS_AFTER_UNMAP" if residual else "ACTOR_PIXELS_CLEARED_AFTER_UNMAP"
    print(
        "SD_CI_UNMAP_PIXEL "
        f"before_right={before['outside_right']:.1f} "
        f"before_bottom={before['outside_bottom']:.1f} "
        f"after_inside={after['inside']:.1f} "
        f"after_right={after['outside_right']:.1f} "
        f"after_bottom={after['outside_bottom']:.1f} "
        f"original_leak={int(leak_before)} state={result}"
    )
    if residual:
        raise SystemExit("FAIL: rendered native actor pixels persist after its verified unmap")
    print("Native actor unmap cleared sampled pixels; this does not prove viewport clipping.")


if __name__ == "__main__":
    main()
