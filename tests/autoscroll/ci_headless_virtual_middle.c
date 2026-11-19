#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <linux/input-event-codes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wayland-client.h>
#include "wlr-virtual-pointer-unstable-v1-client-protocol.h"

/* Disposable Linux CI ONLY. This emitter creates an actual wlroots VIRTUAL
 * pointer. It is NOT a physical libinput witness and MUST NEVER authorize
 * PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT or enable production autoscroll.
 */
struct sender {
    struct wl_display *display;
    struct wl_seat *seat;
    struct zwlr_virtual_pointer_manager_v1 *manager;
};

static void on_global(void *data, struct wl_registry *registry,
        uint32_t name, const char *interface, uint32_t version) {
    struct sender *sender = data;
    if (!strcmp(interface, wl_seat_interface.name)) {
        sender->seat = wl_registry_bind(registry, name,
            &wl_seat_interface, version < 4 ? version : 4);
    } else if (!strcmp(interface,
            zwlr_virtual_pointer_manager_v1_interface.name)) {
        sender->manager = wl_registry_bind(registry, name,
            &zwlr_virtual_pointer_manager_v1_interface, 1);
    }
}
static void on_global_remove(void *data, struct wl_registry *registry,
        uint32_t name) {
    (void)data; (void)registry; (void)name;
}
static const struct wl_registry_listener registry_listener = {
    .global = on_global, .global_remove = on_global_remove,
};

static uint32_t event_time(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) exit(9);
    return (uint32_t)(ts.tv_sec * 1000ull + ts.tv_nsec / 1000000ull);
}
static bool flush_and_pause(struct wl_display *display) {
    if (wl_display_roundtrip(display) < 0) return false;
    struct timespec delay = {.tv_sec = 0, .tv_nsec = 130000000};
    return nanosleep(&delay, NULL) == 0;
}
static bool coordinate(const char *value, uint32_t maximum, uint32_t *out) {
    char *end = NULL;
    errno = 0;
    unsigned long result = strtoul(value, &end, 10);
    if (errno || !value[0] || !end || *end || result >= maximum) {
        return false;
    }
    *out = (uint32_t)result;
    return true;
}

int main(int argc, char **argv) {
    const char *ci = getenv("CI"), *actions = getenv("GITHUB_ACTIONS");
    const char *flag = getenv("PA_AUTOSCROLL_CI_VIRTUAL_NEGATIVE");
    const char *tmp = getenv("RUNNER_TEMP"), *runtime = getenv("XDG_RUNTIME_DIR");
    if (!ci || strcmp(ci, "true") || !actions ||
            strcmp(actions, "true") || !flag || strcmp(flag, "1") ||
            !tmp || !runtime || strncmp(runtime, tmp, strlen(tmp)) ||
            runtime[strlen(tmp)] != '/') {
        fputs("PA_AUTOSCROLL_BLOCKED_NOT_EPHEMERAL_CI\n", stderr);
        return 2;
    }
    uint32_t x, y;
    const bool middle = argc == 4 && strcmp(argv[1], "middle") == 0;
    const bool axis_control = argc == 4 && strcmp(argv[1], "axis") == 0;
    if ((!middle && !axis_control) ||
            !coordinate(argv[2], 1920, &x) ||
            !coordinate(argv[3], 1080, &y)) {
        return 3;
    }
    struct sender sender = {0};
    sender.display = wl_display_connect(NULL);
    if (!sender.display) return 4;
    struct wl_registry *registry = wl_display_get_registry(sender.display);
    wl_registry_add_listener(registry, &registry_listener, &sender);
    if (wl_display_roundtrip(sender.display) < 0 ||
            !sender.seat || !sender.manager) {
        fputs("PA_AUTOSCROLL_VIRTUAL_POINTER_PROTOCOL_UNAVAILABLE\n", stderr);
        wl_display_disconnect(sender.display);
        return 5;
    }
    struct zwlr_virtual_pointer_v1 *pointer =
        zwlr_virtual_pointer_manager_v1_create_virtual_pointer(
            sender.manager, sender.seat);
    if (!pointer) return 6;
    bool okay = flush_and_pause(sender.display);
    if (okay) {
        zwlr_virtual_pointer_v1_motion_absolute(pointer, event_time(),
            x, y, 1920, 1080);
        zwlr_virtual_pointer_v1_frame(pointer);
        okay = flush_and_pause(sender.display);
    }
    if (axis_control && okay) {
        /* Positive observer control only, AFTER the default-OFF negative
         * check. Explicit virtual axis is never autoscroll engine output. */
        zwlr_virtual_pointer_v1_axis(pointer, event_time(),
            WL_POINTER_AXIS_VERTICAL_SCROLL, wl_fixed_from_int(16));
        zwlr_virtual_pointer_v1_frame(pointer);
        okay = flush_and_pause(sender.display);
    }
    if (middle && okay) {
        zwlr_virtual_pointer_v1_button(pointer, event_time(), BTN_MIDDLE,
            WL_POINTER_BUTTON_STATE_PRESSED);
        zwlr_virtual_pointer_v1_frame(pointer);
        okay = flush_and_pause(sender.display);
    }
    if (middle && okay) {
        /* Real virtual-pointer protocol motion, not a PA engine injection. */
        zwlr_virtual_pointer_v1_motion(pointer, event_time(),
            wl_fixed_from_int(0), wl_fixed_from_int(12));
        zwlr_virtual_pointer_v1_frame(pointer);
        okay = flush_and_pause(sender.display);
    }
    if (middle && okay) {
        zwlr_virtual_pointer_v1_button(pointer, event_time(), BTN_MIDDLE,
            WL_POINTER_BUTTON_STATE_RELEASED);
        zwlr_virtual_pointer_v1_frame(pointer);
        okay = flush_and_pause(sender.display);
    }
    zwlr_virtual_pointer_v1_destroy(pointer);
    wl_display_roundtrip(sender.display);
    wl_display_disconnect(sender.display);
    if (!okay) return 7;
    if (middle) {
        printf("PA_AUTOSCROLL_CI_SENT source=VIRTUAL button=BTN_MIDDLE "
               "press=1 release=1 motion=1 axis_injected=0 "
               "physical_authority=NONE\n");
    } else {
        printf("PA_AUTOSCROLL_CI_AXIS_CONTROL source=VIRTUAL "
               "explicit_virtual_axis=1 autoscroll_engine=NONE "
               "physical_authority=NONE\n");
    }
    return 0;
}
