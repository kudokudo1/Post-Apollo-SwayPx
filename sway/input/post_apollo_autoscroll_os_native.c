/* G4C2 private compositor observation, no consumption or dispatch.
 * This module is unlinked in the shipping compositor. Every unresolved
 * policy gate stays false/zero. Actual positive authority is NOT supplied. */
#define _POSIX_C_SOURCE 200809L
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <linux/input-event-codes.h>
#include <libinput.h>
#include <libudev.h>
#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_seat.h>
#include "sway/input/cursor.h"
#include "sway/input/input-manager.h"
#include "sway/input/seat.h"
#include "sway/input/post_apollo_autoscroll_os_native.h"
#include "sway/input/post_apollo_autoscroll_owner_snapshot.h"
#include "sway/input/post_apollo_autoscroll_source.h"
#include "sway/input/post_apollo_autoscroll_topology.h"
#include "sway/server.h"
#include "sway/tree/node.h"

static uint64_t monotonic_ms(void) {
    struct timespec ts={0};
    if (clock_gettime(CLOCK_MONOTONIC,&ts)!=0 || ts.tv_sec<0 ||
            ts.tv_nsec<0 || ts.tv_nsec>=1000000000L ||
            (uint64_t)ts.tv_sec>(UINT64_MAX-999)/1000) return 0;
    return (uint64_t)ts.tv_sec*1000+(uint64_t)ts.tv_nsec/1000000;
}
static uint64_t event_ms(uint32_t stamp,uint64_t now) {
    /* wlroots input timestamps are monotonic uint32 milliseconds. Expand
     * only within the bounded recent interval, including 32-bit wrap.
     * Zero/unknown, future and old timestamps refuse; never mint from now. */
    if (!stamp || !now) return 0;
    const uint32_t age=(uint32_t)now-stamp;
    if (age>80 || now<=age) return 0;
    return now-age;
}
bool pa_os_native_observe(struct pa_os_native_slot *slot,
        struct sway_cursor *cursor,struct wlr_pointer_button_event *event,
        struct pa_os_native_observation *out) {
    if (!out) return false;
    memset(out,0,sizeof(*out));
    if (!slot || !cursor || !event || !event->pointer ||
            event->button!=BTN_MIDDLE || !server.input || !cursor->cursor ||
            slot->exhausted) return false;
    struct sway_seat *seat=NULL,*it=NULL;
    wl_list_for_each(it,&server.input->seats,link) {
        if (it==cursor->seat && it->cursor==cursor) {seat=it;break;}
    }
    if (!seat || !seat->wlr_seat) return false;
    const uint64_t epoch=seat_autoscroll_lifetime_epoch(seat);
    if (!epoch || (slot->seat_epoch && slot->seat_epoch!=epoch) ||
            slot->event_sequence>=UINT64_MAX-1 ||
            slot->read_sequence>=UINT64_MAX-1) {
        slot->exhausted=true;
        return false;
    }
    slot->seat_epoch=epoch;
    struct pa_os_scroll_event *e=&out->event;
    struct pa_os_scroll_scene *v=&out->scene;
    e->sequence=++slot->event_sequence;
    v->read_sequence=++slot->read_sequence;
    e->seat_epoch=v->seat_epoch=epoch;
    e->native_callback=true;
    e->middle_press=event->state==WL_POINTER_BUTTON_STATE_PRESSED;
    out->now_ms=monotonic_ms();
    e->observed_ms=event_ms(event->time_msec,out->now_ms);
    struct wlr_input_device *dev=&event->pointer->base;
    struct sway_input_device *native=NULL,*di=NULL;
    wl_list_for_each(di,&server.input->devices,link) {
        if (di->wlr_device==dev) {native=di;break;}
    }
    if (native) {
        out->registered=true;
        e->protocol_virtual=native->is_virtual;
        e->device_epoch=v->device_epoch=input_manager_autoscroll_pointer_lifetime_epoch(native);
        struct sway_seat_device *sd=NULL;
        wl_list_for_each(sd,&seat->devices,link) {
            if (sd->input_device==native && sd->sway_seat==seat) {
                out->attached=true;break;
            }
        }
    }
    v->device_attached=out->attached;
    e->libinput_pointer=dev->type==WLR_INPUT_DEVICE_POINTER &&
        wlr_input_device_is_libinput(dev);
    struct libinput_device *handle=e->libinput_pointer?
        wlr_libinput_get_device_handle(dev):NULL;
    e->live_handle=handle!=NULL;
    if (handle) {
        struct udev_device *udev=libinput_device_get_udev_device(handle);
        if (udev) {
            const char *path=udev_device_get_syspath(udev);
            const char *node=udev_device_get_devnode(udev);
            e->known_synthetic=path && (strstr(path,"/virtual/") ||
                strstr(path,"/uinput/") || strstr(path,"/uhid/"));
            e->coherent_kernel_tree=pa_autoscroll_kernel_tree_candidate(path,node);
            udev_device_unref(udev);
        }
        const struct pa_autoscroll_origin_facts facts=pa_autoscroll_source_probe(dev);
        e->scroll_method_known=facts.native_scroll_method_known;
        e->native_button_scroll=facts.native_scroll_on_button_down;
    }
    struct pa_autoscroll_owner_snapshot owners={0};
    if (out->attached && pa_autoscroll_owner_snapshot_read(cursor,dev,&owners)
            && owners.device==e->device_epoch && owners.seat==epoch) {
        v->owners_live=true;
        v->direct_mapped_hit=true;
        v->no_pending_transaction=true;
        v->target_epoch=owners.target;
        v->output_epoch=owners.output;
        struct wlr_surface *hit=NULL;
        double sx=0,sy=0;
        (void)node_at_coords(seat,cursor->cursor->x,cursor->cursor->y,&hit,&sx,&sy);
        v->focused_root_matches=hit &&
            seat->wlr_seat->pointer_state.focused_surface==hit;
    }
    v->no_constraint=cursor->active_constraint==NULL;
    v->no_emulation=!cursor->simulating_pointer_from_touch &&
        !cursor->simulating_pointer_from_tool_tip &&
        !cursor->simulating_pointer_from_tool_button;
    v->session_unlocked=server.session_lock.lock==NULL;
    /* No approved layout/policy issuer, per-seat lock, grab contract or
     * combined Company containment yet. Do NOT use diagnostic G3O serial,
     * XKB LED, mode from environment, or a caller's bool as clearance. */
    v->mode=PA_OS_SCROLL_OFF;
    v->lock=PA_OS_SCROLL_LOCK_UNKNOWN;
    v->read_ms=monotonic_ms();
    return true;
}
