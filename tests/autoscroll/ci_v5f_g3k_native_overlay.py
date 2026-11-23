#!/usr/bin/env python3
"""G3K disposable compositor COPY: append deny-only guard probe to G3J."""
import os
import subprocess
import ci_v5f_g3j_native_overlay as g3j
import ci_v5f_g2_native_button as g2

CURSOR="sway/input/cursor.c"
MESON="sway/meson.build"

def one(s,old,new,label):
    if s.count(old)!=1:
        raise RuntimeError("PA_G3K_NATIVE_OVERLAY_ANCHOR_DRIFT "+label)
    return s.replace(old,new,1)

def patch_cursor(s):
    declaration=(
        "void pa_g3j_real_libinput_compositor_observe(\n"
        "        struct sway_cursor *cursor,\n"
        "        struct wlr_pointer_button_event *event,\n"
        "        bool weather_paired, bool company_paired);\n")
    s=one(s,declaration,declaration+(
        "void pa_g3k_native_guard_negative_observe(\n"
        "        struct sway_cursor *cursor,\n"
        "        struct wlr_pointer_button_event *event);\n"
    ),"DECLARATION")
    call=("\tpa_g3j_real_libinput_compositor_observe(cursor, event,\n"
          "\t\tweather_paired, company_paired);\n")
    return one(s,call,call+(
        "\t/* G3K: native owner and guard read only, never input consumption. */\n"
        "\tpa_g3k_native_guard_negative_observe(cursor, event);\n"
    ),"NATIVE_CALLBACK")

def patch_meson(s):
    line="\t'../tests/autoscroll/ci_v5f_g3j_native_compositor_probe.c',\n"
    return one(s,line,line+
        "\t'../tests/autoscroll/ci_v5f_g3k_native_guard_negative.c',\n",
        "TEST_ONLY_C")

def main():
    tmp,ws=g2.guard_ci()
    if os.environ.get("PA_G3K_NATIVE_GUARD")!="1":
        raise RuntimeError("BLOCKED_PA_G3K_NOT_DISPOSABLE_CI")
    g3j.main()
    target=tmp/g2.COPY_NAME
    for path,fn in ((CURSOR,patch_cursor),(MESON,patch_meson)):
        p=target/path
        p.write_text(fn(p.read_text()),encoding="utf-8")
    changed=set(subprocess.check_output(
        ["git","-C",str(target),"diff","--name-only"],text=True).splitlines())
    if changed != {CURSOR,MESON,"sway/input/input-manager.c",
                   "sway/input/seat.c"}:
        raise RuntimeError("PA_G3K_DISPOSABLE_DIFF_DRIFT "+repr(changed))
    if subprocess.check_output(
        ["git","-C",str(ws),"status","--porcelain"],text=True):
        raise RuntimeError("PA_G3K_CHECKED_IN_SOURCE_CHANGED")
    print("PA_G3K_NATIVE_GUARD_OVERLAY_PASS "
          "compositor_source=COPY only_diagnostic=1 "
          "normal_button_dispatch=PRESERVED production=HARD_OFF",
          flush=True)

if __name__=="__main__":
    main()
