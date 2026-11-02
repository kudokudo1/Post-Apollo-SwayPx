#!/usr/bin/env python3
"""CI only: require actual black compositor pixels after Chrome SIGKILL.

Not evidence of production containment, browser sandboxing or Weather NEWS.
Uses real grim P6 screenshots before Chrome, during the third mask, and
after the kernel-driven native Wayland actor unmap. Process/cgroup teardown
is separately checked by the caller.
"""
import json
import sys
from pathlib import Path

from assert_news_render_pixels import parse_ppm
from assert_real_chrome_mask_shift_pixels import match, samples


def walk(node):
    if not isinstance(node, dict):
        return
    yield node
    for field in ("nodes", "floating_nodes"):
        for child in node.get(field, []):
            yield from walk(child)


def main():
    if len(sys.argv) != 7:
        raise SystemExit("usage: abrupt_exit_pixels.py before.ppm phase3.ppm "
                         "after-kill.ppm phase3-tree.json after-tree.json pid")
    before, phase3, after = [
        parse_ppm(p) for p in sys.argv[1:4]
    ]
    if any(p[:2] != (1920, 1080) for p in (before, phase3, after)):
        raise ValueError("FAIL_ABORT_SCREEN_RESOLUTION_CHANGED")
    pid = int(sys.argv[6])
    native_id, native_rect = match(sys.argv[4], pid)
    if native_rect != (0, 0, 1920, 1080):
        raise ValueError("FAIL_ABORT_NATIVE_BROWSER_RECT_INVALID")
    tree = json.loads(Path(sys.argv[5]).read_text(encoding="utf-8"))
    if any(node.get("pid") == pid and node.get("shell") == "xdg_shell"
           for node in walk(tree)):
        raise ValueError("FAIL_ABORT_NATIVE_XDG_STILL_MAPPED")

    # Actual third mask was bright immediately before the deliberate kill.
    third = [(1100, 620), (1230, 690), (1280, 730), (1150, 750)]
    prior = [(160, 160), (240, 220),
             (600, 320), (760, 400), (880, 360), (650, 510)]
    edges = [(990, 660), (1100, 530), (1360, 660),
             (1100, 840), (10, 10)]
    before_mask = samples(phase3, third)
    after_mask = samples(after, third)
    after_previous = samples(after, prior)
    after_outside = samples(after, edges)
    baseline = samples(before, third + prior + edges)
    after_all = after_mask + after_previous + after_outside
    print("SD_CI_CHROME_ABORT_PIXELS_MEASURE "
          f"native_id={native_id} third_pre_kill={before_mask} "
          f"third_post_kill={after_mask} prior_post_kill={after_previous} "
          f"outside_post_kill={after_outside}", flush=True)
    if min(before_mask) < 170:
        raise ValueError("FAIL_ABORT_TEST_DID_NOT_HAVE_VISIBLE_BROWSER")
    if max(after_all) > 45:
        raise ValueError("FAIL_ABORT_STALE_CHROME_PIXELS_AFTER_SIGKILL")
    if max(abs(a - b) for a, b in zip(after_all, baseline)) > 30:
        raise ValueError("FAIL_ABORT_SCREEN_NOT_RESTORED_TO_BASELINE")
    print("SD_CI_CHROME_ABRUPT_EXIT_PIXELS: result=PASS "
          "native_xdg_unmapped=1 old_and_new_masks_clear=1 "
          "four_edges_clear=1 compositor_survived=1 authority=NONE")


if __name__ == "__main__":
    main()
