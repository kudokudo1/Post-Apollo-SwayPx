/* Disposable CI policy prototype. Never establishes positive clearance.
 * An observed disable request is sticky until a future trusted reset
 * protocol exists. LED observations are evidence, not an unlock command.
 * Process-wide conservative diagnostic; not a durable seat policy. */
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <linux/input-event-codes.h>
#include <libinput.h>
#include <xkbcommon/xkbcommon.h>
#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_pointer.h>
#include "sway/input/cursor.h"
#include "sway/input/keyboard.h"
#include "sway/input/seat.h"
#include "log.h"

void pa_g3o_event(const char *kind);
static bool disable_requested;
static uint64_t requests;
static bool poisoned;

static int fixture(struct wlr_input_device *dev) {
    if (!dev || !wlr_input_device_is_libinput(dev)) return 0;
    struct libinput_device *h=wlr_libinput_get_device_handle(dev);
    const char *n=h?libinput_device_get_name(h):NULL;
    if (!n) return 0;
    if (!strcmp(n,"PA-G3L-SYNTHETIC-KEYBOARD-NOT-HARDWARE")) return 1;
    if (!strcmp(n,"PA-G3M-SECOND-SYNTHETIC-KEYBOARD")) return 2;
    if (!strcmp(n,"PA-G3J-KERNEL-SYNTHETIC-NOT-PHYSICAL")) return 3;
    return 0;
}
static int led(struct wlr_keyboard *k) {
    if (!k || !k->xkb_state) return -1;
    int n=xkb_state_led_name_is_active(k->xkb_state,XKB_LED_NAME_SCROLL);
    return n==0 || n==1 ? n : -1;
}
void pa_g3s_key(struct sway_keyboard *k, struct wlr_keyboard_key_event *e) {
    if (!k || !k->wlr || !e || e->keycode!=KEY_SCROLLLOCK) return;
    /* Observe only device-specific native callbacks, not a second delivery
     * through the aggregate wlroots keyboard-group callback. */
    int source=fixture(&k->wlr->base);
    if (source!=1 && source!=2) return;
    const bool press=e->state==WL_KEYBOARD_KEY_STATE_PRESSED;
    if (press) {
        disable_requested=true;
        if (requests==UINT64_MAX) poisoned=true;
        else if (!poisoned) ++requests;
        pa_g3o_event("scroll_lock_disable_request");
    }
    sway_log(SWAY_INFO,"PA_G3S_LOCK_KEY source=%d pressed=%d led=%d requests=%" PRIu64
        " latched=%d poisoned=%d can_axis=0",
        source,press,led(k->wlr),requests,disable_requested,poisoned);
}
void pa_g3s_snapshot(struct sway_cursor *c,struct wlr_pointer_button_event *e) {
    if (!c || !c->seat || !e || !e->pointer || e->button!=BTN_MIDDLE ||
        fixture(&e->pointer->base)!=3) return;
    int total=0,on=0,off=0,unknown=0,untrusted=0;
    struct sway_seat_device *sd;
    wl_list_for_each(sd,&c->seat->devices,link) {
        if (!sd->input_device || !sd->input_device->wlr_device ||
            sd->input_device->wlr_device->type!=WLR_INPUT_DEVICE_KEYBOARD) continue;
        ++total;
        /* Every keyboard, including both synthetic fixtures, is untrusted
         * for authorization. Never substitute a group LED for device evidence. */
        ++untrusted;
        int n=led(sd->keyboard?sd->keyboard->wlr:NULL);
        if (n==1) ++on;
        else if (n==0) ++off;
        else ++unknown;
    }
    const char *policy=(disable_requested || poisoned || on>0) ?
        "BLOCKED" : "UNKNOWN_DENY";
    sway_log(SWAY_INFO,"PA_G3S_LOCK_SNAPSHOT phase=%d total=%d on=%d off=%d unknown=%d "
        "untrusted=%d requests=%" PRIu64 " latched=%d poisoned=%d policy=%s can_consume=0 can_axis=0",
        e->state==WL_POINTER_BUTTON_STATE_PRESSED,total,on,off,unknown,
        untrusted,requests,disable_requested,poisoned,policy);
}
