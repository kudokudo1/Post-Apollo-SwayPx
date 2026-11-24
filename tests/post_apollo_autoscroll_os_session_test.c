/* Positive fixture eligibility is not physical input or native delivery. */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "sway/input/post_apollo_autoscroll_os_session.h"
static struct pa_os_scroll_event event(uint64_t seq) {
    return (struct pa_os_scroll_event){
        .sequence=seq,.observed_ms=1000,.native_callback=true,
        .middle_press=true,.libinput_pointer=true,.live_handle=true,
        .coherent_kernel_tree=true,.scroll_method_known=true};
}
static struct pa_os_scroll_scene scene(uint64_t seq,uint64_t ms) {
    return (struct pa_os_scroll_scene){
        .device_epoch=1,.seat_epoch=2,.target_epoch=3,.output_epoch=4,
        .layout_epoch=5,.policy_epoch=6,.read_sequence=seq,.read_ms=ms,
        .mode=PA_OS_SCROLL_MANAGED,.lock=PA_OS_SCROLL_LOCK_ARMED,.selected=true,
        .owners_live=true,.device_attached=true,.direct_mapped_hit=true,
        .focused_root_matches=true,.layout_verified=true,.no_pending_transaction=true,
        .no_constraint=true,.session_unlocked=true,.no_conflicting_grab=true,
        .company_weather_clear=true,.no_emulation=true};
}
static struct pa_os_scroll_session slot(void) {
    struct pa_os_scroll_session s={0};
    assert(pa_os_scroll_init(&s,2));
    return s;
}
static void refusal(struct pa_os_scroll_event e,struct pa_os_scroll_scene v) {
    struct pa_os_scroll_session s=slot();
    assert(!pa_os_scroll_begin(&s,&e,&v,1000));
    assert(!s.active);
    struct pa_os_scroll_event good=event(e.sequence);
    struct pa_os_scroll_scene clean=scene(2,1000);
    assert(!pa_os_scroll_begin(&s,&good,&clean,1000));
}
#define EVENT_DENY(field,value) do {struct pa_os_scroll_event e=event(1); e.field=(value); refusal(e,scene(1,1000)); ++negative;} while(0)
#define SCENE_DENY(field,value) do {struct pa_os_scroll_scene v=scene(1,1000); v.field=(value); refusal(event(1),v); ++negative;} while(0)
#define TICK_DENY(field,value) do {struct pa_os_scroll_session s=slot(); struct pa_os_scroll_event e=event(1); struct pa_os_scroll_scene v=scene(1,1000); assert(pa_os_scroll_begin(&s,&e,&v,1000)); v=scene(2,1016); v.field=(value); assert(!pa_os_scroll_tick(&s,&v,1016)); v=scene(3,1032); assert(!pa_os_scroll_tick(&s,&v,1032)); assert(!s.active && s.last_event_sequence==1 && s.bound.target_epoch==0); ++revocations;} while(0)
int main(void) {
    unsigned negative=0,revocations=0;
    EVENT_DENY(native_callback,false); EVENT_DENY(middle_press,false);
    EVENT_DENY(libinput_pointer,false); EVENT_DENY(live_handle,false);
    EVENT_DENY(coherent_kernel_tree,false); EVENT_DENY(known_synthetic,true);
    EVENT_DENY(protocol_virtual,true); EVENT_DENY(scroll_method_known,false);
    EVENT_DENY(native_button_scroll,true); EVENT_DENY(observed_ms,919);
    EVENT_DENY(observed_ms,1001); EVENT_DENY(sequence,UINT64_MAX);
    SCENE_DENY(mode,PA_OS_SCROLL_OFF); SCENE_DENY(mode,PA_OS_SCROLL_ENROLLED);
    SCENE_DENY(mode,99); SCENE_DENY(selected,false);
    SCENE_DENY(lock,PA_OS_SCROLL_LOCK_UNKNOWN); SCENE_DENY(lock,PA_OS_SCROLL_LOCK_DISARMED);
    SCENE_DENY(lock,99); SCENE_DENY(owners_live,false);
    SCENE_DENY(device_attached,false); SCENE_DENY(direct_mapped_hit,false);
    SCENE_DENY(focused_root_matches,false); SCENE_DENY(layout_verified,false);
    SCENE_DENY(no_pending_transaction,false); SCENE_DENY(no_constraint,false);
    SCENE_DENY(session_unlocked,false); SCENE_DENY(no_conflicting_grab,false);
    SCENE_DENY(company_weather_clear,false); SCENE_DENY(no_emulation,false);
    SCENE_DENY(device_epoch,0); SCENE_DENY(device_epoch,2);
    SCENE_DENY(seat_epoch,22); SCENE_DENY(target_epoch,0);
    SCENE_DENY(output_epoch,0); SCENE_DENY(layout_epoch,0);
    SCENE_DENY(policy_epoch,0); SCENE_DENY(read_sequence,0);
    SCENE_DENY(read_sequence,UINT64_MAX); SCENE_DENY(read_ms,999);
    TICK_DENY(device_epoch,11); TICK_DENY(seat_epoch,12);
    TICK_DENY(target_epoch,13); TICK_DENY(output_epoch,14);
    TICK_DENY(layout_epoch,15); TICK_DENY(policy_epoch,16);
    TICK_DENY(lock,PA_OS_SCROLL_LOCK_DISARMED); TICK_DENY(mode,PA_OS_SCROLL_OFF);
    TICK_DENY(focused_root_matches,false); TICK_DENY(company_weather_clear,false);
    TICK_DENY(session_unlocked,false); TICK_DENY(no_conflicting_grab,false);
    TICK_DENY(read_sequence,1); TICK_DENY(read_ms,1000);
    TICK_DENY(read_ms,1017); TICK_DENY(owners_live,false);
    struct pa_os_scroll_session s=slot();
    struct pa_os_scroll_event e=event(1);
    struct pa_os_scroll_scene v=scene(1,1000);
    assert(!pa_os_scroll_init(&s,2));
    assert(pa_os_scroll_begin(&s,&e,&v,1000));
    /* Stationary pointer, fresh scene; no invented new input. */
    for(uint64_t i=2;i<=101;++i) {
        uint64_t now=1000+(i-1)*16;
        v=scene(i,now);
        assert(pa_os_scroll_tick(&s,&v,now));
    }
    assert(s.active && s.last_event_sequence==1);
    assert(!pa_os_scroll_begin(&s,&e,&v,2600));
    assert(!s.active);
    e=event(2); e.observed_ms=2600;
    v=scene(102,2600);
    assert(pa_os_scroll_begin(&s,&e,&v,2600));
    v=scene(103,2701);
    assert(!pa_os_scroll_tick(&s,&v,2701));
    assert(!pa_os_scroll_begin(&s,&e,&v,2701));
    e=event(3); e.observed_ms=2800; v=scene(104,2800);
    assert(pa_os_scroll_begin(&s,&e,&v,2800));
    pa_os_scroll_cancel(&s);
    assert(!s.active && s.last_event_sequence==3);
    assert(!pa_os_scroll_begin(&s,&e,&v,2800));
    printf("PA_G4C1_OS_POLICY_PASS admission_refusals=%u revocations=%u stationary_ticks=100 physical_attestation=NONE native_delivery=NONE\n",negative,revocations);
    return 0;
}
