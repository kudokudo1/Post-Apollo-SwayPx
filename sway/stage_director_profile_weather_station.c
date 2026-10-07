#include <string.h>
#include "sway/stage_director_profile.h"
#include "sway/tree/view.h"

static bool streq(const char *a, const char *b) {
	return a && b && strcmp(a, b) == 0;
}

bool stage_director_profile_weather_station_classify(
		struct sway_view *view, struct stage_director_identity *identity) {
	if (!view || !identity) {
		return false;
	}

	const char *app_id = view_get_app_id(view);
	if (!streq(app_id, "weather-screen")) {
		return false;
	}

	/*
	 * SD0 currently observes the external Kitty actor only. The Quickshell
	 * Weather Station chassis is a layer-shell surface and remains outside
	 * this first view-only observer path.
	 *
	 * Geometry here is declaration-only. Stage Director does not apply it.
	 * The canonical station is 1000x700 and the Star Map bay currently sits
	 * at 780x416 @ 190,132 within that scene.
	 */
	*identity = (struct stage_director_identity) {
		.scene = "weather-station",
		.role = "star-map",
		.expected_scene_actors = 1,
		.passive = false,
		.anchor = true,
		.can_lead = false,
		.scene_geometry = {
			.declared = true,
			.width = 1000,
			.height = 700,
		},
		.transform = {
			.declared = true,
			.x_rel = 0.0,
			.y_rel = 0.0,
			.width_rel = 0.0,
			.height_rel = 0.0,
			.x_px = 190,
			.y_px = 132,
			.width_px = 780,
			.height_px = 416,
		},
	};

	return true;
}
