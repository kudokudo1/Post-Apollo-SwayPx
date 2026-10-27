#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "sway/stage_director_viewport.h"

/* Pure test only. Native references are fake stable sentinels, not credentials. */
static struct stage_director_viewport_frame_owner good_owner(void) {
	return (struct stage_director_viewport_frame_owner) {
		.epoch = 7,
		.native_host_ref = 0x11,
		.native_actor_ref = 0x22,
		.native_output_ref = 0x33,
		.native_root_surface_ref = 0x44,
		.authorized_host = { .x = 0, .y = 0, .width = 800, .height = 600 },
		.authorized_output = { .x = 0, .y = 0, .width = 1920, .height = 1080 },
		.authorized_viewport = { .x = 180, .y = 160, .width = 320, .height = 240 },
		.authority = {
			.operator_enabled = true,
			.host_session_verified = true,
			.actor_process_verified = true,
			.input_hit_test_guard_installed = true,
			.popup_guard_installed = true,
			.teardown_guard_installed = true,
		},
	};
}

static struct stage_director_viewport_frame_observation good_frame(void) {
	return (struct stage_director_viewport_frame_observation) {
		.epoch = 7,
		.native_host_ref = 0x11,
		.native_actor_ref = 0x22,
		.native_output_ref = 0x33,
		.native_root_surface_ref = 0x44,
		.native_root_surface = true,
		.geometry = {
			.host = { .x = 0, .y = 0, .width = 800, .height = 600 },
			.output = { .x = 0, .y = 0, .width = 1920, .height = 1080 },
			.actor = { .x = 100, .y = 100, .width = 850, .height = 710 },
			.viewport = { .x = 180, .y = 160, .width = 320, .height = 240 },
			.host_mapped = true,
			.actor_mapped = true,
			.output_active = true,
			.actor_identity_verified = true,
		},
		.root_surface_geometry_x = 12,
		.root_surface_geometry_y = 8,
	};
}

static void check_denied(struct stage_director_viewport_frame_decision v,
		enum stage_director_viewport_frame_state reason) {
	assert(v.state == reason);
	assert(!v.proposed_override);
	assert(v.proposed_dest_width == 0 && v.proposed_dest_height == 0);
	assert(v.root_surface_crop.width == 0 && v.root_surface_crop.height == 0);
}

#define DENY(reason) check_denied(stage_director_viewport_frame_policy(&owner, &frame), reason)
#define RESET do { owner = good_owner(); frame = good_frame(); } while (0)

int main(void) {
	struct stage_director_viewport_frame_owner owner = good_owner();
	struct stage_director_viewport_frame_observation frame = good_frame();
	struct stage_director_viewport_frame_decision r =
		stage_director_viewport_frame_policy(&owner, &frame);
	assert(r.state == SD_FRAME_PLAN_DESTINATION_ONLY && r.proposed_override);
	assert(r.proposed_dest_width == 320 && r.proposed_dest_height == 240);
	assert(r.root_surface_crop.x == 92 && r.root_surface_crop.y == 68);
	assert(r.root_surface_crop.width == 320 && r.root_surface_crop.height == 240);

	check_denied(stage_director_viewport_frame_policy(NULL, &frame),
		SD_FRAME_DEFAULT_NO_OWNER);
	check_denied(stage_director_viewport_frame_policy(&owner, NULL),
		SD_FRAME_DEFAULT_NO_OWNER);

	RESET; owner.epoch = 0; DENY(SD_FRAME_DEFAULT_NO_OWNER);
	RESET; owner.native_host_ref = 0; DENY(SD_FRAME_DEFAULT_NO_OWNER);
	RESET; owner.native_actor_ref = 0; DENY(SD_FRAME_DEFAULT_NO_OWNER);
	RESET; owner.native_output_ref = 0; DENY(SD_FRAME_DEFAULT_NO_OWNER);
	RESET; owner.native_root_surface_ref = 0; DENY(SD_FRAME_DEFAULT_NO_OWNER);

	RESET; frame.epoch = 8; DENY(SD_FRAME_DEFAULT_STALE_OWNER);
	RESET; frame.native_host_ref = 0x99; DENY(SD_FRAME_DEFAULT_STALE_OWNER);
	RESET; frame.native_actor_ref = 0x99; DENY(SD_FRAME_DEFAULT_STALE_OWNER);
	RESET; frame.native_output_ref = 0x99; DENY(SD_FRAME_DEFAULT_STALE_OWNER);

	RESET; frame.native_root_surface_ref = 0x55;
	DENY(SD_FRAME_DEFAULT_DIFFERENT_SURFACE);
	RESET; frame.native_root_surface = false;
	DENY(SD_FRAME_DEFAULT_DIFFERENT_SURFACE);

	RESET; frame.geometry.host.x += 1; DENY(SD_FRAME_DEFAULT_STALE_GEOMETRY);
	RESET; frame.geometry.output.width -= 1; DENY(SD_FRAME_DEFAULT_STALE_GEOMETRY);
	RESET; frame.geometry.viewport.width -= 1; DENY(SD_FRAME_DEFAULT_STALE_GEOMETRY);
	RESET; frame.geometry.actor.width = 25; DENY(SD_FRAME_DEFAULT_STALE_GEOMETRY);
	RESET; frame.geometry.host_mapped = false; DENY(SD_FRAME_DEFAULT_STALE_GEOMETRY);
	RESET; frame.geometry.actor_mapped = false; DENY(SD_FRAME_DEFAULT_STALE_GEOMETRY);
	RESET; frame.geometry.output_active = false; DENY(SD_FRAME_DEFAULT_STALE_GEOMETRY);
	RESET; frame.geometry.actor_identity_verified = false; DENY(SD_FRAME_DEFAULT_STALE_GEOMETRY);
	RESET; frame.root_surface_geometry_x = INT_MAX; DENY(SD_FRAME_DEFAULT_GUARDS_MISSING);

	RESET; owner.authority.operator_enabled = false; DENY(SD_FRAME_DEFAULT_GUARDS_MISSING);
	RESET; owner.authority.host_session_verified = false; DENY(SD_FRAME_DEFAULT_GUARDS_MISSING);
	RESET; owner.authority.actor_process_verified = false; DENY(SD_FRAME_DEFAULT_GUARDS_MISSING);
	RESET; owner.authority.input_hit_test_guard_installed = false;
	DENY(SD_FRAME_DEFAULT_GUARDS_MISSING);
	RESET; owner.authority.popup_guard_installed = false; DENY(SD_FRAME_DEFAULT_GUARDS_MISSING);
	RESET; owner.authority.teardown_guard_installed = false; DENY(SD_FRAME_DEFAULT_GUARDS_MISSING);

	/* A move can remain eligible only while the same authorized viewport
	 * stays covered by the newly observed actor; the crop moves accordingly. */
	RESET;
	frame.geometry.actor.x = 150;
	frame.geometry.actor.y = 120;
	r = stage_director_viewport_frame_policy(&owner, &frame);
	assert(r.proposed_override && r.root_surface_crop.x == 42 &&
		r.root_surface_crop.y == 48);
	RESET; frame.geometry.actor.x = 800; DENY(SD_FRAME_DEFAULT_STALE_GEOMETRY);

	/* On CLOSE/unmap a caller passes NULL; default Sway sizing must win. */
	RESET;
	check_denied(stage_director_viewport_frame_policy(NULL, &frame),
		SD_FRAME_DEFAULT_NO_OWNER);
	assert(stage_director_viewport_frame_state_name(SD_FRAME_PLAN_DESTINATION_ONLY)[0] == 'P');

	puts("SD1 FRAME POLICY // PASS: native identity, epoch, host/output, all guards, "
		"geometry, root-only and teardown fail closed; decision never paints");
	return 0;
}
