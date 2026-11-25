#!/usr/bin/env python3
"""Disposable seat runtime with explicit test-only source substitutions."""
import os
import subprocess
import ci_v5f_g2_native_button as g2
import ci_v5f_g3o_native_overlay as g3o
import ci_v5f_g3m_native_overlay as g3m
import ci_v5f_g4e5_native_overlay as g4e5
def main():
    tmp,ws=g2.guard_ci()
    if os.environ.get("PA_G4E6_RUNTIME")!="1":
        raise RuntimeError("BLOCKED_G4E6_NOT_DISPOSABLE_CI")
    g4e5.main()
    copy=tmp/g2.COPY_NAME
    root="sway/tree/root.c"
    cursor="sway/input/cursor.c"
    key="sway/input/keyboard.c"
    seat="sway/input/seat.c"
    sh="include/sway/input/seat.h"
    kh="include/sway/input/keyboard.h"
    meson="sway/meson.build"
    runtime="sway/input/post_apollo_autoscroll_os_runtime.c"
    for path in (root,cursor,key,meson):
        s=(copy/path).read_text()
        if "pa_g4e5_" in s:
            s=s.replace("pa_g4e5_","pa_g4e6_")
        s=s.replace("ci_v5f_g4e5_native_middle.c","ci_v5f_g4e6_native_runtime.c")
        (copy/path).write_text(s)
    s=(copy/sh).read_text()
    s=g3o.patch(s,'#include "sway/input/post_apollo_autoscroll_os_keyboard.h"\n',
        '#include "sway/input/post_apollo_autoscroll_os_runtime.h"\n')
    s=g3o.patch(s,'\tstruct pa_os_key_owner pa_g4d3_owner;\n','\tstruct pa_os_runtime pa_g4e6_runtime;\n')
    (copy/sh).write_text(s)
    s=(copy/kh).read_text()
    s=g3o.patch(s,'\tstruct pa_os_key_binding pa_g4d3_binding;\n','\tstruct pa_os_key_binding pa_g4e6_key;\n')
    (copy/kh).write_text(s)
    s=(copy/key).read_text()
    s=g3o.patch(s,'void pa_g4e6_key(struct sway_keyboard *, struct wlr_keyboard_key_event *);\n',
        'void pa_g4e6_bind(struct sway_keyboard *);\nvoid pa_g4e6_retire_key(struct sway_keyboard *);\n')
    s=g3o.patch(s,'\tpa_g4d3_bind(keyboard);\n','\tpa_g4e6_bind(keyboard);\n')
    s=g3o.patch(s,'\tpa_g4d3_retire(keyboard);\n','\tpa_g4e6_retire_key(keyboard);\n')
    (copy/key).write_text(s)
    s=(copy/cursor).read_text()
    s=g3o.patch(s,'bool pa_g4e6_middle(struct sway_cursor *, struct wlr_pointer_button_event *);\n',
        'bool pa_g4e6_motion(struct sway_cursor *, struct wlr_pointer_motion_event *);\n')
    s=g3o.patch(s,'\tcursor_handle_activity_from_device(cursor, &e->pointer->base);\n'
        '\t/* Default OFF: no input consumed. Preserve native compositor pairing. */\n',
        '\tif (pa_g4e6_motion(cursor,e)) return;\n')
    (copy/cursor).write_text(s)
    s=(copy/seat).read_text()
    s=g3o.patch(s,'static void seat_device_destroy(struct sway_seat_device *seat_device) {\n',
        '\tvoid pa_g4e6_detach(struct sway_seat_device *);\n')
    s=g3o.patch(s,'\tsway_keyboard_destroy(seat_device->keyboard);\n','\tpa_g4e6_detach(seat_device);\n')
    s=g3o.patch(s,'static void handle_seat_destroy(struct wl_listener *listener, void *data) {\n',
        '\tvoid pa_g4e6_finish(struct sway_seat *);\n')
    s=g3o.patch(s,'\tstruct sway_seat *seat = wl_container_of(listener, seat, destroy);\n',
        '\tpa_g4e6_finish(seat);\n')
    (copy/seat).write_text(s)
    s=(copy/meson).read_text()
    s=g3o.patch(s,"\t'input/post_apollo_autoscroll_os_ingress.c',\n",
        "\t'input/post_apollo_autoscroll_os_runtime.c',\n")
    (copy/meson).write_text(s)
    # ONLY copied runtime, no bypass in checked-in code. Force exact
    # native denial before granting the one synthetic test fixture.
    s=(copy/runtime).read_text()
    s=g3o.patch(s,'#include <math.h>\n','#include <inttypes.h>\n#include "log.h"\n')
    s=g3o.patch(s,'    if (o.event.edge==PA_OS_KEY_RELEASE && !o.fresh) return;\n',
        '    if (o.event.source!=PA_OS_KEY_UNKNOWN || !o.known_synthetic) {\n'
        '        sway_log(SWAY_ERROR,"PA_G4E6_FAILED native_key_grant");return;\n    }\n'
        '    if (o.event.key==PA_OS_KEY_SCROLL_LOCK) o.event.source=PA_OS_KEY_MANAGED; /* CI ONLY */\n')
    s=g3o.patch(s,'    struct pa_os_ingress_clearance g={0};\n    (void)scene_clear(r,&o.scene,&g);\n',
        '    if (!o.event.known_synthetic || o.event.coherent_kernel_tree) {\n'
        '        sway_log(SWAY_ERROR,"PA_G4E6_FAILED native_pointer_grant");return false;\n    }\n'
        '    o.event.known_synthetic=false;o.event.coherent_kernel_tree=true;\n'
        '    o.event.scroll_method_known=true;o.event.native_button_scroll=false; /* CI ONLY */\n')
    s=g3m.one(s,'        !o->source_candidate || !o->layout_matches ||',
        '        !o->layout_matches ||',"G4E6 fixture scene source")
    s=g3o.patch(s,'    if (!scene_clear(r,&o->scene,&g)) return false;\n',
        '    if (o->source_candidate) {sway_log(SWAY_ERROR,"PA_G4E6_FAILED native_timer_grant");return false;}\n'
        '    o->source_candidate=true; /* CI ONLY */\n')
    s=g3o.patch(s,'    if (b.vertical_pixels || b.horizontal_pixels) wlr_seat_pointer_notify_frame(s->wlr_seat);\n',
        '    if (b.vertical_pixels || b.horizontal_pixels) sway_log(SWAY_INFO,"PA_G4E6_BATCH time=%" PRIu32 '
        '" target=%" PRIu64 " read=%" PRIu64 " event=%" PRIu64 " vertical=%.8f horizontal=%.8f",'
        'stamp,b.target_epoch,b.read_sequence,r->native.event_sequence,b.vertical_pixels,b.horizontal_pixels);\n')
    (copy/runtime).write_text(s)
    changed=set(subprocess.check_output(["git","-C",str(copy),"diff","--name-only"],text=True).splitlines())
    if changed!={root,cursor,key,seat,sh,kh,meson,runtime,
        "sway/input/input-manager.c","tests/autoscroll/ci_v5f_g3o_native_epoch_observer.c"}:
        raise RuntimeError("G4E6_COPY_DIFF "+repr(changed))
    if subprocess.check_output(["git","-C",str(ws),"status","--porcelain"],text=True):
        raise RuntimeError("G4E6_PRODUCTION_CHECKOUT_CHANGED")
    print("PA_G4E6_NATIVE_OVERLAY_PASS",flush=True)
if __name__=="__main__":
    main()
