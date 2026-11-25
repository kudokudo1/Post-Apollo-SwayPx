/* Positive inputs in this file are portable fixtures, not native grants. */
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include "sway/input/post_apollo_autoscroll_os_controller.h"
static struct pa_os_scroll_scene scene(struct pa_os_controller *c,uint64_t n,uint64_t ms) {
    return (struct pa_os_scroll_scene){.device_epoch=1,.seat_epoch=2,.target_epoch=3,
        .output_epoch=4,.layout_epoch=5,.policy_epoch=c->controls.policy_epoch,
        .read_sequence=n,.read_ms=ms,.mode=c->controls.mode,.lock=c->controls.lock,
        .selected=true,.owners_live=true,.device_attached=true,.direct_mapped_hit=true,
        .focused_root_matches=true,.layout_verified=true,.no_pending_transaction=true,
        .no_constraint=true,.session_unlocked=true,.no_conflicting_grab=true,
        .company_weather_clear=true,.no_emulation=true};
}
static struct pa_os_scroll_event ev(struct pa_os_controller *c,uint64_t n,uint64_t ms,bool press) {
    return (struct pa_os_scroll_event){.sequence=n,.observed_ms=ms,.device_epoch=1,
        .seat_epoch=2,.layout_epoch=5,.policy_epoch=c->controls.policy_epoch,
        .native_callback=true,.middle_press=press,.libinput_pointer=true,.live_handle=true,
        .coherent_kernel_tree=true,.scroll_method_known=true};
}
static bool button(struct pa_os_controller *c,uint64_t n,uint64_t read,uint64_t ms,bool press) {
    struct pa_os_scroll_event e=ev(c,n,ms,press);
    struct pa_os_scroll_scene v=scene(c,read,ms);
    return pa_os_controller_middle(c,&e,&v,100,200,ms);
}
static bool key(struct pa_os_controller *c,struct pa_os_keyboard_controls *k,
        uint64_t n,enum pa_os_control_key which,enum pa_os_control_edge edge) {
    struct pa_os_control_event e={.seat_epoch=2,.device_epoch=9,.sequence=n,
        .key=which,.edge=edge,.source=PA_OS_KEY_MANAGED};
    return pa_os_controller_key(c,k,&e);
}
static void empty(const struct pa_os_scroll_batch *b) {
    assert(!b->seat_epoch && !b->device_epoch && !b->target_epoch && !b->output_epoch &&
        !b->layout_epoch && !b->policy_epoch && !b->read_sequence &&
        b->horizontal_pixels==0 && b->vertical_pixels==0);
}
static void arm(struct pa_os_controller *c,struct pa_os_keyboard_controls *k) {
    assert(pa_os_controller_init(c,2));
    assert(pa_os_keyboard_controls_init(k,2,9));
    assert(pa_os_controller_select(c,PA_OS_SCROLL_MANAGED));
    assert(key(c,k,1,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_PRESS));
    assert(!key(c,k,2,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_RELEASE));
}
int main(void) {
    struct pa_os_controller c={0};struct pa_os_keyboard_controls k={0};
    struct pa_os_scroll_batch b={0};
    assert(pa_os_controller_init(&c,2));
    assert(!button(&c,1,1,1000,true));
    assert(pa_os_controller_select(&c,PA_OS_SCROLL_MANAGED));
    assert(!button(&c,2,2,1001,true));
    assert(pa_os_keyboard_controls_init(&k,2,9));
    assert(key(&c,&k,1,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_PRESS));
    assert(!key(&c,&k,2,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_RELEASE));
    assert(button(&c,3,3,1002,true));
    assert(c.session.active && c.engine.active && c.pending_middle_device==1);
    assert(button(&c,4,4,1003,true)); /* held duplicate */
    assert(button(&c,5,5,1004,false));
    assert(pa_os_controller_motion(&c,2,1,6,true,100,-180));
    for (uint64_t i=0;i<100;++i) {
        uint64_t now=1018+16*i;
        struct pa_os_scroll_scene v=scene(&c,6+i,now);
        assert(pa_os_controller_tick(&c,&v,true,now,&b));
        assert(b.target_epoch==3 && b.device_epoch==1 && b.output_epoch==4 &&
            b.horizontal_pixels>0 && b.vertical_pixels<0 && c.input_sequence==6);
    }
    assert(key(&c,&k,3,PA_OS_KEY_ESCAPE,PA_OS_KEY_PRESS));
    assert(!c.session.active && !c.engine.active && c.controls.lock==PA_OS_SCROLL_LOCK_ARMED);
    struct pa_os_scroll_scene v=scene(&c,106,2620);
    assert(!pa_os_controller_tick(&c,&v,true,2620,&b));empty(&b);
    assert(button(&c,7,107,2630,true));
    assert(key(&c,&k,4,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_PRESS));
    assert(c.controls.lock==PA_OS_SCROLL_LOCK_DISARMED && !c.engine.active);
    struct pa_os_scroll_event foreign=ev(&c,8,2631,false);foreign.device_epoch=8;
    assert(!pa_os_controller_middle(&c,&foreign,NULL,0,0,2631));
    assert(c.pending_middle_device==1 && button(&c,9,108,2632,false));
    assert(!key(&c,&k,5,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_RELEASE));
    assert(key(&c,&k,6,PA_OS_KEY_SCROLL_LOCK,PA_OS_KEY_PRESS));
    assert(button(&c,10,109,2640,true));
    assert(button(&c,11,110,2641,false));
    assert(button(&c,12,111,2642,true)); /* toggle off */
    assert(!c.engine.active && c.controls.lock==PA_OS_SCROLL_LOCK_ARMED);
    assert(button(&c,13,112,2643,false));
    unsigned revoked=0;
    for (unsigned i=0;i<10;++i) {
        struct pa_os_controller q={0};struct pa_os_keyboard_controls kk={0};
        arm(&q,&kk);
        assert(button(&q,1,1,1000,true));
        assert(button(&q,2,2,1001,false));
        assert(pa_os_controller_motion(&q,2,1,3,true,100,100));
        struct pa_os_scroll_scene bad=scene(&q,3,1016);
        bool candidate=true;
        switch(i) {
            case 0: bad.target_epoch=7;break;
            case 1: bad.output_epoch=7;break;
            case 2: bad.layout_epoch=7;break;
            case 3: bad.policy_epoch++;break;
            case 4: bad.focused_root_matches=false;break;
            case 5: bad.company_weather_clear=false;break;
            case 6: bad.no_conflicting_grab=false;break;
            case 7: candidate=false;break;
            case 8: bad.read_sequence=2;break;
            case 9: bad.session_unlocked=false;break;
        }
        assert(!pa_os_controller_tick(&q,&bad,candidate,1016,&b));empty(&b);
        assert(!q.engine.active && !q.session.active);
        struct pa_os_scroll_scene clean=scene(&q,4,1032);
        assert(!pa_os_controller_tick(&q,&clean,true,1032,&b));empty(&b);
        ++revoked;
    }
    {struct pa_os_controller q={0};struct pa_os_keyboard_controls kk={0};
     arm(&q,&kk);assert(button(&q,1,1,1000,true));
     assert(!pa_os_controller_motion(&q,2,1,2,true,NAN,0));
     assert(!q.session.active && q.pending_middle_device==1);
     assert(button(&q,3,2,1002,false));
     assert(button(&q,4,3,1003,true));pa_os_controller_retire_device(&q,1);
     assert(!q.engine.active && !q.pending_middle_device);
     assert(button(&q,5,4,1004,true));
     assert(!pa_os_controller_motion(&q,2,1,UINT64_MAX,true,100,0));
     assert(q.exhausted && !q.engine.active && q.pending_middle_device==1);
     assert(button(&q,UINT64_MAX,5,1005,false));}
    puts("PA_G4E2_OS_CONTROLLER_PASS stationary_scroll_batches=100 revocations=10 escape_arm_preserved=1 middle_toggle=1 release_pairing=1 source_recheck=1 real_axes=NONE");
    return 0;
}
