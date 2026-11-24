#!/usr/bin/env python3
"""Observe actual membership in disposable G3O compositor only."""
import os
import subprocess
import ci_v5f_g2_native_button as g2
import ci_v5f_g3o_native_overlay as g3o
import ci_v5f_g3m_native_overlay as g3m

def main():
    tmp,ws=g2.guard_ci()
    if os.environ.get("PA_G3Q_GROUP")!="1":
        raise RuntimeError("BLOCKED_G3Q_NOT_DISPOSABLE_CI")
    g3o.main()
    copy=tmp/g2.COPY_NAME
    key="sway/input/keyboard.c"
    cursor="sway/input/cursor.c"
    meson="sway/meson.build"
    s=(copy/key).read_text()
    s=g3o.patch(s,'#include "sway/input/keyboard.h"\n',
        'void pa_g3q_transition(struct wlr_keyboard *, struct wlr_keyboard_group *, bool, int, bool);\n')
    for line,group,add in (
        ("\twlr_keyboard_group_remove_keyboard(keyboard->wlr->group, keyboard->wlr);\n","wlr_group",False),
        ("\t\t\t\twlr_keyboard_group_add_keyboard(wlr_group, keyboard->wlr);\n","wlr_group",True),
        ("\twlr_keyboard_group_add_keyboard(sway_group->wlr_group, keyboard->wlr);\n","sway_group->wlr_group",True),
    ):
        repl=("{\n"
            f"    int pa_before=wl_list_length(&{group}->devices);\n"
            f"    bool pa_member=keyboard->wlr->group=={group};\n"
            +line+
            f"    pa_g3q_transition(keyboard->wlr,{group},{str(add).lower()},pa_before,pa_member);\n"
            "}\n")
        s=g3m.one(s,line,repl,"G3Q actual membership")
    (copy/key).write_text(s)
    s=(copy/cursor).read_text()
    s=g3o.patch(s,'void pa_g3o_middle(struct wlr_pointer_button_event *event);\n',
        'void pa_g3q_snapshot(struct sway_cursor *, struct wlr_pointer_button_event *);\n')
    s=g3o.patch(s,'\tpa_g3o_middle(event);\n','\tpa_g3q_snapshot(cursor,event);\n')
    (copy/cursor).write_text(s)
    s=(copy/meson).read_text()
    s=g3o.patch(s,"\t'../tests/autoscroll/ci_v5f_g3o_native_epoch_observer.c',\n",
        "\t'../tests/autoscroll/ci_v5f_g3q_native_group_observer.c',\n")
    (copy/meson).write_text(s)
    changed=set(subprocess.check_output(
        ["git","-C",str(copy),"diff","--name-only"],text=True).splitlines())
    if changed!={key,cursor,meson,"sway/input/seat.c","sway/tree/root.c","sway/input/input-manager.c"}:
        raise RuntimeError("G3Q_UNEXPECTED_COPY_DIFF "+repr(changed))
    if subprocess.check_output(["git","-C",str(ws),"status","--porcelain"],text=True):
        raise RuntimeError("G3Q_PRODUCTION_CHECKOUT_CHANGED")
    print("PA_G3Q_DISPOSABLE_GROUP_OVERLAY_PASS",flush=True)

if __name__=="__main__":
    main()
