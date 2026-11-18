/* Post-Apollo autoscroll: portable evidence policy, no compositor hooks. */
#include <stddef.h>
#include "sway/input/post_apollo_autoscroll_witness.h"

bool pa_autoscroll_witness_admissible(
    const struct pa_autoscroll_witness *witness, uint64_t now_ms) {
    if (witness == NULL
            || witness->origin != PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT
            || !witness->compositor_event
            || !witness->surface_hit_verified
            || !witness->target_mapped
            || !witness->output_enabled
            || !witness->layout_generation_verified
            || witness->session_locked
            || witness->native_scroll_lock_active
            || witness->stage_director_pairing_consumed
            || witness->pointer_constrained
            || witness->device_id == 0
            || witness->seat_id == 0
            || witness->target_node_id == 0
            || witness->output_id == 0
            || witness->layout_generation == 0
            || witness->observed_at_ms == 0
            || now_ms < witness->observed_at_ms) {
        return false;
    }
    return now_ms - witness->observed_at_ms
        <= PA_AUTOSCROLL_WITNESS_MAX_AGE_MS;
}

bool pa_autoscroll_witness_same_session(
    const struct pa_autoscroll_witness *started,
    const struct pa_autoscroll_witness *current, uint64_t now_ms) {
    if (started == NULL || current == NULL
            || !pa_autoscroll_witness_admissible(
                started, started->observed_at_ms)
            || !pa_autoscroll_witness_admissible(current, now_ms)
            || current->observed_at_ms < started->observed_at_ms) {
        return false;
    }

    return started->device_id == current->device_id
        && started->seat_id == current->seat_id
        && started->target_node_id == current->target_node_id
        && started->output_id == current->output_id
        && started->layout_generation == current->layout_generation;
}
