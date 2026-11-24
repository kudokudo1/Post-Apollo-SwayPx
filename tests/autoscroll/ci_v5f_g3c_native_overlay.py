#!/usr/bin/env python3
"""G3C: C1+C2 REAL native owner overlays in ONE copied SwayPX compositor.

Everything here runs ONLY inside a disposable GitHub CI runner. Production
source, Meson and the operator's desktop never receive test probes. NO
physical credentials, positive input authorization, or Wayland axis.
"""
from __future__ import annotations
import os
from pathlib import Path
import subprocess
import ci_v5f_c1_native_overlay as c1
import ci_v5f_c2_native_overlay as c2

INPUT = "sway/input/input-manager.c"
SEAT = "sway/input/seat.c"
TARGET = "sway/tree/container.c"
OUTPUT = "sway/tree/output.c"
MESON = "sway/meson.build"

def one(src: str, old: str, new: str, why: str) -> str:
    if src.count(old) != 1 or new in src:
        raise RuntimeError("PA_G3C_ANCHOR_DRIFT " + why)
    return src.replace(old, new, 1)

def device_patch(src: str) -> str:
    src = one(src,
        '#include "sway/input/input-manager.h"\n',
        '#include "sway/input/input-manager.h"\n'
        'void pa_g3c_virtual_pointer_bound(struct sway_input_device *device,\n'
        '        struct sway_seat *seat);\n'
        'void pa_g3c_virtual_pointer_revoked(struct sway_input_device *device);\n',
        "virtual_device_probe_declarations")
    src = one(src,
        '\tseat_add_device(seat, input_device);\n\n'
        '\tif (event->suggested_output) {',
        '\tseat_add_device(seat, input_device);\n'
        '\tpa_g3c_virtual_pointer_bound(input_device, seat);\n\n'
        '\tif (event->suggested_output) {',
        "virtual_binding_after_seat_add")
    src = one(src,
        '\t\t\tinput_manager_autoscroll_pointer_lifetime_epoch(input_device));\n'
        '\t}\n\n\tstruct sway_seat *seat = NULL;',
        '\t\t\tinput_manager_autoscroll_pointer_lifetime_epoch(input_device));\n'
        '\t}\n'
        '\tpa_g3c_virtual_pointer_revoked(input_device);\n\n'
        '\tstruct sway_seat *seat = NULL;',
        "device_revoke_after_native_owner")
    return src

def seat_patch(src: str) -> str:
    src = one(src,
        '#include "sway/input/seat.h"\n',
        '#include "sway/input/seat.h"\n'
        'void pa_g3c_transient_seat_revoked(struct sway_seat *seat);\n',
        "transient_seat_probe_decl")
    src = one(src,
        '\t\t\tpa_fc2_before, seat_autoscroll_lifetime_epoch(seat));\n'
        '\t}\n\n\tif (seat == config->handler_context.seat) {',
        '\t\t\tpa_fc2_before, seat_autoscroll_lifetime_epoch(seat));\n'
        '\t}\n'
        '\tpa_g3c_transient_seat_revoked(seat);\n\n'
        '\tif (seat == config->handler_context.seat) {',
        "seat_revoke_after_native_owner")
    return src

def target_patch(src: str) -> str:
    src = one(src,
        '#include "sway/tree/view.h"\n',
        '#include "sway/tree/view.h"\n'
        'void pa_g3c_target_bound(struct sway_container *target);\n'
        'void pa_g3c_target_revoked(struct sway_container *target);\n',
        "mapped_target_probe_decls")
    src = one(src,
        '\twl_signal_emit_mutable(&root->events.new_node, &c->node);',
        '\tpa_g3c_target_bound(c);\n'
        '\twl_signal_emit_mutable(&root->events.new_node, &c->node);',
        "target_after_real_bind")
    src = one(src,
        '\tif (con->view) {\n\t\tipc_event_window(con, "close");',
        '\tpa_g3c_target_revoked(con);\n'
        '\tif (con->view) {\n\t\tipc_event_window(con, "close");',
        "target_after_real_revocation")
    return src

def output_patch(src: str) -> str:
    src = one(src,
        '#include "sway/output.h"\n',
        '#include "sway/output.h"\n'
        'void pa_g3c_output_changed(struct sway_output *output, int reason);\n',
        "output_probe_decl")
    src = one(src,
        '\n}\n\nvoid output_enable(struct sway_output *output)',
        '\n\tpa_g3c_output_changed(output, 1);\n'
        '}\n\nvoid output_enable(struct sway_output *output)',
        "output_after_config_token_rotation")
    src = one(src,
        '\n\t// Remove the output now to avoid interacting with it during e.g.,',
        '\n\tpa_g3c_output_changed(output, 2);\n'
        '\n\t// Remove the output now to avoid interacting with it during e.g.,',
        "output_after_disable_token_revoke")
    return src

def meson_patch(src: str) -> str:
    return one(src,
        "\t'input/post_apollo_autoscroll_runtime.c',\n",
        "\t'input/post_apollo_autoscroll_runtime.c',\n"
        "\t'input/post_apollo_autoscroll_continuity.c',\n"
        "\t'../tests/autoscroll/ci_v5f_g3c_native_probe.c',\n",
        "disposable_only_policy_probe_linkage")

def main() -> None:
    e = os.environ
    tmp = Path(e.get("RUNNER_TEMP", "/missing"))
    ws = Path(e.get("GITHUB_WORKSPACE", "/missing"))
    if not (e.get("CI") == "true" and e.get("GITHUB_ACTIONS") == "true"
            and e.get("PA_AUTOSCROLL_CI_NATIVE_TRACE") == "1"
            and e.get("PA_G3C_NATIVE_FOUR_OWNER") == "1"
            and str(tmp) == "/home/runner/work/_temp"
            and tmp.is_dir() and ws.is_dir() and os.getuid() != 0):
        raise RuntimeError("BLOCKED_PA_G3C_NON_DISPOSABLE_HOST")
    c1.main()  # tested native target+output copy; NEVER edits original
    dst = tmp / "pa-fc1-native-src"
    for path, parent, patch in (
            (INPUT, c2.device_patch, device_patch),
            (SEAT, c2.seat_patch, seat_patch),
            (TARGET, lambda s: s, target_patch),
            (OUTPUT, lambda s: s, output_patch),
            (MESON, lambda s: s, meson_patch)):
        full = dst / path
        full.write_text(patch(parent(full.read_text(encoding="utf-8"))),
                        encoding="utf-8")
    changed = set(subprocess.check_output([
        "git", "-C", str(dst), "diff", "--name-only"
    ], text=True).splitlines())
    if changed != {INPUT, SEAT, TARGET, OUTPUT, MESON}:
        raise RuntimeError("PA_G3C_DISPOSABLE_OVERLAY_FOREIGN_PATHS "
                           + repr(changed))
    if subprocess.check_output(
            ["git", "-C", str(ws), "status", "--porcelain"], text=True):
        raise RuntimeError("PA_G3C_ORIGINAL_CHECKOUT_MUTATED")
    print("PA_G3C_FIVE_PATH_NATIVE_CI_OVERLAY_PASS "
          "owners=DEVICE_SEAT_TARGET_OUTPUT original_checkout=UNCHANGED "
          "production_linkage=NONE", flush=True)

if __name__ == "__main__":
    main()
