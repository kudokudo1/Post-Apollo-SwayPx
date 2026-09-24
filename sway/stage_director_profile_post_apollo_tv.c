#include <string.h>
#include "sway/stage_director_profile.h"
#include "sway/tree/view.h"

static bool streq(const char *a, const char *b) {
	return a && b && strcmp(a, b) == 0;
}

bool stage_director_profile_post_apollo_tv_classify(
		struct sway_view *view, struct stage_director_identity *identity) {
	if (!view || !identity) {
		return false;
	}

	const char *app_id = view_get_app_id(view);
	const char *title = view_get_title(view);

	*identity = (struct stage_director_identity) {
		.scene = "post-apollo-tv",
		.expected_scene_actors = 7,
		.passive = false,
		.anchor = false,
	};

	if (streq(app_id, "post-apollo-terminal")) {
		identity->role = "screen";
		identity->anchor = true;
		return true;
	}
	if (streq(title, "Post-Apollo Side")) {
		identity->role = "right-panel";
		return true;
	}
	if (streq(title, "Post-Apollo Deck")) {
		identity->role = "receiver";
		return true;
	}
	if (streq(title, "Post-Apollo Bezel Top")) {
		identity->role = "bezel-top";
		identity->passive = true;
		return true;
	}
	if (streq(title, "Post-Apollo Bezel Left")) {
		identity->role = "bezel-left";
		identity->passive = true;
		return true;
	}
	if (streq(title, "Post-Apollo Bezel Right")) {
		identity->role = "bezel-right";
		identity->passive = true;
		return true;
	}
	if (streq(title, "Post-Apollo Bezel Bottom")) {
		identity->role = "bezel-bottom";
		identity->passive = true;
		return true;
	}

	return false;
}
