#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "sway/stage_director_news_shadow.h"

static void check(int hosts, int actors,
		enum stage_director_news_shadow_state expected,
		bool unique) {
	const struct stage_director_news_shadow_result r =
		stage_director_news_shadow_evaluate(hosts, actors);
	assert(r.state == expected);
	assert(r.unique_candidates == unique);
	assert(!r.owner_verified);
	assert(!r.can_bind_viewer);
	assert(!r.can_launch);
	assert(!r.can_clip);
	assert(!r.can_accept_input);
}

int main(void) {
	check(-1, 0, SD_NEWS_SHADOW_INVALID, false);
	check(0, -1, SD_NEWS_SHADOW_INVALID, false);
	check(0, 0, SD_NEWS_SHADOW_EMPTY, false);
	check(1, 0, SD_NEWS_SHADOW_HOST_ONLY, false);
	check(0, 1, SD_NEWS_SHADOW_ACTOR_ONLY, false);
	check(1, 1, SD_NEWS_SHADOW_UNIQUE_CANDIDATES, true);
	check(2, 0, SD_NEWS_SHADOW_DUPLICATE_HOSTS, false);
	check(2, 1, SD_NEWS_SHADOW_DUPLICATE_HOSTS, false);
	check(1, 2, SD_NEWS_SHADOW_DUPLICATE_ACTORS, false);
	check(0, 2, SD_NEWS_SHADOW_DUPLICATE_ACTORS, false);
	check(2, 2, SD_NEWS_SHADOW_AMBIGUOUS_BOTH, false);
	check(200, 100, SD_NEWS_SHADOW_AMBIGUOUS_BOTH, false);
	assert(strcmp(stage_director_news_shadow_state_name(
		SD_NEWS_SHADOW_UNIQUE_CANDIDATES), "UNIQUE_CANDIDATES") == 0);
	assert(strcmp(stage_director_news_shadow_state_name(
		(enum stage_director_news_shadow_state)999), "INVALID") == 0);
	puts("Stage Director NEWS shadow host/actor reconciliation: PASS");
	return 0;
}
