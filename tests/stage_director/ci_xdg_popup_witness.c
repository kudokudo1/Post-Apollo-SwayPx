#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
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
#include "xdg-shell-client-protocol.h"

/* Real Wayland XDG toplevel + XDG popup, NOT browser/NEWS authorization.
 * The exact same native protocol emitter runs in two different cgroups:
 * a runner-owned control (popup should render), and a disposable Chrome
 * service descendant (CI compositor must refuse root popup creation).
 * Only GitHub's disposable headless runtime can invoke this witness. */
struct witness {
	struct wl_display *display;
	struct wl_compositor *compositor;
	struct wl_shm *shm;
	struct xdg_wm_base *wm;
	struct wl_surface *parent_surface, *popup_surface;
	struct xdg_surface *parent_xdg, *popup_xdg;
	struct xdg_toplevel *parent_top;
	struct xdg_popup *popup;
	struct wl_buffer *parent_buffer, *popup_buffer;
	bool parent_ready, popup_ready, popup_closed;
};
static volatile sig_atomic_t stopping;
static void stop_signal(int signal) {
	(void)signal;
	stopping = 1;
}
static void global(void *data, struct wl_registry *reg,
		uint32_t name, const char *interface, uint32_t version) {
	struct witness *s = data;
	if (!strcmp(interface, wl_compositor_interface.name)) {
		s->compositor = wl_registry_bind(reg, name,
			&wl_compositor_interface, version < 4 ? version : 4);
	} else if (!strcmp(interface, wl_shm_interface.name)) {
		s->shm = wl_registry_bind(reg, name, &wl_shm_interface, 1);
	} else if (!strcmp(interface, xdg_wm_base_interface.name)) {
		s->wm = wl_registry_bind(reg, name,
			&xdg_wm_base_interface, version < 3 ? version : 3);
	}
}
static void removed(void *data, struct wl_registry *reg, uint32_t name) {
	(void)data; (void)reg; (void)name;
}
static const struct wl_registry_listener registry_listener = {
	.global = global, .global_remove = removed,
};
static void ping(void *data, struct xdg_wm_base *wm, uint32_t serial) {
	(void)data;
	xdg_wm_base_pong(wm, serial);
}
static const struct xdg_wm_base_listener wm_listener = {.ping = ping};
static void toplevel_configure(void *data, struct xdg_toplevel *top,
		int32_t width, int32_t height, struct wl_array *states) {
	(void)data; (void)top; (void)width; (void)height; (void)states;
}
static void toplevel_close(void *data, struct xdg_toplevel *top) {
	(void)top;
	((struct witness *)data)->popup_closed = true;
}
static const struct xdg_toplevel_listener top_listener = {
	.configure = toplevel_configure, .close = toplevel_close,
};
static struct wl_buffer *solid(struct witness *s, uint32_t w,
		uint32_t h, uint32_t color) {
	if (w == 0 || h == 0 || w > 200 || h > 150) return NULL;
	const char *runtime = getenv("XDG_RUNTIME_DIR");
	if (!runtime || strlen(runtime) > PATH_MAX - 48) return NULL;
	char filename[PATH_MAX];
	if (snprintf(filename, sizeof(filename),
			"%s/sd-ci-xdg-popup-shm-XXXXXX", runtime) <= 0) return NULL;
	int fd = mkstemp(filename);
	if (fd < 0) return NULL;
	(void)unlink(filename);
	size_t length = (size_t)w * h * 4;
	if (ftruncate(fd, (off_t)length) < 0) {
		close(fd); return NULL;
	}
	uint32_t *pix = mmap(NULL, length, PROT_READ | PROT_WRITE,
		MAP_SHARED, fd, 0);
	if (pix == MAP_FAILED) {
		close(fd); return NULL;
	}
	for (size_t i = 0; i < length / 4; ++i) pix[i] = color;
	munmap(pix, length);
	struct wl_shm_pool *pool = wl_shm_create_pool(s->shm, fd, (int32_t)length);
	struct wl_buffer *buffer = pool ?
		wl_shm_pool_create_buffer(pool, 0, (int32_t)w,
			(int32_t)h, (int32_t)(w * 4), WL_SHM_FORMAT_ARGB8888) : NULL;
	if (pool) wl_shm_pool_destroy(pool);
	close(fd);
	return buffer;
}
static void parent_configured(void *data, struct xdg_surface *xdg,
		uint32_t serial) {
	struct witness *s = data;
	xdg_surface_ack_configure(xdg, serial);
	if (s->parent_ready) return;
	s->parent_buffer = solid(s, 24, 24, 0xff000000);
	if (!s->parent_buffer) return;
	wl_surface_attach(s->parent_surface, s->parent_buffer, 0, 0);
	wl_surface_damage(s->parent_surface, 0, 0, 24, 24);
	wl_surface_commit(s->parent_surface);
	s->parent_ready = true;
}
static void popup_configured(void *data, struct xdg_surface *xdg,
		uint32_t serial) {
	struct witness *s = data;
	xdg_surface_ack_configure(xdg, serial);
	if (s->popup_ready) return;
	/* Exact visible RGB #DD44DD, not used elsewhere in the test scene. */
	s->popup_buffer = solid(s, 160, 100, 0xffdd44dd);
	if (!s->popup_buffer) return;
	wl_surface_attach(s->popup_surface, s->popup_buffer, 0, 0);
	wl_surface_damage(s->popup_surface, 0, 0, 160, 100);
	wl_surface_commit(s->popup_surface);
	s->popup_ready = true;
	printf("SD_CI_NATIVE_XDG_POPUP_BUFFER_ATTACHED mapped=1 rgb=DD44DD\n");
	fflush(stdout);
}
static const struct xdg_surface_listener parent_listener = {
	.configure = parent_configured,
};
static const struct xdg_surface_listener popup_surface_listener = {
	.configure = popup_configured,
};
static void popup_configure(void *data, struct xdg_popup *popup,
		int32_t x, int32_t y, int32_t width, int32_t height) {
	(void)data; (void)popup;
	printf("SD_CI_NATIVE_XDG_POPUP_CONFIGURE x=%d y=%d width=%d height=%d\n",
		x, y, width, height);
	fflush(stdout);
}
static void popup_done(void *data, struct xdg_popup *popup) {
	(void)popup;
	((struct witness *)data)->popup_closed = true;
}
static void popup_repositioned(void *data, struct xdg_popup *popup,
		uint32_t token) {
	(void)data; (void)popup; (void)token;
}
static const struct xdg_popup_listener popup_listener = {
	.configure = popup_configure, .popup_done = popup_done,
	.repositioned = popup_repositioned,
};
int main(int argc, char **argv) {
	const char *runtime = getenv("XDG_RUNTIME_DIR");
	const char *temp = getenv("RUNNER_TEMP");
	if (argc != 2 || strcmp(argv[1], "emit") != 0 ||
			!runtime || !temp || !*temp ||
			strncmp(runtime, temp, strlen(temp)) ||
			runtime[strlen(temp)] != '/' ||
			!getenv("GITHUB_ACTIONS") ||
			strcmp(getenv("GITHUB_ACTIONS"), "true") ||
			!getenv("CI") || strcmp(getenv("CI"), "true")) {
		fputs("BLOCKED_XDG_POPUP_NOT_ISOLATED_GITHUB_RUNNER\n", stderr);
		return 2;
	}
	alarm(30);
	struct witness s = {0};
	s.display = wl_display_connect(NULL);
	if (!s.display) return 1;
	struct wl_registry *registry = wl_display_get_registry(s.display);
	if (!registry || wl_registry_add_listener(registry, &registry_listener, &s) ||
			wl_display_roundtrip(s.display) < 0 || !s.compositor || !s.shm || !s.wm) {
		fputs("FAIL_XDG_POPUP_REGISTRY\n", stderr); return 1;
	}
	xdg_wm_base_add_listener(s.wm, &wm_listener, &s);
	s.parent_surface = wl_compositor_create_surface(s.compositor);
	s.parent_xdg = xdg_wm_base_get_xdg_surface(s.wm, s.parent_surface);
	s.parent_top = xdg_surface_get_toplevel(s.parent_xdg);
	xdg_surface_add_listener(s.parent_xdg, &parent_listener, &s);
	xdg_toplevel_add_listener(s.parent_top, &top_listener, &s);
	xdg_toplevel_set_title(s.parent_top, "SD-CI-POPUP-WITNESS");
	xdg_toplevel_set_app_id(s.parent_top, "sd-ci-popup-witness");
	xdg_toplevel_set_min_size(s.parent_top, 24, 24);
	xdg_toplevel_set_max_size(s.parent_top, 24, 24);
	wl_surface_commit(s.parent_surface);
	for (int i = 0; i < 20 && !s.parent_ready; ++i) {
		if (wl_display_roundtrip(s.display) < 0) return 1;
	}
	if (!s.parent_ready) {fputs("FAIL_XDG_POPUP_PARENT_UNMAPPED\n", stderr); return 1;}
	printf("SD_CI_NATIVE_XDG_POPUP_PARENT_READY pid=%ld native=1\n",
		(long)getpid());
	fflush(stdout);
	struct xdg_positioner *positioner = xdg_wm_base_create_positioner(s.wm);
	xdg_positioner_set_size(positioner, 160, 100);
	xdg_positioner_set_anchor_rect(positioner, 0, 0, 24, 24);
	xdg_positioner_set_anchor(positioner, XDG_POSITIONER_ANCHOR_TOP_LEFT);
	xdg_positioner_set_gravity(positioner, XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT);
	s.popup_surface = wl_compositor_create_surface(s.compositor);
	s.popup_xdg = xdg_wm_base_get_xdg_surface(s.wm, s.popup_surface);
	s.popup = xdg_surface_get_popup(s.popup_xdg, s.parent_xdg, positioner);
	xdg_positioner_destroy(positioner);
	xdg_surface_add_listener(s.popup_xdg, &popup_surface_listener, &s);
	xdg_popup_add_listener(s.popup, &popup_listener, &s);
	wl_surface_commit(s.popup_surface);
	/* Real xdg_popup request is flushed to the compositor before reporting. */
	if (wl_display_flush(s.display) < 0) {
		fputs("FAIL_XDG_POPUP_REQUEST_FLUSH\n", stderr); return 1;
	}
	printf("SD_CI_NATIVE_XDG_POPUP_REQUEST_SENT native_protocol=1 pid=%ld\n",
		(long)getpid());
	fflush(stdout);
	struct sigaction sa = {0};
	sa.sa_handler = stop_signal;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGTERM, &sa, NULL);
	for (int i = 0; i < 150 && !stopping && !s.popup_closed; ++i) {
		struct pollfd fd = {.fd = wl_display_get_fd(s.display),
			.events = POLLIN};
		int status = poll(&fd, 1, 100);
		if (status > 0 && (fd.revents & POLLIN)) {
			if (wl_display_dispatch(s.display) < 0) break;
		} else if (wl_display_dispatch_pending(s.display) < 0) {
			break;
		}
	}
	wl_display_disconnect(s.display);
	return 0;
}
