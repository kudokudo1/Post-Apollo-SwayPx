#!/usr/bin/env python3
"""G3M CI COPY: real seat keyboard hotplug snapshot; DENY forever."""
import os
import subprocess
import ci_v5f_g3l_native_overlay as g3l
import ci_v5f_g2_native_button as g2

CURSOR="sway/input/cursor.c"
KEYBOARD="sway/input/keyboard.c"
MESON="sway/meson.build"

def one(s,old,new,where):
    if s.count(old)!=1:
        raise RuntimeError("PA_G3M_OVERLAY_ANCHOR_DRIFT "+where)
    return s.replace(old,new,1)

def patch_cursor(s):
    decl=(
        "void pa_g3l_native_lock_layout_observe(\n"
        "        struct sway_cursor *cursor,\n"
        "        struct wlr_pointer_button_event *event);\n")
    s=one(s,decl,decl+(
        "void pa_g3m_native_multikeyboard_observe(\n"
        "        struct sway_cursor *cursor,\n"
        "        struct wlr_pointer_button_event *event);\n"
    ),"CURSOR_DECL")
    call="\tpa_g3l_native_lock_layout_observe(cursor, event);\n"
    return one(s,call,call+(
        "\t/* G3M: observe all native seat keyboards, still no authority. */\n"
        "\tpa_g3m_native_multikeyboard_observe(cursor, event);\n"
    ),"CURSOR_EVENT")

def patch_keyboard(s):
    decl=(
        "void pa_g3l_native_keyboard_key_observe(\n"
        "        struct sway_keyboard *keyboard,\n"
        "        struct wlr_keyboard_key_event *event);\n")
    s=one(s,decl,decl+(
        "void pa_g3m_native_second_keyboard_key_observe(\n"
        "        struct sway_keyboard *keyboard,\n"
        "        struct wlr_keyboard_key_event *event);\n"
    ),"KEY_DECL")
    call="\tpa_g3l_native_keyboard_key_observe(keyboard, event);\n"
    return one(s,call,call+(
        "\t/* G3M: actual secondary Scroll Lock event, no consumption. */\n"
        "\tpa_g3m_native_second_keyboard_key_observe(keyboard, event);\n"
    ),"KEY_EVENT")

def patch_meson(s):
    old="\t'../tests/autoscroll/ci_v5f_g3l_native_keyboard_layout_probe.c',\n"
    return one(s,old,old+(
        "\t'../tests/autoscroll/ci_v5f_g3m_native_multikeyboard_observer.c',\n"
    ),"MESON")

def main():
    tmp,ws=g2.guard_ci()
    if os.environ.get("PA_G3M_NATIVE_OBSERVE")!="1":
        raise RuntimeError("BLOCKED_G3M_NOT_GITHUB_DISPOSABLE_CI")
    g3l.main()
    copy=tmp/g2.COPY_NAME
    for path,fn in ((CURSOR,patch_cursor),
                    (KEYBOARD,patch_keyboard),(MESON,patch_meson)):
        p=copy/path
        p.write_text(fn(p.read_text()),encoding="utf-8")
    changes=set(subprocess.check_output(
        ["git","-C",str(copy),"diff","--name-only"],
        text=True).splitlines())
    if changes != {CURSOR,KEYBOARD,MESON,
                   "sway/input/input-manager.c","sway/input/seat.c"}:
        raise RuntimeError("PA_G3M_DISPOSABLE_COPY_DIFF_UNEXPECTED "+
                           repr(changes))
    if subprocess.check_output(
        ["git","-C",str(ws),"status","--porcelain"],text=True):
        raise RuntimeError("PA_G3M_TRACKED_PRODUCTION_MODIFIED")
    print("PA_G3M_NATIVE_HOTPLUG_OBSERVER_CI_COPY_PASS "
          "only_read=1 no_input_consume=1 no_axis=1 hard_off=1",
          flush=True)
if __name__=="__main__":
    main()
