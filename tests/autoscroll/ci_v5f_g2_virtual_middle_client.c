/*
 * V5F-G2: real Wayland virtual middle-click, disposable GitHub CI only.
 * This client must never be accepted as a physical input source.
 * It deliberately sends NO motion or axis events.
 */
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/input-event-codes.h>
#include <wayland-client.h>
#include "ext-transient-seat-v1-client-protocol.h"
#include "wlr-virtual-pointer-unstable-v1-client-protocol.h"

struct state {
    struct wl_display *display;
    struct wl_registry *registry;
    struct ext_transient_seat_manager_v1 *seat_manager;
    struct zwlr_virtual_pointer_manager_v1 *pointer_manager;
    struct ext_transient_seat_v1 *transient;
    uint32_t transient_global;
    bool ready, denied;
};

static void global(void *data, struct wl_registry *registry,
        uint32_t name, const char *interface, uint32_t version) {
    struct state *s = data;
    if (strcmp(interface, "ext_transient_seat_manager_v1") == 0) {
        s->seat_manager = wl_registry_bind(registry, name,
            &ext_transient_seat_manager_v1_interface,
            version < 1 ? version : 1);
    } else if (strcmp(interface, "zwlr_virtual_pointer_manager_v1") == 0) {
        s->pointer_manager = wl_registry_bind(registry, name,
            &zwlr_virtual_pointer_manager_v1_interface,
            version < 1 ? version : 1);
    }
}
static void global_remove(void *data, struct wl_registry *registry,
        uint32_t name) {
    (void)data; (void)registry; (void)name;
}
static const struct wl_registry_listener registry_listener = {
    .global = global, .global_remove = global_remove,
};

static void seat_ready(void *data, struct ext_transient_seat_v1 *seat,
        uint32_t global_name) {
    struct state *s = data; (void)seat;
    s->transient_global = global_name;
    s->ready = true;
}
static void seat_denied(void *data, struct ext_transient_seat_v1 *seat) {
    struct state *s = data; (void)seat; s->denied = true;
}
static const struct ext_transient_seat_v1_listener seat_listener = {
    .ready = seat_ready, .denied = seat_denied,
};

static bool input_is(const char *expected) {
    char line[48];
    return fgets(line, sizeof(line), stdin) &&
        strcmp(line, expected) == 0;
}

int main(void) {
    struct state s = {0};
    s.display = wl_display_connect(NULL);
    if (!s.display) {
        fputs("PA_G2_PRIVATE_WAYLAND_CONNECT_FAILED\n", stderr);
        return 2;
    }
    s.registry = wl_display_get_registry(s.display);
    if (!s.registry ||
            wl_registry_add_listener(s.registry, &registry_listener, &s) ||
            wl_display_roundtrip(s.display) < 0 ||
            !s.seat_manager || !s.pointer_manager) {
        fputs("PA_G2_REQUIRED_WAYLAND_PROTOCOL_MISSING\n", stderr);
        return 3;
    }
    s.transient = ext_transient_seat_manager_v1_create(s.seat_manager);
    if (!s.transient ||
            ext_transient_seat_v1_add_listener(s.transient, &seat_listener, &s) ||
            wl_display_roundtrip(s.display) < 0 ||
            s.denied || !s.ready || !s.transient_global) {
        fputs("PA_G2_TRANSIENT_SEAT_CREATION_FAILED\n", stderr);
        return 4;
    }
    struct wl_seat *seat = wl_registry_bind(s.registry, s.transient_global,
        &wl_seat_interface, 1);
    if (!seat || wl_display_roundtrip(s.display) < 0) {
        fputs("PA_G2_TRANSIENT_SEAT_BIND_FAILED\n", stderr);
        return 5;
    }
    struct zwlr_virtual_pointer_v1 *pointer =
        zwlr_virtual_pointer_manager_v1_create_virtual_pointer(
            s.pointer_manager, seat);
    if (!pointer || wl_display_roundtrip(s.display) < 0) {
        fputs("PA_G2_VIRTUAL_POINTER_CREATION_FAILED\n", stderr);
        return 6;
    }
    puts("PA_G2_PROTOCOL_READY transient=1 virtual=1");
    fflush(stdout);

    if (!input_is("click\n")) {
        fputs("PA_G2_CLICK_COMMAND_MISSING\n", stderr);
        return 7;
    }
    zwlr_virtual_pointer_v1_button(pointer, 1001, BTN_MIDDLE,
        WL_POINTER_BUTTON_STATE_PRESSED);
    if (wl_display_roundtrip(s.display) < 0) {
        fputs("PA_G2_PRESS_ROUNDTRIP_FAILED\n", stderr);
        return 8;
    }
    zwlr_virtual_pointer_v1_button(pointer, 1002, BTN_MIDDLE,
        WL_POINTER_BUTTON_STATE_RELEASED);
    if (wl_display_roundtrip(s.display) < 0) {
        fputs("PA_G2_RELEASE_ROUNDTRIP_FAILED\n", stderr);
        return 9;
    }
    puts("PA_G2_PROTOCOL_MIDDLE_SENT pressed=1 released=1 axis=0");
    fflush(stdout);

    if (!input_is("stop\n")) {
        fputs("PA_G2_STOP_COMMAND_MISSING\n", stderr);
        return 10;
    }
    zwlr_virtual_pointer_v1_destroy(pointer);
    if (wl_display_roundtrip(s.display) < 0) {
        fputs("PA_G2_DEVICE_DESTROY_FAILED\n", stderr);
        return 11;
    }
    ext_transient_seat_v1_destroy(s.transient);
    if (wl_display_roundtrip(s.display) < 0) {
        fputs("PA_G2_SEAT_DESTROY_FAILED\n", stderr);
        return 12;
    }
    puts("PA_G2_PROTOCOL_REVOKED device_before_seat=1");
    fflush(stdout);
    wl_display_disconnect(s.display);
    return 0;
}
