/* G4E6 unlinked native runtime. Do not add to shipping Meson or call
 * from production before reviewed shared-owner clearance and lifecycle
 * hooks exist. No raw seat/device/surface retained across callbacks. */
#include <math.h>
#include <stddef.h>
#include <linux/input-event-codes.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_seat.h>
#include "sway/input/cursor.h"
#include "sway/input/seat.h"
#include "sway/input/keyboard.h"
#include "sway/input/input-manager.h"
#include "sway/input/post_apollo_autoscroll_os_runtime.h"
#include "sway/input/post_apollo_autoscroll_owner_snapshot.h"
#include "sway/input/post_apollo_autoscroll_source.h"
#include "sway/tree/node.h"
#include "sway/tree/container.h"
#include "sway/tree/view.h"
#include "sway/server.h"

static struct sway_seat *live_seat(uint64_t epoch) {
    if (!epoch || !server.input) return NULL;
    struct sway_seat *s=NULL;
    wl_list_for_each(s,&server.input->seats,link) {
        if (seat_autoscroll_lifetime_epoch(s)==epoch) return s;
    }
    return NULL;
}
static struct sway_input_device *live_device(uint64_t epoch) {
    if (!epoch || !server.input) return NULL;
    struct sway_input_device *d=NULL;
    wl_list_for_each(d,&server.input->devices,link) {
        if (input_manager_autoscroll_pointer_lifetime_epoch(d)==epoch) return d;
    }
    return NULL;
}
static uint64_t callback_device(struct sway_seat *s,struct wlr_input_device *d) {
    if (!s || !d) return 0;
    struct sway_seat_device *sd=NULL;
    wl_list_for_each(sd,&s->devices,link) {
        if (sd->sway_seat==s && sd->input_device && sd->input_device->wlr_device==d) {
            uint64_t epoch=input_manager_autoscroll_pointer_lifetime_epoch(sd->input_device);
            return live_device(epoch)==sd->input_device?epoch:0;
        }
    }
    return 0;
}
static struct sway_seat *resolve(const struct pa_os_scroll_scene *v) {
    struct sway_seat *s=live_seat(v->seat_epoch);
    struct sway_input_device *d=live_device(v->device_epoch);
    struct pa_autoscroll_owner_snapshot snap={0};
    if (!s || !d || !s->cursor || !s->cursor->cursor || !s->wlr_seat ||
        server.session_lock.lock || wlr_seat_pointer_has_grab(s->wlr_seat) ||
        s->cursor->pressed_button_count>1 ||
        !pa_autoscroll_owner_snapshot_read(s->cursor,d->wlr_device,&snap) ||
        snap.seat!=v->seat_epoch || snap.device!=v->device_epoch ||
        snap.target!=v->target_epoch || snap.output!=v->output_epoch) return NULL;
    struct wlr_surface *hit=NULL;
    double x=0,y=0;
    struct sway_node *n=node_at_coords(s,s->cursor->cursor->x,s->cursor->cursor->y,&hit,&x,&y);
    if (!n || n->type!=N_CONTAINER || !n->sway_container ||
        !n->sway_container->view || n->sway_container->view->type!=SWAY_VIEW_XDG_SHELL ||
        !hit || hit!=n->sway_container->view->surface ||
        hit!=s->wlr_seat->pointer_state.focused_surface || !seat_is_input_allowed(s,hit)) return NULL;
    return s;
}
static bool scene_clear(struct pa_os_runtime *r,const struct pa_os_scroll_scene *v,
        struct pa_os_ingress_clearance *g) {
    *g=(struct pa_os_ingress_clearance){0};
    struct sway_seat *s=resolve(v);
    uint64_t revision=pa_os_layout_revision(r->layout);
    if (!s || !revision || !r->clearance || !r->clearance(s,v,g) ||
        g->layout_epoch!=revision || !g->layout_verified ||
        !g->no_conflicting_grab || !g->company_weather_clear) {
        *g=(struct pa_os_ingress_clearance){0};return false;
    }
    return true;
}
void pa_os_runtime_cancel(struct pa_os_runtime *r) {
    if (!r) return;
    pa_os_controller_cancel(&r->controller);
    if (r->timer) {wl_event_source_remove(r->timer);r->timer=NULL;}
}
void pa_os_runtime_invalidate(struct pa_os_runtime *r) {
    if (!r || r->retired) return;
    pa_os_controller_invalidate(&r->controller);
    pa_os_runtime_cancel(r);
}
void pa_os_runtime_finish(struct pa_os_runtime *r) {
    if (!r) return;
    pa_os_runtime_cancel(r);
    r->retired=true;r->layout=NULL;r->clearance=NULL;
    r->controller.pending_middle_device=0;
}
void pa_os_runtime_retire_device(struct pa_os_runtime *r,uint64_t device) {
    if (!r || r->retired) return;
    pa_os_controller_retire_device(&r->controller,device);
    pa_os_runtime_invalidate(r);
}
bool pa_os_runtime_init(struct pa_os_runtime *r,struct sway_seat *s,
        const struct pa_os_layout_owner *layout,pa_os_scene_clearance_reader clear) {
    if (!r || r->retired || r->controller.session.seat_epoch || !s || !layout) return false;
    uint64_t seat=seat_autoscroll_lifetime_epoch(s);
    if (!seat || live_seat(seat)!=s || !pa_os_controller_init(&r->controller,seat)) return false;
    r->native.seat_epoch=seat;
    r->layout=layout;r->clearance=clear;
    return true;
}
bool pa_os_runtime_select(struct pa_os_runtime *r,enum pa_os_scroll_mode mode) {
    if (!r || r->retired || !live_seat(r->native.seat_epoch)) return false;
    pa_os_runtime_cancel(r);
    if (mode!=PA_OS_SCROLL_OFF && !r->clearance) return false;
    return pa_os_controller_select(&r->controller,mode);
}
bool pa_os_runtime_bind_key(struct pa_os_runtime *r,struct pa_os_key_binding *b,struct sway_keyboard *k) {
    return r && !r->retired && k && k->seat_device &&
        live_seat(r->native.seat_epoch)==k->seat_device->sway_seat &&
        pa_os_key_bind(&r->keyboard_owner,b,k);
}
void pa_os_runtime_key(struct pa_os_runtime *r,struct pa_os_key_binding *b,
        struct sway_keyboard *k,const struct wlr_keyboard_key_event *e) {
    if (!r || r->retired || !e || (e->keycode!=KEY_SCROLLLOCK && e->keycode!=KEY_ESC)) return;
    struct pa_os_key_observation o={0};
    if (!pa_os_key_observe(&r->keyboard_owner,b,k,e,&o)) {
        pa_os_runtime_invalidate(r);return;
    }
    if (o.event.edge==PA_OS_KEY_RELEASE && !o.fresh) return;
    (void)pa_os_controller_key(&r->controller,&b->control,&o.event);
    if (!r->controller.session.active) pa_os_runtime_cancel(r);
}
void pa_os_runtime_retire_key(struct pa_os_runtime *r,struct pa_os_key_binding *b) {
    if (!r || r->retired || !b || !b->control.seat_epoch) return;
    pa_os_keyboard_controls_retire(&r->controller.controls,&b->control,&r->controller.session);
    pa_os_key_retire(b);pa_os_runtime_cancel(r);
}
static bool current_scene(struct pa_os_runtime *r,struct pa_os_scene_read *o) {
    const struct pa_os_scroll_scene *bound=&r->controller.session.bound;
    if (!r->controller.session.active ||
        !pa_os_scene_read(&r->native,r->layout,bound->device_epoch,bound->layout_epoch,o) ||
        !o->source_candidate || !o->layout_matches ||
        o->scene.target_epoch!=bound->target_epoch || o->scene.output_epoch!=bound->output_epoch) return false;
    struct pa_os_ingress_clearance g={0};
    if (!scene_clear(r,&o->scene,&g)) return false;
    o->scene.mode=r->controller.controls.mode;
    o->scene.lock=r->controller.controls.lock;
    o->scene.policy_epoch=r->controller.controls.policy_epoch;
    o->scene.selected=o->scene.mode==PA_OS_SCROLL_MANAGED;
    o->scene.layout_verified=g.layout_verified;
    o->scene.no_conflicting_grab=g.no_conflicting_grab;
    o->scene.company_weather_clear=g.company_weather_clear;
    return true;
}
static int timer_tick(void *data) {
    struct pa_os_runtime *r=data;
    struct pa_os_scene_read o={0};
    struct pa_os_scroll_batch b={0};
    if (r->retired || !current_scene(r,&o) ||
        !pa_os_controller_tick(&r->controller,&o.scene,o.source_candidate,o.scene.read_ms,&b)) {
        pa_os_runtime_cancel(r);return 0;
    }
    struct sway_seat *s=resolve(&o.scene);
    if (!s || b.seat_epoch!=o.scene.seat_epoch || b.device_epoch!=o.scene.device_epoch ||
        b.target_epoch!=o.scene.target_epoch || b.output_epoch!=o.scene.output_epoch ||
        b.layout_epoch!=o.scene.layout_epoch || b.policy_epoch!=o.scene.policy_epoch ||
        b.read_sequence!=o.scene.read_sequence || !isfinite(b.vertical_pixels) ||
        !isfinite(b.horizontal_pixels)) {pa_os_runtime_cancel(r);return 0;}
    /* No queued batch, no re-entry between axes/frame. Only this live
     * focused seat. These are delivery calls, not client receipt claims. */
    uint32_t stamp=(uint32_t)o.scene.read_ms;
    if (b.vertical_pixels) wlr_seat_pointer_notify_axis(s->wlr_seat,stamp,WL_POINTER_AXIS_VERTICAL_SCROLL,
        b.vertical_pixels,0,WL_POINTER_AXIS_SOURCE_CONTINUOUS,WL_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL);
    if (b.horizontal_pixels) wlr_seat_pointer_notify_axis(s->wlr_seat,stamp,WL_POINTER_AXIS_HORIZONTAL_SCROLL,
        b.horizontal_pixels,0,WL_POINTER_AXIS_SOURCE_CONTINUOUS,WL_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL);
    if (b.vertical_pixels || b.horizontal_pixels) wlr_seat_pointer_notify_frame(s->wlr_seat);
    if (wl_event_source_timer_update(r->timer,20)<0) pa_os_runtime_cancel(r);
    return 0;
}
bool pa_os_runtime_middle(struct pa_os_runtime *r,struct sway_cursor *c,struct wlr_pointer_button_event *e) {
    if (!r || r->retired || !c || !e || !e->pointer) return false;
    struct sway_seat *s=live_seat(r->native.seat_epoch);
    if (!s || s->cursor!=c) {pa_os_runtime_cancel(r);return false;}
    if (e->button!=BTN_MIDDLE) {pa_os_runtime_cancel(r);return false;}
    if (e->state!=WL_POINTER_BUTTON_STATE_PRESSED && e->state!=WL_POINTER_BUTTON_STATE_RELEASED) {
        pa_os_runtime_cancel(r);return false;
    }
    uint64_t device=callback_device(s,&e->pointer->base);
    struct pa_os_native_observation o={0};
    if (!pa_os_native_observe(&r->native,c,e,&o)) {
        pa_os_runtime_cancel(r);
        /* Even exhausted reader may owe this exact still-live release. */
        if (e->state==WL_POINTER_BUTTON_STATE_RELEASED && device &&
            r->controller.pending_middle_device==device) {
            r->controller.pending_middle_device=0;return true;
        }
        return false;
    }
    struct pa_os_ingress_clearance g={0};
    (void)scene_clear(r,&o.scene,&g);
    bool consumed=pa_os_ingress_middle(&r->controller,&o,&g,c->cursor->x,c->cursor->y);
    if (r->controller.session.active && !r->timer) {
        r->timer=wl_event_loop_add_timer(server.wl_event_loop,timer_tick,r);
        if (!r->timer || wl_event_source_timer_update(r->timer,20)<0) pa_os_runtime_cancel(r);
    } else if (!r->controller.session.active) pa_os_runtime_cancel(r);
    return consumed;
}
bool pa_os_runtime_motion(struct pa_os_runtime *r,struct sway_cursor *c,struct wlr_pointer_motion_event *e) {
    if (!r || r->retired || !r->controller.session.active || !c || !e || !e->pointer) return false;
    struct sway_seat *s=live_seat(r->native.seat_epoch);
    uint64_t device=callback_device(s,&e->pointer->base);
    struct pa_os_scene_read o={0};
    if (!s || s->cursor!=c || !device || device!=r->controller.session.bound.device_epoch ||
        !e->time_msec || r->native.event_sequence>=UINT64_MAX-1 || !current_scene(r,&o) ||
        !pa_os_controller_motion(&r->controller,r->native.seat_epoch,device,
            ++r->native.event_sequence,o.source_candidate,e->delta_x,e->delta_y)) {
        pa_os_runtime_cancel(r);return false;
    }
    return true;
}
