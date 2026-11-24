#!/usr/bin/env python3
"""Create a disposable CI-only SwayPX instrumented source copy.

PRODUCTION FILES REMAIN UNTOUCHED. Event receipts are emitted by the REAL
mapped-window/output owner callbacks, not by synthetic C mock functions.
Do not enable this outside an isolated GitHub runner. The numerical tokens
in this test log are NOT credentials and must never authorize input.
"""
from __future__ import annotations
import os
from pathlib import Path
import shutil
import subprocess

TARGET = "sway/tree/container.c"
OUTPUT = "sway/tree/output.c"

def replace_one(src: str, old: str, new: str, label: str) -> str:
    if src.count(old) != 1 or new in src:
        raise RuntimeError("PA_FC1_OVERLAY_ANCHOR_DRIFT " + label)
    return src.replace(old, new, 1)

def target_patch(src: str) -> str:
    src = replace_one(src,
        '#include "sway/output.h"\n',
        '#include "sway/output.h"\n#include <inttypes.h>\n',
        "target_include")
    src = replace_one(src,
        "\t}\n\twl_signal_emit_mutable(&root->events.new_node, &c->node);",
        "\t}\n\tif (view != NULL && view->surface != NULL) {\n"
        '\t\tsway_log(SWAY_INFO, "PA_FC1_NATIVE TARGET_BIND node=%" PRIu64 '
        '" epoch=%" PRIu64,\n'
        "\t\t\t(uint64_t)c->node.id,\n"
        "\t\t\tcontainer_autoscroll_target_lifetime_epoch(c));\n"
        "\t}\n\twl_signal_emit_mutable(&root->events.new_node, &c->node);",
        "target_after_bind_before_publish")
    src = replace_one(src,
        "\tpa_autoscroll_lifetime_revoke(&con->autoscroll_target_lifetime);\n"
        "\tif (con->view) {",
        "\tconst uint64_t pa_fc1_before =\n"
        "\t\tcontainer_autoscroll_target_lifetime_epoch(con);\n"
        "\tpa_autoscroll_lifetime_revoke(&con->autoscroll_target_lifetime);\n"
        "\tif (con->view != NULL) {\n"
        '\t\tsway_log(SWAY_INFO, "PA_FC1_NATIVE TARGET_REVOKE node=%" PRIu64 '
        '" before=%" PRIu64 " after=%" PRIu64,\n'
        "\t\t\t(uint64_t)con->node.id, pa_fc1_before,\n"
        "\t\t\tcontainer_autoscroll_target_lifetime_epoch(con));\n"
        "\t}\n"
        "\tif (con->view) {",
        "target_first_revoke")
    return src

def output_patch(src: str) -> str:
    src = replace_one(src,
        '#include "sway/output.h"\n',
        '#include "sway/output.h"\n#include <inttypes.h>\n',
        "output_include")
    src = replace_one(src,
        "void output_autoscroll_config_committed(struct sway_output *output) {\n"
        "\tif (output == NULL) {\n\t\treturn;\n\t}\n",
        "void output_autoscroll_config_committed(struct sway_output *output) {\n"
        "\tif (output == NULL) {\n\t\treturn;\n\t}\n"
        "\tconst uint64_t pa_fc1_previous =\n"
        "\t\toutput_autoscroll_lifetime_epoch(output);\n",
        "output_config_previous")
    src = replace_one(src,
        "\t\t&output->autoscroll_output_lifetime, PA_LIFETIME_OUTPUT);\n"
        "}\n\nvoid output_enable(",
        "\t\t&output->autoscroll_output_lifetime, PA_LIFETIME_OUTPUT);\n"
        '\tsway_log(SWAY_INFO, "PA_FC1_NATIVE OUTPUT_CONFIG old=%" PRIu64 '
        '" new=%" PRIu64,\n'
        "\t\tpa_fc1_previous, output_autoscroll_lifetime_epoch(output));\n"
        "}\n\nvoid output_enable(",
        "output_config_after_bind")
    src = replace_one(src,
        "\toutput->enabled = true;\n\tlist_add(root->outputs, output);",
        "\toutput->enabled = true;\n"
        '\tsway_log(SWAY_INFO, "PA_FC1_NATIVE OUTPUT_ENABLE epoch=%" PRIu64,\n'
        "\t\toutput_autoscroll_lifetime_epoch(output));\n"
        "\tlist_add(root->outputs, output);",
        "output_enable_before_publish")
    src = replace_one(src,
        "\tpa_autoscroll_lifetime_revoke(&output->autoscroll_output_lifetime);\n"
        "\n\t// Remove the output now",
        "\tconst uint64_t pa_fc1_before =\n"
        "\t\toutput_autoscroll_lifetime_epoch(output);\n"
        "\tpa_autoscroll_lifetime_revoke(&output->autoscroll_output_lifetime);\n"
        '\tsway_log(SWAY_INFO, "PA_FC1_NATIVE OUTPUT_DISABLE before=%" PRIu64 '
        '" after=%" PRIu64,\n'
        "\t\tpa_fc1_before, output_autoscroll_lifetime_epoch(output));\n"
        "\n\t// Remove the output now",
        "output_revoke_before_evacuation")
    return src

def main() -> None:
    e = os.environ
    tmp = Path(e.get("RUNNER_TEMP", "/missing"))
    ws = Path(e.get("GITHUB_WORKSPACE", "/missing")).resolve()
    if not (e.get("CI") == "true" and e.get("GITHUB_ACTIONS") == "true"
            and e.get("PA_AUTOSCROLL_CI_NATIVE_TRACE") == "1"
            and str(tmp) == "/home/runner/work/_temp"
            and tmp.is_dir() and ws.is_dir() and os.getuid() != 0):
        raise RuntimeError("BLOCKED_FC1_NON_DISPOSABLE_RUNNER")
    if subprocess.check_output(
            ["git", "-C", str(ws), "status", "--porcelain"], text=True):
        raise RuntimeError("PA_FC1_PRODUCTION_CHECKOUT_NOT_CLEAN")
    dst = tmp / "pa-fc1-native-src"
    if dst.exists():
        raise RuntimeError("PA_FC1_DISPOSABLE_SOURCE_ALREADY_EXISTS")
    shutil.copytree(ws, dst, symlinks=False, ignore=shutil.ignore_patterns(
        "__pycache__", "result"))
    for path, fn in ((TARGET, target_patch), (OUTPUT, output_patch)):
        source = dst / path
        source.write_text(fn(source.read_text(encoding="utf-8")),
                          encoding="utf-8")
    changes = subprocess.check_output(
        ["git", "-C", str(dst), "diff", "--name-only"], text=True).splitlines()
    if changes != sorted((TARGET, OUTPUT)):
        raise RuntimeError("PA_FC1_OVERLAY_FOREIGN_FILES " + repr(changes))
    if subprocess.check_output(
            ["git", "-C", str(ws), "status", "--porcelain"], text=True):
        raise RuntimeError("PA_FC1_SOURCE_CHECKOUT_MUTATED")
    print("PA_FC1_DISPOSABLE_OWNER_CALLBACK_OVERLAY "
          "target=REAL output=REAL original_checkout=UNMODIFIED", flush=True)

if __name__ == "__main__":
    main()
