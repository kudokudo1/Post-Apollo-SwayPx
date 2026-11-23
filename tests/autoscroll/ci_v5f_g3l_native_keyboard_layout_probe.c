/*
 * V5F-G3L test-only native state snapshot, never an authorization.
 * Observe actual kernel->libinput->wlroots mouse button events, a separate
 * uinput keyboard's XKB Scroll Lock LED, and live output-layout geometry.
 * Real kernel uinput is SOFTWARE, not physical HID provenance.
 * No known keyboard/ambiguous group => UNKNOWN/DENY. A single observed
 * XKB LED is still not independently trustworthy multi-keyboard policy.
 */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <linux/input-event-codes.h>
#include <libinput.h>
#include <xkbcommon/xkbcommon.h>
#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_pointer.h>
#include "log.h"
#include "sway/input/cursor.h"
#include "sway/input/keyboard.h"
#include "sway/input/seat.h"
#include "sway/input/post_apollo_autoscroll_source.h"
#include "sway/input/post_apollo_autoscroll_origin_policy.h"
#include "sway/tree/root.h"

void pa_g3l_native_lock_layout_observe(
        struct sway_cursor *cursor,
        struct wlr_pointer_button_event *event) {
    if (!cursor || !cursor->seat || !event || !event->pointer ||
            event->button != BTN_MIDDLE) {
        return;
    }
    struct wlr_input_device *mouse=&event->pointer->base;
    if (!wlr_input_device_is_libinput(mouse)) {
        return;
    }
    struct libinput_device *handle=wlr_libinput_get_device_handle(mouse);
    if (!handle || !libinput_device_get_name(handle) ||
            strcmp(libinput_device_get_name(handle),
            "PA-G3J-KERNEL-SYNTHETIC-NOT-PHYSICAL")!=0) {
        return;
    }

    /* Only the exact separate synthetic keyboard registered on this seat
     * may be inspected. Do NOT borrow the seat's current preferred keyboard
     * and claim it represents all physical/synthetic keyboards. */
    int keyboard_count=0,led=-1;
    struct sway_seat_device *sd=NULL;
    wl_list_for_each(sd,&cursor->seat->devices,link) {
        if (!sd->input_device || !sd->keyboard || !sd->keyboard->wlr) {
            continue;
        }
        struct wlr_input_device *device=sd->input_device->wlr_device;
        if (!device || !wlr_input_device_is_libinput(device)) {
            continue;
        }
        struct libinput_device *kh=wlr_libinput_get_device_handle(device);
        if (!kh || !libinput_device_get_name(kh) ||
                strcmp(libinput_device_get_name(kh),
                "PA-G3L-SYNTHETIC-KEYBOARD-NOT-HARDWARE")!=0) {
            continue;
        }
        keyboard_count++;
        if (sd->keyboard->wlr->xkb_state) {
            led=xkb_state_led_name_is_active(
                sd->keyboard->wlr->xkb_state,XKB_LED_NAME_SCROLL);
        }
    }
    if (keyboard_count != 1 || (led != 0 && led != 1)) {
        led=-1; /* UNKNOWN/DENY if not exactly one real registered fixture. */
    }

    /* This is actual live compositor layout geometry, not a complete
     * monotonic scene-generation witness. NEVER use extent as a token. */
    const struct wlr_box *box=root && root->output_layout ?
        wlr_output_layout_get_box(root->output_layout,NULL) : NULL;
    const int x=box ? box->x : -1, y=box ? box->y : -1;
    const int width=box ? box->width : 0;
    const int height=box ? box->height : 0;
    const struct pa_autoscroll_origin_facts facts=
        pa_autoscroll_source_probe(mouse);
    const bool source_candidate=pa_autoscroll_origin_classify(&facts)==
        PA_AUTOSCROLL_ORIGIN_LIBINPUT_CANDIDATE;

    sway_log(SWAY_INFO,
        "PA_G3L_NATIVE_XKB_LAYOUT phase=%d "
        "keyboard_count=%d xkb_scroll_led=%d "
        "layout_x=%d layout_y=%d layout_w=%d layout_h=%d "
        "source_candidate=%d "
        "native_scroll_policy=UNKNOWN_DENY "
        "layout_generation=UNATTESTED physical_hid=NOT_ATTESTED "
        "can_consume=0 can_axis=0",
        event->state==WL_POINTER_BUTTON_STATE_PRESSED,
        keyboard_count,led,x,y,width,height,source_candidate);
}
