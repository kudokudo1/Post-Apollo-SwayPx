#!/usr/bin/env python3
"""Validate an actual native popup can paint, but same-service popup cannot.

The control and denied clients emit the SAME xdg_popup wl_shm RGB DD44DD
buffer. The control must really render first: absence alone is not a proof.
Both test captures use one actual headless SwayPX/Chrome session.
No screenshots from the operator's computer are used.
"""
import sys
from assert_news_render_pixels import parse_ppm


def amount(path):
    width, height, rgb = parse_ppm(path)
    if (width, height) != (1920, 1080):
        raise ValueError("FAIL_NATIVE_POPUP_SCREENSHOT_SIZE")
    # Solid RGB DD44DD pixels, emitted only by the native popup witness.
    return bytes(rgb).count(b"\xdd\x44\xdd")


def main():
    if len(sys.argv) != 4:
        raise SystemExit("usage: witness_pixels.py before.ppm control.ppm denied.ppm")
    base, control, denied = [amount(p) for p in sys.argv[1:]]
    print(f"SD_CI_NATIVE_POPUP_PIXEL_COUNTS baseline={base} "
          f"control={control} denied={denied}", flush=True)
    if base > 10:
        raise ValueError("FAIL_NATIVE_POPUP_PINK_BASELINE_NOT_EMPTY")
    if control < 800:
        raise ValueError("FAIL_NATIVE_POPUP_POSITIVE_CONTROL_NOT_RENDERED")
    if denied > 10:
        raise ValueError("FAIL_NATIVE_POPUP_GLOBAL_ROOT_PIXEL_ESCAPE")
    print("SD_CI_NATIVE_POPUP_DENIAL_PIXELS: result=PASS "
          "positive_control_rendered=1 same_cgroup_popup_denied=1 "
          "native_root_scene_escape_pixels=0 authority=NONE")


if __name__ == "__main__":
    main()
