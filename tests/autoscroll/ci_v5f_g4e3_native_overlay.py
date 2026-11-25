#!/usr/bin/env python3
"""CI-only real axis delivery: no shipping source change or native grant."""
import os
import subprocess
import ci_v5f_g2_native_button as g2
import ci_v5f_g3o_native_overlay as g3o
import ci_v5f_g4e1_native_overlay as g4e1
def main():
    tmp,ws=g2.guard_ci()
    if os.environ.get("PA_G4E3_RECEIPTS")!="1":
        raise RuntimeError("BLOCKED_G4E3_NOT_DISPOSABLE_CI")
    g4e1.main()
    copy=tmp/g2.COPY_NAME
    obs="tests/autoscroll/ci_v5f_g4e1_stationary_scene.c"
    meson="sway/meson.build"
    s=(copy/obs).read_text()
    s=g3o.patch(s,'#include "sway/input/post_apollo_autoscroll_os_scene.h"\n',
        'void pa_g4e3_tick(const struct pa_os_scene_read *,const struct pa_os_native_slot *);\n')
    s=g3o.patch(s,'    struct pa_os_scroll_scene *v=&o.scene;\n','    pa_g4e3_tick(&o,&slot);\n')
    (copy/obs).write_text(s)
    s=(copy/meson).read_text()
    s=g3o.patch(s,"\t'input/post_apollo_autoscroll_os_scene.c',\n",
        "\t'input/post_apollo_autoscroll_os_controller.c',\n"
        "\t'../tests/autoscroll/ci_v5f_g4e3_native_axis.c',\n")
    (copy/meson).write_text(s)
    changed=set(subprocess.check_output(["git","-C",str(copy),"diff","--name-only"],text=True).splitlines())
    if changed!={obs,meson,"sway/tree/root.c","sway/input/cursor.c","include/sway/input/seat.h",
        "include/sway/input/keyboard.h","sway/input/keyboard.c","sway/input/seat.c",
        "sway/input/input-manager.c","tests/autoscroll/ci_v5f_g3o_native_epoch_observer.c"}:
        raise RuntimeError("G4E3_COPY_DIFF "+repr(changed))
    if subprocess.check_output(["git","-C",str(ws),"status","--porcelain"],text=True):
        raise RuntimeError("G4E3_PRODUCTION_CHECKOUT_CHANGED")
    print("PA_G4E3_NATIVE_OVERLAY_PASS",flush=True)
if __name__=="__main__":
    main()
