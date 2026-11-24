#!/usr/bin/env python3
"""Only instrument the disposable G3M compositor copy; no production changes."""
import os
import subprocess
import ci_v5f_g2_native_button as g2
import ci_v5f_g3m_native_overlay as g3m

def patch(s, anchor, addition):
    return g3m.one(s, anchor, anchor+addition, "G3O "+anchor)

def main():
    tmp, ws = g2.guard_ci()
    if os.environ.get("RUNNER_ENVIRONMENT") != "github-hosted" or \
            os.environ.get("PA_G3O_NATIVE_EPOCH") != "1":
        raise RuntimeError("BLOCKED_G3O_NOT_DISPOSABLE_CI")
    g3m.main()
    copy = tmp/g2.COPY_NAME
    seat = "sway/input/seat.c"
    key = "sway/input/keyboard.c"
    root = "sway/tree/root.c"
    cursor = "sway/input/cursor.c"
    meson = "sway/meson.build"
    s = (copy/seat).read_text()
    s = patch(s, '#include "sway/input/seat.h"\n',
              'void pa_g3o_event(const char *kind);\n'
              'void pa_g3o_device_event(struct wlr_input_device *dev, bool add);\n')
    s = patch(s, '\twl_list_insert(&seat->devices, &seat_device->link);\n',
              '\tpa_g3o_device_event(input_device->wlr_device, true);\n')
    anchor = '\tseat_device_destroy(seat_device);\n\n\tseat_update_capabilities(seat);'
    s = g3m.one(s, anchor,
        '\tpa_g3o_device_event(input_device->wlr_device, false);\n'+anchor,
        "G3O remove before destruction")
    s = patch(s, '\tpa_autoscroll_lifetime_revoke(&seat->autoscroll_lifetime);\n',
              '\tpa_g3o_event("seat_destroy");\n')
    (copy/seat).write_text(s)
    s = (copy/key).read_text()
    s = 'void pa_g3o_event(const char *kind);\n'+s
    for anchor, kind in (
        ('\twlr_keyboard_group_remove_keyboard(keyboard->wlr->group, keyboard->wlr);\n',
         "group_remove_call"),
        ('\t\t\t\twlr_keyboard_group_add_keyboard(wlr_group, keyboard->wlr);\n',
         "group_add_call"),
        ('\twlr_keyboard_group_add_keyboard(sway_group->wlr_group, keyboard->wlr);\n',
         "group_add_call"),
    ):
        s = patch(s, anchor, '\tpa_g3o_event("'+kind+'");\n')
    (copy/key).write_text(s)
    s = (copy/root).read_text()
    s = patch(s, '#include <wlr/types/wlr_output_layout.h>\n',
              'void pa_g3o_attach_layout(struct wlr_output_layout *layout);\n')
    s = patch(s, '\troot->output_layout = wlr_output_layout_create(wl_display);\n',
              '\tpa_g3o_attach_layout(root->output_layout);\n')
    (copy/root).write_text(s)
    s = (copy/cursor).read_text()
    s = patch(s, 'void pa_g3m_native_multikeyboard_observe(\n'
                 '        struct sway_cursor *cursor,\n'
                 '        struct wlr_pointer_button_event *event);\n',
              'void pa_g3o_middle(struct wlr_pointer_button_event *event);\n')
    s = patch(s, '\tpa_g3m_native_multikeyboard_observe(cursor, event);\n',
              '\tpa_g3o_middle(event);\n')
    (copy/cursor).write_text(s)
    s = (copy/meson).read_text()
    s = patch(s, "\t'../tests/autoscroll/ci_v5f_g3m_native_multikeyboard_observer.c',\n",
              "\t'../tests/autoscroll/ci_v5f_g3o_native_epoch_observer.c',\n")
    (copy/meson).write_text(s)
    changed = set(subprocess.check_output(
        ["git","-C",str(copy),"diff","--name-only"],text=True).splitlines())
    if changed != {seat,key,root,cursor,meson,"sway/input/input-manager.c"}:
        raise RuntimeError("G3O_UNEXPECTED_COPY_DIFF "+repr(changed))
    if subprocess.check_output(
        ["git","-C",str(ws),"status","--porcelain"],text=True):
        raise RuntimeError("G3O_PRODUCTION_CHECKOUT_CHANGED")
    print("PA_G3O_DISPOSABLE_EVENT_OVERLAY_PASS",flush=True)

if __name__ == "__main__":
    main()
