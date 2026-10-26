#ifndef _SWAY_STAGE_DIRECTOR_VIEWER_SESSION_H
#define _SWAY_STAGE_DIRECTOR_VIEWER_SESSION_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/stage_director_viewport.h"

/*
 * Stage Director // V2 foundation // one candidate viewer session.
 *
 * These are native compositor observations, NOT external IPC identifiers.
 * This module NEVER starts/owns a child process, changes a scene, takes focus,
 * authorizes clipping, or trusts an app_id as proof of launcher provenance.
 *
 * The future trusted launcher must provide a process start marker (distinct
 * from PID reuse) and a new monotonic epoch for each operator-initiated WATCH.
 * The future compositor owner must authenticate the mapped layer host and
 * observed actor before promoting any candidate into an executable scene.
 */
enum stage_director_viewer_state {
	SD_VIEWER_EMPTY,
	SD_VIEWER_HOST_RESERVED,
	SD_VIEWER_ACTOR_CANDIDATE,
};

struct stage_director_viewer_session {
	enum stage_director_viewer_state state;
	uint64_t last_epoch; /* Survives close to reject old launch tickets. */
	uint64_t epoch;
	uintptr_t host_ref;
	uintptr_t actor_ref;
	int64_t expected_pid;
	uint64_t expected_process_start;
	struct stage_director_rect host;
	struct stage_director_rect output;
	struct stage_director_rect viewport;
	struct stage_director_rect actor;
};

struct stage_director_viewer_probe {
	bool candidate_matched;
	enum stage_director_viewport_result geometry_result;
	/* All remain false until trusted host/actor/clip/input owner exists. */
	bool can_launch;
	bool can_clip;
	bool can_accept_input;
};

void stage_director_viewer_session_init(
		struct stage_director_viewer_session *session);

/* Accept one prospective operator-initiated launch ticket, not a permission. */
bool stage_director_viewer_session_reserve(
		struct stage_director_viewer_session *session,
		uint64_t epoch, uintptr_t host_ref,
		int64_t expected_pid, uint64_t expected_process_start,
		struct stage_director_rect host,
		struct stage_director_rect output,
		struct stage_director_rect viewport);

/* Correlate native observations. PID and process start must both match. */
bool stage_director_viewer_session_attach(
		struct stage_director_viewer_session *session,
		uint64_t epoch, uintptr_t host_ref, uintptr_t actor_ref,
		int64_t observed_pid, uint64_t observed_process_start,
		struct stage_director_rect actor);

/* Geometry/identity inspection, with NO authority to activate a clip. */
struct stage_director_viewer_probe stage_director_viewer_session_probe(
		const struct stage_director_viewer_session *session,
		uint64_t epoch, uintptr_t host_ref, uintptr_t actor_ref,
		const struct stage_director_viewport_request *current);

/* Retire the candidate on unmap/destroy, process exit or explicit CLOSE. */
bool stage_director_viewer_session_host_lost(
		struct stage_director_viewer_session *session, uintptr_t host_ref);
bool stage_director_viewer_session_actor_lost(
		struct stage_director_viewer_session *session, uintptr_t actor_ref);
bool stage_director_viewer_session_close(
		struct stage_director_viewer_session *session, uint64_t epoch);

#endif
