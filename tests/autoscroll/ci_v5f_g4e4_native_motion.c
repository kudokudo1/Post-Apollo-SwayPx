/* Disposable CI only. Invented admission; genuine native relative motion and Escape.
 * Failure watchdog is NOT a successful frame cap. Never ship this module. */
#include <inttypes.h>
#include <math.h>
#include <string.h>
#include <linux/input-event-codes.h>
#include <libinput.h>
#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_keyboard.h>
#include "sway/input/keyboard.h"
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_cursor.h>
#include "sway/input/cursor.h"
#include "sway/input/seat.h"
#include "sway/input/input-manager.h"
#include "sway/input/post_apollo_autoscroll_os_scene.h"
#include "sway/input/post_apollo_autoscroll_os_controller.h"
#include "sway/input/post_apollo_autoscroll_owner_snapshot.h"
#include "sway/server.h"
#include "sway/tree/node.h"
#include "sway/tree/container.h"
#include "sway/tree/view.h"
#include "log.h"

static struct pa_os_controller controller;
static struct pa_os_keyboard_controls fixture_keyboard;
static unsigned sent,post_cancel_reads,motions;
static bool escaped;
static bool started,terminal;
static uint64_t original_event;
static void failure(const char *reason) {
    pa_os_controller_cancel(&controller);
    terminal=true;
    sway_log(SWAY_ERROR,"PA_G4E4_FAILED reason=%s",reason);
}
static struct sway_seat *resolve(const struct pa_os_scroll_scene *v) {
    if (!v || !server.input) return NULL;
    struct sway_seat *s=NULL,*it=NULL;
    wl_list_for_each(it,&server.input->seats,link) {
        if (seat_autoscroll_lifetime_epoch(it)==v->seat_epoch) {s=it;break;}
    }
    if (!s || !s->wlr_seat || !s->cursor || !s->cursor->cursor ||
        server.session_lock.lock || wlr_seat_pointer_has_grab(s->wlr_seat)) return NULL;
    struct sway_input_device *d=NULL,*di=NULL;
    wl_list_for_each(di,&server.input->devices,link) {
        if (input_manager_autoscroll_pointer_lifetime_epoch(di)==v->device_epoch) {d=di;break;}
    }
    struct pa_autoscroll_owner_snapshot snap={0};
    if (!d || !pa_autoscroll_owner_snapshot_read(s->cursor,d->wlr_device,&snap) ||
        snap.seat!=v->seat_epoch || snap.device!=v->device_epoch ||
        snap.target!=v->target_epoch || snap.output!=v->output_epoch) return NULL;
    struct wlr_surface *hit=NULL;
    double sx=0,sy=0;
    struct sway_node *n=node_at_coords(s,s->cursor->cursor->x,s->cursor->cursor->y,&hit,&sx,&sy);
    if (!n || n->type!=N_CONTAINER || !n->sway_container ||
        !n->sway_container->view || n->sway_container->view->type!=SWAY_VIEW_XDG_SHELL ||
        !hit || hit!=s->wlr_seat->pointer_state.focused_surface ||
        hit!=n->sway_container->view->surface) return NULL;
    return s;
}
static struct pa_os_scroll_scene fixture_scene(const struct pa_os_scroll_scene *native) {
    struct pa_os_scroll_scene v=*native;
    /* Explicit positive fixture policy. Never mutate or promote native
     * source evidence, never present these values as production clearance. */
    v.mode=controller.controls.mode;
    v.lock=controller.controls.lock;
    v.policy_epoch=controller.controls.policy_epoch;
    v.selected=true;
    v.layout_verified=true; /* test fixture; actual layout revision observed */
    v.no_conflicting_grab=true; /* actual pointer grab also independently checked */
    v.company_weather_clear=true; /* test compositor only, no Company actor */
    return v;
}
void pa_g4e4_tick(const struct pa_os_scene_read *o,const struct pa_os_native_slot *slot) {
    if (terminal || !o || !slot) return;
    const struct pa_os_scroll_scene *v=&o->scene;
    if (o->source_candidate || v->mode!=PA_OS_SCROLL_OFF || v->company_weather_clear) {
        failure("native_policy_promoted");return;
    }
    if (!started) {
        if (!o->layout_matches || !v->owners_live || !v->focused_root_matches || !resolve(v) ||
            !pa_os_controller_init(&controller,v->seat_epoch) ||
            !pa_os_controller_select(&controller,PA_OS_SCROLL_MANAGED) ||
            !pa_os_keyboard_controls_init(&fixture_keyboard,v->seat_epoch,UINT64_MAX-1)) {
            failure("fixture_init");return;
        }
        struct pa_os_control_event key={.seat_epoch=v->seat_epoch,
            .device_epoch=fixture_keyboard.device_epoch,.sequence=1,.key=PA_OS_KEY_SCROLL_LOCK,
            .edge=PA_OS_KEY_PRESS,.source=PA_OS_KEY_MANAGED};
        if (!pa_os_controller_key(&controller,&fixture_keyboard,&key)) {
            failure("fixture_arm");return;
        }
        /* Invented fixture press, not derived from a native callback's
         * authority flags. Only current real owner epochs are used. */
        struct pa_os_scroll_scene scene=fixture_scene(v);
        struct pa_os_scroll_event fake={.sequence=1,.observed_ms=v->read_ms,
            .seat_epoch=v->seat_epoch,.device_epoch=v->device_epoch,
            .layout_epoch=v->layout_epoch,.policy_epoch=scene.policy_epoch,
            .native_callback=true,.middle_press=true,.libinput_pointer=true,
            .live_handle=true,.coherent_kernel_tree=true,.scroll_method_known=true};
        if (!pa_os_controller_middle(&controller,&fake,&scene,100,100,v->read_ms)) {
            failure("fixture_start");return;
        }
        original_event=slot->event_sequence;
        started=true;
        sway_log(SWAY_INFO,"PA_G4E4_FIXTURE_START seat=%" PRIu64 " device=%" PRIu64
            " target=%" PRIu64 " output=%" PRIu64 " layout=%" PRIu64
            " native_event=%" PRIu64 " grant=FABRICATED_CI_ONLY native_candidate=0",
            v->seat_epoch,v->device_epoch,v->target_epoch,v->output_epoch,v->layout_epoch,original_event);
        return;
    }
    if (slot->event_sequence!=original_event) {failure("invented_native_event");return;}
    struct pa_os_scroll_scene scene=fixture_scene(v);
    struct pa_os_scroll_batch b={0};
    struct sway_seat *seat=resolve(v);
    if (!seat || !o->layout_matches) {failure("unexpected_owner_loss");return;}
    if (escaped) {
        bool active=pa_os_controller_tick(&controller,&scene,true,v->read_ms,&b);
        if (active || controller.session.active || controller.engine.active ||
            b.target_epoch || b.horizontal_pixels || b.vertical_pixels ||
            controller.controls.lock!=PA_OS_SCROLL_LOCK_ARMED) {
            failure("post_escape_output");return;
        }
        if (++post_cancel_reads==10) {
            sway_log(SWAY_INFO,"PA_G4E4_POST_ESCAPE_PASS reads=10 target_live=1 extra_axes=0 armed=1 frames=%u",sent);
            terminal=true;
        }
        return;
    }
    if (!pa_os_controller_tick(&controller,&scene,true,v->read_ms,&b)) {
        failure("fixture_tick");return;
    }
    if (!b.horizontal_pixels && !b.vertical_pixels) return;
    if (!motions || sent>=100 || b.seat_epoch!=v->seat_epoch || b.device_epoch!=v->device_epoch ||
        b.target_epoch!=v->target_epoch || b.output_epoch!=v->output_epoch ||
        b.layout_epoch!=v->layout_epoch || b.read_sequence!=v->read_sequence ||
        !isfinite(b.horizontal_pixels) || !isfinite(b.vertical_pixels) ||
        b.horizontal_pixels<=0 || b.vertical_pixels<=0 ||
        b.horizontal_pixels>280 || b.vertical_pixels>280) {
        failure("batch");return;
    }
    uint32_t stamp=(uint32_t)v->read_ms;
    wlr_seat_pointer_notify_axis(seat->wlr_seat,stamp,WL_POINTER_AXIS_VERTICAL_SCROLL,
        b.vertical_pixels,0,WL_POINTER_AXIS_SOURCE_CONTINUOUS,WL_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL);
    wlr_seat_pointer_notify_axis(seat->wlr_seat,stamp,WL_POINTER_AXIS_HORIZONTAL_SCROLL,
        b.horizontal_pixels,0,WL_POINTER_AXIS_SOURCE_CONTINUOUS,WL_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL);
    wlr_seat_pointer_notify_frame(seat->wlr_seat);
    ++sent;
    sway_log(SWAY_INFO,"PA_G4E4_BATCH n=%u time=%" PRIu32 " target=%" PRIu64
        " read=%" PRIu64 " vertical=%.8f horizontal=%.8f fixture=1",
        sent,stamp,b.target_epoch,b.read_sequence,b.vertical_pixels,b.horizontal_pixels);
}

static bool named(struct wlr_input_device *d,const char *name) {
    if (!d || !wlr_input_device_is_libinput(d)) return false;
    struct libinput_device *h=wlr_libinput_get_device_handle(d);
    const char *n=h?libinput_device_get_name(h):NULL;
    return n && !strcmp(n,name);
}
bool pa_g4e4_motion(struct sway_cursor *c,struct wlr_pointer_motion_event *e) {
    if (!started || terminal || escaped || !c || !e || !e->pointer ||
        !named(&e->pointer->base,"PA-G3J-KERNEL-SYNTHETIC-NOT-PHYSICAL")) return false;
    const struct pa_os_scroll_scene *v=&controller.session.bound;
    struct sway_seat *s=resolve(v);
    struct pa_autoscroll_owner_snapshot snap={0};
    if (!s || s->cursor!=c || !pa_autoscroll_owner_snapshot_read(c,&e->pointer->base,&snap) ||
        snap.device!=v->device_epoch || snap.seat!=v->seat_epoch ||
        !e->time_msec || !isfinite(e->delta_x) || !isfinite(e->delta_y)) {
        failure("native_motion_owner");return false;
    }
    /* Actual callback deltas; source permission is deliberately fixture-only. */
    if (!pa_os_controller_motion(&controller,snap.seat,snap.device,
            controller.input_sequence+1,true,e->delta_x,e->delta_y)) {
        failure("motion");return false;
    }
    ++motions;
    sway_log(SWAY_INFO,"PA_G4E4_MOTION n=%u dx=%.8f dy=%.8f seat=%" PRIu64
        " device=%" PRIu64 " native=1 fixture_grant=1",
        motions,e->delta_x,e->delta_y,snap.seat,snap.device);
    return true;
}
void pa_g4e4_key(struct sway_keyboard *k,struct wlr_keyboard_key_event *e) {
    if (!started || terminal || escaped || !k || !k->wlr || !e ||
        e->keycode!=KEY_ESC || e->state!=WL_KEYBOARD_KEY_STATE_PRESSED ||
        !named(&k->wlr->base,"PA-G3L-SYNTHETIC-KEYBOARD-NOT-HARDWARE")) return;
    struct sway_seat *s=resolve(&controller.session.bound);
    if (!s || !k->seat_device || k->seat_device->sway_seat!=s ||
        k->pa_g4d3_binding.control.seat_epoch!=controller.session.seat_epoch ||
        !k->pa_g4d3_binding.control.device_epoch || !e->time_msec || sent<8) {
        failure("escape_owner_or_too_early");return;
    }
    /* UNKNOWN can cancel, never arm. Separate controller key state. */
    struct pa_os_keyboard_controls slot={0};
    if (!pa_os_keyboard_controls_init(&slot,controller.session.seat_epoch,
        k->pa_g4d3_binding.control.device_epoch)) {failure("key_slot");return;}
    struct pa_os_control_event key={.seat_epoch=controller.session.seat_epoch,
        .device_epoch=slot.device_epoch,.sequence=controller.controls.last_sequence+1,
        .key=PA_OS_KEY_ESCAPE,.edge=PA_OS_KEY_PRESS,.source=PA_OS_KEY_UNKNOWN};
    if (!pa_os_controller_key(&controller,&slot,&key) ||
        controller.session.active || controller.engine.active ||
        controller.controls.lock!=PA_OS_SCROLL_LOCK_ARMED) {
        failure("escape_cancel");return;
    }
    escaped=true;
    sway_log(SWAY_INFO,"PA_G4E4_ESCAPE frames=%u motions=%u engine=0 session=0 armed=1 source=UNKNOWN",sent,motions);
}
