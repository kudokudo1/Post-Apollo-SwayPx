#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <wayland-client.h>
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "post-apollo-weather-watch-v1-client-protocol.h"

/* Real layer-shell AND actual production WATCH protocol client on the
 * SAME wl_display/wl_client, unlike an unrelated native helper socket.
 * CI-only code; no QML, browser, process launch, scene or seat privilege. */
struct test_state {
	struct wl_display *display;
	struct wl_compositor *compositor;
	struct wl_shm *shm;
	struct zwlr_layer_shell_v1 *shell;
	struct wl_surface *surface;
	struct zwlr_layer_surface_v1 *layer;
	struct wl_buffer *buffer;
	struct post_apollo_weather_watch_v1 *watch;
	unsigned results[4];
	bool mapped;
	bool closed;
	bool watch_seen;
	bool watch_mode;
	bool spoof_mode;
};
static void on_result(void *data, struct post_apollo_weather_watch_v1 *watch,
		uint32_t status) {
	(void)watch;
	struct test_state *s = data;
	if (status < 4) ++s->results[status];
}
static const struct post_apollo_weather_watch_v1_listener watch_listener = {
	.result = on_result,
};
static void on_global(void *data, struct wl_registry *reg,
		uint32_t name, const char *iface, uint32_t version) {
	struct test_state *s = data;
	if (!strcmp(iface, wl_compositor_interface.name)) {
		s->compositor = wl_registry_bind(reg, name,
			&wl_compositor_interface, version < 4 ? version : 4);
	} else if (!strcmp(iface, wl_shm_interface.name)) {
		s->shm = wl_registry_bind(reg, name, &wl_shm_interface, 1);
	} else if (!strcmp(iface, zwlr_layer_shell_v1_interface.name)) {
		s->shell = wl_registry_bind(reg, name, &zwlr_layer_shell_v1_interface, 1);
	} else if (!strcmp(iface, post_apollo_weather_watch_v1_interface.name)) {
		s->watch_seen = true;
		if (s->watch_mode && !s->watch) {
			s->watch = wl_registry_bind(reg, name,
				&post_apollo_weather_watch_v1_interface, 1);
			assert(s->watch);
			assert(post_apollo_weather_watch_v1_add_listener(s->watch,
				&watch_listener, s) == 0);
		}
	}
}
static void on_remove(void *data, struct wl_registry *reg, uint32_t name) {
	(void)data;
	(void)reg;
	(void)name;
}
static const struct wl_registry_listener registry_listener = {
	.global = on_global, .global_remove = on_remove,
};

static struct wl_buffer *solid_buffer(struct test_state *s,
		uint32_t width, uint32_t height) {
	if (!s->shm || !width || !height || width > 600 || height > 600) return NULL;
	const uint32_t stride = width * 4;
	const size_t size = (size_t)stride * height;
	const char *runtime = getenv("XDG_RUNTIME_DIR");
	if (!runtime || strlen(runtime) > PATH_MAX - 48) return NULL;
	char name[PATH_MAX];
	if (snprintf(name, sizeof(name), "%s/watch-shm-XXXXXX", runtime) <= 0)
		return NULL;
	int fd = mkstemp(name);
	if (fd < 0) return NULL;
	(void)unlink(name);
	if (ftruncate(fd, (off_t)size) != 0) {
		(void)close(fd);
		return NULL;
	}
	uint32_t *pixels = mmap(NULL, size, PROT_READ | PROT_WRITE,
		MAP_SHARED, fd, 0);
	if (pixels == MAP_FAILED) {
		(void)close(fd);
		return NULL;
	}
	for (size_t i = 0; i < size / 4; ++i)
		pixels[i] = 0xff55cfca;
	(void)munmap(pixels, size);
	struct wl_shm_pool *pool = wl_shm_create_pool(s->shm, fd, (int32_t)size);
	struct wl_buffer *out = pool ? wl_shm_pool_create_buffer(pool, 0,
		(int32_t)width, (int32_t)height, (int32_t)stride,
		WL_SHM_FORMAT_ARGB8888) : NULL;
	if (pool) wl_shm_pool_destroy(pool);
	(void)close(fd);
	return out;
}
static void configure(void *data, struct zwlr_layer_surface_v1 *layer,
		uint32_t serial, uint32_t width, uint32_t height) {
	struct test_state *s = data;
	zwlr_layer_surface_v1_ack_configure(layer, serial);
	if (s->mapped) return;
	s->buffer = solid_buffer(s, width, height);
	if (!s->buffer) {
		s->closed = true;
		return;
	}
	wl_surface_attach(s->surface, s->buffer, 0, 0);
	wl_surface_damage(s->surface, 0, 0, (int32_t)width, (int32_t)height);
	wl_surface_commit(s->surface);
	s->mapped = true;
}
static void layer_closed(void *data, struct zwlr_layer_surface_v1 *layer) {
	(void)layer;
	((struct test_state *)data)->closed = true;
}
static const struct zwlr_layer_surface_v1_listener layer_listener = {
	.configure = configure, .closed = layer_closed,
};

int main(int argc, char **argv) {
	alarm(25);
	const char *runtime = getenv("XDG_RUNTIME_DIR");
	const char *temp = getenv("RUNNER_TEMP");
	const char *ci = getenv("CI");
	const char *gha = getenv("GITHUB_ACTIONS");
	if (argc != 2 || !runtime || !temp || !*temp ||
			strncmp(runtime, temp, strlen(temp)) ||
			runtime[strlen(temp)] != '/' ||
			!ci || strcmp(ci, "true") || !gha || strcmp(gha, "true")) {
		fputs("BLOCKED_NOT_DISPOSABLE_HEADLESS_WATCH_CLIENT\n", stderr);
		return 2;
	}
	struct test_state s = {0};
	const char *ns = "post-apollo-weather-station";
	s.watch_mode = !strcmp(argv[1], "weather-watch");
	s.spoof_mode = !strcmp(argv[1], "spoof");
	const bool other = !strcmp(argv[1], "other");
	if (!s.watch_mode && !s.spoof_mode && !other) {
		fputs("BLOCKED_UNKNOWN_CI_ROLE\n", stderr);
		return 2;
	}
	if (other) ns = "post-apollo-different-layer";
	s.display = wl_display_connect(NULL);
	if (!s.display) return 1;
	struct wl_registry *reg = wl_display_get_registry(s.display);
	if (!reg || wl_registry_add_listener(reg, &registry_listener, &s) ||
			wl_display_roundtrip(s.display) < 0 ||
			!s.compositor || !s.shm || !s.shell) {
		fputs("FAIL: no layer shell or compositor on isolated Wayland display\n",
			stderr);
		return 1;
	}
	/* Only the real Weather host is allowed to see the WATCH global.
	 * Other clients must have no access even if the namespace matches. */
	if (s.spoof_mode || other) {
		if (s.watch_seen) {
			fputs("FAIL: unpaired test client discovered private WATCH\n", stderr);
			return 1;
		}
	}
	s.surface = wl_compositor_create_surface(s.compositor);
	s.layer = zwlr_layer_shell_v1_get_layer_surface(s.shell, s.surface,
		NULL, ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY, ns);
	assert(s.layer && s.surface);
	zwlr_layer_surface_v1_add_listener(s.layer, &layer_listener, &s);
	zwlr_layer_surface_v1_set_size(s.layer, 320, 240);
	zwlr_layer_surface_v1_set_anchor(s.layer,
		ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
		(s.spoof_mode
			? ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT
			: ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT));
	zwlr_layer_surface_v1_set_exclusive_zone(s.layer, 0);
	zwlr_layer_surface_v1_set_keyboard_interactivity(s.layer, 0);
	wl_surface_commit(s.surface);
	for (int i = 0; i < 25 && !s.mapped && !s.closed; ++i) {
		if (wl_display_roundtrip(s.display) < 0) return 1;
	}
	if (!s.mapped || s.closed) {
		fputs("FAIL: real layer shell never mapped\n", stderr);
		return 1;
	}
	printf("SD_CI_REAL_WATCH_LAYER_MAPPED role=%s mapped=1 namespace=%s\n",
		argv[1], ns);
	fflush(stdout);

	if (s.watch_mode) {
		/* A newly published global must reach the already-live registry
		 * after the actual layer maps, on this exact SAME connection. */
		for (int i = 0; i < 80 && !s.watch; ++i) {
			if (wl_display_roundtrip(s.display) < 0) return 1;
			struct timespec delay = {.tv_sec=0, .tv_nsec=30000000L};
			nanosleep(&delay, NULL);
		}
		if (!s.watch) {
			fputs("FAIL: mapped Weather host did not receive private global\n",
				stderr);
			return 1;
		}
		post_apollo_weather_watch_v1_watch(s.watch, "bbc");
		if (wl_display_roundtrip(s.display) < 0 || s.results[1] != 1)
			return 1;
		post_apollo_weather_watch_v1_watch(s.watch,
			"https://example.invalid/anything");
		if (wl_display_roundtrip(s.display) < 0 || s.results[2] != 1)
			return 1;
		post_apollo_weather_watch_v1_close(s.watch);
		if (wl_display_roundtrip(s.display) < 0 || s.results[3] != 1)
			return 1;
		post_apollo_weather_watch_v1_watch(s.watch, "bbc");
		if (wl_display_roundtrip(s.display) < 0 || s.results[0] != 1)
			return 1;
		printf("SD_CI_REAL_WATCH_END_TO_END selected=1 "
			"known_source_candidate=1 invalid_url_refused=1 "
			"close_revoked=1 stale_watch_denied=1 authority=NONE PASS\n");
		fflush(stdout);
	} else {
		/* Keep the unpaired client alive across another real layer's
		 * map/pairing and CLOSE, so a global broadcast would be detected. */
		for (int i = 0; i < 45; ++i) {
			if (wl_display_roundtrip(s.display) < 0) return 1;
			if (s.watch_seen) {
				fputs("FAIL: unpaired client saw WATCH global\n", stderr);
				return 1;
			}
			struct timespec delay = {.tv_sec=0, .tv_nsec=40000000L};
			nanosleep(&delay, NULL);
		}
		printf("SD_CI_REAL_WATCH_UNPAIRED role=%s discovered=0 "
			"same_namespace_spoof=%d PASS\n", argv[1], s.spoof_mode ? 1 : 0);
		fflush(stdout);
	}
	wl_display_disconnect(s.display);
	return 0;
}
