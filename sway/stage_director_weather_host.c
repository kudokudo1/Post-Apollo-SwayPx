#include <string.h>
#include "sway/stage_director_weather_host.h"

/* No rendering, compositor lifecycle, OS processes or IPC in this policy. */
struct stage_director_weather_host_result stage_director_weather_host_probe(
		const struct stage_director_weather_host_observation *observation) {
	struct stage_director_weather_host_result result = {
		.status = SD_WEATHER_HOST_MISSING,
		.owner_verified = false,
		.can_bind_viewer = false,
		.can_clip = false,
	};
	if (!observation || !observation->layer_ref ||
			!observation->output_ref || !observation->layer_namespace) {
		return result;
	}
	if (strcmp(observation->layer_namespace,
			STAGE_DIRECTOR_WEATHER_NAMESPACE) != 0) {
		result.status = SD_WEATHER_HOST_WRONG_NAMESPACE;
		return result;
	}
	if (!observation->initialized || !observation->mapped) {
		result.status = SD_WEATHER_HOST_UNMAPPED;
		return result;
	}
	if (!observation->overlay_layer) {
		result.status = SD_WEATHER_HOST_WRONG_LAYER;
		return result;
	}
	if (!observation->output_active) {
		result.status = SD_WEATHER_HOST_OUTPUT_INACTIVE;
		return result;
	}
	if (!stage_director_rect_valid(observation->host) ||
			!stage_director_rect_valid(observation->output)) {
		result.status = SD_WEATHER_HOST_GEOMETRY_INVALID;
		return result;
	}
	if (!stage_director_rect_contains(observation->output,
			observation->host)) {
		result.status = SD_WEATHER_HOST_OUTSIDE_OUTPUT;
		return result;
	}
	result.status = SD_WEATHER_HOST_CANDIDATE_ONLY;
	result.candidate_observed = true;
	return result;
}
