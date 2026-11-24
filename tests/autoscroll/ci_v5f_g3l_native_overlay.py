#!/usr/bin/env python3
"""G3L: append native read-only XKB LED + output geometry in CI COPY."""
import os
import subprocess
import ci_v5f_g3k_native_overlay as g3k
import ci_v5f_g2_native_button as g2

CURSOR="sway/input/cursor.c"
MESON="sway/meson.build"
KEYBOARD="sway/input/keyboard.c"

def one(s,old,new,where):
    if s.count(old)!=1:
        raise RuntimeError("PA_G3L_OVERLAY_ANCHOR_CHANGED "+where)
    return s.replace(old,new,1)

def patch_cursor(s):
    decl=(
        "void pa_g3k_native_guard_negative_observe(\n"
        "        struct sway_cursor *cursor,\n"
        "        struct wlr_pointer_button_event *event);\n")
    s=one(s,decl,decl+(
        "void pa_g3l_native_lock_layout_observe(\n"
        "        struct sway_cursor *cursor,\n"
        "        struct wlr_pointer_button_event *event);\n"
    ),"DECL")
    call="\tpa_g3k_native_guard_negative_observe(cursor, event);\n"
    return one(s,call,call+(
        "\t/* G3L: read-only native keyboard/layout, never provenance. */\n"
        "\tpa_g3l_native_lock_layout_observe(cursor, event);\n"
    ),"EVENT")

def patch_meson(s):
    old="\t'../tests/autoscroll/ci_v5f_g3k_native_guard_negative.c',\n"
    return one(s,old,old+
        "\t'../tests/autoscroll/ci_v5f_g3l_native_keyboard_layout_probe.c',\n",
        "MESON")

def patch_keyboard(s):
    marker='#include "sway/input/keyboard.h"\n'
    s=one(s,marker,marker+(
        "void pa_g3l_native_keyboard_key_observe(\n"
        "        struct sway_keyboard *keyboard,\n"
        "        struct wlr_keyboard_key_event *event);\n"
    ),"KEYBOARD_DECL")
    marker=("\tupdate_keyboard_state(keyboard, event->keycode, "
            "event->state, &keyinfo);\n")
    return one(s,marker,marker+
        "\t/* G3L: real Scroll Lock keyboard callback, read only. */\n"
        "\tpa_g3l_native_keyboard_key_observe(keyboard, event);\n",
        "KEYBOARD_KEY_EVENT")

def main():
    temp,ws=g2.guard_ci()
    if os.environ.get("PA_G3L_NATIVE_OBSERVE")!="1":
        raise RuntimeError("PA_G3L_NOT_DISPOSABLE_NATIVE_CI")
    g3k.main()
    target=temp/g2.COPY_NAME
    for path,fn in ((CURSOR,patch_cursor),(MESON,patch_meson),(KEYBOARD,patch_keyboard)):
        p=target/path
        p.write_text(fn(p.read_text()),encoding="utf-8")
    changes=set(subprocess.check_output(
        ["git","-C",str(target),"diff","--name-only"],text=True).splitlines())
    if changes != {CURSOR,MESON,KEYBOARD,"sway/input/input-manager.c",
                   "sway/input/seat.c"}:
        raise RuntimeError("PA_G3L_NATIVE_COPY_DIFF_DRIFT "+repr(changes))
    if subprocess.check_output(
        ["git","-C",str(ws),"status","--porcelain"],text=True):
        raise RuntimeError("PA_G3L_TRACKED_SOURCE_MODIFIED")
    print("PA_G3L_NATIVE_OBSERVER_CI_COPY_PASS "
          "source=TEST_ONLY no_input_consume=1 no_axis=1 hard_off=1",
          flush=True)
if __name__=="__main__":
    main()
