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

struct stage_director_viewport_clip_plan stage_director_viewport_prepare_clip(
		const struct stage_director_viewport_request *request,
		const struct stage_director_viewport_authority *authority,
		int root_surface_geometry_x,
		int root_surface_geometry_y) {
	struct stage_director_viewport_clip_plan result = {
		.geometry_result = SD_VIEWPORT_DENY_UNVERIFIED_ACTOR,
	};
	const struct stage_director_viewport_plan geometry =
		stage_director_viewport_plan(request);
	result.geometry_result = geometry.result;
	if (geometry.result != SD_VIEWPORT_GEOMETRY_ONLY || !geometry.geometry_valid) {
		return result;
	}
	if (!authority || !authority->operator_enabled ||
			!authority->host_session_verified ||
			!authority->actor_process_verified ||
			!authority->input_hit_test_guard_installed ||
			!authority->popup_guard_installed ||
			!authority->teardown_guard_installed) {
		return result;
	}
	/*
	 * XDG window geometry can differ from the root wl_surface origin.
	 * Convert actor-local viewport coordinates into wl_surface-local box.
	 * Check 64-bit sums before narrowing back to compositor int.
	 */
	const int64_t crop_x = (int64_t)geometry.actor_local_crop.x +
		root_surface_geometry_x;
	const int64_t crop_y = (int64_t)geometry.actor_local_crop.y +
		root_surface_geometry_y;
	if (crop_x < INT32_MIN || crop_x > INT32_MAX ||
			crop_y < INT32_MIN || crop_y > INT32_MAX) {
		return result;
	}
	struct stage_director_rect crop = {
		.x = (int)crop_x,
		.y = (int)crop_y,
		.width = geometry.actor_local_crop.width,
		.height = geometry.actor_local_crop.height,
	};
	if (!stage_director_rect_valid(crop)) {
		return result;
	}
	result.root_surface_clip = crop;
	result.allowed = true;
	return result;
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


/* This is a pure policy, never a credential issuer nor a scene setter. */
static bool frame_same_rect(struct stage_director_rect a,
		struct stage_director_rect b) {
	return a.x == b.x && a.y == b.y &&
		a.width == b.width && a.height == b.height;
}

struct stage_director_viewport_frame_decision
stage_director_viewport_frame_policy(
		const struct stage_director_viewport_frame_owner *owner,
		const struct stage_director_viewport_frame_observation *frame) {
	struct stage_director_viewport_frame_decision result = {
		.state = SD_FRAME_DEFAULT_NO_OWNER,
	};
	if (!owner || !frame || owner->epoch == 0 ||
			!owner->native_host_ref || !owner->native_actor_ref ||
			!owner->native_output_ref || !owner->native_root_surface_ref) {
		return result;
	}
	if (frame->epoch != owner->epoch ||
			frame->native_host_ref != owner->native_host_ref ||
			frame->native_actor_ref != owner->native_actor_ref ||
			frame->native_output_ref != owner->native_output_ref) {
		result.state = SD_FRAME_DEFAULT_STALE_OWNER;
		return result;
	}
	if (!frame->native_root_surface ||
			frame->native_root_surface_ref != owner->native_root_surface_ref) {
		result.state = SD_FRAME_DEFAULT_DIFFERENT_SURFACE;
		return result;
	}
	/*
	 * Any host/output/viewport reconfiguration invalidates the old frame
	 * lease. Actor movement is evaluated using fresh geometry every frame;
	 * it must still contain the exact authorized viewport.
	 */
	if (!frame_same_rect(owner->authorized_host, frame->geometry.host) ||
			!frame_same_rect(owner->authorized_output, frame->geometry.output) ||
			!frame_same_rect(owner->authorized_viewport,
				frame->geometry.viewport)) {
		result.state = SD_FRAME_DEFAULT_STALE_GEOMETRY;
		return result;
	}
	const struct stage_director_viewport_plan geometry =
		stage_director_viewport_plan(&frame->geometry);
	if (geometry.result != SD_VIEWPORT_GEOMETRY_ONLY ||
			!geometry.geometry_valid) {
		result.state = SD_FRAME_DEFAULT_STALE_GEOMETRY;
		return result;
	}
	const struct stage_director_viewport_clip_plan clip =
		stage_director_viewport_prepare_clip(&frame->geometry,
			&owner->authority, frame->root_surface_geometry_x,
			frame->root_surface_geometry_y);
	if (!clip.allowed) {
		result.state = SD_FRAME_DEFAULT_GUARDS_MISSING;
		return result;
	}
	/*
	 * Root wl_surface SOURCE crop and renderer DESTINATION must advance
	 * together. PR #28 demonstrated that a narrow src crop alone will be
	 * stretched by SwayPX output_configure_scene to full content geometry.
	 * This function intentionally does not call any scene setter.
	 */
	result.root_surface_crop = clip.root_surface_clip;
	result.proposed_dest_width = clip.root_surface_clip.width;
	result.proposed_dest_height = clip.root_surface_clip.height;
	result.proposed_override = true;
	result.state = SD_FRAME_PLAN_DESTINATION_ONLY;
	return result;
}

const char *stage_director_viewport_frame_state_name(
		enum stage_director_viewport_frame_state state) {
	switch (state) {
	case SD_FRAME_DEFAULT_NO_OWNER: return "DEFAULT_NO_OWNER";
	case SD_FRAME_DEFAULT_STALE_OWNER: return "DEFAULT_STALE_OWNER";
	case SD_FRAME_DEFAULT_DIFFERENT_SURFACE: return "DEFAULT_DIFFERENT_SURFACE";
	case SD_FRAME_DEFAULT_STALE_GEOMETRY: return "DEFAULT_STALE_GEOMETRY";
	case SD_FRAME_DEFAULT_GUARDS_MISSING: return "DEFAULT_GUARDS_MISSING";
	case SD_FRAME_PLAN_DESTINATION_ONLY: return "PLAN_DESTINATION_ONLY";
	}
	return "DEFAULT_UNKNOWN";
}
