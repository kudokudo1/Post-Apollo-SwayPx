#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-server-core.h>
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

struct watch_binding {
	uint64_t pairing_epoch;
};

static bool request_epoch_current(struct wl_client *client,
		struct wl_resource *resource) {
	struct watch_binding *binding = wl_resource_get_user_data(resource);
	return binding && binding->pairing_epoch &&
		binding->pairing_epoch ==
			stage_director_news_watch_current_epoch(client);
}

static void destroy_binding(struct wl_resource *resource) {
	free(wl_resource_get_user_data(resource));
}

bool stage_director_weather_watch_protocol_is_global(
		const struct wl_global *global) {
	/*
	 * wl_global_create() synchronously advertises the new global BEFORE
	 * returning its pointer. The display filter is invoked inside that
	 * create call, when watch_global is still NULL. Comparing only the
	 * stored pointer leaks the private interface to existing registries.
	 * Match the generated Wayland protocol interface identity FIRST.
	 */
	return global &&
		wl_global_get_interface(global) ==
			&post_apollo_weather_watch_v1_interface;
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
			!request_epoch_current(client, resource) ||
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
	/* The first valid WATCH records one compositor-owned native candidate.
	 * A second WATCH (even on the same wl_client) is denied until CLOSE
	 * and a fresh trusted pairing epoch. The ledger never launches browser
	 * or grants scene, input, focus, popup or supervisor authority. */
	if (!stage_director_news_watch_record_candidate(resource, source_id)) {
		post_apollo_weather_watch_v1_send_result(
			resource, POST_APOLLO_WEATHER_WATCH_V1_STATUS_DENIED);
		return;
	}
	post_apollo_weather_watch_v1_send_result(
		resource, POST_APOLLO_WEATHER_WATCH_V1_STATUS_CANDIDATE_ONLY);
}

/* v2 only. A wl_resource for this client must already have been bound to
 * the exact physical-input-paired host. No URL, PID, renderer or input
 * permission crosses this request. v1 WATCH/CLOSE wire ABI stays unchanged. */
static void handle_report_viewport(struct wl_client *client,
		struct wl_resource *resource, int32_t x, int32_t y,
		int32_t width, int32_t height) {
	if (wl_resource_get_version(resource) < 2) return;
	struct stage_director_news_watch_viewport_ack ack = {
		.status = SD_NEWS_WATCH_VIEWPORT_DENIED,
	};
	if (client == wl_resource_get_client(resource) &&
			request_epoch_current(client, resource)) {
		ack = stage_director_news_watch_report_viewport(
			resource, x, y, width, height);
	}
	post_apollo_weather_watch_v1_send_viewport_result(resource,
		(uint32_t)ack.status, (uint32_t)(ack.host_revision >> 32),
		(uint32_t)ack.host_revision);
}

static void handle_close(struct wl_client *client,
		struct wl_resource *resource) {
	if (client != wl_resource_get_client(resource) ||
			!request_epoch_current(client, resource) ||
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
	.report_viewport = handle_report_viewport,
};

static void bind_watch(struct wl_client *client, void *data,
		uint32_t version, uint32_t id) {
	(void)data;
	if (version < 1 || version > 2 ||
			!stage_director_news_watch_selected_client(client)) {
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
	struct watch_binding *binding = calloc(1, sizeof(*binding));
	if (!binding) {
		wl_resource_destroy(resource);
		wl_client_post_no_memory(client);
		return;
	}
	binding->pairing_epoch =
		stage_director_news_watch_current_epoch(client);
	if (!binding->pairing_epoch) {
		free(binding);
		wl_resource_destroy(resource);
		wl_client_post_implementation_error(client,
			"Stage Director WATCH pairing expired during bind");
		return;
	}
	wl_resource_set_implementation(
		resource, &watch_impl, binding, destroy_binding);
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
		&post_apollo_weather_watch_v1_interface, 2, NULL, bind_watch);
	if (!watch_global) {
		return false;
	}
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
