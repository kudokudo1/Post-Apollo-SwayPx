#!/usr/bin/env python3
"""Classify actual NEWS overflow across two independently booted CI compositors.

A cold allocation comparison is only evidence, not pixel acceptance.
The caller separately executes the original fail-closed screenshot test on
BOTH snapshots, regardless of the classification printed here.
"""
import sys
from compare_news_reallocated_pixels import screenshot


def main():
    if len(sys.argv) != 5:
        raise SystemExit(
            "usage: compare_news_cold_pixels.py first.ppm second.ppm first-tree.json second-tree.json"
        )
    first = screenshot(sys.argv[1], sys.argv[3])
    fresh = screenshot(sys.argv[2], sys.argv[4])
    if first["size"] != (1920, 1080) or fresh["size"] != (1888, 1056):
        raise SystemExit(
            "FAIL: independent virtual output sizes not proven: "
            f"first={first['size']} fresh={fresh['size']}"
        )
    if first["inside"] < 180 or fresh["inside"] < 180:
        raise SystemExit("FAIL: native NEWS test surface missing in a fresh compositor")
    if first["desktop"] > 45 or fresh["desktop"] > 45:
        raise SystemExit("FAIL: independent compositor black desktop not verified")
    original_leak = max(first["outside_right"], first["outside_bottom"]) > 45
    fresh_leak = max(fresh["outside_right"], fresh["outside_bottom"]) > 45
    state = (
        "BOTH_COLD_OUTPUTS_LEAK" if original_leak and fresh_leak
        else "INITIAL_COLD_OUTPUT_ONLY" if original_leak
        else "FRESH_COLD_OUTPUT_ONLY" if fresh_leak
        else "NO_LEAK_SAMPLED"
    )
    print(
        "SD_CI_COLD_PIXEL "
        f"first={first['size'][0]}x{first['size'][1]} "
        f"fresh={fresh['size'][0]}x{fresh['size'][1]} "
        f"first_right={first['outside_right']:.1f} fresh_right={fresh['outside_right']:.1f} "
        f"first_bottom={first['outside_bottom']:.1f} fresh_bottom={fresh['outside_bottom']:.1f} "
        f"state={state}"
    )
    print(
        "Independent compositor starts do not establish stale-versus-live paint "
        "within either frame; final pixel acceptance remains a separate requirement."
    )


if __name__ == "__main__":
    main()
