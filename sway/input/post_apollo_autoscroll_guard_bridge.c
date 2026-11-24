/*
 * V5F-B negative-only bridge from actual compositor owner lifetimes to the
 * exact V5C epoch guard. A complete four-token snapshot is NOT a physical
 * input credential or a trusted layout generation.
 *
 * Do not promote native libinput candidacy to PHYSICAL_LIBINPUT. A kernel
 * synthetic HID can still look bus-backed; positive provenance needs
 * independent attestation and a fresh callback-produced witness.
 */
#include <stddef.h>
#include <stdint.h>

#include "sway/input/post_apollo_autoscroll_guard_bridge.h"
#include "sway/input/post_apollo_autoscroll_owner_snapshot.h"

static struct pa_autoscroll_object_epochs owner_epochs(
        const struct pa_autoscroll_owner_snapshot *owner) {
    return (struct pa_autoscroll_object_epochs) {
        .device = owner->device,
        .seat = owner->seat,
        .target = owner->target,
        .output = owner->output,
    };
}

static struct pa_autoscroll_witness native_unverified_witness(
        const struct pa_autoscroll_owner_snapshot *owner,
        uint64_t event_at_ms) {
    return (struct pa_autoscroll_witness) {
        /* Neither a libinput candidate nor udev syspath makes this true. */
        .origin = PA_POINTER_ORIGIN_UNKNOWN,
        .compositor_event = event_at_ms != 0,
        .surface_hit_verified = true, /* direct live owner snapshot ONLY */
        .target_mapped = true,
        .output_enabled = true,
        /* Enabled output generation does not prove COMPLETE scene layout. */
        .layout_generation_verified = false,
        .native_scroll_lock_active = true, /* independent gate UNKNOWN */
        .device_id = owner->device,
        .seat_id = owner->seat,
        .target_node_id = owner->target,
        .output_id = owner->output,
        .layout_generation = 0,
        .observed_at_ms = event_at_ms,
    };
}

void pa_autoscroll_guard_bridge_cancel(
        struct pa_autoscroll_guard_bridge *bridge) {
    if (bridge != NULL) {
        pa_autoscroll_epoch_cancel(&bridge->epoch);
    }
}

bool pa_autoscroll_guard_bridge_begin(
        struct pa_autoscroll_guard_bridge *bridge,
        struct sway_cursor *cursor, struct wlr_input_device *device,
        uint64_t event_at_ms, uint64_t now_ms) {
    if (bridge == NULL) {
        return false;
    }
    pa_autoscroll_guard_bridge_cancel(bridge);
    struct pa_autoscroll_owner_snapshot owner = {0};
    if (!pa_autoscroll_owner_snapshot_read(cursor, device, &owner)) {
        return false;
    }
    const struct pa_autoscroll_object_epochs epochs = owner_epochs(&owner);
    const struct pa_autoscroll_witness evidence =
        native_unverified_witness(&owner, event_at_ms);
    /* Expected to FAIL: origin UNKNOWN, layout unverified, Scroll Lock
     * unresolved. No positive witness can be supplied by an IPC caller.
     * This is wired for safety, not an activation or permission API. */
    if (!pa_autoscroll_epoch_begin(
            &bridge->epoch, &evidence, &epochs, now_ms)) {
        pa_autoscroll_guard_bridge_cancel(bridge);
        return false;
    }
    /* Defense in depth: the native witness has NO path to admit a click.
     * A future positive implementation needs a separately reviewed
     * physical-origin, lock-state and scene/layout attestation contract. */
    pa_autoscroll_guard_bridge_cancel(bridge);
    return false;
}

bool pa_autoscroll_guard_bridge_tick(
        struct pa_autoscroll_guard_bridge *bridge,
        struct sway_cursor *cursor, struct wlr_input_device *device,
        uint64_t now_ms) {
    if (bridge == NULL) {
        return false;
    }
    struct pa_autoscroll_owner_snapshot owner = {0};
    if (!pa_autoscroll_owner_snapshot_read(cursor, device, &owner)) {
        pa_autoscroll_guard_bridge_cancel(bridge);
        return false;
    }
    const struct pa_autoscroll_object_epochs epochs = owner_epochs(&owner);
    /* A timer is NOT a new physical pointer event. Never invent an
     * observed_at_ms from now_ms just to satisfy V5C freshness checks. */
    const struct pa_autoscroll_witness evidence =
        native_unverified_witness(&owner, 0);
    if (!pa_autoscroll_epoch_tick(
            &bridge->epoch, &evidence, &epochs, now_ms)) {
        pa_autoscroll_guard_bridge_cancel(bridge);
        return false;
    }
    /* No positive native provenance/lock/layout producer exists. */
    pa_autoscroll_guard_bridge_cancel(bridge);
    return false;
}
