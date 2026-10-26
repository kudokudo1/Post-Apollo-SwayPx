#ifndef _SWAY_STAGE_DIRECTOR_NEWS_CONTAINMENT_H
#define _SWAY_STAGE_DIRECTOR_NEWS_CONTAINMENT_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/stage_director_viewport.h"

/*
 * Candidate-only compositor-global *logical* input and popup containment.
 * Geometric eligibility is not proof of correct wlroots scene targeting.
 * No active viewer is authorized by this API.
 */
enum stage_director_news_containment_status {
	SD_NEWS_CONTAINMENT_NO_SESSION,
	SD_NEWS_CONTAINMENT_STALE_GEOMETRY,
	SD_NEWS_CONTAINMENT_INVALID_COORDS,
	SD_NEWS_CONTAINMENT_FOREIGN_SURFACE,
	SD_NEWS_CONTAINMENT_OUTSIDE_VIEWPORT,
	SD_NEWS_CONTAINMENT_POPUP_OVERFLOW,
	SD_NEWS_CONTAINMENT_CANDIDATE_ONLY,
};

struct stage_director_news_containment_snapshot {
	struct stage_director_viewport_request request;
	uint64_t epoch;
	uintptr_t native_host_ref;
	uintptr_t native_actor_ref;
	/* Future trusted compositor owner must bind these native references. */
	bool session_owner_verified;
	bool actor_process_verified;
};

struct stage_director_news_containment_result {
	enum stage_director_news_containment_status status;
	bool geometry_candidate;
	bool pointer_candidate;
	bool popup_candidate;
	/* Critical: geometry probes never grant event dispatch or popup display. */
	bool can_dispatch_input;
	bool can_show_popup;
	bool can_enable_clip;
};

/*
 * Captured native compositor association must be supplied by a future
 * session owner. candidate_target_ref is the NATIVE observed actor ref,
 * not browser-provided app_id/class/title/URL. Coordinates may be fractional.
 */
struct stage_director_news_containment_result
stage_director_news_pointer_probe(
		const struct stage_director_news_containment_snapshot *snapshot,
		uint64_t current_epoch, uintptr_t current_host_ref,
		uintptr_t candidate_target_ref, double global_x, double global_y);

/* Strict full-rectangle containment; overflow popup must be denied. */
struct stage_director_news_containment_result
stage_director_news_popup_probe(
		const struct stage_director_news_containment_snapshot *snapshot,
		uint64_t current_epoch, uintptr_t current_host_ref,
		uintptr_t candidate_actor_ref,
		struct stage_director_rect popup_global_bounds);

#endif
