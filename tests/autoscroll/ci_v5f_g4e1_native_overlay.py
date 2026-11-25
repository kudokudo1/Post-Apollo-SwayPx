#!/usr/bin/env python3
"""Compose native stationary read-only timer in disposable compositor."""
import os
import subprocess
import ci_v5f_g2_native_button as g2
import ci_v5f_g3o_native_overlay as g3o
import ci_v5f_g4d3_native_overlay as g4d3
def main():
    tmp,ws=g2.guard_ci()
    if os.environ.get("PA_G4E1_SCENE")!="1":
        raise RuntimeError("BLOCKED_G4E1_NOT_DISPOSABLE_CI")
    g4d3.main()
    copy=tmp/g2.COPY_NAME
    root="sway/tree/root.c"
    cursor="sway/input/cursor.c"
    meson="sway/meson.build"
    s=(copy/root).read_text()
    s=g3o.patch(s,'void pa_g3o_attach_layout(struct wlr_output_layout *layout);\n',
        'void pa_g4e1_attach(struct wlr_output_layout *);\n')
    s=g3o.patch(s,'\tpa_g3o_attach_layout(root->output_layout);\n',
        '\tpa_g4e1_attach(root->output_layout);\n')
    (copy/root).write_text(s)
    s=(copy/cursor).read_text()
    s=g3o.patch(s,'void pa_g4d2_mouse(struct sway_cursor *, struct wlr_pointer_button_event *);\n',
        'void pa_g4e1_middle(struct sway_cursor *, struct wlr_pointer_button_event *);\n')
    s=g3o.patch(s,'\tpa_g4c2_observe(cursor,event);\n','\tpa_g4e1_middle(cursor,event);\n')
    (copy/cursor).write_text(s)
    s=(copy/meson).read_text()
    s=g3o.patch(s,"\t'input/post_apollo_autoscroll_os_keyboard.c',\n",
        "\t'input/post_apollo_autoscroll_os_scene.c',\n"
        "\t'../tests/autoscroll/ci_v5f_g4e1_stationary_scene.c',\n")
    (copy/meson).write_text(s)
    changed=set(subprocess.check_output(["git","-C",str(copy),"diff","--name-only"],text=True).splitlines())
    if changed!={root,cursor,meson,"include/sway/input/seat.h","include/sway/input/keyboard.h",
        "sway/input/keyboard.c","sway/input/seat.c","sway/input/input-manager.c",
        "tests/autoscroll/ci_v5f_g3o_native_epoch_observer.c"}:
        raise RuntimeError("G4E1_COPY_DIFF "+repr(changed))
    if subprocess.check_output(["git","-C",str(ws),"status","--porcelain"],text=True):
        raise RuntimeError("G4E1_PRODUCTION_CHECKOUT_CHANGED")
    print("PA_G4E1_NATIVE_OVERLAY_PASS",flush=True)
if __name__=="__main__":
    main()
