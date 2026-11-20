/*
 * V5F-C2 disposable Wayland protocol client.
 *
 * Requests REAL compositor-owned transient seats and virtual pointers over
 * private Wayland, never fabricates a wlr_* C object or sends input events.
 * No credentials, no hardware. CI harness controls one create/destroy cycle
 * per fresh process; all virtual pointers MUST remain ineligible as physical.
 */
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    bool ready;
    bool denied;
};

static void registry_global(void *data, struct wl_registry *registry,
        uint32_t name, const char *interface, uint32_t version) {
    struct state *state = data;
    if (strcmp(interface, "ext_transient_seat_manager_v1") == 0) {
        state->seat_manager = wl_registry_bind(registry, name,
            &ext_transient_seat_manager_v1_interface,
            version < 1 ? version : 1);
    } else if (strcmp(interface, "zwlr_virtual_pointer_manager_v1") == 0) {
        state->pointer_manager = wl_registry_bind(registry, name,
            &zwlr_virtual_pointer_manager_v1_interface,
            version < 1 ? version : 1);
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

static void transient_ready(void *data, struct ext_transient_seat_v1 *seat,
        uint32_t global_name) {
    struct state *state = data;
    (void)seat;
    state->transient_global = global_name;
    state->ready = true;
}

static void transient_denied(void *data, struct ext_transient_seat_v1 *seat) {
    struct state *state = data;
    (void)seat;
    state->denied = true;
}

static const struct ext_transient_seat_v1_listener transient_listener = {
    .ready = transient_ready,
    .denied = transient_denied,
};

int main(void) {
    struct state state = {0};
    state.display = wl_display_connect(NULL);
    if (!state.display) {
        fprintf(stderr, "PA_FC2_PROTOCOL_NO_PRIVATE_WAYLAND_DISPLAY\n");
        return 2;
    }
    state.registry = wl_display_get_registry(state.display);
    if (!state.registry ||
            wl_registry_add_listener(state.registry, &registry_listener,
                &state) != 0 ||
            wl_display_roundtrip(state.display) < 0 ||
            !state.seat_manager || !state.pointer_manager) {
        fprintf(stderr, "PA_FC2_PROTOCOL_REQUIRED_MANAGERS_NOT_ADVERTISED\n");
        return 3;
    }
    state.transient = ext_transient_seat_manager_v1_create(state.seat_manager);
    if (!state.transient ||
            ext_transient_seat_v1_add_listener(state.transient,
                &transient_listener, &state) != 0 ||
            wl_display_roundtrip(state.display) < 0 ||
            state.denied || !state.ready || state.transient_global == 0) {
        fprintf(stderr, "PA_FC2_REAL_TRANSIENT_SEAT_NOT_READY\n");
        return 4;
    }
    struct wl_seat *seat = wl_registry_bind(state.registry,
        state.transient_global, &wl_seat_interface, 1);
    if (!seat || wl_display_roundtrip(state.display) < 0) {
        fprintf(stderr, "PA_FC2_TRANSIENT_WL_SEAT_BIND_FAILED\n");
        return 5;
    }
    struct zwlr_virtual_pointer_v1 *pointer =
        zwlr_virtual_pointer_manager_v1_create_virtual_pointer(
            state.pointer_manager, seat);
    if (!pointer || wl_display_roundtrip(state.display) < 0) {
        fprintf(stderr, "PA_FC2_REAL_VIRTUAL_POINTER_NOT_CREATED\n");
        return 6;
    }
    puts("PA_FC2_PROTOCOL_OBJECTS_READY transient=1 virtual=1");
    fflush(stdout);

    char *line = NULL;
    size_t capacity = 0;
    ssize_t n = getline(&line, &capacity, stdin);
    bool stop = n >= 4 && strcmp(line, "stop\n") == 0;
    free(line);
    if (!stop) {
        fprintf(stderr, "PA_FC2_PROTOCOL_BAD_SHUTDOWN_REQUEST\n");
        return 7;
    }
    zwlr_virtual_pointer_v1_destroy(pointer);
    if (wl_display_roundtrip(state.display) < 0) {
        fprintf(stderr, "PA_FC2_DEVICE_DESTROY_ACK_MISSING\n");
        return 8;
    }
    ext_transient_seat_v1_destroy(state.transient);
    if (wl_display_roundtrip(state.display) < 0) {
        fprintf(stderr, "PA_FC2_SEAT_DESTROY_ACK_MISSING\n");
        return 9;
    }
    puts("PA_FC2_PROTOCOL_OBJECTS_DESTROYED device_before_seat=1");
    fflush(stdout);
    wl_display_disconnect(state.display);
    return 0;
}
