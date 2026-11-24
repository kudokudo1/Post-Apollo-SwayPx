#ifndef SWAY_INPUT_POST_APOLLO_AUTOSCROLL_CONTINUITY_H
#define SWAY_INPUT_POST_APOLLO_AUTOSCROLL_CONTINUITY_H

/*
 * V5F-G3A: DETACHED, UNLINKED continuity candidate. Not installed or armed.
 *
 * A real native middle-click must be authenticated by an independent,
 * compositor-internal physical event producer BEFORE calling begin().
 * Nothing in this file attests a mouse or grants permission to scroll.
 * Do not expose either function through IPC, QML, tests-to-production toggles,
 * or any untrusted caller. All scene data must be reread from live owners
 * immediately before EACH future axis/frame dispatch.
 */
#include <stdbool.h>
#include <stdint.h>
#include "sway/input/post_apollo_autoscroll_epoch.h"

/* Scene age is about an actual owner-state read, NOT new input activity. */
#define PA_AUTOSCROLL_CONTINUITY_MAX_SCENE_AGE_MS UINT64_C(48)
/* Missed timer / resume after pause must cancel rather than burst-scroll. */
#define PA_AUTOSCROLL_CONTINUITY_MAX_TICK_GAP_MS UINT64_C(250)

struct pa_autoscroll_continuity_scene {
    struct pa_autoscroll_object_epochs owners;
    uint64_t device_id;
    uint64_t seat_id;
    uint64_t target_node_id;
    uint64_t output_id;
    uint64_t layout_generation;
    uint64_t captured_at_ms;
    bool surface_hit_verified;
    bool target_mapped;
    bool output_enabled;
    bool layout_generation_verified;
    bool session_locked;
    bool native_scroll_lock_active;
    bool stage_director_pairing_consumed;
    bool pointer_constrained;
};

struct pa_autoscroll_continuity_guard {
    struct pa_autoscroll_epoch_guard start;
    uint64_t last_tick_ms;
    bool active;
};

/* Begin is test-only data validation, NOT physical origin authentication.
 * The caller must independently verify actual native event provenance.
 */
bool pa_autoscroll_continuity_begin(
    struct pa_autoscroll_continuity_guard *guard,
    const struct pa_autoscroll_witness *event,
    const struct pa_autoscroll_object_epochs *owners,
    uint64_t now_ms);

/* The timer never supplies a new input witness. An independently sampled
 * current scene must remain bound to the original trusted event + owners.
 * A failure permanently cancels until a NEW, authenticated begin().
 */
bool pa_autoscroll_continuity_tick(
    struct pa_autoscroll_continuity_guard *guard,
    const struct pa_autoscroll_continuity_scene *scene,
    uint64_t now_ms);

void pa_autoscroll_continuity_cancel(
    struct pa_autoscroll_continuity_guard *guard);

#endif
