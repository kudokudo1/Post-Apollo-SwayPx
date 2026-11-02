#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#include "virtual-keyboard-unstable-v1-client-protocol.h"
#include "wlr-virtual-pointer-unstable-v1-client-protocol.h"

/* CI-only actual Wayland virtual input protocol emitter. No compositor
 * instrumentation, injection into a workstation, or shell command calls.
 * The Python harness decides the target using exact Sway IPC geometry. */
struct context {
	struct wl_display *display;
	struct wl_seat *seat;
	struct zwp_virtual_keyboard_manager_v1 *keyboards;
	struct zwlr_virtual_pointer_manager_v1 *pointers;
};
static void global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version) {
	struct context *ctx = data;
	if (strcmp(interface, wl_seat_interface.name) == 0) {
		ctx->seat = wl_registry_bind(registry, name, &wl_seat_interface,
			version < 4 ? version : 4);
	} else if (strcmp(interface, zwp_virtual_keyboard_manager_v1_interface.name) == 0) {
		ctx->keyboards = wl_registry_bind(registry, name,
			&zwp_virtual_keyboard_manager_v1_interface, 1);
	} else if (strcmp(interface, zwlr_virtual_pointer_manager_v1_interface.name) == 0) {
		ctx->pointers = wl_registry_bind(registry, name,
			&zwlr_virtual_pointer_manager_v1_interface, 1);
	}
}
static void global_remove(void *data, struct wl_registry *r, uint32_t name) {
	(void)data; (void)r; (void)name;
}
static const struct wl_registry_listener listeners = {
	.global = global, .global_remove = global_remove,
};
static uint32_t now_msec(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint32_t)(ts.tv_sec * 1000ull + ts.tv_nsec / 1000000ull);
}
static void pause_dispatch(struct context *ctx) {
	if (wl_display_roundtrip(ctx->display) < 0) {
		fprintf(stderr, "FAIL_SD_CI_VIRTUAL_INPUT_ROUNDTRIP\n");
		exit(7);
	}
	struct timespec delay = {.tv_sec = 0, .tv_nsec = 130000000};
	nanosleep(&delay, NULL);
}
static bool actual_keyboard(struct context *ctx) {
	struct xkb_context *xc = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	if (!xc) return false;
	struct xkb_keymap *km = xkb_keymap_new_from_names(xc, NULL,
		XKB_KEYMAP_COMPILE_NO_FLAGS);
	if (!km) { xkb_context_unref(xc); return false; }
	char *txt = xkb_keymap_get_as_string(km, XKB_KEYMAP_FORMAT_TEXT_V1);
	if (!txt) {
		xkb_keymap_unref(km); xkb_context_unref(xc); return false;
	}
	size_t len = strlen(txt) + 1;
	if (len > UINT32_MAX) {
		free(txt); xkb_keymap_unref(km); xkb_context_unref(xc); return false;
	}
	int fd = memfd_create("sd-ci-native-virtual-keymap", MFD_CLOEXEC);
	if (fd < 0 || write(fd, txt, len) != (ssize_t)len) {
		if (fd >= 0) close(fd);
		free(txt); xkb_keymap_unref(km); xkb_context_unref(xc); return false;
	}
	struct zwp_virtual_keyboard_v1 *kb =
		zwp_virtual_keyboard_manager_v1_create_virtual_keyboard(
			ctx->keyboards, ctx->seat);
	if (!kb) {
		close(fd); free(txt); xkb_keymap_unref(km);
		xkb_context_unref(xc); return false;
	}
	zwp_virtual_keyboard_v1_keymap(kb, WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1,
		fd, (uint32_t)len);
	close(fd);
	pause_dispatch(ctx);
	/* KEY_A evdev code 30. No dangerous command hotkey. */
	zwp_virtual_keyboard_v1_key(kb, now_msec(), KEY_A,
		WL_KEYBOARD_KEY_STATE_PRESSED);
	pause_dispatch(ctx);
	zwp_virtual_keyboard_v1_key(kb, now_msec(), KEY_A,
		WL_KEYBOARD_KEY_STATE_RELEASED);
	pause_dispatch(ctx);
	zwp_virtual_keyboard_v1_destroy(kb);
	free(txt);
	xkb_keymap_unref(km);
	xkb_context_unref(xc);
	printf("SD_CI_VIRTUAL_INPUT_SENT kind=KEYBOARD key=KEY_A press=1 release=1 "
		"actual_wayland_protocol=1 authority=NONE\n");
	return true;
}
static bool actual_pointer(struct context *ctx, uint32_t x, uint32_t y) {
	if (x >= 1920 || y >= 1080) return false;
	struct zwlr_virtual_pointer_v1 *ptr =
		zwlr_virtual_pointer_manager_v1_create_virtual_pointer(
			ctx->pointers, ctx->seat);
	if (!ptr) return false;
	pause_dispatch(ctx);
	zwlr_virtual_pointer_v1_motion_absolute(ptr, now_msec(),
		x, y, 1920, 1080);
	zwlr_virtual_pointer_v1_frame(ptr);
	pause_dispatch(ctx);
	zwlr_virtual_pointer_v1_destroy(ptr);
	printf("SD_CI_VIRTUAL_INPUT_SENT kind=POINTER x=%u y=%u "
		"actual_wayland_protocol=1 authority=NONE\n", x, y);
	return true;
}
/* Real virtual pointer button events. A drag keeps the same virtual
 * pointer alive from press through motion and release so wlroots may use
 * its normal implicit pointer grab, not a sequence of fake pointer hits. */
static bool actual_pointer_button(struct context *ctx, uint32_t x,
		uint32_t y, bool drag, uint32_t dest_x, uint32_t dest_y) {
	if (x >= 1920 || y >= 1080 || (drag && (dest_x >= 1920 ||
			dest_y >= 1080))) return false;
	struct zwlr_virtual_pointer_v1 *ptr =
		zwlr_virtual_pointer_manager_v1_create_virtual_pointer(
			ctx->pointers, ctx->seat);
	if (!ptr) return false;
	pause_dispatch(ctx);
	zwlr_virtual_pointer_v1_motion_absolute(ptr, now_msec(),
		x, y, 1920, 1080);
	zwlr_virtual_pointer_v1_frame(ptr);
	pause_dispatch(ctx);
	zwlr_virtual_pointer_v1_button(ptr, now_msec(), BTN_LEFT,
		WL_POINTER_BUTTON_STATE_PRESSED);
	zwlr_virtual_pointer_v1_frame(ptr);
	pause_dispatch(ctx);
	if (drag) {
		zwlr_virtual_pointer_v1_motion_absolute(ptr, now_msec(),
			dest_x, dest_y, 1920, 1080);
		zwlr_virtual_pointer_v1_frame(ptr);
		pause_dispatch(ctx);
	}
	zwlr_virtual_pointer_v1_button(ptr, now_msec(), BTN_LEFT,
		WL_POINTER_BUTTON_STATE_RELEASED);
	zwlr_virtual_pointer_v1_frame(ptr);
	pause_dispatch(ctx);
	if (drag) {
		/* After implicit grab release, test the hidden region again. */
		zwlr_virtual_pointer_v1_motion_absolute(ptr, now_msec(),
			dest_x, dest_y, 1920, 1080);
		zwlr_virtual_pointer_v1_frame(ptr);
		pause_dispatch(ctx);
	}
	zwlr_virtual_pointer_v1_destroy(ptr);
	printf("SD_CI_VIRTUAL_INPUT_SENT kind=%s x=%u y=%u "
		"dest_x=%u dest_y=%u button=BTN_LEFT press=1 release=1 "
		"same_native_pointer=1 authority=NONE\\n",
		drag ? "GRAB" : "BUTTON", x, y, dest_x, dest_y);
	return true;
}
static bool parse_coord(const char *raw, uint32_t limit, uint32_t *out) {
	char *end = NULL;
	errno = 0;
	unsigned long value = strtoul(raw, &end, 10);
	if (errno || !end || !raw[0] || *end || value >= limit) return false;
	*out = (uint32_t)value;
	return true;
}
int main(int argc, char **argv) {
	const char *ci = getenv("CI"), *actions = getenv("GITHUB_ACTIONS");
	const char *flag = getenv("STAGE_DIRECTOR_CI_CHROME_REAL_INPUT");
	const char *runner = getenv("RUNNER_TEMP");
	if (!ci || strcmp(ci, "true") || !actions ||
			strcmp(actions, "true") || !flag || strcmp(flag, "1") ||
			!runner || strcmp(runner, "/home/runner/work/_temp")) {
		fprintf(stderr, "BLOCKED_VIRTUAL_INPUT_NON_CI\n");
		return 2;
	}
	bool keyboard = argc == 2 && strcmp(argv[1], "keyboard") == 0;
	bool pointer = argc == 4 && strcmp(argv[1], "pointer") == 0;
	bool button = argc == 4 && strcmp(argv[1], "button") == 0;
	bool grab = argc == 6 && strcmp(argv[1], "grab") == 0;
	if (!keyboard && !pointer && !button && !grab) return 3;
	uint32_t x = 0, y = 0, dx = 0, dy = 0;
	if (!keyboard && (!parse_coord(argv[2], 1920, &x) ||
			!parse_coord(argv[3], 1080, &y))) return 4;
	if (grab && (!parse_coord(argv[4], 1920, &dx) ||
			!parse_coord(argv[5], 1080, &dy))) return 4;
	struct context ctx = {0};
	ctx.display = wl_display_connect(NULL);
	if (!ctx.display) return 5;
	struct wl_registry *reg = wl_display_get_registry(ctx.display);
	wl_registry_add_listener(reg, &listeners, &ctx);
	if (wl_display_roundtrip(ctx.display) < 0 || !ctx.seat ||
			(keyboard && !ctx.keyboards) || (pointer && !ctx.pointers)) {
		fprintf(stderr, "FAIL_SD_CI_VIRTUAL_INPUT_PROTOCOL_UNAVAILABLE\n");
		wl_display_disconnect(ctx.display);
		return 6;
	}
	bool ok = keyboard ? actual_keyboard(&ctx) :
		grab ? actual_pointer_button(&ctx, x, y, true, dx, dy) :
		button ? actual_pointer_button(&ctx, x, y, false, x, y) :
		actual_pointer(&ctx, x, y);
	wl_display_roundtrip(ctx.display);
	wl_display_disconnect(ctx.display);
	return ok ? 0 : 8;
}
