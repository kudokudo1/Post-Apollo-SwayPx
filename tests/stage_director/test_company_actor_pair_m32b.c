#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "sway/stage_director_company_actor_pair.h"

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

static void no_authority(struct stage_director_company_actor_pair_result r) {
    assert(!r.can_pair_actor && !r.can_position &&
           !r.can_render && !r.can_clip &&
           !r.can_accept_input && !r.can_focus && !r.can_show_popup);
}
static void deny(struct stage_director_company_actor_pair_evidence e,
        enum stage_director_company_actor_pair_status expected) {
    const struct stage_director_company_actor_pair_result r =
        stage_director_company_actor_pair_probe(&e);
    assert(r.status == expected);
    no_authority(r);
}
static void deny_no_rectangle(struct stage_director_company_actor_pair_evidence e,
        enum stage_director_company_actor_pair_status expected) {
    const struct stage_director_company_actor_pair_result r =
        stage_director_company_actor_pair_probe(&e);
    assert(r.status == expected);
    assert(!r.target_geometry_candidate);
    assert(!r.native_process_candidate);
    assert(!r.viewport_global.width && !r.actor_root_observed_global.width);
    no_authority(r);
}

static void real_company_target_preserved_when_actor_is_unplaced(void) {
    struct stage_director_company_actor_pair_evidence e = sample();
    struct stage_director_company_actor_pair_result r =
        stage_director_company_actor_pair_probe(&e);
    assert(r.status == SD_COMPANY_PAIR_DIGEST_UNPINNED);
    assert(r.native_process_candidate && r.target_geometry_candidate);
    assert(r.executable_inode_candidate && !r.exact_elf_digest_candidate);
    /* Host 100,40 + frame 12,9 + map 200,110. */
    assert(r.viewport_global.x == 312 && r.viewport_global.y == 159);
    assert(r.viewport_global.width == 650 && r.viewport_global.height == 450);
    assert(r.actor_root_observed_global.x == 1800);
    no_authority(r);

    /* Even synthetic exact-hash match grants no scene or pointer authority. */
    e.digest.status = SD_COMPANY_DIGEST_PIN_MATCH_CANDIDATE;
    e.digest.pinned_elf_hash_candidate = true;
    r = stage_director_company_actor_pair_probe(&e);
    assert(r.status == SD_COMPANY_PAIR_EXACT_ELF_CANDIDATE_ONLY);
    assert(r.target_geometry_candidate && r.exact_elf_digest_candidate);
    no_authority(r);
    assert(strcmp(stage_director_company_actor_pair_status_name(r.status),
                  "EXACT_ELF_CANDIDATE_ONLY") == 0);
    assert(stage_director_company_actor_pair_probe(NULL).status ==
           SD_COMPANY_PAIR_NO_SELECTED_HOST);
}

static void selection_intent_and_report_generation_are_required(void) {
    struct stage_director_company_actor_pair_evidence e = sample();
    e.selected_native_company_host = false;
    deny_no_rectangle(e, SD_COMPANY_PAIR_NO_SELECTED_HOST);
    e = sample(); e.host.candidate = false;
    deny_no_rectangle(e, SD_COMPANY_PAIR_NO_SELECTED_HOST);
    e = sample(); e.viewport.candidate_only = false;
    deny_no_rectangle(e, SD_COMPANY_PAIR_NO_INTENT_OR_VIEWPORT);
    e = sample(); e.intent.present = false;
    deny_no_rectangle(e, SD_COMPANY_PAIR_NO_INTENT_OR_VIEWPORT);
    e = sample(); e.viewport.selection_epoch++;
    deny_no_rectangle(e, SD_COMPANY_PAIR_NO_INTENT_OR_VIEWPORT);
    e = sample(); e.viewport.intent_sequence++;
    deny_no_rectangle(e, SD_COMPANY_PAIR_NO_INTENT_OR_VIEWPORT);
    e = sample(); e.viewport.host_revision++;
    deny_no_rectangle(e, SD_COMPANY_PAIR_NO_INTENT_OR_VIEWPORT);
    e = sample(); e.intent.mode = 2;
    deny_no_rectangle(e, SD_COMPANY_PAIR_NO_INTENT_OR_VIEWPORT);
    e = sample(); e.viewport.native_host_client_ref=0x999;
    deny_no_rectangle(e, SD_COMPANY_PAIR_NO_INTENT_OR_VIEWPORT);
}

static void native_resources_and_kernel_pidfd_are_not_labels(void) {
    struct stage_director_company_actor_pair_evidence e = sample();
    e.host.native_client_ref = e.actor.native_client_ref;
    deny_no_rectangle(e, SD_COMPANY_PAIR_NATIVE_PEER_CHANGED);
    e = sample(); e.actor.native_root_surface_ref=e.host.native_surface_ref;
    deny_no_rectangle(e, SD_COMPANY_PAIR_NATIVE_PEER_CHANGED);
    e = sample(); e.actor.native_view_ref=0;
    deny_no_rectangle(e, SD_COMPANY_PAIR_NATIVE_PEER_CHANGED);
    e = sample(); e.actor.candidate=false;
    deny_no_rectangle(e, SD_COMPANY_PAIR_NATIVE_PEER_CHANGED);
    e = sample(); e.host.native_client_ref=0x999;
    deny_no_rectangle(e, SD_COMPANY_PAIR_NATIVE_PEER_CHANGED);
    e = sample(); e.kernel_pidfd_actor_candidate=false;
    deny_no_rectangle(e, SD_COMPANY_PAIR_PROCESS_NOT_PROVEN);
    e = sample(); e.actor.uid=0;
    deny_no_rectangle(e, SD_COMPANY_PAIR_PROCESS_NOT_PROVEN);
    e = sample(); e.actor.uid=1001;
    deny_no_rectangle(e, SD_COMPANY_PAIR_PROCESS_NOT_PROVEN);
    e = sample(); e.actor.start_ticks=0;
    deny_no_rectangle(e, SD_COMPANY_PAIR_PROCESS_NOT_PROVEN);
    e = sample(); e.actor.pid=e.host.pid;
    deny_no_rectangle(e, SD_COMPANY_PAIR_PROCESS_NOT_PROVEN);
    e = sample(); e.lineage.direct_child_candidate=false;
    deny_no_rectangle(e, SD_COMPANY_PAIR_PROCESS_NOT_PROVEN);
}

static void geometry_is_relative_to_the_real_company_host_not_screen(void) {
    struct stage_director_company_actor_pair_evidence e = sample();
    e.viewport.map_in_host.x = 0;
    deny_no_rectangle(e, SD_COMPANY_PAIR_UNSAFE_TARGET);
    e = sample(); e.viewport.frame_in_host.x = -1;
    deny_no_rectangle(e, SD_COMPANY_PAIR_UNSAFE_TARGET);
    e = sample(); e.viewport.map_in_frame.width = 9999;
    deny_no_rectangle(e, SD_COMPANY_PAIR_UNSAFE_TARGET);
    e = sample(); e.host.host_global.width = 500;
    deny_no_rectangle(e, SD_COMPANY_PAIR_UNSAFE_TARGET);
    e = sample(); e.host.host_global.x = INT_MAX;
    deny_no_rectangle(e, SD_COMPANY_PAIR_UNSAFE_TARGET);
    e = sample(); e.host.output_global.width = 500;
    deny_no_rectangle(e, SD_COMPANY_PAIR_UNSAFE_TARGET);
    e = sample(); e.actor.root_global.width = 0;
    deny_no_rectangle(e, SD_COMPANY_PAIR_UNSAFE_TARGET);

    /* Negative-origin second output and formerly offscreen actor. */
    e = sample();
    e.host.host_global = (struct stage_director_rect){-1200, 10, 1000, 700};
    e.host.output_global = (struct stage_director_rect){-1360, 0, 1360, 768};
    const struct stage_director_company_actor_pair_result r =
        stage_director_company_actor_pair_probe(&e);
    assert(r.status == SD_COMPANY_PAIR_DIGEST_UNPINNED);
    assert(r.viewport_global.x == -988);
    assert(r.viewport_global.y == 129);
    no_authority(r);
}

static void executable_and_digest_are_distinct_refusals(void) {
    struct stage_director_company_actor_pair_evidence e = sample();
    e.executable.running_current_install_inode_candidate = false;
    deny(e, SD_COMPANY_PAIR_EXECUTABLE_NOT_PROVEN);
    e = sample(); e.executable.status = SD_COMPANY_EXE_DIFFERENT_INODE;
    deny(e, SD_COMPANY_PAIR_EXECUTABLE_NOT_PROVEN);
    e = sample(); e.digest.status = SD_COMPANY_DIGEST_MISMATCH;
    deny(e, SD_COMPANY_PAIR_DIGEST_UNTRUSTED);
    e = sample(); e.digest.status = SD_COMPANY_DIGEST_PIN_MATCH_CANDIDATE;
    e.digest.pinned_elf_hash_candidate = false;
    deny(e, SD_COMPANY_PAIR_DIGEST_UNTRUSTED);
    e = sample(); e.digest.status = SD_COMPANY_DIGEST_UNPINNED;
    e.digest.pinned_elf_hash_candidate = true; /* contradictory evidence */
    deny(e, SD_COMPANY_PAIR_DIGEST_UNPINNED);
}

int main(void) {
    real_company_target_preserved_when_actor_is_unplaced();
    selection_intent_and_report_generation_are_required();
    native_resources_and_kernel_pidfd_are_not_labels();
    geometry_is_relative_to_the_real_company_host_not_screen();
    executable_and_digest_are_distinct_refusals();
    puts("SD_COMPANY_M32B_NATIVE_ACTOR_TARGET PASS "
         "real_protocol_viewport=1 native_peers=1 kernel_pidfd_required=1 "
         "ELF_manifest_unpinned=BLOCKED exact_elf_never_grants_scene=1 "
         "multioutput_negative_origin=PASS scene=OFF input=OFF");
    return 0;
}
