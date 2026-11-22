#!/usr/bin/env python3
"""V5F-G3B test-only overlay: C1 native owners + detached G3A callback probe.

CI copy ONLY. Source checkout, production Meson and user desktop stay intact.
No native input producer, no axis dispatch and no physical-origin authority.
"""
from __future__ import annotations
import os
from pathlib import Path
import subprocess
import ci_v5f_c1_native_overlay as c1

TARGET = "sway/tree/container.c"
OUTPUT = "sway/tree/output.c"
MESON = "sway/meson.build"

def once(src: str, old: str, new: str, why: str) -> str:
    if src.count(old) != 1 or new in src:
        raise RuntimeError("PA_G3B_CI_ONLY_OVERLAY_ANCHOR_DRIFT " + why)
    return src.replace(old, new, 1)

def target_patch(src: str) -> str:
    src = once(src,
        '#include "sway/tree/view.h"\n',
        '#include "sway/tree/view.h"\n'
        'void pa_g3b_native_target_bound(struct sway_container *target);\n'
        'void pa_g3b_native_target_revoked(struct sway_container *target);\n',
        "target_probe_decl")
    src = once(src,
        '\twl_signal_emit_mutable(&root->events.new_node, &c->node);',
        '\tpa_g3b_native_target_bound(c);\n'
        '\twl_signal_emit_mutable(&root->events.new_node, &c->node);',
        "after_real_target_bind")
    src = once(src,
        '\tif (con->view) {\n\t\tipc_event_window(con, "close");',
        '\tpa_g3b_native_target_revoked(con);\n'
        '\tif (con->view) {\n\t\tipc_event_window(con, "close");',
        "after_real_target_revoke")
    return src

def output_patch(src: str) -> str:
    src = once(src,
        '#include "sway/output.h"\n',
        '#include "sway/output.h"\n'
        'void pa_g3b_native_output_changed(struct sway_output *output, int reason);\n',
        "output_probe_decl")
    src = once(src,
        '\n}\n\nvoid output_enable(struct sway_output *output)',
        '\n\tpa_g3b_native_output_changed(output, 1);\n'
        '}\n\nvoid output_enable(struct sway_output *output)',
        "after_real_output_config_commit")
    src = once(src,
        '\n\t// Remove the output now to avoid interacting with it during e.g.,',
        '\n\tpa_g3b_native_output_changed(output, 2);\n'
        '\n\t// Remove the output now to avoid interacting with it during e.g.,',
        "after_real_output_disable_revoke")
    return src

def meson_patch(src: str) -> str:
    # The only build-file edit occurs inside RUNNER_TEMP copied checkout.
    src = once(src,
        "\t'input/post_apollo_autoscroll_runtime.c',\n",
        "\t'input/post_apollo_autoscroll_runtime.c',\n"
        "\t'input/post_apollo_autoscroll_continuity.c',\n"
        "\t'../tests/autoscroll/ci_v5f_g3b_native_probe.c',\n",
        "disposable_only_build_wiring")
    return src

def main() -> None:
    e = os.environ
    tmp = Path(e.get("RUNNER_TEMP", "/missing"))
    ws = Path(e.get("GITHUB_WORKSPACE", "/missing"))
    if not (e.get("CI") == "true" and e.get("GITHUB_ACTIONS") == "true"
            and e.get("PA_AUTOSCROLL_CI_NATIVE_TRACE") == "1"
            and e.get("PA_G3B_NATIVE_REVOCATION") == "1"
            and str(tmp) == "/home/runner/work/_temp"
            and os.getuid() != 0 and tmp.is_dir() and ws.is_dir()):
        raise RuntimeError("BLOCKED_PA_G3B_NON_DISPOSABLE_HOST")
    c1.main()  # audited preexisting native target/output overlay
    dst = tmp / "pa-fc1-native-src"
    for path, patch in (
            (TARGET, target_patch),
            (OUTPUT, output_patch),
            (MESON, meson_patch)):
        full = dst / path
        full.write_text(patch(full.read_text(encoding="utf-8")),
                        encoding="utf-8")
    changes = set(subprocess.check_output(
        ["git", "-C", str(dst), "diff", "--name-only"],
        text=True).splitlines())
    if changes != {TARGET, OUTPUT, MESON}:
        raise RuntimeError("PA_G3B_DISPOSABLE_OVERLAY_FOREIGN_PATHS "
                           + repr(changes))
    if subprocess.check_output(
            ["git", "-C", str(ws), "status", "--porcelain"], text=True):
        raise RuntimeError("PA_G3B_ORIGINAL_SOURCE_CHECKOUT_CHANGED")
    print("PA_G3B_CI_ONLY_NATIVE_OWNER_CALLBACK_AND_MESON_OVERLAY_PASS "
          "original_checkout=UNCHANGED production_linkage=NONE",
          flush=True)

if __name__ == "__main__":
    main()
