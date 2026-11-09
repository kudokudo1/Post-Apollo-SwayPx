#ifndef _SWAY_STAGE_DIRECTOR_COMPANY_INPUT_H
#define _SWAY_STAGE_DIRECTOR_COMPANY_INPUT_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/stage_director_viewport.h"

/* M32d: actual compositor-side containment policy for the native Company
 * scene. Identity comes solely from the compositor's active actor scene
 * and wl_surface root; public client metadata cannot set active or refs.
 *
 * Production has NO active scene, hence this is a pass-through until the
 * approved scene lease and operator executable pin are implemented.
 */
struct stage_director_company_input_snapshot {
    bool scene_active;
    uint64_t selection_epoch;
    uintptr_t native_actor_view_ref;
    uintptr_t native_actor_root_surface_ref;
    struct stage_director_rect map_global;
};

enum stage_director_company_input_verdict {
    SD_COMPANY_INPUT_NO_EMBED,
    SD_COMPANY_INPUT_UNRELATED_SURFACE,
    SD_COMPANY_INPUT_WITHIN_MAP,
    SD_COMPANY_INPUT_DENY_STALE_OR_OUTSIDE,
    SD_COMPANY_INPUT_DENY_ACTOR_POPUP,
    SD_COMPANY_INPUT_DENY_ACTOR_KEYBOARD,
};

struct stage_director_company_input_result {
    enum stage_director_company_input_verdict verdict;
    bool deny_event;
};

/* Half-open global logical rectangle, no rounding or implicit margins.
 * A null/foreign root is never recognized as our native actor. */
struct stage_director_company_input_result
stage_director_company_input_pointer(
    const struct stage_director_company_input_snapshot *scene,
    uintptr_t compositor_observed_hit_root_ref,
    double global_x, double global_y, bool session_locked);

/* All native XDG popups and keyboard focus from an active embedded actor
 * are denied until independently reviewed brokers can contain them. */
bool stage_director_company_input_reject_popup(
    const struct stage_director_company_input_snapshot *scene,
    uintptr_t compositor_observed_actor_view_ref);
bool stage_director_company_input_reject_keyboard(
    const struct stage_director_company_input_snapshot *scene,
    uintptr_t compositor_observed_actor_root_ref);

/* Concrete compositor runtime bridge. No active lease can be issued here. */
struct sway_view;
struct wlr_surface;
bool stage_director_company_scene_reject_popup(struct sway_view *view);
bool stage_director_company_scene_reject_keyboard_view(struct sway_view *view);
bool stage_director_company_scene_reject_keyboard_surface(
    struct wlr_surface *surface);
bool stage_director_company_scene_reject_pointer(
    struct wlr_surface *hit_surface,
    double global_x, double global_y, bool session_locked);

#endif
