#!/usr/bin/env python3
"""G3H CI-only bridge from real cursor callback to inert G3F OS diagnostic.

Extends G3E's disposable copy, preserving its native ingress and normal
seatop dispatch. The production checkout stays unchanged.
"""
from __future__ import annotations
import os
from pathlib import Path
import subprocess
import ci_v5f_g3e_native_overlay as g3e
import ci_v5f_g2_native_button as g2

CURSOR = "sway/input/cursor.c"
MESON = "sway/meson.build"
DEVICE = "sway/input/input-manager.c"
SEAT = "sway/input/seat.c"

def one(src: str, needle: str, replacement: str, label: str) -> str:
    if src.count(needle) != 1 or replacement in src:
        raise RuntimeError("PA_G3H_EXACT_NATIVE_ANCHOR_DRIFT " + label)
    return src.replace(needle, replacement, 1)

def patch_cursor(src: str) -> str:
    decl = ('void pa_g3e_native_middle_negative_observe(\n'
            '        struct sway_cursor *cursor,\n'
            '        struct wlr_pointer_button_event *event,\n'
            '        bool weather_paired, bool company_paired);\n')
    new_decl = decl + (
        'void pa_g3h_native_os_managed_diagnostic_observe(\n'
        '        struct sway_cursor *cursor,\n'
        '        struct wlr_pointer_button_event *event,\n'
        '        bool weather_paired, bool company_paired);\n')
    src = one(src, decl, new_decl, "G3H_DECLARATION")
    call = ('\tpa_g3e_native_middle_negative_observe(cursor, event,\n'
            '\t\tweather_paired, company_paired);\n')
    src = one(src, call, call + (
        '\t/* G3H: native callback diagnostic, not an input consumer. */\n'
        '\tpa_g3h_native_os_managed_diagnostic_observe(cursor, event,\n'
        '\t\tweather_paired, company_paired);\n'), "G3H_EVENT_OBSERVER")
    return src

def patch_meson(src: str) -> str:
    line = "\t'../tests/autoscroll/ci_v5f_g3e_native_probe.c',\n"
    return one(src, line, line + (
        "\t'../tests/autoscroll/ci_v5f_g3h_native_os_observer.c',\n"
        "\t'../tests/autoscroll/ci_v5f_g3f_trust_policy.c',\n"
    ), "G3H_TEST_ONLY_MESON")

def main() -> None:
    tmp, ws = g2.guard_ci()
    if os.environ.get("PA_G3H_NATIVE_OS_DIAG") != "1":
        raise RuntimeError("BLOCKED_PA_G3H_TEST_FLAG_ABSENT")
    g3e.main()  # G2 and C2 owner lifecycle + G3E real callback
    copy = tmp / g2.COPY_NAME
    for path, patch in ((CURSOR, patch_cursor), (MESON, patch_meson)):
        target = copy / path
        target.write_text(patch(target.read_text(encoding="utf-8")),
                          encoding="utf-8")
    paths = set(subprocess.check_output([
        "git", "-C", str(copy), "diff", "--name-only"
    ], text=True).splitlines())
    if paths != {CURSOR, MESON, DEVICE, SEAT}:
        raise RuntimeError("PA_G3H_DISPOSABLE_COPY_SOURCE_DRIFT "
                           + repr(paths))
    if subprocess.check_output([
        "git", "-C", str(ws), "status", "--porcelain"
    ], text=True):
        raise RuntimeError("PA_G3H_ORIGINAL_GIT_CHECKOUT_CHANGED")
    print("PA_G3H_NATIVE_OS_DIAG_OVERLAY_PASS "
          "real_callback=1 G3F_model=TEST_ONLY "
          "normal_input_dispatch=PRESERVED production=HARD_OFF", flush=True)

if __name__ == "__main__":
    main()
