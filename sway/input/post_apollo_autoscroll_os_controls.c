/* G4D1 detached seat-local controls. No native input ownership yet. */
#include <stddef.h>
#include "sway/input/post_apollo_autoscroll_os_controls.h"

static bool same_session(const struct pa_os_controls *c,
        const struct pa_os_scroll_session *s) {
    return c && s && c->seat_epoch && c->seat_epoch==s->seat_epoch;
}
static void advance(struct pa_os_controls *c,struct pa_os_scroll_session *s) {
    pa_os_scroll_cancel(s);
    if (c->policy_epoch>=UINT64_MAX-1) {
        c->exhausted=true;
        c->policy_epoch=UINT64_MAX;
        c->mode=PA_OS_SCROLL_OFF;
        c->lock=PA_OS_SCROLL_LOCK_DISARMED;
    } else {
        ++c->policy_epoch;
    }
}
bool pa_os_controls_init(struct pa_os_controls *c,uint64_t seat_epoch) {
    if (!c || c->seat_epoch || !seat_epoch) return false;
    *c=(struct pa_os_controls){.seat_epoch=seat_epoch,.policy_epoch=1,
        .mode=PA_OS_SCROLL_OFF,.lock=PA_OS_SCROLL_LOCK_DISARMED};
    return true;
}
bool pa_os_keyboard_controls_init(struct pa_os_keyboard_controls *k,
        uint64_t seat_epoch,uint64_t device_epoch) {
    if (!k || k->seat_epoch || k->retired || !seat_epoch || !device_epoch ||
        seat_epoch==device_epoch) return false;
    *k=(struct pa_os_keyboard_controls){.seat_epoch=seat_epoch,.device_epoch=device_epoch};
    return true;
}
bool pa_os_controls_select(struct pa_os_controls *c,struct pa_os_scroll_session *s,
        enum pa_os_scroll_mode mode) {
    if (!same_session(c,s) || c->exhausted) return false;
    const bool valid=mode==PA_OS_SCROLL_OFF || mode==PA_OS_SCROLL_MANAGED;
    c->mode=valid?mode:PA_OS_SCROLL_OFF;
    c->lock=PA_OS_SCROLL_LOCK_DISARMED;
    advance(c,s);
    return valid && !c->exhausted;
}
void pa_os_controls_invalidate(struct pa_os_controls *c,struct pa_os_scroll_session *s) {
    if (!same_session(c,s)) return;
    c->lock=PA_OS_SCROLL_LOCK_DISARMED;
    advance(c,s);
}
void pa_os_keyboard_controls_retire(struct pa_os_controls *c,
        struct pa_os_keyboard_controls *k,struct pa_os_scroll_session *s) {
    if (!same_session(c,s) || !k || k->seat_epoch!=c->seat_epoch || k->retired) return;
    k->retired=true;
    k->scroll_held=k->escape_held=false;
    pa_os_controls_invalidate(c,s);
}
bool pa_os_controls_key(struct pa_os_controls *c,struct pa_os_keyboard_controls *k,
        struct pa_os_scroll_session *s,const struct pa_os_control_event *e) {
    if (!same_session(c,s) || !e || c->exhausted ||
        e->seat_epoch!=c->seat_epoch || !e->sequence ||
        e->sequence<=c->last_sequence) return false;
    c->last_sequence=e->sequence;
    if (e->sequence==UINT64_MAX) {
        c->exhausted=true;
        c->mode=PA_OS_SCROLL_OFF;
        pa_os_controls_invalidate(c,s);
        return true;
    }
    if ((e->key!=PA_OS_KEY_SCROLL_LOCK && e->key!=PA_OS_KEY_ESCAPE) ||
        (e->edge!=PA_OS_KEY_PRESS && e->edge!=PA_OS_KEY_RELEASE &&
         e->edge!=PA_OS_KEY_REPEAT)) return false;
    const bool matched=k && !k->retired && k->seat_epoch==c->seat_epoch &&
        k->device_epoch && k->device_epoch==e->device_epoch;
    const bool trusted=matched && e->source==PA_OS_KEY_MANAGED;
    bool *held=matched?(e->key==PA_OS_KEY_SCROLL_LOCK?&k->scroll_held:&k->escape_held):NULL;
    if (e->edge==PA_OS_KEY_RELEASE) {
        if (held) *held=false;
        return false;
    }
    /* An untrusted or unmatched press/repeat can never arm. It can
     * conservatively disable or cancel, including after a trust downgrade. */
    if (!trusted) {
        if (held) *held=true;
        if (e->key==PA_OS_KEY_SCROLL_LOCK) pa_os_controls_invalidate(c,s);
        else advance(c,s);
        return true;
    }
    if (e->edge==PA_OS_KEY_REPEAT || *held) return false;
    *held=true;
    if (e->key==PA_OS_KEY_ESCAPE) {
        advance(c,s);
        return true;
    }
    if (c->mode!=PA_OS_SCROLL_MANAGED) {
        c->lock=PA_OS_SCROLL_LOCK_DISARMED;
        return false;
    }
    c->lock=c->lock==PA_OS_SCROLL_LOCK_ARMED?
        PA_OS_SCROLL_LOCK_DISARMED:PA_OS_SCROLL_LOCK_ARMED;
    advance(c,s);
    return true;
}
