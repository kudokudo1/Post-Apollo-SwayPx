#!/usr/bin/env python3
"""CI only: validate real Chrome across THREE screenshot masks, two moves.

The compositor's native XDG ID, PID, border and full 1920x1080 rectangle
stay the same. Actual screenshots must prove both preceding crops cleared,
and the third, smaller crop is visible only inside its own four edges.
This does not authenticate a production Weather NEWS viewport.
"""
import sys
from assert_news_render_pixels import parse_ppm
from assert_real_chrome_mask_shift_pixels import match, samples


def main() -> None:
    if len(sys.argv) != 9:
        raise SystemExit(
            "usage: multi_mask.py before.ppm initial.ppm phase2.ppm "
            "phase3.ppm initial-tree.json phase2-tree.json "
            "phase3-tree.json native-chrome-pid")
    before, initial, phase2, phase3 = [
        parse_ppm(path) for path in sys.argv[1:5]
    ]
    if any(p[:2] != (1920, 1080) for p in (before, initial, phase2, phase3)):
        raise ValueError("FAIL_OUTPUT_RESOLUTION_CHANGED")
    pid = int(sys.argv[8])
    native = [match(path, pid) for path in sys.argv[5:8]]
    if not (native[0] == native[1] == native[2] and
            native[0][1] == (0, 0, 1920, 1080)):
        raise ValueError(f"FAIL_NATIVE_CHROME_WINDOW_MOVED_OR_REPLACED {native}")

    phase1_points = [(160, 160), (240, 220)]
    phase2_points = [(600, 320), (760, 400), (880, 360), (650, 510)]
    # Last crop: x=1040..1320 and y=560..780, 280x220, inside original XDG.
    phase3_points = [(1100, 620), (1230, 690), (1280, 730), (1150, 750)]
    third_outside = [
        (990, 660), (1100, 530), (1360, 660), (1100, 840), (10, 10)
    ]
    initial_inside = samples(initial, phase1_points)
    second_inside = samples(phase2, phase2_points)
    third_inside = samples(phase3, phase3_points)
    first_after = samples(phase3, phase1_points)
    second_after = samples(phase3, phase2_points)
    outside_after = samples(phase3, third_outside)
    outside_before = samples(before, third_outside)
    prior_third = samples(phase2, phase3_points)
    before_third = samples(before, phase3_points)

    print("SD_CI_CHROME_MULTI_MASK_PIXELS_MEASURE "
          f"native={native[0]} first={initial_inside} second={second_inside} "
          f"third={third_inside} stale1={first_after} stale2={second_after} "
          f"outside={outside_after}", flush=True)
    if min(initial_inside) < 180 or min(second_inside) < 170:
        raise ValueError("FAIL_PREVIOUS_CHROME_MASKS_NOT_RENDERED")
    if max(first_after + second_after) > 45:
        raise ValueError("FAIL_STALE_CHROME_PIXELS_PREVIOUS_MASKS")
    if min(third_inside) < 170 or max(prior_third) > 45 or max(before_third) > 45:
        raise ValueError("FAIL_THIRD_CHROME_MASK_NOT_NEWLY_VISIBLE")
    if max(outside_after) > 45:
        raise ValueError("FAIL_THIRD_CHROME_MASK_FOUR_EDGE_LEAK")
    if max(abs(a-b) for a, b in zip(outside_after, outside_before)) > 30:
        raise ValueError("FAIL_OUTSIDE_BACKGROUND_CHANGED")
    if min(third_inside) - max(outside_after) < 120:
        raise ValueError("FAIL_THIRD_CHROME_MASK_CONTRAST_INSUFFICIENT")
    print("SD_CI_CHROME_MULTI_MASK_PIXELS: result=PASS "
          "three_screenshots=1 two_mask_moves=1 native_xdg_stationary=1 "
          "previous_pixels_cleared=1 third_mask_280x220=1 "
          "four_edges_clear=1 authority=NONE")


if __name__ == "__main__":
    main()
