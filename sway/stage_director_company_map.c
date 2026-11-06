#include <limits.h>
#include <stdint.h>
#include "sway/stage_director_company_map.h"

/*
 * The compositor must have independently admitted the exact host layer,
 * native actor ROOT wl_surface, and Linux process generation. This is pure
 * policy on those observations, not a way for public data to mint a lease.
 */
static struct stage_director_company_map_result denied(
		enum stage_director_company_map_status status) {
	return (struct stage_director_company_map_result) {
		.status = status,
	};
}

static bool lease_valid(const struct stage_director_company_map_lease *lease) {
	return lease && lease->compositor_selected && lease->epoch &&
		lease->native_host_layer_ref && lease->native_host_surface_ref &&
		lease->native_host_client_ref && lease->native_output_ref &&
		lease->native_actor_view_ref &&
		lease->native_actor_root_surface_ref &&
		lease->native_actor_client_ref && lease->actor_pid > 0 &&
		lease->actor_uid != 0 && lease->actor_start_ticks > 0 &&
		lease->native_host_surface_ref != lease->native_actor_root_surface_ref &&
		lease->native_host_client_ref != lease->native_actor_client_ref;
}

static bool native_peers_match(
		const struct stage_director_company_map_lease *lease,
		const struct stage_director_company_map_observed *now) {
	return now->native_host_layer_ref == lease->native_host_layer_ref &&
		now->native_host_surface_ref == lease->native_host_surface_ref &&
		now->native_host_client_ref == lease->native_host_client_ref &&
		now->native_output_ref == lease->native_output_ref &&
		now->native_actor_view_ref == lease->native_actor_view_ref &&
		now->native_actor_root_surface_ref ==
			lease->native_actor_root_surface_ref &&
		now->native_actor_client_ref == lease->native_actor_client_ref;
}

struct stage_director_company_map_result
stage_director_company_map_probe(
		const struct stage_director_company_map_lease *lease,
		const struct stage_director_company_map_observed *now,
		const struct stage_director_company_map_report *report) {
	if (!lease_valid(lease) || !now || !report) {
		return denied(SD_COMPANY_MAP_NO_LEASE);
	}
	if (!native_peers_match(lease, now) ||
			!now->actor_root_is_toplevel ||
			!now->actor_process_attested) {
		return denied(SD_COMPANY_MAP_WRONG_NATIVE_PEER);
	}
	/* Actor PID reuse is NOT actor continuity. Native view, root surface,
	 * UID and Linux process start time must all remain the same. */
	if (now->actor_pid != lease->actor_pid ||
			now->actor_uid != lease->actor_uid ||
			now->actor_start_ticks != lease->actor_start_ticks) {
		return denied(SD_COMPANY_MAP_STALE_ACTOR);
	}
	/* The report never supplies native pointers or a PID. Its epoch and
	 * host geometry revision are checked against compositor observations. */
	if (!now->epoch || !now->host_geometry_revision ||
			now->epoch != lease->epoch ||
			report->epoch != lease->epoch ||
			report->host_geometry_revision != now->host_geometry_revision) {
		return denied(SD_COMPANY_MAP_STALE_HOST);
	}
	if (!report->company_open || !report->coverage_active ||
			!report->native_map_running || !now->host_mapped ||
			!now->actor_mapped || !now->output_active ||
			(report->map_in_frame.x == 0 &&
			 report->map_in_frame.y == 0 &&
			 report->map_in_frame.width == 0 &&
			 report->map_in_frame.height == 0)) {
		return denied(SD_COMPANY_MAP_HIDDEN);
	}

	/* Company M18's coordinates start at companyFrame, NOT the native
	 * layer-shell host origin. Its frame must first be bounded in host
	 * coordinates. Compute via 64-bit intermediates to reject overflow. */
	const struct stage_director_rect native_local = {
		.x = 0,
		.y = 0,
		.width = now->host_global.width,
		.height = now->host_global.height,
	};
	const struct stage_director_rect frame_local = {
		.x = 0,
		.y = 0,
		.width = report->frame_in_host.width,
		.height = report->frame_in_host.height,
	};
	if (!stage_director_rect_valid(now->host_global) ||
			!stage_director_rect_valid(now->output_global) ||
			!stage_director_rect_contains(now->output_global,
				now->host_global) ||
			!stage_director_rect_contains(native_local,
				report->frame_in_host) ||
			!stage_director_rect_contains(frame_local,
				report->map_in_frame)) {
		return denied(SD_COMPANY_MAP_INVALID_GEOMETRY);
	}
	const int64_t x = (int64_t)report->frame_in_host.x +
		report->map_in_frame.x;
	const int64_t y = (int64_t)report->frame_in_host.y +
		report->map_in_frame.y;
	if (x < INT_MIN || x > INT_MAX || y < INT_MIN || y > INT_MAX) {
		return denied(SD_COMPANY_MAP_INVALID_GEOMETRY);
	}
	const struct stage_director_rect local_canvas = {
		.x = (int)x,
		.y = (int)y,
		.width = report->map_in_frame.width,
		.height = report->map_in_frame.height,
	};
	struct stage_director_rect global = {0};
	if (!stage_director_viewport_from_host_local(
			now->host_global, local_canvas, &global) ||
			!stage_director_rect_contains(now->output_global, global)) {
		return denied(SD_COMPANY_MAP_INVALID_GEOMETRY);
	}

	/* Reuse SwayPX's existing, conservative geometry-only actor crop
	 * policy. Valid geometry does NOT grant render or input ownership. */
	const struct stage_director_viewport_request geometry = {
		.host = now->host_global,
		.output = now->output_global,
		.actor = now->actor_global,
		.viewport = global,
		.host_mapped = now->host_mapped,
		.actor_mapped = now->actor_mapped,
		.output_active = now->output_active,
		.actor_identity_verified = now->actor_process_attested,
	};
	const struct stage_director_viewport_plan plan =
		stage_director_viewport_plan(&geometry);
	if (plan.result != SD_VIEWPORT_GEOMETRY_ONLY ||
			!plan.geometry_valid || plan.can_launch ||
			plan.can_render || plan.can_accept_input) {
		return denied(SD_COMPANY_MAP_ACTOR_NOT_COVERING);
	}
	return (struct stage_director_company_map_result) {
		.status = SD_COMPANY_MAP_CANDIDATE_ONLY,
		.geometry_candidate = true,
		.viewport_global = global,
		.actor_local_crop = plan.actor_local_crop,
		/* The remaining authority fields are zero by construction. */
	};
}

const char *stage_director_company_map_status_name(
		enum stage_director_company_map_status status) {
	switch (status) {
	case SD_COMPANY_MAP_NO_LEASE: return "NO_LEASE";
	case SD_COMPANY_MAP_WRONG_NATIVE_PEER: return "WRONG_NATIVE_PEER";
	case SD_COMPANY_MAP_STALE_HOST: return "STALE_HOST";
	case SD_COMPANY_MAP_STALE_ACTOR: return "STALE_ACTOR";
	case SD_COMPANY_MAP_HIDDEN: return "HIDDEN";
	case SD_COMPANY_MAP_INVALID_GEOMETRY: return "INVALID_GEOMETRY";
	case SD_COMPANY_MAP_ACTOR_NOT_COVERING: return "ACTOR_NOT_COVERING";
	case SD_COMPANY_MAP_CANDIDATE_ONLY: return "CANDIDATE_ONLY";
	}
	return "DENIED_UNKNOWN";
}
