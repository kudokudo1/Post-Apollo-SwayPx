#ifndef _SWAY_STAGE_DIRECTOR_COMPANY_VIEWPORT_STATE_H
#define _SWAY_STAGE_DIRECTOR_COMPANY_VIEWPORT_STATE_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/stage_director_viewport.h"

/*
 * M32a // Real Company Wayland report -> compositor-owned viewport state.
 *
 * The source rectangles are CLIENT-REPORTED geometry, not native identity
 * or rendering permission. Only the selected Company's private v2/v1
 * Wayland handler may record them, after re-observing its genuine mapped
 * layer client and revision. A compositor consumer must obtain all identity
 * fields from the current selection/intent, never from an app's strings.
 *
 * This is persistent production SOURCE infrastructure, not test-only.
 * There is deliberately NO scene, clip, seat, focus, popup or spawn grant.
 */
struct stage_director_company_viewport_state {
    bool candidate;
    uint64_t selection_epoch;
    uint64_t intent_sequence;
    uint64_t host_revision;
    uintptr_t native_host_client_ref;
    struct stage_director_rect frame_in_host;
    struct stage_director_rect map_in_frame;
};

/* Copyable momentary observation. Revocation invalidates previous copies;
 * never cache it as proof of continuing compositor authority. */
struct stage_director_company_viewport_snapshot {
    bool candidate_only;
    uint64_t selection_epoch;
    uint64_t intent_sequence;
    uint64_t host_revision;
    uintptr_t native_host_client_ref;
    struct stage_director_rect frame_in_host;
    struct stage_director_rect map_in_frame;
    struct stage_director_rect map_in_host;
};

void stage_director_company_viewport_state_reset(
    struct stage_director_company_viewport_state *state);

/* Each report replaces the previous candidate, including hidden/invalid.
 * Returning false always leaves the candidate absent. */
bool stage_director_company_viewport_state_record(
    struct stage_director_company_viewport_state *state,
    uint64_t selected_epoch, uint64_t intent_sequence,
    uint64_t current_host_revision, uintptr_t selected_host_client_ref,
    struct stage_director_rect current_native_host_local,
    struct stage_director_rect frame_in_host,
    struct stage_director_rect map_in_frame);

/* Revalidate on EVERY compositor use. A moved host, new intent, revoked
 * selected client, or hidden map yields no rectangle. This does not verify
 * a native map actor and never allows rendering or input. */
bool stage_director_company_viewport_state_snapshot(
    const struct stage_director_company_viewport_state *state,
    uint64_t selected_epoch, uint64_t current_intent_sequence,
    uint64_t current_host_revision, uintptr_t selected_host_client_ref,
    struct stage_director_rect current_native_host_local,
    struct stage_director_company_viewport_snapshot *out);

#endif
