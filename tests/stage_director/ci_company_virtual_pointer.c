/* M30h disposable Wayland virtual-pointer sender.
 * Actual zwlr_virtual_pointer_v1 messages travel through SwayPX seat routing.
 * Never run outside disposable GitHub Actions or activate production input.
 * Commands on stdin: move X Y, press, release, wheel, quit.
 */
#define _POSIX_C_SOURCE 200809L
#include <linux/input-event-codes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wayland-client.h>
#include "wlr-virtual-pointer-unstable-v1-client-protocol.h"

struct probe {
    struct zwlr_virtual_pointer_manager_v1 *manager;
};

static void reg_global(void *data, struct wl_registry *registry,
        uint32_t name, const char *interface, uint32_t version) {
    struct probe *p = data;
    if (strcmp(interface,
            zwlr_virtual_pointer_manager_v1_interface.name) == 0 &&
            version >= 1 && !p->manager) {
        p->manager = wl_registry_bind(registry, name,
            &zwlr_virtual_pointer_manager_v1_interface,
            version >= 2 ? 2 : 1);
    }
}

static void reg_remove(void *data, struct wl_registry *registry,
        uint32_t name) {
    (void)data;
    (void)registry;
    (void)name;
}

static const struct wl_registry_listener registry_listener = {
    .global = reg_global,
    .global_remove = reg_remove,
};

static uint32_t now_ms(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 1;
    return (uint32_t)((ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL)
        & UINT32_MAX);
}

int main(void) {
    const char *ci = getenv("CI");
    const char *github = getenv("GITHUB_ACTIONS");
    const char *gate = getenv("STAGE_DIRECTOR_CI_COMPANY_INPUT");
    const char *runtime = getenv("XDG_RUNTIME_DIR");
    const char *temp = getenv("RUNNER_TEMP");
    if (!ci || strcmp(ci, "true") != 0 ||
            !github || strcmp(github, "true") != 0 ||
            !gate || strcmp(gate, "1") != 0 ||
            !runtime || !temp ||
            strcmp(temp, "/home/runner/work/_temp") != 0 ||
            strncmp(runtime, temp, strlen(temp)) != 0 ||
            runtime[strlen(temp)] != '/') {
        fputs("BLOCKED_M30H_POINTER_NOT_DISPOSABLE_CI\n", stderr);
        return 2;
    }

    struct wl_display *display = wl_display_connect(NULL);
    if (!display) {
        fputs("M30H_ERROR_NO_PRIVATE_WAYLAND\n", stderr);
        return 3;
    }
    struct wl_registry *registry = wl_display_get_registry(display);
    struct probe p = {0};
    if (!registry ||
            wl_registry_add_listener(registry, &registry_listener, &p) != 0 ||
            wl_display_roundtrip(display) < 0 || !p.manager) {
        fputs("M30H_ERROR_NO_VIRTUAL_POINTER_MANAGER\n", stderr);
        wl_display_disconnect(display);
        return 4;
    }
    struct zwlr_virtual_pointer_v1 *pointer =
        zwlr_virtual_pointer_manager_v1_create_virtual_pointer(
            p.manager, NULL);
    if (!pointer || wl_display_roundtrip(display) < 0) {
        fputs("M30H_ERROR_VIRTUAL_POINTER_CREATE\n", stderr);
        wl_display_disconnect(display);
        return 5;
    }
    puts("SD_CI_COMPANY_POINTER_READY real_zwlr_virtual_pointer=1");
    fflush(stdout);

    char input[100];
    int executed = 0;
    while (fgets(input, sizeof(input), stdin)) {
        input[strcspn(input, "\r\n")] = '\0';
        int x = -1, y = -1;
        if (strcmp(input, "quit") == 0) break;
        if (sscanf(input, "move %d %d", &x, &y) == 2 &&
                x >= 0 && x < 1920 && y >= 0 && y < 1080) {
            zwlr_virtual_pointer_v1_motion_absolute(pointer,
                now_ms(), (uint32_t)x, (uint32_t)y, 1920, 1080);
        } else if (strcmp(input, "press") == 0) {
            zwlr_virtual_pointer_v1_button(pointer, now_ms(),
                BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
        } else if (strcmp(input, "release") == 0) {
            zwlr_virtual_pointer_v1_button(pointer, now_ms(),
                BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);
        } else if (strcmp(input, "wheel") == 0) {
            zwlr_virtual_pointer_v1_axis_source(pointer,
                WL_POINTER_AXIS_SOURCE_WHEEL);
            zwlr_virtual_pointer_v1_axis(pointer, now_ms(),
                WL_POINTER_AXIS_VERTICAL_SCROLL,
                wl_fixed_from_double(15.0));
        } else {
            fprintf(stderr, "M30H_ERROR_UNRECOGNIZED_POINTER_COMMAND %s\n",
                input);
            zwlr_virtual_pointer_v1_destroy(pointer);
            wl_display_disconnect(display);
            return 6;
        }
        zwlr_virtual_pointer_v1_frame(pointer);
        if (wl_display_roundtrip(display) < 0) {
            fputs("M30H_ERROR_POINTER_DISCONNECTED\n", stderr);
            wl_display_disconnect(display);
            return 7;
        }
        printf("SD_CI_COMPANY_POINTER_ACK %s\n", input);
        fflush(stdout);
        executed++;
    }
    zwlr_virtual_pointer_v1_destroy(pointer);
    zwlr_virtual_pointer_manager_v1_destroy(p.manager);
    wl_registry_destroy(registry);
    if (wl_display_roundtrip(display) < 0) {
        fputs("M30H_POINTER_CLEANUP_ERROR\n", stderr);
        wl_display_disconnect(display);
        return 8;
    }
    wl_display_disconnect(display);
    printf("SD_CI_COMPANY_POINTER_STOP commands=%d PASS\n", executed);
    return 0;
}
