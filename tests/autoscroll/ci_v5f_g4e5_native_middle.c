/* Disposable native-only integration. Explicit positive fixture copies
 * source eligibility; the actual native observation is never modified.
 * No real native grant, axis output, or shipping activation. */
#include <inttypes.h>
#include <string.h>
#include <linux/input-event-codes.h>
#include <libinput.h>
#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_seat.h>
#include "sway/input/cursor.h"
#include "sway/input/seat.h"
#include "sway/input/keyboard.h"
#include "sway/input/post_apollo_autoscroll_os_ingress.h"
#include "sway/input/post_apollo_autoscroll_os_scene.h"
#include "log.h"
static struct pa_os_controller controller,negative;
static struct pa_os_native_slot native;
static struct pa_os_layout_owner layout;
static struct pa_os_keyboard_controls key_slot,negative_key;
static unsigned buttons,keys;
static bool failed;
static void fail(const char *why) {
    failed=true;pa_os_controller_cancel(&controller);
    sway_log(SWAY_ERROR,"PA_G4E5_FAILED %s",why);
}
static bool named(struct wlr_input_device *d,const char *name) {
    if (!d || !wlr_input_device_is_libinput(d)) return false;
    struct libinput_device *h=wlr_libinput_get_device_handle(d);
    const char *n=h?libinput_device_get_name(h):NULL;
    return n && !strcmp(n,name);
}
void pa_g4e5_attach(struct wlr_output_layout *l) {
    if (!pa_os_layout_attach(&layout,l)) fail("layout");
}
bool pa_g4e5_middle(struct sway_cursor *c,struct wlr_pointer_button_event *e) {
    if (failed || !c || !c->seat || !e || !e->pointer || e->button!=BTN_MIDDLE ||
        !named(&e->pointer->base,"PA-G3J-KERNEL-SYNTHETIC-NOT-PHYSICAL")) return false;
    struct pa_os_native_observation o={0};
    if (!pa_os_native_observe(&native,c,e,&o) || !o.registered || !o.attached ||
        !o.event.known_synthetic || o.event.coherent_kernel_tree ||
        o.scene.mode!=PA_OS_SCROLL_OFF || o.scene.company_weather_clear ||
        !o.event.observed_ms || !o.scene.owners_live || !o.scene.focused_root_matches) {
        fail("native_observation");return false;
    }
    if (!controller.session.seat_epoch &&
        (!pa_os_controller_init(&controller,o.scene.seat_epoch) ||
         !pa_os_controller_init(&negative,o.scene.seat_epoch))) {
        fail("init");return false;
    }
    struct pa_os_ingress_clearance g={.layout_epoch=pa_os_layout_revision(&layout),
        .layout_verified=true,.no_conflicting_grab=!wlr_seat_pointer_has_grab(c->seat->wlr_seat),
        .company_weather_clear=true}; /* explicit fixture clearance only */
    bool denied=pa_os_ingress_middle(&negative,&o,&g,c->cursor->x,c->cursor->y);
    if (denied || negative.session.active || negative.pending_middle_device) {
        fail("synthetic_native_admitted");return false;
    }
    struct pa_os_native_observation test=o;
    /* Only this CI copy receives invented source authority. IDs, actual
     * edge, sequence, event timestamp, current target and focus are real. */
    test.event.known_synthetic=false;
    test.event.coherent_kernel_tree=true;
    test.event.scroll_method_known=true;
    test.event.native_button_scroll=false;
    bool consumed=pa_os_ingress_middle(&controller,&test,&g,c->cursor->x,c->cursor->y);
    ++buttons;
    sway_log(SWAY_INFO,"PA_G4E5_BUTTON n=%u press=%d seq=%" PRIu64
        " device=%" PRIu64 " seat=%" PRIu64 " consumed=%d active=%d lock=%d debt=%" PRIu64
        " native_grant=0 fixture=1",
        buttons,o.event.middle_press,o.event.sequence,o.event.device_epoch,o.event.seat_epoch,
        consumed,controller.session.active,controller.controls.lock,controller.pending_middle_device);
    return consumed;
}
void pa_g4e5_key(struct sway_keyboard *k,struct wlr_keyboard_key_event *e) {
    if (failed || !k || !k->wlr || !e || e->keycode!=KEY_SCROLLLOCK ||
        !named(&k->wlr->base,"PA-G3L-SYNTHETIC-KEYBOARD-NOT-HARDWARE")) return;
    if (!controller.session.seat_epoch || !k->seat_device ||
        k->pa_g4d3_binding.control.seat_epoch!=controller.session.seat_epoch ||
        !k->pa_g4d3_binding.control.device_epoch) {fail("key_owner");return;}
    uint64_t id=k->pa_g4d3_binding.control.device_epoch;
    if (!key_slot.seat_epoch) {
        if (!pa_os_keyboard_controls_init(&key_slot,controller.session.seat_epoch,id) ||
            !pa_os_keyboard_controls_init(&negative_key,controller.session.seat_epoch,id) ||
            !pa_os_controller_select(&controller,PA_OS_SCROLL_MANAGED) ||
            !pa_os_controller_select(&negative,PA_OS_SCROLL_MANAGED)) {
            fail("key_init");return;
        }
    }
    ++keys;
    struct pa_os_control_event key={.seat_epoch=controller.session.seat_epoch,.device_epoch=id,
        .sequence=keys,.key=PA_OS_KEY_SCROLL_LOCK,
        .edge=e->state==WL_KEYBOARD_KEY_STATE_PRESSED?PA_OS_KEY_PRESS:PA_OS_KEY_RELEASE,
        .source=PA_OS_KEY_MANAGED}; /* fixture-only arming */
    (void)pa_os_controller_key(&negative,&negative_key,&key); /* arm negative model too */
    if (keys>=3) key.source=PA_OS_KEY_UNKNOWN; /* disable-only native request */
    (void)pa_os_controller_key(&controller,&key_slot,&key);
    sway_log(SWAY_INFO,"PA_G4E5_KEY n=%u press=%d active=%d lock=%d debt=%" PRIu64
        " source=%d fixture=1",
        keys,key.edge==PA_OS_KEY_PRESS,controller.session.active,controller.controls.lock,
        controller.pending_middle_device,key.source);
}
