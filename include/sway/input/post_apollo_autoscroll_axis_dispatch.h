#ifndef SWAY_INPUT_POST_APOLLO_AUTOSCROLL_AXIS_DISPATCH_H
#define SWAY_INPUT_POST_APOLLO_AUTOSCROLL_AXIS_DISPATCH_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/input/post_apollo_autoscroll_adapter.h"

/*
 * Pure, portable axis intent dispatch. NO live Wayland input operations.
 *
 * The compositor-owning bridge must map PA_SCROLL_AXIS_VERTICAL/HORIZONTAL
 * to its native wl_pointer.axis API and issue exactly one pointer frame after
 * each successful tick with any nonzero axis intent.
 *
 * This is not integrated into cursor.c or seatop_default.c and is NOT
 * compiled into the shipping compositor.
 */
enum pa_scroll_axis {
    PA_SCROLL_AXIS_VERTICAL = 1,
    PA_SCROLL_AXIS_HORIZONTAL = 2,
};
struct pa_scroll_axis_event {
    enum pa_scroll_axis axis;
    double delta_pixels;
    int32_t discrete_steps; /* zero, continuous pixel scroll */
    uint64_t target_node_id;
};

typedef bool (*pa_scroll_emit_axis)(
    void *context, const struct pa_scroll_axis_event *event);
typedef void (*pa_scroll_emit_frame)(void *context);

struct pa_autoscroll_dispatch {
    struct pa_autoscroll_adapter adapter;
    /* Number of successful frames; diagnostics only, never authority. */
    uint64_t frames;
};

/* Explicitly disabled after initialization. No live enable source exists. */
void pa_scroll_dispatch_init(struct pa_autoscroll_dispatch *dispatch);
void pa_scroll_dispatch_enable(struct pa_autoscroll_dispatch *dispatch,
                               bool explicitly_enabled);
void pa_scroll_dispatch_cancel(struct pa_autoscroll_dispatch *dispatch);

/*
 * The compositor must pass an authenticated physical pointer event. The
 * adapter's boolean inputs are not themselves proof of device or target.
 */
bool pa_scroll_dispatch_middle(
    struct pa_autoscroll_dispatch *dispatch, bool pressed,
    bool trusted_physical_pointer, uint64_t device_id, uint64_t seat_id,
    bool verified_surface_hit, uint64_t target_node_id,
    uint64_t layout_generation, double pointer_x, double pointer_y,
    uint64_t monotonic_ms, bool session_locked,
    bool native_scroll_lock_active);

bool pa_scroll_dispatch_motion(
    struct pa_autoscroll_dispatch *dispatch, bool trusted_physical_pointer,
    uint64_t device_id, uint64_t seat_id, double dx, double dy,
    bool session_locked, bool native_scroll_lock_active);

bool pa_scroll_dispatch_other_button(
    struct pa_autoscroll_dispatch *dispatch, uint32_t button, bool pressed,
    bool trusted_physical_pointer, uint64_t device_id, uint64_t seat_id);

bool pa_scroll_dispatch_escape(
    struct pa_autoscroll_dispatch *dispatch,
    bool trusted_escape_pressed, uint64_t seat_id);

/*
 * One authorized tick emits zero, one or two continuous-axis intentions,
 * then exactly one frame when any axis was successfully emitted.
 * Missing / rejected emission cancels the session, without a frame claim.
 */
bool pa_scroll_dispatch_tick(
    struct pa_autoscroll_dispatch *dispatch,
    uint64_t device_id, uint64_t seat_id,
    bool verified_surface_hit, uint64_t target_node_id,
    uint64_t layout_generation, uint64_t monotonic_ms,
    bool session_locked, bool native_scroll_lock_active,
    pa_scroll_emit_axis emit_axis, pa_scroll_emit_frame emit_frame,
    void *context);

#endif
