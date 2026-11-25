#!/usr/bin/env python3
"""CI-only native middle button and Scroll Lock pairing."""
import os
import subprocess
import ci_v5f_g2_native_button as g2
import ci_v5f_g3o_native_overlay as g3o
import ci_v5f_g4d3_native_overlay as g4d3
def main():
    tmp,ws=g2.guard_ci()
    if os.environ.get("PA_G4E5_PAIR")!="1":
        raise RuntimeError("BLOCKED_G4E5_NOT_DISPOSABLE_CI")
    g4d3.main()
    copy=tmp/g2.COPY_NAME
    root="sway/tree/root.c"
    cursor="sway/input/cursor.c"
    key="sway/input/keyboard.c"
    meson="sway/meson.build"
    s=(copy/root).read_text()
    s=g3o.patch(s,'void pa_g3o_attach_layout(struct wlr_output_layout *layout);\n',
        'void pa_g4e5_attach(struct wlr_output_layout *);\n')
    s=g3o.patch(s,'\tpa_g3o_attach_layout(root->output_layout);\n',
        '\tpa_g4e5_attach(root->output_layout);\n')
    (copy/root).write_text(s)
    s=(copy/cursor).read_text()
    s=g3o.patch(s,'void pa_g4c2_observe(struct sway_cursor *, struct wlr_pointer_button_event *);\n',
        'bool pa_g4e5_middle(struct sway_cursor *, struct wlr_pointer_button_event *);\n')
    s=g3o.patch(s,'\tpa_g4c2_observe(cursor,event);\n',
        '\tif (pa_g4e5_middle(cursor,event)) return;\n')
    (copy/cursor).write_text(s)
    s=(copy/key).read_text()
    s=g3o.patch(s,'void pa_g4d3_retire(struct sway_keyboard *);\n',
        'void pa_g4e5_key(struct sway_keyboard *, struct wlr_keyboard_key_event *);\n')
    s=g3o.patch(s,'\tpa_g4d3_key(keyboard,data);\n','\tpa_g4e5_key(keyboard,data);\n')
    (copy/key).write_text(s)
    s=(copy/meson).read_text()
    s=g3o.patch(s,"\t'input/post_apollo_autoscroll_os_keyboard.c',\n",
        "\t'input/post_apollo_autoscroll_os_scene.c',\n"
        "\t'input/post_apollo_autoscroll_os_controller.c',\n"
        "\t'input/post_apollo_autoscroll_os_ingress.c',\n"
        "\t'../tests/autoscroll/ci_v5f_g4e5_native_middle.c',\n")
    (copy/meson).write_text(s)
    changed=set(subprocess.check_output(["git","-C",str(copy),"diff","--name-only"],text=True).splitlines())
    if changed!={root,cursor,key,meson,"include/sway/input/seat.h","include/sway/input/keyboard.h",
        "sway/input/seat.c","sway/input/input-manager.c",
        "tests/autoscroll/ci_v5f_g3o_native_epoch_observer.c"}:
        raise RuntimeError("G4E5_COPY_DIFF "+repr(changed))
    if subprocess.check_output(["git","-C",str(ws),"status","--porcelain"],text=True):
        raise RuntimeError("G4E5_PRODUCTION_CHECKOUT_CHANGED")
    print("PA_G4E5_NATIVE_OVERLAY_PASS",flush=True)
if __name__=="__main__":
    main()
