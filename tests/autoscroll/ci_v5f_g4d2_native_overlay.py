#!/usr/bin/env python3
"""CI-only seat/keyboard lifetime bindings and key observer."""
import os
import subprocess
import ci_v5f_g2_native_button as g2
import ci_v5f_g3o_native_overlay as g3o
import ci_v5f_g3m_native_overlay as g3m
import ci_v5f_g4c2_native_overlay as g4c2
def main():
    tmp,ws=g2.guard_ci()
    if os.environ.get("PA_G4D2_KEYS")!="1":
        raise RuntimeError("BLOCKED_G4D2_NOT_DISPOSABLE_CI")
    g4c2.main()
    copy=tmp/g2.COPY_NAME
    seat="include/sway/input/seat.h"
    keyh="include/sway/input/keyboard.h"
    key="sway/input/keyboard.c"
    cursor="sway/input/cursor.c"
    meson="sway/meson.build"
    s=(copy/seat).read_text()
    s=g3o.patch(s,'#include "sway/input/post_apollo_autoscroll_os_native.h"\n',
        '#include "sway/input/post_apollo_autoscroll_os_controls.h"\n')
    s=g3o.patch(s,'\tstruct pa_os_scroll_session pa_g4c2_session;\n',
        '\tstruct pa_os_controls pa_g4d2_controls;\n'
        '\tstruct pa_os_scroll_session pa_g4d2_shadow;\n'
        '\tstruct pa_os_keyboard_controls pa_g4d2_fixture_key;\n'
        '\tuint64_t pa_g4d2_next_keyboard;\n'
        '\tunsigned pa_g4d2_pairs;\n')
    (copy/seat).write_text(s)
    s=(copy/keyh).read_text()
    s=g3o.patch(s,'struct sway_keyboard {\n','\tstruct pa_os_keyboard_controls pa_g4d2_key;\n')
    (copy/keyh).write_text(s)
    s=(copy/key).read_text()
    s=g3o.patch(s,'void pa_g3s_key(struct sway_keyboard *, struct wlr_keyboard_key_event *);\n',
        'void pa_g4d2_key(struct sway_keyboard *, struct wlr_keyboard_key_event *);\n'
        'void pa_g4d2_retire(struct sway_keyboard *);\n')
    s=g3o.patch(s,'\tpa_g3s_key(keyboard,event);\n','\tpa_g4d2_key(keyboard,event);\n')
    s=g3o.patch(s,'void sway_keyboard_destroy(struct sway_keyboard *keyboard) {\n',
        '\tpa_g4d2_retire(keyboard);\n')
    (copy/key).write_text(s)
    s=(copy/cursor).read_text()
    s=g3o.patch(s,'void pa_g4c2_observe(struct sway_cursor *, struct wlr_pointer_button_event *);\n',
        'void pa_g4d2_mouse(struct sway_cursor *, struct wlr_pointer_button_event *);\n')
    s=g3o.patch(s,'\tpa_g4c2_observe(cursor,event);\n','\tpa_g4d2_mouse(cursor,event);\n')
    (copy/cursor).write_text(s)
    s=(copy/meson).read_text()
    s=g3o.patch(s,"\t'input/post_apollo_autoscroll_os_session.c',\n",
        "\t'input/post_apollo_autoscroll_os_controls.c',\n"
        "\t'../tests/autoscroll/ci_v5f_g4d2_native_controls.c',\n")
    (copy/meson).write_text(s)
    changed=set(subprocess.check_output(
        ["git","-C",str(copy),"diff","--name-only"],text=True).splitlines())
    if changed!={seat,keyh,key,cursor,meson,"sway/input/seat.c","sway/tree/root.c",
        "sway/input/input-manager.c","tests/autoscroll/ci_v5f_g3o_native_epoch_observer.c"}:
        raise RuntimeError("G4D2_COPY_DIFF "+repr(changed))
    if subprocess.check_output(["git","-C",str(ws),"status","--porcelain"],text=True):
        raise RuntimeError("G4D2_PRODUCTION_CHECKOUT_CHANGED")
    print("PA_G4D2_NATIVE_OVERLAY_PASS",flush=True)
if __name__=="__main__":
    main()
