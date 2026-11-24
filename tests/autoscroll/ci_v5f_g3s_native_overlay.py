#!/usr/bin/env python3
"""G3S: test-only lock diagnostics, no production input changes."""
import os
import subprocess
import ci_v5f_g2_native_button as g2
import ci_v5f_g3o_native_overlay as g3o
import ci_v5f_g3r_native_overlay as g3r

def main():
    tmp,ws=g2.guard_ci()
    if os.environ.get("PA_G3S_LOCK")!="1":
        raise RuntimeError("BLOCKED_G3S_NOT_DISPOSABLE_CI")
    g3r.main()
    copy=tmp/g2.COPY_NAME
    key="sway/input/keyboard.c"
    cursor="sway/input/cursor.c"
    meson="sway/meson.build"
    s=(copy/key).read_text()
    s=g3o.patch(s,'#include "sway/input/keyboard.h"\n',
        'void pa_g3s_key(struct sway_keyboard *, struct wlr_keyboard_key_event *);\n')
    s=g3o.patch(s,'\tpa_g3m_native_second_keyboard_key_observe(keyboard, event);\n',
        '\tpa_g3s_key(keyboard,event);\n')
    (copy/key).write_text(s)
    s=(copy/cursor).read_text()
    s=g3o.patch(s,'void pa_g3r_snapshot(struct sway_cursor *, struct wlr_pointer_button_event *);\n',
        'void pa_g3s_snapshot(struct sway_cursor *, struct wlr_pointer_button_event *);\n')
    s=g3o.patch(s,'\tpa_g3r_snapshot(cursor,event);\n','\tpa_g3s_snapshot(cursor,event);\n')
    (copy/cursor).write_text(s)
    s=(copy/meson).read_text()
    s=g3o.patch(s,"\t'../tests/autoscroll/ci_v5f_g3r_native_seat_observer.c',\n",
        "\t'../tests/autoscroll/ci_v5f_g3s_native_lock_observer.c',\n")
    (copy/meson).write_text(s)
    changed=set(subprocess.check_output(
        ["git","-C",str(copy),"diff","--name-only"],text=True).splitlines())
    if changed!={key,cursor,meson,"sway/input/seat.c","sway/tree/root.c","sway/input/input-manager.c"}:
        raise RuntimeError("G3S_UNEXPECTED_COPY_DIFF "+repr(changed))
    if subprocess.check_output(["git","-C",str(ws),"status","--porcelain"],text=True):
        raise RuntimeError("G3S_PRODUCTION_CHECKOUT_CHANGED")
    print("PA_G3S_DISPOSABLE_LOCK_OVERLAY_PASS",flush=True)
if __name__=="__main__":
    main()
