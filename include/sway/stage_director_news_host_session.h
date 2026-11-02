#ifndef _SWAY_STAGE_DIRECTOR_NEWS_HOST_SESSION_H
#define _SWAY_STAGE_DIRECTOR_NEWS_HOST_SESSION_H

#include <stdbool.h>
#include <stdint.h>
#include <wayland-server-core.h>
#include "sway/stage_director_news_request_peer.h"

/*
 * Native compositor-owned Weather host CANDIDATE custody.
 *
 * There is deliberately no operator-selection issuer here. The future trusted
 * physical-input ceremony must independently select a mapped Weather layer
 * before any producer may record a candidate; no Sway IPC or QML caller is
 * allowed. This module cannot authenticate WATCH or grant any permission.
 */
enum stage_director_news_host_session_state {
	SD_NEWS_HOST_SESSION_EMPTY,
	SD_NEWS_HOST_SESSION_NATIVE_CANDIDATE_ONLY,
	SD_NEWS_HOST_SESSION_FOREIGN_REQUEST,
	SD_NEWS_HOST_SESSION_STALE,
	SD_NEWS_HOST_SESSION_INVALID,
};

struct stage_director_news_host_session_probe {
	enum stage_director_news_host_session_state state;
	bool native_candidate_matches;
	bool operator_watch_authenticated;
	bool can_reserve_lease;
	bool can_launch;
	bool can_bind_viewer;
	bool can_clip;
	bool can_render;
	bool can_accept_input;
	bool can_show_popup;
};

struct stage_director_news_host_session {
	struct wl_client *client;
	struct wl_resource *layer_role;
	struct wl_resource *host_surface;
	uintptr_t native_host_ref;
	uintptr_t native_output_ref;
	uint64_t epoch;
	uint64_t last_epoch;
	bool initialized;
	bool candidate;
	/* Called after native refs have been invalidated, including disconnect. */
	void (*on_revoked)(void *data);
	void *on_revoked_data;
	struct wl_listener client_destroy;
	struct wl_listener role_destroy;
	struct wl_listener surface_destroy;
};

void stage_director_news_host_session_init(
	struct stage_director_news_host_session *session);

void stage_director_news_host_session_on_revoked(
	struct stage_director_news_host_session *session,
	void (*callback)(void *data), void *data);

/* Native resource candidate, NOT a trusted operator pairing. No production
 * caller until the separately reviewed input provenance path exists. */
bool stage_director_news_host_session_record_native_candidate(
	struct stage_director_news_host_session *session,
	struct wl_resource *layer_role,
	struct wl_resource *host_surface,
	uintptr_t native_host_ref, uintptr_t native_output_ref,
	uint64_t epoch);

/* Incoming request must belong to the exact candidate Wayland connection.
 * Re-check scene/host/output generation separately before any later action. */
struct stage_director_news_host_session_probe
stage_director_news_host_session_probe_request(
	const struct stage_director_news_host_session *session,
	struct wl_resource *request_resource,
	uintptr_t current_host_ref, uintptr_t current_output_ref,
	uint64_t current_epoch);

/* Handles compositor layer-unmap/destroy without dereferencing stale refs. */
void stage_director_news_host_session_host_lost(
	struct stage_director_news_host_session *session,
	uintptr_t native_host_ref);

/* A scene/output/peer re-check can invalidate the native candidate. */
void stage_director_news_host_session_invalidate(
	struct stage_director_news_host_session *session);

bool stage_director_news_host_session_tracks(
	const struct stage_director_news_host_session *session,
	uintptr_t native_host_ref);

#endif
