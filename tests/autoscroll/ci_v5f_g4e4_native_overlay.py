#!/usr/bin/env python3
"""Separate disposable motion/Escape test; G4E3 source remains unchanged."""
import os
import subprocess
import ci_v5f_g2_native_button as g2
import ci_v5f_g3o_native_overlay as g3o
import ci_v5f_g3m_native_overlay as g3m
import ci_v5f_g4e3_native_overlay as g4e3
def main():
    tmp,ws=g2.guard_ci()
    if os.environ.get("PA_G4E4_MOTION")!="1":
        raise RuntimeError("BLOCKED_G4E4_NOT_DISPOSABLE_CI")
    g4e3.main()
    copy=tmp/g2.COPY_NAME
    obs="tests/autoscroll/ci_v5f_g4e1_stationary_scene.c"
    meson="sway/meson.build"
    cursor="sway/input/cursor.c"
    key="sway/input/keyboard.c"
    s=(copy/obs).read_text()
    if s.count("pa_g4e3_tick")!=2: raise RuntimeError("G4E4_TIMER_ANCHOR")
    (copy/obs).write_text(s.replace("pa_g4e3_tick","pa_g4e4_tick"))
    s=(copy/meson).read_text()
    s=g3m.one(s,"'../tests/autoscroll/ci_v5f_g4e3_native_axis.c'",
        "'../tests/autoscroll/ci_v5f_g4e4_native_motion.c'","G4E4_test_only")
    (copy/meson).write_text(s)
    s=(copy/cursor).read_text()
    s=g3o.patch(s,'void pa_g4e1_middle(struct sway_cursor *, struct wlr_pointer_button_event *);\n',
        'bool pa_g4e4_motion(struct sway_cursor *, struct wlr_pointer_motion_event *);\n')
    s=g3o.patch(s,'\tcursor_handle_activity_from_device(cursor, &e->pointer->base);\n'
        '\t/* Default OFF: no input consumed. Preserve native compositor pairing. */\n',
        '\tif (pa_g4e4_motion(cursor,e)) return;\n')
    (copy/cursor).write_text(s)
    s=(copy/key).read_text()
    s=g3o.patch(s,'void pa_g4d3_retire(struct sway_keyboard *);\n',
        'void pa_g4e4_key(struct sway_keyboard *, struct wlr_keyboard_key_event *);\n')
    s=g3o.patch(s,'\tpa_g4d3_key(keyboard,data);\n','\tpa_g4e4_key(keyboard,data);\n')
    (copy/key).write_text(s)
    changed=set(subprocess.check_output(["git","-C",str(copy),"diff","--name-only"],text=True).splitlines())
    if changed!={obs,meson,"sway/tree/root.c",cursor,"include/sway/input/seat.h",
        "include/sway/input/keyboard.h",key,"sway/input/seat.c","sway/input/input-manager.c",
        "tests/autoscroll/ci_v5f_g3o_native_epoch_observer.c"}:
        raise RuntimeError("G4E4_COPY_DIFF "+repr(changed))
    if subprocess.check_output(["git","-C",str(ws),"status","--porcelain"],text=True):
        raise RuntimeError("G4E4_PRODUCTION_CHECKOUT_CHANGED")
    print("PA_G4E4_NATIVE_OVERLAY_PASS",flush=True)
if __name__=="__main__":
    main()
