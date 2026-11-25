/* G4E1 detached stationary scene reader. No native input fabrication,
 * focus changes, scrolling, mode selection, or production hook. */
#define _POSIX_C_SOURCE 200809L
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_output_layout.h>
#include "sway/input/cursor.h"
#include "sway/input/seat.h"
#include "sway/input/input-manager.h"
#include "sway/input/post_apollo_autoscroll_os_scene.h"
#include "sway/input/post_apollo_autoscroll_owner_snapshot.h"
#include "sway/input/post_apollo_autoscroll_source.h"
#include "sway/server.h"
#include "sway/tree/root.h"

static void changed(struct wl_listener *l,void *data) {
    (void)data;
    struct pa_os_layout_owner *o=wl_container_of(l,o,change);
    if (o->revision>=UINT64_MAX-1) o->revision=0;
    else if (o->revision) ++o->revision;
}
static void destroyed(struct wl_listener *l,void *data) {
    (void)data;
    struct pa_os_layout_owner *o=wl_container_of(l,o,destroy);
    wl_list_remove(&o->change.link);
    wl_list_remove(&o->destroy.link);
    o->layout=NULL;o->revision=0;o->retired=true;
}
bool pa_os_layout_attach(struct pa_os_layout_owner *o,struct wlr_output_layout *layout) {
    if (!o || !layout || o->layout || o->revision || o->retired) return false;
    o->layout=layout;o->revision=1;
    o->change.notify=changed;o->destroy.notify=destroyed;
    wl_signal_add(&layout->events.change,&o->change);
    wl_signal_add(&layout->events.destroy,&o->destroy);
    return true;
}
uint64_t pa_os_layout_revision(const struct pa_os_layout_owner *o) {
    return o && !o->retired && o->layout && root && root->output_layout==o->layout?
        o->revision:0;
}
static uint64_t now_ms(void) {
    struct timespec ts={0};
    if (clock_gettime(CLOCK_MONOTONIC,&ts)!=0 || ts.tv_sec<0 ||
        ts.tv_nsec<0 || ts.tv_nsec>=1000000000L ||
        (uint64_t)ts.tv_sec>(UINT64_MAX-999)/1000) return 0;
    return (uint64_t)ts.tv_sec*1000+(uint64_t)ts.tv_nsec/1000000;
}
bool pa_os_scene_read(struct pa_os_native_slot *slot,const struct pa_os_layout_owner *layout,
        uint64_t device_epoch,uint64_t expected_layout,struct pa_os_scene_read *out) {
    if (!out) return false;
    memset(out,0,sizeof(*out));
    if (!slot || slot->exhausted || !slot->seat_epoch || !device_epoch ||
        !expected_layout || !server.input || !root) return false;
    if (slot->read_sequence>=UINT64_MAX-1) {slot->exhausted=true;return false;}
    struct pa_os_scroll_scene *v=&out->scene;
    v->read_sequence=++slot->read_sequence;
    v->seat_epoch=slot->seat_epoch;
    v->layout_epoch=pa_os_layout_revision(layout);
    out->layout_matches=v->layout_epoch && v->layout_epoch==expected_layout;
    v->mode=PA_OS_SCROLL_OFF;
    v->lock=PA_OS_SCROLL_LOCK_UNKNOWN;
    /* Resolve the original seat by owner epoch, not a saved pointer. */
    struct sway_seat *seat=NULL,*it=NULL;
    wl_list_for_each(it,&server.input->seats,link) {
        if (seat_autoscroll_lifetime_epoch(it)==slot->seat_epoch) {seat=it;break;}
    }
    out->seat_resolved=seat!=NULL;
    struct sway_input_device *dev=NULL,*di=NULL;
    wl_list_for_each(di,&server.input->devices,link) {
        if (input_manager_autoscroll_pointer_lifetime_epoch(di)==device_epoch) {dev=di;break;}
    }
    out->device_resolved=dev!=NULL;
    if (dev) {
        v->device_epoch=device_epoch;
        const struct pa_autoscroll_origin_facts facts=pa_autoscroll_source_probe(dev->wlr_device);
        out->source_candidate=pa_autoscroll_origin_classify(&facts)==PA_AUTOSCROLL_ORIGIN_LIBINPUT_CANDIDATE;
    }
    if (seat && dev && seat->cursor && seat->cursor->cursor && seat->wlr_seat) {
        struct sway_cursor *cursor=seat->cursor;
        struct sway_seat_device *sd=NULL;
        wl_list_for_each(sd,&seat->devices,link) {
            if (sd->input_device==dev && sd->sway_seat==seat) {v->device_attached=true;break;}
        }
        struct pa_autoscroll_owner_snapshot snap={0};
        if (v->device_attached && pa_autoscroll_owner_snapshot_read(cursor,dev->wlr_device,&snap)
                && snap.seat==slot->seat_epoch && snap.device==device_epoch) {
            v->owners_live=true;v->direct_mapped_hit=true;v->no_pending_transaction=true;
            v->target_epoch=snap.target;v->output_epoch=snap.output;
            struct wlr_surface *hit=NULL;
            double sx=0,sy=0;
            (void)node_at_coords(seat,cursor->cursor->x,cursor->cursor->y,&hit,&sx,&sy);
            v->focused_root_matches=hit && seat->wlr_seat->pointer_state.focused_surface==hit;
        }
        v->no_constraint=cursor->active_constraint==NULL;
        v->no_emulation=!cursor->simulating_pointer_from_touch &&
            !cursor->simulating_pointer_from_tool_tip && !cursor->simulating_pointer_from_tool_button;
        v->session_unlocked=server.session_lock.lock==NULL;
    }
    /* Layout revision is only a layout observation. Complete scene ABA,
     * selection, grab and Company/Weather clearance still unknown. */
    v->read_ms=now_ms();
    return v->read_ms!=0;
}
