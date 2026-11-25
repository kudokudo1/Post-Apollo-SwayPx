/* Disposable test binding. Exact runtime code is used except for explicitly
 * marked positive synthetic source substitutions in the temporary copy.
 * No production provider, no production configuration. */
#include <inttypes.h>
#include <string.h>
#include <libinput.h>
#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_keyboard.h>
#include <linux/input-event-codes.h>
#include "sway/input/cursor.h"
#include "sway/input/seat.h"
#include "sway/input/keyboard.h"
#include "sway/input/post_apollo_autoscroll_os_runtime.h"
#include "sway/input/input-manager.h"
#include "log.h"
static struct pa_os_layout_owner layout;
static unsigned buttons,keys,motions;
static bool named(struct wlr_input_device *d,const char *name) {
    if (!d || !wlr_input_device_is_libinput(d)) return false;
    struct libinput_device *h=wlr_libinput_get_device_handle(d);
    const char *n=h?libinput_device_get_name(h):NULL;
    return n && !strcmp(n,name);
}
void pa_g4e6_attach(struct wlr_output_layout *l) {
    if (!pa_os_layout_attach(&layout,l)) sway_log(SWAY_ERROR,"PA_G4E6_FAILED layout");
}
static bool clearance(struct sway_seat *s,const struct pa_os_scroll_scene *v,
        struct pa_os_ingress_clearance *g) {
    (void)s;(void)v;
    *g=(struct pa_os_ingress_clearance){.layout_epoch=pa_os_layout_revision(&layout),
        .layout_verified=true,.no_conflicting_grab=true,.company_weather_clear=true};
    return g->layout_epoch!=0; /* CI-only, not reviewed Company clearance */
}
static struct pa_os_runtime *runtime(struct sway_seat *s) {
    struct pa_os_runtime *r=&s->pa_g4e6_runtime;
    if (!r->controller.session.seat_epoch &&
        !pa_os_runtime_init(r,s,&layout,clearance)) {
        sway_log(SWAY_ERROR,"PA_G4E6_FAILED init");return NULL;
    }
    return r;
}
void pa_g4e6_bind(struct sway_keyboard *k) {
    if (!k || !k->wlr || !named(&k->wlr->base,"PA-G3L-SYNTHETIC-KEYBOARD-NOT-HARDWARE")) return;
    struct pa_os_runtime *r=runtime(k->seat_device->sway_seat);
    if (!r || !pa_os_runtime_bind_key(r,&k->pa_g4e6_key,k))
        sway_log(SWAY_ERROR,"PA_G4E6_FAILED key_bind");
}
bool pa_g4e6_middle(struct sway_cursor *c,struct wlr_pointer_button_event *e) {
    if (!c || !e || !e->pointer || !named(&e->pointer->base,"PA-G3J-KERNEL-SYNTHETIC-NOT-PHYSICAL")) return false;
    struct pa_os_runtime *r=runtime(c->seat);
    if (!r) return false;
    bool consumed=pa_os_runtime_middle(r,c,e);
    sway_log(SWAY_INFO,"PA_G4E6_BUTTON n=%u press=%d consumed=%d active=%d lock=%d debt=%" PRIu64
        " event=%" PRIu64 " read=%" PRIu64 " timer=%d",
        ++buttons,e->state==WL_POINTER_BUTTON_STATE_PRESSED,consumed,r->controller.session.active,
        r->controller.controls.lock,r->controller.pending_middle_device,r->native.event_sequence,
        r->native.read_sequence,r->timer!=NULL);
    return consumed;
}
bool pa_g4e6_motion(struct sway_cursor *c,struct wlr_pointer_motion_event *e) {
    if (!c || !e || !e->pointer || !named(&e->pointer->base,"PA-G3J-KERNEL-SYNTHETIC-NOT-PHYSICAL")) return false;
    struct pa_os_runtime *r=runtime(c->seat);
    if (!r) return false;
    bool consumed=pa_os_runtime_motion(r,c,e);
    sway_log(SWAY_INFO,"PA_G4E6_MOTION n=%u consumed=%d dx=%.8f dy=%.8f event=%" PRIu64,
        ++motions,consumed,e->delta_x,e->delta_y,r->native.event_sequence);
    return consumed;
}
void pa_g4e6_key(struct sway_keyboard *k,struct wlr_keyboard_key_event *e) {
    if (!k || !k->wlr || !e || (e->keycode!=KEY_ESC && e->keycode!=KEY_SCROLLLOCK) ||
        !named(&k->wlr->base,"PA-G3L-SYNTHETIC-KEYBOARD-NOT-HARDWARE")) return;
    struct pa_os_runtime *r=runtime(k->seat_device->sway_seat);
    if (!r) return;
    if (!keys && !pa_os_runtime_select(r,PA_OS_SCROLL_MANAGED)) {
        sway_log(SWAY_ERROR,"PA_G4E6_FAILED select");return;
    }
    pa_os_runtime_key(r,&k->pa_g4e6_key,k,e);
    sway_log(SWAY_INFO,"PA_G4E6_KEY n=%u code=%u press=%d active=%d lock=%d timer=%d debt=%" PRIu64,
        ++keys,e->keycode,e->state==WL_KEYBOARD_KEY_STATE_PRESSED,r->controller.session.active,
        r->controller.controls.lock,r->timer!=NULL,r->controller.pending_middle_device);
}
void pa_g4e6_retire_key(struct sway_keyboard *k) {
    if (!k || !k->seat_device) return;
    pa_os_runtime_retire_key(&k->seat_device->sway_seat->pa_g4e6_runtime,&k->pa_g4e6_key);
}
void pa_g4e6_finish(struct sway_seat *s) {
    pa_os_runtime_finish(&s->pa_g4e6_runtime);
}
void pa_g4e6_detach(struct sway_seat_device *sd) {
    if (!sd || !sd->sway_seat || !sd->input_device) return;
    struct pa_os_runtime *r=&sd->sway_seat->pa_g4e6_runtime;
    if (r->controller.session.seat_epoch && !r->retired)
        pa_os_runtime_retire_device(r,input_manager_autoscroll_pointer_lifetime_epoch(sd->input_device));
}
