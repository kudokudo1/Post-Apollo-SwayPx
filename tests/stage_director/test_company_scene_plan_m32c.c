#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sway/stage_director_company_scene.h"

static struct stage_director_company_actor_pair_evidence sample(void) {
    return (struct stage_director_company_actor_pair_evidence) {
        .selected_native_company_host = true,
        .kernel_pidfd_actor_candidate = true,
        .host = {
            .state = SD_COMPANY_NATIVE_CANDIDATE_ONLY,
            .candidate = true,
            .native_layer_ref = 0x101,
            .native_layer_role_ref = 0x102,
            .native_surface_ref = 0x103,
            .native_client_ref = 0x104,
            .native_output_ref = 0x105,
            .pid = 1200, .uid = 1000, .start_ticks = 100000,
            .host_global = {100, 40, 1000, 700},
            .output_global = {0, 0, 2560, 1440},
        },
        .actor = {
            .state = SD_COMPANY_NATIVE_CANDIDATE_ONLY,
            .candidate = true,
            .native_view_ref = 0x201,
            .native_root_surface_ref = 0x202,
            .native_client_ref = 0x203,
            .pid = 1267, .uid = 1000, .start_ticks = 101000,
            /* Existing top-level is elsewhere. No scene operation yet. */
            .root_global = {1800, 100, 900, 650},
        },
        .intent = {
            .present = true, .selection_epoch = 7,
            .sequence = 3, .host_revision = 12,
            .native_host_client_ref = 0x104, .mode = 0,
        },
        .viewport = {
            .candidate_only = true, .selection_epoch = 7,
            .intent_sequence = 3, .host_revision = 12,
            .native_host_client_ref = 0x104,
            .frame_in_host = {12, 9, 976, 679},
            .map_in_frame = {200, 110, 650, 450},
            .map_in_host = {212, 119, 650, 450},
        },
        .lineage = {
            .status = SD_COMPANY_LINEAGE_CANDIDATE_ONLY,
            .direct_child_candidate = true,
        },
        .executable = {
            .status = SD_COMPANY_EXE_CURRENT_INSTALL_INODE_CANDIDATE,
            .running_current_install_inode_candidate = true,
        },
        .digest = {.status = SD_COMPANY_DIGEST_UNPINNED},
    };
}


static struct stage_director_company_scene_guard all_test_guards(void) {
    return (struct stage_director_company_scene_guard) {
        .operator_authorized_native_actor = true,
        .approved_executable_and_launch = true,
        .compositor_scene_lease_current = true,
        .input_hit_test_guard_installed = true,
        .keyboard_focus_guard_installed = true,
        .popup_guard_installed = true,
        .teardown_guard_installed = true,
        .host_mapped = true,
        .actor_mapped = true,
        .output_active = true,
        .selection_epoch = 7, .intent_sequence = 3, .host_revision = 12,
        .native_host_client_ref = 0x104,
        .native_actor_root_surface_ref = 0x202,
    };
}

static struct stage_director_company_actor_pair_evidence ready_actor(void) {
    struct stage_director_company_actor_pair_evidence e=sample();
    e.digest.status=SD_COMPANY_DIGEST_PIN_MATCH_CANDIDATE;
    e.digest.pinned_elf_hash_candidate=true;
    /* In private fixture the source has already dynamically resized to
     * Company's current Canvas instead of remaining at 840x540. */
    e.actor.root_global.width=650;
    e.actor.root_global.height=450;
    return e;
}

static void denied(struct stage_director_company_scene_plan p,
        enum stage_director_company_scene_state status) {
    assert(p.state==status);
    assert(!p.geometry_ready);
    assert(!p.can_apply_in_production);
    assert(!p.target_global.width && !p.root_surface_clip.width);
}

static void only_private_ci_can_prepare_full_scene(void) {
    const struct stage_director_company_actor_pair_evidence e=ready_actor();
    const struct stage_director_company_scene_guard gate=all_test_guards();
    const struct stage_director_company_scene_plan p =
        stage_director_company_scene_prepare(
            &e,&gate,(struct stage_director_rect){0,0,650,450});
    assert(p.state==SD_COMPANY_SCENE_READY_FOR_PRIVATE_CI_ONLY);
    assert(p.geometry_ready && !p.can_apply_in_production);
    assert(p.target_global.x==312 && p.target_global.y==159);
    assert(p.target_global.width==650 && p.target_global.height==450);
    assert(p.root_surface_clip.x==0 && p.root_surface_clip.y==0);
    assert(p.root_surface_clip.width==650 &&
           p.root_surface_clip.height==450);
}

static void missing_or_fake_grant_is_refused(void) {
    struct stage_director_company_actor_pair_evidence e=ready_actor();
    struct stage_director_company_scene_guard g=all_test_guards();
    const struct stage_director_rect exact={0,0,650,450};
    denied(stage_director_company_scene_prepare(&e,NULL,exact),
           SD_COMPANY_SCENE_GUARDS_INCOMPLETE);
    denied(stage_director_company_scene_prepare(&e,
           &(struct stage_director_company_scene_guard){0},exact),
           SD_COMPANY_SCENE_GUARDS_INCOMPLETE);
#define REFUSE_GATE(field) do { \
        struct stage_director_company_scene_guard broken=g; \
        broken.field=false; \
        denied(stage_director_company_scene_prepare(&e,&broken,exact), \
               SD_COMPANY_SCENE_GUARDS_INCOMPLETE); \
    } while (0)
    REFUSE_GATE(operator_authorized_native_actor);
    REFUSE_GATE(approved_executable_and_launch);
    REFUSE_GATE(compositor_scene_lease_current);
    REFUSE_GATE(input_hit_test_guard_installed);
    REFUSE_GATE(keyboard_focus_guard_installed);
    REFUSE_GATE(popup_guard_installed);
    REFUSE_GATE(teardown_guard_installed);
    REFUSE_GATE(host_mapped);
    REFUSE_GATE(actor_mapped);
    REFUSE_GATE(output_active);
#undef REFUSE_GATE
    e.digest.status=SD_COMPANY_DIGEST_UNPINNED;
    e.digest.pinned_elf_hash_candidate=false;
    denied(stage_director_company_scene_prepare(&e,&g,exact),
           SD_COMPANY_SCENE_UNTRUSTED_EXECUTABLE);
    e=ready_actor();
    e.kernel_pidfd_actor_candidate=false;
    denied(stage_director_company_scene_prepare(&e,&g,exact),
           SD_COMPANY_SCENE_NO_PAIR);
    e=ready_actor();
    e.viewport.candidate_only=false;
    denied(stage_director_company_scene_prepare(&e,&g,exact),
           SD_COMPANY_SCENE_NO_PAIR);
}

static void changed_native_selection_or_source_is_refused(void) {
    const struct stage_director_company_actor_pair_evidence e=ready_actor();
    const struct stage_director_company_scene_guard g=all_test_guards();
    const struct stage_director_rect exact={0,0,650,450};
    struct stage_director_company_scene_guard stale=g;
    stale.selection_epoch++;
    denied(stage_director_company_scene_prepare(&e,&stale,exact),
           SD_COMPANY_SCENE_STALE_OWNER);
    stale=g;stale.intent_sequence++;
    denied(stage_director_company_scene_prepare(&e,&stale,exact),
           SD_COMPANY_SCENE_STALE_OWNER);
    stale=g;stale.host_revision++;
    denied(stage_director_company_scene_prepare(&e,&stale,exact),
           SD_COMPANY_SCENE_STALE_OWNER);
    stale=g;stale.native_actor_root_surface_ref=0xdead;
    denied(stage_director_company_scene_prepare(&e,&stale,exact),
           SD_COMPANY_SCENE_STALE_OWNER);
    stale=g;stale.native_host_client_ref=0xdead;
    denied(stage_director_company_scene_prepare(&e,&stale,exact),
           SD_COMPANY_SCENE_STALE_OWNER);
    denied(stage_director_company_scene_prepare(
        &e,&g,(struct stage_director_rect){0,0,840,540}),
        SD_COMPANY_SCENE_SOURCE_NOT_EXACT);
    denied(stage_director_company_scene_prepare(
        &e,&g,(struct stage_director_rect){0,0,1494,450}),
        SD_COMPANY_SCENE_SOURCE_NOT_EXACT);
    denied(stage_director_company_scene_prepare(
        &e,&g,(struct stage_director_rect){1,0,650,450}),
        SD_COMPANY_SCENE_SOURCE_NOT_EXACT);
    denied(stage_director_company_scene_prepare(
        &e,&g,(struct stage_director_rect){0,0,0,0}),
        SD_COMPANY_SCENE_SOURCE_NOT_EXACT);
}

static void real_negative_coordinate_output_and_resize(void) {
    struct stage_director_company_actor_pair_evidence e=ready_actor();
    const struct stage_director_company_scene_guard g=all_test_guards();
    e.host.host_global=(struct stage_director_rect){-1200,10,1000,700};
    e.host.output_global=(struct stage_director_rect){-1360,0,1360,768};
    const struct stage_director_company_scene_plan p =
        stage_director_company_scene_prepare(
            &e,&g,(struct stage_director_rect){0,0,650,450});
    assert(p.state==SD_COMPANY_SCENE_READY_FOR_PRIVATE_CI_ONLY);
    assert(p.target_global.x==-988 && p.target_global.y==129);
    assert(!p.can_apply_in_production);
    e.viewport.map_in_frame.width=900; /* stale/oversized Company rect */
    denied(stage_director_company_scene_prepare(
        &e,&g,(struct stage_director_rect){0,0,900,450}),
        SD_COMPANY_SCENE_NO_PAIR);
}

int main(void) {
    only_private_ci_can_prepare_full_scene();
    missing_or_fake_grant_is_refused();
    changed_native_selection_or_source_is_refused();
    real_negative_coordinate_output_and_resize();
    puts("SD_COMPANY_M32C_SCENE_PLAN PASS "
         "real_pair_input=1 all_guards_required=1 "
         "wrong_source_or_stale_epoch=DENIED "
         "negative_output=PASS "
         "production_scene_activation=OFF input=OFF");
    return 0;
}
