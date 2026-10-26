#include <scenefx/types/fx/blur_data.h>
#include <string.h>
#include "sway/stage_director_profile.h"
#include "sway/tree/view.h"

/*
 * Register the external NEWS browser as a child actor by stable native
 * app_id or XWayland WM_CLASS. Never match window titles, URL text,
 * browser brand or arbitrary windows. Identity is observation ONLY.
 *
 * The Weather Station is a layer-shell host, not a sway_view. The actor's
 * scene cannot declare itself ready without a separately verified host,
 * viewport, clip, input and process lifecycle handshake.
 */
bool stage_director_profile_weather_news_classify(
		struct sway_view *view, struct stage_director_identity *identity) {
	if (!view || !identity) {
		return false;
	}

	const char *app_id = view_get_app_id(view);
	const char *window_class = view_get_class(view);
	if ((!app_id || strcmp(app_id, "post-apollo-weather-news") != 0) &&
			(!window_class ||
			 strcmp(window_class, "post-apollo-weather-news") != 0)) {
		return false;
	}

	*identity = (struct stage_director_identity) {
		.scene = "weather-news",
		.role = "browser",
		.expected_scene_actors = 1,
		/* No actor is allowed to appoint itself compositor scene leader. */
		.passive = false,
		.anchor = false,
		.can_lead = false,
	};
	return true;
}
