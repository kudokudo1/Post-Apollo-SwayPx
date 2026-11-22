/*
 * V5F-G3B, DISPOSABLE COMPOSITOR ONLY.
 *
 * This probe is NEVER included by production Meson. A GitHub CI overlay
 * temporarily compiles it into a copied SwayPX to observe real native
 * mapped-XDG TARGET and enabled-OUTPUT lifetime changes.
 *
 * Device and seat ids/epochs, physical origin, hit-test and layout are
 * deliberately SYNTHETIC test values. They are NOT credentials. Real
 * target/output tokens are sampled from the actual compositor owners.
 *
 * A deliberately synthetic all-positive start is required to exercise
 * the detached G3A policy's NEGATIVE revocation paths. No input event,
 * timer, axis, frame or normal runtime activation exists in this probe.
 */
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "log.h"
#include "util.h"
#include "sway/output.h"
#include "sway/server.h"
#include "sway/tree/container.h"
#include "sway/tree/root.h"
#include "sway/tree/view.h"
#include "sway/input/post_apollo_autoscroll_continuity.h"

/* Synthetic fixture values are distinctive and cannot be misreported
 * as compositor-generated physical input/device/seat/layout authority. */
#define G3B_TEST_DEVICE_ID UINT64_C(90011)
#define G3B_TEST_SEAT_ID UINT64_C(90012)
#define G3B_TEST_DEVICE_EPOCH UINT64_C(99011)
#define G3B_TEST_SEAT_EPOCH UINT64_C(99012)
#define G3B_TEST_LAYOUT_GENERATION UINT64_C(99013)

static struct {
    struct pa_autoscroll_continuity_guard guard;
    struct sway_container *target;
    struct sway_output *output;
} g3b;

static uint64_t next_test_tick(void) {
    /* This is TEST timer data, not a physical input-event timestamp. */
    uint64_t now = get_current_time_in_msec();
    if (now <= g3b.guard.last_tick_ms) {
        now = g3b.guard.last_tick_ms + 1;
    }
    return now;
}

static struct pa_autoscroll_continuity_scene current_scene(uint64_t now) {
    struct pa_autoscroll_continuity_scene scene = {0};
    const struct pa_autoscroll_witness *started = &g3b.guard.start.started;

    /* These two owner epochs come DIRECTLY from live native objects. */
    scene.owners.device = G3B_TEST_DEVICE_EPOCH;
    scene.owners.seat = G3B_TEST_SEAT_EPOCH;
    scene.owners.target = container_autoscroll_target_lifetime_epoch(g3b.target);
    scene.owners.output = output_autoscroll_lifetime_epoch(g3b.output);

    /* These values deliberately do NOT establish real physical authority,
     * hit-testing, target-output association, or Scroll Lock semantics.
     * They are frozen synthetic fixtures to isolate owner-lifetime denial.
     */
    scene.device_id = G3B_TEST_DEVICE_ID;
    scene.seat_id = G3B_TEST_SEAT_ID;
    scene.target_node_id = g3b.target == NULL ? 0 : g3b.target->node.id;
    scene.output_id = g3b.output == NULL ? 0 : g3b.output->node.id;
    scene.layout_generation = G3B_TEST_LAYOUT_GENERATION;
    scene.captured_at_ms = now; /* native callback sampled just now */
    scene.surface_hit_verified = true; /* TEST FIXTURE, not a hit test */
    scene.target_mapped = scene.owners.target != 0;
    scene.output_enabled = scene.owners.output != 0;
    scene.layout_generation_verified = true; /* TEST FIXTURE */
    scene.session_locked = server.session_lock.lock != NULL;
    scene.native_scroll_lock_active = false; /* TEST FIXTURE */
    scene.stage_director_pairing_consumed = false; /* TEST FIXTURE */
    scene.pointer_constrained = false; /* TEST FIXTURE */

    /* Force the C source to refer to the original start record and make
     * accidental all-positive claims visible in source review. */
    if (started->device_id != G3B_TEST_DEVICE_ID
            || started->seat_id != G3B_TEST_SEAT_ID) {
        scene.device_id = 0;
    }
    return scene;
}

static void deny_on_native_change(const char *reason) {
    if (!g3b.guard.active || g3b.target == NULL || g3b.output == NULL) {
        return;
    }
    uint64_t now = next_test_tick();
    struct pa_autoscroll_continuity_scene scene = current_scene(now);
    uint64_t before_target = g3b.guard.start.started_epochs.target;
    uint64_t before_output = g3b.guard.start.started_epochs.output;
    bool admitted = pa_autoscroll_continuity_tick(&g3b.guard, &scene, now);
    if (!admitted && !g3b.guard.active && !g3b.guard.start.active) {
        sway_log(SWAY_INFO, "PA_G3B_NATIVE_DENY reason=%s "
            "old_target=%" PRIu64 " now_target=%" PRIu64 " "
            "old_output=%" PRIu64 " now_output=%" PRIu64 " "
            "canceled=1 axis_frames=0",
            reason, before_target, scene.owners.target,
            before_output, scene.owners.output);
    } else {
        sway_log(SWAY_ERROR, "PA_G3B_NATIVE_POLICY_DENIAL_FAILED "
            "reason=%s admitted=%d", reason, admitted);
        pa_autoscroll_continuity_cancel(&g3b.guard);
    }
    /* The next XDG map can use a fresh synthetic test begin; the previous
     * session does NOT automatically rearm on this callback. */
    g3b.target = NULL;
    g3b.output = NULL;
}

void pa_g3b_native_target_bound(struct sway_container *target) {
    if (target == NULL || target->view == NULL
            || target->view->surface == NULL
            || root == NULL || root->outputs == NULL
            || root->outputs->length != 1 || g3b.guard.active) {
        return;
    }
    struct sway_output *output = root->outputs->items[0];
    uint64_t target_epoch = container_autoscroll_target_lifetime_epoch(target);
    uint64_t output_epoch = output_autoscroll_lifetime_epoch(output);
    if (!target_epoch || !output_epoch || output == root->fallback_output) {
        return;
    }
    /* Begin uses EXACT real target/output owner epochs but the following
     * data-policy credential is SYNTHETIC; never label this as hardware. */
    uint64_t now = get_current_time_in_msec();
    if (now == 0) {
        return;
    }
    struct pa_autoscroll_witness synthetic = {
        .origin = PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT, /* TEST FORGERY */
        .compositor_event = true, /* TEST FIXTURE */
        .surface_hit_verified = true, /* TEST FIXTURE */
        .target_mapped = true,
        .output_enabled = true,
        .layout_generation_verified = true, /* TEST FIXTURE */
        .session_locked = false, /* ONLY a synthetic start */
        .native_scroll_lock_active = false, /* TEST FIXTURE */
        .stage_director_pairing_consumed = false, /* TEST FIXTURE */
        .pointer_constrained = false, /* TEST FIXTURE */
        .device_id = G3B_TEST_DEVICE_ID,
        .seat_id = G3B_TEST_SEAT_ID,
        .target_node_id = target->node.id,
        .output_id = output->node.id,
        .layout_generation = G3B_TEST_LAYOUT_GENERATION,
        .observed_at_ms = now, /* TEST FIXTURE, no actual pointer event */
    };
    struct pa_autoscroll_object_epochs owners = {
        .device = G3B_TEST_DEVICE_EPOCH,
        .seat = G3B_TEST_SEAT_EPOCH,
        .target = target_epoch,
        .output = output_epoch,
    };
    g3b.target = target;
    g3b.output = output;
    if (!pa_autoscroll_continuity_begin(
            &g3b.guard, &synthetic, &owners, now)) {
        sway_log(SWAY_ERROR, "PA_G3B_NATIVE_SYNTHETIC_START_REFUSED");
        g3b.target = NULL;
        g3b.output = NULL;
        return;
    }
    /* One test-only baseline check, before real owner lifecycle changes. */
    uint64_t tick = next_test_tick();
    struct pa_autoscroll_continuity_scene scene = current_scene(tick);
    if (!pa_autoscroll_continuity_tick(&g3b.guard, &scene, tick)) {
        sway_log(SWAY_ERROR, "PA_G3B_NATIVE_BASELINE_POLICY_FAILED");
        g3b.target = NULL;
        g3b.output = NULL;
        return;
    }
    sway_log(SWAY_INFO, "PA_G3B_NATIVE_SYNTHETIC_BASELINE "
        "node=%" PRIu64 " target_epoch=%" PRIu64
        " output_epoch=%" PRIu64
        " device_seat=TEST_ONLY origin=FORGED_TEST_ONLY axis_frames=0",
        (uint64_t)target->node.id, target_epoch, output_epoch);
}

void pa_g3b_native_target_revoked(struct sway_container *target) {
    if (target == g3b.target) {
        deny_on_native_change("target_unmap");
    }
}

void pa_g3b_native_output_changed(struct sway_output *output, int reason) {
    if (output == g3b.output) {
        if (reason == 1) {
            deny_on_native_change("output_reconfigure");
        } else if (reason == 2) {
            deny_on_native_change("output_disable");
        }
    }
}
