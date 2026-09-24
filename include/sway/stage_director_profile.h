#ifndef _SWAY_STAGE_DIRECTOR_PROFILE_H
#define _SWAY_STAGE_DIRECTOR_PROFILE_H

#include <stdbool.h>

struct sway_view;

struct stage_director_identity {
	const char *scene;
	const char *role;
	int expected_scene_actors;
	bool passive;
	bool anchor;
};

/*
 * Profiles translate application-specific identity into universal
 * Stage Director vocabulary. The core does not know what a TV, Discord,
 * weather station, or receiver is.
 */
bool stage_director_profile_post_apollo_tv_classify(
		struct sway_view *view, struct stage_director_identity *identity);

#endif
