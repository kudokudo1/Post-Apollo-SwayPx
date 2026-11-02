#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sway/stage_director_weather_host.h"

static struct stage_director_weather_host_observation baseline(void) {
	return (struct stage_director_weather_host_observation){
		.layer_namespace = STAGE_DIRECTOR_WEATHER_NAMESPACE,
		.layer_ref = (uintptr_t)0x1234u,
		.output_ref = (uintptr_t)0xabcd,
		.initialized = true,
		.mapped = true,
		.overlay_layer = true,
		.output_active = true,
		.host = {100, 40, 1000, 700},
		.output = {0, 0, 2560, 1440},
	};
}

static void check_no_authority(
		struct stage_director_weather_host_result result,
		enum stage_director_weather_host_status expected_status,
		bool expected_candidate) {
	assert(result.status == expected_status);
	assert(result.candidate_observed == expected_candidate);
	assert(!result.owner_verified);
	assert(!result.can_bind_viewer);
	assert(!result.can_clip);
}

static void test_exact_host_and_hostile_namespace(void) {
	struct stage_director_weather_host_observation s = baseline();
	check_no_authority(stage_director_weather_host_probe(&s),
		SD_WEATHER_HOST_CANDIDATE_ONLY, true);
	s.layer_namespace = "quickshell";
	check_no_authority(stage_director_weather_host_probe(&s),
		SD_WEATHER_HOST_WRONG_NAMESPACE, false);
	s.layer_namespace = "post-apollo-weather-station-evil";
	check_no_authority(stage_director_weather_host_probe(&s),
		SD_WEATHER_HOST_WRONG_NAMESPACE, false);
	s.layer_namespace = "";
	check_no_authority(stage_director_weather_host_probe(&s),
		SD_WEATHER_HOST_WRONG_NAMESPACE, false);
	s.layer_namespace = NULL;
	check_no_authority(stage_director_weather_host_probe(&s),
		SD_WEATHER_HOST_MISSING, false);
	check_no_authority(stage_director_weather_host_probe(NULL),
		SD_WEATHER_HOST_MISSING, false);
}

static void test_lifecycle_and_layer_denials(void) {
	struct stage_director_weather_host_observation s = baseline();
	s.layer_ref = 0;
	check_no_authority(stage_director_weather_host_probe(&s),
		SD_WEATHER_HOST_MISSING, false);
	s = baseline();
	s.output_ref = 0;
	check_no_authority(stage_director_weather_host_probe(&s),
		SD_WEATHER_HOST_MISSING, false);
	s = baseline();
	s.initialized = false;
	check_no_authority(stage_director_weather_host_probe(&s),
		SD_WEATHER_HOST_UNMAPPED, false);
	s = baseline();
	s.mapped = false;
	check_no_authority(stage_director_weather_host_probe(&s),
		SD_WEATHER_HOST_UNMAPPED, false);
	s = baseline();
	s.overlay_layer = false;
	check_no_authority(stage_director_weather_host_probe(&s),
		SD_WEATHER_HOST_WRONG_LAYER, false);
	s = baseline();
	s.output_active = false;
	check_no_authority(stage_director_weather_host_probe(&s),
		SD_WEATHER_HOST_OUTPUT_INACTIVE, false);
}

static void test_global_coordinates_and_output_shift(void) {
	struct stage_director_weather_host_observation s = baseline();
	s.host = (struct stage_director_rect){-1290, 50, 1000, 700};
	s.output = (struct stage_director_rect){-1360, 0, 1360, 768};
	check_no_authority(stage_director_weather_host_probe(&s),
		SD_WEATHER_HOST_CANDIDATE_ONLY, true);
	s.output = (struct stage_director_rect){0, 0, 2560, 1440};
	check_no_authority(stage_director_weather_host_probe(&s),
		SD_WEATHER_HOST_OUTSIDE_OUTPUT, false);
	s = baseline();
	s.host.x = INT_MAX - 100;
	check_no_authority(stage_director_weather_host_probe(&s),
		SD_WEATHER_HOST_GEOMETRY_INVALID, false);
	s = baseline();
	s.output.width = 0;
	check_no_authority(stage_director_weather_host_probe(&s),
		SD_WEATHER_HOST_GEOMETRY_INVALID, false);
	s = baseline();
	s.host.width = 3000;
	check_no_authority(stage_director_weather_host_probe(&s),
		SD_WEATHER_HOST_OUTSIDE_OUTPUT, false);
}

int main(void) {
	test_exact_host_and_hostile_namespace();
	test_lifecycle_and_layer_denials();
	test_global_coordinates_and_output_shift();
	puts("Stage Director Weather layer host candidate: PASS");
	return 0;
}
