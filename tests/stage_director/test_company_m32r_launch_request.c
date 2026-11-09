#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "sway/stage_director_company_launch_request.h"

static struct stage_director_company_launch_request_identity identity(void) {
    return (struct stage_director_company_launch_request_identity) {
        .selection_epoch=11, .intent_sequence=23, .host_revision=4,
        .native_host_layer_ref=100, .native_host_surface_ref=101,
        .native_host_client_ref=102, .native_output_ref=103,
        .mode=1,
        .map_in_host={.x=12,.y=30,.width=1493,.height=788},
    };
}

static void no_authority(
        struct stage_director_company_launch_request_receipt r) {
    assert(!r.operator_authenticated && !r.launch_authenticated);
    assert(!r.scene_lease_issued && !r.can_apply_scene);
    assert(!r.can_accept_input && !r.can_focus_keyboard);
    assert(!r.can_show_popup);
}

int main(void) {
    struct stage_director_company_launch_request r = {0};
    stage_director_company_launch_request_init(&r);
    struct stage_director_company_launch_request_identity id = identity();
    uint64_t g = 99;
    assert(stage_director_company_launch_request_reserve(
        &r,&id,100,50,&g) && g==1);
    assert(!stage_director_company_launch_request_reserve(
        &r,&id,101,50,&g) && g==0);
    struct stage_director_company_launch_request_receipt p =
        stage_director_company_launch_request_probe(&r,1,&id,120);
    assert(p.state==SD_COMPANY_REQUEST_CONTEXT_ONLY &&
        p.current_request_context);
    no_authority(p);
    p = stage_director_company_launch_request_probe(&r,0,&id,120);
    assert(p.state==SD_COMPANY_REQUEST_STALE_GENERATION && r.pending);
    no_authority(p);
    id.native_output_ref++;
    p = stage_director_company_launch_request_probe(&r,1,&id,120);
    assert(p.state==SD_COMPANY_REQUEST_NATIVE_CHANGED && !r.pending);
    no_authority(p);
    id=identity();
    stage_director_company_launch_request_init(&r);
    assert(r.last_generation==1);
    assert(stage_director_company_launch_request_reserve(
        &r,&id,200,50,&g) && g==2);
    p=stage_director_company_launch_request_probe(&r,1,&id,220);
    assert(p.state==SD_COMPANY_REQUEST_STALE_GENERATION && r.pending);
    p=stage_director_company_launch_request_probe(&r,2,&id,249);
    assert(p.state==SD_COMPANY_REQUEST_CONTEXT_ONLY);
    p=stage_director_company_launch_request_probe(&r,2,&id,250);
    assert(p.state==SD_COMPANY_REQUEST_EXPIRED && !r.pending);
    no_authority(p);

    assert(stage_director_company_launch_request_reserve(
        &r,&id,300,50,&g) && g==3);
    p=stage_director_company_launch_request_probe(&r,3,&id,299);
    assert(p.state==SD_COMPANY_REQUEST_EXPIRED && !r.pending);
    assert(stage_director_company_launch_request_reserve(
        &r,&id,400,50,&g) && g==4);
    assert(stage_director_company_launch_request_revoke(&r));
    assert(!stage_director_company_launch_request_revoke(&r));
    assert(stage_director_company_launch_request_reserve(
        &r,&id,500,50,&g) && g==5);
    assert(!stage_director_company_launch_request_reserve(
        &r,&id,500,50,&g));
    stage_director_company_launch_request_revoke(&r);

    assert(!stage_director_company_launch_request_reserve(
        &r,&id,0,1,&g));
    assert(!stage_director_company_launch_request_reserve(
        &r,&id,1,0,&g));
    assert(!stage_director_company_launch_request_reserve(
        &r,&id,1,SD_COMPANY_LAUNCH_REQUEST_MAX_NS+1,&g));
    assert(!stage_director_company_launch_request_reserve(
        &r,&id,UINT64_MAX-10,20,&g));
    id.mode=2;
    assert(!stage_director_company_launch_request_reserve(
        &r,&id,700,50,&g));
    id=identity();
    id.map_in_host.width=0;
    assert(!stage_director_company_launch_request_reserve(
        &r,&id,700,50,&g));
    id=identity();
    id.map_in_host.x=-1;
    assert(!stage_director_company_launch_request_reserve(
        &r,&id,700,50,&g));
    id=identity();
    id.map_in_host.x=INT32_MAX;
    assert(!stage_director_company_launch_request_reserve(
        &r,&id,700,50,&g));
    id=identity();
    assert(stage_director_company_launch_request_reserve(
        &r,&id,800,50,&g) && g==6);
    struct stage_director_company_launch_request_identity changed=id;
    changed.mode=0;
    p=stage_director_company_launch_request_probe(&r,g,&changed,810);
    assert(p.state==SD_COMPANY_REQUEST_NATIVE_CHANGED);
    no_authority(p);
    printf("COMPANY_M32R request_context=PASS replay=DENIED expired=REVOKED "
        "geometry_output_mode=EXACT authority=NONE PASS\n");
    return 0;
}
