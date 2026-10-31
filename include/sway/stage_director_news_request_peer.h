#ifndef _SWAY_STAGE_DIRECTOR_NEWS_REQUEST_PEER_H
#define _SWAY_STAGE_DIRECTOR_NEWS_REQUEST_PEER_H

#include <stdbool.h>

/*
 * SD1 primitive: inspect actual compositor-owned wl_resource objects.
 * A same-client match is only candidate evidence, NOT operator WATCH intent,
 * a trusted Weather host, a session lease, or permission to act on a scene.
 *
 * This header deliberately exposes no raw client pointer, PID, URL, or
 * authorization setter to the caller. It has no protocol/global/IPC endpoint.
 */
struct wl_resource;

enum stage_director_news_request_peer_state {
	SD_NEWS_REQUEST_PEER_INVALID_RESOURCES,
	SD_NEWS_REQUEST_PEER_HOST_CLIENT_MISMATCH,
	SD_NEWS_REQUEST_PEER_FOREIGN_CLIENT,
	SD_NEWS_REQUEST_PEER_NATIVE_MATCH_CANDIDATE_ONLY,
};

struct stage_director_news_request_peer_result {
	enum stage_director_news_request_peer_state state;
	bool same_native_client_candidate;
	bool operator_watch_authenticated;
	bool can_reserve_lease;
	bool can_launch;
	bool can_bind_viewer;
	bool can_clip;
	bool can_accept_input;
	bool can_show_popup;
};

/*
 * The eventual private Wayland request handler may use this AFTER its trusted
 * host pairing/launch proof, mapped-layer role and generation checks.
 * All three resource arguments MUST be real in-process native resources:
 *   - mapped Weather layer-shell role resource
 *   - underlying Weather wl_surface resource
 *   - resource receiving the candidate WATCH request
 * A match NEVER substitutes for those missing checks.
 */
struct stage_director_news_request_peer_result
stage_director_news_request_peer_probe(
	struct wl_resource *host_layer_role,
	struct wl_resource *host_surface,
	struct wl_resource *watch_request);

#endif
