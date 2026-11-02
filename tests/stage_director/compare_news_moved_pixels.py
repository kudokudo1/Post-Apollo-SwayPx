#!/usr/bin/env python3
"""Classify real mapped NEWS actor pixels before and after Sway IPC move.

The native test actor must remain mapped, keep its ID, move far enough for
an old-only sample, and stay within the same screenshot resolution.
This is evidence, not render acceptance. The original fail-closed pixel
assertions run independently on both mapped snapshots.
"""
import json
import sys
from pathlib import Path

from assert_news_render_pixels import parse_ppm, patch_average


def actor(path):
    root = json.loads(Path(path).read_text(encoding="utf-8"))

    def walk(node):
        yield node
        for key in ("nodes", "floating_nodes"):
            for sub in node.get(key, []):
                yield from walk(sub)

    matches = [
        n for n in walk(root)
        if n.get("app_id") == "post-apollo-weather-news"
        and n.get("name") == "SD1-CI-DISPOSABLE-NOT-BROWSER"
        and n.get("shell") == "xdg_shell"
    ]
    if len(matches) != 1:
        raise SystemExit("FAIL: mapped move lost unique native NEWS test actor")
    n = matches[0]
    rect = n.get("rect") or {}
    dims = [rect.get(k) for k in ("x", "y", "width", "height")]
    if not isinstance(n.get("id"), int) or not all(isinstance(x, int) for x in dims):
        raise SystemExit("FAIL: mapped move actor ID or geometry invalid")
    if dims[2] < 600 or dims[3] < 440:
        raise SystemExit("FAIL: mapped move actor was unexpectedly resized")
    return n["id"], dims


def main():
    if len(sys.argv) != 5:
        raise SystemExit(
            "usage: compare_news_moved_pixels.py before.ppm after.ppm "
            "before-tree.json after-tree.json"
        )

    before_id, (x0, y0, w0, h0) = actor(sys.argv[3])
    after_id, (x1, y1, w1, h1) = actor(sys.argv[4])
    if before_id != after_id:
        raise SystemExit("FAIL: moved NEWS actor was replaced, not moved")
    dx, dy = x1 - x0, y1 - y0
    if not (200 <= dx <= 450 and 20 <= dy <= 180):
        raise SystemExit(f"FAIL: bounded mapped actor move not verified: dx={dx} dy={dy}")
    if abs(w1 - w0) > 5 or abs(h1 - h0) > 5:
        raise SystemExit("FAIL: native NEWS actor dimensions changed unexpectedly")

    bw, bh, bpx = parse_ppm(sys.argv[1])
    aw, ah, apx = parse_ppm(sys.argv[2])
    if (bw, bh) != (1920, 1080) or (aw, ah) != (bw, bh):
        raise SystemExit("FAIL: mapped move changed virtual output geometry")

    # The old-only coordinate is inside the previously white part of the
    # original actor but to the LEFT of the moved actor, with no overlap.
    old_x, old_y = x0 + 160, y0 + 160
    if old_x + 8 >= x1:
        raise SystemExit("FAIL: actor movement left no independent old-only point")

    old_before = patch_average(bpx, bw, bh, old_x, old_y)
    old_after = patch_average(apx, aw, ah, old_x, old_y)
    moved_inside = patch_average(apx, aw, ah, x1 + 160, y1 + 160)
    moved_right = patch_average(apx, aw, ah, x1 + 450, y1 + 150)
    moved_bottom = patch_average(apx, aw, ah, x1 + 160, y1 + 370)
    desktop = patch_average(apx, aw, ah, 10, 10)
    if old_before < 180 or moved_inside < 180 or desktop > 45:
        raise SystemExit("FAIL: mapped actor reference pixel or desktop not valid")

    leak_follows = max(moved_right, moved_bottom) > 45
    stale_old = old_after > 45
    state = (
        "MOVED_LEAK_WITH_OLD_RESIDUAL" if leak_follows and stale_old else
        "LEAK_FOLLOWS_MAPPED_ACTOR" if leak_follows else
        "OLD_RESIDUAL_WITHOUT_NEW_LEAK" if stale_old else
        "NO_LEAK_AT_MOVED_SAMPLES"
    )
    print(
        "SD_CI_MOVED_PIXEL "
        f"actor_id={before_id} dx={dx} dy={dy} "
        f"old_before={old_before:.1f} old_after={old_after:.1f} "
        f"moved_inside={moved_inside:.1f} moved_right={moved_right:.1f} "
        f"moved_bottom={moved_bottom:.1f} desktop={desktop:.1f} "
        f"state={state}"
    )
    print("Move classification alone cannot prove current-frame paint nor viewport containment.")


if __name__ == "__main__":
    main()
