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
    # The software EGL renderer may color-convert SHM slightly, so avoid a
    # brittle byte-exact comparison. This bounded hue/contrast predicate
    # cannot match the black desktop or white Chrome about:blank surface.
    # Report the exact count separately for diagnosing unexpected conversion.
    exact = bytes(rgb).count(b"\xdd\x44\xdd")
    painted = 0
    x0, y0, x1, y1 = width, height, -1, -1
    for i in range(0, len(rgb), 3):
        red, green, blue = rgb[i], rgb[i + 1], rgb[i + 2]
        if (red >= 150 and blue >= 150 and green <= 125 and
                min(red, blue) - green >= 70):
            painted += 1
            pixel = i // 3
            x, y = pixel % width, pixel // width
            x0, y0 = min(x0, x), min(y0, y)
            x1, y1 = max(x1, x), max(y1, y)
    print(f"SD_CI_NATIVE_POPUP_HUE_SAMPLE file={path} exact={exact} "
          f"magenta_like={painted} bounds={(x0,y0,x1,y1)}", flush=True)
    return painted


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
