#!/usr/bin/env python3
"""G3J disposable compositor source COPY: add true wlroots libinput observer.

Retains G3H/G3E/G2 checks, Weather/Company first refusal, and production
HARD OFF. Never modifies checked-in compositor source or normal Meson.
"""
from pathlib import Path
import os
import subprocess
import ci_v5f_g3h_native_overlay as g3h
import ci_v5f_g2_native_button as g2

CURSOR="sway/input/cursor.c"
MESON="sway/meson.build"
def one(s, old, new, label):
    if s.count(old)!=1:
        raise RuntimeError("PA_G3J_NATIVE_OVERLAY_ANCHOR_DRIFT "+label)
    return s.replace(old,new,1)
def cursor_patch(s):
    declaration=(
        "void pa_g3h_native_os_managed_diagnostic_observe(\n"
        "        struct sway_cursor *cursor,\n"
        "        struct wlr_pointer_button_event *event,\n"
        "        bool weather_paired, bool company_paired);\n")
    s=one(s,declaration,declaration+(
        "void pa_g3j_real_libinput_compositor_observe(\n"
        "        struct sway_cursor *cursor,\n"
        "        struct wlr_pointer_button_event *event,\n"
        "        bool weather_paired, bool company_paired);\n"
    ),"DECLARE")
    call=("\tpa_g3h_native_os_managed_diagnostic_observe(cursor, event,\n"
          "\t\tweather_paired, company_paired);\n")
    return one(s,call,call+(
        "\t/* G3J: read native kernel-backed libinput source, log only. */\n"
        "\tpa_g3j_real_libinput_compositor_observe(cursor, event,\n"
        "\t\tweather_paired, company_paired);\n"
    ),"EVENT")
def meson_patch(s):
    old="\t'../tests/autoscroll/ci_v5f_g3h_native_os_observer.c',\n"
    return one(s,old,old+
        "\t'../tests/autoscroll/ci_v5f_g3j_native_compositor_probe.c',\n",
        "MESON")
def main():
    temp,ws=g2.guard_ci()
    if os.environ.get("PA_G3J_NATIVE_LIBINPUT_DIAG")!="1":
        raise RuntimeError("PA_G3J_NOT_DISPOSABLE_CI")
    g3h.main()
    target=temp/g2.COPY_NAME
    for path,patch in ((CURSOR,cursor_patch),(MESON,meson_patch)):
        x=target/path
        x.write_text(patch(x.read_text()),encoding="utf-8")
    changed=set(subprocess.check_output(
        ["git","-C",str(target),"diff","--name-only"],text=True).splitlines())
    if changed != {CURSOR,MESON,"sway/input/input-manager.c",
                   "sway/input/seat.c"}:
        raise RuntimeError("PA_G3J_NATIVE_OVERLAY_MUTATION "+repr(changed))
    if subprocess.check_output(
        ["git","-C",str(ws),"status","--porcelain"],text=True):
        raise RuntimeError("PA_G3J_CHECKED_IN_SOURCE_MUTATED")
    print("PA_G3J_REAL_NATIVE_LIBINPUT_OVERLAY_PASS "
          "compositor_callback=READ_ONLY native_build=COPY "
          "no_input_consume=1 no_axis=1 production=HARD_OFF",flush=True)
if __name__=="__main__":
    main()
