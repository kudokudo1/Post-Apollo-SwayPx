#!/usr/bin/env python3
"""Add disposable seat lifetime instrumentation to G3Q overlay only."""
import os
import subprocess
import ci_v5f_g2_native_button as g2
import ci_v5f_g3o_native_overlay as g3o
import ci_v5f_g3q_native_overlay as g3q
import ci_v5f_g3m_native_overlay as g3m

def main():
    tmp,ws=g2.guard_ci()
    if os.environ.get("PA_G3R_SEAT")!="1":
        raise RuntimeError("BLOCKED_G3R_NOT_DISPOSABLE_CI")
    g3q.main()
    copy=tmp/g2.COPY_NAME
    seat="sway/input/seat.c"
    cursor="sway/input/cursor.c"
    meson="sway/meson.build"
    s=(copy/seat).read_text()
    s=g3o.patch(s,'#include "sway/input/seat.h"\n',
        'void pa_g3r_created(struct sway_seat *);\n'
        'void pa_g3r_revoked(struct sway_seat *, uint64_t);\n')
    s=g3m.one(s,'\tseatop_begin_default(seat);\n\n\treturn seat;',
        '\tseatop_begin_default(seat);\n\tpa_g3r_created(seat);\n\n\treturn seat;',
        "G3R successful seat creation")
    anchor='\tpa_autoscroll_lifetime_revoke(&seat->autoscroll_lifetime);\n'
    s=g3m.one(s,anchor,
        '\tuint64_t pa_g3r_old_epoch=seat_autoscroll_lifetime_epoch(seat);\n'
        +anchor+'\tpa_g3r_revoked(seat,pa_g3r_old_epoch);\n',
        "G3R revoke before destruction")
    (copy/seat).write_text(s)
    s=(copy/cursor).read_text()
    s=g3o.patch(s,'void pa_g3q_snapshot(struct sway_cursor *, struct wlr_pointer_button_event *);\n',
        'void pa_g3r_snapshot(struct sway_cursor *, struct wlr_pointer_button_event *);\n')
    s=g3o.patch(s,'\tpa_g3q_snapshot(cursor,event);\n','\tpa_g3r_snapshot(cursor,event);\n')
    (copy/cursor).write_text(s)
    s=(copy/meson).read_text()
    s=g3o.patch(s,"\t'../tests/autoscroll/ci_v5f_g3q_native_group_observer.c',\n",
        "\t'../tests/autoscroll/ci_v5f_g3r_native_seat_observer.c',\n")
    (copy/meson).write_text(s)
    changed=set(subprocess.check_output(
        ["git","-C",str(copy),"diff","--name-only"],text=True).splitlines())
    if changed!={seat,cursor,meson,"sway/input/keyboard.c","sway/tree/root.c","sway/input/input-manager.c"}:
        raise RuntimeError("G3R_UNEXPECTED_COPY_DIFF "+repr(changed))
    if subprocess.check_output(["git","-C",str(ws),"status","--porcelain"],text=True):
        raise RuntimeError("G3R_PRODUCTION_CHECKOUT_CHANGED")
    print("PA_G3R_DISPOSABLE_SEAT_OVERLAY_PASS",flush=True)

if __name__=="__main__":
    main()
