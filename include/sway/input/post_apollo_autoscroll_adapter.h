#ifndef SWAY_INPUT_POST_APOLLO_AUTOSCROLL_ADAPTER_H
#define SWAY_INPUT_POST_APOLLO_AUTOSCROLL_ADAPTER_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/input/post_apollo_autoscroll.h"

/*
 * Isolated adapter for compositor-owned event ingress. This is NOT linked
 * into SwayPX and never registers input listeners, timers or bindings.
 * Identifiers/hit-test facts must come from trusted compositor code,
 * never a Wayland client or unverified IPC reply.
 */
#define PA_AUTOSCROLL_BTN_LEFT 272u
#define PA_AUTOSCROLL_BTN_RIGHT 273u
#define PA_AUTOSCROLL_BTN_MIDDLE 274u

struct pa_autoscroll_adapter {
    struct pa_autoscroll_state engine;
    bool enabled; /* Always false after initialization */
    uint64_t device_id;
    uint64_t seat_id;
    bool middle_release_pending;
    uint64_t middle_release_device;
    bool other_release_pending;
    uint32_t other_release_button;
    uint64_t other_release_device;
};

typedef void (*pa_autoscroll_axis_sink)(
    void *context, double horizontal_pixels, double vertical_pixels);

void pa_autoscroll_adapter_init(struct pa_autoscroll_adapter *adapter);
void pa_autoscroll_adapter_configure(struct pa_autoscroll_adapter *adapter,
                                    bool explicitly_enabled);
void pa_autoscroll_adapter_cancel(struct pa_autoscroll_adapter *adapter);

/*
 * Handle physical middle-click press/release. True means the compositor
 * would suppress the normal button event. False means pass it unchanged.
 *
 * Default OFF passes every event. During an active session, only a middle
 * press from the same source device and seat can toggle off.
 */
bool pa_autoscroll_adapter_middle(
    struct pa_autoscroll_adapter *adapter, bool pressed,
    bool trusted_physical_pointer, uint64_t device_id, uint64_t seat_id,
    bool verified_surface_hit, uint64_t target_node_id,
    uint64_t layout_generation, double pointer_x, double pointer_y,
    uint64_t monotonic_ms, bool session_locked,
    bool native_scroll_lock_active);

/* Any left/right physical click cancels; its press/release are both swallowed
 * to avoid unexpectedly activating a UI control at the anchored pointer.
 */
bool pa_autoscroll_adapter_other_button(
    struct pa_autoscroll_adapter *adapter, uint32_t button, bool pressed,
    bool trusted_physical_pointer, uint64_t device_id, uint64_t seat_id);

/*
 * While active, consumed pointer motion changes speed but not pointer
 * location. Only a trusted same-device/seat compositor event may drive it.
 */
bool pa_autoscroll_adapter_motion(
    struct pa_autoscroll_adapter *adapter,
    bool trusted_physical_pointer, uint64_t device_id, uint64_t seat_id,
    double dx, double dy, bool session_locked,
    bool native_scroll_lock_active);

/* Escape cancellation only; no global keyboard listener is installed. */
bool pa_autoscroll_adapter_escape(
    struct pa_autoscroll_adapter *adapter,
    bool trusted_escape_pressed, uint64_t seat_id);

/*
 * Standalone intent: caller must supply current compositor hit-tested target
 * and layout generation. The sink is only a mockable callback and is never
 * wired to Wayland axis emission in this PR.
 */
bool pa_autoscroll_adapter_tick(
    struct pa_autoscroll_adapter *adapter,
    uint64_t device_id, uint64_t seat_id,
    bool verified_surface_hit, uint64_t target_node_id,
    uint64_t layout_generation, uint64_t monotonic_ms,
    bool session_locked, bool native_scroll_lock_active,
    pa_autoscroll_axis_sink sink, void *sink_context);

#endif
