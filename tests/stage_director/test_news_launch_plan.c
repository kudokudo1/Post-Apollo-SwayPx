#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sway/stage_director_news_launch_plan.h"

static struct stage_director_news_launch_request request(void) {
	return (struct stage_director_news_launch_request) {
		.source_id = "house",
		.browser = SD_NEWS_BROWSER_CHROMIUM,
		.uid = 1000,
		.epoch = 123,
		.host_ref = 0x1234,
		.operator_intent_authenticated = true,
		.native_host_verified = true,
		.compositor_clip_verified = true,
		.compositor_input_guard_verified = true,
		.compositor_popup_guard_verified = true,
		.compositor_teardown_guard_verified = true,
	};
}

static void test_default_denial(void) {
	struct stage_director_news_launch_plan p = stage_director_news_plan(NULL);
	assert(p.status == SD_NEWS_LAUNCH_INVALID && !p.can_spawn);
	struct stage_director_news_launch_request r = request();
	r.operator_intent_authenticated = false;
	p = stage_director_news_plan(&r);
	assert(p.status == SD_NEWS_LAUNCH_AWAITING_CONTAINMENT);
	assert(!p.can_spawn && !p.all_claimed_gates_present);
	const char *argv[9] = {0};
	assert(stage_director_news_plan_argv(&p, argv, 9) == 0);
	r = request(); r.native_host_verified = false;
	assert(stage_director_news_plan(&r).status ==
		SD_NEWS_LAUNCH_AWAITING_CONTAINMENT);
	r = request(); r.compositor_clip_verified = false;
	assert(stage_director_news_plan(&r).status ==
		SD_NEWS_LAUNCH_AWAITING_CONTAINMENT);
	r = request(); r.compositor_input_guard_verified = false;
	assert(stage_director_news_plan(&r).status ==
		SD_NEWS_LAUNCH_AWAITING_CONTAINMENT);
	r = request(); r.compositor_popup_guard_verified = false;
	assert(stage_director_news_plan(&r).status ==
		SD_NEWS_LAUNCH_AWAITING_CONTAINMENT);
	r = request(); r.compositor_teardown_guard_verified = false;
	assert(stage_director_news_plan(&r).status ==
		SD_NEWS_LAUNCH_AWAITING_CONTAINMENT);
}

static void test_registered_commands_without_execution(void) {
	struct stage_director_news_launch_request r = request();
	struct stage_director_news_launch_plan p = stage_director_news_plan(&r);
	assert(p.status == SD_NEWS_LAUNCH_CANDIDATE_ONLY);
	assert(p.all_claimed_gates_present);
	assert(!p.can_spawn);
	const char *argv[9] = {0};
	assert(stage_director_news_plan_argv(&p, argv, 8) == 0);
	assert(stage_director_news_plan_argv(&p, argv, 9) == 8);
	assert(strcmp(argv[0], "/usr/bin/chromium") == 0);
	assert(strcmp(argv[5], "--class=post-apollo-weather-news") == 0);
	assert(strcmp(argv[6],
		"--user-data-dir=/run/user/1000/post-apollo/weather-news/123") == 0);
	assert(strcmp(argv[7], "--app=https://live.house.gov/") == 0);
	assert(argv[8] == NULL);
	assert(stage_director_news_sources_count() == 18);

	/* Neither an argument flag, shell expression nor arbitrary HTTPS URL
	 * can be treated as a valid Weather Station source identity. */
	const char *invalid[] = {
		NULL, "", "unknown", "--disable-web-security", "../house",
		"house;sh", "https://evil.example/", "house\n", "House", "123",
	};
	for (size_t i = 0; i < sizeof(invalid)/sizeof(invalid[0]); ++i) {
		r.source_id = invalid[i];
		p = stage_director_news_plan(&r);
		assert(p.status == SD_NEWS_LAUNCH_UNKNOWN_SOURCE);
		assert(!p.can_spawn);
		assert(stage_director_news_plan_argv(&p, argv, 9) == 0);
	}
}

static void test_exact_sources_and_binaries(void) {
	static const char *registered[] = {
		"aje", "livenow", "nbcnow", "abcnews", "cnn", "foxnews",
		"house", "senate", "hearings", "cspan", "bbc", "npr",
		"nasa", "nws", "groundnews", "allsides", "jubilee", "theonion",
	};
	for (size_t i = 0; i < 18; ++i) {
		const char *url = stage_director_news_source_url(registered[i]);
		assert(url != NULL && strncmp(url, "https://", 8) == 0);
		struct stage_director_news_launch_request r = request();
		r.source_id = registered[i];
		for (int browser = SD_NEWS_BROWSER_CHROMIUM;
				browser <= SD_NEWS_BROWSER_EDGE; ++browser) {
			r.browser = (enum stage_director_news_browser)browser;
			const struct stage_director_news_launch_plan p =
				stage_director_news_plan(&r);
			assert(p.status == SD_NEWS_LAUNCH_CANDIDATE_ONLY);
			assert(p.binary[0] == '/' && !p.can_spawn);
			assert(strncmp(p.app_flag, "--app=https://", 14) == 0);
		}
	}
	struct stage_director_news_launch_request r = request();
	r.browser = (enum stage_director_news_browser)100;
	assert(stage_director_news_plan(&r).status ==
		SD_NEWS_LAUNCH_INVALID_SESSION);
	r = request(); r.uid = 0;
	assert(stage_director_news_plan(&r).status ==
		SD_NEWS_LAUNCH_INVALID_SESSION);
	r = request(); r.epoch = 0;
	assert(stage_director_news_plan(&r).status ==
		SD_NEWS_LAUNCH_INVALID_SESSION);
	r = request(); r.host_ref = 0;
	assert(stage_director_news_plan(&r).status ==
		SD_NEWS_LAUNCH_INVALID_SESSION);
}

int main(void) {
	test_default_denial();
	test_registered_commands_without_execution();
	test_exact_sources_and_binaries();
	puts("Stage Director NEWS fixed-source browser launch planning: PASS");
	return 0;
}
