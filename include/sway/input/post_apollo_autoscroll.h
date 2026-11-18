#ifndef SWAY_INPUT_POST_APOLLO_AUTOSCROLL_H
#define SWAY_INPUT_POST_APOLLO_AUTOSCROLL_H

#include <stdbool.h>
#include <stdint.h>

/*
 * SwayPX internal, nonactivated autoscroll core.
 * No compositor hooks, IPC, Wayland surface, keyboard capture or event
 * injection are installed by this module. Only the compositor event owner
 * may eventually create a session after a real middle-button scene hit-test.
 */
struct pa_autoscroll_state {
    bool active;
    uint64_t target_node_id;
    uint64_t layout_generation;
    uint64_t start_ms;
    uint64_t last_tick_ms;
    double anchor_x;
    double anchor_y;
    double displacement_x;
    double displacement_y;
    uint64_t generation;
};

struct pa_autoscroll_delta {
    double horizontal_pixels;
    double vertical_pixels;
    bool active;
};

/* Zero-initialize before use, then increment generation across sessions. */
void pa_autoscroll_init(struct pa_autoscroll_state *state);

/*
 * Begin a session only when called by a future opt-in compositor input owner,
 * with a positive scene hit-test target, output-layout generation, and finite
 * compositor cursor coordinates. This is NOT a public trust/authentication API.
 */
bool pa_autoscroll_begin(struct pa_autoscroll_state *state,
                        bool compositor_mode_enabled,
                        bool real_pointer_target,
                        uint64_t target_node_id,
                        uint64_t layout_generation,
                        double anchor_x, double anchor_y,
                        uint64_t monotonic_ms);

/* Displacement is relative to the anchored physical cursor location. */
void pa_autoscroll_move(struct pa_autoscroll_state *state,
                        double delta_x, double delta_y);

/*
 * Emit scroll-distance intent only, not Linux/Wayland input.
 * Abort on target/layout generation change, stale tick, or expiry.
 */
struct pa_autoscroll_delta pa_autoscroll_tick(
                        struct pa_autoscroll_state *state,
                        uint64_t current_target_node_id,
                        uint64_t current_layout_generation,
                        uint64_t monotonic_ms);

void pa_autoscroll_cancel(struct pa_autoscroll_state *state);

#endif
