/*
 * G3K native safety-check observation; DISPOSABLE CI COMPOSITOR COPY ONLY.
 * Source and scene snapshots are native. No Scroll Lock/layout attestation
 * exists yet: both remain UNKNOWN/DENY, never substituted with a guess.
 * Neither source nor four-owner lifetime is permission to consume/scroll.
 */
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <linux/input-event-codes.h>
#include <libinput.h>
#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_pointer.h>
#include <wayland-server-core.h>
#include "log.h"
#include "sway/input/cursor.h"
#include "sway/input/post_apollo_autoscroll_origin_policy.h"
#include "sway/input/post_apollo_autoscroll_source.h"
#include "sway/input/post_apollo_autoscroll_owner_snapshot.h"
#include "sway/input/post_apollo_autoscroll_guard_bridge.h"

void pa_g3k_native_guard_negative_observe(
        struct sway_cursor *cursor,
        struct wlr_pointer_button_event *event) {
    if (cursor == NULL || event == NULL || event->pointer == NULL ||
            event->button != BTN_MIDDLE) {
        return;
    }
    struct wlr_input_device *device = &event->pointer->base;
    if (!wlr_input_device_is_libinput(device)) {
        return;
    }
    struct libinput_device *handle = wlr_libinput_get_device_handle(device);
    if (handle == NULL || libinput_device_get_name(handle) == NULL ||
            strcmp(libinput_device_get_name(handle),
            "PA-G3J-KERNEL-SYNTHETIC-NOT-PHYSICAL") != 0) {
        return;
    }

    struct pa_autoscroll_owner_snapshot snapshot = {0};
    const bool owner_valid = pa_autoscroll_owner_snapshot_read(
        cursor, device, &snapshot);
    const struct pa_autoscroll_origin_facts source =
        pa_autoscroll_source_probe(device);
    const bool source_candidate = pa_autoscroll_origin_classify(&source) ==
        PA_AUTOSCROLL_ORIGIN_LIBINPUT_CANDIDATE;

    /* Actual native guard bridge checks owner lifetimes, but its witness
     * has origin UNKNOWN, layout UNVERIFIED and Scroll Lock UNKNOWN/DENY.
     * Both begin and tick MUST fail even for a real mapped XDG target.
     * The local bridge is never connected to a live autoscroll runtime. */
    struct pa_autoscroll_guard_bridge guard = {0};
    const bool began = pa_autoscroll_guard_bridge_begin(
        &guard, cursor, device, event->time_msec, event->time_msec);
    const bool ticked = pa_autoscroll_guard_bridge_tick(
        &guard, cursor, device, event->time_msec);
    pa_autoscroll_guard_bridge_cancel(&guard);
    sway_log(SWAY_INFO,
        "PA_G3K_NATIVE_GUARD_NEGATIVE phase=%d "
        "native_libinput=1 owner=%d "
        "target_epoch=%" PRIu64 " output_epoch=%" PRIu64
        " source_candidate=%d began=%d ticked=%d active=%d "
        "layout_witness=UNKNOWN_DENY scroll_lock=UNKNOWN_DENY "
        "physical_origin=UNKNOWN_DENY can_consume=0 can_axis=0",
        event->state == WL_POINTER_BUTTON_STATE_PRESSED,
        owner_valid, snapshot.target, snapshot.output,
        source_candidate,began,ticked,guard.epoch.active);
}
