/* G4E5: bind actual native observation to seat-local controller policy.
 * No source promotion, no dispatch and no activation. */
#include <stddef.h>
#include "sway/input/post_apollo_autoscroll_os_ingress.h"
bool pa_os_ingress_middle(struct pa_os_controller *c,
        const struct pa_os_native_observation *o,
        const struct pa_os_ingress_clearance *g,double x,double y) {
    if (!c || !o) {pa_os_controller_cancel(c);return false;}
    struct pa_os_scroll_event e=o->event;
    struct pa_os_scroll_scene v=o->scene;
    if (!c->session.seat_epoch || e.seat_epoch!=c->session.seat_epoch ||
        !e.native_callback || !e.device_epoch) {
        pa_os_controller_cancel(c);return false;
    }
    /* A matching release belongs to the already-consumed press even if
     * source/scene policy has since failed. Never borrow a different ID. */
    if (!e.middle_press && c->pending_middle_device==e.device_epoch)
        return pa_os_controller_middle(c,&e,&v,x,y,v.read_ms);
    v.mode=c->controls.mode;
    v.lock=c->controls.lock;
    e.policy_epoch=v.policy_epoch=c->controls.policy_epoch;
    v.selected=c->controls.mode==PA_OS_SCROLL_MANAGED && !c->controls.exhausted;
    e.layout_epoch=v.layout_epoch=g?g->layout_epoch:0;
    v.layout_verified=g && g->layout_verified;
    v.no_conflicting_grab=g && g->no_conflicting_grab;
    v.company_weather_clear=g && g->company_weather_clear;
    v.device_attached=v.device_attached && o->registered && o->attached;
    /* Source eligibility remains exactly the native observation. */
    return pa_os_controller_middle(c,&e,&v,x,y,v.read_ms);
}
