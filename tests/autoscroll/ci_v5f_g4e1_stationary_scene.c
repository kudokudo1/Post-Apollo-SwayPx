/* Real timer observation in disposable compositor. Stores only epochs.
 * No active session, no dispatch, no fake input. One fixture read slot
 * is copied at the original callback so inherited receipts stay intact. */
#include <inttypes.h>
#include <string.h>
#include <linux/input-event-codes.h>
#include <libinput.h>
#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_pointer.h>
#include "sway/input/cursor.h"
#include "sway/input/seat.h"
#include "sway/input/post_apollo_autoscroll_os_scene.h"
#include "sway/input/post_apollo_autoscroll_owner_snapshot.h"
#include "sway/server.h"
#include "log.h"
static struct pa_os_layout_owner layout;
static struct pa_os_native_slot slot;
static struct wl_event_source *timer;
static uint64_t dev_epoch,target_epoch,output_epoch,initial_layout,last_layout;
static unsigned good_count;
static bool started,target_lost,device_lost;
void pa_g4e1_attach(struct wlr_output_layout *l) {
    if (!pa_os_layout_attach(&layout,l)) sway_log(SWAY_ERROR,"PA_G4E1_FAILED attach");
}
static int tick(void *unused) {
    (void)unused;
    struct pa_os_scene_read o={0};
    if (!pa_os_scene_read(&slot,&layout,dev_epoch,initial_layout,&o)) {
        sway_log(SWAY_ERROR,"PA_G4E1_FAILED read");
        wl_event_source_remove(timer);timer=NULL;return 0;
    }
    struct pa_os_scroll_scene *v=&o.scene;
    bool same=v->owners_live && v->target_epoch==target_epoch && v->output_epoch==output_epoch;
    if (same && good_count<3) ++good_count;
    bool emit=(same && good_count<=3 && !target_lost && slot.read_sequence<=5) ||
        (!same && !target_lost) || v->layout_epoch!=last_layout || (!o.device_resolved && !device_lost);
    if (emit) {
        sway_log(SWAY_INFO,"PA_G4E1_READ event=%" PRIu64 " read=%" PRIu64
            " ms=%" PRIu64 " seat=%" PRIu64 " device=%" PRIu64
            " target=%" PRIu64 " output=%" PRIu64 " layout=%" PRIu64
            " same=%d focus=%d layout_match=%d seat_live=%d device_live=%d candidate=%d mode=%d clearance=%d",
            slot.event_sequence,v->read_sequence,v->read_ms,v->seat_epoch,v->device_epoch,
            v->target_epoch,v->output_epoch,v->layout_epoch,same,v->focused_root_matches,
            o.layout_matches,o.seat_resolved,o.device_resolved,o.source_candidate,
            v->mode,v->company_weather_clear);
    }
    last_layout=v->layout_epoch;
    if (!same) target_lost=true;
    if (!o.device_resolved) {
        device_lost=true;wl_event_source_remove(timer);timer=NULL;return 0;
    }
    if (wl_event_source_timer_update(timer,20)<0) {
        sway_log(SWAY_ERROR,"PA_G4E1_FAILED timer");
        wl_event_source_remove(timer);timer=NULL;
    }
    return 0;
}
void pa_g4e1_middle(struct sway_cursor *c,struct wlr_pointer_button_event *e) {
    if (started || !c || !c->seat || !e || !e->pointer || e->button!=BTN_MIDDLE ||
        e->state!=WL_POINTER_BUTTON_STATE_RELEASED) return;
    struct wlr_input_device *d=&e->pointer->base;
    if (!wlr_input_device_is_libinput(d)) return;
    struct libinput_device *h=wlr_libinput_get_device_handle(d);
    const char *n=h?libinput_device_get_name(h):NULL;
    if (!n || strcmp(n,"PA-G3J-KERNEL-SYNTHETIC-NOT-PHYSICAL")) return;
    /* G4C2 has just read this callback. Use its seat sequence as starting
     * point, separate fixture slot for subsequent timer-only observations. */
    slot=c->seat->pa_g4c2_slot;
    struct pa_autoscroll_owner_snapshot snap={0};
    if (!pa_autoscroll_owner_snapshot_read(c,d,&snap) || !slot.event_sequence ||
        !(initial_layout=pa_os_layout_revision(&layout))) {
        sway_log(SWAY_ERROR,"PA_G4E1_FAILED capture");return;
    }
    dev_epoch=snap.device;target_epoch=snap.target;output_epoch=snap.output;
    last_layout=initial_layout;started=true;
    timer=wl_event_loop_add_timer(server.wl_event_loop,tick,NULL);
    if (!timer || wl_event_source_timer_update(timer,20)<0) {
        sway_log(SWAY_ERROR,"PA_G4E1_FAILED start");
        if (timer) wl_event_source_remove(timer);
        timer=NULL;
    }
}
