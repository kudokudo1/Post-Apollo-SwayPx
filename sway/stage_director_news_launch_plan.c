#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "sway/stage_director_news_launch_plan.h"

struct news_source {
	const char *id;
	const char *url;
};
/* Exact Weather NEWS source IDs + URLs from source registry schema 1. */
static const struct news_source sources[] = {
	{"aje", "https://www.aljazeera.com/video/live"},
	{"livenow", "https://www.livenowfox.com/live"},
	{"nbcnow", "https://www.nbcnews.com/now"},
	{"abcnews", "https://abc.com/watch-live"},
	{"cnn", "https://www.cnn.com/"},
	{"foxnews", "https://www.fox.com/stream/fox-news"},
	{"house", "https://live.house.gov/"},
	{"senate", "https://www.senate.gov/floor/"},
	{"hearings", "https://www.congress.gov/committees"},
	{"cspan", "https://www.c-span.org/"},
	{"bbc", "https://www.bbc.com/news"},
	{"npr", "https://www.npr.org/sections/news/"},
	{"nasa", "https://www.nasa.gov/news/"},
	{"nws", "https://www.weather.gov/"},
	{"groundnews", "https://ground.news/"},
	{"allsides", "https://www.allsides.com/unbiased-balanced-news"},
	{"jubilee", "https://www.jubileemedia.com/"},
	{"theonion", "https://theonion.com/"},
};

size_t stage_director_news_sources_count(void) {
	return sizeof(sources) / sizeof(sources[0]);
}

const char *stage_director_news_source_url(const char *source_id) {
	if (!source_id || strlen(source_id) > 40) {
		return NULL;
	}
	for (size_t i = 0; i < stage_director_news_sources_count(); ++i) {
		if (strcmp(source_id, sources[i].id) == 0) {
			return sources[i].url;
		}
	}
	return NULL;
}

static const char *browser_binary(enum stage_director_news_browser browser) {
	switch (browser) {
	case SD_NEWS_BROWSER_CHROMIUM: return "/usr/bin/chromium";
	case SD_NEWS_BROWSER_BRAVE: return "/usr/bin/brave-browser";
	case SD_NEWS_BROWSER_CHROME: return "/usr/bin/google-chrome-stable";
	case SD_NEWS_BROWSER_EDGE: return "/usr/bin/microsoft-edge-stable";
	}
	return NULL;
}

struct stage_director_news_launch_plan stage_director_news_plan(
		const struct stage_director_news_launch_request *request) {
	struct stage_director_news_launch_plan plan = {
		.status = SD_NEWS_LAUNCH_INVALID,
		.can_spawn = false,
	};
	if (!request) {
		return plan;
	}
	plan.source_url = stage_director_news_source_url(request->source_id);
	plan.source_registered = plan.source_url != NULL;
	if (!plan.source_registered) {
		plan.status = SD_NEWS_LAUNCH_UNKNOWN_SOURCE;
		return plan;
	}
	plan.binary = browser_binary(request->browser);
	if (!plan.binary || request->uid == 0 ||
			request->epoch == 0 || request->host_ref == 0) {
		plan.status = SD_NEWS_LAUNCH_INVALID_SESSION;
		return plan;
	}

	/*
	 * This is only an argv string. No directory is created. A future trusted
	 * owner must create/check a private (0700), symlink-free profile tree
	 * before launching, and must remove that tree safely on teardown.
	 */
	const int p = snprintf(plan.profile_flag, sizeof(plan.profile_flag),
		"--user-data-dir=/run/user/%" PRIu32
		"/post-apollo/weather-news/%" PRIu64,
		request->uid, request->epoch);
	const int a = snprintf(plan.app_flag, sizeof(plan.app_flag),
		"--app=%s", plan.source_url);
	if (p <= 0 || (size_t)p >= sizeof(plan.profile_flag) ||
			a <= 0 || (size_t)a >= sizeof(plan.app_flag)) {
		plan.status = SD_NEWS_LAUNCH_INVALID_SESSION;
		return plan;
	}
	plan.all_claimed_gates_present =
		request->operator_intent_authenticated &&
		request->native_host_verified &&
		request->compositor_clip_verified &&
		request->compositor_input_guard_verified &&
		request->compositor_popup_guard_verified &&
		request->compositor_teardown_guard_verified;
	plan.status = plan.all_claimed_gates_present
		? SD_NEWS_LAUNCH_CANDIDATE_ONLY
		: SD_NEWS_LAUNCH_AWAITING_CONTAINMENT;
	return plan;
}

size_t stage_director_news_plan_argv(
		const struct stage_director_news_launch_plan *plan,
		const char **argv, size_t capacity) {
	if (!plan || !argv || capacity < 9 ||
			plan->status != SD_NEWS_LAUNCH_CANDIDATE_ONLY ||
			!plan->source_registered || !plan->binary ||
			!plan->profile_flag[0] || !plan->app_flag[0]) {
		return 0;
	}
	/* The plan and arguments stay in-process. Never shell-escape them. */
	argv[0] = plan->binary;
	argv[1] = "--no-first-run";
	argv[2] = "--no-default-browser-check";
	argv[3] = "--new-window";
	argv[4] = "--mute-audio";
	argv[5] = "--class=post-apollo-weather-news";
	argv[6] = plan->profile_flag;
	argv[7] = plan->app_flag;
	argv[8] = NULL;
	return 8;
}
