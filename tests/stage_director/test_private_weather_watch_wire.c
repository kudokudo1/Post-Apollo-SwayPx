/*
 * Real generated-protocol integration smoke against production WATCH handler.
 * Only the Stage Director native host verifier is stubbed to create a selected
 * client fixture. The actual global filter, bind/request checks, protocol code
 * and destructor are exercised with two independent wl_client connections.
 * No browser, compositor, Qt, scene or input authority is enabled by the test.
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
#include <wayland-server.h>
#include <wayland-client.h>
#include "sway/stage_director_weather_watch_protocol.h"
#include "post-apollo-weather-watch-v1-client-protocol.h"

static struct wl_client *paired_client;
static bool paired;
static struct wl_display *server;
static atomic_bool running;

bool stage_director_news_watch_selected_client(struct wl_client *client) {
	return paired && client && client == paired_client;
}
bool stage_director_news_watch_probe_request(struct wl_resource *resource) {
	return resource && stage_director_news_watch_selected_client(
		wl_resource_get_client(resource));
}
void stage_director_news_pairing_clear_host(void) {
	paired = false;
	stage_director_weather_watch_protocol_revoke();
}

static bool filter_globals(const struct wl_client *client,
		const struct wl_global *global, void *data) {
	(void)data;
	if (stage_director_weather_watch_protocol_is_global(global)) {
		return stage_director_news_watch_selected_client(
			(struct wl_client *)client);
	}
	return true;
}
static void *event_thread(void *ctx) {
	(void)ctx;
	struct wl_event_loop *loop = wl_display_get_event_loop(server);
	while (atomic_load(&running)) {
		assert(wl_event_loop_dispatch(loop, 10) >= 0);
		wl_display_flush_clients(server);
	}
	return NULL;
}
static struct wl_client *add_client(int *fd) {
	int pair[2] = {-1, -1};
	assert(socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == 0);
	struct wl_client *out = wl_client_create(server, pair[0]);
	assert(out);
	*fd = pair[1];
	return out;
}

struct test_client {
	struct wl_display *display;
	struct wl_registry *registry;
	struct post_apollo_weather_watch_v1 *watch;
	uint32_t global_name;
	unsigned count[4];
};

static void on_result(void *data,
		struct post_apollo_weather_watch_v1 *watch,
		uint32_t status) {
	struct test_client *c = data;
	(void)watch;
	assert(status < 4);
	++c->count[status];
}
static const struct post_apollo_weather_watch_v1_listener watch_listener = {
	.result = on_result,
};
static void on_global(void *data, struct wl_registry *reg,
		uint32_t name, const char *iface, uint32_t version) {
	struct test_client *c = data;
	if (strcmp(iface, "post_apollo_weather_watch_v1") != 0) {
		return;
	}
	assert(version >= 1);
	assert(!c->watch);
	c->global_name = name;
	c->watch = wl_registry_bind(reg, name,
		&post_apollo_weather_watch_v1_interface, 1);
	assert(c->watch);
	assert(post_apollo_weather_watch_v1_add_listener(
		c->watch, &watch_listener, c) == 0);
}
static void on_remove(void *data, struct wl_registry *reg,
		uint32_t name) {
	(void)data;
	(void)reg;
	(void)name;
}
static const struct wl_registry_listener reg_listener = {
	.global = on_global,
	.global_remove = on_remove,
};
static void connect_client(struct test_client *c, int fd) {
	c->display = wl_display_connect_to_fd(fd);
	assert(c->display);
	c->registry = wl_display_get_registry(c->display);
	assert(c->registry);
	assert(wl_registry_add_listener(c->registry, &reg_listener, c) == 0);
	assert(wl_display_roundtrip(c->display) >= 0);
	assert(wl_display_roundtrip(c->display) >= 0);
}
static void sync_client(struct test_client *c) {
	assert(wl_display_roundtrip(c->display) >= 0);
}
static void close_client(struct test_client *c) {
	if (c->watch) {
		post_apollo_weather_watch_v1_destroy(c->watch);
	}
	wl_registry_destroy(c->registry);
	wl_display_disconnect(c->display);
}

int main(void) {
	alarm(20);
	server = wl_display_create();
	assert(server);
	wl_display_set_global_filter(server, filter_globals, NULL);
	int chosen_fd = -1, foreign_fd = -1;
	paired_client = add_client(&chosen_fd);
	struct wl_client *foreign = add_client(&foreign_fd);
	assert(foreign != paired_client);
	paired = true;
	stage_director_weather_watch_protocol_init(server);
	assert(stage_director_weather_watch_protocol_publish());
	atomic_init(&running, true);
	pthread_t th;
	assert(pthread_create(&th, NULL, event_thread, NULL) == 0);

	struct test_client chosen = {0}, spoof = {0};
	connect_client(&chosen, chosen_fd);
	connect_client(&spoof, foreign_fd);
	assert(chosen.watch && chosen.global_name != 0);
	assert(!spoof.watch && spoof.global_name == 0);

	post_apollo_weather_watch_v1_watch(chosen.watch, "bbc");
	sync_client(&chosen);
	assert(chosen.count[1] == 1);
	post_apollo_weather_watch_v1_watch(chosen.watch,
		"https://untrusted.invalid/watch");
	sync_client(&chosen);
	assert(chosen.count[2] == 1);
	post_apollo_weather_watch_v1_watch(chosen.watch, "cspan");
	sync_client(&chosen);
	assert(chosen.count[1] == 2);

	/* Spoof tries a guessed global name despite not seeing the global.
	 * Either protocol-level rejection or bind-level rejection disconnects it.
	 */
	struct post_apollo_weather_watch_v1 *forged = wl_registry_bind(
		spoof.registry, chosen.global_name,
		&post_apollo_weather_watch_v1_interface, 1);
	assert(forged);
	assert(wl_display_roundtrip(spoof.display) < 0);

	post_apollo_weather_watch_v1_close(chosen.watch);
	sync_client(&chosen);
	assert(chosen.count[3] == 1);
	assert(!paired);
	/* Existing bound resources can no longer ask for a WATCH candidate. */
	post_apollo_weather_watch_v1_watch(chosen.watch, "bbc");
	sync_client(&chosen);
	assert(chosen.count[0] == 1);
	assert(chosen.count[1] == 2);

	close_client(&chosen);
	/* Spoof was already rejected; free locally without another request. */
	post_apollo_weather_watch_v1_destroy(forged);
	wl_registry_destroy(spoof.registry);
	wl_display_disconnect(spoof.display);
	atomic_store(&running, false);
	assert(pthread_join(th, NULL) == 0);
	stage_director_weather_watch_protocol_finish();
	wl_display_destroy_clients(server);
	wl_display_destroy(server);
	puts("SD_PRIVATE_WATCH_WIRE: dynamic paired-only global, foreign "
		"discovery/bind refused, real WATCH/CLOSE and stale request "
		"refused; no launch/scene/input authority: PASS");
	return 0;
}
