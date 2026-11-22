/*
 * G3C disposable Wayland client: actual transient seat + virtual pointer,
 * no middle-click, movement, axis or keyboard input ever transmitted.
 * Supports test-directed independent device/seat destruction order.
 */
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <wayland-client.h>
#include "ext-transient-seat-v1-client-protocol.h"
#include "wlr-virtual-pointer-unstable-v1-client-protocol.h"

struct state {
    struct wl_display *display;
    struct wl_registry *registry;
    struct ext_transient_seat_manager_v1 *seat_manager;
    struct zwlr_virtual_pointer_manager_v1 *pointer_manager;
    struct ext_transient_seat_v1 *transient;
    uint32_t global_name;
    bool ready, denied;
};

static void global(void *data, struct wl_registry *registry,
        uint32_t name, const char *iface, uint32_t version) {
    struct state *s = data;
    if (strcmp(iface, "ext_transient_seat_manager_v1") == 0) {
        s->seat_manager = wl_registry_bind(registry, name,
            &ext_transient_seat_manager_v1_interface,
            version < 1 ? version : 1);
    } else if (strcmp(iface, "zwlr_virtual_pointer_manager_v1") == 0) {
        s->pointer_manager = wl_registry_bind(registry, name,
            &zwlr_virtual_pointer_manager_v1_interface,
            version < 1 ? version : 1);
    }
}
static void removed(void *data, struct wl_registry *registry,
        uint32_t name) {
    (void)data; (void)registry; (void)name;
}
static const struct wl_registry_listener registry_listener = {
    .global = global, .global_remove = removed,
};
static void ready(void *data, struct ext_transient_seat_v1 *seat,
        uint32_t global_name) {
    struct state *s = data; (void)seat;
    s->global_name = global_name;
    s->ready = true;
}
static void denied(void *data, struct ext_transient_seat_v1 *seat) {
    struct state *s = data; (void)seat;
    s->denied = true;
}
static const struct ext_transient_seat_v1_listener seat_listener = {
    .ready = ready, .denied = denied,
};

int main(void) {
    struct state s = {0};
    s.display = wl_display_connect(NULL);
    if (!s.display) {
        fputs("PA_G3C_PRIVATE_WAYLAND_CONNECT_FAILED\n", stderr);
        return 2;
    }
    s.registry = wl_display_get_registry(s.display);
    if (!s.registry ||
            wl_registry_add_listener(s.registry, &registry_listener, &s) ||
            wl_display_roundtrip(s.display) < 0 ||
            !s.seat_manager || !s.pointer_manager) {
        fputs("PA_G3C_WAYLAND_MANAGERS_MISSING\n", stderr);
        return 3;
    }
    s.transient = ext_transient_seat_manager_v1_create(s.seat_manager);
    if (!s.transient ||
            ext_transient_seat_v1_add_listener(s.transient, &seat_listener, &s) ||
            wl_display_roundtrip(s.display) < 0 ||
            !s.ready || s.denied || !s.global_name) {
        fputs("PA_G3C_TRANSIENT_SEAT_FAILED\n", stderr);
        return 4;
    }
    struct wl_seat *seat = wl_registry_bind(s.registry,
        s.global_name, &wl_seat_interface, 1);
    if (!seat || wl_display_roundtrip(s.display) < 0) {
        fputs("PA_G3C_TRANSIENT_SEAT_BIND_FAILED\n", stderr);
        return 5;
    }
    struct zwlr_virtual_pointer_v1 *pointer =
        zwlr_virtual_pointer_manager_v1_create_virtual_pointer(
            s.pointer_manager, seat);
    if (!pointer || wl_display_roundtrip(s.display) < 0) {
        fputs("PA_G3C_VIRTUAL_POINTER_FAILED\n", stderr);
        return 6;
    }
    puts("PA_G3C_PROTOCOL_READY real_transient=1 real_virtual=1");
    fflush(stdout);
    bool pointer_alive = true, seat_alive = true;
    char *line = NULL;
    size_t cap = 0;
    while (getline(&line, &cap, stdin) >= 0) {
        if (strcmp(line, "device\n") == 0 && pointer_alive) {
            zwlr_virtual_pointer_v1_destroy(pointer);
            pointer_alive = false;
            if (wl_display_roundtrip(s.display) < 0) {
                fputs("PA_G3C_DEVICE_DESTROY_ROUNDTRIP_FAILED\n", stderr);
                free(line); return 7;
            }
            puts("PA_G3C_PROTOCOL_DEVICE_REVOKED");
            fflush(stdout);
        } else if (strcmp(line, "seat\n") == 0 && seat_alive) {
            ext_transient_seat_v1_destroy(s.transient);
            seat_alive = false;
            if (wl_display_roundtrip(s.display) < 0) {
                fputs("PA_G3C_SEAT_DESTROY_ROUNDTRIP_FAILED\n", stderr);
                free(line); return 8;
            }
            puts("PA_G3C_PROTOCOL_SEAT_REVOKED");
            fflush(stdout);
        } else if (strcmp(line, "stop\n") == 0) {
            if (pointer_alive) {
                zwlr_virtual_pointer_v1_destroy(pointer);
                pointer_alive = false;
                if (wl_display_roundtrip(s.display) < 0) {
                    free(line); return 9;
                }
            }
            if (seat_alive) {
                ext_transient_seat_v1_destroy(s.transient);
                seat_alive = false;
                if (wl_display_roundtrip(s.display) < 0) {
                    free(line); return 10;
                }
            }
            puts("PA_G3C_PROTOCOL_DONE no_input_events=1");
            fflush(stdout);
            free(line);
            wl_display_disconnect(s.display);
            return 0;
        } else {
            fputs("PA_G3C_UNEXPECTED_PROTOCOL_TEST_COMMAND\n", stderr);
            free(line); return 11;
        }
    }
    fputs("PA_G3C_MISSING_STOP_COMMAND\n", stderr);
    free(line);
    return 12;
}
