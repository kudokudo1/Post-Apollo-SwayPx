#include <limits.h>
#include <stdint.h>
#include "sway/stage_director_company_wire_policy.h"

static struct stage_director_company_wire_result result(
		enum stage_director_company_wire_state state) {
	return (struct stage_director_company_wire_result){
		.state = state,
	};
}

static bool zero_rect(struct stage_director_rect r) {
	return !r.x && !r.y && !r.width && !r.height;
}

struct stage_director_company_wire_result stage_director_company_wire_check(
		bool native_host_current, bool launch_intent_observed,
		struct stage_director_rect native_host_local,
		struct stage_director_rect frame_in_host,
		struct stage_director_rect map_in_frame) {
	if (!native_host_current || !launch_intent_observed ||
			!stage_director_rect_valid(native_host_local) ||
			native_host_local.x != 0 || native_host_local.y != 0) {
		return result(SD_COMPANY_WIRE_DENIED);
	}
	/* Exact empty octuple, no old viewport to retain. */
	if (zero_rect(frame_in_host) && zero_rect(map_in_frame)) {
		return result(SD_COMPANY_WIRE_HIDDEN);
	}
	/* Only strict parent containment. Reject negative, clipped and
	 * overflow geometry rather than silently crop an unsafe rectangle. */
	const struct stage_director_rect frame_local = {
		0, 0, frame_in_host.width, frame_in_host.height,
	};
	if (!stage_director_rect_contains(
			native_host_local, frame_in_host) ||
			!stage_director_rect_contains(frame_local, map_in_frame)) {
		return result(SD_COMPANY_WIRE_INVALID_GEOMETRY);
	}
	const int64_t x = (int64_t)frame_in_host.x + map_in_frame.x;
	const int64_t y = (int64_t)frame_in_host.y + map_in_frame.y;
	if (x < 0 || y < 0 || x > INT_MAX || y > INT_MAX) {
		return result(SD_COMPANY_WIRE_INVALID_GEOMETRY);
	}
	const struct stage_director_rect canvas = {
		.x = (int)x, .y = (int)y,
		.width = map_in_frame.width, .height = map_in_frame.height,
	};
	if (!stage_director_rect_contains(native_host_local, canvas)) {
		return result(SD_COMPANY_WIRE_INVALID_GEOMETRY);
	}
	return (struct stage_director_company_wire_result) {
		.state = SD_COMPANY_WIRE_VIEWPORT_CANDIDATE,
		.map_in_host = canvas,
		.geometry_candidate = true,
		/* Never a live XDG actor identity, render, seat or clipping grant. */
	};
}
