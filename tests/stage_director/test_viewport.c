#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "sway/stage_director_viewport.h"

static struct stage_director_viewport_request baseline(void) {
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

static void check_ready_geometry_not_authority(void) {
	struct stage_director_viewport_request r = baseline();
	struct stage_director_viewport_plan p = stage_director_viewport_plan(&r);
	assert(p.result == SD_VIEWPORT_GEOMETRY_ONLY);
	assert(strcmp(stage_director_viewport_result_name(p.result),
			"GEOMETRY_ONLY") == 0);
	assert(p.geometry_valid);
	assert(p.actor_local_crop.x == 25);
	assert(p.actor_local_crop.y == 15);
	assert(p.actor_local_crop.width == 824);
	assert(p.actor_local_crop.height == 494);
	assert(!p.can_launch && !p.can_render && !p.can_accept_input);
	assert(stage_director_rect_contains_point(r.viewport, 205, 120));
	assert(stage_director_rect_contains_point(r.viewport, 1028, 613));
	assert(!stage_director_rect_contains_point(r.viewport, 1029, 613));
	assert(!stage_director_rect_contains_point(r.viewport, 1028, 614));
}

static void check_fail_closed(void) {
	struct stage_director_viewport_request r = baseline();
	assert(stage_director_viewport_plan(NULL).result ==
		SD_VIEWPORT_DENY_UNVERIFIED_ACTOR);
	r.actor_identity_verified = false;
	assert(stage_director_viewport_plan(&r).result ==
		SD_VIEWPORT_DENY_UNVERIFIED_ACTOR);
	r = baseline();
	r.host_mapped = false;
	assert(stage_director_viewport_plan(&r).result == SD_VIEWPORT_DENY_UNMAPPED);
	r = baseline();
	r.actor_mapped = false;
	assert(stage_director_viewport_plan(&r).result == SD_VIEWPORT_DENY_UNMAPPED);
	r = baseline();
	r.output_active = false;
	assert(stage_director_viewport_plan(&r).result ==
		SD_VIEWPORT_DENY_OUTPUT_INACTIVE);
	r = baseline();
	r.viewport.width = 0;
	assert(stage_director_viewport_plan(&r).result ==
		SD_VIEWPORT_DENY_INVALID_GEOMETRY);
	r = baseline();
	r.viewport.x = INT_MAX - 50;
	assert(stage_director_viewport_plan(&r).result ==
		SD_VIEWPORT_DENY_INVALID_GEOMETRY);
	r = baseline();
	r.viewport.x = r.host.x - 1;
	assert(stage_director_viewport_plan(&r).result ==
		SD_VIEWPORT_DENY_VIEWPORT_OUTSIDE_HOST);
	r = baseline();
	r.output.width = 500;
	assert(stage_director_viewport_plan(&r).result ==
		SD_VIEWPORT_DENY_VIEWPORT_OUTSIDE_OUTPUT);
	r = baseline();
	r.actor.width = 800;
	assert(stage_director_viewport_plan(&r).result ==
		SD_VIEWPORT_DENY_ACTOR_NOT_COVERING_VIEWPORT);
	r = baseline();
	r.viewport.y += 100;
	r.viewport.height += 100;
	assert(stage_director_viewport_plan(&r).result ==
		SD_VIEWPORT_DENY_VIEWPORT_OUTSIDE_HOST);
}

static void check_second_monitor_and_negative_origins(void) {
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
	struct stage_director_viewport_plan p = stage_director_viewport_plan(&r);
	assert(p.result == SD_VIEWPORT_GEOMETRY_ONLY);
	assert(p.actor_local_crop.x == 90);
	assert(p.actor_local_crop.y == 70);
	assert(!p.can_render);
	assert(!stage_director_rect_contains_point(r.viewport, -1111, 80));
	r.output.x = 0; // Output changed without moving host/actor.
	assert(stage_director_viewport_plan(&r).result ==
		SD_VIEWPORT_DENY_VIEWPORT_OUTSIDE_OUTPUT);
}

static void check_overflow_and_edge_cases(void) {
	assert(!stage_director_rect_valid((struct stage_director_rect){
		INT_MAX - 3, 0, 10, 1
	}));
	assert(!stage_director_rect_valid((struct stage_director_rect){
		0, 0, -1, 1
	}));
	assert(stage_director_rect_valid((struct stage_director_rect){
		INT_MIN, -10, 1, 1
	}));
	assert(!stage_director_rect_contains((struct stage_director_rect){
		0, 0, 100, 100
	}, (struct stage_director_rect){10, 10, 100, 100}));
	assert(!stage_director_rect_contains_point((struct stage_director_rect){
		0, 0, 0, 1
	}, 0, 0));
}

int main(void) {
	check_ready_geometry_not_authority();
	check_fail_closed();
	check_second_monitor_and_negative_origins();
	check_overflow_and_edge_cases();
	puts("Stage Director SD1 viewport geometry policy: PASS");
	return 0;
}
