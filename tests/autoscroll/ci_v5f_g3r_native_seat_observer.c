/* CI-only read of native compositor seat lifetime. No admission authority. */
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include <linux/input-event-codes.h>
#include <libinput.h>
#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_pointer.h>
#include "sway/input/cursor.h"
#include "sway/input/seat.h"
#include "log.h"

void pa_g3o_event(const char *kind);

void pa_g3r_created(struct sway_seat *seat) {
    if (!seat || !seat->wlr_seat) return;
    pa_g3o_event("seat_created");
    sway_log(SWAY_INFO, "PA_G3R_SEAT_CREATED name=%s epoch=%" PRIu64 " devices=%d can_axis=0",
        seat->wlr_seat->name,seat_autoscroll_lifetime_epoch(seat),
        wl_list_length(&seat->devices));
}
void pa_g3r_revoked(struct sway_seat *seat, uint64_t before) {
    if (!seat || !seat->wlr_seat) return;
    sway_log(SWAY_INFO, "PA_G3R_SEAT_REVOKED name=%s before=%" PRIu64
        " after=%" PRIu64 " devices=%d can_axis=0",
        seat->wlr_seat->name,before,seat_autoscroll_lifetime_epoch(seat),
        wl_list_length(&seat->devices));
}
void pa_g3r_snapshot(struct sway_cursor *cursor,
        struct wlr_pointer_button_event *event) {
    if (!cursor || !cursor->seat || !event || !event->pointer ||
        event->button!=BTN_MIDDLE) return;
    struct wlr_input_device *dev=&event->pointer->base;
    if (!wlr_input_device_is_libinput(dev)) return;
    struct libinput_device *h=wlr_libinput_get_device_handle(dev);
    const char *name=h?libinput_device_get_name(h):NULL;
    if (!name || strcmp(name,"PA-G3J-KERNEL-SYNTHETIC-NOT-PHYSICAL")) return;
    sway_log(SWAY_INFO,"PA_G3R_CURSOR_SEAT phase=%d name=%s epoch=%" PRIu64
        " devices=%d can_consume=0 can_axis=0",
        event->state==WL_POINTER_BUTTON_STATE_PRESSED,
        cursor->seat->wlr_seat->name,seat_autoscroll_lifetime_epoch(cursor->seat),
        wl_list_length(&cursor->seat->devices));
}
