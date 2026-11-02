/*
 * Disposable wire-level request proof. This program is NOT built by SwayPX
 * Meson, does not register a production Wayland global, does not pair an
 * operator-selected host, and never grants a viewer or compositor authority.
 *
 * Two different wl_client connections live in the same OS process and claim
 * the same (fixture-only) Weather namespace. A real Wayland protocol request
 * reaches its resource-bound compositor handler over each socket.
 */
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-server.h>
#include "sway/stage_director_news_request_peer.h"
#include "ci-watch-client.h"
#include "ci-watch-server.h"

#define WEATHER_NS "post-apollo-weather-station"

struct fixture {
	struct wl_display *server;
	struct wl_client *selected;
	struct wl_client *spoof;
	struct wl_resource *selected_role;
	struct wl_resource *selected_surface;
	struct wl_resource *spoof_role;
	struct wl_resource *spoof_surface;
	const char *selected_namespace;
	const char *spoof_namespace;
	atomic_bool running;
	atomic_bool selected_mapped;
	atomic_int selected_bind;
	atomic_int spoof_bind;
	atomic_int matched_watch;
	atomic_int foreign_watch;
	atomic_int invalid_source;
	atomic_int unmapped_watch;
	atomic_int matched_close;
	atomic_int foreign_close;
};
struct client_connection {
	struct wl_display *display;
	struct wl_registry *registry;
	struct stage_director_ci_watch_v1 *watch;
};

static void assert_zero_authority(
		struct stage_director_news_request_peer_result r) {
	assert(!r.operator_watch_authenticated);
	assert(!r.can_reserve_lease);
	assert(!r.can_launch);
	assert(!r.can_bind_viewer);
	assert(!r.can_clip);
	assert(!r.can_accept_input);
	assert(!r.can_show_popup);
}

static void wire_watch(struct wl_client *client,
		struct wl_resource *resource, const char *source_id) {
	struct fixture *f = wl_resource_get_user_data(resource);
	assert(f != NULL);
	assert(client == wl_resource_get_client(resource));
	/* Both clients have *claimed* an identical Weather namespace. */
	assert(strcmp(f->selected_namespace, f->spoof_namespace) == 0);

	const struct stage_director_news_request_peer_result owner =
		stage_director_news_request_peer_probe(
			f->selected_role, f->selected_surface, resource);
	assert_zero_authority(owner);

	if (client == f->spoof) {
		/* A valid native identity for a spoofed host is still not trusted. */
		const struct stage_director_news_request_peer_result impostor =
			stage_director_news_request_peer_probe(
				f->spoof_role, f->spoof_surface, resource);
		assert_zero_authority(impostor);
		assert(impostor.same_native_client_candidate);
		assert(!owner.same_native_client_candidate);
		assert(owner.state == SD_NEWS_REQUEST_PEER_FOREIGN_CLIENT);
		atomic_fetch_add(&f->foreign_watch, 1);
		return;
	}
	assert(client == f->selected);
	assert(owner.same_native_client_candidate);
	assert(owner.state == SD_NEWS_REQUEST_PEER_NATIVE_MATCH_CANDIDATE_ONLY);

	/* Test-only additional fail-closed gates. No lease or effect is granted. */
	if (!atomic_load(&f->selected_mapped)) {
		atomic_fetch_add(&f->unmapped_watch, 1);
	} else if (!source_id ||
			(strcmp(source_id, "bbc") != 0 &&
			 strcmp(source_id, "cspan") != 0)) {
		atomic_fetch_add(&f->invalid_source, 1);
	} else {
		atomic_fetch_add(&f->matched_watch, 1);
	}
}

static void wire_close(struct wl_client *client, struct wl_resource *resource) {
	struct fixture *f = wl_resource_get_user_data(resource);
	assert(f != NULL);
	assert(client == wl_resource_get_client(resource));
	const struct stage_director_news_request_peer_result owner =
		stage_director_news_request_peer_probe(
			f->selected_role, f->selected_surface, resource);
	assert_zero_authority(owner);
	if (owner.same_native_client_candidate &&
			atomic_load(&f->selected_mapped)) {
		atomic_fetch_add(&f->matched_close, 1);
	} else {
		atomic_fetch_add(&f->foreign_close, 1);
	}
}

static const struct stage_director_ci_watch_v1_interface watch_impl = {
	.watch = wire_watch,
	.close = wire_close,
};

static void bind_watch(struct wl_client *client, void *data,
		uint32_t version, uint32_t id) {
	struct fixture *f = data;
	assert(version == 1);
	struct wl_resource *resource = wl_resource_create(
		client, &stage_director_ci_watch_v1_interface, version, id);
	assert(resource != NULL);
	/*
	 * The fixture deliberately advertises/binds the protocol to BOTH peers
	 * to prove request-time checking, rather than assuming hiding a global
	 * is sufficient security. Even a bind gives no WATCH permission.
	 */
	wl_resource_set_implementation(resource, &watch_impl, f, NULL);
	if (client == f->selected) {
		atomic_fetch_add(&f->selected_bind, 1);
	} else {
		assert(client == f->spoof);
		atomic_fetch_add(&f->spoof_bind, 1);
	}
}

static void *server_loop(void *context) {
	struct fixture *f = context;
	struct wl_event_loop *loop = wl_display_get_event_loop(f->server);
	assert(loop != NULL);
	while (atomic_load(&f->running)) {
		assert(wl_event_loop_dispatch(loop, 10) >= 0);
		wl_display_flush_clients(f->server);
	}
	return NULL;
}

static void on_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version) {
	struct client_connection *c = data;
	if (strcmp(interface, stage_director_ci_watch_v1_interface.name) != 0) {
		return;
	}
	assert(version >= 1);
	assert(c->watch == NULL);
	c->watch = wl_registry_bind(registry, name,
		&stage_director_ci_watch_v1_interface, 1);
	assert(c->watch != NULL);
}
static void on_global_remove(void *data, struct wl_registry *registry,
		uint32_t name) {
	(void)data;
	(void)registry;
	(void)name;
}
static const struct wl_registry_listener registry_listener = {
	.global = on_global,
	.global_remove = on_global_remove,
};

static void open_client(struct client_connection *c, int fd) {
	c->display = wl_display_connect_to_fd(fd);
	assert(c->display != NULL);
	c->registry = wl_display_get_registry(c->display);
	assert(c->registry != NULL);
	assert(wl_registry_add_listener(c->registry, &registry_listener, c) == 0);
	assert(wl_display_roundtrip(c->display) >= 0);
	assert(c->watch != NULL);
	assert(wl_display_roundtrip(c->display) >= 0);
}
static void synced(struct client_connection *c) {
	assert(wl_display_roundtrip(c->display) >= 0);
}

static struct wl_client *new_client(struct wl_display *server, int *fd) {
	int sockets[2] = {-1, -1};
	assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
	struct wl_client *client = wl_client_create(server, sockets[0]);
	assert(client != NULL);
	*fd = sockets[1];
	return client;
}
static struct wl_resource *new_host_resource(struct wl_client *client) {
	/* Object ID 0 lets libwayland allocate its server-owned resource ID. */
	struct wl_resource *resource =
		wl_resource_create(client, &wl_surface_interface, 1, 0);
	assert(resource != NULL);
	return resource;
}
static void close_client(struct client_connection *c) {
	stage_director_ci_watch_v1_destroy(c->watch);
	wl_registry_destroy(c->registry);
	wl_display_disconnect(c->display);
}

int main(void) {
	/* A hung client roundtrip is a test failure, not an unbounded runner. */
	alarm(20);
	struct fixture f = {0};
	f.server = wl_display_create();
	assert(f.server != NULL);
	f.selected_namespace = WEATHER_NS;
	f.spoof_namespace = WEATHER_NS;
	atomic_init(&f.running, true);
	atomic_init(&f.selected_mapped, true);
	int selected_fd = -1, spoof_fd = -1;
	f.selected = new_client(f.server, &selected_fd);
	f.spoof = new_client(f.server, &spoof_fd);
	assert(f.selected != f.spoof);
	f.selected_role = new_host_resource(f.selected);
	f.selected_surface = new_host_resource(f.selected);
	f.spoof_role = new_host_resource(f.spoof);
	f.spoof_surface = new_host_resource(f.spoof);
	/* Fixture-selected host is not an actual trusted operator pairing. */
	struct wl_global *global = wl_global_create(f.server,
		&stage_director_ci_watch_v1_interface, 1, &f, bind_watch);
	assert(global != NULL);
	pthread_t thread;
	assert(pthread_create(&thread, NULL, server_loop, &f) == 0);

	struct client_connection selected = {0}, spoof = {0};
	open_client(&selected, selected_fd);
	open_client(&spoof, spoof_fd);

	stage_director_ci_watch_v1_watch(selected.watch, "bbc");
	synced(&selected);
	stage_director_ci_watch_v1_watch(spoof.watch, "bbc");
	synced(&spoof);
	stage_director_ci_watch_v1_watch(selected.watch, "https://example.com");
	synced(&selected);
	/* Simulated unmap gate in the fixture, not an actual compositor hook. */
	atomic_store(&f.selected_mapped, false);
	stage_director_ci_watch_v1_watch(selected.watch, "bbc");
	synced(&selected);
	atomic_store(&f.selected_mapped, true);
	stage_director_ci_watch_v1_close(spoof.watch);
	synced(&spoof);
	stage_director_ci_watch_v1_close(selected.watch);
	synced(&selected);

	assert(atomic_load(&f.selected_bind) == 1);
	assert(atomic_load(&f.spoof_bind) == 1);
	assert(atomic_load(&f.matched_watch) == 1);
	assert(atomic_load(&f.foreign_watch) == 1);
	assert(atomic_load(&f.invalid_source) == 1);
	assert(atomic_load(&f.unmapped_watch) == 1);
	assert(atomic_load(&f.matched_close) == 1);
	assert(atomic_load(&f.foreign_close) == 1);

	close_client(&selected);
	close_client(&spoof);
	atomic_store(&f.running, false);
	assert(pthread_join(thread, NULL) == 0);
	wl_global_destroy(global);
	wl_display_destroy_clients(f.server);
	wl_display_destroy(f.server);
	puts("SD_CI_REAL_WATCH_REQUEST: real Wayland wire WATCH/CLOSE, "
	     "two same-namespace peers, request-time foreign refusal, "
	     "source/unmap fixture gates and ALL authority denied: PASS");
	return 0;
}
