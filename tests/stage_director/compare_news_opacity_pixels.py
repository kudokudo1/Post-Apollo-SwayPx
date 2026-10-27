#!/usr/bin/env python3
"""Compare genuine compositor screenshots at full, zero and restored alpha.

Classifies whether white actor-region pixels respond to the existing SwayPX
opacity command while the SAME XDG actor remains mapped and stationary.
Never substitutes for the fail-closed mapped-actor pixel acceptance test.
"""
import sys

from assert_news_render_pixels import parse_ppm, patch_average
from compare_news_moved_pixels import actor


def sample(path, x, y):
    w, h, pixels = parse_ppm(path)
    if (w, h) != (1920, 1080):
        raise SystemExit("FAIL: opacity experiment changed fixed CI output size")
    return {
        "inside": patch_average(pixels, w, h, x + 160, y + 160),
        "right": patch_average(pixels, w, h, x + 450, y + 150),
        "bottom": patch_average(pixels, w, h, x + 160, y + 370),
        "desktop": patch_average(pixels, w, h, 10, 10),
    }


def main():
    if len(sys.argv) != 7:
        raise SystemExit(
            "usage: compare_news_opacity_pixels.py "
            "full.ppm zero.ppm restored.ppm "
            "full-tree.json zero-tree.json restored-tree.json"
        )
    trees = [actor(path) for path in sys.argv[4:]]
    ids = [p[0] for p in trees]
    rects = [p[1] for p in trees]
    if len(set(ids)) != 1 or any(rect != rects[0] for rect in rects):
        raise SystemExit("FAIL: opacity toggle did not preserve same mapped actor and geometry")
    x, y, _, _ = rects[0]
    before, zero, after = [sample(path, x, y) for path in sys.argv[1:4]]
    if any(frame["desktop"] > 45 for frame in (before, zero, after)):
        raise SystemExit("FAIL: expected reference desktop to remain black")
    if before["inside"] < 180 or after["inside"] < 180:
        raise SystemExit("FAIL: native actor failed to return on opacity restoration")

    cleared = zero["inside"] < 45 and zero["right"] < 45 and zero["bottom"] < 45
    state = (
        "ACTOR_PIXELS_CLEAR_ON_ALPHA_ZERO" if cleared
        else "ACTOR_PIXELS_PERSIST_AT_ALPHA_ZERO"
    )
    print(
        "SD_CI_OPACITY_PIXEL "
        f"actor_id={ids[0]} "
        f"full_inside={before['inside']:.1f} zero_inside={zero['inside']:.1f} "
        f"restored_inside={after['inside']:.1f} "
        f"full_right={before['right']:.1f} zero_right={zero['right']:.1f} "
        f"restored_right={after['right']:.1f} "
        f"full_bottom={before['bottom']:.1f} zero_bottom={zero['bottom']:.1f} "
        f"restored_bottom={after['bottom']:.1f} state={state}"
    )
    print("Opacity response alone does not certify current-frame paint or NEWS viewport clipping.")


if __name__ == "__main__":
    main()
