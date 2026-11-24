#!/usr/bin/env python3
"""Bind shadow test solely to real diagnostic event callbacks in CI COPY."""
import os
import subprocess
import ci_v5f_g2_native_button as g2
import ci_v5f_g3o_native_overlay as g3o
import ci_v5f_g3s_native_overlay as g3s

def main():
    tmp,ws=g2.guard_ci()
    if os.environ.get("PA_G3T_SHADOW")!="1":
        raise RuntimeError("BLOCKED_G3T_NOT_DISPOSABLE_CI")
    g3s.main()
    copy=tmp/g2.COPY_NAME
    observer="tests/autoscroll/ci_v5f_g3o_native_epoch_observer.c"
    meson="sway/meson.build"
    s=(copy/observer).read_text()
    s=g3o.patch(s,'#include "log.h"\n',
        'void pa_g3t_event(const char *, uint64_t, bool);\n'
        'void pa_g3t_middle(bool, uint64_t);\n')
    s=g3o.patch(s,'        captured && (poisoned || first != serial));\n',
        '    pa_g3t_event(kind,serial,poisoned);\n')
    s=g3o.patch(s,'        poisoned || !attached || first != serial);\n',
        '    pa_g3t_middle(event->state == WL_POINTER_BUTTON_STATE_PRESSED,serial);\n')
    (copy/observer).write_text(s)
    s=(copy/meson).read_text()
    s=g3o.patch(s,"\t'../tests/autoscroll/ci_v5f_g3s_native_lock_observer.c',\n",
        "\t'../tests/autoscroll/ci_v5f_g3t_shadow_cancel_observer.c',\n")
    (copy/meson).write_text(s)
    changed=set(subprocess.check_output(
        ["git","-C",str(copy),"diff","--name-only"],text=True).splitlines())
    if changed!={observer,meson,"sway/input/cursor.c","sway/input/keyboard.c",
            "sway/input/seat.c","sway/tree/root.c","sway/input/input-manager.c"}:
        raise RuntimeError("G3T_UNEXPECTED_COPY_DIFF "+repr(changed))
    if subprocess.check_output(["git","-C",str(ws),"status","--porcelain"],text=True):
        raise RuntimeError("G3T_PRODUCTION_CHECKOUT_CHANGED")
    print("PA_G3T_DISPOSABLE_SHADOW_OVERLAY_PASS",flush=True)
if __name__=="__main__":
    main()
