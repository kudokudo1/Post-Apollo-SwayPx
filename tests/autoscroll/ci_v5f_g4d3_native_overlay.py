#!/usr/bin/env python3
"""Bind G4D3 only in disposable compositor; preserve G4D2 receipts."""
import os
import subprocess
import ci_v5f_g2_native_button as g2
import ci_v5f_g3o_native_overlay as g3o
import ci_v5f_g4d2_native_overlay as g4d2
def main():
    tmp,ws=g2.guard_ci()
    if os.environ.get("PA_G4D3_KEYS")!="1":
        raise RuntimeError("BLOCKED_G4D3_NOT_DISPOSABLE_CI")
    g4d2.main()
    copy=tmp/g2.COPY_NAME
    seat="include/sway/input/seat.h"
    keyh="include/sway/input/keyboard.h"
    key="sway/input/keyboard.c"
    meson="sway/meson.build"
    s=(copy/seat).read_text()
    s=g3o.patch(s,'#include "sway/input/post_apollo_autoscroll_os_controls.h"\n',
        '#include "sway/input/post_apollo_autoscroll_os_keyboard.h"\n')
    s=g3o.patch(s,'\tstruct pa_os_controls pa_g4d2_controls;\n',
        '\tstruct pa_os_key_owner pa_g4d3_owner;\n'
        '\tstruct pa_os_controls pa_g4d3_controls;\n'
        '\tstruct pa_os_scroll_session pa_g4d3_session;\n')
    (copy/seat).write_text(s)
    s=(copy/keyh).read_text()
    s=g3o.patch(s,'\tstruct pa_os_keyboard_controls pa_g4d2_key;\n',
        '\tstruct pa_os_key_binding pa_g4d3_binding;\n')
    (copy/keyh).write_text(s)
    s=(copy/key).read_text()
    s=g3o.patch(s,'void pa_g4d2_retire(struct sway_keyboard *);\n',
        'void pa_g4d3_bind(struct sway_keyboard *);\n'
        'void pa_g4d3_key(struct sway_keyboard *, struct wlr_keyboard_key_event *);\n'
        'void pa_g4d3_retire(struct sway_keyboard *);\n')
    s=g3o.patch(s,'\tkeyboard->keyboard_key.notify = handle_keyboard_key;\n',
        '\tpa_g4d3_bind(keyboard);\n')
    s=g3o.patch(s,'\tstruct sway_keyboard *keyboard =\n\t\twl_container_of(listener, keyboard, keyboard_key);\n',
        '\tpa_g4d3_key(keyboard,data);\n')
    s=g3o.patch(s,'\tpa_g4d2_retire(keyboard);\n','\tpa_g4d3_retire(keyboard);\n')
    (copy/key).write_text(s)
    s=(copy/meson).read_text()
    s=g3o.patch(s,"\t'input/post_apollo_autoscroll_os_controls.c',\n",
        "\t'input/post_apollo_autoscroll_os_keyboard.c',\n"
        "\t'../tests/autoscroll/ci_v5f_g4d3_native_key_authority.c',\n")
    (copy/meson).write_text(s)
    changed=set(subprocess.check_output(["git","-C",str(copy),"diff","--name-only"],text=True).splitlines())
    if changed!={seat,keyh,key,meson,"sway/input/cursor.c","sway/input/seat.c",
        "sway/tree/root.c","sway/input/input-manager.c",
        "tests/autoscroll/ci_v5f_g3o_native_epoch_observer.c"}:
        raise RuntimeError("G4D3_COPY_DIFF "+repr(changed))
    if subprocess.check_output(["git","-C",str(ws),"status","--porcelain"],text=True):
        raise RuntimeError("G4D3_PRODUCTION_CHECKOUT_CHANGED")
    print("PA_G4D3_NATIVE_OVERLAY_PASS",flush=True)
if __name__=="__main__":
    main()
