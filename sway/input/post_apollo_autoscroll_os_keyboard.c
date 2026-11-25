/* Private G4D3 native key producer; not linked or activated in shipping build. */
#define _POSIX_C_SOURCE 200809L
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <linux/input-event-codes.h>
#include <libinput.h>
#include <libudev.h>
#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_keyboard_group.h>
#include "sway/input/keyboard.h"
#include "sway/input/seat.h"
#include "sway/input/input-manager.h"
#include "sway/input/post_apollo_autoscroll_os_keyboard.h"
#include "sway/input/post_apollo_autoscroll_topology.h"
#include "sway/server.h"

static struct sway_seat *live_owner(struct sway_keyboard *k) {
    if (!k || !k->wlr || !k->seat_device || !server.input ||
        wlr_keyboard_group_from_wlr_keyboard(k->wlr)) return NULL;
    struct sway_seat *seat=NULL,*it=NULL;
    wl_list_for_each(it,&server.input->seats,link) {
        if (it==k->seat_device->sway_seat) {seat=it;break;}
    }
    if (!seat || !seat->wlr_seat || !seat_autoscroll_lifetime_epoch(seat)) return NULL;
    struct sway_seat_device *sd=NULL;
    wl_list_for_each(sd,&seat->devices,link) {
        if (sd==k->seat_device && sd->keyboard==k && sd->input_device &&
            sd->input_device->wlr_device==&k->wlr->base) {
            struct sway_input_device *dev=NULL;
            wl_list_for_each(dev,&server.input->devices,link) {
                if (dev==sd->input_device) return seat;
            }
        }
    }
    return NULL;
}
bool pa_os_key_bind(struct pa_os_key_owner *owner,struct pa_os_key_binding *b,
        struct sway_keyboard *k) {
    if (!owner || !b || owner->exhausted || b->control.retired) return false;
    struct sway_seat *seat=live_owner(k);
    if (!seat) return false;
    uint64_t epoch=seat_autoscroll_lifetime_epoch(seat);
    if (owner->seat_epoch && owner->seat_epoch!=epoch) {
        owner->exhausted=true;return false;
    }
    owner->seat_epoch=epoch;
    if (!b->control.seat_epoch) {
        if (owner->next_keyboard>=UINT64_MAX-2) {
            owner->exhausted=true;return false;
        }
        uint64_t id=++owner->next_keyboard;
        if (id==epoch) id=++owner->next_keyboard;
        if (!pa_os_keyboard_controls_init(&b->control,epoch,id)) return false;
    } else if (b->control.seat_epoch!=epoch) return false;
    /* Seed held state at initial binding/reconfigure. A key already
     * down cannot arm when its repeat arrives. Never clear an existing
     * held bit based solely on reconfiguration; require a real release. */
    for (size_t i=0;i<k->wlr->num_keycodes;++i) {
        if (k->wlr->keycodes[i]==KEY_SCROLLLOCK) b->control.scroll_held=true;
        if (k->wlr->keycodes[i]==KEY_ESC) b->control.escape_held=true;
    }
    return true;
}
static uint64_t event_ms(uint32_t stamp) {
    struct timespec ts={0};
    if (!stamp || clock_gettime(CLOCK_MONOTONIC,&ts)!=0 ||
        ts.tv_sec<0 || ts.tv_nsec<0 || ts.tv_nsec>=1000000000L ||
        (uint64_t)ts.tv_sec>(UINT64_MAX-999)/1000) return 0;
    uint64_t now=(uint64_t)ts.tv_sec*1000+(uint64_t)ts.tv_nsec/1000000;
    uint32_t age=(uint32_t)now-stamp;
    return age<=80 && now>age?now-age:0;
}
bool pa_os_key_observe(struct pa_os_key_owner *owner,struct pa_os_key_binding *b,
        struct sway_keyboard *k,const struct wlr_keyboard_key_event *e,
        struct pa_os_key_observation *out) {
    if (!out) return false;
    memset(out,0,sizeof(*out));
    if (!owner || !b || !e || owner->exhausted || b->control.retired ||
        (e->keycode!=KEY_SCROLLLOCK && e->keycode!=KEY_ESC) ||
        (e->state!=WL_KEYBOARD_KEY_STATE_PRESSED && e->state!=WL_KEYBOARD_KEY_STATE_RELEASED))
        return false;
    struct sway_seat *seat=live_owner(k);
    if (!seat || !owner->seat_epoch || b->control.seat_epoch!=owner->seat_epoch ||
        !b->control.device_epoch || seat_autoscroll_lifetime_epoch(seat)!=owner->seat_epoch)
        return false;
    if (owner->sequence>=UINT64_MAX-1) {owner->exhausted=true;return false;}
    out->registered=out->attached=true;
    struct pa_os_control_event *v=&out->event;
    v->seat_epoch=owner->seat_epoch;
    v->device_epoch=b->control.device_epoch;
    v->sequence=++owner->sequence;
    v->key=e->keycode==KEY_SCROLLLOCK?PA_OS_KEY_SCROLL_LOCK:PA_OS_KEY_ESCAPE;
    v->edge=e->state==WL_KEYBOARD_KEY_STATE_PRESSED?PA_OS_KEY_PRESS:PA_OS_KEY_RELEASE;
    struct wlr_input_device *d=&k->wlr->base;
    out->protocol_virtual=k->seat_device->input_device->is_virtual;
    out->native_libinput=d->type==WLR_INPUT_DEVICE_KEYBOARD && wlr_input_device_is_libinput(d);
    struct libinput_device *handle=out->native_libinput?wlr_libinput_get_device_handle(d):NULL;
    out->live_handle=handle!=NULL;
    if (handle) {
        struct udev_device *udev=libinput_device_get_udev_device(handle);
        if (udev) {
            const char *path=udev_device_get_syspath(udev);
            out->known_synthetic=path && (strstr(path,"/virtual/") ||
                strstr(path,"/uinput/") || strstr(path,"/uhid/"));
            out->coherent_tree=pa_autoscroll_kernel_tree_candidate(path,udev_device_get_devnode(udev));
            udev_device_unref(udev);
        }
    }
    uint64_t *last=v->key==PA_OS_KEY_SCROLL_LOCK?&b->last_scroll_ms:&b->last_escape_ms;
    uint64_t stamp=event_ms(e->time_msec);
    out->fresh=stamp && stamp>*last;
    if (out->fresh) *last=stamp;
    out->unlocked=server.session_lock.lock==NULL;
    if (out->fresh && out->unlocked && out->native_libinput && out->live_handle &&
        out->coherent_tree && !out->known_synthetic && !out->protocol_virtual)
        v->source=PA_OS_KEY_MANAGED;
    /* Stale releases must not clear a held press and permit later arming.
     * Caller ignores these observations for key-state change. A stale
     * press remains UNKNOWN (disable-only). No actual mode selection here. */
    return true;
}
void pa_os_key_retire(struct pa_os_key_binding *b) {
    if (!b) return;
    b->control.retired=true;
    b->control.scroll_held=b->control.escape_held=false;
}
