/* M32o // synthetic in-memory tests, no production lease issuer.
 * This test must NOT be confused with a physically authenticated Company
 * session or a live Wayland scene admission. */
#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "sway/stage_director_company_scene_lease_candidate.h"

typedef struct stage_director_company_scene_lease_candidate Candidate;
typedef struct stage_director_company_scene_lease_identity Identity;
typedef struct stage_director_company_scene_lease_conditions Conditions;
typedef struct stage_director_company_scene_lease_receipt Receipt;

static Identity identity(uint64_t epoch) {
    return (Identity) {
        .selection_epoch = epoch,
        .intent_sequence = 11,
        .host_revision = 7,
        .native_host_layer_ref = 0x1001,
        .native_host_surface_ref = 0x1002,
        .native_host_client_ref = 0x1003,
        .native_output_ref = 0x1004,
        .native_actor_view_ref = 0x2001,
        .native_actor_root_surface_ref = 0x2002,
        .native_actor_client_ref = 0x2003,
        .actor_pid = 4242,
        .actor_start_ticks = 70001,
    };
}

static Conditions all_conditions(void) {
    return (Conditions) {
        .selected_native_host = true,
        .live_intent_and_viewport = true,
        .current_kernel_pidfd_actor = true,
        .executable_inode_candidate = true,
        .exact_digest_candidate = true, /* SYNTHETIC: real digest UNPINNED */
        .host_mapped = true,
        .actor_mapped = true,
        .output_active = true,
    };
}

static void deny_all_rights(Receipt r) {
    assert(!r.operator_authenticated);
    assert(!r.launch_authenticated);
    assert(!r.scene_lease_issued);
    assert(!r.can_apply_scene);
    assert(!r.can_accept_input);
    assert(!r.can_focus_keyboard);
    assert(!r.can_show_popup);
}

static bool reserve(Candidate *c, uint64_t epoch) {
    Identity i=identity(epoch);
    Conditions cond=all_conditions();
    return stage_director_company_scene_lease_candidate_reserve(
        c,&i,&cond,UINT64_C(9000000000),UINT64_C(1000000000));
}

static Receipt probe(Candidate *c, Identity *i, Conditions *cond,
        uint64_t at) {
    Receipt r=stage_director_company_scene_lease_candidate_probe(
        c,i,cond,at);
    deny_all_rights(r);
    return r;
}

static void test_lifecycle(void) {
    Candidate c={0};
    Identity i=identity(1);
    Conditions cond=all_conditions();
    Receipt r=probe(&c,&i,&cond,UINT64_C(9000000001));
    assert(r.state==SD_COMPANY_LEASE_NO_CANDIDATE);
    stage_director_company_scene_lease_candidate_init(&c);
    assert(reserve(&c,1));
    assert(!reserve(&c,2)); /* no parallel occupant */
    r=probe(&c,&i,&cond,UINT64_C(9000000001));
    assert(r.state==SD_COMPANY_LEASE_MATCHED_CANDIDATE_ONLY);
    assert(r.native_candidate_current);
    r=probe(&c,&i,&cond,UINT64_C(9999999999));
    assert(r.state==SD_COMPANY_LEASE_MATCHED_CANDIDATE_ONLY);
    assert(stage_director_company_scene_lease_candidate_revoke(&c));
    assert(!stage_director_company_scene_lease_candidate_revoke(&c));
    assert(c.last_epoch==1 && !c.expires_at_ns &&
        !c.identity.native_actor_root_surface_ref);
    /* Reinitialization must not erase the anti-replay high-water mark. */
    stage_director_company_scene_lease_candidate_init(&c);
    assert(c.last_epoch==1);
    assert(!reserve(&c,1));
    assert(reserve(&c,2));
    r=probe(&c,&i,&cond,UINT64_C(9000000001));
    assert(r.state==SD_COMPANY_LEASE_NATIVE_CHANGED);
    assert(!reserve(&c,2)); /* invalid probe was irreversible */
    assert(reserve(&c,3));
    i=identity(3);
    r=probe(&c,&i,&cond,UINT64_C(10000000000));
    assert(r.state==SD_COMPANY_LEASE_EXPIRED);
    assert(!reserve(&c,3));
}

static void test_time_and_ttl(void) {
    Candidate c={0};
    stage_director_company_scene_lease_candidate_init(&c);
    Identity i=identity(1);
    Conditions cond=all_conditions();
    assert(!stage_director_company_scene_lease_candidate_reserve(
        &c,&i,&cond,0,1));
    assert(!stage_director_company_scene_lease_candidate_reserve(
        &c,&i,&cond,1,0));
    assert(!stage_director_company_scene_lease_candidate_reserve(
        &c,&i,&cond,1,SD_COMPANY_LEASE_CANDIDATE_MAX_NS+1));
    assert(!stage_director_company_scene_lease_candidate_reserve(
        &c,&i,&cond,UINT64_MAX-3,5));
    assert(stage_director_company_scene_lease_candidate_reserve(
        &c,&i,&cond,100,10));
    Receipt r=probe(&c,&i,&cond,99);
    assert(r.state==SD_COMPANY_LEASE_EXPIRED);
    assert(!stage_director_company_scene_lease_candidate_reserve(
        &c,&i,&cond,100,10));
    i.selection_epoch=2;
    assert(stage_director_company_scene_lease_candidate_reserve(
        &c,&i,&cond,200,10));
    r=probe(&c,&i,&cond,209);
    assert(r.state==SD_COMPANY_LEASE_MATCHED_CANDIDATE_ONLY);
    r=probe(&c,&i,&cond,210);
    assert(r.state==SD_COMPANY_LEASE_EXPIRED);
}

static void mutate_identity(Identity *i, int dimension) {
    switch (dimension) {
    case 0: i->selection_epoch++; break;
    case 1: i->intent_sequence++; break;
    case 2: i->host_revision++; break;
    case 3: i->native_host_layer_ref++; break;
    case 4: i->native_host_surface_ref++; break;
    case 5: i->native_host_client_ref++; break;
    case 6: i->native_output_ref++; break;
    case 7: i->native_actor_view_ref++; break;
    case 8: i->native_actor_root_surface_ref++; break;
    case 9: i->native_actor_client_ref++; break;
    case 10: i->actor_pid++; break;
    case 11: i->actor_start_ticks++; break;
    default: assert(false);
    }
}

static void test_exact_identity_and_evidence_loss(void) {
    Candidate c={0};
    stage_director_company_scene_lease_candidate_init(&c);
    Conditions conditions=all_conditions();
    uint64_t epoch=1;
    for (int field=0;field<12;field++) {
        assert(reserve(&c,epoch));
        Identity different=identity(epoch);
        mutate_identity(&different,field);
        Receipt r=probe(&c,&different,&conditions,UINT64_C(9000000001));
        assert(r.state==SD_COMPANY_LEASE_NATIVE_CHANGED);
        assert(!reserve(&c,epoch));
        epoch++;
    }
    for(int flag=0;flag<8;flag++) {
        assert(reserve(&c,epoch));
        Conditions bad=all_conditions();
        switch(flag) {
        case 0: bad.selected_native_host=false;break;
        case 1: bad.live_intent_and_viewport=false;break;
        case 2: bad.current_kernel_pidfd_actor=false;break;
        case 3: bad.executable_inode_candidate=false;break;
        case 4: bad.exact_digest_candidate=false;break;
        case 5: bad.host_mapped=false;break;
        case 6: bad.actor_mapped=false;break;
        case 7: bad.output_active=false;break;
        default: assert(false);
        }
        Identity exact=identity(epoch);
        Receipt r=probe(&c,&exact,&bad,UINT64_C(9000000001));
        assert(r.state==SD_COMPANY_LEASE_EVIDENCE_LOST);
        assert(!reserve(&c,epoch));
        epoch++;
    }
    /* Real production has no pinned ELF, so a normal actual observation
     * cannot even reserve this in-memory candidate. */
    Conditions live=all_conditions();
    live.exact_digest_candidate=false;
    Identity id=identity(epoch);
    assert(!stage_director_company_scene_lease_candidate_reserve(
        &c,&id,&live,42,100));
    assert(c.state==SD_COMPANY_LEASE_CANDIDATE_EMPTY);
    assert(!stage_director_company_scene_lease_candidate_reserve(
        &c,&id,NULL,42,100));
    id.native_actor_client_ref=id.native_host_client_ref;
    assert(!stage_director_company_scene_lease_candidate_reserve(
        &c,&id,&conditions,42,100));
}

static void test_exact_revocation(void) {
    Candidate c={0};
    stage_director_company_scene_lease_candidate_init(&c);
    assert(reserve(&c,1));
    assert(!stage_director_company_scene_lease_candidate_host_lost(&c,0xA11));
    assert(!stage_director_company_scene_lease_candidate_actor_lost(&c,0xB11));
    assert(!stage_director_company_scene_lease_candidate_output_lost(&c,0xC11));
    assert(!stage_director_company_scene_lease_candidate_process_lost(&c,
        4242,70002)); /* PID alone is unsafe; start-ticks must match */
    assert(c.state==SD_COMPANY_LEASE_CANDIDATE_RESERVED);
    assert(stage_director_company_scene_lease_candidate_host_lost(&c,0x1001));
    assert(!reserve(&c,1));
    assert(reserve(&c,2));
    assert(stage_director_company_scene_lease_candidate_actor_lost(&c,0x2001));
    assert(reserve(&c,3));
    assert(stage_director_company_scene_lease_candidate_output_lost(&c,0x1004));
    assert(reserve(&c,4));
    assert(stage_director_company_scene_lease_candidate_process_lost(
        &c,4242,70001));
    assert(c.last_epoch==4);
}

int main(void) {
    test_lifecycle();
    test_time_and_ttl();
    test_exact_identity_and_evidence_loss();
    test_exact_revocation();
    puts("SD_COMPANY_M32O_CANDIDATE_LIFECYCLE "
         "fresh_epoch=PASS identity_dimensions=12 evidence_dimensions=8 "
         "expiry=PASS revoke_paths=PASS replay=DENIED "
         "operator_authority=NONE scene_apply=OFF PASS");
    return 0;
}
