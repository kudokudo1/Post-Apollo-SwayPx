/* Disposable native verification of actual G4D3 producer, separate from
 * existing G4D2 shadow fixture. No positive native source is fabricated. */
#include <inttypes.h>
#include <string.h>
#include <linux/input-event-codes.h>
#include <libinput.h>
#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_keyboard.h>
#include "sway/input/keyboard.h"
#include "sway/input/seat.h"
#include "log.h"
static int fixture(struct sway_keyboard *k) {
    if (!k || !k->wlr || !wlr_input_device_is_libinput(&k->wlr->base)) return 0;
    struct libinput_device *h=wlr_libinput_get_device_handle(&k->wlr->base);
    const char *n=h?libinput_device_get_name(h):NULL;
    if (!n) return 0;
    if (!strcmp(n,"PA-G3L-SYNTHETIC-KEYBOARD-NOT-HARDWARE")) return 1;
    if (!strcmp(n,"PA-G3M-SECOND-SYNTHETIC-KEYBOARD")) return 2;
    return 0;
}
void pa_g4d3_bind(struct sway_keyboard *k) {
    if (!fixture(k)) return;
    struct sway_seat *s=k->seat_device->sway_seat;
    if (!pa_os_key_bind(&s->pa_g4d3_owner,&k->pa_g4d3_binding,k))
        sway_log(SWAY_ERROR,"PA_G4D3_FAILED bind");
}
void pa_g4d3_key(struct sway_keyboard *k,struct wlr_keyboard_key_event *e) {
    int who=fixture(k);
    if (!who || !e || (e->keycode!=KEY_SCROLLLOCK && e->keycode!=KEY_ESC)) return;
    struct sway_seat *s=k->seat_device->sway_seat;
    struct pa_os_key_observation o={0};
    if (!pa_os_key_observe(&s->pa_g4d3_owner,&k->pa_g4d3_binding,k,e,&o)) {
        sway_log(SWAY_ERROR,"PA_G4D3_FAILED observe");return;
    }
    if (!s->pa_g4d3_controls.seat_epoch &&
        (!pa_os_controls_init(&s->pa_g4d3_controls,o.event.seat_epoch) ||
         !pa_os_scroll_init(&s->pa_g4d3_session,o.event.seat_epoch))) {
        sway_log(SWAY_ERROR,"PA_G4D3_FAILED init");return;
    }
    if (o.event.edge!=PA_OS_KEY_RELEASE || o.fresh)
        (void)pa_os_controls_key(&s->pa_g4d3_controls,&k->pa_g4d3_binding.control,
            &s->pa_g4d3_session,&o.event);
    sway_log(SWAY_INFO,"PA_G4D3_KEY source=%d key=%d press=%d seat=%" PRIu64
        " device=%" PRIu64 " seq=%" PRIu64
        " registered=%d attached=%d native=%d handle=%d synthetic=%d topology=%d virtual=%d fresh=%d unlocked=%d authority=%d mode=%d lock=%d active=%d",
        who,o.event.key,o.event.edge==PA_OS_KEY_PRESS,o.event.seat_epoch,o.event.device_epoch,
        o.event.sequence,o.registered,o.attached,o.native_libinput,o.live_handle,o.known_synthetic,
        o.coherent_tree,o.protocol_virtual,o.fresh,o.unlocked,o.event.source,
        s->pa_g4d3_controls.mode,s->pa_g4d3_controls.lock,s->pa_g4d3_session.active);
}
void pa_g4d3_retire(struct sway_keyboard *k) {
    if (!k || !k->pa_g4d3_binding.control.seat_epoch) return;
    struct pa_os_key_binding *b=&k->pa_g4d3_binding;
    struct sway_seat *s=k->seat_device->sway_seat;
    if (s->pa_g4d3_controls.seat_epoch)
        pa_os_keyboard_controls_retire(&s->pa_g4d3_controls,&b->control,&s->pa_g4d3_session);
    pa_os_key_retire(b);
    sway_log(SWAY_INFO,"PA_G4D3_RETIRE seat=%" PRIu64 " device=%" PRIu64 " retired=%d",
        b->control.seat_epoch,b->control.device_epoch,b->control.retired);
}
