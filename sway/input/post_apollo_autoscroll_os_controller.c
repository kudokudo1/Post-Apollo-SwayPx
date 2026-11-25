/* G4E2: combine actual G4C1/G4D1 contracts with existing scroll math.
 * Never pass OS-managed source evidence into the legacy physical adapter. */
#include <stddef.h>
#include <math.h>
#include <string.h>
#include "sway/input/post_apollo_autoscroll_os_controller.h"

void pa_os_controller_cancel(struct pa_os_controller *c) {
    if (!c) return;
    pa_os_scroll_cancel(&c->session);
    if (c->engine.active) pa_autoscroll_cancel(&c->engine);
}
bool pa_os_controller_init(struct pa_os_controller *c,uint64_t seat) {
    if (!c || c->session.seat_epoch || c->controls.seat_epoch || !seat) return false;
    memset(c,0,sizeof(*c));
    pa_autoscroll_init(&c->engine);
    return pa_os_controls_init(&c->controls,seat) && pa_os_scroll_init(&c->session,seat);
}
static bool policy_matches(const struct pa_os_controller *c,const struct pa_os_scroll_scene *v) {
    return c && v && !c->exhausted && !c->controls.exhausted &&
        c->controls.seat_epoch==c->session.seat_epoch &&
        v->seat_epoch==c->controls.seat_epoch &&
        c->controls.mode==PA_OS_SCROLL_MANAGED && c->controls.lock==PA_OS_SCROLL_LOCK_ARMED &&
        v->mode==c->controls.mode && v->lock==c->controls.lock &&
        v->policy_epoch==c->controls.policy_epoch;
}
static void synchronize(struct pa_os_controller *c) {
    if (!c->session.active || !c->engine.active) pa_os_controller_cancel(c);
}
bool pa_os_controller_select(struct pa_os_controller *c,enum pa_os_scroll_mode mode) {
    if (!c || c->exhausted) return false;
    bool ok=pa_os_controls_select(&c->controls,&c->session,mode);
    synchronize(c);
    return ok;
}
void pa_os_controller_invalidate(struct pa_os_controller *c) {
    if (!c) return;
    pa_os_controls_invalidate(&c->controls,&c->session);
    pa_os_controller_cancel(c);
}
bool pa_os_controller_key(struct pa_os_controller *c,struct pa_os_keyboard_controls *k,
        const struct pa_os_control_event *e) {
    if (!c || c->exhausted) return false;
    bool changed=pa_os_controls_key(&c->controls,k,&c->session,e);
    synchronize(c);
    return changed;
}
static bool spend_input(struct pa_os_controller *c,uint64_t seat,uint64_t seq) {
    if (!c || !c->session.seat_epoch || seat!=c->session.seat_epoch ||
        !seq || seq<=c->input_sequence || c->exhausted) return false;
    c->input_sequence=seq;
    if (seq==UINT64_MAX) {
        c->exhausted=true;
        pa_os_controller_invalidate(c);
        return false;
    }
    return true;
}
static void spend_scene(struct pa_os_controller *c,const struct pa_os_scroll_scene *v) {
    if (v && v->seat_epoch==c->session.seat_epoch &&
        v->read_sequence>c->session.last_read_sequence)
        c->session.last_read_sequence=v->read_sequence;
}
bool pa_os_controller_middle(struct pa_os_controller *c,const struct pa_os_scroll_event *e,
        const struct pa_os_scroll_scene *v,double x,double y,uint64_t now) {
    if (!c || !e || !e->native_callback || !e->device_epoch ||
        e->seat_epoch!=c->session.seat_epoch) return false;
    /* Matching release debt is independent of source eligibility, policy
     * and even sequence exhaustion. The caller is the real native event
     * owner, not a client. No debt can be transferred to a new device. */
    if (!e->middle_press && e->device_epoch==c->pending_middle_device) {
        c->pending_middle_device=0;
        (void)spend_input(c,e->seat_epoch,e->sequence);
        spend_scene(c,v);
        return true;
    }
    if (!spend_input(c,e->seat_epoch,e->sequence)) return false;
    if (!e->middle_press) {spend_scene(c,v);return false;}
    if (c->pending_middle_device) {
        bool same=e->device_epoch==c->pending_middle_device;
        spend_scene(c,v);
        if (!same) pa_os_controller_cancel(c);
        return same; /* duplicate held press, no new session */
    }
    bool was_active=c->session.active && c->engine.active;
    uint64_t previous_device=c->session.bound.device_epoch;
    pa_os_controller_cancel(c);
    if ((was_active && previous_device!=e->device_epoch) || !policy_matches(c,v)) {
        spend_scene(c,v);return false;
    }
    /* Revalidate a fresh second press before consuming it as a stop.
     * Both start and stop spend event and scene sequence in G4C1. */
    if (!pa_os_scroll_begin(&c->session,e,v,now)) return false;
    if (was_active) {
        pa_os_controller_cancel(c);
        c->pending_middle_device=e->device_epoch;
        return true;
    }
    if (!pa_autoscroll_begin(&c->engine,true,true,v->target_epoch,v->layout_epoch,x,y,now)) {
        pa_os_controller_cancel(c);return false;
    }
    c->pending_middle_device=e->device_epoch;
    return true;
}
bool pa_os_controller_motion(struct pa_os_controller *c,uint64_t seat,uint64_t device,
        uint64_t seq,bool candidate,double dx,double dy) {
    if (!c || seat!=c->session.seat_epoch) return false;
    if (!spend_input(c,seat,seq)) return false;
    if (!c->session.active || !c->engine.active) {synchronize(c);return false;}
    if (!candidate || device!=c->session.bound.device_epoch ||
        !policy_matches(c,&c->session.bound) || !isfinite(dx) || !isfinite(dy)) {
        pa_os_controller_cancel(c);return false;
    }
    pa_autoscroll_move(&c->engine,dx,dy);
    synchronize(c);
    return c->session.active;
}
bool pa_os_controller_tick(struct pa_os_controller *c,const struct pa_os_scroll_scene *v,
        bool candidate,uint64_t now,struct pa_os_scroll_batch *out) {
    if (!out) {pa_os_controller_cancel(c);return false;}
    memset(out,0,sizeof(*out));
    if (!c) return false;
    if (!c->engine.active || !candidate || !policy_matches(c,v)) {
        spend_scene(c,v);
        pa_os_controller_cancel(c);return false;
    }
    if (!pa_os_scroll_tick(&c->session,v,now)) {
        pa_os_controller_cancel(c);return false;
    }
    struct pa_autoscroll_delta d=pa_autoscroll_tick(&c->engine,v->target_epoch,v->layout_epoch,now);
    if (!d.active || !isfinite(d.horizontal_pixels) || !isfinite(d.vertical_pixels)) {
        pa_os_controller_cancel(c);return false;
    }
    *out=(struct pa_os_scroll_batch){.seat_epoch=v->seat_epoch,.device_epoch=v->device_epoch,
        .target_epoch=v->target_epoch,.output_epoch=v->output_epoch,
        .layout_epoch=v->layout_epoch,.policy_epoch=v->policy_epoch,.read_sequence=v->read_sequence,
        .horizontal_pixels=d.horizontal_pixels,.vertical_pixels=d.vertical_pixels};
    return true;
}
void pa_os_controller_retire_device(struct pa_os_controller *c,uint64_t device) {
    if (!c || !device) return;
    if (c->session.bound.device_epoch==device) pa_os_controller_cancel(c);
    /* No real release can arrive from a retired owner epoch. */
    if (c->pending_middle_device==device) c->pending_middle_device=0;
}
