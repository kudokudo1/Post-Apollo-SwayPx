#!/usr/bin/env python3
"""Bind private native producer only in disposable G3T compositor copy."""
import os
import subprocess
import ci_v5f_g2_native_button as g2
import ci_v5f_g3o_native_overlay as g3o
import ci_v5f_g3t_native_overlay as g3t
import ci_v5f_g3m_native_overlay as g3m
def main():
    tmp,ws=g2.guard_ci()
    if os.environ.get("PA_G4C2_NATIVE")!="1":
        raise RuntimeError("BLOCKED_G4C2_NOT_DISPOSABLE_CI")
    g3t.main()
    copy=tmp/g2.COPY_NAME
    seat="include/sway/input/seat.h"
    cursor="sway/input/cursor.c"
    meson="sway/meson.build"
    s=(copy/seat).read_text()
    s=g3o.patch(s,'#include "sway/input/input-manager.h"\n',
        '#include "sway/input/post_apollo_autoscroll_os_native.h"\n')
    s=g3o.patch(s,'\tstruct pa_autoscroll_lifetime_slot autoscroll_lifetime;\n',
        '\tstruct pa_os_native_slot pa_g4c2_slot;\n'
        '\tstruct pa_os_scroll_session pa_g4c2_session;\n')
    (copy/seat).write_text(s)
    s=(copy/cursor).read_text()
    s=g3o.patch(s,'void pa_g3s_snapshot(struct sway_cursor *, struct wlr_pointer_button_event *);\n',
        'void pa_g4c2_observe(struct sway_cursor *, struct wlr_pointer_button_event *);\n')
    s=g3m.one(s,'\tif (weather_paired || company_paired) {\n\t\treturn;\n\t}\n',
        '\tif (weather_paired || company_paired) {\n\t\treturn;\n\t}\n'
        '\tpa_g4c2_observe(cursor,event);\n',"G4C2 after actual pairing refusal")
    (copy/cursor).write_text(s)
    s=(copy/meson).read_text()
    s=g3o.patch(s,"\t'../tests/autoscroll/ci_v5f_g3t_shadow_cancel_observer.c',\n",
        "\t'../tests/autoscroll/ci_v5f_g4c2_native_observer.c',\n"
        "\t'input/post_apollo_autoscroll_os_native.c',\n"
        "\t'input/post_apollo_autoscroll_os_session.c',\n")
    (copy/meson).write_text(s)
    changed=set(subprocess.check_output(
        ["git","-C",str(copy),"diff","--name-only"],text=True).splitlines())
    if changed!={seat,cursor,meson,"sway/input/keyboard.c","sway/input/seat.c",
        "sway/tree/root.c","sway/input/input-manager.c",
        "tests/autoscroll/ci_v5f_g3o_native_epoch_observer.c"}:
        raise RuntimeError("G4C2_COPY_DIFF "+repr(changed))
    if subprocess.check_output(["git","-C",str(ws),"status","--porcelain"],text=True):
        raise RuntimeError("G4C2_PRODUCTION_CHECKOUT_CHANGED")
    print("PA_G4C2_NATIVE_OVERLAY_PASS",flush=True)
if __name__=="__main__":
    main()
