#!/usr/bin/env python3
"""G3E disposable C2 + G2 native ingress overlay with G3E deny-only probe.

Never changes ordinary source, never links probe to production. Only a
GitHub-hosted disposable SwayPX copy gets the logging hooks.
"""
from __future__ import annotations
import os
from pathlib import Path
import subprocess
import ci_v5f_g2_native_button as g2

CURSOR = "sway/input/cursor.c"
MESON = "sway/meson.build"
DEVICE = "sway/input/input-manager.c"
SEAT = "sway/input/seat.c"

def one(src: str, old: str, new: str, label: str) -> str:
    if src.count(old) != 1 or new in src:
        raise RuntimeError("PA_G3E_CALLBACK_ANCHOR_DRIFT "+label)
    return src.replace(old,new,1)

def cursor_patch(src: str) -> str:
    src = one(src,
        '#include "sway/input/cursor.h"\n',
        '#include "sway/input/cursor.h"\n'
        'void pa_g3e_native_middle_negative_observe(\n'
        '        struct sway_cursor *cursor,\n'
        '        struct wlr_pointer_button_event *event,\n'
        '        bool weather_paired, bool company_paired);\n',
        "probe_decl")
    src = one(src,
        '\tif (weather_paired || company_paired) {\n\t\treturn;\n'
        '\t}\n\t/* V5E-B shared cursor seam',
        '\t/* Diagnostic only, AFTER pairing decisions and BEFORE normal\n'
        '\t * button dispatch. Probe never modifies event or consumes it. */\n'
        '\tpa_g3e_native_middle_negative_observe(cursor, event,\n'
        '\t\tweather_paired, company_paired);\n'
        '\tif (weather_paired || company_paired) {\n\t\treturn;\n'
        '\t}\n\t/* V5E-B shared cursor seam',
        "first_refusal_preserved")
    return src

def meson_patch(src: str) -> str:
    return one(src,
        "\t'input/post_apollo_autoscroll_runtime.c',\n",
        "\t'input/post_apollo_autoscroll_runtime.c',\n"
        "\t'../tests/autoscroll/ci_v5f_g3e_native_probe.c',\n",
        "test_only_probe")

def main() -> None:
    temp, ws = g2.guard_ci()
    if os.environ.get("PA_G3E_REAL_MIDDLE_NEGATIVE") != "1":
        raise RuntimeError("BLOCKED_PA_G3E_NOT_EXPLICIT_CI")
    g2.overlay() # invokes C2 real transient seat/device hooks in CI copy
    dst = temp / g2.COPY_NAME
    targets=((CURSOR,cursor_patch),(MESON,meson_patch))
    for path,fn in targets:
        full=dst/path
        full.write_text(fn(full.read_text(encoding="utf-8")),
                        encoding="utf-8")
    changed=set(subprocess.check_output(
        ["git","-C",str(dst),"diff","--name-only"],text=True).splitlines())
    if changed != {CURSOR,MESON,DEVICE,SEAT}:
        raise RuntimeError("PA_G3E_DISPOSABLE_COPY_FOREIGN_CHANGES "
                           +repr(changed))
    if subprocess.check_output(
            ["git","-C",str(ws),"status","--porcelain"],text=True):
        raise RuntimeError("PA_G3E_ORIGINAL_CHECKOUT_MUTATED")
    print("PA_G3E_REAL_NATIVE_INGRESS_OVERLAY_PASS "
          "native_device_and_seat=REAL virtual_hardware_trust=DENIED "
          "production_checkout=UNCHANGED normal_build_linkage=NONE",
          flush=True)

if __name__=="__main__":
    main()
