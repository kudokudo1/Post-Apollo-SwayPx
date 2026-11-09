#ifndef _SWAY_STAGE_DIRECTOR_COMPANY_SCENE_H
#define _SWAY_STAGE_DIRECTOR_COMPANY_SCENE_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/stage_director_company_actor_pair.h"

/*
 * M32c // compositor-owned scene placement and bounded crop.
 *
 * The ordinary SwayPX build retains this backend but cannot call its scene
 * setters: no operator-approved executable digest/lease and no completed
 * focus/seat/popup input guards exist. Only a private GitHub CI build can
 * compile the scene-actuation experiment.
 *
 * The future issuer MUST derive every gate from actual compositor state:
 * selected native layer, kernel-held pidfd, signed/pinned native executable,
 * exact launch, compositor-owned input/focus/popup interception, and teardown.
 * No public IPC, QML prop, app-id, argv or Wayland title can issue a gate.
 */
struct stage_director_company_scene_guard {
    bool operator_authorized_native_actor;
    bool approved_executable_and_launch;
    bool compositor_scene_lease_current;
    bool input_hit_test_guard_installed;
    bool keyboard_focus_guard_installed;
    bool popup_guard_installed;
    bool teardown_guard_installed;
    bool host_mapped;
    bool actor_mapped;
    bool output_active;
    /* Require exact current Company selection/intent/viewport generation. */
    uint64_t selection_epoch;
    uint64_t intent_sequence;
    uint64_t host_revision;
    uintptr_t native_host_client_ref;
    uintptr_t native_actor_root_surface_ref;
};

enum stage_director_company_scene_state {
    SD_COMPANY_SCENE_NO_PAIR,
    SD_COMPANY_SCENE_UNTRUSTED_EXECUTABLE,
    SD_COMPANY_SCENE_GUARDS_INCOMPLETE,
    SD_COMPANY_SCENE_STALE_OWNER,
    SD_COMPANY_SCENE_SOURCE_NOT_EXACT,
    SD_COMPANY_SCENE_UNSAFE_GEOMETRY,
    SD_COMPANY_SCENE_READY_FOR_PRIVATE_CI_ONLY,
};

struct stage_director_company_scene_plan {
    enum stage_director_company_scene_state state;
    bool geometry_ready;
    bool can_apply_in_production;
    struct stage_director_rect target_global;
    /* Source clip in the original root wl_surface's logical coordinates.
     * No arbitrary destination rescaling: source must match canvas 1:1. */
    struct stage_director_rect root_surface_clip;
};

struct stage_director_company_scene_plan
stage_director_company_scene_prepare(
    const struct stage_director_company_actor_pair_evidence *current,
    const struct stage_director_company_scene_guard *guard,
    struct stage_director_rect exact_native_source);

const char *stage_director_company_scene_state_name(
    enum stage_director_company_scene_state state);

struct sway_view;

/* The production implementation refuses all apply calls; the private CI
 * experimental build may use actual scene-node operations after ALL gates.
 * No public API exists to issue a non-ephemeral operator authorization. */
bool stage_director_company_scene_apply_ci_only(
    struct sway_view *view,
    const struct stage_director_company_actor_pair_evidence *current,
    const struct stage_director_company_scene_guard *guard);

/* Teardown never confers authority. Safe to call with a NULL view.
 * Must run before actor destruction or container scene-tree deletion. */
void stage_director_company_scene_revoke(struct sway_view *view);

#ifdef SD_STAGE_DIRECTOR_CI_COMPANY_SCENE_ACTUATION
/* M32e: disposable GitHub-only positive scene-operations probe. Its synthetic
 * approval flags are NOT real custody or a production scene lease. The
 * ordinary compositor never declares or calls this function. */
struct stage_director_company_native_host;
bool stage_director_company_scene_apply_m32e_fixture_only(
    struct sway_view *view,
    const struct stage_director_company_native_host *observed_host);
#endif

#endif
