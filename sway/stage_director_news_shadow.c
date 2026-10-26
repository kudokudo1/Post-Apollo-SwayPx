#include "sway/stage_director_news_shadow.h"

struct stage_director_news_shadow_result stage_director_news_shadow_evaluate(
		int host_candidates, int actor_candidates) {
	struct stage_director_news_shadow_result r = {
		.state = SD_NEWS_SHADOW_INVALID,
		.unique_candidates = false,
		.owner_verified = false,
		.can_bind_viewer = false,
		.can_launch = false,
		.can_clip = false,
		.can_accept_input = false,
	};
	if (host_candidates < 0 || actor_candidates < 0) {
		return r;
	}
	if (host_candidates > 1 && actor_candidates > 1) {
		r.state = SD_NEWS_SHADOW_AMBIGUOUS_BOTH;
	} else if (host_candidates > 1) {
		r.state = SD_NEWS_SHADOW_DUPLICATE_HOSTS;
	} else if (actor_candidates > 1) {
		r.state = SD_NEWS_SHADOW_DUPLICATE_ACTORS;
	} else if (host_candidates == 1 && actor_candidates == 1) {
		r.state = SD_NEWS_SHADOW_UNIQUE_CANDIDATES;
		r.unique_candidates = true;
	} else if (host_candidates == 1) {
		r.state = SD_NEWS_SHADOW_HOST_ONLY;
	} else if (actor_candidates == 1) {
		r.state = SD_NEWS_SHADOW_ACTOR_ONLY;
	} else {
		r.state = SD_NEWS_SHADOW_EMPTY;
	}
	return r;
}

const char *stage_director_news_shadow_state_name(
		enum stage_director_news_shadow_state state) {
	switch (state) {
	case SD_NEWS_SHADOW_INVALID: return "INVALID";
	case SD_NEWS_SHADOW_EMPTY: return "EMPTY";
	case SD_NEWS_SHADOW_HOST_ONLY: return "HOST_ONLY";
	case SD_NEWS_SHADOW_ACTOR_ONLY: return "ACTOR_ONLY";
	case SD_NEWS_SHADOW_UNIQUE_CANDIDATES: return "UNIQUE_CANDIDATES";
	case SD_NEWS_SHADOW_DUPLICATE_HOSTS: return "DUPLICATE_HOSTS";
	case SD_NEWS_SHADOW_DUPLICATE_ACTORS: return "DUPLICATE_ACTORS";
	case SD_NEWS_SHADOW_AMBIGUOUS_BOTH: return "AMBIGUOUS_BOTH";
	}
	return "INVALID";
}
