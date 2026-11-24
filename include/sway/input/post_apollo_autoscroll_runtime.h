#ifndef SWAY_INPUT_POST_APOLLO_AUTOSCROLL_RUNTIME_H
#define SWAY_INPUT_POST_APOLLO_AUTOSCROLL_RUNTIME_H

#include <stdbool.h>
#include <stdint.h>
#include <wayland-server-core.h>

/* Compositor private. No public IPC, device permissions, or activation API. */
struct sway_cursor;
struct wlr_input_device;
struct pa_autoscroll_runtime;
struct pa_autoscroll_origin_facts;

struct pa_autoscroll_runtime *pa_autoscroll_runtime_create(
    struct sway_cursor *cursor);
void pa_autoscroll_runtime_destroy(struct pa_autoscroll_runtime *runtime);

/* All ingress functions are no-ops while the compile-time hard gate is OFF. */
bool pa_autoscroll_runtime_button(struct pa_autoscroll_runtime *runtime,
    struct wlr_input_device *device, uint32_t timestamp, uint32_t button,
    bool pressed, const struct pa_autoscroll_origin_facts *origin);
bool pa_autoscroll_runtime_motion(struct pa_autoscroll_runtime *runtime,
    struct wlr_input_device *device, double dx, double dy);
void pa_autoscroll_runtime_cancel(struct pa_autoscroll_runtime *runtime);

#endif
