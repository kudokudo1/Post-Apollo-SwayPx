#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wayland-client.h>
#include "wlr-virtual-pointer-unstable-v1-client-protocol.h"
#include "sway/input/post_apollo_autoscroll_axis_dispatch.h"

/*
 * TEST TRANSPORT ONLY: actual V1/V2/V3 autoscroll engine calculations are
 * translated to Wayland by the zwlr_virtual_pointer_v1 test protocol.
 * No physical/libinput evidence exists in this process, and the production
 * SwayPX autoscroll runtime stays compiled HARD OFF.
 *
 * Synthetic "trusted" booleans below are test fixtures, not assertions of
 * authority. This program is forbidden outside private disposable CI.
 */
#define CI_DEVICE 7u
#define CI_SEAT 3u
#define CI_TARGET 91u
#define CI_LAYOUT 8u
#define CI_START 1000u

struct sender {
    struct wl_display *display;
    struct wl_seat *seat;
    struct zwlr_virtual_pointer_manager_v1 *manager;
};
struct transport {
    struct zwlr_virtual_pointer_v1 *pointer;
    unsigned int axes;
    unsigned int frames;
    double total_pixels;
};

static void on_global(void *data, struct wl_registry *registry,
        uint32_t name, const char *interface, uint32_t version) {
    struct sender *s = data;
    if (!strcmp(interface, wl_seat_interface.name)) {
        s->seat = wl_registry_bind(registry, name,
            &wl_seat_interface, version < 4 ? version : 4);
    } else if (!strcmp(interface,
            zwlr_virtual_pointer_manager_v1_interface.name)) {
        s->manager = wl_registry_bind(registry, name,
            &zwlr_virtual_pointer_manager_v1_interface, 1);
    }
}
static void on_remove(void *data, struct wl_registry *registry, uint32_t name) {
    (void)data; (void)registry; (void)name;
}
static const struct wl_registry_listener registry_listener = {
    .global = on_global, .global_remove = on_remove,
};
static uint32_t event_time(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) exit(9);
    return (uint32_t)(ts.tv_sec * 1000ull + ts.tv_nsec / 1000000ull);
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
static bool pause_and_roundtrip(struct wl_display *display) {
    if (wl_display_roundtrip(display) < 0) return false;
    struct timespec pause = {.tv_sec = 0, .tv_nsec = 120000000};
    return nanosleep(&pause, NULL) == 0;
}
static bool emit_axis(void *opaque, const struct pa_scroll_axis_event *event) {
    struct transport *t = opaque;
    if (!t || !t->pointer || !event ||
            event->target_node_id != CI_TARGET ||
            event->discrete_steps != 0 ||
            !isfinite(event->delta_pixels) ||
            event->delta_pixels == 0.0 ||
            event->axis != PA_SCROLL_AXIS_VERTICAL) {
        return false;
    }
    /* PROTOCOL SOURCE VIRTUAL, even though the numerical delta was computed
     * by the actual autoscroll engine, adapter and dispatch layers. */
    zwlr_virtual_pointer_v1_axis(t->pointer, event_time(),
        WL_POINTER_AXIS_VERTICAL_SCROLL,
        wl_fixed_from_double(event->delta_pixels));
    t->total_pixels += event->delta_pixels;
    t->axes++;
    return true;
}
static void emit_frame(void *opaque) {
    struct transport *t = opaque;
    if (!t || !t->pointer) return;
    zwlr_virtual_pointer_v1_frame(t->pointer);
    t->frames++;
}
static bool is_disposable_ci(void) {
    const char *ci = getenv("CI"), *actions = getenv("GITHUB_ACTIONS");
    const char *flag = getenv("PA_AUTOSCROLL_CI_ENGINE_SYNTHETIC");
    const char *tmp = getenv("RUNNER_TEMP");
    const char *runtime = getenv("XDG_RUNTIME_DIR");
    return ci && !strcmp(ci, "true")
        && actions && !strcmp(actions, "true")
        && flag && !strcmp(flag, "1")
        && tmp && runtime
        && !strncmp(runtime, tmp, strlen(tmp))
        && runtime[strlen(tmp)] == '/';
}
int main(int argc, char **argv) {
    if (!is_disposable_ci()) {
        fputs("PA_ENGINE_VIRTUAL_DENIED_OUTSIDE_EPHEMERAL_CI\n", stderr);
        return 2;
    }
    uint32_t x, y;
    if (argc != 3 || !coordinate(argv[1], 1920, &x) ||
            !coordinate(argv[2], 1080, &y)) {
        return 3;
    }
    struct sender s = {0};
    s.display = wl_display_connect(NULL);
    if (!s.display) return 4;
    struct wl_registry *registry = wl_display_get_registry(s.display);
    wl_registry_add_listener(registry, &registry_listener, &s);
    if (wl_display_roundtrip(s.display) < 0 || !s.seat || !s.manager) {
        fputs("PA_ENGINE_VIRTUAL_PROTOCOL_UNAVAILABLE\n", stderr);
        wl_display_disconnect(s.display);
        return 5;
    }
    struct zwlr_virtual_pointer_v1 *pointer =
        zwlr_virtual_pointer_manager_v1_create_virtual_pointer(s.manager, s.seat);
    if (!pointer) {
        wl_display_disconnect(s.display);
        return 6;
    }

    /* The pointer focus comes from the real mapped native observer, but the
     * sender is a virtual CI-only device. Never call it libinput evidence. */
    zwlr_virtual_pointer_v1_motion_absolute(pointer, event_time(),
        x, y, 1920, 1080);
    zwlr_virtual_pointer_v1_frame(pointer);
    bool ok = pause_and_roundtrip(s.display);
    struct pa_autoscroll_dispatch d;
    struct transport transport = {.pointer = pointer};
    pa_scroll_dispatch_init(&d);
    if (ok && !d.adapter.enabled) {
        /* This explicit enable lives ONLY in the isolated test executable.
         * Production pa_autoscroll_runtime_create() is still hard OFF. */
        pa_scroll_dispatch_enable(&d, true);
        ok = pa_scroll_dispatch_middle(&d, true, true,
            CI_DEVICE, CI_SEAT, true, CI_TARGET, CI_LAYOUT,
            300.0, 400.0, CI_START, false, false);
        ok = ok && pa_scroll_dispatch_middle(&d, false, true,
            CI_DEVICE, CI_SEAT, true, CI_TARGET, CI_LAYOUT,
            300.0, 400.0, CI_START, false, false);
        ok = ok && pa_scroll_dispatch_motion(&d, true,
            CI_DEVICE, CI_SEAT, 0.0, 190.0, false, false);
        for (unsigned int n = 1; n <= 2 && ok; ++n) {
            ok = pa_scroll_dispatch_tick(&d, CI_DEVICE, CI_SEAT,
                true, CI_TARGET, CI_LAYOUT, CI_START + n * 16u,
                false, false, emit_axis, emit_frame, &transport);
            ok = ok && pause_and_roundtrip(s.display);
        }
        ok = ok && pa_scroll_dispatch_escape(&d, true, CI_SEAT);
        ok = ok && !pa_scroll_dispatch_tick(&d, CI_DEVICE, CI_SEAT,
            true, CI_TARGET, CI_LAYOUT, CI_START + 48u,
            false, false, emit_axis, emit_frame, &transport);
    }
    zwlr_virtual_pointer_v1_destroy(pointer);
    if (wl_display_roundtrip(s.display) < 0) ok = false;
    wl_display_disconnect(s.display);
    if (!ok || transport.axes != 2 || transport.frames != 2 ||
            transport.total_pixels <= 0.0 || d.adapter.engine.active) {
        fprintf(stderr,
            "PA_ENGINE_VIRTUAL_DELIVERY_FAILED axes=%u frames=%u sum=%.3f\n",
            transport.axes, transport.frames, transport.total_pixels);
        return 7;
    }
    printf("PA_ENGINE_VIRTUAL_DELIVERY_SENDER_PASS "
           "engine_axes=%u engine_frames=%u pixels=%.3f "
           "transport=zwlr_virtual_pointer_v1 physical_authority=NONE "
           "production_autoscroll=OFF\n",
           transport.axes, transport.frames, transport.total_pixels);
    return 0;
}
