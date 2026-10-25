#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include "sway/stage_director_viewport.h"

static struct stage_director_viewport_request request(void) {
	return (struct stage_director_viewport_request) {
		.host = {100, 40, 1000, 700},
		.output = {0, 0, 2560, 1440},
		.actor = {180, 105, 950, 630},
		.viewport = {205, 120, 824, 494},
		.host_mapped = true,
		.actor_mapped = true,
		.output_active = true,
		.actor_identity_verified = true,
	};
}

static struct stage_director_viewport_authority approved(void) {
	return (struct stage_director_viewport_authority) {
		.operator_enabled = true,
		.host_session_verified = true,
		.actor_process_verified = true,
		.input_hit_test_guard_installed = true,
		.popup_guard_installed = true,
		.teardown_guard_installed = true,
	};
}

static void test_fail_closed_authorities(void) {
	struct stage_director_viewport_request r = request();
	struct stage_director_viewport_authority a = {0};
	struct stage_director_viewport_clip_plan clip =
		stage_director_viewport_prepare_clip(&r, &a, 0, 0);
	assert(!clip.allowed);
	assert(clip.geometry_result == SD_VIEWPORT_GEOMETRY_ONLY);
	assert(!stage_director_viewport_prepare_clip(&r, NULL, 0, 0).allowed);
	a = approved();
	assert(stage_director_viewport_prepare_clip(&r, &a, 0, 0).allowed);
	a.operator_enabled = false;
	assert(!stage_director_viewport_prepare_clip(&r, &a, 0, 0).allowed);
	a = approved();
	a.host_session_verified = false;
	assert(!stage_director_viewport_prepare_clip(&r, &a, 0, 0).allowed);
	a = approved();
	a.actor_process_verified = false;
	assert(!stage_director_viewport_prepare_clip(&r, &a, 0, 0).allowed);
	a = approved();
	a.input_hit_test_guard_installed = false;
	assert(!stage_director_viewport_prepare_clip(&r, &a, 0, 0).allowed);
	a = approved();
	a.popup_guard_installed = false;
	assert(!stage_director_viewport_prepare_clip(&r, &a, 0, 0).allowed);
	a = approved();
	a.teardown_guard_installed = false;
	assert(!stage_director_viewport_prepare_clip(&r, &a, 0, 0).allowed);
}

static void test_root_surface_crop_and_overflow(void) {
	struct stage_director_viewport_request r = request();
	struct stage_director_viewport_authority a = approved();
	struct stage_director_viewport_clip_plan clip =
		stage_director_viewport_prepare_clip(&r, &a, 14, -3);
	assert(clip.allowed);
	assert(clip.root_surface_clip.x == 39);
	assert(clip.root_surface_clip.y == 12);
	assert(clip.root_surface_clip.width == 824);
	assert(clip.root_surface_clip.height == 494);
	assert(!stage_director_viewport_prepare_clip(&r, &a,
			INT_MAX, 0).allowed);
	assert(!stage_director_viewport_prepare_clip(&r, &a,
			0, INT_MAX).allowed);
	assert(!stage_director_viewport_prepare_clip(&r, &a,
			INT_MIN, 0).allowed);
	r.viewport.width = 0;
	clip = stage_director_viewport_prepare_clip(&r, &a, 14, -3);
	assert(!clip.allowed);
	assert(clip.geometry_result == SD_VIEWPORT_DENY_INVALID_GEOMETRY);
}

static void test_second_output(void) {
	struct stage_director_viewport_request r = {
		.host = {-1200, 10, 1000, 700},
		.output = {-1360, 0, 1360, 768},
		.actor = {-1200, 10, 1000, 700},
		.viewport = {-1110, 80, 824, 494},
		.host_mapped = true,
		.actor_mapped = true,
		.output_active = true,
		.actor_identity_verified = true,
	};
	const struct stage_director_viewport_authority a = approved();
	struct stage_director_viewport_clip_plan clip =
		stage_director_viewport_prepare_clip(&r, &a, 0, 0);
	assert(clip.allowed);
	assert(clip.root_surface_clip.x == 90);
	assert(clip.root_surface_clip.y == 70);
	r.host_mapped = false;
	assert(!stage_director_viewport_prepare_clip(&r, &a, 0, 0).allowed);
	r.host_mapped = true;
	r.actor_mapped = false;
	assert(!stage_director_viewport_prepare_clip(&r, &a, 0, 0).allowed);
}

int main(void) {
	test_fail_closed_authorities();
	test_root_surface_crop_and_overflow();
	test_second_output();
	puts("Stage Director V1 guarded clip policy: PASS");
	return 0;
}
