/* Disposable CI-only integration of real key callbacks with G4D1.
 * Positive setup is an invented, private shadow session. Native fixture
 * keyboards remain UNKNOWN; neither key consumption nor axis is possible.
 * Seat/keyboard slots exist only in the temporary compositor copy. */
#include <inttypes.h>
#include <string.h>
#include <linux/input-event-codes.h>
#include <libinput.h>
#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_pointer.h>
#include "sway/input/keyboard.h"
#include "sway/input/cursor.h"
#include "sway/input/seat.h"
#include "log.h"

static int fixture(struct wlr_input_device *d) {
    if (!d || !wlr_input_device_is_libinput(d)) return 0;
    struct libinput_device *h=wlr_libinput_get_device_handle(d);
    const char *n=h?libinput_device_get_name(h):NULL;
    if (!n) return 0;
    if (!strcmp(n,"PA-G3L-SYNTHETIC-KEYBOARD-NOT-HARDWARE")) return 1;
    if (!strcmp(n,"PA-G3M-SECOND-SYNTHETIC-KEYBOARD")) return 2;
    if (!strcmp(n,"PA-G3J-KERNEL-SYNTHETIC-NOT-PHYSICAL")) return 3;
    return 0;
}
static void fail(const char *why) {
    sway_log(SWAY_ERROR,"PA_G4D2_FAILED reason=%s",why);
}
static bool prepare(struct sway_seat *seat) {
    struct pa_os_controls *c=&seat->pa_g4d2_controls;
    struct pa_os_scroll_session *s=&seat->pa_g4d2_shadow;
    uint64_t epoch=seat_autoscroll_lifetime_epoch(seat);
    if (!epoch) return false;
    if (!c->seat_epoch && (!pa_os_controls_init(c,epoch) ||
            !pa_os_scroll_init(s,epoch))) return false;
    return c->seat_epoch==epoch && s->seat_epoch==epoch;
}
void pa_g4d2_mouse(struct sway_cursor *cursor,struct wlr_pointer_button_event *e) {
    if (!cursor || !cursor->seat || !e || !e->pointer || e->button!=BTN_MIDDLE ||
            e->state!=WL_POINTER_BUTTON_STATE_RELEASED || fixture(&e->pointer->base)!=3) return;
    struct sway_seat *seat=cursor->seat;
    unsigned round=++seat->pa_g4d2_pairs;
    if (round!=2 && round!=12) return;
    if (!prepare(seat)) {fail("shadow_prepare");return;}
    struct pa_os_controls *c=&seat->pa_g4d2_controls;
    struct pa_os_scroll_session *s=&seat->pa_g4d2_shadow;
    /* Explicit private fixture setup, NOT the actual native keyboard
     * source. Invented device/target IDs never resolve to a live surface. */
    struct pa_os_keyboard_controls *fake=&seat->pa_g4d2_fixture_key;
    if (!fake->seat_epoch && !pa_os_keyboard_controls_init(fake,c->seat_epoch,UINT64_MAX-1)) {
        fail("fixture_key_init");return;
    }
    if (!pa_os_controls_select(c,s,PA_OS_SCROLL_MANAGED)) {fail("fixture_select");return;}
    struct pa_os_control_event k={.seat_epoch=c->seat_epoch,.device_epoch=fake->device_epoch,
        .sequence=c->last_sequence+1,.key=PA_OS_KEY_SCROLL_LOCK,
        .edge=PA_OS_KEY_PRESS,.source=PA_OS_KEY_MANAGED};
    if (!pa_os_controls_key(c,fake,s,&k)) {fail("fixture_arm");return;}
    ++k.sequence;k.edge=PA_OS_KEY_RELEASE;
    (void)pa_os_controls_key(c,fake,s,&k);
    uint64_t seq=s->last_event_sequence+1,ms=round*1000;
    struct pa_os_scroll_event ev={.sequence=seq,.observed_ms=ms,
        .device_epoch=UINT64_MAX-2,.seat_epoch=c->seat_epoch,.layout_epoch=1,
        .policy_epoch=c->policy_epoch,.native_callback=true,.middle_press=true,
        .libinput_pointer=true,.live_handle=true,.coherent_kernel_tree=true,
        .scroll_method_known=true};
    struct pa_os_scroll_scene v={.device_epoch=ev.device_epoch,.seat_epoch=c->seat_epoch,
        .target_epoch=UINT64_MAX-3,.output_epoch=UINT64_MAX-4,.layout_epoch=1,
        .policy_epoch=c->policy_epoch,.read_sequence=seq,.read_ms=ms,
        .mode=c->mode,.lock=c->lock,.selected=true,.owners_live=true,
        .device_attached=true,.direct_mapped_hit=true,.focused_root_matches=true,
        .layout_verified=true,.no_pending_transaction=true,.no_constraint=true,
        .session_unlocked=true,.no_conflicting_grab=true,.company_weather_clear=true,
        .no_emulation=true};
    if (!pa_os_scroll_begin(s,&ev,&v,ms)) {fail("fixture_session");return;}
    sway_log(SWAY_INFO,"PA_G4D2_SHADOW_ARM round=%u seat=%" PRIu64
        " mode=%d lock=%d active=%d",round,c->seat_epoch,c->mode,c->lock,s->active);
}
void pa_g4d2_key(struct sway_keyboard *k,struct wlr_keyboard_key_event *e) {
    if (!k || !k->wlr || !k->seat_device || !e ||
        (e->keycode!=KEY_SCROLLLOCK && e->keycode!=KEY_ESC)) return;
    int source=fixture(&k->wlr->base);
    if (source!=1 && source!=2) return; /* excludes aggregate group */
    struct sway_seat_device *sd=k->seat_device;
    struct sway_seat *seat=sd->sway_seat;
    if (!seat || sd->keyboard!=k || !sd->input_device ||
        sd->input_device->wlr_device!=&k->wlr->base || !prepare(seat)) {
        fail("native_owner");return;
    }
    bool attached=false;
    struct sway_seat_device *it=NULL;
    wl_list_for_each(it,&seat->devices,link) {
        if (it==sd) {attached=true;break;}
    }
    if (!attached) {fail("native_attachment");return;}
    struct pa_os_controls *c=&seat->pa_g4d2_controls;
    struct pa_os_scroll_session *s=&seat->pa_g4d2_shadow;
    struct pa_os_keyboard_controls *slot=&k->pa_g4d2_key;
    if (!slot->seat_epoch) {
        /* Per-seat keyboard lifetime namespace for CI only; allocated
         * on first relevant callback, never retained after destruction. */
        uint64_t id=++seat->pa_g4d2_next_keyboard;
        if (id==c->seat_epoch) id=++seat->pa_g4d2_next_keyboard;
        if (!id || !pa_os_keyboard_controls_init(slot,c->seat_epoch,id)) {
            fail("native_key_slot");return;
        }
    }
    bool before=s->active;
    struct pa_os_control_event ev={.seat_epoch=c->seat_epoch,
        .device_epoch=slot->device_epoch,.sequence=c->last_sequence+1,
        .key=e->keycode==KEY_SCROLLLOCK?PA_OS_KEY_SCROLL_LOCK:PA_OS_KEY_ESCAPE,
        .edge=e->state==WL_KEYBOARD_KEY_STATE_PRESSED?PA_OS_KEY_PRESS:PA_OS_KEY_RELEASE,
        .source=PA_OS_KEY_UNKNOWN};
    bool changed=pa_os_controls_key(c,slot,s,&ev);
    sway_log(SWAY_INFO,"PA_G4D2_KEY source=%d key=%d press=%d seat=%" PRIu64
        " device=%" PRIu64 " seq=%" PRIu64 " before=%d active=%d mode=%d lock=%d changed=%d trust=UNKNOWN can_consume=0",
        source,ev.key,ev.edge==PA_OS_KEY_PRESS,c->seat_epoch,slot->device_epoch,
        ev.sequence,before,s->active,c->mode,c->lock,changed);
}
void pa_g4d2_retire(struct sway_keyboard *k) {
    if (!k || !k->seat_device || !k->pa_g4d2_key.seat_epoch) return;
    struct sway_seat *seat=k->seat_device->sway_seat;
    if (!seat) return;
    struct pa_os_keyboard_controls *slot=&k->pa_g4d2_key;
    pa_os_keyboard_controls_retire(&seat->pa_g4d2_controls,slot,&seat->pa_g4d2_shadow);
    sway_log(SWAY_INFO,"PA_G4D2_RETIRE seat=%" PRIu64 " device=%" PRIu64 " retired=%d active=%d",
        slot->seat_epoch,slot->device_epoch,slot->retired,seat->pa_g4d2_shadow.active);
}
