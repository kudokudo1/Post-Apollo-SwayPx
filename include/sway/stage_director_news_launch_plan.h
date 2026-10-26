#ifndef _SWAY_STAGE_DIRECTOR_NEWS_LAUNCH_PLAN_H
#define _SWAY_STAGE_DIRECTOR_NEWS_LAUNCH_PLAN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * NEWS // V0 fixed-source browser command planning.
 *
 * NO process launch, network, browser inspection, profile creation, IPC,
 * focus, scene mutation or renderer activation happens in this component.
 *
 * Any eventual spawn must be made by a separately authenticated compositor
 * session owner after real clip, input, popup and teardown guards exist.
 * Caller-supplied readiness booleans are NOT sufficient authorization.
 */
enum stage_director_news_browser {
	SD_NEWS_BROWSER_CHROMIUM,
	SD_NEWS_BROWSER_BRAVE,
	SD_NEWS_BROWSER_CHROME,
	SD_NEWS_BROWSER_EDGE,
};

enum stage_director_news_launch_status {
	SD_NEWS_LAUNCH_INVALID,
	SD_NEWS_LAUNCH_UNKNOWN_SOURCE,
	SD_NEWS_LAUNCH_INVALID_SESSION,
	SD_NEWS_LAUNCH_AWAITING_CONTAINMENT,
	SD_NEWS_LAUNCH_CANDIDATE_ONLY,
};

struct stage_director_news_launch_request {
	const char *source_id;  /* registered Weather NEWS catalog ID only */
	enum stage_director_news_browser browser;
	uint32_t uid;
	uint64_t epoch;
	uintptr_t host_ref;
	bool operator_intent_authenticated;
	bool native_host_verified;
	bool compositor_clip_verified;
	bool compositor_input_guard_verified;
	bool compositor_popup_guard_verified;
	bool compositor_teardown_guard_verified;
};

struct stage_director_news_launch_plan {
	enum stage_director_news_launch_status status;
	bool source_registered;
	bool all_claimed_gates_present;
	bool can_spawn; /* ALWAYS false until a trusted launcher exists */
	const char *binary; /* static, absolute, allowlisted candidate */
	const char *source_url; /* fixed URL, never arbitrary user input */
	char profile_flag[192]; /* --user-data-dir=/run/user/UID/.../EPOCH */
	char app_flag[512]; /* --app= registered HTTPS destination */
};

/* All source IDs and destinations mirror Weather NEWS source registry v1. */
size_t stage_director_news_sources_count(void);
const char *stage_director_news_source_url(const char *source_id);

struct stage_director_news_launch_plan stage_director_news_plan(
		const struct stage_director_news_launch_request *request);

/*
 * Fill caller-owned argv array from plan; no shell, expansion, or execution.
 * Plan must remain alive while argv is used. Returns zero on invalid plan.
 */
size_t stage_director_news_plan_argv(
		const struct stage_director_news_launch_plan *plan,
		const char **argv, size_t capacity);

#endif
