/*
 * Post-Apollo native autoscroll cursor bridge, compiled but HARD OFF.
 *
 * The current security model deliberately refuses activation. Future code
 * must verify physical libinput provenance, absence of Sway Scroll Lock,
 * exact pointer hit-test/target, output-layout generation, Esc binding and
 * a non-interactive indicator before enabling this runtime.
 *
 * No public IPC or external device access is introduced by this module.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_seat.h>
#include "config.h"
#include "util.h"
#include "sway/input/cursor.h"
#include "sway/input/post_apollo_autoscroll_axis_dispatch.h"
#include "sway/input/post_apollo_autoscroll_runtime.h"
#include "sway/server.h"
#include "sway/tree/node.h"

/* Compile-time safety boundary. No user-facing option to change it. */
#define PA_AUTOSCROLL_LIVE_ACTIVATION 0

struct pa_autoscroll_runtime {
    struct sway_cursor *cursor;
    struct pa_autoscroll_dispatch dispatch;
    struct wl_event_source *timer;
    struct wlr_input_device *source_device;
};

static uint64_t pointer_target(struct sway_cursor *cursor) {
    if (cursor == NULL || cursor->cursor == NULL || cursor->seat == NULL) {
        return 0;
    }
    struct wlr_surface *surface = NULL;
    double sx, sy;
    struct sway_node *node = node_at_coords(cursor->seat,
        cursor->cursor->x, cursor->cursor->y, &surface, &sx, &sy);
    if (node == NULL || node->type != N_CONTAINER ||
            node->destroying || surface == NULL) {
        return 0;
    }
    return (uint64_t)node->id;
}

static bool send_axis_intent(void *opaque,
        const struct pa_scroll_axis_event *event) {
    struct pa_autoscroll_runtime *runtime = opaque;
    if (!PA_AUTOSCROLL_LIVE_ACTIVATION || runtime == NULL ||
            runtime->cursor == NULL || event == NULL ||
            !runtime->dispatch.adapter.enabled ||
            runtime->cursor->seat == NULL ||
            event->target_node_id == 0 ||
            event->target_node_id != pointer_target(runtime->cursor)) {
        return false;
    }
    enum wl_pointer_axis axis = event->axis == PA_SCROLL_AXIS_VERTICAL
        ? WL_POINTER_AXIS_VERTICAL_SCROLL
        : WL_POINTER_AXIS_HORIZONTAL_SCROLL;
    wlr_seat_pointer_notify_axis(runtime->cursor->seat->wlr_seat,
        get_current_time_in_msec(), axis, event->delta_pixels,
        event->discrete_steps, WL_POINTER_AXIS_SOURCE_CONTINUOUS,
        WL_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL);
    return true;
}

static void send_axis_frame(void *opaque) {
    struct pa_autoscroll_runtime *runtime = opaque;
    if (PA_AUTOSCROLL_LIVE_ACTIVATION && runtime != NULL &&
            runtime->dispatch.adapter.enabled &&
            runtime->cursor != NULL && runtime->cursor->seat != NULL) {
        wlr_seat_pointer_notify_frame(runtime->cursor->seat->wlr_seat);
    }
}

static int autoscroll_timer_tick(void *opaque) {
    struct pa_autoscroll_runtime *runtime = opaque;
    if (runtime == NULL || !PA_AUTOSCROLL_LIVE_ACTIVATION ||
            !runtime->dispatch.adapter.enabled ||
            !runtime->dispatch.adapter.engine.active ||
            runtime->source_device == NULL ||
            runtime->cursor == NULL ||
            server.session_lock.lock != NULL) {
        if (runtime != NULL) {
            pa_scroll_dispatch_cancel(&runtime->dispatch);
            runtime->source_device = NULL;
        }
        return 0;
    }
    uint64_t target = pointer_target(runtime->cursor);
    /* No verified layout-generation or native Scroll Lock witness yet.
     * Passing layout_generation=0 and native_scroll_lock_active=true makes
     * this path fail closed, even in a hypothetical accidental enable.
     */
    pa_scroll_dispatch_tick(&runtime->dispatch,
        (uint64_t)(uintptr_t)runtime->source_device,
        (uint64_t)(uintptr_t)runtime->cursor->seat,
        target != 0, target, 0,
        get_current_time_in_msec(), false, true,
        send_axis_intent, send_axis_frame, runtime);
    if (runtime->dispatch.adapter.engine.active) {
        wl_event_source_timer_update(runtime->timer, 16);
    } else {
        runtime->source_device = NULL;
    }
    return 0;
}

struct pa_autoscroll_runtime *pa_autoscroll_runtime_create(
        struct sway_cursor *cursor) {
    /* Production hard-OFF means ZERO allocation, zero timer registration
     * and no changes to normal compositor startup or event-loop behavior. */
    if (cursor == NULL || !PA_AUTOSCROLL_LIVE_ACTIVATION) {
        return NULL;
    }
    struct pa_autoscroll_runtime *runtime = calloc(1, sizeof(*runtime));
    if (runtime == NULL) {
        return NULL;
    }
    runtime->cursor = cursor;
    pa_scroll_dispatch_init(&runtime->dispatch);
    /* No enable call. Production mode cannot activate. */
    runtime->timer = wl_event_loop_add_timer(server.wl_event_loop,
        autoscroll_timer_tick, runtime);
    if (runtime->timer == NULL) {
        free(runtime);
        return NULL;
    }
    return runtime;
}

void pa_autoscroll_runtime_cancel(struct pa_autoscroll_runtime *runtime) {
    if (runtime == NULL) {
        return;
    }
    pa_scroll_dispatch_cancel(&runtime->dispatch);
    runtime->source_device = NULL;
    if (runtime->timer != NULL) {
        wl_event_source_timer_update(runtime->timer, -1);
    }
}

void pa_autoscroll_runtime_destroy(struct pa_autoscroll_runtime *runtime) {
    if (runtime == NULL) {
        return;
    }
    pa_autoscroll_runtime_cancel(runtime);
    if (runtime->timer != NULL) {
        wl_event_source_remove(runtime->timer);
        runtime->timer = NULL;
    }
    free(runtime);
}

bool pa_autoscroll_runtime_button(struct pa_autoscroll_runtime *runtime,
        struct wlr_input_device *device, uint32_t timestamp,
        uint32_t button, bool pressed) {
    if (!PA_AUTOSCROLL_LIVE_ACTIVATION || runtime == NULL ||
            !runtime->dispatch.adapter.enabled ||
            device == NULL || runtime->cursor == NULL) {
        return false;
    }
    /* No verified physical pointer or libinput Scroll Lock source exists.
     * These false/true gates intentionally prevent consumption for now.
     */
    const bool physical_verified = false;
    const bool native_scroll_lock_active = true;
    if (button == PA_AUTOSCROLL_BTN_MIDDLE) {
        uint64_t target = pointer_target(runtime->cursor);
        bool consumed = pa_scroll_dispatch_middle(&runtime->dispatch,
            pressed, physical_verified,
            (uint64_t)(uintptr_t)device,
            (uint64_t)(uintptr_t)runtime->cursor->seat,
            target != 0, target, 0,
            runtime->cursor->cursor->x, runtime->cursor->cursor->y,
            timestamp, server.session_lock.lock != NULL,
            native_scroll_lock_active);
        if (consumed && runtime->dispatch.adapter.engine.active) {
            runtime->source_device = device;
            wl_event_source_timer_update(runtime->timer, 16);
        } else if (!runtime->dispatch.adapter.engine.active) {
            pa_autoscroll_runtime_cancel(runtime);
        }
        return consumed;
    }
    return pa_scroll_dispatch_other_button(&runtime->dispatch,
        button, pressed, physical_verified,
        (uint64_t)(uintptr_t)device,
        (uint64_t)(uintptr_t)runtime->cursor->seat);
}

bool pa_autoscroll_runtime_motion(struct pa_autoscroll_runtime *runtime,
        struct wlr_input_device *device, double dx, double dy) {
    if (!PA_AUTOSCROLL_LIVE_ACTIVATION || runtime == NULL ||
            !runtime->dispatch.adapter.enabled ||
            device == NULL || runtime->cursor == NULL) {
        return false;
    }
    return pa_scroll_dispatch_motion(&runtime->dispatch,
        false, (uint64_t)(uintptr_t)device,
        (uint64_t)(uintptr_t)runtime->cursor->seat, dx, dy,
        server.session_lock.lock != NULL, true);
}
