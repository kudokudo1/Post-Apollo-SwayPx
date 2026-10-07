#include <stddef.h>
#include "sway/stage_director_profile.h"

static const struct stage_director_profile profiles[] = {
	{
		.id = "post-apollo-tv",
		.classify = stage_director_profile_post_apollo_tv_classify,
	},
	{
		.id = "weather-station",
		.classify = stage_director_profile_weather_station_classify,
	},
};

bool stage_director_profile_classify(
		struct sway_view *view,
		struct stage_director_identity *identity) {
	if (!view || !identity) {
		return false;
	}

	for (size_t i = 0; i < sizeof(profiles) / sizeof(profiles[0]); ++i) {
		struct stage_director_identity candidate = {0};

		if (!profiles[i].classify(view, &candidate)) {
			continue;
		}

		candidate.profile = profiles[i].id;
		*identity = candidate;
		return true;
	}

	return false;
}
