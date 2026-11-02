#ifndef _SWAY_STAGE_DIRECTOR_NEWS_VIEWPORT_CANDIDATE_H
#define _SWAY_STAGE_DIRECTOR_NEWS_VIEWPORT_CANDIDATE_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/stage_director_news_host_session.h"
#include "sway/stage_director_viewport.h"

/*
 * SD1 // Read-only bridge between a revisioned native Weather host and a
 * proposed interior NEWS viewport. This does NOT establish trusted interior
 * provenance, authorize scene mutation or bind a browser. No QML/IPC caller
 * may supply native references or turn an asserted flag into permission.
 *
 * A future compositor owner must independently observe the inner surface,
 * its lifecycle and the browser actor before calling this preflight. Even
 * a positive result remains a CANDIDATE, never an executable authority.
 */
struct stage_director_news_viewport_candidate_request {
	uint64_t epoch;
	uint64_t host_geometry_revision;
	uintptr_t native_host_ref;
	uintptr_t native_host_client_ref;
	uintptr_t native_output_ref;
	uintptr_t native_inner_surface_ref;
	bool inner_native_observation;
	bool host_mapped;
	bool actor_mapped;
	bool output_active;
	bool actor_identity_verified;
	struct stage_director_rect inner_global;
	struct stage_director_rect actor_global;
};

enum stage_director_news_viewport_candidate_status {
	SD_NEWS_VIEWPORT_NO_HOST,
	SD_NEWS_VIEWPORT_STALE_HOST,
	SD_NEWS_VIEWPORT_INNER_UNOBSERVED,
	SD_NEWS_VIEWPORT_INVALID_GEOMETRY,
	SD_NEWS_VIEWPORT_GEOMETRY_CANDIDATE_ONLY,
};

struct stage_director_news_viewport_candidate_result {
	enum stage_director_news_viewport_candidate_status status;
	bool geometry_candidate;
	struct stage_director_rect actor_local_crop;
	/* Always false. Geometry is not render/launch/seat/popup authority. */
	bool can_launch;
	bool can_render;
	bool can_clip;
	bool can_accept_input;
	bool can_show_popup;
};

struct stage_director_news_viewport_candidate_result
stage_director_news_viewport_candidate_probe(
	const struct stage_director_news_host_session *session,
	const struct stage_director_news_viewport_candidate_request *request);

#endif
