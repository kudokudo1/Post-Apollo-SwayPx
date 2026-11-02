#include <wayland-server-core.h>
#include "sway/stage_director_news_request_peer.h"

/*
 * Deliberately dormant until an authorized WATCH-origin protocol exists.
 * Wayland resources are inspected inside the compositor. Never compare
 * caller-supplied integer pointers, PIDs, app IDs or layer-shell namespaces.
 */
struct stage_director_news_request_peer_result
stage_director_news_request_peer_probe(
		struct wl_resource *host_layer_role,
		struct wl_resource *host_surface,
		struct wl_resource *watch_request) {
	struct stage_director_news_request_peer_result result = {
		.state = SD_NEWS_REQUEST_PEER_INVALID_RESOURCES,
		/* Every authority field remains false on EVERY return path. */
	};
	if (!host_layer_role || !host_surface || !watch_request ||
			host_layer_role == host_surface ||
			watch_request == host_layer_role ||
			watch_request == host_surface) {
		return result;
	}

	struct wl_client *role_client = wl_resource_get_client(host_layer_role);
	struct wl_client *surface_client = wl_resource_get_client(host_surface);
	if (!role_client || !surface_client || role_client != surface_client) {
		result.state = SD_NEWS_REQUEST_PEER_HOST_CLIENT_MISMATCH;
		return result;
	}
	struct wl_client *request_client = wl_resource_get_client(watch_request);
	if (!request_client || request_client != role_client) {
		result.state = SD_NEWS_REQUEST_PEER_FOREIGN_CLIENT;
		return result;
	}

	result.state = SD_NEWS_REQUEST_PEER_NATIVE_MATCH_CANDIDATE_ONLY;
	result.same_native_client_candidate = true;
	return result;
}
