/*
 * SwayPX internal autoscroll math. No runtime hook or permission changes.
 * Intentionally no include of compositor cursor, seat, wlroots or IPC APIs.
 */
#include <math.h>
#include <stddef.h>
#include <string.h>
#include "sway/input/post_apollo_autoscroll.h"

#define PA_SCROLL_DEADZONE 15.0
#define PA_SCROLL_DISPLACEMENT_CAP 1200.0
#define PA_SCROLL_MAX_PIXELS_PER_SECOND 2800.0
#define PA_SCROLL_RATE_FACTOR 0.008
#define PA_SCROLL_MAX_TICK_MS 100
#define PA_SCROLL_MAX_SESSION_MS 180000

static double pa_scroll_clamp(double value, double max) {
    return fmax(-max, fmin(max, value));
}

static double pa_scroll_speed(double distance) {
    if (!isfinite(distance) || fabs(distance) <= PA_SCROLL_DEADZONE) {
        return 0.0;
    }
    double excess = fabs(distance) - PA_SCROLL_DEADZONE;
    double magnitude = PA_SCROLL_RATE_FACTOR * pow(excess, 2.2);
    return copysign(fmin(PA_SCROLL_MAX_PIXELS_PER_SECOND, magnitude),
                    distance);
}

void pa_autoscroll_init(struct pa_autoscroll_state *state) {
    if (state == NULL) {
        return;
    }
    memset(state, 0, sizeof(*state));
}

void pa_autoscroll_cancel(struct pa_autoscroll_state *state) {
    if (state == NULL) {
        return;
    }
    state->active = false;
    state->target_node_id = 0;
    state->layout_generation = 0;
    state->start_ms = 0;
    state->last_tick_ms = 0;
    state->anchor_x = 0;
    state->anchor_y = 0;
    state->displacement_x = 0;
    state->displacement_y = 0;
    if (state->generation < UINT64_MAX) {
        state->generation++;
    }
}

bool pa_autoscroll_begin(struct pa_autoscroll_state *state,
                        bool compositor_mode_enabled,
                        bool real_pointer_target,
                        uint64_t target_node_id,
                        uint64_t layout_generation,
                        double anchor_x, double anchor_y,
                        uint64_t monotonic_ms) {
    if (state == NULL || state->active || !compositor_mode_enabled
            || !real_pointer_target || target_node_id == 0
            || layout_generation == 0 || monotonic_ms == 0
            || !isfinite(anchor_x) || !isfinite(anchor_y)
            || state->generation == UINT64_MAX) {
        return false;
    }
    state->active = true;
    state->target_node_id = target_node_id;
    state->layout_generation = layout_generation;
    state->start_ms = monotonic_ms;
    state->last_tick_ms = monotonic_ms;
    state->anchor_x = anchor_x;
    state->anchor_y = anchor_y;
    state->displacement_x = 0.0;
    state->displacement_y = 0.0;
    state->generation++;
    return true;
}

void pa_autoscroll_move(struct pa_autoscroll_state *state,
                        double delta_x, double delta_y) {
    if (state == NULL || !state->active) {
        return;
    }
    if (!isfinite(delta_x) || !isfinite(delta_y)) {
        pa_autoscroll_cancel(state);
        return;
    }
    state->displacement_x = pa_scroll_clamp(
        state->displacement_x + delta_x, PA_SCROLL_DISPLACEMENT_CAP);
    state->displacement_y = pa_scroll_clamp(
        state->displacement_y + delta_y, PA_SCROLL_DISPLACEMENT_CAP);
}

struct pa_autoscroll_delta pa_autoscroll_tick(
                        struct pa_autoscroll_state *state,
                        uint64_t current_target_node_id,
                        uint64_t current_layout_generation,
                        uint64_t monotonic_ms) {
    struct pa_autoscroll_delta result = {0};
    if (state == NULL || !state->active) {
        return result;
    }
    if (current_target_node_id == 0
            || current_target_node_id != state->target_node_id
            || current_layout_generation != state->layout_generation
            || monotonic_ms <= state->last_tick_ms
            || monotonic_ms < state->start_ms
            || monotonic_ms - state->start_ms > PA_SCROLL_MAX_SESSION_MS
            || monotonic_ms - state->last_tick_ms > PA_SCROLL_MAX_TICK_MS) {
        pa_autoscroll_cancel(state);
        return result;
    }
    double dt_s = (double)(monotonic_ms - state->last_tick_ms) / 1000.0;
    state->last_tick_ms = monotonic_ms;
    result.horizontal_pixels = pa_scroll_speed(state->displacement_x) * dt_s;
    result.vertical_pixels = pa_scroll_speed(state->displacement_y) * dt_s;
    result.active = true;
    return result;
}
