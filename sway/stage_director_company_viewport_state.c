#include <string.h>
#include "sway/stage_director_company_viewport_state.h"
#include "sway/stage_director_company_wire_policy.h"

void stage_director_company_viewport_state_reset(
        struct stage_director_company_viewport_state *state) {
    if (state) {
        memset(state, 0, sizeof(*state));
    }
}

bool stage_director_company_viewport_state_record(
        struct stage_director_company_viewport_state *state,
        uint64_t selected_epoch, uint64_t intent_sequence,
        uint64_t current_host_revision, uintptr_t selected_host_client_ref,
        struct stage_director_rect current_native_host_local,
        struct stage_director_rect frame_in_host,
        struct stage_director_rect map_in_frame) {
    /* Never let a malformed or hidden report retain an older good crop. */
    stage_director_company_viewport_state_reset(state);
    if (!state || !selected_epoch || !intent_sequence ||
            !current_host_revision || !selected_host_client_ref) {
        return false;
    }
    const struct stage_director_company_wire_result checked =
        stage_director_company_wire_check(
            true, true, current_native_host_local,
            frame_in_host, map_in_frame);
    if (checked.state != SD_COMPANY_WIRE_VIEWPORT_CANDIDATE ||
            !checked.geometry_candidate ||
            checked.can_launch || checked.can_render || checked.can_clip ||
            checked.can_accept_input || checked.can_focus ||
            checked.can_show_popup) {
        return false;
    }
    *state = (struct stage_director_company_viewport_state) {
        .candidate = true,
        .selection_epoch = selected_epoch,
        .intent_sequence = intent_sequence,
        .host_revision = current_host_revision,
        .native_host_client_ref = selected_host_client_ref,
        .frame_in_host = frame_in_host,
        .map_in_frame = map_in_frame,
    };
    return true;
}

bool stage_director_company_viewport_state_snapshot(
        const struct stage_director_company_viewport_state *state,
        uint64_t selected_epoch, uint64_t current_intent_sequence,
        uint64_t current_host_revision, uintptr_t selected_host_client_ref,
        struct stage_director_rect current_native_host_local,
        struct stage_director_company_viewport_snapshot *out) {
    if (!out) return false;
    *out = (struct stage_director_company_viewport_snapshot){0};
    if (!state || !state->candidate ||
            !selected_epoch || !current_intent_sequence ||
            !current_host_revision || !selected_host_client_ref ||
            state->selection_epoch != selected_epoch ||
            state->intent_sequence != current_intent_sequence ||
            state->host_revision != current_host_revision ||
            state->native_host_client_ref != selected_host_client_ref) {
        return false;
    }

    /* The app's previous rectangle is not trusted merely because it was
     * valid at report time. Re-run bounds with the compositor's freshly
     * observed native host dimensions and current selection generation. */
    const struct stage_director_company_wire_result checked =
        stage_director_company_wire_check(
            true, true, current_native_host_local,
            state->frame_in_host, state->map_in_frame);
    if (checked.state != SD_COMPANY_WIRE_VIEWPORT_CANDIDATE ||
            !checked.geometry_candidate ||
            checked.can_launch || checked.can_render || checked.can_clip ||
            checked.can_accept_input || checked.can_focus ||
            checked.can_show_popup) {
        return false;
    }
    *out = (struct stage_director_company_viewport_snapshot) {
        .candidate_only = true,
        .selection_epoch = selected_epoch,
        .intent_sequence = current_intent_sequence,
        .host_revision = current_host_revision,
        .native_host_client_ref = selected_host_client_ref,
        .frame_in_host = state->frame_in_host,
        .map_in_frame = state->map_in_frame,
        .map_in_host = checked.map_in_host,
    };
    return true;
}
