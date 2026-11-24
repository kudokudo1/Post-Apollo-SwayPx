/* CI-only shadow session: invented positive witness, no native authority.
 * All axis/frame outputs go exclusively to private counters. This does
 * not use the runtime, Wayland output, a real seat, or input consumption.
 * A real native event serial invalidates the shadow before the next tick. */
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include "log.h"
#include "sway/input/post_apollo_autoscroll_epoch.h"
#include "sway/input/post_apollo_autoscroll_axis_dispatch.h"

static struct pa_autoscroll_epoch_guard guard;
static struct pa_autoscroll_dispatch dispatch;
static struct pa_autoscroll_witness original;
static const struct pa_autoscroll_object_epochs epochs={101,102,103,104};
static unsigned pairs,armed_round,axes,frames;
static uint64_t armed_serial;
static bool pending,failed;

static bool count_axis(void *ctx,const struct pa_scroll_axis_event *event) {
    (void)ctx;
    if (!event || event->target_node_id!=103 ||
        event->axis!=PA_SCROLL_AXIS_VERTICAL || event->delta_pixels<=0) {
        failed=true;
        return false;
    }
    ++axes;
    return true;
}
static void count_frame(void *ctx) {
    (void)ctx;
    ++frames;
}
static bool tick_dispatch(uint64_t now) {
    return pa_scroll_dispatch_tick(&dispatch,101,102,true,103,
        original.layout_generation,now,false,false,count_axis,count_frame,NULL);
}
static void fail(const char *where) {
    failed=true;
    pa_autoscroll_epoch_cancel(&guard);
    pa_scroll_dispatch_cancel(&dispatch);
    pending=false;
    sway_log(SWAY_ERROR,"PA_G3T_FAILED where=%s",where);
}
void pa_g3t_middle(bool pressed,uint64_t serial) {
    if (pressed || failed) return;
    ++pairs;
    if (pairs!=2 && pairs!=3 && pairs!=5 && pairs!=7 && pairs!=11) return;
    if (pending || guard.active || dispatch.adapter.engine.active || !serial) {
        fail("previous_session_not_cancelled");
        return;
    }
    /* These constants are fictitious; no claim about the synthetic device
     * or live target is passed into the positive witness. */
    original=(struct pa_autoscroll_witness){
        .origin=PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT,
        .compositor_event=true,.surface_hit_verified=true,.target_mapped=true,
        .output_enabled=true,.layout_generation_verified=true,
        .device_id=101,.seat_id=102,.target_node_id=103,.output_id=104,
        .layout_generation=serial,.observed_at_ms=1000,
    };
    axes=frames=0;
    pa_scroll_dispatch_init(&dispatch);
    pa_scroll_dispatch_enable(&dispatch,true);
    if (!pa_autoscroll_epoch_begin(&guard,&original,&epochs,1000) ||
        !pa_scroll_dispatch_middle(&dispatch,true,true,101,102,true,103,
            serial,0,0,1000,false,false) ||
        !pa_scroll_dispatch_middle(&dispatch,false,true,101,102,true,103,
            serial,0,0,1001,false,false) ||
        !pa_scroll_dispatch_motion(&dispatch,true,101,102,0,150,false,false)) {
        fail("shadow_begin");
        return;
    }
    struct pa_autoscroll_witness fresh=original;
    fresh.observed_at_ms=1016;
    if (!pa_autoscroll_epoch_tick(&guard,&fresh,&epochs,1016) ||
        !tick_dispatch(1016) || axes!=1 || frames!=1 ||
        !guard.active || !dispatch.adapter.engine.active) {
        fail("positive_memory_control");
        return;
    }
    armed_round=pairs;
    armed_serial=serial;
    pending=true;
    sway_log(SWAY_INFO,"PA_G3T_SHADOW_ARM round=%u serial=%" PRIu64
        " axes=1 frames=1 guard=1 engine=1 real_axis=0",
        armed_round,armed_serial);
}
void pa_g3t_event(const char *kind,uint64_t serial,bool poisoned) {
    if (!pending || failed) return;
    if (!kind || (!poisoned && serial<=armed_serial)) {
        fail("event_serial_not_fresh");
        return;
    }
    /* Check the real invalidation signal before the next memory-sink tick.
     * The portable guard rejects a mismatched layout generation. Treat
     * every topology event as a conservative revocation of the shadow. */
    struct pa_autoscroll_witness revoked=original;
    revoked.observed_at_ms=1032;
    revoked.layout_generation=poisoned?0:serial;
    if (!strcmp(kind,"scroll_lock_disable_request"))
        revoked.native_scroll_lock_active=true;
    if (pa_autoscroll_epoch_tick(&guard,&revoked,&epochs,1032) || guard.active) {
        fail("guard_accepted_revocation");
        return;
    }
    pa_scroll_dispatch_cancel(&dispatch);
    /* Reuse old layout with a newer fixture timestamp, as if ABA restored
     * the original state. Neither guard nor dispatch may revive itself. */
    struct pa_autoscroll_witness replay=original;
    replay.observed_at_ms=1048;
    if (pa_autoscroll_epoch_tick(&guard,&replay,&epochs,1048) ||
        tick_dispatch(1048) || axes!=1 || frames!=1 || guard.active ||
        dispatch.adapter.engine.active || guard.started.device_id ||
        dispatch.adapter.device_id || dispatch.adapter.seat_id) {
        fail("post_cancel_replay_or_axis");
        return;
    }
    pending=false;
    sway_log(SWAY_INFO,"PA_G3T_SHADOW_CANCEL round=%u kind=%s serial=%" PRIu64
        " axes_before=1 axes_after=1 frames_before=1 frames_after=1 guard=0 engine=0 replay=DENIED real_axis=0",
        armed_round,kind,serial);
}
