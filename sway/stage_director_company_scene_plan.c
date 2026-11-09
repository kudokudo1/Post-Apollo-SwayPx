#include "sway/stage_director_company_scene.h"
#include "sway/stage_director_viewport.h"

static struct stage_director_company_scene_plan refuse(
        enum stage_director_company_scene_state state) {
    return (struct stage_director_company_scene_plan) {
        .state = state,
        /* No compositor action or pointer authority, including CI plans. */
        .can_apply_in_production = false,
    };
}

struct stage_director_company_scene_plan
stage_director_company_scene_prepare(
        const struct stage_director_company_actor_pair_evidence *current,
        const struct stage_director_company_scene_guard *guard,
        struct stage_director_rect exact_native_source) {
    /* M32b internally rejects mismatched host, XDG, kernel pidfd, live
     * process, UID/lineage, viewport, and untrusted executable inode. */
    const struct stage_director_company_actor_pair_result pair =
        stage_director_company_actor_pair_probe(current);
    if (!current || !pair.native_process_candidate ||
            !pair.target_geometry_candidate) {
        return refuse(SD_COMPANY_SCENE_NO_PAIR);
    }
    if (pair.status != SD_COMPANY_PAIR_EXACT_ELF_CANDIDATE_ONLY ||
            !pair.executable_inode_candidate ||
            !pair.exact_elf_digest_candidate) {
        return refuse(SD_COMPANY_SCENE_UNTRUSTED_EXECUTABLE);
    }

    /* The current program has NO owner for these gates. A future physical
     * Company actor lease must issue them after input, popup and teardown
     * containment have been actually installed, not merely requested. */
    if (!guard || !guard->operator_authorized_native_actor ||
            !guard->approved_executable_and_launch ||
            !guard->compositor_scene_lease_current ||
            !guard->input_hit_test_guard_installed ||
            !guard->keyboard_focus_guard_installed ||
            !guard->popup_guard_installed ||
            !guard->teardown_guard_installed ||
            !guard->host_mapped || !guard->actor_mapped ||
            !guard->output_active) {
        return refuse(SD_COMPANY_SCENE_GUARDS_INCOMPLETE);
    }
    if (!guard->selection_epoch ||
            !guard->intent_sequence || !guard->host_revision ||
            guard->selection_epoch != current->intent.selection_epoch ||
            guard->intent_sequence != current->intent.sequence ||
            guard->host_revision != current->intent.host_revision ||
            guard->native_host_client_ref != current->host.native_client_ref ||
            guard->native_actor_root_surface_ref !=
                    current->actor.native_root_surface_ref) {
        return refuse(SD_COMPANY_SCENE_STALE_OWNER);
    }

    /* No scaling, no pretending a 840px source can fill 1493px.
     * M31a/M31c proved that misinterpreted RGB565 rows shear the image.
     * Start with only native Qt roots matching the desired canvas exactly. */
    if (exact_native_source.x != 0 || exact_native_source.y != 0 ||
            !stage_director_rect_valid(exact_native_source) ||
            exact_native_source.width != pair.viewport_global.width ||
            exact_native_source.height != pair.viewport_global.height ||
            exact_native_source.width != current->actor.root_global.width ||
            exact_native_source.height != current->actor.root_global.height) {
        return refuse(SD_COMPANY_SCENE_SOURCE_NOT_EXACT);
    }
    if (!stage_director_rect_contains(current->host.output_global,
                    pair.viewport_global) ||
            !stage_director_rect_contains(current->host.host_global,
                    pair.viewport_global)) {
        return refuse(SD_COMPANY_SCENE_UNSAFE_GEOMETRY);
    }

    return (struct stage_director_company_scene_plan) {
        .state = SD_COMPANY_SCENE_READY_FOR_PRIVATE_CI_ONLY,
        .geometry_ready = true,
        .can_apply_in_production = false,
        .target_global = pair.viewport_global,
        .root_surface_clip = {
            .x = 0, .y = 0,
            .width = pair.viewport_global.width,
            .height = pair.viewport_global.height,
        },
    };
}

const char *stage_director_company_scene_state_name(
        enum stage_director_company_scene_state state) {
    switch (state) {
    case SD_COMPANY_SCENE_NO_PAIR: return "NO_PAIR";
    case SD_COMPANY_SCENE_UNTRUSTED_EXECUTABLE: return "UNTRUSTED_EXECUTABLE";
    case SD_COMPANY_SCENE_GUARDS_INCOMPLETE: return "GUARDS_INCOMPLETE";
    case SD_COMPANY_SCENE_STALE_OWNER: return "STALE_OWNER";
    case SD_COMPANY_SCENE_SOURCE_NOT_EXACT: return "SOURCE_NOT_EXACT";
    case SD_COMPANY_SCENE_UNSAFE_GEOMETRY: return "UNSAFE_GEOMETRY";
    case SD_COMPANY_SCENE_READY_FOR_PRIVATE_CI_ONLY:
        return "READY_FOR_PRIVATE_CI_ONLY";
    }
    return "UNKNOWN";
}
