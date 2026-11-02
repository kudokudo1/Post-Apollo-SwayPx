#include "sway/stage_director_news_viewport_candidate.h"

/* Deliberately pure: no scene nodes, process spawning, focus, input or IPC. */
static struct stage_director_news_viewport_candidate_result result(
		enum stage_director_news_viewport_candidate_status status) {
	return (struct stage_director_news_viewport_candidate_result) {
		.status = status,
	};
}

struct stage_director_news_viewport_candidate_result
stage_director_news_viewport_candidate_probe(
		const struct stage_director_news_host_session *session,
		const struct stage_director_news_viewport_candidate_request *request) {
	if (!session || !request || !request->epoch ||
			!request->native_host_ref || !request->native_host_client_ref ||
			!request->native_output_ref) {
		return result(SD_NEWS_VIEWPORT_NO_HOST);
	}
	const struct stage_director_news_host_geometry_snapshot host =
		stage_director_news_host_session_geometry_snapshot(session,
			request->native_host_ref, request->native_host_client_ref,
			request->native_output_ref, request->epoch);
	if (!host.candidate_only ||
			!stage_director_news_host_session_geometry_current(session, &host)) {
		return result(SD_NEWS_VIEWPORT_NO_HOST);
	}
	if (!request->host_geometry_revision ||
			request->host_geometry_revision != host.revision) {
		return result(SD_NEWS_VIEWPORT_STALE_HOST);
	}
	/* An arbitrary rectangle never proves it is the NEWS interior. The
	 * future owner must observe a native in-process surface separately.
	 * This precondition is necessary but NOT sufficient for authentication. */
	if (!request->inner_native_observation ||
			!request->native_inner_surface_ref ||
			request->native_inner_surface_ref == request->native_host_ref) {
		return result(SD_NEWS_VIEWPORT_INNER_UNOBSERVED);
	}
	const struct stage_director_viewport_request geometry = {
		.host = host.host,
		.output = host.output,
		.actor = request->actor_global,
		.viewport = request->inner_global,
		.host_mapped = request->host_mapped,
		.actor_mapped = request->actor_mapped,
		.output_active = request->output_active,
		.actor_identity_verified = request->actor_identity_verified,
	};
	const struct stage_director_viewport_plan plan =
		stage_director_viewport_plan(&geometry);
	if (plan.result != SD_VIEWPORT_GEOMETRY_ONLY ||
			!plan.geometry_valid || plan.can_launch ||
			plan.can_render || plan.can_accept_input) {
		return result(SD_NEWS_VIEWPORT_INVALID_GEOMETRY);
	}
	struct stage_director_news_viewport_candidate_result out =
		result(SD_NEWS_VIEWPORT_GEOMETRY_CANDIDATE_ONLY);
	out.geometry_candidate = true;
	out.actor_local_crop = plan.actor_local_crop;
	return out; /* No paint, pointer, keyboard, popup or browser permission. */
}
