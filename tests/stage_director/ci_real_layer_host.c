#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <wayland-client.h>
#include "wlr-layer-shell-unstable-v1-client-protocol.h"

/*
 * Disposable Wayland layer-shell host used ONLY on an isolated CI runner.
 * Generated client protocol comes from this repository's own pinned XML.
 * No Qt, browser, URLs, Quickshell IPC, authenticated WATCH or session lease.
 */
struct layer_test {
	struct wl_display *display;
	struct wl_compositor *compositor;
	struct wl_shm *shm;
	struct zwlr_layer_shell_v1 *layer_shell;
	struct wl_surface *surface;
	struct zwlr_layer_surface_v1 *layer;
	struct wl_buffer *buffer;
	bool mapped;
	bool closed;
};

static void registry_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version) {
	struct layer_test *s = data;
	if (strcmp(interface, wl_compositor_interface.name) == 0) {
		s->compositor = wl_registry_bind(registry, name,
			&wl_compositor_interface, version < 4 ? version : 4);
	} else if (strcmp(interface, wl_shm_interface.name) == 0) {
		s->shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
	} else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
		s->layer_shell = wl_registry_bind(registry, name,
			&zwlr_layer_shell_v1_interface, 1);
	}
}

static void registry_global_remove(void *data, struct wl_registry *registry,
		uint32_t name) {
	(void)data;
	(void)registry;
	(void)name;
}

static const struct wl_registry_listener registry_listener = {
	.global = registry_global,
	.global_remove = registry_global_remove,
};

static struct wl_buffer *create_buffer(struct layer_test *s,
		uint32_t width, uint32_t height) {
	if (!s->shm || width == 0 || height == 0 || width > 600 || height > 600) {
		return NULL;
	}
	const uint32_t stride = width * 4;
	const size_t length = (size_t)stride * height;
	const char *runtime = getenv("XDG_RUNTIME_DIR");
	if (!runtime || !*runtime || strlen(runtime) > PATH_MAX - 50) {
		return NULL;
	}
	char path[PATH_MAX];
	if (snprintf(path, sizeof(path), "%s/sd1-layer-shm-XXXXXX",
			runtime) <= 0) return NULL;
	int fd = mkstemp(path);
	if (fd < 0) return NULL;
	(void)unlink(path);
	if (ftruncate(fd, (off_t)length) != 0) {
		(void)close(fd);
		return NULL;
	}
	uint32_t *pixels = mmap(NULL, length, PROT_READ | PROT_WRITE,
		MAP_SHARED, fd, 0);
	if (pixels == MAP_FAILED) {
		(void)close(fd);
		return NULL;
	}
	for (size_t i = 0; i < length / 4; ++i) {
		pixels[i] = 0xff55cfca; /* fixed cyan CI-only rectangle */
	}
	(void)munmap(pixels, length);
	struct wl_shm_pool *pool = wl_shm_create_pool(s->shm, fd,
		(int32_t)length);
	struct wl_buffer *buffer = pool
		? wl_shm_pool_create_buffer(pool, 0, (int32_t)width,
			(int32_t)height, (int32_t)stride, WL_SHM_FORMAT_ARGB8888)
		: NULL;
	if (pool) wl_shm_pool_destroy(pool);
	(void)close(fd);
	return buffer;
}

static void layer_configure(void *data, struct zwlr_layer_surface_v1 *layer,
		uint32_t serial, uint32_t width, uint32_t height) {
	struct layer_test *s = data;
	zwlr_layer_surface_v1_ack_configure(layer, serial);
	if (s->mapped) return;
	/* No wl_surface buffer was attached before first configure. */
	s->buffer = create_buffer(s, width, height);
	if (!s->buffer) {
		fprintf(stderr, "FAIL: CI layer buffer allocation/configuration\n");
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
	((struct layer_test *)data)->closed = true;
}

static const struct zwlr_layer_surface_v1_listener layer_listener = {
	.configure = layer_configure,
	.closed = layer_closed,
};
static volatile sig_atomic_t stopping;
static void stop_handler(int signo) {
	(void)signo;
	stopping = 1;
}

int main(int argc, char **argv) {
	const char *runtime = getenv("XDG_RUNTIME_DIR");
	const char *ci = getenv("CI");
	const char *actions = getenv("GITHUB_ACTIONS");
	const char *temp = getenv("RUNNER_TEMP");
	if (argc != 2 || !runtime || !temp || !*temp ||
			strncmp(runtime, temp, strlen(temp)) != 0 ||
			runtime[strlen(temp)] != '/' ||
			!ci || strcmp(ci, "true") != 0 ||
			!actions || strcmp(actions, "true") != 0) {
		fputs("BLOCKED_NOT_DISPOSABLE_CI_LAYER_HOST\n", stderr);
		return 2;
	}
	const char *ns = NULL;
	if (strcmp(argv[1], "weather") == 0 ||
			strcmp(argv[1], "spoof") == 0) {
		ns = "post-apollo-weather-station";
	} else if (strcmp(argv[1], "other") == 0) {
		ns = "post-apollo-different-layer";
	} else {
		fputs("BLOCKED_UNKNOWN_CI_LAYER_ROLE\n", stderr);
		return 2;
	}
	struct layer_test s = {0};
	s.display = wl_display_connect(NULL);
	if (!s.display) {
		fputs("FAIL: disposable Wayland compositor not reachable\n", stderr);
		return 1;
	}
	struct wl_registry *registry = wl_display_get_registry(s.display);
	if (!registry ||
			wl_registry_add_listener(registry, &registry_listener, &s) != 0 ||
			wl_display_roundtrip(s.display) < 0 ||
			!s.compositor || !s.shm || !s.layer_shell) {
		fputs("FAIL: compositor missing native layer-shell or SHM\n", stderr);
		return 1;
	}
	s.surface = wl_compositor_create_surface(s.compositor);
	if (!s.surface) return 1;
	s.layer = zwlr_layer_shell_v1_get_layer_surface(s.layer_shell,
		s.surface, NULL, ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY, ns);
	if (!s.layer) return 1;
	zwlr_layer_surface_v1_add_listener(s.layer, &layer_listener, &s);
	zwlr_layer_surface_v1_set_size(s.layer, 320, 240);
	zwlr_layer_surface_v1_set_anchor(s.layer,
		ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
		ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT);
	zwlr_layer_surface_v1_set_exclusive_zone(s.layer, 0);
	zwlr_layer_surface_v1_set_keyboard_interactivity(s.layer, 0);
	/* First commit must have NO attached buffer per layer-shell protocol. */
	wl_surface_commit(s.surface);
	for (int i = 0; i < 15 && !s.mapped && !s.closed; ++i) {
		if (wl_display_roundtrip(s.display) < 0) break;
	}
	if (!s.mapped || s.closed) {
		fputs("FAIL: disposable native layer did not map\n", stderr);
		return 1;
	}
	printf("SD_CI_LAYER_HOST_READY role=%s pid=%ld namespace=%s mapped=1 "
		"authority=NONE\n", argv[1], (long)getpid(), ns);
	fflush(stdout);
	struct sigaction sa = {0};
	sa.sa_handler = stop_handler;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	/* A CI shell owns this exact process and terminates it after inspection. */
	for (int i = 0; i < 150 && !stopping && !s.closed; ++i) {
		if (wl_display_flush(s.display) < 0) break;
		struct timespec pause = { .tv_sec = 0, .tv_nsec = 100000000L };
		nanosleep(&pause, NULL);
	}
	/* wl_display_disconnect destroys the connection's mapped surface. */
	wl_display_disconnect(s.display);
	return 0;
}
