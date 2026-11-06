#include <wayland-server-core.h>
#include "sway/stage_director_company_native.h"

/* This checks actual resource->wl_client identity, not an app_id, pid,
 * title or layer namespace. Distinct clients are necessary but NOT
 * sufficient to prove that an XDG actor belongs to the paired Company. */
struct stage_director_company_native_peer_result
stage_director_company_native_peer_probe(
		struct wl_resource *layer_role,
		struct wl_resource *host_surface,
		struct wl_resource *actor_root_surface) {
	struct stage_director_company_native_peer_result out = {
		.state = SD_COMPANY_PEER_INVALID_RESOURCES,
	};
	if (!layer_role || !host_surface || !actor_root_surface ||
			layer_role == host_surface || layer_role == actor_root_surface ||
			host_surface == actor_root_surface) {
		return out;
	}
	struct wl_client *role_client = wl_resource_get_client(layer_role);
	struct wl_client *surface_client = wl_resource_get_client(host_surface);
	struct wl_client *actor_client = wl_resource_get_client(actor_root_surface);
	if (!role_client || !surface_client || !actor_client) {
		return out;
	}
	if (role_client != surface_client) {
		out.state = SD_COMPANY_PEER_SPLIT_HOST_CLIENT;
		return out;
	}
	if (actor_client == role_client) {
		out.state = SD_COMPANY_PEER_NOT_SEPARATE_ACTOR;
		return out;
	}
	out.state = SD_COMPANY_PEER_DISTINCT_CANDIDATE_ONLY;
	out.distinct_native_clients_candidate = true;
	return out;
}
