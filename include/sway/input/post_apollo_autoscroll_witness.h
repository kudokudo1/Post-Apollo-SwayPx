#ifndef SWAY_INPUT_POST_APOLLO_AUTOSCROLL_WITNESS_H
#define SWAY_INPUT_POST_APOLLO_AUTOSCROLL_WITNESS_H

#include <stdbool.h>
#include <stdint.h>

/*
 * A fail-closed policy for compositor-supplied pointer evidence.
 * These fields are NOT proof by themselves: the caller must derive every
 * claim from trusted SwayPX/libinput, hit-test and output-layout state.
 * Never populate a positive witness from a Wayland client or ordinary IPC.
 *
 * This portable module does not read devices, activate autoscroll, or
 * dispatch events. No production caller exists in V5B.
 */
enum pa_autoscroll_pointer_origin {
    PA_POINTER_ORIGIN_UNKNOWN = 0,
    PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT = 1,
    PA_POINTER_ORIGIN_VIRTUAL = 2,
};

struct pa_autoscroll_witness {
    enum pa_autoscroll_pointer_origin origin;
    bool compositor_event;
    bool surface_hit_verified;
    bool target_mapped;
    bool output_enabled;
    bool layout_generation_verified;
    bool session_locked;
    bool native_scroll_lock_active;
    bool stage_director_pairing_consumed;
    bool pointer_constrained;
    uint64_t device_id;
    uint64_t seat_id;
    uint64_t target_node_id;
    uint64_t output_id;
    uint64_t layout_generation;
    uint64_t observed_at_ms;
};

/* Maximum age of one observed event at the moment it is consumed. */
#define PA_AUTOSCROLL_WITNESS_MAX_AGE_MS 80u

bool pa_autoscroll_witness_admissible(
    const struct pa_autoscroll_witness *witness, uint64_t now_ms);

/* Compare a verified start snapshot to a newly observed, fresh snapshot.
 * A long session may outlive the start snapshot's freshness window; only
 * the current observation must remain fresh at each tick. The caller must
 * have checked the start snapshot when the session was created.
 */
bool pa_autoscroll_witness_same_session(
    const struct pa_autoscroll_witness *started,
    const struct pa_autoscroll_witness *current, uint64_t now_ms);

#endif
