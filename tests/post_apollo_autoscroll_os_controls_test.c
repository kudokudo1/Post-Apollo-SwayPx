#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "sway/input/post_apollo_autoscroll_os_controls.h"
static bool key(struct pa_os_controls *c,struct pa_os_keyboard_controls *k,
        struct pa_os_scroll_session *s,uint64_t seq,enum pa_os_control_key which,
        enum pa_os_control_edge edge,enum pa_os_control_source source) {
    struct pa_os_control_event e={.seat_epoch=c->seat_epoch,
        .device_epoch=k?k->device_epoch:0,.sequence=seq,.key=which,
        .edge=edge,.source=source};
    return pa_os_controls_key(c,k,s,&e);
}
static void arm_session(struct pa_os_controls *c,struct pa_os_scroll_session *s,
        uint64_t seq,uint64_t ms) {
    struct pa_os_scroll_event e={.sequence=seq,.observed_ms=ms,
        .device_epoch=10,.seat_epoch=c->seat_epoch,.layout_epoch=11,
        .policy_epoch=c->policy_epoch,.native_callback=true,.middle_press=true,
        .libinput_pointer=true,.live_handle=true,.coherent_kernel_tree=true,
        .scroll_method_known=true};
    struct pa_os_scroll_scene v={.device_epoch=10,.seat_epoch=c->seat_epoch,
        .target_epoch=12,.output_epoch=13,.layout_epoch=11,.policy_epoch=c->policy_epoch,
        .read_sequence=seq,.read_ms=ms,.mode=c->mode,.lock=c->lock,.selected=true,
        .owners_live=true,.device_attached=true,.direct_mapped_hit=true,
        .focused_root_matches=true,.layout_verified=true,.no_pending_transaction=true,
        .no_constraint=true,.session_unlocked=true,.no_conflicting_grab=true,
        .company_weather_clear=true,.no_emulation=true};
    assert(pa_os_scroll_begin(s,&e,&v,ms));
}
int main(void) {
    struct pa_os_controls c={0},other={0};
    struct pa_os_scroll_session s={0},o={0};
    struct pa_os_keyboard_controls a={0},b={0},foreign={0},replacement={0};
    assert(pa_os_controls_init(&c,1)&&pa_os_scroll_init(&s,1));
    assert(pa_os_controls_init(&other,2)&&pa_os_scroll_init(&o,2));
    assert(pa_os_keyboard_controls_init(&a,1,3));
    assert(pa_os_keyboard_controls_init(&b,1,4));
    assert(pa_os_keyboard_controls_init(&foreign,2,5));
    assert(c.mode==PA_OS_SCROLL_OFF && c.lock==PA_OS_SCROLL_LOCK_DISARMED);
    assert(!key(&c,&a,&s,1,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_PRESS,PA_OS_KEY_MANAGED));
    assert(pa_os_controls_select(&c,&s,PA_OS_SCROLL_MANAGED));
    assert(!key(&c,&a,&s,2,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_PRESS,PA_OS_KEY_MANAGED));
    assert(!key(&c,&a,&s,3,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_RELEASE,PA_OS_KEY_MANAGED));
    assert(key(&c,&a,&s,4,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_PRESS,PA_OS_KEY_MANAGED));
    assert(c.lock==PA_OS_SCROLL_LOCK_ARMED);
    uint64_t epoch=c.policy_epoch;
    assert(!key(&c,&a,&s,5,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_REPEAT,PA_OS_KEY_MANAGED));
    assert(!key(&c,&a,&s,6,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_PRESS,PA_OS_KEY_MANAGED));
    assert(c.policy_epoch==epoch);
    assert(!key(&c,&a,&s,7,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_RELEASE,PA_OS_KEY_MANAGED));
    arm_session(&c,&s,1,1000);
    assert(key(&c,&b,&s,8,PA_OS_KEY_ESCAPE,PA_OS_KEY_PRESS,PA_OS_KEY_MANAGED));
    assert(!s.active && c.lock==PA_OS_SCROLL_LOCK_ARMED && c.policy_epoch>epoch);
    assert(!key(&c,&b,&s,9,PA_OS_KEY_ESCAPE,PA_OS_KEY_REPEAT,PA_OS_KEY_MANAGED));
    assert(!key(&c,&b,&s,10,PA_OS_KEY_ESCAPE,PA_OS_KEY_RELEASE,PA_OS_KEY_MANAGED));
    arm_session(&c,&s,2,1100);
    assert(key(&c,&b,&s,11,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_PRESS,PA_OS_KEY_MANAGED));
    assert(!s.active && c.lock==PA_OS_SCROLL_LOCK_DISARMED);
    assert(!key(&c,&b,&s,12,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_RELEASE,PA_OS_KEY_MANAGED));
    assert(key(&c,&a,&s,13,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_PRESS,PA_OS_KEY_MANAGED));
    assert(c.lock==PA_OS_SCROLL_LOCK_ARMED);
    arm_session(&c,&s,3,1200);
    assert(key(&c,&b,&s,14,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_PRESS,PA_OS_KEY_UNKNOWN));
    assert(!s.active && c.lock==PA_OS_SCROLL_LOCK_DISARMED);
    assert(!key(&c,&b,&s,15,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_PRESS,PA_OS_KEY_MANAGED));
    assert(!key(&c,&b,&s,16,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_RELEASE,PA_OS_KEY_MANAGED));
    assert(key(&c,&b,&s,17,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_PRESS,PA_OS_KEY_MANAGED));
    assert(c.lock==PA_OS_SCROLL_LOCK_ARMED);
    arm_session(&c,&s,4,1300);
    struct pa_os_control_event cross={.seat_epoch=2,.device_epoch=5,.sequence=18,
        .key=PA_OS_KEY_SCROLL_LOCK,.edge=PA_OS_KEY_PRESS,.source=PA_OS_KEY_MANAGED};
    assert(!pa_os_controls_key(&c,&foreign,&s,&cross));
    assert(s.active && other.lock==PA_OS_SCROLL_LOCK_DISARMED);
    assert(!pa_os_controls_key(&c,&b,&o,&cross));
    assert(s.active);
    pa_os_keyboard_controls_retire(&c,&b,&s);
    assert(!s.active && c.lock==PA_OS_SCROLL_LOCK_DISARMED && b.retired);
    assert(!pa_os_keyboard_controls_init(&b,1,4));
    assert(key(&c,&b,&s,19,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_PRESS,PA_OS_KEY_MANAGED));
    assert(c.lock==PA_OS_SCROLL_LOCK_DISARMED);
    assert(pa_os_keyboard_controls_init(&replacement,1,6));
    assert(key(&c,&replacement,&s,20,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_PRESS,PA_OS_KEY_MANAGED));
    assert(c.lock==PA_OS_SCROLL_LOCK_ARMED);
    assert(!key(&c,&replacement,&s,20,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_PRESS,PA_OS_KEY_MANAGED));
    arm_session(&c,&s,5,1400);
    pa_os_controls_invalidate(&c,&s);
    assert(!s.active && c.lock==PA_OS_SCROLL_LOCK_DISARMED);
    assert(!pa_os_controls_select(&c,&s,PA_OS_SCROLL_ENROLLED));
    assert(c.mode==PA_OS_SCROLL_OFF);
    assert(pa_os_controls_select(&c,&s,PA_OS_SCROLL_MANAGED));
    assert(!key(&c,&a,&s,21,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_PRESS,PA_OS_KEY_MANAGED));
    assert(!key(&c,&a,&s,22,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_RELEASE,PA_OS_KEY_MANAGED));
    assert(key(&c,&a,&s,23,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_PRESS,PA_OS_KEY_MANAGED));
    assert(c.lock==PA_OS_SCROLL_LOCK_ARMED);
    arm_session(&c,&s,6,1500);
    assert(key(&c,&a,&s,UINT64_MAX,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_PRESS,PA_OS_KEY_MANAGED));
    assert(c.exhausted && !s.active && c.mode==PA_OS_SCROLL_OFF);
    assert(!pa_os_controls_select(&c,&s,PA_OS_SCROLL_MANAGED));
    assert(pa_os_controls_select(&other,&o,PA_OS_SCROLL_MANAGED));
    other.policy_epoch=UINT64_MAX-1;
    assert(key(&other,&foreign,&o,1,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_PRESS,PA_OS_KEY_MANAGED));
    assert(other.exhausted && other.mode==PA_OS_SCROLL_OFF && other.lock==PA_OS_SCROLL_LOCK_DISARMED);
    puts("PA_G4D1_SEAT_CONTROLS_PASS active_session_cancellations=6 two_keyboards=1 cross_seat=DENIED repeat=DENIED overflow=DENIED native_delivery=NONE");
    return 0;
}
