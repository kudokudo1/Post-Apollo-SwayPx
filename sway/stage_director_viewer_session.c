#include <string.h>
#include "sway/stage_director_viewer_session.h"

/*
 * Candidate-only lifecycle policy. Never expose a false authority flag:
 * even a matching PID + process start marker and viewport are *observations*.
 * The compositor must eventually cross-check these against its own trusted
 * launcher and specific mapped layer-shell surface.
 */
static bool same_rect(struct stage_director_rect a,
		struct stage_director_rect b) {
	return a.x == b.x && a.y == b.y &&
		a.width == b.width && a.height == b.height;
}

static void clear_candidate(struct stage_director_viewer_session *session) {
	if (!session) {
		return;
	}
	const uint64_t last_epoch = session->last_epoch;
	memset(session, 0, sizeof(*session));
	session->state = SD_VIEWER_EMPTY;
	session->last_epoch = last_epoch;
}

void stage_director_viewer_session_init(
		struct stage_director_viewer_session *session) {
	if (session) {
		memset(session, 0, sizeof(*session));
		session->state = SD_VIEWER_EMPTY;
	}
}

bool stage_director_viewer_session_reserve(
		struct stage_director_viewer_session *session,
		uint64_t epoch, uintptr_t host_ref,
		int64_t expected_pid, uint64_t expected_process_start,
		struct stage_director_rect host,
		struct stage_director_rect output,
		struct stage_director_rect viewport) {
	if (!session || session->state != SD_VIEWER_EMPTY ||
			epoch == 0 || epoch <= session->last_epoch ||
			host_ref == 0 || expected_pid <= 0 ||
			expected_process_start == 0 ||
			!stage_director_rect_contains(host, viewport) ||
			!stage_director_rect_contains(output, viewport)) {
		return false;
	}
	session->state = SD_VIEWER_HOST_RESERVED;
	session->last_epoch = epoch;
	session->epoch = epoch;
	session->host_ref = host_ref;
	session->expected_pid = expected_pid;
	session->expected_process_start = expected_process_start;
	session->host = host;
	session->output = output;
	session->viewport = viewport;
	return true;
}

bool stage_director_viewer_session_attach(
		struct stage_director_viewer_session *session,
		uint64_t epoch, uintptr_t host_ref, uintptr_t actor_ref,
		int64_t observed_pid, uint64_t observed_process_start,
		struct stage_director_rect actor) {
	if (!session || session->state != SD_VIEWER_HOST_RESERVED ||
			epoch == 0 || epoch != session->epoch ||
			host_ref == 0 || host_ref != session->host_ref ||
			actor_ref == 0 || actor_ref == session->host_ref ||
			observed_pid != session->expected_pid ||
			observed_process_start != session->expected_process_start ||
			!stage_director_rect_contains(actor, session->viewport)) {
		return false;
	}
	session->state = SD_VIEWER_ACTOR_CANDIDATE;
	session->actor_ref = actor_ref;
	session->actor = actor;
	return true;
}

struct stage_director_viewer_probe stage_director_viewer_session_probe(
		const struct stage_director_viewer_session *session,
		uint64_t epoch, uintptr_t host_ref, uintptr_t actor_ref,
		const struct stage_director_viewport_request *current) {
	struct stage_director_viewer_probe probe = {
		.geometry_result = SD_VIEWPORT_DENY_UNVERIFIED_ACTOR,
		.can_launch = false,
		.can_clip = false,
		.can_accept_input = false,
	};
	if (!session || !current ||
			session->state != SD_VIEWER_ACTOR_CANDIDATE ||
			epoch != session->epoch || epoch == 0 ||
			host_ref != session->host_ref || host_ref == 0 ||
			actor_ref != session->actor_ref || actor_ref == 0 ||
			!same_rect(session->host, current->host) ||
			!same_rect(session->output, current->output) ||
			!same_rect(session->viewport, current->viewport) ||
			!same_rect(session->actor, current->actor)) {
		return probe;
	}
	/* These booleans are current compositor observations, not a grant. */
	const struct stage_director_viewport_plan geometry =
		stage_director_viewport_plan(current);
	probe.geometry_result = geometry.result;
	probe.candidate_matched = geometry.geometry_valid &&
		geometry.result == SD_VIEWPORT_GEOMETRY_ONLY;
	return probe;
}

bool stage_director_viewer_session_host_lost(
		struct stage_director_viewer_session *session, uintptr_t host_ref) {
	if (!session || session->state == SD_VIEWER_EMPTY ||
			host_ref == 0 || session->host_ref != host_ref) {
		return false;
	}
	clear_candidate(session);
	return true;
}

bool stage_director_viewer_session_actor_lost(
		struct stage_director_viewer_session *session, uintptr_t actor_ref) {
	if (!session || session->state != SD_VIEWER_ACTOR_CANDIDATE ||
			actor_ref == 0 || session->actor_ref != actor_ref) {
		return false;
	}
	clear_candidate(session);
	return true;
}

bool stage_director_viewer_session_close(
		struct stage_director_viewer_session *session, uint64_t epoch) {
	if (!session || session->state == SD_VIEWER_EMPTY ||
			epoch == 0 || session->epoch != epoch) {
		return false;
	}
	clear_candidate(session);
	return true;
}
