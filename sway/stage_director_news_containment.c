#include <math.h>
#include "sway/stage_director_news_containment.h"

static struct stage_director_news_containment_result state(
		enum stage_director_news_containment_status status) {
	return (struct stage_director_news_containment_result) {
		.status = status,
		.geometry_candidate = false,
		.pointer_candidate = false,
		.popup_candidate = false,
		.can_dispatch_input = false,
		.can_show_popup = false,
		.can_enable_clip = false,
	};
}

/* A candidate is valid only for the same native refs and scene generation. */
static struct stage_director_news_containment_result check(
		const struct stage_director_news_containment_snapshot *snapshot,
		uint64_t current_epoch, uintptr_t current_host_ref,
		uintptr_t actor_ref) {
	if (!snapshot || !snapshot->session_owner_verified ||
			!snapshot->actor_process_verified ||
			snapshot->epoch == 0 || !snapshot->native_host_ref ||
			!snapshot->native_actor_ref ||
			current_epoch != snapshot->epoch ||
			current_host_ref != snapshot->native_host_ref ||
			actor_ref != snapshot->native_actor_ref) {
		return state(SD_NEWS_CONTAINMENT_NO_SESSION);
	}
	const struct stage_director_viewport_plan geometry =
		stage_director_viewport_plan(&snapshot->request);
	if (!geometry.geometry_valid ||
			geometry.result != SD_VIEWPORT_GEOMETRY_ONLY) {
		return state(SD_NEWS_CONTAINMENT_STALE_GEOMETRY);
	}
	struct stage_director_news_containment_result result =
		state(SD_NEWS_CONTAINMENT_CANDIDATE_ONLY);
	result.geometry_candidate = true;
	return result;
}

struct stage_director_news_containment_result
stage_director_news_pointer_probe(
		const struct stage_director_news_containment_snapshot *snapshot,
		uint64_t current_epoch, uintptr_t current_host_ref,
		uintptr_t candidate_target_ref, double global_x, double global_y) {
	struct stage_director_news_containment_result result = check(
		snapshot, current_epoch, current_host_ref, candidate_target_ref);
	if (!result.geometry_candidate) return result;
	if (!isfinite(global_x) || !isfinite(global_y)) {
		return state(SD_NEWS_CONTAINMENT_INVALID_COORDS);
	}
	const struct stage_director_rect b = snapshot->request.viewport;
	/* Keep fractional coordinates, never round into the protected bay. */
	if (global_x < (double)b.x || global_y < (double)b.y ||
			global_x >= (double)b.x + b.width ||
			global_y >= (double)b.y + b.height) {
		return state(SD_NEWS_CONTAINMENT_OUTSIDE_VIEWPORT);
	}
	result.pointer_candidate = true;
	return result;
}

struct stage_director_news_containment_result
stage_director_news_popup_probe(
		const struct stage_director_news_containment_snapshot *snapshot,
		uint64_t current_epoch, uintptr_t current_host_ref,
		uintptr_t candidate_actor_ref,
		struct stage_director_rect popup_global_bounds) {
	struct stage_director_news_containment_result result = check(
		snapshot, current_epoch, current_host_ref, candidate_actor_ref);
	if (!result.geometry_candidate) return result;
	/* Popups can exist in Sway's independent root->layers.popup scene tree.
	 * A normal browser surface clip does NOT confine them. */
	if (!stage_director_rect_valid(popup_global_bounds)) {
		return state(SD_NEWS_CONTAINMENT_INVALID_COORDS);
	}
	if (!stage_director_rect_contains(snapshot->request.viewport,
			popup_global_bounds)) {
		return state(SD_NEWS_CONTAINMENT_POPUP_OVERFLOW);
	}
	result.popup_candidate = true;
	return result;
}
