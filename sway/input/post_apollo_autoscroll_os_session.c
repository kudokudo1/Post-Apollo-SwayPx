/* G4C1 detached policy-scoped session eligibility. No IO, native
 * event hooks, pointer storage, timers, credentials or physical claims. */
#include <stddef.h>
#include <string.h>
#include "sway/input/post_apollo_autoscroll_os_session.h"
#define MAX_EVENT_AGE_MS UINT64_C(80)
#define MAX_SCENE_AGE_MS UINT64_C(48)
#define MAX_TICK_GAP_MS UINT64_C(100)
#define MAX_SESSION_MS UINT64_C(180000)

bool pa_os_scroll_init(struct pa_os_scroll_session *s,uint64_t seat_epoch) {
    if (!s || s->seat_epoch || !seat_epoch) return false;
    memset(s,0,sizeof(*s));
    s->seat_epoch=seat_epoch;
    return true;
}
void pa_os_scroll_cancel(struct pa_os_scroll_session *s) {
    if (!s) return;
    s->active=false;
    s->begin_ms=0;
    s->last_tick_ms=0;
    memset(&s->bound,0,sizeof(s->bound));
}
static bool age_ok(uint64_t stamp,uint64_t now,uint64_t maximum) {
    return stamp && now>=stamp && now-stamp<=maximum;
}
static bool scene_ok(const struct pa_os_scroll_session *s,
        const struct pa_os_scroll_scene *v,uint64_t now) {
    return v && s->seat_epoch && v->seat_epoch==s->seat_epoch &&
        v->device_epoch && v->target_epoch && v->output_epoch &&
        v->device_epoch!=v->seat_epoch && v->device_epoch!=v->target_epoch &&
        v->device_epoch!=v->output_epoch && v->seat_epoch!=v->target_epoch &&
        v->seat_epoch!=v->output_epoch && v->target_epoch!=v->output_epoch &&
        v->layout_epoch && v->policy_epoch &&
        v->read_sequence && v->read_sequence!=UINT64_MAX &&
        v->read_sequence>s->last_read_sequence &&
        age_ok(v->read_ms,now,MAX_SCENE_AGE_MS) &&
        v->mode==PA_OS_SCROLL_MANAGED && v->selected &&
        v->lock==PA_OS_SCROLL_LOCK_ARMED &&
        v->owners_live && v->device_attached && v->direct_mapped_hit &&
        v->focused_root_matches && v->layout_verified &&
        v->no_pending_transaction && v->no_constraint && v->session_unlocked &&
        v->no_conflicting_grab && v->company_weather_clear && v->no_emulation;
}
static bool event_ok(const struct pa_os_scroll_event *e,uint64_t now) {
    return e->sequence!=UINT64_MAX && age_ok(e->observed_ms,now,MAX_EVENT_AGE_MS) &&
        e->native_callback && e->middle_press && e->libinput_pointer &&
        e->live_handle && e->coherent_kernel_tree && !e->known_synthetic &&
        !e->protocol_virtual && e->scroll_method_known && !e->native_button_scroll;
}
bool pa_os_scroll_begin(struct pa_os_scroll_session *s,
        const struct pa_os_scroll_event *e,const struct pa_os_scroll_scene *v,
        uint64_t now) {
    if (!s || !s->seat_epoch) return false;
    const bool was_active=s->active;
    pa_os_scroll_cancel(s);
    /* Consume observed sequence even on refusal. A later configuration
     * change or restored scene must never upgrade a previously seen press. */
    if (!e || !e->sequence || e->sequence<=s->last_event_sequence) return false;
    s->last_event_sequence=e->sequence;
    if (was_active || !event_ok(e,now) || !scene_ok(s,v,now) ||
            v->read_ms<e->observed_ms) return false;
    s->last_read_sequence=v->read_sequence;
    s->bound=*v;
    s->begin_ms=now;
    s->last_tick_ms=now;
    s->active=true;
    return true;
}
bool pa_os_scroll_tick(struct pa_os_scroll_session *s,
        const struct pa_os_scroll_scene *v,uint64_t now) {
    if (!s || !s->active) return false;
    const struct pa_os_scroll_scene *b=&s->bound;
    if (now<=s->last_tick_ms || now-s->last_tick_ms>MAX_TICK_GAP_MS ||
        now-s->begin_ms>MAX_SESSION_MS || !scene_ok(s,v,now) ||
        v->read_ms<=b->read_ms || v->device_epoch!=b->device_epoch ||
        v->seat_epoch!=b->seat_epoch || v->target_epoch!=b->target_epoch ||
        v->output_epoch!=b->output_epoch || v->layout_epoch!=b->layout_epoch ||
        v->policy_epoch!=b->policy_epoch) {
        pa_os_scroll_cancel(s);
        return false;
    }
    s->last_read_sequence=v->read_sequence;
    s->bound=*v;
    s->last_tick_ms=now;
    return true;
}
