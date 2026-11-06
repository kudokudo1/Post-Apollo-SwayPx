#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <wayland-server.h>
#include "sway/stage_director_company_native.h"

static struct wl_client *client_create(struct wl_display *display,
		int *owned_fd) {
	int fds[2] = {-1, -1};
	assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);
	struct wl_client *client = wl_client_create(display, fds[0]);
	assert(client);
	*owned_fd = fds[1];
	return client;
}

static struct wl_resource *resource(struct wl_client *client) {
	struct wl_resource *r =
		wl_resource_create(client, &wl_surface_interface, 1, 0);
	assert(r);
	return r;
}

static void no_authority(struct stage_director_company_native_peer_result r) {
	assert(!r.owner_verified);
	assert(!r.can_clip);
	assert(!r.can_accept_input);
}

int main(void) {
	struct wl_display *display = wl_display_create();
	assert(display);
	int host_fd = -1, actor_fd = -1, foreign_fd = -1;
	struct wl_client *host = client_create(display, &host_fd);
	struct wl_client *actor = client_create(display, &actor_fd);
	struct wl_client *foreign = client_create(display, &foreign_fd);
	struct wl_resource *role_host = resource(host);
	struct wl_resource *surface_host = resource(host);
	struct wl_resource *root_actor = resource(actor);
	struct wl_resource *root_host = resource(host);
	struct wl_resource *surface_foreign = resource(foreign);

	struct stage_director_company_native_peer_result result =
		stage_director_company_native_peer_probe(
			role_host, surface_host, root_actor);
	no_authority(result);
	assert(result.state == SD_COMPANY_PEER_DISTINCT_CANDIDATE_ONLY);
	assert(result.distinct_native_clients_candidate);

	result = stage_director_company_native_peer_probe(NULL,
			surface_host, root_actor);
	assert(result.state == SD_COMPANY_PEER_INVALID_RESOURCES);
	assert(!result.distinct_native_clients_candidate);
	no_authority(result);
	result = stage_director_company_native_peer_probe(role_host,
			role_host, root_actor);
	assert(result.state == SD_COMPANY_PEER_INVALID_RESOURCES);
	no_authority(result);

	result = stage_director_company_native_peer_probe(role_host,
			surface_foreign, root_actor);
	assert(result.state == SD_COMPANY_PEER_SPLIT_HOST_CLIENT);
	assert(!result.distinct_native_clients_candidate);
	no_authority(result);

	/* A popup or sibling wl_surface from the host client isn't the
	 * independently launched Company MapLibre actor. */
	result = stage_director_company_native_peer_probe(role_host,
			surface_host, root_host);
	assert(result.state == SD_COMPANY_PEER_NOT_SEPARATE_ACTOR);
	assert(!result.distinct_native_clients_candidate);
	no_authority(result);

	wl_display_destroy_clients(display);
	close(host_fd);
	close(actor_fd);
	close(foreign_fd);
	wl_display_destroy(display);
	puts("STAGE_DIRECTOR_COMPANY_M20: PASS actual Wayland resources, no authority");
	return 0;
}
