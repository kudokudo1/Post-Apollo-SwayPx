/* Disposable CI only. Actual group membership, not admission authority. */
#include <stdbool.h>
#include <string.h>
#include <linux/input-event-codes.h>
#include <libinput.h>
#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_keyboard_group.h>
#include <wlr/types/wlr_pointer.h>
#include "sway/input/cursor.h"
#include "sway/input/keyboard.h"
#include "sway/input/seat.h"
#include "log.h"

void pa_g3o_event(const char *kind);

static bool named(struct wlr_input_device *dev, const char *name) {
    if (!dev || !wlr_input_device_is_libinput(dev)) return false;
    struct libinput_device *h = wlr_libinput_get_device_handle(dev);
    const char *actual = h ? libinput_device_get_name(h) : NULL;
    return actual && strcmp(actual,name)==0;
}

/* The caller defers destruction of an empty group until idle. */
void pa_g3q_transition(struct wlr_keyboard *keyboard,
        struct wlr_keyboard_group *group, bool add, int before,
        bool was_member) {
    if (!keyboard || !group) return;
    const bool member = keyboard->group == group;
    const int after = wl_list_length(&group->devices);
    const bool changed = add ? (!was_member && member && after == before+1)
                             : (was_member && !member && after == before-1);
    if (changed) pa_g3o_event(add ? "group_member_added" : "group_member_removed");
    if (named(&keyboard->base,"PA-G3L-SYNTHETIC-KEYBOARD-NOT-HARDWARE")) {
        sway_log(SWAY_INFO,
            "PA_G3Q_PRIMARY_TRANSITION add=%d before=%d after=%d was=%d member=%d changed=%d can_axis=0",
            add,before,after,was_member,member,changed);
    }
}

void pa_g3q_snapshot(struct sway_cursor *cursor,
        struct wlr_pointer_button_event *event) {
    if (!cursor || !cursor->seat || !event || !event->pointer ||
        event->button != BTN_MIDDLE ||
        !named(&event->pointer->base,"PA-G3J-KERNEL-SYNTHETIC-NOT-PHYSICAL")) return;
    int primary=0,member=0,members=0,groups=0;
    struct sway_seat_device *sd;
    wl_list_for_each(sd,&cursor->seat->devices,link) {
        if (!sd->keyboard || !sd->keyboard->wlr ||
            !named(&sd->keyboard->wlr->base,"PA-G3L-SYNTHETIC-KEYBOARD-NOT-HARDWARE")) continue;
        primary++;
        struct wlr_keyboard_group *g=sd->keyboard->wlr->group;
        if (g) {
            member++;
            members=wl_list_length(&g->devices);
        }
    }
    struct sway_keyboard_group *sg;
    wl_list_for_each(sg,&cursor->seat->keyboard_groups,link) groups++;
    sway_log(SWAY_INFO,
        "PA_G3Q_GROUP_SNAPSHOT phase=%d primary=%d member=%d members=%d groups=%d can_consume=0 can_axis=0",
        event->state==WL_POINTER_BUTTON_STATE_PRESSED,primary,member,members,groups);
}
