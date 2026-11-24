/*
 * V5F-G3M CI-only REAL native seat keyboard membership observation.
 * Never mint globally unlocked Scroll Lock or physical-origin witness.
 * A keyboard advertised as USB through uinput is a SOFTWARE fixture.
 * Missing/unexpected/grouped/conflicting keyboards => UNKNOWN/DENY;
 * even two matching LED-OFF reports do not prove global lock clearance.
 */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <linux/input-event-codes.h>
#include <libinput.h>
#include <xkbcommon/xkbcommon.h>
#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_keyboard.h>
#include "log.h"
#include "sway/input/cursor.h"
#include "sway/input/keyboard.h"
#include "sway/input/seat.h"

static const char *const pointer_name =
    "PA-G3J-KERNEL-SYNTHETIC-NOT-PHYSICAL";
static const char *const primary_name =
    "PA-G3L-SYNTHETIC-KEYBOARD-NOT-HARDWARE";
static const char *const secondary_name =
    "PA-G3M-SECOND-SYNTHETIC-KEYBOARD";

static bool device_named(struct wlr_input_device *dev,const char *name) {
    if (!dev || !wlr_input_device_is_libinput(dev)) {
        return false;
    }
    struct libinput_device *handle=wlr_libinput_get_device_handle(dev);
    const char *actual=handle ? libinput_device_get_name(handle) : NULL;
    return actual && strcmp(actual,name)==0;
}

void pa_g3m_native_multikeyboard_observe(
        struct sway_cursor *cursor,
        struct wlr_pointer_button_event *event) {
    if (!cursor || !cursor->seat || !event || !event->pointer ||
            event->button!=BTN_MIDDLE ||
            !device_named(&event->pointer->base,pointer_name)) {
        return;
    }
    int keyboards=0,primary=0,secondary=0,unexpected=0;
    int primary_led=-1,secondary_led=-1;
    struct sway_seat_device *sd=NULL;
    wl_list_for_each(sd,&cursor->seat->devices,link) {
        if(!sd->keyboard || !sd->input_device ||
                !sd->input_device->wlr_device ||
                !sd->keyboard->wlr) {
            continue;
        }
        keyboards++;
        struct wlr_input_device *dev=sd->input_device->wlr_device;
        int led=-1;
        if (sd->keyboard->wlr->xkb_state) {
            led=xkb_state_led_name_is_active(
                sd->keyboard->wlr->xkb_state,
                XKB_LED_NAME_SCROLL);
        }
        if(device_named(dev,primary_name)) {
            primary++;
            primary_led=led;
        } else if(device_named(dev,secondary_name)) {
            secondary++;
            secondary_led=led;
        } else {
            unexpected++;
        }
    }
    int groups=0;
    struct sway_keyboard_group *group=NULL;
    wl_list_for_each(group,&cursor->seat->keyboard_groups,link) {
        groups++;
    }
    const bool known_fixture_topology=primary==1 &&
        secondary<=1 && unexpected==0 &&
        keyboards==primary+secondary && groups==0;
    const bool leds_observed=(primary_led==0 || primary_led==1) &&
        (secondary==0 || secondary_led==0 || secondary_led==1);
    /* Diagnostic topology does not authorize global keyboard policy.
     * Native lock reports can be remapped, stale or contradictory. */
    const bool global_lock_clear_authorized=false;
    sway_log(SWAY_INFO,
        "PA_G3M_NATIVE_MULTIKEYBOARD "
        "phase=%d keyboards=%d primary=%d second=%d unexpected=%d "
        "groups=%d primary_led=%d second_led=%d "
        "known_test_topology=%d leds_observed=%d "
        "global_lock_clear_authorized=%d "
        "lock_authority=UNKNOWN_DENY origin=SOFTWARE_DENIED "
        "can_consume=0 can_axis=0",
        event->state==WL_POINTER_BUTTON_STATE_PRESSED,
        keyboards,primary,secondary,unexpected,groups,
        primary_led,secondary_led,known_fixture_topology,
        leds_observed,global_lock_clear_authorized);
}

/* An actual SwayPX keyboard callback for the 2nd device, not the
 * actor's claim that a kernel evdev write succeeded. */
void pa_g3m_native_second_keyboard_key_observe(
        struct sway_keyboard *keyboard,
        struct wlr_keyboard_key_event *event) {
    if(!keyboard || !keyboard->seat_device ||
            !keyboard->seat_device->input_device || !event ||
            event->keycode!=KEY_SCROLLLOCK ||
            !device_named(
                keyboard->seat_device->input_device->wlr_device,
                secondary_name)) {
        return;
    }
    int led=-1;
    if(keyboard->wlr && keyboard->wlr->xkb_state) {
        led=xkb_state_led_name_is_active(
            keyboard->wlr->xkb_state,XKB_LED_NAME_SCROLL);
    }
    sway_log(SWAY_INFO,
        "PA_G3M_SECOND_NATIVE_SCROLLLOCK "
        "pressed=%d keycode=%u xkb_led=%d "
        "origin=SOFTWARE_DENIED lock_authority=UNKNOWN_DENY can_axis=0",
        event->state==WL_KEYBOARD_KEY_STATE_PRESSED,
        event->keycode,led);
}
