/*
 * V5F-G3A: UNLINKED, DETACHED continuity policy candidate.
 *
 * The old V5C epoch guard rightly refuses a replayed input event at each
 * timer tick, but a stationary pointer produces no new input events.
 * Retain exact V5C begin checks; tick instead revalidates a FRESHLY READ
 * trusted compositor scene and all FOUR original owner epochs.
 *
 * This module does not read a physical input event, authorize a new origin,
 * dispatch Wayland axes/frames, or register a timer. NOT linked by Meson.
 * It cannot authenticate a dishonest caller: never wire up until an
 * independently reviewed native evidence producer and dispatch ordering exist.
 */
#include <stddef.h>
#include <string.h>
#include "sway/input/post_apollo_autoscroll_continuity.h"

void pa_autoscroll_continuity_cancel(
        struct pa_autoscroll_continuity_guard *guard) {
    if (guard != NULL) {
        memset(guard, 0, sizeof(*guard));
    }
}

bool pa_autoscroll_continuity_begin(
        struct pa_autoscroll_continuity_guard *guard,
        const struct pa_autoscroll_witness *event,
        const struct pa_autoscroll_object_epochs *owners,
        uint64_t now_ms) {
    if (guard == NULL) {
        return false;
    }
    pa_autoscroll_continuity_cancel(guard);
    if (!pa_autoscroll_epoch_begin(&guard->start, event, owners, now_ms)) {
        return false;
    }
    guard->last_tick_ms = now_ms;
    guard->active = true;
    return true;
}

static bool same_owners(
        const struct pa_autoscroll_object_epochs *started,
        const struct pa_autoscroll_object_epochs *current) {
    return started != NULL && current != NULL
        && started->device != 0 && started->seat != 0
        && started->target != 0 && started->output != 0
        && started->device == current->device
        && started->seat == current->seat
        && started->target == current->target
        && started->output == current->output;
}

static bool scene_matches(
        const struct pa_autoscroll_epoch_guard *begin,
        const struct pa_autoscroll_continuity_scene *scene,
        uint64_t now_ms) {
    if (begin == NULL || scene == NULL || !begin->active
            || !same_owners(&begin->started_epochs, &scene->owners)
            || scene->captured_at_ms == 0
            || now_ms < scene->captured_at_ms
            || now_ms - scene->captured_at_ms
                > PA_AUTOSCROLL_CONTINUITY_MAX_SCENE_AGE_MS
            || !scene->surface_hit_verified
            || !scene->target_mapped
            || !scene->output_enabled
            || !scene->layout_generation_verified
            || scene->session_locked
            || scene->native_scroll_lock_active
            || scene->stage_director_pairing_consumed
            || scene->pointer_constrained) {
        return false;
    }
    /* IDs/layout are not authorization, only binding to the start event.
     * A kernel-origin event and the scene read need independent custody.
     */
    const struct pa_autoscroll_witness *start = &begin->started;
    return scene->device_id == start->device_id
        && scene->seat_id == start->seat_id
        && scene->target_node_id == start->target_node_id
        && scene->output_id == start->output_id
        && scene->layout_generation == start->layout_generation;
}

bool pa_autoscroll_continuity_tick(
        struct pa_autoscroll_continuity_guard *guard,
        const struct pa_autoscroll_continuity_scene *scene,
        uint64_t now_ms) {
    if (guard == NULL || !guard->active) {
        return false;
    }
    /* Timestamp is a MONOTONIC timer clock. It is never evidence of a new
     * physical event. A lengthy scheduler gap cancels before scroll output.
     */
    if (now_ms == 0 || now_ms <= guard->last_tick_ms
            || now_ms - guard->last_tick_ms
                > PA_AUTOSCROLL_CONTINUITY_MAX_TICK_GAP_MS
            || !scene_matches(&guard->start, scene, now_ms)) {
        pa_autoscroll_continuity_cancel(guard);
        return false;
    }
    guard->last_tick_ms = now_ms;
    return true;
}
