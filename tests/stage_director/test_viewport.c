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

static void check_host_local_weather_bridge_geometry(void) {
	/* Mirrors the existing QML rectangle in Station chassis local units,
	 * not a new Wayland surface or any sort of browser authorization. */
	struct stage_director_rect host = {100, 40, 1000, 700};
	const struct stage_director_rect local = {120, 80, 800, 500};
	struct stage_director_rect global = {99, 99, 99, 99};
	assert(stage_director_viewport_from_host_local(host, local, &global));
	assert(global.x == 220 && global.y == 120);
	assert(global.width == 800 && global.height == 500);
	/* When the entire panel moves, the interior must be freshly rebased. */
	host.x = 400;
	host.y = 250;
	assert(stage_director_viewport_from_host_local(host, local, &global));
	assert(global.x == 520 && global.y == 330);
	/* Multi-monitor layouts can have negative compositor-global origins. */
	host = (struct stage_director_rect){-1360, -20, 1000, 700};
	assert(stage_director_viewport_from_host_local(host, local, &global));
	assert(global.x == -1240 && global.y == 60);
	assert(stage_director_rect_contains(host, global));

	/* A closed viewer, missing host, or a geometry leak fails closed. */
	const struct stage_director_rect closed = {0, 0, 0, 0};
	const struct stage_director_rect outside = {700, 80, 400, 500};
	const struct stage_director_rect negative = {-1, 80, 800, 500};
	const struct stage_director_rect overflow = {INT_MAX - 100, 0, 200, 700};
	global = (struct stage_director_rect){99, 99, 99, 99};
	assert(!stage_director_viewport_from_host_local(host, closed, &global));
	assert(global.width == 0 && global.height == 0);
	global = (struct stage_director_rect){99, 99, 99, 99};
	assert(!stage_director_viewport_from_host_local(host, outside, &global));
	assert(global.x == 0 && global.y == 0);
	assert(!stage_director_viewport_from_host_local(host, negative, &global));
	assert(!stage_director_viewport_from_host_local(overflow, local, &global));
	assert(!stage_director_viewport_from_host_local(closed, local, &global));
	assert(!stage_director_viewport_from_host_local(host, local, NULL));
}

int main(void) {
	check_ready_geometry_not_authority();
	check_fail_closed();
	check_second_monitor_and_negative_origins();
	check_overflow_and_edge_cases();
	check_host_local_weather_bridge_geometry();
	puts("Stage Director SD1 viewport geometry policy: PASS");
	return 0;
}
