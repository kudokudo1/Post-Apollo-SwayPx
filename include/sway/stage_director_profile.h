#ifndef _SWAY_STAGE_DIRECTOR_PROFILE_H
#define _SWAY_STAGE_DIRECTOR_PROFILE_H

#include <stdbool.h>

struct sway_view;

struct stage_director_identity {
	const char *profile;
	const char *scene;
	const char *role;
	int expected_scene_actors;
	bool passive;
	bool anchor;
	bool can_lead;
};

typedef bool (*stage_director_profile_classifier)(
		struct sway_view *view,
		struct stage_director_identity *identity);

struct stage_director_profile {
	const char *id;
	stage_director_profile_classifier classify;
};

/*
 * Profiles translate application-specific identity into universal Stage
 * Director vocabulary. The core knows scenes and actors, not TVs, weather
 * stations, chat clients, or any other product-specific surface.
 */
bool stage_director_profile_classify(
		struct sway_view *view,
		struct stage_director_identity *identity);

/* Built-in profiles register through sway/stage_director_profiles.c. */
bool stage_director_profile_post_apollo_tv_classify(
		struct sway_view *view,
		struct stage_director_identity *identity);

#endif
