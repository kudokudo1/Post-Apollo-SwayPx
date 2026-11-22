#!/usr/bin/env python3
"""G3D runner-only build overlay for REAL compositor IPC cursor scene checks."""
from __future__ import annotations
import os
from pathlib import Path
import shutil
import subprocess

COMMAND = "sway/commands/seat/cursor.c"
MESON = "sway/meson.build"

def one(src: str, old: str, new: str, why: str) -> str:
    if src.count(old) != 1 or new in src:
        raise RuntimeError("PA_G3D_NATIVE_SOURCE_ANCHOR_DRIFT " + why)
    return src.replace(old,new,1)

def command_patch(src: str) -> str:
    src = one(src,
        '#include "sway/input/cursor.h"\n',
        '#include "sway/input/cursor.h"\n'
        'void pa_g3d_native_sample_on_cursor_set(struct sway_cursor *cursor);\n',
        "probe_decl")
    return one(src,
        '\t\twlr_cursor_warp_absolute(cursor->cursor, NULL, x, y);\n'
        '\t\tcursor_rebase(cursor);\n'
        '\t\twlr_seat_pointer_notify_frame(cursor->seat->wlr_seat);',
        '\t\twlr_cursor_warp_absolute(cursor->cursor, NULL, x, y);\n'
        '\t\tcursor_rebase(cursor);\n'
        '\t\tpa_g3d_native_sample_on_cursor_set(cursor);\n'
        '\t\twlr_seat_pointer_notify_frame(cursor->seat->wlr_seat);',
        "native_ipc_cursor_set_after_real_scene_update")

def meson_patch(src: str) -> str:
    return one(src,
        "\t'input/post_apollo_autoscroll_runtime.c',\n",
        "\t'input/post_apollo_autoscroll_runtime.c',\n"
        "\t'../tests/autoscroll/ci_v5f_g3d_native_probe.c',\n",
        "ci_only_native_probe_compilation")

def main() -> None:
    e=os.environ
    tmp=Path(e.get("RUNNER_TEMP","/missing"))
    ws=Path(e.get("GITHUB_WORKSPACE","/missing")).resolve()
    if not (e.get("GITHUB_ACTIONS")=="true" and e.get("CI")=="true"
            and e.get("PA_G3D_NATIVE_SCENE_AUDIT")=="1"
            and str(tmp)=="/home/runner/work/_temp"
            and tmp.is_dir() and ws.is_dir() and os.getuid()!=0):
        raise RuntimeError("BLOCKED_PA_G3D_NON_DISPOSABLE_RUNNER")
    if subprocess.check_output(["git","-C",str(ws),"status","--porcelain"],
            text=True):
        raise RuntimeError("PA_G3D_ORIGINAL_CHECKOUT_NOT_CLEAN")
    dst=tmp/"pa-g3d-native-src"
    if dst.exists():
        raise RuntimeError("PA_G3D_REUSING_PRIOR_CI_COPY_DENIED")
    shutil.copytree(ws,dst,symlinks=False,
                    ignore=shutil.ignore_patterns("__pycache__","result"))
    for path,patch in ((COMMAND,command_patch),(MESON,meson_patch)):
        target=dst/path
        target.write_text(patch(target.read_text(encoding="utf-8")),
                          encoding="utf-8")
    actual=set(subprocess.check_output(
        ["git","-C",str(dst),"diff","--name-only"],
        text=True).splitlines())
    if actual!={COMMAND,MESON}:
        raise RuntimeError("PA_G3D_OVERLAY_FOREIGN_PRODUCTION_PATHS "+repr(actual))
    if subprocess.check_output(["git","-C",str(ws),"status","--porcelain"],text=True):
        raise RuntimeError("PA_G3D_ORIGINAL_CHECKOUT_MUTATED")
    print("PA_G3D_DISPOSABLE_SCENE_SCROLL_LED_OVERLAY_PASS "
          "original_checkout=UNCHANGED production_probe=ABSENT",
          flush=True)

if __name__=="__main__":
    main()
