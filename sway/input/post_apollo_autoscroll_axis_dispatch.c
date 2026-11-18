/*
 * SwayPX autoscroll axis-intent bridge.
 *
 * Does NOT contain seat, cursor, wlroots, timer, IPC or compositor hooks.
 * The only output side effects are callbacks injected by standalone tests.
 */
#include <math.h>
#include <stddef.h>
#include "sway/input/post_apollo_autoscroll_axis_dispatch.h"

struct dispatch_sink_context {
    struct pa_autoscroll_dispatch *dispatch;
    pa_scroll_emit_axis send;
    void *context;
    uint64_t target_node_id;
    bool emitted;
    bool failed;
};

static void send_one_axis(struct dispatch_sink_context *out,
                          enum pa_scroll_axis axis, double delta) {
    if (out->failed || delta == 0.0) {
        return;
    }
    if (!isfinite(delta) || out->target_node_id == 0) {
        out->failed = true;
        return;
    }
    const struct pa_scroll_axis_event event = {
        .axis = axis,
        .delta_pixels = delta,
        .discrete_steps = 0,
        .target_node_id = out->target_node_id,
    };
    if (!out->send(out->context, &event)) {
        out->failed = true;
        return;
    }
    out->emitted = true;
}

static void prepare_axes(void *opaque, double horizontal, double vertical) {
    struct dispatch_sink_context *out = opaque;
    if (out == NULL || out->failed) {
        return;
    }
    /* Vertical before horizontal for predictable output order. */
    send_one_axis(out, PA_SCROLL_AXIS_VERTICAL, vertical);
    send_one_axis(out, PA_SCROLL_AXIS_HORIZONTAL, horizontal);
}

void pa_scroll_dispatch_init(struct pa_autoscroll_dispatch *dispatch) {
    if (dispatch == NULL) {
        return;
    }
    pa_autoscroll_adapter_init(&dispatch->adapter);
    dispatch->frames = 0;
}

void pa_scroll_dispatch_enable(struct pa_autoscroll_dispatch *dispatch,
                               bool explicitly_enabled) {
    if (dispatch == NULL) {
        return;
    }
    pa_autoscroll_adapter_configure(&dispatch->adapter, explicitly_enabled);
}

void pa_scroll_dispatch_cancel(struct pa_autoscroll_dispatch *dispatch) {
    if (dispatch != NULL) {
        pa_autoscroll_adapter_cancel(&dispatch->adapter);
    }
}

bool pa_scroll_dispatch_middle(
    struct pa_autoscroll_dispatch *dispatch, bool pressed,
    bool trusted_physical_pointer, uint64_t device_id, uint64_t seat_id,
    bool verified_surface_hit, uint64_t target_node_id,
    uint64_t layout_generation, double pointer_x, double pointer_y,
    uint64_t monotonic_ms, bool session_locked,
    bool native_scroll_lock_active) {
    return dispatch != NULL && pa_autoscroll_adapter_middle(
        &dispatch->adapter, pressed, trusted_physical_pointer, device_id,
        seat_id, verified_surface_hit, target_node_id, layout_generation,
        pointer_x, pointer_y, monotonic_ms, session_locked,
        native_scroll_lock_active);
}

bool pa_scroll_dispatch_motion(
    struct pa_autoscroll_dispatch *dispatch, bool trusted_physical_pointer,
    uint64_t device_id, uint64_t seat_id, double dx, double dy,
    bool session_locked, bool native_scroll_lock_active) {
    return dispatch != NULL && pa_autoscroll_adapter_motion(
        &dispatch->adapter, trusted_physical_pointer, device_id, seat_id,
        dx, dy, session_locked, native_scroll_lock_active);
}

bool pa_scroll_dispatch_other_button(
    struct pa_autoscroll_dispatch *dispatch, uint32_t button, bool pressed,
    bool trusted_physical_pointer, uint64_t device_id, uint64_t seat_id) {
    return dispatch != NULL && pa_autoscroll_adapter_other_button(
        &dispatch->adapter, button, pressed, trusted_physical_pointer,
        device_id, seat_id);
}

bool pa_scroll_dispatch_escape(
    struct pa_autoscroll_dispatch *dispatch,
    bool trusted_escape_pressed, uint64_t seat_id) {
    return dispatch != NULL && pa_autoscroll_adapter_escape(
        &dispatch->adapter, trusted_escape_pressed, seat_id);
}

bool pa_scroll_dispatch_tick(
    struct pa_autoscroll_dispatch *dispatch,
    uint64_t device_id, uint64_t seat_id,
    bool verified_surface_hit, uint64_t target_node_id,
    uint64_t layout_generation, uint64_t monotonic_ms,
    bool session_locked, bool native_scroll_lock_active,
    pa_scroll_emit_axis emit_axis, pa_scroll_emit_frame emit_frame,
    void *context) {
    if (dispatch == NULL) {
        return false;
    }
    /* The sink must be complete before the adapter advances its timer. */
    if (emit_axis == NULL || emit_frame == NULL) {
        pa_scroll_dispatch_cancel(dispatch);
        return false;
    }
    struct dispatch_sink_context output = {
        .dispatch = dispatch,
        .send = emit_axis,
        .context = context,
        .target_node_id = target_node_id,
        .emitted = false,
        .failed = false,
    };
    bool active = pa_autoscroll_adapter_tick(
        &dispatch->adapter, device_id, seat_id,
        verified_surface_hit, target_node_id, layout_generation,
        monotonic_ms, session_locked, native_scroll_lock_active,
        prepare_axes, &output);
    if (!active || output.failed) {
        if (output.failed) {
            pa_scroll_dispatch_cancel(dispatch);
        }
        return false;
    }
    if (output.emitted) {
        emit_frame(context);
        if (dispatch->frames < UINT64_MAX) {
            dispatch->frames++;
        }
    }
    return true;
}
