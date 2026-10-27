#!/usr/bin/env python3
"""Compare the same mapped actor across a headless background layer refresh.

The native actor is never unmapped, moved or resized in this experiment.
Changing the background does not itself guarantee a complete output repaint.
The original viewport pixel acceptance remains an independent hard gate.
"""
import sys

from compare_news_moved_pixels import actor
from compare_news_opacity_pixels import sample


def main():
    if len(sys.argv) != 5:
        raise SystemExit(
            "usage: compare_news_background_pixels.py "
            "before.ppm after.ppm before-tree.json after-tree.json"
        )
    first_id, first_rect = actor(sys.argv[3])
    second_id, second_rect = actor(sys.argv[4])
    if first_id != second_id or first_rect != second_rect:
        raise SystemExit("FAIL: background refresh moved or replaced mapped actor")
    x, y, _, _ = first_rect
    before = sample(sys.argv[1], x, y)
    after = sample(sys.argv[2], x, y)
    if max(before["desktop"], after["desktop"]) > 45:
        raise SystemExit("FAIL: expected dark background was not restored")
    if min(before["inside"], after["inside"]) < 180:
        raise SystemExit("FAIL: mapped native actor disappeared during background refresh")

    leaked_before = max(before["right"], before["bottom"]) > 45
    leaked_after = max(after["right"], after["bottom"]) > 45
    state = (
        "LEAK_PERSISTS_AFTER_BACKGROUND_REFRESH" if leaked_after
        else "LEAK_CLEARED_AFTER_BACKGROUND_REFRESH" if leaked_before
        else "NO_LEAK_IN_EITHER_FRAME"
    )
    print(
        "SD_CI_BACKGROUND_PIXEL "
        f"actor_id={first_id} "
        f"inside_before={before['inside']:.1f} inside_after={after['inside']:.1f} "
        f"right_before={before['right']:.1f} right_after={after['right']:.1f} "
        f"bottom_before={before['bottom']:.1f} bottom_after={after['bottom']:.1f} "
        f"desktop_before={before['desktop']:.1f} desktop_after={after['desktop']:.1f} "
        f"state={state}"
    )
    print(
        "Background replacement can trigger damage but cannot certify full-output "
        "repaint, current-frame render source, or NEWS viewport containment."
    )


if __name__ == "__main__":
    main()
