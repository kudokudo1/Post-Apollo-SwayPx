#include <stdint.h>
#include "sway/stage_director_viewport.h"

/*
 * int64_t prevents a rectangle at INT_MAX from wrapping around and passing
 * a containment check. Zero/negative sizes are always rejected. Global
 * compositor coordinates may legitimately be negative on another output.
 */
bool stage_director_rect_valid(struct stage_director_rect rect) {
	return rect.width > 0 && rect.height > 0 &&
		(int64_t)rect.x + rect.width <= INT32_MAX &&
		(int64_t)rect.y + rect.height <= INT32_MAX &&
		(int64_t)rect.x + rect.width >= INT32_MIN &&
		(int64_t)rect.y + rect.height >= INT32_MIN;
}

bool stage_director_rect_contains(struct stage_director_rect outer,
		struct stage_director_rect inner) {
	if (!stage_director_rect_valid(outer) || !stage_director_rect_valid(inner)) {
		return false;
	}
	return inner.x >= outer.x && inner.y >= outer.y &&
		(int64_t)inner.x + inner.width <= (int64_t)outer.x + outer.width &&
		(int64_t)inner.y + inner.height <= (int64_t)outer.y + outer.height;
}

bool stage_director_rect_contains_point(struct stage_director_rect rect,
		int x, int y) {
	return stage_director_rect_valid(rect) &&
		x >= rect.x && y >= rect.y &&
		(int64_t)x < (int64_t)rect.x + rect.width &&
		(int64_t)y < (int64_t)rect.y + rect.height;
}

struct stage_director_viewport_plan stage_director_viewport_plan(
		const struct stage_director_viewport_request *request) {
	struct stage_director_viewport_plan plan = {
		.result = SD_VIEWPORT_DENY_UNVERIFIED_ACTOR,
		/* No geometry computation authorizes compositor mutation. */
		.can_launch = false,
		.can_render = false,
		.can_accept_input = false,
	};

	if (!request || !request->actor_identity_verified) {
		return plan;
	}
	if (!request->host_mapped || !request->actor_mapped) {
		plan.result = SD_VIEWPORT_DENY_UNMAPPED;
		return plan;
	}
	if (!request->output_active) {
		plan.result = SD_VIEWPORT_DENY_OUTPUT_INACTIVE;
		return plan;
	}
	if (!stage_director_rect_valid(request->host) ||
			!stage_director_rect_valid(request->actor) ||
			!stage_director_rect_valid(request->output) ||
			!stage_director_rect_valid(request->viewport)) {
		plan.result = SD_VIEWPORT_DENY_INVALID_GEOMETRY;
		return plan;
	}
	if (!stage_director_rect_contains(request->host, request->viewport)) {
		plan.result = SD_VIEWPORT_DENY_VIEWPORT_OUTSIDE_HOST;
		return plan;
	}
	if (!stage_director_rect_contains(request->output, request->viewport)) {
		plan.result = SD_VIEWPORT_DENY_VIEWPORT_OUTSIDE_OUTPUT;
		return plan;
	}
	if (!stage_director_rect_contains(request->actor, request->viewport)) {
		plan.result = SD_VIEWPORT_DENY_ACTOR_NOT_COVERING_VIEWPORT;
		return plan;
	}

	plan.actor_local_crop = (struct stage_director_rect) {
		.x = request->viewport.x - request->actor.x,
		.y = request->viewport.y - request->actor.y,
		.width = request->viewport.width,
		.height = request->viewport.height,
	};
	plan.geometry_valid = true;
	plan.result = SD_VIEWPORT_GEOMETRY_ONLY;
	return plan;
}

const char *stage_director_viewport_result_name(
		enum stage_director_viewport_result result) {
	switch (result) {
	case SD_VIEWPORT_DENY_UNVERIFIED_ACTOR:
		return "DENY_UNVERIFIED_ACTOR";
	case SD_VIEWPORT_DENY_UNMAPPED:
		return "DENY_UNMAPPED";
	case SD_VIEWPORT_DENY_OUTPUT_INACTIVE:
		return "DENY_OUTPUT_INACTIVE";
	case SD_VIEWPORT_DENY_INVALID_GEOMETRY:
		return "DENY_INVALID_GEOMETRY";
	case SD_VIEWPORT_DENY_VIEWPORT_OUTSIDE_HOST:
		return "DENY_VIEWPORT_OUTSIDE_HOST";
	case SD_VIEWPORT_DENY_VIEWPORT_OUTSIDE_OUTPUT:
		return "DENY_VIEWPORT_OUTSIDE_OUTPUT";
	case SD_VIEWPORT_DENY_ACTOR_NOT_COVERING_VIEWPORT:
		return "DENY_ACTOR_NOT_COVERING_VIEWPORT";
	case SD_VIEWPORT_GEOMETRY_ONLY:
		return "GEOMETRY_ONLY";
	}
	return "DENY_UNKNOWN";
}
