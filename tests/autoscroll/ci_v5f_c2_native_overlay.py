#!/usr/bin/env python3
"""V5F-C2: test-only instrumentation of genuine SwayPX owner callbacks.

Patch ONLY an isolated, GitHub-runner copy. The normal checkout and desktop
remain unchanged. Virtual protocol pointers are NOT physical provenance.
"""
from __future__ import annotations
import os
from pathlib import Path
import shutil
import subprocess

DEVICE = "sway/input/input-manager.c"
SEAT = "sway/input/seat.c"

def one(src: str, anchor: str, replacement: str, reason: str) -> str:
    if src.count(anchor) != 1 or replacement in src:
        raise RuntimeError("PA_FC2_OVERLAY_ANCHOR_DRIFT " + reason)
    return src.replace(anchor, replacement, 1)

def device_patch(src: str) -> str:
    src = one(src,
        '#include "sway/input/input-manager.h"\n',
        '#include "sway/input/input-manager.h"\n#include <inttypes.h>\n',
        "device_include")
    # Register an actual protocol-origin virtual-pointer callback.
    # It owns a lifetime slot but must remain ineligible as physical input.
    src = one(src,
        '\tpa_autoscroll_bind_pointer_lifetime(input_manager, input_device);\n'
        '\twl_list_insert(&input_manager->devices, &input_device->link);',
        '\tpa_autoscroll_bind_pointer_lifetime(input_manager, input_device);\n'
        '\tsway_log(SWAY_INFO, "PA_FC2_NATIVE DEVICE_BIND epoch=%" PRIu64 '
        '" eligible=%" PRIu64 " virtual=%d",\n'
        '\t\tpa_autoscroll_lifetime_epoch(\n'
        '\t\t\t&input_device->autoscroll_lifetime, PA_LIFETIME_DEVICE),\n'
        '\t\tinput_manager_autoscroll_pointer_lifetime_epoch(input_device),\n'
        '\t\tinput_device->is_virtual);\n'
        '\twl_list_insert(&input_manager->devices, &input_device->link);',
        "real_virtual_pointer_callback")
    src = one(src,
        '\tpa_autoscroll_lifetime_revoke(&input_device->autoscroll_lifetime);\n'
        '\n\tstruct sway_seat *seat = NULL;',
        '\tconst uint64_t pa_fc2_before = pa_autoscroll_lifetime_epoch(\n'
        '\t\t&input_device->autoscroll_lifetime, PA_LIFETIME_DEVICE);\n'
        '\tpa_autoscroll_lifetime_revoke(&input_device->autoscroll_lifetime);\n'
        '\tif (input_device->is_virtual) {\n'
        '\t\tsway_log(SWAY_INFO, "PA_FC2_NATIVE DEVICE_REVOKE before=%" PRIu64 '
        '" after=%" PRIu64 " eligible=%" PRIu64,\n'
        '\t\t\tpa_fc2_before,\n'
        '\t\t\tpa_autoscroll_lifetime_epoch(\n'
        '\t\t\t\t&input_device->autoscroll_lifetime, PA_LIFETIME_DEVICE),\n'
        '\t\t\tinput_manager_autoscroll_pointer_lifetime_epoch(input_device));\n'
        '\t}\n'
        '\n\tstruct sway_seat *seat = NULL;',
        "device_destroy_before_detach")
    return src

def seat_patch(src: str) -> str:
    src = one(src,
        '#include "sway/input/seat.h"\n',
        '#include "sway/input/seat.h"\n#include <inttypes.h>\n',
        "seat_include")
    # This callback executes from a protocol request, not a C test fake.
    src = one(src,
        '\tseat->destroy.notify = handle_seat_destroy;\n',
        '\tif (strncmp(seat_name, "transient-", 10) == 0) {\n'
        '\t\tsway_log(SWAY_INFO, "PA_FC2_NATIVE SEAT_BIND epoch=%" PRIu64,\n'
        '\t\t\tseat_autoscroll_lifetime_epoch(seat));\n'
        '\t}\n'
        '\tseat->destroy.notify = handle_seat_destroy;\n',
        "real_transient_seat_bind")
    src = one(src,
        '\tpa_autoscroll_lifetime_revoke(&seat->autoscroll_lifetime);\n'
        '\n\tif (seat == config->handler_context.seat) {',
        '\tconst uint64_t pa_fc2_before = seat_autoscroll_lifetime_epoch(seat);\n'
        '\tpa_autoscroll_lifetime_revoke(&seat->autoscroll_lifetime);\n'
        '\tif (strncmp(seat->wlr_seat->name, "transient-", 10) == 0) {\n'
        '\t\tsway_log(SWAY_INFO, "PA_FC2_NATIVE SEAT_REVOKE before=%" PRIu64 '
        '" after=%" PRIu64,\n'
        '\t\t\tpa_fc2_before, seat_autoscroll_lifetime_epoch(seat));\n'
        '\t}\n'
        '\n\tif (seat == config->handler_context.seat) {',
        "seat_destroy_before_focus_and_free")
    return src

def main() -> None:
    e = os.environ
    tmp = Path(e.get("RUNNER_TEMP", "/missing"))
    ws = Path(e.get("GITHUB_WORKSPACE", "/missing")).resolve()
    if not (e.get("GITHUB_ACTIONS") == "true" and e.get("CI") == "true"
            and e.get("PA_AUTOSCROLL_CI_NATIVE_TRACE") == "2"
            and str(tmp) == "/home/runner/work/_temp" and tmp.is_dir()
            and ws.is_dir() and os.getuid() != 0):
        raise RuntimeError("BLOCKED_PA_FC2_NON_DISPOSABLE_RUNNER")
    if subprocess.check_output(
            ["git","-C",str(ws),"status","--porcelain"], text=True):
        raise RuntimeError("PA_FC2_ORIGINAL_CHECKOUT_NOT_CLEAN")
    dst = tmp / "pa-fc2-native-src"
    if dst.exists():
        raise RuntimeError("PA_FC2_DISPOSABLE_COPY_ALREADY_EXISTS")
    shutil.copytree(ws, dst, symlinks=False,
                    ignore=shutil.ignore_patterns("__pycache__", "result"))
    for path, fn in ((DEVICE, device_patch),(SEAT, seat_patch)):
        full = dst / path
        full.write_text(fn(full.read_text(encoding="utf-8")),
                        encoding="utf-8")
    diff = subprocess.check_output(
        ["git","-C",str(dst),"diff","--name-only"],text=True).splitlines()
    if diff != sorted((DEVICE,SEAT)):
        raise RuntimeError("PA_FC2_FOREIGN_DIFF " + repr(diff))
    if subprocess.check_output(
            ["git","-C",str(ws),"status","--porcelain"],text=True):
        raise RuntimeError("PA_FC2_ORIGINAL_CHECKOUT_MUTATED")
    print("PA_FC2_DISPOSABLE_OWNER_CALLBACK_OVERLAY "
          "virtual_device=REAL transient_seat=REAL "
          "original_checkout=UNMODIFIED",flush=True)

if __name__ == "__main__":
    main()
