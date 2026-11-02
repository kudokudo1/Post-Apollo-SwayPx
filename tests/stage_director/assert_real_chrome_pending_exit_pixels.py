#!/usr/bin/env python3
"""CI-only pending-request abort: actual Chrome pixels and compositor cleanup.

A real Chrome XDG starts with the 320x240 initial crop. An acknowledged
PENDING request must leave that crop unchanged. After SIGKILL, all old
and proposed crop positions must match the original black desktop.
Never interpret this as testing a mutation already in progress.
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
        raise SystemExit("usage: pending_pixels.py before.ppm held.ppm "
                         "after.ppm held-tree.json after-tree.json pid")
    baseline, held, after = [parse_ppm(p) for p in sys.argv[1:4]]
    if any(p[:2] != (1920, 1080) for p in (baseline, held, after)):
        raise ValueError("FAIL_PENDING_OUTPUT_RESOLUTION")
    pid = int(sys.argv[6])
    _, rect = match(sys.argv[4], pid)
    if rect != (0, 0, 1920, 1080):
        raise ValueError("FAIL_PENDING_BROWSER_OUTER_WINDOW_MOVED")
    tree = json.loads(Path(sys.argv[5]).read_text(encoding="utf-8"))
    if any(node.get("pid") == pid and node.get("shell") == "xdg_shell"
           for node in walk(tree)):
        raise ValueError("FAIL_PENDING_NATIVE_ACTOR_STILL_MAPPED")

    initial_points = [(160, 160), (240, 220)]
    proposed_points = [(600, 320), (760, 400), (880, 360), (650, 510)]
    later_points = [(1100, 620), (1230, 690), (1280, 730), (1150, 750)]
    outside_points = [(24, 160), (160, 24), (450, 150), (160, 370),
                      (990, 660), (1100, 840), (10, 10)]
    all_points = initial_points + proposed_points + later_points + outside_points
    before_held = samples(held, initial_points)
    proposed_held = samples(held, proposed_points + later_points)
    killed = samples(after, all_points)
    initial_baseline = samples(baseline, all_points)
    print("SD_CI_CHROME_PENDING_PIXELS_MEASURE "
          f"initial_held={before_held} proposed_unapplied={proposed_held} "
          f"after_sigkill={killed}", flush=True)
    if min(before_held) < 180:
        raise ValueError("FAIL_PENDING_INITIAL_CROP_NOT_VISIBLE")
    if max(proposed_held) > 45:
        raise ValueError("FAIL_PENDING_REQUEST_APPLIED_PREMATURELY")
    if max(killed) > 45:
        raise ValueError("FAIL_PENDING_STALE_CHROME_PIXELS_AFTER_SIGKILL")
    if max(abs(a - b) for a, b in zip(killed, initial_baseline)) > 30:
        raise ValueError("FAIL_PENDING_BACKGROUND_NOT_RESTORED")
    print("SD_CI_CHROME_PENDING_EXIT_PIXELS: result=PASS "
          "pending_unapplied=1 initial_clip_held=1 native_actor_unmapped=1 "
          "prior_and_future_crops_clear=1 output_black=1 authority=NONE")


if __name__ == "__main__":
    main()
