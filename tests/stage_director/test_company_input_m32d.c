#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include "sway/stage_director_company_input.h"

static struct stage_director_company_input_snapshot scene(void) {
    return (struct stage_director_company_input_snapshot) {
        .scene_active=true,
        .selection_epoch=11,
        .native_actor_view_ref=0xabc,
        .native_actor_root_surface_ref=0xdef,
        .map_global=(struct stage_director_rect){100,200,650,450},
    };
}
static void expect(struct stage_director_company_input_snapshot s,
        uintptr_t root, uintptr_t focused, double x,double y,bool locked,
        enum stage_director_company_input_verdict verdict,bool denied) {
    const struct stage_director_company_input_result r =
        stage_director_company_input_pointer(&s,root,focused,x,y,locked);
    assert(r.verdict==verdict);
    assert(r.deny_event==denied);
}
static void positive_pointer_hits_are_half_open(void) {
    struct stage_director_company_input_snapshot s=scene();
    expect(s,0xdef,0,100,200,false,SD_COMPANY_INPUT_WITHIN_MAP,false);
    expect(s,0xdef,0,749.9999,649.9999,false,
           SD_COMPANY_INPUT_WITHIN_MAP,false);
    expect(s,0xdef,0,750,400,false,
           SD_COMPANY_INPUT_DENY_STALE_OR_OUTSIDE,true);
    expect(s,0xdef,0,120,650,false,
           SD_COMPANY_INPUT_DENY_STALE_OR_OUTSIDE,true);
    expect(s,0xdef,0,99.999,240,false,
           SD_COMPANY_INPUT_DENY_STALE_OR_OUTSIDE,true);
    expect(s,0xdef,0,180,199.999,false,
           SD_COMPANY_INPUT_DENY_STALE_OR_OUTSIDE,true);
    expect(s,0xdef,0,NAN,300,false,
           SD_COMPANY_INPUT_DENY_STALE_OR_OUTSIDE,true);
    expect(s,0xdef,0,INFINITY,300,false,
           SD_COMPANY_INPUT_DENY_STALE_OR_OUTSIDE,true);
    expect(s,0xdef,0,300,-INFINITY,false,
           SD_COMPANY_INPUT_DENY_STALE_OR_OUTSIDE,true);
    expect(s,0xdef,0,300,300,true,
           SD_COMPANY_INPUT_DENY_STALE_OR_OUTSIDE,true);
}
static void unrelated_clients_and_unembedded_scene_are_unaffected(void) {
    struct stage_director_company_input_snapshot s=scene();
    expect(s,0x111,0,800,700,false,
           SD_COMPANY_INPUT_UNRELATED_SURFACE,false);
    expect(s,0,0,300,300,false,
           SD_COMPANY_INPUT_UNRELATED_SURFACE,false);
    /* A prior native pointer focus can carry an implicit grab beyond
     * the crop even when hit-testing now sees another client. Deny it. */
    expect(s,0x111,0xdef,300,300,false,
           SD_COMPANY_INPUT_DENY_STALE_OR_OUTSIDE,true);
    expect(s,0x111,0xdef,800,700,false,
           SD_COMPANY_INPUT_DENY_STALE_OR_OUTSIDE,true);
    expect(s,0,0xdef,300,300,false,
           SD_COMPANY_INPUT_DENY_STALE_OR_OUTSIDE,true);
    /* A foreign focused client cannot authorize a native actor hit. */
    expect(s,0xdef,0x111,300,300,false,
           SD_COMPANY_INPUT_WITHIN_MAP,false);
    s.scene_active=false;
    expect(s,0xdef,0,9999,9999,false,SD_COMPANY_INPUT_NO_EMBED,false);
    assert(!stage_director_company_input_reject_popup(&s,0xabc));
    assert(!stage_director_company_input_reject_keyboard(&s,0xdef));
    const struct stage_director_company_input_result null =
        stage_director_company_input_pointer(NULL,0xdef,0,0,0,false);
    assert(null.verdict==SD_COMPANY_INPUT_NO_EMBED && !null.deny_event);
}
static void stale_or_malformed_active_map_never_gets_actor_events(void) {
    struct stage_director_company_input_snapshot s=scene();
    s.selection_epoch=0;
    expect(s,0xdef,0,101,201,false,
           SD_COMPANY_INPUT_DENY_STALE_OR_OUTSIDE,true);
    s=scene();s.map_global.width=0;
    expect(s,0xdef,0,101,201,false,
           SD_COMPANY_INPUT_DENY_STALE_OR_OUTSIDE,true);
    s=scene();s.map_global.x=INT32_MAX;
    expect(s,0xdef,0,101,201,false,
           SD_COMPANY_INPUT_DENY_STALE_OR_OUTSIDE,true);
}
static void popup_and_keyboard_focus_are_separately_refused(void) {
    struct stage_director_company_input_snapshot s=scene();
    assert(stage_director_company_input_reject_popup(&s,0xabc));
    assert(!stage_director_company_input_reject_popup(&s,0x555));
    assert(!stage_director_company_input_reject_popup(&s,0));
    assert(stage_director_company_input_reject_keyboard(&s,0xdef));
    assert(!stage_director_company_input_reject_keyboard(&s,0x555));
    assert(!stage_director_company_input_reject_keyboard(&s,0));
    s.selection_epoch=0;
    /* An active-but-corrupt session fails closed for its actor. */
    assert(stage_director_company_input_reject_popup(&s,0xabc));
    assert(stage_director_company_input_reject_keyboard(&s,0xdef));
}
static void negative_origin_monitor_not_confused_with_zero(void) {
    struct stage_director_company_input_snapshot s=scene();
    s.map_global=(struct stage_director_rect){-1100,30,540,300};
    expect(s,0xdef,0,-1100,30,false,
           SD_COMPANY_INPUT_WITHIN_MAP,false);
    expect(s,0xdef,0,-560,200,false,
           SD_COMPANY_INPUT_DENY_STALE_OR_OUTSIDE,true);
    expect(s,0xdef,0,-800,-1,false,
           SD_COMPANY_INPUT_DENY_STALE_OR_OUTSIDE,true);
}
int main(void) {
    positive_pointer_hits_are_half_open();
    unrelated_clients_and_unembedded_scene_are_unaffected();
    stale_or_malformed_active_map_never_gets_actor_events();
    popup_and_keyboard_focus_are_separately_refused();
    negative_origin_monitor_not_confused_with_zero();
    puts("SD_COMPANY_M32D_INPUT_CONTAINMENT PASS "
         "pointer_motion_click_wheel_half_open=1 "
         "fractional_negative_output=PASS popup=DENIED "
         "actor_keyboard=DENIED pointer_grab_escape=DENIED "
         "other_clients=UNAFFECTED "
         "default_scene=OFF");
    return 0;
}
