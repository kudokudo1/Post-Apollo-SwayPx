#include <stdbool.h>
#include <string.h>
#include <wayland-server-core.h>
#include "log.h"
#include "sway/stage_director.h"
#include "sway/stage_director_weather_watch_protocol.h"
#include "post-apollo-weather-watch-v1-protocol.h"

/*
 * One dynamic global per compositor. It does not exist before selection.
 * Global filtering prevents unrelated clients from seeing the new global;
 * bind and EVERY request check the selected client again. Global publication
 * is not sufficient by itself as a security boundary.
 */
static struct wl_display *watch_display;
static struct wl_global *watch_global;

bool stage_director_weather_watch_protocol_is_global(
		const struct wl_global *global) {
	return global && global == watch_global;
}

static bool known_source(const char *id) {
	/* Small internal source tokens for initial bridge contract. These are not
	 * actual URLs or a production Weather catalog registry. No playback yet. */
	return id && (strcmp(id, "bbc") == 0 ||
		strcmp(id, "cspan") == 0);
}

static void handle_watch(struct wl_client *client,
		struct wl_resource *resource, const char *source_id) {
	if (client != wl_resource_get_client(resource) ||
			!stage_director_news_watch_probe_request(resource)) {
		post_apollo_weather_watch_v1_send_result(
			resource, POST_APOLLO_WEATHER_WATCH_V1_STATUS_DENIED);
		return;
	}
	if (!known_source(source_id)) {
		post_apollo_weather_watch_v1_send_result(
			resource, POST_APOLLO_WEATHER_WATCH_V1_STATUS_INVALID_SOURCE);
		return;
	}
	/* Intentionally no browser supervisor, lease, scene or seat call. */
	post_apollo_weather_watch_v1_send_result(
		resource, POST_APOLLO_WEATHER_WATCH_V1_STATUS_CANDIDATE_ONLY);
}

static void handle_close(struct wl_client *client,
		struct wl_resource *resource) {
	if (client != wl_resource_get_client(resource) ||
			!stage_director_news_watch_probe_request(resource)) {
		post_apollo_weather_watch_v1_send_result(
			resource, POST_APOLLO_WEATHER_WATCH_V1_STATUS_DENIED);
		return;
	}
	post_apollo_weather_watch_v1_send_result(
		resource, POST_APOLLO_WEATHER_WATCH_V1_STATUS_CLOSED);
	stage_director_news_pairing_clear_host();
}

static void handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	(void)client;
	wl_resource_destroy(resource);
}

static const struct post_apollo_weather_watch_v1_interface watch_impl = {
	.watch = handle_watch,
	.close = handle_close,
	.destroy = handle_destroy,
};

static void bind_watch(struct wl_client *client, void *data,
		uint32_t version, uint32_t id) {
	(void)data;
	if (version != 1 || !stage_director_news_watch_selected_client(client)) {
		/* A forged bind is a Wayland protocol violation. No resource or
		 * WATCH authority must ever be given to an unpaired client. */
		wl_client_post_implementation_error(client,
			"Stage Director WATCH bind requires paired Weather client");
		return;
	}
	struct wl_resource *resource = wl_resource_create(client,
		&post_apollo_weather_watch_v1_interface, version, id);
	if (!resource) {
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(resource, &watch_impl, NULL, NULL);
}

void stage_director_weather_watch_protocol_init(struct wl_display *display) {
	/* Called once during server initialization, not from external IPC. */
	if (!watch_display) {
		watch_display = display;
	}
}

bool stage_director_weather_watch_protocol_publish(void) {
	if (!watch_display || watch_global) {
		return false;
	}
	watch_global = wl_global_create(watch_display,
		&post_apollo_weather_watch_v1_interface, 1, NULL, bind_watch);
	if (!watch_global) {
		return false;
	}
	sway_log(SWAY_INFO, "[stage-director][SD1][WATCH] private paired "
		"global published; all playback/scene/input authority denied");
	return true;
}

void stage_director_weather_watch_protocol_revoke(void) {
	if (watch_global) {
		wl_global_destroy(watch_global);
		watch_global = NULL;
	}
}

void stage_director_weather_watch_protocol_finish(void) {
	stage_director_weather_watch_protocol_revoke();
	watch_display = NULL;
}
