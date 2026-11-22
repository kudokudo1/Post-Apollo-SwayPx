/*
 * V5F-G3C four-NATIVE-owner negative evidence, disposable CI ONLY.
 *
 * Real Wayland virtual-pointer/transient-seat and real XDG/output owners
 * issue the FOUR lifetime epochs. The virtual pointer ALWAYS remains
 * physical-ineligible. A deliberately FAKE positive start-event and scene
 * flags exercise the detached G3A cancellation policy, NOT authorization.
 *
 * Only a copied CI SwayPX binary links this probe. No axis, frame, timer,
 * IPC authority, real HID, production runtime edit, or input emission.
 */
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include "log.h"
#include "util.h"
#include "sway/input/input-manager.h"
#include "sway/input/seat.h"
#include "sway/input/post_apollo_autoscroll_continuity.h"
#include "sway/output.h"
#include "sway/server.h"
#include "sway/tree/container.h"
#include "sway/tree/root.h"
#include "sway/tree/view.h"

#define G3C_FAKE_LAYOUT UINT64_C(99013)

static struct {
    struct sway_input_device *device;
    struct sway_seat *seat;
    struct sway_container *target;
    struct sway_output *output;
    struct pa_autoscroll_continuity_guard guard;
} g3c;

static struct pa_autoscroll_object_epochs native_owners(void) {
    /* Read raw owner-issued device epoch, but NEVER convert a virtual
     * pointer into an eligible physical pointer. That accessor stays 0.
     */
    struct pa_autoscroll_object_epochs e = {0};
    if (g3c.device != NULL) {
        e.device = pa_autoscroll_lifetime_epoch(
            &g3c.device->autoscroll_lifetime, PA_LIFETIME_DEVICE);
    }
    if (g3c.seat != NULL) {
        e.seat = seat_autoscroll_lifetime_epoch(g3c.seat);
    }
    if (g3c.target != NULL) {
        e.target = container_autoscroll_target_lifetime_epoch(g3c.target);
    }
    if (g3c.output != NULL) {
        e.output = output_autoscroll_lifetime_epoch(g3c.output);
    }
    return e;
}

static uint64_t tick_time(void) {
    uint64_t now = get_current_time_in_msec();
    if (now <= g3c.guard.last_tick_ms) {
        now = g3c.guard.last_tick_ms + 1;
    }
    return now;
}

static struct pa_autoscroll_continuity_scene scene_now(uint64_t now) {
    struct pa_autoscroll_continuity_scene s = {0};
    s.owners = native_owners();
    s.device_id = (uint64_t)(uintptr_t)g3c.device;
    s.seat_id = (uint64_t)(uintptr_t)g3c.seat;
    s.target_node_id = g3c.target == NULL ? 0 : g3c.target->node.id;
    s.output_id = g3c.output == NULL ? 0 : g3c.output->node.id;
    s.layout_generation = G3C_FAKE_LAYOUT; /* TEST ONLY */
    s.captured_at_ms = now; /* actual callback sample, not physical input */
    s.surface_hit_verified = true; /* TEST ONLY, no real hit-test */
    s.target_mapped = s.owners.target != 0;
    s.output_enabled = s.owners.output != 0;
    s.layout_generation_verified = true; /* TEST ONLY */
    s.session_locked = server.session_lock.lock != NULL;
    s.native_scroll_lock_active = false; /* TEST ONLY */
    s.stage_director_pairing_consumed = false; /* TEST ONLY */
    s.pointer_constrained = false; /* TEST ONLY */
    return s;
}

static void check_native_revoke(const char *reason) {
    if (!g3c.guard.active) {
        return;
    }
    struct pa_autoscroll_object_epochs before =
        g3c.guard.start.started_epochs;
    uint64_t now = tick_time();
    struct pa_autoscroll_continuity_scene s = scene_now(now);
    bool ok = pa_autoscroll_continuity_tick(&g3c.guard, &s, now);
    if (!ok && !g3c.guard.active && !g3c.guard.start.active) {
        sway_log(SWAY_INFO,
            "PA_G3C_NATIVE_DENY reason=%s "
            "device=%" PRIu64 ":%" PRIu64 " "
            "seat=%" PRIu64 ":%" PRIu64 " "
            "target=%" PRIu64 ":%" PRIu64 " "
            "output=%" PRIu64 ":%" PRIu64 " "
            "canceled=1 axis_frames=0",
            reason, before.device, s.owners.device,
            before.seat, s.owners.seat,
            before.target, s.owners.target,
            before.output, s.owners.output);
    } else {
        sway_log(SWAY_ERROR, "PA_G3C_NATIVE_DENIAL_FAILURE reason=%s admitted=%d",
            reason, ok);
        pa_autoscroll_continuity_cancel(&g3c.guard);
    }
}

void pa_g3c_virtual_pointer_bound(
        struct sway_input_device *device, struct sway_seat *seat) {
    if (device == NULL || !device->is_virtual || seat == NULL
            || g3c.device != NULL) {
        return;
    }
    g3c.device = device;
    g3c.seat = seat;
    struct pa_autoscroll_object_epochs e = native_owners();
    uint64_t physical = input_manager_autoscroll_pointer_lifetime_epoch(device);
    sway_log(SWAY_INFO,
        "PA_G3C_NATIVE_VIRTUAL_BIND device=%" PRIu64
        " seat=%" PRIu64 " eligible=%" PRIu64 " virtual=%d",
        e.device, e.seat, physical, device->is_virtual);
    if (physical != 0) {
        sway_log(SWAY_ERROR, "PA_G3C_VIRTUAL_MISTAKEN_AS_PHYSICAL");
    }
}

void pa_g3c_virtual_pointer_revoked(struct sway_input_device *device) {
    if (device == NULL || device != g3c.device) {
        return;
    }
    check_native_revoke("device_destroy");
    g3c.device = NULL;
    if (!g3c.guard.active) {
        g3c.target = NULL;
        g3c.output = NULL;
    }
}

void pa_g3c_transient_seat_revoked(struct sway_seat *seat) {
    if (seat == NULL || seat != g3c.seat) {
        return;
    }
    check_native_revoke("seat_destroy");
    g3c.seat = NULL;
    if (!g3c.guard.active) {
        g3c.target = NULL;
        g3c.output = NULL;
    }
}

void pa_g3c_target_bound(struct sway_container *target) {
    if (target == NULL || target->view == NULL
            || target->view->surface == NULL
            || g3c.device == NULL || g3c.seat == NULL
            || g3c.guard.active || root == NULL
            || root->outputs == NULL || root->outputs->length != 1) {
        return;
    }
    g3c.target = target;
    g3c.output = root->outputs->items[0];
    struct pa_autoscroll_object_epochs e = native_owners();
    if (g3c.output == root->fallback_output ||
            !e.device || !e.seat || !e.target || !e.output ||
            !g3c.device->is_virtual ||
            input_manager_autoscroll_pointer_lifetime_epoch(g3c.device) != 0) {
        g3c.target = NULL;
        g3c.output = NULL;
        return;
    }

    uint64_t now = get_current_time_in_msec();
    if (!now) {
        g3c.target = NULL;
        g3c.output = NULL;
        return;
    }
    struct pa_autoscroll_witness fake = {
        .origin = PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT, /* INTENTIONAL FORGERY */
        .compositor_event = true, /* TEST FIXTURE */
        .surface_hit_verified = true, /* TEST FIXTURE */
        .target_mapped = true,
        .output_enabled = true,
        .layout_generation_verified = true, /* TEST FIXTURE */
        .session_locked = false, /* TEST FIXTURE */
        .native_scroll_lock_active = false, /* TEST FIXTURE */
        .stage_director_pairing_consumed = false, /* TEST FIXTURE */
        .pointer_constrained = false, /* TEST FIXTURE */
        .device_id = (uint64_t)(uintptr_t)g3c.device,
        .seat_id = (uint64_t)(uintptr_t)g3c.seat,
        .target_node_id = target->node.id,
        .output_id = g3c.output->node.id,
        .layout_generation = G3C_FAKE_LAYOUT,
        .observed_at_ms = now, /* NO physical event occurred */
    };
    if (!pa_autoscroll_continuity_begin(&g3c.guard, &fake, &e, now)) {
        sway_log(SWAY_ERROR, "PA_G3C_NATIVE_FAKE_START_FAILURE");
        g3c.target = NULL;
        g3c.output = NULL;
        return;
    }
    uint64_t tick = tick_time();
    struct pa_autoscroll_continuity_scene scene = scene_now(tick);
    if (!pa_autoscroll_continuity_tick(&g3c.guard, &scene, tick)) {
        sway_log(SWAY_ERROR, "PA_G3C_NATIVE_FAKE_BASELINE_FAILURE");
        g3c.target = NULL;
        g3c.output = NULL;
        return;
    }
    sway_log(SWAY_INFO, "PA_G3C_NATIVE_FOUR_REAL_EPOCHS "
        "node=%" PRIu64
        " device=%" PRIu64 " seat=%" PRIu64
        " target=%" PRIu64 " output=%" PRIu64
        " virtual_eligible=0 origin=FORGED_TEST_ONLY "
        "scene_layout=FAKE_TEST_ONLY axis_frames=0",
        (uint64_t)target->node.id,
        e.device, e.seat, e.target, e.output);
}

void pa_g3c_target_revoked(struct sway_container *target) {
    if (target != NULL && target == g3c.target) {
        check_native_revoke("target_unmap");
        g3c.target = NULL;
        g3c.output = NULL;
    }
}
void pa_g3c_output_changed(struct sway_output *output, int reason) {
    if (output != NULL && output == g3c.output) {
        check_native_revoke(reason == 1 ? "output_reconfigure" : "output_disable");
        if (!g3c.guard.active) {
            /* Keep target until unmap, but clear output after disable. */
            if (reason == 2) {
                g3c.target = NULL;
                g3c.output = NULL;
            }
        }
    }
}
