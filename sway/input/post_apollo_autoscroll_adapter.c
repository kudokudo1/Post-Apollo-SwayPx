/*
 * Standalone compositor-input adapter for Post-Apollo autoscroll.
 *
 * THIS FILE IS NOT WIRED TO MESON, cursor.c, seat.c, IPC, input devices,
 * keyboard dispatch, timer sources or Wayland event emission.
 */
#include <stddef.h>
#include <string.h>
#include "sway/input/post_apollo_autoscroll_adapter.h"

static bool same_owner(const struct pa_autoscroll_adapter *adapter,
                       uint64_t device_id, uint64_t seat_id) {
    return adapter != NULL && device_id != 0 && seat_id != 0
        && adapter->device_id == device_id && adapter->seat_id == seat_id;
}

void pa_autoscroll_adapter_init(struct pa_autoscroll_adapter *adapter) {
    if (adapter == NULL) {
        return;
    }
    memset(adapter, 0, sizeof(*adapter));
    pa_autoscroll_init(&adapter->engine);
    /* enabled remains FALSE; there is no production path to set it. */
}

void pa_autoscroll_adapter_cancel(struct pa_autoscroll_adapter *adapter) {
    if (adapter == NULL) {
        return;
    }
    pa_autoscroll_cancel(&adapter->engine);
    adapter->device_id = 0;
    adapter->seat_id = 0;
    /*
     * Do not discard a consumed press's pending matching release. Letting
     * just that release leak to another app would violate input pairing.
     */
}

void pa_autoscroll_adapter_configure(struct pa_autoscroll_adapter *adapter,
                                    bool explicitly_enabled) {
    if (adapter == NULL) {
        return;
    }
    if (!explicitly_enabled) {
        pa_autoscroll_adapter_cancel(adapter);
    }
    adapter->enabled = explicitly_enabled;
}

bool pa_autoscroll_adapter_middle(
    struct pa_autoscroll_adapter *adapter, bool pressed,
    bool trusted_physical_pointer, uint64_t device_id, uint64_t seat_id,
    bool verified_surface_hit, uint64_t target_node_id,
    uint64_t layout_generation, double pointer_x, double pointer_y,
    uint64_t monotonic_ms, bool session_locked,
    bool native_scroll_lock_active) {
    if (adapter == NULL) {
        return false;
    }
    if (!pressed && adapter->middle_release_pending
            && device_id == adapter->middle_release_device) {
        adapter->middle_release_pending = false;
        adapter->middle_release_device = 0;
        return true;
    }
    if (pressed && adapter->middle_release_pending
            && device_id == adapter->middle_release_device) {
        /* Ignore duplicate physical down before paired release. */
        return true;
    }
    if (!adapter->enabled) {
        return false;
    }
    if (!trusted_physical_pointer || device_id == 0 || seat_id == 0
            || session_locked || native_scroll_lock_active) {
        if (adapter->engine.active) {
            pa_autoscroll_adapter_cancel(adapter);
        }
        return false;
    }
    if (!pressed) {
        return false;
    }
    if (adapter->engine.active) {
        if (!same_owner(adapter, device_id, seat_id)) {
            pa_autoscroll_adapter_cancel(adapter);
            return false;
        }
        pa_autoscroll_adapter_cancel(adapter);
        adapter->middle_release_pending = true;
        adapter->middle_release_device = device_id;
        return true;
    }
    if (!verified_surface_hit || target_node_id == 0
            || layout_generation == 0 || monotonic_ms == 0) {
        return false;
    }
    if (!pa_autoscroll_begin(&adapter->engine, adapter->enabled,
            verified_surface_hit, target_node_id, layout_generation,
            pointer_x, pointer_y, monotonic_ms)) {
        return false;
    }
    adapter->device_id = device_id;
    adapter->seat_id = seat_id;
    adapter->middle_release_pending = true;
    adapter->middle_release_device = device_id;
    return true;
}

bool pa_autoscroll_adapter_other_button(
    struct pa_autoscroll_adapter *adapter, uint32_t button, bool pressed,
    bool trusted_physical_pointer, uint64_t device_id, uint64_t seat_id) {
    if (adapter == NULL ||
            (button != PA_AUTOSCROLL_BTN_LEFT
             && button != PA_AUTOSCROLL_BTN_RIGHT)) {
        return false;
    }
    if (!pressed && adapter->other_release_pending
            && button == adapter->other_release_button
            && device_id == adapter->other_release_device) {
        adapter->other_release_pending = false;
        adapter->other_release_button = 0;
        adapter->other_release_device = 0;
        return true;
    }
    if (pressed && adapter->other_release_pending
            && button == adapter->other_release_button
            && device_id == adapter->other_release_device) {
        return true;
    }
    if (!adapter->engine.active) {
        return false;
    }
    bool owner = trusted_physical_pointer
        && same_owner(adapter, device_id, seat_id);
    pa_autoscroll_adapter_cancel(adapter);
    if (!pressed || !owner) {
        return false;
    }
    adapter->other_release_pending = true;
    adapter->other_release_button = button;
    adapter->other_release_device = device_id;
    return true;
}

bool pa_autoscroll_adapter_motion(
    struct pa_autoscroll_adapter *adapter,
    bool trusted_physical_pointer, uint64_t device_id, uint64_t seat_id,
    double dx, double dy, bool session_locked,
    bool native_scroll_lock_active) {
    if (adapter == NULL || !adapter->engine.active) {
        return false;
    }
    if (!adapter->enabled || !trusted_physical_pointer
            || !same_owner(adapter, device_id, seat_id)
            || session_locked || native_scroll_lock_active) {
        pa_autoscroll_adapter_cancel(adapter);
        return false;
    }
    pa_autoscroll_move(&adapter->engine, dx, dy);
    /* Even invalid nonfinite motion is consumed before cancellation. */
    return true;
}

bool pa_autoscroll_adapter_escape(
    struct pa_autoscroll_adapter *adapter,
    bool trusted_escape_pressed, uint64_t seat_id) {
    if (adapter == NULL || !adapter->engine.active
            || !trusted_escape_pressed || seat_id == 0
            || seat_id != adapter->seat_id) {
        return false;
    }
    pa_autoscroll_adapter_cancel(adapter);
    return true;
}

bool pa_autoscroll_adapter_tick(
    struct pa_autoscroll_adapter *adapter,
    uint64_t device_id, uint64_t seat_id,
    bool verified_surface_hit, uint64_t target_node_id,
    uint64_t layout_generation, uint64_t monotonic_ms,
    bool session_locked, bool native_scroll_lock_active,
    pa_autoscroll_axis_sink sink, void *sink_context) {
    if (adapter == NULL || !adapter->engine.active) {
        return false;
    }
    if (!adapter->enabled || !same_owner(adapter, device_id, seat_id)
            || !verified_surface_hit || session_locked
            || native_scroll_lock_active || sink == NULL) {
        pa_autoscroll_adapter_cancel(adapter);
        return false;
    }
    struct pa_autoscroll_delta delta = pa_autoscroll_tick(
        &adapter->engine, target_node_id, layout_generation, monotonic_ms);
    if (!delta.active) {
        adapter->device_id = 0;
        adapter->seat_id = 0;
        return false;
    }
    if (delta.horizontal_pixels != 0.0 || delta.vertical_pixels != 0.0) {
        sink(sink_context, delta.horizontal_pixels, delta.vertical_pixels);
    }
    return true;
}
