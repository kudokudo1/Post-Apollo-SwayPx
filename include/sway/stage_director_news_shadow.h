#ifndef _SWAY_STAGE_DIRECTOR_NEWS_SHADOW_H
#define _SWAY_STAGE_DIRECTOR_NEWS_SHADOW_H

#include <stdbool.h>

/*
 * SD1 // NEWS scene shadow reconciliation.
 *
 * Counts are native compositor observations: mapped, exact-namespace
 * Weather layer candidates and exact-app-id/class NEWS browser candidates.
 * A unique pair is only a *discovery result*, never a trusted relationship.
 *
 * This small policy is independently testable without compositor headers.
 */
enum stage_director_news_shadow_state {
	SD_NEWS_SHADOW_INVALID,
	SD_NEWS_SHADOW_EMPTY,
	SD_NEWS_SHADOW_HOST_ONLY,
	SD_NEWS_SHADOW_ACTOR_ONLY,
	SD_NEWS_SHADOW_UNIQUE_CANDIDATES,
	SD_NEWS_SHADOW_DUPLICATE_HOSTS,
	SD_NEWS_SHADOW_DUPLICATE_ACTORS,
	SD_NEWS_SHADOW_AMBIGUOUS_BOTH,
};

struct stage_director_news_shadow_result {
	enum stage_director_news_shadow_state state;
	bool unique_candidates;
	/* Neither discovery nor counting can authenticate or activate. */
	bool owner_verified;
	bool can_bind_viewer;
	bool can_launch;
	bool can_clip;
	bool can_accept_input;
};

struct stage_director_news_shadow_result stage_director_news_shadow_evaluate(
		int host_candidates, int actor_candidates);
const char *stage_director_news_shadow_state_name(
		enum stage_director_news_shadow_state state);

#endif
