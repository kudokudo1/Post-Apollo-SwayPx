#include <string.h>
#include <wayland-server-core.h>
#include "sway/stage_director_news_host_session.h"

/*
 * This source is built by the ordinary compositor, but has no admission
 * producer and no protocol listener. A native connection match is not
 * operator intent. The true WATCH global, trusted input pairing gesture,
 * source allowlist and browser/scene/seat permissions are separate gates.
 */

static struct stage_director_news_host_session_probe result(
		enum stage_director_news_host_session_state state) {
	return (struct stage_director_news_host_session_probe) {
		.state = state,
		/* All permissions stay FALSE, including successful candidate match. */
	};
}

void stage_director_news_host_session_init(
		struct stage_director_news_host_session *session) {
	if (!session || session->initialized) {
		return;
	}
	memset(session, 0, sizeof(*session));
	wl_list_init(&session->client_destroy.link);
	wl_list_init(&session->role_destroy.link);
	wl_list_init(&session->surface_destroy.link);
	session->initialized = true;
}

void stage_director_news_host_session_invalidate(
		struct stage_director_news_host_session *session) {
	if (!session || !session->initialized) {
		return;
	}
	if (session->candidate) {
		/* All listeners are compositor-local, never serialized to clients. */
		wl_list_remove(&session->client_destroy.link);
		wl_list_remove(&session->role_destroy.link);
		wl_list_remove(&session->surface_destroy.link);
		wl_list_init(&session->client_destroy.link);
		wl_list_init(&session->role_destroy.link);
		wl_list_init(&session->surface_destroy.link);
	}
	session->client = NULL;
	session->layer_role = NULL;
	session->host_surface = NULL;
	session->native_host_ref = 0;
	session->native_output_ref = 0;
	session->epoch = 0;
	session->candidate = false;
	/* last_epoch is intentionally retained to refuse stale replays. */
}

static void on_client_destroy(struct wl_listener *listener, void *data) {
	(void)data;
	struct stage_director_news_host_session *session =
		wl_container_of(listener, session, client_destroy);
	stage_director_news_host_session_invalidate(session);
}
static void on_role_destroy(struct wl_listener *listener, void *data) {
	(void)data;
	struct stage_director_news_host_session *session =
		wl_container_of(listener, session, role_destroy);
	stage_director_news_host_session_invalidate(session);
}
static void on_surface_destroy(struct wl_listener *listener, void *data) {
	(void)data;
	struct stage_director_news_host_session *session =
		wl_container_of(listener, session, surface_destroy);
	stage_director_news_host_session_invalidate(session);
}

bool stage_director_news_host_session_record_native_candidate(
		struct stage_director_news_host_session *session,
		struct wl_resource *layer_role, struct wl_resource *host_surface,
		uintptr_t native_host_ref, uintptr_t native_output_ref,
		uint64_t epoch) {
	if (!session || !session->initialized || session->candidate ||
			!layer_role || !host_surface || layer_role == host_surface ||
			!native_host_ref || !native_output_ref || !epoch ||
			epoch <= session->last_epoch) {
		return false;
	}
	struct wl_client *role_client = wl_resource_get_client(layer_role);
	struct wl_client *surface_client = wl_resource_get_client(host_surface);
	if (!role_client || role_client != surface_client) {
		return false;
	}
	session->client = role_client;
	session->layer_role = layer_role;
	session->host_surface = host_surface;
	session->native_host_ref = native_host_ref;
	session->native_output_ref = native_output_ref;
	session->epoch = epoch;
	session->last_epoch = epoch;
	session->candidate = true;

	session->client_destroy.notify = on_client_destroy;
	session->role_destroy.notify = on_role_destroy;
	session->surface_destroy.notify = on_surface_destroy;
	wl_client_add_destroy_listener(role_client, &session->client_destroy);
	wl_resource_add_destroy_listener(layer_role, &session->role_destroy);
	wl_resource_add_destroy_listener(host_surface, &session->surface_destroy);
	return true;
}

struct stage_director_news_host_session_probe
stage_director_news_host_session_probe_request(
		const struct stage_director_news_host_session *session,
		struct wl_resource *request_resource,
		uintptr_t current_host_ref, uintptr_t current_output_ref,
		uint64_t current_epoch) {
	if (!session || !session->initialized || !session->candidate) {
		return result(SD_NEWS_HOST_SESSION_EMPTY);
	}
	if (!request_resource || !current_host_ref || !current_output_ref ||
			!current_epoch) {
		return result(SD_NEWS_HOST_SESSION_INVALID);
	}
	if (current_epoch != session->epoch ||
			current_host_ref != session->native_host_ref ||
			current_output_ref != session->native_output_ref) {
		return result(SD_NEWS_HOST_SESSION_STALE);
	}
	const struct stage_director_news_request_peer_result peer =
		stage_director_news_request_peer_probe(
			session->layer_role, session->host_surface, request_resource);
	if (!peer.same_native_client_candidate ||
			peer.state != SD_NEWS_REQUEST_PEER_NATIVE_MATCH_CANDIDATE_ONLY) {
		return result(SD_NEWS_HOST_SESSION_FOREIGN_REQUEST);
	}
	if (wl_resource_get_client(request_resource) != session->client) {
		return result(SD_NEWS_HOST_SESSION_FOREIGN_REQUEST);
	}
	struct stage_director_news_host_session_probe out =
		result(SD_NEWS_HOST_SESSION_NATIVE_CANDIDATE_ONLY);
	out.native_candidate_matches = true;
	return out;
}

bool stage_director_news_host_session_tracks(
		const struct stage_director_news_host_session *session,
		uintptr_t native_host_ref) {
	return session && session->initialized && session->candidate &&
		native_host_ref && session->native_host_ref == native_host_ref;
}

void stage_director_news_host_session_host_lost(
		struct stage_director_news_host_session *session,
		uintptr_t native_host_ref) {
	if (stage_director_news_host_session_tracks(session, native_host_ref)) {
		stage_director_news_host_session_invalidate(session);
	}
}
