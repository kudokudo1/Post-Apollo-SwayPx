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

void stage_director_news_host_session_on_revoked(
		struct stage_director_news_host_session *session,
		void (*callback)(void *data), void *data) {
	if (!session || !session->initialized) {
		return;
	}
	session->on_revoked = callback;
	session->on_revoked_data = data;
}

void stage_director_news_host_session_clear_viewport_candidate(
		struct stage_director_news_host_session *session) {
	if (!session) return;
	session->viewport_candidate_valid = false;
	session->viewport_host_revision = 0;
	session->viewport_local = (struct stage_director_rect){0};
	session->viewport_global = (struct stage_director_rect){0};
}

void stage_director_news_host_session_invalidate(
		struct stage_director_news_host_session *session) {
	if (!session || !session->initialized) {
		return;
	}
	const bool was_candidate = session->candidate;
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
	session->geometry_revision = 0;
	session->geometry_valid = false;
	stage_director_news_host_session_clear_viewport_candidate(session);
	memset(&session->observed_host, 0, sizeof(session->observed_host));
	memset(&session->observed_output, 0, sizeof(session->observed_output));
	session->candidate = false;
	/* last_epoch is intentionally retained to refuse stale replays. */
	if (was_candidate && session->on_revoked) {
		session->on_revoked(session->on_revoked_data);
	}
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


/* Only called after a compositor-owned native layer/peer observation.
 * A matching rectangle remains CANDIDATE ONLY, not a clip/input credential. */
static bool same_rect(struct stage_director_rect a,
		struct stage_director_rect b) {
	return a.x == b.x && a.y == b.y &&
		a.width == b.width && a.height == b.height;
}

bool stage_director_news_host_session_update_geometry(
		struct stage_director_news_host_session *session,
		uintptr_t current_host_ref, uintptr_t current_client_ref,
		uintptr_t current_output_ref, uint64_t current_epoch,
		struct stage_director_rect host, struct stage_director_rect output) {
	if (!session || !session->initialized || !session->candidate ||
			!current_host_ref || current_host_ref != session->native_host_ref ||
			!current_client_ref ||
			current_client_ref != (uintptr_t)session->client ||
			!current_output_ref ||
			current_output_ref != session->native_output_ref ||
			!current_epoch || current_epoch != session->epoch ||
			!stage_director_rect_valid(host) ||
			!stage_director_rect_valid(output) ||
			!stage_director_rect_contains(output, host)) {
		return false;
	}
	if (session->geometry_valid &&
			same_rect(session->observed_host, host) &&
			same_rect(session->observed_output, output)) {
		return true;
	}
	if (session->last_geometry_revision == UINT64_MAX) {
		return false; /* Never wrap into a stale snapshot's revision. */
	}
	session->last_geometry_revision++;
	session->geometry_revision = session->last_geometry_revision;
	/* In-flight reports were relative to the prior outer host. */
	stage_director_news_host_session_clear_viewport_candidate(session);
	session->observed_host = host;
	session->observed_output = output;
	session->geometry_valid = true;
	return true;
}

struct stage_director_news_host_geometry_snapshot
stage_director_news_host_session_geometry_snapshot(
		const struct stage_director_news_host_session *session,
		uintptr_t current_host_ref, uintptr_t current_client_ref,
		uintptr_t current_output_ref, uint64_t current_epoch) {
	struct stage_director_news_host_geometry_snapshot out = {0};
	if (!session || !session->initialized || !session->candidate ||
			!session->geometry_valid || !session->geometry_revision ||
			!current_host_ref || current_host_ref != session->native_host_ref ||
			!current_client_ref ||
			current_client_ref != (uintptr_t)session->client ||
			!current_output_ref ||
			current_output_ref != session->native_output_ref ||
			!current_epoch || current_epoch != session->epoch) {
		return out;
	}
	out.candidate_only = true;
	out.epoch = current_epoch;
	out.revision = session->geometry_revision;
	out.native_host_ref = current_host_ref;
	out.native_client_ref = current_client_ref;
	out.native_output_ref = current_output_ref;
	out.host = session->observed_host;
	out.output = session->observed_output;
	return out; /* Permission flags deliberately remain false. */
}

bool stage_director_news_host_session_geometry_current(
		const struct stage_director_news_host_session *session,
		const struct stage_director_news_host_geometry_snapshot *snapshot) {
	if (!snapshot || !snapshot->candidate_only || !snapshot->revision ||
			snapshot->can_launch || snapshot->can_clip ||
			snapshot->can_accept_input || snapshot->can_show_popup) {
		return false;
	}
	const struct stage_director_news_host_geometry_snapshot now =
		stage_director_news_host_session_geometry_snapshot(session,
			snapshot->native_host_ref, snapshot->native_client_ref,
			snapshot->native_output_ref, snapshot->epoch);
	return now.candidate_only && now.revision == snapshot->revision &&
		same_rect(now.host, snapshot->host) &&
		same_rect(now.output, snapshot->output);
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

/* Same-client and WATCH lease validation happens BEFORE this helper in
 * the actual Wayland request handler. Even a bounded client rectangle is
 * geometrical evidence, not an authenticated inner Wayland surface. */
struct stage_director_news_host_viewport_receipt
stage_director_news_host_session_report_viewport(
		struct stage_director_news_host_session *session,
		uintptr_t native_host_ref, uintptr_t native_client_ref,
		uintptr_t native_output_ref, uint64_t epoch,
		struct stage_director_rect inner_host_local) {
	struct stage_director_news_host_viewport_receipt out = {
		.status = SD_NEWS_HOST_VIEWPORT_DENIED,
	};
	if (!session || !session->initialized || !session->candidate) {
		return out;
	}
	const struct stage_director_news_host_geometry_snapshot host =
		stage_director_news_host_session_geometry_snapshot(session,
			native_host_ref, native_client_ref, native_output_ref, epoch);
	if (!host.candidate_only || !host.revision ||
			!stage_director_news_host_session_geometry_current(session, &host)) {
		return out; /* Foreign requester must not clear a real client's report. */
	}
	/* Every response echoes the compositor's current revision. An invalid
	 * report from the paired client revokes its previously accepted viewport. */
	stage_director_news_host_session_clear_viewport_candidate(session);
	out.epoch = host.epoch;
	out.host_geometry_revision = host.revision;
	if (inner_host_local.x == 0 && inner_host_local.y == 0 &&
			inner_host_local.width == 0 && inner_host_local.height == 0) {
		out.status = SD_NEWS_HOST_VIEWPORT_HIDDEN;
		return out;
	}
	struct stage_director_rect translated = {0};
	if (!stage_director_viewport_from_host_local(
			host.host, inner_host_local, &translated) ||
			!stage_director_rect_contains(host.output, translated)) {
		return out;
	}
	session->viewport_candidate_valid = true;
	session->viewport_host_revision = host.revision;
	session->viewport_local = inner_host_local;
	session->viewport_global = translated;
	out.status = SD_NEWS_HOST_VIEWPORT_CANDIDATE_ONLY;
	out.inner_global = translated;
	/* All can_* permissions remain FALSE, including a successful report. */
	return out;
}

bool stage_director_news_host_session_viewport_current(
		const struct stage_director_news_host_session *session,
		const struct stage_director_news_host_viewport_receipt *receipt) {
	if (!session || !receipt ||
			receipt->status != SD_NEWS_HOST_VIEWPORT_CANDIDATE_ONLY ||
			receipt->can_launch || receipt->can_clip ||
			receipt->can_render || receipt->can_accept_input ||
			receipt->can_show_popup || !session->viewport_candidate_valid ||
			!session->viewport_host_revision ||
			receipt->host_geometry_revision !=
				session->viewport_host_revision ||
			receipt->epoch != session->epoch) {
		return false;
	}
	const struct stage_director_news_host_geometry_snapshot host =
		stage_director_news_host_session_geometry_snapshot(session,
			session->native_host_ref, (uintptr_t)session->client,
			session->native_output_ref, receipt->epoch);
	return host.candidate_only &&
		stage_director_news_host_session_geometry_current(session, &host) &&
		host.revision == receipt->host_geometry_revision &&
		session->viewport_global.x == receipt->inner_global.x &&
		session->viewport_global.y == receipt->inner_global.y &&
		session->viewport_global.width == receipt->inner_global.width &&
		session->viewport_global.height == receipt->inner_global.height &&
		stage_director_rect_contains(host.host, receipt->inner_global) &&
		stage_director_rect_contains(host.output, receipt->inner_global);
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
