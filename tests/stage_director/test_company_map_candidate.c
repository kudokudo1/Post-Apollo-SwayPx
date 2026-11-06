#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sway/stage_director_company_map.h"

static struct stage_director_company_map_lease lease(void) {
	return (struct stage_director_company_map_lease) {
		.compositor_selected = true,
		.epoch = 7,
		.native_host_layer_ref = 0x101,
		.native_host_surface_ref = 0x102,
		.native_host_client_ref = 0x103,
		.native_output_ref = 0x104,
		.native_actor_view_ref = 0x201,
		.native_actor_root_surface_ref = 0x202,
		.native_actor_client_ref = 0x203,
		.actor_pid = 7654,
		.actor_uid = 1000,
		.actor_start_ticks = 123456,
	};
}

static struct stage_director_company_map_observed observed(void) {
	return (struct stage_director_company_map_observed) {
		.epoch = 7,
		.host_geometry_revision = 12,
		.native_host_layer_ref = 0x101,
		.native_host_surface_ref = 0x102,
		.native_host_client_ref = 0x103,
		.native_output_ref = 0x104,
		.native_actor_view_ref = 0x201,
		.native_actor_root_surface_ref = 0x202,
		.native_actor_client_ref = 0x203,
		.actor_pid = 7654,
		.actor_uid = 1000,
		.actor_start_ticks = 123456,
		.host_mapped = true,
		.actor_mapped = true,
		.output_active = true,
		.actor_root_is_toplevel = true,
		.actor_process_attested = true,
		.host_global = {100, 40, 1000, 700},
		.output_global = {0, 0, 2560, 1440},
		.actor_global = {300, 150, 780, 600},
	};
}

static struct stage_director_company_map_report report(void) {
	return (struct stage_director_company_map_report) {
		.epoch = 7,
		.host_geometry_revision = 12,
		.company_open = true,
		.coverage_active = true,
		.native_map_running = true,
		/* companyFrame inside the native PanelWindow, not screen. */
		.frame_in_host = {12, 9, 976, 679},
		.map_in_frame = {200, 110, 650, 450},
	};
}

static void no_authority(struct stage_director_company_map_result result) {
	assert(!result.can_launch);
	assert(!result.can_render);
	assert(!result.can_clip);
	assert(!result.can_accept_input);
	assert(!result.can_show_popup);
	assert(!result.can_change_focus);
}

static struct stage_director_company_map_result check(
		struct stage_director_company_map_lease l,
		struct stage_director_company_map_observed o,
		struct stage_director_company_map_report r,
		enum stage_director_company_map_status expected) {
	const struct stage_director_company_map_result got =
		stage_director_company_map_probe(&l, &o, &r);
	assert(got.status == expected);
	no_authority(got);
	if (expected != SD_COMPANY_MAP_CANDIDATE_ONLY) {
		assert(!got.geometry_candidate);
		assert(got.viewport_global.width == 0);
		assert(got.actor_local_crop.width == 0);
	}
	return got;
}

static void valid_candidate_is_not_scene_authority(void) {
	const struct stage_director_company_map_result result = check(
		lease(), observed(), report(), SD_COMPANY_MAP_CANDIDATE_ONLY);
	assert(result.geometry_candidate);
	assert(result.viewport_global.x == 312);
	assert(result.viewport_global.y == 159);
	assert(result.viewport_global.width == 650);
	assert(result.viewport_global.height == 450);
	assert(result.actor_local_crop.x == 12);
	assert(result.actor_local_crop.y == 9);
	assert(strcmp(stage_director_company_map_status_name(result.status),
		"CANDIDATE_ONLY") == 0);
	assert(stage_director_company_map_probe(NULL, NULL, NULL).status ==
		SD_COMPANY_MAP_NO_LEASE);
}

static void wrong_host_and_actor_never_pair(void) {
	struct stage_director_company_map_lease l = lease();
	struct stage_director_company_map_observed o = observed();
	struct stage_director_company_map_report r = report();
	l.compositor_selected = false;
	check(l, o, r, SD_COMPANY_MAP_NO_LEASE);
	l = lease();
	l.native_host_client_ref = l.native_actor_client_ref;
	check(l, o, r, SD_COMPANY_MAP_NO_LEASE);
	l = lease();
	o.native_host_client_ref = 0x999; // spoofed host/client
	check(l, o, r, SD_COMPANY_MAP_WRONG_NATIVE_PEER);
	o = observed();
	o.native_host_layer_ref = 0x999;
	check(l, o, r, SD_COMPANY_MAP_WRONG_NATIVE_PEER);
	o = observed();
	o.native_output_ref = 0x999;
	check(l, o, r, SD_COMPANY_MAP_WRONG_NATIVE_PEER);
	o = observed();
	o.native_actor_view_ref = 0x999; // unrelated XDG window
	check(l, o, r, SD_COMPANY_MAP_WRONG_NATIVE_PEER);
	o = observed();
	o.native_actor_root_surface_ref = 0x999; // popup / sibling
	check(l, o, r, SD_COMPANY_MAP_WRONG_NATIVE_PEER);
	o = observed();
	o.actor_root_is_toplevel = false;
	check(l, o, r, SD_COMPANY_MAP_WRONG_NATIVE_PEER);
	o = observed();
	o.actor_process_attested = false;
	check(l, o, r, SD_COMPANY_MAP_WRONG_NATIVE_PEER);
}

static void stale_epoch_and_reused_pid_are_not_continuity(void) {
	struct stage_director_company_map_lease l = lease();
	struct stage_director_company_map_observed o = observed();
	struct stage_director_company_map_report r = report();
	o.actor_start_ticks++; // same PID, new process generation
	check(l, o, r, SD_COMPANY_MAP_STALE_ACTOR);
	o = observed();
	o.actor_pid++;
	check(l, o, r, SD_COMPANY_MAP_STALE_ACTOR);
	o = observed();
	o.actor_uid++;
	check(l, o, r, SD_COMPANY_MAP_STALE_ACTOR);
	o = observed();
	o.epoch++;
	check(l, o, r, SD_COMPANY_MAP_STALE_HOST);
	o = observed();
	r.epoch++;
	check(l, o, r, SD_COMPANY_MAP_STALE_HOST);
	r = report();
	r.host_geometry_revision--;
	check(l, o, r, SD_COMPANY_MAP_STALE_HOST);
	/* The compositor moves the host and advances its revision. The old
	 * report is rejected even though its rectangle is still positive. */
	o.host_global.x += 200;
	o.host_geometry_revision++;
	check(l, o, r, SD_COMPANY_MAP_STALE_HOST);
	r.host_geometry_revision = o.host_geometry_revision;
	check(l, o, r, SD_COMPANY_MAP_CANDIDATE_ONLY);
}

static void hidden_and_unmapped_publish_no_rectangle(void) {
	struct stage_director_company_map_lease l = lease();
	struct stage_director_company_map_observed o = observed();
	struct stage_director_company_map_report r = report();
	r.company_open = false;
	check(l, o, r, SD_COMPANY_MAP_HIDDEN);
	r = report();
	r.coverage_active = false;
	check(l, o, r, SD_COMPANY_MAP_HIDDEN);
	r = report();
	r.native_map_running = false;
	check(l, o, r, SD_COMPANY_MAP_HIDDEN);
	r = report();
	r.map_in_frame = (struct stage_director_rect){0};
	check(l, o, r, SD_COMPANY_MAP_HIDDEN);
	r = report();
	o.actor_mapped = false;
	check(l, o, r, SD_COMPANY_MAP_HIDDEN);
	o = observed();
	o.host_mapped = false;
	check(l, o, r, SD_COMPANY_MAP_HIDDEN);
	o = observed();
	o.output_active = false;
	check(l, o, r, SD_COMPANY_MAP_HIDDEN);
}

static void geometry_bounds_are_not_a_window_manager_shortcut(void) {
	struct stage_director_company_map_lease l = lease();
	struct stage_director_company_map_observed o = observed();
	struct stage_director_company_map_report r = report();
	r.frame_in_host.x = -1;
	check(l, o, r, SD_COMPANY_MAP_INVALID_GEOMETRY);
	r = report();
	r.map_in_frame.x = -1;
	check(l, o, r, SD_COMPANY_MAP_INVALID_GEOMETRY);
	r = report();
	r.frame_in_host.width = INT_MAX;
	check(l, o, r, SD_COMPANY_MAP_INVALID_GEOMETRY);
	r = report();
	r.map_in_frame.width = INT_MAX;
	check(l, o, r, SD_COMPANY_MAP_INVALID_GEOMETRY);
	r = report();
	r.map_in_frame.x = 500;
	r.map_in_frame.width = 500;
	check(l, o, r, SD_COMPANY_MAP_INVALID_GEOMETRY);
	r = report();
	o.output_global.width = 800; // no complete host containment
	check(l, o, r, SD_COMPANY_MAP_INVALID_GEOMETRY);
	o = observed();
	o.actor_global.width = 300; // actor doesn't yet cover viewport
	check(l, o, r, SD_COMPANY_MAP_ACTOR_NOT_COVERING);
}

static void nonprimary_output_and_reactive_resize(void) {
	struct stage_director_company_map_lease l = lease();
	struct stage_director_company_map_observed o = observed();
	struct stage_director_company_map_report r = report();
	o.host_global = (struct stage_director_rect){-1200, 10, 1000, 700};
	o.output_global = (struct stage_director_rect){-1360, 0, 1360, 768};
	o.actor_global = (struct stage_director_rect){-1150, 80, 850, 600};
	r.map_in_frame = (struct stage_director_rect){70, 80, 600, 400};
	const struct stage_director_company_map_result left = check(
		l, o, r, SD_COMPANY_MAP_CANDIDATE_ONLY);
	assert(left.viewport_global.x == -1118);
	assert(left.viewport_global.y == 99);
	assert(left.actor_local_crop.x == 32);
	assert(left.actor_local_crop.y == 19);

	/* A compact panel shrinks its frame and moves the live map. This must
	 * generate another candidate, rather than reusing a stale fixed mask. */
	o.host_global.width = 640;
	o.host_global.height = 490;
	o.host_geometry_revision++;
	o.actor_global = (struct stage_director_rect){-1180, 50, 640, 490};
	r.host_geometry_revision = o.host_geometry_revision;
	r.frame_in_host = (struct stage_director_rect){12, 9, 616, 469};
	r.map_in_frame = (struct stage_director_rect){80, 65, 380, 250};
	const struct stage_director_company_map_result compact = check(
		l, o, r, SD_COMPANY_MAP_CANDIDATE_ONLY);
	assert(compact.viewport_global.x == -1108);
	assert(compact.viewport_global.y == 84);
	assert(compact.viewport_global.width == 380);
	assert(compact.viewport_global.height == 250);
}

int main(void) {
	valid_candidate_is_not_scene_authority();
	wrong_host_and_actor_never_pair();
	stale_epoch_and_reused_pid_are_not_continuity();
	hidden_and_unmapped_publish_no_rectangle();
	geometry_bounds_are_not_a_window_manager_shortcut();
	nonprimary_output_and_reactive_resize();
	puts("STAGE_DIRECTOR_COMPANY_M19: PASS candidate-only, no scene/seat authority");
	return 0;
}
