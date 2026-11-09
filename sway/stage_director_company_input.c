#include <math.h>
#include "sway/stage_director_company_input.h"

/* No code in this file can mint a scene lease, change focus or forward a
 * pointer event. The live compositor must supply observed root/view refs. */
static bool active(const struct stage_director_company_input_snapshot *scene) {
    return scene && scene->scene_active && scene->selection_epoch &&
        scene->native_actor_view_ref &&
        scene->native_actor_root_surface_ref &&
        stage_director_rect_valid(scene->map_global);
}

struct stage_director_company_input_result
stage_director_company_input_pointer(
        const struct stage_director_company_input_snapshot *scene,
        uintptr_t hit_root_ref, double x, double y, bool locked) {
    if (!scene || !scene->scene_active)
        return (struct stage_director_company_input_result) {
            .verdict = SD_COMPANY_INPUT_NO_EMBED,
        };
    if (hit_root_ref != scene->native_actor_root_surface_ref ||
            !hit_root_ref)
        return (struct stage_director_company_input_result) {
            .verdict = SD_COMPANY_INPUT_UNRELATED_SURFACE,
        };
    if (!active(scene) || locked || !isfinite(x) || !isfinite(y) ||
            x < (double)scene->map_global.x ||
            y < (double)scene->map_global.y ||
            x >= (double)scene->map_global.x + scene->map_global.width ||
            y >= (double)scene->map_global.y + scene->map_global.height)
        return (struct stage_director_company_input_result) {
            .verdict = SD_COMPANY_INPUT_DENY_STALE_OR_OUTSIDE,
            .deny_event = true,
        };
    return (struct stage_director_company_input_result) {
        .verdict = SD_COMPANY_INPUT_WITHIN_MAP,
    };
}

bool stage_director_company_input_reject_popup(
        const struct stage_director_company_input_snapshot *scene,
        uintptr_t view_ref) {
    return scene && scene->scene_active &&
        view_ref && view_ref == scene->native_actor_view_ref;
}

bool stage_director_company_input_reject_keyboard(
        const struct stage_director_company_input_snapshot *scene,
        uintptr_t surface_ref) {
    return scene && scene->scene_active &&
        surface_ref && surface_ref == scene->native_actor_root_surface_ref;
}
